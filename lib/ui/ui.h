#include "ui_types.h"

/* ============================================================
 *  Display registry
 * ============================================================ */

/**
 * Register a display driver with the library.
 * Must be called before ui_display_init() or ui_page_register().
 * Returns 0 on success, -1 if registry is full or id is duplicate.
 */
int ui_display_register(ui_display_t *disp);

/**
 * Initialise a previously registered display.
 * Calls disp->init(disp) and clears it to its default background colour.
 */
void ui_display_init(ui_display_t *disp);

/**
 * Retrieve a registered display by id.
 * Returns NULL if not found.
 */
ui_display_t *ui_display_get(uint8_t id);

/* ============================================================
 *  Page registry
 * ============================================================ */

/**
 * Register a page with the library.
 *
 * Actions performed at registration time:
 *  - Resolves page->display from the display registry via disp_id.
 *  - Propagates page display/bg_color down to every widget and
 *    its children (parent settings overwrite child settings).
 *  - Resolves each widget's _tick_tier from its fps field.
 *  - Initialises bounce velocity state for WIDGET_BOUNCE widgets.
 *
 * Returns 0 on success, -1 on error.
 */
int ui_page_register(ui_page_t *page);

/**
 * Retrieve a registered page by its idx.
 * Returns NULL if not found.
 */
ui_page_t *ui_page_get(uint8_t idx);

/* ============================================================
 *  Selection
 * ============================================================ */

/**
 * Select the widget with the given selection index on the given page.
 * Deselects the previously selected widget.
 * idx == 0 clears selection.
 */
void ui_page_select(ui_page_t *page, uint8_t idx);

/* ============================================================
 *  Main update pump
 *
 *  Call from your job handler ("ui_job") with the priority the
 *  ISR passed in.  The library only advances widgets whose
 *  _tick_tier <= priority, keeping slower animations from
 *  over-updating on fast ticks.
 * ============================================================ */
void ui_update(ui_update_priority_t priority);

/**
 * Force a full redraw of the given page (clears display then
 * draws every widget).  Useful after a page switch.
 */
void ui_page_draw(ui_page_t *page);