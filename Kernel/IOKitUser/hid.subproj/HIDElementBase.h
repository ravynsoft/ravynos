//
//  HIDElementBase.h
//  iohidobjc
//
//  Created by dekom on 10/4/18.
//

#ifndef HIDElementBase_h
#define HIDElementBase_h

#if __OBJC__

#import "HIDElementIvar.h"
#import <CoreFoundation/CoreFoundation.h>
#import <objc/NSObject.h>

@interface HIDElement : NSObject {
@protected
    HIDElementStruct _element;
}

@end

#endif /* __OBJC__ */

#endif /* HIDElementBase_h */
