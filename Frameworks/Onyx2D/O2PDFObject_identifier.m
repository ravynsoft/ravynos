/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFObject_identifier.h>
#import <string.h>
#import <Foundation/NSString.h>

O2PDFIdentifier O2PDFClassifyIdentifier(const char *bytes,unsigned length) {
   char name[length+1];
   
   strncpy(name,bytes,length);
   name[length]='\0';

   
   if(strcmp(name,"true")==0)
    return O2PDFIdentifier_true;
   if(strcmp(name,"false")==0)
    return O2PDFIdentifier_false;
   if(strcmp(name,"null")==0)
    return O2PDFIdentifier_null;

   if(strcmp(name,"f")==0)
    return O2PDFIdentifier_f;
   if(strcmp(name,"n")==0)
    return O2PDFIdentifier_n;
   if(strcmp(name,"R")==0)
    return O2PDFIdentifier_R;
   if(strcmp(name,"xref")==0)
    return O2PDFIdentifier_xref;
   if(strcmp(name,"trailer")==0)
    return O2PDFIdentifier_trailer;
   if(strcmp(name,"startxref")==0)
    return O2PDFIdentifier_startxref;
   if(strcmp(name,"obj")==0)
    return O2PDFIdentifier_obj;
   if(strcmp(name,"endobj")==0)
    return O2PDFIdentifier_endobj;
   if(strcmp(name,"stream")==0)
    return O2PDFIdentifier_stream;
   if(strcmp(name,"endstream")==0)
    return O2PDFIdentifier_endstream;
    
   //NSLog(@"O2PDFClassifyIdentifier UNKNOWN [%s]",name);
   
   return O2PDFIdentifierUnknown;
}


@implementation O2PDFObject_identifier

-initWithIdentifier:(O2PDFIdentifier)identifier name:(const char *)bytes length:(unsigned)length {
   _identifier=identifier;
   _length=length;
   _bytes=NSZoneMalloc(NULL,_length+1);
   strncpy(_bytes,bytes,_length);
   _bytes[_length]='\0';
   return self;
}

-(void)dealloc {
   NSZoneFree(NULL,_bytes);
   [super dealloc];
}

+pdfObjectWithIdentifier:(O2PDFIdentifier)identifier name:(const char *)bytes length:(unsigned)length {
   return [[[self alloc] initWithIdentifier:identifier name:bytes length:length] autorelease];
}

-(O2PDFObjectType)objectType {
   return O2PDFObjectType_identifier;
}

-(O2PDFIdentifier)identifier {
   return _identifier;
}

-(const char *)name {
   return _bytes;
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %d %s",isa,_identifier,_bytes];
}
@end
