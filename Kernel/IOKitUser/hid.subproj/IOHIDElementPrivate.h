//
//  IOHIDElementPrivate.h
//  IOKitUser
//
//  Created by dekom on 8/31/18.
//

#ifndef IOHIDElementPrivate_h
#define IOHIDElementPrivate_h

#include <IOKit/hid/IOHIDElement.h>

__BEGIN_DECLS

CF_ASSUME_NONNULL_BEGIN
CF_IMPLICIT_BRIDGING_ENABLED

CF_EXPORT
IOHIDElementRef _Nullable _IOHIDElementCreatePrivate(CFAllocatorRef _Nullable allocator);

CF_EXPORT
void _IOHIDElementReleasePrivate(IOHIDElementRef element);

CF_IMPLICIT_BRIDGING_DISABLED
CF_ASSUME_NONNULL_END

__END_DECLS

#endif /* IOHIDElementPrivate_h */
