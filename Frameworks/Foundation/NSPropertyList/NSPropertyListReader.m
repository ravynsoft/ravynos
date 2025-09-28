/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#include <stdio.h>

#import <Foundation/NSPropertyListReader.h>
#import <Foundation/NSPropertyListReader_xml1.h>
#import "NSPropertyListReader_binary1.h"
#import <Foundation/NSPropertyListReader_vintage.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSArray.h>

@implementation NSPropertyListReader

+propertyListFromData:(NSData *)data {
   id result = nil;
    
   if(data==nil)
    return nil;

    @try {
        result=[NSPropertyListReader_binary1 propertyListFromData:data];
        if(result==nil)
            result=[NSPropertyListReader_xml1 propertyListFromData:data];
        if(result==nil)
            result=[NSPropertyListReader_vintage propertyListFromData:data];
        
    }
    @catch (NSException *exception) {
        // Don't use NSLog here as we might be called from some early NSLog, when formating the timestamp...
        fprintf(stderr, "propertyListFromData: error while decoding plist content : %s\n", [[exception description] UTF8String]);
        result = nil;
    }
    @finally {
    }
   return result;
}

+propertyListFromString:(NSString *)string {
// FIX
   NSData *data=[string dataUsingEncoding:NSNEXTSTEPStringEncoding];

   return [self propertyListFromData:data];
}

+(NSObject *)propertyListWithContentsOfFile:(NSString *)path {
   NSData *data=[NSData dataWithContentsOfFile:path];

   return [self propertyListFromData:data];
}

+(NSDictionary *)dictionaryWithContentsOfFile:(NSString *)path {
   NSObject *result=[self propertyListWithContentsOfFile:path];

   if([result isKindOfClass:[NSDictionary class]])
    return (NSDictionary *)result;

   return nil;
}

+(NSArray *)arrayWithContentsOfFile:(NSString *)path {
   NSObject *result=[self propertyListWithContentsOfFile:path];

   if([result isKindOfClass:[NSArray class]])
    return (NSArray *)result;

   return nil;
}

@end
