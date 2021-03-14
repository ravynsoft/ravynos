/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/NSKeyboardBinding.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>

@implementation NSKeyboardBinding

+ (NSKeyboardBinding *)keyBindingWithString:(NSString *)string modifierMask:(int)mask selectorNames:(NSArray *)selNames {
    return [[[self alloc] initWithString:string modifierMask:mask selectorNames:selNames] autorelease];
}

- (id)initWithString:(NSString *)string modifierMask:(unsigned)mask selectorNames:(NSArray *)selNames {
    [super init];

    _string = [string retain];
    _modifierMask = mask;
    _selectorNames = [selNames retain];

    return self;
}

- (void)dealloc {
    [_selectorNames release];
    [_string release];

    [super dealloc];
}

- (NSString *)string { return _string; }
- (unsigned)modifierMask { return _modifierMask; }

- (NSArray *)selectorNames { return _selectorNames; }

- (NSString *)description {
    return [NSString stringWithFormat:@"<%@[0x%lx] string: %@ modifierMask: 0x%lx selectorNames: %@>",
        isa, self, _string, _modifierMask, _selectorNames];
}

@end
