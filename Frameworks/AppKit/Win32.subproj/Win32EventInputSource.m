/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
                 2009 Markus Hitter <mah@jump-ing.de>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/Win32EventInputSource.h>
#import <AppKit/Win32Display.h>
#import <AppKit/Win32Event.h>
#import <AppKit/NSEvent_periodic.h>
#import <AppKit/NSEvent_CoreGraphics.h>

@implementation Win32EventInputSource

/* We only post periodic events if there are no other normal events, otherwise
   a long event handling can constantly only generate periodics
 */
-(BOOL)processInputImmediately {
   BOOL hadPeriodic=[[Win32Display currentDisplay] containsAndRemovePeriodicEvents];
   MSG  msg;

   if(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){
    NSAutoreleasePool *pool=[NSAutoreleasePool new];
    
    BYTE keyState[256];
    BYTE *keyboardState=NULL;
    
    if(GetKeyboardState(keyState))
        keyboardState=keyState;
    
    if(![(Win32Display *)[Win32Display currentDisplay] postMSG:msg keyboardState:keyboardState]){
     Win32Event *cgEvent=[Win32Event eventWithMSG:msg];
     NSEvent    *event=[[[NSEvent_CoreGraphics alloc] initWithDisplayEvent:cgEvent] autorelease];

     [[Win32Display currentDisplay] postEvent:event atStart:NO];
    }
        
    [pool release];
    return YES;
   }

   if(hadPeriodic){
    NSEvent *event=[[[NSEvent_periodic alloc] initWithType:NSPeriodic location:NSMakePoint(0,0) modifierFlags:0 window:nil] autorelease];

    [[Win32Display currentDisplay] postEvent:event atStart:NO];
   }

   return NO;
}

-(NSUInteger)waitForEventsAndMultipleObjects:(HANDLE *)objects count:(NSUInteger)count milliseconds:(DWORD)milliseconds {
   if(count==0){
    UINT timer=SetTimer(NULL,0,milliseconds,NULL);

    WaitMessage();

    KillTimer(NULL,timer);
    return WAIT_TIMEOUT;
   }

   return MsgWaitForMultipleObjects(count,objects,FALSE,milliseconds,QS_ALLINPUT);
}


@end
