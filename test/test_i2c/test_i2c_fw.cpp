#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "i2c_base.h"
#include "syserr.h"

void test_jes_bootup(void) {
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

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
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_i2c_scan_bus(void) {
    uint8_t found_devices[32];
    int32_t device_count;
    device_count = i2c_base_scan_bus(NULL, 32, pdMS_TO_TICKS(100));
    TEST_ASSERT_EQUAL(e_syserr_param, device_count);
    device_count = i2c_base_scan_bus(found_devices, 0, pdMS_TO_TICKS(100));
    TEST_ASSERT_EQUAL(0, device_count);
    device_count = i2c_base_scan_bus(found_devices, 32, pdMS_TO_TICKS(10));
    TEST_ASSERT_GREATER_OR_EQUAL(0, device_count);
    if(device_count > 0) {
        for(int32_t i = 0; i < device_count; i++) {
            TEST_ASSERT_GREATER_OR_EQUAL(0x08, found_devices[i]);
            TEST_ASSERT_LESS_OR_EQUAL(0xFF, found_devices[i]);
        }
    }
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_i2c_init);
    RUN_TEST(test_i2c_init_default);
    RUN_TEST(test_i2c_double_init);
    RUN_TEST(test_i2c_scan_bus);
    UNITY_END();
}

void loop() {
}
