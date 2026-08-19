#include "syserr.h"
#include "ui_types.h"
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>

// Display configuration
#define WAVESHARE_TRIPLE_RIGHT_CS     10
#define WAVESHARE_TRIPLE_RIGHT_DC     8 
#define WAVESHARE_TRIPLE_RIGHT_RST    9

#define WAVESHARE_TRIPLE_LEFT_CS      7
#define WAVESHARE_TRIPLE_LEFT_DC      3
#define WAVESHARE_TRIPLE_LEFT_RST     18

#define WAVESHARE_TRIPLE_SMALL_MOSI   11
#define WAVESHARE_TRIPLE_SMALL_SCLK   12

#define WAVESHARE_TRIPLE_SMALL_TAG    INITR_MINI160x80_PLUGIN

#define WAVESHARE_TRIPLE_CENTER_CS    5
#define WAVESHARE_TRIPLE_CENTER_DC    4
#define WAVESHARE_TRIPLE_CENTER_RST   13
#define WAVESHARE_TRIPLE_CENTER_MOSI  41
#define WAVESHARE_TRIPLE_CENTER_SCLK  42

// Display dimensions
#define WAVESHARE_TRIPLE_SMALL_WIDTH   80
#define WAVESHARE_TRIPLE_SMALL_HEIGHT  160
#define WAVESHARE_TRIPLE_CENTER_WIDTH  240
#define WAVESHARE_TRIPLE_CENTER_HEIGHT 240

// Colors
#define WAVESHARE_TRIPLE_COLOR_BLACK      (ui_color_t)ST77XX_BLACK
#define WAVESHARE_TRIPLE_COLOR_WHITE      (ui_color_t)ST77XX_WHITE
#define WAVESHARE_TRIPLE_COLOR_RED        (ui_color_t)ST77XX_RED
#define WAVESHARE_TRIPLE_COLOR_GREEN      (ui_color_t)ST77XX_GREEN
#define WAVESHARE_TRIPLE_COLOR_BLUE       (ui_color_t)ST77XX_BLUE
#define WAVESHARE_TRIPLE_COLOR_CYAN       (ui_color_t)ST77XX_CYAN
#define WAVESHARE_TRIPLE_COLOR_MAGENTA    (ui_color_t)ST77XX_MAGENTA
#define WAVESHARE_TRIPLE_COLOR_YELLOW     (ui_color_t)ST77XX_YELLOW

// Background colors per display
#define WAVESHARE_TRIPLE_RIGHT_BG_COLOR  WAVESHARE_TRIPLE_COLOR_WHITE
#define WAVESHARE_TRIPLE_LEFT_BG_COLOR   WAVESHARE_TRIPLE_COLOR_WHITE
#define WAVESHARE_TRIPLE_CENTER_BG_COLOR WAVESHARE_TRIPLE_COLOR_BLACK

// Resolution matching factors
#define WAVESHARE_TRIPLE_CENTER_RES 1
#define WAVESHARE_TRIPLE_SMALL_RES  1
 
// basic UI types
typedef enum{
    WAVESHARE_TRIPLE_RIGHT = 0,
    WAVESHARE_TRIPLE_LEFT = 1,
    WAVESHARE_TRIPLE_CENTER = 2,
    WAVESHARE_TRIPLE_COUNT = 3
} waveshare_triple_display_idx_t;

// Static display objects
static SPIClass hspi(HSPI);

/* ============================================================
 *  Public display structs (extern, defined in .c)
 *  Use these directly in ui_display_register() and page setup.
 * ============================================================ */
// extern ui_display_t waveshare_triple_left;
// extern ui_display_t waveshare_triple_center;
// extern ui_display_t waveshare_triple_right;

static Adafruit_ST7735 waveshare_triple_right(WAVESHARE_TRIPLE_RIGHT_CS, WAVESHARE_TRIPLE_RIGHT_DC, WAVESHARE_TRIPLE_RIGHT_RST);
static Adafruit_ST7735 waveshare_triple_left(WAVESHARE_TRIPLE_LEFT_CS, WAVESHARE_TRIPLE_LEFT_DC, WAVESHARE_TRIPLE_LEFT_RST);
static Adafruit_ST7789 waveshare_triple_center(&hspi, WAVESHARE_TRIPLE_CENTER_CS, WAVESHARE_TRIPLE_CENTER_DC, WAVESHARE_TRIPLE_CENTER_RST);

// Display pointers array
static Adafruit_ST77xx* waveshare_triple_displays[WAVESHARE_TRIPLE_COUNT] = {
    (Adafruit_ST77xx*)&waveshare_triple_right,
    (Adafruit_ST77xx*)&waveshare_triple_left,
    (Adafruit_ST77xx*)&waveshare_triple_center
};

// Display dimensions
static const uint16_t fr2_ui_display_widths[WAVESHARE_TRIPLE_COUNT] = {
    WAVESHARE_TRIPLE_SMALL_WIDTH,
    WAVESHARE_TRIPLE_SMALL_WIDTH,
    WAVESHARE_TRIPLE_CENTER_WIDTH
};

static const uint16_t display_heights[WAVESHARE_TRIPLE_COUNT] = {
    WAVESHARE_TRIPLE_SMALL_HEIGHT,
    WAVESHARE_TRIPLE_SMALL_HEIGHT,
    WAVESHARE_TRIPLE_CENTER_HEIGHT
};

// Background colors
static const uint16_t waveshare_triple_bg_colors[WAVESHARE_TRIPLE_COUNT] = {
    WAVESHARE_TRIPLE_RIGHT_BG_COLOR,
    WAVESHARE_TRIPLE_LEFT_BG_COLOR,
    WAVESHARE_TRIPLE_CENTER_BG_COLOR
};

 
/* ============================================================
 *  Waveshare Triple TFT – display identifiers
 *  Three sub-displays on a single board, typically driven via SPI.
 *  Each is 128 × 128 px, 1-bit mono, with a two-colour palette
 *  (black + one accent colour that differs per panel variant).
 * ============================================================ */
#define WAVESHARE_TRIPLE_ID_LEFT    1
#define WAVESHARE_TRIPLE_ID_CENTER  2
#define WAVESHARE_TRIPLE_ID_RIGHT   3

 

 
/* ============================================================
 *  Driver context (one per sub-display)
 *  Holds any SPI / chip-select state needed by the HAL layer.
 *  Fill in the cs_pin / spi_host fields before calling
 *  ui_display_init().
 * ============================================================ */
typedef struct {
    uint8_t  panel_id;    /* mirrors the display id                    */
    int      cs_pin;      /* GPIO chip-select for this sub-display      */
    int      dc_pin;      /* data/command pin (shared across panels?)   */
    int      spi_host;    /* ESP-IDF spi_host_device_t cast to int      */
    /* add DMA handle, spi_device_handle_t, etc. as needed */
} waveshare_triple_ctx_t;
 
extern waveshare_triple_ctx_t wt_ctx_left;
extern waveshare_triple_ctx_t wt_ctx_center;
extern waveshare_triple_ctx_t wt_ctx_right;
 
/* ============================================================
 *  Driver function prototypes (registered as function pointers)
 * ============================================================ */
void waveshare_triple_init (ui_display_t *disp);
void waveshare_triple_draw (ui_display_t *disp,
                            uint16_t x, uint16_t y,
                            const ui_bitmap_t *bmp,
                            ui_color_t fg, ui_color_t bg);
void waveshare_triple_fill (ui_display_t *disp,
                            uint16_t x, uint16_t y,
                            uint16_t w, uint16_t h,
                            ui_color_t color);
void waveshare_triple_clear(ui_display_t *disp, ui_color_t color);

