//
//  HIDServiceClientBase.h
//  iohidobjc
//
//  Created by dekom on 10/5/18.
//

#ifndef HIDServiceClientBase_h
#define HIDServiceClientBase_h

#if __OBJC__

#import "HIDServiceClientIvar.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/NSObject.h>

@interface HIDServiceClient : NSObject {
@protected
    HIDServiceClientStruct _client;
}

@end

#endif /* __OBJC__ */

#endif /* HIDServiceClientBase_h */
