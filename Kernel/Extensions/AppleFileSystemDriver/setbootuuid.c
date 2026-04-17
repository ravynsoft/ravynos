/*
 *  setbootuuid.c
 *  AppleFileSystemDriver
 *
 *  Created by Shantonu Sen on 9/14/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach_error.h>
#include <err.h>

void usage(void);

int main(int argc, char *argv[]) {

    CFStringRef uuidstr;
    CFUUIDRef uuid;
    io_service_t ioresources = IO_OBJECT_NULL;
    kern_return_t kret;
    
    if (argc != 2) {
        usage();
    }

    uuidstr = CFStringCreateWithCString(kCFAllocatorDefault, argv[1], kCFStringEncodingUTF8);
    uuid = CFUUIDCreateFromString(kCFAllocatorDefault, uuidstr);
    if (!uuid) {
        errx(1, "Invalid UUID string %s", argv[1]);
    }

    CFRelease(uuid);
    
    ioresources = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching(kIOResourcesClass));
    if (ioresources == IO_OBJECT_NULL) {
        errx(1, "Can't find IOResources");
    }
    
    kret = IORegistryEntrySetCFProperty(ioresources, CFSTR("boot-uuid"), uuidstr);
    if (kret) {
        errx(1, "Can't set boot-uuid: %d %s", kret, mach_error_string(kret));
    }
    
    IOObjectRelease(ioresources);
    
    CFRelease(uuidstr);
    
    return 0;
}

void usage(void)
{
    fprintf(stderr, "Usage: %s 92DEB3E6-96DE-44CD-BE4F-152A4A4A2365\n", getprogname());
    exit(1);
}
