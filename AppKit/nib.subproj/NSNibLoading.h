/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSBundle.h>

// clang has these as builtins, guard against that
#ifndef IBOutlet
#if 0
#define NIBDEBUG(desc, ...)                                                                   \
    do {                                                                                      \
        NSString *location = [NSString stringWithFormat:@"%s %ld", __FILE__, (long)__LINE__]; \
        NSString *msg = [NSString stringWithFormat:desc, ##__VA_ARGS__];                      \
        NSLog(@"%@: %@", location, msg);                                                      \
    } while(0)
#else
#define NIBDEBUG(desc, ...)
#endif
#define IBOutlet
#endif
#define NIBDEBUG(desc, ...)

#ifndef IBAction
#define IBAction void
#endif

@class NSString, NSDictionary;

@interface NSObject (NSNibLoading)
- (void)awakeFromNib;
@end

@interface NSBundle (NSNibLoading)

+ (BOOL)loadNibFile:(NSString *)path externalNameTable:(NSDictionary *)nameTable withZone:(NSZone *)zone;

+ (BOOL)loadNibNamed:(NSString *)name owner:owner;

- (BOOL)loadNibFile:(NSString *)fileName externalNameTable:(NSDictionary *)nameTable withZone:(NSZone *)zone;

@end
