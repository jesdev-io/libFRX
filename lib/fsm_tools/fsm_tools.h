#ifndef _FSM_TOOLS_H_
#define _FSM_TOOLS_H_

#ifdef FRX_ENABLE_MODULE_FSM_TOOLS

#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "syserr.h"

#define FRX_FSM_NO_STATE 0xFFFF

#ifndef FRX_FSM_MAX_SLOTS
#define FRX_FSM_MAX_SLOTS 2
#endif

/// @brief FSM state ID type.
typedef uint16_t frx_fsm_state_id_t;

/// @brief Snapshot slot ID type.
typedef uint8_t frx_fsm_slot_id_t;

struct frx_fsm_t;
struct frx_fsm_state_t;

/// @brief State enter routine signature.
/// @param fsm FSM instance.
/// @param state State descriptor being entered.
/// @param ctx User context pointer for the state.
/// @return Error code.
typedef e_syserr_t (*frx_fsm_enter_t)(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx);

/// @brief State routine signature.
/// @param fsm FSM instance.
/// @param state State descriptor being executed.
/// @param ctx User context pointer for the state.
typedef void (*frx_fsm_routine_t)(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx);

/// @brief State exit routine signature.
/// @param fsm FSM instance.
/// @param state State descriptor being exited.
/// @param ctx User context pointer for the state.
/// @return Error code.
typedef e_syserr_t (*frx_fsm_exit_t)(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx);

/// @brief Generic FSM state descriptor.
typedef struct frx_fsm_state_t{
    frx_fsm_state_id_t id;
    frx_fsm_enter_t enter;
    frx_fsm_routine_t routine;
    frx_fsm_exit_t exit;
    void* ctx;
}frx_fsm_state_t;

/// @brief One user-defined snapshot slot.
typedef struct frx_fsm_slot_t{
    void* storage;
    size_t size;
}frx_fsm_slot_t;

/// @brief Generic FSM instance. The application owns the storage.
typedef struct frx_fsm_t{
    frx_fsm_state_id_t cur_state;
    frx_fsm_state_id_t transition_state;
    frx_fsm_state_t* states;
    uint16_t n_states;
    SemaphoreHandle_t routine_lock;
    SemaphoreHandle_t snapshot_lock;
    frx_fsm_slot_t slots[FRX_FSM_MAX_SLOTS];
    void* block_ctx;
}frx_fsm_t;

/// @brief Initialize a generic FSM instance.
/// @param fsm FSM instance storage.
/// @param states State descriptor array owned by caller.
/// @param n_states Number of descriptors in states.
/// @param initial_state Initial current state ID.
/// @param transition_state Optional transition marker ID. Use FRX_FSM_NO_STATE if unused.
/// @return Error code.
e_syserr_t frx_fsm_init(frx_fsm_t* fsm, frx_fsm_state_t* states, uint16_t n_states, frx_fsm_state_id_t initial_state, frx_fsm_state_id_t transition_state);

/// @brief Attach storage for one user-defined snapshot slot.
/// @param fsm FSM instance.
/// @param slot Application-defined slot ID. Must be less than FRX_FSM_MAX_SLOTS.
/// @param storage Snapshot storage owned by caller.
/// @param size Snapshot byte size.
/// @return Error code.
/// @note Slots are not history. Each publish overwrites the current value in that slot.
e_syserr_t frx_fsm_set_slot_storage(frx_fsm_t* fsm, frx_fsm_slot_id_t slot, void* storage, size_t size);

/// @brief Find a state descriptor by ID.
/// @param fsm FSM instance.
/// @param id State ID.
/// @return State descriptor pointer, or NULL if missing.
frx_fsm_state_t* frx_fsm_find_state(frx_fsm_t* fsm, frx_fsm_state_id_t id);

/// @brief Call a state's enter routine and update current state on success.
/// @param fsm FSM instance.
/// @param id State ID to enter.
/// @return Error code.
e_syserr_t frx_fsm_enter_state(frx_fsm_t* fsm, frx_fsm_state_id_t id);

/// @brief Call a state's exit routine and set transition marker on success.
/// @param fsm FSM instance.
/// @param id State ID to exit.
/// @return Error code.
e_syserr_t frx_fsm_exit_state(frx_fsm_t* fsm, frx_fsm_state_id_t id);

/// @brief Execute a state's routine under the routine synchronization guard.
/// @param fsm FSM instance.
/// @param id State ID to execute.
/// @return Error code.
e_syserr_t frx_fsm_routine_state(frx_fsm_t* fsm, frx_fsm_state_id_t id);

/// @brief Transition from current state to target state.
/// @param fsm FSM instance.
/// @param target_state Target state ID.
/// @param await_sync If nonzero, function blocks before transition until the last routine block is finished.
/// @return Error code.
e_syserr_t frx_fsm_transition(frx_fsm_t* fsm, frx_fsm_state_id_t target_state, uint8_t await_sync);

/// @brief Block until no state routine is executing.
/// @param fsm FSM instance.
/// @return Error code.
e_syserr_t frx_fsm_await_sync(frx_fsm_t* fsm);

/// @brief Publish bytes to a user-defined snapshot slot.
/// @param fsm FSM instance.
/// @param slot Application-defined slot ID.
/// @param data Source bytes.
/// @param len Source byte length. Must match configured slot size.
/// @return Error code.
e_syserr_t frx_fsm_publish_slot(frx_fsm_t* fsm, frx_fsm_slot_id_t slot, const void* data, size_t len);

/// @brief Read bytes from a user-defined snapshot slot.
/// @param fsm FSM instance.
/// @param slot Application-defined slot ID.
/// @param data Destination bytes.
/// @param len Destination byte length. Must match configured slot size.
/// @return Error code.
e_syserr_t frx_fsm_get_slot(frx_fsm_t* fsm, frx_fsm_slot_id_t slot, void* data, size_t len);

/// @brief Set the current block-processing context pointer.
/// @param fsm FSM instance.
/// @param block_ctx Callback-local block context pointer.
/// @return Error code.
e_syserr_t frx_fsm_set_block_context(frx_fsm_t* fsm, void* block_ctx);

/// @brief Get the current block-processing context pointer.
/// @param fsm FSM instance.
/// @return Current block context pointer, or NULL.
void* frx_fsm_get_block_context(frx_fsm_t* fsm);

/// @brief Execute the current state's routine with a callback-local block context.
/// @param fsm FSM instance.
/// @param block_ctx Callback-local block context pointer.
/// @return Error code.
e_syserr_t frx_fsm_process_current_with_block(frx_fsm_t* fsm, void* block_ctx);

#endif // FRX_ENABLE_MODULE_FSM_TOOLS
#endif // _FSM_TOOLS_H_
