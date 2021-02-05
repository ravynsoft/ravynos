/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSURLConnection.h>
#import <Foundation/NSURLRequest.h>
#import <Foundation/NSURLProtocol.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSStream.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSHost.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSURLError.h>
#import <Foundation/NSURLCache.h>
#import <Foundation/NSCachedURLResponse.h>
#import <Foundation/NSError.h>
#import "NSURLConnectionState.h"

@interface NSURLProtocol(private)
+(Class)_URLProtocolClassForRequest:(NSURLRequest *)request;
-(void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;
-(void)unscheduleFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;
@end;

@interface NSURLConnection(private) <NSURLProtocolClient>
@end

@implementation NSURLConnection

+(BOOL)canHandleRequest:(NSURLRequest *)request {
   return ([NSURLProtocol _URLProtocolClassForRequest:request]!=nil)?YES:NO;
}

+(NSData *)sendSynchronousRequest:(NSURLRequest *)request returningResponse:(NSURLResponse **)responsep error:(NSError **)errorp {
   NSURLConnectionState *state=[[[NSURLConnectionState alloc] init] autorelease];
   NSURLConnection      *connection=[[self alloc] initWithRequest:request delegate:state];
   
   if(connection==nil){
   
    if(errorp!=NULL){
     *errorp=[NSError errorWithDomain:NSURLErrorDomain code:NSURLErrorCannotConnectToHost userInfo:nil];
    }
    
    return nil;
   }

   NSString *mode=@"NSURLConnectionRequestMode";
   
    [connection scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:mode];

	[state receiveAllDataInMode:mode];
    [connection unscheduleFromRunLoop:[NSRunLoop currentRunLoop] forMode:mode];

    [connection cancel];

	// Now let's see what we should return to the caller...
	
	NSData *result= nil; 

	if([state error]) {
        if (errorp) {
            *errorp=[state error];
        } else {
            NSLog(@"error occurred during request: %@", [state error]);
        }
	} else {
		// Looks good - give them the data
		result = [[connection->_mutableData retain] autorelease];
	}
	
   if(responsep!=NULL)
    *responsep=[[connection->_response retain] autorelease];
    
	// The memory management isn't clear - NSURLConnection wants to request autorelease of self in some cases (see URLProtocolDidFinishLoading:
	// But that conflicts with this explicit release - which matches the alloc of the connection above.
   [connection release];
 
   return result;
}

+(NSURLConnection *)connectionWithRequest:(NSURLRequest *)request delegate:delegate {
   return [[[self alloc] initWithRequest:request delegate:delegate] autorelease];
}

-initWithRequest:(NSURLRequest *)request delegate:delegate startImmediately:(BOOL)startLoading {
   _request=[request copy];
   Class cls=[NSURLProtocol _URLProtocolClassForRequest:request];
   
   if((_protocol=[[cls alloc] initWithRequest:_request cachedResponse:[[NSURLCache sharedURLCache] cachedResponseForRequest:_request] client:self])==nil){
    [self dealloc];
    return nil;
   }
   
   _delegate=[delegate retain];
   
   [self retain];

   if(startLoading)
    [self start];


   return self;
}

-initWithRequest:(NSURLRequest *)request delegate:delegate {
   return [self initWithRequest:request delegate:delegate startImmediately:YES];
}

-(void)dealloc {
   [_request release];
   [_protocol release];
   [_delegate release];
   [_response release];
   [_mutableData release];
   [super dealloc];
}

-(void)start {
   [_protocol startLoading];
}

-(void)cancel {
   [_protocol stopLoading];
   }

-(void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   [_protocol scheduleInRunLoop:runLoop forMode:mode];
}

-(void)unscheduleFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   [_protocol unscheduleFromRunLoop:runLoop forMode:mode];
}

-(void)URLProtocol:(NSURLProtocol *)urlProtocol wasRedirectedToRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)redirect {
#if DEBUG
	NSLog(@"wasRedirectedToRequest: %@", request);
#endif
	[_delegate connection:self willSendRequest:request redirectResponse:redirect];
}

-(void)URLProtocol:(NSURLProtocol *)urlProtocol didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
#if DEBUG
	NSLog(@"didReceiveAuthenticationChallenge: %@", challenge);
#endif
  // [_delegate connection:self didReceiveAuthenticationChallenge];
}

-(void)URLProtocol:(NSURLProtocol *)urlProtocol didCancelAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge {
#if DEBUG
	NSLog(@"didCancelAuthenticationChallenge: %@", challenge);
#endif
  // [_delegate connection:self didCancelAuthenticationChallenge];
}

-(void)URLProtocol:(NSURLProtocol *)urlProtocol didReceiveResponse:(NSURLResponse *)response cacheStoragePolicy:(NSURLCacheStoragePolicy)policy {
#if DEBUG
	NSLog(@"didReceiveResponse: %@", response);
#endif
    _response=[response retain];
    _storagePolicy=policy;
    
   if([_delegate respondsToSelector:@selector(connection:didReceiveResponse:)])
    [_delegate connection:self didReceiveResponse:response];
}

-(void)URLProtocol:(NSURLProtocol *)urlProtocol cachedResponseIsValid:(NSCachedURLResponse *)cachedResponse {
#if DEBUG
	NSLog(@"cachedResponseIsValid: %@", cachedResponse);
#endif
}

-(void)URLProtocol:(NSURLProtocol *)urlProtocol didLoadData:(NSData *)data {

#if DEBUG
    NSString *str = [[[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding] autorelease];
	NSLog(@"didLoadData: %@", str);
#endif
	
   if(_mutableData==nil)
    _mutableData=[[NSMutableData alloc] init];
    
   [_mutableData appendData:data];
   
   [_delegate connection:self didReceiveData:data];
}

-(void)URLProtocol:(NSURLProtocol *)urlProtocol didFailWithError:(NSError *)error {
#if DEBUG
	NSLog(@"didFailWithError: %@", error);
#endif
	
	[_delegate connection:self didFailWithError:error];

	// The memory-management isn't clear - see sendSynchronousRequest: - it explicitly releases the connection - so this
	// autorelease means it will be over-released if there's an error (like a 404 code) and crash
	// [self autorelease];	
}

-(void)URLProtocolDidFinishLoading:(NSURLProtocol *)urlProtocol {
#if DEBUG
	NSLog(@"URLProtocolDidFinishLoading: %@", urlProtocol);
#endif
   if(_storagePolicy==NSURLCacheStorageNotAllowed){
    [[NSURLCache sharedURLCache] removeCachedResponseForRequest:_request];
   }
   else {
    NSCachedURLResponse *cachedResponse=[[NSCachedURLResponse alloc] initWithResponse:_response data:_mutableData userInfo:nil storagePolicy:_storagePolicy];
   
    if([_delegate respondsToSelector:@selector(connection:willCacheResponse:)])
     cachedResponse=[_delegate connection:self willCacheResponse:cachedResponse];

    if(cachedResponse!=nil){
     [[NSURLCache sharedURLCache] storeCachedResponse:cachedResponse forRequest:_request];
    }
   }
   
	if([_delegate respondsToSelector:@selector(connectionDidFinishLoading:)])
		[_delegate performSelector:@selector(connectionDidFinishLoading:) withObject:self];

	// The memory-management isn't clear - see sendSynchronousRequest: - it explicitly releases the connection - so this
	// autorelease means it will most likely be over-released on a successful download and crash
	// [self autorelease];

}


@end
