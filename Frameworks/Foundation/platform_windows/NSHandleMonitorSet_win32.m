/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS
#import <Foundation/NSHandleMonitorSet_win32.h>
#import <Foundation/NSHandleMonitor_win32.h>
#import <Foundation/NSPlatform_win32.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSDebug.h>

#include <windows.h>

@interface NSInputSource(Win32EventInputSource)
-(NSUInteger)waitForEventsAndMultipleObjects:(HANDLE *)objects count:(NSUInteger)count milliseconds:(DWORD)milliseconds;
@end

@implementation NSHandleMonitorSet_win32

-init {
   [super init];
   _eventInputSource=nil;
   return self;
}

-(void)dealloc {
   [_eventInputSource release];
   [super dealloc];
}

-(NSUInteger)count {
   return [[self validInputSources] count];
}

-(BOOL)recognizesInputSource:(NSInputSource *)source {
   if([source isKindOfClass:[NSHandleMonitor_win32 class]])
    return YES;
   if([source respondsToSelector:@selector(waitForEventsAndMultipleObjects:count:milliseconds:)])
    return YES;
   return NO;
}

-(void)addInputSource:(NSInputSource *)source {
   [super addInputSource:source];

   if([source respondsToSelector:@selector(waitForEventsAndMultipleObjects:count:milliseconds:)]){
    [_eventInputSource autorelease];
    _eventInputSource=[source retain];
   }
}

-(void)removeInputSource:(NSInputSource *)source {
   [super removeInputSource:source];
   
   if(source==_eventInputSource){
    [_eventInputSource autorelease];
    _eventInputSource=nil;
   }
}

-(NSHandleMonitor_win32 *)monitorWithHandle:(void *)handle {
   NSEnumerator          *state=[[self validInputSources] objectEnumerator];
   NSHandleMonitor_win32 *monitor;

   while((monitor=[state nextObject])!=nil){
    if([monitor isKindOfClass:[NSHandleMonitor_win32 class]] && ([monitor handle]==handle))
     return monitor;
   }
   return nil;
}

-(NSHandleMonitor_win32 *)waitForHandleActivityBeforeDate:(NSDate *)date mode:(NSString *)mode {
   NSSet                 *validSources=[self validInputSources];
   NSEnumerator          *state=[validSources objectEnumerator];
   NSHandleMonitor_win32    *monitor;
   NSTimeInterval         interval=[date timeIntervalSinceNow];
   DWORD                  msec;
   HANDLE                 objectList[[validSources count]];
   int                    objectCount=0;
   DWORD                  waitResult;

   objectCount=0;
   while((monitor=[state nextObject])!=nil)
    if([monitor isKindOfClass:[NSHandleMonitor_win32 class]])
     objectList[objectCount++]=[monitor handle];

   if(interval>1000000)
    interval=10000;
   if(interval<0)
    interval=0;

   msec=interval*1000;

   NSCooperativeThreadBlocking();

   if(_eventInputSource!=nil){
    waitResult=[_eventInputSource waitForEventsAndMultipleObjects:objectList count:objectCount milliseconds:msec];
   }
   else {
    if(objectCount==0){
     Win32ThreadSleepForTimeInterval(interval);
     waitResult=WAIT_TIMEOUT;
    }
    else {
     waitResult=WaitForMultipleObjects(objectCount,objectList,FALSE,msec);
    }
   }

   NSCooperativeThreadWaiting();
   
   if(waitResult==WAIT_FAILED)
    Win32Assert("WaitForMultipleObjects");

   if(waitResult==WAIT_TIMEOUT)
    return nil;

   if(waitResult>=WAIT_OBJECT_0 && (waitResult<WAIT_OBJECT_0+objectCount)){
    DWORD index=waitResult-WAIT_OBJECT_0;
    NSHandleMonitor_win32 *result=[self monitorWithHandle:objectList[index]];

    [result setCurrentActivity:Win32HandleSignaled];
    return result;
   }

   if(waitResult>=WAIT_ABANDONED_0 && waitResult<(WAIT_ABANDONED_0+objectCount)){
    DWORD index=waitResult-WAIT_ABANDONED_0;
    NSHandleMonitor_win32 *result=[self monitorWithHandle:objectList[index]];

    [result setCurrentActivity:Win32HandleAbandoned];
    return result;
   }

   return nil;
}

-(BOOL)waitForInputInMode:(NSString *)mode beforeDate:(NSDate *)date {

   if([[self validInputSources] count]>0){
    NSHandleMonitor_win32 *monitor=[self waitForHandleActivityBeforeDate:date mode:mode];

    [monitor notifyDelegateOfCurrentActivity];

    return (monitor!=nil)?YES:NO;
   }

   [NSThread sleepUntilDate:date];
   return NO;
}

@end
#endif

