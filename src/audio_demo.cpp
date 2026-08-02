#include <Arduino.h>
#include "jescore.h"
#include "audio.h"

static inline void feedthrough_cb(audio_io_t* iobuf){
    if(!iobuf->in || !iobuf->out) return;

    for(uint32_t i = 0; i < iobuf->len; i++){
        for(uint8_t ch = 0; ch < iobuf->nch; ch++){
            audio_sample_base_t sample = iobuf->in[i].ch[ch];

            // Insert single-channel per-sample DSP here if needed.
            // DSP FRX helpers can also operate on multichannel audio_sample_t arrays,
            // so this double loop expansion is optional.

            iobuf->out[i].ch[ch] = sample;
        }
    }
}

void setup(){
    jes_init();
    audio_init_default();
    audio_set_callback(feedthrough_cb);
    audio_start();
}

void loop(){
    /*
    This is a jescore compat-layer demo for the audio module.

    The firmware initializes the default I2S audio topology and starts a
    feedthrough callback that copies input blocks to output blocks.

    Install jescorecli: https://github.com/jesdev-io/jescorecli

    List of available calls:
    - stop: stop the audio sampler loop.
    - restart: restart audio with default settings.
    - restart -sr <44100|48000|96000>: restart with a sample rate override.
    - restart -bps <8|16|24|32>: restart with a bit-depth override.
    - restart -gain <0..1>: restart with a gain override.
    - vol <0..1>: set output gain. Values are clipped to 0..1.
    - mute: set output gain to 0.

    Want more? Contribute!
    https://github.com/jesdev-io/libFRX/issues
    https://github.com/jesdev-io/libFRX/pulls
    */
}
