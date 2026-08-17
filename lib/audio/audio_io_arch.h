#ifndef _AUDIO_IO_ARCH_H_
#define _AUDIO_IO_ARCH_H_

#if defined(FRX_ENABLE_MODULE_AUDIO) || defined(FRX_ENABLE_MODULE_DSP_FRX)

#ifndef AUDIO_IO_IN_CH
#define AUDIO_IO_IN_CH          2
#endif

#ifndef AUDIO_IO_OUT_CH
#define AUDIO_IO_OUT_CH         2
#endif

#define AUDIO_IO_BUS_ARCH_SHARED    1
#define AUDIO_IO_BUS_ARCH_SPLIT     2

#ifndef AUDIO_IO_BUS_ARCH
#define AUDIO_IO_BUS_ARCH AUDIO_IO_BUS_ARCH_SHARED
#endif

#if AUDIO_IO_IN_CH < 0 || AUDIO_IO_IN_CH > 4
#error "AUDIO_IO_IN_CH must be in range 0..4"
#endif

#if AUDIO_IO_OUT_CH < 0 || AUDIO_IO_OUT_CH > 4
#error "AUDIO_IO_OUT_CH must be in range 0..4"
#endif

#if AUDIO_IO_IN_CH == 0 && AUDIO_IO_OUT_CH == 0
#error "At least one audio input or output channel must be configured"
#endif

#if AUDIO_IO_IN_CH > 0 && AUDIO_IO_OUT_CH > 0 && AUDIO_IO_IN_CH != AUDIO_IO_OUT_CH
#error "Audio input/output channel counts must match unless one side is disabled"
#endif

#if AUDIO_IO_BUS_ARCH != AUDIO_IO_BUS_ARCH_SHARED && AUDIO_IO_BUS_ARCH != AUDIO_IO_BUS_ARCH_SPLIT
#error "Unsupported AUDIO_IO_BUS_ARCH"
#endif

#if AUDIO_IO_BUS_ARCH == AUDIO_IO_BUS_ARCH_SPLIT && (AUDIO_IO_IN_CH > 2 || AUDIO_IO_OUT_CH > 2)
#error "Split input/output I2S bus architecture supports at most two channels per side"
#endif

#ifndef AUDIO_CFG_HAS_INPUT
#if AUDIO_IO_IN_CH > 0
#define AUDIO_CFG_HAS_INPUT     1
#else
#define AUDIO_CFG_HAS_INPUT     0
#endif
#endif

#ifndef AUDIO_CFG_HAS_OUTPUT
#if AUDIO_IO_OUT_CH > 0
#define AUDIO_CFG_HAS_OUTPUT    1
#else
#define AUDIO_CFG_HAS_OUTPUT    0
#endif
#endif

#ifndef AUDIO_MAX_NUM_CH
#if AUDIO_IO_IN_CH > AUDIO_IO_OUT_CH
#define AUDIO_MAX_NUM_CH        AUDIO_IO_IN_CH
#else
#define AUDIO_MAX_NUM_CH        AUDIO_IO_OUT_CH
#endif
#endif

#endif // defined(FRX_ENABLE_MODULE_AUDIO) || defined(FRX_ENABLE_MODULE_DSP_FRX)

#endif // _AUDIO_IO_ARCH_H_
