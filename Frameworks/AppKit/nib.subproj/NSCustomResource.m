/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import "NSCustomResource.h"
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSImage.h>

@implementation NSCustomResource

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _className=[[keyed decodeObjectForKey:@"NSClassName"] retain];
    _resourceName=[[keyed decodeObjectForKey:@"NSResourceName"] retain];
   }
   else
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] can not decode from a %@",[self class],sel_getName(_cmd),[coder class]];
    
   return self;
}

-(void)dealloc {
   [_className release];
   [_resourceName release];
   [super dealloc];
}

-awakeAfterUsingCoder:(NSCoder *)coder {
   if([_className isEqualToString:@"NSImage"]){
    NSImage *image=[NSImage imageNamed:_resourceName];
    
    if([_resourceName hasSuffix:@"Template"])
     [image setTemplate:YES];
     
    if(image!=nil){
        [self release];
        return [image retain];
    }
   }

   NSLog(@"Could not find image named '%@'", _resourceName);
   
   return [[NSImage alloc] init];
}

@end
