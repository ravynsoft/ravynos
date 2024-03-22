/**************************************************************************
 *
 * Copyright 2012 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * @file
 * u_debug_flush.h - Header for debugging flush- and map- related issues.
 * - Flush while synchronously mapped.
 * - Command stream reference while synchronously mapped.
 * - Synchronous map while referenced on command stream.
 * - Recursive maps.
 * - Unmap while not mapped.
 *
 * @author Thomas Hellstrom <thellstrom@vmware.com>
 */
#ifdef DEBUG

#ifndef U_DEBUG_FLUSH_H_
#define U_DEBUG_FLUSH_H_

struct debug_flush_buf;
struct debug_flush_ctx;

/**
 * Create a buffer (AKA allocation) representation.
 *
 * @param supports_persistent Whether persistent maps are truly supported.
 * @param bt_depth Depth of backtrace to be captured for this buffer
 * representation.
 */
struct debug_flush_buf *
debug_flush_buf_create(bool supports_persistent, unsigned bt_depth);

/**
 * Reference a buffer representation.
 *
 * @param dst Pointer copy destination
 * @param src Pointer copy source (may be NULL).
 *
 * Replace a pointer to a buffer representation with proper refcounting.
 */
void
debug_flush_buf_reference(struct debug_flush_buf **dst,
                          struct debug_flush_buf *src);

/**
 * Create a context representation.
 *
 * @param catch_map_of_referenced Whether to catch synchronous maps of buffers
 * already present on the command stream.
 * @param bt_depth Depth of backtrace to be captured for this context
 * representation.
 */
struct debug_flush_ctx *
debug_flush_ctx_create(bool catch_map_of_referenced, unsigned bt_depth);

/**
 * Destroy a context representation.
 *
 * @param fctx The context representation to destroy.
 */
void
debug_flush_ctx_destroy(struct debug_flush_ctx *fctx);

/**
 * Map annotation
 *
 * @param fbuf The buffer representation to map.
 * @param flags Pipebuffer flags for the map.
 *
 * Used to annotate a map of the buffer described by the buffer representation.
 */
void debug_flush_map(struct debug_flush_buf *fbuf, unsigned flags);

/**
 * Unmap annotation
 *
 * @param fbuf The buffer representation to map.
 *
 * Used to annotate an unmap of the buffer described by the
 * buffer representation.
 */
void debug_flush_unmap(struct debug_flush_buf *fbuf);

/**
 * Might flush annotation
 *
 * @param fctx The context representation that might be flushed.
 *
 * Used to annotate a conditional (possible) flush of the given context.
 */
void debug_flush_might_flush(struct debug_flush_ctx *fctx);

/**
 * Flush annotation
 *
 * @param fctx The context representation that is flushed.
 *
 * Used to annotate a real flush of the given context.
 */
void debug_flush_flush(struct debug_flush_ctx *fctx);


/**
 * Flush annotation
 *
 * @param fctx The context representation that is flushed.
 *
 * Used to annotate a real flush of the given context.
 */
void debug_flush_cb_reference(struct debug_flush_ctx *fctx,
                              struct debug_flush_buf *fbuf);

#endif
#endif
