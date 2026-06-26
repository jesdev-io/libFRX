#ifndef _AUDIO_DEFAULT_CFG_H_
#define _AUDIO_DEFAULT_CFG_H_

#ifdef FRX_ENABLE_MODULE_AUDIO

#ifndef AUDIO_SERVER_JOB_MEM
#define AUDIO_SERVER_JOB_MEM    (4096)
#endif

#ifndef AUDIO_FRAME_LEN
#define AUDIO_FRAME_LEN         (1024)
#endif

#ifndef AUDIO_MAX_NUM_CH
#define AUDIO_MAX_NUM_CH        2
#endif

#if AUDIO_MAX_NUM_CH>4
#error "Hardware platform only supports up to 4 channels"
#endif

#if AUDIO_MAX_NUM_CH<1
#error "Audio module needs at least one audio channel to be active!"
#endif   

#ifndef AUDIO_PIN_I2S_BCLK
#error "AUDIO_PIN_I2S_BCLK must be defined in platformio.ini"
#endif

#ifndef AUDIO_PIN_I2S_WS
#error "AUDIO_PIN_I2S_WS must be defined in platformio.ini"
#endif

#ifndef AUDIO_PIN_I2S_IN_A
#error "AUDIO_PIN_I2S_IN_A must be defined in platformio.ini"
#endif

#ifndef AUDIO_PIN_I2S_OUT_A
#error "AUDIO_PIN_I2S_OUT_A must be defined in platformio.ini"
#endif

#if AUDIO_MAX_NUM_CH>2

#ifndef AUDIO_PIN_I2S_IN_B
#error "AUDIO_PIN_I2S_IN_B must be defined in platformio.ini"
#endif

#ifndef AUDIO_PIN_I2S_OUT_B
#error "AUDIO_PIN_I2S_OUT_B must be defined in platformio.ini"
#endif
#endif // AUDIO_MAX_NUM_CH>2

#ifndef AUDIO_BPS_DEFAULT
#define AUDIO_BPS_DEFAULT       32
#endif

#ifndef AUDIO_SR_44100
#define AUDIO_SR_44100          44100
#endif

#ifndef AUDIO_SR_48000
#define AUDIO_SR_48000          48000
#endif

#ifndef AUDIO_SR_96000
#define AUDIO_SR_96000          96000
#endif

#ifndef AUDIO_SR_MAX
#define AUDIO_SR_MAX            AUDIO_SR_96000
#endif

#ifndef AUDIO_SR_DEFAULT
#define AUDIO_SR_DEFAULT        AUDIO_SR_48000
#endif

#ifndef AUDIO_I2S_RESTART_MS
#define AUDIO_I2S_RESTART_MS    40
#endif

#define AUDIO_BANKS_CFG_SINGLE_MONO_I { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_i, \
        .ch = audio_i2s_ch_mono, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_A, \
        .data_tx = -1 \
    } \
}

#define AUDIO_BANKS_CFG_SINGLE_MONO_O { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_o, \
        .ch = audio_i2s_ch_mono, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = -1, \
        .data_tx = AUDIO_PIN_I2S_OUT_A \
    } \
}

#define AUDIO_BANKS_CFG_SINGLE_MONO_IO { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_io, \
        .ch = audio_i2s_ch_mono, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_A, \
        .data_tx = AUDIO_PIN_I2S_OUT_A \
    } \
}

#define AUDIO_BANKS_CFG_SINGLE_STEREO_I { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_i, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_A, \
        .data_tx = -1 \
    } \
}

#define AUDIO_BANKS_CFG_SINGLE_STEREO_O { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_o, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = -1, \
        .data_tx = AUDIO_PIN_I2S_OUT_A \
    } \
}

#define AUDIO_BANKS_CFG_SINGLE_STEREO_IO { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_io, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_A, \
        .data_tx = AUDIO_PIN_I2S_OUT_A \
    } \
}

#if AUDIO_MAX_NUM_CH > 2


#define AUDIO_BANKS_CFG_SINGLE_MONO_SINGLE_STEREO_I { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_i, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_A, \
        .data_tx = -1 \
    } \
    ,{ \
        .bank = audio_i2s_bank_b, \
        .ad = audio_i2s_bank_devi_i, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_B, \
        .data_tx = -1 \
    }
}

#define AUDIO_BANKS_CFG_SINGLE_MONO_SINGLE_STEREO_O { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_o, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = -1, \
        .data_tx = AUDIO_PIN_I2S_OUT_A \
    } \
    ,{ \
        .bank = audio_i2s_bank_b, \
        .ad = audio_i2s_bank_devi_o, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = -1, \
        .data_tx = AUDIO_PIN_I2S_OUT_B \
    }
}

#define AUDIO_BANKS_CFG_SINGLE_MONO_SINGLE_STEREO_IO { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_io, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_A, \
        .data_tx = AUDIO_PIN_I2S_OUT_A \
    } \
    ,{ \
        .bank = audio_i2s_bank_b, \
        .ad = audio_i2s_bank_devi_io, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_B, \
        .data_tx = AUDIO_PIN_I2S_OUT_B \
    }
}


#define AUDIO_BANKS_CFG_DOUBLE_STEREO_I { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_i, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_A, \
        .data_tx = -1 \
    } \
    ,{ \
        .bank = audio_i2s_bank_b, \
        .ad = audio_i2s_bank_devi_i, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_B, \
        .data_tx = -1 \
    }
}

#define AUDIO_BANKS_CFG_DOUBLE_STEREO_O { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_o, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = -1, \
        .data_tx = AUDIO_PIN_I2S_OUT_A \
    } \
    ,{ \
        .bank = audio_i2s_bank_b, \
        .ad = audio_i2s_bank_devi_o, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = -1, \
        .data_tx = AUDIO_PIN_I2S_OUT_B \
    }
}

#define AUDIO_BANKS_CFG_DOUBLE_STEREO_IO { \
    { \
        .bank = audio_i2s_bank_a, \
        .ad = audio_i2s_bank_host_io, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_A, \
        .data_tx = AUDIO_PIN_I2S_OUT_A \
    } \
    ,{ \
        .bank = audio_i2s_bank_b, \
        .ad = audio_i2s_bank_devi_io, \
        .ch = audio_i2s_ch_stereo, \
        .bclk = AUDIO_PIN_I2S_BCLK, \
        .ws = AUDIO_PIN_I2S_WS, \
        .data_rx = AUDIO_PIN_I2S_IN_B, \
        .data_tx = AUDIO_PIN_I2S_OUT_B \
    }
}

#ifndef AUDIO_BANKS_CFG_DEFAULT
#define AUDIO_BANKS_CFG_DEFAULT AUDIO_BANKS_CFG_SINGLE_STEREO_IO
#endif 

#endif // #if AUDIO_MAX_NUM_CH > 2

#endif // FRX_ENABLE_MODULE_AUDIO

#endif // _AUDIO_DEFAULT_CFG_H_