/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS
#import <Foundation/NSReadInBackground_win32.h>
#import <Foundation/NSFileHandle_win32.h>
#import <Foundation/NSHandleMonitor_win32.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#include <stdio.h>

@implementation NSReadInBackground_win32

static DWORD WINAPI readInBackground(LPVOID arg){
   NSReadInBackground_win32 *self=arg;
   DWORD                  numberRead=0;

   self->_bufferSize=0;
   if(ReadFile(self->_readHandle,self->_buffer,self->_bufferCapacity,&numberRead,NULL))
    self->_bufferSize=numberRead;

   return 0;
}

-initWithFileHandle:(NSFileHandle *)fileHandle modes:(NSArray *)modes {
   NSInteger    i,count=[modes count];
   DWORD  threadID;

   _fileHandle=fileHandle;
   _readHandle=[(NSFileHandle_win32 *)_fileHandle fileHandle];
   _bufferCapacity=4096;
   _bufferSize=0;
   _buffer=NSZoneMalloc([self zone],_bufferCapacity);

   _modes=[modes copy];
   _threadHandle=CreateThread(NULL,0,readInBackground,self,0,&threadID);
   _threadMonitor=[[NSHandleMonitor_win32 handleMonitorWithHandle:_threadHandle] retain];
   [_threadMonitor setDelegate:self];

   for(i=0;i<count;i++)
    [[NSRunLoop currentRunLoop] addInputSource:_threadMonitor forMode:[modes objectAtIndex:i]];

   return self;
}

+readInBackgroundWithFileHandle:(NSFileHandle *)fileHandle modes:(NSArray *)modes {
// we autorelease when things are done
   return [[self alloc] initWithFileHandle:fileHandle modes:modes];
}

-(void)dealloc {
   [_threadMonitor release];

   CloseHandle(_threadHandle);

   [_modes release];

   NSZoneFree([self zone],_buffer);

   [super dealloc];
}

-(void)detach {
   _fileHandle=nil;
}

-(void)_removeFromModes {
   NSInteger i,count=[_modes count];

   for(i=0;i<count;i++)
    [[NSRunLoop currentRunLoop] removeInputSource:_threadMonitor forMode:[_modes objectAtIndex:i]];

   [_threadMonitor setDelegate:nil];
}

-(void)handleMonitorIndicatesSignaled:(NSHandleMonitor_win32 *)monitor {
   NSData         *data;

   data=[NSData dataWithBytes:_buffer length:_bufferSize];
   [_fileHandle performSelector:@selector(readInBackground:data:) withObject:self withObject:data];
  // [_fileHandle readInBackground:self data:data];
   [self _removeFromModes];
   [self autorelease];
}

@end
#endif
