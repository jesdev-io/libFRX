#ifndef WAV_H
#define WAV_H

#ifdef FRX_ENABLE_MODULE_WAV

#include <stdint.h>
#include <stdio.h>
#include "syserr.h"
#include "audio.h"

// Configuration macros - can be overridden by consuming projects
#ifndef WAV_HEADER_SIZE
#define WAV_HEADER_SIZE 44
#endif

#ifndef WAV_HEADER_PAD
#define WAV_HEADER_PAD 468
#endif

#ifndef WAV_HEADER_SIZE_TOTAL
#define WAV_HEADER_SIZE_TOTAL (WAV_HEADER_SIZE + WAV_HEADER_PAD)
#endif

#ifndef WAV_FN_LEN 
#define WAV_FN_LEN 256
#endif

/// @brief In-memory wav-header struct.
typedef struct {
    char     chunkID[4];                   // "RIFF"
    uint32_t chunkSize;                    // File size - 8
    char     format[4];                    // "WAVE"
    char     subchunk1ID[4];               // "fmt "
    uint32_t subchunk1Size;                // 16 for PCM
    uint16_t audioFormat;                  // 1 for PCM
    uint16_t numChannels;                  // 1 for mono, 2 for stereo
    uint32_t sampleRate;                   // 44100, 16000, etc.
    uint32_t byteRate;                     // sampleRate * numChannels * bitsPerSample/8
    uint16_t blockAlign;                   // numChannels * bitsPerSample/8
    uint16_t bitsPerSample;                // 8, 16, 24, etc.
    char     subchunk2ID[4];               // "data"
    uint32_t subchunk2Size;                // data size
    uint8_t  padding[WAV_HEADER_PAD];      // padding to 512 bytes
} wav_hdr_t;

/// @brief In-memory wav file context struct.
typedef struct {
    char filename[WAV_FN_LEN];
    FILE* file;
    wav_hdr_t header;
    uint32_t samples_transfered;
} wav_file_t;

/// @brief Create a WAV header.
/// @param numChannels Number of audio channels.
/// @param sampleRate Sample rate for audio.
/// @param bitsPerSample Sample resolution.
/// @return Filled `wav_hdr_t` struct (by value).
wav_hdr_t wav_create_header(uint16_t numChannels, uint32_t sampleRate, uint16_t bitsPerSample);

/// @brief Open a new WAV file and write the header.
/// @param wav Empty wav file context.
/// @param filename Name of wav file in FS.
/// @param numChannels Number of audio channels.
/// @param sampleRate Sample rate for audio.
/// @param bitsPerSample Sample resolution.
/// @return Error code.
/// @note Passes parameters to `wav_create_header()`.
e_syserr_t wav_open_for_write(wav_file_t* wav, const char* filename, uint16_t numChannels, uint32_t sampleRate, uint16_t bitsPerSample);

/// @brief Open a new WAV file and read the header.
/// @param wav Existing wav file context.
/// @param filename Name of wav file in FS.
/// @return Error code.
/// @note Stores header in `wav->header` and sets read pointer to start of data with `fseek()`.
e_syserr_t wav_open_for_read(wav_file_t* wav, const char* filename);

/// @brief Write audio samples to the WAV file.
/// @param wav Existing wav file context.
/// @param samples Data to write.
/// @param sample_count Amount of data to write.
/// @return Error code.
e_syserr_t wav_write_samples(wav_file_t* wav, const void* samples, uint32_t sample_count);

/// @brief Read audio samples from the WAV file.
/// @param wav Existing wav file context.
/// @param samples Data to read.
/// @param sample_count Amount of data to read.
/// @return Error code.
e_syserr_t wav_read_samples(wav_file_t* wav, const void* samples, uint32_t sample_count);

/// @brief Close the freshly written WAV file and update the header.
/// @param wav Existing wav file context.
/// @return Error code.
e_syserr_t wav_close_for_write(wav_file_t* wav);

/// @brief Close the freshly read WAV file and update the header.
/// @param wav Existing wav file context.
/// @return Error code.
e_syserr_t wav_close_for_read(wav_file_t* wav);

/// @brief Update the WAV header with the final data size
/// @param wav Existing wav file context.
/// @return Error code.
e_syserr_t wav_update_header(wav_file_t* wav);

#endif // FRX_ENABLE_MODULE_WAV
#endif // WAV_H
