#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "i2c_rtc_ds3231.h"

void test_jes_bootup(void){
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_rtc_init_default(void){
    TEST_ASSERT_EQUAL(e_syserr_none, i2c_rtc_ds3231_init_default());
    TEST_ASSERT_EQUAL_UINT8(1, i2c_rtc_ds3231_is_initialized());
}

void test_rtc_set_time_optional(void){
#ifdef RTC_RESET_TIME
    struct tm timeinfo;
    memset(&timeinfo, 0, sizeof(timeinfo));
    timeinfo.tm_sec = 0;
    timeinfo.tm_min = FR2_LAST_FLASH_MINUTE;
    timeinfo.tm_hour = FR2_LAST_FLASH_HOUR;
    timeinfo.tm_wday = FR2_LAST_FLASH_WDAY;
    timeinfo.tm_mday = FR2_LAST_FLASH_DAY;
    timeinfo.tm_mon = FR2_LAST_FLASH_MONTH - 1;
    timeinfo.tm_year = FR2_LAST_FLASH_YEAR - 1900;
    TEST_ASSERT_EQUAL(e_syserr_none, i2c_rtc_ds3231_set_time(&timeinfo));
#endif
}

void test_rtc_get_time(void){
    struct tm timeinfo;
    TEST_ASSERT_EQUAL(e_syserr_none, i2c_rtc_ds3231_get_time(&timeinfo));
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, timeinfo.tm_sec);
    TEST_ASSERT_LESS_OR_EQUAL_INT(59, timeinfo.tm_sec);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, timeinfo.tm_min);
    TEST_ASSERT_LESS_OR_EQUAL_INT(59, timeinfo.tm_min);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, timeinfo.tm_hour);
    TEST_ASSERT_LESS_OR_EQUAL_INT(23, timeinfo.tm_hour);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(1, timeinfo.tm_mday);
    TEST_ASSERT_LESS_OR_EQUAL_INT(31, timeinfo.tm_mday);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, timeinfo.tm_mon);
    TEST_ASSERT_LESS_OR_EQUAL_INT(11, timeinfo.tm_mon);
    TEST_ASSERT_EQUAL_INT(FR2_LAST_FLASH_YEAR, timeinfo.tm_year + 1900);
}

void test_rtc_get_temp(void){
    float temp = 0.0f;
    TEST_ASSERT_EQUAL(e_syserr_none, i2c_rtc_ds3231_get_temp(&temp));
    TEST_ASSERT_FLOAT_WITHIN(40.0f, 25.0f, temp);
}

void setup(){
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_rtc_init_default);
    RUN_TEST(test_rtc_set_time_optional);
    RUN_TEST(test_rtc_get_time);
    RUN_TEST(test_rtc_get_temp);
    UNITY_END();
}

void loop(){
}
