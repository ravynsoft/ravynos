/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS
#import <Foundation/NSFileHandle_win32.h>
#import <Foundation/NSReadInBackground_win32.h>
#import <Foundation/NSPlatform_win32.h>
#import <Foundation/NSData.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>

@implementation NSFileHandle(ImplementedInSubclass)

+(Class)concreteSubclass {
   return [NSFileHandle_win32 class];
}

@end

@implementation NSFileHandle_win32

-initWithHandle:(HANDLE)handle closeOnDealloc:(BOOL)closeOnDealloc {
   _handle=handle;
   _closeOnDealloc=closeOnDealloc;
   return self;
}

-initWithFileDescriptor:(int)descriptor closeOnDealloc:(BOOL)closeOnDealloc {
   _handle=(HANDLE)descriptor;
   _closeOnDealloc=closeOnDealloc;
   return self;
}

-(void)dealloc {
   if(_closeOnDealloc)
    [self closeFile];

   [_background detach];

   [super dealloc];
}


+fileHandleForReadingAtPath:(NSString *)path {
   HANDLE handle=CreateFileW([path fileSystemRepresentationW],
    GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

   if(handle==NULL)
    return nil;

   return [[[self allocWithZone:NULL] initWithHandle:handle closeOnDealloc:YES] autorelease]; 
}

+fileHandleForWritingAtPath:(NSString *)path {
   HANDLE handle=CreateFileW([path fileSystemRepresentationW],
    GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

   if(handle==NULL)
    return nil;

   return [[[self allocWithZone:NULL] initWithHandle:handle closeOnDealloc:YES] autorelease]; 
}

+fileHandleForUpdatingAtPath:(NSString *)path {
   HANDLE handle=CreateFileW([path fileSystemRepresentationW],
    GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

   if(handle==NULL)
    return nil;

   return [[[self allocWithZone:NULL] initWithHandle:handle closeOnDealloc:YES] autorelease]; 
}

+fileHandleWithNullDevice {
   NSUnimplementedMethod();
   return nil;
}

+fileHandleWithStandardInput {
   return [[[self allocWithZone:NULL] initWithHandle:GetStdHandle(STD_INPUT_HANDLE) closeOnDealloc:NO] autorelease];
}

+fileHandleWithStandardOutput {
   return [[[self allocWithZone:NULL] initWithHandle:GetStdHandle(STD_OUTPUT_HANDLE) closeOnDealloc:NO] autorelease];
}

+fileHandleWithStandardError {
   return [[[self allocWithZone:NULL] initWithHandle:GetStdHandle(STD_ERROR_HANDLE) closeOnDealloc:NO] autorelease];
}

-(HANDLE)fileHandle {
   return _handle;
}

-(void)closeFile {
    if (_handle != NULL) {
        CloseHandle(_handle);
        _handle=NULL;
    }
}

-(void)synchronizeFile {
   if(!FlushFileBuffers(_handle))
    Win32Assert("FlushFileBuffers");
}

-(uint64_t)offsetInFile {
   LARGE_INTEGER li;
   
   li.QuadPart=0;
   
   li.LowPart=SetFilePointer(_handle,li.LowPart,&li.HighPart,FILE_CURRENT);
   
   if(li.LowPart==INVALID_SET_FILE_POINTER && GetLastError()!=NO_ERROR)
    [NSException raise:NSInvalidArgumentException format:@"GetLastError()=%d",GetLastError()];
    
   return li.QuadPart;
}

-(void)seekToFileOffset:(uint64_t)offset {
   LONG  highWord=offset>>32;

   SetFilePointer(_handle,offset&0xFFFFFFFF,&highWord,FILE_BEGIN);

//   Win32Assert("SetFilePointer");
}

-(uint64_t)seekToEndOfFile {
   uint64_t result=0;
   LONG  highWord=0;
   DWORD lowWord=SetFilePointer(_handle,0,&highWord,FILE_END);

   if(lowWord==INVALID_SET_FILE_POINTER)
    Win32Assert("SetFilePointer");

   result= highWord;
   result<<=32;
   result|=lowWord;

   return result;
}

-(NSData *)readDataOfLength:(NSUInteger)length {
   NSMutableData *result=[NSMutableData dataWithLength:length];
   DWORD          readLength;

   if(!ReadFile(_handle,[result mutableBytes],length,&readLength,NULL)){
    return nil;
   }

   [result setLength:readLength];

   return result;
}

- (NSData *)readDataToEndOfFile {
	#define B 4096
NSMutableData *result=[NSMutableData dataWithLength:B]; 
DWORD          readLength=0; 
DWORD		   sum=0;
DWORD			error;
LPVOID lpMsgBuf;
 

while(GetLastError()!=ERROR_BROKEN_PIPE && 
			  ReadFile(_handle,[result mutableBytes]+sum,B,&readLength,NULL))
	  { 
		       //[result appendBytes:buffer length:readLength]; 
			sum+=readLength;
		    [result increaseLengthBy:B];
	  }
	[result setLength:sum];
#undef B
	
	error=GetLastError();
	if(error!=ERROR_BROKEN_PIPE)
	{
		FormatMessage(
					  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					  FORMAT_MESSAGE_FROM_SYSTEM |
					  FORMAT_MESSAGE_IGNORE_INSERTS,
					  NULL,
					  error,
					  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					  (LPTSTR) &lpMsgBuf,
					  0, NULL );
	NSRaiseException(NSFileHandleOperationException, self, _cmd,
			@"read(%d) %s", _handle,lpMsgBuf);
		return nil;
	}

	return result;
}

- (NSData *)availableData {
    NSMutableData *mutableData = [NSMutableData dataWithLength:0];
    DWORD   count = 0;
    int     length = 0;
	DWORD   error;
	LPVOID  lpMsgBuf;
    
    do {
        [mutableData increaseLengthBy:4096];
        if (!ReadFile(_handle,  &((char*)[mutableData mutableBytes])[length], 4096,&count,NULL)) {
            error = GetLastError();
            if (error == ERROR_BROKEN_PIPE || error == ERROR_HANDLE_EOF) {
                break;
            }
            else {
                FormatMessage(
                              FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                              FORMAT_MESSAGE_FROM_SYSTEM |
                              FORMAT_MESSAGE_IGNORE_INSERTS,
                              NULL,
                              error,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                              (LPTSTR) &lpMsgBuf,
                              0, NULL );
                NSRaiseException(NSFileHandleOperationException, self, _cmd,
                                 @"read(%d) %s", _handle,lpMsgBuf);
                return nil;
                
            }

        }

        length += count;
        
        if (count < 4096 && GetFileType(_handle) == FILE_TYPE_DISK) {
            //end of file
            break;
        }
        
    } while(length <= 0 || count == 4096);
    
    [mutableData setLength:length];
    
    return mutableData;
}

-(void)writeData:(NSData *)data {
    DWORD bytesWritten=0;
    
    if(!WriteFile(_handle,[data bytes],[data length],&bytesWritten,NULL))
        Win32Assert("WriteFile");
}

-(void)truncateFileAtOffset:(uint64_t)offset {
   LONG  highWord=offset>>32;

   SetFilePointer(_handle,offset&0xFFFFFFFF,&highWord,FILE_BEGIN);
   SetEndOfFile(_handle);
}

-(void)readInBackground:(NSReadInBackground_win32 *)rib data:(NSData *)data {
   NSDictionary   *userInfo;
   NSNotification *note;

   userInfo=[NSDictionary dictionaryWithObject:data forKey:NSFileHandleNotificationDataItem];
   note=[NSNotification notificationWithName:NSFileHandleReadCompletionNotification object:self userInfo:userInfo];

   [_background detach];
   _background=nil;

   [[NSNotificationCenter defaultCenter] postNotification:note];
}

-(void)readInBackgroundAndNotifyForModes:(NSArray *)modes {
   if(_background!=nil)
    [NSException raise:NSInternalInconsistencyException format:@"file handle has background activity already"];

   _background=[NSReadInBackground_win32 readInBackgroundWithFileHandle:self modes:modes];
}

@end
#endif

