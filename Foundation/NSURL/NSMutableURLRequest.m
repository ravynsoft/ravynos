/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSMutableURLRequest.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import <Foundation/NSStream.h>
#import <Foundation/NSURL.h>

@interface NSURLRequest(private)
-initWithURLRequest:(NSURLRequest *)other;
@end

@implementation NSMutableURLRequest : NSURLRequest

-copyWithZone:(NSZone *)zone {
   return [[NSURLRequest alloc] initWithURLRequest:self];
}

-(void)setURL:(NSURL *)value {
   value=[value copy];
   [_url release];
   _url=value;
}

-(void)setCachePolicy:(NSURLRequestCachePolicy)value {
   _cachePolicy=value;
}

-(void)setTimeoutInterval:(NSTimeInterval)value {
   _timeoutInterval=value;
}

-(void)setHTTPMethod:(NSString *)value {
   value=[value copy];
   [_method release];
   _method=value;
}

-(void)setHTTPBody:(NSData *)value {
   value=[value copy];
   [_bodyDataOrStream release];
   _bodyDataOrStream=value;
}

-(void)setHTTPBodyStream:(NSInputStream *)value {
   value=[value retain];
   [_bodyDataOrStream release];
   _bodyDataOrStream=value;
}

-(void)setAllHTTPHeaderFields:(NSDictionary *)allValues {
   NSEnumerator *state=[allValues keyEnumerator];
   NSString     *key;
   
   [_headerFields removeAllObjects];
   while((key=[state nextObject])!=nil){
    NSString *value=[allValues objectForKey:key];
    
    if([key isKindOfClass:[NSString class]] && [value isKindOfClass:[NSString class]])
     [_headerFields setObject:value forKey:key];
   }
}

-(void)setValue:(NSString *)value forHTTPHeaderField:(NSString *)field {
   [_headerFields setObject:value forKey:field];
}

-(void)addValue:(NSString *)value forHTTPHeaderField:(NSString *)field {
   NSString *existing;
   
   existing=[_headerFields objectForKey:field];
   if(existing!=nil)
    value=[[existing stringByAppendingString:@","] stringByAppendingString:value];

   [_headerFields setObject:value forKey:field];
}

-(void)setHTTPShouldHandleCookies:(BOOL)value {
   _handleCookies=value;
}

-(void)setMainDocumentURL:(NSURL *)value {
   value=[value copy];
   [_mainDocumentURL release];
   _mainDocumentURL=value;
}

@end
