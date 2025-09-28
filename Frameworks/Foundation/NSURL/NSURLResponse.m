/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSURLResponse.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSRaise.h>

@implementation NSURLResponse

-initWithURL:(NSURL *)url MIMEType:(NSString *)mimeType expectedContentLength:(NSInteger)expectedLength textEncodingName:(NSString *)encoding {
   _url=[url retain];
   _mimeType=[mimeType retain];
   _expectedContentLength=expectedLength;
   _encoding=[encoding retain];
   return self;
}

-(void)dealloc {
   [_url release];
   [_mimeType release];
   [_encoding release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-initWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
   return nil;
}

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}


- (NSURL *)URL
{
   return _url;
}


-(NSString *)MIMEType {
   return _mimeType;
}

-(long long)expectedContentLength {
   return _expectedContentLength;
}

-(NSString *)textEncodingName {
   return _encoding;
}

-(NSString *)suggestedFilename {
   NSUnimplementedMethod();
   return nil;
}


@end
