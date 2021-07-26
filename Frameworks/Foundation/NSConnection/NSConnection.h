/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSDate.h>
#import <Foundation/NSMapTable.h>

@class NSPortNameServer, NSPort, NSRunLoop, NSDistantObject, NSMutableArray, NSMutableDictionary;

FOUNDATION_EXPORT NSString *const NSConnectionReplyMode;

@interface NSConnection : NSObject {
    NSPort *_receivePort;
    NSPort *_sendPort;
    id _delegate;
    BOOL _isValid;
    BOOL _independentConversationQueueing;
    BOOL _multipleThreadsEnabled;
    NSTimeInterval _replyTimeout;
    NSTimeInterval _requestTimeout;
    NSMutableArray *_requestModes;
    NSMutableDictionary *_statistics;
}

+ (NSArray *)allConnections;

+ (NSConnection *)defaultConnection;

- initWithReceivePort:(NSPort *)receivePort sendPort:(NSPort *)sendPort;

+ (NSConnection *)connectionWithReceivePort:(NSPort *)receivePort sendPort:(NSPort *)sendPort;

+ (NSConnection *)connectionWithRegisteredName:(NSString *)name host:(NSString *)hostName usingNameServer:(NSPortNameServer *)nameServer;
+ (NSConnection *)connectionWithRegisteredName:(NSString *)name host:(NSString *)hostName;

+ (NSDistantObject *)rootProxyForConnectionWithRegisteredName:(NSString *)name host:(NSString *)hostName usingNameServer:(NSPortNameServer *)nameServer;
+ (NSDistantObject *)rootProxyForConnectionWithRegisteredName:(NSString *)name host:(NSString *)hostName;

+ currentConversation;

- delegate;
- (BOOL)isValid;
- (BOOL)independentConversationQueueing;
- (BOOL)multipleThreadsEnabled;

- (NSTimeInterval)replyTimeout;
- (NSTimeInterval)requestTimeout;

- (NSPort *)sendPort;
- (NSPort *)receivePort;

- (NSArray *)requestModes;

- rootObject;
- (NSDistantObject *)rootProxy;

- (NSArray *)localObjects;
- (NSArray *)remoteObjects;

- (void)setDelegate:delegate;

- (void)invalidate;

- (void)setIndependentConversationQueueing:(BOOL)flag;
- (void)enableMultipleThreads;
- (void)setReplyTimeout:(NSTimeInterval)seconds;

- (void)setRequestTimeout:(NSTimeInterval)seconds;

- (void)addRequestMode:(NSString *)mode;
- (void)removeRequestMode:(NSString *)mode;

- (void)setRootObject:rootObject;

- (void)runInNewThread;

- (void)addRunLoop:(NSRunLoop *)runLoop;
- (void)removeRunLoop:(NSRunLoop *)runLoop;

- (BOOL)registerName:(NSString *)name withNameServer:(NSPortNameServer *)nameServer;
- (BOOL)registerName:(NSString *)name;

- (NSDictionary *)statistics;

@end
