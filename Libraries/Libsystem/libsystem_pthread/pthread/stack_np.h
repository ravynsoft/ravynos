/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __PTHREAD_STACK_NP__
#define __PTHREAD_STACK_NP__

#include <Availability.h>
#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <os/base.h>

OS_ASSUME_NONNULL_BEGIN

/*! @header
 * Low-level API to introspect thread stacks.
 */

__BEGIN_DECLS

/*!
 * @function pthread_stack_frame_decode_np
 *
 * @abstract
 * Decodes the return address and the next stack frame address
 * from the given stack frame address.
 *
 * @discussion
 * Validation of the frame address is not performed by this function.
 * The caller is responsible for making sure the frame address is valid,
 * for example using pthread_get_stackaddr_np() and pthread_get_stacksize_np().
 *
 * @param frame_addr
 * A valid stack frame address such as __builtin_frame_address(0) or the return
 * value of a previous call to pthread_stack_frame_decode_np().
 *
 * @param return_addr
 * An optional out paramter that will be filled with the return address stored
 * at the specified stack frame.
 *
 * @returns
 * This returns the next frame address stored at the specified stack frame.
 */
__OSX_AVAILABLE(10.14) __IOS_AVAILABLE(12.0)
__TVOS_AVAILABLE(12.0) __WATCHOS_AVAILABLE(5.0)
uintptr_t
pthread_stack_frame_decode_np(uintptr_t frame_addr,
		uintptr_t *_Nullable return_addr);

__END_DECLS

OS_ASSUME_NONNULL_END

#endif // __PTHREAD_STACK_NP__
