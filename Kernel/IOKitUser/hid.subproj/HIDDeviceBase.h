//
//  HIDDeviceBase.h
//  iohidobjc
//
//  Created by dekom on 10/17/18.
//

#ifndef HIDDeviceBase_h
#define HIDDeviceBase_h

#if __OBJC__

#import "HIDDeviceIvar.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/NSObject.h>

@interface HIDDevice : NSObject {
@protected
    HIDDeviceStruct _device;
}

@end

#endif /* __OBJC__ */

#endif /* HIDDeviceBase_h */
