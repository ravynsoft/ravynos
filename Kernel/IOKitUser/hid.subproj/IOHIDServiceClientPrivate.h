//
//  IOHIDServiceClientPrivate.h
//  IOKitUser
//
//  Created by dekom on 8/31/18.
//

#ifndef IOHIDServiceClientPrivate_h
#define IOHIDServiceClientPrivate_h

#include <IOKit/hid/IOHIDServiceClient.h>

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

CF_EXPORT
IOHIDServiceClientRef _Nullable _IOHIDServiceClientCreatePrivate(CFAllocatorRef _Nullable allocator);

CF_EXPORT
void _IOHIDServiceClientReleasePrivate(IOHIDServiceClientRef service);

CF_EXPORT
CFStringRef IOHIDServiceClientCopyDescription(IOHIDServiceClientRef service);

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* IOHIDServiceClientPrivate_h */
