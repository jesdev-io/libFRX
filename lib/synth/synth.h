#ifndef _SYNTH_H_
#define _SYNTH_H_

#ifdef FRX_ENABLE_MODULE_SYNTH

#include <inttypes.h>
#include "audio.h"

#define SYNTH_ARGS_N 2

// Configuration macros - can be overridden by consuming projects
#ifndef SYNTH_SWEEP_DUR_S
#define SYNTH_SWEEP_DUR_S 1
#endif

#ifndef SYNTH_SWEEP_LEN
#define SYNTH_SWEEP_LEN  (48000 * SYNTH_SWEEP_DUR_S)
#endif

#ifndef SYNTH_SERVER_JOB_NAME
#define SYNTH_SERVER_JOB_NAME "synth"
#endif

#ifndef SYNTH_SERVER_JOB_MEM
#define SYNTH_SERVER_JOB_MEM 4096
#endif

/// @brief Types of supported synths as enum.
typedef enum synth_t{
    synth_sine,
    synth_square,
    synth_saw,
    synth_sweep,
    SYNTH_N
}synth_t;

/// @brief Synth config struct. Holds all information of current synth.
typedef struct synth_cfg_t{
    uint32_t freq;      // not used when sweep is selected
    float amp;
    synth_t type;       // see synth_t
    uint32_t fs;        // sampling frequency (must match peripheral)
    int32_t _fstart;    // only used by sweep (deprecated)
    int32_t _fstop;     // only used by sweep (deprecated)
    double d_phase;     // phase state for double
    uint32_t ul_phase;  // phase state for uint32_t
}synth_cfg_t;

/// @brief Concrete synth type computation function signature.
typedef void (*synth_write_t)(audio_sample_t* data, uint32_t n_samples);

#define SYNTH_CFG_DEFAULT ((synth_cfg_t){.freq = 1000,\
                                         .amp = 0.2,\
                                         .type = synth_sine,\
                                         .fs = 48000,\
                                         ._fstart = 20,\
                                         ._fstop = 20000,\
                                         .ul_phase = 0})

/// @brief Set a given synth config to the current synth.
/// @param cfg Reference to synth config struct.
/// @return Error code.
/// @note Referring to a local scope config is supported, because
///       values are copied and stored internally.
e_syserr_t synth_init(synth_cfg_t* cfg);

/// @brief Set a default synth config to the current synth.
/// @return Error code.
/// @note Is part of the common signature interface for the init routine.
e_syserr_t synth_init_default(void);

/// @brief Write synthetic data according to currently selected synth.
/// @param data Destination array of data.
/// @param n_samples Number of requested samples.
void synth_write(audio_sample_t* data, uint32_t n_samples);

/// @brief Write a sine wave according to config.
/// @param data Destination array of data.
/// @param n_samples Number of requested samples.
void synth_write_sine(audio_sample_t* data, uint32_t n_samples);

/// @brief Write a square wave according to config.
/// @param data Destination array of data.
/// @param n_samples Number of requested samples.
void synth_write_square(audio_sample_t* data, uint32_t n_samples);

/// @brief Write a saw wave according to config.
/// @param data Destination array of data.
/// @param n_samples Number of requested samples.
void synth_write_saw(audio_sample_t* data, uint32_t n_samples);

/// @brief Write an exponential sweep according to config.
/// @param data Destination array of data.
/// @param n_samples Number of requested samples.
void synth_write_sine_sweep(audio_sample_t* data, uint32_t n_samples);

/// @brief Synthesizer managing job. CLI callable.
/// @param p Pointer to job parameters.
void synth_job(void* p);

/// @brief Translate CLI args to the synth struct.
/// @param synth_args Whitespace delimited args from jescore.
/// @param pcfg Pointer to synth config to be set.
/// @return Error code.
e_syserr_t synth_job_parse_args(char* synth_args, synth_cfg_t* pcfg);

#endif // FRX_ENABLE_MODULE_SYNTH
#endif
