/*    CFMachPort_Internal.h
 Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
 Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
 Licensed under Apache License v2.0 with Runtime Library Exception
 See http://swift.org/LICENSE.txt for license information
 See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
 */

#if TARGET_OS_MAC

/// Encapsulates the complexities of cleaning up after unsent mach messages.
/// Call this after you mach_msg to clean up in case of errors. 
///
/// @param kr return value of mach_msg
/// @param msg that was attempted to be sent
CF_EXPORT void __CFMachMessageCheckForAndDestroyUnsentMessage(kern_return_t const kr, mach_msg_header_t *const msg);

#endif
