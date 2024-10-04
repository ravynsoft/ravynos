/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSException.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSThread-Private.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSRaiseException.h>
#include <stdio.h>

NSString * const NSGenericException=@"NSGenericException";
NSString * const NSInvalidArgumentException=@"NSInvalidArgumentException";
NSString * const NSRangeException=@"NSRangeException";

NSString * const NSInternalInconsistencyException=@"NSInternalInconsistencyException";
NSString * const NSMallocException=@"NSMallocException";

NSString * const NSParseErrorException=@"NSParseErrorException";
NSString * const NSInconsistentArchiveException=@"NSInconsistentArchiveException";

@implementation NSException

+(void)raise:(NSString *)name format:(NSString *)format,... {
   va_list  arguments;

   va_start(arguments,format);

   return [self raise:name format:format arguments:arguments];
}

+(void)raise:(NSString *)name format:(NSString *)format arguments:(va_list)arguments {
   [[self exceptionWithName:name
     reason:NSStringWithFormatArguments(format,arguments) userInfo:nil] raise];
}

-initWithName:(NSString *)name reason:(NSString *)reason
  userInfo:(NSDictionary *)userInfo {
   _name=[name copy];
   _reason=[reason copy];
   _userInfo=[userInfo retain];
   _callStack=nil;
   return self;
}

-(void)dealloc {
   [_name release];
   [_reason release];
   [_userInfo release];
   [_callStack release];
   NSDeallocateObject(self);
   return;
   [super dealloc];
}

+(NSException *)exceptionWithName:(NSString *)name reason:(NSString *)reason
  userInfo:(NSDictionary *)userInfo {
   return [[[self allocWithZone:NULL] initWithName:name reason:reason userInfo:userInfo] autorelease];
}

-(NSString *)description {
   return _reason;
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-initWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
   return self;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-(void)raise {
   if(NSDebugEnabled){
    NSCLog("RAISE %s",[[self description] UTF8String]);
    return;
   }
   [_callStack release];
   _callStack=[[NSThread callStackReturnAddresses] retain];
   NSArray *stackTrace = [NSThread callStackSymbols];
   NSCLog("%s: %s\n", [_name UTF8String], [_reason UTF8String]);
   for(int i = 0; i < [stackTrace count]; ++i)
    NSCLog("  %u. %s", i, [[stackTrace objectAtIndex:i] UTF8String]);
   NSCLog("");
   [stackTrace release];
   _NSRaiseException(self);
//   objc_exception_throw(self);
}

-(NSString *)name {
   return _name;
}

-(NSString *)reason {
   return _reason;
}

-(NSDictionary *)userInfo {
   return _userInfo;
}

-(NSArray *)callStackReturnAddresses {
   return _callStack;
}

@end
