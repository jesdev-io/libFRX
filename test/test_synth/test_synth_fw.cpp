#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "synth.h"
#include "audio.h"
#include "syserr.h"

void test_synth_init(void) {
    synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
    e_syserr_t e = synth_init(&cfg);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_synth_init_default(void) {
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
    synth_cfg_t cfg = SYNTH_CFG_DEFAULT;
    cfg.freq = 100000; // > fs/2 (Nyquist)
    e_syserr_t e = synth_init(&cfg);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_synth_init);
    RUN_TEST(test_synth_init_default);
    RUN_TEST(test_synth_init_invalid_amp);
    RUN_TEST(test_synth_init_invalid_freq);
    UNITY_END();
}

void loop() {
}
