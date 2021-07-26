/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSStream.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSSocket.h>
#import <Foundation/NSHost.h>
#import "NSInputStream_socket.h"
#import "NSOutputStream_socket.h"

NSString * const NSStreamDataWrittenToMemoryStreamKey=@"kCFStreamPropertyDataWritten";
NSString * const NSStreamFileCurrentOffsetKey=@"kCFStreamPropertyFileCurrentOffset";

/************* These values are also in CFStream, keep in sync */

NSString * const NSStreamSocketSecurityLevelKey=@"kCFStreamSSLLevel";

NSString * const NSStreamSocketSecurityLevelNone=@"kCFStreamSocketSecurityLevelNone";
NSString * const NSStreamSocketSecurityLevelSSLv2=@"kCFStreamSocketSecurityLevelSSLv2";
NSString * const NSStreamSocketSecurityLevelSSLv3=@"kCFStreamSocketSecurityLevelSSLv3";
NSString * const NSStreamSocketSecurityLevelTLSv1=@"kCFStreamSocketSecurityLevelTLSv1";
NSString * const NSStreamSocketSecurityLevelNegotiatedSSL=@"kCFStreamSocketSecurityLevelNegotiatedSSL";

@implementation NSStream

+(void)getStreamsToHost:(NSHost *)host port:(NSInteger)port inputStream:(NSInputStream **)inputStreamp outputStream:(NSOutputStream **)outputStreamp {
   NSSocket              *socket=[[[NSSocket alloc] initTCPStream] autorelease];
   NSError               *error;
   BOOL                   immediate;
   NSInputStream_socket  *input;
   NSOutputStream_socket *output;

   if((error=[socket connectToHost:host port:port immediate:&immediate])!=nil){
    *inputStreamp=nil;
    *outputStreamp=nil;
    return;
   }

   if (inputStreamp)
    *inputStreamp=input=[[[NSInputStream_socket alloc] initWithSocket:socket streamStatus:NSStreamStatusNotOpen] autorelease];
   if (outputStreamp)
    *outputStreamp=output=[[[NSOutputStream_socket alloc] initWithSocket:socket streamStatus:NSStreamStatusNotOpen] autorelease];
}

-delegate {
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)setDelegate:delegate {
   NSInvalidAbstractInvocation();
}

-(NSError *)streamError {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSStreamStatus)streamStatus {
   NSInvalidAbstractInvocation();
   return 0;
}

-propertyForKey:(NSString *)key {
   NSInvalidAbstractInvocation();
   return nil;
}

-(BOOL)setProperty:property forKey:(NSString *)key {
   NSInvalidAbstractInvocation();
   return NO;
}

-(void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   NSInvalidAbstractInvocation();
}

-(void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   NSInvalidAbstractInvocation();
}

-(void)open {
   NSInvalidAbstractInvocation();
}

-(void)close {
   NSInvalidAbstractInvocation();
}

@end
