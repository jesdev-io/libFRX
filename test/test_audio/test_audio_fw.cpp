#include <Arduino.h>
#include <jescore.h>
#include "audio.h"
#include "syserr.h"

#ifdef UNIT_TEST
#include <unity.h>
#else
#define TEST_ASSERT_EQUAL(ex, ac)
#define RUN_TEST(test) test()
#define TEST_ASSERT_FLOAT_WITHIN(delta, ex, ac)
#define TEST_ASSERT_LESS_THAN_UINT32(ex, ac)
#endif

void test_jes_bootup(void) {
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_audio_init_default(void) {
    e_syserr_t e = audio_init_default();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_audio_init_custom(void) {
    e_syserr_t e = audio_init(48000, 23, 4, 27);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_audio_init_invalid_sr(void) {
    e_syserr_t e = audio_init(99999, 23, 4, 27);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
}

void test_audio_get_buffer(void) {
    audio_sample_t* buf = _audio_get_buffer();
    TEST_ASSERT_EQUAL(true, buf != NULL);
}

void setup() {
    #ifdef UNIT_TEST
    delay(2000);
    UNITY_BEGIN();
    #endif
    
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_audio_init_default);
    RUN_TEST(test_audio_init_custom);
    RUN_TEST(test_audio_init_invalid_sr);
    RUN_TEST(test_audio_get_buffer);

    #ifdef UNIT_TEST
    UNITY_END();
    #endif
}

void loop() {
}
