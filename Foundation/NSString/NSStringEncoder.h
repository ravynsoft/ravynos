/* Copyright (c) 2013 Aiy André - plasq

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSString.h>

// returns a buffer with a Unicode representation of string made from the bytes using the given encoding
// the caller is responsible to free the returned buffer when done
// *resultLength will contain the number of unichar of the result
unichar *NSBytesToUnicode(const unsigned char *bytes, NSUInteger length, NSStringEncoding encoding, NSUInteger *resultLength, NSZone *zone);

// returns a buffer with a char representation of string made from the unichar using the given encoding
// the caller is responsible to free the returned buffer when done
// *resultLength will contain the number of unichar of the result
unsigned char *NSBytesFromUnicode(const unichar *characters, NSUInteger length, NSStringEncoding encoding, BOOL lossy, NSUInteger *resultLength, NSZone *zone);
