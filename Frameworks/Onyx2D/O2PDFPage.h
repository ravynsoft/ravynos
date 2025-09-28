/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/O2AffineTransform.h>
#import <stdbool.h>

@class O2PDFPage;

typedef O2PDFPage *O2PDFPageRef;

typedef enum {
    kO2PDFMediaBox,
    kO2PDFCropBox,
    kO2PDFBleedBox,
    kO2PDFTrimBox,
    kO2PDFArtBox,
} O2PDFBox;

@class O2PDFDocument, O2PDFDictionary, O2Context;

@interface O2PDFPage : NSObject {
    O2PDFDocument *_document;
    int _pageNumber;
    O2PDFDictionary *_dictionary;
}

- initWithDocument:(O2PDFDocument *)document pageNumber:(int)pageNumber dictionary:(O2PDFDictionary *)dictionary;

+ (O2PDFPage *)pdfPageWithDocument:(O2PDFDocument *)document pageNumber:(int)pageNumber dictionary:(O2PDFDictionary *)dictionary;

- (O2PDFDocument *)document;
- (int)pageNumber;

- (O2PDFDictionary *)dictionary;

- (BOOL)getRect:(O2Rect *)rect forBox:(O2PDFBox)box;

- (int)rotationAngle;

O2AffineTransform O2PDFPageGetDrawingTransform(O2PDFPageRef self, O2PDFBox box, O2Rect rect, int clockwiseDegrees, bool preserveAspectRatio);

- (void)drawInContext:(O2Context *)context;

@end
