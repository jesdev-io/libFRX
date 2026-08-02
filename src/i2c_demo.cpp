#include <Arduino.h>
#include "jescore.h"
#include "i2c_base.h"

void setup(){
    jes_init();
    i2c_base_init_default();
}

void loop(){
    /*
    This is a jescore compat-layer demo for the I2C base module.

    Install jescorecli: https://github.com/jesdev-io/jescorecli

    List of available calls:
    - i2cscan: scan the configured I2C bus and print found slave addresses.

    Want more? Contribute!
    https://github.com/jesdev-io/libFRX/issues
    https://github.com/jesdev-io/libFRX/pulls
    */
}
