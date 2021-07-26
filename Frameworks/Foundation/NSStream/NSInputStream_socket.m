/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSInputStream_socket.h>
#import <Foundation/NSSelectInputSource.h>
#import <Foundation/NSSelectSet.h>
#import <Foundation/NSError.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/CFSSLHandler.h>
#import <Foundation/NSData.h>
#import <CFNetwork/CFSocketStream.h>

@implementation NSInputStream_socket

-initWithSocket:(NSSocket *)socket streamStatus:(NSStreamStatus)status {
   _events=0;
   _callBack=NULL;
   _delegate=self;
   _error=nil;
   _status=status;
   _socket=[socket retain];
   _inputSource=nil;
   return self;
}

-(void)dealloc {
   [_error release];
   [_socket release];
   [_inputSource release];
   [super dealloc];
}

-(NSSocket *)socket {
   return _socket;
}

-(int)fileDescriptor {
   return [_socket fileDescriptor];
}

-delegate {
   return _delegate;
}

-(void)setClientEvents:(CFOptionFlags)events callBack:(CFReadStreamClientCallBack)callBack context:(CFStreamClientContext *)context {
   _events=events;
   _callBack=callBack;

   if(context!=NULL && context->info!=NULL && context->retain!=NULL)
    context->retain(context->info);

   _context.version=0;
   if(_context.info!=NULL && _context.release!=NULL)
    _context.release(_context.info);
   _context.info=NULL;
   _context.retain=NULL;
   _context.release=NULL;

   if(context!=NULL)
    _context=*context;
}

-(void)setDelegate:delegate {
   _delegate=delegate;
   if(_delegate==nil)
    _delegate=self;
}

-propertyForKey:(NSString *)key {
   if([key isEqualToString:(NSString *)kCFStreamPropertySocketNativeHandle]){
    CFSocketNativeHandle value=(_socket==nil)?-1:[_socket fileDescriptor];

    return [NSData dataWithBytes:&value length:sizeof(value)];
   }

   NSUnimplementedMethod();
   return nil;
}

-(BOOL)setProperty:property forKey:(NSString *)key {
   if([key isEqualToString:(NSString *)kCFStreamPropertySSLSettings])
    return [_socket setSSLProperties:(CFDictionaryRef)property];

   NSUnimplementedMethod();
   return NO;
}

-(void)open {
   if(_status==NSStreamStatusNotOpen){
    _status=NSStreamStatusOpening;
   }
}

-(NSError *)streamError {
   return _error;
}

-(NSStreamStatus)streamStatus {
   return _status;
}

-(void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   if(_inputSource==nil){
    _inputSource=[[NSSelectInputSource alloc] initWithSocket:_socket];
    [_inputSource setDelegate:self];
    [_inputSource setSelectEventMask:NSSelectReadEvent];
   }

   [runLoop addInputSource:_inputSource forMode:mode];
}

-(void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode {
   if(_inputSource!=nil)
    [runLoop removeInputSource:_inputSource forMode:mode];
}

-(void)close {
   [_inputSource setSelectEventMask:0];
   [_inputSource invalidate];
   [_socket close];
}

-(BOOL)getBuffer:(uint8_t **)buffer length:(NSUInteger *)length {
   *buffer=NULL;
   *length=0;
   return NO;
}

-(BOOL)hasBytesAvailable {
   BOOL result=NO;

   if(_status==NSStreamStatusOpen){
    CFSSLHandler *sslHandler=[_socket sslHandler];

    if(sslHandler==nil)
     result=[_socket hasBytesAvailable];
    else {
     if([_socket hasBytesAvailable])
      if([sslHandler transferOneBufferFromSocketToSSL:_socket]<=0){
       // If the read failed we want to return YES so that the end of stream can be read
       return YES;
      }

     result=([sslHandler readBytesAvailable]>0)?YES:NO;
    }
   }

   return result;
}

-(NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)maxLength {
   NSInteger result;

   if(_status==NSStreamStatusAtEnd)
    return 0;

   if(_status!=NSStreamStatusOpen && _status!=NSStreamStatusOpening)
    return -1;

   CFSSLHandler *sslHandler=[_socket sslHandler];

   if(sslHandler==nil)
    result=[_socket read:buffer maxLength:maxLength];
   else {
    [sslHandler runWithSocket:_socket];

    result=[sslHandler readPlaintext:buffer maxLength:maxLength];

    [sslHandler runWithSocket:_socket];
   }

   if(result==0)
    _status=NSStreamStatusAtEnd;
   if(result==-1)
    _status=NSStreamStatusError;

   return result;
}


- (void)selectInputSource:(NSSelectInputSource *)inputSource selectEvent:(NSUInteger)selectEvent
{
    NSStreamEvent event;

    switch(_status) {
        case NSStreamStatusOpening:
            _status = NSStreamStatusOpen;
            event = NSStreamEventOpenCompleted;
            break;

        case NSStreamStatusOpen:;
            if ([self hasBytesAvailable]) {
                event = NSStreamEventHasBytesAvailable;
            } else {
                event = NSStreamEventNone;
            }
            break;

        case NSStreamStatusAtEnd:
            event = NSStreamEventEndEncountered;
            break;

        default:
            event = NSStreamEventNone;
            break;
    }

    if (event != NSStreamEventNone) {
        if (_callBack != NULL) {
            if (_events & event) {
                _callBack((CFReadStreamRef)self, (CFStreamEventType)event, _context.info);
            }
        } else {
            if ([_delegate respondsToSelector:@selector(stream:handleEvent:)]) {
                [_delegate stream:self handleEvent:event];
            }
        }
    }
}


@end
