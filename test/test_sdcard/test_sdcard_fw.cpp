#include <Arduino.h>
#include <jescore.h>
#include "sdcard.h"
#include "syserr.h"

#ifdef UNIT_TEST
#include <unity.h>
#else
#define TEST_ASSERT_EQUAL(ex, ac)
#define RUN_TEST(test) test()
#endif

void test_sd_init(void) {
    e_syserr_t e = sd_init(5, 0, sd_mode_spi);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_sd_init_default(void) {
    e_syserr_t e = sd_init_default();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_sd_is_not_mounted(void) {
    uint8_t mounted = sd_is_mounted();
    TEST_ASSERT_EQUAL(0, mounted);
}

void setup() {
    #ifdef UNIT_TEST
    delay(2000);
    UNITY_BEGIN();
    #endif
    
    RUN_TEST(test_sd_init);
    RUN_TEST(test_sd_init_default);
    RUN_TEST(test_sd_is_not_mounted);

    #ifdef UNIT_TEST
    UNITY_END();
    #endif
}

void loop() {
}
