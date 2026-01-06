//
//  HIDElementIvar.h
//  iohidobjc
//
//  Created by dekom on 10/4/18.
//

#ifndef HIDElementIvar_h
#define HIDElementIvar_h

#import "hidobjcbase.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/objc.h> // for objc_object

#define HIDElementIvar \
IOHIDDeviceDeviceInterface  **deviceInterface; \
IOHIDDeviceRef              device; \
IOHIDValueRef               value; \
IOHIDElementStruct          *elementStructPtr; \
uint32_t                    index; \
CFDataRef                   data; \
CFMutableArrayRef           attachedElements; \
CFArrayRef                  childElements; \
IOHIDElementRef             parentElement; \
IOHIDElementRef             originalElement; \
IOHIDCalibrationInfo        *calibrationPtr; \
CFMutableDictionaryRef      properties; \
CFStringRef                 rootKey; \
Boolean                     isDirty;

typedef struct  {
    HIDElementIvar
} HIDElementStruct;

#endif /* HIDElementIvar_h */
