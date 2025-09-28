/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/AppKit.h>

@class NSColor;

APPKIT_EXPORT NSString *const NSColorListDidChangeNotification;

@interface NSColorList : NSObject {
    NSMutableArray *_keys;
    NSMutableArray *_colors;
    NSString *_name;
    NSString *_path;
    BOOL _isEditable;
}

+ (NSArray *)availableColorLists;

- initWithName:(NSString *)name fromFile:(NSString *)path;
- initWithName:(NSString *)name;

+ (NSColorList *)colorListNamed:(NSString *)name;

- (BOOL)isEditable;
- (BOOL)editable;
- (NSString *)name;
- (NSArray *)allKeys;
- (NSColor *)colorWithKey:(NSString *)key;

- (void)setColor:(NSColor *)color forKey:(NSString *)key;
- (void)removeColorWithKey:(NSString *)key;
- (void)insertColor:(NSColor *)color key:(NSString *)key atIndex:(unsigned)index;

- (BOOL)writeToFile:(NSString *)path;
- (void)removeFile;

@end
