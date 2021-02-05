/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSHTTPURLResponse.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSRaise.h>

@implementation NSHTTPURLResponse

static NSString *valueInHeaders(NSDictionary *headers,NSString *key){
   NSEnumerator *state=[headers keyEnumerator];
   NSString     *checkKey;
   
   key=[key lowercaseString];
   
   while((checkKey=[state nextObject])!=nil){
    if([[checkKey lowercaseString] isEqualToString:key])
     return [headers objectForKey:checkKey];
   }
   
   return nil;
}

static NSString *mimeTypeFromContentType(NSString *contentType){
// FIXME
   return contentType;
}

static NSString *textEncodingNameFromContentType(NSString *contentType){
// FIXME
   return contentType;
}

-initWithURL:(NSURL *)url statusCode:(NSInteger)statusCode headers:(NSDictionary *)headers {
   NSString *contentType=valueInHeaders(headers,@"content-type");
   NSInteger contentLength=[valueInHeaders(headers,@"content-length") integerValue];
   NSString *mimeType=mimeTypeFromContentType(contentType);
   NSString *textEncodingName=textEncodingNameFromContentType(contentType);
   
   [super initWithURL:url MIMEType:mimeType expectedContentLength:contentLength textEncodingName:textEncodingName];
   
   _statusCode=statusCode;
   _allHeaderFields=[headers retain];
   return self;
}

+(NSString *)localizedStringForStatusCode:(NSInteger)statusCode {
   NSUnimplementedMethod();
   return nil;
}

-(NSDictionary *)allHeaderFields {
   return _allHeaderFields;
}

-(NSInteger)statusCode {
   return _statusCode;
}

@end
