#include <Arduino.h>
#include <jescore.h>
#include "ext_flash.h"
#include "syserr.h"

#ifdef UNIT_TEST
#include <unity.h>
#else
#define TEST_ASSERT_EQUAL(ex, ac)
#define RUN_TEST(test) test()
#endif

void test_ef_init(void) {
    e_syserr_t e = ef_init();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_ef_init_default(void) {
    e_syserr_t e = ef_init_default();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_ef_deinit(void) {
    e_syserr_t e = ef_init();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_deinit();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_ef_uninitialized_deinit(void) {
    e_syserr_t e = ef_deinit();
    TEST_ASSERT_EQUAL(e_syserr_uninitialized, e);
}

void setup() {
    #ifdef UNIT_TEST
    delay(2000);
    UNITY_BEGIN();
    #endif
    
    RUN_TEST(test_ef_init);
    RUN_TEST(test_ef_init_default);
    RUN_TEST(test_ef_deinit);
    RUN_TEST(test_ef_uninitialized_deinit);

    #ifdef UNIT_TEST
    UNITY_END();
    #endif
}

void loop() {
}
