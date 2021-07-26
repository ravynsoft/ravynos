/* Copyright (c) 2008 John Engelhart

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

#ifdef NS_BLOCK_ASSERTIONS
#define _NSAssertBody(condition, desc, ...)
#define _NSCAssertBody(condition, desc, ...)
#else // NS_BLOCK_ASSERTIONS not defined
#define _NSAssertBody(condition, desc, ...)                                                                                                                                                   \
    do {                                                                                                                                                                                      \
        if(!(condition)) {                                                                                                                                                                    \
            [[NSAssertionHandler currentHandler] handleFailureInMethod:_cmd object:self file:[NSString stringWithUTF8String:__FILE__] lineNumber:__LINE__ description:(desc), ##__VA_ARGS__]; \
        }                                                                                                                                                                                     \
    } while(0)
#define _NSCAssertBody(condition, desc, ...)                                                                                                                                                                                       \
    do {                                                                                                                                                                                                                           \
        if(!(condition)) {                                                                                                                                                                                                         \
            [[NSAssertionHandler currentHandler] handleFailureInFunction:[NSString stringWithUTF8String:__PRETTY_FUNCTION__] file:[NSString stringWithUTF8String:__FILE__] lineNumber:__LINE__ description:(desc), ##__VA_ARGS__]; \
        }                                                                                                                                                                                                                          \
    } while(0)
#endif // NS_BLOCK_ASSERTIONS

/*
 * Asserts to use in Objective-C methods:
 */

#define NSAssert(condition, desc, ...) _NSAssertBody((condition), (desc), ##__VA_ARGS__)
#define NSAssert1(condition, desc, val1) NSAssert(condition, desc, val1)
#define NSAssert2(condition, desc, val1, val2) NSAssert(condition, desc, val1, val2)
#define NSAssert3(condition, desc, val1, val2, val3) NSAssert(condition, desc, val1, val2, val3)
#define NSAssert4(condition, desc, val1, val2, val3, val4) NSAssert(condition, desc, val1, val2, val3, val4)
#define NSAssert5(condition, desc, val1, val2, val3, val4, val5) NSAssert(condition, desc, val1, val2, val3, val4, val5)

#define NSParameterAssert(condition) _NSAssertBody((condition), @"Invalid parameter not satisfying: %s", #condition)

/*
 * Asserts to use in C function calls:
 */

#define NSCAssert(condition, desc, ...) _NSCAssertBody((condition), (desc), ##__VA_ARGS__)
#define NSCAssert1(condition, desc, val1) NSCAssert(condition, desc, val1)
#define NSCAssert2(condition, desc, val1, val2) NSCAssert(condition, desc, val1, val2)
#define NSCAssert3(condition, desc, val1, val2, val3) NSCAssert(condition, desc, val1, val2, val3)
#define NSCAssert4(condition, desc, val1, val2, val3, val4) NSCAssert(condition, desc, val1, val2, val3, val4)
#define NSCAssert5(condition, desc, val1, val2, val3, val4, val5) NSCAssert(condition, desc, val1, val2, val3, val4, val5)

#define NSCParameterAssert(condition) _NSCAssertBody((condition), @"Invalid parameter not satisfying: %s", #condition)

@interface NSAssertionHandler : NSObject

+ (NSAssertionHandler *)currentHandler;
- (void)handleFailureInMethod:(SEL)selector object:(id)object file:(NSString *)fileName lineNumber:(NSInteger)line description:(NSString *)format, ...;
- (void)handleFailureInFunction:(NSString *)functionName file:(NSString *)fileName lineNumber:(NSInteger)line description:(NSString *)format, ...;

@end
