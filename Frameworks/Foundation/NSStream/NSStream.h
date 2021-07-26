/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <CoreFoundation/CFStream.h>

@class NSError, NSHost, NSRunLoop, NSInputStream, NSOutputStream;

typedef enum {
    NSStreamStatusNotOpen,
    NSStreamStatusOpening,
    NSStreamStatusOpen,
    NSStreamStatusReading,
    NSStreamStatusWriting,
    NSStreamStatusAtEnd,
    NSStreamStatusClosed,
    NSStreamStatusError
} NSStreamStatus;

// These values must match CFStream
typedef enum {
    NSStreamEventNone,
    NSStreamEventOpenCompleted = kCFStreamEventOpenCompleted,
    NSStreamEventHasBytesAvailable = kCFStreamEventHasBytesAvailable,
    NSStreamEventHasSpaceAvailable = kCFStreamEventCanAcceptBytes,
    NSStreamEventErrorOccurred = kCFStreamEventErrorOccurred,
    NSStreamEventEndEncountered = kCFStreamEventEndEncountered,
} NSStreamEvent;

FOUNDATION_EXPORT NSString *const NSStreamDataWrittenToMemoryStreamKey;
FOUNDATION_EXPORT NSString *const NSStreamFileCurrentOffsetKey;

FOUNDATION_EXPORT NSString *const NSStreamSocketSecurityLevelKey;

FOUNDATION_EXPORT NSString *const NSStreamSocketSecurityLevelNone;
FOUNDATION_EXPORT NSString *const NSStreamSocketSecurityLevelSSLv2;
FOUNDATION_EXPORT NSString *const NSStreamSocketSecurityLevelSSLv3;
FOUNDATION_EXPORT NSString *const NSStreamSocketSecurityLevelTLSv1;
FOUNDATION_EXPORT NSString *const NSStreamSocketSecurityLevelNegotiatedSSL;

@interface NSStream : NSObject

+ (void)getStreamsToHost:(NSHost *)host port:(NSInteger)port inputStream:(NSInputStream **)inputStream outputStream:(NSOutputStream **)outputStream;

- delegate;
- (void)setDelegate:delegate;

- (NSError *)streamError;
- (NSStreamStatus)streamStatus;

- propertyForKey:(NSString *)key;
- (BOOL)setProperty:property forKey:(NSString *)key;

- (void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;
- (void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;

- (void)open;
- (void)close;

@end

@interface NSObject (NSStream_delegate)
- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)streamEvent;
@end

#import <Foundation/NSInputStream.h>
#import <Foundation/NSOutputStream.h>
