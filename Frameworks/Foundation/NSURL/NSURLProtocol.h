/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <Foundation/NSURLCache.h>

@class NSURLProtocol, NSURLRequest, NSURLResponse, NSURLAuthenticationChallenge, NSCachedURLResponse, NSData, NSError, NSMutableURLRequest;

@protocol NSURLProtocolClient
- (void)URLProtocol:(NSURLProtocol *)urlProtocol wasRedirectedToRequest:(NSURLRequest *)request redirectResponse:(NSURLResponse *)redirect;
- (void)URLProtocol:(NSURLProtocol *)urlProtocol didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge;
- (void)URLProtocol:(NSURLProtocol *)urlProtocol didCancelAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge;
- (void)URLProtocol:(NSURLProtocol *)urlProtocol didReceiveResponse:(NSURLResponse *)response cacheStoragePolicy:(NSURLCacheStoragePolicy)policy;
- (void)URLProtocol:(NSURLProtocol *)urlProtocol cachedResponseIsValid:(NSCachedURLResponse *)response;
- (void)URLProtocol:(NSURLProtocol *)urlProtocol didLoadData:(NSData *)data;
- (void)URLProtocol:(NSURLProtocol *)urlProtocol didFailWithError:(NSError *)error;
- (void)URLProtocolDidFinishLoading:(NSURLProtocol *)urlProtocol;
@end

@interface NSURLProtocol : NSObject {
    NSURLRequest *_request;
    NSCachedURLResponse *_cachedResponse;
    id<NSURLProtocolClient> _client;
}

+ (BOOL)registerClass:(Class)cls;
+ (void)unregisterClass:(Class)cls;

+ propertyForKey:(NSString *)key inRequest:(NSURLRequest *)request;
+ (void)removePropertyForKey:(NSString *)key inRequest:(NSMutableURLRequest *)request;
+ (void)setProperty:value forKey:(NSString *)key inRequest:(NSMutableURLRequest *)request;

+ (BOOL)canInitWithRequest:(NSURLRequest *)request;
+ (NSURLRequest *)canonicalRequestForRequest:(NSURLRequest *)request;
+ (BOOL)requestIsCacheEquivalent:(NSURLRequest *)request toRequest:(NSURLRequest *)other;

- initWithRequest:(NSURLRequest *)request cachedResponse:(NSCachedURLResponse *)response client:(id<NSURLProtocolClient>)client;

- (NSURLRequest *)request;
- (NSCachedURLResponse *)cachedResponse;
- (id<NSURLProtocolClient>)client;

- (void)startLoading;
- (void)stopLoading;

@end
