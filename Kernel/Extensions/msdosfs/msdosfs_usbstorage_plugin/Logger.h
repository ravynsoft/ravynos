/* Copyright © 2017 Apple Inc. All rights reserved.
 *
 * Logger.h
 * usbstorage_plugin
 *
 * Created by Yakov Ben Zaken on 29/10/2017.
 *
 */

#ifndef __Logger_h_
#define __Logger_h_

#include "Common.h"

typedef enum
{
    LEVEL_DEBUG,
    LEVEL_DEFAULT,
    LEVEL_ERROR,
    
    LEVEL_AMOUNT,
    
} MsDosLogsLevel_e;

extern os_log_t             gpsMsDosLog;         // Default log destination
extern const os_log_type_t  gpeMsDosToOsLevel[LEVEL_AMOUNT];
extern bool                 gbIsLoggerInit;


#define MSDOS_LOG( _level, ... )                                                        \
    do {                                                                                \
        if ( gbIsLoggerInit )                                                           \
        {                                                                               \
            os_log_with_type((gpsMsDosLog), gpeMsDosToOsLevel[_level], ##__VA_ARGS__);  \
        }                                                                               \
    } while(0)                                                                          \

int MSDOS_LoggerInit( void );

#endif /* __Logger_h_ */
