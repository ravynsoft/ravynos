#ifdef WINDOWS
/* Copyright (c) 1010 Sven Weidauer
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSCondition_win32.h"
#import <Foundation/NSDebug.h>

@implementation NSCondition_win32

-init {
	if (nil == [super init]) return nil;

	InitializeCriticalSection( &lock );
	
	events[Event_Signal] = CreateEvent( 0, FALSE, FALSE, 0 );
	events[Event_Broadcast] = CreateEvent( 0, TRUE, FALSE, 0 );
	waitersCount = 0;
	
	return self;
}

-(void) dealloc {
	[self lock];
	CloseHandle( events[Event_Signal] );
	CloseHandle( events[Event_Broadcast] );
	[self unlock];
	DeleteCriticalSection( &lock );
	
	[super dealloc];
}

-(void) lock {
	EnterCriticalSection( &lock );
}

-(void) unlock {
	LeaveCriticalSection( &lock );
}

-(void) wait {
	InterlockedIncrement( &waitersCount );
	
	[self unlock];
    
    NSCooperativeThreadBlocking();
    
	int result = WaitForMultipleObjects( Event_Count, events, FALSE, INFINITE );

    NSCooperativeThreadWaiting();

	LONG newWaitersCount = InterlockedDecrement( &waitersCount );
	BOOL lastWaiter = (result == WAIT_OBJECT_0 + Event_Broadcast) && (0 == newWaitersCount);
	
	if (lastWaiter) ResetEvent( events[Event_Broadcast] );
	
	[self lock];
}

-(void) signal;
{
	BOOL haveWaiters = waitersCount > 0;
	
	if (haveWaiters) SetEvent( events[Event_Signal] );
}

- (void) broadcast;
{
	BOOL haveWaiters = waitersCount > 0;
	
	if (haveWaiters) SetEvent( events[Event_Broadcast] );	
}

@end
#endif
