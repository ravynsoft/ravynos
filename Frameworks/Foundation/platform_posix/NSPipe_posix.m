/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_IS_POSIX
#import <Foundation/NSPipe_posix.h>
#import <Foundation/NSFileHandle_posix.h>
#import <Foundation/Foundation.h>
#import <Foundation/NSRaiseException.h>

#include <unistd.h>
#include <errno.h>

@implementation NSPipe_posix

- init {
    int fds[2];

    if (pipe(fds) == -1)
        NSRaiseException(NSInvalidArgumentException, self, _cmd,
                         @"pipe() failed: %s", strerror(errno));

    _fileHandleForReading = [[NSFileHandle_posix alloc] initWithFileDescriptor:fds[0]];
    _fileHandleForWriting = [[NSFileHandle_posix alloc] initWithFileDescriptor:fds[1]];
    
    return self;
}

-(void)dealloc {
    [_fileHandleForReading release];
    [_fileHandleForWriting release];
    
    [super dealloc];
}

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] readfd: %d writefd: %d>",
        [[self class] description], self,
        [(NSFileHandle_posix *)_fileHandleForReading fileDescriptor],
        [(NSFileHandle_posix *)_fileHandleForWriting fileDescriptor]];
}

- (NSFileHandle *)fileHandleForReading {
    return _fileHandleForReading;
}

- (NSFileHandle *)fileHandleForWriting {
    return _fileHandleForWriting;
}

@end
#endif

