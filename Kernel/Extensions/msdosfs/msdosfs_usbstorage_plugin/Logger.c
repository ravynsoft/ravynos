/* Copyright © 2017 Apple Inc. All rights reserved.
*
* Logger.c
* usbstorage_plugin
*
* Created by Yakov Ben Zaken on 29/10/2017.
*
*/

#include "Logger.h"

os_log_t gpsMsDosLog;
bool     gbIsLoggerInit = false;

const os_log_type_t gpeMsDosToOsLevel [LEVEL_AMOUNT] = {
    [ LEVEL_DEBUG   ]       = OS_LOG_TYPE_DEBUG,
    [ LEVEL_DEFAULT ]       = OS_LOG_TYPE_DEFAULT,
    [ LEVEL_ERROR   ]       = OS_LOG_TYPE_ERROR
};

int
MSDOS_LoggerInit( void )
{
    int iErr = 0;
    if ( (gpsMsDosLog = os_log_create("com.apple.filesystems.msdosfs_usbstorage", "plugin")) == NULL) {
        iErr = 1;
    }
    else
    {
        gbIsLoggerInit = true;
    }
    
    return iErr;
}
