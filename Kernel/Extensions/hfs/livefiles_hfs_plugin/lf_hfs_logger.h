/*  Copyright © 2018 Apple Inc. All rights reserved.
 *
 *  lf_hfs_logger.h
 *  livefiles_hfs
 *
 *  Created by Yakov Ben Zaken on 20/03/2018.
 */

#ifndef lf_hfs_logger_h
#define lf_hfs_logger_h

#include <os/log.h>

typedef enum
{
    LEVEL_DEBUG,
    LEVEL_DEFAULT,
    LEVEL_ERROR,
    LEVEL_AMOUNT,

} HFSLogLevel_e;

extern os_log_t             gpsHFSLog;
extern const os_log_type_t  gpeHFSToOsLevel[ LEVEL_AMOUNT ];
extern bool                 gbIsLoggerInit;


#define LFHFS_LOG( _level, ... )                                                          \
    do {                                                                                \
        if ( gbIsLoggerInit )                                                           \
        {                                                                               \
            os_log_with_type((gpsHFSLog), gpeHFSToOsLevel[_level], ##__VA_ARGS__);  \
        }                                                                               \
    } while(0)

int LFHFS_LoggerInit( void );

#endif /* lf_hfs_logger_h */
