#ifndef _UI_TIMER_H_
#define _UI_TIMER_H_

#include "ui_types.h"
#include "syserr.h"
#include "driver/timer.h"
#include "jescore.h"

// Timer configuration
#define UI_TIMER_GROUP      TIMER_GROUP_0
#define UI_TIMER_NUM        TIMER_0
#define UI_TIMER_FPS        20
#define UI_TIMER_PERIOD_US  (1000000 / UI_TIMER_FPS)  // 50ms period

/// @brief Initialize UI timer
/// @return e_syserr_none on success, error code otherwise
e_syserr_t ui_timer_init(void);

/// @brief Deinitialize UI timer
void ui_timer_deinit(void);

/// @brief Start UI timer
/// @return e_syserr_none on success, error code otherwise
e_syserr_t ui_timer_start(void);

/// @brief Stop UI timer
/// @return e_syserr_none on success, error code otherwise
e_syserr_t ui_timer_stop(void);

/// @brief Set timer callback
/// @param callback Function to call on timer interrupt
void ui_timer_set_callback(void (*callback)(void*));

#endif // UI_TIMER_H