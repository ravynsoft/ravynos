/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
/*
cc -o /tmp/iodt graphics.subproj/IODisplayTest.c -framework CoreGraphics -Wall -undefined warning -g
*/

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphicsPrivate.h>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <stdlib.h>

#include "IODisplayInterface.h"

void KeyArrayCallback(const void *key, const void *value, void *context)
{
    CFArrayAppendValue(context, key);
}

int main(int argc, char * argv[])
{
    io_service_t        service;
    CFDictionaryRef     dict;
    CFDictionaryRef     names;
    CFArrayRef          langKeys;
    CFArrayRef          orderLangKeys;
    CFNumberRef         num;
    CGSError            err;
    SInt32              numValue;
    int                 i;
    CGSDisplayNumber    max;
    CGSDisplayID        displayIDs[8];
    CGSInitialize();

    err = CGSGetDisplayList(8, displayIDs, &max);
    if(err != kCGSErrorSuccess)
        exit(1);
    if(max > 8)
        max = 8;

    for(i = 0; i < max; i++ ) {
        err = CGSServiceForDisplayNumber(displayIDs[i], &service);
        if(err != kCGSErrorSuccess)
            continue;
    
        dict = IODisplayCreateInfoDictionary(service, kNilOptions );
        if( dict) {
            printf("Display %d\n", i);

            num = CFDictionaryGetValue(dict, CFSTR(kDisplayVendorID));
            if(num) {
                CFNumberGetValue(num, kCFNumberSInt32Type, &numValue);
                printf("vendorID %ld\n", numValue);
            }
            num = CFDictionaryGetValue(dict, CFSTR(kDisplayProductID));
            if(num) {
                CFNumberGetValue(num, kCFNumberSInt32Type, &numValue);
                printf("productID %ld\n", numValue);
            }

            names = CFDictionaryGetValue(dict, CFSTR(kDisplayProductName));
            langKeys = CFArrayCreateMutable(kCFAllocatorDefault, 0,
                                            &kCFTypeArrayCallBacks);
            CFDictionaryApplyFunction(names, KeyArrayCallback, (void *) langKeys);
            orderLangKeys = CFBundleCopyPreferredLocalizationsFromArray(langKeys);
            CFRelease(langKeys);
            if(orderLangKeys && CFArrayGetCount(orderLangKeys)) {
                char            cName[256];
                CFStringRef     langKey;
                CFStringRef     localName;

                langKey = CFArrayGetValueAtIndex(orderLangKeys, 0);
                localName = CFDictionaryGetValue(names, langKey);
                CFStringGetCString(localName, cName, sizeof(cName),
                                    CFStringGetSystemEncoding());
                printf("local name \"%s\"\n", cName);
            }
            CFRelease(orderLangKeys);

            printf("\nAll info:\n");
            CFShow(dict);

            CFRelease(dict);
        }

        // --
        {
            IOCFPlugInInterface ** interface;
            IODisplayCreateInterface( service, kNilOptions,
                                        kIODisplayControlInterfaceID, &interface );
        }
        {
            CFMutableDictionaryRef      dict;
            CFNumberRef                 num;
            SInt32                      value;

            dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0,
                                                &kCFTypeDictionaryKeyCallBacks,
                                                &kCFTypeDictionaryValueCallBacks );

            for( value = 0; value < 0x60; value++) {
                num = CFNumberCreate( kCFAllocatorDefault, kCFNumberSInt32Type, &value );
                CFDictionarySetValue( dict, CFSTR("contrast"), num );
                CFRelease(num);
                err = IODisplaySetParameters( service, kNilOptions, dict );
            }
            CFRelease(dict);
        }
        // --
    }
    
    exit(0);
    return(0);
}

