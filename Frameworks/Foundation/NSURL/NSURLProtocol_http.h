/* Copyright (c) 2008 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSURLProtocol.h>
#import <Foundation/NSRange.h>

@class NSInputStream, NSOutputStream, NSMutableData, NSMutableDictionary, NSMutableArray, NSTimer;

@interface NSURLProtocol_http : NSURLProtocol {
    NSMutableArray *_modes;
    NSInputStream *_inputStream;
    NSOutputStream *_outputStream;
    NSTimer *_timeout;

    // output state
    NSMutableArray *_outputQueue;
    NSInteger _outputNextOffset;

    // parsing state
    NSMutableData *_data;
    const uint8_t *_bytes;
    NSUInteger _length;
    int _state;
    NSRange _range;

    NSInteger _statusCode;
    NSString *_currentKey;
    NSMutableDictionary *_rawHeaders;
    NSMutableDictionary *_headers;
    NSInteger _expectedContentLength;
    NSInteger _totalContentReceived;

    NSInteger _chunkSize;
}

@end
