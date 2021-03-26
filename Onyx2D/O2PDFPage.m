/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFPage.h>
#import <Onyx2D/O2PDFContentStream.h>
#import <Onyx2D/O2PDFOperatorTable.h>
#import <Onyx2D/O2PDFScanner.h>
#import <Onyx2D/O2PDFDocument.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFArray.h>

@implementation O2PDFPage

-initWithDocument:(O2PDFDocument *)document pageNumber:(int)pageNumber dictionary:(O2PDFDictionary *)dictionary {
   _document=[document retain];
   _pageNumber=pageNumber;
   _dictionary=[dictionary retain];
   return self;
}

-(void)dealloc {
   [_document release];
   [_dictionary release];
   [super dealloc];
}

+(O2PDFPage *)pdfPageWithDocument:(O2PDFDocument *)document pageNumber:(int)pageNumber dictionary:(O2PDFDictionary *)dictionary {
   return [[[self alloc] initWithDocument:document pageNumber:pageNumber dictionary:dictionary] autorelease];
}

-(O2PDFDocument *)document {
   return _document;
}

-(int)pageNumber {
   return _pageNumber;
}

-(O2PDFDictionary *)dictionary {
   return _dictionary;
}

BOOL O2PDFGetPageObjectForKey(O2PDFPage *page,const char *key,O2PDFObject **object){
   O2PDFDictionary *dictionary=[page dictionary];
   
   do{
    O2PDFObject *check;
    
    if([dictionary getObjectForKey:key value:&check]){
     *object=check;
     return YES;
    }
    
   }while([dictionary getDictionaryForKey:"Parent" value:&dictionary]);
   
   return NO;
}

BOOL O2PDFGetPageArrayForKey(O2PDFPage *page,const char *key,O2PDFArray **arrayp){
   O2PDFObject *check;
   
   if(!O2PDFGetPageObjectForKey(page,key,&check))
    return NO;
    
   return [check checkForType:kO2PDFObjectTypeArray value:arrayp];
}

-(BOOL)getRect:(O2Rect *)rect forBox:(O2PDFBox)box {
   const char *string=NULL;
   O2PDFArray *array;
   O2PDFReal  *numbers;
   unsigned    count;
   
   switch(box){
    case kO2PDFMediaBox: string="MediaBox"; break;
    case kO2PDFCropBox:  string="CropBox"; break;
    case kO2PDFBleedBox: string="BleedBox"; break;
    case kO2PDFTrimBox:  string="TrimBox"; break;
    case kO2PDFArtBox:   string="ArtBox"; break;
   }
   
   if(string==NULL)
    return NO;
   if(!O2PDFGetPageArrayForKey(self,string,&array))
    return NO;
   
   if(![array getNumbers:&numbers count:&count])
    return NO;
    
   if(count!=4){
    NSZoneFree(NULL,numbers);
    return NO;
   }
   
   rect->origin.x=numbers[0];
   rect->origin.y=numbers[1];
   rect->size.width=numbers[2]-numbers[0];
   rect->size.height=numbers[3]-numbers[1];
   
   NSZoneFree(NULL,numbers);
   
   return YES;
}

-(int)rotationAngle {
   return 0;
}


O2AffineTransform O2PDFPageGetDrawingTransform(O2PDFPageRef self,O2PDFBox box,O2Rect rect,int clockwiseDegrees,bool preserveAspectRatio) {
   O2AffineTransform result=O2AffineTransformIdentity;
   O2Rect boxRect;
   
   if([self getRect:&boxRect forBox:box]){   
    result=O2AffineTransformTranslate(result,-boxRect.origin.x,-boxRect.origin.y);
    result=O2AffineTransformTranslate(result,rect.origin.x,rect.origin.y);
    result=O2AffineTransformScale(result,rect.size.width/boxRect.size.width,rect.size.height/boxRect.size.height);
   }

   return result;
}

-(void)drawInContext:(O2Context *)context {
   O2PDFContentStream *contentStream=[[[O2PDFContentStream alloc] initWithPage:self] autorelease];
   O2PDFOperatorTable *operatorTable=[O2PDFOperatorTable renderingOperatorTable];
   O2PDFScanner       *scanner=[[[O2PDFScanner alloc] initWithContentStream:contentStream operatorTable:operatorTable info:context] autorelease];

   [scanner scan];
}

@end
