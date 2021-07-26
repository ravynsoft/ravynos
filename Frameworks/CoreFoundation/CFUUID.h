/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreFoundation/CFBase.h>

typedef struct CFUUID *CFUUIDRef;

typedef struct {
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
    uint8_t byte4;
    uint8_t byte5;
    uint8_t byte6;
    uint8_t byte7;
    uint8_t byte8;
    uint8_t byte9;
    uint8_t byte10;
    uint8_t byte11;
    uint8_t byte12;
    uint8_t byte13;
    uint8_t byte14;
    uint8_t byte15;
} CFUUIDBytes;

COREFOUNDATION_EXPORT CFTypeID CFUUIDGetTypeID(void);

COREFOUNDATION_EXPORT CFUUIDRef CFUUIDCreate(CFAllocatorRef alloc);
COREFOUNDATION_EXPORT CFUUIDRef CFUUIDCreateFromString(CFAllocatorRef allocator, CFStringRef string);
COREFOUNDATION_EXPORT CFUUIDRef CFUUIDCreateFromUUIDBytes(CFAllocatorRef allocator, CFUUIDBytes bytes);
COREFOUNDATION_EXPORT CFUUIDRef CFUUIDCreateWithBytes(CFAllocatorRef allocator, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8, uint8_t byte9, uint8_t byte10, uint8_t byte11, uint8_t byte12, uint8_t byte13, uint8_t byte14, uint8_t byte15);

COREFOUNDATION_EXPORT CFUUIDRef CFUUIDGetConstantUUIDWithBytes(CFAllocatorRef allocator, uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8, uint8_t byte9, uint8_t byte10, uint8_t byte11, uint8_t byte12, uint8_t byte13, uint8_t byte14, uint8_t byte15);

COREFOUNDATION_EXPORT CFUUIDBytes CFUUIDGetUUIDBytes(CFUUIDRef self);

COREFOUNDATION_EXPORT CFStringRef CFUUIDCreateString(CFAllocatorRef allocator, CFUUIDRef self);
