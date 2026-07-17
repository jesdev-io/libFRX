#ifdef FRX_ENABLE_MODULE_AUDIO

#include <Arduino.h>
#include <string.h>
#include <jescore.h>
#include <soc/i2s_reg.h>
#include "audio.h"
#include "libfrx_sys.h"

static audio_meta_t audio_meta = {0};

/// @brief Uninstall the I2S config for a given bank.
/// @param audio_bank Audio bank
/// @return Error code indicating success or failure.
static inline e_syserr_t __audio_bank_deinit(audio_bank_t* audio_bank){
    esp_err_t e;
    if(audio_meta.audio_evt_qlist[audio_bank->bank]){
        audio_clear();
        xQueueRemoveFromSet(audio_meta.audio_evt_qlist[audio_bank->bank], audio_meta.audio_evt_qset);
    }
    e = i2s_driver_uninstall((i2s_port_t)audio_bank->bank);
    if(e != ESP_OK) return e_syserr_driver_fail;
    audio_meta.audio_evt_qlist[audio_bank->bank] = NULL;
    return e_syserr_none;
}

/// @brief Install the I2S config for a given bank.
/// @param audio_bank Audio bank
/// @param settings Global audio settings
/// @return Error code indicating success or failure.
static inline e_syserr_t __audio_bank_init(audio_bank_t* audio_bank, const audio_settings_t* settings){
    if(!AUDIO_SR_VALID(settings->sr)) return e_syserr_param;
    if(!AUDIO_BPS_VALID(settings->bps)) return e_syserr_param;
    const i2s_config_t audio_cfg = {
        .mode = (i2s_mode_t)audio_bank->ad,
        .sample_rate = settings->sr,
        .bits_per_sample = (i2s_bits_per_sample_t)settings->bps,
        .channel_format = (i2s_channel_fmt_t)audio_bank->ch,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = AUDIO_I2S_DMA_BUF_COUNT,
        .dma_buf_len = AUDIO_BLOCK_SAMPLES,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0,
    };
    i2s_pin_config_t audio_pins = {
        .bck_io_num = settings->bclk,
        .ws_io_num = settings->ws,
        .data_out_num = audio_bank->data_tx,
        .data_in_num = audio_bank->data_rx,
    };
    esp_err_t esp_e;
    esp_e = i2s_driver_install((i2s_port_t)audio_bank->bank, &audio_cfg, AUDIO_EVT_QUEUE_LEN, &audio_meta.audio_evt_qlist[audio_bank->bank]);
    if (esp_e != ESP_OK) {
        return e_syserr_driver_fail;
    }
    esp_e = i2s_set_pin((i2s_port_t)audio_bank->bank, &audio_pins);
    if (esp_e != ESP_OK) { 
        return e_syserr_driver_fail;
    }
    xQueueAddToSet(audio_meta.audio_evt_qlist[audio_bank->bank], audio_meta.audio_evt_qset);
    return e_syserr_none;
}

/// @brief Convert a bank config into the amount of channels it serves.
/// @param bank Pointer to bank instance.
/// @return Number of channels (1 or 2).
static inline uint8_t __audio_bank_nch(audio_bank_t* bank){
    return (bank->ch == audio_i2s_ch_mono) ? 1 : 2;
}

/// @brief Convert raw i2s byte data into audio samples. 
/// @param p Pointer to byte buffer for one sample base type.
/// @param bps Bits per sample.
/// @return Sample as audio_sample_base_t.
static inline audio_sample_base_t __audio_sample_from_bytes(uint8_t* p, uint8_t bps){
    switch(bps){
        case 8:
            return (int8_t)p[0];
        case 16:
            return (int16_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8));
        case 24:{
            int32_t v = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
            if(v & 0x00800000) v |= 0xFF000000;
            return v;
        }
        default:
            return (audio_sample_base_t)((uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
    }
}

/// @brief Convert audio samples into raw i2s byte data. 
/// @param p Pointer to byte buffer.
/// @param v Sample value.
/// @param bps Bits per sample.
static inline void __audio_sample_to_bytes(uint8_t* p, audio_sample_base_t v, uint8_t bps){
    p[0] = (uint8_t)(v);
    if(bps > 8) p[1] = (uint8_t)(v >> 8);
    if(bps > 16) p[2] = (uint8_t)(v >> 16);
    if(bps > 24) p[3] = (uint8_t)(v >> 24);
}

/// @brief Read audio data from an i2s bank.
/// @param bank Pointer to bank instance.
/// @param data Destination array for audio samples.
/// @param scratch Byte array for storing temp data, see @note.
/// @param len Amount of samples.
/// @param bps Bits per sample.
/// @param audio_ch_idx Mapping from I2S bank channel index to audio buffer channel index.
/// @return Error code indicating success or failure.
/// @note The scratch array exists to have modifiable memory space that can either be
///       a dedicated buffer or the same buffer as the final output to save memory.
static e_syserr_t __audio_read(audio_bank_t* bank, audio_sample_t* data, audio_sample_t* scratch, uint32_t len, uint8_t bps, const uint8_t* audio_ch_idx){
    if(!data || !scratch || !audio_ch_idx) return e_syserr_param;
    if(len > AUDIO_PINGPONG_SAMPLES) return e_syserr_param;
    const uint8_t sample_bytes = bps/8;
    const uint8_t bank_nch = __audio_bank_nch(bank);
    for(uint8_t ch = 0; ch < bank_nch; ch++){
        if(audio_ch_idx[ch] >= AUDIO_MAX_NUM_CH) return e_syserr_param;
    }
    uint8_t* scratch_bytes = (uint8_t*)scratch;
    uint32_t bytesRead = 0;
    uint32_t bytesExpected = len * bank_nch * sample_bytes;
    esp_err_t e = i2s_read((i2s_port_t)bank->bank, scratch_bytes, bytesExpected, &bytesRead, AUDIO_I2S_READ_TIMEOUT_TICKS);
    if(e != ESP_OK || bytesRead != bytesExpected) return e_syserr_driver_fail;
    for(uint32_t sidx = 0; sidx < len; sidx++){
        for(uint8_t ch = 0; ch < bank_nch; ch++){
            uint32_t off = ((sidx * bank_nch) + ch) * sample_bytes;
            data[sidx].ch[audio_ch_idx[ch]] = __audio_sample_from_bytes(&scratch_bytes[off], bps);
        }
    }
    return e_syserr_none;
}

/// @brief Write audio data to an i2s bank.
/// @param bank Pointer to bank instance.
/// @param data Source array for audio samples.
/// @param scratch Byte array for storing temp data, see @note.
/// @param len Amount of samples.
/// @param bps Bits per sample.
/// @param audio_ch_idx Mapping from I2S bank channel index to audio buffer channel index.
/// @return Error code indicating success or failure.
/// @note The scratch array exists to have modifiable memory space that can either be
///       a dedicated buffer or the same buffer as the final output to save memory.
static e_syserr_t __audio_write(audio_bank_t* bank, audio_sample_t *data, audio_sample_t* scratch, uint32_t len, uint8_t bps, const uint8_t* audio_ch_idx){
    if(!data || !scratch || !audio_ch_idx) return e_syserr_param;
    if(len > AUDIO_PINGPONG_SAMPLES) return e_syserr_param;
    uint8_t sample_bytes = bps/8;
    uint8_t bank_nch = __audio_bank_nch(bank);
    for(uint8_t ch = 0; ch < bank_nch; ch++){
        if(audio_ch_idx[ch] >= AUDIO_MAX_NUM_CH) return e_syserr_param;
    }
    uint8_t* scratch_bytes = (uint8_t*)scratch;
    uint32_t bytesWritten = 0;
    uint32_t bytesExpected = len * bank_nch * sample_bytes;
    for(uint32_t sidx = 0; sidx < len; sidx++){
        for(uint8_t ch = 0; ch < bank_nch; ch++){
            uint32_t off = ((sidx * bank_nch) + ch) * sample_bytes;
            audio_sample_base_t v = data[sidx].ch[audio_ch_idx[ch]];
            __audio_sample_to_bytes(&scratch_bytes[off], v, bps);
        }
    }
    esp_err_t e = i2s_write((i2s_port_t)bank->bank, scratch_bytes, bytesExpected, &bytesWritten, AUDIO_I2S_WRITE_TIMEOUT_TICKS);
    if(e != ESP_OK || bytesWritten != bytesExpected) return e_syserr_driver_fail;
    return e_syserr_none;
}

e_syserr_t audio_init(audio_settings_t settings, audio_bank_t audio_banks[], uint8_t num_banks){
    if(!AUDIO_SR_VALID(settings.sr)) return e_syserr_param;
    if(!AUDIO_BPS_VALID(settings.bps)) return e_syserr_param;
    if(!audio_banks) return e_syserr_param;
    if(num_banks > 2 || num_banks == 0) return e_syserr_param;
    esp_err_t esp_e;
    e_syserr_t e;
    job_struct_t* audio_job = __job_get_job_by_name(AUDIO_SERVER_JOB_NAME);
    if(!audio_job){
        jes_err_t je = jes_register_job(AUDIO_SERVER_JOB_NAME, AUDIO_SERVER_JOB_MEM, 1, audio_sampler, 1, 1);
        if(je != e_err_no_err) return (e_syserr_t)je;
        audio_job = __job_get_job_by_name(AUDIO_SERVER_JOB_NAME);
        if(!audio_job) return e_syserr_driver_fail;
    }
    if(!audio_meta.audio_evt_qset){
        const UBaseType_t num_bank_queues = sizeof(audio_meta.audio_evt_qlist) / sizeof(audio_meta.audio_evt_qlist[0]);
        audio_meta.audio_evt_qset = xQueueCreateSet((num_bank_queues + 1) * AUDIO_EVT_QUEUE_LEN);
        if(!audio_meta.audio_evt_qset) return e_syserr_driver_fail;
    }
    if(!audio_meta.audio_evt_qlist[audio_ctrl_queue_idx]){
        audio_meta.audio_evt_qlist[audio_ctrl_queue_idx] = xQueueCreate(AUDIO_EVT_QUEUE_LEN, sizeof(i2s_event_t));
        if(!audio_meta.audio_evt_qlist[audio_ctrl_queue_idx]) return e_syserr_driver_fail;
        xQueueAddToSet(audio_meta.audio_evt_qlist[audio_ctrl_queue_idx], audio_meta.audio_evt_qset);
    }

    uint8_t relaunch_sampler = 0;
    if(audio_job->instances){
        relaunch_sampler = 1;
        i2s_event_t evt;
        evt.type = (i2s_event_type_t)AUDIO_CTRL_EVT_STOP;
        if(xQueueSend(audio_meta.audio_evt_qlist[audio_ctrl_queue_idx], &evt, pdMS_TO_TICKS(AUDIO_SAMPLER_STOP_TIMEOUT_MS)) != pdPASS){
            return e_syserr_driver_fail;
        }
        uint32_t stop_wait_ms = 0;
        while(audio_job->instances){
            if(stop_wait_ms++ >= AUDIO_SAMPLER_STOP_TIMEOUT_MS) return e_syserr_driver_fail;
            jes_delay_job_ms(1);
        }
    }
    for(uint8_t i = audio_i2s_bank_a; i < audio_meta.__num_banks; i++){
        if(audio_meta.audio_evt_qlist[audio_meta.banks[i].bank]){
            if((e = __audio_bank_deinit(&audio_meta.banks[i])) != e_syserr_none) return e;
        }
    }
    uint8_t num_hosts = 0;
    for(uint8_t i = audio_i2s_bank_a; i < num_banks; i++){
        if(audio_banks[i].ad < audio_i2s_bank_HOST_NUM) num_hosts++;
        if(num_hosts > 1) return e_syserr_param; // only one host can exist in the config
        if((e = __audio_bank_init(&audio_banks[i], &settings)) != e_syserr_none) return e;
    }
    if(num_hosts == 0){
        uart_unif_write(LIBFRX_SYS_WARN_PFX "No I2S host configured. Is an external codec the host?\n\r");
    }
    audio_meta.settings = settings;
    memcpy(audio_meta.banks, audio_banks, sizeof(audio_bank_t)*num_banks);
    audio_meta.__num_banks = num_banks;
    #ifdef AUDIO_TIMING_ENABLE
    audio_timing_reset();
    #endif

    for(uint8_t i = audio_i2s_bank_a; i < audio_meta.__num_banks; i++){
        if(audio_meta.banks[i].ad & I2S_MODE_TX){
            if(i2s_zero_dma_buffer((i2s_port_t)audio_meta.banks[i].bank) != ESP_OK) return e_syserr_driver_fail;
        }
    }
    jes_delay_job_ms(AUDIO_I2S_CLOCK_SETTLE_MS);
    for(uint8_t warmup = 0; warmup < AUDIO_I2S_WARMUP_BLOCKS; warmup++){
        uint32_t block_off = (warmup & 1) * AUDIO_BLOCK_SAMPLES;
        audio_sample_t* in_block = &audio_meta.audio_buf[block_off];
        audio_sample_t* out_block = &audio_meta.audio_buf[AUDIO_PINGPONG_SAMPLES + block_off];
        uint8_t rx_ch_idx = 0;
        uint8_t tx_ch_idx = 0;
        memset(audio_meta.audio_buf, 0, sizeof(audio_meta.audio_buf));
        for(uint8_t i = audio_i2s_bank_a; i < audio_meta.__num_banks; i++){
            audio_i2s_direction_t mode = audio_meta.banks[i].ad;
            uint8_t bank_nch = __audio_bank_nch(&audio_meta.banks[i]);
            if(mode & I2S_MODE_RX){
                uint8_t rx_ch_map[2] = {rx_ch_idx, (uint8_t)(rx_ch_idx + 1)};
                if(__audio_read(&audio_meta.banks[i], in_block, out_block, AUDIO_BLOCK_SAMPLES, audio_meta.settings.bps, rx_ch_map) != e_syserr_none){
                    return e_syserr_driver_fail;
                }
                rx_ch_idx += bank_nch;
            }
            if(mode & I2S_MODE_TX){
                uint8_t tx_ch_map[2] = {tx_ch_idx, (uint8_t)(tx_ch_idx + 1)};
                if(__audio_write(&audio_meta.banks[i], in_block, out_block, AUDIO_BLOCK_SAMPLES, audio_meta.settings.bps, tx_ch_map) != e_syserr_none){
                    return e_syserr_driver_fail;
                }
                tx_ch_idx += bank_nch;
            }
        }
    }
    audio_clear();
    if(relaunch_sampler){
        audio_job->error = e_err_no_err;
        if(jes_launch_job(AUDIO_SERVER_JOB_NAME) != e_err_no_err) return e_syserr_driver_fail;
        uint32_t start_wait_ms = 0;
        while(!audio_job->handle){
            if(audio_job->error != e_err_no_err) return e_syserr_driver_fail;
            if(start_wait_ms++ >= AUDIO_SAMPLER_STOP_TIMEOUT_MS) return e_syserr_driver_fail;
            jes_delay_job_ms(1);
        }
    }
    return e_syserr_none;
}

e_syserr_t audio_init_default(void){
    audio_settings_t settings = AUDIO_SETTINGS_CFG_DEFAULT;
    audio_bank_t banks[] = AUDIO_BANKS_CFG_SINGLE_STEREO_IO;
    return audio_init(settings, banks, audio_i2s_bank_N);
}

void audio_sampler(void* p){
    job_struct_t* pj = (job_struct_t*)p;
    static bool rx_ready[audio_i2s_bank_N] = {0};
    static bool tx_ready[audio_i2s_bank_N] = {0};
    static uint8_t audio_buf_phase = 0;
    uint8_t rx_bank[audio_i2s_bank_N] = {0};
    uint8_t tx_bank[audio_i2s_bank_N] = {0};
    uint8_t rx_ch_idx[audio_i2s_bank_N] = {0};
    uint8_t tx_ch_idx[audio_i2s_bank_N] = {0};
    uint8_t rx_bank_count = 0;
    uint8_t tx_bank_count = 0;
    uint8_t nch_rx = 0;
    uint8_t nch_tx = 0;
    for(uint8_t i = audio_i2s_bank_a; i < audio_i2s_bank_N; i++){
        rx_ready[i] = false;
        tx_ready[i] = false;
    }
    for(uint8_t i = audio_i2s_bank_a; i < audio_meta.__num_banks; i++){
        uint8_t nch = __audio_bank_nch(&audio_meta.banks[i]);
        if(audio_meta.banks[i].ad & I2S_MODE_RX){
            rx_bank[rx_bank_count] = i;
            rx_ch_idx[rx_bank_count] = nch_rx;
            rx_bank_count++;
            nch_rx += nch;
        }
        if(audio_meta.banks[i].ad & I2S_MODE_TX){
            tx_bank[tx_bank_count] = i;
            tx_ch_idx[tx_bank_count] = nch_tx;
            tx_bank_count++;
            nch_tx += nch;
        }
    }
    if(nch_rx && nch_tx && nch_rx != nch_tx){
        LIBFRX_SYS_DEBUG_PRINT_PJ(pj, "Audio input/output channel count mismatch!");
        jes_throw_error((jes_err_t)e_syserr_param);
        return;
    }
    const uint8_t nch = nch_rx ? nch_rx : nch_tx;
    audio_buf_phase = 0;
    audio_clear();
    LIBFRX_SYS_DEBUG_PRINT_PJ(pj, "Audio started!");
    while(1){
        i2s_event_t evt;
        QueueSetMemberHandle_t active_queue = xQueueSelectFromSet(audio_meta.audio_evt_qset, portMAX_DELAY);
        while(active_queue){
            if (xQueueReceive((QueueHandle_t)active_queue, &evt, 0) == pdPASS){
                if(active_queue == audio_meta.audio_evt_qlist[audio_ctrl_queue_idx]){
                    switch (evt.type)
                    {
                    case (i2s_event_type_t)AUDIO_CTRL_EVT_STOP:
                        LIBFRX_SYS_DEBUG_PRINT_PJ(pj, "Audio stopped!");
                        return;
                    case (i2s_event_type_t)AUDIO_CTRL_EVT_SET_CALLBACK:
                        audio_meta.cb = (audio_cb_t)evt.size;
                        LIBFRX_SYS_DEBUG_PRINT_PJ(pj, "Audio callback set!");
                        break;
                    default:
                        LIBFRX_SYS_DEBUG_PRINT_PJ(pj, "Unknown control event <%d>!", evt.type);
                        break;
                    }
                }
                else{
                    int8_t active_bank = -1;
                    for(uint8_t i = audio_i2s_bank_a; i < audio_meta.__num_banks; i++){
                        if(active_queue == audio_meta.audio_evt_qlist[i]){
                            active_bank = i;
                            break;
                        }
                    }
                    if(active_bank >= 0){
                        if(evt.type == I2S_EVENT_RX_DONE){
                            rx_ready[active_bank] = true;
                            #ifdef AUDIO_TIMING_ENABLE
                            audio_timing_mark_i2s_event(&audio_meta.timing, active_bank, true);
                            #endif
                        }
                        else if(evt.type == I2S_EVENT_TX_DONE){
                            tx_ready[active_bank] = true;
                            #ifdef AUDIO_TIMING_ENABLE
                            audio_timing_mark_i2s_event(&audio_meta.timing, active_bank, false);
                            #endif
                        }
                        else{
                            LIBFRX_SYS_DEBUG_PRINT_PJ(pj, "Audio Overflow (code %d) on bank %d!", evt.type, active_bank);
                            jes_throw_error((jes_err_t)e_syserr_audio_dead);
                        }
                    }
                }
            }
            active_queue = xQueueSelectFromSet(audio_meta.audio_evt_qset, 0);
        }

        bool io_ready = true;
        for(uint8_t i = 0; i < rx_bank_count; i++){
            if(!rx_ready[rx_bank[i]]) io_ready = false;
        }
        for(uint8_t i = 0; i < tx_bank_count; i++){
            if(!tx_ready[tx_bank[i]]) io_ready = false;
        }
        if(io_ready){
            #ifdef AUDIO_TIMING_ENABLE
            uint64_t process_start_us = audio_timing_mark_process_start(&audio_meta.timing);
            #endif
            uint32_t block_off = audio_buf_phase * AUDIO_BLOCK_SAMPLES;
            audio_io_t io = {
                .in = nch_rx ? &audio_meta.audio_buf[block_off] : NULL,
                .out = nch_tx ? &audio_meta.audio_buf[AUDIO_PINGPONG_SAMPLES + block_off] : NULL,
                .len = AUDIO_BLOCK_SAMPLES,
                .nch = nch,
            };
            if(io.in){
                audio_sample_t* scratch = io.out ? io.out : &audio_meta.audio_buf[AUDIO_PINGPONG_SAMPLES];
                for(uint8_t i = 0; i < rx_bank_count; i++){
                    uint8_t bank_idx = rx_bank[i];
                    uint8_t audio_ch_map[2] = {rx_ch_idx[i], (uint8_t)(rx_ch_idx[i] + 1)};
                    #ifdef AUDIO_TIMING_ENABLE
                    uint64_t read_start_us = audio_timing_now_us();
                    #endif
                    e_syserr_t read_e = __audio_read(&audio_meta.banks[bank_idx], io.in, scratch, io.len, audio_meta.settings.bps, audio_ch_map);
                    #ifdef AUDIO_TIMING_ENABLE
                    audio_timing_mark_read_runtime(&audio_meta.timing, read_start_us);
                    #endif
                    if(read_e != e_syserr_none){
                        #ifdef AUDIO_TIMING_ENABLE
                        audio_timing_mark_read_error(&audio_meta.timing);
                        #endif
                        jes_throw_error((jes_err_t)e_syserr_audio_dead);
                    }
                }
            }
            if(io.out) memset(io.out, 0, sizeof(audio_sample_t) * io.len);
            if(audio_meta.cb){
                __job_set_timing_begin(__get_systime_ms(), pj);
                #ifdef AUDIO_TIMING_ENABLE
                uint64_t callback_start_us = audio_timing_now_us();
                #endif
                audio_meta.cb(&io);
                #ifdef AUDIO_TIMING_ENABLE
                audio_timing_mark_callback_runtime(&audio_meta.timing, callback_start_us);
                #endif
                __job_set_timing_end(__get_systime_ms(), pj);
            }
            if(io.out){
                audio_sample_t* scratch = io.in ? io.in : &audio_meta.audio_buf[0];
                for(uint8_t i = 0; i < tx_bank_count; i++){
                    uint8_t bank_idx = tx_bank[i];
                    uint8_t audio_ch_map[2] = {tx_ch_idx[i], (uint8_t)(tx_ch_idx[i] + 1)};
                    #ifdef AUDIO_TIMING_ENABLE
                    uint64_t write_start_us = audio_timing_now_us();
                    #endif
                    e_syserr_t write_e = __audio_write(&audio_meta.banks[bank_idx], io.out, scratch, io.len, audio_meta.settings.bps, audio_ch_map);
                    #ifdef AUDIO_TIMING_ENABLE
                    audio_timing_mark_write_runtime(&audio_meta.timing, write_start_us);
                    #endif
                    if(write_e != e_syserr_none){
                        #ifdef AUDIO_TIMING_ENABLE
                        audio_timing_mark_write_error(&audio_meta.timing);
                        #endif
                        jes_throw_error((jes_err_t)e_syserr_audio_dead);
                    }
                }
            }
            #ifdef AUDIO_TIMING_ENABLE
            audio_timing_mark_process_end(&audio_meta.timing, process_start_us);
            #endif
            audio_buf_phase ^= 1;
            for(uint8_t i = 0; i < rx_bank_count; i++){
                rx_ready[rx_bank[i]] = false;
            }
            for(uint8_t i = 0; i < tx_bank_count; i++){
                tx_ready[tx_bank[i]] = false;
            }
        }
    }
}

void audio_clear(void){
    if(!audio_meta.audio_evt_qset) return;
    i2s_event_t evt;
    QueueSetMemberHandle_t active_queue = xQueueSelectFromSet(audio_meta.audio_evt_qset, 0);
    while(active_queue){
        xQueueReceive((QueueHandle_t)active_queue, &evt, 0);
        active_queue = xQueueSelectFromSet(audio_meta.audio_evt_qset, 0);
    }
}

e_syserr_t audio_set_callback(audio_cb_t cb){
    audio_meta.cb = cb;
    i2s_event_t evt;
    evt.type = (i2s_event_type_t)AUDIO_CTRL_EVT_SET_CALLBACK;
    evt.size = (size_t)cb;
    if(xQueueSend(audio_meta.audio_evt_qlist[audio_ctrl_queue_idx], &evt, pdMS_TO_TICKS(AUDIO_SAMPLER_STOP_TIMEOUT_MS)) != pdPASS){
        return e_syserr_driver_fail;
    }
    return e_syserr_none;
}

uint32_t audio_get_sr(void){
    return audio_meta.settings.sr;
}

#ifdef AUDIO_TIMING_ENABLE
void audio_timing_reset(void){
    audio_timing_reset_state(&audio_meta.timing);
}

void audio_timing_get(audio_timing_t* timing){
    if(!timing) return;
    audio_timing_copy_state(timing, &audio_meta.timing);
}
#endif

#endif // FRX_ENABLE_MODULE_AUDIO
