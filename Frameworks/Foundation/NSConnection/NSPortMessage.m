/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSPortMessage.h>
#import <Foundation/NSPort.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSRaise.h>

@implementation NSPortMessage

-initWithSendPort:(NSPort *)sendPort receivePort:(NSPort *)receivePort components:(NSArray *)components {
   _sendPort=[sendPort retain];
   _receivePort=[receivePort retain];
   _components=[[NSMutableArray alloc] initWithArray:components];
   _msgid=0;
   return self;
}

-(void)dealloc {
   [_sendPort release];
   [_receivePort release];
   [_components release];
   [super dealloc];
}

-(uint32_t)msgid {
   return _msgid;
}

-(NSArray *)components {
   return _components;
}

-(NSPort *)sendPort {
   return _sendPort;
}

-(NSPort *)receivePort {
   return _receivePort;
}

-(void)setMsgid:(uint32_t)msgid {
   _msgid=msgid;
}

-(BOOL)sendBeforeDate:(NSDate *)date {
   return [_sendPort sendBeforeDate:date msgid:_msgid components:_components from:_receivePort reserved:0];
}

@end
