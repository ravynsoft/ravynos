/*
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
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *      Adam Rak <adam.rak@streamnovation.com>
 */

#include "pipe/p_defines.h"
#include "pipe/p_state.h"
#include "pipe/p_context.h"
#include "util/u_blitter.h"
#include "util/list.h"
#include "util/u_transfer.h"
#include "util/u_surface.h"
#include "util/u_pack_color.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "util/u_framebuffer.h"
#include "r600_shader.h"
#include "r600_pipe.h"
#include "r600_formats.h"
#include "compute_memory_pool.h"
#include "evergreen_compute.h"
#include "evergreen_compute_internal.h"
#include <inttypes.h>

#define ITEM_ALIGNMENT 1024

/* A few forward declarations of static functions */
static void compute_memory_shadow(struct compute_memory_pool* pool,
	struct pipe_context *pipe, int device_to_host);

static void compute_memory_defrag(struct compute_memory_pool *pool,
	struct pipe_resource *src, struct pipe_resource *dst,
	struct pipe_context *pipe);

static int compute_memory_promote_item(struct compute_memory_pool *pool,
	struct compute_memory_item *item, struct pipe_context *pipe,
	int64_t allocated);

static void compute_memory_move_item(struct compute_memory_pool *pool,
	struct pipe_resource *src, struct pipe_resource *dst,
	struct compute_memory_item *item, uint64_t new_start_in_dw,
	struct pipe_context *pipe);

static void compute_memory_transfer(struct compute_memory_pool* pool,
	struct pipe_context * pipe, int device_to_host,
	struct compute_memory_item* chunk, void* data,
	int offset_in_chunk, int size);

/**
 * Creates a new pool.
 */
struct compute_memory_pool* compute_memory_pool_new(
	struct r600_screen * rscreen)
{
	struct compute_memory_pool* pool = (struct compute_memory_pool*)
				CALLOC(sizeof(struct compute_memory_pool), 1);
	if (!pool)
		return NULL;

	COMPUTE_DBG(rscreen, "* compute_memory_pool_new()\n");

	pool->screen = rscreen;
	pool->item_list = (struct list_head *)
				CALLOC(sizeof(struct list_head), 1);
	pool->unallocated_list = (struct list_head *)
				CALLOC(sizeof(struct list_head), 1);
	list_inithead(pool->item_list);
	list_inithead(pool->unallocated_list);
	return pool;
}

/**
 * Initializes the pool with a size of \a initial_size_in_dw.
 * \param pool			The pool to be initialized.
 * \param initial_size_in_dw	The initial size.
 * \see compute_memory_grow_defrag_pool
 */
static void compute_memory_pool_init(struct compute_memory_pool * pool,
	unsigned initial_size_in_dw)
{

	COMPUTE_DBG(pool->screen, "* compute_memory_pool_init() initial_size_in_dw = %u\n",
		initial_size_in_dw);

	pool->size_in_dw = initial_size_in_dw;
	pool->bo = r600_compute_buffer_alloc_vram(pool->screen,
						  pool->size_in_dw * 4);
}

/**
 * Frees all stuff in the pool and the pool struct itself too.
 */
void compute_memory_pool_delete(struct compute_memory_pool* pool)
{
	COMPUTE_DBG(pool->screen, "* compute_memory_pool_delete()\n");
	free(pool->shadow);
	r600_resource_reference(&pool->bo, NULL);
	/* In theory, all of the items were freed in compute_memory_free.
	 * Just delete the list heads
	 */
	free(pool->item_list);
	free(pool->unallocated_list);
	/* And then the pool itself */
	free(pool);
}

/**
 * Reallocates and defragments the pool, conserves data.
 * \returns -1 if it fails, 0 otherwise
 * \see compute_memory_finalize_pending
 */
static int compute_memory_grow_defrag_pool(struct compute_memory_pool *pool,
	struct pipe_context *pipe, int new_size_in_dw)
{
	new_size_in_dw = align(new_size_in_dw, ITEM_ALIGNMENT);

	COMPUTE_DBG(pool->screen, "* compute_memory_grow_defrag_pool() "
		"new_size_in_dw = %d (%d bytes)\n",
		new_size_in_dw, new_size_in_dw * 4);

	assert(new_size_in_dw >= pool->size_in_dw);

	if (!pool->bo) {
		compute_memory_pool_init(pool, MAX2(new_size_in_dw, 1024 * 16));
	} else {
		struct r600_resource *temp = NULL;

		temp = r600_compute_buffer_alloc_vram(pool->screen, new_size_in_dw * 4);

		if (temp != NULL) {
			struct pipe_resource *src = (struct pipe_resource *)pool->bo;
			struct pipe_resource *dst = (struct pipe_resource *)temp;

			COMPUTE_DBG(pool->screen, "  Growing and defragmenting the pool "
					"using a temporary resource\n");

			compute_memory_defrag(pool, src, dst, pipe);

			/* Release the old buffer */
			r600_resource_reference(&pool->bo, NULL);
			pool->bo = temp;
			pool->size_in_dw = new_size_in_dw;
		}
		else {
			COMPUTE_DBG(pool->screen, "  The creation of the temporary resource failed\n"
				"  Falling back to using 'shadow'\n");

			compute_memory_shadow(pool, pipe, 1);
			pool->shadow = realloc(pool->shadow, new_size_in_dw * 4);
			if (pool->shadow == NULL)
				return -1;

			pool->size_in_dw = new_size_in_dw;
			/* Release the old buffer */
			r600_resource_reference(&pool->bo, NULL);
			pool->bo = r600_compute_buffer_alloc_vram(pool->screen, pool->size_in_dw * 4);
			compute_memory_shadow(pool, pipe, 0);

			if (pool->status & POOL_FRAGMENTED) {
				struct pipe_resource *src = (struct pipe_resource *)pool->bo;
				compute_memory_defrag(pool, src, src, pipe);
			}
		}
	}

	return 0;
}

/**
 * Copy pool from device to host, or host to device.
 * \param device_to_host 1 for device->host, 0 for host->device
 * \see compute_memory_grow_defrag_pool
 */
static void compute_memory_shadow(struct compute_memory_pool* pool,
	struct pipe_context * pipe, int device_to_host)
{
	struct compute_memory_item chunk;

	COMPUTE_DBG(pool->screen, "* compute_memory_shadow() device_to_host = %d\n",
		device_to_host);

	chunk.id = 0;
	chunk.start_in_dw = 0;
	chunk.size_in_dw = pool->size_in_dw;
	compute_memory_transfer(pool, pipe, device_to_host, &chunk,
				pool->shadow, 0, pool->size_in_dw*4);
}

/**
 * Moves all the items marked for promotion from the \a unallocated_list
 * to the \a item_list.
 * \return -1 if it fails, 0 otherwise
 * \see evergreen_set_global_binding
 */
int compute_memory_finalize_pending(struct compute_memory_pool* pool,
	struct pipe_context * pipe)
{
	struct compute_memory_item *item, *next;

	int64_t allocated = 0;
	int64_t unallocated = 0;
	int64_t last_pos;

	int err = 0;

	COMPUTE_DBG(pool->screen, "* compute_memory_finalize_pending()\n");

	LIST_FOR_EACH_ENTRY(item, pool->item_list, link) {
		COMPUTE_DBG(pool->screen, "  + list: offset = %"PRIi64" id = %"PRIi64" size = %"PRIi64" "
			"(%"PRIi64" bytes)\n", item->start_in_dw, item->id,
			item->size_in_dw, item->size_in_dw * 4);
	}

	/* Calculate the total allocated size */
	LIST_FOR_EACH_ENTRY(item, pool->item_list, link) {
		allocated += align(item->size_in_dw, ITEM_ALIGNMENT);
	}

	/* Calculate the total unallocated size of the items that
	 * will be promoted to the pool */
	LIST_FOR_EACH_ENTRY(item, pool->unallocated_list, link) {
		if (item->status & ITEM_FOR_PROMOTING)
			unallocated += align(item->size_in_dw, ITEM_ALIGNMENT);
	}

	if (unallocated == 0) {
		return 0;
	}

	if (pool->size_in_dw < allocated + unallocated) {
		err = compute_memory_grow_defrag_pool(pool, pipe, allocated + unallocated);
		if (err == -1)
			return -1;
	}
	else if (pool->status & POOL_FRAGMENTED) {
		/* Loop through all unallocated items marked for promoting to
		 * insert them into an appropriate existing hole prior to defrag. */
		LIST_FOR_EACH_ENTRY_SAFE(item, next, pool->unallocated_list, link) {
			if (!(item->status & ITEM_FOR_PROMOTING))
				continue;

			int64_t hole_start = 0, hole_size = 0;
			int64_t item_size = align(item->size_in_dw, ITEM_ALIGNMENT);
			struct compute_memory_item *alloc_item, *alloc_next;
			LIST_FOR_EACH_ENTRY_SAFE(alloc_item, alloc_next, pool->item_list, link) {
				if (alloc_item->start_in_dw == hole_start) {
					hole_start += align(alloc_item->size_in_dw, ITEM_ALIGNMENT);
					hole_size = 0;
				} else if (alloc_item->start_in_dw > hole_start) {
					hole_size = alloc_item->start_in_dw - hole_start;
				}
			}

			/* Space after all items is also a hole. */
			if (hole_size == 0 && hole_start < pool->size_in_dw)
				hole_size = pool->size_in_dw - hole_start;

			if (hole_size >= item_size) {
				if (compute_memory_promote_item(pool, item, pipe, hole_start) != -1) {
					item->status &= ~ITEM_FOR_PROMOTING;
					unallocated -= item_size;
					allocated += item_size;
				}
			}
		}

		if (allocated == pool->size_in_dw)
			pool->status &= ~POOL_FRAGMENTED;

		if (unallocated == 0)
			return 0;

		struct pipe_resource *src = (struct pipe_resource *)pool->bo;
		compute_memory_defrag(pool, src, src, pipe);
	}

	/* After defragmenting the pool, allocated is equal to the first available
	 * position for new items in the pool */
	last_pos = allocated;

	/* Loop through all the unallocated items, check if they are marked
	 * for promoting, allocate space for them and add them to the item_list. */
	LIST_FOR_EACH_ENTRY_SAFE(item, next, pool->unallocated_list, link) {
		if (item->status & ITEM_FOR_PROMOTING) {
			err = compute_memory_promote_item(pool, item, pipe, last_pos);
			item->status &= ~ITEM_FOR_PROMOTING;

			last_pos += align(item->size_in_dw, ITEM_ALIGNMENT);

			if (err == -1)
				return -1;
		}
	}

	return 0;
}

/**
 * Defragments the pool, so that there's no gap between items.
 * \param pool	The pool to be defragmented
 * \param src	The origin resource
 * \param dst	The destination resource
 * \see compute_memory_grow_defrag_pool and compute_memory_finalize_pending
 */
static void compute_memory_defrag(struct compute_memory_pool *pool,
	struct pipe_resource *src, struct pipe_resource *dst,
	struct pipe_context *pipe)
{
	struct compute_memory_item *item;
	int64_t last_pos;

	COMPUTE_DBG(pool->screen, "* compute_memory_defrag()\n");

	last_pos = 0;
	LIST_FOR_EACH_ENTRY(item, pool->item_list, link) {
		if (src != dst || item->start_in_dw != last_pos) {
			assert(last_pos <= item->start_in_dw);

			compute_memory_move_item(pool, src, dst,
					item, last_pos, pipe);
		}

		last_pos += align(item->size_in_dw, ITEM_ALIGNMENT);
	}

	pool->status &= ~POOL_FRAGMENTED;
}

/**
 * Moves an item from the \a unallocated_list to the \a item_list.
 * \param item	The item that will be promoted.
 * \return -1 if it fails, 0 otherwise
 * \see compute_memory_finalize_pending
 */
static int compute_memory_promote_item(struct compute_memory_pool *pool,
		struct compute_memory_item *item, struct pipe_context *pipe,
		int64_t start_in_dw)
{
	struct pipe_screen *screen = (struct pipe_screen *)pool->screen;
	struct r600_context *rctx = (struct r600_context *)pipe;
	struct pipe_resource *src = (struct pipe_resource *)item->real_buffer;
	struct pipe_resource *dst = (struct pipe_resource *)pool->bo;
	struct pipe_box box;

	COMPUTE_DBG(pool->screen, "* compute_memory_promote_item()\n"
			"  + Promoting Item: %"PRIi64" , starting at: %"PRIi64" (%"PRIi64" bytes) "
			"size: %"PRIi64" (%"PRIi64" bytes)\n\t\t\tnew start: %"PRIi64" (%"PRIi64" bytes)\n",
			item->id, item->start_in_dw, item->start_in_dw * 4,
			item->size_in_dw, item->size_in_dw * 4,
			start_in_dw, start_in_dw * 4);

	/* Remove the item from the unallocated list */
	list_del(&item->link);

	/* Add it back to the item_list */
	list_addtail(&item->link, pool->item_list);
	item->start_in_dw = start_in_dw;

	if (src) {
		u_box_1d(0, item->size_in_dw * 4, &box);

		rctx->b.b.resource_copy_region(pipe,
				dst, 0, item->start_in_dw * 4, 0 ,0,
				src, 0, &box);

		/* We check if the item is mapped for reading.
		 * In this case, we need to keep the temporary buffer 'alive'
		 * because it is possible to keep a map active for reading
		 * while a kernel (that reads from it) executes */
		if (!(item->status & ITEM_MAPPED_FOR_READING) && !is_item_user_ptr(item)) {
			pool->screen->b.b.resource_destroy(screen, src);
			item->real_buffer = NULL;
		}
	}

	return 0;
}

/**
 * Moves an item from the \a item_list to the \a unallocated_list.
 * \param item	The item that will be demoted
 * \see r600_compute_global_transfer_map
 */
void compute_memory_demote_item(struct compute_memory_pool *pool,
	struct compute_memory_item *item, struct pipe_context *pipe)
{
	struct r600_context *rctx = (struct r600_context *)pipe;
	struct pipe_resource *src = (struct pipe_resource *)pool->bo;
	struct pipe_resource *dst;
	struct pipe_box box;

	COMPUTE_DBG(pool->screen, "* compute_memory_demote_item()\n"
			"  + Demoting Item: %"PRIi64", starting at: %"PRIi64" (%"PRIi64" bytes) "
			"size: %"PRIi64" (%"PRIi64" bytes)\n", item->id, item->start_in_dw,
			item->start_in_dw * 4, item->size_in_dw, item->size_in_dw * 4);

	/* First, we remove the item from the item_list */
	list_del(&item->link);

	/* Now we add it to the unallocated list */
	list_addtail(&item->link, pool->unallocated_list);

	/* We check if the intermediate buffer exists, and if it
	 * doesn't, we create it again */
	if (item->real_buffer == NULL) {
		item->real_buffer = r600_compute_buffer_alloc_vram(
				pool->screen, item->size_in_dw * 4);
	}

	dst = (struct pipe_resource *)item->real_buffer;

	/* We transfer the memory from the item in the pool to the
	 * temporary buffer. Download is skipped for items:
	 * - Not mapped for reading or writing (PIPE_MAP_DISCARD_RANGE).
	 * - Not writable by the device. */
	if ((item->status & (ITEM_MAPPED_FOR_READING|ITEM_MAPPED_FOR_WRITING)) &&
		!(r600_resource(dst)->flags & RADEON_FLAG_READ_ONLY)) {

		u_box_1d(item->start_in_dw * 4, item->size_in_dw * 4, &box);

		rctx->b.b.resource_copy_region(pipe,
			dst, 0, 0, 0, 0,
			src, 0, &box);
	}

	/* Remember to mark the buffer as 'pending' by setting start_in_dw to -1 */
	item->start_in_dw = -1;

	if (item->link.next != pool->item_list) {
		pool->status |= POOL_FRAGMENTED;
	}
}

/**
 * Moves the item \a item forward from the resource \a src to the
 * resource \a dst at \a new_start_in_dw
 *
 * This function assumes two things:
 * 1) The item is \b only moved forward, unless src is different from dst
 * 2) The item \b won't change it's position inside the \a item_list
 *
 * \param item			The item that will be moved
 * \param new_start_in_dw	The new position of the item in \a item_list
 * \see compute_memory_defrag
 */
static void compute_memory_move_item(struct compute_memory_pool *pool,
	struct pipe_resource *src, struct pipe_resource *dst,
	struct compute_memory_item *item, uint64_t new_start_in_dw,
	struct pipe_context *pipe)
{
	struct pipe_screen *screen = (struct pipe_screen *)pool->screen;
	struct r600_context *rctx = (struct r600_context *)pipe;
	struct pipe_box box;

	COMPUTE_DBG(pool->screen, "* compute_memory_move_item()\n"
			"  + Moving item %"PRIi64" from %"PRIi64" (%"PRIi64" bytes) to %"PRIu64" (%"PRIu64" bytes)\n",
			item->id, item->start_in_dw, item->start_in_dw * 4,
			new_start_in_dw, new_start_in_dw * 4);

	if (pool->item_list != item->link.prev) {
		ASSERTED struct compute_memory_item *prev;
		prev = container_of(item->link.prev, struct compute_memory_item, link);
		assert(prev->start_in_dw + prev->size_in_dw <= new_start_in_dw);
	}

	u_box_1d(item->start_in_dw * 4, item->size_in_dw * 4, &box);

	/* If the ranges don't overlap, or we are copying from one resource
	 * to another, we can just copy the item directly */
	if (src != dst || new_start_in_dw + item->size_in_dw <= item->start_in_dw) {

		rctx->b.b.resource_copy_region(pipe,
			dst, 0, new_start_in_dw * 4, 0, 0,
			src, 0, &box);
	} else {
		/* The ranges overlap, we will try first to use an intermediate
		 * resource to move the item */
		struct pipe_resource *tmp = (struct pipe_resource *)
			r600_compute_buffer_alloc_vram(pool->screen, item->size_in_dw * 4);

		if (tmp != NULL) {
			rctx->b.b.resource_copy_region(pipe,
				tmp, 0, 0, 0, 0,
				src, 0, &box);

			box.x = 0;

			rctx->b.b.resource_copy_region(pipe,
				dst, 0, new_start_in_dw * 4, 0, 0,
				tmp, 0, &box);

			pool->screen->b.b.resource_destroy(screen, tmp);

		} else {
			/* The allocation of the temporary resource failed,
			 * falling back to use mappings */
			uint32_t *map;
			int64_t offset;
			struct pipe_transfer *trans;

			offset = item->start_in_dw - new_start_in_dw;

			u_box_1d(new_start_in_dw * 4, (offset + item->size_in_dw) * 4, &box);

			map = pipe->buffer_map(pipe, src, 0, PIPE_MAP_READ_WRITE,
				&box, &trans);

			assert(map);
			assert(trans);

			memmove(map, map + offset, item->size_in_dw * 4);

			pipe->buffer_unmap(pipe, trans);
		}
	}

	item->start_in_dw = new_start_in_dw;
}

/**
 * Frees one item for compute_memory_free()
 */
static void compute_memory_free_item(struct pipe_screen *screen,
	struct compute_memory_item *item)
{
	struct pipe_resource *res = (struct pipe_resource *)item->real_buffer;

	list_del(&item->link);

	if (res && !is_item_user_ptr(item))
		screen->resource_destroy(screen, res);

	free(item);
}

/**
 * Frees the memory associated to the item with id \a id from the pool.
 * \param id	The id of the item to be freed.
 */
void compute_memory_free(struct compute_memory_pool* pool, int64_t id)
{
	struct compute_memory_item *item, *next;
	struct pipe_screen *screen = (struct pipe_screen *)pool->screen;

	COMPUTE_DBG(pool->screen, "* compute_memory_free() id + %"PRIi64" \n", id);

	LIST_FOR_EACH_ENTRY_SAFE(item, next, pool->item_list, link) {
		if (item->id == id) {
			if (item->link.next != pool->item_list) {
				pool->status |= POOL_FRAGMENTED;
			}

			compute_memory_free_item(screen, item);
			return;
		}
	}

	LIST_FOR_EACH_ENTRY_SAFE(item, next, pool->unallocated_list, link) {
		if (item->id == id) {
			compute_memory_free_item(screen, item);
			return;
		}
	}

	fprintf(stderr, "Internal error, invalid id %"PRIi64" "
		"for compute_memory_free\n", id);

	assert(0 && "error");
}

/**
 * Creates pending allocations for new items, these items are
 * placed in the unallocated_list.
 * \param size_in_dw	The size, in double words, of the new item.
 * \return The new item
 * \see r600_compute_global_buffer_create
 */
struct compute_memory_item* compute_memory_alloc(
	struct compute_memory_pool* pool,
	int64_t size_in_dw)
{
	struct compute_memory_item *new_item = NULL;

	COMPUTE_DBG(pool->screen, "* compute_memory_alloc() size_in_dw = %"PRIi64" (%"PRIi64" bytes)\n",
			size_in_dw, 4 * size_in_dw);

	new_item = (struct compute_memory_item *)
				CALLOC(sizeof(struct compute_memory_item), 1);
	if (!new_item)
		return NULL;

	new_item->size_in_dw = size_in_dw;
	new_item->start_in_dw = -1; /* mark pending */
	new_item->id = pool->next_id++;
	new_item->pool = pool;
	new_item->real_buffer = NULL;

	list_addtail(&new_item->link, pool->unallocated_list);

	COMPUTE_DBG(pool->screen, "  + Adding item %p id = %"PRIi64" size = %"PRIi64" (%"PRIi64" bytes)\n",
			new_item, new_item->id, new_item->size_in_dw,
			new_item->size_in_dw * 4);
	return new_item;
}

/**
 * Transfer data host<->device, offset and size is in bytes.
 * \param device_to_host 1 for device->host, 0 for host->device.
 * \see compute_memory_shadow
 */
static void compute_memory_transfer(
	struct compute_memory_pool* pool,
	struct pipe_context * pipe,
	int device_to_host,
	struct compute_memory_item* chunk,
	void* data,
	int offset_in_chunk,
	int size)
{
	int64_t aligned_size = pool->size_in_dw;
	struct pipe_resource* gart = (struct pipe_resource*)pool->bo;
	int64_t internal_offset = chunk->start_in_dw*4 + offset_in_chunk;

	struct pipe_transfer *xfer;
	uint32_t *map;

	assert(gart);

	COMPUTE_DBG(pool->screen, "* compute_memory_transfer() device_to_host = %d, "
		"offset_in_chunk = %d, size = %d\n", device_to_host,
		offset_in_chunk, size);

	if (device_to_host) {
		map = pipe->buffer_map(pipe, gart, 0, PIPE_MAP_READ,
			&(struct pipe_box) { .width = aligned_size * 4,
			.height = 1, .depth = 1 }, &xfer);
		assert(xfer);
		assert(map);
		memcpy(data, map + internal_offset, size);
		pipe->buffer_unmap(pipe, xfer);
	} else {
		map = pipe->buffer_map(pipe, gart, 0, PIPE_MAP_WRITE,
			&(struct pipe_box) { .width = aligned_size * 4,
			.height = 1, .depth = 1 }, &xfer);
		assert(xfer);
		assert(map);
		memcpy(map + internal_offset, data, size);
		pipe->buffer_unmap(pipe, xfer);
	}
}
