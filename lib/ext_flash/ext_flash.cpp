#ifdef FRX_ENABLE_MODULE_EXT_FLASH

#include "ext_flash.h"
#include <SPI.h>
#include <Arduino.h>
#include "jescore.h"

static bool initialized = false;

e_syserr_t ef_init(void) {
    if (initialized) {
        return e_syserr_none;
    }

    pinMode(EF_PIN_CS, OUTPUT);
    digitalWrite(EF_PIN_CS, HIGH);
    
    SPI.begin(EF_PIN_SCLK, EF_PIN_MISO, EF_PIN_MOSI, EF_PIN_CS);
    
    initialized = true;
    jes_err_t je;
    je = jes_register_job(EF_SERVER_JOB_NAME, EF_SERVER_JOB_MEM, 1, ef_job, 0, 1);
    if(je != e_err_no_err && je != e_err_duplicate) { 
        jes_throw_error(je); 
        return (e_syserr_t)je;
    }
    return e_syserr_none;
}

e_syserr_t ef_deinit(void) {
    if (!initialized) {
        return e_syserr_uninitialized;
    }
    
    SPI.end();
    pinMode(EF_PIN_CS, INPUT);
    
    initialized = false;
    return e_syserr_none;
}

e_syserr_t ef_init_default(void){
    return ef_init();
}

/// @brief Begin a SPI transaction for the W25Qxx.
static void __flash_begin() {
    digitalWrite(EF_PIN_CS, LOW);
    SPI.beginTransaction(SPISettings(EF_CLK_SPEED_HZ, MSBFIRST, SPI_MODE0));
}

/// @brief End a SPI transaction for the W25Qxx.
static void __flash_end() {
    SPI.endTransaction();
    digitalWrite(EF_PIN_CS, HIGH);
}

/// @brief Enable the W25Qxx for write operations.
/// @return Error code.
static e_syserr_t __write_enable() {
    __flash_begin();
    SPI.transfer(EF_CMD_WRITE_ENABLE);
    __flash_end();
    return e_syserr_none;
}

/// @brief Poll for the next open transaction.
/// @return Error code.
static e_syserr_t __wait_ready() {
    uint8_t status;
    do {
        __flash_begin();
        SPI.transfer(EF_CMD_READ_STATUS_REG_1);
        status = SPI.transfer(0x00);
        __flash_end();
    } while (status & 0x01);
    return e_syserr_none;
}

e_syserr_t ef_read_id(uint32_t* pid) {
    if (!initialized) return e_syserr_uninitialized;
    if (pid == NULL) return e_syserr_null;

    __flash_begin();
    SPI.transfer(EF_CMD_READ_JEDEC_ID);
    *pid = SPI.transfer(0x00) << 16;
    *pid |= SPI.transfer(0x00) << 8;
    *pid |= SPI.transfer(0x00);
    __flash_end();
    
    return e_syserr_none;
}

e_syserr_t ef_read(uint32_t addr, uint8_t *data, uint32_t len) {
    if (!initialized) return e_syserr_uninitialized;
    if (data == NULL) return e_syserr_null;
    if (len == 0) return e_syserr_param;

    __flash_begin();
    SPI.transfer(EF_CMD_READ_DATA);
    SPI.transfer(addr >> 16);
    SPI.transfer(addr >> 8);
    SPI.transfer(addr);
    
    for (uint32_t i = 0; i < len; i++) {
        data[i] = SPI.transfer(0x00);
    }
    __flash_end();
    
    return e_syserr_none;
}

e_syserr_t ef_write_page(uint32_t addr, uint8_t *data, uint32_t len) {
    if (!initialized) return e_syserr_uninitialized;
    if (data == NULL) return e_syserr_null;
    if (len == 0) return e_syserr_param;
    if (len > 256) return e_syserr_too_long;

    __write_enable();
    
    __flash_begin();
    SPI.transfer(EF_CMD_PAGE_PROGRAM);
    SPI.transfer(addr >> 16);
    SPI.transfer(addr >> 8);
    SPI.transfer(addr);
    
    for (uint32_t i = 0; i < len; i++) {
        SPI.transfer(data[i]);
    }
    __flash_end();
    
    return __wait_ready();
}

e_syserr_t ef_erase_sector(uint32_t addr) {
    if (!initialized) return e_syserr_uninitialized;

    __write_enable();
    
    __flash_begin();
    SPI.transfer(EF_CMD_SECTOR_ERASE);
    SPI.transfer(addr >> 16);
    SPI.transfer(addr >> 8);
    SPI.transfer(addr);
    __flash_end();
    
    return __wait_ready();
}

e_syserr_t ef_read_rom(ext_flash_rom_t* rom){
    return ef_read(EF_ROM_ADDR, (uint8_t*)rom, sizeof(ext_flash_rom_t));
}

e_syserr_t ef_write_rom(ext_flash_rom_t* rom){
    return ef_write_page(EF_ROM_ADDR, (uint8_t*)rom, sizeof(ext_flash_rom_t));
}

e_syserr_t ef_read_settings(ext_flash_settings_t* settings){
    return ef_read(EF_SETT_ADDR, (uint8_t*)settings, sizeof(ext_flash_settings_t));
}

e_syserr_t ef_write_settings(ext_flash_settings_t* settings){
    return ef_write_page(EF_SETT_ADDR, (uint8_t*)settings, sizeof(ext_flash_settings_t));
}

void ef_job(void* p){
    char* args = jes_job_get_args();
    char* arg = strtok(args, " ");
    

    if(arg == NULL){
        jes_print("No command specified for ef job.\n\r");
        return;
    }
    

    if(strcmp(arg, "rom") == 0){
        e_syserr_t e;
        ext_flash_rom_t rom;
        memset(&rom, 0, sizeof(ext_flash_rom_t));
        uint32_t pid = 0;

        e = ef_read_rom(&rom);
        if(e != e_syserr_none){
            jes_print("Error while reading external flash ROM!\n\r");
            return;
        }
        e = ef_read_id(&pid);
        if(e != e_syserr_none){
            jes_print("Error while reading external flash PID!\n\r");
            return;
        }
        jes_print("PID: 0x%X\n\r", pid);
        jes_print("SN: %d\n\r", rom.sn);
        jes_print("FW: %d.%d%c\n\r", rom.fw_ver_maj, rom.fw_ver_min, rom.fw_ver_mod);
        jes_print("UT: %d\n\r", rom.last_flash);
    }
}

#endif // FRX_ENABLE_MODULE_EXT_FLASH
