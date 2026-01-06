//
//  IOHIDServiceClientObjc.m
//  iohidobjc
//
//  Created by dekom on 10/5/18.
//

#import <IOKit/hid/IOHIDServiceClient.h>
#import "HIDServiceClientBase.h"

CFTypeID IOHIDServiceClientGetTypeID(void)
{
    return (CFTypeID)[HIDServiceClient self];
}

IOHIDServiceClientRef _IOHIDServiceClientCreatePrivate(CFAllocatorRef allocator __unused)
{
    return (__bridge IOHIDServiceClientRef)[[HIDServiceClient alloc] init];
}

