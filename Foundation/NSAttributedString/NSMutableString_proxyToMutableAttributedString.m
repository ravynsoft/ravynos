/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSMutableString_proxyToMutableAttributedString.h>

@implementation NSMutableString_proxyToMutableAttributedString

-initWithMutableAttributedString:(NSMutableAttributedString *)attributedString {
   _attributedString=[attributedString retain];
   return self;
}

-(void)dealloc {
   [_attributedString release];
   [super dealloc];
}

-(NSUInteger)length {
   return [_attributedString length];
}

-(unichar)characterAtIndex:(NSUInteger)location {
   return [[_attributedString string] characterAtIndex:location];
}

-(void)replaceCharactersInRange:(NSRange)range withString:(NSString *)string {
   [_attributedString replaceCharactersInRange:range withString:string];
}

@end
