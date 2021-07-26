/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSDecimal.h>

@class NSDictionary, NSCharacterSet;

@interface NSScanner : NSObject <NSCopying>

+ scannerWithString:(NSString *)string;
+ localizedScannerWithString:(NSString *)string;

- initWithString:(NSString *)string;

- (NSString *)string;

- (NSCharacterSet *)charactersToBeSkipped;
- (BOOL)caseSensitive;
- (NSDictionary *)locale;

- (void)setCharactersToBeSkipped:(NSCharacterSet *)set;
- (void)setCaseSensitive:(BOOL)flag;
- (void)setLocale:(NSDictionary *)locale;

- (BOOL)isAtEnd;
- (NSUInteger)scanLocation;
- (void)setScanLocation:(NSUInteger)location;

- (BOOL)scanInt:(int *)valuep;
- (BOOL)scanInteger:(NSInteger *)valuep;
- (BOOL)scanLongLong:(long long *)valuep;
- (BOOL)scanFloat:(float *)valuep;
- (BOOL)scanDouble:(double *)valuep;
- (BOOL)scanDecimal:(NSDecimal *)valuep;
- (BOOL)scanHexInt:(unsigned *)valuep;
- (BOOL)scanHexLongLong:(unsigned long long *)valuep;

- (BOOL)scanString:(NSString *)string intoString:(NSString **)stringp;
- (BOOL)scanUpToString:(NSString *)string intoString:(NSString **)stringp;

- (BOOL)scanCharactersFromSet:(NSCharacterSet *)charset intoString:(NSString **)stringp;
- (BOOL)scanUpToCharactersFromSet:(NSCharacterSet *)charset intoString:(NSString **)stringp;

@end
