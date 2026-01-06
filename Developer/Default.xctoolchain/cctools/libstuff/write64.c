/*
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef HAVE_WRITE64 /* cctools-port */

#include "stuff/write64.h"

ssize_t write64(int fildes, const void *buf, size_t nbyte)
{
    unsigned char* uchars = (unsigned char*)buf;
    ssize_t total = 0;
    
    while (nbyte)
    {
        /*
         * If we were writing socket- or stream-safe code we'd chuck the
         * entire buf to write(2) and then gracefully re-request bytes that
         * didn't get written. But write(2) will return EINVAL if you ask it to
         * write more than 2^31-1 bytes. So instead we actually need to throttle
         * the input to write.
         *
         * Historically code using write(2) to write to disk will assert that
         * that all of the requested bytes were written. It seems harmless to
         * re-request bytes as one does when writing to streams, with the
         * compromise that we will return immediately when write(2) returns 0
         * bytes written.
         */
        size_t limit = 0x7FFFFFFF;
        size_t towrite = nbyte < limit ? nbyte : limit;
        ssize_t wrote = write(fildes, uchars, towrite);
        if (-1 == wrote)
        {
            return -1;
        }
        else if (0 == wrote)
        {
            break;
        }
        else
        {
            nbyte -= wrote;
            uchars += wrote;
            total += wrote;
        }
    }
    
    return total;
}

#endif /* ! HAVE_WRITE64 */
