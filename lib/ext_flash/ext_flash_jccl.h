#ifndef _EXT_FLASH_JCCL_H_
#define _EXT_FLASH_JCCL_H_

#ifdef FRX_ENABLE_MODULE_EXT_FLASH

#define EF_CMD_ROM                       "rom"

#define EF_MSG_ERROR_NO_CMD              "No command specified for ef job."
#define EF_MSG_ERROR_READ_ROM            "Error while reading external flash ROM!"
#define EF_MSG_ERROR_READ_PID            "Error while reading external flash PID!"
#define EF_MSG_PID_FORMAT                "PID: 0x%X"
#define EF_MSG_SN_FORMAT                 "SN: %d"
#define EF_MSG_FW_FORMAT                 "FW: %d.%d%c"
#define EF_MSG_UT_FORMAT                 "UT: %d"

#endif // FRX_ENABLE_MODULE_EXT_FLASH
#endif // _EXT_FLASH_JCCL_H_
