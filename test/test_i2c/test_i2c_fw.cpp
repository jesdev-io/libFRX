#include <Arduino.h>
#include <jescore.h>
#include "i2c_base.h"
#include "syserr.h"

#ifdef UNIT_TEST
#include <unity.h>
#else
#define TEST_ASSERT_EQUAL(ex, ac)
#define RUN_TEST(test) test()
#endif

void test_i2c_init(void) {
    e_syserr_t e = i2c_base_init(21, 20, 100000);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_i2c_init_default(void) {
    e_syserr_t e = i2c_base_init_default();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_i2c_double_init(void) {
    e_syserr_t e = i2c_base_init(21, 20, 100000);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = i2c_base_init(21, 20, 100000);
    TEST_ASSERT_EQUAL(e_syserr_none, e); // Should succeed on second call
}

void setup() {
    #ifdef UNIT_TEST
    delay(2000);
    UNITY_BEGIN();
    #endif
    
    RUN_TEST(test_i2c_init);
    RUN_TEST(test_i2c_init_default);
    RUN_TEST(test_i2c_double_init);

    #ifdef UNIT_TEST
    UNITY_END();
    #endif
}

void loop() {
}
