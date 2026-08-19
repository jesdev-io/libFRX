// #include "ui_display.h"

// e_syserr_t ui_display_init_all(uint32_t display_count){
//     e_syserr_t error = e_syserr_none;

//     for(int display = 0; display < display_count; display++) {
//         error = ui_display_init((ui_display_t)display);
//         if (error != e_syserr_none) {
//             return error;
//         }
//     }
//     return e_syserr_none;
// }

// e_syserr_t ui_display_init(ui_display_t display){
//     if(display>=UI_DISPLAY_COUNT){
//         return e_syserr_param;
//     }
//     switch(display){
//         case UI_DISPLAY_CENTER: {
//             Adafruit_ST7789* p_display = (Adafruit_ST7789*)displays[display];

//             // Init SPI for center display
//             hspi.begin(UI_DISPLAY_CENTER_SCLK, -1, UI_DISPLAY_CENTER_MOSI, UI_DISPLAY_CENTER_CS);

//             // Init display
//             p_display->init(UI_DISPLAY_CENTER_WIDTH, UI_DISPLAY_CENTER_HEIGHT);
//             p_display->setRotation(2); // fixed rotation for center display
//             p_display->fillScreen(display_bg_colors[display]);
//             break;
//         }

//         case UI_DISPLAY_RIGHT:
//         case UI_DISPLAY_LEFT: {
//             Adafruit_ST7735* p_display = (Adafruit_ST7735*)displays[display];
            
//             // Init small displays
//             p_display->initR(INITR_MINI160x80);
//             p_display->setRotation(0);  // Fixed rotation for small displays
//             p_display->fillScreen(display_bg_colors[display]);
//             break;
//         }

//         default:
//             return e_syserr_param;
//     }

//     return e_syserr_none;
// }

// Adafruit_ST77xx* ui_display_get(ui_display_t display) {
//     if (display >= UI_DISPLAY_COUNT) {
//         return NULL;
//     }
//     return displays[display];
// }

// void ui_display_clear(ui_display_t display) {
//     Adafruit_ST77xx* p_display = ui_display_get(display);
//     if (p_display) {
//         p_display->fillScreen(display_bg_colors[display]);
//     }
// }

// void ui_display_get_dimensions(ui_display_t display, uint16_t* width, uint16_t* height) {
//     if (display >= UI_DISPLAY_COUNT) {
//         if (width) *width = 0;
//         if (height) *height = 0;
//         return;
//     }
    
//     if (width) *width = display_widths[display];
//     if (height) *height = display_heights[display];
// }