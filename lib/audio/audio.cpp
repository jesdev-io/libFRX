#ifdef FRX_ENABLE_MODULE_AUDIO

#include <Arduino.h>
#include <jescore.h>
#include <soc/i2s_reg.h>
#include "audio.h"

static audio_meta_t audio_meta = {0};

/// @brief Uninstall the I2S config for a given bank.
/// @param audio_bank Audio bank
/// @return Error code indicating success or failure.
static inline e_syserr_t __audio_bank_deinit(audio_bank_t* audio_bank){
    esp_err_t e;
    e = i2s_driver_uninstall((i2s_port_t)audio_bank->bank);
    if(e != ESP_OK) return e_syserr_driver_fail;
    return e_syserr_none;
}

/// @brief Install the I2S config for a given bank.
/// @param audio_bank Audio bank
/// @param sr Sampling rate
/// @param bps Bit depth
/// @return Error code indicating success or failure.
static inline e_syserr_t __audio_bank_init(audio_bank_t* audio_bank, uint32_t sr, uint8_t bps){
    if(!AUDIO_SR_VALID(sr)) return e_syserr_param;
    if(!AUDIO_BPS_VALID(bps)) return e_syserr_param;
    const i2s_config_t audio_cfg = {
        .mode = (i2s_mode_t)audio_bank->ad,
        .sample_rate = sr,
        .bits_per_sample = (i2s_bits_per_sample_t)bps,
        .channel_format = (i2s_channel_fmt_t)audio_bank->ch,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
    };
    i2s_pin_config_t audio_pins = {
        .bck_io_num = audio_bank->bclk,
        .ws_io_num = audio_bank->ws,
        .data_out_num = audio_bank->data_tx,
        .data_in_num = audio_bank->data_rx,
    };
    esp_err_t esp_e;
    esp_e = i2s_driver_install((i2s_port_t)audio_bank->bank, &audio_cfg, 4, &audio_meta.audio_evt_queue);
    if (esp_e != ESP_OK) {
        return e_syserr_driver_fail;
    }
    esp_e = i2s_set_pin((i2s_port_t)audio_bank->bank, &audio_pins);
    if (esp_e != ESP_OK) { 
        return e_syserr_driver_fail;
    }
}

e_syserr_t audio_init(uint32_t sr, uint8_t bps, audio_bank_t audio_banks[], uint8_t num_banks){
    if(!AUDIO_SR_VALID(sr)) return e_syserr_param;
    if(!AUDIO_BPS_VALID(bps)) return e_syserr_param;
    if(!audio_banks) return e_syserr_param;
    if(num_banks > 2) return e_syserr_param;
    esp_err_t esp_e;
    e_syserr_t e;
    jes_err_t je = jes_register_job(AUDIO_SERVER_JOB_NAME, AUDIO_SERVER_JOB_MEM, 1, audio_sampler, 1, 0);
    if(je != e_err_no_err && je != e_err_duplicate) return (e_syserr_t)je;

    if(__job_get_job_by_name(AUDIO_SERVER_JOB_NAME)->instances){
        /*If the audio loop is already running, send a restart signal that delays
        the next execution of the loop until the new driver settings are applied.*/
        for(uint8_t i = audio_i2s_bank_a; i < num_banks; i++){
            i2s_event_t evt;
            evt.type = (i2s_event_type_t)I2S_EVENT_RESTART;
            xQueueSend(audio_meta.audio_evt_queue, &evt, portMAX_DELAY);
            jes_delay_job_ms(AUDIO_I2S_RESTART_MS);
            if((e = __audio_bank_deinit(&audio_banks[i])) != e_syserr_none) return e;
        }
    }
    audio_meta.sr = sr;
    audio_meta.bps = bps;
    memcpy(audio_meta.banks, audio_banks, sizeof(audio_bank_t)*num_banks);
    for(uint8_t i = audio_i2s_bank_a; i < num_banks; i++){
        if((e = __audio_bank_init(&audio_banks[i], sr, bps)) != e_syserr_none) return e;
    }
    return e_syserr_none;
}

e_syserr_t audio_init_default(void){
    audio_bank_t banks[] = AUDIO_BANKS_CFG_SINGLE_STEREO_IO;
    return audio_init(AUDIO_SR_DEFAULT, AUDIO_BPS_DEFAULT, banks, audio_i2s_bank_N);
}

void audio_sampler(void* p){
    job_struct_t* pj = (job_struct_t*)p;
    pj->role = e_role_core;
    static audio_cb_t cur_cb = NULL;
    while(1){
        i2s_event_t evt;
        static bool tx_occ = false;
        static bool rx_occ = false;
        if (xQueueReceive(audio_meta.audio_evt_queue, &evt, portMAX_DELAY) == pdPASS){
            if(evt.type == (i2s_event_type_t)I2S_EVENT_RESTART){
                jes_delay_job_ms(AUDIO_I2S_RESTART_MS);
                jes_print_pj(pj, "Audio was restarted!");
                continue;
            }
            if(evt.type == (i2s_event_type_t)AUDIO_CMD_SET_CALLBACK){
                cur_cb = (audio_cb_t)evt.size;
                continue;
            }
            if (evt.type != I2S_EVENT_TX_DONE && evt.type != I2S_EVENT_RX_DONE){
                jes_print_pj(pj, "Audio Overflow!");
                jes_throw_error((jes_err_t)e_syserr_audio_dead);
                continue;
            }
            if(cur_cb == NULL){
                continue;
            }
            if(evt.type == I2S_EVENT_TX_DONE) tx_occ = true;
            if(evt.type == I2S_EVENT_RX_DONE) rx_occ = true;

            if(tx_occ && rx_occ){
                __job_set_timing_begin(__get_systime_ms(), pj);
                cur_cb(audio_meta.audio_buf);
                tx_occ = false;
                rx_occ = false;
                __job_set_timing_end(__get_systime_ms(), pj);
            } 
        }
    }
}

void audio_read(audio_sample_t* data, uint32_t len, uint8_t bps, uint8_t nch){
    uint32_t bytesRead = 0;
    // TODO: Couple nch with .channel_format
    esp_err_t e = i2s_read((i2s_port_t)AUDIO_I2S_PORT, (uint8_t *)data, len*(bps/8)*nch, &bytesRead, portMAX_DELAY);
    if(e != ESP_OK){
        uart_unif_writef("I2S read fail: %d\n\r", e);
    }
}

void audio_write(audio_sample_t *data, uint32_t len, uint8_t bps, uint8_t nch){
    uint32_t bytesWritten = 0;
    esp_err_t e = i2s_write((i2s_port_t)AUDIO_I2S_PORT, (uint8_t*) data, len*(bps/8)*nch, &bytesWritten, portMAX_DELAY);
}

void audio_suspend_short(void){
    i2s_event_t evt;
    evt.type = (i2s_event_type_t)I2S_EVENT_RESTART;
    xQueueSend(audio_meta.audio_evt_queue, &evt, portMAX_DELAY);
}

void audio_clear(void){
    xQueueReset(audio_meta.audio_evt_queue);
}

e_syserr_t audio_set_callback(audio_cb_t cb){
    i2s_event_t evt;
    evt.type = (i2s_event_type_t)AUDIO_CMD_SET_CALLBACK;
    evt.size = (size_t)cb;
    xQueueSend(audio_meta.audio_evt_queue, &evt, portMAX_DELAY);
    return e_syserr_none;
}

uint32_t audio_get_sr(void){
    return audio_meta.sr;
}

#endif // FRX_ENABLE_MODULE_AUDIO
