/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSRange.h>
#import <Foundation/NSEnumerator.h>

@class NSString, NSDictionary, NSPredicate, NSIndexSet, NSURL;

@interface NSArray : NSObject <NSCopying, NSMutableCopying, NSCoding, NSFastEnumeration>

- initWithArray:(NSArray *)array;
- initWithArray:(NSArray *)array copyItems:(BOOL)copyItems;
- initWithContentsOfFile:(NSString *)path;
- initWithContentsOfURL:(NSURL *)url;
- initWithObjects:(id *)objects count:(NSUInteger)count;
- initWithObjects:object, ...;

+ array;
+ arrayWithContentsOfFile:(NSString *)path;
+ arrayWithContentsOfURL:(NSURL *)url;
+ arrayWithObject:object;
+ arrayWithObjects:object, ...;

+ arrayWithArray:(NSArray *)array;
+ arrayWithObjects:(id *)objects count:(NSUInteger)count;

- (NSUInteger)count;
- objectAtIndex:(NSUInteger)index;

- (void)getObjects:(id *)objects;
- (void)getObjects:(id *)objects range:(NSRange)range;

- (NSArray *)objectsAtIndexes:(NSIndexSet *)indexes;

- (NSArray *)subarrayWithRange:(NSRange)range;

- (BOOL)isEqualToArray:(NSArray *)array;

- (NSUInteger)indexOfObject:object;
- (NSUInteger)indexOfObject:object inRange:(NSRange)range;

- (NSUInteger)indexOfObjectIdenticalTo:object;
- (NSUInteger)indexOfObjectIdenticalTo:object inRange:(NSRange)range;

- (NSEnumerator *)objectEnumerator;
- (NSEnumerator *)reverseObjectEnumerator;

- (NSArray *)arrayByAddingObject:object;
- (NSArray *)arrayByAddingObjectsFromArray:(NSArray *)array;
- (NSString *)componentsJoinedByString:(NSString *)string;

- (BOOL)containsObject:object;

- firstObjectCommonWithArray:(NSArray *)array;

- firstObject;
- lastObject;

- (NSArray *)sortedArrayUsingSelector:(SEL)selector;
- (NSArray *)sortedArrayUsingFunction:(NSInteger (*)(id, id, void *))function
                              context:(void *)context;

- (BOOL)writeToFile:(NSString *)path atomically:(BOOL)atomically;
- (BOOL)writeToURL:(NSURL *)aURL atomically:(BOOL)atomically;

- (void)makeObjectsPerformSelector:(SEL)selector;
- (void)makeObjectsPerformSelector:(SEL)selector withObject:object;

- (NSString *)descriptionWithLocale:(NSDictionary *)locale;
- (NSString *)descriptionWithLocale:(NSDictionary *)locale indent:(NSUInteger)indent;

- (NSArray *)sortedArrayUsingDescriptors:(NSArray *)descriptors;
- (NSArray *)filteredArrayUsingPredicate:(NSPredicate *)predicate;
@end

#import <Foundation/NSMutableArray.h>
