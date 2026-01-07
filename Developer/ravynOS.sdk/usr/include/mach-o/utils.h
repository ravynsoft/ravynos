/*
 * Copyright (c) 2021 Apple Inc. All rights reserved.
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
#ifndef _MACH_O_UTILS_H_
#define _MACH_O_UTILS_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <mach-o/loader.h>
#include <Availability.h>


#if __cplusplus
extern "C" {
#endif



/*!
 * @function macho_cpu_type_for_arch_name
 *
 * @abstract
 *      Converts an architecture name into a cpu type/subtype pair.
 *
 * @param archName
 *      An architecture name (e.g "arm64e" or "x86_64").
 *
 * @param type
 *      A pointer to where to store the cpu type of the given name.
 *
 * @param subtype
 *      A pointer to where to store the cpu subtype of the given name.
 *
 * @return
 *		If the archName is known, returns true and fills in the type/subtype.
 *		If the archName is unknown, returns false.
 */
extern bool macho_cpu_type_for_arch_name(const char* _Nonnull archName, cpu_type_t* _Nonnull type, cpu_subtype_t* _Nonnull subtype)
__API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(8.0));


/*!
 * @function macho_arch_name_for_cpu_type
 *
 * @abstract
 *      Converts a cpu type/subtype pair into the architecture name.
 *
 * @param type
 *      The cpu type from <machine/machine.h> (e.g CPU_TYPE_ARM64)
 *
 * @param subtype
 *      The cpu subtype from <machine/machine.h> (e.g CPU_SUBTYPE_ARM64E)
 *
 * @return
 *		Returns a static c-string which is the name for the cpu type/subtype (e.g. "arm64e").
 *		If the cpu type/subtype is unknown, NULL will be returned.
 *		The string returned is static and does not need to be deallocated.
 */
extern const char* _Nullable macho_arch_name_for_cpu_type(cpu_type_t type, cpu_subtype_t subtype)
__API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(8.0));


/*!
 * @function macho_arch_name_for_mach_header
 *
 * @abstract
 *      Returns the architecture name from the cpu type/subtype in a mach_header.
 *      This is a convenience wrapper around macho_arch_name_for_cpu_type().
 *
 * @param mh
 *      A pointer to the header of a mach-o file.
 *      If NULL is passed, the architecture name of the main executable will be returned.
 *
 * @return
 *		Returns a static c-string which is the name for architecture of the mach-o file (e.g. "arm64e").
 *		If the architecture is unknown, NULL will be returned.
 *		The string returned is static and does not need to be deallocated.
 */
extern const char* _Nullable macho_arch_name_for_mach_header(const struct mach_header* _Nullable mh)
__API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(8.0));

#ifdef __BLOCKS__
  #if __has_attribute(noescape)
    #define __MACHO_NOESCAPE __attribute__((__noescape__))
  #else
    #define __MACHO_NOESCAPE
  #endif

/*!
 * @function macho_for_each_slice
 *
 * @abstract
 *      Temporarily maps a mach-o or universal file and iterates the slices.
 *      If the file is mach-o, the block is called once with the mach-o file mapped.
 *      If the file is universal (aka fat), the block is called once per slice in the order in the header.
 *      If the path does not exist or does, but is not a mach-o file, the block is never called.
 *
 * @param path
 *      The path to the file to inspect.
 *
 * @param callback
 *      A block to call once per slice.
 *      Can be NULL.  In which case the return value tells you if the file is mach-o or fat.
 *      The slice pointer is only valid during the block invocation.
 *      To stop iterating the slices, set *stop to true.
 *
 * @return
 *      Returns zero on success, otherwise it returns an errno value.
 *      Common returned errors:
 *          ENOENT  - path does not exist
 *          EACCES - path exists put caller does not have permission to access it
 *          EFTYPE - path exists but it is not a mach-o or fat file
 *          EBADMACHO - path is a mach-o file, but it is malformed
 */
extern int macho_for_each_slice(const char* _Nonnull path, void (^ _Nullable callback)(const struct mach_header* _Nonnull slice, uint64_t sliceFileOffset, size_t size, bool* _Nonnull stop) __MACHO_NOESCAPE)
__API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(8.0));


/*!
 * @function macho_for_each_slice_in_fd
 *
 * @abstract
 *      Temporarily maps a mach-o or universal file and iterates the slices.
 *      If the fd is to a mach-o, the block is called once with the mach-o file mapped.
 *      If the fd is to a universal (aka fat), the block is called once per slice in the order in the header.
 *      If the fd is closed or not mmap()able, the block is never called.
 *
 * @param fd
 *      An open file descriptor to a mmap()able file.
 *
 * @param callback
 *      A block to call once per slice.
 *      Can be NULL.  In which case the return value tells you if the file is mach-o or fat.
 *      The slice pointer is only valid during the block invocation.
 *      To stop iterating the slices, set *stop to true.
 *
 * @return
 *      Returns zero on success, otherwise it returns an errno value.
 *      Common returned errors:
 *          EFTYPE - fd content is not a mach-o or fat file
 *          EBADMACHO - fd content is a mach-o file, but it is malformed
 */
extern int macho_for_each_slice_in_fd(int fd, void (^ _Nullable callback)(const struct mach_header* _Nonnull slice, uint64_t sliceFileOffset, size_t size, bool* _Nonnull stop)__MACHO_NOESCAPE)
__API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(8.0));


/*!
 * @function macho_best_slice
 *
 * @abstract
 *      Examines a mach-o or universal file to find the slice that would be loaded.  That is, for dylib/bundles, which
 *      slice dyld would load.  For main executables, which slice the kernel would use.
 *      In simulator processes, only other simulator main executables will be considered loadable.
 *      If the file is mach-o and is the right arch and platform to load, the block is called once with the mach-o file mapped.
 *      If the file is universal (aka fat) file, the best slice is found and the block is called once with the mapped slice.
 *      If the file is universal (aka fat) file, but none of the slices are loadable, the callback is not called, and EBADARCH is returned.
 *      If the path does not exist or does but is not a mach-o or universal file, the block is never called, and an error is returned.
 *
 * @param path
 *      The path to the file to inspect.
 *
 * @param callback
 *      A block to call once with the best slice.
 *      Can be NULL.  In which case the return value tells you if there was a loadable slice
 *      The slice pointer is only valid during the block invocation.
 *
 * @return
 *      Returns zero on success (meaning there is a best slice), otherwise it returns an errno value.
 *      Common returned errors:
 *          ENOENT  - path does not exist
 *          EACCES - path exists put caller does not have permission to access it
 *          EFTYPE - path exists but it is not a mach-o or fat file
 *          EBADARCH - path exists and is mach-o or fat, but none of the slices are loadable
 *          EBADMACHO - path is a mach-o file, but it is malformed
 */
extern int macho_best_slice(const char* _Nonnull path, void (^ _Nullable bestSlice)(const struct mach_header* _Nonnull slice, uint64_t sliceFileOffset, size_t sliceSize)__MACHO_NOESCAPE)
__API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(8.0));


/*!
 * @function macho_best_slice_in_fd
 *
 * @abstract
 *      Examines a mach-o or universal file to find the slice that would be loaded.  That is, for dylib/bundles, which
 *      slice dyld would load.  For main executables, which slice the kernel would use.
 *      In simulator processes, only other simulator main executables will be considered loadable.
 *      If the fd is to a mach-o and is the right arch and platform to load, the block is call once with the mach-o file mapped.
 *      If the fd is to a universal (aka fat) file, the best slice is found and the block is called once with the mapped slice.
 *      If the fd is closed or not mmap()able, the block is never called.
 *
 * @param fd
 *      An open file descriptor to a mmap()able file.
 *
 * @param callback
 *      A block to call once with the best slice.
 *      Can be NULL.  In which case the return value tells you if there was a loadable slice
 *      The slice pointer is only valid during the block invocation.
 *
 * @return
 *      Returns zero on success (meaning there is a best slice), otherwise it returns an errno value.
 *      Common returned errors:
 *          EFTYPE - fd content is not a mach-o or fat file
 *          EBADMACHO - fd content is a mach-o file, but it is malformed
 *          EBADARCH - fd content is a mach-o or fat, but none of the slices are loadable
 */
extern int macho_best_slice_in_fd(int fd, void (^ _Nullable bestSlice)(const struct mach_header* _Nonnull slice, uint64_t sliceFileOffset, size_t sliceSize)__MACHO_NOESCAPE)
__API_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(8.0));

#endif // __BLOCKS__





#if __cplusplus
}
#endif


#endif // _MACH_O_UTILS_H_

