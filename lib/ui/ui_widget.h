#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include "ui_types.h"
#include "ui_display.h"
#include "syserr.h"
#include "ui_timer.h"
#include <string.h>
#include <stdlib.h>

// Constants
#define UI_WIDGET_MAX_NAME_LEN   32
#define UI_WIDGET_MAX_CHILDREN   8
#define UI_WIDGET_INVALID_ID     0xFF

// Callback types
typedef void (*ui_widget_update_cb_t)(ui_widget_t* widget, void* user_data);
typedef void (*ui_widget_draw_cb_t)(const ui_widget_t* widget);
typedef void (*ui_widget_cleanup_cb_t)(ui_widget_t* widget);

// Widget structure
struct ui_widget_s {
    // Basic properties
    char name[UI_WIDGET_MAX_NAME_LEN];
    uint16_t id;
    
    
    // Geometry
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    
    // Visual properties
    const ui_bitmap_t *bitmap;
    uint16_t color;
    ui_display_t display;
    
    // State
    bool visible;
    bool enabled;
    bool selectable;
    bool selected;
    bool dynamic;
    float value;
    
    // Hierarchy
    ui_widget_t* p_parent;
    uint8_t child_count;
    ui_widget_t* p_children[UI_WIDGET_MAX_CHILDREN];
    
    // Callbacks
    ui_widget_update_cb_t fp_update_callback;
    ui_widget_draw_cb_t fp_draw_callback;
    ui_widget_cleanup_cb_t fp_cleanup_callback;
    void* p_user_data;
};

struct ui_widget_old {
    /* --- identity & layout ---------------------------------- */
    char name[UI_WIDGET_MAX_NAME_LEN];
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
 
    /* --- runtime state (managed by ui_anim / ui_render) ----- */
    uint32_t          _frame;          /* current frame counter          */
    uint8_t           _active_child;   /* currently visible child index  */
    ui_update_priority_t _tick_tier;   /* resolved priority tier         */
    uint32_t          _tick_counter;   /* sub-frame accumulator          */
    ui_display_t     *_display;        /* back-pointer set at register   */
};

// Widget system management

/// @brief Initialize the widget system
/// @param max_widgets Maximum number of widgets to allocate
/// @return e_syserr_none on success, error code otherwise
e_syserr_t ui_widget_system_init(uint16_t max_widgets);

/// @brief Clean up the widget system and free all resources
void ui_widget_system_cleanup(void);

// Widget creation/destruction
ui_widget_t* ui_widget_create(const char* name, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, ui_display_t display);
void ui_widget_destroy(ui_widget_t* widget);

// Widget hierarchy
e_syserr_t ui_widget_add_child(ui_widget_t* parent, ui_widget_t* child);
e_syserr_t ui_widget_remove_child(ui_widget_t* parent, ui_widget_t* child);

// Widget operations
void ui_widget_draw(const ui_widget_t* widget);
void ui_widget_update(ui_widget_t* widget);
void ui_widget_set_position(ui_widget_t* widget, uint16_t x, uint16_t y);
void ui_widget_set_size(ui_widget_t* widget, uint16_t width, uint16_t height);
void ui_widget_set_visible(ui_widget_t* widget, bool visible);
void ui_widget_set_enabled(ui_widget_t* widget, bool enabled);

// Widget utilities
ui_widget_t* ui_widget_find_by_name(ui_widget_t* root, const char* name);
ui_widget_t* ui_widget_find_by_id(ui_widget_t* root, uint16_t id);


// Specific widget types (declarations)
extern ui_widget_t widget_battery;
extern ui_widget_t widget_sd_card;
extern ui_widget_t widget_clock;
extern ui_widget_t widget_vu_meter;
extern ui_widget_t ui_widget_vum_sleepy;


#endif // UI_WIDGET_H