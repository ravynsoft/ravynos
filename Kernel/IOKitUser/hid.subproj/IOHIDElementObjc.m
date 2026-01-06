//
//  IOHIDElementObjc.m
//  iohidobjc
//
//  Created by dekom on 10/4/18.
//

#import <Foundation/Foundation.h>
#import <IOKit/hid/IOHIDElement.h>
#import "HIDElementBase.h"

CFTypeID IOHIDElementGetTypeID(void)
{
    return (CFTypeID)[HIDElement self];
}

IOHIDElementRef _IOHIDElementCreatePrivate(CFAllocatorRef allocator __unused)
{
    return (__bridge IOHIDElementRef)[[HIDElement alloc] init];
}
