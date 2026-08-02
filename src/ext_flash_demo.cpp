#include <Arduino.h>
#include "jescore.h"
#include "ext_flash.h"

void setup(){
    jes_init();
    ef_init_default();
}

void loop(){
    /*
    This is a jescore compat-layer demo for the external flash module.

    Install jescorecli: https://github.com/jesdev-io/jescorecli

    List of available calls:
    - ef rom: display external flash JEDEC/PID and ROM metadata.

    Want more? Contribute!
    https://github.com/jesdev-io/libFRX/issues
    https://github.com/jesdev-io/libFRX/pulls
    */
}
