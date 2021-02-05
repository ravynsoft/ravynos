/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSCachedURLResponse.h>

@implementation NSCachedURLResponse

-initWithResponse:(NSURLResponse *)response data:(NSData *)data userInfo:(NSDictionary *)userInfo storagePolicy:(NSURLCacheStoragePolicy)storagePolicy {
   _response=[response copy];
   _data=[data copy];
   _userInfo=[userInfo copy];
   _storagePolicy=storagePolicy;
   return self;
}

-initWithResponse:(NSURLResponse *)response data:(NSData *)data {
   return [self initWithResponse:response data:data userInfo:nil storagePolicy:NSURLCacheStorageAllowed];
}

-(void)dealloc {
   [_response release];
   [_data release];
   [_userInfo release];
   [super dealloc];
}

-(NSURLResponse *)response {
   return _response;
}

-(NSData *)data {
   return _data;
}

-(NSDictionary *)userInfo {
   return _userInfo;
}

-(NSURLCacheStoragePolicy)storagePolicy {
   return _storagePolicy;
}

@end
