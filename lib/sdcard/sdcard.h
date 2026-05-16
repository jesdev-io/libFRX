/// @file sdcard.h
/// @brief
/*
Combined abstraction for ESP32-SDMMC/SPI driver code and file-IO.
Provides functionalities to mount, unmount, create, delete,
write to, read from, append to *files* on the SD card.

Additionally, a jescore compatible SD card job can be invoked
by registering the function `sd_job` as a job.

Supports both SPI and SDMMC modes. Mode is selected via sd_mode_t parameter
in sd_init() or via SDCARD_MODE macro.

NOTE: Projects MUST define pin macros in platformio.ini (see PIN_DEFS.md)
*/
/// @author jake-is-ESD-protected. jesdev.io

#ifndef _SDCARD_H_
#define _SDCARD_H_

#ifdef FRX_ENABLE_MODULE_SDCARD

#include "esp_err.h"
#include "syserr.h"
#include "audio.h"
#include "sdmmc_cmd.h"

// Configuration macros - can be overridden by consuming projects
// See PIN_DEFS.md for complete list of required macros

#ifndef SDCARD_BASE_PATH
#define SDCARD_BASE_PATH            "/sdcard"
#endif

#ifndef SDCARD_PAGE_SIZE_BYTE
#define SDCARD_PAGE_SIZE_BYTE       512
#endif

#ifndef SDCARD_SERVER_JOB_NAME
#define SDCARD_SERVER_JOB_NAME      "sdcard"
#endif

#ifndef SDCARD_STREAMER_JOB_NAME
#define SDCARD_STREAMER_JOB_NAME    "sdstrm"
#endif

#ifndef SDCARD_MAX_FILES_DEFAULT
#define SDCARD_MAX_FILES_DEFAULT    5
#endif

#ifndef SDCARD_MAX_FREQ_BUS_DEFAULT
#define SDCARD_MAX_FREQ_BUS_DEFAULT SDMMC_FREQ_DEFAULT
#endif

// SD mode - REQUIRED: Projects MUST define in platformio.ini
// Options: sd_mode_spi, sd_mode_sdmmc, sd_mode_auto
#ifndef SDCARD_MODE
#error "SDCARD_MODE must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

// SPI pin configuration - REQUIRED when SDCARD_MODE=sd_mode_spi or sd_mode_auto
// Projects MUST define these in platformio.ini (see PIN_DEFS.md)
#if SDCARD_MODE == sd_mode_spi || SDCARD_MODE == sd_mode_auto
#ifndef SDCARD_SPI_PIN_CS
#error "SDCARD_SPI_PIN_CS must be defined in platformio.ini for SPI mode (see PIN_DEFS.md)"
#endif

#ifndef SDCARD_SPI_PIN_MOSI
#error "SDCARD_SPI_PIN_MOSI must be defined in platformio.ini for SPI mode (see PIN_DEFS.md)"
#endif

#ifndef SDCARD_SPI_PIN_MISO
#error "SDCARD_SPI_PIN_MISO must be defined in platformio.ini for SPI mode (see PIN_DEFS.md)"
#endif

#ifndef SDCARD_SPI_PIN_SCK
#error "SDCARD_SPI_PIN_SCK must be defined in platformio.ini for SPI mode (see PIN_DEFS.md)"
#endif
#endif // SDCARD_MODE == sd_mode_spi || SDCARD_MODE == sd_mode_auto

// SDMMC pin configuration - REQUIRED when SDCARD_MODE=sd_mode_sdmmc or sd_mode_auto
// Projects MUST define these in platformio.ini (see PIN_DEFS.md)
#if SDCARD_MODE == sd_mode_sdmmc || SDCARD_MODE == sd_mode_auto
#ifndef SDCARD_SDMMC_PIN_CLK
#error "SDCARD_SDMMC_PIN_CLK must be defined in platformio.ini for SDMMC mode (see PIN_DEFS.md)"
#endif

#ifndef SDCARD_SDMMC_PIN_CMD
#error "SDCARD_SDMMC_PIN_CMD must be defined in platformio.ini for SDMMC mode (see PIN_DEFS.md)"
#endif

#ifndef SDCARD_SDMMC_PIN_D0
#error "SDCARD_SDMMC_PIN_D0 must be defined in platformio.ini for SDMMC mode (see PIN_DEFS.md)"
#endif

#ifndef SDCARD_SDMMC_PIN_D1
#error "SDCARD_SDMMC_PIN_D1 must be defined in platformio.ini for SDMMC mode (see PIN_DEFS.md)"
#endif

#ifndef SDCARD_SDMMC_PIN_D2
#error "SDCARD_SDMMC_PIN_D2 must be defined in platformio.ini for SDMMC mode (see PIN_DEFS.md)"
#endif

#ifndef SDCARD_SDMMC_PIN_D3
#error "SDCARD_SDMMC_PIN_D3 must be defined in platformio.ini for SDMMC mode (see PIN_DEFS.md)"
#endif
#endif // SDCARD_MODE == sd_mode_sdmmc || SDCARD_MODE == sd_mode_auto
#endif

#ifndef SDCARD_LS_MAX_CHAR
#define SDCARD_LS_MAX_CHAR          256
#endif

#ifndef SDCARD_CAT_MAX_CHAR
#define SDCARD_CAT_MAX_CHAR         256
#endif

#ifndef SDCARD_PATH_MAX_CHAR
#define SDCARD_PATH_MAX_CHAR        64
#endif

#ifndef SDCARD_DEFAULT_FNAME_WAV
#define SDCARD_DEFAULT_FNAME_WAV    "fr1_rec_0000.wav"
#endif

#ifndef SDCARD_METADATA_FNAME
#define SDCARD_METADATA_FNAME       ".fr2_metadata.txt"
#endif

/// @brief SD card mode (SPI or SDMMC)
typedef enum {
    sd_mode_spi,
    sd_mode_sdmmc,
    sd_mode_auto  // Try SDMMC first, fall back to SPI
} sd_mode_t;

/// @deprecated
/// @enum SD card control commands.
typedef enum {
    sd_cmd_act_none,
    sd_cmd_mnt,
    sd_cmd_unmnt,
    sd_cmd_write_chunk,
    sd_cmd_read_chunk,
    NUM_SD_ACTIONS
} sd_cmd_t;

typedef enum{
    sd_stream_direction_in,
    sd_stream_direction_out
}sd_stream_direction_t;

typedef struct{
    FILE* f;
    audio_sample_t* data;
    uint32_t block_len;
    uint32_t type_in_byte;
    sd_stream_direction_t direction;
}sd_stream_descriptor_t;

/// @brief Initialize the SD card config struct.
/// @param max_files Max amount of files in FS.
/// @param max_freq_khz Max bus transfer speed. Pass 0 to use the default value.
/// @param mode SD card mode: sd_mode_spi or sd_mode_sdmmc.
/// @return Error code. Either returns `e_syserr_none` or `e_syserr_param`.
/// @note Does not perform calls to ESP-IDF functions, only sets a struct.
e_syserr_t sd_init(int32_t max_files, uint32_t max_freq_khz, sd_mode_t mode);

/// @brief Initialize the SD card with default values.
/// @return Error code.
/// @note Is part of the common signature interface for the init routine.
e_syserr_t sd_init_default(void);

/// @brief Mount the SD card.
/// @return Error code. 
/// @note Immediately returns with `e_syserr_none` if already mounted.
e_syserr_t sd_mnt(void);

/// @brief Unmount the SD card.
/// @return Error code.
/// @note Immediately returns with `e_syserr_none` if already unmounted.
e_syserr_t sd_unmnt(void);

/// @brief Checks the mounting state of the SD card.
/// @return Mounting state expressed as 0 (umounted) and 1 (mounted).
uint8_t sd_is_mounted(void);

/// @brief Get the number of free and total kilobytes in the FS.
/// @param free_kbytes Pointer to variable to hold the free kbyte number.
/// @param all_kbytes Pointer to variable to hold the total kbyte number.
/// @return Error code.
e_syserr_t sd_get_free_kbytes(uint32_t* free_kbytes, uint32_t* all_kbytes);

/// @brief Create a file on the FS.
/// @param path Absolute path to a file.
/// @return Error code.
/// @note The absolute path has to be given with `SDCARD_BASE_PATH` as base.
e_syserr_t sd_create_file(const char* path);

/// @brief Delete file on the FS.
/// @param path Absolute path to a file.
/// @return Error code. Only returns `e_syserr_file_generic` in case of file-IO errors.
e_syserr_t sd_delete_file(const char* path);

/// @brief Write content to a file.
/// @param data Pointer to content to be written.
/// @param type_size Size of data type in byte.
/// @param len Length of data (not in byte).
/// @param fname Name of the file. File is created if it does not exist.
/// @param mode IO mode. Text and binary forms supported (w, wb)
/// @param pos Position to start writing from. Uses `fseek`. This parameter is ignored when using mode "a"/"ab".
/// @param  points_w Amount of data points that were actually written to the file.
/// @return Error code.
e_syserr_t sd_write(void* data, uint16_t type_size, uint32_t len, const char* fname, const char* mode, uint32_t pos, uint32_t*  points_w);

/// @brief Write text (char*) to a file.
/// @param data Pointer to c-string.
/// @param len Length of string. This must include the trailing "\0"!
/// @param fname Name of the file. File is created if it does not exist.
/// @param pos IO mode. Text forms supported (r, w, a).
/// @param  points_w Amount of chars that were actually written to the file. For text, this will usually be len-1, because the trailing "\0" is not written.
/// @return Error code.
e_syserr_t sd_write_txt(char* data, uint32_t len, const char* fname, uint32_t pos, uint32_t*  points_w);

/// @brief Append content to a file.
/// @param data Pointer to content to be appended.
/// @param type_size Size of data type in byte.
/// @param len Length of data (not in byte).
/// @param fname Name of the file. Has to exist.
/// @param  points_w Amount of bytes that were actually written to the file.
/// @return Error code.
e_syserr_t sd_append(void* data, uint16_t type_size, uint32_t len, const char* fname, uint32_t*  points_w);

/// @brief Append text to a file.
/// @param data Pointer to c-string to be appended.
/// @param len Length of string. This must include the trailing "\0"!
/// @param fname Name of the file. Has to exist.
/// @param  points_w Amount of chars that were actually written to the file. For text, this will usually be len-1, because the trailing "\0" is not written.
/// @return Error code.
e_syserr_t sd_append_txt(void* data, uint32_t len, const char* fname, uint32_t*  points_w);

/// @brief Read content from a file.
/// @param data Pointer to empty data array.
/// @param type_size Size of data type in byte.
/// @param len Length of data (not in byte).
/// @param fname Name of the file. Has to exist.
/// @param mode IO mode. Text and binary forms supported (r, rb)
/// @param pos Position to start reading from. Uses `fseek`.
/// @param  points_r Amount of data points that were actually read from the file.
/// @return Error code.
e_syserr_t sd_read(void* data, uint16_t type_size, uint32_t len, const char* fname, const char* mode, uint32_t pos, uint32_t*  points_r);

/// @brief Read text from a file.
/// @param data Pointer to empty char array.
/// @param len Length of char array including the trailing "\0".
/// @param fname Name of the file. Has to exist.
/// @param pos Position to start reading from. Uses `fseek`.
/// @param  points_r Amount of chars that were actually read from the file.
/// @return Error code.
e_syserr_t sd_read_txt(char* data, uint32_t len, const char* fname, uint32_t pos, uint32_t*  points_r);

/// @brief Write to SD with an open file pointer supplied from outside.
/// @param data Pointer to empty audio data array.
/// @param len Length of data (not in byte).
/// @param bps Bits per single channel sample (resolution).
/// @param nch Amount of active channels.
/// @param f Already opened file pointer.
/// @param points_w Amount of data points that were actually written to the file.
/// @return Error code.
/// @note The file pointer **needs** to be opened in 'ab' mode, otherwise data will be destroyed.
/// Additionally, should an error occur, the function **does not** close the file. Close it from outside!
e_syserr_t sd_stream_in(audio_sample_t* data, uint32_t len, uint8_t bps, uint8_t nch, FILE* f, uint32_t* points_w);

/// @brief Read from SD with an open file pointer supplied from outside.
/// @param data Pointer to audio data array.
/// @param len Length of data (not in byte).
/// @param bps Bits per single channel sample (resolution).
/// @param nch Amount of active channels.
/// @param f Already opened file pointer.
/// @param points_r Amount of data points that were actually read from the file.
/// @return Error code.
e_syserr_t sd_stream_out(audio_sample_t* data, uint32_t len, uint8_t bps, uint8_t nch, FILE* f, uint32_t* points_r);

/// @brief Open file stream indefinitely for out-of-scope operations.
/// @param fname Name of the file.
/// @param mode IO mode. Use either "ab" or "rb" for write or read streams
/// @return FILE pointer to opened file.
/// @note Needs to be closed in separate call with `sd_stream_close()`.
FILE* sd_stream_open(const char* fname, const char* mode);

/// @brief Open file stream indefinitely for out-of-scope reading.
/// @param fname Name of the file. Has to exist.
/// @return FILE pointer to opened file.
/// @note Needs to be closed in separate call with `sd_stream_close()`.
FILE* sd_stream_read_open(const char* fname);

/// @brief Open file stream indefinitely for out-of-scope writing.
/// @param fname Name of the file. Has to exist.
/// @return FILE pointer to opened file.
/// @note Needs to be closed in separate call with `sd_stream_close()`.
FILE* sd_stream_write_open(const char* fname);

/// @brief Close an opened file stream.
/// @param f FILE pointer to opened file.
void sd_stream_close(FILE* f);

/// @brief List all files of a folder.
/// @param dirname Path to directory which contains entries to be listed.
/// @param pret Pointer to empty char array.
/// @param len Length of char array including the trailing "\0".
/// @return Error code.
/// @note Single entries are delimited with a newlines.
e_syserr_t sd_ls(const char *dirname, char* pret, uint16_t len);

/// @brief List content of a file.
/// @param fname Name of the file. Has to exist.
/// @param pret Pointer to empty char array.
/// @param len Length of char array including the trailing "\0".
/// @return Error code.
/// @note If the buffer is not large enough for all of the content in the file, only the initial values in the buffer will be shown.
e_syserr_t sd_cat(const char *fname, char* pret, uint16_t len);

/// @brief Remove a file.
/// @param fname Path to file.
/// @return Error code.
e_syserr_t sd_rm(const char *fname);

/// @brief Check whether a file exists or not.
/// @param fname Path to file.
/// @return 0 if not existing, 1 if existing.
/// @note Also returns 0 if the SD card is not mounted!
uint8_t sd_file_exists(const char *fname);

/// @brief Get a unique filename that does not yet exist in the FS.
/// @param proposed Proposed name of form "fr1_rec_xxxx.wav". Will be written to.
/// @return Error code.
e_syserr_t sd_get_unique_fname(char* proposed);

/// @brief jescore CLI handler for the "sdcard" subcommand
/// @param p jescore job struct, set from outside.
/// @note This function only performs CLI responses and should not be called by user code.
void sd_job(void* p);

#endif // FRX_ENABLE_MODULE_SDCARD
#endif // _SDCARD_H_
