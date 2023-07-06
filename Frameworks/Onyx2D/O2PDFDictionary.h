/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFObject.h>
#import <Foundation/NSMapTable.h>

@class O2PDFObject, O2PDFArray, O2PDFStream, O2PDFString;

@interface O2PDFDictionary : O2PDFObject {
    NSMapTable *_table;
}

+ (O2PDFDictionary *)pdfDictionary;

- (void)setObjectForKey:(const char *)key value:(O2PDFObject *)object;
- (void)setBooleanForKey:(const char *)key value:(O2PDFBoolean)value;
- (void)setIntegerForKey:(const char *)key value:(O2PDFInteger)value;
- (void)setNumberForKey:(const char *)key value:(O2PDFReal)value;
- (void)setNameForKey:(const char *)key value:(const char *)value;

- (O2PDFObject *)inheritedForCStringKey:(const char *)cStringKey
                              typecheck:(O2PDFObjectType)aType;

- (BOOL)getObjectForKey:(const char *)key value:(O2PDFObject **)objectp;
- (BOOL)getNullForKey:(const char *)key;
- (BOOL)getBooleanForKey:(const char *)key value:(O2PDFBoolean *)valuep;
- (BOOL)getIntegerForKey:(const char *)key value:(O2PDFInteger *)valuep;
- (BOOL)getNumberForKey:(const char *)key value:(O2PDFReal *)valuep;
- (BOOL)getNameForKey:(const char *)key value:(const char **)namep;
- (BOOL)getStringForKey:(const char *)key value:(O2PDFString **)stringp;
- (BOOL)getArrayForKey:(const char *)key value:(O2PDFArray **)arrayp;
- (BOOL)getDictionaryForKey:(const char *)key value:(O2PDFDictionary **)dictionaryp;
- (BOOL)getStreamForKey:(const char *)key value:(O2PDFStream **)streamp;

@end

unsigned long O2PDFHashCString(NSMapTable *table, const void *data);
BOOL O2PDFIsEqualCString(NSMapTable *table, const void *data1, const void *data2);
void O2PDFFreeCString(NSMapTable *table, void *data);
extern NSMapTableKeyCallBacks O2PDFOwnedCStringKeyCallBacks;
