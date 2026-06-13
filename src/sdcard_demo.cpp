#include <Arduino.h>
#include "jescore.h"
#include "sdcard.h"

void setup(){
    jes_init();
    sd_init_default();
}

void loop(){
    /*
    This is a jescore compat-layer demo for the sdcard module.

    Install jescorecli: https://github.com/jesdev-io/jescorecli

    List of available calls:
    - mnt: mount the sd card
    - unmnt: unmount the sd card
    - ls: list files in the home dir. 
        Home dir set by SDCARD_BASE_PATH.
        Output size set by SDCARD_LS_MAX_CHAR.
    - cat <file>: read and print text content of a file.
        Output len set by SDCARD_CAT_MAX_CHAR
    - mk <file>: Create an empty file.
    - rm <file>: Remove a file. This is irreversible!
    - mem: List available memory on the card.

    Want more? Contribute! 
    https://github.com/jesdev-io/libFRX/issues
    https://github.com/jesdev-io/libFRX/pulls
    */
}