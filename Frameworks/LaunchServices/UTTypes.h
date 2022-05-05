/*
 * ravynOS LaunchServices - unified types functions
 *
 * Copyright (C) 2021 Zoe Knox <zoe@pixin.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#import <CoreFoundation/CFString.h>
#import <CoreFoundation/CFArray.h>
#import <CoreFoundation/CFURL.h>

#ifdef __cplusplus
extern "C" {
#endif

const CFStringRef kUTTagClassFilenameExtension = CFSTR("kUTTagClassFilenameExtension");
const CFStringRef kUTTagClassMIMEType = CFSTR("kUTTagClassMIMEType");
const CFStringRef kUTTagClassNSPboardType = CFSTR("kUTTagClassNSPboardType");
const CFStringRef kUTTagClassOSType = CFSTR("kUTTagClassOSType");

Boolean UTTypeEqual(CFStringRef inUTI1, CFStringRef inUTI2);
Boolean UTTypeConformsTo(CFStringRef inUTI1, CFStringRef inUTI2);
CFStringRef UTTypeCreatePreferredIdentifierForTag(CFStringRef inTagClass,
	CFStringRef inTag, CFStringRef inConformingToUTI);
CFArrayRef UTTypeCreateAllIdentifiersForTag(CFStringRef inTagClass,
	CFStringRef inUTI, CFStringRef inConformingToUTI);
CFStringRef UTTypeCopyPreferredTagWithClass(CFStringRef inUTI,
	CFStringRef inTagClass);

// Not implemented yet
CFStringRef UTCreateStringForOSType(OSType inOSType);
OSType UTGetOSTypeFromString(CFStringRef inTag);

CFDictionaryRef UTTypeCopyDeclaration(CFStringRef inUTI);
CFURLRef UTTypeCopyDeclaringBundleURL(CFStringRef inUTI);
CFStringRef UTTypeCopyDescription(CFStringRef inUTI);

#ifdef __cplusplus
}
#endif

