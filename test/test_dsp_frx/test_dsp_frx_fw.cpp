#include <Arduino.h>
#include <jescore.h>
#include "dsp_frx.h"
#include "audio.h"
#include "syserr.h"

#ifdef UNIT_TEST
#include <unity.h>
#else
#define TEST_ASSERT_EQUAL(ex, ac)
#define TEST_ASSERT_FLOAT_WITHIN(delta, ex, ac)
#define RUN_TEST(test) test()
#endif

void test_dsp_sin_bhaskara(void) {
    float result = dsp_frx_sin_bhaskara_I(0);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, result);
    
    result = dsp_frx_sin_bhaskara_I(M_PI/2);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 1.0, result);
    
    result = dsp_frx_sin_bhaskara_I(M_PI);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 0.0, result);
}

void test_dsp_msqr_conversion(void) {
    audio_sample_t samples[4];
    for (int i = 0; i < 4; i++) {
        samples[i].ch[0] = 1000000;  // Large value
        samples[i].ch[1] = 500000;
    }
    
    audio_val_t result = dsp_frx_samples_to_msqr_32b(samples, 4, 2);
    // Verify non-zero result
    TEST_ASSERT_EQUAL(true, result.ch[0] > 0);
    TEST_ASSERT_EQUAL(true, result.ch[1] > 0);
}

void test_dsp_dbfs_conversion(void) {
    audio_sample_t samples[4];
    for (int i = 0; i < 4; i++) {
        samples[i].ch[0] = 1000000;
        samples[i].ch[1] = 1000000;
    }
    
    audio_val_t result = dsp_frx_samples_to_dbfs_32b(samples, 4, 2);
    // dBFS should be negative for values below full scale
    TEST_ASSERT_EQUAL(true, result.ch[0] < 0);
    TEST_ASSERT_EQUAL(true, result.ch[1] < 0);
}

void setup() {
    #ifdef UNIT_TEST
    delay(2000);
    UNITY_BEGIN();
    #endif
    
    RUN_TEST(test_dsp_sin_bhaskara);
    RUN_TEST(test_dsp_msqr_conversion);
    RUN_TEST(test_dsp_dbfs_conversion);

    #ifdef UNIT_TEST
    UNITY_END();
    #endif
}

void loop() {
}
