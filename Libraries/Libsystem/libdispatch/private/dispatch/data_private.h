/*
 * Copyright (c) 2011-2013 Apple Inc. All rights reserved.
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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __DISPATCH_DATA_PRIVATE__
#define __DISPATCH_DATA_PRIVATE__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

DISPATCH_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

/*!
 * @const DISPATCH_DATA_DESTRUCTOR_NONE
 * @discussion The destructor for dispatch data objects that require no buffer
 * memory management. This can be used to allow a data object to efficiently
 * encapsulate buffers that should not be copied or freed by the system.
 */
#define DISPATCH_DATA_DESTRUCTOR_NONE (_dispatch_data_destructor_none)
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_DATA_DESTRUCTOR_TYPE_DECL(none);

/*!
 * @const DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE
 * @discussion The destructor for dispatch data objects that have been created
 * from buffers that require deallocation using vm_deallocate.
 */
#define DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE \
		(_dispatch_data_destructor_vm_deallocate)
API_AVAILABLE(macos(10.8), ios(6.0)) DISPATCH_LINUX_UNAVAILABLE()
DISPATCH_DATA_DESTRUCTOR_TYPE_DECL(vm_deallocate);

/*!
 * @function dispatch_data_create_f
 * Creates a dispatch data object from the given contiguous buffer of memory. If
 * a non-default destructor is provided, ownership of the buffer remains with
 * the caller (i.e. the bytes will not be copied). The last release of the data
 * object will result in the invocation of the specified destructor function on
 * specified queue to free the buffer (passed as the context parameter).
 *
 * If the DISPATCH_DATA_DESTRUCTOR_FREE destructor is provided the buffer will
 * be freed via free(3) and the queue argument ignored.
 *
 * If the DISPATCH_DATA_DESTRUCTOR_DEFAULT destructor is provided, data object
 * creation will copy the buffer into internal memory managed by the system.
 *
 * @param buffer	A contiguous buffer of data.
 * @param size		The size of the contiguous buffer of data.
 * @param queue		The queue to which the destructor should be submitted.
 * @param destructor	The destructor function responsible for freeing the
 *			data buffer when it is no longer needed.
 * @result		A newly created dispatch data object.
 */
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_EXPORT DISPATCH_RETURNS_RETAINED DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_data_t
dispatch_data_create_f(const void *buffer,
	size_t size,
	dispatch_queue_t _Nullable queue,
	dispatch_function_t _Nullable destructor);

/*!
 * @function dispatch_data_create_alloc
 * Creates a dispatch data object representing a newly allocated memory region
 * of the given size. If a non-NULL reference to a pointer is provided, it is
 * filled with the location of the memory region.
 *
 * It is the responsibility of the application to ensure that the data object
 * becomes immutable (i.e. the returned memory region is not further modified)
 * once the dispatch data object is passed to other API.
 *
 * @param size		The size of the required allocation.
 * @param buffer_ptr	A pointer to a pointer variable to be filled with the
 *			location of the newly allocated memory region, or NULL.
 * @result		A newly created dispatch data object.
 */
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_RETURNS_RETAINED
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_data_t
dispatch_data_create_alloc(size_t size, void *_Nullable *_Nullable buffer_ptr);

/*!
 * @typedef dispatch_data_applier_function_t
 * A function to be invoked for every contiguous memory region in a data object.
 *
 * @param context	Application-defined context parameter.
 * @param region	A data object representing the current region.
 * @param offset	The logical offset of the current region to the start
 *					of the data object.
 * @param buffer	The location of the memory for the current region.
 * @param size		The size of the memory for the current region.
 * @result		A Boolean indicating whether traversal should continue.
 */
typedef bool (*dispatch_data_applier_function_t)(void *_Nullable context,
	dispatch_data_t region, size_t offset, const void *buffer, size_t size);

/*!
 * @function dispatch_data_apply_f
 * Traverse the memory regions represented by the specified dispatch data object
 * in logical order and invoke the specified function once for every contiguous
 * memory region encountered.
 *
 * Each invocation of the function is passed a data object representing the
 * current region and its logical offset, along with the memory location and
 * extent of the region. These allow direct read access to the memory region,
 * but are only valid until the passed-in region object is released. Note that
 * the region object is released by the system when the function returns, it is
 * the responsibility of the application to retain it if the region object or
 * the associated memory location are needed after the function returns.
 *
 * @param data		The data object to traverse.
 * @param context	The application-defined context to pass to the function.
 * @param applier	The function to be invoked for every contiguous memory
 *			region in the data object.
 * @result		A Boolean indicating whether traversal completed
 *			successfully.
 */
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
bool
dispatch_data_apply_f(dispatch_data_t data, void *_Nullable context,
	dispatch_data_applier_function_t applier);

#if TARGET_OS_MAC
/*!
 * @function dispatch_data_make_memory_entry
 * Return a mach memory entry for the memory regions represented by the
 * specified dispatch data object.
 *
 * For data objects created with the DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE
 * destructor, directly makes a memory entry from the represented region;
 * otherwise, makes a memory entry from newly allocated pages containing a copy
 * of the represented memory regions.
 *
 * @param data		The data object to make a memory entry for.
 * @result		A mach port for the newly made memory entry, or
 *			MACH_PORT_NULL if an error occurred.
 */
API_AVAILABLE(macos(10.9), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_NOTHROW
mach_port_t
dispatch_data_make_memory_entry(dispatch_data_t data);
#endif

/*!
 * @functiongroup Dispatch data transform SPI
 */

/*!
 * @typedef dispatch_data_format_type_t
 *
 * @abstract
 * Data formats are used to specify the input and output types of data supplied
 * to dispatch_data_create_transform.
 */
typedef const struct dispatch_data_format_type_s *dispatch_data_format_type_t;

#define DISPATCH_DATA_FORMAT_TYPE_DECL(name) \
	DISPATCH_EXPORT const struct dispatch_data_format_type_s \
	_dispatch_data_format_type_##name

/*!
 * @const DISPATCH_DATA_FORMAT_TYPE_NONE
 * @discussion A data format denoting that the given input or output format is,
 * or should be, comprised of raw data bytes with no given encoding.
 */
#define DISPATCH_DATA_FORMAT_TYPE_NONE (&_dispatch_data_format_type_none)
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_DATA_FORMAT_TYPE_DECL(none);

/*!
 * @const DISPATCH_DATA_FORMAT_TYPE_BASE32
 * @discussion A data format denoting that the given input or output format is,
 * or should be, encoded in Base32 (RFC 4648) format. On input, this format will
 * skip whitespace characters. Cannot be used in conjunction with UTF format
 * types.
 */
#define DISPATCH_DATA_FORMAT_TYPE_BASE32 (&_dispatch_data_format_type_base32)
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_DATA_FORMAT_TYPE_DECL(base32);

/*!
 * @const DISPATCH_DATA_FORMAT_TYPE_BASE32HEX
 * @discussion A data format denoting that the given input or output format is,
 * or should be, encoded in Base32Hex (RFC 4648) format. On input, this format
 * will skip whitespace characters. Cannot be used in conjunction with UTF
 * format types.
 */
#define DISPATCH_DATA_FORMAT_TYPE_BASE32HEX \
		(&_dispatch_data_format_type_base32hex)
API_AVAILABLE(macos(10.9), ios(7.0))
DISPATCH_DATA_FORMAT_TYPE_DECL(base32hex);

/*!
 * @const DISPATCH_DATA_FORMAT_TYPE_BASE64
 * @discussion A data format denoting that the given input or output format is,
 * or should be, encoded in Base64 (RFC 4648) format. On input, this format will
 * skip whitespace characters. Cannot be used in conjunction with UTF format
 * types.
 */
#define DISPATCH_DATA_FORMAT_TYPE_BASE64 (&_dispatch_data_format_type_base64)
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_DATA_FORMAT_TYPE_DECL(base64);

/*!
 * @const DISPATCH_DATA_FORMAT_TYPE_UTF8
 * @discussion A data format denoting that the given input or output format is,
 * or should be, encoded in UTF-8 format. Is only valid when used in conjunction
 * with other UTF format types.
 */
#define DISPATCH_DATA_FORMAT_TYPE_UTF8 (&_dispatch_data_format_type_utf8)
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_DATA_FORMAT_TYPE_DECL(utf8);

/*!
 * @const DISPATCH_DATA_FORMAT_TYPE_UTF16LE
 * @discussion A data format denoting that the given input or output format is,
 * or should be, encoded in UTF-16LE format. Is only valid when used in
 * conjunction with other UTF format types.
 */
#define DISPATCH_DATA_FORMAT_TYPE_UTF16LE (&_dispatch_data_format_type_utf16le)
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_DATA_FORMAT_TYPE_DECL(utf16le);

/*!
 * @const DISPATCH_DATA_FORMAT_TYPE_UTF16BE
 * @discussion A data format denoting that the given input or output format is,
 * or should be, encoded in UTF-16BE format. Is only valid when used in
 * conjunction with other UTF format types.
 */
#define DISPATCH_DATA_FORMAT_TYPE_UTF16BE (&_dispatch_data_format_type_utf16be)
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_DATA_FORMAT_TYPE_DECL(utf16be);

/*!
 * @const DISPATCH_DATA_FORMAT_TYPE_UTFANY
 * @discussion A data format denoting that dispatch_data_create_transform should
 * attempt to automatically detect the input type based on the presence of a
 * byte order mark character at the beginning of the data. In the absence of a
 * BOM, the data will be assumed to be in UTF-8 format. Only valid as an input
 * format.
 */
#define DISPATCH_DATA_FORMAT_TYPE_UTF_ANY (&_dispatch_data_format_type_utf_any)
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_DATA_FORMAT_TYPE_DECL(utf_any);

/*!
 * @function dispatch_data_create_transform
 * Returns a new dispatch data object after transforming the given data object
 * from the supplied format, into the given output format.
 *
 * @param data
 * The data object representing the region(s) of memory to transform.
 * @param input_type
 * Flags specifying the input format of the source dispatch_data_t
 *
 * @param output_type
 * Flags specifying the expected output format of the resulting transformation.
 *
 * @result
 * A newly created dispatch data object, dispatch_data_empty if no has been
 * produced, or NULL if an error occurred.
 */
API_AVAILABLE(macos(10.8), ios(6.0))
DISPATCH_EXPORT DISPATCH_NONNULL_ALL DISPATCH_RETURNS_RETAINED
DISPATCH_WARN_RESULT DISPATCH_NOTHROW
dispatch_data_t
dispatch_data_create_with_transform(dispatch_data_t data,
	dispatch_data_format_type_t input_type,
	dispatch_data_format_type_t output_type);

/*!
 * @function dispatch_data_get_flattened_bytes_4libxpc
 *
 * Similar to dispatch_data_create_map() but attaches it to the passed in
 * dispatch data.
 *
 * The returned mapping, if not NULL, has the size returned by
 * dispatch_data_get_size() for the specified object, and its lifetime is tied
 * to the one of the dispatch data itself.
 *
 * @discussion
 * This interface is reserved for XPC usage and is not considered stable ABI.
 *
 *
 * @result
 * A newly created linear mapping for this data object, may return NULL if
 * making the dispatch data contiguous failed to allocate memory.
 */
API_AVAILABLE(macos(10.14), ios(12.0), tvos(12.0), watchos(5.0), bridgeos(4.0))
const void *_Nullable
dispatch_data_get_flattened_bytes_4libxpc(dispatch_data_t data);

__END_DECLS

DISPATCH_ASSUME_NONNULL_END

#endif // __DISPATCH_DATA_PRIVATE__
