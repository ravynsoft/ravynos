/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSInputStream_data.h>
#import <Foundation/NSData.h>
#import <Foundation/NSNumber.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSError.h>

@implementation NSInputStream_data

-initWithData:(NSData *)data {
   _delegate=self;
   _error=nil;
   _status=NSStreamStatusNotOpen;
   _data=[data retain];
   _position=0;
   return self;
}

-(void)dealloc {
   [_error release];
   [_data release];
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

-propertyForKey:(NSString *)key {
#if 0
// As of 10.5, data based streams do not implement NSStreamFileCurrentOffsetKey
   if([key isEqualToString:NSStreamFileCurrentOffsetKey])
    return [NSNumber numberWithLongLong:_position];
#endif
   return nil;
}

-(BOOL)setProperty:property forKey:(NSString *)key {
#if 0
// As of 10.5, data based streams do not implement NSStreamFileCurrentOffsetKey
   if([key isEqualToString:NSStreamFileCurrentOffsetKey]){
    _position=[property longLongValue];
    return YES;
   }
#endif
   return NO;
}

-(void)open {
   if(_status==NSStreamStatusNotOpen){
    _status=NSStreamStatusOpen;
   }
}

-(void)close {
   _status=NSStreamStatusClosed;
}

-(BOOL)getBuffer:(uint8_t **)buffer length:(NSUInteger *)length {
   return NO;
}

-(BOOL)hasBytesAvailable {
   return _position<[_data length];
}

-(NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)maxLength {
   if(_status!=NSStreamStatusOpen)
    return -1;
   else {
    const uint8_t *bytes=[_data bytes];
    NSUInteger i,length=[_data length];
   
    for(i=0;i<maxLength && _position<length;i++,_position++)
     buffer[i]=bytes[_position];
    
    return i;
   }
}

@end
