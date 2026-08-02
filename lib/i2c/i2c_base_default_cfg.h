#ifndef _I2C_BASE_DEFAULT_CFG_H_
#define _I2C_BASE_DEFAULT_CFG_H_

#ifdef FRX_ENABLE_MODULE_I2C

// REQUIRED: Projects MUST define in platformio.ini (see PIN_DEFS.md)
// No defaults - each project has different hardware
#ifndef I2C_BASE_SCL
#error "I2C_BASE_SCL must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

#ifndef I2C_BASE_SDA
#error "I2C_BASE_SDA must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

#ifndef I2C_BASE_NUM
#error "I2C_BASE_NUM must be defined in platformio.ini (see PIN_DEFS.md)"
#endif

#ifndef I2C_BASE_SPEED
#define I2C_BASE_SPEED  100000
#endif

#ifndef I2C_BASE_BUS_TXRX_TIMEOUT
#define I2C_BASE_BUS_TXRX_TIMEOUT pdMS_TO_TICKS(1000)
#endif

#ifndef I2C_BASE_BUS_LOCK_TIMEOUT
#define I2C_BASE_BUS_LOCK_TIMEOUT pdMS_TO_TICKS(1000)
#endif

#ifndef I2C_BASE_SCAN_JOB_MEM
#define I2C_BASE_SCAN_JOB_MEM 2048
#endif

#ifndef I2C_BASE_SCAN_MAX_DEVICES
#define I2C_BASE_SCAN_MAX_DEVICES 32
#endif

#endif // FRX_ENABLE_MODULE_I2C
#endif // _I2C_BASE_DEFAULT_CFG_H_
