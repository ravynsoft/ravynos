#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/util/log.h>
#include "render/pixel_format.h"
#include "render/vulkan.h"

static const struct wlr_texture_impl texture_impl;

struct wlr_vk_texture *vulkan_get_texture(struct wlr_texture *wlr_texture) {
	assert(wlr_texture->impl == &texture_impl);
	return (struct wlr_vk_texture *)wlr_texture;
}

static VkImageAspectFlagBits mem_plane_aspect(unsigned i) {
	switch (i) {
	case 0: return VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT;
	case 1: return VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT;
	case 2: return VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT;
	case 3: return VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT;
	default: abort(); // unreachable
	}
}

static bool vulkan_texture_is_opaque(struct wlr_texture *wlr_texture) {
	struct wlr_vk_texture *texture = vulkan_get_texture(wlr_texture);
	const struct wlr_pixel_format_info *format_info = drm_get_pixel_format_info(
			texture->format->drm_format);
	assert(format_info);
	return !format_info->has_alpha;
}

// Will transition the texture to shaderReadOnlyOptimal layout for reading
// from fragment shader later on
static bool write_pixels(struct wlr_texture *wlr_texture,
		uint32_t stride, uint32_t width, uint32_t height, uint32_t src_x,
		uint32_t src_y, uint32_t dst_x, uint32_t dst_y, const void *vdata,
		VkImageLayout old_layout, VkPipelineStageFlags src_stage,
		VkAccessFlags src_access) {
	VkResult res;
	struct wlr_vk_texture *texture = vulkan_get_texture(wlr_texture);
	struct wlr_vk_renderer *renderer = texture->renderer;
	VkDevice dev = texture->renderer->dev->dev;

	// make sure assumptions are met
	assert(src_x + width <= texture->wlr_texture.width);
	assert(src_y + height <= texture->wlr_texture.height);
	assert(dst_x + width <= texture->wlr_texture.width);
	assert(dst_y + height <= texture->wlr_texture.height);

	const struct wlr_pixel_format_info *format_info = drm_get_pixel_format_info(
			texture->format->drm_format);
	assert(format_info);

	// deferred upload by transfer; using staging buffer
	// calculate maximum side needed
	uint32_t bsize = 0;
	unsigned bytespb = format_info->bpp / 8;
	bsize += height * bytespb * width;

	// get staging buffer
	struct wlr_vk_buffer_span span = vulkan_get_stage_span(renderer, bsize);
	if (!span.buffer || span.alloc.size != bsize) {
		wlr_log(WLR_ERROR, "Failed to retrieve staging buffer");
		return false;
	}

	void *vmap;
	res = vkMapMemory(dev, span.buffer->memory, span.alloc.start,
		bsize, 0, &vmap);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkMapMemory", res);
		return false;
	}
	char *map = (char *)vmap;

	// record staging cb
	// will be executed before next frame
	VkCommandBuffer cb = vulkan_record_stage_cb(renderer);
	vulkan_change_layout(cb, texture->image,
		old_layout, src_stage, src_access,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT);

	// upload data
	const char *pdata = vdata; // data iterator

	uint32_t packed_stride = bytespb * width;
	uint32_t buf_off = span.alloc.start + (map - (char *)vmap);

	// write data into staging buffer span
	pdata += stride * src_y;
	pdata += bytespb * src_x;
	if (src_x == 0 && width == texture->wlr_texture.width &&
			stride == packed_stride) {
		memcpy(map, pdata, packed_stride * height);
		map += packed_stride * height;
	} else {
		for (unsigned i = 0u; i < height; ++i) {
			memcpy(map, pdata, packed_stride);
			pdata += stride;
			map += packed_stride;
		}
	}

	VkBufferImageCopy copy;
	copy.imageExtent.width = width;
	copy.imageExtent.height = height;
	copy.imageExtent.depth = 1;
	copy.imageOffset.x = dst_x;
	copy.imageOffset.y = dst_y;
	copy.imageOffset.z = 0;
	copy.bufferOffset = buf_off;
	copy.bufferRowLength = width;
	copy.bufferImageHeight = height;
	copy.imageSubresource.mipLevel = 0;
	copy.imageSubresource.baseArrayLayer = 0;
	copy.imageSubresource.layerCount = 1;
	copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	assert((uint32_t)(map - (char *)vmap) == bsize);
	vkUnmapMemory(dev, span.buffer->memory);

	vkCmdCopyBufferToImage(cb, span.buffer->buffer, texture->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
	vulkan_change_layout(cb, texture->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_ACCESS_SHADER_READ_BIT);
	texture->last_used = renderer->frame;

	return true;
}

static bool vulkan_texture_write_pixels(struct wlr_texture *wlr_texture,
		uint32_t stride, uint32_t width, uint32_t height, uint32_t src_x,
		uint32_t src_y, uint32_t dst_x, uint32_t dst_y, const void *vdata) {
	return write_pixels(wlr_texture, stride, width, height, src_x, src_y,
		dst_x, dst_y, vdata, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
}

void vulkan_texture_destroy(struct wlr_vk_texture *texture) {
	if (!texture->renderer) {
		free(texture);
		return;
	}

	// when we recorded a command to fill this image _this_ frame,
	// it has to be executed before the texture can be destroyed.
	// Add it to the renderer->destroy_textures list, destroying
	// _after_ the stage command buffer has exectued
	if (texture->last_used == texture->renderer->frame) {
		assert(texture->destroy_link.next == NULL); // not already inserted
		wl_list_insert(&texture->renderer->destroy_textures,
			&texture->destroy_link);
		return;
	}

	wl_list_remove(&texture->link);
	wl_list_remove(&texture->buffer_destroy.link);

	VkDevice dev = texture->renderer->dev->dev;
	if (texture->ds && texture->ds_pool) {
		vulkan_free_ds(texture->renderer, texture->ds_pool, texture->ds);
	}

	vkDestroyImageView(dev, texture->image_view, NULL);
	vkDestroyImage(dev, texture->image, NULL);

	for (unsigned i = 0u; i < texture->mem_count; ++i) {
		vkFreeMemory(dev, texture->memories[i], NULL);
	}

	free(texture);
}

static void vulkan_texture_unref(struct wlr_texture *wlr_texture) {
	struct wlr_vk_texture *texture = vulkan_get_texture(wlr_texture);
	if (texture->buffer != NULL) {
		// Keep the texture around, in case the buffer is re-used later. We're
		// still listening to the buffer's destroy event.
		wlr_buffer_unlock(texture->buffer);
	} else {
		vulkan_texture_destroy(texture);
	}
}

static const struct wlr_texture_impl texture_impl = {
	.is_opaque = vulkan_texture_is_opaque,
	.write_pixels = vulkan_texture_write_pixels,
	.destroy = vulkan_texture_unref,
};

static struct wlr_vk_texture *vulkan_texture_create(
		struct wlr_vk_renderer *renderer, uint32_t width, uint32_t height) {
	struct wlr_vk_texture *texture =
		calloc(1, sizeof(struct wlr_vk_texture));
	if (texture == NULL) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return NULL;
	}
	wlr_texture_init(&texture->wlr_texture, &texture_impl, width, height);
	texture->renderer = renderer;
	wl_list_insert(&renderer->textures, &texture->link);
	wl_list_init(&texture->buffer_destroy.link);
	return texture;
}

static struct wlr_texture *vulkan_texture_from_pixels(struct wlr_renderer *wlr_renderer,
		uint32_t drm_fmt, uint32_t stride, uint32_t width,
		uint32_t height, const void *data) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);

	VkResult res;
	VkDevice dev = renderer->dev->dev;

	wlr_log(WLR_DEBUG, "vulkan_texture_from_pixels: %.4s, %dx%d",
		(const char*) &drm_fmt, width, height);

	const struct wlr_vk_format_props *fmt =
		vulkan_format_props_from_drm(renderer->dev, drm_fmt);
	if (fmt == NULL) {
		wlr_log(WLR_ERROR, "Unsupported pixel format %"PRIx32 " (%.4s)",
			drm_fmt, (const char*) &drm_fmt);
		return NULL;
	}

	struct wlr_vk_texture *texture = vulkan_texture_create(renderer, width, height);
	if (texture == NULL) {
		return NULL;
	}

	texture->format = &fmt->format;

	// create image
	unsigned mem_bits = 0xFFFFFFFF;

	VkImageCreateInfo img_info = {0};
	img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_info.imageType = VK_IMAGE_TYPE_2D;
	img_info.format = texture->format->vk_format;
	img_info.mipLevels = 1;
	img_info.arrayLayers = 1;
	img_info.samples = VK_SAMPLE_COUNT_1_BIT;
	img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	img_info.extent = (VkExtent3D) { width, height, 1 };
	img_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

	img_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	img_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	mem_bits = vulkan_find_mem_type(renderer->dev,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mem_bits);
	VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	res = vkCreateImage(dev, &img_info, NULL, &texture->image);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateImage failed", res);
		goto error;
	}

	// memory
	VkMemoryRequirements mem_reqs;
	vkGetImageMemoryRequirements(dev, texture->image, &mem_reqs);

	VkMemoryAllocateInfo mem_info = {0};
	mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_info.allocationSize = mem_reqs.size;
	mem_info.memoryTypeIndex = mem_bits & mem_reqs.memoryTypeBits;
	res = vkAllocateMemory(dev, &mem_info, NULL, &texture->memories[0]);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkAllocatorMemory failed", res);
		goto error;
	}

	texture->mem_count = 1;
	res = vkBindImageMemory(dev, texture->image, texture->memories[0], 0);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkBindMemory failed", res);
		goto error;
	}

	const struct wlr_pixel_format_info *format_info = drm_get_pixel_format_info(drm_fmt);
	assert(format_info);

	// view
	VkImageViewCreateInfo view_info = {0};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = texture->format->vk_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = format_info->has_alpha
		? VK_COMPONENT_SWIZZLE_IDENTITY
		: VK_COMPONENT_SWIZZLE_ONE;

	view_info.subresourceRange = (VkImageSubresourceRange) {
		VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
	};
	view_info.image = texture->image;

	res = vkCreateImageView(dev, &view_info, NULL,
		&texture->image_view);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateImageView failed", res);
		goto error;
	}

	// descriptor
	texture->ds_pool = vulkan_alloc_texture_ds(renderer, &texture->ds);
	if (!texture->ds_pool) {
		wlr_log(WLR_ERROR, "failed to allocate descriptor");
		goto error;
	}

	VkDescriptorImageInfo ds_img_info = {0};
	ds_img_info.imageView = texture->image_view;
	ds_img_info.imageLayout = layout;

	VkWriteDescriptorSet ds_write = {0};
	ds_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ds_write.descriptorCount = 1;
	ds_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ds_write.dstSet = texture->ds;
	ds_write.pImageInfo = &ds_img_info;

	vkUpdateDescriptorSets(dev, 1, &ds_write, 0, NULL);

	// write data
	if (!write_pixels(&texture->wlr_texture, stride,
			width, height, 0, 0, 0, 0, data, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0)) {
		goto error;
	}

	return &texture->wlr_texture;

error:
	vulkan_texture_destroy(texture);
	return NULL;
}

static bool is_dmabuf_disjoint(const struct wlr_dmabuf_attributes *attribs) {
	if (attribs->n_planes == 1) {
		return false;
	}

	struct stat first_stat;
	if (fstat(attribs->fd[0], &first_stat) != 0) {
		wlr_log_errno(WLR_ERROR, "fstat failed");
		return true;
	}

	for (int i = 1; i < attribs->n_planes; i++) {
		struct stat plane_stat;
		if (fstat(attribs->fd[i], &plane_stat) != 0) {
			wlr_log_errno(WLR_ERROR, "fstat failed");
			return true;
		}

		if (first_stat.st_ino != plane_stat.st_ino) {
			return true;
		}
	}

	return false;
}

VkImage vulkan_import_dmabuf(struct wlr_vk_renderer *renderer,
		const struct wlr_dmabuf_attributes *attribs,
		VkDeviceMemory mems[static WLR_DMABUF_MAX_PLANES], uint32_t *n_mems,
		bool for_render) {
	VkResult res;
	VkDevice dev = renderer->dev->dev;
	*n_mems = 0u;

	wlr_log(WLR_DEBUG, "vulkan_import_dmabuf: %.4s (mod %"PRIx64"), %dx%d, %d planes",
		(const char *)&attribs->format, attribs->modifier,
		attribs->width, attribs->height, attribs->n_planes);

	struct wlr_vk_format_props *fmt = vulkan_format_props_from_drm(renderer->dev,
		attribs->format);
	if (fmt == NULL) {
		wlr_log(WLR_ERROR, "Unsupported pixel format %"PRIx32 " (%.4s)",
			attribs->format, (const char*) &attribs->format);
		return VK_NULL_HANDLE;
	}

	uint32_t plane_count = attribs->n_planes;
	assert(plane_count < WLR_DMABUF_MAX_PLANES);
	struct wlr_vk_format_modifier_props *mod =
		vulkan_format_props_find_modifier(fmt, attribs->modifier, for_render);
	if (!mod || !(mod->dmabuf_flags & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
		wlr_log(WLR_ERROR, "Format %"PRIx32" (%.4s) can't be used with modifier "
			"%"PRIx64, attribs->format, (const char*) &attribs->format,
			attribs->modifier);
		return VK_NULL_HANDLE;
	}

	if ((uint32_t) attribs->width > mod->max_extent.width ||
			(uint32_t) attribs->height > mod->max_extent.height) {
		wlr_log(WLR_ERROR, "dmabuf is too large to import");
		return VK_NULL_HANDLE;
	}

	if (mod->props.drmFormatModifierPlaneCount != plane_count) {
		wlr_log(WLR_ERROR, "Number of planes (%d) does not match format (%d)",
			plane_count, mod->props.drmFormatModifierPlaneCount);
		return VK_NULL_HANDLE;
	}

	// check if we have to create the image disjoint
	bool disjoint = is_dmabuf_disjoint(attribs);
	if (disjoint && !(mod->props.drmFormatModifierTilingFeatures
			& VK_FORMAT_FEATURE_DISJOINT_BIT)) {
		wlr_log(WLR_ERROR, "Format/Modifier does not support disjoint images");
		return VK_NULL_HANDLE;
	}

	// image
	VkExternalMemoryHandleTypeFlagBits htype =
		VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

	VkImageCreateInfo img_info = {0};
	img_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	img_info.imageType = VK_IMAGE_TYPE_2D;
	img_info.format = fmt->format.vk_format;
	img_info.mipLevels = 1;
	img_info.arrayLayers = 1;
	img_info.samples = VK_SAMPLE_COUNT_1_BIT;
	img_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	img_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	img_info.extent = (VkExtent3D) { attribs->width, attribs->height, 1 };
	img_info.usage = for_render ?
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT :
		VK_IMAGE_USAGE_SAMPLED_BIT;
	if (disjoint) {
		img_info.flags = VK_IMAGE_CREATE_DISJOINT_BIT;
	}

	VkExternalMemoryImageCreateInfo eimg = {0};
	eimg.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
	eimg.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
	img_info.pNext = &eimg;

	VkSubresourceLayout plane_layouts[WLR_DMABUF_MAX_PLANES] = {0};
	VkImageDrmFormatModifierExplicitCreateInfoEXT mod_info = {0};

	img_info.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
	for (unsigned i = 0u; i < plane_count; ++i) {
		plane_layouts[i].offset = attribs->offset[i];
		plane_layouts[i].rowPitch = attribs->stride[i];
		plane_layouts[i].size = 0;
	}

	mod_info.sType = VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_EXPLICIT_CREATE_INFO_EXT;
	mod_info.drmFormatModifierPlaneCount = plane_count;
	mod_info.drmFormatModifier = mod->props.drmFormatModifier;
	mod_info.pPlaneLayouts = plane_layouts;
	eimg.pNext = &mod_info;

	VkImage image;
	res = vkCreateImage(dev, &img_info, NULL, &image);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateImage", res);
		return VK_NULL_HANDLE;
	}

	unsigned mem_count = disjoint ? plane_count : 1u;
	VkBindImageMemoryInfo bindi[WLR_DMABUF_MAX_PLANES] = {0};
	VkBindImagePlaneMemoryInfo planei[WLR_DMABUF_MAX_PLANES] = {0};

	for (unsigned i = 0u; i < mem_count; ++i) {
		struct VkMemoryFdPropertiesKHR fdp = {0};
		fdp.sType = VK_STRUCTURE_TYPE_MEMORY_FD_PROPERTIES_KHR;
		res = renderer->dev->api.getMemoryFdPropertiesKHR(dev, htype,
			attribs->fd[i], &fdp);
		if (res != VK_SUCCESS) {
			wlr_vk_error("getMemoryFdPropertiesKHR", res);
			goto error_image;
		}

		VkImageMemoryRequirementsInfo2 memri = {0};
		memri.image = image;
		memri.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;

		VkImagePlaneMemoryRequirementsInfo planeri = {0};
		if (disjoint) {
			planeri.sType = VK_STRUCTURE_TYPE_IMAGE_PLANE_MEMORY_REQUIREMENTS_INFO;
			planeri.planeAspect = mem_plane_aspect(i);
			memri.pNext = &planeri;
		}

		VkMemoryRequirements2 memr = {0};
		memr.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;

		vkGetImageMemoryRequirements2(dev, &memri, &memr);
		int mem = vulkan_find_mem_type(renderer->dev, 0,
			memr.memoryRequirements.memoryTypeBits & fdp.memoryTypeBits);
		if (mem < 0) {
			wlr_log(WLR_ERROR, "no valid memory type index");
			goto error_image;
		}

		// Since importing transfers ownership of the FD to Vulkan, we have
		// to duplicate it since this operation does not transfer ownership
		// of the attribs to this texture. Will be closed by Vulkan on
		// vkFreeMemory.
		int dfd = fcntl(attribs->fd[i], F_DUPFD_CLOEXEC, 0);
		if (dfd < 0) {
			wlr_log_errno(WLR_ERROR, "fcntl(F_DUPFD_CLOEXEC) failed");
			goto error_image;
		}

		VkMemoryAllocateInfo memi = {0};
		memi.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memi.allocationSize = memr.memoryRequirements.size;
		memi.memoryTypeIndex = mem;

		VkImportMemoryFdInfoKHR importi = {0};
		importi.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR;
		importi.fd = dfd;
		importi.handleType = htype;
		memi.pNext = &importi;

		VkMemoryDedicatedAllocateInfo dedi = {0};
		dedi.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
		dedi.image = image;
		importi.pNext = &dedi;

		res = vkAllocateMemory(dev, &memi, NULL, &mems[i]);
		if (res != VK_SUCCESS) {
			close(dfd);
			wlr_vk_error("vkAllocateMemory failed", res);
			goto error_image;
		}

		++(*n_mems);

		// fill bind info
		bindi[i].image = image;
		bindi[i].memory = mems[i];
		bindi[i].memoryOffset = 0;
		bindi[i].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;

		if (disjoint) {
			planei[i].sType = VK_STRUCTURE_TYPE_BIND_IMAGE_PLANE_MEMORY_INFO;
			planei[i].planeAspect = planeri.planeAspect;
			bindi[i].pNext = &planei[i];
		}
	}

	res = vkBindImageMemory2(dev, mem_count, bindi);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkBindMemory failed", res);
		goto error_image;
	}

	return image;

error_image:
	vkDestroyImage(dev, image, NULL);
	for (size_t i = 0u; i < *n_mems; ++i) {
		vkFreeMemory(dev, mems[i], NULL);
		mems[i] = VK_NULL_HANDLE;
	}

	return VK_NULL_HANDLE;
}

static struct wlr_texture *vulkan_texture_from_dmabuf(struct wlr_renderer *wlr_renderer,
		struct wlr_dmabuf_attributes *attribs) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);

	VkResult res;
	VkDevice dev = renderer->dev->dev;

	const struct wlr_vk_format_props *fmt = vulkan_format_props_from_drm(
		renderer->dev, attribs->format);
	if (fmt == NULL) {
		wlr_log(WLR_ERROR, "Unsupported pixel format %"PRIx32 " (%.4s)",
			attribs->format, (const char*) &attribs->format);
		return NULL;
	}

	struct wlr_vk_texture *texture = vulkan_texture_create(renderer,
		attribs->width, attribs->height);
	if (texture == NULL) {
		return NULL;
	}

	texture->format = &fmt->format;
	texture->image = vulkan_import_dmabuf(renderer, attribs,
		texture->memories, &texture->mem_count, false);
	if (!texture->image) {
		goto error;
	}

	const struct wlr_pixel_format_info *format_info = drm_get_pixel_format_info(attribs->format);
	assert(format_info);

	// view
	VkImageViewCreateInfo view_info = {0};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = texture->format->vk_format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = format_info->has_alpha
		? VK_COMPONENT_SWIZZLE_IDENTITY
		: VK_COMPONENT_SWIZZLE_ONE;

	view_info.subresourceRange = (VkImageSubresourceRange) {
		VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
	};
	view_info.image = texture->image;

	res = vkCreateImageView(dev, &view_info, NULL, &texture->image_view);
	if (res != VK_SUCCESS) {
		wlr_vk_error("vkCreateImageView failed", res);
		goto error;
	}

	// descriptor
	texture->ds_pool = vulkan_alloc_texture_ds(renderer, &texture->ds);
	if (!texture->ds_pool) {
		wlr_log(WLR_ERROR, "failed to allocate descriptor");
		goto error;
	}

	VkDescriptorImageInfo ds_img_info = {0};
	ds_img_info.imageView = texture->image_view;
	ds_img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet ds_write = {0};
	ds_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	ds_write.descriptorCount = 1;
	ds_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ds_write.dstSet = texture->ds;
	ds_write.pImageInfo = &ds_img_info;

	vkUpdateDescriptorSets(dev, 1, &ds_write, 0, NULL);
	texture->dmabuf_imported = true;

	return &texture->wlr_texture;

error:
	vulkan_texture_destroy(texture);
	return NULL;
}

static void texture_handle_buffer_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_vk_texture *texture =
		wl_container_of(listener, texture, buffer_destroy);
	vulkan_texture_destroy(texture);
}

static struct wlr_texture *vulkan_texture_from_dmabuf_buffer(
		struct wlr_vk_renderer *renderer, struct wlr_buffer *buffer,
		struct wlr_dmabuf_attributes *dmabuf) {
	struct wlr_vk_texture *texture;
	wl_list_for_each(texture, &renderer->textures, link) {
		if (texture->buffer == buffer) {
			wlr_buffer_lock(texture->buffer);
			return &texture->wlr_texture;
		}
	}

	struct wlr_texture *wlr_texture =
		vulkan_texture_from_dmabuf(&renderer->wlr_renderer, dmabuf);
	if (wlr_texture == NULL) {
		return false;
	}

	texture = vulkan_get_texture(wlr_texture);
	texture->buffer = wlr_buffer_lock(buffer);

	texture->buffer_destroy.notify = texture_handle_buffer_destroy;
	wl_signal_add(&buffer->events.destroy, &texture->buffer_destroy);

	return &texture->wlr_texture;
}

struct wlr_texture *vulkan_texture_from_buffer(
		struct wlr_renderer *wlr_renderer,
		struct wlr_buffer *buffer) {
	struct wlr_vk_renderer *renderer = vulkan_get_renderer(wlr_renderer);

	void *data;
	uint32_t format;
	size_t stride;
	struct wlr_dmabuf_attributes dmabuf;
	if (wlr_buffer_get_dmabuf(buffer, &dmabuf)) {
		return vulkan_texture_from_dmabuf_buffer(renderer, buffer, &dmabuf);
	} else if (wlr_buffer_begin_data_ptr_access(buffer,
			WLR_BUFFER_DATA_PTR_ACCESS_READ, &data, &format, &stride)) {
		struct wlr_texture *tex = vulkan_texture_from_pixels(wlr_renderer,
			format, stride, buffer->width, buffer->height, data);
		wlr_buffer_end_data_ptr_access(buffer);
		return tex;
	} else {
		return NULL;
	}
}
