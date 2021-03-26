/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2Context.h>
#import <Foundation/NSMapTable.h>

@class O2PDFContext, O2PDFxref, O2PDFObject, O2PDFDictionary, O2PDFArray, NSMutableData, NSMutableDictionary, O2DataConsumer;

extern const NSString *kO2PDFContextTitle;

@interface O2PDFContext : O2Context {
    O2DataConsumer *_dataConsumer;
    NSMutableDictionary *_fontCache;
    NSMapTable *_objectToRef;
    NSMutableArray *_indirectObjects;
    NSMutableArray *_indirectEntries;
    unsigned _nextNumber;
    O2PDFxref *_xref;
    O2PDFDictionary *_info;
    O2PDFDictionary *_catalog;
    O2PDFDictionary *_pages;
    O2PDFArray *_kids;
    O2PDFDictionary *_page;
    NSMutableDictionary *_categoryToNext;
    NSMutableArray *_textStateStack;
    NSMutableArray *_contentStreamStack;
    size_t _length;
}

- initWithConsumer:(O2DataConsumer *)consumer mediaBox:(const O2Rect *)mediaBox auxiliaryInfo:(NSDictionary *)auxiliaryInfo;

- (unsigned)length;

- (void)appendData:(NSData *)data;
- (void)appendBytes:(const void *)ptr length:(unsigned)length;
- (void)appendCString:(const char *)cString;
- (void)appendString:(NSString *)string;
- (void)appendFormat:(NSString *)format, ...;
- (void)appendPDFStringWithBytes:(const void *)bytes length:(unsigned)length;

- (O2PDFObject *)referenceForFontWithName:(NSString *)name size:(float)size;
- (void)setReference:(O2PDFObject *)reference forFontWithName:(NSString *)name size:(float)size;
- (O2PDFObject *)referenceForObject:(O2PDFObject *)object;

- (void)encodePDFObject:(O2PDFObject *)object;
- (O2PDFObject *)encodeIndirectPDFObject:(O2PDFObject *)object;

@end
