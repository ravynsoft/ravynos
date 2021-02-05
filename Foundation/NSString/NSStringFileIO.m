/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSStringFileIO.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSString_nextstep.h>
#import <Foundation/NSUnicodeCaseMapping.h>
#import <Foundation/NSData.h>

// FIX, inefficient
unichar *NSCharactersWithContentsOfFile(NSString *path,
  NSUInteger *length,NSZone *zone) {
   NSData *data=[NSData dataWithContentsOfFile:path];

   if(data==nil)
    return NULL;

   // guess encoding
   const unsigned char * bytes = [data bytes];
   NSUInteger dataLength=[data length];
   
   if((dataLength>=2) && ((bytes[0]==0xFE && bytes[1]==0xFF) || (bytes[0]==0xFF && bytes[1]==0xFE)))
    // UTF16 BOM
    return NSUnicodeFromBytes(bytes,dataLength,length);
   else
    // No BOM, (probably wrongly) assume NEXTSTEP encoding
    // TODO: check for UTF8, or assume UTF8 but use another one if it fails
    return NSNEXTSTEPToUnicode([data bytes],[data length],length,zone);
}
