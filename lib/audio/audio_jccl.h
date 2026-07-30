#ifndef _AUDIO_JCCL_H_
#define _AUDIO_JCCL_H_

#ifdef FRX_ENABLE_MODULE_AUDIO

#define AUDIO_SERVER_JOB_NAME   "audiosrv"
#define AUDIO_CONTROL_JOB_NAME  "audioctrl"

#define AUDIO_CMD_RESTART       "restart"
#define AUDIO_CMD_STOP          "stop"
#define AUDIO_CMD_VOLUME        "vol"
#define AUDIO_CMD_MUTE          "mute"
#define AUDIO_CMD_STATUS        "status"

#define AUDIO_OPT_SR            "sr"
#define AUDIO_OPT_SR_DASH       "-sr"
#define AUDIO_OPT_BPS           "bps"
#define AUDIO_OPT_BPS_DASH      "-bps"
#define AUDIO_OPT_GAIN          "gain"
#define AUDIO_OPT_GAIN_DASH     "-gain"

#define AUDIO_CMDS              "Commands: "\
                                AUDIO_CMD_RESTART ", "\
                                AUDIO_CMD_STOP ", "\
                                AUDIO_CMD_VOLUME " <0..1>, "\
                                AUDIO_CMD_MUTE ", "\
                                AUDIO_CMD_STATUS
#define AUDIO_RESTART_USAGE     AUDIO_CMD_RESTART " opts: [sr|-sr] <hz> [bps|-bps] <bits> [gain|-gain] <0..1>"

#define AUDIO_MSG_UNKNOWN_CMD   "Unknown audio command."
#define AUDIO_MSG_OFFLINE       "Audio already offline."
#define AUDIO_MSG_ERROR_USAGE   "Audio command usage error."
#define AUDIO_MSG_ERROR_ERRNUM  "Audio command failed with error %d."
#define AUDIO_MSG_RESTARTED     "Audio restarted."
#define AUDIO_MSG_STOPPED       "Audio stopped."
#define AUDIO_MSG_VOLUME        "Audio volume set to %d/1000."
#define AUDIO_MSG_STATUS        "Audio status: running=%u sr=%lu bps=%u gain=%d/1000 nch=%u banks=%u."

#endif // FRX_ENABLE_MODULE_AUDIO

#endif // _AUDIO_JCCL_H_