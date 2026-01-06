//
//  HIDConnectionIvar.h
//  IOKitUser
//
//  Created by dekom on 9/16/18.
//

#ifndef HIDConnectionIvar_h
#define HIDConnectionIvar_h

#import "hidobjcbase.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/objc.h> // for objc_object
#import <os/lock_private.h>

#define HIDConnectionIvar \
IOHIDEventSystemRef                             system; \
CFMutableDictionaryRef                          notifications; \
IOHIDEventQueueRef                              queue; \
IOMIGMachPortRef                                port; \
mach_port_t                                     reply_port; \
IOHIDEventSystemConnectionDemuxCallback         demuxCallback; \
void                                            *demuxRefcon; \
IOHIDEventSystemConnectionTerminationCallback   terminationCallback; \
void                                            *terminationRefcon; \
CFMutableSetRef                                 services; \
pid_t                                           pid; \
dispatch_queue_t                                dispatchQueue; \
mach_port_t                                     sendPossiblePort; \
dispatch_source_t                               sendPossibleSource; \
boolean_t                                       sendPossible; \
CFMutableSetRef                                 propertySet; \
CFStringRef                                     caller; \
CFStringRef                                     procName; \
CFStringRef                                     uuid; \
const char                                      *uuidStr; \
int                                             type; \
CFDictionaryRef                                 attributes; \
task_t                                          task_name_port; \
os_unfair_recursive_lock                        lock; \
IOHIDEventSystemConnectionEntitlements          *entitlements; \
boolean_t                                       disableProtectedServices; \
int                                             filterPriority; \
uint32_t                                        state; \
os_unfair_lock                                  notificationsLock; \
CFMutableDictionaryRef                          virtualServices; \
uint64_t                                        eventFilterMask; \
uint32_t                                        eventFilteredCount; \
uint32_t                                        eventFilterTimeoutCount; \
uint32_t                                        droppedEventCount; \
uint32_t                                        currentDroppedEventCount; \
uint64_t                                        droppedEventTypeMask; \
uint32_t                                        eventCount; \
uint64_t                                        eventMask; \
struct timeval                                  lastDroppedEventTime; \
struct timeval                                  firstDroppedEventTime; \
uint64_t                                        maxEventLatency; \
IOReturn                                        droppedEventStatus; \
uint64_t                                        propertyChangeNotificationHandlingTime; \
IOHIDSimpleQueueRef                             eventLog; \
uint32_t                                        *eventTypeCnt; \
uint32_t                                        activityState; \
uint32_t                                        activityStateChangeCount; \
uint64_t                                        idleNotificationTime; \
dispatch_source_t                               activityDispatchSource; \
IOHIDNotificationRef                            activityNotification; \
IOHIDSimpleQueueRef                             activityLog; \
IOHIDConnectionFilterRef                        filter; \

typedef struct  {
    HIDConnectionIvar
} HIDConnectionStruct;

#endif /* HIDConnectionIvar_h */
