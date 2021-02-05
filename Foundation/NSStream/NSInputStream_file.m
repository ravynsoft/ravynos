/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <Foundation/NSInputStream_file.h>
#import <Foundation/NSString.h>
#import <Foundation/NSError.h>
#import <Foundation/NSFileHandle.h>

@implementation NSInputStream_file

-initWithFileAtPath:(NSString *)path {
   _delegate=self;
   _error=nil;
   _status=NSStreamStatusNotOpen;
   _path=[path copy];
   _fileHandle=nil;
   return self;
}

-(void)dealloc {
   [_error release];
   [_path release];
   [_fileHandle release];
   [super dealloc];
}

-delegate {
   return _delegate;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
   if(_delegate==nil)
    _delegate=self;
}

-(NSError *)streamError {
   return _error;
}

-(NSStreamStatus)streamStatus {
   return _status;
}

-(void)open {
   if(_status==NSStreamStatusNotOpen){
    _status=NSStreamStatusOpen;
    _fileHandle=[[NSFileHandle fileHandleForReadingAtPath:_path] retain];
   }
}

-(void)close {
   _status=NSStreamStatusClosed;
   [_fileHandle closeFile];
}

-propertyForKey:(NSString *)key {
   return nil;
}

-(BOOL)getBuffer:(uint8_t **)buffer length:(NSUInteger *)length {
   return NO;
}

-(BOOL)hasBytesAvailable {
    return NO;
}

-(NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)length {
    return -1;
}

@end
