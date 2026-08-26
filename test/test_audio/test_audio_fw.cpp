#include <Arduino.h>
#include <string.h>
#include <jescore.h>
#include <unity.h>
#include "esp_dsp.h"
#include "audio.h"
#include "syserr.h"

#define MAX_JOB_ARGS_LEN_BYTE 512

static volatile uint32_t audio_cb_calls = 0;
static volatile bool audio_cb_saw_input = false;
static volatile bool audio_cb_saw_output = false;
static volatile uint32_t audio_cb_last_len = 0;
static volatile uint8_t audio_cb_last_nch = 0;
static volatile float audio_fft_energy = 0.0f;

#define AUDIO_FFT_BENCH_N 128

static float audio_fft_data[2 * AUDIO_FFT_BENCH_N];
static float audio_fft_window[AUDIO_FFT_BENCH_N];
static bool audio_fft_ready = false;

static e_syserr_t audio_fft_bench_init(void){
    if(audio_fft_ready) return e_syserr_none;
    if(dsps_fft2r_init_fc32(NULL, AUDIO_FFT_BENCH_N) != ESP_OK) return e_syserr_driver_fail;
    dsps_wind_hann_f32(audio_fft_window, AUDIO_FFT_BENCH_N);
    audio_fft_ready = true;
    return e_syserr_none;
}

static void audio_fft_bench_ch(audio_sample_t* in, uint32_t len, uint8_t ch){
    if(!audio_fft_ready) return;
    uint32_t n = len < AUDIO_FFT_BENCH_N ? len : AUDIO_FFT_BENCH_N;
    for(uint32_t i = 0; i < AUDIO_FFT_BENCH_N; i++){
        audio_fft_data[2 * i] = (i < n) ? ((float)(in[i].ch[ch] >> 8) / 8388608.0f) * audio_fft_window[i] : 0.0f;
        audio_fft_data[2 * i + 1] = 0.0f;
    }
    if(dsps_fft2r_fc32(audio_fft_data, AUDIO_FFT_BENCH_N) != ESP_OK) return;
    if(dsps_bit_rev_fc32(audio_fft_data, AUDIO_FFT_BENCH_N) != ESP_OK) return;
    float energy = 0.0f;
    for(uint8_t k = 1; k < 16; k++){
        float re = audio_fft_data[2 * k];
        float im = audio_fft_data[2 * k + 1];
        energy += re * re + im * im;
    }
    audio_fft_energy += energy;
}

static inline void feedthrough_cb(audio_io_t* iobuf){
    audio_cb_calls++;
    audio_cb_saw_input = iobuf->in != NULL;
    audio_cb_saw_output = iobuf->out != NULL;
    audio_cb_last_len = iobuf->len;
    audio_cb_last_nch = iobuf->nch;
    if(!iobuf->in || !iobuf->out) return;
    memcpy(iobuf->out, iobuf->in, sizeof(audio_sample_t) * iobuf->len);
}

static void fft_feedthrough_cb(audio_io_t* iobuf){
    feedthrough_cb(iobuf);
    if(!iobuf || !iobuf->in) return;
    for(uint8_t ch = 0; ch < iobuf->nch && ch < 2; ch++){
        audio_fft_bench_ch(iobuf->in, iobuf->len, ch);
    }
}

void test_jes_bootup(void) {
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_audio_init_default(void) {
    e_syserr_t e = audio_init_default();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_audio_run(void) {
    audio_set_callback(feedthrough_cb);
    e_syserr_t e = audio_start();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_audio_sampler_static_read_write(void) {
    uint32_t calls_before = audio_cb_calls;
    jes_delay_job_ms(500);
    TEST_ASSERT_EQUAL(e_err_no_err, jes_error_get(AUDIO_SERVER_JOB_NAME));
    TEST_ASSERT_GREATER_THAN_UINT32(calls_before, audio_cb_calls);
#if AUDIO_CFG_HAS_INPUT
    TEST_ASSERT_TRUE(audio_cb_saw_input);
#else
    TEST_ASSERT_FALSE(audio_cb_saw_input);
#endif
#if AUDIO_CFG_HAS_OUTPUT
    TEST_ASSERT_TRUE(audio_cb_saw_output);
#else
    TEST_ASSERT_FALSE(audio_cb_saw_output);
#endif
    TEST_ASSERT_EQUAL_UINT32(AUDIO_BLOCK_SAMPLES, audio_cb_last_len);
    TEST_ASSERT_GREATER_THAN_UINT8(0, audio_cb_last_nch);
}

void test_audio_timing_probe(void) {
    audio_timing_reset();
    jes_delay_job_ms(2000);
    audio_timing_t timing;
    audio_timing_get(&timing);
    uart_unif_writef("AUDIO_TIMING proc c=%lu l=%lu n=%lu x=%lu r=%lu\n\r",
                  (unsigned long)timing.process_count,
                  (unsigned long)timing.process_last_delta_us,
                  (unsigned long)timing.process_min_delta_us,
                  (unsigned long)timing.process_max_delta_us,
                  (unsigned long)timing.process_max_runtime_us);
    uart_unif_writef("AUDIO_TIMING read c=%lu l=%lu x=%lu e=%lu\n\r",
                  (unsigned long)timing.read_count,
                  (unsigned long)timing.read_last_runtime_us,
                  (unsigned long)timing.read_max_runtime_us,
                  (unsigned long)timing.read_error_count);
    uart_unif_writef("AUDIO_TIMING cb c=%lu l=%lu x=%lu\n\r",
                  (unsigned long)timing.callback_count,
                  (unsigned long)timing.callback_last_runtime_us,
                  (unsigned long)timing.callback_max_runtime_us);
    uart_unif_writef("AUDIO_TIMING intcb c=%lu l=%lu x=%lu\n\r",
                  (unsigned long)timing.internal_callback_count,
                  (unsigned long)timing.internal_callback_last_runtime_us,
                  (unsigned long)timing.internal_callback_max_runtime_us);
    uart_unif_writef("AUDIO_TIMING write c=%lu l=%lu x=%lu e=%lu\n\r",
                  (unsigned long)timing.write_count,
                  (unsigned long)timing.write_last_runtime_us,
                  (unsigned long)timing.write_max_runtime_us,
                  (unsigned long)timing.write_error_count);
    for(uint8_t i = audio_i2s_bank_a; i < audio_i2s_bank_N; i++){
        uart_unif_writef("AUDIO_TIMING bank=%u c=%lu rx=%lu tx=%lu l=%lu n=%lu x=%lu\n\r",
                      i,
                      (unsigned long)timing.i2s_evt_count[i],
                      (unsigned long)timing.i2s_rx_evt_count[i],
                      (unsigned long)timing.i2s_tx_evt_count[i],
                      (unsigned long)timing.i2s_evt_last_delta_us[i],
                      (unsigned long)timing.i2s_evt_min_delta_us[i],
                      (unsigned long)timing.i2s_evt_max_delta_us[i]);
    }
    TEST_ASSERT_EQUAL(e_err_no_err, jes_error_get(AUDIO_SERVER_JOB_NAME));
    TEST_ASSERT_GREATER_THAN_UINT32(0, timing.process_count);
    TEST_ASSERT_EQUAL_UINT32(0, timing.read_error_count);
    TEST_ASSERT_EQUAL_UINT32(0, timing.write_error_count);
}

void test_audio_set_callback(void) {
    e_syserr_t e = audio_set_callback(feedthrough_cb);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_audio_fft_callback_benchmark(void) {
    TEST_ASSERT_EQUAL(e_syserr_none, audio_fft_bench_init());
    TEST_ASSERT_EQUAL(e_syserr_none, audio_set_callback(fft_feedthrough_cb));
    audio_timing_reset();
    audio_fft_energy = 0.0f;
    jes_delay_job_ms(2000);
    audio_timing_t timing;
    audio_timing_get(&timing);
    uint32_t expected_block_us = (uint32_t)(((uint64_t)AUDIO_BLOCK_SAMPLES * 1000000ULL) / audio_get_sr());
    uart_unif_writef("AUDIO_FFT_BENCH b=esp-dsp-r2 n=%u c=%lu p=%lu u=%lu e=%d\n\r",
                  (unsigned int)AUDIO_FFT_BENCH_N,
                  (unsigned long)timing.callback_max_runtime_us,
                  (unsigned long)timing.process_max_runtime_us,
                  (unsigned long)expected_block_us,
                  (int)(audio_fft_energy * 1000.0f));
    TEST_ASSERT_EQUAL(e_err_no_err, jes_error_get(AUDIO_SERVER_JOB_NAME));
    TEST_ASSERT_GREATER_THAN_UINT32(0, timing.callback_count);
    TEST_ASSERT_LESS_THAN_UINT32(expected_block_us / 2, timing.callback_max_runtime_us);
    TEST_ASSERT_EQUAL(e_syserr_none, audio_set_callback(feedthrough_cb));
}

void test_audio_volume_mute(void) {
    TEST_ASSERT_EQUAL(e_syserr_none, audio_set_gain(0.5f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, audio_get_gain());
    TEST_ASSERT_EQUAL(e_syserr_none, audio_set_gain(-1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, audio_get_gain());
    TEST_ASSERT_EQUAL(e_syserr_none, audio_set_gain(2.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, audio_get_gain());
    TEST_ASSERT_EQUAL(e_syserr_none, audio_set_gain(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, audio_get_gain());
    TEST_ASSERT_EQUAL(e_syserr_none, audio_set_gain(1.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, audio_get_gain());
}

void test_audio_init_invalid_sr(void) {
    audio_settings_t settings = AUDIO_SETTINGS_CFG_DEFAULT;
    audio_bank_t banks[] = AUDIO_BANKS_CFG_DEFAULT;
    settings.sr = 99999;
    e_syserr_t e = audio_init(settings, banks, audio_i2s_bank_N);
    TEST_ASSERT_EQUAL(e_syserr_param, e);
}

void test_audio_reinit_44100(void) {
    audio_settings_t settings = AUDIO_SETTINGS_CFG_DEFAULT;
    audio_bank_t banks[] = AUDIO_BANKS_CFG_DEFAULT;
    settings.sr = AUDIO_SR_44100;
    e_syserr_t e = audio_init(settings, banks, audio_i2s_bank_N);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = audio_start();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    uint32_t calls_before = audio_cb_calls;
    jes_delay_job_ms(500);
    TEST_ASSERT_GREATER_THAN_UINT32(calls_before, audio_cb_calls);
}

void test_audio_reinit_96000(void) {
    audio_settings_t settings = AUDIO_SETTINGS_CFG_DEFAULT;
    audio_bank_t banks[] = AUDIO_BANKS_CFG_DEFAULT;
    settings.sr = AUDIO_SR_96000;
    e_syserr_t e = audio_init(settings, banks, audio_i2s_bank_N);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = audio_start();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    uint32_t calls_before = audio_cb_calls;
    jes_delay_job_ms(500);
    TEST_ASSERT_GREATER_THAN_UINT32(calls_before, audio_cb_calls);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_audio_init_default);
    RUN_TEST(test_audio_run);
    RUN_TEST(test_audio_set_callback);
    RUN_TEST(test_audio_sampler_static_read_write);
    RUN_TEST(test_audio_timing_probe);
    RUN_TEST(test_audio_fft_callback_benchmark);
    RUN_TEST(test_audio_volume_mute);
    RUN_TEST(test_audio_init_invalid_sr);
    delay(2000);
    RUN_TEST(test_audio_reinit_44100);
    delay(2000);
    RUN_TEST(test_audio_reinit_96000);
    jes_delay_job_ms(2000);
    UNITY_END();
}

void loop() {
}
