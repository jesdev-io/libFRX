#include <Arduino.h>
#include <jescore.h>
#include "utils.h"
#include "syserr.h"
#include <string.h>

#ifdef UNIT_TEST
#include <unity.h>
#else
#define TEST_ASSERT_EQUAL(ex, ac)
#define TEST_ASSERT_EQUAL_STRING(ex, ac) (strcmp(ex, ac) == 0)
#define RUN_TEST(test) test()
#endif

void test_uint_to_4digit_str(void) {
    char buf[5];
    e_syserr_t e = uint_to_4digit_str(123, buf);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL_STRING("0123", buf);
    
    e = uint_to_4digit_str(0, buf);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL_STRING("0000", buf);
    
    e = uint_to_4digit_str(9999, buf);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL_STRING("9999", buf);
}

void test_uint_to_4digit_str_too_large(void) {
    char buf[5];
    e_syserr_t e = uint_to_4digit_str(10000, buf);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
}

void test_uint_to_4digit_str_null_buf(void) {
    e_syserr_t e = uint_to_4digit_str(123, NULL);
    TEST_ASSERT_EQUAL(e_syserr_null, e);
}

void test_str_to_4digit_uint(void) {
    uint16_t result;
    e_syserr_t e = str_to_4digit_uint("1234", &result);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(1234, result);
}

void test_str_to_4digit_uint_invalid(void) {
    uint16_t result;
    e_syserr_t e = str_to_4digit_uint("123", &result); // Too short
    TEST_ASSERT_EQUAL(e_syserr_param, e);
    
    e = str_to_4digit_uint("12345", &result); // Too long
    TEST_ASSERT_EQUAL(e_syserr_param, e);
    
    e = str_to_4digit_uint("12ab", &result); // Non-digit
    TEST_ASSERT_EQUAL(e_syserr_param, e);
}

void setup() {
    #ifdef UNIT_TEST
    delay(2000);
    UNITY_BEGIN();
    #endif
    
    RUN_TEST(test_uint_to_4digit_str);
    RUN_TEST(test_uint_to_4digit_str_too_large);
    RUN_TEST(test_uint_to_4digit_str_null_buf);
    RUN_TEST(test_str_to_4digit_uint);
    RUN_TEST(test_str_to_4digit_uint_invalid);

    #ifdef UNIT_TEST
    UNITY_END();
    #endif
}

void loop() {
}
