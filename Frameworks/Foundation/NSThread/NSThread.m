/* Copyright (c) 2006-2007 Christopher J. W. Lloyd, 2008 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSThread.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSPlatform.h>
#import <Foundation/NSNotificationCenter.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSThread-Private.h>
#import <Foundation/NSSynchronization.h>
#import <Foundation/NSConditionLock.h>
#if !defined(GCC_RUNTIME_3) && !defined(APPLE_RUNTIME_4)
#import <Foundation/objc_debugHelpers.h>
#endif
//#import <Foundation/debugHelpers.h>
#import <Foundation/NSSelectSet.h>
#include <pthread.h>

#if defined(LINUX) ||  defined(__APPLE__) ||  defined(__FreeBSD__)
#include <execinfo.h>
#include <sys/resource.h>
#endif


NSString * const NSDidBecomeSingleThreadedNotification=@"NSDidBecomeSingleThreadedNotification";
NSString * const NSWillBecomeMultiThreadedNotification=@"NSWillBecomeMultiThreadedNotification";
NSString * const NSThreadWillExitNotification=@"NSThreadWillExitNotification";

@implementation NSThread

static BOOL isMultiThreaded = NO;
static NSThread* mainThread = nil;

+(void)initialize
{
	if(self==[NSThread class])
	{
      if(!mainThread)
      {
         mainThread = [NSThread new];
         NSPlatformSetCurrentThread(mainThread);
      }
	}
}

+ (BOOL) isMultiThreaded {
	return isMultiThreaded;
}

+(BOOL)isMainThread {
	return NSCurrentThread()==mainThread;
}

+(NSThread *)mainThread {
   return mainThread;
}


- (id) initWithTarget: (id) aTarget selector: (SEL) aSelector object: (id) anArgument {
   [self init];
   _target   = [aTarget retain];
   _selector = aSelector;
   _argument = [anArgument retain];
   return self;
}

- (void) main {
	[_target performSelector: _selector withObject: _argument];
}

#ifdef WINDOWS
// Be sure the stack is aligned in case the thread wants to do exotic things like SSE2
static  __attribute__((force_align_arg_pointer)) unsigned __stdcall nsThreadStartThread(void* t)
#else
static void *nsThreadStartThread(void* t)
#endif
{
   NSThread    *thread = t;
	NSPlatformSetCurrentThread(thread);
	[thread setExecuting:YES];
    NSCooperativeThreadWaiting();
	@try {
		[thread main];
	}
	@catch (NSException * e) {
        NSLog(@"Exception occured : %@", [e description]);
	}
	[thread setExecuting:NO];
	[thread setFinished:YES];

    NSSelectSetShutdownForCurrentThread();
	
	// We need a pool here in case release triggers some autoreleased object allocations
	// so they won't stay in limbo
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	[thread release];
	NSPlatformSetCurrentThread(nil);
	[pool drain];
	
	return 0;
}

+(void)detachNewThreadSelector:(SEL)selector toTarget:target withObject:argument {
	id newThread = [[self alloc] initWithTarget: target selector: selector object: argument];
	[newThread start];
	[newThread release];
}

+(NSThread *)currentThread {
   return NSPlatformCurrentThread();
}

+(NSArray *)callStackReturnAddresses {
    NSMutableArray *ret=[NSMutableArray array];

    void* callstack[128];
    int i, frameCount = backtrace(callstack, 128);
    //ignore current frame
    for (i = 1; i < frameCount; i++) {
        [ret addObject:[NSValue valueWithPointer:callstack[i]]];
    }
    
    return ret;
}

+(NSArray *)callStackSymbols {
    NSMutableArray *ret=[NSMutableArray array];
    void* callstack[128];
    int i, frameCount = backtrace(callstack, 128);
    char** symbols = backtrace_symbols(callstack, frameCount);
    //ignore current frame
    for (i = 1; i < frameCount; ++i) {
        [ret addObject:[NSString stringWithCString:symbols[i] encoding:NSISOLatin1StringEncoding]];
    }
    free(symbols);
    
    return ret;
}

+(double)threadPriority {
   NSUnimplementedMethod();
   return 0;
}

+(BOOL)setThreadPriority:(double)value {
   struct sched_param scheduling;

   value=MAX(0,MIN(value,1.0));

   int policy;

   pthread_getschedparam(pthread_self(),&policy,&scheduling);
   int min=sched_get_priority_min(policy);
   int max=sched_get_priority_min(policy);

   scheduling.sched_priority=min+(max-min)*value;

   pthread_setschedparam(pthread_self(),policy,&scheduling);

   return YES;
}

+(void)sleepUntilDate:(NSDate *)date {
   NSTimeInterval interval=[date timeIntervalSinceNow];

   NSCooperativeThreadBlocking();
   NSPlatformSleepThreadForTimeInterval(interval);
   NSCooperativeThreadWaiting();
}

+(void)sleepForTimeInterval:(NSTimeInterval)value {
   NSCooperativeThreadBlocking();
   NSPlatformSleepThreadForTimeInterval(value);
   NSCooperativeThreadWaiting();
}

+(void)exit {
   NSUnimplementedMethod();
}

-init {
   _dictionary=[NSMutableDictionary new];
   _sharedObjects=[NSMutableDictionary new];
   if(isMultiThreaded)
      _sharedObjectLock=[NSLock new];
   return self;
}

- (void) dealloc {
	if([self isExecuting])
		[NSException raise:NSInternalInconsistencyException format:@"trying to dealloc thread %@ while it's running", self];
	[_dictionary release]; _dictionary=nil;

   id oldSharedObjects=_sharedObjects;
   _sharedObjects=nil;
	[oldSharedObjects release];

   [_sharedObjectLock release]; _sharedObjectLock=nil;
   [_name release]; _name=nil;
	[_argument release]; _argument=nil;
	[_target release]; _target=nil;
	[super dealloc];
}

-(void)start {
	[self retain]; // balanced by release in nsThreadStartThread

   if (!isMultiThreaded) {
		[[NSNotificationCenter defaultCenter] postNotificationName: NSWillBecomeMultiThreadedNotification
                                                          object: nil
                                                        userInfo: nil];
		isMultiThreaded = YES;
      // lazily initialize mainThread's lock
      mainThread->_sharedObjectLock=[NSLock new];
#if !defined(GCC_RUNTIME_3) && !defined(APPLE_RUNTIME_4)
		_NSInitializeSynchronizedDirective();
#endif
	}
   // if we were init'ed before didBecomeMultithreaded, we won't have a lock either
   if(!_sharedObjectLock)
      _sharedObjectLock=[NSLock new];
    NSError *error = nil;
	if (NSPlatformDetachThread( &nsThreadStartThread, self, &error) == 0) {
		// No thread has been created. Don't leak:
		[self release];
		[NSException raise: @"NSThreadCreationFailedException"
					format: @"Creation of Objective-C thread failed [%@].", error];
	}
}

-(BOOL)isMainThread {
   return self==mainThread;
}

-(BOOL)isCancelled {
	return _cancelled;
}

-(BOOL)isExecuting {
	return _executing;
}

-(BOOL)isFinished {
	return _finished;
}

-(void)cancel {
	_cancelled=YES;
}

-(NSString *)name {
	return _name;
}

-(void)setExecuting:(BOOL)executing
{
	_executing=executing;
}

-(void)setFinished:(BOOL)finished
{
	_finished=finished;
}

-(NSUInteger)stackSize {
   NSUnimplementedMethod();
   return 0;
}

-(NSMutableDictionary *)threadDictionary {
   return _dictionary;
}

-(void)setName:(NSString *)value {
	if(value!=_name)
	{
		[_name release];
		_name=[value copy];
	}
}

-(void)setStackSize:(NSUInteger)value {
   NSUnimplementedMethod();
}

-(NSMutableDictionary *)sharedDictionary {
   return _sharedObjects;
}

static inline id _NSThreadSharedInstance(NSThread *thread,NSString *className,BOOL create) {
   NSMutableDictionary *shared=thread->_sharedObjects;
   if(!shared)
      return nil;
	id result=nil;
   [thread->_sharedObjectLock lock];
   result=[shared objectForKey:className];
   [thread->_sharedObjectLock unlock];

   if(result==nil && create){
      // do not hold lock during object allocation
      result=[NSClassFromString(className) new];
      [thread->_sharedObjectLock lock];
      [shared setObject:result forKey:className];
      [thread->_sharedObjectLock unlock];
      [result release];
   }

   return result;
}

FOUNDATION_EXPORT id NSThreadSharedInstance(NSString *className) {
   return _NSThreadSharedInstance(NSPlatformCurrentThread(),className,YES);
}

FOUNDATION_EXPORT id NSThreadSharedInstanceDoNotCreate(NSString *className) {
   return _NSThreadSharedInstance(NSPlatformCurrentThread(),className,NO);
}

-sharedObjectForClassName:(NSString *)className {
   return _NSThreadSharedInstance(self,className,YES);
}

-(void)setSharedObject:object forClassName:(NSString *)className {
   [_sharedObjectLock lock];
   if(object==nil)
    [_sharedObjects removeObjectForKey:className];
   else
    [_sharedObjects setObject:object forKey:className];
   [_sharedObjectLock unlock];
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@[0x%lx] threadDictionary: %@ currentPool: %@>", isa, self, _dictionary, _currentPool];
}

NSAutoreleasePool *NSThreadCurrentPool(void) {
   return NSPlatformCurrentThread()->_currentPool;
}

void NSThreadSetCurrentPool(NSAutoreleasePool *pool){
   NSPlatformCurrentThread()->_currentPool=pool;
}

@end

@implementation NSObject(NSThread)

-(void)_performSelectorOnThreadHelper:(NSArray*)selectorAndArguments {
   NSConditionLock *waitingLock=[selectorAndArguments objectAtIndex:0];
   SEL              selector=NSSelectorFromString([selectorAndArguments objectAtIndex:1]);
   id               object=[[selectorAndArguments objectAtIndex:2] pointerValue];

   [waitingLock lockWhenCondition:0];

   [self performSelector:selector withObject:object];

   [waitingLock unlockWithCondition:1];
   [selectorAndArguments release];
}

- (void)performSelector:(SEL)selector onThread:(NSThread *)thread withObject:(id)object waitUntilDone:(BOOL)waitUntilDone modes:(NSArray *)modes
{
   if(thread==nil){
    [NSException raise:NSInvalidArgumentException format:@"Thread is nil"];
    return;
   }
	id runloop=_NSThreadSharedInstance(thread, @"NSRunLoop", NO);
	if(waitUntilDone)
	{
		if(thread==[NSThread currentThread])
		{
			[self performSelector:selector withObject:object];
		}
		else
		{
			if(!runloop)
				[NSException raise:NSInvalidArgumentException format:@"thread %@ has no runloop in %@", thread, NSStringFromSelector(_cmd)];
			NSConditionLock *waitingLock=[[NSConditionLock alloc] initWithCondition:0];

         // array retain balanced in _performSelectorOnThreadHelper:
			[runloop performSelector:@selector(_performSelectorOnThreadHelper:)
							  target:self
							argument:[[NSArray arrayWithObjects:waitingLock, NSStringFromSelector(selector), [NSValue valueWithPointer:object], nil] retain]
							   order:0
							   modes:modes];

			[waitingLock lockWhenCondition:1];
			[waitingLock unlock];
			[waitingLock release];
		}
	}
	else
	{
		if(!runloop)
			[NSException raise:NSInvalidArgumentException format:@"thread %@ has no runloop in %@", thread, NSStringFromSelector(_cmd)];

		[runloop performSelector:selector target:self argument:object order:0 modes:modes];
	}
}

- (void)performSelector:(SEL)selector onThread:(NSThread *)thread withObject:(id)object waitUntilDone:(BOOL)waitUntilDone
{
	[self performSelector:selector onThread:thread withObject:object waitUntilDone:waitUntilDone modes:[NSArray arrayWithObject:NSRunLoopCommonModes]];
}

-(void)performSelectorOnMainThread:(SEL)selector withObject:(id)object waitUntilDone:(BOOL)waitUntilDone modes:(NSArray *)modes {
	[self performSelector:selector onThread:[NSThread mainThread] withObject:object waitUntilDone:waitUntilDone modes:modes];
}

-(void)performSelectorOnMainThread:(SEL)selector withObject:(id)object waitUntilDone:(BOOL)waitUntilDone {
	[self performSelectorOnMainThread:selector withObject:object waitUntilDone:waitUntilDone modes:[NSArray arrayWithObject:NSRunLoopCommonModes]];
}

-(void)performSelectorInBackground:(SEL)selector withObject:object {
	[NSThread detachNewThreadSelector:selector toTarget:self withObject:object];
}

@end

FOUNDATION_EXPORT NSThread *NSCurrentThread(void) {
   return NSPlatformCurrentThread();
}

