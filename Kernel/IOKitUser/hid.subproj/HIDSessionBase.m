//
//  HIDSessionBase.m
//  iohidobjc
//
//  Created by dekom on 9/13/18.
//

#import "IOHIDSessionPrivate.h"
#import "HIDSessionBase.h"
#import <CoreFoundation/CoreFoundation.h>

@implementation HIDSession

- (CFTypeID)_cfTypeID {
    return IOHIDSessionGetTypeID();
}

- (void)dealloc
{
    _IOHIDSessionReleasePrivate((__bridge IOHIDSessionRef)self);
    [super dealloc];
}

@end
