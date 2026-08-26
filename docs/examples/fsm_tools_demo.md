# FSM Tools Demo App

Tiny `fsm_tools` demo. The FSM is passive storage/coordination, not a task.

Two CLI commands simulate active tasks:

- `on` / `off`: a controller task writes a control slot and transitions state.
- `worker`: a worker task runs the current state's routine, reads control, writes status.

Slots are application-named current snapshots:

```cpp
enum demo_slot_t {
    DEMO_SLOT_CONTROL = 0,
    DEMO_SLOT_STATUS = 1,
};
```

Build:

```bash
pio run -e frx_demo_fsm_tools
```

Flash/test:

```bash
JESCORE_CLI_PORT=/dev/ttyACM0 .venv/bin/python shared/scripts/flash_demo_apps_cli_test.py frx_demo_fsm_tools
```

Commands:

```bash
jescore "fsm_demo state"   # read current state
jescore "fsm_demo on"      # publish control + transition ON
jescore "fsm_demo off"     # publish control + transition OFF
jescore "fsm_demo worker"  # run current routine as if a worker task called it
jescore "fsm_demo status"  # read control/status slots
```
