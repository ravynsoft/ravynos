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

// Dispatch data objects are dispatch objects with standard retain/release
// memory management. A dispatch data object either points to a number of other
// dispatch data objects or is a leaf data object. A leaf data object contains
// a pointer to represented memory. A composite data object specifies the total
// size of data it represents and list of constituent records.
//
// A leaf data object always points to a full represented buffer, a composite
// dispatch data object is needed to represent a subrange of a memory region.

#if USE_OBJC
#define _dispatch_data_retain(x) _dispatch_objc_retain(x)
#define _dispatch_data_release(x) _dispatch_objc_release(x)
#else
#define _dispatch_data_retain(x) dispatch_retain(x)
#define _dispatch_data_release(x) dispatch_release(x)
#endif

const dispatch_block_t _dispatch_data_destructor_free = ^{
	DISPATCH_CRASH("free destructor called");
};

const dispatch_block_t _dispatch_data_destructor_none = ^{
	DISPATCH_CRASH("none destructor called");
};

const dispatch_block_t _dispatch_data_destructor_vm_deallocate = ^{
	DISPATCH_CRASH("vmdeallocate destructor called");
};

const dispatch_block_t _dispatch_data_destructor_inline = ^{
	DISPATCH_CRASH("inline destructor called");
};

struct dispatch_data_s _dispatch_data_empty = {
	.do_vtable = DISPATCH_DATA_EMPTY_CLASS,
#if !USE_OBJC
	.do_ref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_xref_cnt = DISPATCH_OBJECT_GLOBAL_REFCNT,
	.do_next = DISPATCH_OBJECT_LISTLESS,
#endif
};

DISPATCH_ALWAYS_INLINE
static inline dispatch_data_t
_dispatch_data_alloc(size_t n, size_t extra)
{
	dispatch_data_t data = _dispatch_alloc(DISPATCH_DATA_CLASS,
			sizeof(struct dispatch_data_s) + extra +
			(n ? n * sizeof(range_record) - sizeof(data->buf) : 0));
	data->num_records = n;
#if !USE_OBJC
	data->do_targetq = dispatch_get_global_queue(
			DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
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
	} else if (destructor == DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE) {
		mach_vm_size_t vm_size = size;
		mach_vm_address_t vm_addr = (uintptr_t)buffer;
		mach_vm_deallocate(mach_task_self(), vm_addr, vm_size);
	} else {
		if (!queue) {
			queue = dispatch_get_global_queue(
					DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
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
#if DISPATCH_DATA_USE_LEAF_MEMBER
	data->leaf = true;
	data->num_records = 1;
#endif
	if (queue) {
		_dispatch_retain(queue);
		data->do_targetq = queue;
	}
}

void
dispatch_data_init(dispatch_data_t data, const void *buffer, size_t size,
		dispatch_block_t destructor)
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
		if (slowpath(!data_buf)) {
			return NULL;
		}
		buffer = memcpy(data_buf, buffer, size);
		data = _dispatch_data_alloc(0, 0);
		destructor = DISPATCH_DATA_DESTRUCTOR_FREE;
	} else if (destructor == DISPATCH_DATA_DESTRUCTOR_INLINE) {
		data = _dispatch_data_alloc(0, size);
		buffer = memcpy((void*)((uint8_t *)data + sizeof(struct dispatch_data_s)), buffer,
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
			destructor != DISPATCH_DATA_DESTRUCTOR_VM_DEALLOCATE &&
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

	if (slowpath(!size)) {
		goto out;
	}
	data = _dispatch_data_alloc(0, size);
	buffer = (void*)((uint8_t *)data + sizeof(struct dispatch_data_s));
	_dispatch_data_init(data, buffer, size, NULL,
			DISPATCH_DATA_DESTRUCTOR_NONE);
out:
	if (buffer_ptr) {
		*buffer_ptr = buffer;
	}
	return data;
}

void
_dispatch_data_dispose(dispatch_data_t dd)
{
	dispatch_block_t destructor = dd->destructor;
	if (destructor == NULL) {
		size_t i;
		for (i = 0; i < _dispatch_data_num_records(dd); ++i) {
			_dispatch_data_release(dd->records[i].data_object);
		}
	} else {
		_dispatch_data_destroy_buffer(dd->buf, dd->size, dd->do_targetq,
				destructor);
	}
}

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
	if (!dd1->size) {
		_dispatch_data_retain(dd2);
		return dd2;
	}
	if (!dd2->size) {
		_dispatch_data_retain(dd1);
		return dd1;
	}
	data = _dispatch_data_alloc(_dispatch_data_num_records(dd1) +
			_dispatch_data_num_records(dd2), 0);
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
	} else if ((offset + length) > dd->size) {
		length = dd->size - offset;
	} else if (length == dd->size) {
		_dispatch_data_retain(dd);
		return dd;
	}
	if (_dispatch_data_leaf(dd)) {
		data = _dispatch_data_alloc(1, 0);
		data->size = length;
		data->records[0].from = offset;
		data->records[0].length = length;
		data->records[0].data_object = dd;
		_dispatch_data_retain(dd);
		return data;
	}
	// Subrange of a composite dispatch data object: find the record containing
	// the specified offset
	data = dispatch_data_empty;
	size_t i = 0, bytes_left = length;
	while (i < _dispatch_data_num_records(dd) &&
			offset >= dd->records[i].length) {
		offset -= dd->records[i++].length;
	}
	while (i < _dispatch_data_num_records(dd)) {
		size_t record_len = dd->records[i].length - offset;
		if (record_len > bytes_left) {
			record_len = bytes_left;
		}
		dispatch_data_t subrange = dispatch_data_create_subrange(
				dd->records[i].data_object, dd->records[i].from + offset,
				record_len);
		dispatch_data_t concat = dispatch_data_create_concat(data, subrange);
		_dispatch_data_release(data);
		_dispatch_data_release(subrange);
		data = concat;
		bytes_left -= record_len;
		if (!bytes_left) {
			return data;
		}
		offset = 0;
		i++;
	}
	// Crashing here indicates memory corruption of passed in data object
	DISPATCH_CRASH("dispatch_data_create_subrange out of bounds");
	return NULL;
}

// When mapping a leaf object or a subrange of a leaf object, return a direct
// pointer to the represented buffer. For all other data objects, copy the
// represented buffers into a contiguous area. In the future it might
// be possible to relocate the buffers instead (if not marked as locked).
dispatch_data_t
dispatch_data_create_map(dispatch_data_t dd, const void **buffer_ptr,
		size_t *size_ptr)
{
	dispatch_data_t data = dd;
	const void *buffer = NULL;
	size_t size = dd->size, offset = 0;
	if (!size) {
		data = dispatch_data_empty;
		goto out;
	}
	if (!_dispatch_data_leaf(dd) && _dispatch_data_num_records(dd) == 1 &&
			_dispatch_data_leaf(dd->records[0].data_object)) {
		offset = dd->records[0].from;
		dd = dd->records[0].data_object;
	}
	if (_dispatch_data_leaf(dd)) {
		_dispatch_data_retain(data);
		buffer = (uint8_t *)dd->buf + offset;
		goto out;
	}
	// Composite data object, copy the represented buffers
	buffer = malloc(size);
	if (!buffer) {
		data = NULL;
		size = 0;
		goto out;
	}
	dispatch_data_apply(dd, ^(dispatch_data_t region DISPATCH_UNUSED,
			size_t off, const void* buf, size_t len) {
			memcpy((void*)((uint8_t *)buffer + off), buf, len);
			return (bool)true;
	});
	data = dispatch_data_create(buffer, size, NULL,
			DISPATCH_DATA_DESTRUCTOR_FREE);
out:
	if (buffer_ptr) {
		*buffer_ptr = buffer;
	}
	if (size_ptr) {
		*size_ptr = size;
	}
	return data;
}

static bool
_dispatch_data_apply(dispatch_data_t dd, size_t offset, size_t from,
		size_t size, void *ctxt, dispatch_data_applier_function_t applier)
{
	bool result = true;
	dispatch_data_t data = dd;
	const void *buffer;
	dispatch_assert(dd->size);
	if (!_dispatch_data_leaf(dd) && _dispatch_data_num_records(dd) == 1 &&
			_dispatch_data_leaf(dd->records[0].data_object)) {
		from = dd->records[0].from;
		dd = dd->records[0].data_object;
	}
	if (_dispatch_data_leaf(dd)) {
		buffer = (uint8_t *)dd->buf + from;
		return _dispatch_client_callout3(ctxt, data, offset, buffer, size,
				applier);
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

// Returs either a leaf object or an object composed of a single leaf object
dispatch_data_t
dispatch_data_copy_region(dispatch_data_t dd, size_t location,
		size_t *offset_ptr)
{
	if (location >= dd->size) {
		*offset_ptr = 0;
		return dispatch_data_empty;
	}
	dispatch_data_t data;
	size_t size = dd->size, offset = 0, from = 0;
	while (true) {
		if (_dispatch_data_leaf(dd)) {
			_dispatch_data_retain(dd);
			*offset_ptr = offset;
			if (size == dd->size) {
				return dd;
			} else {
				// Create a new object for the requested subrange of the leaf
				data = _dispatch_data_alloc(1, 0);
				data->size = size;
				data->records[0].from = from;
				data->records[0].length = size;
				data->records[0].data_object = dd;
				return data;
			}
		} else {
			// Find record at the specified location
			size_t i, pos;
			for (i = 0; i < _dispatch_data_num_records(dd); ++i) {
				pos = offset + dd->records[i].length;
				if (location < pos) {
					size = dd->records[i].length;
					from = dd->records[i].from;
					data = dd->records[i].data_object;
					if (_dispatch_data_num_records(dd) == 1 &&
							_dispatch_data_leaf(data)) {
						// Return objects composed of a single leaf node
						*offset_ptr = offset;
						_dispatch_data_retain(dd);
						return dd;
					} else {
						// Drill down into other objects
						dd = data;
						break;
					}
				} else {
					offset = pos;
				}
			}
		}
	}
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
	flags = (vm_prot_t)(VM_PROT_DEFAULT|VM_PROT_IS_MASK|MAP_MEM_VM_COPY);
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
