//
//  HIDElementBase.m
//  iohidobjc
//
//  Created by dekom on 10/4/18.
//

#import "IOHIDElementPrivate.h"
#import "HIDElementBase.h"
#import <CoreFoundation/CoreFoundation.h>

@implementation HIDElement

- (CFTypeID)_cfTypeID {
    return IOHIDElementGetTypeID();
}

- (void)dealloc
{
    _IOHIDElementReleasePrivate((__bridge IOHIDElementRef)self);
    [super dealloc];
}

- (NSUInteger)hash
{
    return (NSUInteger)IOHIDElementGetCookie((__bridge IOHIDElementRef)self);
}

- (BOOL)isEqual:(id)object {
    if (self == object) {
        return YES;
    }

    if (![object isKindOfClass:[HIDElement class]]) {
        return NO;
    }

    return (IOHIDElementGetCookie((__bridge IOHIDElementRef)self) ==
            IOHIDElementGetCookie((__bridge IOHIDElementRef)object));
}

@end
