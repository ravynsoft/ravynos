/* Copyright (c) 2008 Johannes Fortmann
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS

#import <Foundation/NSConditionLock_win32.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDebug.h>

#define UPDATE_TIME time=MIN(MAX([date timeIntervalSinceNow]*1000.0, 0), INFINITE)

@implementation NSConditionLock_win32

-(BOOL)_waitForConditionBeforeDate:(NSDate *)date
{
   NSUInteger time=0;

   EnterCriticalSection(&_waitersNumber);
   _numberOfWaiters++;
   LeaveCriticalSection(&_waitersNumber);
   
   UPDATE_TIME;
   
   if(SignalObjectAndWait(_mutex, _semaphore, time, NO)) {
      EnterCriticalSection(&_waitersNumber);
      _numberOfWaiters--;
      LeaveCriticalSection(&_waitersNumber);

      return NO;
   }
   
   EnterCriticalSection(&_waitersNumber);

   _numberOfWaiters--;
   BOOL wereLast=_numberOfWaiters == 0 && _conditionWasBroadcast;
   
   LeaveCriticalSection(&_waitersNumber);
   
   UPDATE_TIME;
   
   if(wereLast)
   {
    NSCooperativeThreadBlocking();
    DWORD result=SignalObjectAndWait(_waitersDone, _mutex, time, NO);
    NSCooperativeThreadWaiting();
    
    if(result)
         return NO;
      }
   else
   {
    NSCooperativeThreadBlocking();
    DWORD result=WaitForSingleObject(_mutex, time);
    NSCooperativeThreadWaiting();
    
    if(result)
         return NO;
      }
   return YES;
}

-(void)_broadcastCondition {
   EnterCriticalSection(&_waitersNumber);
   _conditionWasBroadcast=_numberOfWaiters>0;

   if(_conditionWasBroadcast) {
      ReleaseSemaphore(_semaphore, _numberOfWaiters, 0);
      LeaveCriticalSection(&_waitersNumber);
      NSCooperativeThreadBlocking();
      WaitForSingleObject(_waitersDone, INFINITE);
      NSCooperativeThreadWaiting();
      _conditionWasBroadcast=NO;
   }
   else
      LeaveCriticalSection(&_waitersNumber);  
}

-(void)lockWhenCondition:(NSInteger)condition {
   [self lockWhenCondition:condition beforeDate:[NSDate distantFuture]];
}

-(void)tryLockWhenCondition:(NSInteger)condition {
   [self lockWhenCondition:condition beforeDate:[NSDate date]];
}

-(BOOL)lockWhenCondition:(NSInteger)condition beforeDate:(NSDate *)date; {
   NSUInteger time=0;
   UPDATE_TIME;

   DWORD result;
   
   NSCooperativeThreadBlocking();
   result=WaitForSingleObject(_mutex,time);
   NSCooperativeThreadWaiting();
    
   if(result!=0)
      return NO;

   while(_value!=condition) {
      if(![self _waitForConditionBeforeDate:date]) {
         return NO;
      }
   }
   return YES;
}

-(BOOL)lockBeforeDate:(NSDate *)date {
    NSUInteger time=0;
    UPDATE_TIME;

    DWORD result;
    
    NSCooperativeThreadBlocking();
    result=WaitForSingleObject(_mutex, time);
    NSCooperativeThreadWaiting();
    
    if(result!=0)
        return NO;

    return YES;
}

-(void)lock {
   [self lockBeforeDate:[NSDate distantFuture]];
}

-(void)unlockWithCondition:(NSInteger)condition {
   _value=condition;
   [self _broadcastCondition];
   ReleaseMutex(_mutex);
}

-(void)unlock {
   [self unlockWithCondition:_value];
}

-(id)init {
   return [self initWithCondition:0];
}

-(id)initWithCondition:(NSInteger)value {
   if((self = [super init])!=nil) {
      _value=value;
      _semaphore=CreateSemaphore(NULL, 0, INT_MAX, NULL);
      _waitersDone=CreateEvent(NULL, NO, NO, NULL);
      _mutex=CreateMutex(NULL, NO, NULL);
      InitializeCriticalSection(&_waitersNumber);
   }
   return self;
}

-(void)dealloc {
   CloseHandle(_semaphore);
   CloseHandle(_waitersDone);
   CloseHandle(_mutex);
   DeleteCriticalSection(&_waitersNumber);
   [_name release];
   [super dealloc];
}

-(NSInteger)condition {
    return _value;
}

@end
#endif

