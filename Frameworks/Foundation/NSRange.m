/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/NSRange.h>
#import <Foundation/NSStringFormatter.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSCharacterSet.h>

NSRange NSMakeRange(NSUInteger location,NSUInteger length) {
   NSRange range={location,length};
   return range;
}

BOOL NSEqualRanges(NSRange range, NSRange otherRange) {
   return (range.location==otherRange.location && range.length==otherRange.length);
}

NSUInteger NSMaxRange(NSRange range){
   return range.location+range.length;
}

NSString *NSStringFromRange(NSRange range){
   return NSStringWithFormat(@"{%u, %u}",range.location,range.length);
}

NSRange NSRangeFromString(NSString * s) 
{ 
        NSRange result = NSMakeRange(0,0); 
        NSScanner * scanner = [NSScanner scannerWithString: s]; 
        NSCharacterSet * digitSet = [NSCharacterSet 
decimalDigitCharacterSet]; 

        [scanner scanUpToCharactersFromSet: digitSet intoString: (id *) nil]; 
        if(![scanner isAtEnd]) 
                { 
                [scanner scanInt: (int *) &result.location]; 
                [scanner scanUpToCharactersFromSet: digitSet intoString: (id *) 
nil]; 
                if(![scanner isAtEnd]) 
                        { 
                        [scanner scanInt: (int *)&result.length]; 
                        } 
                } 
        return result; 

} 

BOOL NSLocationInRange(NSUInteger location,NSRange range){
   return (location>=range.location && location<NSMaxRange(range))?YES:NO;
}

NSRange NSIntersectionRange(NSRange range,NSRange otherRange){
   NSUInteger min,loc,max1=NSMaxRange(range),max2=NSMaxRange(otherRange);
   NSRange result;

   min=(max1<max2)?max1:max2;
   loc=(range.location>otherRange.location)?range.location:otherRange.location;

   if(min<loc)
    result.location=result.length=0;
   else{
    result.location=loc;
    result.length=min-loc;
   }

   return result;
}

NSRange NSUnionRange(NSRange range,NSRange otherRange){
   NSUInteger max,loc,max1=NSMaxRange(range),max2=NSMaxRange(otherRange);
   NSRange result;

   max=(max1>max2)?max1:max2;
   loc=(range.location<otherRange.location)?range.location:otherRange.location;

   result.location=loc;
   result.length=max-result.location;
   return result;
}

