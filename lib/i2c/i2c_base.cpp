#ifdef FRX_ENABLE_MODULE_I2C

#include "driver/i2c.h"
#include "jescore.h"
#include "i2c_base.h"

static uint8_t init = 0;
static SemaphoreHandle_t i2c_lock;

e_syserr_t i2c_base_init(uint8_t scl, uint8_t sda, uint32_t speed){
    if(init) return e_syserr_none;

    esp_err_t ee;
    i2c_config_t conf;
    // https://github.com/mkfrey/u8g2-hal-esp-idf/issues/1#issuecomment-899016331
    memset(&conf, 0, sizeof(i2c_config_t));

    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = speed;

    i2c_lock = xSemaphoreCreateMutex();
    if(!i2c_lock) return e_syserr_null;

    ee = i2c_param_config(I2C_BASE_NUM, &conf);
    if(ee != ESP_OK) return e_syserr_driver_fail;
    ee = i2c_driver_install(I2C_BASE_NUM, I2C_MODE_MASTER, 0, 0, 0);
    if(ee != ESP_OK) return e_syserr_driver_fail;
    jes_err_t je = jes_register_job(I2C_BASE_SCAN_JOB_NAME, I2C_BASE_SCAN_JOB_MEM, 1, i2c_base_scan_job, 0, 1);
    if (je != e_err_no_err && je != e_err_duplicate) return (e_syserr_t)je;
    init = 1;
    return e_syserr_none;
}

e_syserr_t i2c_base_init_default(void){
    return i2c_base_init(I2C_BASE_SCL, I2C_BASE_SDA, I2C_BASE_SPEED);
}

e_syserr_t i2c_base_transmit(uint8_t addr, uint8_t *tx_buf, uint32_t len, TickType_t timeout){
    if(!init) return e_syserr_uninitialized;
    if(tx_buf == NULL) return e_syserr_param;
    esp_err_t ee;
    if(xSemaphoreTake(i2c_lock, I2C_BASE_BUS_LOCK_TIMEOUT) == pdFALSE){
        return e_syserr_locked;
    }
    ee = i2c_master_write_to_device(I2C_BASE_NUM,
                                    addr,
                                    (const uint8_t*)tx_buf,
                                    (size_t)len,
                                    timeout);
    xSemaphoreGive(i2c_lock);
    if(ee != ESP_OK) return e_syserr_driver_fail;
    return e_syserr_none;
}

e_syserr_t i2c_base_receive(uint8_t addr, uint8_t *rx_buf, uint32_t len, TickType_t timeout){
    if(!init) return e_syserr_uninitialized;
    if(rx_buf == NULL) return e_syserr_param;
    esp_err_t ee;
    if(xSemaphoreTake(i2c_lock, I2C_BASE_BUS_LOCK_TIMEOUT) == pdFALSE){
        return e_syserr_locked;
    }
    ee = i2c_master_read_from_device(I2C_BASE_NUM, 
                                     addr, 
                                     rx_buf, 
                                     (size_t)len, 
                                     timeout);
    xSemaphoreGive(i2c_lock);
    if(ee != ESP_OK) return e_syserr_driver_fail;
    return e_syserr_none;
}

int32_t i2c_base_scan_bus(uint8_t *found_devices, uint32_t max_devices, TickType_t timeout) {
    if(!init) return e_syserr_uninitialized;
    if(found_devices == NULL) return e_syserr_param;
    if(max_devices == 0) return 0;
    int32_t device_count = 0;
    uint8_t num_addresses = 255;
    for(uint8_t i = 0; i < num_addresses; i++) {
        uint8_t test_byte = 0x00;
        e_syserr_t e = i2c_base_transmit(i, &test_byte, 1, timeout);        
        if(e == e_syserr_none) {
            found_devices[device_count++] = i;
            if(device_count >= max_devices) break;
        }
    }
    return device_count;
}

void i2c_base_scan_job(void* p) {
    job_struct_t* pj = (job_struct_t*)p; 
    jes_print_pj(pj, I2C_BASE_MSG_SCAN_START "\n\r");
    uint8_t found_devices[I2C_BASE_SCAN_MAX_DEVICES];
    int32_t device_count = i2c_base_scan_bus(found_devices, I2C_BASE_SCAN_MAX_DEVICES, I2C_BASE_BUS_TXRX_TIMEOUT);
    jes_print_pj(pj, I2C_BASE_MSG_SCAN_FOUND "\n\r", device_count);
    for(uint8_t i = 0; i < device_count; i++){
        jes_print_pj(pj, I2C_BASE_MSG_SCAN_ADDR "\n\r", (uint32_t)found_devices[i]);
    }
}

#endif // FRX_ENABLE_MODULE_I2C
