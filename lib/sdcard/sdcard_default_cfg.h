#ifndef _SDCARD_DEFAULT_CFG_H_
#define _SDCARD_DEFAULT_CFG_H_

#ifdef FRX_ENABLE_MODULE_SDCARD

#if !defined(SDCARD_MODE_SPI) && !defined(SDCARD_MODE_SDMMC)
#error "SDCARD_MODE must either be sd_mode_sdmmc or sd_mode_spi!"
#endif

// SPI pin configuration - REQUIRED when SDCARD_MODE_SPI is defined.
// Projects MUST define these in platformio.ini
#ifdef SDCARD_MODE_SPI
#ifndef SDCARD_SPI_PIN_CS
#error "SDCARD_SPI_PIN_CS must be defined in platformio.ini for SPI mode"
#endif

#ifndef SDCARD_SPI_PIN_MOSI
#error "SDCARD_SPI_PIN_MOSI must be defined in platformio.ini for SPI mode"
#endif

#ifndef SDCARD_SPI_PIN_MISO
#error "SDCARD_SPI_PIN_MISO must be defined in platformio.ini for SPI mode"
#endif

#ifndef SDCARD_SPI_PIN_SCK
#error "SDCARD_SPI_PIN_SCK must be defined in platformio.ini for SPI mode"
#endif
#endif // SDCARD_MODE_SPI

// SDMMC pin configuration - REQUIRED when SDCARD_MODE_SDMMC is defined.
// Projects MUST define these in platformio.ini
#ifdef SDCARD_MODE_SDMMC
#ifndef SDCARD_SDMMC_PIN_CLK
#error "SDCARD_SDMMC_PIN_CLK must be defined in platformio.ini for SDMMC mode"
#endif

#ifndef SDCARD_SDMMC_PIN_CMD
#error "SDCARD_SDMMC_PIN_CMD must be defined in platformio.ini for SDMMC mode"
#endif

#ifndef SDCARD_SDMMC_PIN_D0
#error "SDCARD_SDMMC_PIN_D0 must be defined in platformio.ini for SDMMC mode"
#endif

#ifndef SDCARD_SDMMC_PIN_D1
#error "SDCARD_SDMMC_PIN_D1 must be defined in platformio.ini for SDMMC mode"
#endif

#ifndef SDCARD_SDMMC_PIN_D2
#error "SDCARD_SDMMC_PIN_D2 must be defined in platformio.ini for SDMMC mode"
#endif

#ifndef SDCARD_SDMMC_PIN_D3
#error "SDCARD_SDMMC_PIN_D3 must be defined in platformio.ini for SDMMC mode"
#endif
#ifndef SDCARD_SDMMC_WIDTH
#define SDCARD_SDMMC_WIDTH 4
#endif

#if SDCARD_SDMMC_WIDTH != 1 && SDCARD_SDMMC_WIDTH != 4
#error "SDCARD_SDMMC_WIDTH must be 1 or 4"
#endif

#ifndef SDCARD_SDMMC_INTERNAL_PULLUP
#define SDCARD_SDMMC_INTERNAL_PULLUP 0
#endif
#endif // SDCARD_MODE_SDMMC

#ifndef SDCARD_BASE_PATH
#define SDCARD_BASE_PATH            "/sdcard"
#endif

#ifndef SDCARD_PAGE_SIZE_BYTE
#define SDCARD_PAGE_SIZE_BYTE       512
#endif

#ifndef SDCARD_MAX_FILES_DEFAULT
#define SDCARD_MAX_FILES_DEFAULT    5
#endif

#ifndef SDCARD_MAX_FREQ_BUS_DEFAULT
#define SDCARD_MAX_FREQ_BUS_DEFAULT SDMMC_FREQ_DEFAULT
#endif

#ifndef SDCARD_SD_STREAM_POOL_SIZE
#define SDCARD_SD_STREAM_POOL_SIZE  4096
#endif

#ifndef SDCARD_LS_MAX_CHAR
#define SDCARD_LS_MAX_CHAR          256
#endif

#ifndef SDCARD_LS_MIN_CHAR
#define SDCARD_LS_MIN_CHAR          4
#endif

#ifndef SDCARD_LS_MAX_ENTRIES
#define SDCARD_LS_MAX_ENTRIES       8
#endif

#ifndef SDCARD_LS_ENTRY_MAX_LEN
#define SDCARD_LS_ENTRY_MAX_LEN     32
#endif

#ifndef SDCARD_LS_ENTRY_SEPARATOR
#define SDCARD_LS_ENTRY_SEPARATOR   "\n"
#endif

#ifndef SDCARD_CAT_MAX_CHAR
#define SDCARD_CAT_MAX_CHAR         256
#endif

#ifndef SDCARD_PATH_MAX_CHAR
#define SDCARD_PATH_MAX_CHAR        64
#endif

#ifndef SDCARD_JOB_TIMEOUT          
#define SDCARD_JOB_TIMEOUT          1000
#endif

#endif // FRX_ENABLE_MODULE_SDCARD

#endif // _SDCARD_DEFAULT_CFG_H_