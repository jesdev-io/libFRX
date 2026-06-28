#ifndef _LIBFRX_SYS_
#define _LIBFRX_SYS_

#define LIBFRX_SYS_PFX          "libFRX"
#define LIBFRX_SYS_INFO_PFX     LIBFRX_SYS_PFX " INFO: "
#define LIBFRX_SYS_WARN_PFX     LIBFRX_SYS_PFX " WARN: "
#define LIBFRX_SYS_FAIL_PFX     LIBFRX_SYS_PFX " FAIL: "

#ifdef LIBFRX_SYS_DEBUG_PRINT_ENABLE
#define LIBFRX_SYS_DEBUG_PRINT(format, ...) jes_print(format, ##__VA_ARGS__)
#define LIBFRX_SYS_DEBUG_PRINT_PJ(pj, format, ...) jes_print_pj(pj, format, ##__VA_ARGS__)
#else
#define LIBFRX_SYS_DEBUG_PRINT(format, ...)
#define LIBFRX_SYS_DEBUG_PRINT_PJ(pj, format, ...)
#endif

#endif // _LIBFRX_SYS_