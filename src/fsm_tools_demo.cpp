#include <Arduino.h>
#include <jescore.h>
#include <string.h>
#include "fsm_tools.h"

#ifdef FRX_ENABLE_MODULE_FSM_TOOLS

#define FSM_DEMO_JOB_NAME "fsm"

enum demo_state_t { DEMO_OFF = 0, DEMO_ON = 1 };
enum demo_slot_t { DEMO_SLOT_CONTROL = 0, DEMO_SLOT_STATUS = 1 };

typedef struct { uint8_t requested_on; } demo_control_t;
typedef struct { uint32_t worker_runs; uint8_t state_seen; } demo_status_t;

static frx_fsm_t fsm;
static demo_control_t control;
static demo_status_t status;

static e_syserr_t enter_cb(frx_fsm_t*, frx_fsm_state_t* s, void*) {
    uart_unif_writef("enter %u\n\r", s->id);
    return e_syserr_none;
}

static e_syserr_t exit_cb(frx_fsm_t*, frx_fsm_state_t* s, void*) {
    uart_unif_writef("exit %u\n\r", s->id);
    return e_syserr_none;
}

static void worker_routine_cb(frx_fsm_t* f, frx_fsm_state_t* s, void*) {
    demo_control_t c = {0};
    demo_status_t st = {0};
    frx_fsm_get_slot(f, DEMO_SLOT_CONTROL, &c, sizeof(c));
    frx_fsm_get_slot(f, DEMO_SLOT_STATUS, &st, sizeof(st));
    st.worker_runs++;
    st.state_seen = s->id;
    frx_fsm_publish_slot(f, DEMO_SLOT_STATUS, &st, sizeof(st));
    uart_unif_writef("worker state=%u requested_on=%u runs=%lu\n\r", s->id, c.requested_on, (unsigned long)st.worker_runs);
}

static frx_fsm_state_t states[] = {
    {.id = DEMO_OFF, .enter = enter_cb, .routine = worker_routine_cb, .exit = exit_cb, .ctx = NULL},
    {.id = DEMO_ON,  .enter = enter_cb, .routine = worker_routine_cb, .exit = exit_cb, .ctx = NULL},
};

static void help() {
    uart_unif_writef("fsm_demo: state, on, off, worker, status\n\r");
}

static void fsm_jccl(void*) {
    char* cmd = strtok(jes_job_get_args(), " ");
    if (!cmd || strcmp(cmd, "help") == 0) { help(); return; }

    if (strcmp(cmd, "state") == 0) {
        uart_unif_writef("state %u\n\r", fsm.cur_state);
    } else if (strcmp(cmd, "on") == 0) {
        demo_control_t c = {.requested_on = 1};
        frx_fsm_publish_slot(&fsm, DEMO_SLOT_CONTROL, &c, sizeof(c));
        uart_unif_writef("transition %d\n\r", frx_fsm_transition(&fsm, DEMO_ON, 1));
    } else if (strcmp(cmd, "off") == 0) {
        demo_control_t c = {.requested_on = 0};
        frx_fsm_publish_slot(&fsm, DEMO_SLOT_CONTROL, &c, sizeof(c));
        uart_unif_writef("transition %d\n\r", frx_fsm_transition(&fsm, DEMO_OFF, 1));
    } else if (strcmp(cmd, "worker") == 0) {
        uart_unif_writef("worker_ret %d\n\r", frx_fsm_process_current_with_block(&fsm, (void*)"worker-task"));
    } else if (strcmp(cmd, "status") == 0) {
        demo_control_t c = {0};
        demo_status_t st = {0};
        frx_fsm_get_slot(&fsm, DEMO_SLOT_CONTROL, &c, sizeof(c));
        frx_fsm_get_slot(&fsm, DEMO_SLOT_STATUS, &st, sizeof(st));
        uart_unif_writef("control_on %u status_runs %lu status_state %u\n\r", c.requested_on, (unsigned long)st.worker_runs, st.state_seen);
    } else {
        help();
    }
}

void setup() {
    jes_init();
    frx_fsm_init(&fsm, states, 2, DEMO_OFF, FRX_FSM_NO_STATE);
    frx_fsm_set_slot_storage(&fsm, DEMO_SLOT_CONTROL, &control, sizeof(control));
    frx_fsm_set_slot_storage(&fsm, DEMO_SLOT_STATUS, &status, sizeof(status));
    frx_fsm_enter_state(&fsm, DEMO_OFF);
    jes_register_job(FSM_DEMO_JOB_NAME, 3072, 1, fsm_jccl, 0, 1);
    help();
}

void loop() { delay(1000); }

#else
void setup() {}
void loop() {}
#endif
