#include <Arduino.h>
#include <jescore.h>
#include "wav.h"
#include "syserr.h"

#ifdef UNIT_TEST
#include <unity.h>
#else
#define TEST_ASSERT_EQUAL(ex, ac)
#define RUN_TEST(test) test()
#endif

void test_wav_create_header(void) {
    wav_hdr_t header = wav_create_header(2, 48000, 16);
    TEST_ASSERT_EQUAL(2, header.numChannels);
    TEST_ASSERT_EQUAL(48000, header.sampleRate);
    TEST_ASSERT_EQUAL(16, header.bitsPerSample);
    TEST_ASSERT_EQUAL(4, header.byteRate / 1000); // ~4 bytes per sample * 48000 = 192000
    TEST_ASSERT_EQUAL(4, header.blockAlign); // 2 channels * 16 bits / 8 = 4 bytes
}

void setup() {
    #ifdef UNIT_TEST
    delay(2000);
    UNITY_BEGIN();
    #endif
    
    RUN_TEST(test_wav_create_header);

    #ifdef UNIT_TEST
    UNITY_END();
    #endif
}

void loop() {
}
