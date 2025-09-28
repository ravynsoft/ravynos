/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSURLProtectionSpace.h>
#import <Foundation/NSRaise.h>

@implementation NSURLProtectionSpace

-initWithHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm authenticationMethod:(NSString *)authenticationMethod {
   _host=[host copy];
   _port=port;
   _protocol=[protocol copy];
   _realm=[realm copy];
   _authenticationMethod=[authenticationMethod copy];
   _isProxy=NO;
   return self;
}

-initWithProxyHost:(NSString *)host port:(int)port protocol:(NSString *)protocol realm:(NSString *)realm authenticationMethod:(NSString *)authenticationMethod {
   _host=[host copy];
   _port=port;
   _protocol=[protocol copy];
   _realm=[realm copy];
   _authenticationMethod=[authenticationMethod copy];
   _isProxy=YES;
   return self;
}

-(void)dealloc {
   [_host release];
   [_protocol release];
   [_realm release];
   [_authenticationMethod release];
   [super dealloc];
}

-copyWithZone:(NSZone *)zone {
   return [self retain];
}

-(NSString *)host {
   return _host;
}

-(int)port {
   return _port;
}

-(NSString *)protocol {
   return _protocol;
}

-(NSString *)realm {
   return _realm;
}

-(NSString *)authenticationMethod {
   return _authenticationMethod;
}

-(NSString *)proxyType {
   NSUnimplementedMethod();
   return nil;
}

-(BOOL)receivesCredentialsSecurely {
   NSUnimplementedMethod();
   return NO;
}

-(BOOL)isProxy {
   return _isProxy;
}

@end
