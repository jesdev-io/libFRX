#ifndef _I2C_RTC_DS3231_JCCL_H_
#define _I2C_RTC_DS3231_JCCL_H_

#ifdef FRX_ENABLE_MODULE_RTC_DS3231

#define DS3231_JOB_NAME                 "rtc"

#define DS3231_CMD_TIME                 "time"
#define DS3231_CMD_TEMP                 "temp"
#define DS3231_CMD_HELP                 "help"

#define DS3231_CMDS                     "Available commands: "\
                                        DS3231_CMD_TIME ", "\
                                        DS3231_CMD_TEMP

#define DS3231_MSG_ERROR_TIME_errnum    "Error while reading RTC time. (%d)"
#define DS3231_MSG_ERROR_TEMP_errnum    "Error while reading RTC temperature. (%d)"
#define DS3231_MSG_TIME_FORMAT          "%04d-%02d-%02d %02d:%02d:%02d"
#define DS3231_MSG_TEMP_FORMAT          "%.2f C"
#define DS3231_MSG_UNKNOWN_CMD          "Unknown RTC command."

#endif // FRX_ENABLE_MODULE_RTC_DS3231
#endif // _I2C_RTC_DS3231_JCCL_H_
