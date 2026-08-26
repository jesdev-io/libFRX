# FSM Tools

Generic finite-state-machine helpers for firmware that needs state descriptors, guarded routine execution, transition ordering, and snapshot publication without baking a concrete application state graph into `libFRX`.

This module intentionally does **not** provide states such as idle, record, play, synth, or settings. Those names and procedures belong to the consuming firmware.

!!! tip "Quickstart"
    Enable the module and include the header:

    ```ini
    build_flags =
        -DFRX_ENABLE_MODULE_FSM_TOOLS
    ```

    ```cpp
    #include "fsm_tools.h"

    static e_syserr_t enter(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx) {
        return e_syserr_none;
    }

    static void routine(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx) {
        void* block = frx_fsm_get_block_context(fsm);
        // interpret block according to the application callback seam
    }

    static e_syserr_t exit(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx) {
        return e_syserr_none;
    }

    enum { app_state_idle = 1, app_state_record = 2 };

    frx_fsm_t fsm;
    frx_fsm_state_t states[] = {
        {.id = app_state_idle, .enter = enter, .routine = routine, .exit = exit, .ctx = NULL},
        {.id = app_state_record, .enter = enter, .routine = routine, .exit = exit, .ctx = NULL},
    };

    void setup() {
        frx_fsm_init(&fsm, states, 2, app_state_idle, FRX_FSM_NO_STATE);
        frx_fsm_enter_state(&fsm, app_state_idle);
    }
    ```

## Design intent

`fsm_tools` exists for reusable FSM mechanics only:

- state descriptors with `enter`, `routine`, and `exit` callbacks;
- transition sequencing with first-error return;
- routine synchronization so control code can wait for callback code to leave a state routine;
- lock-protected user-defined snapshot slots;
- callback-local block context passing for audio, sensor, or other block-processing loops.

## Snapshot slots

In `fsm_tools`, a **slot** is one named storage location inside an FSM instance. It is not a state, not a transition, not a GPIO slot, and not a scheduling queue. It is simply a lock-protected byte buffer registered under a small integer ID.

Slots can hold whatever the application says they hold. For example, one application might use two semantic slots:

```cpp
typedef enum {
    app_slot_stateful_input = 0,   // commands/config that control state behavior
    app_slot_process_output = 1,   // status/results produced by routines
} app_slot_t;
```

Then register storage:

```cpp
frx_fsm_set_slot_storage(&fsm, app_slot_stateful_input, &input, sizeof(input));
frx_fsm_set_slot_storage(&fsm, app_slot_process_output, &output, sizeof(output));
```

And copy current slot contents in/out:

```cpp
frx_fsm_publish_slot(&fsm, app_slot_stateful_input, &input, sizeof(input));
frx_fsm_get_slot(&fsm, app_slot_process_output, &output, sizeof(output));
```

Slots are **current snapshots**, not history. Publishing to the same slot overwrites the previous contents. `FRX_FSM_MAX_SLOTS` controls the compile-time number of slots stored in each `frx_fsm_t`.

The consuming firmware still owns:

- the state enum;
- the state graph and legal transitions;
- concrete state context structs;
- procedures such as opening files, checking free space, starting synths, or updating UI.

## Types

::: api fsm_tools.frx_fsm_state_id_t

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_slot_id_t

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_enter_t

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_routine_t

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_exit_t

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_state_t

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_slot_t

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_t

<!-- Add handwritten notes here. -->

## Functions

::: api fsm_tools.frx_fsm_init

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_set_slot_storage

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_find_state

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_enter_state

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_exit_state

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_routine_state

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_transition

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_await_sync

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_publish_slot

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_get_slot

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_set_block_context

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_get_block_context

<!-- Add handwritten notes here. -->

::: api fsm_tools.frx_fsm_process_current_with_block

<!-- Add handwritten notes here. -->

## FSM Tools Macros

| Macro | Purpose |
|---|---|
| `FRX_ENABLE_MODULE_FSM_TOOLS` | Includes the generic FSM tools module in the build. |
| `FRX_FSM_NO_STATE` | Sentinel state ID for applications that do not need a distinct transition marker. |
| `FRX_FSM_MAX_SLOTS` | Compile-time number of snapshot slots in each FSM instance. Defaults to `2`. |
