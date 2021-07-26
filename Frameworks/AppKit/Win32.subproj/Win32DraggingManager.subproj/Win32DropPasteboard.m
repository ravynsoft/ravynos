/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import <AppKit/Win32DropPasteboard.h>

@implementation Win32DropPasteboard

-initWithIDataObject:(IDataObject *)dataObject {
   _dataClient=[[Win32IDataObjectClient alloc] initWithIDataObject:dataObject];
   return self;
}

-(void)dealloc {
   [_dataClient release];
   [super dealloc];
}

-(NSArray *)types {
   return [_dataClient availableTypes];
}

-(NSData *)dataForType:(NSString *)type {
   if([type isEqualToString:NSFilenamesPboardType]) {
    NSString *error=nil;
    NSData *result=[NSPropertyListSerialization dataFromPropertyList:[_dataClient filenames] format:NSPropertyListXMLFormat_v1_0 errorDescription:&error];
    if(error) {
     NSLog(@"Error: %@", error);
     [error release];
    }
    return result;
   } else
    return [_dataClient dataForType:type];
}

-(id)propertyListForType:(NSString *)type {
   if([type isEqualToString:NSFilenamesPboardType])
    return [_dataClient filenames];
   else
    return [super propertyListForType:type];
}

@end
