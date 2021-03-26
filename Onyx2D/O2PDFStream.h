/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFObject.h>

@class NSData, NSMutableData;
@class O2PDFDictionary, O2PDFxref;

@interface O2PDFStream : O2PDFObject {
    O2PDFDictionary *_dictionary;
    NSData *_data;
    NSData *_resultData;
    O2PDFxref *_xref;
}

+ (O2PDFStream *)pdfStream;
+ (O2PDFStream *)pdfStreamWithData:(NSData *)data;
+ (O2PDFStream *)pdfStreamWithBytes:(const void *)bytes length:(unsigned)length;

- initWithDictionary:(O2PDFDictionary *)dictionary xref:(O2PDFxref *)xref position:(O2PDFInteger)position;

- (O2PDFxref *)xref;
- (O2PDFDictionary *)dictionary;
- (NSData *)data;
- (NSMutableData *)mutableData;

@end
