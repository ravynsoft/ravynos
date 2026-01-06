//
//  HIDConnectionBase.h
//  IOKitUser
//
//  Created by dekom on 9/16/18.
//

#ifndef HIDConnectionBase_h
#define HIDConnectionBase_h

#if __OBJC__

#import "HIDConnectionIvar.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/NSObject.h>

@interface HIDConnection : NSObject {
@protected
    HIDConnectionStruct _connection;
}

@end

#endif /* __OBJC__ */

#endif /* HIDConnectionBase_h */
