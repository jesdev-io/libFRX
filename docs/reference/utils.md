# Utils

Small utility functions shared by other modules.

This page combines generated API reference cards with module-specific notes.

!!! tip "Quickstart"
    Enable the module in your PlatformIO environment:

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_UTILS
    ```

    Utils is a helper-only module and has no runtime initializer. Include it and call the helper you need:

    ```cpp
    #include "utils.h"

    char buf[5];
    uint_to_4digit_str(42, buf);
    ```

    See [Configuration and defaults](configuration.md) for the full build-flag/defaults model.

## Functions

Public standalone utility helpers shared by other modules.

::: api utils.str_to_4digit_uint

<!-- Add handwritten notes here. -->

::: api utils.strremove

<!-- Add handwritten notes here. -->

::: api utils.uint_to_4digit_str

<!-- Add handwritten notes here. -->
## Utils Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_UTILS` | Includes the utils module in the build. |
| `UTILS_CLIP_MAX(max, val)` | Clip a value to an upper bound. |
| `UTILS_CLIP_MIN(min, val)` | Clip a value to a lower bound. |

