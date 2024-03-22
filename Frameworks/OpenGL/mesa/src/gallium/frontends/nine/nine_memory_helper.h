/*
 * Copyright 2020 Axel Davy <davyaxel0@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

#ifndef _NINE_MEMORY_HELPER_H_
#define _NINE_MEMORY_HELPER_H_


struct NineDevice9;

struct nine_allocator;
struct nine_allocation;

/* Note: None of these functions are thread safe, thus the worker thread is disallowed
 * to call any of them. Only exception is nine_free_worker reserved for it. */

struct nine_allocation *
nine_allocate(struct nine_allocator *allocator, unsigned size);

/* Note: Suballocations MUST be freed before their parent */
void nine_free(struct nine_allocator *allocator, struct nine_allocation *allocation);
void nine_free_worker(struct nine_allocator *allocator, struct nine_allocation *allocation);

void *nine_get_pointer(struct nine_allocator *allocator, struct nine_allocation *allocation);

/* We don't need the pointer anymore, but we are likely to need it again soon */
void nine_pointer_weakrelease(struct nine_allocator *allocator, struct nine_allocation *allocation);

/* We don't need the pointer anymore, probably for a long time */
void nine_pointer_strongrelease(struct nine_allocator *allocator, struct nine_allocation *allocation);

/* You can strong release when counter becomes 0.
 * Once a counter is used for a given allocation, the same must keep being used */
void nine_pointer_delayedstrongrelease(struct nine_allocator *allocator,
                                       struct nine_allocation *allocation,
                                       unsigned *counter);

/* Note: It is disallowed to release a suballocation before its parent.
 * It is disallowed to suballocate on a suballocation. */
struct nine_allocation *
nine_suballocate(struct nine_allocator* allocator, struct nine_allocation *allocation, int offset);

/* Won't be freed - but at least we can use the same interface */
struct nine_allocation *
nine_wrap_external_pointer(struct nine_allocator* allocator, void* data);


/* memfd_virtualsizelimit: Limit for the virtual memory usage (in MB)
 * above which memfd files are unmapped (to reduce virtual memory usage).
 * If negative, disables memfd usage. */
struct nine_allocator *
nine_allocator_create(struct NineDevice9 *device, int memfd_virtualsizelimit);

void
nine_allocator_destroy(struct nine_allocator *allocator);

#endif /* _NINE_MEMORY_HELPER_H_ */
