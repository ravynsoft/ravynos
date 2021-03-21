/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFObject.h>

@class O2PDFString;

typedef O2PDFString *O2PDFStringRef;

@interface O2PDFString : O2PDFObject {
    unsigned _length : 31;
    unsigned _noCopyNoFree : 1;
    unsigned char *_bytes;
}

+ pdfObjectWithBytes:(const unsigned char *)bytes length:(unsigned)length;
+ pdfObjectWithBytesNoCopyNoFree:(const unsigned char *)bytes length:(unsigned)length;
+ pdfObjectWithCString:(const char *)cString;
+ pdfObjectWithString:(NSString *)string;

- (unsigned)length;
- (const unsigned char *)bytes;

size_t O2PDFStringGetLength(O2PDFStringRef string);
const unsigned char *O2PDFStringGetBytePtr(O2PDFStringRef string);

@end
