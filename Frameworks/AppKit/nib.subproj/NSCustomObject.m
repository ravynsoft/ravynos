/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - Christopher Lloyd <cjwl@objc.net>
#import "NSCustomObject.h"
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSApplication.h>
#import <AppKit/NSFontManager.h>

@implementation NSCustomObject

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _className=[[keyed decodeObjectForKey:@"NSClassName"] retain];
   }
   else 
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] does not handle %@",isa,sel_getName(_cmd),[coder class]];

   return self;
}

-(void)dealloc {
   [_className release];
   [super dealloc];
}

-(id)createCustomInstance {
   Class class=NSClassFromString(_className);
   id ret=nil;

   if(class==Nil)
    NSLog(@"NSCustomObject unknown class %@",_className);
   
   if([_className isEqualToString:@"NSApplication"]) {
      ret=[[NSApplication sharedApplication] retain];

   } else if ([_className isEqualToString: @"NSFontManager"]) {
       ret = [[NSFontManager sharedFontManager] retain];

   // Make sure we don't create some other object that should be shared already (for
   // example the NSColorPanel).
   // This is a bit fragile because if it's not created yet then this will still create
   // a duplicate. The only alternative is to specifically handle the shared objects here
   // like NSApplication and NSFontManager above... Perhaps that's more explicit and it
   // prevents the race condition - but seems quite high maintenance
   } else if (NSThreadSharedInstanceDoNotCreate(_className) != nil) {
       ret = [NSThreadSharedInstanceDoNotCreate(_className) retain];
   }
   else {
      ret=[[class alloc] init];  
   }
          
   return ret;
}

#if 0
-awakeAfterUsingCoder:(NSCoder *)coder {
   Class class=NSClassFromString(_className);
   id ret=nil;

   if(class==Nil)
    NSLog(@"NSCustomObject unknown class %@",_className);
   
   if([_className isEqualToString:@"NSApplication"]) {
      ret=[[NSApplication sharedApplication] retain];
   }
   else {
      ret=[[class alloc] init];  
   }
   [self release];
          
   return ret;
}
#endif

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@:%p:class name=%@>",isa,self,_className];
}

@end
