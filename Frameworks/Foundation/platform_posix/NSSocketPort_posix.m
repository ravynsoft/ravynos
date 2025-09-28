/* Copyright (c) 2012 Andy Van Ness
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_IS_POSIX
#import "NSSocketPort_posix.h"

#include <errno.h>
#import <sys/types.h>
#import <sys/socket.h>
#import <netinet/in.h>
#import <sys/ioctl.h>
#include <unistd.h>
#import <arpa/inet.h>
#include <strings.h>

@implementation NSSocketPort_posix

-initWithTCPPort:(unsigned short)port {
    struct sockaddr_in address;
    address.sin_family = PF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    _socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    bzero(&address.sin_zero, sizeof(address.sin_zero));
    
    if (bind(_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        [self release];
        return nil;
    }
    
    return self;
}

-(NSSocketNativeHandle)socket {
    return _socket;
}

@end
#endif
