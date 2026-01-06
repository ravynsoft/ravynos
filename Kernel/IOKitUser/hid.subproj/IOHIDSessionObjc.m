//
//  IOHIDSessionObjc.m
//  iohidobjc
//
//  Created by dekom on 9/13/18.
//

#include "HIDSessionBase.h"

CFTypeID IOHIDSessionGetTypeID(void)
{
    return (CFTypeID)[HIDSession self];
}

IOHIDSessionRef _IOHIDSessionCreatePrivate(CFAllocatorRef allocator)
{
    return (__bridge IOHIDSessionRef)[[HIDSession allocWithZone:(struct _NSZone *)allocator] init];
}
