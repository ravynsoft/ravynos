/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSCoder.h>
#import <Foundation/NSMapTable.h>

@class NSMutableArray;

@interface NSUnarchiver : NSCoder {
    NSData *_data;
    const uint8_t *_bytes;
    NSUInteger _position, _length;
    NSZone *_objectZone;

    uint32_t _version;
    NSMapTable *_objects;
    NSMapTable *_classes;
    NSMapTable *_cStrings;
    NSMapTable *_classVersions;
    NSMutableArray *_allObjects;
}

+ unarchiveObjectWithData:(NSData *)data;
+ unarchiveObjectWithFile:(NSString *)path;

+ (void)decodeClassName:(NSString *)archiveName asClassName:(NSString *)runtimeName;
+ (NSString *)classNameDecodedForArchiveClassName:(NSString *)className;

- (void)decodeValueOfObjCType:(const char *)type at:(void *)data;
- (NSData *)decodeDataObject;
- (NSInteger)versionForClassName:(NSString *)className;

- initForReadingWithData:(NSData *)data;

- (BOOL)isAtEnd;
- (NSZone *)objectZone;
- (void)setObjectZone:(NSZone *)zone;

- (void)decodeClassName:(NSString *)archiveName asClassName:(NSString *)runtimeName;
- (NSString *)classNameDecodedForArchiveClassName:(NSString *)className;

- (void)replaceObject:original withObject:replacement;

@end
