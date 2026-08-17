# Audio

The audio module owns the ESP32 I2S service loop, performs I2S bank-wide reads and
writes, and exposes a managed realtime callback Interface. The public API is
small on purpose: applications configure topology, start/stop the sampler, and
process sample blocks at the callback seam. The Implementation keeps DMA events,
queue sets, packing/unpacking, gain, timing counters, and CLI control local to
the module.

!!! tip "Quickstart"
    Enable the module and define the default audio IO shape and pins in your PlatformIO environment. Simple macros describe hardware by input/output channel counts; ESP32 I2S bank mapping is derived internally.

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_AUDIO
        -DAUDIO_IO_IN_CH=2
        -DAUDIO_IO_OUT_CH=2
        -DAUDIO_PIN_I2S_BCLK=15
        -DAUDIO_PIN_I2S_WS=17
        -DAUDIO_PIN_I2S_IN_A=16
        -DAUDIO_PIN_I2S_OUT_A=6
    ```

    Then set a callback, initialize the default topology, and start the sampler from firmware code:

    ```cpp
    #include "audio.h"

    static void audio_callback(audio_io_t* io) {
    }

    void setup() {
        audio_init_default();
        audio_set_callback(audio_callback);
        audio_start();
    }
    ```

    See [Configuration and defaults](configuration.md) for the full build-flag/defaults model.

## Example

See the [audio demo application](../examples/audio_demo.md) for a minimal `jescore`-controlled firmware entry point.

## Design model

Audio is block-based. I2S DMA produces/consumes buffers, the sampler job waits on
I2S event queues plus a control queue, and the user callback receives an
`audio_io_t` block once the relevant input/output sides are ready. The callback is your realm: as long as you stay within block time, you can do whatever you want with your new audio block. **How much is that?** Generally, $\frac{\mathrm{blocksize}}{\mathrm{samplerate}} - T_o$ where $T_o$ is the overhead the system has internally. 

There's also much work being done under the hood. The module handles input and output sync, but also inter-bank sync. As the platform's I2S banks are independent, they are also not synchronized. With this module you can rest assured that the audio you get is perfectly in sync across channels and inputs and outputs (output of course lags one block behind, it can't see into the future). The audio sampler is a loop that you control via the public API, which in turn sends control notifications to the loop, effectively steering it. 

The depth of the module comes from hiding the awkward parts:

- ESP-IDF queue/event handling
- bank-to-channel mapping
- byte packing and sign extension for different bit depths
- ping-pong buffer ownership
- start/stop/restart coordination
- realtime timing probes
- `jescore`/CLI control messages

## Channels and banks

The ESP32 has two I2S instances. In this module, an I2S instance is treated as a
stereo **bank**. A full two-bank topology can therefore represent four channels,
with each active bank providing mono or stereo input/output according to its
configuration. If input and output are both enabled, the active input and output
channel count is expected to match. A disabled side contributes zero channels.

!!! success "TL;DR"
    x input channels means x output channels. If the stream is not duplex, one direction has 0 channels. Something like 2 input 1 output is not possible.


## Channel sync

The intended sync model is hardware-led. One I2S bank may act as host and another
as device chained to the same BCLK/WS timing. Because ESP32 pins can be read and
written at the same time, this can synchronize channels more tightly than a
software-only scheme. A configuration without a host is also possible, but then
an external codec or MCU must provide BCLK and WS.

## Queues and control path

The audio module shares queue usage with ESP-IDF and its own control path. It
keeps the I2S event queues and the custom audio control queue in one FreeRTOS
queue set. The sampler drains ready events in a tight local loop so RX/TX events
that arrive close together are handled in one processing block.

Control messages such as stop and callback replacement use the control queue.
This keeps `jescore`/CLI control outside the realtime callback, while still letting
external tools manage the audio module deterministically.

## Timing model

The timing counters separate **when audio wakes up** from **how much work the
critical path costs**.

- **cadence**: time between sampler processing opportunities; this should match
  the audio block period.
- **read**: I2S read plus unpack/transpose overhead.
- **callback**: user callback runtime.
- **internal callback**: module-owned post-callback processing such as gain.
- **write**: pack/transpose plus I2S write overhead.
- **other**: scheduler/queue/bookkeeping time not attributed to those measured
  segments.
- **headroom**: block period minus worst observed process span.

The timing plots in `docs/media/` visualize this stack for tested hardware.

## Audio API

Public callable functions for initialization, stream lifecycle, callback/control state, and diagnostics.

::: api audio_init

Use this when the caller needs explicit sample rate, bit depth, bank topology, or
non-default channel layout. `audio_init()` checks settings, registers or finds
the sampler job, creates the queue set if needed, installs the control queue,
deinitializes/reinitializes active banks, applies settings, resets timing, warms
up DMA, clears stale events, and relaunches the sampler after a restart if it had
been running before.

If the sampler is running during reinitialization, a stop event is sent and the
call waits for shutdown. This makes reconfiguration deterministic instead of
leaving old I2S events racing against new topology.

::: api audio_init_default

Default initialization is for bring-up, demos, and tests. Defaults live in
`lib/audio/audio_default_cfg.h` and can be overridden with PlatformIO build flag
macros. Production code should prefer `audio_init()` when topology and sample
rate are part of the application contract.

::: api audio_start

`audio_start()` is idempotent. It is safe for CLI/API control paths to call it
when audio is already running; the important behavior is that one sampler loop is
running afterwards, not that a new one is created.

::: api audio_stop

`audio_stop()` is idempotent and waits for the sampler job to exit. This matters
for reproducible tests and for reconfiguration: stop before changing topology or
sample-rate state.

::: api audio_clear

Clears queued audio events by draining the active queues from the queue set. Use
this at explicit reset/recovery seams or around initialization, not inside the
realtime callback path.

::: api audio_set_callback

The callback must be realtime-safe: no blocking I/O, no heap allocation, no long
critical sections, and complete before the audio block deadline. The timing probe
reports this callback separately so callback cost can be distinguished from I2S
read/write overhead.

::: api audio_set_gain

Gain is a simple post-callback linear output gain and is clipped to the safe
range. Use callback DSP for application-specific mixing, metering, or nonlinear
processing.

::: api audio_get_gain

Use for status displays, CLI/API confirmation, and tests.

::: api audio_is_running

Use this for control/status surfaces. It answers whether the sampler job has a
running instance; it is not a realtime synchronization primitive.

::: api audio_get_nch

Returns the active channel count inferred from current topology. This is useful
for callbacks that need to adapt to mono/stereo/four-channel configurations.

::: api audio_get_sr

Use this to derive block deadlines and DSP coefficients from the actual running
sample rate.

::: api audio_timing_reset

Reset before a measurement window so plots and assertions describe a specific
operation rather than all time since boot.

::: api audio_timing_get

Copy counters out of the audio module for tests, CLI status, and plotting. The
plotting script uses these values to show how the timing stack fits into the
block budget.

## Audio Types

See [Shared audio sample/value types](audio_types.md) for the cross-module dependency map.

The remaining public structs, enums, unions, and callback typedefs describe audio data, topology, settings, and diagnostics.

::: api audio_io_t

`audio_io_t` is the main audio processing Interface. User DSP receives block
input and writes block output here; the Implementation owns DMA, queue handling,
packing, and unpacking.

::: api audio_timing_t

Timing counters are diagnostic data, not audio data. They let tests and plots
separate block cadence, read/write overhead, callback cost, and headroom.

::: api audio_settings_t

Global settings shared by all configured banks: sample rate, bit depth, and
related format constraints.

::: api audio_bank_t

A bank describes a stereo I2S block. It records which ESP32 I2S peripheral is
used, whether the bank is host/device, pins, direction, and channel mode.

::: api audio_meta_t

Internal descriptor tying together buffers, queues, settings, and banks.
Application code should prefer the public lifecycle/control functions instead of
using this as an external state object.

::: api audio_i2s_bank_t

Bank naming for ESP32 I2S peripherals.

::: api audio_i2s_direction_t

Direction naming for input/output bank endpoints.

::: api audio_i2s_ch_t

Channel naming inside a bank.

::: api i2s_event_type_ext_t

Extended event tags used by the audio Implementation for queue/event handling.

## Audio Jobs

FreeRTOS/`jescore` job entry points. These are runnable tasks rather than ordinary application API calls.

::: api audio_sampler

This is the internal sampler loop and `jescore` job. Application code should not
call it directly; use `audio_start()`, `audio_stop()`, and
`audio_set_callback()`.

The sampler sets up ping-pong processing, clears stale events on entry, waits on
the queue set, drains ready queue messages, handles control events, matches I2S
RX/TX events to banks, updates timing counters when enabled, and processes a
block once the required sides are ready. It exits only through the control stop
event.

::: api audio_ctrl_job

This is the `jescore`/CLI Adapter. It exposes operational control without making
the CLI parser part of the realtime audio path. Current control commands cover
status, restart/reconfiguration, gain/volume, and stop.

## Audio Macros

User-facing audio macros are grouped by purpose. Define overrides in `build_flags` before the library headers are compiled.

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_AUDIO` | Includes the audio module in the build and brings in the shared sample/value data shapes used by callbacks. |
| `AUDIO_IO_IN_CH`, `AUDIO_IO_OUT_CH` | Public default IO shape: number of input and output channels. Supported counts are 0, 1, 2, 3, and 4; at least one side must be nonzero. When both sides are enabled, counts must match. |
| `AUDIO_IO_BUS_ARCH` | Public bus shape when both input and output exist. Defaults to `AUDIO_IO_BUS_ARCH_SHARED`, meaning full-duplex on filled I2S buses. Omit this macro for normal shared-bus hardware; set `AUDIO_IO_BUS_ARCH_SPLIT` only for rare hardware with separate input/output I2S buses up to two channels per side. |
| `AUDIO_PIN_I2S_BCLK`, `AUDIO_PIN_I2S_WS` | Required shared I2S clock pins. |
| `AUDIO_PIN_I2S_IN_A`, `AUDIO_PIN_I2S_OUT_A` | Required data pins for the first derived I2S adapter. |
| `AUDIO_PIN_I2S_IN_B`, `AUDIO_PIN_I2S_OUT_B` | Required data pins when the derived adapter needs a second I2S bus. |
| `AUDIO_MAX_NUM_CH` | Maximum channel count compiled into audio sample/value unions. Derived from `AUDIO_IO_IN_CH`/`AUDIO_IO_OUT_CH` unless explicitly overridden. |
| `AUDIO_CFG_HAS_INPUT`, `AUDIO_CFG_HAS_OUTPUT` | Derived capability macros for topology-aware tests and modules. |
| `AUDIO_SETTINGS_CFG_DEFAULT` | Default `audio_settings_t` initializer. |
| `AUDIO_SR_DEFAULT`, `AUDIO_SR_44100`, `AUDIO_SR_48000`, `AUDIO_SR_96000`, `AUDIO_SR_MAX` | Sample-rate defaults and accepted named rates. |
| `AUDIO_BPS_DEFAULT` | Default bit depth. |
| `AUDIO_GAIN_DEFAULT` | Default linear output gain. |
| `AUDIO_TIMING_ENABLE` | Enables audio timing structs/functions and sampler timing probes. |
| `AUDIO_PINGPONG_SAMPLES`, `AUDIO_BLOCK_SAMPLES` | Audio buffer and processing block sizes. |
| `AUDIO_I2S_DMA_BUF_COUNT`, `AUDIO_EVT_QUEUE_LEN` | I2S DMA/event queue sizing. |
| `AUDIO_I2S_RESTART_MS`, `AUDIO_SAMPLER_STOP_TIMEOUT_MS`, `AUDIO_I2S_IO_TIMEOUT_MS` | Restart, stop, and I/O timeout values. |
| `AUDIO_I2S_READ_TIMEOUT_TICKS`, `AUDIO_I2S_WRITE_TIMEOUT_TICKS` | FreeRTOS tick-form I/O timeouts. |
| `AUDIO_I2S_CLOCK_SETTLE_MS`, `AUDIO_I2S_WARMUP_BLOCKS` | Clock-settle and DMA warmup behavior. |
| `AUDIO_SERVER_JOB_MEM`, `AUDIO_CONTROL_JOB_MEM` | `jescore` job stack sizes. |
| `AUDIO_SERVER_JOB_NAME`, `AUDIO_CONTROL_JOB_NAME` | `jescore` job names. |
| `AUDIO_CMD_RESTART`, `AUDIO_CMD_STOP`, `AUDIO_CMD_VOLUME`, `AUDIO_CMD_MUTE`, `AUDIO_CMD_STATUS` | CLI command strings. |
| `AUDIO_OPT_SR`, `AUDIO_OPT_SR_DASH`, `AUDIO_OPT_BPS`, `AUDIO_OPT_BPS_DASH`, `AUDIO_OPT_GAIN`, `AUDIO_OPT_GAIN_DASH` | CLI option strings. |
| `AUDIO_CMDS`, `AUDIO_RESTART_USAGE`, `AUDIO_MSG_UNKNOWN_CMD`, `AUDIO_MSG_OFFLINE`, `AUDIO_MSG_ERROR_USAGE`, `AUDIO_MSG_ERROR_ERRNUM`, `AUDIO_MSG_RESTARTED`, `AUDIO_MSG_STOPPED`, `AUDIO_MSG_VOLUME`, `AUDIO_MSG_STATUS` | CLI help/status/error text. |

## Internal implementation notes

These helpers are intentionally not part of the public user Interface, but they
explain what is happening inside the module.

### `__audio_bank_init(...)`

Installs the ESP-IDF I2S driver for one bank. It validates settings, translates
`audio_bank_t` plus `audio_settings_t` into ESP-I2S configuration, sets pins,
creates/attaches the bank event queue, and adds that queue to the shared queue
set.

### `__audio_bank_deinit(...)`

Uninstalls the I2S driver for one bank. If the bank event queue exists, queued
events are cleared, the queue is removed from the queue set, and the handle is
reset.

### `__audio_bank_nch(...)`

Converts the bank channel enum into a concrete channel count. The enum values are
not directly `1` and `2` because they abstract ESP-IDF channel configuration
macros.

### `__audio_sample_from_bytes(...)` and `__audio_sample_to_bytes(...)`

Convert between raw DMA bytes and `audio_sample_base_t` values for the active bit
depth. These helpers operate at one sample position, not by casting a whole DMA
buffer.

### `__audio_read(...)`

Reads one I2S bank with a bounded timeout, converts raw DMA bytes into sample
values, and soft-transposes bank-local channel data into the module's
multichannel block layout. The scratch buffer may be dedicated memory or alias
the final output buffer to save memory.

### `__audio_write(...)`

The reverse path of `__audio_read(...)`: converts the module's multichannel block
layout into bank-local raw DMA bytes and writes them to I2S with a bounded
timeout.
