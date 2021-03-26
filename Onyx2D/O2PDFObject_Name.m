/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFObject_Name.h>
#import <Onyx2D/O2PDFContext.h>
#import <Foundation/NSString.h>
#import <string.h>

@implementation O2PDFObject_Name

-initWithBytes:(const char *)bytes length:(unsigned)length {
   _length=length;
   _bytes=NSZoneMalloc(NULL,length+1);
   strncpy(_bytes,bytes,length);
   _bytes[length]='\0';
   return self;
}

-(void)dealloc {
   NSZoneFree(NULL,_bytes);
   [super dealloc];
}

+pdfObjectWithBytes:(const char *)bytes length:(unsigned)length {
   return [[(O2PDFObject_Name *)[self alloc] initWithBytes:bytes length:length] autorelease];
}

+pdfObjectWithCString:(const char *)cString {
   return [self pdfObjectWithBytes:cString length:strlen(cString)];
}

-(const char *)name {
   return _bytes;
}

-(O2PDFObjectType)objectType {
   return kO2PDFObjectTypeName;
}

-(BOOL)checkForType:(O2PDFObjectType)type value:(void *)value {
   if(type!=kO2PDFObjectTypeName)
    return NO;
   
   *((char **)value)=_bytes;
   return YES;
}

-(void)encodeWithPDFContext:(O2PDFContext *)encoder {
   [encoder appendFormat:@"/%s ",_bytes];
}

-(NSString *)description {
   return [NSString stringWithFormat:@"/%s",_bytes];
}

@end
