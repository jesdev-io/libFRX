#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "ext_flash.h"
#include "syserr.h"

#define TEST_SECTOR_ADDR    0x1000
#define TEST_PAGE_ADDR      0x1100
#define TEST_DATA_SIZE      256
#define TEST_PATTERN        0xAA

static uint8_t test_write_data[TEST_DATA_SIZE];
static uint8_t test_read_data[TEST_DATA_SIZE];

void test_jes_bootup(void) {
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_ef_init_deinit(void) {
    uint32_t dummy_id;
    e_syserr_t e = ef_read_id(&dummy_id);
    TEST_ASSERT_EQUAL(e_syserr_uninitialized, e);
    e = ef_init();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_init();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_deinit();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_read_id(&dummy_id);
    TEST_ASSERT_EQUAL(e_syserr_uninitialized, e);
    e = ef_init();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_ef_read_id(void) {
    uint32_t flash_id;
    e_syserr_t e = ef_read_id(NULL);
    TEST_ASSERT_EQUAL(e_syserr_null, e);
    e = ef_read_id(&flash_id);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_GREATER_THAN_INT32(0, flash_id);
    TEST_ASSERT_EQUAL(0xEF, (flash_id >> 16) & 0xFF);
}

void test_ef_basic_rw(void) {
    memset(test_write_data, TEST_PATTERN, TEST_DATA_SIZE);
    memset(test_read_data, 0, TEST_DATA_SIZE);
    e_syserr_t e = ef_read(TEST_PAGE_ADDR, NULL, TEST_DATA_SIZE);
    TEST_ASSERT_EQUAL(e_syserr_null, e);

    e = ef_write_page(TEST_PAGE_ADDR, NULL, TEST_DATA_SIZE);
    TEST_ASSERT_EQUAL(e_syserr_null, e);
    e = ef_read(TEST_PAGE_ADDR, test_read_data, 0);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
    e = ef_write_page(TEST_PAGE_ADDR, test_write_data, 0);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
    e = ef_erase_sector(TEST_SECTOR_ADDR);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_write_page(TEST_PAGE_ADDR, test_write_data, TEST_DATA_SIZE);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_read(TEST_PAGE_ADDR, test_read_data, TEST_DATA_SIZE);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL_MEMORY(test_write_data, test_read_data, TEST_DATA_SIZE);
}

void test_ef_boundary_conditions(void) {
    e_syserr_t e = ef_write_page(TEST_PAGE_ADDR, test_write_data, 256);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_write_page(TEST_PAGE_ADDR, test_write_data, 257);
    TEST_ASSERT_EQUAL(e_syserr_too_long, e);
    e = ef_read(TEST_PAGE_ADDR + 250, test_read_data, 10);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_read(0xFFFFFF, test_read_data, 1);
    TEST_ASSERT_TRUE(e == e_syserr_none || e == e_syserr_driver_fail);
}

void test_ef_erase_operations(void) {
    e_syserr_t e = ef_write_page(TEST_PAGE_ADDR, test_write_data, TEST_DATA_SIZE);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = ef_erase_sector(TEST_SECTOR_ADDR);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    uint8_t erased_value = 0xFF;
    memset(test_read_data, 0, TEST_DATA_SIZE);
    e = ef_read(TEST_PAGE_ADDR, test_read_data, TEST_DATA_SIZE);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    for (int i = 0; i < TEST_DATA_SIZE; i++) {
        TEST_ASSERT_EQUAL(erased_value, test_read_data[i]);
    }
    e = ef_erase_sector(0xFFFFFF);
    TEST_ASSERT_TRUE(e == e_syserr_none || e == e_syserr_driver_fail);
}

void test_ef_rom(void){
    ext_flash_rom_t rom = {
        #ifdef FR2_SER_NUM
        .sn = FR2_SER_NUM,
        #else
        .sn = 9999,
        #endif
        #ifdef FR2_LAST_FLASH_TIME
        .last_flash = FR2_LAST_FLASH_TIME,
        #else
        .last_flash = 9999,
        #endif
        #ifdef FRX_FW_VER_MAJ
        .fw_ver_maj = FRX_FW_VER_MAJ,
        #else
        .fw_ver_maj = 9999,
        #endif 
        #ifdef FRX_FW_VER_MIN
        .fw_ver_min = FRX_FW_VER_MIN,
        #else
        .fw_ver_min = 9999,
        #endif 
        #ifdef FRX_FW_VER_MOD
        .fw_ver_mod = FRX_FW_VER_MOD
        #else
        .fw_ver_mod = 'a'
        #endif 
    };
    
    e_syserr_t e = ef_write_rom(&rom);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    ext_flash_rom_t rom_check;
    e = ef_read_rom(&rom_check);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(rom.sn, rom_check.sn);
    TEST_ASSERT_EQUAL(rom.last_flash, rom_check.last_flash);
    TEST_ASSERT_EQUAL(rom.fw_ver_maj, rom_check.fw_ver_maj);
    TEST_ASSERT_EQUAL(rom.fw_ver_min, rom_check.fw_ver_min);
    TEST_ASSERT_EQUAL(rom.fw_ver_mod, rom_check.fw_ver_mod);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_ef_init_deinit);
    RUN_TEST(test_ef_read_id);
    RUN_TEST(test_ef_basic_rw);
    RUN_TEST(test_ef_boundary_conditions);
    RUN_TEST(test_ef_erase_operations);
    RUN_TEST(test_ef_rom);
    UNITY_END();
}

void loop() {

}