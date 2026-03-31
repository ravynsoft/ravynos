//
//  posix_utilities.c
//  mDNSResponder
//
//  Copyright (c) 2019 Apple Inc. All rights reserved.
//

#include "posix_utilities.h"
#include "mDNSEmbeddedAPI.h"
#include <stdlib.h>                 // for NULL
#include <stdio.h>                  // for snprintf
#include <time.h>
#include <sys/time.h>               // for gettimeofday

mDNSexport void getLocalTimestamp(char * const buffer, mDNSu32 buffer_len)
{
    struct timeval      now;
    struct tm           local_time;
    char                date_time_str[32];
    char                time_zone_str[32];

    gettimeofday(&now, NULL);
    localtime_r(&now.tv_sec, &local_time);

    strftime(date_time_str, sizeof(date_time_str), "%F %T", &local_time);
    strftime(time_zone_str, sizeof(time_zone_str), "%z", &local_time);
    snprintf(buffer, buffer_len, "%s.%06lu%s", date_time_str, (unsigned long)now.tv_usec, time_zone_str);
}
