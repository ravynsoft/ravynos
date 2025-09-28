/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSRaise.h>
#import <Foundation/NSString.h>
#import <objc/runtime.h>
#include <stdio.h>
#ifdef WINDOWS
#include <windows.h>
#endif

// DO NOT USE IN NEW CODE AND REPLACE USAGE. Use NSAssert().
void NSRaiseException(NSString *name,id self,SEL cmd,NSString *fmt,...) {
   NSString *where=[NSString stringWithFormat:@"-[%@ %s]",[self class],sel_getName(cmd)];
   NSString *why;
   va_list   args;

   va_start(args,fmt);

   why=[[[NSString allocWithZone:NULL] initWithFormat:fmt arguments:args] autorelease];

   va_end(args);
   [NSException raise:name format:@"%@ %@",where,why];
}

void NSCLogThreadId(){
#ifdef WINDOWS
   fprintf(stderr,"threadId=%p:tick=%d:",GetCurrentThreadId(),GetTickCount());
#endif
}

void NSCLogNewline(){
#ifdef WINDOWS
   fprintf(stderr,"\n",GetCurrentThreadId());
#else
   fprintf(stderr,"\n");
#endif
   fflush(stderr);
}

void NSCLogFormatWithArguments(const char *format,va_list arguments){
   vfprintf(stderr,format,arguments);
   fflush(stderr);
}

void NSCLogFormat(const char *format,...){
   va_list arguments;

   va_start(arguments,format);
   NSCLogFormatWithArguments(format,arguments);
   va_end(arguments);
}

void NSCLogv(const char *format,va_list arguments) {
   NSCLogThreadId();
   NSCLogFormatWithArguments(format,arguments);
   NSCLogNewline();
}

void NSCLog(const char *format,...) {
   va_list arguments;

   va_start(arguments,format);
   NSCLogv(format,arguments);
   va_end(arguments);
}
