/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Original - David Young <daver@geeks.org>
#import <AppKit/NSKeyboardBindingManager.h>
#import <AppKit/NSKeyboardBinding.h>
#import <AppKit/NSEvent.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSScanner.h>

@implementation NSKeyboardBindingManager

+ (NSArray *)keyBindingPaths {
    return [NSArray arrayWithObjects:
        [[NSBundle bundleForClass:[NSKeyboardBindingManager class]] pathForResource:@"StandardKeyBindings" ofType:@"keyBindings"],
        [[NSBundle mainBundle] pathForResource:@"KeyBindings" ofType:@"keyBindings"],
        nil];
}

+ (NSKeyboardBindingManager *)defaultKeyBindingManager {
    static NSKeyboardBindingManager *firstKeyBindingManager = nil;
    if (firstKeyBindingManager == nil) {
        NSArray *paths = [self keyBindingPaths];
        int i, count = [paths count];

        for (i = 0; i < count; ++i) {
            NSString *path = [paths objectAtIndex:i];
            NSDictionary *keyBindingDictionary = [NSDictionary dictionaryWithContentsOfFile:path];

            if (keyBindingDictionary != nil) {
                NSKeyboardBindingManager *manager = [[self alloc] initWithDictionary:keyBindingDictionary];

                [manager setNextKeyBindingManager:firstKeyBindingManager];
                firstKeyBindingManager = manager;
            }
        }
    }

    return firstKeyBindingManager;
}

// we'll try the [modifier1..],hex = (selector, selectorN)... format first.
// unless the keybinding dictionary becomes *huge*, I can't see this being too much of a performance hit,
// especially since this is only done once.
// ...of course you could feed this implementation stuff like "controlQQQQ", but why would you do that?
+ (unsigned)modifierMaskWithKeyComponents:(NSArray *)components {
    int i, count = [components count];
    unsigned mask = 0;

    for (i = 0; i < count; ++i) {
        NSString *component = [components objectAtIndex:i];

        if ([component isEqualToString:@"control"])
            mask |= NSControlKeyMask;
        else if ([component isEqualToString:@"alt"])
            mask |= NSAlternateKeyMask;
        else if ([component isEqualToString:@"shift"])
            mask |= NSShiftKeyMask;
        else if ([component isEqualToString:@"keypad"])
            mask |= NSNumericPadKeyMask;
        else if ([component isEqualToString:@"command"])
            mask |= NSCommandKeyMask;
        else
            [NSException raise:NSInvalidArgumentException
                        format:@"unknown modifier mask name %@", component];
    }

    return mask;
}

+ (NSArray *)keyBindingsFromDictionary:(NSDictionary *)dict {
    NSArray *allKeys = [dict allKeys];
    NSMutableArray *keyBindings = [NSMutableArray array];
    int i, count = [allKeys count];

    for (i = 0; i < count; ++i) {
        NSKeyboardBinding *keyBinding;
        NSString *key = [allKeys objectAtIndex:i];
        id value = [dict objectForKey:key];
        NSMutableArray *keyComponents = [[[key componentsSeparatedByString:@","] mutableCopy] autorelease];
        unsigned short hexCode;
        unsigned modifierMask = 0,hexInt=0;
        NSScanner *scanner = [NSScanner scannerWithString:[keyComponents lastObject]];

        [scanner scanHexInt:&hexInt];
        hexCode=hexInt;

        [keyComponents removeLastObject];
        if ([keyComponents count] > 0)
            modifierMask = [self modifierMaskWithKeyComponents:keyComponents];

        if ([value isKindOfClass:[NSString class]])
            value = [NSArray arrayWithObject:value];

        // we can change this later to support multi-character unicode
        keyBinding = [NSKeyboardBinding keyBindingWithString:[NSString stringWithCharacters:&hexCode length:1]
                                           modifierMask:modifierMask selectorNames:value];

        [keyBindings addObject:keyBinding];
    }

    return keyBindings;
}

- (id)initWithDictionary:(NSDictionary *)dictionary {
    [super init];

    _dictionary = [dictionary retain];
    _keyBindings = [[NSKeyboardBindingManager keyBindingsFromDictionary:_dictionary] retain];

    return self;
}

- (void)dealloc {
    [_dictionary release];
    [_keyBindings release];
    [_nextKeyBindingManager release];
    
    [super dealloc];
}

- (NSArray *)keyBindings {
    return _keyBindings;
}

- (void)setNextKeyBindingManager:(NSKeyboardBindingManager *)manager {
    [_nextKeyBindingManager autorelease];
    _nextKeyBindingManager = [manager retain];
}

- (NSKeyboardBindingManager *)nextKeyBindingManager {
    return _nextKeyBindingManager;
}

- (NSKeyboardBinding *)keyBindingWithString:(NSString *)string modifierFlags:(unsigned)flags {
    int i, count = [_keyBindings count];

    flags&=~ NSNumericPadKeyMask;
    flags&=~ NSAlphaShiftKeyMask;

    for (i = 0; i < count; ++i) {
        NSKeyboardBinding *keyBinding = [_keyBindings objectAtIndex:i];

        if ([[keyBinding string] isEqualToString:string] && (flags==([keyBinding modifierMask]&~ NSNumericPadKeyMask)))
            return keyBinding;
    }

    return [_nextKeyBindingManager keyBindingWithString:string modifierFlags:flags];
}

@end
