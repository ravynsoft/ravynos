/*
 * Copyright (c) 2020 Apple Inc. All rights reserved.
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

#ifndef dyld_introspection_h
#define dyld_introspection_h

#include <Availability.h>

#include <mach/task.h>
#include <mach/machine.h>
#include <mach-o/loader.h>
#include <dispatch/dispatch.h>

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(DYLD_PLATFORM_T_DEFINED)
#define DYLD_PLATFORM_T_DEFINED (1)
typedef uint32_t dyld_platform_t;
#endif

// FIXME: We should improve the documentation once rdar://58760015 is fixed
#define DYLD_MACOS_12_SPI SPI_AVAILABLE(macos(12.0)) API_UNAVAILABLE(ios,watchos,tvos,bridgeos)

#define DYLD_MACOS_12_ALIGNED_SPI SPI_AVAILABLE(macos(12.0), ios(15.0), tvos(15.0), watchos(8.0), bridgeos(6.0))
#define DYLD_MACOS_13_ALIGNED_SPI SPI_AVAILABLE(macos(13.0), ios(16.0), tvos(16.0), watchos(9.0), bridgeos(7.0))

#if BUILDING_CACHE_BUILDER || ENABLE_DYLD_STATIC_ROSETTA_RUNTIME_SUPPORT
#define DYLD_MACOS_12_ALIGNED_AND_STATIC_SPI
#else
#define DYLD_MACOS_12_ALIGNED_AND_STATIC_SPI DYLD_MACOS_12_ALIGNED_SPI
#endif

typedef struct dyld_process_s*              dyld_process_t;
typedef struct dyld_process_snapshot_s*     dyld_process_snapshot_t;
typedef struct dyld_shared_cache_s*         dyld_shared_cache_t;
typedef struct dyld_image_s*                dyld_image_t;

/* Notes on memory ownership:
 *   All parameter passed into blocks in this function are only valid for the lifetime of the block. That means if the caller needs something
 *   to persist that the caller must copy (or vm_copy) that data during their block invocation.
 */

#pragma mark -
#pragma mark Dyld Process Introspection Interfaces

#if !BUILDING_CACHE_BUILDER
/*
 * dyld_process_create_for_current_task
 *   Creates a dyld_process_t for a current process
 */
DYLD_MACOS_12_ALIGNED_SPI
extern dyld_process_t dyld_process_create_for_current_task();

/*
 * dyld_process_create_for_task
 *   Creates a dyld_process_t for a given task
 *   If kr is non-null then it will be set to KERN_SUCCESS if the call succeeds, and return any error codes if it failed
 *   Returns null on failure, otherwise returns a dyld_process_snapshot_t whose resources must be released via dyld_process_dispose()
 */
DYLD_MACOS_12_ALIGNED_SPI
extern dyld_process_t dyld_process_create_for_task(task_read_t task, kern_return_t *kr);

/*
 * dyld_process_dispose
 *   This function release any resource held by the dyld_process_t passed in. This included unregistering all notification handlers
 *   Using the dyld_process_t after this is an error
 */
DYLD_MACOS_12_ALIGNED_SPI
extern void dyld_process_dispose(dyld_process_t process);

/*
 * dyld_process_snapshot_create_for_process
 *   Creates a snapshot of the process that can be used for introspecting the libraries loaded at that point in time.
 *   If kr is non-null then it will be set to KERN_SUCCESS if the call succeeds, and return any error codes if it failed
 *   Returns null on failure, otherwise returns a dyld_process_snapshot_t whose resources must be released via dyld_process_snapshot_dispose()
 */
DYLD_MACOS_12_ALIGNED_AND_STATIC_SPI
extern dyld_process_snapshot_t dyld_process_snapshot_create_for_process(dyld_process_t process, kern_return_t *kr);


/*
 * dyld_process_snapshot_create_from_data
 *   Creates a snapshot from a serialized blob that can be introspected.
 *   buffer: The serialized process info. Currenrtly the only way to obtain this by inspecting a processes memory dor it directly
 *   size: The size of the buffer passed in
 *   reserved1: Must be NULL. In the future this will be used for passing an additional "system" info buffer read from the shared cache
 *   reserved2: Must be 0
 */
DYLD_MACOS_13_ALIGNED_SPI
extern dyld_process_snapshot_t dyld_process_snapshot_create_from_data(void* buffer, size_t size, void* reserved1, size_t reserved2);

/*
 * dyld_process_snapshot_dispose
 *    Disposes of the snapshot and frees any resources held by it
 */
DYLD_MACOS_12_ALIGNED_SPI
extern void dyld_process_snapshot_dispose(dyld_process_snapshot_t snapshot);

/*
 * dyld_process_register_for_image_notifications
 *   Registers for notifications when images are loaded and unloaded.
 *   This function takes a block that will be called once for each load or unload
 *   On initial registration the block will be called for once each image already loaded in the binary
 *   If kr is non-null then it will be set to KERN_SUCCESS if the call succeeds, and return any error codes if it failed
 *   Returns 0 on failure, on success a non-zero will be returned to be used as a handle to dyld_process_unregister_for_notification
 */
DYLD_MACOS_13_ALIGNED_SPI
extern uint32_t dyld_process_register_for_image_notifications(dyld_process_t, kern_return_t *kr,
                                                              dispatch_queue_t queue, void (^block)(dyld_image_t image, bool load));

#define DYLD_REMOTE_EVENT_MAIN (1) // This event is called immediately before main will be executed
// rdar://48435712 (ER: Way to suspend a process just after the shared cache gets mapped in)
#define DYLD_REMOTE_EVENT_SHARED_CACHE_MAPPED (2)
#pragma clang deprecated(DYLD_REMOTE_EVENT_SHARED_CACHE_MAPPED, "DYLD_REMOTE_EVENT_SHARED_CACHE_MAPPED is deprecated, use DYLD_REMOTE_EVENT_BEFORE_INITIALIZERS")
#define DYLD_REMOTE_EVENT_BEFORE_INITIALIZERS (2)  // This event is called before running initializers

/*
 * dyld_process_register_for_event_notification
 *   Registers for notifications when specific dyld events occur.
 *   This function takes a block that will be called once for each load or unload
 *   If kr is non-null then it will be set to KERN_SUCCESS if the call succeeds, and return any error codes if it failed
 *   Returns 0 on failure, on success a non-zero will be returned to be used as a handle to dyld_process_unregister_for_notification
 */
DYLD_MACOS_12_ALIGNED_AND_STATIC_SPI
extern uint32_t dyld_process_register_for_event_notification(dyld_process_t process, kern_return_t *kr, uint32_t event,
                                                             dispatch_queue_t queue, void (^block)());

/*
 * dyld_process_unregister_for_notifications
 *   Disarms a registered notification. Takes a handle return by dyld_process_register_for_image_notifications or
 *   dyld_process_register_for_event_notification
 */
DYLD_MACOS_12_ALIGNED_AND_STATIC_SPI
extern void dyld_process_unregister_for_notification(dyld_process_t, uint32_t handle);

/*
 * dyld_process_snapshot_for_each_image
 *   Iterates over all the images currently loaded in a dyld_process_t. Does not include images that are mapped as part of the shared cache
 *   but that have not actually been loaded into the process.
 */
DYLD_MACOS_13_ALIGNED_SPI
extern void dyld_process_snapshot_for_each_image(dyld_process_snapshot_t snapshot, void (^block)(dyld_image_t image));

#pragma mark Functions to get shared caches

/*
 * dyld_process_snapshot_get_shared_cache
 *   Provides the shared cache object associated with a snapshot
 */
DYLD_MACOS_12_ALIGNED_SPI
extern dyld_shared_cache_t dyld_process_snapshot_get_shared_cache(dyld_process_snapshot_t snapshot);

#endif /* BUILDING_CACHE_BUILDER */

/*
 * dyld_for_each_installed_shared_cache
 *   Iterates over each shared cache provided by the operating system and calls block with the dyld_shared_cache_t for that shared cache
 *   The dyld_shared_cache_t is only valid for the body of the block. As the caches are not used in a live process their base_address will be
 *   set as if the cache was mapped with no ASLR slide, and all dyld_image_t's will have their vmAddr's set as though they are unslid
 *
 *   Equivalent to dyld_for_each_installed_shared_cache_with_system_path("/", block);
 */
DYLD_MACOS_12_ALIGNED_SPI
extern void dyld_for_each_installed_shared_cache(void (^block)(dyld_shared_cache_t cache));

/*
 * dyld_for_each_installed_shared_cache_with_system_path
 *   Iterates over each shared cache provided by the operating system installed at root_path, and calls block with the dyld_shared_cache_t for
 *   that shared cache The dyld_shared_cache_t is only valid for the body of the block. As the caches are not used in a live process their
 *   base_address will be set as if the cache was mapped with no ASLR slide, and all dyld_image_t's will have their vmAddr's set as though they
 *   are unslid
 */
DYLD_MACOS_12_ALIGNED_SPI
extern void dyld_for_each_installed_shared_cache_with_system_path(const char* rootPath, void (^block)(dyld_shared_cache_t cache));


/*
 * dyld_shared_cache_for_file
 *   Maps in a shared cache and invokes block with it
     Returns true if it is able to successfully map in the block, false otherwise
 */
DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_shared_cache_for_file(const char* filePath, void (^block)(dyld_shared_cache_t cache));

#pragma mark Functions to inspect shared caches

/*
 * dyld_shared_cache_pin_mapping
 *    This function maps a shared cache into a contiguous range of memory. Any subsequent calls that return content from the shared cache
 *    will use content from the pinned mapping, and their lifetimes will be extended to that of the pinned mapping. This call may fail if
 *    there is not enough contiguous address space to map in the requested shared cache.
 *    Returns true if the cache was succesfully mapped, false if it was not
 */
DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_shared_cache_pin_mapping(dyld_shared_cache_t cache);

/*
 * dyld_shared_cache_unpin_mapping
 *    This function unmaps a pinned shared cache from memory. This invalidates all pointers to content within the pinned shared cache.
 */
DYLD_MACOS_12_ALIGNED_SPI
extern void dyld_shared_cache_unpin_mapping(dyld_shared_cache_t cache);

/*
 * dyld_shared_cache_get_base_address
 *    Returns the base of address of the shared cache
 */
DYLD_MACOS_12_ALIGNED_SPI
extern uint64_t dyld_shared_cache_get_base_address(dyld_shared_cache_t cache);

/*
 * dyld_shared_cache_get_mapped_size
 *    Returns the size of the mapping of the shared cache
 *    This value is equivalent to subtracting the base of the shared cache from the last address in the last region of the shared cache
 */
DYLD_MACOS_12_ALIGNED_SPI
extern uint64_t dyld_shared_cache_get_mapped_size(dyld_shared_cache_t cache);

/*
 * dyld_shared_cache_is_mapped_private
 *    Returns true if the shared cache is using a private mapping, returns false of the shared cache is using a shared system mapping
 */
DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_shared_cache_is_mapped_private(dyld_shared_cache_t cache);

/*
 * dyld_shared_cache_copy_uuid
 *    Copies the UUID of a shared cache into a buffer passed in via the uuid parameter
 */
DYLD_MACOS_12_ALIGNED_SPI
extern void dyld_shared_cache_copy_uuid(dyld_shared_cache_t cache, uuid_t* uuid);

/*
 * dyld_shared_cache_for_each_file
 *    Calls the block parameter once with the path of every file that is used as backing storage for the shared cache
 *    The file_path parameters memory is valid for the life of the block. It is the caller's responsibility to copy the string
 *    if it needs to reference it later
 */
DYLD_MACOS_12_ALIGNED_SPI
extern void dyld_shared_cache_for_each_file(dyld_shared_cache_t cache, void (^block)(const char* file_path));

/*
 * dyld_shared_cache_for_each_image
 *   Iterates over every image in a shared cache. If this is run against a shared cache from an active task it includes all images in that
 *   cache, even if they have not been loaded by the process
 */
DYLD_MACOS_12_ALIGNED_SPI
extern void dyld_shared_cache_for_each_image(dyld_shared_cache_t cache, void (^block)(dyld_image_t image));

#pragma mark -
#pragma mark Image Introspection Interfaces

/*
 * dyld_image_copy_uuid
 *   Copies the UUID of a dyld_image_t into a buffer
 *   This function may fail either due to the underlying buffer being unavailable, or the image may not have an embedded UUID
 *   Returns true if a UUID was returned, false if not
 */
DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_image_copy_uuid(dyld_image_t cache, uuid_t* uuid);

/*
 * dyld_image_get_installname
 *   Returns the installname of an image. This may return NULL underlying buffer being unavailable, or if the image does not have an
 *   install_name
 */
DYLD_MACOS_12_ALIGNED_SPI
extern const char* dyld_image_get_installname(dyld_image_t image);
/*
 * dyld_image_get_file_path
 *   Returns the path of the file backing an image of an image. This may return NULL underlying buffer being unavailable, such as
 *   if the file has been deleted. Returns null if there is not a mach-o file backing the image
 */
DYLD_MACOS_13_ALIGNED_SPI
extern const char* dyld_image_get_file_path(dyld_image_t image);

/*
 * dyld_image_for_each_segment
 *   Iterates over all segments in a dyld_image_t
 *   The segmentName passed into the block is only valid for the lifetime of the block, it is the responsibility of the block to copy
 *   any data it needs before returning. This function may fail if the underylying file or process backing the image has been deleted or the
 *   data is otherwise unavailable.
 *   Returns true if it was able to iterate over segments, false if it was not.
 */
DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_image_for_each_segment_info(dyld_image_t image, void (^)(const char* segmentName, uint64_t vmAddr, uint64_t vmSize, int perm));

/*
 * dyld_image_content_for_segment
 *   This function materializes the content for the segment as it appears on disk and passes it to contentReader. This data, and do not contain
 *   any fixups or changes that may have been made during runtime. This pointer is only valid as long as the for the lifetime of the block,
 *   unless the image is backed via a pinned mapping (see `dyld_shared_cache_pin_mapping()`). This function may return false if the underlying
 *   buffer backing the dyld_image_t has been deleted or is otherwise inaccessible.
 */
DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_image_content_for_segment(dyld_image_t image, const char* segmentName,
                                      void (^contentReader)(const void* content, uint64_t vmAddr, uint64_t vmSize));

/*
 * dyld_image_for_each section
 *   Iterates over all sections in a dyld_image_t
 *   The segmentName and sectionName passed into the block is only valid for the lifetime of the block, it is the responsibility of the block
 *   to copy any data it needs before returning. This function may fail if the underylying file or process backing the image has been deleted
 *   or the data is otherwise unavailable.
 *   Returns true if it was able to iterate over segments, false if it was not.
 */

DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_image_for_each_section_info(dyld_image_t image,
                                             void (^)(const char* segmentName, const char* sectionName, uint64_t vmAddr, uint64_t vmSize));

/*
 * dyld_image_content_for_segment
 *   This function materializes the content for the segment as it appears on disk and passes it to contentReader. This data is read only, and
 *   does not contain any fixups or changes that may have been made during runtime. dyld_shared_cache_pin_mapping. This function may return
 *   false if the underlying buffer backing the dyld_image_t has been deleted or is otherwise inaccessible.
 */
DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_image_content_for_section(dyld_image_t image, const char* segmentName, const char* sectionName,
                                           void (^contentReader)(const void* content, uint64_t vmAddr, uint64_t vmSize));

/*
 * dyld_image_local_nlist_content_4Symbolication
 */
DYLD_MACOS_12_ALIGNED_SPI
extern bool dyld_image_local_nlist_content_4Symbolication(dyld_image_t image,
                                                 void (^contentReader)(const void* nlistStart, uint64_t nlistCount,
                                                                       const char* stringTable));


#ifdef __cplusplus

}
#endif

#endif /* dyld_introspection_h */
