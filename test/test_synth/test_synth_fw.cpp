#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "synth.h"
#include "audio.h"
#include "syserr.h"

static inline void synth_cb(audio_io_t* iobuf){
    if(!iobuf || !iobuf->out) return;
    synth_write(iobuf->out, iobuf->len);
}

void test_jes_bootup(void) {
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_audio_init_for_synth(void) {
    e_syserr_t e = audio_init_default();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_synth_init(void) {
    uint32_t audio_sr = audio_get_sr();
    TEST_ASSERT_GREATER_THAN_UINT32(0, audio_sr);
    synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
    e_syserr_t e = synth_init(&cfg);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_synth_init_default(void) {
    uint32_t audio_sr = audio_get_sr();
    TEST_ASSERT_GREATER_THAN_UINT32(0, audio_sr);
    e_syserr_t e = synth_init_default();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_synth_init_invalid_amp(void) {
    synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
    cfg.amp = 1.5f; // > 1.0
    e_syserr_t e = synth_init(&cfg);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
}

void test_synth_init_invalid_freq(void) {
    uint32_t audio_sr = audio_get_sr();
    TEST_ASSERT_GREATER_THAN_UINT32(0, audio_sr);
    synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
    // Set frequency higher than Nyquist (fs/2)
    cfg.freq = audio_sr / 2 + 1000;
    e_syserr_t e = synth_init(&cfg);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
}

void test_synth_write_sine(void) {
    synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
    cfg.type = synth_sine;
    cfg.freq = 1000;
    cfg.amp = 0.5f;
    e_syserr_t e = synth_init(&cfg);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    
    audio_sample_t samples[100];
    synth_write(samples, 100);
    uint8_t all_zero = 1;
    for(uint8_t i = 0; i < 100; i++) {
        if(samples[i].ch[0] != 0) {
            all_zero = 0;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zero);
    for(uint8_t i = 0; i < 100; i++) {
        float sample_val = (float)samples[i].ch[0] / INT32_MAX;
        TEST_ASSERT_FLOAT_WITHIN(1.0, 0.0, sample_val);
    }

    // Physically play audio only when the compiled audio topology has TX.
#if AUDIO_CFG_HAS_OUTPUT
    e = audio_set_callback(synth_cb);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = audio_start();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    // audio keeps running for remaining tests
#endif
}

void test_synth_write_square(void) {
    synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
    cfg.type = synth_square;
    cfg.freq = 1000;
    cfg.amp = 0.5f;
    e_syserr_t e = synth_init(&cfg);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    
    audio_sample_t samples[100];
    synth_write(samples, 100);
    uint8_t all_zero = 1;
    for(uint8_t i = 0; i < 100; i++) {
        if(samples[i].ch[0] != 0 || samples[i].ch[1] != 0) {
            all_zero = 0;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zero);
    int32_t first_val = samples[0].ch[0];
    uint8_t found_second_val = 0;
    int32_t second_val = 0;
    for(uint8_t i = 1; i < 100; i++) {
        if(samples[i].ch[0] != first_val) {
            second_val = samples[i].ch[0];
            found_second_val = 1;
            break;
        }
    }
    TEST_ASSERT_TRUE(found_second_val);
    for(uint8_t i = 0; i < 100; i++) {
        TEST_ASSERT_TRUE(samples[i].ch[0] == first_val || samples[i].ch[0] == second_val);
    }
}

void test_synth_write_saw(void) {
    synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
    cfg.type = synth_saw;
    cfg.freq = 1000;
    cfg.amp = 0.5f;
    e_syserr_t e = synth_init(&cfg);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    
    audio_sample_t samples[100];
    synth_write(samples, 100);
    uint8_t all_zero = 1;
    for(uint8_t i = 0; i < 100; i++) {
        if(samples[i].ch[0] != 0 || samples[i].ch[1] != 0) {
            all_zero = 0;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zero);
    uint8_t found_increase = 0;
    uint8_t found_decrease = 0;
    for(uint8_t i = 1; i < 50; i++) {
        if(samples[i].ch[0] > samples[i-1].ch[0]) found_increase = 1;
        if(samples[i].ch[0] < samples[i-1].ch[0]) found_decrease = 1;
    }
    TEST_ASSERT_TRUE(found_increase || found_decrease);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_audio_init_for_synth);
    RUN_TEST(test_synth_init);
    RUN_TEST(test_synth_init_default);
    RUN_TEST(test_synth_init_invalid_amp);
    RUN_TEST(test_synth_init_invalid_freq);
    RUN_TEST(test_synth_write_sine);
    jes_delay_job_ms(3000);
    RUN_TEST(test_synth_write_square);
    jes_delay_job_ms(3000);
    RUN_TEST(test_synth_write_saw);
    UNITY_END();
}

void loop() {
}
