//
//  HIDServiceIvar.h
//  IOKitUser
//
//  Created by dekom on 9/13/18.
//

#ifndef HIDServiceIvar_h
#define HIDServiceIvar_h

#import "hidobjcbase.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/objc.h> // for objc_object
#include <os/lock.h>

#define HIDServiceIvar \
IOHIDSessionRef             session; \
io_service_t                service; \
IOHIDServiceInterface       **serviceInterface; \
IOHIDServiceInterface2      **serviceInterface2; \
IOCFPlugInInterface         **plugInInterface; \
CFNumberRef                 registryID; \
__IOHIDServiceQueueContext  *queueContext; \
dispatch_queue_t            dispatchQueue; \
IONotificationPortRef       notificationPort; \
io_object_t                 notification; \
CFMutableSetRef             removalNotificationSet; \
CFMutableSetRef             propertyNotificationSet; \
void *                      eventTarget; \
void *                      eventRefcon; \
IOHIDServiceEventCallback   eventCallback; \
uint32_t                    lastLEDMask; \
uint32_t                    lastButtonMask; \
uint32_t                    currentReportInterval; \
uint32_t                    currentBatchInterval; \
uint32_t                    defaultReportInterval; \
uint32_t                    defaultBatchInterval; \
uint32_t                    primaryUsagePage; \
uint32_t                    primaryUsage; \
char                        transport[32]; \
uint32_t                    queueSize; \
boolean_t                   containsReportInterval; \
uint32_t                    state; \
uint32_t                    eventCount; \
uint64_t                    eventMask; \
CFMutableDictionaryRef      clientCacheDict; \
CFMutableArrayRef           simpleFilters; \
CFMutableArrayRef           filters; \
CFMutableSetRef             keyboardEventInProgress; \
uint64_t                    nullEventMask; \
boolean_t                   displayIntegratedDigitizer; \
boolean_t                   builtIn; \
boolean_t                   inMomentumPhase; \
boolean_t                   inDigitizerPhase; \
boolean_t                   supportReportLatency; \
boolean_t                   hidden; \
boolean_t                   protectedAccess; \
CFMutableDictionaryRef      propertyCache; \
uint32_t                    propertyCacheHit; \
uint32_t                    propertyCacheMiss; \
uint64_t                    activityLastTimestamp; \
struct { \
CFTypeRef                           connection; \
void                                *target; \
void                                *refcon; \
IOHIDServiceVirtualCallbacksV2      *callbacks; \
} virtual; \
CFTypeRef                   *connections; \
uint64_t                    propertySetTime; \
uint64_t                    propertyGetTime; \
uint64_t                    regID; \
IOHIDSimpleQueueRef         eventLog; \
uint64_t                    *eventTypeCnt; \
dispatch_block_t            pluginCancelHandler; \
dispatch_block_t            filterCancelHandler; \
bool                        pendingPluginCancel; \
uint32_t                    pendingFilterCancels; \
dispatch_source_t           cancelTimer; \
os_unfair_lock              dataLock; \
CFTypeRef                   hidAnalyticsEvent;\
CFTypeRef                   hidAnalyticsDispatchEvent;\
struct { \
    void        *interface; \
    CFStringRef name; \
    SEL         getProperty; \
    SEL         setProperty; \
    SEL         eventMatching; \
    SEL         setEventDispatcher; \
    SEL         setCancelHandler; \
    SEL         activate; \
    SEL         cancel; \
    SEL         setDispatchQueue; \
    SEL         clientNotification; \
    SEL         copyEvent; \
    SEL         setOutputEvent; \
} objc;

typedef struct  {
    HIDServiceIvar
} HIDServiceStruct;

#endif /* HIDServiceIvar_h */
