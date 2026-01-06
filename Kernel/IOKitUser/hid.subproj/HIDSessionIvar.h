//
//  HIDSessionIvar.h
//  iohidobjc
//
//  Created by dekom on 9/13/18.
//

#ifndef HIDSessionIvar_h
#define HIDSessionIvar_h

#import "hidobjcbase.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/objc.h> // for objc_object

#define HIDSessionIvar \
IOHIDEventSystemRef         client; \
IOHIDEventCallback          callback; \
void                        *refCon; \
uint32_t                    lockCount; \
pthread_mutex_t             lock; \
pthread_cond_t              stateCondition; \
boolean_t                   state; \
boolean_t                   stateBusy; \
dispatch_queue_t            eventDispatchQueueSession; \
dispatch_source_t           eventDispatchSource; \
CFMutableArrayRef           eventDipsatchPending; \
CFMutableDictionaryRef      properties; \
uint32_t                    logLevel; \
CFMutableSetRef             serviceSet; \
CFMutableArrayRef           simpleSessionFilters; \
CFMutableArrayRef           sessionFilters; \
CFMutableArrayRef           pendingSessionFilters; \
uint64_t                    activityLastTimestamp; \
struct timeval              activityLastTime; \
CFMutableSetRef             activityNotificationSet;

typedef struct  {
    HIDSessionIvar
} HIDSessionStruct;

#endif /* HIDSessionIvar_h */
