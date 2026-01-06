//
//  IOHIDDevicePrivate.h
//  IOKitUser
//
//  Created by dekom on 8/31/18.
//

#ifndef IOHIDDevicePrivate_h
#define IOHIDDevicePrivate_h

#include <IOKit/hid/IOHIDDevice.h>

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

CF_EXPORT
IOHIDDeviceRef _Nullable _IOHIDDeviceCreatePrivate(CFAllocatorRef _Nullable allocator);

CF_EXPORT
CFStringRef IOHIDDeviceCopyDescription(IOHIDDeviceRef device);

CF_EXPORT
void _IOHIDDeviceReleasePrivate(IOHIDDeviceRef device);

uint64_t IOHIDDeviceGetRegistryEntryID(IOHIDDeviceRef device);

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* IOHIDDevicePrivate_h */
