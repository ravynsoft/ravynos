/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNotificationQueue-Private.h>
#import <Foundation/NSThread-Private.h>
#import <Foundation/NSDelayedPerform.h>
#import <Foundation/NSOrderedPerform.h>

#import <Foundation/NSRunLoopState.h>
#import <Foundation/NSRunLoop-InputSource.h>
#import <Foundation/NSPlatform.h>
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSRaise.h>

NSString * const NSDefaultRunLoopMode=@"kCFRunLoopDefaultMode";
NSString * const NSRunLoopCommonModes=@"kCFRunLoopCommonModes";

@implementation NSRunLoop

+(NSRunLoop *)currentRunLoop {
   return NSThreadSharedInstance(@"NSRunLoop");
}

+(NSRunLoop *)mainRunLoop {
	return [[NSThread mainThread] sharedObjectForClassName:@"NSRunLoop"];
}

-init {
   NSInputSource *parentDeath;

   _modes=NSCreateMapTableWithZone(NSObjectMapKeyCallBacks,NSObjectMapValueCallBacks,0,[self zone]);
   _commonModes=[[NSMutableArray alloc] init];
   [_commonModes addObject:NSDefaultRunLoopMode];
   _currentMode=NSDefaultRunLoopMode;
   _continue=[[NSMutableArray alloc] init];
   _orderedPerforms=[NSMutableArray new];

   if((parentDeath=[[NSPlatform currentPlatform] parentDeathInputSource])!=nil)
    [self addInputSource:parentDeath forMode:NSDefaultRunLoopMode];

   return self;
}

-(void)dealloc {
	NSFreeMapTable(_modes);
	[_currentMode release];
    [_commonModes release];
    [_continue release];
	[_orderedPerforms release];
	[super dealloc];
}

-(NSRunLoopState *)stateForMode:(NSString *)mode {
   NSRunLoopState *state=NSMapGet(_modes,mode);

   if(state==nil){
    state=[[NSRunLoopState new] autorelease];
    NSMapInsert(_modes,mode,state);
   }

   return state;
}

-(NSArray *)statesForMode:(NSString *)mode {
   NSMutableArray *result=[NSMutableArray array];

   if([mode isEqualToString:NSRunLoopCommonModes]){
    for(NSString *common in _commonModes)
     [result addObject:[self stateForMode:common]];
   }
   else {
    [result addObject:[self stateForMode:mode]];
   }

   return result;
}


-(BOOL)_orderedPerforms {
  BOOL didPerform=NO;
  id performs=nil;

  @synchronized(_orderedPerforms)
  {
    performs=[_orderedPerforms copy];
  }
  int count=[performs count];

  while(--count>=0){
    BOOL tryPerform=NO;
    NSOrderedPerform *check=[performs objectAtIndex:count];

    // if this object is still in _orderedPerforms, set tryPerform and remove it,
    // otherwise it was performed or removed already, and there's nothing to do
    @synchronized(_orderedPerforms)
    {
      int prevCount = [_orderedPerforms count];
      [_orderedPerforms removeObjectIdenticalTo:check];
      if([_orderedPerforms count] < prevCount){
        tryPerform=YES; // it was removed
      }
    }

    if(tryPerform){
      // be careful the lock on _orderedPerforms is not held while we fire the perform
      // TODO: right now, all modes are common modes

      if([check fireInMode:_currentMode]){
        didPerform=YES;
      }
      else{
        // re-add it, so it can be executed another time
        @synchronized(_orderedPerforms)
        {
          [_orderedPerforms insertObject:check atIndex:[_orderedPerforms count]];
        }
      }
    }
  }
  [performs release];
  return didPerform;
}

-(NSString *)currentMode {
   return _currentMode;
}

- (CFRunLoopRef)getCFRunLoop {
	return (CFRunLoopRef)self;
}

-(void)_wakeUp {
   [[self stateForMode:_currentMode] wakeUp];
}


- (NSDate *)limitDateForMode:(NSString *)mode
{
    NSRunLoopState *state = [self stateForMode:mode];

    mode = [mode retain];
    [_currentMode release];
    _currentMode = mode;

    [state startingInMode:mode];

    if ([self _orderedPerforms]) {
        //[self _wakeUp];
    }
    if ([state fireFirstTimer]) {
        //[self _wakeUp];
    }
    [[NSNotificationQueue defaultQueue] asapProcessMode:mode];

    return [state limitDateForMode:mode];
}


-(void)acceptInputForMode:(NSString *)mode beforeDate:(NSDate *)date {
   NSRunLoopState *state=[self stateForMode:mode];

   if([[NSNotificationQueue defaultQueue] hasIdleNotificationsInMode:mode]){
    if(![state pollInputForMode:mode])
     [[NSNotificationQueue defaultQueue] idleProcessMode:mode];
   }
   else {
    [state waitForSingleInputForMode:mode beforeDate:date];
   }
}

-(void)invalidateTimerWithDelayedPerform:(NSDelayedPerform *)delayed {
   NSMapEnumerator state=NSEnumerateMapTable(_modes);
   NSString       *mode;
   NSRunLoopState *modeState;

   while(NSNextMapEnumeratorPair(&state,(void **)&mode,(void **)&modeState))
    [modeState invalidateTimerWithDelayedPerform:delayed];
}

@class NSSelectInputSource, NSSocket;

-(BOOL)runMode:(NSString *)mode beforeDate:(NSDate *)date {
   BOOL didProcessAnything=NO;

   do {
   NSAutoreleasePool *pool=[NSAutoreleasePool new];
   NSDate            *limitDate=[self limitDateForMode:mode];

    if(limitDate==nil){
     [pool release];
     return didProcessAnything;
    }

    limitDate=[limitDate earlierDate:date];

    didProcessAnything=YES;

    NSRunLoopState *state=[self stateForMode:mode];


    if([[NSNotificationQueue defaultQueue] hasIdleNotificationsInMode:mode]){
     if([state pollInputForMode:mode]){
      [pool release];
      return YES;
     }
     else {
      [[NSNotificationQueue defaultQueue] idleProcessMode:mode];
   }
    }
    else {
     [state waitForSingleInputForMode:mode beforeDate:limitDate];
     [pool release];
     return YES;
    }

   [pool release];

   }while([date timeIntervalSinceNow]>0);

   return YES;
}

-(void)runUntilDate:(NSDate *)date {
   while([self runMode:NSDefaultRunLoopMode beforeDate:date])
    if([date timeIntervalSinceNow]<=0)
     break;
}

-(void)run {
// Calling -run w/o a pool in place is valid, which is why we need one here
// for the NSDate. Could get rid of the pool if the date was not autoreleased.
   NSAutoreleasePool *pool=[NSAutoreleasePool new];
   NSDate            *future=[NSDate distantFuture];

   while([self runMode:NSDefaultRunLoopMode beforeDate:future])
    ;

   [pool release];
}

-(void)addPort:(NSPort *)port forMode:(NSString *)mode {
   NSUnimplementedMethod();
}

-(void)removePort:(NSPort *)port forMode:(NSString *)mode {
   NSUnimplementedMethod();
}

-(void)addInputSource:(NSInputSource *)source forMode:(NSString *)mode {
   NSArray *modeStates=[self statesForMode:mode];

   for(NSRunLoopState *state in modeStates)
    [state addInputSource:source];

}

-(void)removeInputSource:(NSInputSource *)source forMode:(NSString *)mode {
   NSArray *modeStates=[self statesForMode:mode];

   for(NSRunLoopState *state in modeStates)
    [state removeInputSource:source];
}

-(void)addTimer:(NSTimer *)timer forMode:(NSString *)mode {
   NSArray *modeStates=[self statesForMode:mode];

   for(NSRunLoopState *state in modeStates)
    [state addTimer:timer];
}

-(NSArray *)resolveCommonModes:(NSArray *)modes {
   NSMutableArray *result=[NSMutableArray array];

   for(NSString *check in modes)
    if([check isEqualToString:NSRunLoopCommonModes])
     [result addObjectsFromArray:_commonModes];
    else
     [result addObject:check];

   return result;
}

-(void)performSelector:(SEL)selector target:target argument:argument order:(NSUInteger)order modes:(NSArray *)modes {
   NSOrderedPerform *perform=[NSOrderedPerform orderedPerformWithSelector:selector target:target argument:argument order:order modes:[self resolveCommonModes:modes]];
	@synchronized(_orderedPerforms)
	{
		NSInteger count=[_orderedPerforms count];

		while(--count>=0){
			NSOrderedPerform *check=[_orderedPerforms objectAtIndex:count];
			NSUInteger          checkOrder=[check order];

			if(checkOrder>order)
				break;
		}
		[_orderedPerforms insertObject:perform atIndex:count+1];
	}
   [self _wakeUp];
}

-(void)cancelPerformSelector:(SEL)selector target:target argument:argument {
	@synchronized(_orderedPerforms)
	{
		NSInteger count=[_orderedPerforms count];

		while(--count>=0){
			NSOrderedPerform *check=[_orderedPerforms objectAtIndex:count];

			if([check selector]==selector && [check target]==target && [check argument]==argument)
				[_orderedPerforms removeObjectAtIndex:count];
		}
	}
}

-(void)cancelPerformSelectorsWithTarget:target {
	@synchronized(_orderedPerforms)
	{
		NSInteger count=[_orderedPerforms count];

		while(--count>=0){
			NSOrderedPerform *check=[_orderedPerforms objectAtIndex:count];

			if([check target]==target)
				[_orderedPerforms removeObjectAtIndex:count];
		}
	}
}


@end

@implementation NSObject(NSDelayedPerform)

+(void)cancelPreviousPerformRequestsWithTarget:target
       selector:(SEL)selector object:argument {
   NSDelayedPerform *delayed=[NSDelayedPerform delayedPerformWithObject:target selector:selector argument:argument];

   [[NSRunLoop currentRunLoop] invalidateTimerWithDelayedPerform:delayed];
}

+(void)cancelPreviousPerformRequestsWithTarget:target {
   NSDelayedPerform *delayed=[NSDelayedPerform delayedPerformWithObject:target selector:NULL argument:nil];

   [[NSRunLoop currentRunLoop] invalidateTimerWithDelayedPerform:delayed];
}

+(void)_delayedPerform:(NSTimer *)timer {
   NSDelayedPerform *delayed=[timer userInfo];

   [delayed perform];
}

+(void)object:object performSelector:(SEL)selector withObject:argument afterDelay:(NSTimeInterval)delay inModes:(NSArray *)modes {
   NSDelayedPerform *delayed=[NSDelayedPerform delayedPerformWithObject:object selector:selector argument:argument];
   NSTimer          *timer=[NSTimer timerWithTimeInterval:delay target:[NSObject class] selector:@selector(_delayedPerform:) userInfo:delayed repeats:NO];
   NSInteger         i,count=[modes count];

   for(i=0;i<count;i++)
    [[NSRunLoop currentRunLoop] addTimer:timer forMode:[modes objectAtIndex:i]];
}

-(void)performSelector:(SEL)selector withObject:object afterDelay:(NSTimeInterval)delay {
   [[self class] object:self performSelector:selector withObject:object afterDelay:delay inModes:[NSArray arrayWithObject:NSDefaultRunLoopMode]];
}

-(void)performSelector:(SEL)selector withObject:object afterDelay:(NSTimeInterval)delay inModes:(NSArray *)modes {
   [[self class] object:self performSelector:selector withObject:object afterDelay:delay inModes:modes];
}

@end

