/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>

@class O2PDFContext;

typedef BOOL O2PDFBoolean;
typedef int O2PDFInteger;
typedef float O2PDFReal;

typedef enum {
    kO2PDFObjectTypeNull = 1,
    kO2PDFObjectTypeBoolean,
    kO2PDFObjectTypeInteger,
    kO2PDFObjectTypeReal,
    kO2PDFObjectTypeName,
    kO2PDFObjectTypeString,
    kO2PDFObjectTypeArray,
    kO2PDFObjectTypeDictionary,
    kO2PDFObjectTypeStream,

    ///------------
    O2PDFObjectType_R,
    O2PDFObjectType_identifier,

    O2PDFObjectTypeMark_array_open,
    O2PDFObjectTypeMark_dictionary_open,
    O2PDFObjectTypeMark_array_close,
    O2PDFObjectTypeMark_dictionary_close,

    // PS calculator
    O2PDFObjectTypeMark_proc_open,
    O2PDFObjectTypeMark_proc_close,
    kO2PDFObjectTypeBlock,

    //
    O2PDFObjectTypeCached,
} O2PDFObjectType;

@interface O2PDFObject : NSObject

- (O2PDFObject *)realObject;

- (O2PDFObjectType)objectTypeNoParsing;
- (O2PDFObjectType)objectType;

- (BOOL)checkForType:(O2PDFObjectType)type value:(void *)value;

- (BOOL)isByReference;
- (void)encodeWithPDFContext:(O2PDFContext *)encoder;

@end

void O2PDFError(const char *file, int line, NSString *format, ...);
void O2PDFFix(const char *file, int line, NSString *format, ...);
