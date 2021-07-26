/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef PLATFORM_USES_BSD_SOCKETS
#import "NSSocket_bsd.h"
#import <Foundation/NSError.h>
#import <Foundation/NSHost.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

#include <errno.h>
#import <sys/types.h>
#import <sys/socket.h>
#import <netinet/in.h>
#import <sys/ioctl.h>
#include <unistd.h>
#import <arpa/inet.h>

#ifdef __svr4__ // Solaris
#import <sys/filio.h>
#import <signal.h>
#endif

@implementation NSSocket(bsd)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([NSSocket_bsd class],0,NULL);
}

@end

@implementation NSSocket_bsd

static inline void byteZero(void *vsrc,size_t size){
   uint8_t *src=vsrc;
   size_t i;

   for(i=0;i<size;i++)
    src[i]=0;
}

+(void)initialize {
#ifdef __svr4__ // Solaris
    sigignore(SIGPIPE);
#endif
}

-initWithDescriptor:(int)descriptor {
   _descriptor=descriptor;
   return self;
}

-initWithFileDescriptor:(int)fd {
   return [self initWithDescriptor:fd];
}

+socketWithDescriptor:(int)descriptor {
   return [[[self alloc] initWithDescriptor:descriptor] autorelease];
}

-(NSError *)errorForReturnValue:(int)returnValue {
   if(returnValue<0){
    return [NSError errorWithDomain:NSPOSIXErrorDomain code:errno userInfo:nil];
   }
   return nil;
}

-initTCPStream {
   NSError *error=[self errorForReturnValue:_descriptor=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)];
   if(error!=nil){
    [self dealloc];
    return nil;
   }
   return self;
}

-initUDPStream {
   NSError *error=[self errorForReturnValue:_descriptor=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP)];
   if(error!=nil){
    [self dealloc];
    return nil;
   }
   return self;
}

-(void)closeAndDealloc {
   [self close];
   [self dealloc];
}


- initConnectedToSocket: (NSSocket **)otherX
{
    int pipes[2];
    if (pipe(pipes) == 0) {
        *otherX = [[[isa alloc] initWithDescriptor:pipes[0]] autorelease];
        return [self initWithDescriptor:pipes[1]];
    } else {
        NSLog(@"NSSocket: could not create pipe: (%d) %s", errno, strerror(errno));
        [self release];
        return nil;
    }
}


-(int)descriptor {
   return _descriptor;
}

-(int)fileDescriptor {
   return _descriptor;
}

-(void)setDescriptor:(int)descriptor {
   _descriptor=descriptor;
}

-(NSUInteger)hash {
   return (NSUInteger)_descriptor;
}

-(BOOL)isEqual:other {
   if(![other isKindOfClass:[NSSocket_bsd class]])
    return NO;

   return (_descriptor==((NSSocket_bsd *)other)->_descriptor)?YES:NO;
}

-(NSError *)close {
   return [self errorForReturnValue:close(_descriptor)];
}

-(NSError *)setOperationWouldBlock:(BOOL)blocks {
   u_long onoff=blocks?NO:YES;

   return [self errorForReturnValue:ioctl(_descriptor,FIONBIO,&onoff)];
}

-(BOOL)operationWouldBlock {
   return (errno==EINPROGRESS);
}

-(NSError *)connectToHost:(NSHost *)host port:(NSInteger)portNumber immediate:(BOOL *)immediate {
   BOOL     block=NO;
   NSArray *addresses=[host addresses];
   NSInteger      i,count=[addresses count];
   NSError *error=nil;

   *immediate=NO;

   if(!block){
    if((error=[self setOperationWouldBlock:NO])!=nil)
     return error;
   }

   for(i=0;i<count;i++){
    struct sockaddr_in try;
    NSString     *stringAddress=[addresses objectAtIndex:i];
    char          cString[[stringAddress cStringLength]+1];
    in_addr_t     address;

    [stringAddress getCString:cString];
    if((address=inet_addr(cString))==-1){
 // FIX
    }

    byteZero(&try,sizeof(struct sockaddr_in));
    try.sin_addr.s_addr=address;
    try.sin_family=AF_INET;
    try.sin_port=htons(portNumber);

    if(connect(_descriptor,(struct sockaddr *)&try,(socklen_t)sizeof(try))==0){
     if(!block){
      if((error=[self setOperationWouldBlock:YES])!=nil)
       return error;
     }
     *immediate=YES;
     return nil;
    }
    else if([self operationWouldBlock]){
     if(!block){
      if((error=[self setOperationWouldBlock:YES])!=nil)
       return error;
     }
     return nil;
    }
    else {
     error=[self errorForReturnValue:-1];
    }
   }

   if(error==nil)
    error=[NSError errorWithDomain:NSPOSIXErrorDomain code:EHOSTUNREACH userInfo:nil];

   return error;
}

-(BOOL)hasBytesAvailable {
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;

    fd_set s;
    FD_ZERO(&s);
    FD_SET(_descriptor, &s);

    return (select(0, &s, NULL, NULL, &t) == 1) ? YES : NO;
}

-(NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)length {
   NSInteger i = recv(_descriptor,(void *)buffer,length,0);

    return i;
}

-(NSInteger)write:(const uint8_t *)buffer maxLength:(NSUInteger)length {
   return send(_descriptor,(void *)buffer,length,0);
}

-(NSSocket *)acceptWithError:(NSError **)errorp {
   struct sockaddr addr;
   socklen_t       addrlen=(socklen_t)sizeof(struct sockaddr);
   int             newSocket;
   NSError        *error;

   error=[self errorForReturnValue:newSocket=accept(_descriptor,&addr,&addrlen)];
   if(errorp!=NULL)
    *errorp=error;

   return (error!=nil)?nil:[[[NSSocket_bsd alloc] initWithDescriptor:newSocket] autorelease];
}

- (CFSSLHandler*)sslHandler {
    return nil;
}

@end


NSData *NSSocketAddressDataForNetworkOrderAddressBytesAndPort(const void *address,NSUInteger length,uint16_t port,uint32_t interface) {
   return nil;
}
#endif

