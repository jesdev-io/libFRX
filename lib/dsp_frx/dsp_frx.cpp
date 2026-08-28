#ifdef FRX_ENABLE_MODULE_DSP_FRX

#include <memory.h>
#include "dsp_frx.h"
#include "syserr.h"

float dsp_frx_sin_bhaskara_I(float x) {
    // https://en.wikipedia.org/wiki/Bh%C4%81skara_I%27s_sine_approximation_formula
    x = fmodf(x, 2.0f * M_PI); // Wrap to [-π, π]
    float x_pi_minus_x = x * (M_PI - x);
    return (16.0f * x_pi_minus_x) / (5 * M_PI * M_PI - 4 * x_pi_minus_x);
}

float dsp_frx_cos_bhaskara_I(float x) {
    // https://en.wikipedia.org/wiki/Bh%C4%81skara_I%27s_sine_approximation_formula
    x = fmodf(x, 2.0f * M_PI); // Wrap to [-π, π]
    return 1 - (5*DSP_FRX_SQUARE(x)/(M_PI * M_PI + DSP_FRX_SQUARE(x)));
}

audio_val_t dsp_frx_samples_to_msqr_32b(audio_sample_t* data, uint32_t len, uint8_t nch){
    audio_val_t msqr = {0};
    for(uint8_t i = 0; i < nch; i++){
        msqr.ch[i] = 0.;
    }
    for (size_t i = 0; i < len; i++){
        audio_val_t s = dsp_frx_sample_cleanup(data[i], nch);
        for(uint8_t j = 0; j < nch; j++){
            msqr.ch[j] += DSP_FRX_SQUARE(s.ch[j]);
        }
    }
    for(uint8_t i = 0; i < nch; i++){
        msqr.ch[i] /= len;
    }
    return msqr;
}

audio_val_t dsp_frx_samples_to_dbfs_32b(audio_sample_t* data, uint32_t len, uint8_t nch){
    audio_val_t msqr = dsp_frx_samples_to_msqr_32b(data, len, nch);
    audio_val_t dbfs;
    for(uint8_t i = 0; i < nch; i++){
        dbfs.ch[i] = 10*log10f(msqr.ch[i]);
    }
    return dbfs;
}

audio_val_t dsp_frx_samples_to_dbfs_32b_from_msqr(audio_val_t msqr, uint8_t nch){
    audio_val_t dbfs;
    for(uint8_t i = 0; i < nch; i++){
        dbfs.ch[i] = 10*log10f(msqr.ch[i]);
    }
    return dbfs;
}

audio_val_t dsp_frx_msqr_rolling_avg(audio_val_t msqr, uint8_t nch){
    static audio_val_t win[DSP_FRX_ROLL_AVG_N] = {0};
    static uint16_t roll_idx = 0;
    static uint16_t samples_count = 0;
    static audio_val_t running_sum = {0};
    if (samples_count == DSP_FRX_ROLL_AVG_N) {
        for(uint8_t i = 0; i < nch; i++){
            running_sum.ch[i] -= win[roll_idx].ch[i];
        }
    } else {
        samples_count++;
    }
    win[roll_idx] = msqr;
    for(uint8_t i = 0; i < nch; i++){
        running_sum.ch[i] += msqr.ch[i];
    }
    if (++roll_idx == DSP_FRX_ROLL_AVG_N) roll_idx = 0;
    audio_val_t msqr_avg;
    for(uint8_t i = 0; i < nch; i++){
        msqr_avg.ch[i] = running_sum.ch[i] / (float)samples_count;
    }
    return msqr_avg;
}

audio_val_t dsp_frx_sample_cleanup(audio_sample_t data, uint8_t nch){
    static audio_val_t dc_values = {0};
    audio_val_t scaled;
    for(uint8_t i = 0; i < nch; i++){
        data.ch[i] >>= 8;
        scaled.ch[i] = (float)data.ch[i] * DSP_FRX_INT24_SCALE;
        dc_values.ch[i] += DSP_FRX_DC_FILTER_ALPHA * (scaled.ch[i] - dc_values.ch[i]);
        scaled.ch[i] -= dc_values.ch[i];
    }
    return scaled;
}

float __dsp_frx_fc_ula_broadside(dsp_frx_ula_t* ula_cfg, float c){
    return c / (2*ula_cfg->dist);
}

void __dsp_frx_tf_ula_broadside(dsp_frx_ula_t* ula_cfg, float c, float* gains, float f, uint16_t steps){
    float step = M_TWOPI / steps;
    float angle = 0;
    for(uint16_t i = 0; i < steps; i++){
        float tau = ula_cfg->dist/c * cosf(angle);
        gains[i] = 2*cosf(M_TWOPI*f*tau/2);
        angle += step;
    }
}

e_syserr_t __dsp_frx_run_ula_broadside(audio_io_t* iobuf, dsp_frx_ula_t* ula_cfg){
    if(!iobuf || !iobuf->in || !iobuf->out || !ula_cfg) return e_syserr_param;
    uint8_t in_chs = 0;
    uint8_t out_chs = 0;
    for(uint8_t i = 0; i < AUDIO_MAX_NUM_CH; i++){
        if(ula_cfg->ch_in_select[i]) in_chs++;
        if(ula_cfg->ch_in_select[i]) out_chs++;
    }
    if(in_chs < 2) return e_syserr_param;
    if(out_chs < 1) return e_syserr_param;
    for(uint32_t n = 0; n < iobuf->len; n++){ // sample iterator
        audio_ovf_safe_sample_base_t acc = 0;
        for(uint8_t i = 0; i < AUDIO_MAX_NUM_CH; i++){ // input channel iterator
            if(!ula_cfg->ch_in_select[i]) continue;
            acc += iobuf->in[n].ch[i];
        }
        for(uint8_t i = 0; i < AUDIO_MAX_NUM_CH; i++){ // output channel iterator
            if(!ula_cfg->ch_out_select[i]) continue;
            iobuf->out[n].ch[i] = acc / in_chs;   
        }
    }
    return e_syserr_none;
}

#endif // FRX_ENABLE_MODULE_DSP_FRX
