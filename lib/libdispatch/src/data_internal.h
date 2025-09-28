/*
 * Copyright (c) 2009-2012 Apple Inc. All rights reserved.
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

#ifndef __DISPATCH_DATA_INTERNAL__
#define __DISPATCH_DATA_INTERNAL__

#ifndef __DISPATCH_INDIRECT__
#error "Please #include <dispatch/dispatch.h> instead of this file directly."
#include <dispatch/base.h> // for HeaderDoc
#endif

#if defined(__LP64__) && !defined(DISPATCH_DATA_USE_LEAF_MEMBER) && !USE_OBJC
// explicit leaf member is free on 64bit due to padding
#define DISPATCH_DATA_USE_LEAF_MEMBER 1
#endif

typedef struct range_record_s {
	dispatch_data_t data_object;
	size_t from;
	size_t length;
} range_record;

#if USE_OBJC
#if OS_OBJECT_USE_OBJC
@interface DISPATCH_CLASS(data) : NSObject <DISPATCH_CLASS(data)>
@end
#endif
DISPATCH_OBJC_CLASS_DECL(data);
DISPATCH_OBJC_CLASS_DECL(data_empty);
#define DISPATCH_DATA_CLASS DISPATCH_OBJC_CLASS(data)
#define DISPATCH_DATA_EMPTY_CLASS DISPATCH_OBJC_CLASS(data_empty)
#else // USE_OBJC
DISPATCH_CLASS_DECL(data);
#define DISPATCH_DATA_CLASS DISPATCH_VTABLE(data)
#define DISPATCH_DATA_EMPTY_CLASS DISPATCH_VTABLE(data)
#endif // USE_OBJC

struct dispatch_data_s {
#if USE_OBJC
	const void *do_vtable;
	dispatch_queue_t do_targetq;
	void *ctxt;
	void *finalizer;
#else // USE_OBJC
	DISPATCH_STRUCT_HEADER(data);
#endif // USE_OBJC
#if DISPATCH_DATA_USE_LEAF_MEMBER
	bool leaf;
#endif
	dispatch_block_t destructor;
	size_t size, num_records;
	union {
		const void* buf;
		range_record records[0];
	};
};

#if DISPATCH_DATA_USE_LEAF_MEMBER
#define _dispatch_data_leaf(d) ((d)->leaf)
#define _dispatch_data_num_records(d) ((d)->num_records)
#else
#define _dispatch_data_leaf(d) ((d)->num_records ? 0 : ((d)->size ? 1 : 0))
#define _dispatch_data_num_records(d) \
		(_dispatch_data_leaf(d) ? 1 : (d)->num_records)
#endif // DISPATCH_DATA_USE_LEAF_MEMBER

typedef dispatch_data_t (*dispatch_transform_t)(dispatch_data_t data);

struct dispatch_data_format_type_s {
	uint64_t type;
	uint64_t input_mask;
	uint64_t output_mask;
	dispatch_transform_t decode;
	dispatch_transform_t encode;
};

void dispatch_data_init(dispatch_data_t data, const void *buffer, size_t size,
		dispatch_block_t destructor);
void _dispatch_data_dispose(dispatch_data_t data);
size_t _dispatch_data_debug(dispatch_data_t data, char* buf, size_t bufsiz);
extern const dispatch_block_t _dispatch_data_destructor_inline;
#define DISPATCH_DATA_DESTRUCTOR_INLINE (_dispatch_data_destructor_inline)

#if !__OBJC2__

static inline const void*
_dispatch_data_map_direct(dispatch_data_t dd)
{
	size_t offset = 0;
	if (slowpath(!dd->size)) {
		return NULL;
	}
	if (slowpath(!_dispatch_data_leaf(dd)) &&
			_dispatch_data_num_records(dd) == 1 &&
			_dispatch_data_leaf(dd->records[0].data_object)) {
		offset = dd->records[0].from;
		dd = dd->records[0].data_object;
	}
	return fastpath(_dispatch_data_leaf(dd)) ? (void *)((uint8_t *)dd->buf + offset) : NULL;
}

#endif // !__OBJC2__

#endif // __DISPATCH_DATA_INTERNAL__
