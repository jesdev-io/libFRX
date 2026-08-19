#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "demo_bmp.h"
#include "waveshare_triple_tft.h"
#include "syserr.h"
#include <string.h>

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

typedef struct ui_display_t ui_display_t;
typedef uint32_t ui_color_t;

typedef void (*ui_display_init_cb)(ui_display_t* self, uint8_t width, 
    uint8_t height);
typedef void (*ui_display_draw_bmp_callback)(ui_display_t* self, 
    ui_bitmap_t* bmp, uint8_t x, uint8_t y, uint8_t w, uint8_t h, 
    ui_color_t col);
typedef void (*ui_display_print_text_callback)(ui_display_t* self, 
    String text, uint8_t x, uint8_t y, uint8_t size, ui_color_t col);
typedef void (*ui_display_fill_callback)(ui_display_t* self, ui_color_t col);

struct ui_display_t {    
    uint8_t id;    
    uint8_t pin1, pin2, pin3, pin4;
    uint8_t width;    
    uint8_t height;
    uint8_t rotation;    
    bool set_text_wrap;    
    uint8_t text_size;
    // callbacks    
    ui_display_init_cb fp_init_callback;    
    ui_display_draw_bmp_callback fp_draw_bmp_callback;    
    ui_display_print_text_callback fp_print_text_callback;    
    ui_display_fill_callback fp_fill_callback;
    // optional: extra user payload separate from struct fields    
    void* user_data;
};

e_syserr_t ui_display_init(ui_display_t* d, uint8_t id, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t width, uint8_t height, uint8_t rotation, bool set_text_wrap, uint8_t text_size,                      ui_display_init_cb init_cb,                      ui_display_draw_bmp_callback draw_cb,                      ui_display_print_text_callback print_cb, ui_display_fill_callback fill_cb, void* user_data){    // require callbacks    
    if (!d || !init_cb || !draw_cb || !print_cb || !fill_cb) {
        return e_syserr_param;
    }

    d->id = id;    
    d->pin1 = pin1; 
    d->pin2 = pin2; 
    d->pin3 = pin3; 
    d->pin4 = pin4;    
    d->width = width; 
    d->height = height;    
    d->rotation = rotation;    
    d->set_text_wrap = set_text_wrap;    
    d->text_size = text_size;
    d->fp_init_callback = init_cb;    
    d->fp_draw_bmp_callback = draw_cb;    
    d->fp_print_text_callback = print_cb;    
    d->fp_fill_callback = fill_cb;
    d->user_data = user_data;    
    return e_syserr_none;
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

    waveshare_triple_center.fillScreen(WAVESHARE_TRIPLE_COLOR_BLACK);
    waveshare_triple_left.fillScreen(WAVESHARE_TRIPLE_COLOR_BLACK);
    waveshare_triple_right.fillScreen(WAVESHARE_TRIPLE_COLOR_BLACK);

    String text_night = "Night";
    String text_good = "Good";
    String text_heart = "<3";
    waveshare_triple_center.print(text_night));
    waveshare_triple_left.print(text_good);
    waveshare_triple_right.print(text_heart);
    delay(del);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    // Test hardware without any ui lib functions
    RUN_TEST(test_waveshare_triple_init);
    
    for(int i = 0; i<10; i++){
        RUN_TEST(test_waveshare_triple_simple_demo);
    }

    UNITY_END();
}

void loop() {
}
