/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSRunLoopState.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSDelayedPerform.h>
#import <Foundation/NSInputSourceSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSSocket.h>
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSPipe.h>
#import <Foundation/NSRaise.h>

@class NSCancelInputSource;

@interface NSInputSource (Canceling)
-(void)cancel;
@end

@implementation NSRunLoopState

-init {
   // This is implemented in the platform specific class
   NSInvalidAbstractInvocation();
   return nil;
}

-(void)dealloc {
   [_cancelSource release];
   [_inputSourceSet release];
   [_asyncInputSourceSets release];
   [_timers release];
   [super dealloc];
}

-(void)wakeUp {
   [_cancelSource cancel];
}

-(void)addTimer:(NSTimer *)timer {
   [_timers addObject:timer];
}

-(void)startingInMode:(NSString *)mode {
   NSInteger i,count=[_asyncInputSourceSets count];

   [_inputSourceSet startingInMode:mode];

   for(i=0;i<count;i++)
    [[_asyncInputSourceSets objectAtIndex:i] startingInMode:mode];
}

-(BOOL)fireFirstTimer {
   NSDate   *now=[NSDate date];
   NSInteger i,count=[_timers count];
   NSTimer  *fireTimer=nil;
      
   for(i=0;i<count;i++){
    NSTimer *check=[_timers objectAtIndex:i];
    
    if([check isValid] && [now compare:[check fireDate]]!=NSOrderedAscending) {
     fireTimer=[check retain];
     [_timers removeObjectAtIndex:i];
     break;
    }
   }
   
   if(fireTimer!=nil){
    [fireTimer fire];
    [_timers addObject:fireTimer];
    [fireTimer release];
   }
      
   count=[_timers count];
   while(--count>=0)
    if(![[_timers objectAtIndex:count] isValid])
     [_timers removeObjectAtIndex:count];
     
   return (fireTimer!=nil)?YES:NO;
}

-(NSDate *)limitDateForMode:(NSString *)mode {
   NSDate *limit=nil;
   NSInteger     count;

   count=[_timers count];
   while(--count>=0){
    NSTimer *timer=[_timers objectAtIndex:count];

    if(![timer isValid])
     [_timers removeObjectAtIndex:count];
    else {
     if(limit==nil){
      limit=[timer fireDate];
     }
     else{
      limit=[limit earlierDate:[timer fireDate]];
     }
    }
   }
   
   if(limit==nil){
    if([[_inputSourceSet validInputSources] count]>0)
     limit=[NSDate distantFuture];
   }
   
   return limit;
}

-(NSInputSourceSet *)inputSourceSetForInputSource:(NSInputSource *)source {

   if([_inputSourceSet recognizesInputSource:source])
    return _inputSourceSet;
   else {
    NSInteger i,count=[_asyncInputSourceSets count];
    
    for(i=0;i<count;i++){
     NSInputSourceSet *check=[_asyncInputSourceSets objectAtIndex:i];
     
     if([check recognizesInputSource:source])
      return check;
    }
   }

   return nil;
}

-(void)addInputSource:(NSInputSource *)source {
   [[self inputSourceSetForInputSource:source] addInputSource:source];
}

-(void)removeInputSource:(NSInputSource *)source {
   [[self inputSourceSetForInputSource:source] removeInputSource:source];
}

-(void)invalidateTimerWithDelayedPerform:(NSDelayedPerform *)delayed {
   NSInteger count=[_timers count];

   while(--count>=0){
    NSTimer          *timer=[_timers objectAtIndex:count];
    NSDelayedPerform *check=[timer userInfo];

    if([check isKindOfClass:[NSDelayedPerform class]]){
     if([check isEqualToPerform:delayed])
      [timer invalidate];
    }
   }
}

-(BOOL)fireSingleImmediateInputInMode:(NSString *)mode {
    NSInteger i,count=[_asyncInputSourceSets count];
    
    for(i=0;i<count;i++)
    if([[_asyncInputSourceSets objectAtIndex:i] fireSingleImmediateInputInMode:mode])
      return YES;
      
   return [_inputSourceSet fireSingleImmediateInputInMode:mode];
   }

-(BOOL)waitForSingleInputForMode:(NSString *)mode beforeDate:(NSDate *)date {
   if([self fireSingleImmediateInputInMode:mode])
    return YES;
   else {
    NSInteger i,count=[_asyncInputSourceSets count];
    
    for(i=0;i<count;i++)
     [[_asyncInputSourceSets objectAtIndex:i] waitInBackgroundInMode:mode];
      
    return [_inputSourceSet waitForInputInMode:mode beforeDate:date];
   }
}

-(BOOL)pollInputForMode:(NSString *)mode {
   return [self waitForSingleInputForMode:mode beforeDate:[NSDate date]];
}

-(id)description {
   return [NSString stringWithFormat:@"%@, %i inputSources", [super description], [_inputSourceSet count]];
}

@end
