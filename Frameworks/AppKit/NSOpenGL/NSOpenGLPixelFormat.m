/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSOpenGLPixelFormat.h>
#import <AppKit/NSRaise.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSData.h>
#import <OpenGL/OpenGL.h>

@implementation NSOpenGLPixelFormat

-initWithAttributes:(NSOpenGLPixelFormatAttribute *)attributes {
   CGLChoosePixelFormat(attributes,(CGLPixelFormatObj *)&_cglPixelFormat,&_numberOfVirtualScreens);
   return self;
}

-(void)dealloc {
   CGLReleasePixelFormat(_cglPixelFormat);
   [super dealloc];
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSData                      *data=[coder decodeObjectForKey:@"NSPixelAttributes"];
    unsigned                     i,length=[data length];
    const unsigned char         *bytes=[data bytes];
    NSOpenGLPixelFormatAttribute attributes[length/4];
    unsigned                     a=0;
    
    if(length%4>0){
     NSLog(@"NSPixelAttributes is not a multiple of 4, length=%d",length);
     return nil;
    }
    
    for(i=0;i<length;){
     unsigned value;
     
     value=bytes[i++];
     value<<=8;
     value|=bytes[i++];
     value<<=8;
     value|=bytes[i++];
     value<<=8;
     value|=bytes[i++];
     attributes[a++]=value;
    }
     
    return [self initWithAttributes:attributes];
   }
   else
    NSUnimplementedMethod();

   return self;
}

-(void *)CGLPixelFormatObj {
   return _cglPixelFormat;
}

-(GLint)numberOfVirtualScreens {
   return _numberOfVirtualScreens;
}

-(void)getValues:(long *)values forAttribute:(NSOpenGLPixelFormatAttribute)attribute forVirtualScreen:(int)screen {
   GLint glValue=0;

   CGLDescribePixelFormat(_cglPixelFormat,screen,attribute,&glValue);

   *values=glValue;
}

@end
