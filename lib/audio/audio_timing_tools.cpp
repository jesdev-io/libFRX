#ifdef FRX_ENABLE_MODULE_AUDIO

#include "audio.h"

#ifdef AUDIO_TIMING_ENABLE

#include <esp_timer.h>
#include <limits.h>
#include "audio_timing_tools.h"

static inline void __audio_timing_minmax(uint32_t* min_v, uint32_t* max_v, uint32_t v){
    if(v < *min_v) *min_v = v;
    if(v > *max_v) *max_v = v;
}

uint64_t audio_timing_now_us(void){
    return esp_timer_get_time();
}

void audio_timing_reset_state(audio_timing_state_t* state){
    if(!state) return;
    memset(state, 0, sizeof(audio_timing_state_t));
    for(uint8_t i = audio_i2s_bank_a; i < audio_i2s_bank_N; i++){
        state->counters.i2s_evt_min_delta_us[i] = UINT32_MAX;
    }
    state->counters.process_min_delta_us = UINT32_MAX;
}

void audio_timing_copy_state(audio_timing_t* dst, const audio_timing_state_t* state){
    if(!dst) return;
    if(!state){
        memset(dst, 0, sizeof(audio_timing_t));
        return;
    }
    memcpy(dst, &state->counters, sizeof(audio_timing_t));
}

void audio_timing_mark_i2s_event(audio_timing_state_t* state, uint8_t bank, bool rx){
    if(!state || bank >= audio_i2s_bank_N) return;
    uint64_t now_us = esp_timer_get_time();
    if(state->i2s_last_us[bank]){
        uint32_t delta_us = (uint32_t)(now_us - state->i2s_last_us[bank]);
        state->counters.i2s_evt_last_delta_us[bank] = delta_us;
        __audio_timing_minmax(&state->counters.i2s_evt_min_delta_us[bank],
                              &state->counters.i2s_evt_max_delta_us[bank],
                              delta_us);
    }
    state->i2s_last_us[bank] = now_us;
    state->counters.i2s_evt_count[bank]++;
    if(rx) state->counters.i2s_rx_evt_count[bank]++;
    else state->counters.i2s_tx_evt_count[bank]++;
}

uint64_t audio_timing_mark_process_start(audio_timing_state_t* state){
    if(!state) return 0;
    uint64_t process_start_us = esp_timer_get_time();
    if(state->process_last_us){
        uint32_t delta_us = (uint32_t)(process_start_us - state->process_last_us);
        state->counters.process_last_delta_us = delta_us;
        __audio_timing_minmax(&state->counters.process_min_delta_us,
                              &state->counters.process_max_delta_us,
                              delta_us);
    }
    state->process_last_us = process_start_us;
    state->counters.process_count++;
    return process_start_us;
}

void audio_timing_mark_process_end(audio_timing_state_t* state, uint64_t process_start_us){
    if(!state || !process_start_us) return;
    uint32_t process_runtime_us = (uint32_t)(esp_timer_get_time() - process_start_us);
    if(process_runtime_us > state->counters.process_max_runtime_us){
        state->counters.process_max_runtime_us = process_runtime_us;
    }
}

void audio_timing_mark_read_runtime(audio_timing_state_t* state, uint64_t start_us){
    if(!state || !start_us) return;
    uint32_t runtime_us = (uint32_t)(esp_timer_get_time() - start_us);
    state->counters.read_count++;
    state->counters.read_last_runtime_us = runtime_us;
    if(runtime_us > state->counters.read_max_runtime_us) state->counters.read_max_runtime_us = runtime_us;
}

void audio_timing_mark_callback_runtime(audio_timing_state_t* state, uint64_t start_us){
    if(!state || !start_us) return;
    uint32_t runtime_us = (uint32_t)(esp_timer_get_time() - start_us);
    state->counters.callback_count++;
    state->counters.callback_last_runtime_us = runtime_us;
    if(runtime_us > state->counters.callback_max_runtime_us) state->counters.callback_max_runtime_us = runtime_us;
}

void audio_timing_mark_internal_callback_runtime(audio_timing_state_t* state, uint64_t start_us){
    if(!state || !start_us) return;
    uint32_t runtime_us = (uint32_t)(esp_timer_get_time() - start_us);
    state->counters.internal_callback_count++;
    state->counters.internal_callback_last_runtime_us = runtime_us;
    if(runtime_us > state->counters.internal_callback_max_runtime_us) state->counters.internal_callback_max_runtime_us = runtime_us;
}

void audio_timing_mark_write_runtime(audio_timing_state_t* state, uint64_t start_us){
    if(!state || !start_us) return;
    uint32_t runtime_us = (uint32_t)(esp_timer_get_time() - start_us);
    state->counters.write_count++;
    state->counters.write_last_runtime_us = runtime_us;
    if(runtime_us > state->counters.write_max_runtime_us) state->counters.write_max_runtime_us = runtime_us;
}

void audio_timing_mark_read_error(audio_timing_state_t* state){
    if(state) state->counters.read_error_count++;
}

void audio_timing_mark_write_error(audio_timing_state_t* state){
    if(state) state->counters.write_error_count++;
}

#endif // AUDIO_TIMING_ENABLE
#endif // FRX_ENABLE_MODULE_AUDIO
