/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_IS_POSIX
#import <Foundation/NSFileHandle_posix.h>
#import <Foundation/NSString.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSData.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSException.h>
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPlatform_posix.h>
#import <Foundation/NSRaiseException.h>
#import "NSSocket_bsd.h"
#import <Foundation/NSThread.h>
#import <Foundation/NSAutoreleasePool.h>

#include <unistd.h>
#import <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#import <sys/un.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#import <netinet/in.h>

@implementation NSFileHandle(ImplementedInSubclass)

+(Class)concreteSubclass {
   return [NSFileHandle_posix class];
}

@end

@implementation NSFileHandle_posix

- (id)initWithFileDescriptor:(int)fileDescriptor closeOnDealloc:(BOOL)closeOnDealloc {
    _fileDescriptor = fileDescriptor;
    _closeOnDealloc = closeOnDealloc;

    return self;
}

- (void)dealloc {
    if (_inputSource != nil)
        [self cancelBackgroundMonitoring];

    if (_closeOnDealloc == YES)
        [self closeFile];

    [super dealloc];
}

static int descriptorForPath(NSString *path,int modes){
    NSDictionary *fileAttributes = [[NSFileManager defaultManager] fileAttributesAtPath:path traverseLink:YES];

    if (fileAttributes == nil)
        return -1;

    if ([[fileAttributes objectForKey:NSFileType] isEqual:NSFileTypeSocket]) {
     int fd;

        if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
         return -1;
        else {
            socklen_t len;
            struct sockaddr_un remote;

            remote.sun_family = AF_UNIX;
            strcpy(remote.sun_path, [path fileSystemRepresentation]);
            len =(socklen_t)(strlen(remote.sun_path) + sizeof(remote.sun_family));
            if (connect(fd, (struct sockaddr *)&remote, len) == -1) {
                close(fd);
                return -1;
            }
        }
        return fd;
    } else {
        return open([path fileSystemRepresentation], modes, FOUNDATION_FILE_MODE);
    }
}

+fileHandleForReadingAtPath:(NSString *)path {
    int fd=descriptorForPath(path,O_RDONLY);

    if (fd == -1)
        return nil;

    return [[[self allocWithZone:NULL] initWithFileDescriptor:fd] autorelease];
}

+fileHandleForWritingAtPath:(NSString *)path {
    int fd=descriptorForPath(path,O_WRONLY|O_CREAT);

    if (fd == -1)
        return nil;

    return [[[self allocWithZone:NULL] initWithFileDescriptor:fd] autorelease];
}

+fileHandleForUpdatingAtPath:(NSString *)path {
    int fd=descriptorForPath(path,O_RDWR);

    if (fd == -1)
        return nil;

    return [[[self allocWithZone:NULL] initWithFileDescriptor:fd] autorelease];
}

+fileHandleWithNullDevice {
    return [self fileHandleForUpdatingAtPath:@"/dev/null"];
}

+fileHandleWithStandardInput {
    return [[[self allocWithZone:NULL] initWithFileDescriptor:STDIN_FILENO closeOnDealloc:NO] autorelease];
}

+fileHandleWithStandardOutput {
    return [[[self allocWithZone:NULL] initWithFileDescriptor:STDOUT_FILENO closeOnDealloc:NO] autorelease];
}

+fileHandleWithStandardError {
    return [[[self allocWithZone:NULL] initWithFileDescriptor:STDERR_FILENO closeOnDealloc:NO] autorelease];
}

/*
CONFORMING TO
       POSIX.1b (formerly POSIX.4)
 */
- (void)closeFile {
    if (_fileDescriptor == -1)
        return;
    if (close(_fileDescriptor) == -1)
    {
        NSRaiseException(NSFileHandleOperationException, self, _cmd,
                         @"close(%d): %s", _fileDescriptor, strerror(errno));
    }
    _fileDescriptor = -1;
}

- (void)synchronizeFile {
    if (fsync(_fileDescriptor) == -1)
        NSRaiseException(NSFileHandleOperationException, self, _cmd,
                         @"fsync(%d): %s", _fileDescriptor, strerror(errno));
}

- (uint64_t)offsetInFile {
    uint64_t result = lseek(_fileDescriptor, 0, SEEK_CUR);

    if (result == -1) {
        NSRaiseException(NSFileHandleOperationException, self, _cmd,
                         @"lseek(%d):", _fileDescriptor, strerror(errno));
        return -1;
    }

    return result;
}

- (void)seekToFileOffset:(uint64_t)offset {
    if (lseek(_fileDescriptor, offset, SEEK_SET) == -1)
        NSRaiseException(NSFileHandleOperationException, self, _cmd,
                         @"lseek(%d): %s", _fileDescriptor, strerror(errno));
}

- (uint64_t)seekToEndOfFile {
    uint64_t result = lseek(_fileDescriptor, 0, SEEK_END);
    if (result == -1) {
        NSRaiseException(NSFileHandleOperationException, self, _cmd,
                         @"lseek(%d): %s", _fileDescriptor, strerror(errno));
        return -1;
    }

    return result;
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] fileDescriptor: %d>",
       [[self class] description], self, _fileDescriptor];
}

// private method for NSTask... this method is actually exposed in OSX 10.2.
- (int)fileDescriptor {
    return _fileDescriptor;
}

// POSIX programmer's guide p. 272
- (BOOL)isNonBlocking {
    int flags = fcntl(_fileDescriptor, F_GETFL);
    if (flags == -1) {
        NSRaiseException(NSFileHandleOperationException, self, _cmd,
                         @"isNonBlocking: %s", strerror(errno));
    }
    
    return (flags & O_NONBLOCK)?YES:NO;
}

- (void)setNonBlocking:(BOOL)flag {
    int flags = fcntl(_fileDescriptor, F_GETFL);
    if (flags == -1) {
        NSRaiseException(NSFileHandleOperationException, self, _cmd,
                         @"setNonBlocking(GETFL)(%d): %s", flag, strerror(errno));
    }

    if (flag)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(_fileDescriptor, F_SETFL, flags) == -1) {
        NSRaiseException(NSFileHandleOperationException, self, _cmd,
                         @"setNonBlocking(SETFL)(%d): %s", flag, strerror(errno));
    }
}

- (NSData *)readDataOfLength:(NSUInteger)length {
    NSMutableData *mutableData = [NSMutableData dataWithLength:length];
    ssize_t count, total = 0;

    [self setNonBlocking:NO];

    do {
        count = read(_fileDescriptor, [mutableData mutableBytes]+total, length-total);
        if (count == -1) {
            NSRaiseException(NSFileHandleOperationException, self, _cmd,
                             @"read(%d): %s", _fileDescriptor, strerror(errno));
            return nil;
        }

        if (count == 0) {	// end of file
            [mutableData setLength:total];
            break;
        }

        total += count;
    } while (total < length);

    return mutableData;
}

- (NSData *)readDataToEndOfFile {
    NSMutableData *mutableData = [NSMutableData dataWithLength:4096];
    ssize_t count, total = 0;

    [self setNonBlocking:NO];

    do {
        count = read(_fileDescriptor, [mutableData mutableBytes]+total, 4096);
        if (count == -1) {
            NSRaiseException(NSFileHandleOperationException, self, _cmd,
                             @"read(%d): %s", _fileDescriptor, strerror(errno));
            return nil;
        }

        if (count == 0) {	// end of file
            [mutableData setLength:total];
            break;
        }

        [mutableData increaseLengthBy:4096];

        total += count;
    } while (YES);

    return mutableData;
}

- (NSData *)availableData {
    NSMutableData *mutableData = [NSMutableData dataWithLength:0];
    int count;
    int length = 0;
    int err;

    do {
        [mutableData increaseLengthBy:4096];
        [self setNonBlocking:YES];
        count = read(_fileDescriptor, &((char*)[mutableData mutableBytes])[length], 4096);
        err = errno; // preserved so that the next fcntl doesn't clobber it
        
        [self setNonBlocking:NO];

        if (count <= 0) {
            while (err == EAGAIN || err == EINTR) {
                [self setNonBlocking:NO];
                count = read(_fileDescriptor, &((char*)[mutableData mutableBytes])[length], 1);
                err = errno; // preserved so that the next fcntl doesn't clobber it
                [self setNonBlocking:YES];
                if (count > 0) {
                    count = read(_fileDescriptor, &((char*)[mutableData mutableBytes])[length + 1], 4096-1);
                    if (count > 0) {
                        count += 1;
                    }
                    else {
                        count = 1;
                    }
                    break;
                }
                else if (count == 0) {
                    //no more data available
                    break;
                }
            }
        }

        if (count < 0) {
            NSRaiseException(NSFileHandleOperationException, self, _cmd,
                             @"read(%d): %s", _fileDescriptor, strerror(err));
        }

        length += count;


    } while(count == 4096);

    [mutableData setLength:length];

    return mutableData;
}

- (void)writeData:(NSData *)data {
    const void *bytes = [data bytes];
    NSUInteger length = [data length],total=0;
    size_t count;

    do {
        count = write(_fileDescriptor, bytes+total, length-total);
        if (count == -1)
            NSRaiseException(NSFileHandleOperationException, self, _cmd,
                             @"write(%d): %s", _fileDescriptor, strerror(errno));

        total += count;
    } while (total < length);
}


- (void)truncateFileAtOffset:(uint64_t)offset
{
    if (ftruncate(_fileDescriptor, offset) != 0) {
        NSLog(@"NSFileHandle: could not truncate file: (%d) %s", errno, strerror(errno));
    }
}


-(void)cancelBackgroundMonitoring {
   NSInteger i, count = [_backgroundModes count];

   for (i = 0; i < count; ++i)
    [[NSRunLoop currentRunLoop] removeInputSource:_inputSource forMode:[_backgroundModes objectAtIndex:i]];

    // we never actually retain the monitor, the run loop does--so we don't need to release it.
   _inputSource = nil;
   [_backgroundModes release];
   _backgroundModes = nil;
}

-(void)readInBackgroundAndNotifyForModes:(NSArray *)modes {
   NSInteger i, count = [modes count];

   if (_inputSource != nil)
    [NSException raise:NSInternalInconsistencyException format:@"%@ already has background activity", [self description]];

    _inputSource=[NSSelectInputSource socketInputSourceWithSocket:[NSSocket_bsd socketWithDescriptor:_fileDescriptor]];
    [_inputSource setSelectEventMask:NSSelectReadEvent];
    [_inputSource setDelegate:self];
    _backgroundModes = [modes retain];

   for(i = 0; i < count; ++i)
    [[NSRunLoop currentRunLoop] addInputSource:_inputSource forMode:[modes objectAtIndex:i]];
}

-(void)_acceptConnectionInBackgroundAndNotifyForModes:(NSArray *)modes {
    NSAutoreleasePool* pool=[NSAutoreleasePool new];
    
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getsockname(_fileDescriptor, (struct sockaddr *)&addr, &len);
    
    listen(_fileDescriptor, 1);
    accept(_fileDescriptor, (struct sockaddr *)&addr, &len);
    
    NSNotification *note=[NSNotification notificationWithName:NSFileHandleConnectionAcceptedNotification object:self];
    [[NSNotificationCenter defaultCenter] postNotification:note];
    
    [pool drain];
}

-(void)acceptConnectionInBackgroundAndNotifyForModes:(NSArray *)modes {
    [NSThread detachNewThreadSelector:@selector(_acceptConnectionInBackgroundAndNotifyForModes:) toTarget:self withObject:modes];
}

-(void)selectInputSource:(NSSelectInputSource *)inputSource selectEvent:(NSUInteger)selectEvent {
    NSData *availableData = [self availableData];
    NSDictionary   *userInfo;
    NSNotification *note;

    [self cancelBackgroundMonitoring];

    userInfo=[NSDictionary dictionaryWithObject:availableData
                                         forKey:NSFileHandleNotificationDataItem];
    note=[NSNotification notificationWithName:NSFileHandleReadCompletionNotification
                                       object:self
                                     userInfo:userInfo];

    [[NSNotificationCenter defaultCenter] postNotification:note];
}

@end
#endif

