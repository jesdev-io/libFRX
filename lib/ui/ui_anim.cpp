#include "ui_anim.h"
#include <stddef.h>

/* ============================================================
 *  Animated widget tick
 * ============================================================ */
static void tick_animated(ui_widget_t *w)
{
    /* Respect finite duration */
    if (w->duration > 0 && w->_frame >= w->duration) return;

    w->_frame++;

    if (w->num_children == 0) return;

    if (w->anim_cb.type == ANIM_CB_SELECTOR &&
        w->anim_cb.cb.selector != NULL) {
        /* Delegate child selection to the user callback */
        uint8_t child = w->anim_cb.cb.selector(w, w->_frame);
        w->_active_child = (child < w->num_children) ? child : 0;
    } else {
        /*
         * Default round-robin: advance one child per frame.
         * Also used when type == ANIM_CB_HOOK (hook fires at render
         * time; we still need to update _active_child for any
         * fallback render path).
         */
        w->_active_child = (uint8_t)(w->_frame % w->num_children);
    }
}

/* ============================================================
 *  Bounce widget tick
 * ============================================================ */
static void tick_bounce(ui_widget_t *w)
{
    if (!w->_display) return;

    uint16_t disp_w = w->_display->width;
    uint16_t disp_h = w->_display->height;

    /* Move */
    w->x += w->bounce._vx;
    w->y += w->bounce._vy;

    /* Reflect on X edges */
    if (w->x <= 0) {
        w->x           = 0;
        w->bounce._vx  = (int16_t)( w->bounce.dx < 0
                                    ? -w->bounce.dx
                                    :  w->bounce.dx );
    } else if ((uint16_t)(w->x + w->width) >= disp_w) {
        w->x           = (int16_t)(disp_w - w->width);
        w->bounce._vx  = (int16_t)( w->bounce.dx < 0
                                    ?  w->bounce.dx
                                    : -w->bounce.dx );
    }

    /* Reflect on Y edges */
    if (w->y <= 0) {
        w->y           = 0;
        w->bounce._vy  = (int16_t)( w->bounce.dy < 0
                                    ? -w->bounce.dy
                                    :  w->bounce.dy );
    } else if ((uint16_t)(w->y + w->height) >= disp_h) {
        w->y           = (int16_t)(disp_h - w->height);
        w->bounce._vy  = (int16_t)( w->bounce.dy < 0
                                    ?  w->bounce.dy
                                    : -w->bounce.dy );
    }
}

/* ============================================================
 *  Public entry point
 * ============================================================ */
void ui_anim_tick(ui_widget_t *widget)
{
    if (!widget) return;

    switch (widget->type) {
        case WIDGET_ANIMATED: tick_animated(widget); break;
        case WIDGET_BOUNCE:   tick_bounce(widget);   break;
        case WIDGET_STATIC:   /* intentional no-op */break;
        default:              break;
    }
}