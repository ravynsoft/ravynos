/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS

#import <Foundation/NSTask_win32.h>
#import <Foundation/NSFileHandle_win32.h>
#import <Foundation/NSHandleMonitor_win32.h>
#import <Foundation/NSRunLoop-InputSource.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString_win32.h>
#import <Foundation/NSPlatform_win32.h>
#include <windows.h>
#import <Foundation/NSPropertyListWriter_vintage.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSException.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSString.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSPipe.h>
#import <Foundation/NSPathUtilities.h>

@implementation NSTask_win32

// private
-(void)finalizeProcess{

   if(_monitor!=nil){
    [[NSRunLoop currentRunLoop] removeInputSource:_monitor forMode: NSDefaultRunLoopMode];
    [_monitor setDelegate:nil];
    [_monitor autorelease];
    _monitor=nil;

    CloseHandle(_processInfo.hProcess);
    CloseHandle(_processInfo.hThread);
   }
}

-(void)dealloc{
   [self finalizeProcess];
   [super dealloc];
}

-(NSData *)_argumentsData {
   NSMutableData *data=[NSMutableData data];
   NSInteger            i,count=[arguments count];

    if (launchPath != nil) {
        [data appendData:NSTaskArgumentDataFromString(launchPath)];
        [data appendBytes:" " length:1];
    }

   for(i=0;i<count;i++){
    NSString *argument=[arguments objectAtIndex:i];

    [data appendData:NSTaskArgumentDataFromString(argument)];
    [data appendBytes:" " length:1];
    
    if ([data length] > 32767){
     [NSException raise:NSInvalidArgumentException format:@"More than 32768 bytes needed for argument list of task %@.", launchPath];
     return nil;
    }
   }

   [data appendBytes:"\0" length:1];

   return data;
}

-(void)launch {
    if ([self isRunning]) {
        [NSException raise:NSInvalidArgumentException
                    format:@"NSTask already launched"];
    }
   STARTUPINFO   startupInfo;

    // For CreateProcess it's not actually an error for the launchPath to be nil
    // From the MS documentation: The lpApplicationName parameter can be NULL. In that case, the module name must be the first white space–delimited token in the lpCommandLine string.
    // See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms682425(v=vs.85).aspx
    
   ZeroMemory(&startupInfo,sizeof(startupInfo));
   startupInfo.cb=sizeof(startupInfo);
   startupInfo.dwFlags|=STARTF_USESTDHANDLES;

   if(standardInput==nil)
    startupInfo.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
   else if([standardInput isKindOfClass:[NSPipe class]])
    startupInfo.hStdInput=[(NSFileHandle_win32 *)[standardInput fileHandleForReading] fileHandle];
   else
    startupInfo.hStdInput=[standardInput fileHandle];
      
    SetHandleInformation([(NSFileHandle_win32 *)[standardInput fileHandleForWriting] fileHandle], HANDLE_FLAG_INHERIT, 0);

    if(standardOutput==nil)
    startupInfo.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);
   else if([standardOutput isKindOfClass:[NSPipe class]])
    startupInfo.hStdOutput=[(NSFileHandle_win32 *)[standardOutput fileHandleForWriting] fileHandle];
   else
    startupInfo.hStdOutput=[standardOutput fileHandle];
    
    SetHandleInformation([(NSFileHandle_win32 *)[standardOutput fileHandleForReading] fileHandle], HANDLE_FLAG_INHERIT, 0);

   if(standardError==nil)
     startupInfo.hStdError=GetStdHandle(STD_ERROR_HANDLE);
   else if([standardError isKindOfClass:[NSPipe class]])
    startupInfo.hStdError=[(NSFileHandle_win32 *)[standardError fileHandleForWriting] fileHandle];
   else
    startupInfo.hStdError=[standardError fileHandle];
    
    SetHandleInformation([(NSFileHandle_win32 *)[standardError fileHandleForReading] fileHandle], HANDLE_FLAG_INHERIT, 0);
    
    ZeroMemory(& _processInfo,sizeof(_processInfo));
    
    char    *cenv = NULL, *cenvp = NULL;
    if(environment != nil) {
        //alloc enough space for environment
        //maybe a better solution is with realloc
        cenv =  NSZoneMalloc(NULL, [[[environment description] dataUsingEncoding:[NSString defaultCStringEncoding]] length]); 
        cenvp = cenv;
        
        NSString *key;
        
        for (key in environment) {
            id          value = [environment objectForKey:key];
            NSData		*data;
            
            data = [key dataUsingEncoding:[NSString defaultCStringEncoding]];
            memcpy(cenvp, [data bytes], [data length]);
            cenvp += [data length];
            *cenvp++ = '=';
            data = [value dataUsingEncoding:[NSString defaultCStringEncoding]];
            memcpy(cenvp, [data bytes], [data length]);
            cenvp += [data length];
            *cenvp++ = 0;
        }
        *cenvp++ = 0;
        
    }
    
    
    
   if(!CreateProcess([[self launchPath] fileSystemRepresentation],
    (char *)[[self _argumentsData] bytes],
    NULL,NULL,TRUE,CREATE_NO_WINDOW,cenv,
    [currentDirectoryPath fileSystemRepresentation],
    &startupInfo,&_processInfo)){

    DWORD   lastError=GetLastError();

    if(cenv) {
        NSZoneFree(NULL, cenv);
    }
    char *launchPathStr = "<nil>";
    if ([launchPath length] > 0) {
       launchPathStr = (char *)[launchPath fileSystemRepresentation];
    }
    [NSException raise:NSInvalidArgumentException
                format:@"CreateProcess(\"%s\", \"%s\", \"%s\") failed with error: %d", launchPathStr, [[self _argumentsData] bytes], [currentDirectoryPath fileSystemRepresentation], lastError];
    return;
   }
    if(cenv) {
        NSZoneFree(NULL, cenv);
    }

   if([standardInput isKindOfClass:[NSPipe class]])
    [[standardInput fileHandleForReading] closeFile];
   if([standardOutput isKindOfClass:[NSPipe class]])
    [[standardOutput fileHandleForWriting] closeFile];
   if([standardError isKindOfClass:[NSPipe class]])
    [[standardError fileHandleForWriting] closeFile];

   _monitor=[[NSHandleMonitor_win32 allocWithZone:NULL] initWithHandle:_processInfo.hProcess];
   [_monitor setDelegate:self];
   [[NSRunLoop currentRunLoop] addInputSource:_monitor forMode: NSDefaultRunLoopMode];
}

-(BOOL)isRunning
{
    if ( _processInfo.hProcess != NULL) {
        GetExitCodeProcess(_processInfo.hProcess, &_exitCode);
        if (_exitCode == STILL_ACTIVE) {
            return YES;
        }
        else {
            return NO;
        }
    }
    else {
        return NO;
    }
}

-(void)terminate {
   TerminateProcess(_processInfo.hProcess,0);
}

-(int)terminationStatus {
    return (int)_exitCode;
}	

-(void)handleMonitorIndicatesSignaled:(NSHandleMonitor_win32 *)monitor {

   GetExitCodeProcess(_processInfo.hProcess,&_exitCode);

   if(_exitCode!=STILL_ACTIVE){
    [self finalizeProcess];
    [[NSNotificationCenter defaultCenter] postNotificationName:NSTaskDidTerminateNotification object:self];
   }
}

-(void)handleMonitorIndicatesAbandoned:(NSHandleMonitor_win32 *)monitor {

   NSLog(@"process abandoned ?");

   [self finalizeProcess];
   [[NSNotificationCenter defaultCenter] postNotificationName:NSTaskDidTerminateNotification object:self];
}

-(int)processIdentifier {
   return _processInfo.dwProcessId;
}

@end
#endif

