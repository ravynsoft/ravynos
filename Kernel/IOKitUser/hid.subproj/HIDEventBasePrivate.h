//
//  HIDEventBasePrivate.h
//  IOKitUser
//
//  Created by dekom on 9/11/18.
//

#ifndef HIDEventBasePrivate_h
#define HIDEventBasePrivate_h

#import "HIDEventBase.h"

@interface HIDEvent (Private)

- (nullable instancetype)initWithSize:(NSUInteger)size
                                 type:(IOHIDEventType)type
                            timestamp:(uint64_t)timestamp
                              options:(uint32_t)options;

@end

#endif /* HIDEventBasePrivate_h */
