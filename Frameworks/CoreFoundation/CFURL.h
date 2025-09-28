/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFData.h>

typedef struct __NSURL *CFURLRef;

typedef enum {
    kCFURLComponentScheme = 1,
    kCFURLComponentNetLocation = 2,
    kCFURLComponentPath = 3,
    kCFURLComponentResourceSpecifier = 4,
    kCFURLComponentUser = 5,
    kCFURLComponentPassword = 6,
    kCFURLComponentUserInfo = 7,
    kCFURLComponentHost = 8,
    kCFURLComponentPort = 9,
    kCFURLComponentParameterString = 10,
    kCFURLComponentQuery = 11,
    kCFURLComponentFragment = 12,
} CFURLComponentType;

typedef enum {
    kCFURLPOSIXPathStyle = 0,
    kCFURLHFSPathStyle = 1,
    kCFURLWindowsPathStyle = 2,
} CFURLPathStyle;

COREFOUNDATION_EXPORT CFTypeID CFURLGetTypeID(void);

COREFOUNDATION_EXPORT CFURLRef CFURLCreateAbsoluteURLWithBytes(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length, CFStringEncoding encoding, CFURLRef baseURL, Boolean useCompatibilityMode);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateWithBytes(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length, CFStringEncoding encoding, CFURLRef baseURL);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateWithFileSystemPath(CFAllocatorRef allocator, CFStringRef path, CFURLPathStyle pathStyle, Boolean isDirectory);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateWithFileSystemPathRelativeToBase(CFAllocatorRef allocator, CFStringRef path, CFURLPathStyle pathStyle, Boolean isDirectory, CFURLRef baseURL);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateWithString(CFAllocatorRef allocator, CFStringRef string, CFURLRef baseURL);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateFromFileSystemRepresentation(CFAllocatorRef allocator, const uint8_t *buffer, CFIndex length, Boolean isDirectory);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateFromFileSystemRepresentationRelativeToBase(CFAllocatorRef allocator, const uint8_t *buffer, CFIndex length, Boolean isDirectory, CFURLRef baseURL);

COREFOUNDATION_EXPORT CFURLRef CFURLCopyAbsoluteURL(CFURLRef url);

COREFOUNDATION_EXPORT CFStringRef CFURLGetString(CFURLRef self);
COREFOUNDATION_EXPORT CFURLRef CFURLGetBaseURL(CFURLRef self);
COREFOUNDATION_EXPORT Boolean CFURLCanBeDecomposed(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyFileSystemPath(CFURLRef self, CFURLPathStyle pathStyle);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyFragment(CFURLRef self, CFStringRef charactersToLeaveEscaped);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyHostName(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyLastPathComponent(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyNetLocation(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyParameterString(CFURLRef self, CFStringRef charactersToLeaveEscaped);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyPassword(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyPath(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyPathExtension(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyQueryString(CFURLRef self, CFStringRef charactersToLeaveEscaped);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyResourceSpecifier(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyScheme(CFURLRef self);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyStrictPath(CFURLRef self, Boolean *isAbsolute);
COREFOUNDATION_EXPORT CFStringRef CFURLCopyUserName(CFURLRef self);
COREFOUNDATION_EXPORT CFInteger CFURLGetPortNumber(CFURLRef self);
COREFOUNDATION_EXPORT Boolean CFURLHasDirectoryPath(CFURLRef self);

COREFOUNDATION_EXPORT CFURLRef CFURLCreateCopyAppendingPathComponent(CFAllocatorRef allocator, CFURLRef self, CFStringRef pathComponent, Boolean isDirectory);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateCopyAppendingPathExtension(CFAllocatorRef allocator, CFURLRef self, CFStringRef extension);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateCopyDeletingLastPathComponent(CFAllocatorRef allocator, CFURLRef self);
COREFOUNDATION_EXPORT CFURLRef CFURLCreateCopyDeletingPathExtension(CFAllocatorRef allocator, CFURLRef self);
COREFOUNDATION_EXPORT CFDataRef CFURLCreateData(CFAllocatorRef allocator, CFURLRef self, CFStringEncoding encoding, Boolean escapeWhitespace);
COREFOUNDATION_EXPORT CFStringRef CFURLCreateStringByAddingPercentEscapes(CFAllocatorRef allocator, CFStringRef string, CFStringRef charactersToLeaveUnescaped, CFStringRef charactersToBeEscaped, CFStringEncoding encoding);
COREFOUNDATION_EXPORT CFStringRef CFURLCreateStringByReplacingPercentEscapes(CFAllocatorRef allocator, CFStringRef string, CFStringRef charactersToLeaveEscaped);
COREFOUNDATION_EXPORT CFStringRef CFURLCreateStringByReplacingPercentEscapesUsingEncoding(CFAllocatorRef allocator, CFStringRef string, CFStringRef charactersToLeaveEscaped, CFStringEncoding encoding);
COREFOUNDATION_EXPORT CFRange CFURLGetByteRangeForComponent(CFURLRef self, CFURLComponentType component, CFRange *rangeIncludingSeparators);
COREFOUNDATION_EXPORT CFIndex CFURLGetBytes(CFURLRef self, uint8_t *buffer, CFIndex bufferLength);
COREFOUNDATION_EXPORT Boolean CFURLGetFileSystemRepresentation(CFURLRef self, Boolean resolveAgainstBase, uint8_t *buffer, CFIndex bufferLength);
