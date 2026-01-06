//
//  HIDDeviceIvar.h
//  iohidobjc
//
//  Created by dekom on 10/17/18.
//

#ifndef HIDDeviceIvar_h
#define HIDDeviceIvar_h

#import "hidobjcbase.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/objc.h> // for objc_object

#define HIDDeviceIvar \
io_service_t                            service; \
IOHIDDeviceDeviceInterface              **deviceInterface; \
IOHIDDeviceTimeStampedDeviceInterface   **deviceTimeStampedInterface; \
IOCFPlugInInterface                     **plugInInterface; \
CFMutableDictionaryRef                  properties; \
CFMutableSetRef                         elements; \
CFStringRef                             rootKey; \
CFStringRef                             UUIDKey; \
IONotificationPortRef                   notificationPort; \
io_object_t                             notification; \
CFRunLoopSourceRef                      asyncEventSource; \
CFRunLoopSourceContext1                 sourceContext; \
CFMachPortRef                           queuePort; \
CFRunLoopRef                            runLoop; \
CFStringRef                             runLoopMode; \
dispatch_queue_t                        dispatchQueue; \
dispatch_mach_t                         dispatchMach; \
dispatch_block_t                        cancelHandler; \
dispatch_block_t                        queueCancelHandler; \
_Atomic uint32_t                        dispatchStateMask; \
IOHIDQueueRef                           queue; \
CFArrayRef                              inputMatchingMultiple; \
Boolean                                 loadProperties; \
Boolean                                 isDirty; \
CFMutableSetRef                         removalCallbackSet; \
CFMutableSetRef                         inputReportCallbackSet; \
CFMutableSetRef                         inputValueCallbackSet; \
void                                    *elementHandler; \
void                                    *removalHandler; \
void                                    *inputReportHandler; \
CFMutableDataRef                        reportBuffer; \
void                                    *transaction; \
CFMutableArrayRef                       batchElements; \
uint64_t                                regID;

typedef struct  {
    HIDDeviceIvar
} HIDDeviceStruct;

#endif /* HIDDeviceIvar_h */
