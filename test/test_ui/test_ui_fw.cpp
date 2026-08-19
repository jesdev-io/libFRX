#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "demo_bmp.h"
#include "waveshare_triple_tft.h"
#include "syserr.h"
#include <string.h>


// void test_draw_smiley(void){

//     ui_display_init(&fr2_ui_display_center);
//     ui_widget_t widget = {
//         .x = 30,
//         .y = 30,
//         .bitmap = (uint8_t*)ui_bmp_vum_happy,
//         .animation_cb = NULL
//     };

//     if (widget.bitmap) {
//         Adafruit_ST77xx* display = ui_display_get(widget->display);
//         if (display) {
//             display->drawBitmap(widget->x, widget->y,
//                               widget->bitmap, 
//                               widget->width, widget->height,
//                               widget->color);
//         }
//     }
// }

static inline void animate_tape_recorder(void* wgt){
    static uint8_t amination_step = 0;
    // other calls to ui_graphics
}

// void test_animate_tape_recorder(void){
//     ui_widget_t tape_rec = {
//         .x = 30,
//         .y = 30,
//         .width = 80,
//         .height = 80, 
//         .bitmap = NULL,
//         .color = ST77XX_WHITE,
//         .display = WAVESHARE_TRIPLE_CENTER,
//         .animation_cb = animate_tape_recorder
//     };
// }

void test_waveshare_triple_init(void){
    hspi.begin(WAVESHARE_TRIPLE_CENTER_SCLK, -1, WAVESHARE_TRIPLE_CENTER_MOSI, WAVESHARE_TRIPLE_CENTER_CS);

    waveshare_triple_left.initR(WAVESHARE_TRIPLE_SMALL_TAG);
    waveshare_triple_right.initR(WAVESHARE_TRIPLE_SMALL_TAG);
    waveshare_triple_center.init(WAVESHARE_TRIPLE_CENTER_WIDTH, WAVESHARE_TRIPLE_CENTER_HEIGHT);

    waveshare_triple_center.setRotation(2);
    waveshare_triple_left.setRotation(0);
    waveshare_triple_right.setRotation(0);
    
    waveshare_triple_center.setTextWrap(false);
    waveshare_triple_left.setTextWrap(false);
    waveshare_triple_right.setTextWrap(false);

    waveshare_triple_center.setTextSize(3*WAVESHARE_TRIPLE_CENTER_RES);
    waveshare_triple_left.setTextSize(3*WAVESHARE_TRIPLE_SMALL_RES);
    waveshare_triple_right.setTextSize(3*WAVESHARE_TRIPLE_SMALL_RES);
    
}

void test_waveshare_triple_simple_demo(void){
    uint16_t del = 1000;
    waveshare_triple_center.fillScreen(WAVESHARE_TRIPLE_COLOR_MAGENTA);
    waveshare_triple_left.fillScreen(WAVESHARE_TRIPLE_COLOR_MAGENTA);
    waveshare_triple_right.fillScreen(WAVESHARE_TRIPLE_COLOR_MAGENTA);
    waveshare_triple_center.drawBitmap(0, 0, ui_bmp_vum_sleepy, UI_BMP_W_VUM_SLEEPY*WAVESHARE_TRIPLE_CENTER_RES, UI_BMP_H_VUM_SLEEPY*WAVESHARE_TRIPLE_CENTER_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    waveshare_triple_left.drawBitmap(0, 0, ui_bmp_vum_sleepy, UI_BMP_W_VUM_SLEEPY*WAVESHARE_TRIPLE_SMALL_RES, UI_BMP_H_VUM_SLEEPY*WAVESHARE_TRIPLE_SMALL_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    waveshare_triple_right.drawBitmap(0, 0, ui_bmp_vum_sleepy, UI_BMP_W_VUM_SLEEPY*WAVESHARE_TRIPLE_SMALL_RES, UI_BMP_H_VUM_SLEEPY*WAVESHARE_TRIPLE_SMALL_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    delay(del);

    waveshare_triple_center.fillScreen(WAVESHARE_TRIPLE_COLOR_GREEN);
    waveshare_triple_left.fillScreen(WAVESHARE_TRIPLE_COLOR_GREEN);
    waveshare_triple_right.fillScreen(WAVESHARE_TRIPLE_COLOR_GREEN);
    waveshare_triple_center.drawBitmap(0, 0, ui_bmp_vum_happy, UI_BMP_W_VUM_HAPPY*WAVESHARE_TRIPLE_CENTER_RES, UI_BMP_H_VUM_HAPPY*WAVESHARE_TRIPLE_CENTER_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    waveshare_triple_left.drawBitmap(0, 0, ui_bmp_vum_happy, UI_BMP_W_VUM_HAPPY*WAVESHARE_TRIPLE_SMALL_RES, UI_BMP_H_VUM_HAPPY*WAVESHARE_TRIPLE_SMALL_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    waveshare_triple_right.drawBitmap(0, 0, ui_bmp_vum_happy, UI_BMP_W_VUM_HAPPY*WAVESHARE_TRIPLE_SMALL_RES, UI_BMP_H_VUM_HAPPY*WAVESHARE_TRIPLE_SMALL_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    delay(del);

    waveshare_triple_center.fillScreen(WAVESHARE_TRIPLE_COLOR_BLUE);
    waveshare_triple_left.fillScreen(WAVESHARE_TRIPLE_COLOR_BLUE);
    waveshare_triple_right.fillScreen(WAVESHARE_TRIPLE_COLOR_BLUE);
    waveshare_triple_center.drawBitmap(0, 0, ui_bmp_vum_dead, UI_BMP_W_VUM_DEAD*WAVESHARE_TRIPLE_CENTER_RES, UI_BMP_H_VUM_DEAD*WAVESHARE_TRIPLE_CENTER_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    waveshare_triple_left.drawBitmap(0, 0, ui_bmp_vum_dead, UI_BMP_W_VUM_DEAD*WAVESHARE_TRIPLE_SMALL_RES, UI_BMP_H_VUM_DEAD*WAVESHARE_TRIPLE_CENTER_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    waveshare_triple_right.drawBitmap(0, 0, ui_bmp_vum_dead, UI_BMP_W_VUM_DEAD*WAVESHARE_TRIPLE_SMALL_RES, UI_BMP_H_VUM_DEAD*WAVESHARE_TRIPLE_CENTER_RES, WAVESHARE_TRIPLE_COLOR_WHITE);
    delay(del);

    // waveshare_triple_center.fillScreen(WAVESHARE_TRIPLE_COLOR_BLACK);
    // waveshare_triple_left.fillScreen(WAVESHARE_TRIPLE_COLOR_BLACK);
    // waveshare_triple_right.fillScreen(WAVESHARE_TRIPLE_COLOR_BLACK);

    String text_night = "Night";
    String text_good = "Good";
    String text_heart = "<3";
    waveshare_triple_center.print(text_night);
    waveshare_triple_left.print(text_good);
    waveshare_triple_right.print(text_heart);
    delay(del);
}

void test_text_printing(void){
    uint16_t del = 500;
    String text_night = "Night";
    String text_good = "Good";
    String text_heart = "<3";
    waveshare_triple_center.print(text_night);
    waveshare_triple_left.print(text_good);
    waveshare_triple_right.print(text_heart);
    delay(del);
}

void test_Adafruit_ST7735(void){
    SPIClass *hspi = new SPIClass(HSPI);
    Adafruit_ST7735 tft_l = Adafruit_ST7735(hspi, WAVESHARE_TRIPLE_LEFT_CS, WAVESHARE_TRIPLE_LEFT_DC, WAVESHARE_TRIPLE_LEFT_RST);
    hspi->begin(WAVESHARE_TRIPLE_SMALL_SCLK, -1, WAVESHARE_TRIPLE_SMALL_MOSI, WAVESHARE_TRIPLE_LEFT_CS);
    tft_l.initR(WAVESHARE_TRIPLE_SMALL_TAG);
    tft_l.setRotation(1);
    tft_l.fillScreen(WAVESHARE_TRIPLE_COLOR_MAGENTA);
    tft_l.drawBitmap(0, 0, ui_bmp_vum_sleepy, UI_BMP_W_VUM_SLEEPY, UI_BMP_H_VUM_SLEEPY, WAVESHARE_TRIPLE_COLOR_WHITE);
    delay(2000);
    tft_l.fillScreen(WAVESHARE_TRIPLE_COLOR_GREEN);
    tft_l.drawBitmap(0, 0, ui_bmp_vum_happy, UI_BMP_W_VUM_HAPPY, UI_BMP_H_VUM_HAPPY, WAVESHARE_TRIPLE_COLOR_WHITE);
    delay(2000);
    tft_l.fillScreen(WAVESHARE_TRIPLE_COLOR_BLUE);
    tft_l.drawBitmap(0, 0, ui_bmp_vum_dead, UI_BMP_W_VUM_DEAD, UI_BMP_H_VUM_DEAD, WAVESHARE_TRIPLE_COLOR_WHITE);
    delay(2000);
    tft_l.fillScreen(WAVESHARE_TRIPLE_COLOR_BLACK);
    delay(2000);
}

void test_display(void){
// general display testing, different colors and stuff, just to see if it works
}

// void test_demo(void){
//     ui_widget_t anim1 = {
//         .idx = 0, // for selection. is zero if cannot be selected
//         .x = 30,
//         .y = 30,
//         .height = ui_bmp_demo_anim1.height,
//         .width = ui_bmp_demo_anim1.width,
//         .color = WAVESHARE_TRIPLE_COLOR_WHITE,
//         .bitmap = (uint8_t*)bmp_demo_anim1.bmp
//     } 

//     ui_widget_t anim2 = {
//     .idx = 0,
//     .x = 30,
//     .y = 30, 
//     .height = ui_bmp_demo_anim2.height,
//     .width = ui_bmp_demo_anim2.width,
//     .color = WAVESHARE_TRIPLE_COLOR_WHITE,
//     .bitmap = (uint8_t*)bmp_demo_anim2.bmp
//     }

//     ui_widget_t anim3 = {
//     .idx = 0,
//     .x = 30,
//     .y = 30, 
//     .height = ui_bmp_demo_anim3.height,
//     .width = ui_bmp_demo_anim3.width,
//     .color = WAVESHARE_TRIPLE_COLOR_WHITE,
//     .bitmap = (uint8_t*)bmp_demo_anim3.bmp
//     }

//     ui_widget_t anim = {
//     .num_children = 3,
//     .animation_cb = &demo_anim_cb, // this should be a proper function pointer reference
//     .widgets = [anim1, anim2, anim3],
//     .fps = 3,
//     .duration = 0 // 0 for infinite
//     }

//     ui_page_t page1 = {
//     .idx = 1,
//     .disp_id = 1,
//     .bg_color = WAVESHARE_TRIPLE_COLOR_BLACK,
//     .num_widgets = 1,
//     .widgets = [anim],
//     .display = waveshare_triple_center // if sub-widgets (children) have different display, color or whatever parameter registered, the parent overwrites it
//     }

//     ui_page_register(page1); // links page to index, builds widgets and subwidgets on respective displays and such... if necessary I guess

//     // ui_display_init(waveshare_triple_center);
//     // ui_page_select(page1);
// }

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_waveshare_triple_init);
    
    for(int i = 0; i<1; i++){
        RUN_TEST(test_waveshare_triple_simple_demo);
    }
    UNITY_END();
}

void loop() {
}
