/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS
#import "NSSocket_windows.h"
#import <Foundation/NSError.h>
#import <Foundation/NSHost.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSData.h>
#import <Foundation/CFSSLHandler.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSRaiseException.h>

#undef WINVER
#define WINVER 0x501
#include <ws2tcpip.h>

// The treatment of SOCKET's as int's is lame, there should probably be a little more formality on the [fF]ileDescriptor methods (typedef int/SOCKET NSFileDescriptor?)
// What would be nice is enough API in NSFileHandle/NSStream to never need the fd
 
@implementation NSSocket(windows)

+allocWithZone:(NSZone *)zone {
   return NSAllocateObject([NSSocket_windows class],0,NULL);
}

@end

@implementation NSSocket_windows

static inline void byteZero(void *vsrc,size_t size){
   uint8_t *src=vsrc;
   size_t i;

   for(i=0;i<size;i++)
    src[i]=0;
}

+(void)initialize {
   DWORD   vR=MAKEWORD(2,2);
   WSADATA wsaData;
   
   WSAStartup(vR, &wsaData);

   NSString *path=[[NSBundle bundleForClass:[self class]] pathForResource:@"CFSSLHandler_openssl" ofType:@"bundle"];
        
   if(path!=nil){
    NSBundle *bundle=[NSBundle bundleWithPath:path];

    [bundle load];
   }
}

-initWithSocketHandle:(SOCKET)handle {
#ifdef DEBUG
    NSCLog("NSSocket_windows -initWithSocketHandle:");
#endif
   _handle=handle;
   return self;
}

+socketWithSocketHandle:(SOCKET)handle {
#ifdef DEBUG
    NSCLog("NSSocket_windows +socketWithSocketHandle:");
#endif
   return [[[self alloc] initWithSocketHandle:handle] autorelease];
}

-(NSError *)errorForReturnValue:(int)returnValue {
   if(returnValue<0){
    return [NSError errorWithDomain:NSWINSOCKErrorDomain code:WSAGetLastError() userInfo:nil];
   }
   return nil;
}

-initTCPStream {
#ifdef DEBUG
    NSCLog("NSSocket_windows -initTCPStream");
#endif
   NSError *error=[self errorForReturnValue:_handle=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)];
   if(error!=nil){
    [self dealloc];
    return nil;
   }
   return self;
}

-initUDPStream {
#ifdef DEBUG
    NSCLog("NSSocket_windows -initUDPStream");
#endif
   NSError *error=[self errorForReturnValue:_handle=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP)];
   if(error!=nil){
    [self dealloc];
    return nil;
   }
   return self;
}

-initWithFileDescriptor:(int)descriptor {
#ifdef DEBUG
    NSCLog("NSSocket_windows -initWithFileDescriptor:");
#endif
   SOCKET handle=(SOCKET)descriptor;
   u_long arg;
   
   if(ioctlsocket(handle,FIONREAD,&arg)!=0){
    [self dealloc];
    return nil;
   }
   
   return [self initWithSocketHandle:handle];
}

-(void)dealloc {
#ifdef DEBUG
    NSCLog("dealloc");
#endif
   [_sslHandler release];
   [super dealloc];
}

-(void)closeAndDealloc {
#ifdef DEBUG
    NSCLog("NSSocket_windows -closeAndDealloc");
#endif
   [self close];
   [self dealloc];
}

-initConnectedToSocket:(NSSocket **)otherX {
#ifdef DEBUG
    NSCLog("NSSocket_windows -initConnectedToSocket:");
#endif
   NSSocket_windows  *other;
   NSError           *error;
   struct sockaddr_in address;
   int                namelen;

   *otherX = nil;
    
   if([self initUDPStream]==nil)
    return nil;
    
   if((other=[[[NSSocket alloc] initUDPStream] autorelease])==nil){
    [self closeAndDealloc];
    return nil;
   }
     
   byteZero(&address,sizeof(struct sockaddr_in));
   address.sin_family=AF_INET;
   address.sin_addr.s_addr=inet_addr("127.0.0.1");
   address.sin_port=0;
   if((error=[self errorForReturnValue:bind(other->_handle,(struct sockaddr *)&address,sizeof(struct sockaddr_in))])!=nil){
    [self closeAndDealloc];
    [other closeAndDealloc];
    return nil;
   }
   
   namelen=sizeof(address);
   if((error=[self errorForReturnValue:getsockname(other->_handle,(struct sockaddr *)&address,&namelen)])!=nil){
    [self closeAndDealloc];
    [other closeAndDealloc];
    return nil;
   }

   if((error=[self errorForReturnValue:connect(_handle,(struct sockaddr *)&address,sizeof(struct sockaddr_in))])!=nil){
    [self closeAndDealloc];
    [other closeAndDealloc];
    return nil;
   }

   *otherX=other;
   return self;
}

-(int)fileDescriptor {
   return (int)_handle;
}

-(SOCKET)socketHandle {
   return _handle;
}

-(void)setSocketHandle:(SOCKET)handle {
#ifdef DEBUG
    NSCLog("NSSocket_windows -setSocketHandle:");
#endif
   _handle=handle;
}

-(NSUInteger)hash {
   return (NSUInteger)_handle;
}

-(BOOL)isEqual:other {
   if(![other isKindOfClass:[NSSocket_windows class]])
    return NO;
    
   return (_handle==((NSSocket_windows *)other)->_handle)?YES:NO;
}

-(NSError *)close {
#ifdef DEBUG
    NSCLog("NSSocket_windows -close");
#endif
   return [self errorForReturnValue:closesocket(_handle)];
}

-(NSError *)setOperationWouldBlock:(BOOL)blocks {
   u_long onoff=blocks?NO:YES;

   return [self errorForReturnValue:ioctlsocket(_handle,FIONBIO,&onoff)];
}

-(BOOL)operationWouldBlock {
   return (WSAGetLastError()==WSAEWOULDBLOCK);
}

-(NSError *)connectToHost:(NSHost *)host port:(NSInteger)portNumber immediate:(BOOL *)immediate {
#ifdef DEBUG
    NSCLog("NSSocket_windows -connectToHost: %s port: %d immediate:", [[host name] cStringUsingEncoding: NSASCIIStringEncoding], (int)portNumber);
#endif
   BOOL     block=NO;
   NSArray *addresses=[host addresses];
   int      i,count=[addresses count];
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
    unsigned long address;
    
    [stringAddress getCString:cString];
    if((address=inet_addr(cString))==-1){
 // FIX
    }
    
    byteZero(&try,sizeof(struct sockaddr_in));
    try.sin_addr.s_addr=address;
    try.sin_family=AF_INET;
	short port=portNumber;
    try.sin_port=htons(port);

    if(connect(_handle,(struct sockaddr *)&try,sizeof(try))==0){
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
    error=[NSError errorWithDomain:NSWINSOCKErrorDomain code:WSAHOST_NOT_FOUND userInfo:nil];
    
   return error;
}

-(BOOL)hasBytesAvailable {
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;

    fd_set s;
    FD_ZERO(&s);
    FD_SET(_handle, &s);
    BOOL bytesAvailable = (select(0, &s, NULL, NULL, &t) == 1) ? YES : NO;
#ifdef DEBUG
    NSCLog("NSSocket_windows - hasBytesAvailable: %s", bytesAvailable ? "YES" : "NO");
#endif
    return bytesAvailable;
}

-(NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)length {
#ifdef DEBUG
    NSCLog("read: <buffer> maxLength: %d",length);
#endif
   NSInteger result;
   
   result=recv(_handle,(void *)buffer,length,0);
#ifdef DEBUG
   NSCLog("recv() result: %d",result);
#endif
   return result;
}

-(NSInteger)write:(const uint8_t *)buffer maxLength:(NSUInteger)length {
#ifdef DEBUG
    NSCLog("NSSocket_windows - write: maxLength: %d", length);
#endif
   NSInteger result;
   
   result=send(_handle,(void *)buffer,length,0);
#ifdef DEBUG
    NSCLog("send() result: %d",result);
#endif
    
   return result;
}

-(NSSocket *)acceptWithError:(NSError **)errorp {
#ifdef DEBUG
    NSCLog("NSSocket_windows - acceptWithError:");
#endif
   struct sockaddr addr;
   int             addrlen=sizeof(struct sockaddr);
   SOCKET          newSocket; 
   NSError        *error;
   
   error=[self errorForReturnValue:newSocket=accept(_handle,&addr,&addrlen)];
    if(errorp!=nil) {
    *errorp=error;
#ifdef DEBUG
        NSCLog("accept() error: %zd",  [error code]);
#endif
    }
   return (error!=nil)?nil:[[[NSSocket_windows alloc] initWithSocketHandle:newSocket] autorelease];
}

-(CFSSLHandler *)sslHandler {
   return _sslHandler;
}

-(BOOL)setSSLProperties:(CFDictionaryRef )sslProperties {

   if(_sslHandler==nil){
    _sslHandler=[[NSClassFromString(@"CFSSLHandler_openssl") alloc] initWithProperties:sslProperties];
   }
   else {
    // FIXME: what do we do if different properties are set
   }
   return YES;
}

@end

NSData *NSSocketAddressDataForNetworkOrderAddressBytesAndPort(const void *address,NSUInteger length,uint16_t port,uint32_t interface) {
   if(length==4){ // IPV4
          struct sockaddr_in ip4;
                    
          size_t ip4Length = sizeof (struct sockaddr_in);
          
          memset(&ip4, 0, ip4Length);
          
          ip4.sin_addr.s_addr=*(uint32_t *)address;
          ip4.sin_family = AF_INET;
          ip4.sin_port = port;
          
          return [NSData dataWithBytes:&ip4 length:ip4Length];
    }
   
   return nil;

   if(length==16){ // IPV6
          struct sockaddr_in6 ip6;
                    
          size_t ip6Length = sizeof (struct sockaddr_in6);
          
          memset(&ip6, 0, ip6Length);
          memcpy(&ip6.sin6_addr,address,16);

#ifdef SIN6_LEN
          ip6.sin6_len = sizeof ip6;
#endif
          ip6.sin6_family = AF_INET6;
          ip6.sin6_port = port;
          ip6.sin6_flowinfo = 0;
          ip6.sin6_scope_id = interface;
          
          return [NSData dataWithBytes:&ip6 length:ip6Length];
   }

   return nil;
}
#endif

