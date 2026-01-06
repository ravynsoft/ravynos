/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
#include <libc.h>
#include "printPList_new.h"
#include "misc_util.h"
#include <os/overflow.h>

static void _indent(CFMutableStringRef string, unsigned indentLevel)
{
    unsigned int i;

    for (i = 0; i < indentLevel; i++) {
        CFStringAppendCString(string, " ", kCFStringEncodingUTF8);
    }
    return;
}

static void _appendCFString(
    CFMutableStringRef string,
    CFStringRef aString,
    Boolean withQuotes)
{
    char * quote = "";

    if (withQuotes) quote = "\"";
    CFStringAppendFormat(string, NULL, CFSTR("%s%@%s"), quote, aString, quote);
    return;
}

static void _appendCFURL(CFMutableStringRef string,
    CFURLRef anURL)
{
    CFURLRef absURL = NULL;     // must release
    CFStringRef absPath = NULL; // must release
    char pathBuffer[PATH_MAX];

    absURL = CFURLCopyAbsoluteURL(anURL);
    if (!absURL) {
        goto finish;
    }

    absPath = CFURLCopyFileSystemPath(anURL, kCFURLPOSIXPathStyle);
    if (!absPath) {
        goto finish;
    }
    CFURLGetFileSystemRepresentation(anURL, true, (UInt8 *)pathBuffer,
        PATH_MAX);
    CFStringAppendCString(string, pathBuffer, kCFStringEncodingUTF8);

finish:
    if (absURL)  CFRelease(absURL);
    if (absPath) CFRelease(absPath);
    return;
}

static void _appendPlist(
    CFMutableStringRef string,
    CFTypeRef          plist,
    CFTypeRef          plistRoot,
    PListStyle         style,
    unsigned           indentLevel)
{
    CFTypeID typeID = 0;
    unsigned int indentIncrement = 4;

    if (!plist) {
        return;
    }

   /* Don't indent the top-level keys/indices of the plist.
    */
    if (plist == plistRoot) {
        indentIncrement = 0;
    }

    typeID = CFGetTypeID(plist);

    if (typeID == CFDictionaryGetTypeID()) {
        CFDictionaryRef dict = (CFDictionaryRef)plist;
        CFIndex count, i;
        CFStringRef * keys = NULL;   // must free
        CFTypeRef * values = NULL; // must free
        count = CFDictionaryGetCount(dict);
        size_t malloc_size;
        if (os_mul_overflow(count, sizeof(CFStringRef), &malloc_size)) {
                return;
        }
        keys = (CFStringRef *)malloc(malloc_size);
        if (keys == NULL) {
                return;
        }
        if (os_mul_overflow(count, sizeof(CFTypeRef), &malloc_size)) {
                free(keys);
                return;
        }
        values = (CFTypeRef *)malloc(malloc_size);
        if (values == NULL) {
                free(keys);
                return;
        }

        CFDictionaryGetKeysAndValues(dict, (const void **)keys,
            (const void **)values);

        if (style == kPListStyleDiagnostics) {
            // no newline at beginning of diagnostics printout
            if (plist != plistRoot) {
                CFStringAppendCString(string, "\n", kCFStringEncodingUTF8);
            }
        } else {
            // no indent before first brace, it's on the end of the line
            CFStringAppendCString(string, "{\n", kCFStringEncodingUTF8);
        }
        for (i = 0; i < count; i++) {

            _indent(string, indentLevel + indentIncrement);
            _appendCFString(string, keys[i],
                /* quotes */ (style != kPListStyleDiagnostics));

            if ( (CFBooleanGetTypeID() == CFGetTypeID(values[i])) &&
                 (style == kPListStyleDiagnostics) ) {
                 
                 CFStringAppendFormat(string, NULL, CFSTR("\n"));
                 continue;  // diagnostics doesn't print bools!
             }

            if (style == kPListStyleDiagnostics) {
                CFStringAppendCString(string, ": ", kCFStringEncodingUTF8);
            } else {
                CFStringAppendCString(string, " = ", kCFStringEncodingUTF8);
            }
            if (CFGetTypeID(values[i]) == CFStringGetTypeID()) {
                CFIndex keyLength = CFStringGetLength(keys[i]);
                CFIndex valueLength = CFStringGetLength(values[i]);

               /* drop value for a really long key+string combo down a line */
                if (indentLevel + indentIncrement + keyLength + valueLength > 72) {
                    CFStringAppendCString(string, "\n", kCFStringEncodingUTF8);
                    _indent(string, indentLevel + (4 + indentIncrement));
                }
            }
            _appendPlist(string, values[i], plistRoot, style, indentLevel +
                indentIncrement);

           /* Put newlines bewteen top-level dict properties for diagnostics.
            */
            if (style == kPListStyleDiagnostics && (i + 1 < count)) {
                if (plist == plistRoot) {
                    CFStringAppendCString(string, "\n", kCFStringEncodingUTF8);
                }
            }
        }
        if (style != kPListStyleDiagnostics) {
            _indent(string, indentLevel);
            CFStringAppendCString(string, "}\n", kCFStringEncodingUTF8);
        }
        free(keys);
        free(values);
    } else if (typeID == CFArrayGetTypeID()) {
        CFArrayRef array = (CFArrayRef)plist;
        CFIndex count, i;
        count = CFArrayGetCount(array);

        // no indent before first parenthesis
        // no newline at beginning of diagnostics printout
        if (style == kPListStyleDiagnostics && (plist != plistRoot)) {
            // no newline at beginning of diagnostics printout
            if (plist != plistRoot) {
                CFStringAppendCString(string, "\n", kCFStringEncodingUTF8);
            }
        } else {
            CFStringAppendCString(string, "(\n", kCFStringEncodingUTF8);
        }
        for (i = 0; i < count; i++) {
            _indent(string, indentLevel + indentIncrement);
            _appendPlist(string, CFArrayGetValueAtIndex(array, i), plistRoot,
                style, indentLevel + indentIncrement);
        }
        if (style != kPListStyleDiagnostics) {
            _indent(string, indentLevel);
            CFStringAppendCString(string, ")\n", kCFStringEncodingUTF8);
        }
    } else if (typeID == CFStringGetTypeID()) {
        _appendCFString(string, (CFStringRef)plist,
            /* quotes */ ((indentLevel > 0) && (style != kPListStyleDiagnostics)));
        CFStringAppendCString(string, "\n", kCFStringEncodingUTF8);
    } else if (typeID == CFURLGetTypeID()) {
        _appendCFURL(string, (CFURLRef)plist);
        CFStringAppendCString(string, "\n", kCFStringEncodingUTF8);
    } else if (typeID == CFDataGetTypeID()) {
        // only show a little bit of the data up front
        CFStringRef dataString = createCFStringForData((CFDataRef)plist, 16);
        if (dataString) {
            CFStringAppend(string, dataString);
            CFStringAppendCString(string, "\n", kCFStringEncodingUTF8);
            SAFE_RELEASE(dataString);
        } else {
            CFStringAppendCString(string, "(data object)\n", kCFStringEncodingUTF8);
        }
    } else if (typeID == CFNumberGetTypeID()) {
        CFStringAppendFormat(string, NULL, CFSTR("%@"), (CFNumberRef)plist);
        CFStringAppendCString(string, "\n", kCFStringEncodingUTF8);
    } else if (typeID == CFBooleanGetTypeID()) {
        CFBooleanRef booleanValue = (CFBooleanRef)plist;
        CFStringAppendFormat(string, NULL, CFSTR("%s\n"),
            CFBooleanGetValue(booleanValue) ? "true" : "false");
    } else if (typeID == CFDateGetTypeID()) {
        CFStringAppendCString(string, "(date object)\n", kCFStringEncodingUTF8);
    } else {
        CFStringAppendCString(string, "(unknown object)\n", kCFStringEncodingUTF8);
    }

    return;
}

void printPList_new(FILE * stream, CFTypeRef plist, PListStyle style)
{
    CFMutableStringRef string = NULL;  // must release
    CFIndex bufSize;
    char * c_string = NULL; // must free

    string = createCFStringForPlist_new(plist, style);
    if (!string) {
        goto finish;
    }

    bufSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(string),
	    kCFStringEncodingUTF8) + sizeof('\0');
    c_string = (char *)malloc(bufSize);
    if (!c_string) {
        goto finish;
    }

    if (CFStringGetCString(string, c_string, bufSize, kCFStringEncodingUTF8)) {
        fprintf(stream, "%s", c_string);
    }

finish:
    if (string) CFRelease(string);
    if (c_string) free(c_string);
    return;
}

// use in GDB
void showPList_new(CFPropertyListRef plist, PListStyle style)
{
    printPList_new(stdout, plist, style);
    return;
}

CFMutableStringRef createCFStringForPlist_new(CFTypeRef plist, PListStyle style)
{
    CFMutableStringRef string = NULL;  // must release

    string = CFStringCreateMutable(kCFAllocatorDefault, 0);
    if (!string) {
        goto finish;
    }

    _appendPlist(string, plist, plist, style, 0);

finish:
    return string;
}
