/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

@class NSString, NSData;

@interface NSCoder : NSObject

- (unsigned)systemVersion;

- (void)setObjectZone:(NSZone *)zone;
- (NSZone *)objectZone;

- (BOOL)allowsKeyedCoding;

- (void)encodeValueOfObjCType:(const char *)type at:(const void *)ptr;
- (void)encodeDataObject:(NSData *)data;

- (void)encodeObject:object;
- (void)encodePropertyList:propertyList;
- (void)encodeRootObject:rootObject;
- (void)encodeBycopyObject:object;
- (void)encodeByrefObject:object;

- (void)encodeConditionalObject:object;
- (void)encodeValuesOfObjCTypes:(const char *)types, ...;
- (void)encodeArrayOfObjCType:(const char *)type count:(NSUInteger)count at:(const void *)ptr;
- (void)encodeBytes:(const void *)ptr length:(NSUInteger)length;

- (void)encodePoint:(NSPoint)point;
- (void)encodeSize:(NSSize)size;
- (void)encodeRect:(NSRect)rect;

- (void)encodePoint:(NSPoint)value forKey:(NSString *)key;
- (void)encodeRect:(NSRect)value forKey:(NSString *)key;
- (void)encodeSize:(NSSize)value forKey:(NSString *)key;

- (void)encodeBool:(BOOL)value forKey:(NSString *)key;
- (void)encodeConditionalObject:object forKey:(NSString *)key;
- (void)encodeDouble:(double)value forKey:(NSString *)key;
- (void)encodeFloat:(float)value forKey:(NSString *)key;
- (void)encodeInt:(int)value forKey:(NSString *)key;
- (void)encodeObject:object forKey:(NSString *)key;

- (void)encodeInt32:(int32_t)value forKey:(NSString *)key;
- (void)encodeInt64:(int64_t)value forKey:(NSString *)key;
- (void)encodeInteger:(NSInteger)value forKey:(NSString *)key;

- (void)encodeBytes:(const uint8_t *)bytes length:(NSUInteger)length forKey:(NSString *)key;

- (void)decodeValueOfObjCType:(const char *)type at:(void *)ptr;
- (NSData *)decodeDataObject;
- (NSInteger)versionForClassName:(NSString *)className;

- decodeObject;
- decodePropertyList;
- (void)decodeValuesOfObjCTypes:(const char *)types, ...;
- (void)decodeArrayOfObjCType:(const char *)type count:(NSUInteger)count at:(void *)array;
- (void *)decodeBytesWithReturnedLength:(NSUInteger *)lengthp;

- (NSPoint)decodePoint;
- (NSSize)decodeSize;
- (NSRect)decodeRect;

- (BOOL)containsValueForKey:(NSString *)key;

- (const uint8_t *)decodeBytesForKey:(NSString *)key returnedLength:(NSUInteger *)lengthp;
- (NSPoint)decodePointForKey:(NSString *)key;
- (NSRect)decodeRectForKey:(NSString *)key;
- (NSSize)decodeSizeForKey:(NSString *)key;

- (BOOL)decodeBoolForKey:(NSString *)key;
- (double)decodeDoubleForKey:(NSString *)key;
- (float)decodeFloatForKey:(NSString *)key;
- (int)decodeIntForKey:(NSString *)key;
- decodeObjectForKey:(NSString *)key;

- (int32_t)decodeInt32ForKey:(NSString *)key;
- (int64_t)decodeInt64ForKey:(NSString *)key;
- (NSInteger)decodeIntegerForKey:(NSString *)key;

@end
