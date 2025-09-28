/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSEPSImageRep.h>
#import <Foundation/NSData.h>
#import <Foundation/NSArray.h>
#import <AppKit/NSRaise.h>

@implementation NSEPSImageRep

+(NSArray *)imageRepsWithData:(NSData *)data {
   id result=[self imageRepWithData:data];
   
   if(result==nil)
    return nil;
  
   return [NSArray arrayWithObject:result];
}

+imageRepWithData:(NSData *)data {
   return [[[self alloc] initWithData:data] autorelease];
}

-initWithData:(NSData *)data {
   _data=[data retain];
   return self;
}

-(void)dealloc {
   [_data release];
   [super dealloc];
}

-(NSData *)EPSRepresentation {
   return _data;
}

-(NSRect)boundingBox {
   NSUnimplementedMethod();
   return NSMakeRect(0,0,0,0);
}

-(void)prepareGState {
   // do nothing
}

-(BOOL)draw {
   [self prepareGState];
   NSUnimplementedMethod();
   return NO;
}

@end
