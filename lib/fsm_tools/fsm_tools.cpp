#ifdef FRX_ENABLE_MODULE_FSM_TOOLS

#include <string.h>
#include "fsm_tools.h"

static inline e_syserr_t __frx_fsm_check(frx_fsm_t* fsm) {
    if (fsm == NULL) return e_syserr_null;
    if (fsm->states == NULL) return e_syserr_uninitialized;
    if (fsm->n_states == 0) return e_syserr_uninitialized;
    return e_syserr_none;
}

static inline e_syserr_t __frx_fsm_check_snapshot(void* storage, size_t storage_len, const void* data, size_t len) {
    if (data == NULL) return e_syserr_null;
    if (storage == NULL) return e_syserr_uninitialized;
    if (storage_len == 0 || len == 0) return e_syserr_zero;
    if (storage_len != len) return e_syserr_param;
    return e_syserr_none;
}

e_syserr_t frx_fsm_init(frx_fsm_t* fsm, frx_fsm_state_t* states, uint16_t n_states, frx_fsm_state_id_t initial_state, frx_fsm_state_id_t transition_state) {
    if (fsm == NULL) return e_syserr_null;
    if (states == NULL) return e_syserr_null;
    if (n_states == 0) return e_syserr_zero;

    memset(fsm, 0, sizeof(frx_fsm_t));
    fsm->cur_state = initial_state;
    fsm->transition_state = transition_state;
    fsm->states = states;
    fsm->n_states = n_states;
    fsm->routine_lock = xSemaphoreCreateMutex();
    if (fsm->routine_lock == NULL) return e_syserr_null;
    fsm->snapshot_lock = xSemaphoreCreateMutex();
    if (fsm->snapshot_lock == NULL) return e_syserr_null;
    if (frx_fsm_find_state(fsm, initial_state) == NULL) return e_syserr_param;
    return e_syserr_none;
}

e_syserr_t frx_fsm_set_slot_storage(frx_fsm_t* fsm, frx_fsm_slot_id_t slot, void* storage, size_t size) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    if (slot >= FRX_FSM_MAX_SLOTS) return e_syserr_param;
    if (storage == NULL) return e_syserr_null;
    if (size == 0) return e_syserr_zero;
    xSemaphoreTake(fsm->snapshot_lock, portMAX_DELAY);
    fsm->slots[slot].storage = storage;
    fsm->slots[slot].size = size;
    xSemaphoreGive(fsm->snapshot_lock);
    return e_syserr_none;
}

frx_fsm_state_t* frx_fsm_find_state(frx_fsm_t* fsm, frx_fsm_state_id_t id) {
    if (__frx_fsm_check(fsm) != e_syserr_none) return NULL;
    for (uint16_t i = 0; i < fsm->n_states; i++) {
        if (fsm->states[i].id == id) return &fsm->states[i];
    }
    return NULL;
}

e_syserr_t frx_fsm_enter_state(frx_fsm_t* fsm, frx_fsm_state_id_t id) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    frx_fsm_state_t* state = frx_fsm_find_state(fsm, id);
    if (state == NULL) return e_syserr_param;
    if (state->enter != NULL) {
        e = state->enter(fsm, state, state->ctx);
        if (e != e_syserr_none) return e;
    }
    fsm->cur_state = id;
    return e_syserr_none;
}

e_syserr_t frx_fsm_exit_state(frx_fsm_t* fsm, frx_fsm_state_id_t id) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    if (id == fsm->transition_state) return e_syserr_none;
    frx_fsm_state_t* state = frx_fsm_find_state(fsm, id);
    if (state == NULL) return e_syserr_param;
    if (state->exit != NULL) {
        e = state->exit(fsm, state, state->ctx);
        if (e != e_syserr_none) return e;
    }
    fsm->cur_state = fsm->transition_state;
    return e_syserr_none;
}

e_syserr_t frx_fsm_routine_state(frx_fsm_t* fsm, frx_fsm_state_id_t id) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    if (id == fsm->transition_state) return e_syserr_none;
    frx_fsm_state_t* state = frx_fsm_find_state(fsm, id);
    if (state == NULL) return e_syserr_param;
    if (state->routine == NULL) return e_syserr_none;
    xSemaphoreTake(fsm->routine_lock, portMAX_DELAY);
    state->routine(fsm, state, state->ctx);
    xSemaphoreGive(fsm->routine_lock);
    return e_syserr_none;
}

e_syserr_t frx_fsm_transition(frx_fsm_t* fsm, frx_fsm_state_id_t target_state, uint8_t await_sync) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    if (frx_fsm_find_state(fsm, target_state) == NULL) return e_syserr_param;
    if (await_sync) {
        e = frx_fsm_await_sync(fsm);
        if (e != e_syserr_none) return e;
    }
    e = frx_fsm_exit_state(fsm, fsm->cur_state);
    if (e != e_syserr_none) return e;
    return frx_fsm_enter_state(fsm, target_state);
}

e_syserr_t frx_fsm_await_sync(frx_fsm_t* fsm) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    xSemaphoreTake(fsm->routine_lock, portMAX_DELAY);
    xSemaphoreGive(fsm->routine_lock);
    return e_syserr_none;
}

e_syserr_t frx_fsm_publish_slot(frx_fsm_t* fsm, frx_fsm_slot_id_t slot, const void* data, size_t len) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    if (slot >= FRX_FSM_MAX_SLOTS) return e_syserr_param;
    e = __frx_fsm_check_snapshot(fsm->slots[slot].storage, fsm->slots[slot].size, data, len);
    if (e != e_syserr_none) return e;
    xSemaphoreTake(fsm->snapshot_lock, portMAX_DELAY);
    memcpy(fsm->slots[slot].storage, data, len);
    xSemaphoreGive(fsm->snapshot_lock);
    return e_syserr_none;
}

e_syserr_t frx_fsm_get_slot(frx_fsm_t* fsm, frx_fsm_slot_id_t slot, void* data, size_t len) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    if (slot >= FRX_FSM_MAX_SLOTS) return e_syserr_param;
    e = __frx_fsm_check_snapshot(fsm->slots[slot].storage, fsm->slots[slot].size, data, len);
    if (e != e_syserr_none) return e;
    xSemaphoreTake(fsm->snapshot_lock, portMAX_DELAY);
    memcpy(data, fsm->slots[slot].storage, len);
    xSemaphoreGive(fsm->snapshot_lock);
    return e_syserr_none;
}

e_syserr_t frx_fsm_set_block_context(frx_fsm_t* fsm, void* block_ctx) {
    e_syserr_t e = __frx_fsm_check(fsm);
    if (e != e_syserr_none) return e;
    fsm->block_ctx = block_ctx;
    return e_syserr_none;
}

void* frx_fsm_get_block_context(frx_fsm_t* fsm) {
    if (__frx_fsm_check(fsm) != e_syserr_none) return NULL;
    return fsm->block_ctx;
}

e_syserr_t frx_fsm_process_current_with_block(frx_fsm_t* fsm, void* block_ctx) {
    e_syserr_t e = frx_fsm_set_block_context(fsm, block_ctx);
    if (e != e_syserr_none) return e;
    e = frx_fsm_routine_state(fsm, fsm->cur_state);
    fsm->block_ctx = NULL;
    return e;
}

#endif // FRX_ENABLE_MODULE_FSM_TOOLS
