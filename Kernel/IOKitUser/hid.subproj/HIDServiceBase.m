//
//  HIDServiceBase.m
//  IOKitUser
//
//  Created by dekom on 9/13/18.
//

#import "IOHIDServicePrivate.h"
#import "HIDServiceBase.h"
#import <CoreFoundation/CoreFoundation.h>

@implementation HIDEventService

- (CFTypeID)_cfTypeID {
    return IOHIDServiceGetTypeID();
}

- (void)dealloc
{
    _IOHIDServiceReleasePrivate((__bridge IOHIDServiceRef)self);
    [super dealloc];
}

- (NSString *)description
{
    NSString *desc = (__bridge NSString *)IOHIDServiceCopyDescription(
                                                (__bridge IOHIDServiceRef)self);
    return [desc autorelease];
}

@end
