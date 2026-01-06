//
//  IOHIDEventObjc.m
//  iohidobjc
//
//  Created by dekom on 7/30/18.
//

#import <CoreFoundation/CoreFoundation.h>
#import <IOKit/hid/IOHIDEvent.h>
#import "HIDEventBasePrivate.h"

CFTypeID IOHIDEventGetTypeID(void)
{
    return (CFTypeID)[HIDEvent self];
}

IOHIDEventRef _IOHIDEventCreate(CFAllocatorRef allocator,
                                CFIndex dataSize,
                                IOHIDEventType type,
                                uint64_t timeStamp,
                                IOOptionBits options)
{
    return (__bridge IOHIDEventRef)[[HIDEvent allocWithZone:(struct _NSZone *)allocator] initWithSize:dataSize
                                                                                                 type:type
                                                                                            timestamp:timeStamp
                                                                                              options:options];
}
