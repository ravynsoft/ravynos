#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <drm_fourcc.h>
#include <vulkan/vulkan.h>
#include <wlr/render/interface.h>
#include <wlr/types/wlr_drm.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>
#include <wlr/render/vulkan.h>
#include <wlr/backend/interface.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>

#include "render/pixel_format.h"
#include "render/vulkan.h"
#include "render/vulkan/shaders/common.vert.h"
#include "render/vulkan/shaders/texture.frag.h"
#include "render/vulkan/shaders/quad.frag.h"
#include "types/wlr_buffer.h"

// TODO:
// - simplify stage allocation, don't track allocations but use ringbuffer-like
// - use a pipeline cache (not sure when to save though, after every pipeline
//   creation?)
// - create pipelines as derivatives of each other
// - evaluate if creating VkDeviceMemory pools is a good idea.
//   We can expect wayland client images to be fairly large (and shouldn't
//   have more than 4k of those I guess) but pooling memory allocations
//   might still be a good idea.

static const VkDeviceSize min_stage_size = 1024 * 1024; // 1MB
static const VkDeviceSize max_stage_size = 64 * min_stage_size; // 64MB
static const size_t start_descriptor_pool_size = 256u;
static bool default_debug = true;

static const struct wlr_renderer_impl renderer_impl;

struct wlr_vk_renderer *vulkan_get_renderer(struct wlr_renderer *wlr_renderer) {
	assert(wlr_renderer->impl == &renderer_impl);
	return (struct wlr_vk_renderer *)wlr_renderer;
}

static struct wlr_vk_render_format_setup *find_or_create_render_setup(
		struct wlr_vk_renderer *renderer, VkFormat format);

// vertex shader push constant range data
struct vert_pcr_data {
	float mat4[4][4];
	float uv_off[2];
	float uv_size[2];
};

// https://www.w3.org/Graphics/Color/srgb
static float color_to_linear(float non_linear) {
	return (non_linear > 0.04045) ?
		pow((non_linear + 0.055) / 1.055, 2.4) :
		non_linear / 12.92;
}

// renderer
// util
static void mat3_to_mat4(const float mat3[9], float mat4[4][4]) {
	memset(mat4, 0, sizeof(float) * 16);
	mat4[0][0] = mat3[0];
	mat4[0][1] = mat3[1];
	mat4[0][3] = mat3[2];

	mat4[1][0] = mat3[3];
	mat4[1][1] = mat3[4];
	mat4[1][3] = mat3[5];

	mat4[2][2] = 1.f;
	mat4[3][3] = 1.f;
}

struct wlr_vk_descriptor_pool *vulkan_alloc_texture_ds(
		struct wlr_vk_renderer *renderer, VkDescriptorSet *ds) {
	VkResult res;
	VkDescriptorSetAllocateInfo ds_info = {0};
	ds_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ds_info.descriptorSetCount = 1;
	ds_info.pSetLayouts = &renderer->ds_layout;

	bool found = false;
	struct wlr_vk_descriptor_pool *pool;
	wl_list_for_each(pool, &renderer->descriptor_pools, link) {
		if (pool->free > 0) {
			found = true;
			break;
		}
	}

	if (!found) { // create new pool
		pool = calloc(1, sizeof(*pool));
		if (!pool) {
			wlr_log_errno(WLR_ERROR, "allocation failed");
			return NULL;
		}

		size_t count = renderer->last_pool_size;
		if (!count) {
			count = start_descriptor_pool_size;
		}

		pool->free = count;
		VkDescriptorPoolSize pool_size = {0};
		pool_size.descriptorCount = count;
		pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolCreateInfo dpool_info = {0};
		dpool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpool_info.maxSets = count;
		dpool_info.poolSizeCount = 1;
		dpool_info.pPoolSizes = &pool_size;
		dpool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		res = vkCreateDescriptorPool(renderer->dev->dev, &dpool_info, NULL,
			&pool->pool);
		if (res != VK_SUCCESS) {
			wlr_vk_error("vkCreateDescriptorPool", res);
			free(pool);
			return NULL;
		}

		wl_list_insert(&renderer->descriptor_pools, &pool->link);
	}

	ds_info.descriptorPool = pool->pool;
	res = vkAllocateDescriptorSets(renderer->dev->dev, &ds_info, ds);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkAllocateDescriptorSets", res);
		return NULL;
	}

	--pool->free;
	return pool;
}

void vulkan_free_ds(struct wlr_vk_renderer *renderer,
		struct wlr_vk_descriptor_pool *pool, VkDescriptorSet ds) {
	vkFreeDescriptorSets(renderer->dev->dev, pool->pool, 1, &ds);
	++pool->free;
}

static void destroy_render_format_setup(struct wlr_vk_renderer *renderer,
		struct wlr_vk_render_format_setup *setup) {
	if (!setup) {
		return;
	}

	VkDevice dev = renderer->dev->dev;
	vkDestroyRenderPass(dev, setup->render_pass, NULL);
	vkDestroyPipeline(dev, setup->tex_pipe, NULL);
	vkDestroyPipeline(dev, setup->quad_pipe, NULL);
}

static void shared_buffer_destroy(struct wlr_vk_renderer *r,
		struct wlr_vk_shared_buffer *buffer) {
	if (!buffer) {
		return;
	}

	if (buffer->allocs_size > 0) {
		wlr_log(WLR_ERROR, "shared_buffer_finish: %d allocations left",
			(unsigned) buffer->allocs_size);
	}

	free(buffer->allocs);
	if (buffer->buffer) {
		vkDestroyBuffer(r->dev->dev, buffer->buffer, NULL);
	}
	if (buffer->memory) {
		vkFreeMemory(r->dev->dev, buffer->memory, NULL);
	}

	wl_list_remove(&buffer->link);
	free(buffer);
}

static void release_stage_allocations(struct wlr_vk_renderer *renderer) {
	struct wlr_vk_shared_buffer *buf;
	wl_list_for_each(buf, &renderer->stage.buffers, link) {
		buf->allocs_size = 0u;
	}
}

struct wlr_vk_buffer_span vulkan_get_stage_span(struct wlr_vk_renderer *r,
		VkDeviceSize size) {
	// try to find free span
	// simple greedy allocation algorithm - should be enough for this usecase
	// since all allocations are freed together after the frame
	struct wlr_vk_shared_buffer *buf;
	wl_list_for_each_reverse(buf, &r->stage.buffers, link) {
		VkDeviceSize start = 0u;
		if (buf->allocs_size > 0) {
			struct wlr_vk_allocation *last = &buf->allocs[buf->allocs_size - 1];
			start = last->start + last->size;
		}

		assert(start <= buf->buf_size);
		if (buf->buf_size - start < size) {
			continue;
		}

		++buf->allocs_size;
		if (buf->allocs_size > buf->allocs_capacity) {
			buf->allocs_capacity = buf->allocs_size * 2;
			void *allocs = realloc(buf->allocs,
				buf->allocs_capacity * sizeof(*buf->allocs));
			if (!allocs) {
				wlr_log_errno(WLR_ERROR, "Allocation failed");
				goto error_alloc;
			}

			buf->allocs = allocs;
		}

		struct wlr_vk_allocation *a = &buf->allocs[buf->allocs_size - 1];
		a->start = start;
		a->size = size;
		return (struct wlr_vk_buffer_span) {
			.buffer = buf,
			.alloc = *a,
		};
	}

	// we didn't find a free buffer - create one
	// size = clamp(max(size * 2, prev_size * 2), min_size, max_size)
	VkDeviceSize bsize = size * 2;
	bsize = bsize < min_stage_size ? min_stage_size : bsize;
	if (!wl_list_empty(&r->stage.buffers)) {
		struct wl_list *last_link = r->stage.buffers.prev;
		struct wlr_vk_shared_buffer *prev = wl_container_of(
			last_link, prev, link);
		VkDeviceSize last_size = 2 * prev->buf_size;
		bsize = bsize < last_size ? last_size : bsize;
	}

	if (bsize > max_stage_size) {
		wlr_log(WLR_INFO, "vulkan stage buffers have reached max size");
		bsize = max_stage_size;
	}

	// create buffer
	buf = calloc(1, sizeof(*buf));
	if (!buf) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		goto error_alloc;
	}

	VkResult res;
	VkBufferCreateInfo buf_info = {0};
	buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buf_info.size = bsize;
	buf_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	res = vkCreateBuffer(r->dev->dev, &buf_info, NULL, &buf->buffer);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateBuffer", res);
		goto error;
	}

	VkMemoryRequirements mem_reqs;
	vkGetBufferMemoryRequirements(r->dev->dev, buf->buffer, &mem_reqs);

	VkMemoryAllocateInfo mem_info = {0};
	mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_info.allocationSize = mem_reqs.size;
	mem_info.memoryTypeIndex = vulkan_find_mem_type(r->dev,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, mem_reqs.memoryTypeBits);
	res = vkAllocateMemory(r->dev->dev, &mem_info, NULL, &buf->memory);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkAllocatorMemory", res);
		goto error;
	}

	res = vkBindBufferMemory(r->dev->dev, buf->buffer, buf->memory, 0);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkBindBufferMemory", res);
		goto error;
	}

	size_t start_count = 8u;
	buf->allocs = calloc(start_count, sizeof(*buf->allocs));
	if (!buf->allocs) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		goto error;
	}

	wlr_log(WLR_DEBUG, "Created new vk staging buffer of size %" PRIu64, bsize);
	buf->buf_size = bsize;
	wl_list_insert(&r->stage.buffers, &buf->link);

	buf->allocs_capacity = start_count;
	buf->allocs_size = 1u;
	buf->allocs[0].start = 0u;
	buf->allocs[0].size = size;
	return (struct wlr_vk_buffer_span) {
		.buffer = buf,
		.alloc = buf->allocs[0],
	};

error:
	shared_buffer_destroy(r, buf);

error_alloc:
	return (struct wlr_vk_buffer_span) {
		.buffer = NULL,
		.alloc = (struct wlr_vk_allocation) {0, 0},
	};
}

VkCommandBuffer vulkan_record_stage_cb(struct wlr_vk_renderer *renderer) {
	if (!renderer->stage.recording) {
		VkCommandBufferBeginInfo begin_info = {0};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkBeginCommandBuffer(renderer->stage.cb, &begin_info);
		renderer->stage.recording = true;
	}

	return renderer->stage.cb;
}

bool vulkan_submit_stage_wait(struct wlr_vk_renderer *renderer) {
	if (!renderer->stage.recording) {
		return false;
	}

	vkEndCommandBuffer(renderer->stage.cb);
	renderer->stage.recording = false;

	VkSubmitInfo submit_info = {0};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1u;
	submit_info.pCommandBuffers = &renderer->stage.cb;
	VkResult res = vkQueueSubmit(renderer->dev->queue, 1,
		&submit_info, renderer->fence);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkQueueSubmit", res);
		return false;
	}

	res = vkWaitForFences(renderer->dev->dev, 1, &renderer->fence, true,
		UINT64_MAX);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkWaitForFences", res);
		return false;
	}

	// NOTE: don't release stage allocations here since they may still be
	// used for reading. Will be done next frame.
	res = vkResetFences(renderer->dev->dev, 1, &renderer->fence);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkResetFences", res);
		return false;
	}

	return true;
}

struct wlr_vk_format_props *vulkan_format_props_from_drm(
		struct wlr_vk_device *dev, uint32_t drm_fmt) {
	for (size_t i = 0u; i < dev->format_prop_count; ++i) {
		if (dev->format_props[i].format.drm_format == drm_fmt) {
			return &dev->format_props[i];
		}
	}
	return NULL;
}

// buffer import
static void destroy_render_buffer(struct wlr_vk_render_buffer *buffer) {
	wl_list_remove(&buffer->link);
	wl_list_remove(&buffer->buffer_destroy.link);

	assert(buffer->renderer->current_render_buffer != buffer);

	VkDevice dev = buffer->renderer->dev->dev;

	vkDestroyFramebuffer(dev, buffer->framebuffer, NULL);
	vkDestroyImageView(dev, buffer->image_view, NULL);
	vkDestroyImage(dev, buffer->image, NULL);

	for (size_t i = 0u; i < buffer->mem_count; ++i) {
		vkFreeMemory(dev, buffer->memories[i], NULL);
	}

	free(buffer);
}

static struct wlr_vk_render_buffer *get_render_buffer(
		struct wlr_vk_renderer *renderer, struct wlr_buffer *wlr_buffer) {
	struct wlr_vk_render_buffer *buffer;
	wl_list_for_each(buffer, &renderer->render_buffers, link) {
		if (buffer->wlr_buffer == wlr_buffer) {
			return buffer;
		}
	}
	return NULL;
}

static void handle_render_buffer_destroy(struct wl_listener *listener, void *data) {
	struct wlr_vk_render_buffer *buffer =
		wl_container_of(listener, buffer, buffer_destroy);
	destroy_render_buffer(buffer);
}

static struct wlr_vk_render_buffer *create_render_buffer(
		struct wlr_vk_renderer *renderer, struct wlr_buffer *wlr_buffer) {
	VkResult res;

	struct wlr_vk_render_buffer *buffer = calloc(1, sizeof(*buffer));
	if (buffer == NULL) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}
	buffer->wlr_buffer = wlr_buffer;
	buffer->renderer = renderer;

	struct wlr_dmabuf_attributes dmabuf = {0};
	if (!wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf)) {
		goto error_buffer;
	}

	wlr_log(WLR_DEBUG, "vulkan create_render_buffer: %.4s, %dx%d",
		(const char*) &dmabuf.format, dmabuf.width, dmabuf.height);

	buffer->image = vulkan_import_dmabuf(renderer, &dmabuf,
		buffer->memories, &buffer->mem_count, true);
	if (!buffer->image) {
		goto error_buffer;
	}

	VkDevice dev = renderer->dev->dev;
	const struct wlr_vk_format_props *fmt = vulkan_format_props_from_drm(
		renderer->dev, dmabuf.format);
	if (fmt == NULL) {
		wlr_log(WLR_ERROR, "Unsupported pixel format %"PRIx32 " (%.4s)",
			dmabuf.format, (const char*) &dmabuf.format);
		goto error_buffer;
	}

	VkImageViewCreateInfo view_info = {0};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = buffer->image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = fmt->format.vk_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange = (VkImageSubresourceRange) {
		VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
	};

	res = vkCreateImageView(dev, &view_info, NULL, &buffer->image_view);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateImageView failed", res);
		goto error_view;
	}

	buffer->render_setup = find_or_create_render_setup(
		renderer, fmt->format.vk_format);
	if (!buffer->render_setup) {
		goto error_view;
	}

	VkFramebufferCreateInfo fb_info = {0};
	fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.attachmentCount = 1u;
	fb_info.pAttachments = &buffer->image_view;
	fb_info.flags = 0u;
	fb_info.width = dmabuf.width;
	fb_info.height = dmabuf.height;
	fb_info.layers = 1u;
	fb_info.renderPass = buffer->render_setup->render_pass;

	res = vkCreateFramebuffer(dev, &fb_info, NULL, &buffer->framebuffer);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateFramebuffer", res);
		goto error_view;
	}

	buffer->buffer_destroy.notify = handle_render_buffer_destroy;
	wl_signal_add(&wlr_buffer->events.destroy, &buffer->buffer_destroy);
	wl_list_insert(&renderer->render_buffers, &buffer->link);

	return buffer;

error_view:
	vkDestroyFramebuffer(dev, buffer->framebuffer, NULL);
	vkDestroyImageView(dev, buffer->image_view, NULL);
	vkDestroyImage(dev, buffer->image, NULL);
	for (size_t i = 0u; i < buffer->mem_count; ++i) {
		vkFreeMemory(dev, buffer->memories[i], NULL);
	}
error_buffer:
	wlr_dmabuf_attributes_finish(&dmabuf);
	free(buffer);
	return NULL;
}

// interface implementation
static bool vulkan_bind_buffer(struct wlr_renderer *wlr_renderer,
		struct wlr_buffer *wlr_buffer) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);

	if (renderer->current_render_buffer) {
		wlr_buffer_unlock(renderer->current_render_buffer->wlr_buffer);
		renderer->current_render_buffer = NULL;
	}

	if (!wlr_buffer) {
		return true;
	}

	struct wlr_vk_render_buffer *buffer = get_render_buffer(renderer, wlr_buffer);
	if (!buffer) {
		buffer = create_render_buffer(renderer, wlr_buffer);
		if (!buffer) {
			return false;
		}
	}

	wlr_buffer_lock(wlr_buffer);
	renderer->current_render_buffer = buffer;
	return true;
}

static void vulkan_begin(struct wlr_renderer *wlr_renderer,
		uint32_t width, uint32_t height) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	assert(renderer->current_render_buffer);

	VkCommandBuffer cb = renderer->cb;
	VkCommandBufferBeginInfo begin_info = {0};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(cb, &begin_info);

	// begin render pass
	VkFramebuffer fb = renderer->current_render_buffer->framebuffer;

	VkRect2D rect = {{0, 0}, {width, height}};
	renderer->scissor = rect;

	VkRenderPassBeginInfo rp_info = {0};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rp_info.renderArea = rect;
	rp_info.renderPass = renderer->current_render_buffer->render_setup->render_pass;
	rp_info.framebuffer = fb;
	rp_info.clearValueCount = 0;
	vkCmdBeginRenderPass(cb, &rp_info, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport vp = {0.f, 0.f, (float) width, (float) height, 0.f, 1.f};
	vkCmdSetViewport(cb, 0, 1, &vp);
	vkCmdSetScissor(cb, 0, 1, &rect);

	// Refresh projection matrix.
	// wlr_matrix_projection assumes a GL corrdinate system so we need
	// to pass WL_OUTPUT_TRANSFORM_FLIPPED_180 to adjust it for vulkan.
	wlr_matrix_projection(renderer->projection, width, height,
		WL_OUTPUT_TRANSFORM_FLIPPED_180);

	renderer->render_width = width;
	renderer->render_height = height;
	renderer->bound_pipe = VK_NULL_HANDLE;
}

static void vulkan_end(struct wlr_renderer *wlr_renderer) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	assert(renderer->current_render_buffer);

	VkCommandBuffer render_cb = renderer->cb;
	VkCommandBuffer pre_cb = vulkan_record_stage_cb(renderer);

	renderer->render_width = 0u;
	renderer->render_height = 0u;
	renderer->bound_pipe = VK_NULL_HANDLE;

	vkCmdEndRenderPass(render_cb);

	// insert acquire and release barriers for dmabuf-images
	unsigned barrier_count = wl_list_length(&renderer->foreign_textures) + 1;
	VkImageMemoryBarrier* acquire_barriers = calloc(barrier_count, sizeof(VkImageMemoryBarrier));
	VkImageMemoryBarrier* release_barriers = calloc(barrier_count, sizeof(VkImageMemoryBarrier));

	struct wlr_vk_texture *texture, *tmp_tex;
	unsigned idx = 0;

	wl_list_for_each_safe(texture, tmp_tex, &renderer->foreign_textures, foreign_link) {
		VkImageLayout src_layout = VK_IMAGE_LAYOUT_GENERAL;
		if (!texture->transitioned) {
			src_layout = VK_IMAGE_LAYOUT_UNDEFINED;
			texture->transitioned = true;
		}

		// acquire
		acquire_barriers[idx].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		acquire_barriers[idx].srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
		acquire_barriers[idx].dstQueueFamilyIndex = renderer->dev->queue_family;
		acquire_barriers[idx].image = texture->image;
		acquire_barriers[idx].oldLayout = src_layout;
		acquire_barriers[idx].newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		acquire_barriers[idx].srcAccessMask = 0u; // ignored anyways
		acquire_barriers[idx].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		acquire_barriers[idx].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		acquire_barriers[idx].subresourceRange.layerCount = 1;
		acquire_barriers[idx].subresourceRange.levelCount = 1;

		// releaes
		release_barriers[idx].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		release_barriers[idx].srcQueueFamilyIndex = renderer->dev->queue_family;
		release_barriers[idx].dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
		release_barriers[idx].image = texture->image;
		release_barriers[idx].oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		release_barriers[idx].newLayout = VK_IMAGE_LAYOUT_GENERAL;
		release_barriers[idx].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		release_barriers[idx].dstAccessMask = 0u; // ignored anyways
		release_barriers[idx].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		release_barriers[idx].subresourceRange.layerCount = 1;
		release_barriers[idx].subresourceRange.levelCount = 1;
		++idx;

		wl_list_remove(&texture->foreign_link);
		texture->owned = false;
	}

	// also add acquire/release barriers for the current render buffer
	VkImageLayout src_layout = VK_IMAGE_LAYOUT_GENERAL;
	if (!renderer->current_render_buffer->transitioned) {
		src_layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		renderer->current_render_buffer->transitioned = true;
	}

	// acquire render buffer before rendering
	acquire_barriers[idx].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	acquire_barriers[idx].srcQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
	acquire_barriers[idx].dstQueueFamilyIndex = renderer->dev->queue_family;
	acquire_barriers[idx].image = renderer->current_render_buffer->image;
	acquire_barriers[idx].oldLayout = src_layout;
	acquire_barriers[idx].newLayout = VK_IMAGE_LAYOUT_GENERAL;
	acquire_barriers[idx].srcAccessMask = 0u; // ignored anyways
	acquire_barriers[idx].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	acquire_barriers[idx].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	acquire_barriers[idx].subresourceRange.layerCount = 1;
	acquire_barriers[idx].subresourceRange.levelCount = 1;

	// release render buffer after rendering
	release_barriers[idx].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	release_barriers[idx].srcQueueFamilyIndex = renderer->dev->queue_family;
	release_barriers[idx].dstQueueFamilyIndex = VK_QUEUE_FAMILY_FOREIGN_EXT;
	release_barriers[idx].image = renderer->current_render_buffer->image;
	release_barriers[idx].oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	release_barriers[idx].newLayout = VK_IMAGE_LAYOUT_GENERAL;
	release_barriers[idx].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	release_barriers[idx].dstAccessMask = 0u; // ignored anyways
	release_barriers[idx].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	release_barriers[idx].subresourceRange.layerCount = 1;
	release_barriers[idx].subresourceRange.levelCount = 1;
	++idx;

	vkCmdPipelineBarrier(pre_cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0, 0, NULL, 0, NULL, barrier_count, acquire_barriers);

	vkCmdPipelineBarrier(render_cb, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL,
		barrier_count, release_barriers);

	free(acquire_barriers);
	free(release_barriers);

	vkEndCommandBuffer(renderer->cb);

	unsigned submit_count = 0u;
	VkSubmitInfo submit_infos[2] = {0};

	// No semaphores needed here.
	// We don't need a semaphore from the stage/transfer submission
	// to the render submissions since they are on the same queue
	// and we have a renderpass dependency for that.
	if (renderer->stage.recording) {
		vkEndCommandBuffer(renderer->stage.cb);
		renderer->stage.recording = false;

		VkSubmitInfo *stage_sub = &submit_infos[submit_count];
		stage_sub->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		stage_sub->commandBufferCount = 1u;
		stage_sub->pCommandBuffers = &pre_cb;
		++submit_count;
	}

	VkSubmitInfo *render_sub = &submit_infos[submit_count];
	render_sub->sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	render_sub->pCommandBuffers = &render_cb;
	render_sub->commandBufferCount = 1u;
	++submit_count;

	VkResult res = vkQueueSubmit(renderer->dev->queue, submit_count,
		submit_infos, renderer->fence);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkQueueSubmit", res);
		return;
	}

	// sadly this is required due to the current api/rendering model of wlr
	// ideally we could use gpu and cpu in parallel (_without_ the
	// implicit synchronization overhead and mess of opengl drivers)
	res = vkWaitForFences(renderer->dev->dev, 1, &renderer->fence, true,
		UINT64_MAX);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkWaitForFences", res);
		return;
	}

	++renderer->frame;
	release_stage_allocations(renderer);

	// destroy pending textures
	wl_list_for_each_safe(texture, tmp_tex, &renderer->destroy_textures, destroy_link) {
		wlr_texture_destroy(&texture->wlr_texture);
	}

	wl_list_init(&renderer->destroy_textures); // reset the list
	res = vkResetFences(renderer->dev->dev, 1, &renderer->fence);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkResetFences", res);
		return;
	}
}

static bool vulkan_render_subtexture_with_matrix(struct wlr_renderer *wlr_renderer,
		struct wlr_texture *wlr_texture, const struct wlr_fbox *box,
		const float matrix[static 9], float alpha) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	VkCommandBuffer cb = renderer->cb;

	struct wlr_vk_texture *texture = vulkan_get_texture(wlr_texture);
	assert(texture->renderer == renderer);
	if (texture->dmabuf_imported && !texture->owned) {
		// Store this texture in the list of textures that need to be
		// acquired before rendering and released after rendering.
		// We don't do it here immediately since barriers inside
		// a renderpass are suboptimal (would require additional renderpass
		// dependency and potentially multiple barriers) and it's
		// better to issue one barrier for all used textures anyways.
		texture->owned = true;
		assert(texture->foreign_link.prev == NULL);
		assert(texture->foreign_link.next == NULL);
		wl_list_insert(&renderer->foreign_textures, &texture->foreign_link);
	}

	VkPipeline pipe = renderer->current_render_buffer->render_setup->tex_pipe;
	if (pipe != renderer->bound_pipe) {
		vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
		renderer->bound_pipe = pipe;
	}

	vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS,
		renderer->pipe_layout, 0, 1, &texture->ds, 0, NULL);

	float final_matrix[9];
	wlr_matrix_multiply(final_matrix, renderer->projection, matrix);

	struct vert_pcr_data vert_pcr_data;
	mat3_to_mat4(final_matrix, vert_pcr_data.mat4);

	vert_pcr_data.uv_off[0] = box->x / wlr_texture->width;
	vert_pcr_data.uv_off[1] = box->y / wlr_texture->height;
	vert_pcr_data.uv_size[0] = box->width / wlr_texture->width;
	vert_pcr_data.uv_size[1] = box->height / wlr_texture->height;

	vkCmdPushConstants(cb, renderer->pipe_layout,
		VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vert_pcr_data), &vert_pcr_data);
	vkCmdPushConstants(cb, renderer->pipe_layout,
		VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(vert_pcr_data), sizeof(float),
		&alpha);
	vkCmdDraw(cb, 4, 1, 0, 0);
	texture->last_used = renderer->frame;

	return true;
}

static void vulkan_clear(struct wlr_renderer *wlr_renderer,
		const float color[static 4]) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	VkCommandBuffer cb = renderer->cb;

	VkClearAttachment att = {0};
	att.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	att.colorAttachment = 0u;

	// Input color values are given in srgb space, vulkan expects
	// them in linear space. We explicitly import argb8 render buffers
	// as srgb, vulkan will convert the input values we give here to
	// srgb first.
	// But in other parts of wlroots we just always assume
	// srgb so that's why we have to convert here.
	att.clearValue.color.float32[0] = color_to_linear(color[0]);
	att.clearValue.color.float32[1] = color_to_linear(color[1]);
	att.clearValue.color.float32[2] = color_to_linear(color[2]);
	att.clearValue.color.float32[3] = color[3]; // no conversion for alpha

	VkClearRect rect = {0};
	rect.rect = renderer->scissor;
	rect.layerCount = 1;
	vkCmdClearAttachments(cb, 1, &att, 1, &rect);
}

static void vulkan_scissor(struct wlr_renderer *wlr_renderer,
		struct wlr_box *box) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	VkCommandBuffer cb = renderer->cb;

	uint32_t w = renderer->render_width;
	uint32_t h = renderer->render_height;
	struct wlr_box dst = {0, 0, w, h};
	if (box && !wlr_box_intersection(&dst, box, &dst)) {
		dst = (struct wlr_box) {0, 0, 0, 0}; // empty
	}

	VkRect2D rect = (VkRect2D) {{dst.x, dst.y}, {dst.width, dst.height}};
	renderer->scissor = rect;
	vkCmdSetScissor(cb, 0, 1, &rect);
}

static const uint32_t *vulkan_get_shm_texture_formats(
		struct wlr_renderer *wlr_renderer, size_t *len) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	*len = renderer->dev->shm_format_count;
	return renderer->dev->shm_formats;
}

static void vulkan_render_quad_with_matrix(struct wlr_renderer *wlr_renderer,
		const float color[static 4], const float matrix[static 9]) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	VkCommandBuffer cb = renderer->cb;

	VkPipeline pipe = renderer->current_render_buffer->render_setup->quad_pipe;
	if (pipe != renderer->bound_pipe) {
		vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
		renderer->bound_pipe = pipe;
	}

	float final_matrix[9];
	wlr_matrix_multiply(final_matrix, renderer->projection, matrix);

	struct vert_pcr_data vert_pcr_data;
	mat3_to_mat4(final_matrix, vert_pcr_data.mat4);
	vert_pcr_data.uv_off[0] = 0.f;
	vert_pcr_data.uv_off[1] = 0.f;
	vert_pcr_data.uv_size[0] = 1.f;
	vert_pcr_data.uv_size[1] = 1.f;

	// Input color values are given in srgb space, shader expects
	// them in linear space. The shader does all computation in linear
	// space and expects in inputs in linear space since it outputs
	// colors in linear space as well (and vulkan then automatically
	// does the conversion for out SRGB render targets).
	// But in other parts of wlroots we just always assume
	// srgb so that's why we have to convert here.
	float linear_color[4];
	linear_color[0] = color_to_linear(color[0]);
	linear_color[1] = color_to_linear(color[1]);
	linear_color[2] = color_to_linear(color[2]);
	linear_color[3] = color[3]; // no conversion for alpha

	vkCmdPushConstants(cb, renderer->pipe_layout,
		VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vert_pcr_data), &vert_pcr_data);
	vkCmdPushConstants(cb, renderer->pipe_layout,
		VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(vert_pcr_data), sizeof(float) * 4,
		linear_color);
	vkCmdDraw(cb, 4, 1, 0, 0);
}

static const struct wlr_drm_format_set *vulkan_get_dmabuf_texture_formats(
		struct wlr_renderer *wlr_renderer) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	return &renderer->dev->dmabuf_texture_formats;
}

static const struct wlr_drm_format_set *vulkan_get_render_formats(
		struct wlr_renderer *wlr_renderer) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	return &renderer->dev->dmabuf_render_formats;
}

static uint32_t vulkan_preferred_read_format(
		struct wlr_renderer *wlr_renderer) {
	// TODO: implement!
	wlr_log(WLR_ERROR, "vulkan_preferred_read_format not implemented");
	return DRM_FORMAT_XBGR8888;
}

static void vulkan_destroy(struct wlr_renderer *wlr_renderer) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	struct wlr_vk_device *dev = renderer->dev;
	if (!dev) {
		free(renderer);
		return;
	}

	assert(!renderer->current_render_buffer);

	// stage.cb automatically freed with command pool
	struct wlr_vk_shared_buffer *buf, *tmp_buf;
	wl_list_for_each_safe(buf, tmp_buf, &renderer->stage.buffers, link) {
		shared_buffer_destroy(renderer, buf);
	}

	struct wlr_vk_texture *tex, *tex_tmp;
	wl_list_for_each_safe(tex, tex_tmp, &renderer->textures, link) {
		vulkan_texture_destroy(tex);
	}

	struct wlr_vk_render_buffer *render_buffer, *render_buffer_tmp;
	wl_list_for_each_safe(render_buffer, render_buffer_tmp,
			&renderer->render_buffers, link) {
		destroy_render_buffer(render_buffer);
	}

	struct wlr_vk_render_format_setup *setup, *tmp_setup;
	wl_list_for_each_safe(setup, tmp_setup,
			&renderer->render_format_setups, link) {
		destroy_render_format_setup(renderer, setup);
	}

	struct wlr_vk_descriptor_pool *pool, *tmp_pool;
	wl_list_for_each_safe(pool, tmp_pool, &renderer->descriptor_pools, link) {
		vkDestroyDescriptorPool(dev->dev, pool->pool, NULL);
		free(pool);
	}

	vkDestroyShaderModule(dev->dev, renderer->vert_module, NULL);
	vkDestroyShaderModule(dev->dev, renderer->tex_frag_module, NULL);
	vkDestroyShaderModule(dev->dev, renderer->quad_frag_module, NULL);

	vkDestroyFence(dev->dev, renderer->fence, NULL);
	vkDestroyPipelineLayout(dev->dev, renderer->pipe_layout, NULL);
	vkDestroyDescriptorSetLayout(dev->dev, renderer->ds_layout, NULL);
	vkDestroySampler(dev->dev, renderer->sampler, NULL);
	vkDestroyCommandPool(dev->dev, renderer->command_pool, NULL);

	struct wlr_vk_instance *ini = dev->instance;
	vulkan_device_destroy(dev);
	vulkan_instance_destroy(ini);
	free(renderer);
}

static bool vulkan_read_pixels(struct wlr_renderer *wlr_renderer,
		uint32_t drm_format, uint32_t *flags, uint32_t stride,
		uint32_t width, uint32_t height, uint32_t src_x, uint32_t src_y,
		uint32_t dst_x, uint32_t dst_y, void *data) {
	// TODO: implement!
	wlr_log(WLR_ERROR, "vulkan_read_pixels not implemented");
	return false;
}

static int vulkan_get_drm_fd(struct wlr_renderer *wlr_renderer) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);
	return renderer->dev->drm_fd;
}

static uint32_t vulkan_get_render_buffer_caps(struct wlr_renderer *wlr_renderer) {
	return WLR_BUFFER_CAP_DMABUF;
}

static const struct wlr_renderer_impl renderer_impl = {
	.bind_buffer = vulkan_bind_buffer,
	.begin = vulkan_begin,
	.end = vulkan_end,
	.clear = vulkan_clear,
	.scissor = vulkan_scissor,
	.render_subtexture_with_matrix = vulkan_render_subtexture_with_matrix,
	.render_quad_with_matrix = vulkan_render_quad_with_matrix,
	.get_shm_texture_formats = vulkan_get_shm_texture_formats,
	.get_dmabuf_texture_formats = vulkan_get_dmabuf_texture_formats,
	.get_render_formats = vulkan_get_render_formats,
	.preferred_read_format = vulkan_preferred_read_format,
	.read_pixels = vulkan_read_pixels,
	.destroy = vulkan_destroy,
	.get_drm_fd = vulkan_get_drm_fd,
	.get_render_buffer_caps = vulkan_get_render_buffer_caps,
	.texture_from_buffer = vulkan_texture_from_buffer,
};

// Initializes the VkDescriptorSetLayout and VkPipelineLayout needed
// for the texture rendering pipeline using the given VkSampler.
static bool init_tex_layouts(struct wlr_vk_renderer *renderer,
		VkSampler tex_sampler, VkDescriptorSetLayout *out_ds_layout,
		VkPipelineLayout *out_pipe_layout) {
	VkResult res;
	VkDevice dev = renderer->dev->dev;

	// layouts
	// descriptor set
	VkDescriptorSetLayoutBinding ds_bindings[1] = {{
		0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
		VK_SHADER_STAGE_FRAGMENT_BIT, &tex_sampler,
	}};

	VkDescriptorSetLayoutCreateInfo ds_info = {0};
	ds_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ds_info.bindingCount = 1;
	ds_info.pBindings = ds_bindings;

	res = vkCreateDescriptorSetLayout(dev, &ds_info, NULL, out_ds_layout);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateDescriptorSetLayout", res);
		return false;
	}

	// pipeline layout
	VkPushConstantRange pc_ranges[2] = {0};
	pc_ranges[0].size = sizeof(struct vert_pcr_data);
	pc_ranges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pc_ranges[1].offset = pc_ranges[0].size;
	pc_ranges[1].size = sizeof(float) * 4; // alpha or color
	pc_ranges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo pl_info = {0};
	pl_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pl_info.setLayoutCount = 1;
	pl_info.pSetLayouts = out_ds_layout;
	pl_info.pushConstantRangeCount = 2;
	pl_info.pPushConstantRanges = pc_ranges;

	res = vkCreatePipelineLayout(dev, &pl_info, NULL, out_pipe_layout);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreatePipelineLayout", res);
		return false;
	}

	return true;
}

// Initializes the pipeline for rendering textures and using the given
// VkRenderPass and VkPipelineLayout.
static bool init_tex_pipeline(struct wlr_vk_renderer *renderer,
		VkRenderPass rp, VkPipelineLayout pipe_layout, VkPipeline *pipe) {
	VkResult res;
	VkDevice dev = renderer->dev->dev;

	// shaders
	VkPipelineShaderStageCreateInfo vert_stage = {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		NULL, 0, VK_SHADER_STAGE_VERTEX_BIT, renderer->vert_module,
		"main", NULL
	};

	VkPipelineShaderStageCreateInfo tex_stages[2] = {vert_stage, {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		NULL, 0, VK_SHADER_STAGE_FRAGMENT_BIT, renderer->tex_frag_module,
		"main", NULL
	}};

	// info
	VkPipelineInputAssemblyStateCreateInfo assembly = {0};
	assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

	VkPipelineRasterizationStateCreateInfo rasterization = {0};
	rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization.cullMode = VK_CULL_MODE_NONE;
	rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization.lineWidth = 1.f;

	VkPipelineColorBlendAttachmentState blend_attachment = {0};
	blend_attachment.blendEnable = true;
	// we generally work with pre-multiplied alpha
	blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
	blend_attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo blend = {0};
	blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend.attachmentCount = 1;
	blend.pAttachments = &blend_attachment;

	VkPipelineMultisampleStateCreateInfo multisample = {0};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineViewportStateCreateInfo viewport = {0};
	viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport.viewportCount = 1;
	viewport.scissorCount = 1;

	VkDynamicState dynStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	VkPipelineDynamicStateCreateInfo dynamic = {0};
	dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic.pDynamicStates = dynStates;
	dynamic.dynamicStateCount = 2;

	VkPipelineVertexInputStateCreateInfo vertex = {0};
	vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkGraphicsPipelineCreateInfo pinfo = {0};
	pinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pinfo.layout = pipe_layout;
	pinfo.renderPass = rp;
	pinfo.subpass = 0;
	pinfo.stageCount = 2;
	pinfo.pStages = tex_stages;

	pinfo.pInputAssemblyState = &assembly;
	pinfo.pRasterizationState = &rasterization;
	pinfo.pColorBlendState = &blend;
	pinfo.pMultisampleState = &multisample;
	pinfo.pViewportState = &viewport;
	pinfo.pDynamicState = &dynamic;
	pinfo.pVertexInputState = &vertex;

	// NOTE: use could use a cache here for faster loading
	// store it somewhere like $XDG_CACHE_HOME/wlroots/vk_pipe_cache
	VkPipelineCache cache = VK_NULL_HANDLE;
	res = vkCreateGraphicsPipelines(dev, cache, 1, &pinfo, NULL, pipe);
	if (res != VK_SUCCESS) {
		wlr_vk_error("failed to create vulkan pipelines:", res);
		return false;
	}

	return true;
}

// Creates static render data, such as sampler, layouts and shader modules
// for the given rednerer.
// Cleanup is done by destroying the renderer.
static bool init_static_render_data(struct wlr_vk_renderer *renderer) {
	VkResult res;
	VkDevice dev = renderer->dev->dev;

	// default sampler (non ycbcr)
	VkSamplerCreateInfo sampler_info = {0};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.maxAnisotropy = 1.f;
	sampler_info.minLod = 0.f;
	sampler_info.maxLod = 0.25f;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

	res = vkCreateSampler(dev, &sampler_info, NULL, &renderer->sampler);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Failed to create sampler", res);
		return false;
	}

	if (!init_tex_layouts(renderer, renderer->sampler,
			&renderer->ds_layout, &renderer->pipe_layout)) {
		return false;
	}

	// load vert module and tex frag module since they are needed to
	// initialize the tex pipeline
	VkShaderModuleCreateInfo sinfo = {0};
	sinfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	sinfo.codeSize = sizeof(common_vert_data);
	sinfo.pCode = common_vert_data;
	res = vkCreateShaderModule(dev, &sinfo, NULL, &renderer->vert_module);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Failed to create vertex shader module", res);
		return false;
	}

	// tex frag
	sinfo.codeSize = sizeof(texture_frag_data);
	sinfo.pCode = texture_frag_data;
	res = vkCreateShaderModule(dev, &sinfo, NULL, &renderer->tex_frag_module);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Failed to create tex fragment shader module", res);
		return false;
	}

	// quad frag
	sinfo.codeSize = sizeof(quad_frag_data);
	sinfo.pCode = quad_frag_data;
	res = vkCreateShaderModule(dev, &sinfo, NULL, &renderer->quad_frag_module);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Failed to create quad fragment shader module", res);
		return false;
	}

	return true;
}

static struct wlr_vk_render_format_setup *find_or_create_render_setup(
		struct wlr_vk_renderer *renderer, VkFormat format) {
	struct wlr_vk_render_format_setup *setup;
	wl_list_for_each(setup, &renderer->render_format_setups, link) {
		if (setup->render_format == format) {
			return setup;
		}
	}

	setup = calloc(1u, sizeof(*setup));
	if (!setup) {
		wlr_log(WLR_ERROR, "Allocation failed");
		return NULL;
	}

	setup->render_format = format;

	// util
	VkDevice dev = renderer->dev->dev;
	VkResult res;

	VkAttachmentDescription attachment = {0};
	attachment.format = format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
	attachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkAttachmentReference color_ref = {0};
	color_ref.attachment = 0u;
	color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_ref;

	VkSubpassDependency deps[2] = {0};
	deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	deps[0].srcStageMask = VK_PIPELINE_STAGE_HOST_BIT |
		VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT |
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT |
		VK_ACCESS_TRANSFER_WRITE_BIT |
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	deps[0].dstSubpass = 0;
	deps[0].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	deps[0].dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT |
		VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
		VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
		VK_ACCESS_SHADER_READ_BIT;

	deps[1].srcSubpass = 0;
	deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	deps[1].dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT |
		VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	deps[1].dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT |
		VK_ACCESS_MEMORY_READ_BIT;

	VkRenderPassCreateInfo rp_info = {0};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.attachmentCount = 1;
	rp_info.pAttachments = &attachment;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 2u;
	rp_info.pDependencies = deps;

	res = vkCreateRenderPass(dev, &rp_info, NULL, &setup->render_pass);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Failed to create render pass", res);
		free(setup);
		return NULL;
	}

	if (!init_tex_pipeline(renderer, setup->render_pass, renderer->pipe_layout,
			&setup->tex_pipe)) {
		goto error;
	}

	VkPipelineShaderStageCreateInfo vert_stage = {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		NULL, 0, VK_SHADER_STAGE_VERTEX_BIT, renderer->vert_module,
		"main", NULL
	};

	VkPipelineShaderStageCreateInfo quad_stages[2] = {vert_stage, {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		NULL, 0, VK_SHADER_STAGE_FRAGMENT_BIT,
		renderer->quad_frag_module, "main", NULL
	}};

	// info
	VkPipelineInputAssemblyStateCreateInfo assembly = {0};
	assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

	VkPipelineRasterizationStateCreateInfo rasterization = {0};
	rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization.polygonMode = VK_POLYGON_MODE_FILL;
	rasterization.cullMode = VK_CULL_MODE_NONE;
	rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterization.lineWidth = 1.f;

	VkPipelineColorBlendAttachmentState blend_attachment = {0};
	blend_attachment.blendEnable = true;
	blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
	blend_attachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo blend = {0};
	blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blend.attachmentCount = 1;
	blend.pAttachments = &blend_attachment;

	VkPipelineMultisampleStateCreateInfo multisample = {0};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineViewportStateCreateInfo viewport = {0};
	viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport.viewportCount = 1;
	viewport.scissorCount = 1;

	VkDynamicState dynStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	VkPipelineDynamicStateCreateInfo dynamic = {0};
	dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic.pDynamicStates = dynStates;
	dynamic.dynamicStateCount = 2;

	VkPipelineVertexInputStateCreateInfo vertex = {0};
	vertex.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkGraphicsPipelineCreateInfo pinfo = {0};
	pinfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pinfo.layout = renderer->pipe_layout;
	pinfo.renderPass = setup->render_pass;
	pinfo.subpass = 0;
	pinfo.stageCount = 2;
	pinfo.pStages = quad_stages;

	pinfo.pInputAssemblyState = &assembly;
	pinfo.pRasterizationState = &rasterization;
	pinfo.pColorBlendState = &blend;
	pinfo.pMultisampleState = &multisample;
	pinfo.pViewportState = &viewport;
	pinfo.pDynamicState = &dynamic;
	pinfo.pVertexInputState = &vertex;

	// NOTE: use could use a cache here for faster loading
	// store it somewhere like $XDG_CACHE_HOME/wlroots/vk_pipe_cache.bin
	VkPipelineCache cache = VK_NULL_HANDLE;
	res = vkCreateGraphicsPipelines(dev, cache, 1, &pinfo, NULL, &setup->quad_pipe);
	if (res != VK_SUCCESS) {
		wlr_log(WLR_ERROR, "failed to create vulkan quad pipeline: %d", res);
		goto error;
	}

	wl_list_insert(&renderer->render_format_setups, &setup->link);
	return setup;

error:
	destroy_render_format_setup(renderer, setup);
	return NULL;
}

struct wlr_renderer *vulkan_renderer_create_for_device(struct wlr_vk_device *dev) {
	struct wlr_vk_renderer *renderer;
	VkResult res;
	if (!(renderer = calloc(1, sizeof(*renderer)))) {
		wlr_log_errno(WLR_ERROR, "failed to allocate wlr_vk_renderer");
		return NULL;
	}

	renderer->dev = dev;
	wlr_renderer_init(&renderer->wlr_renderer, &renderer_impl);
	wl_list_init(&renderer->stage.buffers);
	wl_list_init(&renderer->destroy_textures);
	wl_list_init(&renderer->foreign_textures);
	wl_list_init(&renderer->textures);
	wl_list_init(&renderer->descriptor_pools);
	wl_list_init(&renderer->render_format_setups);
	wl_list_init(&renderer->render_buffers);

	if (!init_static_render_data(renderer)) {
		goto error;
	}

	// command pool
	VkCommandPoolCreateInfo cpool_info = {0};
	cpool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cpool_info.queueFamilyIndex = dev->queue_family;
	res = vkCreateCommandPool(dev->dev, &cpool_info, NULL,
		&renderer->command_pool);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateCommandPool", res);
		goto error;
	}

	VkCommandBufferAllocateInfo cbai = {0};
	cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbai.commandBufferCount = 1u;
	cbai.commandPool = renderer->command_pool;
	cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	res = vkAllocateCommandBuffers(dev->dev, &cbai, &renderer->cb);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkAllocateCommandBuffers", res);
		goto error;
	}

	VkFenceCreateInfo fence_info = {0};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	res = vkCreateFence(dev->dev, &fence_info, NULL,
		&renderer->fence);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateFence", res);
		goto error;
	}

	// staging command buffer
	VkCommandBufferAllocateInfo cmd_buf_info = {0};
	cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buf_info.commandPool = renderer->command_pool;
	cmd_buf_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_buf_info.commandBufferCount = 1u;
	res = vkAllocateCommandBuffers(dev->dev, &cmd_buf_info,
		&renderer->stage.cb);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkAllocateCommandBuffers", res);
		goto error;
	}

	return &renderer->wlr_renderer;

error:
	vulkan_destroy(&renderer->wlr_renderer);
	return NULL;
}

struct wlr_renderer *wlr_vk_renderer_create_with_drm_fd(int drm_fd) {
	wlr_log(WLR_INFO, "The vulkan renderer is only experimental and "
		"not expected to be ready for daily use");

	// NOTE: we could add functionality to allow the compositor passing its
	// name and version to this function. Just use dummies until then,
	// shouldn't be relevant to the driver anyways
	struct wlr_vk_instance *ini = vulkan_instance_create(0, NULL, default_debug);
	if (!ini) {
		wlr_log(WLR_ERROR, "creating vulkan instance for renderer failed");
		return NULL;
	}

	VkPhysicalDevice phdev = vulkan_find_drm_phdev(ini, drm_fd);
	if (!phdev) {
		// We rather fail here than doing some guesswork
		wlr_log(WLR_ERROR, "Could not match drm and vulkan device");
		return NULL;
	}

	// queue families
	uint32_t qfam_count;
	vkGetPhysicalDeviceQueueFamilyProperties(phdev, &qfam_count, NULL);
	VkQueueFamilyProperties queue_props[qfam_count];
	vkGetPhysicalDeviceQueueFamilyProperties(phdev, &qfam_count,
		queue_props);

	struct wlr_vk_device *dev = vulkan_device_create(ini, phdev, 0, NULL);
	if (!dev) {
		wlr_log(WLR_ERROR, "Failed to create vulkan device");
		vulkan_instance_destroy(ini);
		return NULL;
	}

	// We duplicate it so it's not closed while we still need it.
	dev->drm_fd = fcntl(drm_fd, F_DUPFD_CLOEXEC, 0);
	if (dev->drm_fd < 0) {
		wlr_log_errno(WLR_ERROR, "fcntl(F_DUPFD_CLOEXEC) failed");
		vulkan_device_destroy(dev);
		vulkan_instance_destroy(ini);
		return NULL;
	}

	return vulkan_renderer_create_for_device(dev);
}
