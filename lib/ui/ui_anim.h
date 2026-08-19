#include "ui_types.h"

/**
 * Advance one tick for a widget.
 *
 * For WIDGET_ANIMATED:
 *   - Increments _frame counter.
 *   - If anim_cb.type == ANIM_CB_SELECTOR, calls selector to pick
 *     _active_child for this frame.
 *   - If anim_cb.type == ANIM_CB_HOOK, the hook is called from
 *     ui_render_widget() instead; ui_anim_tick() still advances _frame.
 *   - Handles duration: stops advancing when _frame == duration (unless 0).
 *   - Default (no cb / NULL): round-robins through children.
 *
 * For WIDGET_BOUNCE:
 *   - Moves widget->x / widget->y by _vx / _vy.
 *   - Reflects velocity on hitting display edges (reads dimensions
 *     from widget->_display).
 *
 * For WIDGET_STATIC:
 *   - No-op.
 */
void ui_anim_tick(ui_widget_t *widget);