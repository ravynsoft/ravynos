/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFObject_R.h>
#import <Onyx2D/O2PDFxref.h>
#import <Onyx2D/O2PDFContext.h>
#import <Foundation/NSString.h>

@implementation O2PDFObject_R

-initWithNumber:(O2PDFInteger)number generation:(O2PDFInteger)generation xref:(O2PDFxref *)xref {
   _number=number;
   _generation=generation;
   _xref=[xref retain];
   return self;
}

-(void)dealloc {
   [_xref release];
   [super dealloc];
}

+pdfObjectWithNumber:(O2PDFInteger)number generation:(O2PDFInteger)generation xref:(O2PDFxref *)xref {
   return [[[self alloc] initWithNumber:number generation:generation xref:xref] autorelease];
}

-(O2PDFObject *)realObject {
   return [_xref objectAtNumber:_number generation:_generation];
}

-(O2PDFObjectType)objectTypeNoParsing {
   return O2PDFObjectType_R;
}

-(O2PDFObjectType)objectType {
   return [[_xref objectAtNumber:_number generation:_generation] objectType];
}

-(BOOL)checkForType:(O2PDFObjectType)type value:(void *)value {
   return [[_xref objectAtNumber:_number generation:_generation] checkForType:type value:value];
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ (%d %d) %@>",isa,_number,_generation,[self realObject]];
}

-(void)encodeWithPDFContext:(O2PDFContext *)encoder {
   [encoder appendFormat:@"%d %d R ",_number,_generation];
}

@end
