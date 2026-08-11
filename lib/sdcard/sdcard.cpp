#ifdef FRX_ENABLE_MODULE_SDCARD

#include <jescore.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_types.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "driver/sdspi_host.h"
#include "driver/gpio.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sdcard.h"
#include "sdcard_jccl.h"
#include "utils.h"
#include "libfrx_sys.h"

/// @brief Internal data streaming server.
/// @param p jescore job pointer.
static inline void __sd_transfer(void* p);

// Common state
static esp_vfs_fat_sdmmc_mount_config_t mount_config;
static sdmmc_card_t* card = NULL;
static uint8_t mounted = 0;
static SemaphoreHandle_t stream_lock = NULL;
static SemaphoreHandle_t stream_done = NULL;
static sd_stream_descriptor_t active_stream = {0};
static volatile uint8_t stream_busy = 0;
static e_syserr_t stream_last_error = e_syserr_none;
static uint32_t stream_last_points = 0;

// SPI-specific state
#ifdef SDCARD_MODE_SPI
static sdmmc_host_t spi_host = SDSPI_HOST_DEFAULT();
static sdspi_device_config_t spi_slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
static spi_bus_config_t spi_bus_cfg;
#endif // SDCARD_MODE_SPI

// SDMMC-specific state
#ifdef SDCARD_MODE_SDMMC
static sdmmc_host_t sdmmc_host = SDMMC_HOST_DEFAULT();
static sdmmc_slot_config_t sdmmc_slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
#endif // SDCARD_MODE_SDMMC

/// @brief Initialize SDMMC-specific configuration
static void __sdcard_config(int32_t max_files, uint32_t max_freq_khz) {
    #ifdef SDCARD_MODE_SDMMC
    sdmmc_host.flags = SDMMC_HOST_FLAG_DEINIT_ARG;
    #if SDCARD_SDMMC_WIDTH == 4
    sdmmc_host.flags |= SDMMC_HOST_FLAG_4BIT;
    #endif
    if (max_freq_khz > SDMMC_FREQ_HIGHSPEED) return;
    if (max_freq_khz != 0) {
        sdmmc_host.max_freq_khz = max_freq_khz;
    } else {
        sdmmc_host.max_freq_khz = SDCARD_MAX_FREQ_BUS_DEFAULT;
    }
    sdmmc_slot_config.clk = (gpio_num_t)SDCARD_SDMMC_PIN_CLK;
    sdmmc_slot_config.cmd = (gpio_num_t)SDCARD_SDMMC_PIN_CMD;
    sdmmc_slot_config.d0 = (gpio_num_t)SDCARD_SDMMC_PIN_D0;
    sdmmc_slot_config.d1 = (gpio_num_t)SDCARD_SDMMC_PIN_D1;
    sdmmc_slot_config.d2 = (gpio_num_t)SDCARD_SDMMC_PIN_D2;
    sdmmc_slot_config.d3 = (gpio_num_t)SDCARD_SDMMC_PIN_D3;
    sdmmc_slot_config.width = SDCARD_SDMMC_WIDTH;
    #if SDCARD_SDMMC_INTERNAL_PULLUP
    sdmmc_slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    #endif

    mount_config.format_if_mount_failed = false;
    mount_config.max_files = max_files;
    mount_config.allocation_unit_size = 16 * 1024;
    #endif // SDCARD_MODE_SDMMC

    #ifdef SDCARD_MODE_SPI
    if (max_freq_khz > SDMMC_FREQ_52M) return;
    if (max_freq_khz != 0) {
        spi_host.max_freq_khz = max_freq_khz;
    } else {
        spi_host.max_freq_khz = SDCARD_MAX_FREQ_BUS_DEFAULT;
    }

    spi_bus_cfg.mosi_io_num = SDCARD_SPI_PIN_MOSI;
    spi_bus_cfg.miso_io_num = SDCARD_SPI_PIN_MISO;
    spi_bus_cfg.sclk_io_num = SDCARD_SPI_PIN_SCK;
    spi_bus_cfg.quadwp_io_num = -1;
    spi_bus_cfg.quadhd_io_num = -1;
    spi_bus_cfg.max_transfer_sz = 16384;

    spi_slot_config.gpio_cs = (gpio_num_t)SDCARD_SPI_PIN_CS;
    spi_slot_config.host_id = (spi_host_device_t)spi_host.slot;

    mount_config.format_if_mount_failed = false;
    mount_config.max_files = max_files;
    mount_config.allocation_unit_size = 16 * 1024;
    #endif // SDCARD_MODE_SPI
}

e_syserr_t sd_init(int32_t max_files, uint32_t max_freq_khz) {
    __sdcard_config(max_files, max_freq_khz);
    
    #ifdef SDCARD_MODE_SPI
    // Initialize SPI bus (must be done after config but before mount)
    esp_err_t spi_ret = spi_bus_initialize((spi_host_device_t)spi_host.slot, &spi_bus_cfg, SPI_DMA_CH_AUTO);
    if (spi_ret != ESP_OK) {
        return e_syserr_driver_fail;
    }
    #endif // SDCARD_MODE_SPI
    
    stream_lock = xSemaphoreCreateMutex();
    if (stream_lock == NULL) return e_syserr_null;
    stream_done = xSemaphoreCreateBinary();
    if (stream_done == NULL) return e_syserr_null;
    jes_err_t je;
    je = jes_register_job(SDCARD_JOB_NAME, 2*4096, 1, sd_job, 0, 1);
    if (je != e_err_no_err) { 
        jes_throw_error(je); 
        return (e_syserr_t)je; 
    }
    je = jes_register_and_launch_job(SDCARD_STREAMER_JOB_NAME, 4*4096, 1, __sd_transfer, 1, 1);
    if (je != e_err_no_err) { 
        jes_throw_error(je); 
        return (e_syserr_t)je; 
    }
    return e_syserr_none;
}

e_syserr_t sd_init_default(void) {
    return sd_init(SDCARD_MAX_FILES_DEFAULT, SDCARD_MAX_FREQ_BUS_DEFAULT);
}

e_syserr_t sd_mnt(void) {
    if (mounted) return e_syserr_none;
    esp_err_t ret;

    #ifdef SDCARD_MODE_SDMMC
    ret = esp_vfs_fat_sdmmc_mount(
        SDCARD_BASE_PATH,
        &sdmmc_host,
        &sdmmc_slot_config,
        &mount_config,
        &card
    );
    #endif // SDCARD_MODE_SDMMC
    #ifdef SDCARD_MODE_SPI
    ret = esp_vfs_fat_sdspi_mount(
        SDCARD_BASE_PATH,
        &spi_host,
        &spi_slot_config,
        &mount_config,
        &card
    );
    #endif // SDCARD_MODE_SPI
    if (ret != ESP_OK) return e_syserr_driver_fail;
    if (card == NULL) return e_syserr_driver_fail;
    mounted = 1;
    return e_syserr_none;
}

e_syserr_t sd_unmnt(void) {
    if (!mounted) return e_syserr_none;
    esp_err_t stat = esp_vfs_fat_sdcard_unmount(SDCARD_BASE_PATH, card);
    if (stat != ESP_OK) return e_syserr_driver_fail;
    mounted = 0;
    card = NULL;
    return e_syserr_none;
}

uint8_t sd_is_mounted(void) {
    return mounted;
}

e_syserr_t sd_get_free_kbytes(uint32_t* free_kbytes, uint32_t* all_kbytes) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    FATFS *fs;
    DWORD fre_clust;

    FRESULT res = f_getfree("0:", &fre_clust, &fs);
    if (res != FR_OK) return e_syserr_driver_fail;

    // Use 512 for filesystem sector size (FatFs always uses 512-byte sectors for SD cards)
    // Do NOT use card->csd.sector_size which may report physical sector size differently
    uint64_t bytes_per_cluster = (uint64_t)fs->csize * 512;
    uint64_t free_bytes_64 = (uint64_t)fre_clust * bytes_per_cluster;
    uint64_t all_bytes_64 = (uint64_t)(fs->n_fatent - 2) * bytes_per_cluster;
    *free_kbytes = (uint32_t)(free_bytes_64 / 1000);
    *all_kbytes = (uint32_t)(all_bytes_64 / 1000);
    return e_syserr_none;
}

e_syserr_t sd_create_file(const char* path) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    FILE* file = fopen(path, "w");
    if (file == NULL) return e_syserr_file_generic;
    if (fclose(file) != 0) return e_syserr_file_generic;
    return e_syserr_none;
}

e_syserr_t sd_delete_file(const char* path) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    if (remove(path) != 0) return e_syserr_file_generic;
    return e_syserr_none;
}

e_syserr_t sd_write(void* data, uint16_t type_size, uint32_t len, const char* fname, const char* mode, uint32_t pos, uint32_t* points_w) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    FILE* f = fopen(fname, mode);
    if (f == NULL) return e_syserr_file_generic;
    if (pos > 0) {
        if (fseek(f, pos, SEEK_SET) != 0) {
            fclose(f);
            return e_syserr_file_generic;
        }
    }
    *points_w = fwrite(data, type_size, len, f);
    if (*points_w != len) { 
        fclose(f);
        return e_syserr_oom; 
    }
    if (fclose(f) != 0) return e_syserr_file_generic;
    return e_syserr_none;
}

e_syserr_t sd_write_txt(char* data, uint32_t len, const char* fname, uint32_t pos, uint32_t* points_w) {
    return sd_write((void*)data, sizeof(char), len-1, fname, "w", pos, points_w);
}

e_syserr_t sd_append(void* data, uint16_t type_size, uint32_t len, const char* fname, uint32_t* points_w) {
    return sd_write(data, type_size, len, fname, "ab", 0, points_w);
}

e_syserr_t sd_append_txt(void* data, uint32_t len, const char* fname, uint32_t* points_w) {
    return sd_write(data, 1, len-1, fname, "a", 0, points_w);
}

e_syserr_t sd_read(void* data, uint16_t type_size, uint32_t len, const char* fname, const char* mode, uint32_t pos, uint32_t* points_r) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    FILE* f = fopen(fname, mode);
    if (f == NULL) return e_syserr_file_generic;
    if (pos > 0) {
        if (fseek(f, pos, SEEK_SET) != 0) {
            fclose(f);
            return e_syserr_file_generic;
        }
    }
    *points_r = fread(data, type_size, len, f);
    if (*points_r != len) { 
        fclose(f);
        return e_syserr_file_eof; 
    }
    if (fclose(f) != 0) return e_syserr_file_generic;
    return e_syserr_none;
}

e_syserr_t sd_read_txt(char* data, uint32_t len, const char* fname, uint32_t pos, uint32_t* points_r) {
    e_syserr_t e = sd_read(data, sizeof(char), len-1, fname, "r", pos, points_r);
    data[len-1] = '\0';
    return e;
}

static e_syserr_t __sd_stream_wait(TickType_t timeout) {
    if (!stream_busy) return stream_last_error;
    if (xSemaphoreTake(stream_done, timeout) != pdTRUE) return e_syserr_locked;
    return stream_last_error;
}

static e_syserr_t __sd_stream_submit(void* data, uint32_t len, uint8_t bps, uint8_t nch, FILE* f, sd_stream_direction_t direction, uint32_t* points) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    if (f == NULL) return e_syserr_file_generic;
    if (data == NULL) return e_syserr_null;
    if (points == NULL) return e_syserr_null;
    uint32_t type_in_byte = (bps / 8) * nch;
    if (type_in_byte == 0 || len == 0) return e_syserr_param;
    if (type_in_byte * len > SDCARD_SD_STREAM_POOL_SIZE) return e_syserr_too_long;

    xSemaphoreTake(stream_lock, portMAX_DELAY);
    if (stream_busy) {
        xSemaphoreGive(stream_lock);
        return e_syserr_locked;
    }
    while (xSemaphoreTake(stream_done, 0) == pdTRUE) {}
    *points = 0;
    stream_last_points = 0;
    stream_last_error = e_syserr_none;
    active_stream.f = f;
    active_stream.data = data;
    active_stream.block_len = len;
    active_stream.type_in_byte = type_in_byte;
    active_stream.direction = direction;
    stream_busy = 1;
    jes_err_t je = jes_notify_job(SDCARD_STREAMER_JOB_NAME, &active_stream);
    if (je != e_err_no_err) {
        stream_busy = 0;
        stream_last_error = (e_syserr_t)je;
        xSemaphoreGive(stream_lock);
        return stream_last_error;
    }
    xSemaphoreGive(stream_lock);

    e_syserr_t e = __sd_stream_wait(portMAX_DELAY);
    *points = stream_last_points;
    return e;
}

e_syserr_t sd_stream_in(void* data, uint32_t len, uint8_t bps, uint8_t nch, FILE* f, uint32_t* points_w) {
    return __sd_stream_submit(data, len, bps, nch, f, sd_stream_direction_in, points_w);
}

e_syserr_t sd_stream_out(void* data, uint32_t len, uint8_t bps, uint8_t nch, FILE* f, uint32_t* points_r) {
    return __sd_stream_submit(data, len, bps, nch, f, sd_stream_direction_out, points_r);
}

FILE* sd_stream_open(const char* fname, const char* mode) {
    if (!mounted) return NULL;
    FILE* f = fopen(fname, mode);
    return f;
}

FILE* sd_stream_read_open(const char* fname) {
    if (!mounted) return NULL;
    FILE* f = fopen(fname, "rb");
    return f;
}

FILE* sd_stream_write_open(const char* fname) {
    if (!mounted) return NULL;
    FILE* f = fopen(fname, "ab");
    return f;
}

e_syserr_t sd_stream_close(FILE* f) {
    if (f == NULL) return e_syserr_file_generic;
    e_syserr_t stream_error = __sd_stream_wait(portMAX_DELAY);
    xSemaphoreTake(stream_lock, portMAX_DELAY);
    int close_ret = fclose(f);
    xSemaphoreGive(stream_lock);
    if (stream_error != e_syserr_none) return stream_error;
    if (close_ret != 0) return e_syserr_file_generic;
    return e_syserr_none;
}

e_syserr_t sd_ls(const char *dirname, char* pret, uint16_t n_entries, uint16_t len) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    if (!dirname) return e_syserr_param;
    if (!pret) return e_syserr_param;
    if(len < SDCARD_LS_MIN_CHAR) return e_syserr_param;
    DIR *dir = opendir(dirname);
    if (!dir) {
        return e_syserr_file_generic;
    }
    struct dirent *entry;
    uint16_t j = 0;
    int16_t len_safe = UTILS_CLIP_MAX(INT16_MAX, len);
    int16_t chars_left = len_safe;
    const char cont[] = "...";
    uint16_t offset = 0;
    while ((entry = readdir(dir)) != NULL && j < n_entries) {
        char* pr = entry->d_name;
        uint8_t pr_len = strlen(pr);
        pr_len = UTILS_CLIP_MAX(SDCARD_LS_ENTRY_MAX_LEN, pr_len);
        if(!strncmp(pr, "System Volume Information", pr_len)) continue;
        if(!strncmp(pr, ".Trash-1000", pr_len)) continue;
        if (chars_left < pr_len + 1 + sizeof(cont)) {
            if (chars_left >= sizeof(cont)) {
                strncpy(&pret[offset], cont, sizeof(cont));
            }
            break;
        }
        snprintf(&pret[offset], chars_left, "%.*s" SDCARD_LS_ENTRY_SEPARATOR, pr_len, pr);
        offset += pr_len + 1;
        chars_left -= pr_len + 1;
        j++;
        if (j == n_entries) {
            // Check if there are more files to indicate with "..."
            struct dirent *next_entry = readdir(dir);
            if (next_entry != NULL) {
                // There are more files, add "..." if space allows
                if (chars_left >= sizeof(cont)) {
                    strncpy(&pret[offset], cont, sizeof(cont));
                }
            }
            break;
        }
    }
    closedir(dir);
    return e_syserr_none;
}

e_syserr_t sd_cat(const char *fname, char* pret, uint16_t len) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    uint32_t points_r = 0;
    e_syserr_t e = sd_read_txt(pret, len, fname, 0, &points_r);
    if (e == e_syserr_file_eof) {
        return e_syserr_none;
    }
    return e;
}

e_syserr_t sd_mk(const char* fname) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    if (sd_file_exists(fname)) {
        return e_syserr_file_duplicate;
    }
    return sd_create_file(fname);
}

e_syserr_t sd_rm(const char* fname) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    if (!sd_file_exists(fname)) {
        return e_syserr_file_missing;
    }
    return sd_delete_file(fname);
}

uint8_t sd_file_exists(const char *fname) {
    if (!mounted) return 0;
    FILE* f = fopen(fname, "r");
    if (f == NULL) return 0;
    fclose(f);
    return 1;
}

e_syserr_t sd_get_unique_fname(char* proposed) {
    if (!mounted) return e_syserr_sdcard_unmnted;
    uint8_t len = strlen(SDCARD_BASE_PATH "/" SDCARD_DEFAULT_FNAME_WAV);
    if (strlen(proposed) != len) return e_syserr_param;
    uint16_t idx = 0;
    e_syserr_t e = e_syserr_none;
    while (sd_file_exists(proposed)) {
        char digits[5] = "xxxx";
        memcpy(digits, &proposed[len - 8], 4);
        if ((e = str_to_4digit_uint(digits, &idx)) != e_syserr_none) return e;
        if ((e = uint_to_4digit_str(++idx, digits)) != e_syserr_none) return e;
        memcpy(&proposed[len - 8], digits, 4);
    }
    return e;
}

void sd_job(void* p) {
    char* args = jes_job_get_args();
    char* arg = strtok(args, " ");
    job_struct_t* pj = (job_struct_t*)p;
    e_syserr_t e;
    
    if (jes_job_is_arg(arg, SDCARD_CMD_MOUNT)) {
        if ((e = sd_mnt()) != e_syserr_none) {
            jes_print_pj(pj, SDCARD_MSG_MOUNT_FAIL);
        } else {
            jes_print_pj(pj, SDCARD_MSG_MOUNTED);
        }
    }
    else if (jes_job_is_arg(arg, SDCARD_CMD_UNMOUNT)) {
        if ((e = sd_unmnt()) != e_syserr_none) {
            jes_print_pj(pj, SDCARD_MSG_UNMOUNT_FAIL);
        } else {
            jes_print_pj(pj, SDCARD_MSG_UNMOUNTED);
        }
    }
    else if (jes_job_is_arg(arg, SDCARD_CMD_HELP)) {
        jes_print_pj(pj, SDCARD_CMDS);
    }
    else if (jes_job_is_arg(arg, SDCARD_CMD_LIST)) {
        arg = strtok(NULL, " ");
        if (arg == NULL) { arg = (char*)"\0"; }
        char buf[SDCARD_PATH_MAX_CHAR];
        char ret[SDCARD_LS_MAX_CHAR];
        sprintf(buf, "%s/%s\0", SDCARD_BASE_PATH, arg);
        if ((e = sd_ls(buf, ret, SDCARD_LS_MAX_ENTRIES, SDCARD_LS_MAX_CHAR)) != e_syserr_none) {
            jes_print_pj(pj, SDCARD_MSG_LS_ERROR_errnum, e);
            return;
        }
        char* ent = strtok(ret, SDCARD_LS_ENTRY_SEPARATOR);
        while(ent){
            jes_print_pj(pj, "%s\n", ent);
            ent = strtok(NULL, SDCARD_LS_ENTRY_SEPARATOR);
        }
    }
    else if (jes_job_is_arg(arg, SDCARD_CMD_READ)) {
        arg = strtok(NULL, " ");
        if (arg == NULL) {
            jes_print_pj(pj, SDCARD_MSG_CAT_ERROR_USAGE);
            return;
        }
        char buf[SDCARD_PATH_MAX_CHAR];
        char ret[SDCARD_CAT_MAX_CHAR];
        sprintf(buf, "%s/%s\0", SDCARD_BASE_PATH, arg);
        if ((e = sd_cat(buf, ret, SDCARD_CAT_MAX_CHAR)) != e_syserr_none) {
            jes_print_pj(pj, SDCARD_MSG_CAT_ERROR_errnum, e);
            return;
        }
        jes_print_pj(pj, ret);
    }
    else if (jes_job_is_arg(arg, SDCARD_CMD_CREATE)) {
        arg = strtok(NULL, " ");
        if (arg == NULL) {
            jes_print_pj(pj, SDCARD_MSG_MK_ERROR_USAGE);
            return;
        }
        char buf[SDCARD_PATH_MAX_CHAR];
        sprintf(buf, "%s/%s\0", SDCARD_BASE_PATH, arg);
        if ((e = sd_mk(buf)) != e_syserr_none) {
            jes_print_pj(pj, SDCARD_MSG_MK_ERROR_errnum, e);
            return;
        }
    }
    else if (jes_job_is_arg(arg, SDCARD_CMD_REMOVE)) {
        arg = strtok(NULL, " ");
        if (arg == NULL) {
            jes_print_pj(pj, SDCARD_MSG_RM_ERROR_USAGE);
            return;
        }
        char buf[SDCARD_PATH_MAX_CHAR];
        sprintf(buf, "%s/%s\0", SDCARD_BASE_PATH, arg);
        if ((e = sd_rm(buf)) != e_syserr_none) {
            jes_print_pj(pj, SDCARD_MSG_RM_ERROR_errnum, e);
            return;
        }
    }
    else if (jes_job_is_arg(arg, SDCARD_CMD_MEMORY)) {
        uint32_t free_kbytes = 0;
        uint32_t all_kbytes = 0;
        e_syserr_t e;
        if ((e = sd_get_free_kbytes(&free_kbytes, &all_kbytes)) != e_syserr_none) {
            jes_print_pj(pj, SDCARD_MSG_MEM_ERROR_errnum, e);
            return;
        }
        jes_print_pj(pj, SDCARD_MSG_MEM_FORMAT_free_tot, free_kbytes, all_kbytes);
    }
    else {
        jes_print_pj(pj, SDCARD_MSG_UNKNOWN_CMD "\n");
        jes_print_pj(pj, SDCARD_CMDS);
    }
}

static inline void __sd_transfer(void* p) {
    job_struct_t* pj = (job_struct_t*)p;
    pj->role = e_role_core;
    /* A local buffer is required because the file operations (function scopes
    in fread() and fwrite()) seem to take too long for being called as blocking
    functions when the data transfer occurs via SPI. DMA is fast, but the
    abstraction around it isn't, which is why this non-blocking stream server
    exists. It requires a local buffer. This means that the notification stack
    + server loop + memcpy is "faster" (in a blockwise task context) than just 
    calling fread()/fwrite() in the audio callback. */
    static uint8_t local_buf[SDCARD_SD_STREAM_POOL_SIZE];
    while (1) {
        sd_stream_descriptor_t stream = *(sd_stream_descriptor_t*)jes_wait_for_notification();
        __job_set_timing_begin(__get_systime_ms(), pj);
        size_t points_transferred = 0;
        memcpy(local_buf, stream.data, stream.type_in_byte * stream.block_len);
        xSemaphoreTake(stream_lock, portMAX_DELAY);
        if (stream.direction == sd_stream_direction_in) {
            points_transferred = fwrite(local_buf, stream.type_in_byte, stream.block_len, stream.f);
        }
        if (stream.direction == sd_stream_direction_out) {
            points_transferred = fread(local_buf, stream.type_in_byte, stream.block_len, stream.f);
        }
        xSemaphoreGive(stream_lock);
        e_syserr_t transfer_error = e_syserr_none;
        if (points_transferred != stream.block_len) { 
            LIBFRX_SYS_DEBUG_PRINT_PJ(pj, "Data given: %d, transferred: %d", stream.block_len, points_transferred);
            transfer_error = e_syserr_file_generic;
            jes_throw_error((jes_err_t)transfer_error); 
        }
        xSemaphoreTake(stream_lock, portMAX_DELAY);
        stream_last_points = points_transferred;
        stream_last_error = transfer_error;
        stream_busy = 0;
        xSemaphoreGive(stream_done);
        xSemaphoreGive(stream_lock);
        __job_set_timing_end(__get_systime_ms(), pj);
    }
}

#endif // FRX_ENABLE_MODULE_SDCARD
