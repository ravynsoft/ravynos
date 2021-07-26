/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// *** FOR INTERNAL COCOTRON USE ONLY

#import <Foundation/NSException.h>
#import <Foundation/NSString.h>

static inline void _NSInvalidAbstractInvocation(SEL selector, id object, const char *file, int line) {
    [NSException raise:NSInvalidArgumentException
                format:@"-%s only defined for abstract class. Define -[%@ %s] in %s:%d!",
                sel_getName(selector), [object class], sel_getName(selector), file, line];
}

static inline void _NSUnimplementedMethod(SEL selector, id object, const char *file, int line) {
    NSLog(@"-[%@ %s] unimplemented in %s at %d", [object class], sel_getName(selector), file, line);
}

static inline void _NSUnimplementedFunction(const char *function, const char *file, int line) {
    NSLog(@"%s() unimplemented in %s at %d", function, file, line);
}

#define NSInvalidAbstractInvocation() \
    _NSInvalidAbstractInvocation(_cmd, self, __FILE__, __LINE__)

#define NSUnimplementedMethod() \
    _NSUnimplementedMethod(_cmd, self, __FILE__, __LINE__)

#define NSUnimplementedFunction() \
    _NSUnimplementedFunction(__PRETTY_FUNCTION__, __FILE__, __LINE__)
