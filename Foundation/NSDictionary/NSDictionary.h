/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSEnumerator.h>

@class NSArray, NSURL;

@interface NSDictionary : NSObject <NSCoding, NSCopying, NSMutableCopying, NSFastEnumeration>

- initWithObjects:(id *)objects forKeys:(id *)keys count:(NSUInteger)count;
- initWithObjects:(NSArray *)objects forKeys:(NSArray *)keys;
- initWithDictionary:(NSDictionary *)dictionary;
- initWithDictionary:(NSDictionary *)dictionary copyItems:(BOOL)copyItems;
- initWithObjectsAndKeys:object, ...;
- initWithContentsOfFile:(NSString *)path;
- initWithContentsOfURL:(NSURL *)url;

+ dictionary;
+ dictionaryWithObjects:(id *)objects forKeys:(id *)keys count:(NSUInteger)count;
+ dictionaryWithObjects:(NSArray *)objects forKeys:(NSArray *)keys;
+ dictionaryWithDictionary:(NSDictionary *)other;
+ dictionaryWithObjectsAndKeys:first, ...;
+ dictionaryWithObject:object forKey:key;
+ dictionaryWithContentsOfFile:(NSString *)path;
+ dictionaryWithContentsOfURL:(NSURL *)url;

- objectForKey:key;
- (NSUInteger)count;
- (NSEnumerator *)keyEnumerator;
- (NSEnumerator *)objectEnumerator;

- (void)getObjects:(id *)objects andKeys:(id *)keys;

- (BOOL)isEqualToDictionary:(NSDictionary *)dictionary;

- (NSArray *)allKeys;
- (NSArray *)allKeysForObject:object;
- (NSArray *)keysSortedByValueUsingSelector:(SEL)selector;

- (NSArray *)allValues;
- (NSArray *)objectsForKeys:(NSArray *)keys notFoundMarker:marker;

- (BOOL)writeToFile:(NSString *)path atomically:(BOOL)atomically;
- (BOOL)writeToURL:(NSURL *)url atomically:(BOOL)atomically;

- (NSString *)description;
- (NSString *)descriptionInStringsFileFormat;
- (NSString *)descriptionWithLocale:locale;
- (NSString *)descriptionWithLocale:locale indent:(NSUInteger)indent;

@end

#import <Foundation/NSMutableDictionary.h>
