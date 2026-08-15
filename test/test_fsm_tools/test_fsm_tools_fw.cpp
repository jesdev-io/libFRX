#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include "fsm_tools.h"

typedef struct{
    uint16_t entered;
    uint16_t exited;
    uint16_t ran;
    uint16_t block_seen;
} test_state_ctx_t;

typedef struct{
    uint16_t state;
    uint16_t count;
} test_args_t;

typedef struct{
    uint16_t ran;
    uint16_t last_block;
} test_values_t;

static frx_fsm_t fsm;
static test_state_ctx_t ctx_a;
static test_state_ctx_t ctx_b;
static test_args_t args_snapshot;
static test_values_t values_snapshot;

static e_syserr_t test_enter(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx) {
    (void)fsm;
    (void)state;
    ((test_state_ctx_t*)ctx)->entered++;
    return e_syserr_none;
}

static e_syserr_t test_exit(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx) {
    (void)fsm;
    (void)state;
    ((test_state_ctx_t*)ctx)->exited++;
    return e_syserr_none;
}

static void test_routine(frx_fsm_t* fsm, frx_fsm_state_t* state, void* ctx) {
    (void)state;
    test_state_ctx_t* sctx = (test_state_ctx_t*)ctx;
    sctx->ran++;
    uint16_t* block = (uint16_t*)frx_fsm_get_block_context(fsm);
    if (block != NULL) sctx->block_seen = *block;
    test_values_t values = {.ran = sctx->ran, .last_block = sctx->block_seen};
    frx_fsm_publish_values(fsm, &values, sizeof(values));
}

static frx_fsm_state_t states[] = {
    {.id = 1, .enter = test_enter, .routine = test_routine, .exit = test_exit, .ctx = &ctx_a},
    {.id = 2, .enter = test_enter, .routine = test_routine, .exit = test_exit, .ctx = &ctx_b},
};

void test_jes_bootup(void){
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_fsm_tools_init_and_lookup(void){
    memset(&ctx_a, 0, sizeof(ctx_a));
    memset(&ctx_b, 0, sizeof(ctx_b));
    e_syserr_t e = frx_fsm_init(&fsm, states, 2, 1, FRX_FSM_NO_STATE);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(1, fsm.cur_state);
    TEST_ASSERT_EQUAL(&states[0], frx_fsm_find_state(&fsm, 1));
    TEST_ASSERT_EQUAL(NULL, frx_fsm_find_state(&fsm, 9));
    e = frx_fsm_set_snapshot_storage(&fsm, &args_snapshot, sizeof(args_snapshot), &values_snapshot, sizeof(values_snapshot));
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_fsm_tools_snapshot(void){
    test_args_t args = {.state = 1, .count = 42};
    test_args_t args_read = {0};
    e_syserr_t e = frx_fsm_publish_args(&fsm, &args, sizeof(args));
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = frx_fsm_get_args(&fsm, &args_read, sizeof(args_read));
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(args.state, args_read.state);
    TEST_ASSERT_EQUAL(args.count, args_read.count);
}

void test_fsm_tools_transition(void){
    e_syserr_t e = frx_fsm_transition(&fsm, 2, 1);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(2, fsm.cur_state);
    TEST_ASSERT_EQUAL(1, ctx_a.exited);
    TEST_ASSERT_EQUAL(1, ctx_b.entered);
}

void test_fsm_tools_routine_and_block(void){
    uint16_t block = 77;
    e_syserr_t e = frx_fsm_process_current_with_block(&fsm, &block);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(1, ctx_b.ran);
    TEST_ASSERT_EQUAL(77, ctx_b.block_seen);
    TEST_ASSERT_EQUAL(NULL, frx_fsm_get_block_context(&fsm));

    test_values_t values = {0};
    e = frx_fsm_get_values(&fsm, &values, sizeof(values));
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(1, values.ran);
    TEST_ASSERT_EQUAL(77, values.last_block);
}

void setup(){
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_fsm_tools_init_and_lookup);
    RUN_TEST(test_fsm_tools_snapshot);
    RUN_TEST(test_fsm_tools_transition);
    RUN_TEST(test_fsm_tools_routine_and_block);
    UNITY_END();
}

void loop(){
}
