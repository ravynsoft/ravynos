/*
Original Author: Michael Ash on 11/9/08.
Copyright (c) 2008 Rogue Amoeba Software LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#import <Foundation/NSObject.h>
@class NSArray;
@class NSMutableArray;

enum {
    NSOperationQueuePriorityVeryLow = -8,
    NSOperationQueuePriorityLow = -4,
    NSOperationQueuePriorityNormal = 0,
    NSOperationQueuePriorityHigh = 4,
    NSOperationQueuePriorityVeryHigh = 8
};
typedef NSInteger NSOperationQueuePriority;

@interface NSOperation : NSObject {
    NSOperationQueuePriority priority;
    NSMutableArray *dependencies;

    int executing : 1;
    int cancelled : 1;
    int finished : 1;
}

- (void)start;

// abstract, override this to create a concrete subclass, don't call super
- (void)main;

- (NSArray *)dependencies;
- (void)addDependency:(NSOperation *)operation;
- (void)removeDependency:(NSOperation *)operation;

- (NSOperationQueuePriority)queuePriority;
- (void)setQueuePriority:(NSOperationQueuePriority)priority;

- (BOOL)isCancelled;
- (void)cancel;

- (BOOL)isConcurrent;
- (BOOL)isExecuting;
- (BOOL)isFinished;
- (BOOL)isReady;

@end

extern NSString *const NSInvocationOperationVoidResultException;
extern NSString *const NSInvocationOperationCancelledException;

@interface NSInvocationOperation : NSOperation {
    NSInvocation *_invocation;
}

- initWithInvocation:(NSInvocation *)invocation;
- initWithTarget:target selector:(SEL)selector object:argument;

- (NSInvocation *)invocation;

- result;

@end
