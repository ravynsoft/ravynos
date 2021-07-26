/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFStream.h>

typedef CFTypeRef CFPropertyListRef;

typedef enum {
    kCFPropertyListOpenStepFormat = 1,
    kCFPropertyListXMLFormat_v1_0 = 100,
    kCFPropertyListBinaryFormat_v1_0 = 200,
} CFPropertyListFormat;

typedef enum {
    kCFPropertyListImmutable = 0,
    kCFPropertyListMutableContainers = 1,
    kCFPropertyListMutableContainersAndLeaves = 2,
} CFPropertyListMutabilityOptions;

COREFOUNDATION_EXPORT CFPropertyListRef CFPropertyListCreateFromStream(CFAllocatorRef allocator, CFReadStreamRef stream, CFIndex length, CFOptionFlags options, CFPropertyListFormat *format, CFStringRef *error);
COREFOUNDATION_EXPORT CFPropertyListRef CFPropertyListCreateFromXMLData(CFAllocatorRef allocator, CFDataRef data, CFOptionFlags options, CFStringRef *error);

COREFOUNDATION_EXPORT CFPropertyListRef CFPropertyListCreateDeepCopy(CFAllocatorRef allocator, CFPropertyListRef self, CFOptionFlags options);

COREFOUNDATION_EXPORT CFDataRef CFPropertyListCreateXMLData(CFAllocatorRef allocator, CFPropertyListRef self);
COREFOUNDATION_EXPORT Boolean CFPropertyListIsValid(CFPropertyListRef self, CFPropertyListFormat format);
COREFOUNDATION_EXPORT CFIndex CFPropertyListWriteToStream(CFPropertyListRef self, CFWriteStreamRef stream, CFPropertyListFormat format, CFStringRef *error);