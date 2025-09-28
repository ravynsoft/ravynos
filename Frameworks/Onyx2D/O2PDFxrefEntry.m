/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFxrefEntry.h>
#import <Onyx2D/O2PDFObject.h>

@implementation O2PDFxrefEntry

-initWithPosition:(O2PDFInteger)position number:(O2PDFInteger)number generation:(O2PDFInteger)generation {
   _position=position;
   _number=number;
   _generation=generation;
   return self;
}

+(O2PDFxrefEntry *)xrefEntryWithPosition:(O2PDFInteger)position number:(O2PDFInteger)number generation:(O2PDFInteger)generation {
   return [[[self alloc] initWithPosition:position number:number generation:generation] autorelease];
}

-(unsigned)hash {
   return _number;
}

-(BOOL)isEqual:other {
   O2PDFxrefEntry *otherEntry;
   
   if(![other isKindOfClass:[O2PDFxrefEntry class]])
    return NO;
   
   otherEntry=other;
   
   return (_number==otherEntry->_number) && (_generation==otherEntry->_generation);
}

-(O2PDFInteger)position {
   return _position;
}

-(O2PDFInteger)number {
   return _number;
}

-(O2PDFInteger)generation {
   return _generation;
}

-(void)setPosition:(O2PDFInteger)value {
   _position=value;
}

@end
