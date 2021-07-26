/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSConnection.h>
#import <Foundation/NSPortNameServer.h>
#import <Foundation/NSPort.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSRaise.h>

NSString * const NSConnectionReplyMode=@"NSConnectionReplyMode";

@implementation NSConnection

+(NSArray *)allConnections {
   NSUnimplementedMethod();
   return nil;
}

+(NSConnection *)defaultConnection {
   NSUnimplementedMethod();
   return nil;
}

-initWithReceivePort:(NSPort *)receivePort sendPort:(NSPort *)sendPort {
   _receivePort=[receivePort retain];
   _sendPort=[sendPort retain];
   return self;
}

-(void)dealloc {
   [_receivePort release];
   [_sendPort release];
   [super dealloc];
}

+(NSConnection *)connectionWithReceivePort:(NSPort *)receivePort sendPort:(NSPort *)sendPort {
   return [[[self alloc] initWithReceivePort:receivePort sendPort:sendPort] autorelease];
}

+(NSConnection *)connectionWithRegisteredName:(NSString *)name host:(NSString *)hostName usingNameServer:(NSPortNameServer *)nameServer {
   NSUnimplementedMethod();
   return nil;
}

+(NSConnection *)connectionWithRegisteredName:(NSString *)name host:(NSString *)hostName {
   return [self connectionWithRegisteredName:name host:hostName usingNameServer:[NSPortNameServer systemDefaultPortNameServer]];
}

+(NSDistantObject *)rootProxyForConnectionWithRegisteredName:(NSString *)name host:(NSString *)hostName usingNameServer:(NSPortNameServer *)nameServer {
   return [[self connectionWithRegisteredName:name host:hostName usingNameServer:nameServer] rootProxy];
}

+(NSDistantObject *)rootProxyForConnectionWithRegisteredName:(NSString *)name host:(NSString *)hostName {
   return [[self connectionWithRegisteredName:name host:hostName] rootProxy];
}

+currentConversation {
   NSUnimplementedMethod();
   return nil;
}

-delegate {
   return _delegate;
}

-(BOOL)isValid {
   return _isValid;
}

-(BOOL)independentConversationQueueing {
   return _independentConversationQueueing;
}

-(BOOL)multipleThreadsEnabled {
   return _multipleThreadsEnabled;
}

-(NSTimeInterval)replyTimeout {
   return _replyTimeout;
}

-(NSTimeInterval)requestTimeout {
   return _requestTimeout;
}

-(NSPort *)sendPort {
   return _sendPort;
}

-(NSPort *)receivePort {
   return _receivePort;
}

-(NSArray *)requestModes {
   return _requestModes;
}

-rootObject {
   NSUnimplementedMethod();
   return nil;
}

-(NSDistantObject *)rootProxy {
   NSUnimplementedMethod();
   return nil;
}

-(NSArray *)localObjects {
   NSUnimplementedMethod();
   return nil;
}

-(NSArray *)remoteObjects {
   NSUnimplementedMethod();
   return nil;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
}

-(void)invalidate {
   NSUnimplementedMethod();
}

-(void)setIndependentConversationQueueing:(BOOL)flag {
   NSUnimplementedMethod();
}

-(void)enableMultipleThreads {
   NSUnimplementedMethod();
}

-(void)setReplyTimeout:(NSTimeInterval)seconds {
   _replyTimeout=seconds;
   NSUnimplementedMethod();
}

-(void)setRequestTimeout:(NSTimeInterval)seconds {
   _requestTimeout=seconds;
   NSUnimplementedMethod();
}

-(void)addRequestMode:(NSString *)mode {
   NSUnimplementedMethod();
}

-(void)removeRequestMode:(NSString *)mode {
   NSUnimplementedMethod();
}

-(void)setRootObject:rootObject {
   NSUnimplementedMethod();
}

-(void)runInNewThread {
   NSUnimplementedMethod();
}

-(void)addRunLoop:(NSRunLoop *)runLoop {
   NSUnimplementedMethod();
}

-(void)removeRunLoop:(NSRunLoop *)runLoop {
   NSUnimplementedMethod();
}

-(BOOL)registerName:(NSString *)name withNameServer:(NSPortNameServer *)nameServer {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)registerName:(NSString *)name {
   NSUnimplementedMethod();
   return NO;
}

-(NSDictionary *)statistics {
   return _statistics;
}

@end
