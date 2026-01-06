//
//  IOHIDDeviceObjc.m
//  iohidobjc
//
//  Created by dekom on 10/17/18.
//

#import <IOKit/hid/IOHIDDevice.h>
#import "HIDDeviceBase.h"

CFTypeID IOHIDDeviceGetTypeID(void)
{
    return (CFTypeID)[HIDDevice self];
}

IOHIDDeviceRef _IOHIDDeviceCreatePrivate(CFAllocatorRef allocator __unused)
{
    return (__bridge IOHIDDeviceRef)[[HIDDevice alloc] init];
}
