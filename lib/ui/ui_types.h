#ifndef UI_TYPES_H
#define UI_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================================
 *  Timer / update priority tiers
 *  Matches the ISR priority bucketing from ui_timer_isr().
 *  Values are divisors – the ISR fires at UI_UPDATE_FAST rate,
 *  mid/all fire every N fast-ticks.
 * ============================================================ */
typedef enum {
    UI_UPDATE_FAST = 1,   /* every ISR tick  – e.g. 30 fps base */
    UI_UPDATE_MID  = 3,   /* every 3rd tick  – e.g. 10 fps      */
    UI_UPDATE_ALL  = 9,   /* every 9th tick  – e.g.  3 fps      */
} ui_update_priority_t;

/* Base ISR tick rate in Hz – tune to your timer config */
#define UI_TIMER_BASE_HZ   30u
 
/* ============================================================
 *  Bitmap
 * ============================================================ */
typedef enum {
    BMP_MONO_1BIT  = 0,   /* 1 bit per pixel, packed rows         */
    BMP_COLOR_RGB565 = 1, /* 16 bit per pixel, RGB565 little-endian */
} ui_bmp_type_t;

typedef struct {
    uint16_t      width;
    uint16_t      height;
    ui_bmp_type_t type;
    const uint8_t *data;  /* caller owns the memory               */
} ui_bitmap_t;

/* ============================================================
 *  Display
 * ============================================================ */
 
/* Every display colour constant lives in the display driver
 * header; the library only stores the raw uint32_t value.     */
typedef uint32_t ui_color_t;
 
/* Forward-declare display struct so function pointers can use it */
typedef struct ui_display_s ui_display_t;

/*
 * display_init_fn  – hardware bring-up; called once by ui_display_init().
 * display_draw_fn  – push a 1-bit or RGB565 bitmap region to the hardware.
 *                    x, y, w, h describe the destination rectangle.
 * display_fill_fn  – flood-fill a rectangle with a solid colour.
 * display_clear_fn – clear the whole framebuffer to bg_color.
 */
typedef void (*display_init_fn)(ui_display_t *disp);
typedef void (*display_draw_fn)(ui_display_t *disp,
                                uint16_t x, uint16_t y,
                                const ui_bitmap_t *bmp,
                                ui_color_t fg, ui_color_t bg);
typedef void (*display_fill_fn)(ui_display_t *disp,
                                uint16_t x, uint16_t y,
                                uint16_t w, uint16_t h,
                                ui_color_t color);
typedef void (*display_clear_fn)(ui_display_t *disp, ui_color_t color);
 
struct ui_display_s {
    uint8_t          id;          /* unique display ID                    */
    uint16_t         width;
    uint16_t         height;
    ui_color_t       default_fg;
    ui_color_t       default_bg;
    display_init_fn  init;
    display_draw_fn  draw;
    display_fill_fn  fill;
    display_clear_fn clear;
    void            *driver_ctx; /* opaque pointer to driver state        */
    bool             initialized;
};

/* ============================================================
 *  Animation callback – two flavours, union-tagged
 * ============================================================ */
typedef enum {
    ANIM_CB_HOOK     = 0, /* called every frame; do your own drawing  */
    ANIM_CB_SELECTOR = 1, /* returns child index to show this frame   */
} ui_anim_cb_type_t;
 
/* Forward-declare widget so the callbacks can reference it */
typedef struct ui_widget_s ui_widget_t;
 
/* Hook: called each frame tick; idx = current frame counter */
typedef void (*anim_hook_fn)(ui_widget_t *widget, uint32_t frame_idx);
 
/* Selector: returns which child widget index to render this frame */
typedef uint8_t (*anim_selector_fn)(ui_widget_t *widget, uint32_t frame_idx);
 
typedef struct {
    ui_anim_cb_type_t type;
    union {
        anim_hook_fn     hook;
        anim_selector_fn selector;
    } cb;
} ui_anim_cb_t;

/* ============================================================
 *  Bounce config  (for the DVD-style bouncing logo widget)
 * ============================================================ */
typedef struct {
    int16_t dx;          /* pixels per fast-tick in X              */
    int16_t dy;          /* pixels per fast-tick in Y              */
    /* runtime state – zero-init at registration */
    int16_t _vx;         /* current velocity (sign encodes direction) */
    int16_t _vy;
} ui_bounce_t;
 
/* ============================================================
 *  Widget
 * ============================================================ */
typedef enum {
    WIDGET_STATIC    = 0, /* plain bitmap, no motion                 */
    WIDGET_ANIMATED  = 1, /* cycles through child widgets            */
    WIDGET_BOUNCE    = 2, /* bounces around the display              */
} ui_widget_type_t;
 
struct ui_widget_s {
    /* --- identity & layout ---------------------------------- */
    uint8_t           idx;        /* 0 = not selectable; >0 = tab order */
    int16_t           x;
    int16_t           y;
    uint16_t          width;
    uint16_t          height;
 
    /* --- appearance ----------------------------------------- */
    ui_color_t        color;      /* foreground colour                  */
    ui_color_t        bg_color;   /* background colour                  */
    const ui_bitmap_t *bitmap;    /* NULL for container/animated widgets */
 
    /* --- selection state ------------------------------------ */
    bool              selected;
 
    /* --- type & behaviour ----------------------------------- */
    ui_widget_type_t  type;
 
    /* --- children (animation frames or sub-widgets) --------- */
    ui_widget_t      **children;  /* NULL-terminated array of pointers  */
    uint8_t           num_children;
 
    /* --- animation ------------------------------------------ */
    ui_anim_cb_t      anim_cb;    /* callback config (HOOK or SELECTOR) */
    uint8_t           fps;        /* desired frames per second          */
    uint32_t          duration;   /* frames total; 0 = infinite         */
 
    /* --- bounce --------------------------------------------- */
    ui_bounce_t       bounce;     /* only used when type == WIDGET_BOUNCE */
 
    /* --- runtime state (managed by ui_anim / ui_render) ----- */
    uint32_t          _frame;          /* current frame counter          */
    uint8_t           _active_child;   /* currently visible child index  */
    ui_update_priority_t _tick_tier;   /* resolved priority tier         */
    uint32_t          _tick_counter;   /* sub-frame accumulator          */
    ui_display_t     *_display;        /* back-pointer set at register   */
};
 
/* ============================================================
 *  Page
 * ============================================================ */
typedef struct {
    uint8_t       idx;           /* page index; must be unique, ≥ 1    */
    uint8_t       disp_id;       /* which physical display to render on */
    ui_color_t    bg_color;
    ui_widget_t **widgets;       /* NULL-terminated array of pointers   */
    uint8_t       num_widgets;
    ui_display_t *display;       /* resolved by ui_page_register()      */
} ui_page_t;
 
/* ============================================================
 *  Registry limits
 * ============================================================ */
#define UI_MAX_PAGES    8
#define UI_MAX_DISPLAYS 4


#endif // UI_TYPES_H