/*
 * Copyright (c) 2022 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef _MACH_O_UTILS_PRIV_H_
#define _MACH_O_UTILS_PRIV_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <mach-o/loader.h>
#include <Availability.h>


#if __cplusplus
extern "C" {
#endif


/*!
 * @function macho_dylib_install_name
 *
 * @abstract
 *      Returns the install_name from the LC_ID_DYLIB of an MH_DYLIB mach_header.
 *
 * @param mh
 *      A pointer to the header of a mach-o dylib.
 *
 * @return
 *		Returns a static c-string which is the -install_name the dylib was built with.
 *		If mh is not a mach_header or not a dylib (MH_DYLIB), NULL will be returned.
 *		The string returned is static and does not need to be deallocated.
 */
extern const char* _Nullable macho_dylib_install_name(const struct mach_header* _Nonnull mh)
__API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(8.0));




#if __cplusplus
}
#endif


#endif // _MACH_O_UTILS_PRIV_H_

