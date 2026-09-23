#include "ui_widget.h"

// Widget pool management
static ui_widget_t* widget_pool = NULL;
static uint16_t widget_count = 0;
static uint16_t widget_capacity = 0;

// Init widget system
e_syserr_t ui_widget_system_init(uint16_t max_widgets){
    if(max_widgets == 0){
        return e_syserr_param;
    }

    widget_pool = (ui_widget_t*)calloc(max_widgets, sizeof(ui_widget_t));
    if (!widget_pool){
        return e_syserr_oom;
    }

    widget_capacity = max_widgets;
    widget_count = 0;

    return e_syserr_none;
}

// Cleanup widget system
void ui_widget_system_cleanup(void) {
    for (uint16_t i = 0; i < widget_count; i++) {
        ui_widget_t* widget = &widget_pool[i];
        
        // Clear widget data
        memset(widget, 0, sizeof(ui_widget_t));
    }
    
    free(widget_pool);
    widget_pool = NULL;
    widget_count = 0;
    widget_capacity = 0;
}

ui_widget_t* ui_widget_create(const char* name, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color, ui_display_t display) {
    if (!name || width == 0 || height == 0) {
        return NULL;
    }
    
    if (widget_count >= widget_capacity) {
        return NULL;
    }
    
    ui_widget_t* widget = &widget_pool[widget_count];
    
    // Initialize widget
    memset(widget, 0, sizeof(ui_widget_t));
    widget->id = widget_count;
    strncpy(widget->name, name, UI_WIDGET_MAX_NAME_LEN - 1);
    widget->name[UI_WIDGET_MAX_NAME_LEN - 1] = '\0';
    
    widget->x = x;
    widget->y = y;
    widget->width = width;
    widget->height = height;
    
    widget->visible = true;
    widget->enabled = true;
    widget->color = color;
    widget->display = display;
    
    widget_count++;
    
    return widget;
}

void ui_widget_destroy(ui_widget_t* widget) {
    if (!widget) {
        return;
    }
    
    // Remove from parent if any
    if (widget->p_parent) {
        ui_widget_remove_child(widget->p_parent, widget);
    }
    
    // Destroy children recursively
    for (uint8_t i = 0; i < widget->child_count; i++) {
        ui_widget_destroy(widget->p_children[i]);
    }
    
    // Call cleanup callback
    if (widget->fp_cleanup_callback) {
        widget->fp_cleanup_callback(widget);
    }
    
    // Mark as invalid
    widget->id = UI_WIDGET_INVALID_ID;
}

e_syserr_t ui_widget_add_child(ui_widget_t* parent, ui_widget_t* child){
    if (!parent || !child){
        return e_syserr_param;
    }

    if (parent->child_count >= UI_WIDGET_MAX_CHILDREN) {
        return e_syserr_oom;
    }

    // Check if already a child
    for (uint8_t i = 0; i < parent->child_count; i++) {
        if (parent->p_children[i] == child) {
            return e_syserr_none;  // Already a child
        }
    }

    // Remove from previous parent
    if (child->p_parent) {
        ui_widget_remove_child(child->p_parent, child);
    }

    // Add to new parent
    parent->p_children[parent->child_count] = child;
    parent->child_count++;
    child->p_parent = parent;
    
    return e_syserr_none;
}

e_syserr_t ui_widget_remove_child(ui_widget_t* parent, ui_widget_t* child){
    if (!parent || !child) {
        return e_syserr_param;
    }
    // Find and remove child
    for (uint8_t i = 0; i < parent->child_count; i++) {
        if (parent->p_children[i] == child) {
            // Shift remaining children
            for (uint8_t j = i; j < parent->child_count - 1; j++) {
                parent->p_children[j] = parent->p_children[j + 1];
            }
            
            parent->child_count--;
            child->p_parent = NULL;
            return e_syserr_none;
        }
    }
    
    return e_syserr_param;
}

void ui_widget_draw(const ui_widget_t* widget, ui_display_t* display) {
    if (!widget || !widget->visible) {
        return;
    }
    
    // Draw widget using bitmap if available
    if (widget->bitmap) {
        display->draw(display, widget->x, widget->y, widget->width, widget->height, widget->bitmap, widget->color, display->default_bg);
    }
    
    // Call custom draw callback if set
    if (widget->fp_draw_callback) {
        widget->fp_draw_callback(widget);
    }
    
    // Draw children recursively
    for (uint8_t i = 0; i < widget->child_count; i++) {
        ui_widget_draw(widget->p_children[i]);
    }
}

void ui_widget_update(ui_widget_t* widget) {
    if (!widget || !widget->enabled) {
        return;
    }
    
    // Update children first
    for (uint8_t i = 0; i < widget->child_count; i++) {
        ui_widget_update(widget->p_children[i]);
    }
    
    // Call update callback if set
    if (widget->fp_update_callback) {
        widget->fp_update_callback(widget, widget->p_user_data);
    }
}

void ui_widget_set_position(ui_widget_t* widget, uint16_t x, uint16_t y){
    //tbd
}

void ui_widget_set_size(ui_widget_t* widget, uint16_t width, uint16_t height){
    //tbd
}

void ui_widget_set_visible(ui_widget_t* widget, bool visible){
    //tbd
}

void ui_widget_set_enabled(ui_widget_t* widget, bool enabled){
    //tbd
}

void ui_widget_widget_invert_bitmap_in_ram(uint8_t* bitmap, uint16_t w, uint16_t h) {
    uint16_t bytes = (w / 8) * h;
    for (uint16_t i = 0; i < bytes; i++) {
        bitmap[i] = ~bitmap[i];
    }
}