#ifndef _I2C_RTC_DS3231_H_
#define _I2C_RTC_DS3231_H_

#ifdef FRX_ENABLE_MODULE_RTC_DS3231

#ifndef FRX_ENABLE_MODULE_I2C
#error "DS3231 RTC module requires I2C module to be enabled. Define FRX_ENABLE_MODULE_I2C."
#endif

#include <stdint.h>
#include <time.h>
#include "syserr.h"
#include "i2c_base.h"
#include "i2c_rtc_ds3231_default_cfg.h"
#include "i2c_rtc_ds3231_jccl.h"

// DS3231 register addresses
#define DS3231_REG_SECONDS      0x00
#define DS3231_REG_MINUTES      0x01
#define DS3231_REG_HOURS        0x02
#define DS3231_REG_DAY          0x03
#define DS3231_REG_DATE         0x04
#define DS3231_REG_MONTH        0x05
#define DS3231_REG_YEAR         0x06
#define DS3231_REG_ALARM1_SEC   0x07
#define DS3231_REG_ALARM1_MIN   0x08
#define DS3231_REG_ALARM1_HOUR  0x09
#define DS3231_REG_ALARM1_DAY   0x0A
#define DS3231_REG_ALARM2_MIN   0x0B
#define DS3231_REG_ALARM2_HOUR  0x0C
#define DS3231_REG_ALARM2_DAY   0x0D
#define DS3231_REG_CONTROL      0x0E
#define DS3231_REG_STATUS       0x0F
#define DS3231_REG_AGING_OFFSET 0x10
#define DS3231_REG_TEMP_MSB     0x11
#define DS3231_REG_TEMP_LSB     0x12

// Control register bits
#define DS3231_CONTROL_A1IE     (1 << 0)
#define DS3231_CONTROL_A2IE     (1 << 1)
#define DS3231_CONTROL_INTCN    (1 << 2)
#define DS3231_CONTROL_RS1      (1 << 3)
#define DS3231_CONTROL_RS2      (1 << 4)
#define DS3231_CONTROL_CONV     (1 << 5)
#define DS3231_CONTROL_BBSQW    (1 << 6)
#define DS3231_CONTROL_EOSC     (1 << 7)

// Status register bits
#define DS3231_STATUS_A1F       (1 << 0)
#define DS3231_STATUS_A2F       (1 << 1)
#define DS3231_STATUS_BSY       (1 << 2)
#define DS3231_STATUS_EN32KHZ   (1 << 3)
#define DS3231_STATUS_OSF       (1 << 7)

/// @brief Initialize the DS3231 RTC and its I2C bus.
/// @param scl SCL pin number.
/// @param sda SDA pin number.
/// @param speed I2C speed in Hz.
/// @return Error code.
e_syserr_t i2c_rtc_ds3231_init(uint8_t scl, uint8_t sda, uint32_t speed);

/// @brief Initialize the DS3231 RTC with the default I2C bus configuration.
/// @return Error code.
/// @note Uses I2C_BASE_SCL, I2C_BASE_SDA, and I2C_BASE_SPEED.
e_syserr_t i2c_rtc_ds3231_init_default(void);

/// @brief Check whether the DS3231 RTC module has been initialized.
/// @return 1 when initialized, 0 otherwise.
uint8_t i2c_rtc_ds3231_is_initialized(void);

/// @brief Get current time from the DS3231.
/// @param timeinfo Pointer to struct tm to store time.
/// @return Error code.
/// @note tm_year is represented in the standard C form: years since 1900.
e_syserr_t i2c_rtc_ds3231_get_time(struct tm *timeinfo);

/// @brief Set current time on the DS3231.
/// @param timeinfo Pointer to struct tm containing time to set.
/// @return Error code.
/// @note Writes 24-hour mode. tm_year is represented in the standard C form: years since 1900.
e_syserr_t i2c_rtc_ds3231_set_time(const struct tm *timeinfo);

/// @brief Read temperature from the DS3231.
/// @param temp Pointer to float to store temperature in Celsius.
/// @return Error code.
e_syserr_t i2c_rtc_ds3231_get_temp(float *temp);

/// @brief DS3231 RTC jescore job handler.
/// @param p jescore job parameter pointer.
/// @note Available arguments: `time`, `temp`.
void i2c_rtc_ds3231_job(void* p);

#endif // FRX_ENABLE_MODULE_RTC_DS3231
#endif // _I2C_RTC_DS3231_H_
