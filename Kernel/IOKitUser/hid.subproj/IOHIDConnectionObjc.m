//
//  IOHIDConnectionObjc.m
//  IOKitUser
//
//  Created by dekom on 9/16/18.
//

#include "HIDConnectionBase.h"

CFTypeID IOHIDEventSystemConnectionGetTypeID(void)
{
    return (CFTypeID)[HIDConnection self];
}

IOHIDEventSystemConnectionRef _IOHIDEventSystemConnectionCreatePrivate(CFAllocatorRef allocator)
{
    return (__bridge IOHIDEventSystemConnectionRef)[[HIDConnection allocWithZone:(struct _NSZone *)allocator] init];
}
