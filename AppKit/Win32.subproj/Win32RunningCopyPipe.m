/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/Win32RunningCopyPipe.h>
#import <Foundation/NSHandleMonitor_win32.h>
#import <AppKit/NSApplication.h>

@interface NSApplication(private)
-(void)_reopen;
@end

@implementation Win32RunningCopyPipe

static Win32RunningCopyPipe *_runningCopyPipe=nil;

-(void)startReading {
   _readOverlap.Offset=0;
   _readOverlap.OffsetHigh=0;
   _readOverlap.hEvent=_event;

   if(_state==STATE_CONNECTING){
      if(!ConnectNamedPipe(_pipe, &_readOverlap)){
         if(GetLastError()!=ERROR_IO_PENDING) {
            NSLog(@"ConnectNamedPipe failed");
         }
      }
   } else {
      if(!ReadFile(_pipe,_readBuffer, sizeof(_readBuffer), &_readCount ,&_readOverlap)){
         if(GetLastError()!=ERROR_IO_PENDING) {
            NSLog(@"ReadFile failed");
         }
      }
   }
}

-(void)handleMonitorIndicatesSignaled:(NSHandleMonitor_win32 *)monitor {
   if(!GetOverlappedResult(_pipe,&_readOverlap,&_readCount,TRUE)) {
      NSLog(@"GetOverlappedResult failed %d", GetLastError());
   }

   if(_state==STATE_CONNECTING) {
      _state=STATE_READING;

   } else {
      NSData *data = [NSData dataWithBytesNoCopy:_readBuffer length:_readCount freeWhenDone:NO];
      NSArray *nsOpen = [NSKeyedUnarchiver unarchiveObjectWithData:data];

      if(!DisconnectNamedPipe(_pipe)) {
         NSLog(@"DisconnectNamedPipe failed");
      }
      _state=STATE_CONNECTING;
	   if([nsOpen count] == 1 && [[nsOpen lastObject] isEqual:@"@reopen"]) {
         [NSApp _reopen];
      } else {
         [[NSUserDefaults standardUserDefaults] setObject:nsOpen forKey:@"NSOpen"];
         [NSApp performSelector:@selector(openFiles)];
      }
   }

   [self startReading];
}

-initWithPipeHandle:(HANDLE)pipeHandle {
   _pipe=pipeHandle;

   _event=CreateEvent(NULL,FALSE,FALSE,NULL);
   _eventMonitor=[[NSHandleMonitor_win32 handleMonitorWithHandle:_event] retain];
   [_eventMonitor setDelegate:self];
   [_eventMonitor setCurrentActivity:Win32HandleSignaled];
   [[NSRunLoop currentRunLoop] addInputSource:_eventMonitor forMode:NSDefaultRunLoopMode];
   _state=STATE_CONNECTING;

   [self startReading];

   return self;
}

-(void)invalidate {
   [[NSRunLoop currentRunLoop] removeInputSource:_eventMonitor forMode:NSDefaultRunLoopMode];
   CloseHandle(_event);
   CloseHandle(_pipe);
}

+(void)invalidateRunningCopyPipe {
   [_runningCopyPipe invalidate];
}

+(NSString *)pipeName {
   return [NSString stringWithFormat:@"\\\\.\\pipe\\%@~V1~",[[NSProcessInfo processInfo] processName]];
}

+(BOOL)createRunningCopyPipe {
   if(_runningCopyPipe==nil){
    HANDLE pipe=CreateNamedPipe([[self pipeName] cString],
      PIPE_ACCESS_INBOUND|FILE_FLAG_OVERLAPPED,
      PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,
      1,8192,8192,0,NULL);

    if(pipe==INVALID_HANDLE_VALUE)
     return NO;

    _runningCopyPipe=[[Win32RunningCopyPipe alloc] initWithPipeHandle:pipe];
    return YES;
   }

   return YES;
}

+(void)_startRunningCopyPipe {
   if(![self createRunningCopyPipe]) {
      NSArray *paths;

      if([[NSUserDefaults standardUserDefaults] stringForKey:@"NSUseRunningCopy"]!=nil &&
         ![[NSUserDefaults standardUserDefaults] boolForKey:@"NSUseRunningCopy"]) {

         return;
      }

      id nsOpen = [[NSUserDefaults standardUserDefaults] objectForKey:@"NSOpen"];

      if ([nsOpen isKindOfClass:[NSString class]] && [nsOpen length]) {
         paths = [NSArray arrayWithObject:nsOpen];
      } else if ([nsOpen isKindOfClass:[NSArray class]] && [nsOpen count]) {
         paths = nsOpen;
      } else {
         paths = [NSArray arrayWithObject:@"@reopen"];
      }

      HANDLE pipe=CreateFile([[self pipeName] cString],GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

      if(pipe==INVALID_HANDLE_VALUE) {
         return;
      }

      NSData *data = [NSKeyedArchiver archivedDataWithRootObject:paths];
      DWORD written = 0;

      if(!WriteFile(pipe, [data bytes], [data length], &written,NULL) || written != [data length]) {
         NSLog(@"WriteFile failed");
      }
      exit(0);
   }
}

+(void)startRunningCopyPipe {
   NSAutoreleasePool *pool=[NSAutoreleasePool new];
   
   [self _startRunningCopyPipe];

   [pool release];
}

@end
