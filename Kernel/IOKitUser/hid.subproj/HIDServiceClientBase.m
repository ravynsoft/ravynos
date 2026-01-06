//
//  HIDServiceClientBase.m
//  iohidobjc
//
//  Created by dekom on 10/5/18.
//

#import "IOHIDServiceClientPrivate.h"
#import "HIDServiceClientBase.h"
#import <CoreFoundation/CoreFoundation.h>

@implementation HIDServiceClient

- (CFTypeID)_cfTypeID {
    return IOHIDServiceClientGetTypeID();
}

- (void)dealloc
{
    _IOHIDServiceClientReleasePrivate((__bridge IOHIDServiceClientRef)self);
    [super dealloc];
}

- (NSString *)description
{
    NSString *desc =  (__bridge NSString *)IOHIDServiceClientCopyDescription(
                                        (__bridge IOHIDServiceClientRef)self);
    return [desc autorelease];
}

@end

