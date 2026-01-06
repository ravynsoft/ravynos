//
//  HIDDeviceBase.m
//  iohidobjc
//
//  Created by dekom on 10/17/18.
//

#import "IOHIDDevicePrivate.h"
#import "IOHIDLibPrivate.h"
#import "HIDDeviceBase.h"
#import <CoreFoundation/CoreFoundation.h>

@implementation HIDDevice

- (CFTypeID)_cfTypeID {
    return IOHIDDeviceGetTypeID();
}

- (void)dealloc
{
    _IOHIDDeviceReleasePrivate((__bridge IOHIDDeviceRef)self);
    [super dealloc];
}

- (NSString *)description
{
    NSString *desc = (__bridge NSString *)IOHIDDeviceCopyDescription(
                                                (__bridge IOHIDDeviceRef)self);
    return [desc autorelease];
}

@end

