#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include <string.h>
#include "wav.h"
#include "syserr.h"
#include "sdcard.h"
#include "audio.h"

#define SD_TEST_MAX_FILES           5
#define SD_TEST_MAX_FREQ            0 // uses default

void test_jes_bootup(void) {
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_sdcard_setup(void) {
    e_syserr_t e;
    e = sd_init(SD_TEST_MAX_FILES, SD_TEST_MAX_FREQ);
    TEST_ASSERT_EQUAL(e_syserr_none, e);    
    e = sd_mnt();
    TEST_ASSERT_EQUAL(e_syserr_none, e);    
    TEST_ASSERT_EQUAL(1, sd_is_mounted());
}

void test_sdcard_cleanup(void) {
    e_syserr_t e = sd_unmnt();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(0, sd_is_mounted());
}

void test_wav_create_header(void) {
    wav_hdr_t header = wav_create_header(2, 48000, 16);
    TEST_ASSERT_EQUAL(2, header.numChannels);
    TEST_ASSERT_EQUAL(48000, header.sampleRate);
    TEST_ASSERT_EQUAL(16, header.bitsPerSample);
    TEST_ASSERT_EQUAL(192000, header.byteRate); // 48000 * 2 * 16 / 8 = 192000
    TEST_ASSERT_EQUAL(4, header.blockAlign); // 2 channels * 16 bits / 8 = 4 bytes
}

void test_wav_create_header_mono_8bit(void) {
    wav_hdr_t header = wav_create_header(1, 44100, 8);
    TEST_ASSERT_EQUAL(1, header.numChannels);
    TEST_ASSERT_EQUAL(44100, header.sampleRate);
    TEST_ASSERT_EQUAL(8, header.bitsPerSample);
    TEST_ASSERT_EQUAL(44100, header.byteRate); // 44100 * 1 * 8 / 8 = 44100
    TEST_ASSERT_EQUAL(1, header.blockAlign); // 1 channel * 8 bits / 8 = 1 byte
}

void test_wav_create_header_stereo_24bit(void) {
    wav_hdr_t header = wav_create_header(2, 96000, 24);
    TEST_ASSERT_EQUAL(2, header.numChannels);
    TEST_ASSERT_EQUAL(96000, header.sampleRate);
    TEST_ASSERT_EQUAL(24, header.bitsPerSample);
    TEST_ASSERT_EQUAL(576000, header.byteRate); // 96000 * 2 * 24 / 8 = 576000
    TEST_ASSERT_EQUAL(6, header.blockAlign); // 2 channels * 24 bits / 8 = 6 bytes
}

void test_wav_create_header_chunk_ids(void) {
    wav_hdr_t header = wav_create_header(1, 44100, 16);
    TEST_ASSERT_EQUAL(0, memcmp("RIFF", header.chunkID, 4));
    TEST_ASSERT_EQUAL(0, memcmp("WAVE", header.format, 4));
    TEST_ASSERT_EQUAL(0, memcmp("fmt ", header.subchunk1ID, 4));
    TEST_ASSERT_EQUAL(0, memcmp("data", header.subchunk2ID, 4));
}

void test_wav_create_header_pcm_format(void) {
    wav_hdr_t header = wav_create_header(2, 48000, 16);
    TEST_ASSERT_EQUAL(1, header.audioFormat); // Should be PCM (1)
    TEST_ASSERT_EQUAL(16, header.subchunk1Size); // Should be 16 for PCM
}

void test_wav_file_operations(void) {
    const char* test_filename = SDCARD_BASE_PATH "/test_wav_temp.wav";
    wav_file_t wav;
    e_syserr_t e;
    uint32_t start_time;
    if (!sd_is_mounted()) {
        TEST_FAIL_MESSAGE("SD card not mounted for WAV file operations");
        return;
    }
    e = wav_open_for_write(&wav, test_filename, 2, 48000, 16);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_NOT_NULL(wav.file);
    TEST_ASSERT_EQUAL(2, wav.header.numChannels);
    TEST_ASSERT_EQUAL(48000, wav.header.sampleRate);
    TEST_ASSERT_EQUAL(16, wav.header.bitsPerSample);
    
    e = wav_close_for_write(&wav);
    TEST_ASSERT_EQUAL(e_syserr_none, e);    
    TEST_ASSERT_NULL(wav.file);
    TEST_ASSERT_EQUAL(1, sd_file_exists(test_filename));
    
    e = wav_open_for_read(&wav, test_filename);
    TEST_ASSERT_EQUAL(e_syserr_none, e);    
    TEST_ASSERT_NOT_NULL(wav.file);
    TEST_ASSERT_EQUAL(2, wav.header.numChannels);
    TEST_ASSERT_EQUAL(48000, wav.header.sampleRate);
    TEST_ASSERT_EQUAL(16, wav.header.bitsPerSample);
    
    e = wav_close_for_read(&wav);
    TEST_ASSERT_EQUAL(e_syserr_none, e);        
    TEST_ASSERT_NULL(wav.file);
    
    if (sd_file_exists(test_filename)) {
        e = sd_delete_file(test_filename);
        TEST_ASSERT_EQUAL(e_syserr_none, e);
    }
}

void test_wav_header_update(void) {
    const char* test_filename = SDCARD_BASE_PATH "/test_header_temp.wav";
    wav_file_t wav;
    e_syserr_t e;
    uint32_t start_time;
    
    if (!sd_is_mounted()) {
        TEST_FAIL_MESSAGE("SD card not mounted for WAV header update test");
        return;
    }
    
    e = wav_open_for_write(&wav, test_filename, 1, 44100, 16);
    audio_sample_t dummy_samples[100];
    for(int i = 0; i < 100; i++) {
        for(uint8_t ch = 0; ch < AUDIO_MAX_NUM_CH; ch++) {
            dummy_samples[i].ch[ch] = 0;
        }
    }
    e = wav_write_samples(&wav, dummy_samples, 100);
    TEST_ASSERT_EQUAL(e_syserr_none, e);    
    TEST_ASSERT_EQUAL(100, wav.samples_transfered);
    e = wav_close_for_write(&wav);
    TEST_ASSERT_EQUAL(e_syserr_none, e);    

    e = wav_open_for_read(&wav, test_filename);
    TEST_ASSERT_EQUAL(e_syserr_none, e);    
    
    uint32_t expected_data_size = 100 * 1 * (16 / 8);
    uint32_t expected_chunk_size = 36 + expected_data_size;
    
    TEST_ASSERT_EQUAL(expected_data_size, wav.header.subchunk2Size);
    TEST_ASSERT_EQUAL(expected_chunk_size, wav.header.chunkSize);
    
    e = wav_close_for_read(&wav);
    TEST_ASSERT_EQUAL(e_syserr_none, e);    
    
    if (sd_file_exists(test_filename)) {
        e = sd_delete_file(test_filename);
        TEST_ASSERT_EQUAL(e_syserr_none, e);    
    }
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_sdcard_setup);
    RUN_TEST(test_wav_create_header);
    RUN_TEST(test_wav_create_header_mono_8bit);
    RUN_TEST(test_wav_create_header_stereo_24bit);
    RUN_TEST(test_wav_create_header_chunk_ids);
    RUN_TEST(test_wav_create_header_pcm_format);
    RUN_TEST(test_wav_file_operations);
    RUN_TEST(test_wav_header_update);
    RUN_TEST(test_sdcard_cleanup);
    UNITY_END();
}

void loop() {
}
