/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSMutableString_unicodePtr.h>
#import <Foundation/NSAutoreleasePool-private.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSStringFileIO.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSRaiseException.h>
#include <string.h>

@implementation NSMutableString : NSString

+allocWithZone:(NSZone *)zone {
   if(self==objc_lookUpClass("NSMutableString"))
    return NSAllocateObject(objc_lookUpClass("NSMutableString_unicodePtr"),0,zone);

   return NSAllocateObject(self,0,zone);
}

-initWithCapacity:(NSUInteger)capacity {
   NSInvalidAbstractInvocation();
   return nil;
}

-copy {
   return [[NSString allocWithZone:NULL] initWithString:self];
}

-copyWithZone:(NSZone *)zone {
   return [[NSString allocWithZone:zone] initWithString:self];
}

-(Class)classForCoder {
   return objc_lookUpClass("NSMutableString");
}

+string {
   return NSAutorelease(NSMutableString_unicodePtrNew(NULL,NULL,0));
}

+stringWithCharacters:(const unichar *)unicode length:(NSUInteger)length {
   return NSAutorelease(NSMutableString_unicodePtrNew(NULL,unicode,length));
}

+stringWithCString:(const char *)bytes length:(NSUInteger)length {
   return NSAutorelease(
     NSMutableString_unicodePtrNewWithCString(NULL,bytes,length));
}

+stringWithCString:(const char *)bytes {
   return NSAutorelease(
      NSMutableString_unicodePtrNewWithCString(NULL,bytes,strlen(bytes)));
}

+stringWithFormat:(NSString *)format,... {
   va_list   arguments;
   NSUInteger  length;
   unichar  *unicode;

   va_start(arguments,format);

   unicode=NSCharactersNewWithFormat(format,nil,arguments,&length,NULL);
   va_end(arguments);
   
   return NSAutorelease(
     NSMutableString_unicodePtrNewNoCopy(NULL,unicode,length));
}

+stringWithContentsOfFile:(NSString *)path {
   NSUInteger  length;
   unichar  *unicode;

   if((unicode=NSCharactersWithContentsOfFile(path,&length,NULL))==NULL)
    return nil;

   return NSAutorelease(
     NSMutableString_unicodePtrNewNoCopy(NULL,unicode,length));
}

+localizedStringWithFormat:(NSString *)format,... {
   va_list   arguments;
   NSUInteger  length;
   unichar  *unicode;

   va_start(arguments,format);

   unicode=NSCharactersNewWithFormat(format,nil,arguments,&length,NULL);
   va_end(arguments);
   
   return NSAutorelease(
     NSMutableString_unicodePtrNewNoCopy(NULL,unicode,length));
}

+stringWithCapacity:(NSUInteger)capacity {
   return NSAutorelease(
     NSMutableString_unicodePtrNewWithCapacity(NULL,capacity));
}

-(void)appendString:(NSString *)string {
   NSRange range={[self length],0};

   [self replaceCharactersInRange:range withString:string];
}

-(void)appendFormat:(NSString *)format,... {
   NSString *string;
   NSRange   range={[self length],0};
   va_list   arguments;

   va_start(arguments,format);

   string=NSAutorelease(NSStringNewWithFormat(format,nil,arguments,NULL));
   va_end(arguments);
   
   [self replaceCharactersInRange:range withString:string];
}

-(void)deleteCharactersInRange:(NSRange)range {
   if(NSMaxRange(range)>[self length]){
    NSRaiseException(NSRangeException,self,_cmd,@"range %@ beyond length %d",
     NSStringFromRange(range),[self length]);
   }
    
   [self replaceCharactersInRange:range withString:@""];
}

-(void)insertString:(NSString *)string atIndex:(NSUInteger)index {
   NSRange range={index,0};

   if(index>[self length]){
    NSRaiseException(NSRangeException,self,_cmd,@"index %d beyond length %d",
     index,[self length]);
   }

   [self replaceCharactersInRange:range withString:string];
}

-(void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string {
   NSInvalidAbstractInvocation();
}

-(void)setString:(NSString *)string {
   NSRange range={0,[self length]};

   [self replaceCharactersInRange:range withString:string];
}

// returns the number of replacements performed
- (NSUInteger)replaceOccurrencesOfString:(NSString *)target withString:(NSString *)replacement options:(NSStringCompareOptions)opts range:(NSRange)searchRange {
    if (target == nil) {
        NSRaiseException(NSInvalidArgumentException,self,_cmd,@"nil target object");
    }
    if (replacement == nil) {
        NSRaiseException(NSInvalidArgumentException,self,_cmd,@"nil replacement object");
    }
    if (searchRange.location+searchRange.length > [self length]){
        NSRaiseException(NSRangeException,self,_cmd,@"end of search range %d beyond length %d", searchRange.location+searchRange.length, [self length]);
    }

    const BOOL isBackwards = (opts & NSBackwardsSearch) ? YES : NO;
    const NSUInteger replacementLen = [replacement length];
    NSRange subrange;
    
    NSUInteger n = 0;
    while ((subrange = [self rangeOfString:target options:opts range:searchRange]).location != NSNotFound) {
        [self replaceCharactersInRange:subrange withString:replacement];
        
        if ( !isBackwards) {
            searchRange.length -= (subrange.location - searchRange.location) + subrange.length;
            searchRange.location = subrange.location + replacementLen;
        } else {
            searchRange.length = subrange.location - searchRange.location;
        }
        
        n++;
    }
    return n;
}

@end
