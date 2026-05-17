#ifndef _EXT_FLASH_H_
#define _EXT_FLASH_H_

#ifdef FRX_ENABLE_MODULE_EXT_FLASH

#include <stdint.h>
#include "syserr.h"

// REQUIRED: Projects MUST define in platformio.ini (see PIN_DEFS.md)
// No defaults - each project has different hardware
#ifndef EF_SPI_HOST
#error "EF_SPI_HOST must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

#ifndef EF_PIN_MISO
#error "EF_PIN_MISO must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

#ifndef EF_PIN_MOSI
#error "EF_PIN_MOSI must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

#ifndef EF_PIN_SCLK
#error "EF_PIN_SCLK must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

#ifndef EF_PIN_CS
#error "EF_PIN_CS must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

// W25Qxx Commands
#define EF_CMD_WRITE_ENABLE         0x06
#define EF_CMD_PAGE_PROGRAM         0x02
#define EF_CMD_READ_DATA            0x03
#define EF_CMD_READ_STATUS_REG_1    0x05
#define EF_CMD_SECTOR_ERASE         0x20
#define EF_CMD_READ_JEDEC_ID        0x9F

#ifndef EF_CLK_SPEED_HZ
#define EF_CLK_SPEED_HZ             (10 * 1000 * 1000)  // 10 MHz
#endif

// Default addresses for ROM and settings storage
// Projects can override these if they use different addresses
#ifndef EF_ROM_ADDR
#define EF_ROM_ADDR                 0x1000
#endif

#ifndef EF_SETT_ADDR
#define EF_SETT_ADDR                0x2000
#endif

#ifndef EF_SERVER_JOB_NAME
#define EF_SERVER_JOB_NAME          "ef"
#endif

#ifndef EF_SERVER_JOB_MEM
#define EF_SERVER_JOB_MEM           2048
#endif

// Generic types for ROM and settings data
// Projects should define their own types and use ef_read/write_data for custom data
typedef struct {
    uint32_t sn;
    uint32_t last_flash;
    uint32_t fw_ver_maj;
    uint32_t fw_ver_min;
    char fw_ver_mod;
} ext_flash_rom_t;

typedef struct {
    char level_filter;
} ext_flash_settings_t;

/// @brief Initialize the external flash W25Qxx.
/// @return Error code.
e_syserr_t ef_init(void);

/// @brief Initialize the external flash W25Qxx with default values.
/// @return Error code.
/// @note Is part of the common signature interface for the init routine.
e_syserr_t ef_init_default(void);

/// @brief Deinitialize the external flash W25Qxx.
/// @return Error code.
e_syserr_t ef_deinit(void);

/// @brief Read the W25Qxx's ID.
/// @param pid Address to storage variable.
/// @return Error code.
e_syserr_t ef_read_id(uint32_t* pid);

/// @brief Read data from the W25Qxx.
/// @param addr Address to begin reading from.
/// @param data Pointer to storage array.
/// @param len Length of data in bytes.
/// @return Error code.
e_syserr_t ef_read(uint32_t addr, uint8_t *data, uint32_t len);

/// @brief Write data to the W25Qxx (page program, up to 256 bytes).
/// @param addr Address to write to.
/// @param data Pointer to source array.
/// @param len Length of data in bytes (max 256).
/// @return Error code.
e_syserr_t ef_write_page(uint32_t addr, uint8_t *data, uint32_t len);

/// @brief Erase a sector on the W25Qxx.
/// @param addr Address of sector to erase.
/// @return Error code.
e_syserr_t ef_erase_sector(uint32_t addr);

/// @brief Read generic ROM data from the W25Qxx.
/// @param rom ROM data storage.
/// @return Error code.
e_syserr_t ef_read_rom(ext_flash_rom_t* rom);

/// @brief Write generic ROM data to the W25Qxx.
/// @param rom ROM data to write.
/// @return Error code.
/// @note Only use this if you know what you are doing! This violates the "RO" in "ROM"!
e_syserr_t ef_write_rom(ext_flash_rom_t* rom);

/// @brief Read settings from the W25Qxx.
/// @param settings Settings storage.
/// @return Error code.
e_syserr_t ef_read_settings(ext_flash_settings_t* settings);

/// @brief Write settings to the W25Qxx.
/// @param settings Settings to write.
/// @return Error code.
e_syserr_t ef_write_settings(ext_flash_settings_t* settings);

/// @brief External flash diagnostic job.
/// @note Available arguments:
///       - `rom`: Display the PID and ROM data.
void ef_job(void* p);

#endif // FRX_ENABLE_MODULE_EXT_FLASH
#endif
