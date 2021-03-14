/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32GeneralPasteboard.h>
#import <AppKit/Win32IDataObjectClient.h>
#import <AppKit/Win32IDataObjectServer.h>

@implementation Win32GeneralPasteboard

-(int)declareTypes:(NSArray *)types owner:(id)owner {
    int                     result=[super declareTypes:types owner:owner];
    Win32IDataObjectServer *dataServer=[[[Win32IDataObjectServer alloc] initWithPasteboard:self] autorelease];
    
    if(![dataServer setOnClipboard]){
        NSLog(@"unable to set on clipboard");
        [dataServer release];
    }
    
    return result;
}

-(int)addTypes:(NSArray *)types owner:(id)owner {
    int                     result=[super addTypes:types owner:owner];
    Win32IDataObjectServer *dataServer=[[[Win32IDataObjectServer alloc] initWithPasteboard:self] autorelease];
    
    if(![dataServer setOnClipboard]){
        NSLog(@"unable to set on clipboard");
        [dataServer release];
    }
    
    return result;
}

-(NSArray *)types {
   if([self isClient]){
    Win32IDataObjectClient *dataClient=[[[Win32IDataObjectClient alloc] initWithClipboard] autorelease];

    return [dataClient availableTypes];
   }
   else
    return [super types];
}

-(BOOL)setData:(NSData *)data forType:(NSString *)type {
   if([self isClient]){
    return NO;
   }
   else {
    return [super setData:data forType:type];
   }
}

-(NSData *)dataForType:(NSString *)type {
   if([self isClient]){
    Win32IDataObjectClient *dataClient=[[[Win32IDataObjectClient alloc] initWithClipboard] autorelease];

    return [dataClient dataForType:type];
   }
   else {
    return [super dataForType:type];
   }
}

-(id)propertyListForType:(NSString *)type {
   if([self isClient]){
    if([type isEqualToString:NSFilenamesPboardType]){
     Win32IDataObjectClient *dataClient=[[[Win32IDataObjectClient alloc] initWithClipboard] autorelease];

     return [dataClient filenames];
    }
    else
     return [super propertyListForType:type];
   }
   else {
    return [super propertyListForType:type];
   }
}

@end
