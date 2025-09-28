/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFDocument.h>

#import <Onyx2D/O2PDFPage.h>
#import <Onyx2D/O2PDFxref.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFString.h>
#import <Onyx2D/O2PDFScanner.h>

#import <limits.h>

@implementation O2PDFDocument

-initWithData:(NSData *)data {   
   if(!O2PDFScanVersion([data bytes],[data length],&_version)){
    [self dealloc];
    return nil;
   }
   [_version retain];
   
   if(!O2PDFParse_xref(data,&_xref)){
    [self dealloc];
    return nil;
   }
   [_xref retain];
   
   return self;
}

-initWithDataProvider:(O2DataProvider *)provider {
   return [self initWithData:[provider data]];
}

-(void)dealloc{
   [_xref release];
   [super dealloc];
}

-(O2PDFxref *)xref {
   return _xref;
}

-(O2PDFDictionary *)trailer {
   return [_xref trailer];
}

-(O2PDFDictionary *)catalog {
   O2PDFDictionary *result;
   
   if(![[self trailer] getDictionaryForKey:"Root" value:&result])
    return nil;

   return result;
}

-(O2PDFDictionary *)infoDictionary {
   O2PDFDictionary *result;
   
   if(![[self trailer] getDictionaryForKey:"Info" value:&result])
    return nil;
    
   return result;
}

-(O2PDFDictionary *)encryptDictionary {
   O2PDFDictionary *result;
   
   if(![[self trailer] getDictionaryForKey:"Encrypt" value:&result])
    return nil;
    
   return result;
}

-(O2PDFDictionary *)pagesRoot {
   O2PDFDictionary *result;
   
   if(![[self catalog] getDictionaryForKey:"Pages" value:&result])
    return nil;
   
   return result;
}

-(int)pageCount {
   O2PDFInteger result;
   
   if(![[self pagesRoot] getIntegerForKey:"Count" value:&result])
    return 0;
    
   return result;
}

-(O2PDFPage *)pageAtNumber:(int)pageNumber pages:(O2PDFDictionary *)pages pagesOffset:(int)pagesOffset {
   O2PDFArray  *kids;
   O2PDFInteger i,kidsCount,pageCount;
   
   if(![pages getArrayForKey:"Kids" value:&kids])
    return nil;

   if(![pages getIntegerForKey:"Count" value:&pageCount])
    return nil;

   kidsCount=[kids count];
   for(i=0;i<kidsCount;i++){
    O2PDFDictionary *check;
    const char      *type;
        
    if(![kids getDictionaryAtIndex:i value:&check])
     return nil;

    if(![check getNameForKey:"Type" value:&type])
     return nil;
    
    if(strcmp(type,"Page")==0){
     if(pagesOffset==pageNumber)
      return [O2PDFPage pdfPageWithDocument:self pageNumber:pageNumber dictionary:check];
      
     pagesOffset++;
    }
    else if(strcmp(type,"Pages")==0){
     O2PDFPage *checkPage=[self pageAtNumber:pageNumber pages:check pagesOffset:pagesOffset];
     
     if(checkPage!=nil)
      return checkPage;
     else {
      O2PDFInteger checkCount;
     
      if(![check getIntegerForKey:"Count" value:&checkCount])
       return nil;
     
      pagesOffset+=checkCount;
     }
    }
    else
     return nil;
   }
   return nil;
}

-(O2PDFPage *)pageAtNumber:(int)pageNumber {
   O2PDFDictionary *pages=[self pagesRoot];
   O2PDFPage       *page=[self pageAtNumber:pageNumber-1 pages:pages pagesOffset:0];

   return page;
}

@end
