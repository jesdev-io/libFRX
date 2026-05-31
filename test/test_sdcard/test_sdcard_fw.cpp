#include <Arduino.h>
#include <jescore.h>
#include <unity.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "sdcard.h"
#include "utils.h"

#define SD_TEST_MAX_FILES           5
#define SD_TEST_MAX_FREQ            0 // uses default
#define SD_TEST_FNAME_TXT           "unit_test.txt"
#define SD_TEST_FNAME_BIN           "unit_test.bin"
#define SD_TEST_TEXT_BUF_LEN        24
#define SD_TEST_BIN_BUF_INT32_LEN   4 // 16 byte
#define SD_TEST_MSG_TXT             "Written by unit test.\n\r"
#define SD_TEST_MSG_TXT_APX         "Appended by unit test\n\r"

void test_jes_bootup(void){
    jes_err_t e = jes_init();
    TEST_ASSERT_EQUAL(e_err_no_err, e);
}

void test_sd_init(void){
    e_syserr_t e = sd_init(SD_TEST_MAX_FILES, SD_TEST_MAX_FREQ);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_sd_mnt(void){
    TEST_ASSERT_EQUAL(0, sd_is_mounted());
    e_syserr_t e = sd_mnt();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(1, sd_is_mounted());
}

void test_sd_unmnt(void){
    TEST_ASSERT_EQUAL(1, sd_is_mounted());
    e_syserr_t e = sd_unmnt();
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(0, sd_is_mounted());
}

void test_sd_get_free_kbytes(void){
    uint32_t free_bytes;
    uint32_t all_bytes;
    e_syserr_t e = sd_get_free_kbytes(&free_bytes, &all_bytes);
    TEST_ASSERT_EQUAL(e, e_syserr_none);
    TEST_ASSERT_GREATER_THAN_INT32(0, free_bytes);
    TEST_ASSERT_GREATER_THAN_INT32(0, all_bytes);
    TEST_ASSERT_GREATER_THAN_INT32(free_bytes, all_bytes);
}

void test_sd_create_file(void){
    e_syserr_t e = sd_create_file((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_TXT);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

void test_sd_delete_file(void){
    e_syserr_t e = sd_delete_file((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_TXT);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
}

char text[SD_TEST_TEXT_BUF_LEN] = SD_TEST_MSG_TXT;
int32_t bin[SD_TEST_BIN_BUF_INT32_LEN] = {-1, 0, 1, 2};
void test_sd_write(void){
    uint32_t points_w = 0;
    e_syserr_t e = sd_write_txt(text, 
                                SD_TEST_TEXT_BUF_LEN, 
                                (char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_TXT,
                                0,
                                &points_w);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(points_w, SD_TEST_TEXT_BUF_LEN - 1); // "\0" is not written

    e = sd_write((void*)bin,
                 sizeof(int32_t),
                 SD_TEST_BIN_BUF_INT32_LEN,
                 (char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN,
                 "wb",
                 0,
                 &points_w);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(points_w, SD_TEST_BIN_BUF_INT32_LEN);
}

void test_sd_read(void){
    char text_comp[SD_TEST_TEXT_BUF_LEN] = {0};
    int32_t bin_comp[SD_TEST_BIN_BUF_INT32_LEN] = {0};
    uint32_t points_r = 0;
    e_syserr_t e = sd_read_txt(text_comp, 
                               SD_TEST_TEXT_BUF_LEN, 
                               (char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_TXT,
                               0,
                               &points_r);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL_STRING(text, text_comp);
    TEST_ASSERT_EQUAL(points_r, SD_TEST_TEXT_BUF_LEN - 1); // "\0" has not been written, therefore is not read (but appended in memory)
    TEST_ASSERT_EQUAL(strlen(text_comp), SD_TEST_TEXT_BUF_LEN - 1); // does not read "\0", but if the assertion is true, "\0" is at the end of the string

    e = sd_read((void*)bin_comp,
                sizeof(int32_t),
                SD_TEST_BIN_BUF_INT32_LEN,
                (char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN,
                "rb",
                0,
                &points_r);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL_MEMORY(bin, bin_comp, SD_TEST_BIN_BUF_INT32_LEN * sizeof(int32_t));
    TEST_ASSERT_EQUAL(points_r, SD_TEST_BIN_BUF_INT32_LEN);
}

char txt_apx[SD_TEST_TEXT_BUF_LEN] = SD_TEST_MSG_TXT_APX;
int32_t bin_apx[SD_TEST_BIN_BUF_INT32_LEN] = {2, 1, 0, -1};
void test_sd_append(void){
    uint32_t points_w = 0;
    e_syserr_t e = sd_append_txt((void*)txt_apx,
                                 SD_TEST_TEXT_BUF_LEN,
                                 (char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_TXT,
                                 &points_w);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(points_w, SD_TEST_TEXT_BUF_LEN-1);

    char text_comp[SD_TEST_TEXT_BUF_LEN*2] = {0};
    uint32_t points_r = 0;
    e = sd_read_txt(text_comp, 
                    SD_TEST_TEXT_BUF_LEN*2 - 1, // first original "\0" is missing 
                    (char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_TXT,
                    0,
                    &points_r);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    
    char combined_text[SD_TEST_TEXT_BUF_LEN*2] = {0};
    sprintf(combined_text, "%s%s", text, txt_apx); 
    TEST_ASSERT_EQUAL(points_r, SD_TEST_TEXT_BUF_LEN*2 - 2); // both "\0" were never written
    TEST_ASSERT_EQUAL_STRING(combined_text, text_comp);

    e = sd_append((void*)bin_apx,
                  sizeof(int32_t),
                  SD_TEST_BIN_BUF_INT32_LEN,
                  (char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN,
                  &points_w);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(points_w, SD_TEST_BIN_BUF_INT32_LEN);

    int32_t bin_comp[SD_TEST_BIN_BUF_INT32_LEN*2] = {0};
    e = sd_read((void*)bin_comp,
                sizeof(int32_t),
                SD_TEST_BIN_BUF_INT32_LEN*2,
                (char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN,
                "rb",
                0,
                &points_r);
    
    int32_t combined_bin[SD_TEST_BIN_BUF_INT32_LEN*2] = {0};
    memcpy(combined_bin, bin, SD_TEST_BIN_BUF_INT32_LEN * sizeof(int32_t));
    memcpy(combined_bin+SD_TEST_BIN_BUF_INT32_LEN, bin_apx, SD_TEST_BIN_BUF_INT32_LEN * sizeof(int32_t));
    TEST_ASSERT_EQUAL(points_r, SD_TEST_BIN_BUF_INT32_LEN*2);
    TEST_ASSERT_EQUAL_MEMORY(combined_bin, bin_comp, SD_TEST_BIN_BUF_INT32_LEN * 2 * sizeof(int32_t));
}

FILE* f = NULL; // out of scope f-pointer (must be properly managed between tests)
void test_sd_stream_open(void){
    f = sd_stream_open((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN, "rb");
    TEST_ASSERT_NOT_NULL(f);
    sd_stream_close(f);
    f = NULL;
    
    f = sd_stream_read_open((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN);
    TEST_ASSERT_NOT_NULL(f);
    sd_stream_close(f);
    f = NULL;
    
    f = sd_stream_write_open((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN);
    TEST_ASSERT_NOT_NULL(f);
    sd_stream_close(f);
    f = NULL;
}

// Use generic data instead of audio-specific types
int32_t data_chunk_1[2] = {100, 200};
int32_t data_chunk_2[2] = {-100, -200};
int32_t data_chunk[4]; // 2 samples * 2 channels
void test_sd_stream_write(void){
    // Ensure any previously opened file is closed
    if (f != NULL) {
        sd_stream_close(f);
        f = NULL;
    }
    
    f = sd_stream_write_open((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN);
    TEST_ASSERT_NOT_NULL(f);
    e_syserr_t e;
    // Prepare generic data (2 samples, 2 channels each)
    for(uint8_t i = 0; i < 2; i++){
        data_chunk[i*2] = data_chunk_1[i];    // Channel 1
        data_chunk[i*2+1] = data_chunk_2[i];  // Channel 2
    }
    for(uint8_t i = 0; i < 2; i++){
        uint32_t pw = 0;
        e = sd_stream_in(data_chunk, 2, sizeof(int32_t)*8, 2, f, &pw);
        TEST_ASSERT_EQUAL(e_syserr_none, e);
        TEST_ASSERT_EQUAL(2, pw);
    }
    sd_stream_close(f);
    f = NULL; // Explicitly set to NULL after closing
}

void test_sd_stream_read(void){
    // Ensure any previously opened file is closed
    if (f != NULL) {
        sd_stream_close(f);
        f = NULL;
    }
    
    f = sd_stream_read_open((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN);
    TEST_ASSERT_NOT_NULL(f);
    int32_t data[8]; // 2 samples * 2 channels * 2 reads
    e_syserr_t e;
    for(uint8_t i = 0; i < 2; i++){
        uint32_t pr = 0;
        e = sd_stream_out(data, 2, sizeof(int32_t)*8, 2, f, &pr);
        TEST_ASSERT_EQUAL(e_syserr_none, e);
        TEST_ASSERT_EQUAL(2, pr);
        if(i > 4){  // skip the previous data, only test the data from test_sd_stream_write
            TEST_ASSERT_EQUAL_MEMORY(data_chunk, data, 4);
        }
    }
    sd_stream_close(f);
    f = NULL; // Explicitly set to NULL after closing
}

void test_sd_ls(){
    char content[SDCARD_LS_MAX_CHAR] = {0};
    e_syserr_t e = sd_ls((char*)SDCARD_BASE_PATH,
                          content,
                          SDCARD_LS_MAX_CHAR);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    strremove(content, "fr2_rec_0000.wav\nfr2_rec_0001.wav\n"); // created in rec state UT
    TEST_ASSERT_EQUAL_STRING(SD_TEST_FNAME_TXT "\n" SD_TEST_FNAME_BIN "\n", content);
}

void test_sd_cat(){
    char content[SDCARD_CAT_MAX_CHAR] = {0};
    e_syserr_t e = sd_cat((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_TXT,
                          content,
                          SDCARD_CAT_MAX_CHAR);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL_STRING(content, SD_TEST_MSG_TXT SD_TEST_MSG_TXT_APX);
}

void test_sd_file_exists(){
    TEST_ASSERT_EQUAL(1, sd_file_exists((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_TXT));
    TEST_ASSERT_EQUAL(1, sd_file_exists((char*)SDCARD_BASE_PATH "/" SD_TEST_FNAME_BIN));
    TEST_ASSERT_EQUAL(0, sd_file_exists((char*)SDCARD_BASE_PATH "/" "somefile.txt"));
    TEST_ASSERT_EQUAL(0, sd_file_exists((char*)"wrong_path.txt"));
}

void test_sd_get_unique_fname(){
    char fname[] = SDCARD_BASE_PATH "/" SDCARD_DEFAULT_FNAME_WAV;
    uint32_t points_w;
    e_syserr_t e = sd_write_txt((char*)"this is fake wav content.", 
                                26, 
                                fname,
                                0,
                                &points_w);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(25, points_w);
    e = sd_get_unique_fname(fname);
    char new_fname[] = SDCARD_BASE_PATH "/" "fr2_rec_0001.wav";
    TEST_ASSERT_EQUAL_STRING(new_fname, fname);
    TEST_ASSERT_EQUAL(e_syserr_none, e);

    e = sd_write_txt((char*)"more fake wav content.", 
                                23, 
                                new_fname,
                                0,
                                &points_w);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    TEST_ASSERT_EQUAL(22, points_w);
    e = sd_get_unique_fname(fname);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    char newer_fname[] = SDCARD_BASE_PATH "/" "fr2_rec_0002.wav";
    TEST_ASSERT_EQUAL_STRING(newer_fname, fname);

    TEST_ASSERT_EQUAL(1, sd_file_exists(SDCARD_BASE_PATH "/" SDCARD_DEFAULT_FNAME_WAV));
    TEST_ASSERT_EQUAL(1, sd_file_exists((const char*)new_fname));

    e = sd_delete_file(SDCARD_BASE_PATH "/" SDCARD_DEFAULT_FNAME_WAV);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    e = sd_delete_file((const char*)new_fname);
    TEST_ASSERT_EQUAL(e_syserr_none, e);

    TEST_ASSERT_EQUAL(0, sd_file_exists(SDCARD_BASE_PATH "/" SDCARD_DEFAULT_FNAME_WAV));
    TEST_ASSERT_EQUAL(0, sd_file_exists((const char*)new_fname));
}

void test_cleanup(){
    char content[SDCARD_LS_MAX_CHAR] = {0};
    char abspath[64] = {0};
    e_syserr_t e = sd_ls((char*)SDCARD_BASE_PATH,
                          content,
                          SDCARD_LS_MAX_CHAR);
    TEST_ASSERT_EQUAL(e_syserr_none, e);
    // strremove(content, "fr2_rec_0000.wav\nfr2_rec_0001.wav\n"); // created in rec state UT)
    char* fname = strtok(content, "\n");
    while(fname){
        sprintf(abspath, "%s%s", SDCARD_BASE_PATH "/", fname);
        e = sd_delete_file(abspath);
        TEST_ASSERT_EQUAL(e_syserr_none, e);
        fname = strtok(NULL, "\n");
    }
}

void setup(){
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_jes_bootup);
    RUN_TEST(test_sd_init);
    RUN_TEST(test_sd_mnt);
    RUN_TEST(test_sd_get_free_kbytes);
    RUN_TEST(test_sd_create_file);
    RUN_TEST(test_sd_write);
    RUN_TEST(test_sd_read);
    RUN_TEST(test_sd_append);
    RUN_TEST(test_sd_stream_open);
    RUN_TEST(test_sd_stream_write);
    RUN_TEST(test_sd_stream_read);
    RUN_TEST(test_sd_ls);
    RUN_TEST(test_sd_cat);
    RUN_TEST(test_sd_file_exists);
    RUN_TEST(test_sd_get_unique_fname);
    RUN_TEST(test_sd_delete_file);
    RUN_TEST(test_cleanup);
    RUN_TEST(test_sd_unmnt);
    UNITY_END();
}

void loop(){

}
