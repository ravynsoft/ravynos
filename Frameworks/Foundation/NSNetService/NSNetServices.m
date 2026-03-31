/* Copyright (c) 2006 Chris B. Vetter
   Copyright (c) 2008 Dirk Theisen
   Copyright (c) 2008,2010 Christopher J. W. Lloyd

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */
#import <Foundation/NSNetServices.h>

#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSNull.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSStream.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSSocket.h>
#import <Foundation/NSHost.h>
#import <Foundation/NSDebug.h>

#ifdef WIN32
#include <winsock2.h> // for ntohs
#else
#include <arpa/inet.h>
#endif
#include <string.h>

#import "bonjour.h"

NSString * const NSNetServicesErrorCode = @"NSNetServicesErrorCode";
NSString * const NSNetServicesErrorDomain = @"NSNetServicesErrorDomain";

@implementation NSNetService

-(void)_willPublish {
  if([_delegate respondsToSelector:@selector(netServiceWillPublish:)])
    [_delegate netServiceWillPublish:self];
}

-(void)_didPublish {
  if([_delegate respondsToSelector:@selector(netServiceDidPublish:)])
    [_delegate netServiceDidPublish:self];
}

-(void)_didNotPublish:(NSDictionary *) errorDict {
  if([_delegate respondsToSelector:@selector(netService:didNotPublish:)])
    [_delegate netService:self didNotPublish:errorDict];
}

-(void)_willResolve {
  if([_delegate respondsToSelector:@selector(netServiceWillResolve:)])
   [_delegate netServiceWillResolve:self];
}

-(void)_didResolveAddress {
  if([_delegate respondsToSelector:@selector(netServiceDidResolveAddress:)])
   [_delegate netServiceDidResolveAddress:self];
}

-(void)_didNotResolve:(NSDictionary *) errorDict {
  if([_delegate respondsToSelector:@selector(netService:didNotResolve:)])
   [_delegate netService:self didNotResolve:errorDict];
}

-(void)_netServiceDidStop {
  if([_delegate respondsToSelector:@selector(netServiceDidStop:)])
   [_delegate netServiceDidStop:self];
}

-(void)_didUpdateTXTRecordData:(NSData *)data {
  if([_delegate respondsToSelector:@selector(netService:didUpdateTXTRecordData:)])
   [_delegate   netService:self didUpdateTXTRecordData:data];
}

-(void)beginAsynchronousNetService {
   NSSocket *socket=[[NSSocket alloc] initWithFileDescriptor:bonjour_DNSServiceRefSockFD(_netService)];

   _inputSource=[[NSSelectInputSource alloc] initWithSocket:socket];
   [socket release];

   [_inputSource setDelegate:self];
   [_inputSource setSelectEventMask:NSSelectReadEvent];

   [[NSRunLoop currentRunLoop] addInputSource:_inputSource forMode:NSDefaultRunLoopMode];
}

-(void)_invalidateNetService {
   [_inputSource invalidate];
   [_inputSource release];
   _inputSource=nil;

   if( _netService!=NULL ){
    bonjour_DNSServiceRefDeallocate(_netService);
    _netService = NULL;
   }
}

-(void)stopResolving:(NSTimer *) timer {
   [self _invalidateNetService];

   [_resolverTimeout invalidate];
   [_resolverTimeout release];
   _resolverTimeout=nil;

   [self _didNotResolve:bonjour_CreateError(self,NSNetServicesTimeoutError)];
}

-(void)addAddress:(const void *)rdata length:(uint16_t)rdlen type:(uint16_t) rrtype interfaceIndex:(uint32_t)interface {
   NSData *data=nil;

   switch(rrtype){

    case bonjour_kDNSServiceType_A:		// ipv4
     data=NSSocketAddressDataForNetworkOrderAddressBytesAndPort(rdata,4,htons(_port),interface);
     break;

    case bonjour_kDNSServiceType_AAAA:	// ipv6
    case bonjour_kDNSServiceType_A6:  // deprecates AAAA
     data=NSSocketAddressDataForNetworkOrderAddressBytesAndPort(rdata,16,htons(_port),interface);
     break;
   }

   if(data!=nil)
    [_addresses addObject:data];
}

- (void) queryCallback:(bonjour_DNSServiceRef) sdRef
                 flags:(bonjour_DNSServiceFlags) flags
             interface:(uint32_t) interfaceIndex
                 error:(bonjour_DNSServiceErrorType) errorCode
              fullname:(const char *) fullname
                  type:(uint16_t) rrtype
                 class:(uint16_t) rrclass
                length:(uint16_t) rdlen
                  data:(const void *) rdata
                   ttl:(uint32_t) ttl
{
   if(errorCode!=bonjour_kDNSServiceErr_NoError){
      [self _invalidateNetService];

      [self _didNotResolve:bonjour_CreateError(self, errorCode)];

      return;
    }

    switch( rrtype ){

      case bonjour_kDNSServiceType_A:		// 1 -- AF_INET
        [self addAddress:rdata length:rdlen type:rrtype interfaceIndex:interfaceIndex];
        break;

      case bonjour_kDNSServiceType_NS:
      case bonjour_kDNSServiceType_MD:
      case bonjour_kDNSServiceType_MF:
      case bonjour_kDNSServiceType_CNAME:	// 5
      case bonjour_kDNSServiceType_SOA:
      case bonjour_kDNSServiceType_MB:
      case bonjour_kDNSServiceType_MG:
      case bonjour_kDNSServiceType_MR:
      case bonjour_kDNSServiceType_NULL:	// 10
      case bonjour_kDNSServiceType_WKS:
      case bonjour_kDNSServiceType_PTR:
      case bonjour_kDNSServiceType_HINFO:
      case bonjour_kDNSServiceType_MINFO:
      case bonjour_kDNSServiceType_MX:		// 15
        // not handled (yet)
        break;

      case bonjour_kDNSServiceType_TXT:
        [_txtRecord release];
        _txtRecord=[[NSData alloc] initWithBytes:rdata length:rdlen];

        [self _didUpdateTXTRecordData:_txtRecord];
        break;

      case bonjour_kDNSServiceType_RP:
      case bonjour_kDNSServiceType_AFSDB:
      case bonjour_kDNSServiceType_X25:
      case bonjour_kDNSServiceType_ISDN:	// 20
      case bonjour_kDNSServiceType_RT:
      case bonjour_kDNSServiceType_NSAP:
      case bonjour_kDNSServiceType_NSAP_PTR:
      case bonjour_kDNSServiceType_SIG:
      case bonjour_kDNSServiceType_KEY:		// 25
      case bonjour_kDNSServiceType_PX:
      case bonjour_kDNSServiceType_GPOS:
        // not handled (yet)
        break;

      case bonjour_kDNSServiceType_AAAA:	// 28 -- AF_INET6
        [self addAddress:rdata length:rdlen type:rrtype interfaceIndex:interfaceIndex];
        break;

      case bonjour_kDNSServiceType_LOC:
      case bonjour_kDNSServiceType_NXT:		// 30
      case bonjour_kDNSServiceType_EID:
      case bonjour_kDNSServiceType_NIMLOC:
      case bonjour_kDNSServiceType_SRV:
      case bonjour_kDNSServiceType_ATMA:
      case bonjour_kDNSServiceType_NAPTR:	// 35
      case bonjour_kDNSServiceType_KX:
      case bonjour_kDNSServiceType_CERT:
        // not handled (yet)
        break;

      case bonjour_kDNSServiceType_A6:		// 38 -- AF_INET6, deprecates AAAA
        [self addAddress:rdata length:rdlen type:rrtype interfaceIndex:interfaceIndex];
        break;

      case bonjour_kDNSServiceType_DNAME:
      case bonjour_kDNSServiceType_SINK:	// 40
      case bonjour_kDNSServiceType_OPT:
        // not handled (yet)
        break;

      case bonjour_kDNSServiceType_TKEY:	// 249
      case bonjour_kDNSServiceType_TSIG:	// 250
      case bonjour_kDNSServiceType_IXFR:
      case bonjour_kDNSServiceType_AXFR:
      case bonjour_kDNSServiceType_MAILB:
      case bonjour_kDNSServiceType_MAILA:
        // not handled (yet)
        break;

      case bonjour_kDNSServiceType_ANY:
        break;

      default:
        if(NSDebugEnabled)
         NSLog(@"-[%@ %s] Don't know how to handle rrtype <%d>",object_getClass(self),sel_getName(_cmd),rrtype);
        break;
    }

   if(!(flags&bonjour_kDNSServiceFlagsMoreComing)){

    [self _invalidateNetService];

    [_resolverTimeout invalidate];
    [_resolverTimeout release];
    _resolverTimeout = nil;

    if([_addresses count]>0)
     [self _didResolveAddress];
    else
     [self _didNotResolve:bonjour_CreateError(self, NSNetServicesNotFoundError)];
   }
}

static BONJOUR_CALL void QueryCallback(bonjour_DNSServiceRef sdRef,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,bonjour_DNSServiceErrorType errorCode,const char *fullname,uint16_t rrtype,uint16_t rrclass,uint16_t rdlen,const void *rdata,uint32_t ttl,void *context){
   [(NSNetService *) context queryCallback:sdRef flags:flags interface:interfaceIndex error:errorCode fullname:fullname type:rrtype class:rrclass length:rdlen data:rdata ttl:ttl];
}

- (void) resolverCallback:(bonjour_DNSServiceRef) sdRef
                    flags:(bonjour_DNSServiceFlags) flags
                interface:(uint32_t) interfaceIndex
                    error:(bonjour_DNSServiceErrorType) errorCode
                 fullname:(const char *) fullname
                   target:(const char *) hosttarget
                     port:(uint16_t) port
                   length:(uint16_t) txtLen
                   record:(const char *) txtRecord
{
   if(errorCode!=bonjour_kDNSServiceErr_NoError){
    [self _invalidateNetService];
    [self _didNotResolve:bonjour_CreateError(self, errorCode)];
    return;
   }

   _port = ntohs(port);

   [_txtRecord release];
   _txtRecord=nil;
   [_host release];
   _host=nil;

   if( txtRecord!=NULL )
    _txtRecord = [[NSData alloc] initWithBytes:txtRecord length:txtLen];

   if( hosttarget!=NULL )
    _host=[[NSString alloc] initWithUTF8String:hosttarget];

      // Add the interface so all subsequent queries are on the same interface
   _interfaceIndex = interfaceIndex;

   [_inputSource invalidate];
   [_inputSource release];
   _inputSource=nil;

   bonjour_DNSServiceRefDeallocate(_netService);
   _netService = NULL;

   // Prepare query for A and/or AAAA record
   errorCode = bonjour_DNSServiceQueryRecord(&_netService,0,_interfaceIndex,[_host UTF8String],bonjour_kDNSServiceType_ANY,bonjour_kDNSServiceClass_IN,QueryCallback,self);

   if( bonjour_kDNSServiceErr_NoError != errorCode){
    [self _invalidateNetService];

    [self _didNotResolve:bonjour_CreateError(self, errorCode)];
    return;
   }

   [self beginAsynchronousNetService];
}

static BONJOUR_CALL void ResolverCallback(bonjour_DNSServiceRef sdRef, bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,bonjour_DNSServiceErrorType errorCode,const char *fullname,const char *hosttarget,uint16_t port,uint16_t txtLen,const unsigned char *txtRecord,void *context){
   [(NSNetService *) context resolverCallback:sdRef flags:flags interface:interfaceIndex error:errorCode fullname:fullname target:hosttarget port:port length:txtLen record:(const char *)txtRecord];
}

- (void) registerCallback:(bonjour_DNSServiceRef) sdRef
                    flags:(bonjour_DNSServiceFlags) flags
                    error:(bonjour_DNSServiceErrorType) errorCode
                     name:(const char *) name
                     type:(const char *) regtype
                   domain:(const char *) domain
{
   if(errorCode!=bonjour_kDNSServiceErr_NoError){
    [self _invalidateNetService];

    [self _didNotPublish:bonjour_CreateError(self, errorCode)];
    return;
   }

   [self _didPublish];
}

static BONJOUR_CALL void RegistrationCallback(bonjour_DNSServiceRef sdRef,bonjour_DNSServiceFlags flags,bonjour_DNSServiceErrorType errorCode,const char *name,const char *regtype,const char *domain,void *context){
   [(NSNetService *) context registerCallback:sdRef flags:flags error:errorCode name:name type:regtype domain:domain];
}


-(void)selectInputSource:(NSSelectInputSource *)inputSource selectEvent:(NSUInteger)selectEvent {

   if(selectEvent&NSSelectReadEvent){
    bonjour_DNSServiceErrorType err= bonjour_DNSServiceProcessResult(_netService);

    if( err!=bonjour_kDNSServiceErr_NoError ){
		if(  _isPublishing )
			[self _didNotPublish:bonjour_CreateError(self, err)];
		else
			[self _didNotResolve:bonjour_CreateError(self, err)];
	}
   }
}

+ (NSData *) dataFromTXTRecordDictionary:(NSDictionary *) txtDictionary {
   NSUInteger i,count = [txtDictionary count];

   if( count==0){
    NSLog(@"Dictionary seems empty");
    return nil;
   }

   NSArray *keys = [txtDictionary allKeys];

   bonjour_TXTRecordRef txt;
   char keyCString[256];

   bonjour_TXTRecordCreate(&txt, 0, NULL);

   for(i=0 ; i < count; i++ ){
    id key=[keys objectAtIndex:i];
    id value=[txtDictionary objectForKey:key];
    NSInteger length = 0;
    size_t used = 0;
    bonjour_DNSServiceErrorType err = bonjour_kDNSServiceErr_Unknown;

    if( ! [key isKindOfClass:[NSString class]]){
     NSLog(@"%@ is not a string", key);
     break;
    }

    length = [key length];
    [key getCString:keyCString maxLength:sizeof keyCString];
    used = strlen(keyCString);

    if( ! length || (used >= sizeof keyCString) ){
     NSLog(@"incorrect length %d - %d - %d", length, used, sizeof keyCString);
     break;
    }

    strcat(keyCString, "\0");

    if([value isKindOfClass:[NSString class]]){
      char cString[256];

          length = [value length];
          [value getCString:cString maxLength:sizeof cString];
          used = strlen(cString);

          if( used >= sizeof cString ){
            NSLog(@"incorrect length %d - %d - %d", length, used, sizeof cString);
            break;
          }

          err = bonjour_TXTRecordSetValue(&txt,(const char *) keyCString,used,cString);
        }
        else if([value isKindOfClass:[NSData class]] && [value length] < 256){
          err = bonjour_TXTRecordSetValue(&txt,(const char *) keyCString,[value length],[value bytes]);

        }
        else if( value == [NSNull null]){
          err = bonjour_TXTRecordSetValue(&txt,(const char *) keyCString,0,NULL);
        }
        else {
          NSLog(@"unknown value type");
          break;
        }

        if( err != bonjour_kDNSServiceErr_NoError )
        {
          NSLog(@"error creating data type");
          break;
        }
      }

   NSData *result=(i<count)?nil:[NSData dataWithBytes:bonjour_TXTRecordGetBytesPtr(&txt) length:bonjour_TXTRecordGetLength(&txt)];

   bonjour_TXTRecordDeallocate(&txt);

   return result;
}

+ (NSDictionary *) dictionaryFromTXTRecordData:(NSData *) txtData {
   NSMutableDictionary *result = nil;
  NSInteger len = 0;
  const void
    *txt = 0;


  len = [txtData length];
  txt = [txtData bytes];

  //
  // A TXT record cannot exceed 65535 bytes, see Chapter 6.1 of
  // http://files.dns-sd.org/draft-cheshire-dnsext-dns-sd.txt
  //
  if( (len > 0) && (len < 65536) )
  {
    uint16_t
      i = 0,
      count = 0;

    // get number of keys
    count = bonjour_TXTRecordGetCount(len, txt);
    result = [NSMutableDictionary dictionaryWithCapacity:1];

    if( result )
    {
      // go through all keys
      for( ; i < count; i++ )
      {
        char
          keyCString[256];
        uint8_t
          valLen = 0;
        const void
          *value = NULL;
        bonjour_DNSServiceErrorType
          err = bonjour_kDNSServiceErr_NoError;

        err = bonjour_TXTRecordGetItemAtIndex(len, txt, i,
                                      sizeof keyCString, keyCString,
                                      &valLen, &value);

        // only if we can get the keyCString and value...
        if( bonjour_kDNSServiceErr_NoError == err )
        {
          NSString *str = [NSString stringWithUTF8String:keyCString];
          NSData
            *data = nil;

          if( value ){
           data = [NSData dataWithBytes:value length:valLen];
          }
          // only add if keyCString and value were created and keyCString doesn't exist yet
          if( data && str && [str length] && ! [result objectForKey:str])
          {
            [result setObject:data
					   forKey:str];
          }
          // I'm not exactly sure what to do if there is a keyCString WITHOUT a value
          // Theoretically '<6>foobar' should be identical to '<7>foobar='
          // i.e. the value would be [NSNull null]
          else
          {
           [result setObject:[NSNull null] forKey:str];
          }
        }
        else
        {
          NSLog(@"Couldn't get TXTRecord item");
        }
      }
    }
    else
    {
      NSLog(@"Couldn't create dictionary");
    }
  }
  else
  {
    NSLog(@"Incorrect length %d", len);
  }
  return result;
}

- (id) init
{

  return nil;
}


-initWithDomain:(NSString *) domain type:(NSString *) type name:(NSString *) name {
   return [self initWithDomain:domain type:type name:name port:-1]; // -1 to indicate resolution, not publish
}

-initWithDomain:(NSString *) domain type:(NSString *) type name:(NSString *) name port:(int) port {
   _domain=[domain copy];
   _type=[type copy];
   _name=[name copy];
   _port = port;
   _delegate=nil;

   _host=nil;
   _addresses=[[NSMutableArray alloc] init];
   _interfaceIndex = 0;

   _isPublishing = ( port==-1  ) ? NO :YES;

   _netService = NULL;
   _inputSource = nil;
   _resolverTimeout = nil;
   
   _inputStream = nil;
   _outputStream = nil;
   
   return self;
}

- (void) dealloc {
   [self stopMonitoring];
   [self _invalidateNetService];
   [_domain release];
   [_type release];
   [_name release];
   [_host release];
   [_addresses release];

    [_inputStream release];
   [_outputStream release];
   
   _delegate = nil;
   [super dealloc];
}


- (void) scheduleInRunLoop:(NSRunLoop *) runLoop forMode:(NSString *) mode {
   [runLoop addInputSource:_inputSource forMode:mode];
}

- (void) removeFromRunLoop:(NSRunLoop *) runLoop forMode:(NSString *) mode {
   [runLoop removeInputSource:_inputSource forMode:mode];
}

-(void)publishWithOptions:(NSNetServiceOptions)options {
   bonjour_DNSServiceErrorType err = bonjour_kDNSServiceErr_NoError;
   bonjour_DNSServiceFlags flags = 0;

// FIXME:these checks seem contrived, check real behavior
   if( NO == _isPublishing )
    err = NSNetServicesBadArgumentError;
   else if( ! _delegate )
    err = NSNetServicesInvalidError;
   else if( _inputSource!=nil )
    err = NSNetServicesActivityInProgress;
   else {
    if( _resolverTimeout ) {
     [_resolverTimeout invalidate];
     [_resolverTimeout release];
     _resolverTimeout = nil;
    }

    err = bonjour_DNSServiceRegister(&_netService,flags, _interfaceIndex,[_name UTF8String],[_type UTF8String],[_domain UTF8String],NULL, htons(_port), 0, NULL,RegistrationCallback,self);
   }

   if( err!=bonjour_kDNSServiceErr_NoError ) {
    [self _didNotPublish:bonjour_CreateError(self, err)];
    return;
   }

   [self beginAsynchronousNetService];

   [self _willPublish];
}

-(void)publish {
   [self publishWithOptions:0];
}

-(void)resolve {
   [self resolveWithTimeout:5];
}

- (void) resolveWithTimeout:(NSTimeInterval) timeout {
   bonjour_DNSServiceErrorType err = bonjour_kDNSServiceErr_NoError;
   bonjour_DNSServiceFlags flags = 0;

   [_addresses removeAllObjects];

// FIXME:these checks seem contrived, check real behavior
   if(_isPublishing ){
    err = NSNetServicesBadArgumentError;
   }
   else if( ! _delegate ){
    err = NSNetServicesInvalidError;
   }
   else if( _inputSource ){
    err = NSNetServicesActivityInProgress;
   }
   else  {
    if( _resolverTimeout ){
        [_resolverTimeout invalidate];
        [_resolverTimeout release];
        _resolverTimeout = nil;
      }

      err = bonjour_DNSServiceResolve( &_netService,flags, _interfaceIndex,[_name UTF8String],[_type UTF8String],[_domain UTF8String],ResolverCallback, self);

      if(err==bonjour_kDNSServiceErr_NoError){
        _resolverTimeout=[[NSTimer scheduledTimerWithTimeInterval:timeout
                                    target:self
                                  selector:@selector(stopResolving:)
                                  userInfo:nil
                                   repeats:NO] retain];
      }
   }

   if( err!=bonjour_kDNSServiceErr_NoError ) {
    [self _didNotResolve:bonjour_CreateError(self, err)];
    return;
   }

   [self beginAsynchronousNetService];
   [self _willResolve];
}

-(void)stop {
   [self _invalidateNetService];

   [self _netServiceDidStop];
}

-(void)startMonitoring {
    // Obviously this will only work on a resolver
   if(_isPublishing )
    return;
   if(_isMonitoring)
    return;

   bonjour_DNSServiceErrorType err = bonjour_kDNSServiceErr_NoError;

   if( ! _delegate )
    err = NSNetServicesInvalidError;
   else if( _inputSource!=nil )
    err = NSNetServicesActivityInProgress;
   else {
    NSString *fullname = [NSString stringWithFormat:@"%@.%@%@", [self name], [self type], [self domain]];

    err = bonjour_DNSServiceQueryRecord( &_netService,
                                  bonjour_kDNSServiceFlagsLongLivedQuery,
                                  0,
                                  [fullname UTF8String],
                                  bonjour_kDNSServiceType_TXT,
                                  bonjour_kDNSServiceClass_IN,
                                  QueryCallback,
                                  self);

    if( bonjour_kDNSServiceErr_NoError == err ){
     NSSocket *socket=[[NSSocket alloc] initWithFileDescriptor:bonjour_DNSServiceRefSockFD(_netService)];

     _inputSource=[[NSSelectInputSource alloc] initWithSocket:socket];
     [socket release];
     [_inputSource setDelegate:self];
     [_inputSource setSelectEventMask:NSSelectReadEvent];

     [[NSRunLoop currentRunLoop] addInputSource:_inputSource forMode:NSDefaultRunLoopMode];
     _isMonitoring = YES;
     }

    }
}

-(void)stopMonitoring {
   if(_isPublishing)
    return;
   if(!_isMonitoring)
    return;

   [self _invalidateNetService];

   _isMonitoring = NO;
}

-delegate {
  return _delegate;
}

-(void)setDelegate:(id) delegate {
    _delegate = delegate;
}

-(NSArray *)addresses {
  return _addresses;
}

-(NSString *)domain {
  return _domain;
}

-(NSString *)hostName {
  return _host;
}

-(NSString *)name {
   return _name;
}

-(NSString *)type {
    return _type;
}

-(int) port
{
	return _port;
}

- (NSString *) protocolSpecificInformation {
   NSMutableArray *array = nil;

  // I must admit, the following may not be entirely correct...

   NSDictionary *dictionary = [NSNetService dictionaryFromTXTRecordData:[self TXTRecordData]];

   if( dictionary ){
    NSEnumerator *keys = [dictionary keyEnumerator];
    id key = nil;

    array = [NSMutableArray arrayWithCapacity:[dictionary count]];

    while( (key = [keys nextObject])!=nil ) {
     id value = [dictionary objectForKey:key];

     if( value !=  [NSNull null]){
      [array addObject:[NSString stringWithFormat:@"%@=%@", key,
                              [NSString stringWithCString:[value bytes]
                                                   length:[value length]]]];
     }
     else if([key length]) {
      [array addObject:[NSString stringWithFormat:@"%@", key]];
     }
    }
   }

   return ([array count] ? [array componentsJoinedByString:@"\001"] : (NSString *)nil );
}

- (void) setProtocolSpecificInformation:(NSString *) specificInformation {
  // Again, the following may not be entirely correct...

   NSArray   *array  = [specificInformation componentsSeparatedByString:@"\001"];
   NSUInteger i,count=[array count];

   if(count>0){
    NSMutableDictionary *dictionary = [NSMutableDictionary dictionaryWithCapacity:count];

    for(i=0;i<count;i++){
     NSArray *parts = [[array objectAtIndex:i] componentsSeparatedByString:@"="];

     [dictionary setObject:[[parts objectAtIndex:1] dataUsingEncoding:NSUTF8StringEncoding] forKey:[parts objectAtIndex:0]];
    }

    [self setTXTRecordData:[NSNetService dataFromTXTRecordDictionary:dictionary]];
   }
}

- (NSData *) TXTRecordData {
  return _txtRecord;
}

- (BOOL) setTXTRecordData:(NSData *) recordData
{
  BOOL
    result = NO;

    // Not allowed on a resolver...
    if( _isPublishing )
    {
      bonjour_DNSServiceErrorType
        err = bonjour_kDNSServiceErr_NoError;

      // Set the value, or remove it if empty
      recordData=[recordData copy];
      [_txtRecord release];
      _txtRecord=recordData;
      // Assume it worked
      result = YES;

      // Now update the record so others can pick it up
      err = bonjour_DNSServiceUpdateRecord(_netService,
                                   NULL,
                                   0,
                                   recordData ? [recordData length] :0,
                                   recordData ? [recordData bytes] :NULL,
                                   0);
      if( err )
      {
        result = NO;
      }
    }

   return result;
}

- (BOOL) getInputStream:(NSInputStream **) inputStream outputStream:(NSOutputStream **) outputStream {
   NSHost *host=[NSHost hostWithName:_host];
   
   if (inputStream || outputStream) 
   {
     if(!_inputStream || !_outputStream)
     {
        [NSStream getStreamsToHost:host 
                              port:_port 
                       inputStream:(!_inputStream ? inputStream : NULL) 
                      outputStream:(!_outputStream ? outputStream : NULL)];
     }
     if(inputStream)
     { 
        if(!_inputStream) _inputStream = [*inputStream retain];
        else *inputStream = _inputStream;
     }
     if(outputStream)
     { 
        if(!_outputStream) _outputStream = [*outputStream retain];
        else *outputStream = _outputStream;
     }
	 return ((inputStream && _inputStream) || (outputStream && _outputStream));
   }
   return NO;
}

@end


@interface NSNetServiceBrowser(forward)
-(void)enumCallback:(bonjour_DNSServiceRef) sdRef flags:(bonjour_DNSServiceFlags) flags interface:(uint32_t) interfaceIndex error:(bonjour_DNSServiceErrorType) errorCode domain:(const char *) replyDomain;

-(void)browseCallback:(bonjour_DNSServiceRef) sdRef flags:(bonjour_DNSServiceFlags) flags interface:(uint32_t) interfaceIndex error:(bonjour_DNSServiceErrorType) errorCode name:(const char *) replyName type:(const char *) replyType domain:(const char *) replyDomain;
@end

static BONJOUR_CALL void EnumerationCallback(bonjour_DNSServiceRef sdRef,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,bonjour_DNSServiceErrorType  errorCode,const char *replyDomain,void *context) {
  [(id) context enumCallback:sdRef flags:flags interface:interfaceIndex error:errorCode domain:replyDomain];
}

static BONJOUR_CALL void BrowserCallback(bonjour_DNSServiceRef sdRef,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,bonjour_DNSServiceErrorType errorCode,const char *replyName,const char *replyType,const char *replyDomain,void *context){
  [(id) context browseCallback:sdRef flags:flags interface:interfaceIndex error:errorCode name:replyName type:replyType domain:replyDomain];
}

@implementation NSNetServiceBrowser

-init {
   [super init];

   _netServiceBrowser = NULL;
   _delegate = nil;
   _services = [[NSMutableDictionary alloc] init];
   _interfaceIndex = 0;

    return self;
}

- (void) invalidate {
   [_inputSource invalidate];
   [_inputSource release];
   _inputSource=nil;

   if( _netServiceBrowser ){
    bonjour_DNSServiceRefDeallocate(_netServiceBrowser);
    _netServiceBrowser = NULL;
   }

   [_services removeAllObjects];
}

- (void) dealloc {
   [self invalidate];

   [_services release];
   _services = nil;

   _delegate = nil;
   [super dealloc];
}

-  delegate {
  return _delegate;
}


- (void) setDelegate:delegate {
    _delegate = delegate;
}

-(void)_willSearch {
   if([_delegate respondsToSelector:@selector(netServiceBrowserWillSearch:)])
    [_delegate netServiceBrowserWillSearch:self];
}

-(void)_didNotSearch:(NSDictionary *) errorDict {
  if([_delegate respondsToSelector:@selector(netServiceBrowser:didNotSearch:)])
    [_delegate netServiceBrowser:self didNotSearch:errorDict];
}

- (void)_didStopSearch {
  if([_delegate respondsToSelector:@selector(netServiceBrowserDidStopSearch:)])
    [_delegate netServiceBrowserDidStopSearch:self];
}

- (void)_didFindDomain:(NSString *) domainString moreComing:(BOOL) moreComing {

  if([_delegate respondsToSelector:@selector(netServiceBrowser:didFindDomain:moreComing:)])
    [_delegate netServiceBrowser:self didFindDomain:domainString moreComing:moreComing];
}

- (void)_didRemoveDomain:(NSString *) domainString moreComing:(BOOL) moreComing {

  if([_delegate respondsToSelector:@selector(netServiceBrowser:didRemoveDomain:moreComing:) ])
    [_delegate netServiceBrowser:self didRemoveDomain:domainString moreComing:moreComing];

}

- (void)_didFindService:(NSNetService *) aService moreComing:(BOOL) moreComing {
  if([_delegate respondsToSelector:@selector(netServiceBrowser:didFindService:moreComing:)])
    [_delegate netServiceBrowser:self didFindService:aService moreComing:moreComing];

}

- (void)_didRemoveService:(NSNetService *) aService
                moreComing:(BOOL) moreComing {
  if([_delegate respondsToSelector:@selector(netServiceBrowser:didRemoveService:moreComing:)])
    [_delegate netServiceBrowser:self didRemoveService:aService moreComing:moreComing];
}

- (void) executeWithError:(bonjour_DNSServiceErrorType) err
{
    if( bonjour_kDNSServiceErr_NoError == err )
    {
     [self _willSearch];
      NSSocket *socket=[[NSSocket alloc] initWithFileDescriptor:bonjour_DNSServiceRefSockFD(_netServiceBrowser)];

      _inputSource=[[NSSelectInputSource alloc] initWithSocket:socket];
      [socket release];

      [_inputSource setDelegate:self];
      [_inputSource setSelectEventMask:NSSelectReadEvent];

      [[NSRunLoop currentRunLoop] addInputSource:_inputSource forMode:NSDefaultRunLoopMode];
    }
    else // notify the delegate of the error
    {
      [self _didNotSearch:bonjour_CreateError(self, err)];
    }
}

- (void) searchForDomain:(int) flags {
   bonjour_DNSServiceErrorType err;

   if( ! _delegate )
    err = NSNetServicesInvalidError;
   else if( _inputSource )
    err = NSNetServicesActivityInProgress;
   else {
    err = bonjour_DNSServiceEnumerateDomains(&_netServiceBrowser,flags,_interfaceIndex,EnumerationCallback,self);
   }

   [self executeWithError:err];
}

- (void) enumCallback:(bonjour_DNSServiceRef) sdRef
                flags:(bonjour_DNSServiceFlags) flags
            interface:(uint32_t) interfaceIndex
                error:(bonjour_DNSServiceErrorType) errorCode
               domain:(const char *) replyDomain {

   if( errorCode ){
    [self invalidate];

    [self _didNotSearch:bonjour_CreateError(self, errorCode)];
    return;
   }

   if(replyDomain==NULL)
    return;

   BOOL more = (flags & bonjour_kDNSServiceFlagsMoreComing)?YES:NO;

   _interfaceIndex = interfaceIndex;

   if( flags & bonjour_kDNSServiceFlagsAdd ){
    [self _didFindDomain:[NSString stringWithUTF8String:replyDomain] moreComing:more];
   }
   else { // bonjour_kDNSServiceFlagsRemove
    [self _didRemoveDomain:[NSString stringWithUTF8String:replyDomain] moreComing:more];
   }
}

- (void) browseCallback:(bonjour_DNSServiceRef) sdRef
                  flags:(bonjour_DNSServiceFlags) flags
              interface:(uint32_t) interfaceIndex
                  error:(bonjour_DNSServiceErrorType) errorCode
                   name:(const char *) replyName
                   type:(const char *) replyType
                 domain:(const char *) replyDomain {
   if(errorCode!=bonjour_kDNSServiceErr_NoError){
    [self invalidate];

    [self _didNotSearch:bonjour_CreateError(self, errorCode)];
    return;
   }

   BOOL          more = (flags & bonjour_kDNSServiceFlagsMoreComing)?YES:NO;
   NSString     *domain=[NSString stringWithUTF8String:replyDomain];
   NSString     *type=[NSString stringWithUTF8String:replyType];
   NSString     *name=[NSString stringWithUTF8String:replyName];
   NSString     *key=[NSString stringWithFormat:@"%@%@%@", name, type, domain];
   NSNetService *service = nil;

   _interfaceIndex = interfaceIndex;

   if( flags & bonjour_kDNSServiceFlagsAdd ){
    service = [[NSNetService alloc] initWithDomain:domain type:type name:name];

    if( service ){
     [_services setObject:service forKey:key];

     [service autorelease];

     [self _didFindService:service moreComing:more];
    }
    else {
     NSLog(@"WARNING:Could not create an NSNetService for <%s>", replyName);
    }
   }
   else { // bonjour_kDNSServiceFlagsRemove
    service = [_services objectForKey:key];

    if( service ){
     [self _didRemoveService:service moreComing:more];
    }
    else {
     NSLog(@"WARNING:Could not find <%@> in list", key);
    }
   }
}

-(void)selectInputSource:(NSSelectInputSource *)inputSource selectEvent:(NSUInteger)selectEvent {
   if(selectEvent&NSSelectReadEvent){

    bonjour_DNSServiceErrorType err = bonjour_DNSServiceProcessResult(_netServiceBrowser);

    if( err!=bonjour_kDNSServiceErr_NoError ){
     [self _didNotSearch:bonjour_CreateError(self, err)];
    }
   }
}

-(void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   [runLoop addInputSource:_inputSource forMode:mode];
}

-(void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   [runLoop removeInputSource:_inputSource forMode:mode];
}


-(void)searchForAllDomains {
// Search for all visible domains. This method is deprecated in 10.4

   [self searchForDomain:bonjour_kDNSServiceFlagsBrowseDomains|bonjour_kDNSServiceFlagsRegistrationDomains];
}

-(void)searchForBrowsableDomains {
   [self searchForDomain:bonjour_kDNSServiceFlagsBrowseDomains];
}

// Search for all registration domains. These domains can be used to register a service.

-(void)searchForRegistrationDomains {
   [self searchForDomain:bonjour_kDNSServiceFlagsRegistrationDomains];
}

- (void) searchForServicesOfType:(NSString *) serviceType inDomain:(NSString *) domainName {
   bonjour_DNSServiceErrorType err = bonjour_kDNSServiceErr_NoError;

   if( ! _delegate )
    err = NSNetServicesInvalidError;
   else if( _inputSource )
    err = NSNetServicesActivityInProgress;
   else {
    err = bonjour_DNSServiceBrowse( &_netServiceBrowser,0,_interfaceIndex,[serviceType UTF8String],[domainName UTF8String],BrowserCallback,self);
   }

   [self executeWithError:err];
}

-(void) stop {
   [self invalidate];

   [self _didStopSearch];
}

@end


