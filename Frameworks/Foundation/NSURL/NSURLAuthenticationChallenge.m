/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSURLAuthenticationChallenge.h>
#import <Foundation/NSURLProtectionSpace.h>
#import <Foundation/NSURLCredential.h>
#import <Foundation/NSURLResponse.h>
#import <Foundation/NSError.h>

@implementation NSURLAuthenticationChallenge

-initWithProtectionSpace:(NSURLProtectionSpace *)space proposedCredential:(NSURLCredential *)credential previousFailureCount:(int)failureCount failureResponse:(NSURLResponse *)failureResponse error:(NSError *)error sender:(id <NSURLAuthenticationChallengeSender>)sender {
   _protectionSpace=[space copy];
   _proposedCredential=[credential copy];
   _failureCount=failureCount;
   _failureResponse=[failureResponse copy];
   _sender=[sender retain];
   return self;
}

-initWithAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge sender:(id <NSURLAuthenticationChallengeSender>)sender {
   return [self initWithProtectionSpace:[challenge protectionSpace] proposedCredential:[challenge proposedCredential] previousFailureCount:[challenge previousFailureCount] failureResponse:[challenge failureResponse] error:[challenge error] sender:sender];
   return self;
}

-(void)dealloc {
   [_protectionSpace release];
   [_proposedCredential release];
   [_failureResponse release];
   [_error release];
   [_sender release];
   [super dealloc];
}

-(NSURLProtectionSpace *)protectionSpace {
   return _protectionSpace;
}

-(NSURLCredential *)proposedCredential {
   return _proposedCredential;
}

-(NSUInteger)previousFailureCount {
  return _failureCount;
}

-(NSURLResponse *)failureResponse {
   return _failureResponse;
}

-(NSError *)error {
   return _error;
}

-(id<NSURLAuthenticationChallengeSender>)sender {
   return _sender;
}

@end
