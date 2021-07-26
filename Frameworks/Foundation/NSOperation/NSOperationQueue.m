/*
Original Author: Michael Ash on 11/9/08
Copyright (c) 2008 Rogue Amoeba Software LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#import "NSOperationQueue.h"
#import "NSOperation.h"
#import <Foundation/NSThread.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSMutableArray.h>
#import <Foundation/NSDebug.h>

#import <Foundation/NSRaise.h>
#include <string.h>

@implementation NSOperationQueue

-init {
	if (self = [super init]) {
		workAvailable = [[NSCondition alloc] init];
		suspendedCondition = [[NSCondition alloc] init];
		allWorkDone = [[NSCondition alloc] init];
		isSuspended = NO;
		_operations = [[NSMutableArray alloc] init];
		_thread = [[NSThread alloc] initWithTarget: self selector: @selector( _workThread ) object: nil];
		[_thread start];
	}
	return self;
}

-(void) resume {
   [suspendedCondition lock];
	if (isSuspended) {
		isSuspended = NO;
		[suspendedCondition broadcast];
	}
	[suspendedCondition unlock];
}

-(void) suspend {
	[suspendedCondition lock];
	isSuspended = YES;
	[suspendedCondition unlock];
}


- (void)stop
{
	[_thread cancel];
	[self resume];
	[workAvailable broadcast];
}


- (void)dealloc
{
	[self stop];
	
	[_operations release];
	[_thread release];
	[workAvailable release];
	[suspendedCondition release];
	
	[super dealloc];
}

- (void)addOperation: (NSOperation *)op
{
	@synchronized(self) {
		[_operations addObject: op];
		[_operations sortUsingSelector: @selector(comparePriority:)];
		[workAvailable signal];
	}
}

- (void)addOperations:(NSArray *)ops waitUntilFinished:(BOOL)wait {
	@synchronized(self) {
		[_operations addObjectsFromArray: ops];
		[_operations sortUsingSelector: @selector(comparePriority:)];
		[workAvailable signal];
	}
	if (wait) {
		[self waitUntilAllOperationsAreFinished];
	}
}

- (void)cancelAllOperations {
	[[self operations] makeObjectsPerformSelector:@selector(cancel)];
}

- (NSInteger)maxConcurrentOperationCount {
	NSUnimplementedMethod();
	return NSOperationQueueDefaultMaxConcurrentOperationCount;
}

- (void)setMaxConcurrentOperationCount:(NSInteger)count {
// FIXME: implement but dont warn
//	NSUnimplementedMethod();
}

- (NSString *)name {
	return _name;
}

-(void)setName:(NSString *)newName {
	if (_name != newName) {
		[_name release];
		_name = [newName copy];
	}
}

- (NSArray *)operations {
	NSArray* curOps = nil;
	@synchronized(self) {
		curOps = [_operations copy];
	}
	return [curOps autorelease];
}

- (NSUInteger)operationCount {
    return [_operations count];
}

- (BOOL)isSuspended {
	[suspendedCondition lock];
	BOOL result = isSuspended;
	[suspendedCondition unlock];
	return result;
}

-(void)setSuspended:(BOOL)suspend {
	if (suspend)
     [self suspend];
	else
     [self resume];
}

-(BOOL) hasMoreWork {
	@synchronized(self) {
		return [_operations count] > 0;
	}	
	return NO;
}

-(void)waitUntilAllOperationsAreFinished {
   BOOL isWorking;

   [workAvailable lock];
   isWorking=[self hasMoreWork];
   [workAvailable unlock];

   if(isWorking){
    [allWorkDone lock];
    [allWorkDone wait];
    [allWorkDone unlock];
   }
}

- (void)_workThread
{
	NSAutoreleasePool *outerPool = [[NSAutoreleasePool alloc] init];
	
	NSThread *thread = [NSThread currentThread];
	
	BOOL didRun = NO;
	while( ![thread isCancelled] )
	{
		NSAutoreleasePool *innerPool = [[NSAutoreleasePool alloc] init];

		[suspendedCondition lock];
        
		while (isSuspended)
         [suspendedCondition wait];
         
		[suspendedCondition unlock];
		
		if( !didRun ) {
			[workAvailable lock];
            
            if(![self hasMoreWork]){
             [allWorkDone signal];
            }
            
			while (![self hasMoreWork] && ![thread isCancelled])
             [workAvailable wait];

			[workAvailable unlock];
		}
		
		id op = nil;
		@synchronized(self) {
			// Find an operation that can be run
			NSUInteger index = 0;
			NSUInteger count = [_operations count];
			while (op == nil && index < count) {
				op = [_operations objectAtIndex: index];
				if ([op isReady]) {
					[op retain]; // we're going to remove it from the queue - so make sure it doesn't die on us
					[_operations removeObjectAtIndex: index];
				} else {
					// Try the another one
					op = nil;
				}
				index++;
			}	
		}
		if (op) {
			[op start];
			[op release];
		}
		[innerPool release];
	}

	[outerPool release];
}

+ (id)currentQueue
{
	NSUnimplementedMethod();
	return nil;
}

+ (id)mainQueue
{
	NSUnimplementedMethod();
	return nil;
}

@end
