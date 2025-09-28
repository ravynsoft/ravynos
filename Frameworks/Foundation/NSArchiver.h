/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSCoder.h>
#import <Foundation/NSHashTable.h>

@class NSMutableData, NSMutableDictionary;

@interface NSArchiver : NSCoder {
    NSMutableData *_data;
    uint8_t *_bytes;
    NSUInteger _position;

    NSUInteger _pass;
    NSHashTable *_conditionals;
    NSHashTable *_objects;
    NSHashTable *_classes;
    NSHashTable *_cStrings;
}

- init;

+ (NSData *)archivedDataWithRootObject:rootObject;
+ (BOOL)archiveRootObject:rootObject toFile:(NSString *)path;

- (NSMutableData *)archiverData;

- (void)encodeValueOfObjCType:(const char *)type at:(const void *)addr;
- (void)encodeDataObject:(NSData *)data;

- (void)encodeRootObject:rootObject;
- (void)encodeConditionalObject:object;

- (void)encodeClassName:(NSString *)runtime intoClassName:(NSString *)archive;

- (void)replaceObject:original withObject:replacement;

@end

@interface NSObject (NSArchiver)
- replacementObjectForArchiver:(NSArchiver *)archiver;
@end

#import <Foundation/NSUnarchiver.h>
