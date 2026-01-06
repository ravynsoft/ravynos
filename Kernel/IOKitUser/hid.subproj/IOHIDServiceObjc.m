//
//  IOHIDServiceObjc.m
//  iohidobjc
//
//  Created by dekom on 9/13/18.
//

#include "HIDServiceBase.h"

CFTypeID IOHIDServiceGetTypeID(void)
{
    return (CFTypeID)[HIDEventService self];
}

IOHIDServiceRef _IOHIDServiceCreatePrivate(CFAllocatorRef allocator)
{
    return (__bridge IOHIDServiceRef)[[HIDEventService allocWithZone:(struct _NSZone *)allocator] init];
}
