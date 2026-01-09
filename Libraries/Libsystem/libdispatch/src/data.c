/*
 * Copyright (c) 2009-2013 Apple Inc. All rights reserved.
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

#include "internal.h"

/*
 * Dispatch data objects are dispatch objects with standard retain/release
 * memory management. A dispatch data object either points to a number of other
 * dispatch data objects or is a leaf data object.
 * A composite data object specifies the total size of data it represents
 * and list of constituent records.
 *
 *******************************************************************************
 *
 * CURRENT IMPLEMENTATION DETAILS
 *
 *   There are actually 3 kinds of composite objects
 *   - trivial subranges
 *   - unflattened composite data objects
 *   - flattened composite data objects
 *
 * LEAVES (num_records == 0, destructor != nil)
 *
 *   Those objects have a pointer to represented memory in `buf`.
 *
 * UNFLATTENED (num_records > 1, buf == nil, destructor == nil)
 *
 *   This is the generic case of a composite object.
 *
 * FLATTENED (num_records > 1, buf != nil, destructor == nil)
 *
 *   Those objects are non trivial composite objects whose `buf` pointer
 *   is a contiguous representation (copied) of the memory it represents.
 *
 *   Such objects are created when used as an NSData and -bytes is called and
 *   where the dispatch data object is an unflattened composite object.
 *   The underlying implementation is dispatch_data_get_flattened_bytes_4libxpc.
 *
 * TRIVIAL SUBRANGES (num_records == 1, buf == nil, destructor == nil)
 *
 *   Those objects point to a single leaf, never to flattened objects.
 *
 *******************************************************************************
 *
 * Non trivial invariants:
 *
 *   It is forbidden to point into a composite data object and ignore entire
 *   records from it.  (for example by having `from` longer than the first
 *   record length).
 *
 *   dispatch_data_t's are either leaves, or composite objects pointing to
 *   leaves. Depth is never greater than 1.
 *
 *******************************************************************************
 *
 * There are 4 dispatch_data_t constructors who may create non leaf objects,
 * and ensure proper invariants.
 *
 * dispatch_data_copy_region()
 *    This function first sees through trivial subranges, and may in turn
 *    generate new trivial subranges.
 *
 * dispatch_data_create_map()
 *    This function either returns existing data objects, or a leaf.
 *
 * dispatch_data_create_subrange()
 *    This function treats flattened objects like unflattened ones,
 *    and recurses into trivial subranges, it can create trivial subranges.
 *
 * dispatch_data_create_concat()
 *    This function unwraps the top-level composite objects, trivial or not,
 *    and else concatenates the two arguments range lists, hence always creating
 *    unflattened objects, unless one of the arguments was empty.
 *
 *******************************************************************************
 */

#if DISPATCH_DATA_IS_BRIDGED_TO_NSDATA
#define _dispatch_data_retain(x) _dispatch_objc_retain(x)
#define _dispatch_data_release(x) _dispatch_objc_release(x)
#else
#define _dispatch_data_retain(x) dispatch_retain(x)
#define _dispatch_data_release(x) dispatch_release(x)
#endif

DISPATCH_ALWAYS_INLINE
static inline dispatch_data_t
_dispatch_data_alloc(size_t n, size_t extra)
{
	dispatch_data_t data;
	size_t size;
	size_t base_size;

	if (os_add_overflow(sizeof(struct dispatch_data_s), extra, &base_size)) {
		return DISPATCH_OUT_OF_MEMORY;
	}
	if (os_mul_and_add_overflow(n, sizeof(range_record), base_size, &size)) {
		return DISPATCH_OUT_OF_MEMORY;
	}

	data = _dispatch_object_alloc(DISPATCH_DATA_CLASS, size);
	data->num_records = n;
#if !DISPATCH_DATA_IS_BRIDGED_TO_NSDATA
	data->do_targetq = _dispatch_get_default_queue(false);
	data->do_next = DISPATCH_OBJECT_LISTLESS;
#endif
	return data;
}

static void
_dispatch_data_destroy_buffer(const void* buffer, size_t size,
		dispatch_queue_t queue, dispatch_block_t destructor)
{
	if (destructor == DISPATCH_DATA_DESTRUCTOR_FREE) {
		free((void*)buffer);
	} else if (destructor == DISPATCH_DATA_DESTRUCTOR_NONE) {
		// do nothing
#if HAVE_MACH
	} else if (destructor == DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE) {
		mach_vm_size_t vm_size = size;
		mach_vm_address_t vm_addr = (uintptr_t)buffer;
		mach_vm_deallocate(mach_task_self(), vm_addr, vm_size);
#else
		(void)size;
#endif
	} else {
		if (!queue) {
			queue = _dispatch_get_default_queue(false);
		}
		dispatch_async_f(queue, destructor, _dispatch_call_block_and_release);
	}
}

DISPATCH_ALWAYS_INLINE
static inline void
_dispatch_data_init(dispatch_data_t data, const void *buffer, size_t size,
		dispatch_queue_t queue, dispatch_block_t destructor)
{
	data->buf = buffer;
	data->size = size;
	data->destructor = destructor;
	if (queue) {
		_dispatch_retain(queue);
		data->do_targetq = queue;
	}
}

void
_dispatch_data_init_with_bytes(dispatch_data_t data, const void *buffer,
		size_t size, dispatch_block_t destructor)
{
	if (!buffer || !size) {
		if (destructor) {
			_dispatch_data_destroy_buffer(buffer, size, NULL,
					_dispatch_Block_copy(destructor));
		}
		buffer = NULL;
		size = 0;
		destructor = DISPATCH_DATA_DESTRUCTOR_NONE;
	}
	_dispatch_data_init(data, buffer, size, NULL, destructor);
}

dispatch_data_t
dispatch_data_create(const void* buffer, size_t size, dispatch_queue_t queue,
		dispatch_block_t destructor)
{
	dispatch_data_t data;
	void *data_buf = NULL;
	if (!buffer || !size) {
		// Empty data requested so return the singleton empty object. Call
		// destructor immediately in this case to ensure any unused associated
		// storage is released.
		if (destructor) {
			_dispatch_data_destroy_buffer(buffer, size, queue,
					_dispatch_Block_copy(destructor));
		}
		return dispatch_data_empty;
	}
	if (destructor == DISPATCH_DATA_DESTRUCTOR_DEFAULT) {
		// The default destructor was provided, indicating the data should be
		// copied.
		data_buf = malloc(size);
		if (unlikely(!data_buf)) {
			return DISPATCH_OUT_OF_MEMORY;
		}
		buffer = memcpy(data_buf, buffer, size);
		data = _dispatch_data_alloc(0, 0);
		destructor = DISPATCH_DATA_DESTRUCTOR_FREE;
	} else if (destructor == DISPATCH_DATA_DESTRUCTOR_INLINE) {
		data = _dispatch_data_alloc(0, size);
		buffer = memcpy((void*)data + sizeof(struct dispatch_data_s), buffer,
				size);
		destructor = DISPATCH_DATA_DESTRUCTOR_NONE;
	} else {
		data = _dispatch_data_alloc(0, 0);
		destructor = _dispatch_Block_copy(destructor);
	}
	_dispatch_data_init(data, buffer, size, queue, destructor);
	return data;
}

dispatch_data_t
dispatch_data_create_f(const void *buffer, size_t size, dispatch_queue_t queue,
		dispatch_function_t destructor_function)
{
	dispatch_block_t destructor = (dispatch_block_t)destructor_function;
	if (destructor != DISPATCH_DATA_DESTRUCTOR_DEFAULT &&
			destructor != DISPATCH_DATA_DESTRUCTOR_FREE &&
			destructor != DISPATCH_DATA_DESTRUCTOR_NONE &&
#if HAVE_MACH
			destructor != DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE &&
#endif
			destructor != DISPATCH_DATA_DESTRUCTOR_INLINE) {
		destructor = ^{ destructor_function((void*)buffer); };
	}
	return dispatch_data_create(buffer, size, queue, destructor);
}

dispatch_data_t
dispatch_data_create_alloc(size_t size, void** buffer_ptr)
{
	dispatch_data_t data = dispatch_data_empty;
	void *buffer = NULL;

	if (unlikely(!size)) {
		goto out;
	}
	data = _dispatch_data_alloc(0, size);
	buffer = (void*)data + sizeof(struct dispatch_data_s);
	_dispatch_data_init(data, buffer, size, NULL,
			DISPATCH_DATA_DESTRUCTOR_NONE);
out:
	if (buffer_ptr) {
		*buffer_ptr = buffer;
	}
	return data;
}

void
_dispatch_data_dispose(dispatch_data_t dd, DISPATCH_UNUSED bool *allow_free)
{
	if (_dispatch_data_leaf(dd)) {
		_dispatch_data_destroy_buffer(dd->buf, dd->size, dd->do_targetq,
				dd->destructor);
	} else {
		size_t i;
		for (i = 0; i < _dispatch_data_num_records(dd); ++i) {
			_dispatch_data_release(dd->records[i].data_object);
		}
		free((void *)dd->buf);
	}
}

#if DISPATCH_DATA_IS_BRIDGED_TO_NSDATA
void
_dispatch_data_set_target_queue(dispatch_data_t dd, dispatch_queue_t tq)
{
	if (tq == DISPATCH_TARGET_QUEUE_DEFAULT) {
		tq = _dispatch_get_default_queue(false);
	}
	_dispatch_object_set_target_queue_inline(dd, tq);
}
#endif // DISPATCH_DATA_IS_BRIDGED_TO_NSDATA

size_t
_dispatch_data_debug(dispatch_data_t dd, char* buf, size_t bufsiz)
{
	size_t offset = 0;
	offset += dsnprintf(&buf[offset], bufsiz - offset, "data[%p] = { ", dd);
	if (_dispatch_data_leaf(dd)) {
		offset += dsnprintf(&buf[offset], bufsiz - offset,
				"leaf, size = %zd, buf = %p ", dd->size, dd->buf);
	} else {
		offset += dsnprintf(&buf[offset], bufsiz - offset,
				"composite, size = %zd, num_records = %zd ", dd->size,
				_dispatch_data_num_records(dd));
		if (dd->buf) {
			offset += dsnprintf(&buf[offset], bufsiz - offset,
					", flatbuf = %p ", dd->buf);
		}
		size_t i;
		for (i = 0; i < _dispatch_data_num_records(dd); ++i) {
			range_record r = dd->records[i];
			offset += dsnprintf(&buf[offset], bufsiz - offset, "record[%zd] = "
					"{ from = %zd, length = %zd, data_object = %p }, ", i,
					r.from, r.length, r.data_object);
		}
	}
	offset += dsnprintf(&buf[offset], bufsiz - offset, "}");
	return offset;
}

size_t
dispatch_data_get_size(dispatch_data_t dd)
{
	return dd->size;
}

dispatch_data_t
dispatch_data_create_concat(dispatch_data_t dd1, dispatch_data_t dd2)
{
	dispatch_data_t data;
	size_t n;

	if (!dd1->size) {
		_dispatch_data_retain(dd2);
		return dd2;
	}
	if (!dd2->size) {
		_dispatch_data_retain(dd1);
		return dd1;
	}

	if (os_add_overflow(_dispatch_data_num_records(dd1),
			_dispatch_data_num_records(dd2), &n)) {
		return DISPATCH_OUT_OF_MEMORY;
	}
	data = _dispatch_data_alloc(n, 0);
	data->size = dd1->size + dd2->size;
	// Copy the constituent records into the newly created data object
	// Reference leaf objects as sub-objects
	if (_dispatch_data_leaf(dd1)) {
		data->records[0].from = 0;
		data->records[0].length = dd1->size;
		data->records[0].data_object = dd1;
	} else {
		memcpy(data->records, dd1->records, _dispatch_data_num_records(dd1) *
				sizeof(range_record));
	}
	if (_dispatch_data_leaf(dd2)) {
		data->records[_dispatch_data_num_records(dd1)].from = 0;
		data->records[_dispatch_data_num_records(dd1)].length = dd2->size;
		data->records[_dispatch_data_num_records(dd1)].data_object = dd2;
	} else {
		memcpy(data->records + _dispatch_data_num_records(dd1), dd2->records,
				_dispatch_data_num_records(dd2) * sizeof(range_record));
	}
	size_t i;
	for (i = 0; i < _dispatch_data_num_records(data); ++i) {
		_dispatch_data_retain(data->records[i].data_object);
	}
	return data;
}

dispatch_data_t
dispatch_data_create_subrange(dispatch_data_t dd, size_t offset,
		size_t length)
{
	dispatch_data_t data;

	if (offset >= dd->size || !length) {
		return dispatch_data_empty;
	} else if (length > dd->size - offset) {
		length = dd->size - offset;
	} else if (length == dd->size) {
		_dispatch_data_retain(dd);
		return dd;
	}
	/*
	 * we must only optimize leaves and not flattened objects
	 * because lots of users want to keep the end of a buffer and release
	 * as much memory as they can from the beginning of it
	 *
	 * Using the flatbuf here would be very wrong with respect to that goal
	 */
	if (_dispatch_data_leaf(dd)) {
		data = _dispatch_data_alloc(1, 0);
		data->size = length;
		data->records[0].from = offset;
		data->records[0].length = length;
		data->records[0].data_object = dd;
		_dispatch_data_retain(dd);
		return data;
	}

	// Subrange of a composite dispatch data object
	const size_t dd_num_records = _dispatch_data_num_records(dd);
	bool to_the_end = (offset + length == dd->size);
	size_t i = 0;

	// find the record containing the specified offset
	while (i < dd_num_records && offset >= dd->records[i].length) {
		offset -= dd->records[i++].length;
	}

	// Crashing here indicates memory corruption of passed in data object
	if (unlikely(i >= dd_num_records)) {
		DISPATCH_INTERNAL_CRASH(i,
				"dispatch_data_create_subrange out of bounds");
	}

	// if everything is from a single dispatch data object, avoid boxing it
	if (offset + length <= dd->records[i].length) {
		return dispatch_data_create_subrange(dd->records[i].data_object,
				dd->records[i].from + offset, length);
	}

	// find the record containing the end of the current range
	// and optimize the case when you just remove bytes at the origin
	size_t count, last_length = 0;

	if (to_the_end) {
		count = dd_num_records - i;
	} else {
		last_length = length - (dd->records[i].length - offset);
		count = 1;

		while (i + count < dd_num_records) {
			size_t record_length = dd->records[i + count++].length;

			if (last_length <= record_length) {
				break;
			}
			last_length -= record_length;

			// Crashing here indicates memory corruption of passed in data object
			if (unlikely(i + count >= dd_num_records)) {
				DISPATCH_INTERNAL_CRASH(i + count,
						"dispatch_data_create_subrange out of bounds");
			}
		}
	}

	data = _dispatch_data_alloc(count, 0);
	data->size = length;
	memcpy(data->records, dd->records + i, count * sizeof(range_record));

	if (offset) {
		data->records[0].from += offset;
		data->records[0].length -= offset;
	}
	if (!to_the_end) {
		data->records[count - 1].length = last_length;
	}

	for (i = 0; i < count; i++) {
		_dispatch_data_retain(data->records[i].data_object);
	}
	return data;
}

static void*
_dispatch_data_flatten(dispatch_data_t dd)
{
	void *buffer = malloc(dd->size);

	// Composite data object, copy the represented buffers
	if (buffer) {
		dispatch_data_apply(dd, ^(dispatch_data_t region DISPATCH_UNUSED,
				size_t off, const void* buf, size_t len) {
			memcpy(buffer + off, buf, len);
			return (bool)true;
		});
	}

	return buffer;
}

// When mapping a leaf object or a subrange of a leaf object, return a direct
// pointer to the represented buffer. For all other data objects, copy the
// represented buffers into a contiguous area. In the future it might
// be possible to relocate the buffers instead (if not marked as locked).
dispatch_data_t
dispatch_data_create_map(dispatch_data_t dd, const void **buffer_ptr,
		size_t *size_ptr)
{
	dispatch_data_t data = NULL;
	const void *buffer = NULL;
	size_t size = dd->size;

	if (!size) {
		data = dispatch_data_empty;
		goto out;
	}

	buffer = _dispatch_data_map_direct(dd, 0, NULL, NULL);
	if (buffer) {
		_dispatch_data_retain(dd);
		data = dd;
		goto out;
	}

	buffer = _dispatch_data_flatten(dd);
	if (likely(buffer)) {
		data = dispatch_data_create(buffer, size, NULL,
				DISPATCH_DATA_DESTRUCTOR_FREE);
	} else {
		size = 0;
	}

out:
	if (buffer_ptr) {
		*buffer_ptr = buffer;
	}
	if (size_ptr) {
		*size_ptr = size;
	}
	return data;
}

const void *
dispatch_data_get_flattened_bytes_4libxpc(dispatch_data_t dd)
{
	const void *buffer;
	size_t offset = 0;

	if (unlikely(!dd->size)) {
		return NULL;
	}

	buffer = _dispatch_data_map_direct(dd, 0, &dd, &offset);
	if (buffer) {
		return buffer;
	}

	void *flatbuf = _dispatch_data_flatten(dd);
	if (likely(flatbuf)) {
		// we need a release so that readers see the content of the buffer
		if (unlikely(!os_atomic_cmpxchgv2o(dd, buf, NULL, flatbuf,
				&buffer, release))) {
			free(flatbuf);
		} else {
			buffer = flatbuf;
		}
	} else {
		return NULL;
	}

	return buffer + offset;
}

#if DISPATCH_USE_CLIENT_CALLOUT
DISPATCH_NOINLINE
#else
DISPATCH_ALWAYS_INLINE
#endif
static bool
_dispatch_data_apply_client_callout(void *ctxt, dispatch_data_t region, size_t offset,
		const void *buffer, size_t size, dispatch_data_applier_function_t f)
{
	return f(ctxt, region, offset, buffer, size);
}


static bool
_dispatch_data_apply(dispatch_data_t dd, size_t offset, size_t from,
		size_t size, void *ctxt, dispatch_data_applier_function_t applier)
{
	bool result = true;
	const void *buffer;

	buffer = _dispatch_data_map_direct(dd, 0, NULL, NULL);
	if (buffer) {
		return _dispatch_data_apply_client_callout(ctxt, dd,
				offset, buffer + from, size, applier);
	}

	size_t i;
	for (i = 0; i < _dispatch_data_num_records(dd) && result; ++i) {
		result = _dispatch_data_apply(dd->records[i].data_object,
				offset, dd->records[i].from, dd->records[i].length, ctxt,
				applier);
		offset += dd->records[i].length;
	}
	return result;
}

bool
dispatch_data_apply_f(dispatch_data_t dd, void *ctxt,
		dispatch_data_applier_function_t applier)
{
	if (!dd->size) {
		return true;
	}
	return _dispatch_data_apply(dd, 0, 0, dd->size, ctxt, applier);
}

bool
dispatch_data_apply(dispatch_data_t dd, dispatch_data_applier_t applier)
{
	if (!dd->size) {
		return true;
	}
	return _dispatch_data_apply(dd, 0, 0, dd->size, applier,
			(dispatch_data_applier_function_t)_dispatch_Block_invoke(applier));
}

static dispatch_data_t
_dispatch_data_copy_region(dispatch_data_t dd, size_t from, size_t size,
		size_t location, size_t *offset_ptr)
{
	dispatch_data_t reusable_dd = NULL;
	size_t offset = 0;

	if (from == 0 && size == dd->size) {
		reusable_dd = dd;
	}

	if (_dispatch_data_map_direct(dd, from, &dd, &from)) {
		if (reusable_dd) {
			_dispatch_data_retain(reusable_dd);
			return reusable_dd;
		}

		_dispatch_data_retain(dd);
		if (from == 0 && size == dd->size) {
			return dd;
		}

		dispatch_data_t data = _dispatch_data_alloc(1, 0);
		data->size = size;
		data->records[0].from = from;
		data->records[0].length = size;
		data->records[0].data_object = dd;
		return data;
	}

	size_t i;
	for (i = 0; i < _dispatch_data_num_records(dd); ++i) {
		size_t length = dd->records[i].length;

		if (from >= length) {
			from -= length;
			continue;
		}

		length -= from;
		if (location >= offset + length) {
			offset += length;
			from = 0;
			continue;
		}

		from += dd->records[i].from;
		dd = dd->records[i].data_object;
		*offset_ptr += offset;
		location -= offset;
		return _dispatch_data_copy_region(dd, from, length, location, offset_ptr);
	}

	DISPATCH_INTERNAL_CRASH(*offset_ptr+offset,
			"dispatch_data_copy_region out of bounds");
}

// Returs either a leaf object or an object composed of a single leaf object
dispatch_data_t
dispatch_data_copy_region(dispatch_data_t dd, size_t location,
		size_t *offset_ptr)
{
	if (location >= dd->size) {
		*offset_ptr = dd->size;
		return dispatch_data_empty;
	}
	*offset_ptr = 0;
	return _dispatch_data_copy_region(dd, 0, dd->size, location, offset_ptr);
}

#if HAVE_MACH

#ifndef MAP_MEM_VM_COPY
#define MAP_MEM_VM_COPY 0x200000 // <rdar://problem/13336613>
#endif

mach_port_t
dispatch_data_make_memory_entry(dispatch_data_t dd)
{
	mach_port_t mep = MACH_PORT_NULL;
	memory_object_size_t mos;
	mach_vm_size_t vm_size = dd->size;
	mach_vm_address_t vm_addr;
	vm_prot_t flags;
	kern_return_t kr;
	bool copy = (dd->destructor != DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE);

retry:
	if (copy) {
		vm_addr = vm_page_size;
		kr = mach_vm_allocate(mach_task_self(), &vm_addr, vm_size,
				VM_FLAGS_ANYWHERE);
		if (kr) {
			if (kr != KERN_NO_SPACE) {
				(void)dispatch_assume_zero(kr);
			}
			return mep;
		}
		dispatch_data_apply(dd, ^(dispatch_data_t region DISPATCH_UNUSED,
				size_t off, const void* buf, size_t len) {
			memcpy((void*)(vm_addr + off), buf, len);
			return (bool)true;
		});
	} else {
		vm_addr = (uintptr_t)dd->buf;
	}
	flags = VM_PROT_DEFAULT|VM_PROT_IS_MASK|MAP_MEM_VM_COPY;
	mos = vm_size;
	kr = mach_make_memory_entry_64(mach_task_self(), &mos, vm_addr, flags,
			&mep, MACH_PORT_NULL);
	if (kr == KERN_INVALID_VALUE) {
		// Fallback in case MAP_MEM_VM_COPY is not supported
		flags &= ~MAP_MEM_VM_COPY;
		kr = mach_make_memory_entry_64(mach_task_self(), &mos, vm_addr, flags,
				&mep, MACH_PORT_NULL);
	}
	if (dispatch_assume_zero(kr)) {
		mep = MACH_PORT_NULL;
	} else if (mos < vm_size) {
		// Memory object was truncated, e.g. due to lack of MAP_MEM_VM_COPY
		kr = mach_port_deallocate(mach_task_self(), mep);
		(void)dispatch_assume_zero(kr);
		if (!copy) {
			copy = true;
			goto retry;
		}
		mep = MACH_PORT_NULL;
	}
	if (copy) {
		kr = mach_vm_deallocate(mach_task_self(), vm_addr, vm_size);
		(void)dispatch_assume_zero(kr);
	}
	return mep;
}
#endif // HAVE_MACH
