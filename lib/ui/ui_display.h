// #ifndef UI_DISPLAY_H
// #define UI_DISPLAY_H

// #include "syserr.h"
// #include "ui_types.h"

// /// @brief Initialize all displays
// /// @return e_syserr_none on success, error code otherwise
// e_syserr_t ui_display_init_all(void);

// /// @brief Initialize a specific display
// /// @param display Display to initialize
// /// @return e_syserr_none on success, error code otherwise
// e_syserr_t ui_display_init(ui_display_t display);

// /// @brief Get pointer to display object
// /// @param display Display identifier
// /// @return Pointer to Adafruit_ST77xx object, NULL if invalid
// Adafruit_ST77xx* ui_display_get(ui_display_t display);

// /// @brief Clear a display
// /// @param display Display to clear
// void ui_display_clear(ui_display_t display);

// /// @brief Get display dimensions
// /// @param display Display identifier
// /// @param width Output parameter for width (can be NULL)
// /// @param height Output parameter for height (can be NULL)
// void ui_display_get_dimensions(ui_display_t display, uint16_t* width, uint16_t* height);

// #endif // UI_DISPLAY_H