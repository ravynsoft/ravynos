/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

@class NSPortMessage, NSRunLoop, NSConnection, NSDate, NSMutableArray, NSData;

FOUNDATION_EXPORT NSString *const NSPortDidBecomeInvalidNotification;

@interface NSPort : NSObject

+ (NSPort *)port;

- (id)delegate;
- (void)setDelegate:delegate;

- (void)invalidate;
- (BOOL)isValid;

- (void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;
- (void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;

- (void)addConnection:(NSConnection *)connection toRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;
- (void)removeConnection:(NSConnection *)connection fromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;

- (NSUInteger)reservedSpaceLength;

- (BOOL)sendBeforeDate:(NSDate *)beforeDate components:(NSMutableArray *)components from:(NSPort *)fromPort reserved:(NSUInteger)reservedSpace;

- (BOOL)sendBeforeDate:(NSDate *)beforeData msgid:(NSUInteger)msgid components:(NSMutableArray *)components from:(NSPort *)fromPort reserved:(NSUInteger)reservedSpace;

@end

@interface NSObject (NSPortDelegate)
- (void)handlePortMessage:(NSPortMessage *)portMessage;
@end

#import <Foundation/NSSocketPort.h>
