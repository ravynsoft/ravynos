/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFContentStream.h>
#import <Onyx2D/O2PDFPage.h>
#import <Onyx2D/O2PDFArray.h>
#import <Onyx2D/O2PDFDictionary.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>

@implementation O2PDFContentStream

-initWithStreams:(NSArray *)streams resources:(O2PDFDictionary *)resources parent:(O2PDFContentStream *)parent {
   _streams=[streams retain];
   _resources=[resources retain];
   _parent=[parent retain];
   return self;
}

-initWithPage:(O2PDFPage *)page {
   O2PDFDictionary *dictionary=[page dictionary];
   O2PDFDictionary *resources;
   O2PDFObject     *contents;
   NSMutableArray  *streams=[NSMutableArray array];
   
   if(![dictionary getDictionaryForKey:"Resources" value:&resources])
    resources=nil;
   if(![dictionary getObjectForKey:"Contents" value:&contents])
    contents=nil;
   else if([contents objectType]==kO2PDFObjectTypeArray){
    O2PDFArray *array=(O2PDFArray *)contents;
    int         i,count=[array count];
    
    for(i=0;i<count;i++)
     [streams addObject:[array objectAtIndex:i]];
   }
   else if([contents objectType]==kO2PDFObjectTypeStream)
    [streams addObject:contents];
   else {
    NSLog(@"contents is not an array or stream, got %@",contents);
   }
   
   return [self initWithStreams:streams resources:resources parent:nil];
}

-initWithStream:(O2PDFStream *)stream resources:(O2PDFDictionary *)resources parent:(O2PDFContentStream *)parent {
   NSArray *array=[NSArray arrayWithObject:stream];
      
   return [self initWithStreams:array resources:resources parent:parent];
}

-(void)dealloc {
   [_streams release];
   [_resources release];
   [_parent release];
   [super dealloc];
}

-(NSArray *)streams {
   return _streams;
}

-(id)resourceForCategory:(const char *)category name:(const char *)name {
   O2PDFObject     *result;
   O2PDFDictionary *sub;
   
   if([_resources getDictionaryForKey:category value:&sub])
    if([sub getObjectForKey:name value:&result])
     return result;
   
   return [_parent resourceForCategory:category name:name];
}

-(void)replaceResource:(O2PDFObject *)object forCategory:(const char *)category name:(const char *)name withObject:(id)replacement {
   O2PDFObject     *result;
   O2PDFDictionary *sub;
   
   if([_resources getDictionaryForKey:category value:&sub])
    if([sub getObjectForKey:name value:&result]){
     if(result==object)
      [sub setObjectForKey:name value:replacement];
    }
   
   return [_parent replaceResource:object forCategory:category name:name withObject:replacement];
}


@end
