/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSInputStream.h>
#import <Foundation/NSSocket.h>
#import <Foundation/NSSelectInputSource.h>
#import <CoreFoundation/CFStream.h>

@class NSSocket, NSSelectInputSource;

@interface NSInputStream_socket : NSInputStream {
    CFOptionFlags _events;
    CFReadStreamClientCallBack _callBack;
    CFStreamClientContext _context;
    id _delegate;
    NSError *_error;
    NSStreamStatus _status;
    NSSocket *_socket;
    NSSelectInputSource *_inputSource;
}

- initWithSocket:(NSSocket *)socket streamStatus:(NSStreamStatus)status;

- (NSSocket *)socket;

- (void)setClientEvents:(CFOptionFlags)events callBack:(CFReadStreamClientCallBack)callBack context:(CFStreamClientContext *)context;

@end
