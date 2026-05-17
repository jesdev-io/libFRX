#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "audio.h"
#include "syserr.h"

static inline void feedthrough_cb(audio_sample_t* buf){
    static uint8_t frame_pos = 0;
    audio_read(&buf[AUDIO_FRAME_LEN*(frame_pos)], AUDIO_FRAME_LEN, 32, AUDIO_MAX_NUM_CH);
    audio_write(&buf[AUDIO_FRAME_LEN*(!frame_pos)], AUDIO_FRAME_LEN, 32, AUDIO_MAX_NUM_CH);
    frame_pos = !frame_pos;
}

void test_jes_bootup(void) {
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_audio_init_default(void) {
    e_syserr_t e = audio_init_default();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_audio_run(void) {
    audio_set_callback(feedthrough_cb);
    jes_err_t je = jes_launch_job(AUDIO_SERVER_JOB_NAME);
    TEST_ASSERT_EQUAL(e_err_no_err, je);
}

void test_audio_set_callback(void) {
    e_syserr_t e = audio_set_callback(feedthrough_cb);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_audio_init_invalid_sr(void) {
    e_syserr_t e = audio_init(99999, 
                              AUDIO_PIN_MEMS_I2S_BCLK, 
                              AUDIO_PIN_MEMS_I2S_WS, 
                              AUDIO_PIN_MEMS_I2S_IN, 
                              AUDIO_PIN_DAC_I2S_OUT);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
}

void test_audio_reinit_44100(void) {
    e_syserr_t e = audio_init(AUDIO_SR_44100, 
                              AUDIO_PIN_MEMS_I2S_BCLK, 
                              AUDIO_PIN_MEMS_I2S_WS, 
                              AUDIO_PIN_MEMS_I2S_IN,
                              AUDIO_PIN_DAC_I2S_OUT);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_audio_reinit_96000(void) {
    e_syserr_t e = audio_init(AUDIO_SR_96000, 
                              AUDIO_PIN_MEMS_I2S_BCLK, 
                              AUDIO_PIN_MEMS_I2S_WS, 
                              AUDIO_PIN_MEMS_I2S_IN,
                              AUDIO_PIN_DAC_I2S_OUT);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_audio_init_default);
    RUN_TEST(test_audio_run);
    RUN_TEST(test_audio_set_callback);
    // RUN_TEST(test_audio_init_invalid_sr);
    // jes_delay_job_ms(1000);
    // RUN_TEST(test_audio_reinit_44100);
    // jes_delay_job_ms(1000);
    // RUN_TEST(test_audio_reinit_96000);
    jes_delay_job_ms(2000);
    UNITY_END();
}

void loop() {
}
