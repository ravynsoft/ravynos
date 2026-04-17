/*  Copyright © 2017-2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_logger.c
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 31/12/2017.
*/

#include "lf_hfs_logger.h"

os_log_t gpsHFSLog;
bool     gbIsLoggerInit = false;

const os_log_type_t gpeHFSToOsLevel [LEVEL_AMOUNT] = {
    [ LEVEL_DEBUG   ]       = OS_LOG_TYPE_DEBUG,
    [ LEVEL_DEFAULT ]       = OS_LOG_TYPE_DEFAULT,
    [ LEVEL_ERROR   ]       = OS_LOG_TYPE_ERROR
};

int
LFHFS_LoggerInit( void )
{
    int iErr = 0;
    if ( (gpsHFSLog = os_log_create("com.apple.filesystems.livefiles_hfs_plugin", "plugin")) == NULL) {
        iErr = 1;
    }
    else
    {
        gbIsLoggerInit = true;
    }

    return iErr;
}
