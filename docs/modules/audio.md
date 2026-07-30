# Audio

The audio module owns the I2S service loop, performs bank-wide reads and writes, and exposes a managed callback interface.

The generated API section below is rebuilt from Doxygen-style comments in the public headers. Edit the header comments, then run:

```bash
python3 shared/scripts/gen_api_docs.py
```

--8<-- "reference/generated/audio_api.md"

## Design notes

Write long-form explanations here: callback lifecycle, bank mapping, ping-pong buffers, startup warmup, and timing diagnostics.

## Channels
The ESP32 has two I2S instances, both of them full-duplex ready, meaning 4 input and 4 output channels. The audio module abstracts these as channels. It is set up in a way that the amount of input and output channels is always the same, if both are enabled. Otherwise, the disabled mode has 0 channels.

## Channel sync
The system assumes that one of the two I2S banks is the host and the other one the device chained to it with the same pins. Since pins on the ESP32 can be read and written to at the same time, this becomes feasible. This enables perfect sync; a feature which the ESP-IDF does not expose by software. One can provide a config without a host. In that case, an external codec or other MCU has to provide the BCLK and WS lines. 

## On queues
The audio module shares queue usage with the ESP-IDF functions and itself. For this reason, it holds a queue list (array) of two I2S queues (which the ESP-IDF needs for the I2S events) and a custom control queue, totaling to 3 queues. To access them in an event-driven context, they are put into a queue set, which is also part of the audio meta object.

## Functions

### `static inline e_syserr_t __audio_bank_deinit(audio_bank_t* audio_bank)`
Uninstall the I2S config for a given bank. This function takes the pointer to a bank config holding descriptors for the i2s peripheral (called a "bank") and undoes its config with the ESP-level `i2s_driver_uninstall()`. It checks if an event queue associated with the bank exists (it should always exist!) and calls  `audio_clear()`. Then, the bank specific event queue is removed from the queue set and the queue handle set to `NULL`.

### `static inline e_syserr_t __audio_bank_init(audio_bank_t* audio_bank, const audio_settings_t* settings)`
Install the I2S config for a given bank. This function checks for valid parameters in the settings struct and translates them into ESP-I2S compatible data structs. It then installs the driver with the event queue, sets the I2S pins and adds the event queue to the queue set.

### `void audio_clear(void)`
Clear the audio event queue. The function takes the global audio meta object and selects all active queues in iterations from the queue set. It drains queue elements by receiving them in rapid succession and not using them. This is useful for when the queue should not process data anymore that has been sent before `audio_clear()` was called.

### `static inline uint8_t __audio_bank_nch(audio_bank_t* bank)`
Convert a bank config into the amount of channels it serves. The config is set up for mono and stereo. All this function does is convert the "mono" enum to "1" and "stereo" to "2". The enums are not already these values because they abstract the channel configuration macros of the ESP-IDF.

### `static inline audio_sample_base_t __audio_sample_from_bytes(uint8_t* p, uint8_t bps)`
Convert raw i2s byte data into audio samples. correctly uses the given bps variable to compute the raw DMA data dump into samples of type `audio_sample_base_t`. `*p` is not the whole audio buffer as cast. It is a pointer to the position of a single sample in memory. It needs to be a pointer because samples are greater than a byte (except for when `bps = 8`).

### `static inline void __audio_sample_to_bytes(uint8_t* p, audio_sample_base_t v, uint8_t bps)`
Convert audio samples into raw i2s byte data. Opposite of function above.

### `static e_syserr_t __audio_read(audio_bank_t* bank, audio_sample_t* data, audio_sample_t* scratch, uint32_t len, uint8_t bps, const uint8_t* audio_ch_idx)`
Read audio data from an i2s bank. The scratch array exists to have modifiable memory space that can either be a dedicated buffer or the same buffer as the final output to save memory. The function parses the correct amount of channels in the bank and the correct I2S bank enum before calling `i2s_read()`. Read data is then converted with `__audio_sample_from_bytes()` and soft-transposed into the multichannel audio struct memory layout. Soft-transpose: double `for`-loop. The I2S idle block time is `AUDIO_I2S_READ_TIMEOUT_TICKS`.

### `static e_syserr_t __audio_write(audio_bank_t* bank, audio_sample_t *data, audio_sample_t* scratch, uint32_t len, uint8_t bps, const uint8_t* audio_ch_idx)`
Write audio data to an i2s bank. Opposite of above. The I2S idle block time is `AUDIO_I2S_WRITE_TIMEOUT_TICKS`.

### `e_syserr_t audio_init(audio_settings_t settings, audio_bank_t audio_banks[], uint8_t num_banks)`
Initializes the audio module. Checks parameters of settings for boundaries. Tries to get `audio_sampler()` job handle. If it does not exist, it gets registered. If the event queue set does not exist, it is created and given to the audio meta object. If the control queue is not in the queue list, it is added and also added to the queue set. If the sampler is running (meaning the sampler job has an instance), `AUDIO_CTRL_EVT_STOP` is sent to it, which soft-suspends `audio_sampler()`, meaning events are received but not acted out on. If that takes longer than  `AUDIO_SAMPLER_STOP_TIMEOUT_MS`, an error is returned. Per default, both I2S banks are deactivated with `__audio_bank_deinit()` and then reactivated with `__audio_bank_init()`. The process fails if more than one host exists or warns if no host exists, see [Channel sync](#channel-sync). Settings are applied and stored if everything up to this point succeeded. Next, timing (if enabled) is reset and the DMA buffers are "warmed up" with `i2s_zero_dma_buffer()` and then waited for with a delay of `AUDIO_I2S_CLOCK_SETTLE_MS`. Then `AUDIO_I2S_WARMUP_BLOCKS` are ran as dummy data to sync the i2s peripheral (*how useful this is has yet to be understood*). Warmup routine calls `__audio_read()` and `__audio_write()`. After warmup, `audio_clear()` is called and the function is done. If `audio_job->instances` was more than 0 before (a.k.a. a restart), the job is relaunched if its current amount of instances is 0.

### `e_syserr_t audio_init_default(void)`
Initializes the I2S audio interface with default values. The defaults can be found in `audio_default_cfg.h`. These can also be overridden by providing them in `platformio.ini` as build flag macros. 

### `void audio_sampler(void* p)`
Manages the queue ISR for audio I/O. Is a jescore job. Sets up ping pong buffering and channels sync. Sync means equal per sample timesteps for both input and output and across channels. Calls `audio_clear()` on entry for redundancy. Has `while(1)` which can only be broken by `AUDIO_CTRL_EVT_STOP`. Selects the last active queue from the set and subsequently drains them. The draining mechanism is in place because it can be assumed that a hardware sync entails that the events RX1/TX1/RX2/TX2 happen at nearly the same time, which means that it is smarter to prophylactically handle all queues in one time block if ready. If the control queue is ready, a switch-case handles the events `AUDIO_CTRL_EVT_STOP` and `AUDIO_CTRL_EVT_SET_CALLBACK` (a new callback was set and is applied to the private callback address copy of the sampler). If all control events are handled, the I2S bank events are matched with the active queue to find out from what hardware peripheral data arrived. If the events are RX/TX, RX and TX ready flags are set (and timed if enabled). Anything else falls through and causes a jescore error throw in the sampler job and a debug print with the exact error code.

The queue segment is now over and all messages have been extracted. 