# libFRX Required Pin Definitions

Each project consuming libFRX **MUST** define the required pin macros in their `platformio.ini` build flags. No defaults are provided in libFRX - each FRX device has different hardware configurations.

## Audio Module

**Required macros:**
```ini
-DAUDIO_I2S_PORT=0           ; I2S port number (I2S_NUM_0, I2S_NUM_1, etc.)
-DAUDIO_PIN_MEMS_I2S_BCLK=23 ; I2S bit clock pin
-DAUDIO_PIN_MEMS_I2S_WS=4    ; I2S word select pin
-DAUDIO_PIN_MEMS_I2S_IN=27  ; I2S data input pin
```

**Optional macros (with defaults):**
```ini
-DAUDIO_SERVER_JOB_NAME="audio"      ; Job name
-DAUDIO_SERVER_JOB_MEM=4096          ; Job memory
-DAUDIO_FRAME_LEN=1024              ; Frame length
-DAUDIO_MAX_NUM_CH=2                 ; Max channels
-DAUDIO_SR_DEFAULT=48000             ; Default sample rate
-DAUDIO_I2S_RESTART_MS=200           ; Restart delay ms
```

**Example (FR1-mini):**
```ini
build_flags =
    -DAUDIO_I2S_PORT=0
    -DAUDIO_PIN_MEMS_I2S_BCLK=23
    -DAUDIO_PIN_MEMS_I2S_WS=4
    -DAUDIO_PIN_MEMS_I2S_IN=27
```

**Example (FR2):**
```ini
build_flags =
    -DAUDIO_I2S_PORT=0
    -DAUDIO_PIN_MEMS_I2S_BCLK=15
    -DAUDIO_PIN_MEMS_I2S_WS=17
    -DAUDIO_PIN_MEMS_I2S_IN=16
```

---

## I2C Module (i2c_base)

**Required macros:**
```ini
-DI2C_BASE_SCL=21            ; I2C clock pin
-DI2C_BASE_SDA=20            ; I2C data pin
-DI2C_BASE_NUM=0             ; I2C port number (I2C_NUM_0, I2C_NUM_1)
```

**Optional macros (with defaults):**
```ini
-DI2C_BASE_SPEED=100000      ; 100 kHz
-DI2C_BASE_BUS_TXRX_TIMEOUT=pdMS_TO_TICKS(1000)
-DI2C_BASE_BUS_LOCK_TIMEOUT=pdMS_TO_TICKS(1000)
```

**Example (FR2):**
```ini
build_flags =
    -DI2C_BASE_SCL=21
    -DI2C_BASE_SDA=20
    -DI2C_BASE_NUM=0
```

---

## External Flash Module (ext_flash)

**Required macros:**
```ini
-DEF_SPI_HOST=1              ; SPI host (SPI2_HOST, SPI3_HOST, etc.)
-DEF_PIN_MISO=46            ; SPI MISO pin
-DEF_PIN_MOSI=11            ; SPI MOSI pin
-DEF_PIN_SCLK=12            ; SPI clock pin
-DEF_PIN_CS=14              ; SPI chip select pin
```

**Optional macros (with defaults):**
```ini
-DEF_CLK_SPEED_HZ=10000000  ; 10 MHz
-DEF_ROM_ADDR=0x1000        ; ROM data address
-DEF_SETT_ADDR=0x2000       ; Settings address
-DEF_SERVER_JOB_NAME="ef"
-DEF_SERVER_JOB_MEM=2048
```

**Example (FR2):**
```ini
build_flags =
    -DEF_SPI_HOST=1
    -DEF_PIN_MISO=46
    -DEF_PIN_MOSI=11
    -DEF_PIN_SCLK=12
    -DEF_PIN_CS=14
```

---

## SDCard Module

**For SPI mode (FR1-mini):**
```ini
-DSDCARD_MODE=sd_mode_spi
-DSDCARD_SPI_PIN_CS=13       ; SPI chip select
-DSDCARD_SPI_PIN_MOSI=15     ; SPI MOSI
-DSDCARD_SPI_PIN_MISO=2      ; SPI MISO
-DSDCARD_SPI_PIN_SCK=14      ; SPI clock
```

**For SDMMC mode (FR2):**
```ini
-DSDCARD_MODE=sd_mode_sdmmc
-DSDCARD_SDMMC_PIN_CLK=36    ; SDMMC clock
-DSDCARD_SDMMC_PIN_CMD=35    ; SDMMC command
-DSDCARD_SDMMC_PIN_D0=37     ; SDMMC data 0
-DSDCARD_SDMMC_PIN_D1=38     ; SDMMC data 1
-DSDCARD_SDMMC_PIN_D2=45     ; SDMMC data 2
-DSDCARD_SDMMC_PIN_D3=39     ; SDMMC data 3
```

**For auto-detection:**
```ini
-DSDCARD_MODE=sd_mode_auto
; Define both SPI and SDMMC pins - will try SDMMC first, then SPI
```

**Optional macros (with defaults):**
```ini
-DSDCARD_BASE_PATH="/sdcard"
-DSDCARD_PAGE_SIZE_BYTE=512
-DSDCARD_SERVER_JOB_NAME="sdcard"
-DSDCARD_STREAMER_JOB_NAME="sdstrm"
-DSDCARD_MAX_FILES_DEFAULT=5
-DSDCARD_MAX_FREQ_BUS_DEFAULT=SDMMC_FREQ_DEFAULT
-DSDCARD_LS_MAX_CHAR=256
-DSDCARD_CAT_MAX_CHAR=256
-DSDCARD_PATH_MAX_CHAR=64
-DSDCARD_DEFAULT_FNAME_WAV="fr1_rec_0000.wav"
-DSDCARD_METADATA_FNAME=".fr2_metadata.txt"
```

---

## Complete FR1-mini Configuration

```ini
build_flags =
    ; Audio
    -DAUDIO_I2S_PORT=0
    -DAUDIO_PIN_MEMS_I2S_BCLK=23
    -DAUDIO_PIN_MEMS_I2S_WS=4
    -DAUDIO_PIN_MEMS_I2S_IN=27
    
    ; I2C
    -DI2C_BASE_SCL=21
    -DI2C_BASE_SDA=20
    -DI2C_BASE_NUM=0
    
    ; SDCard (SPI mode)
    -DSDCARD_MODE=sd_mode_spi
    -DSDCARD_SPI_PIN_CS=13
    -DSDCARD_SPI_PIN_MOSI=15
    -DSDCARD_SPI_PIN_MISO=2
    -DSDCARD_SPI_PIN_SCK=14
```

---

## Complete FR2 Configuration

```ini
build_flags =
    ; Audio
    -DAUDIO_I2S_PORT=0
    -DAUDIO_PIN_MEMS_I2S_BCLK=15
    -DAUDIO_PIN_MEMS_I2S_WS=17
    -DAUDIO_PIN_MEMS_I2S_IN=16
    
    ; I2C
    -DI2C_BASE_SCL=21
    -DI2C_BASE_SDA=20
    -DI2C_BASE_NUM=0
    
    ; External Flash
    -DEF_SPI_HOST=1
    -DEF_PIN_MISO=46
    -DEF_PIN_MOSI=11
    -DEF_PIN_SCLK=12
    -DEF_PIN_CS=14
    
    ; SDCard (SDMMC mode)
    -DSDCARD_MODE=sd_mode_sdmmc
    -DSDCARD_SDMMC_PIN_CLK=36
    -DSDCARD_SDMMC_PIN_CMD=35
    -DSDCARD_SDMMC_PIN_D0=37
    -DSDCARD_SDMMC_PIN_D1=38
    -DSDCARD_SDMMC_PIN_D2=45
    -DSDCARD_SDMMC_PIN_D3=39
```

---

## PlatformIO Environment Examples

```ini
; FR1-mini environment
[env:fr1_audio_test]
platform = espressif32
board = esp-wrover-kit
framework = arduino
build_flags =
    ; Audio pins
    -DAUDIO_I2S_PORT=0
    -DAUDIO_PIN_MEMS_I2S_BCLK=23
    -DAUDIO_PIN_MEMS_I2S_WS=4
    -DAUDIO_PIN_MEMS_I2S_IN=27
    ; Other required macros...
lib_deps =
    git@github.com:jesdev-io/libFRX.git

; FR2 environment
[env:fr2_audio_test]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
build_flags =
    ; Audio pins
    -DAUDIO_I2S_PORT=0
    -DAUDIO_PIN_MEMS_I2S_BCLK=15
    -DAUDIO_PIN_MEMS_I2S_WS=17
    -DAUDIO_PIN_MEMS_I2S_IN=16
    ; Other required macros...
lib_deps =
    git@github.com:jesdev-io/libFRX.git
```
