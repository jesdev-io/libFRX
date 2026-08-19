#include "ui_render.h"
#include <stddef.h>

/* ============================================================
 *  Internal helpers
 * ============================================================ */

/* Clamp a position so the widget stays inside the display */
static inline int16_t clamp_x(ui_display_t *d, int16_t x, uint16_t w)
{
    if (x < 0) return 0;
    if ((uint16_t)(x + w) > d->width) return (int16_t)(d->width - w);
    return x;
}
static inline int16_t clamp_y(ui_display_t *d, int16_t y, uint16_t h)
{
    if (y < 0) return 0;
    if ((uint16_t)(y + h) > d->height) return (int16_t)(d->height - h);
    return y;
}

/* ============================================================
 *  Bitmap renderers
 * ============================================================ */
void ui_render_bitmap_mono(ui_display_t *disp,
                           int16_t x, int16_t y,
                           const ui_bitmap_t *bmp,
                           ui_color_t fg, ui_color_t bg)
{
    if (!disp || !bmp || !bmp->data || !disp->draw) return;
    /*
     * We delegate to the display driver's draw function which must
     * understand BMP_MONO_1BIT.  The driver handles the bit-unpacking
     * for its specific panel (e.g. SSD1306 page-addressed vs row-major).
     */
    disp->draw(disp, (uint16_t)x, (uint16_t)y, bmp, fg, bg);
}

void ui_render_bitmap_color(ui_display_t *disp,
                            int16_t x, int16_t y,
                            const ui_bitmap_t *bmp)
{
    if (!disp || !bmp || !bmp->data || !disp->draw) return;
    /* fg/bg ignored by driver for color bitmaps; pass 0 */
    disp->draw(disp, (uint16_t)x, (uint16_t)y, bmp, 0, 0);
}

void ui_render_border(ui_display_t *disp,
                      int16_t x, int16_t y,
                      uint16_t w, uint16_t h,
                      ui_color_t color)
{
    if (!disp || !disp->fill) return;
    uint16_t t = 1; /* border thickness in pixels */
    /* top */
    disp->fill(disp, (uint16_t)x, (uint16_t)y, w, t, color);
    /* bottom */
    disp->fill(disp, (uint16_t)x, (uint16_t)(y + h - t), w, t, color);
    /* left */
    disp->fill(disp, (uint16_t)x, (uint16_t)y, t, h, color);
    /* right */
    disp->fill(disp, (uint16_t)(x + w - t), (uint16_t)y, t, h, color);
}

void ui_render_invert_mono(ui_display_t *disp,
                           int16_t x, int16_t y,
                           const ui_bitmap_t *bmp,
                           ui_color_t fg, ui_color_t bg)
{
    /* Redraw with fg and bg swapped */
    ui_render_bitmap_mono(disp, x, y, bmp, bg, fg);
}

/* ============================================================
 *  Resolve which bitmap to draw for a widget
 * ============================================================ */
static const ui_bitmap_t *resolve_bitmap(ui_widget_t *w)
{
    if (w->type == WIDGET_STATIC || w->type == WIDGET_BOUNCE) {
        return w->bitmap;
    }
    /* WIDGET_ANIMATED – use the active child's bitmap */
    if (w->num_children == 0 || !w->children) return NULL;
    uint8_t cidx = w->_active_child;
    if (cidx >= w->num_children)   return NULL;
    if (!w->children[cidx])        return NULL;
    return w->children[cidx]->bitmap;
}

/* ============================================================
 *  Core render dispatch
 * ============================================================ */
void ui_render_widget(ui_widget_t *widget)
{
    if (!widget || !widget->_display) return;

    ui_display_t     *disp = widget->_display;
    const ui_bitmap_t *bmp = resolve_bitmap(widget);
    int16_t           x    = widget->x;
    int16_t           y    = widget->y;
    uint16_t          w    = widget->width;
    uint16_t          h    = widget->height;

    /* Clamp to display bounds */
    x = clamp_x(disp, x, w);
    y = clamp_y(disp, y, h);

    if (!bmp) {
        /*
         * Container widget with an ANIM_CB_HOOK callback –
         * the hook is responsible for its own drawing; we
         * just call it here so the render path is uniform.
         */
        if (widget->type == WIDGET_ANIMATED &&
            widget->anim_cb.type == ANIM_CB_HOOK &&
            widget->anim_cb.cb.hook) {
            widget->anim_cb.cb.hook(widget, widget->_frame);
        }
        return;
    }

    /* ---- Draw the bitmap ----------------------------------- */
    if (bmp->type == BMP_MONO_1BIT) {
        if (widget->selected) {
            ui_render_invert_mono(disp, x, y, bmp,
                                  widget->color, widget->bg_color);
        } else {
            ui_render_bitmap_mono(disp, x, y, bmp,
                                  widget->color, widget->bg_color);
        }
    } else {
        /* RGB565 */
        ui_render_bitmap_color(disp, x, y, bmp);
        if (widget->selected) {
            ui_render_border(disp, x, y, w, h, widget->color);
        }
    }
}