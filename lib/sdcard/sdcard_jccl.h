#ifndef _SDCARD_JCCL_H_
#define _SDCARD_JCCL_H_

#ifdef FRX_ENABLE_MODULE_SDCARD

#define SDCARD_JOB_NAME                 "sdcard"
#define SDCARD_STREAMER_JOB_NAME        "sdstrm"

#define SDCARD_CMD_MOUNT                "mnt"
#define SDCARD_CMD_UNMOUNT              "unmnt"
#define SDCARD_CMD_HELP                 "help"
#define SDCARD_CMD_LIST                 "ls"
#define SDCARD_CMD_READ                 "cat"
#define SDCARD_CMD_CREATE               "mk"
#define SDCARD_CMD_REMOVE               "rm"
#define SDCARD_CMD_MEMORY               "mem"

#define SDCARD_CMDS                     "Available commands: "\
                                        SDCARD_CMD_MOUNT ", "\
                                        SDCARD_CMD_UNMOUNT ", "\
                                        SDCARD_CMD_LIST ", "\
                                        SDCARD_CMD_READ ", "\
                                        SDCARD_CMD_CREATE ", "\
                                        SDCARD_CMD_REMOVE ", "\
                                        SDCARD_CMD_MEMORY
                                        

#define SDCARD_MSG_MOUNTED              "Mounted."                                     
#define SDCARD_MSG_MOUNT_FAIL           "Unable to mount SD card!"                  
#define SDCARD_MSG_UNMOUNTED            "Unmounted."                                 
#define SDCARD_MSG_UNMOUNT_FAIL         "Unable to unmount SD card!"              
#define SDCARD_MSG_LS_ERROR_errnum      "Error while listing files. (%d)"
#define SDCARD_MSG_CAT_ERROR_USAGE      "Specify a file to read."
#define SDCARD_MSG_CAT_ERROR_errnum     "Error while reading file. (%d)"             
#define SDCARD_MSG_MK_ERROR_errnum      "Error while creating file. (%d)"
#define SDCARD_MSG_MK_ERROR_USAGE       "Specify a file to create."
#define SDCARD_MSG_RM_ERROR_errnum      "Error while deleting file. (%d)" 
#define SDCARD_MSG_RM_ERROR_USAGE       "Specify a file to delete."            
#define SDCARD_MSG_MEM_FORMAT_free_tot  "%d/%d kB free"                             
#define SDCARD_MSG_MEM_ERROR_errnum     "Free space can't be identified. (%d)"
#define SDCARD_MSG_UNKNOWN_CMD          "Unknown SD command."

#endif // FRX_ENABLE_MODULE_SDCARD

#endif // _SDCARD_JCCL_H_