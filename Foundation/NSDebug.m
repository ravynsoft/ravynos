/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSDebug.h>
#import <Foundation/NSString.h>
#ifdef WINDOWS
#include <windows.h>
#endif

#if defined(LINUX) ||  defined(__APPLE__)  ||  defined(__FreeBSD__)
#include <execinfo.h>
#endif

BOOL NSZombieEnabled=NO;
BOOL NSDebugEnabled=NO;
BOOL NSCooperativeThreadsEnabled=NO;

const char* _NSPrintForDebugger(id object) {
	if(object && [object respondsToSelector:@selector(description)]) {
		return [[object description] UTF8String];
	}
	return NULL;
}

#ifndef WINDOWS
void NSCooperativeThreadBlocking() {
}

void NSCooperativeThreadWaiting() {
}
#else


static HANDLE NSCooperativeEvent(){
   static HANDLE handle=NULL;

   if(handle==NULL)
    handle=CreateEvent(NULL,FALSE,FALSE,NULL);

   return handle;
}

void NSCooperativeThreadBlocking() {
   if(NSCooperativeThreadsEnabled){
    SetEvent(NSCooperativeEvent());
   }
}

void NSCooperativeThreadWaiting() {
   if(NSCooperativeThreadsEnabled){
    WaitForSingleObject(NSCooperativeEvent(),0);
   }
}
#endif

#define _NS_RETURN_ADDRESS(x) case x: return __builtin_return_address(x + 1)

void *NSFrameAddress(NSUInteger level)
{
    void* callstack[128];

    if (level > 128) {
        return NULL;
    }
    
    int i, frameCount = backtrace(callstack, level + 1);
    if (frameCount < level + 1) {
        return NULL;
    }
    
    return callstack[level + 1];
}

unsigned NSCountFrames(void)
{
    unsigned    x = 0;

    while (NSFrameAddress(x + 1) != NULL){
     x++;
    }

    return x;
}

void *NSReturnAddress(int level)
{
    switch (level) {
            _NS_RETURN_ADDRESS(0);
            _NS_RETURN_ADDRESS(1);
            _NS_RETURN_ADDRESS(2);
            _NS_RETURN_ADDRESS(3);
            _NS_RETURN_ADDRESS(4);
            _NS_RETURN_ADDRESS(5);
            _NS_RETURN_ADDRESS(6);
            _NS_RETURN_ADDRESS(7);
            _NS_RETURN_ADDRESS(8);
            _NS_RETURN_ADDRESS(9);
            _NS_RETURN_ADDRESS(10);
            _NS_RETURN_ADDRESS(11);
            _NS_RETURN_ADDRESS(12);
            _NS_RETURN_ADDRESS(13);
            _NS_RETURN_ADDRESS(14);
            _NS_RETURN_ADDRESS(15);
            _NS_RETURN_ADDRESS(16);
            _NS_RETURN_ADDRESS(17);
            _NS_RETURN_ADDRESS(18);
            _NS_RETURN_ADDRESS(19);
            _NS_RETURN_ADDRESS(20);
            _NS_RETURN_ADDRESS(21);
            _NS_RETURN_ADDRESS(22);
            _NS_RETURN_ADDRESS(23);
            _NS_RETURN_ADDRESS(24);
            _NS_RETURN_ADDRESS(25);
            _NS_RETURN_ADDRESS(26);
            _NS_RETURN_ADDRESS(27);
            _NS_RETURN_ADDRESS(28);
            _NS_RETURN_ADDRESS(29);
            _NS_RETURN_ADDRESS(30);
            _NS_RETURN_ADDRESS(31);
            _NS_RETURN_ADDRESS(32);
            _NS_RETURN_ADDRESS(33);
            _NS_RETURN_ADDRESS(34);
            _NS_RETURN_ADDRESS(35);
            _NS_RETURN_ADDRESS(36);
            _NS_RETURN_ADDRESS(37);
            _NS_RETURN_ADDRESS(38);
            _NS_RETURN_ADDRESS(39);
            _NS_RETURN_ADDRESS(40);
            _NS_RETURN_ADDRESS(41);
            _NS_RETURN_ADDRESS(42);
            _NS_RETURN_ADDRESS(43);
            _NS_RETURN_ADDRESS(44);
            _NS_RETURN_ADDRESS(45);
            _NS_RETURN_ADDRESS(46);
            _NS_RETURN_ADDRESS(47);
            _NS_RETURN_ADDRESS(48);
            _NS_RETURN_ADDRESS(49);
            _NS_RETURN_ADDRESS(50);
        default: return NULL;
    }

    return NULL;
}
