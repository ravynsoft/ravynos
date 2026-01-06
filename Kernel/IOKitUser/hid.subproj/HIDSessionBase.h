//
//  HIDSessionBase.h
//  iohidobjc
//
//  Created by dekom on 9/13/18.
//

#ifndef HIDSessionBase_h
#define HIDSessionBase_h

#if __OBJC__

#import "HIDSessionIvar.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/NSObject.h>

@interface HIDSession : NSObject {
@protected
    HIDSessionStruct _session;
}

@end

#endif /* __OBJC__ */

#endif /* HIDSessionBase_h */
