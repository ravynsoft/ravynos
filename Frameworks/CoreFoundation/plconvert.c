/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

// Mac OS X: clang -F<path-to-CFLite-framework> -framework CoreFoundation Examples/plconvert.c -o plconvert
//  note: When running this sample, be sure to set the environment variable DYLD_FRAMEWORK_PATH to point to the directory containing your new version of CoreFoundation.
//   e.g.
//  DYLD_FRAMEWORK_PATH=/tmp/CF-Root ./plconvert <input> <output>
// 
// Linux: clang -I/usr/local/include -L/usr/local/lib -lCoreFoundation plconvert.c -o plconvert

/*
 This example shows usage of CFString, CFData, and other CFPropertyList types. It takes two arguments:
    1. A property list file to read, in either binary or XML property list format.
    2. A file name to write a converted property list file to.
 If the first input is binary, the output is XML and vice-versa.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <CoreFoundation/CoreFoundation.h>

static void logIt(CFStringRef format, ...) {
    va_list args;
    va_start(args, format); 
    CFStringRef str = CFStringCreateWithFormatAndArguments(kCFAllocatorSystemDefault, NULL, format, args);
    if (!str) return;
    
    CFIndex blen = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8);
    char *buf = str ? (char *)malloc(blen + 1) : 0;
    if (buf) {
        Boolean converted = CFStringGetCString(str, buf, blen, kCFStringEncodingUTF8);
        if (converted) {
            // null-terminate
            buf[blen] = 0;
            printf("%s\n", buf);
        }
    }
    if (buf) free(buf);
    if (str) CFRelease(str);      va_end(args);
}

static CFMutableDataRef createDataFromFile(const char *fname) {
    int fd = open(fname, O_RDONLY);
    CFMutableDataRef res = CFDataCreateMutable(kCFAllocatorSystemDefault, 0);
    char buf[4096];
    
    ssize_t amountRead;
    while ((amountRead = read(fd, buf, 4096)) > 0) {
        CFDataAppendBytes(res, (const UInt8 *)buf, amountRead);
    }
    
    close(fd);
    return res;
}

static bool writeDataToFile(CFDataRef data, const char *fname) {
    int fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0) {
        printf("There was an error creating the file: %d", errno);
        return false;
    }
    int dataLen = CFDataGetLength(data);
    int w = write(fd, CFDataGetBytePtr(data), dataLen);
    fsync(fd);
    close(fd);
    if (w != dataLen) return false;
    return true;
}

int main(int argc, char **argv) {
    
    if (argc != 3) {
        printf("Usage: plconvert <in file> <out file>\nIf the in file is an XML property list, convert to binary property list in out file. If the in file is a binary property list, convert to XML property list in out file.\n");
    } else {
        CFMutableDataRef plistData = createDataFromFile(argv[1]);
        if (!plistData) {
            printf("Unable to create data from file name: %s", argv[1]);
        } else {
            CFPropertyListFormat fmt;
            CFErrorRef err;
            CFPropertyListRef plist = CFPropertyListCreateWithData(kCFAllocatorSystemDefault, (CFDataRef)plistData, 0, &fmt, &err);
            if (!plist) {
                logIt(CFSTR("Unable to create property list from data: %@"), err);
            } else {
                logIt(CFSTR("Property list contents:\n%@"), plist);
                if (fmt == kCFPropertyListBinaryFormat_v1_0) {
                    logIt(CFSTR("Converting to XML property list at: %s"), argv[2]);
                    fmt = kCFPropertyListXMLFormat_v1_0;
                } else if (fmt == kCFPropertyListXMLFormat_v1_0) {
                    logIt(CFSTR("Converting to binary property list at: %s"), argv[2]);
                    fmt = kCFPropertyListBinaryFormat_v1_0;
                } else {
                    logIt(CFSTR("Unknown property list format! Not converting output format."));
                }
                
                CFDataRef outputData = CFPropertyListCreateData(kCFAllocatorSystemDefault, plist, fmt, 0, &err);
                if (!outputData) {
                    logIt(CFSTR("Unable to write property list to data: %@"), err);
                } else {
                    bool success = writeDataToFile(outputData, argv[2]);
                    if (!success) {
                        logIt(CFSTR("Unable to write data to file"));
                    }
                    CFRelease(outputData);
                }
                CFRelease(plist);
            }
            CFRelease(plistData);
        }
    }
}
