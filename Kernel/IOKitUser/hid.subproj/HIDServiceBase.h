//
//  HIDServiceBase.h
//  IOKitUser
//
//  Created by dekom on 9/13/18.
//

#ifndef HIDServiceBase_h
#define HIDServiceBase_h

#if __OBJC__

#import "HIDServiceIvar.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/NSObject.h>

@interface HIDEventService : NSObject {
@protected
    HIDServiceStruct _service;
}

@end

#endif /* __OBJC__ */


#endif /* HIDServiceBase_h */
