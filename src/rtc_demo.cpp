#include <Arduino.h>
#include "jescore.h"
#include "i2c_rtc_ds3231.h"

void setup(){
    jes_init();
    i2c_rtc_ds3231_init_default();
}

void loop(){
    /*
    This is a jescore compat-layer demo for the DS3231 RTC module.

    Install jescorecli: https://github.com/jesdev-io/jescorecli

    List of available calls:
    - rtc help: list available RTC commands.
    - rtc time: print RTC time as YYYY-MM-DD HH:MM:SS.
    - rtc temp: print DS3231 temperature in Celsius.

    Want more? Contribute!
    https://github.com/jesdev-io/libFRX/issues
    https://github.com/jesdev-io/libFRX/pulls
    */
}
