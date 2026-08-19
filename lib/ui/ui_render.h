#include "ui_types.h"

/* ============================================================
 *  Render a single widget (and its active child if animated).
 *
 *  Dispatches based on widget->type:
 *    WIDGET_STATIC   – draw bitmap at (x, y)
 *    WIDGET_ANIMATED – draw the _active_child's bitmap
 *    WIDGET_BOUNCE   – draw bitmap at current bounced position
 *
 *  If widget->selected is true, overlays a selection indicator
 *  (invert for mono bitmaps, border box for color bitmaps).
 * ============================================================ */
void ui_render_widget(ui_widget_t *widget);

/* ============================================================
 *  Draw a 1-bit bitmap onto a display.
 *  set pixels where bit == 1 in fg_color, else bg_color.
 * ============================================================ */
void ui_render_bitmap_mono(ui_display_t *disp,
                           int16_t x, int16_t y,
                           const ui_bitmap_t *bmp,
                           ui_color_t fg, ui_color_t bg);

/* ============================================================
 *  Draw an RGB565 bitmap onto a display.
 * ============================================================ */
void ui_render_bitmap_color(ui_display_t *disp,
                            int16_t x, int16_t y,
                            const ui_bitmap_t *bmp);

/* ============================================================
 *  Draw a hollow rectangle border (selection indicator for
 *  color bitmaps).
 * ============================================================ */
void ui_render_border(ui_display_t *disp,
                      int16_t x, int16_t y,
                      uint16_t w, uint16_t h,
                      ui_color_t color);

/* ============================================================
 *  Invert a 1-bit bitmap region on-display (selection
 *  indicator for mono bitmaps).
 *  Swaps fg and bg colours for the widget's bounding box.
 * ============================================================ */
void ui_render_invert_mono(ui_display_t *disp,
                           int16_t x, int16_t y,
                           const ui_bitmap_t *bmp,
                           ui_color_t fg, ui_color_t bg);