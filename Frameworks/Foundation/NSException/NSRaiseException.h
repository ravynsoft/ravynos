/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// *** FOR INTERNAL COCOTRON USE ONLY

#import <Foundation/NSException.h>
#import <Foundation/NSString.h>

#ifdef __cplusplus
extern "C" {
#endif

// DO NOT USE IN NEW CODE AND REPLACE USAGE. Use NSAssert().
FOUNDATION_EXPORT void NSRaiseException(NSString *name, id self, SEL cmd, NSString *fmt, ...);

// This is just a wrapper for fprintf, it doesn't handle %@
// There are situations (such as localization inside NSLog) where you don't want to use NSLog
FOUNDATION_EXPORT void NSCLogv(const char *format, va_list arguments);
FOUNDATION_EXPORT void NSCLog(const char *format, ...);
FOUNDATION_EXPORT void NSCLogThreadId();
FOUNDATION_EXPORT void NSCLogNewline();
FOUNDATION_EXPORT void NSCLogFormat(const char *format, ...);

#ifdef __cplusplus
}
#endif
