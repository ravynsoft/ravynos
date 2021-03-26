/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFOperatorTable.h>
#import <Onyx2D/O2PDFOperators.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <string.h>

@implementation O2PDFOperatorTable

+(O2PDFOperatorTable *)renderingOperatorTable {
   O2PDFOperatorTable *result=[[[O2PDFOperatorTable alloc] init] autorelease];
   
   O2PDF_render_populateOperatorTable(result);
   
   return result;
}

-init {
   _table=NSCreateMapTable(O2PDFOwnedCStringKeyCallBacks,NSNonOwnedPointerMapValueCallBacks,0);
   return self;
}

-(void)dealloc {
   NSFreeMapTable(_table);
   [super dealloc];
}

-(O2PDFOperatorCallback)callbackForName:(const char *)name {
   return NSMapGet(_table,name);
}

-(void)setCallback:(O2PDFOperatorCallback)callback forName:(const char *)name {
   char *copy=NSZoneMalloc(NULL,strlen(name)+1);
   
   strcpy(copy,name);
   
   NSMapInsert(_table,copy,callback);
}


@end
