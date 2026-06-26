#ifndef _AUDIO_H_
#define _AUDIO_H_

#ifdef FRX_ENABLE_MODULE_AUDIO

#include <driver/i2s.h>
#include <stdio.h>
#include <stdint.h>
#include "syserr.h"
#include "audio_default_cfg.h"
#include "audio_jccl.h"

#define AUDIO_SR_VALID(sr)      ((sr) == 44100 || (sr) == 48000 || (sr) == 96000)
#define AUDIO_BPS_VALID(bps)    ((bps) == 8 || (bps) == 16 || (bps) == 24 || (bps) == 32)

typedef enum{
    audio_i2s_bank_a,
    #if AUDIO_MAX_NUM_CH>2
    audio_i2s_bank_b,
    #endif
    audio_i2s_bank_N
}audio_i2s_bank_t;

typedef enum{
    audio_i2s_bank_host_i = (i2s_mode_t)I2S_MODE_MASTER | I2S_MODE_RX,
    audio_i2s_bank_host_o = (i2s_mode_t)I2S_MODE_MASTER | I2S_MODE_TX,
    audio_i2s_bank_host_io = (i2s_mode_t)I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX,
    audio_i2s_bank_devi_i = (i2s_mode_t)I2S_MODE_SLAVE | I2S_MODE_RX,
    audio_i2s_bank_devi_o = (i2s_mode_t)I2S_MODE_SLAVE | I2S_MODE_TX,
    audio_i2s_bank_devi_io = (i2s_mode_t)I2S_MODE_SLAVE | I2S_MODE_TX | I2S_MODE_RX
}audio_i2s_direction_t;

typedef enum{
    audio_i2s_ch_mono = (i2s_channel_fmt_t)I2S_CHANNEL_FMT_ONLY_LEFT, // left is default for mono
    audio_i2s_ch_stereo = (i2s_channel_fmt_t)I2S_CHANNEL_FMT_RIGHT_LEFT
}audio_i2s_ch_t;

typedef struct{
    audio_i2s_bank_t bank;
    audio_i2s_direction_t ad;
    audio_i2s_ch_t ch;
    uint8_t bclk; 
    uint8_t ws; 
    uint8_t data_rx; 
    uint8_t data_tx;
}audio_bank_t;

typedef struct {
    audio_sample_t audio_buf[AUDIO_FRAME_LEN*2];
    QueueHandle_t audio_evt_queue;
    uint32_t sr;
    uint8_t bps;
    audio_bank_t banks[audio_i2s_bank_N];
} audio_meta_t;


/// @brief Extended I2S event types for libFRX audio module
/// @note These extend ESP-IDF's I2S events (I2S_EVENT_MAX + 1 and above)
typedef enum{
    I2S_EVENT_STOP = I2S_EVENT_MAX + 1,     /// Stop audio streaming
    I2S_EVENT_RESTART,                      /// Restart audio streaming
    AUDIO_CMD_SET_CALLBACK                  /// Set new audio callback (via event queue data field)
}i2s_event_type_ext_t;

typedef int32_t audio_sample_base_t;
typedef float audio_val_base_t;

/// @brief Sample description in time. All channels run in parallel.
/// @note Amount of possible channels set by `AUDIO_MAX_NUM_CH`, but
///       the runtime can use less than that if wanted.
typedef union {
    struct {
        audio_sample_base_t ch1;
        #if AUDIO_MAX_NUM_CH > 1
        audio_sample_base_t ch2;
        #endif
        #if AUDIO_MAX_NUM_CH > 2
        audio_sample_base_t ch3;
        #endif
        #if AUDIO_MAX_NUM_CH > 3
        audio_sample_base_t ch4;
        #endif
    }_chx;
    audio_sample_base_t ch[AUDIO_MAX_NUM_CH];
}audio_sample_t;

typedef union {
    struct {
        audio_val_base_t ch1;
        #if AUDIO_MAX_NUM_CH > 1
        audio_val_base_t ch2;
        #endif
        #if AUDIO_MAX_NUM_CH > 2
        audio_val_base_t ch3;
        #endif
        #if AUDIO_MAX_NUM_CH > 3
        audio_val_base_t ch4;
        #endif
    }_chx;
    audio_val_base_t ch[AUDIO_MAX_NUM_CH];
}audio_val_t;

typedef void (*audio_cb_t)(audio_sample_t* pbuf);

/// @brief Initializes the audio module.
/// @param sr Global sample rate.
/// @param bps Global bit depth.
/// @param audio_banks Array of audio bank configs.
/// @param num_banks number of banks (either 1 or 2).
/// @return Error code indicating success or failure.
e_syserr_t audio_init(uint32_t sr, uint8_t bps, audio_bank_t audio_banks[], uint8_t num_banks);

/// @brief Initializes the I2S audio interface with default values.
/// @return Error code indicating success or failure.
/// @note Is part of the common signature interface for the init routine.
e_syserr_t audio_init_default(void);

/// @brief Manages the queue ISR for audio I/O.
/// @param p Pointer to job parameters.
void audio_sampler(void* p);

/// @brief Read audio samples from I2S peripheral.
/// @param data Destination buffer for audio samples.
/// @param len Number of samples to read.
/// @param bps Bits per single channel sample (resolution).
/// @param nch Amount of active channels.
/// @note Blocking call - waits for data to be available.
void audio_read(audio_sample_t* data, uint32_t len, uint8_t bps, uint8_t nch);

/// @brief Write audio samples to I2S peripheral.
/// @param data Source buffer containing audio samples.
/// @param len Number of samples to write.
/// @param bps Bits per single channel sample (resolution).
/// @param nch Amount of active channels.
/// @note Blocking call - waits for write completion.
void audio_write(audio_sample_t* data, uint32_t len, uint8_t bps, uint8_t nch);

/// @brief Suspend the audio loop for a short amount of time for state transitions. 
/// @note The length of suspension is set in `AUDIO_I2S_RESTART_MS`
void audio_suspend_short(void);

/// @brief Clear the audio event queue.
void audio_clear(void);

/// @brief Set the audio callback function.
/// @param cb Function pointer to invoke with audio buffer on each frame.
///            Callback signature: void cb(audio_sample_t* buffer)
/// @return Error code (e_syserr_none on success, e_syserr_driver_fail if queue full)
/// @note Callback change takes effect on next I2S event. Uses queue for thread-safe update.
e_syserr_t audio_set_callback(audio_cb_t cb);

/// @brief Get the current sample rate.
/// @return Sample rate in Hz (set during audio_init)
uint32_t audio_get_sr(void);

#endif // FRX_ENABLE_MODULE_AUDIO
#endif
