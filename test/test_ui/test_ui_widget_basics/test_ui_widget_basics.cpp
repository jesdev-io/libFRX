#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "demo_bmp.h"
#include "ui.h"
#include "waveshare_triple_tft.h"
#include "syserr.h"
#include <string.h>


static Adafruit_ST7735 waveshare_triple_right(WAVESHARE_TRIPLE_RIGHT_CS, WAVESHARE_TRIPLE_RIGHT_DC, WAVESHARE_TRIPLE_RIGHT_RST);
static Adafruit_ST7735 waveshare_triple_left(WAVESHARE_TRIPLE_LEFT_CS, WAVESHARE_TRIPLE_LEFT_DC, WAVESHARE_TRIPLE_LEFT_RST);
static Adafruit_ST7789 waveshare_triple_center(&hspi, WAVESHARE_TRIPLE_CENTER_CS, WAVESHARE_TRIPLE_CENTER_DC, WAVESHARE_TRIPLE_CENTER_RST);

void interface_init(ui_display_t *disp){
    waveshare_triple_center.init(WAVESHARE_TRIPLE_CENTER_WIDTH, WAVESHARE_TRIPLE_CENTER_HEIGHT);
    // What do we do with the function pointers?
}

void interface_draw(ui_display_t *disp, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const ui_bitmap_t *bmp, ui_color_t fg, ui_color_t bg){
    waveshare_triple_center.drawBitmap(x, y, (uint8_t *) bmp, w, h, fg);
}

void interface_clear(ui_display_t *disp, ui_color_t color){
    waveshare_triple_center.fillScreen(disp->default_bg);
}

ui_display_t display_inst = {
    .id = 0,
    .width = WAVESHARE_TRIPLE_CENTER_WIDTH,
    .height = WAVESHARE_TRIPLE_CENTER_HEIGHT,
    .default_fg = WAVESHARE_TRIPLE_COLOR_WHITE,
    .default_bg = WAVESHARE_TRIPLE_COLOR_BLACK,
    .init = interface_init,
    .draw = interface_draw
};

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

void test_draw_smiley(void){
    const char* name = "happy";
    uint16_t x = 10;
    uint16_t y = 10;
    uint16_t width = UI_BMP_W_VUM_HAPPY;
    uint16_t height = UI_BMP_H_VUM_HAPPY;
    uint16_t color = WAVESHARE_TRIPLE_COLOR_GREEN;
    ui_widget_t* widget = ui_widget_create(name, x, y, width, height, color, display_inst);
}

void test_ui_init(void){
    ui_display_register((ui_display_t*)&waveshare_triple_center);
    ui_display_register((ui_display_t*)&waveshare_triple_left);
    ui_display_register((ui_display_t*)&waveshare_triple_right);

    ui_display_init((ui_display_t*)&waveshare_triple_center);
    ui_display_init((ui_display_t*)&waveshare_triple_left);
    ui_display_init((ui_display_t*)&waveshare_triple_right);
}

// void test_demo(void){

//     ui_widget_t anim1 = {
//         .idx = 0, // for selection. is zero if cannot be selected
//         .x = 30,
//         .y = 30,
//         .width = UI_BMP_W_RECORDER1,
//         .height = UI_BMP_H_RECORDER1,
//         .color = WAVESHARE_TRIPLE_COLOR_CYAN,
//         .bg_color = WAVESHARE_TRIPLE_COLOR_BLACK,
//         .bitmap = (ui_bitmap_t*)ui_bmp_recorder1,
//         .selected = false,
//         .type = WIDGET_ANIMATED//,
//         // .children = NULL,
//         // .num_children = 0,
//         // .anim_cb = NULL,
//         // .fps = 3,
//         // .duration = 0,
//         // .bounce = bounce
//     };

//     ui_widget_t anim2 = {
//         .idx = 0, // for selection. is zero if cannot be selected
//         .x = 30,
//         .y = 30,
//         .width = UI_BMP_W_RECORDER2,
//         .height = UI_BMP_H_RECORDER2,
//         .color = WAVESHARE_TRIPLE_COLOR_MAGENTA,
//         .bg_color = WAVESHARE_TRIPLE_COLOR_BLACK,
//         .bitmap = (ui_bitmap_t*)ui_bmp_recorder2,
//         .selected = false,
//         .type = WIDGET_ANIMATED//,
//         // .children = NULL,
//         // .num_children = 0,
//         // .anim_cb = NULL,
//         // .fps = 3,
//         // .duration = 0,
//         // .bounce = bounce
//     } ;

//     ui_widget_t anim3 = {
//         .idx = 0, // for selection. is zero if cannot be selected
//         .x = 30,
//         .y = 30,
//         .width = UI_BMP_W_RECORDER3,
//         .height = UI_BMP_H_RECORDER3,
//         .color = WAVESHARE_TRIPLE_COLOR_YELLOW,
//         .bg_color = WAVESHARE_TRIPLE_COLOR_BLACK,
//         .bitmap = (ui_bitmap_t*)ui_bmp_recorder3,
//         .selected = false,
//         .type = WIDGET_ANIMATED//,
//         // .children = NULL,
//         // .num_children = 0,
//         // .anim_cb = NULL,
//         // .fps = 3,
//         // .duration = 0,
//         // .bounce = bounce
//     } ;

//     // ui_widget_t anim = {
//     // .children = [anim1, anim2, anim3],
//     // .num_children = 3,
//     // .anim_cb = NULL,// &demo_anim_cb, // this should be a proper function pointer reference
    
//     // .fps = 3,
//     // .duration = 0 // 0 for infinite
//     // }

//     // ui_widget_t *widgets_ptrs[] = {&anim1, &anim2, &anim3, NULL};

//     // ui_page_t page1 = {
//     // .id = 1,
//     // .widgets = widgets_ptrs,
//     // .widget_count = 3,
//     // .display = (ui_display_t*)&waveshare_triple_center // if sub-widgets (children) have different display, color or whatever parameter registered, the parent overwrites it
//     // };

//     // ui_page_register(page1); // links page to index, builds widgets and subwidgets on respective displays and such... if necessary I guess

//     // ui_display_init(waveshare_triple_center);
//     // ui_page_select(page1);

// }

void setup() {
    delay(2000);
    UNITY_BEGIN();

    // Test ui lib basic graphics functions
    RUN_TEST(test_ui_init);
    RUN_TEST(test_draw_smiley);


    UNITY_END();
}

void loop() {
}
