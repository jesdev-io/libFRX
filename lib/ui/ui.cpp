#include "ui.h"
#include "ui_render.h"
#include "ui_anim.h"
#include <string.h>
#include <stddef.h>

/* ============================================================
 *  Internal registries
 * ============================================================ */
static ui_display_t *s_displays[UI_MAX_DISPLAYS] = {0};
static uint8_t       s_display_count = 0;

static ui_page_t    *s_pages[UI_MAX_PAGES] = {0};
static uint8_t       s_page_count = 0;

/* ============================================================
 *  Helpers
 * ============================================================ */

/*
 * Map a requested fps to the coarsest priority tier that can
 * still satisfy it.  Better to fire slightly faster than miss
 * frames entirely.
 *
 * Tier    fires at     max fps
 * FAST    30 Hz        30
 * MID     10 Hz        10
 * ALL      3 Hz         3
 */
static ui_update_priority_t fps_to_tier(uint8_t fps)
{
    if (fps == 0) return UI_UPDATE_ALL;            /* static / manual   */
    uint32_t tier_fps_all  = UI_TIMER_BASE_HZ / UI_UPDATE_ALL;
    uint32_t tier_fps_mid  = UI_TIMER_BASE_HZ / UI_UPDATE_MID;
    if (fps <= tier_fps_all)  return UI_UPDATE_ALL;
    if (fps <= tier_fps_mid)  return UI_UPDATE_MID;
    return UI_UPDATE_FAST;
}

/* Propagate display and bg_color from a page down to a widget
 * and all its descendants.  Called recursively. */
static void propagate_display(ui_widget_t *w,
                               ui_display_t *disp,
                               ui_color_t    bg)
{
    w->_display  = disp;
    w->bg_color  = bg;

    /* Resolve tick tier from fps */
    w->_tick_tier = fps_to_tier(w->fps);

    /* Bounce: initialise velocity from the signed displacement fields */
    if (w->type == WIDGET_BOUNCE) {
        w->bounce._vx = (w->bounce.dx >= 0) ?  w->bounce.dx : -w->bounce.dx;
        w->bounce._vy = (w->bounce.dy >= 0) ?  w->bounce.dy : -w->bounce.dy;
    }

    for (uint8_t i = 0; i < w->num_children; i++) {
        if (w->children[i]) {
            propagate_display(w->children[i], disp, bg);
        }
    }
}

/* ============================================================
 *  Display registry
 * ============================================================ */
int ui_display_register(ui_display_t *disp)
{
    if (!disp || s_display_count >= UI_MAX_DISPLAYS) return -1;
    for (uint8_t i = 0; i < s_display_count; i++) {
        if (s_displays[i]->id == disp->id) return -1; /* duplicate */
    }
    s_displays[s_display_count++] = disp;
    return 0;
}

void ui_display_init(ui_display_t *disp)
{
    if (!disp || !disp->init) return;
    disp->init(disp);
    if (disp->clear) disp->clear(disp, disp->default_bg);
    disp->initialized = true;
}

ui_display_t *ui_display_get(uint8_t id)
{
    for (uint8_t i = 0; i < s_display_count; i++) {
        if (s_displays[i]->id == id) return s_displays[i];
    }
    return NULL;
}

/* ============================================================
 *  Page registry
 * ============================================================ */
int ui_page_register(ui_page_t *page)
{
    if (!page || s_page_count >= UI_MAX_PAGES) return -1;

    /* Resolve display pointer from id */
    ui_display_t *disp = ui_display_get(page->disp_id);
    if (!disp) return -1;
    page->display = disp;

    /* Propagate display and colors to every widget tree */
    for (uint8_t i = 0; i < page->num_widgets; i++) {
        if (page->widgets[i]) {
            propagate_display(page->widgets[i], disp, page->bg_color);
        }
    }

    s_pages[s_page_count++] = page;
    return 0;
}

ui_page_t *ui_page_get(uint8_t idx)
{
    for (uint8_t i = 0; i < s_page_count; i++) {
        if (s_pages[i]->idx == idx) return s_pages[i];
    }
    return NULL;
}

/* ============================================================
 *  Selection
 * ============================================================ */
void ui_page_select(ui_page_t *page, uint8_t sel_idx)
{
    if (!page) return;
    for (uint8_t i = 0; i < page->num_widgets; i++) {
        ui_widget_t *w = page->widgets[i];
        if (!w) continue;
        bool should_be = (sel_idx != 0) && (w->idx == sel_idx);
        if (w->selected != should_be) {
            w->selected = should_be;
            /* Redraw just this widget to reflect new selection state */
            ui_render_widget(w);
        }
    }
}

/* ============================================================
 *  Update pump
 * ============================================================ */
void ui_update(ui_update_priority_t priority)
{
    for (uint8_t p = 0; p < s_page_count; p++) {
        ui_page_t *page = s_pages[p];
        if (!page || !page->display || !page->display->initialized) continue;

        for (uint8_t w = 0; w < page->num_widgets; w++) {
            ui_widget_t *widget = page->widgets[w];
            if (!widget) continue;

            /* Only advance widgets whose tier is satisfied by this tick */
            if (widget->_tick_tier > priority) continue;

            ui_anim_tick(widget);
            ui_render_widget(widget);
        }
    }
}

/* ============================================================
 *  Full page redraw
 * ============================================================ */
void ui_page_draw(ui_page_t *page)
{
    if (!page || !page->display) return;
    if (page->display->clear) {
        page->display->clear(page->display, page->bg_color);
    }
    for (uint8_t i = 0; i < page->num_widgets; i++) {
        if (page->widgets[i]) {
            ui_render_widget(page->widgets[i]);
        }
    }
}