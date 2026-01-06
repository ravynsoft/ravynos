//
//  IOHIDEventPrivate.h
//  IOKitUser
//
//  Created by dekom on 8/20/18.
//

#ifndef IOHIDEventPrivate_h
#define IOHIDEventPrivate_h

#include <IOKit/hid/IOHIDEvent.h>

CF_EXPORT
IOHIDEventRef _IOHIDEventCreate(CFAllocatorRef allocator,
                                CFIndex dataSize,
                                IOHIDEventType type,
                                uint64_t timeStamp,
                                IOOptionBits options);

CF_EXPORT
Boolean _IOHIDEventEqual(CFTypeRef cf1, CFTypeRef cf2);

CF_EXPORT
CFStringRef IOHIDEventCopyDescription(IOHIDEventRef event);

#endif /* IOHIDEventPrivate_h */
