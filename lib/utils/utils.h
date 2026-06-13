#ifndef _UTILS_H_
#define _UTILS_H_

#ifdef FRX_ENABLE_MODULE_UTILS

#include <jescore.h>
#include "syserr.h"

#ifdef UTILS_DEBUG_PRINT_ENABLE
#define UTILS_DEBUG_PRINT(format, ...) jes_print(format, ##__VA_ARGS__)
#define UTILS_DEBUG_PRINT_PJ(pj, format, ...) jes_print_pj(pj, format, ##__VA_ARGS__)
#else
#define UTILS_DEBUG_PRINT(format, ...)
#define UTILS_DEBUG_PRINT_PJ(pj, format, ...)
#endif

#define UTILS_CLIP_MAX(max, val) val > max ? max : val
#define UTILS_CLIP_MIN(min, val) val < min ? min : val

/// @brief Remove all occurrences of a substring from a string.
/// @param str The string to modify (in-place).
/// @param sub The substring to remove.
/// @return Pointer to the modified string.
char *strremove(char *str, const char *sub);

/// @brief Convert a 16-bit unsigned integer to a 4-digit string.
/// @param num The number to convert (must be < 10000).
/// @param buf Buffer to store the result (must be at least 5 bytes including null terminator).
/// @return Error code.
e_syserr_t uint_to_4digit_str(uint16_t num, char* buf);

/// @brief Convert a 4-digit string to a 16-bit unsigned integer.
/// @param str The string to convert (must be exactly 4 digits).
/// @param num Pointer to store the result.
/// @return Error code.
e_syserr_t str_to_4digit_uint(const char* str, uint16_t* num);

#endif // FRX_ENABLE_MODULE_UTILS
#endif
