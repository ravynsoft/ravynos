/* Copyright (c) 2006-2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSProcessInfo.h>

#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSString_cString.h>
#import <Foundation/NSStringUTF8.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSPlatform.h>
#import <Foundation/NSAutoreleasePool-private.h>

#import <objc/runtime.h>
#import <Foundation/objc_size_alignment.h>
#import <objc/objc.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>

typedef void (*NSLogCStringFunc)(const char *string, unsigned length, BOOL withSyslogBanner);

// These are private-yet-sort-of-documented in Cocoa.
FOUNDATION_EXPORT NSLogCStringFunc _NSLogCStringFunction(void);
FOUNDATION_EXPORT void _NSSetLogCStringFunction(NSLogCStringFunc proc);

static void NSLogDefaultCStringFunction(const char *string, unsigned length, BOOL withSyslogBanner);

static NSLogCStringFunc sNSLogCString = NSLogDefaultCStringFunction;


static void NSLogFormat(NSString *format,...) {
   NSString *string;
   va_list   arguments;

   va_start(arguments,format);
   string=NSStringNewWithFormat(format,nil,arguments,NULL);
   va_end(arguments);

   NSPlatformLogString(string);

   [string release];
}

static void NSLogDefaultCStringFunction(const char *string, unsigned length, BOOL withSyslogBanner) {
   NSString *message = [[NSString alloc] initWithBytes:string length:length encoding:[NSString defaultCStringEncoding]];
   if (withSyslogBanner)
   {
      NSString *date=[[NSDate date]
                      descriptionWithCalendarFormat:@"%Y-%m-%d %H:%M:%S.%F"
                      timeZone:[NSTimeZone systemTimeZone] locale:nil];
      NSString *process=[[NSProcessInfo processInfo] processName];

      NSLogFormat(@"%@ %@[%d:%lx] %@",date,process,NSPlatformProcessID(),NSPlatformThreadID(),message);
   }
   else
   {
      NSPlatformLogString(message);
   }
   [message release];
}

void NSLogv(NSString *format,va_list arguments) {
   NSString *string=NSStringNewWithFormat(format,nil,arguments,NULL);

   NSUInteger length=[string length],byteLength;
   unichar  unicode[length];
   char    *bytes;

   [string getCharacters:unicode];
   bytes=NSString_cStringFromCharacters(unicode,length,YES,&byteLength,NULL,NO);
   [string release];
   if (bytes == NULL) return;

   sNSLogCString(bytes,byteLength,YES);
   NSZoneFree(NULL,bytes);
}

void NSLog(NSString *format,...) {
   va_list arguments;

   va_start(arguments,format);
   NSLogv(format,arguments);
   va_end(arguments);
}

NSLogCStringFunc _NSLogCStringFunction(void)
{
   return sNSLogCString;
}

void _NSSetLogCStringFunction(NSLogCStringFunc proc)
{
   sNSLogCString=proc?proc:NSLogDefaultCStringFunction;
}


const char *NSGetSizeAndAlignment(const char *type, NSUInteger *size, NSUInteger *alignment)
{
    NSUInteger ignore = 0;
    if (!size) {
        size = &ignore;
    }
    if (!alignment) {
        alignment = &ignore;
    }

    *size = 0;
    *alignment = 0;

    *size = objc_ext_sizeof_type(type);
    *alignment = objc_ext_alignof_type(type);
    return objc_ext_skip_type_specifier(type, NO);
}


SEL NSSelectorFromString(NSString *selectorName) {
   NSUInteger length=[selectorName length];
   char     cString[length+1];

   [selectorName getCString:cString maxLength:length+1 encoding:NSASCIIStringEncoding];

   return sel_getUid(cString);
}

NSString *NSStringFromSelector(SEL selector) {
   if(selector==NULL)
    return @"";

    const char *name = sel_getName(selector);
    return NSAutorelease(NSString_anyCStringNewWithBytes(NSASCIIStringEncoding, NULL,name, strlen(name)));
}

Class NSClassFromString(NSString *className) {
   if (className != nil) {
    NSUInteger length=[className length];
    char     cString[length+1];

    [className getCString:cString maxLength:length+1 encoding:NSASCIIStringEncoding];

    return objc_lookUpClass(cString);
   }
   else
    return nil;
}

NSString *NSStringFromClass(Class class) {
   if(class==Nil)
    return nil;

    const char *name = class_getName(class);
    return NSAutorelease(NSString_anyCStringNewWithBytes(NSASCIIStringEncoding, NULL,name, strlen(name)));
}

