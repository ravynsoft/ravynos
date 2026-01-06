/*
 * Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
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
#include "misc_util.h"

/*********************************************************************
*********************************************************************/
char * createUTF8CStringForCFString(CFStringRef aString)
{
    char * result = NULL;
    CFIndex bufferLength = 0;

    if (!aString) {
        goto finish;
    }

    bufferLength = sizeof('\0') +
        CFStringGetMaximumSizeForEncoding(CFStringGetLength(aString),
	    kCFStringEncodingUTF8);

    result = (char *)malloc(bufferLength * sizeof(char));
    if (!result) {
        goto finish;
    }
    if (!CFStringGetCString(aString, result, bufferLength,
        kCFStringEncodingUTF8)) {

        SAFE_FREE_NULL(result);
        goto finish;
    }

finish:
    return result;
}

/*********************************************************************
*********************************************************************/
CFStringRef createCFStringForData(CFDataRef aData, CFIndex maxBytes)
{
    CFMutableStringRef  result = NULL;
    const uint8_t     * bytes  = NULL;  // do not free
    CFIndex            count, i;
    
    result = CFStringCreateMutable(kCFAllocatorDefault, /* maxLength */ 0);
    if (!result) {
        goto finish;
    }
    
    count = CFDataGetLength(aData);

    CFStringAppend(result, CFSTR("<"));

    if (count) {
        bytes = CFDataGetBytePtr(aData);;
        for (i = 0; i < count && i < maxBytes; i++) {
            CFStringAppendFormat(result, /* options */ NULL, CFSTR("%02x%s"),
                (unsigned)(bytes[i]),
                (i > 0 && !((i + 1) % 4) && (i + 1 < count)) ? " " : "");
        }
        if (maxBytes < count) {
            CFStringAppendFormat(result, /* options */ NULL,
                CFSTR("...(%u bytes total)"), (unsigned)count);
        }
    }
    CFStringAppend(result, CFSTR(">"));

finish:
    return result;
}
