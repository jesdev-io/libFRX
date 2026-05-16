#ifdef FRX_ENABLE_MODULE_SYNTH

#include <Arduino.h>
#include <jescore.h>
#include "synth.h"
#include "uart_unif.h"
#include "fastmath.h"
#include "fsm.h"

synth_write_t synth_write_list[SYNTH_N] = {
    synth_write_sine,
    synth_write_square,
    synth_write_saw,
    synth_write_sine_sweep
};

char synth_names[SYNTH_N][7] = {
    "sine",
    "square",
    "saw",
    "sweep"
};

char synth_flags[SYNTH_ARGS_N][3] = {
    "-f",   // frequency
    "-a"    // amplitude
    // setting fs is a future issue
};

synth_cfg_t synth_cur;

#ifdef SYNTH_SWEEP_IN_MEM
int32_t sweep_data[SYNTH_SWEEP_LEN] = {
    #include "sweep_values.inc"
};
#endif

e_syserr_t synth_init(synth_cfg_t* cfg){
    if(cfg->amp > 1.0 || cfg->amp <= 0.0){ return e_syserr_param; }
    synth_cur.amp = cfg->amp;
    if(cfg->freq >= (cfg->fs/2)) { return e_syserr_param; } // Nyquist
    synth_cur.freq = cfg->freq;
    synth_cur.fs = cfg->fs;
    if(cfg->type >= SYNTH_N) { return e_syserr_param; }
    synth_cur.type = cfg->type;
    synth_cur._fstart = cfg->_fstart;
    synth_cur._fstop = cfg->_fstop;
    synth_cur.d_phase = 0.0;
    synth_cur.ul_phase = 0;
    return e_syserr_none;
}

e_syserr_t synth_init_default(void){
    synth_cfg_t scfg = SYNTH_CFG_DEFAULT;
    return synth_init(&scfg);
}

void synth_write(audio_sample_t* data, uint32_t n_samples){
    synth_write_list[synth_cur.type](data, n_samples);
}

void synth_write_sine(audio_sample_t* data, uint32_t n_samples){
    double phase_inc = (2.0f * M_PI * synth_cur.freq) / synth_cur.fs;
    for (uint32_t i = 0; i < n_samples; i++) {
        // Write to both channels (stereo)
        float sample_val = synth_cur.amp * sinf(synth_cur.d_phase);
        data[i].ch[0] = data[i].ch[1] = (audio_sample_base_t)(sample_val * INT32_MAX);
        synth_cur.d_phase += phase_inc;

        if (synth_cur.d_phase >= 2.0f * M_PI) synth_cur.d_phase -= 2.0f * M_PI;
    }
}

void synth_write_square(audio_sample_t* data, uint32_t n_samples){
    uint32_t period_s = synth_cur.fs / synth_cur.freq;
    audio_sample_base_t amplitude = (audio_sample_base_t)(synth_cur.amp * INT32_MAX);
    for (uint32_t i = 0; i < n_samples; i++) {
        data[i].ch[0] = data[i].ch[1] = (synth_cur.ul_phase < (period_s/2) ? amplitude : -amplitude);
        synth_cur.ul_phase = (synth_cur.ul_phase + 1) % period_s;
    }
}

void synth_write_saw(audio_sample_t* data, uint32_t n_samples) {
    static uint32_t phase = 0;
    uint32_t period_s = synth_cur.fs / synth_cur.freq;
    audio_sample_base_t amplitude = (audio_sample_base_t)(synth_cur.amp * INT32_MAX);
    uint32_t phase_step = UINT32_MAX / period_s;
    for (uint32_t i = 0; i < n_samples; i++) {
        // Saw wave: linear ramp from -amp to +amp
        data[i].ch[0] = data[i].ch[1] = ((int64_t)synth_cur.ul_phase * amplitude / (UINT32_MAX / 2)) - amplitude;
        synth_cur.ul_phase += phase_step;
    }
}

#ifdef SYNTH_SWEEP_IN_MEM
void synth_write_sine_sweep(audio_sample_t* data, uint32_t n_samples) {
    static uint32_t pos = 0;

    while (n_samples--) {
        int32_t sample = sweep_data[pos] << 16;
        data->ch[0] = (audio_sample_base_t)(sample /* * synth_cur.amp*/);
        data->ch[1] = (audio_sample_base_t)(-sample /* * synth_cur.amp*/);  // Inverted phase
        data++;
        
        if (++pos >= SYNTH_SWEEP_LEN) pos = 0;
    }
}
#else
void synth_write_sine_sweep(audio_sample_t* data, uint32_t n_samples) {
    return;
}
#endif

e_syserr_t synth_job_parse_args(char* synth_args, synth_cfg_t* pcfg){
    char* arg = strtok(synth_args, " ");
    if(arg == NULL) return e_syserr_none; // use default synth
    pcfg->type = SYNTH_N;

    for(uint16_t i = 0; i < SYNTH_N; i++){
        if(strcmp(arg, synth_names[i]) == 0){
            #ifndef SYNTH_SWEEP_IN_MEM
            if(i == synth_sweep){
                return e_syserr_uninitialized;
            }
            #endif
            pcfg->type = (synth_t)i; // synth types scale like the counter
            break;
        }
    }
    if(pcfg->type == SYNTH_N){
        return e_syserr_param;
    }
    uint32_t freq = pcfg->freq;
    float amp = pcfg->amp;

    for (uint8_t i = 0; i < SYNTH_ARGS_N; i++) {
        char* flag = strtok(NULL, " ");
        if (!flag) break; // default
        
        char* value = strtok(NULL, " ");
        if (!value) return e_syserr_param;
        
        if (strcmp(flag, "-f") == 0) {
            freq = atoi(value);
            if (freq == 0) return e_syserr_param;
        } 
        else if (strcmp(flag, "-a") == 0) {
            amp = atof(value);
        }
    }

    pcfg->freq = freq;
    pcfg->amp = amp;
    return e_syserr_none;
}

void synth_job(void* p){
    char* args = jes_job_get_args();
    char* arg = strtok(args, " ");
    job_struct_t* pj = (job_struct_t*)p;
    
    if(arg == NULL || strcmp(arg, "play") != 0){
        SCOPE_LOG_PJ(pj, "Usage: synth play [-f freq] [-a amp] [sine|square|saw|sweep]");
        return;
    }
    
    arg = strtok(NULL, " ");
    if(arg == NULL) {
        // Use default
        synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
        synth_init(&cfg);
        SCOPE_LOG_PJ(pj, "Playing default synth (sine, 1000Hz, 0.2 amp)");
    } else {
        // Parse args starting with type
        char synth_args[64];
        sprintf(synth_args, "%s %s", arg, strtok(NULL, "")); // Get rest of args
        synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
        e_syserr_t e = synth_job_parse_args(synth_args, &cfg);
        if(e != e_syserr_none) {
            SCOPE_LOG_PJ(pj, "Error parsing synth args: %d", e);
            return;
        }
        synth_init(&cfg);
        SCOPE_LOG_PJ(pj, "Playing synth: %s, freq=%d, amp=%.2f", 
                     synth_names[cfg.type], cfg.freq, cfg.amp);
    }
    
    // The actual synthesis would be triggered by the FSM
    // This job just configures the synth
}

#endif // FRX_ENABLE_MODULE_SYNTH
