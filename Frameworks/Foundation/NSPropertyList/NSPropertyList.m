/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSPropertyList.h>
#import <Foundation/NSRaise.h>
#import "NSPropertyListWriter_xml1.h"
#import "NSPropertyListWriter_vintage.h"
#import <Foundation/NSPropertyListReader_xml1.h>
#import <Foundation/NSPropertyListReader_vintage.h>
#import <Foundation/NSPropertyListReader_binary1.h>

@implementation NSPropertyListSerialization

+(BOOL)propertyList:propertyList isValidForFormat:(NSPropertyListFormat)format {
   NSUnimplementedMethod();
   return NO;
}

+(NSData *)dataFromPropertyList:plist format:(NSPropertyListFormat)format errorDescription:(NSString **)errorDescriptionp {
   switch(format){
   
    case NSPropertyListOpenStepFormat:
		return [NSPropertyListWriter_vintage dataWithPropertyList:plist];

     
    case NSPropertyListXMLFormat_v1_0:
     return [NSPropertyListWriter_xml1 dataWithPropertyList:plist];
     
    case NSPropertyListBinaryFormat_v1_0:
     return nil;
   }
   return nil;
}

+propertyListFromData:(NSData *)data mutabilityOption:(NSPropertyListMutabilityOptions)mutability format:(NSPropertyListFormat *)format errorDescription:(NSString **)errorDescriptionp {
   id result;
	
	@try{

   if((result=[NSPropertyListReader_xml1 propertyListFromData:data])!=nil){
    if(format)*format=NSPropertyListXMLFormat_v1_0;
    return result;
   }
   
   if((result=[NSPropertyListReader_binary1 propertyListFromData:data])!=nil){
    if(format)*format=NSPropertyListBinaryFormat_v1_0;
    return result;
   }
   
   if((result=[NSPropertyListReader_vintage propertyListFromData:data])!=nil){
    if(format)*format=NSPropertyListOpenStepFormat;
    return result;
   }
}
	@catch (NSException* e) {
		return nil;	
	}
	
   
   return nil;
}

@end
