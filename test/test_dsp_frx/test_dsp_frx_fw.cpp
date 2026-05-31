#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "dsp_frx.h"
#include "audio.h"
#include "syserr.h"

const float bhaskara_delta = 0.6;

void test_dsp_sin_cos_bhaskara(void) {
    float result; 
    result = dsp_frx_sin_bhaskara_I(0);
    TEST_ASSERT_FLOAT_WITHIN(bhaskara_delta, 0.0, result);
    result = dsp_frx_sin_bhaskara_I(M_PI/2);
    TEST_ASSERT_FLOAT_WITHIN(bhaskara_delta, 1.0, result);
    result = dsp_frx_sin_bhaskara_I(M_PI);
    TEST_ASSERT_FLOAT_WITHIN(bhaskara_delta, 0.0, result);
    result = dsp_frx_cos_bhaskara_I(0);
    TEST_ASSERT_FLOAT_WITHIN(bhaskara_delta, 1.0, result);
    result = dsp_frx_cos_bhaskara_I(M_PI/2);
    TEST_ASSERT_FLOAT_WITHIN(bhaskara_delta, 0.0, result);
    result = dsp_frx_cos_bhaskara_I(M_PI);
    TEST_ASSERT_FLOAT_WITHIN(bhaskara_delta, -1.0, result);
}

void test_dsp_msqr_conversion(void) {
    audio_sample_t samples[4];
    for (int i = 0; i < 4; i++) {
        samples[i].ch[0] = 1000000;
        samples[i].ch[1] = 500000;
    }
    audio_val_t result = dsp_frx_samples_to_msqr_32b(samples, 4, 2);
    TEST_ASSERT_EQUAL(true, result.ch[0] > 0);
    TEST_ASSERT_EQUAL(true, result.ch[1] > 0);
}

void test_dsp_sample_cleanup(void) {
    audio_sample_t sample;
    audio_val_t result;
    // large DC value
    sample.ch[0] = 1000000;
    sample.ch[1] = 500000;
    for(uint8_t i = 0; i < 10; i++) {
        result = dsp_frx_sample_cleanup(sample, 2);
    }
    // Output value will converge to zero after enough steps
    TEST_ASSERT_FLOAT_WITHIN(100000, 0.0, result.ch[0]);
    TEST_ASSERT_FLOAT_WITHIN(100000, 0.0, result.ch[1]);
}

void test_dsp_dbfs_conversion(void) {
    audio_sample_t samples[4];
    for (int i = 0; i < 4; i++) {
        samples[i].ch[0] = 1000000;
        samples[i].ch[1] = 1000000;
    }
    audio_val_t result = dsp_frx_samples_to_dbfs_32b(samples, 4, 2);
    TEST_ASSERT_EQUAL(true, result.ch[0] < 0);
    TEST_ASSERT_EQUAL(true, result.ch[1] < 0);
}

void test_dsp_msqr_rolling_avg(void) {
    audio_val_t constant_value;
    for(uint8_t i = 0; i < AUDIO_MAX_NUM_CH; i++) {
        constant_value.ch[i] = 5.0f;
    }
    for (int i = 0; i < DSP_FRX_ROLL_AVG_N * 2; i++) {
        audio_val_t result = dsp_frx_msqr_rolling_avg(constant_value, 2);
        if (i >= DSP_FRX_ROLL_AVG_N) {
            TEST_ASSERT_FLOAT_WITHIN(0.1, constant_value.ch[0], result.ch[0]);
            TEST_ASSERT_FLOAT_WITHIN(0.1, constant_value.ch[1], result.ch[1]);
        }
    }
    audio_val_t different_value;
    for(uint8_t i = 0; i < AUDIO_MAX_NUM_CH; i++) {
        different_value.ch[i] = 15.0f;
    }
    audio_val_t result1 = dsp_frx_msqr_rolling_avg(different_value, 2);
    TEST_ASSERT_TRUE(fabs(result1.ch[0] - constant_value.ch[0]) > 0.5);
    TEST_ASSERT_TRUE(fabs(result1.ch[1] - constant_value.ch[1]) > 0.5);
    for (int i = 0; i < DSP_FRX_ROLL_AVG_N; i++) {
        audio_val_t result = dsp_frx_msqr_rolling_avg(different_value, 2);
    }
    audio_val_t final_result = dsp_frx_msqr_rolling_avg(different_value, 2);
    TEST_ASSERT_FLOAT_WITHIN(0.1, different_value.ch[0], final_result.ch[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.1, different_value.ch[1], final_result.ch[1]);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_dsp_sin_cos_bhaskara);
    RUN_TEST(test_dsp_sample_cleanup);
    RUN_TEST(test_dsp_msqr_conversion);
    RUN_TEST(test_dsp_dbfs_conversion);
    RUN_TEST(test_dsp_msqr_rolling_avg);
    UNITY_END();
}

void loop() {
}
