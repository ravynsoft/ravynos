/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSURLProtocol_http.h"
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSStream.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSData.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSHost.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSHTTPURLResponse.h>
#import <Foundation/NSError.h>
#import <Foundation/NSURLError.h>
#import <Foundation/NSCachedURLResponse.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSPathUtilities.h>
#import <CFNetwork/CFNetwork.h>

#include <string.h>

enum {
 STATE_waitingForStatusVersion,
 STATE_waitingForStatusCode,
 STATE_waitingForStatusReport,
 STATE_waitingForStatusCR,
 STATE_waitingForStatusLF,
 STATE_waitingForHeader,
 STATE_waitingForContinuationCR,
 STATE_waitingForContinuationLF,
 STATE_waitingForHeaderColon,
 STATE_waitingForSpaceAfterHeaderColon,
 STATE_waitingForHeaderCR,
 STATE_waitingForHeaderLF,
 STATE_waitingForLastLF,
 STATE_waitingForChunkSize,
 STATE_waitingForChunkSizeLF,
 STATE_waitingForChunkCompletion,
 STATE_waitingForChunkCompletionLF,
 STATE_entity_body_check_for_lf,
 STATE_entity_body,
 STATE_didFinishLoading,
};

@interface NSHTTPURLResponse(private)
-initWithURL:(NSURL *)url statusCode:(NSInteger)statusCode headers:(NSDictionary *)headers;
@end

@implementation NSURLProtocol_http

-(void)statusVersion:(NSString *)string {

}

-(void)headers:(NSDictionary *)headers {
#if 0
   if(NSDebugEnabled)
    NSLog(@"statusCode=%d headers: %@",_statusCode,headers);
#endif

   if(_statusCode>=200 && _statusCode<300){
    NSURL             *url=[_request URL];
    NSHTTPURLResponse *response=[[[NSHTTPURLResponse alloc] initWithURL:url statusCode:_statusCode headers:headers] autorelease];
    NSURLCacheStoragePolicy cachePolicy=NSURLCacheStorageNotAllowed;
   
    if([[_request HTTPMethod] isEqualToString:@"GET"]){
     if([[url scheme] isEqualToString:@"http"])
      cachePolicy=NSURLCacheStorageAllowed;
     else if([[url scheme] isEqualToString:@"https"])
      cachePolicy=NSURLCacheStorageAllowedInMemoryOnly;
}

    [_client URLProtocol:self didReceiveResponse:response cacheStoragePolicy:cachePolicy];
	}
   else if(_statusCode==304){
    [_client URLProtocol:self cachedResponseIsValid:_cachedResponse];

    [_client URLProtocol:self didReceiveResponse:[_cachedResponse response] cacheStoragePolicy:NSURLCacheStorageNotAllowed];

    [_client URLProtocol:self didLoadData:[_cachedResponse data]];
}
   else {
    NSDictionary *userInfo=[NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"HTTP status code = %d", _statusCode] forKey:NSLocalizedDescriptionKey];
     
    NSError *error=[NSError errorWithDomain:NSURLErrorDomain code:NSURLErrorBadServerResponse userInfo:userInfo];
    [self stopLoading];
    [_client URLProtocol:self didFailWithError:error];
	}
}

- (NSString *)normalizedHeaderWithName:(NSString *)theName
{
    // Normalize headers like Cocoa does: Make the first
    // character and any character after '-' uppercase
    // and the rest, lowercase.
    if ([theName length]) {
        char *name = strdup([theName UTF8String]);
        int length = strlen(name);
        int ii;
        name[0] &= ~(1 << 5);
        char c = name[0];
        for(ii = 1; ii < length; c = name[ii++]) {
            if (c == '-') {
                name[ii] &= ~(1 << 5);
            } else {
                name[ii] |= 1 << 5;
            }
        }
        return [[[NSString alloc] initWithBytesNoCopy:name length:length
                                            encoding:NSUTF8StringEncoding
                                        freeWhenDone:YES] autorelease];
    }
    return theName;
}

-(void)_headerKey {
   [_currentKey autorelease];
   _currentKey=[[NSString alloc] initWithCString:(char*)_bytes+_range.location length:_range.length];
}

-(void)_headerValue {
   NSString *value=[NSString stringWithCString:(char*)_bytes+_range.location length:_range.length-1];
   NSString *oldValue;
   NSString *normalized = [self normalizedHeaderWithName:_currentKey];
   if((oldValue=[_headers objectForKey:normalized])!=nil)
    value=[[oldValue stringByAppendingString:@" "] stringByAppendingString:value];

   [_rawHeaders setObject:value forKey:_currentKey];
   [_headers setObject:value forKey:normalized];
}

-(void)_continuation {
   NSString *value=[NSString stringWithCString:(char*)_bytes+_range.location length:_range.length-1];
   NSString *normalized = [self normalizedHeaderWithName:_currentKey];
   NSString *oldValue=[_headers objectForKey:normalized];

   value=[[oldValue stringByAppendingString:@" "] stringByAppendingString:value];

   [_rawHeaders setObject:value forKey:_currentKey];
   [_headers setObject:value forKey:normalized];
}

-(BOOL)contentIsChunked {
   return [[_headers objectForKey:@"Transfer-Encoding"] isEqual:@"chunked"];
}
	
-(NSInteger)contentLength {
   return [[_headers objectForKey:@"Content-Length"] integerValue];
}

-(void)didFinishLoading {
   if(_state==STATE_didFinishLoading)
    return;
    
   _state=STATE_didFinishLoading;
   [self stopLoading];
   [_client URLProtocolDidFinishLoading:self];
}

-(void)didLoadData:(NSData *)data {
// NSLog(@"didLoadData %@",[[[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding] autorelease]);

   _totalContentReceived+=[data length];

   [_client URLProtocol:self didLoadData:data];
   
   if(_expectedContentLength > 0 && _totalContentReceived>=_expectedContentLength)
    [self didFinishLoading];
}

-(void)advanceStateWithData:(NSData *)data {

   if(_state==STATE_didFinishLoading)
    return;

   if([data length]==0){
    [self didFinishLoading];
    return;
}

   if(_state==STATE_entity_body){
    [self didLoadData:data];
    return;
   }
   
   [_data appendData:data];
   _bytes=[_data bytes];
   _length=[_data length];
   
   while(NSMaxRange(_range)<_length){
    uint8_t code=_bytes[NSMaxRange(_range)];
    enum  {
     extendLength,
     advanceLocationToNext,
     advanceLocationToCurrent,
    } rangeAction=extendLength;

    switch(_state){

     case STATE_waitingForStatusVersion:
      if(code==' '){
       [self statusVersion:[NSString stringWithCString:(char*)_bytes+_range.location length:_range.length-1]];
       rangeAction=advanceLocationToNext;
       _state=STATE_waitingForStatusCode;
       _statusCode=0;
      }
      else if(code=='\015'){
       [self statusVersion:[NSString stringWithCString:(char*)_bytes+_range.location length:_range.length-1]];
       _state=STATE_waitingForStatusLF;
      }
      break;
     
     case STATE_waitingForStatusCode:
      if(code>='0' && code<='9')
       _statusCode=_statusCode*10+code-'0';
      else if(code=='\015'){
       _state=STATE_waitingForStatusLF;
      }
      else {
       _state=STATE_waitingForStatusReport;
       rangeAction=advanceLocationToNext;
      }
      break;
      
     case STATE_waitingForStatusReport:
      if(code=='\015'){
       _state=STATE_waitingForStatusLF;
      }
      break;
      
     case STATE_waitingForStatusCR:
       if(code=='\015')
       _state=STATE_waitingForStatusLF;
       break;

     case STATE_waitingForStatusLF:
      if(code!='\012')
       _state=STATE_waitingForStatusCR;
      else {
       _state=STATE_waitingForHeader;
       rangeAction=advanceLocationToNext;
      }
      break;

     case STATE_waitingForHeader:
      if(code==' ' || code=='\t')
       _state=STATE_waitingForContinuationCR;
      else if(code=='\015')
       _state=STATE_waitingForLastLF;
      else
       _state=STATE_waitingForHeaderColon;
      break;

     case STATE_waitingForContinuationCR:
      if(code=='\015')
       _state=STATE_waitingForContinuationLF;
      break;

     case STATE_waitingForContinuationLF:
      if(code!='\012')
       _state=STATE_waitingForContinuationCR;
      else {
       [self _continuation];
       _state=STATE_waitingForHeader;
       rangeAction=advanceLocationToNext;
      }
      break;

     case STATE_waitingForHeaderColon:
      if(code==':'){
       [self _headerKey];
       _state=STATE_waitingForSpaceAfterHeaderColon;
       rangeAction=advanceLocationToNext;
      }
      break;

     case STATE_waitingForSpaceAfterHeaderColon:
      if(code==' '){
       rangeAction=advanceLocationToNext;
       break;
      }
      _state=STATE_waitingForHeaderCR;
      // fallthru

     case STATE_waitingForHeaderCR:
      if(code=='\015')
       _state=STATE_waitingForHeaderLF;
      break;

     case STATE_waitingForHeaderLF:
      if(code!='\012')
       _state=STATE_waitingForHeaderCR;
      else {
       [self _headerValue];
       _state=STATE_waitingForHeader;
       rangeAction=advanceLocationToNext;
      }
      break;

     case STATE_waitingForLastLF:
      [self headers:_headers];
      if([self contentIsChunked]){
       _state=STATE_waitingForChunkSize;
       _chunkSize=0;
       rangeAction=advanceLocationToNext;
       break;
      }
      else if([self contentLength]==0){
       [self didFinishLoading];
       return;
      }
      else {
       _expectedContentLength=[self contentLength];
       _totalContentReceived=0;
       _state=STATE_entity_body_check_for_lf;
       rangeAction=advanceLocationToCurrent;
		  }
      break;

     case STATE_waitingForChunkSize:
      if(code>='0' && code<='9')
       _chunkSize=_chunkSize*16+(code-'0');
      else if(code>='a' && code<='f')
       _chunkSize=_chunkSize*16+(code-'a')+10;
      else if(code>='A' && code<='F')
       _chunkSize=_chunkSize*16+(code-'A')+10;
      else if(code=='\015')
       _state=STATE_waitingForChunkSizeLF;
      else{
NSLog(@"parse error %d %o",__LINE__,code);
      }
      break;

     case STATE_waitingForChunkSizeLF:
      if(code=='\012'){
       if(_chunkSize==0){
        [self didFinishLoading];
        return;
NSLog(@"zero chunk");
       }
       else {
NSLog(@"chunk=%d",_chunkSize);
        _state=STATE_waitingForChunkCompletion;
        rangeAction=advanceLocationToNext;
       }
      }
      else {
NSLog(@"parse error %d",__LINE__);
      }
      break;

     case STATE_waitingForChunkCompletion:
      if(_range.length==_chunkSize){
       _state=STATE_waitingForChunkCompletionLF;
       _chunkSize=0;
       if(code=='\015')
        NSLog(@"got cr");
		  NSLog(@"chunk done");
       [self didLoadData:[NSData dataWithBytes:_bytes+_range.location length:_range.length]];
	   _range.location=NSMaxRange(_range);
	   _range.length=0;
      }
      break;

     case STATE_waitingForChunkCompletionLF:
       if(code=='\012')
        NSLog(@"got lf");
      _state=STATE_waitingForChunkSize;
      break;

     case STATE_entity_body_check_for_lf:
      if(code=='\012'){
       _state=STATE_entity_body;
       rangeAction=advanceLocationToNext;
     //  _expectedContentLength--;
       break;
      }
      // fallthrough

     case STATE_entity_body:;
      NSInteger pieceLength=_length-_range.location;      
      _range.length=pieceLength;
      
      [self didLoadData:[NSData dataWithBytes:_bytes+_range.location length:pieceLength]];
      return;
    }

    switch(rangeAction){
     case extendLength:
      _range.length++;
      break;

     case advanceLocationToNext:
      _range.location=NSMaxRange(_range)+1;
      _range.length=0;
      break;

     case advanceLocationToCurrent:
      _range.location=NSMaxRange(_range);
      _range.length=0;
      break;
    }
   }
}

-(void)loadOutputQueue {
   NSURL    *url=[_request URL];
   NSString *path=[url relativePath];
   NSString *query=[url query];
   
   if([query length])
    path=[NSString stringWithFormat:@"%@?%@",path,query];
		
   NSString        *host=[url host];
   NSMutableString *string=[NSMutableString string];
   
   [string appendFormat:@"%@ %@ HTTP/1.1\015\012",[_request HTTPMethod],path];
   [string appendFormat:@"Host: %@\015\012",host];
   [string appendFormat:@"Accept: */*\015\012"];

   NSMutableDictionary *headers=[[[_request allHTTPHeaderFields] mutableCopy] autorelease];
   NSEnumerator *state=[headers keyEnumerator];
   NSString     *key;

    BOOL contentLengthHeaderSetExplicitly = NO;
    
   while((key=[state nextObject])!=nil){     
    NSString *value=[headers objectForKey:key];
    [string appendFormat:@"%@: %@\015\012",key,value];
       if ([key isEqualToString: @"Content-Length"]) {
           contentLengthHeaderSetExplicitly = YES;
       }
   }

    if (contentLengthHeaderSetExplicitly == NO && [[_request HTTPBody] length] > 0) {
        // Many web-servers need to know the Content-Length before they're prepared to accept a POST.
        [string appendFormat:@"Content-Length: %d\015\012", [[_request HTTPBody] length]];
    }
    

   if(_cachedResponse!=nil){
    NSHTTPURLResponse *response=(NSHTTPURLResponse *)[_cachedResponse response];
    NSDictionary      *headers=[response allHeaderFields];
    NSString          *lastModified=nil;
    NSString          *etag=nil;
    
    for(NSString *key in headers){
     if([key caseInsensitiveCompare:@"last-modified"]==NSOrderedSame)
      lastModified=[headers objectForKey:key];
      
     if([key caseInsensitiveCompare:@"etag"]==NSOrderedSame)
      etag=[headers objectForKey:key];
}

    if(lastModified!=nil)
     [string appendFormat:@"If-Modified-Since: %@\015\012",lastModified];
    
    if(etag!=nil)
     [string appendFormat:@"If-None-Match: %@\015\012",etag];
}

   [string appendString:@"\015\012"];

   if(NSDebugEnabled){
#if 0
    NSLog(@"HTTP request=%@",string);
    NSLog(@"body=%@",[[[NSString alloc] initWithData:[_request HTTPBody] encoding:NSUTF8StringEncoding] autorelease]);
#endif
}
   NSData *data=[string dataUsingEncoding:NSUTF8StringEncoding];
   [_outputQueue addObject:data];
   if([[_request HTTPBody] length]){
    [_outputQueue addObject:[_request HTTPBody]];
		}
		}

-(void)startLoading {
   [self loadOutputQueue];
		}

-(void)stopLoading {
   [_inputStream setDelegate:nil];
   [_outputStream setDelegate:nil];
   for(NSString *mode in _modes){
    [_inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:mode];
    [_outputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:mode];
	}
   [_inputStream close];
   [_inputStream release];
   _inputStream=nil;
   [_outputStream close];
   [_outputStream release];
   _outputStream=nil;
	
   [_timeout invalidate];
   [_timeout release];
   _timeout=nil;
}
		
-(void)timeout:(NSTimer *)timer {
    NSDictionary *userInfo=[NSDictionary dictionaryWithObject:@"Connection timed out" forKey:NSLocalizedDescriptionKey];
    NSError *error=[NSError errorWithDomain:NSURLErrorDomain code:NSURLErrorTimedOut userInfo:userInfo];
    
    [self stopLoading];
    
    [_client URLProtocol:self didFailWithError:error];
}


-(void)inputStream:(NSInputStream *)stream handleEvent:(NSStreamEvent)streamEvent  {
    switch(streamEvent){
    
     case NSStreamEventHasBytesAvailable:
		{
		uint8_t   buffer[8192];
		NSInteger size=[stream read:buffer maxLength:8192];
		switch (size) {
			case 0: // We're actually at the end - the stream was lying
			case -1: // Or the stream is not even open - so no data for us.
				[self advanceStateWithData:[NSData data]];
				break;
			default:
				[self advanceStateWithData:[NSData dataWithBytes:buffer length:size]];
				break;
		}
		}  
      break;
     
     case NSStreamEventEndEncountered:
      [self advanceStateWithData:[NSData data]];
      break;
    
     default:
      break;
    }
}

-(void)outputStream:(NSOutputStream *)stream handleEvent:(NSStreamEvent)streamEvent 
{
	if(streamEvent==NSStreamEventHasSpaceAvailable) {
     if([_outputQueue count]==0){
		}
     else {
      NSData   *data=[_outputQueue objectAtIndex:0];
      uint8_t   buffer[8192];
      NSInteger length=[data length]-_outputNextOffset;
		
      length=MIN(length,8192);
		
      [data getBytes:buffer range:NSMakeRange(_outputNextOffset,length)];
		
      _outputNextOffset+=length;
      if(([data length]-_outputNextOffset)==0){
       [_outputQueue removeObjectAtIndex:0];
       _outputNextOffset=0;
	}

      [stream write:buffer maxLength:length];
#ifdef DEBUG
         length = MIN(length, 256);
         NSData *dump = [NSData dataWithBytes: buffer length: length];
         NSString *str = [[[NSString alloc] initWithData: dump encoding: NSUTF8StringEncoding] autorelease];
         NSLog(@"sent: %@ ...", str);
#endif
}
	}

}

-(void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)streamEvent {
   if([stream isKindOfClass:[NSInputStream class]])
    [self inputStream:(NSInputStream *)stream handleEvent:streamEvent];
   else if([stream isKindOfClass:[NSOutputStream class]])
    [self outputStream:(NSOutputStream *)stream handleEvent:streamEvent];
}

+(BOOL)canInitWithRequest:(NSURLRequest *)request {
   NSString *scheme=[[request URL] scheme];
      
   if([scheme isEqualToString:@"http"])
    return YES;
   if([scheme isEqualToString:@"https"])
    return YES;

   return NO;
}
-initWithRequest:(NSURLRequest *)request cachedResponse:(NSCachedURLResponse *)response client:(id <NSURLProtocolClient>)client {
   [super initWithRequest:request cachedResponse:response client:client];
   
   _modes=[[NSMutableArray arrayWithObject:NSDefaultRunLoopMode] retain];
   _outputQueue=[[NSMutableArray alloc] init];
   _outputNextOffset=0;

   NSURL    *url=[_request URL];
   NSString *scheme=[url scheme];
   NSString *hostName=[url host];
   NSNumber *portNumber=[url port];
   
   if(portNumber==nil){
    if([scheme isEqualToString:@"https"])
     portNumber=[NSNumber numberWithInt:443];
    else
     portNumber=[NSNumber numberWithInt:80];
   }
   
   NSHost *host=[NSHost hostWithName:hostName];
   
   [NSStream getStreamsToHost:host port:[portNumber intValue] inputStream:&_inputStream outputStream:&_outputStream];
   
   if(_inputStream==nil || _outputStream==nil){
    [self dealloc];
    return nil;
   }
   
   if([scheme isEqualToString:@"https"]){
    NSMutableDictionary *sslProperties=[NSMutableDictionary new];
    
    [sslProperties setObject:NSStreamSocketSecurityLevelNegotiatedSSL forKey:NSStreamSocketSecurityLevelKey];
    
    [_inputStream setProperty:sslProperties forKey:(NSString *)kCFStreamPropertySSLSettings];
    [_outputStream setProperty:sslProperties forKey:(NSString *)kCFStreamPropertySSLSettings];
   }
   
   [_inputStream setDelegate:self];
   [_outputStream setDelegate:self];
   
   _timeout=[[NSTimer timerWithTimeInterval:[request timeoutInterval] target:self selector:@selector(timeout:) userInfo:nil repeats:NO] retain];
   
   for(NSString *mode in _modes){
    [_inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:mode];
    [_outputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:mode];
    [[NSRunLoop currentRunLoop] addTimer:_timeout forMode:mode];
   }
    
	[_inputStream retain];
	[_outputStream retain];
	[_inputStream open];
	[_outputStream open];

   _data=[NSMutableData new];
   _range=NSMakeRange(0,0);
   _rawHeaders=[NSMutableDictionary new];
   _headers=[NSMutableDictionary new];
	return self;
}

-(void)dealloc {
   [_modes release];
   [_inputStream close];
   [_inputStream release];
   [_outputStream close];
   [_outputStream release];
   
   [_timeout invalidate];
   [_timeout release];
   
   [_data release];
   [_rawHeaders release];
   [_headers release];
   
   [super dealloc];
}

-(void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   [_inputStream scheduleInRunLoop:runLoop forMode:mode];
   [_outputStream scheduleInRunLoop:runLoop forMode:mode];
   [[NSRunLoop currentRunLoop] addTimer:_timeout forMode:mode];
}

-(void)unscheduleFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   [_inputStream removeFromRunLoop:runLoop forMode:mode];
   [_outputStream removeFromRunLoop:runLoop forMode:mode];
//  FIXME: no official way to remove timer
}

@end
