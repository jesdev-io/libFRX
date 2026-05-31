#ifndef _I2C_BASE_H_
#define _I2C_BASE_H_

#ifdef FRX_ENABLE_MODULE_I2C

#include <inttypes.h>
#include <jescore.h>
#include "syserr.h"
#include <driver/i2c.h>

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

e_syserr_t i2c_base_init(uint8_t scl, uint8_t sda, uint32_t speed);

e_syserr_t i2c_base_init_default(void);

e_syserr_t i2c_base_transmit(uint8_t addr, uint8_t *tx_buf, uint32_t len, TickType_t timeout);

e_syserr_t i2c_base_receive(uint8_t addr, uint8_t *rx_buf, uint32_t len, TickType_t timeout);

/// @brief Scan I2C bus for connected slave devices
/// @param found_devices Array to store addresses of found devices
/// @param max_devices Maximum number of devices to detect
/// @param timeout Timeout for each I2C operation
/// @return Number of devices found, or error code if negative
int32_t i2c_base_scan_bus(uint8_t *found_devices, uint32_t max_devices, TickType_t timeout);

/// @brief jescore job handler for I2C slave detection
/// @param p jescore job parameters
void i2c_base_scan_job(void* p);

#endif // FRX_ENABLE_MODULE_I2C
#endif // _I2C_BASE_H_
