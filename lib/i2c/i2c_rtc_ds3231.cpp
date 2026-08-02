#ifdef FRX_ENABLE_MODULE_RTC_DS3231

#include <string.h>
#include <time.h>
#include "jescore.h"
#include "i2c_rtc_ds3231.h"

static uint8_t initialized = 0;

static inline uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static inline uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

static e_syserr_t validate_time(const struct tm *timeinfo){
    if(timeinfo == NULL) return e_syserr_param;
    if(timeinfo->tm_sec < 0 || timeinfo->tm_sec > 59) return e_syserr_param;
    if(timeinfo->tm_min < 0 || timeinfo->tm_min > 59) return e_syserr_param;
    if(timeinfo->tm_hour < 0 || timeinfo->tm_hour > 23) return e_syserr_param;
    if(timeinfo->tm_wday < 0 || timeinfo->tm_wday > 6) return e_syserr_param;
    if(timeinfo->tm_mday < 1 || timeinfo->tm_mday > 31) return e_syserr_param;
    if(timeinfo->tm_mon < 0 || timeinfo->tm_mon > 11) return e_syserr_param;
    if((timeinfo->tm_year + 1900) < DS3231_YEAR_MIN) return e_syserr_param;
    if((timeinfo->tm_year + 1900) > DS3231_YEAR_MAX) return e_syserr_param;
    return e_syserr_none;
}

e_syserr_t i2c_rtc_ds3231_init(uint8_t scl, uint8_t sda, uint32_t speed) {
    e_syserr_t e = i2c_base_init(scl, sda, speed);
    if(e != e_syserr_none) return e;

    jes_err_t je = jes_register_job(DS3231_JOB_NAME, DS3231_JOB_MEM, 1, i2c_rtc_ds3231_job, 0, 1);
    if(je != e_err_no_err && je != e_err_duplicate) {
        jes_throw_error(je);
        return (e_syserr_t)je;
    }
    initialized = 1;
    return e_syserr_none;
}

e_syserr_t i2c_rtc_ds3231_init_default(void) {
    return i2c_rtc_ds3231_init(I2C_BASE_SCL, I2C_BASE_SDA, I2C_BASE_SPEED);
}

uint8_t i2c_rtc_ds3231_is_initialized(void){
    return initialized;
}

e_syserr_t i2c_rtc_ds3231_get_time(struct tm *timeinfo) {
    if(!initialized) return e_syserr_uninitialized;
    if(timeinfo == NULL) return e_syserr_param;

    uint8_t reg_addr = DS3231_REG_SECONDS;
    e_syserr_t e = i2c_base_transmit(DS3231_I2C_ADDRESS, &reg_addr, 1, I2C_BASE_BUS_TXRX_TIMEOUT);
    if(e != e_syserr_none) return e;

    uint8_t data[7];
    e = i2c_base_receive(DS3231_I2C_ADDRESS, data, 7, I2C_BASE_BUS_TXRX_TIMEOUT);
    if(e != e_syserr_none) return e;

    memset(timeinfo, 0, sizeof(struct tm));
    timeinfo->tm_sec = bcd_to_dec(data[0] & 0x7F);
    timeinfo->tm_min = bcd_to_dec(data[1] & 0x7F);

    if(data[2] & 0x40) {
        timeinfo->tm_hour = bcd_to_dec(data[2] & 0x1F);
        if(data[2] & 0x20) timeinfo->tm_hour += 12;
        if(timeinfo->tm_hour == 24) timeinfo->tm_hour = 12;
    }
    else {
        timeinfo->tm_hour = bcd_to_dec(data[2] & 0x3F);
    }

    uint8_t ds3231_wday = bcd_to_dec(data[3] & 0x07);
    timeinfo->tm_wday = ds3231_wday ? ds3231_wday - 1 : 0;
    timeinfo->tm_mday = bcd_to_dec(data[4] & 0x3F);
    timeinfo->tm_mon = bcd_to_dec(data[5] & 0x1F) - 1;

    uint8_t year = bcd_to_dec(data[6]);
    if(data[5] & 0x80) {
        timeinfo->tm_year = 100 + year;
    }
    else {
        timeinfo->tm_year = year;
    }
    return e_syserr_none;
}

e_syserr_t i2c_rtc_ds3231_set_time(const struct tm *timeinfo) {
    if(!initialized) return e_syserr_uninitialized;
    e_syserr_t e = validate_time(timeinfo);
    if(e != e_syserr_none) return e;

    uint8_t data[8];
    data[0] = DS3231_REG_SECONDS;
    data[1] = dec_to_bcd((uint8_t)timeinfo->tm_sec);
    data[2] = dec_to_bcd((uint8_t)timeinfo->tm_min);
    data[3] = dec_to_bcd((uint8_t)timeinfo->tm_hour);
    data[4] = dec_to_bcd((uint8_t)(timeinfo->tm_wday + 1));
    data[5] = dec_to_bcd((uint8_t)timeinfo->tm_mday);
    data[6] = dec_to_bcd((uint8_t)(timeinfo->tm_mon + 1));

    if(timeinfo->tm_year >= 100) {
        data[6] |= 0x80;
        data[7] = dec_to_bcd((uint8_t)(timeinfo->tm_year - 100));
    }
    else {
        data[6] &= 0x7F;
        data[7] = dec_to_bcd((uint8_t)timeinfo->tm_year);
    }

    return i2c_base_transmit(DS3231_I2C_ADDRESS, data, 8, I2C_BASE_BUS_TXRX_TIMEOUT);
}

e_syserr_t i2c_rtc_ds3231_get_temp(float *temp) {
    if(!initialized) return e_syserr_uninitialized;
    if(temp == NULL) return e_syserr_param;

    uint8_t reg_addr = DS3231_REG_TEMP_MSB;
    e_syserr_t e = i2c_base_transmit(DS3231_I2C_ADDRESS, &reg_addr, 1, I2C_BASE_BUS_TXRX_TIMEOUT);
    if(e != e_syserr_none) return e;

    uint8_t data[2];
    e = i2c_base_receive(DS3231_I2C_ADDRESS, data, 2, I2C_BASE_BUS_TXRX_TIMEOUT);
    if(e != e_syserr_none) return e;

    int16_t temp_raw = (int16_t)((data[0] << 8) | data[1]);
    temp_raw >>= 6;
    *temp = (float)temp_raw * 0.25f;
    return e_syserr_none;
}

static void print_time(job_struct_t* pj, const struct tm* timeinfo){
    jes_print_pj(pj, DS3231_MSG_TIME_FORMAT "\n\r",
                 timeinfo->tm_year + 1900,
                 timeinfo->tm_mon + 1,
                 timeinfo->tm_mday,
                 timeinfo->tm_hour,
                 timeinfo->tm_min,
                 timeinfo->tm_sec);
}

void i2c_rtc_ds3231_job(void* p){
    job_struct_t* pj = (job_struct_t*)p;
    char* args = jes_job_get_args();
    char* arg = strtok(args, " ");

    if(arg == NULL || jes_job_is_arg(arg, DS3231_CMD_HELP)){
        jes_print_pj(pj, DS3231_CMDS "\n\r");
        return;
    }

    if(jes_job_is_arg(arg, DS3231_CMD_TIME)){
        struct tm timeinfo;
        e_syserr_t e = i2c_rtc_ds3231_get_time(&timeinfo);
        if(e != e_syserr_none){
            jes_print_pj(pj, DS3231_MSG_ERROR_TIME_errnum "\n\r", e);
            jes_throw_error((jes_err_t)e);
            return;
        }
        print_time(pj, &timeinfo);
    }
    else if(jes_job_is_arg(arg, DS3231_CMD_TEMP)){
        float temp = 0.0f;
        e_syserr_t e = i2c_rtc_ds3231_get_temp(&temp);
        if(e != e_syserr_none){
            jes_print_pj(pj, DS3231_MSG_ERROR_TEMP_errnum "\n\r", e);
            jes_throw_error((jes_err_t)e);
            return;
        }
        jes_print_pj(pj, DS3231_MSG_TEMP_FORMAT "\n\r", temp);
    }
    else{
        jes_print_pj(pj, DS3231_MSG_UNKNOWN_CMD "\n\r");
        jes_print_pj(pj, DS3231_CMDS "\n\r");
        jes_throw_error((jes_err_t)e_syserr_param);
    }
}

#endif // FRX_ENABLE_MODULE_RTC_DS3231
