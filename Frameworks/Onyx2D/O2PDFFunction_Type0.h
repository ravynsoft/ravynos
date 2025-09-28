/* Copyright (c) 2010 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Function+PDF.h>
#import <Onyx2D/O2PDFObject.h>

@class NSArray;

@interface O2PDFFunction_Type0 : O2Function {
    unsigned _sizeCount;
    O2PDFInteger *_size;
    O2PDFInteger _bitsPerSample;
    O2PDFInteger _order;
    unsigned _encodeCount;
    O2PDFReal *_encode;
    unsigned _decodeCount;
    O2PDFReal *_decode;
    NSData *_data;
    NSUInteger _dataLength;
    const void *_bytes;
}

- initWithDomain:(O2PDFArray *)domain range:(O2PDFArray *)range size:(O2PDFArray *)size bitsPerSample:(O2PDFInteger)bps order:(O2PDFInteger)order encode:(O2PDFArray *)encode decode:(O2PDFArray *)decode data:(NSData *)data;

@end
