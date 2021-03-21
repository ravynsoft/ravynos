/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFObject.h>
#import <Foundation/NSMapTable.h>
@class NSData, NSMutableArray;

@class O2PDFObject;
@class O2PDFxrefEntry;
@class O2PDFDictionary;

@interface O2PDFxref : NSObject {
    NSData *_data;
    O2PDFxref *_previous;
    NSMapTable *_numberToEntries;
    NSMapTable *_entryToObject;
    NSMutableArray *_entriesInOrder;
    O2PDFDictionary *_trailer;
}

- initWithData:(NSData *)data;

- (NSData *)data;

- (O2PDFxref *)previous;
- (NSArray *)allEntries;

- (O2PDFObject *)objectAtNumber:(O2PDFInteger)number generation:(O2PDFInteger)generation;

- (O2PDFDictionary *)trailer;

- (void)setPreviousTable:(O2PDFxref *)table;
- (void)addEntry:(O2PDFxrefEntry *)entry;
- (void)addEntry:(O2PDFxrefEntry *)entry object:(O2PDFObject *)object;
- (void)setTrailer:(O2PDFDictionary *)trailer;

- (void)encodeWithPDFContext:(O2PDFContext *)encoder;

@end
