//
//  HIDConnectionBase.m
//  IOKitUser
//
//  Created by dekom on 9/16/18.
//

#import "IOHIDEventSystemConnectionPrivate.h"
#import "HIDConnectionBase.h"
#import <CoreFoundation/CoreFoundation.h>

@implementation HIDConnection

- (CFTypeID)_cfTypeID {
    return IOHIDEventSystemConnectionGetTypeID();
}

- (NSString *)description
{
    NSString *desc =  (__bridge NSString *)IOHIDEventSystemConnectionCopyDescription(
                                        (__bridge IOHIDEventSystemConnectionRef)self);
    return [desc autorelease];
}

- (void)dealloc
{
    _IOHIDEventSystemConnectionReleasePrivate(
                                (__bridge IOHIDEventSystemConnectionRef)self);
    [super dealloc];
}

@end
