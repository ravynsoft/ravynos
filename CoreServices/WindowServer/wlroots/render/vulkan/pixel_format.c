#include <drm_fourcc.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>
#include <wlr/util/log.h>
#include "render/vulkan.h"

// Reversed endianess of shm and vulkan format names
static const struct wlr_vk_format formats[] = {
	{
		.drm_format = DRM_FORMAT_ARGB8888,
		.vk_format = VK_FORMAT_B8G8R8A8_SRGB,
	},
	{
		.drm_format = DRM_FORMAT_XRGB8888,
		.vk_format = VK_FORMAT_B8G8R8A8_SRGB,
	},
	{
		.drm_format = DRM_FORMAT_XBGR8888,
		.vk_format = VK_FORMAT_R8G8B8A8_SRGB,
	},
	{
		.drm_format = DRM_FORMAT_ABGR8888,
		.vk_format = VK_FORMAT_R8G8B8A8_SRGB,
	},
};

const struct wlr_vk_format *vulkan_get_format_list(size_t *len) {
	*len = sizeof(formats) / sizeof(formats[0]);
	return formats;
}

const struct wlr_vk_format *vulkan_get_format_from_drm(uint32_t drm_format) {
	for (unsigned i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
		if (formats[i].drm_format == drm_format) {
			return &formats[i];
		}
	}
	return NULL;
}

static const VkImageUsageFlags render_usage =
	VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
static const VkImageUsageFlags tex_usage =
	VK_IMAGE_USAGE_SAMPLED_BIT |
	VK_IMAGE_USAGE_TRANSFER_DST_BIT;
static const VkImageUsageFlags dma_tex_usage =
	VK_IMAGE_USAGE_SAMPLED_BIT;

static const VkFormatFeatureFlags tex_features =
	VK_FORMAT_FEATURE_TRANSFER_SRC_BIT |
	VK_FORMAT_FEATURE_TRANSFER_DST_BIT |
	VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
	// NOTE: we don't strictly require this, we could create a NEAREST
	// sampler for formats that need it, in case this ever makes problems.
	VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
static const VkFormatFeatureFlags render_features =
	VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
	VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
static const VkFormatFeatureFlags dma_tex_features =
	VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
	// NOTE: we don't strictly require this, we could create a NEAREST
	// sampler for formats that need it, in case this ever makes problems.
	VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;

static bool query_modifier_support(struct wlr_vk_device *dev,
		struct wlr_vk_format_props *props, size_t modifier_count,
		VkPhysicalDeviceImageFormatInfo2 fmti) {
	VkResult res;

	VkFormatProperties2 fmtp = {0};
	fmtp.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;

	VkDrmFormatModifierPropertiesListEXT modp = {0};
	modp.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
	modp.drmFormatModifierCount = modifier_count;
	fmtp.pNext = &modp;

	// the first call to vkGetPhysicalDeviceFormatProperties2 did only
	// retrieve the number of modifiers, we now have to retrieve
	// the modifiers
	modp.pDrmFormatModifierProperties = calloc(modifier_count,
		sizeof(*modp.pDrmFormatModifierProperties));
	if (!modp.pDrmFormatModifierProperties) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		return false;
	}

	vkGetPhysicalDeviceFormatProperties2(dev->phdev,
		props->format.vk_format, &fmtp);

	props->render_mods = calloc(modp.drmFormatModifierCount,
		sizeof(*props->render_mods));
	if (!props->render_mods) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		free(modp.pDrmFormatModifierProperties);
		return false;
	}

	props->texture_mods = calloc(modp.drmFormatModifierCount,
		sizeof(*props->texture_mods));
	if (!props->texture_mods) {
		wlr_log_errno(WLR_ERROR, "Allocation failed");
		free(modp.pDrmFormatModifierProperties);
		free(props->render_mods);
		return false;
	}

	// detailed check
	// format info
	// only added if dmabuf/drm_fmt_ext supported
	VkPhysicalDeviceExternalImageFormatInfo efmti = {0};
	efmti.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO;
	efmti.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;

	fmti.tiling = VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT;
	fmti.pNext = &efmti;

	VkPhysicalDeviceImageDrmFormatModifierInfoEXT modi = {0};
	modi.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_DRM_FORMAT_MODIFIER_INFO_EXT;
	modi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	efmti.pNext = &modi;

	// format properties
	VkExternalImageFormatProperties efmtp = {0};
	efmtp.sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES;

	VkImageFormatProperties2 ifmtp = {0};
	ifmtp.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
	ifmtp.pNext = &efmtp;
	const VkExternalMemoryProperties *emp = &efmtp.externalMemoryProperties;

	bool found = false;

	for (unsigned i = 0u; i < modp.drmFormatModifierCount; ++i) {
		VkDrmFormatModifierPropertiesEXT m =
			modp.pDrmFormatModifierProperties[i];
		wlr_log(WLR_DEBUG, "  modifier: 0x%"PRIx64 ": features 0x%"PRIx32", %d planes",
			m.drmFormatModifier, m.drmFormatModifierTilingFeatures,
			m.drmFormatModifierPlaneCount);

		// check that specific modifier for render usage
		if ((m.drmFormatModifierTilingFeatures & render_features) == render_features) {
			fmti.usage = render_usage;

			modi.drmFormatModifier = m.drmFormatModifier;
			res = vkGetPhysicalDeviceImageFormatProperties2(
				dev->phdev, &fmti, &ifmtp);
			if (res != VK_SUCCESS) {
				if (res != VK_ERROR_FORMAT_NOT_SUPPORTED) {
					wlr_vk_error("vkGetPhysicalDeviceImageFormatProperties2",
						res);
				}

				wlr_log(WLR_DEBUG, "    >> rendering: format not supported");
			} else if (emp->externalMemoryFeatures &
					VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) {
				unsigned c = props->render_mod_count;
				VkExtent3D me = ifmtp.imageFormatProperties.maxExtent;
				VkExternalMemoryProperties emp = efmtp.externalMemoryProperties;
				props->render_mods[c].props = m;
				props->render_mods[c].max_extent.width = me.width;
				props->render_mods[c].max_extent.height = me.height;
				props->render_mods[c].dmabuf_flags = emp.externalMemoryFeatures;
				props->render_mods[c].export_imported =
					(emp.exportFromImportedHandleTypes &
					 VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);
				++props->render_mod_count;

				found = true;
				wlr_drm_format_set_add(&dev->dmabuf_render_formats,
					props->format.drm_format, m.drmFormatModifier);

				wlr_log(WLR_DEBUG, "    >> rendering: supported");
			} else {
				wlr_log(WLR_DEBUG, "    >> rendering: importing not supported");
			}
		} else {
			wlr_log(WLR_DEBUG, "    >> rendering: format features not supported");
		}

		// check that specific modifier for texture usage
		if ((m.drmFormatModifierTilingFeatures & dma_tex_features) == dma_tex_features) {
			fmti.usage = dma_tex_usage;

			modi.drmFormatModifier = m.drmFormatModifier;
			res = vkGetPhysicalDeviceImageFormatProperties2(
				dev->phdev, &fmti, &ifmtp);
			if (res != VK_SUCCESS) {
				if (res != VK_ERROR_FORMAT_NOT_SUPPORTED) {
					wlr_vk_error("vkGetPhysicalDeviceImageFormatProperties2",
						res);
				}

				wlr_log(WLR_DEBUG, "    >> dmatex: format not supported");
			} else if (emp->externalMemoryFeatures &
					VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) {
				unsigned c = props->texture_mod_count;
				VkExtent3D me = ifmtp.imageFormatProperties.maxExtent;
				VkExternalMemoryProperties emp = efmtp.externalMemoryProperties;
				props->texture_mods[c].props = m;
				props->texture_mods[c].max_extent.width = me.width;
				props->texture_mods[c].max_extent.height = me.height;
				props->texture_mods[c].dmabuf_flags = emp.externalMemoryFeatures;
				props->texture_mods[c].export_imported =
					(emp.exportFromImportedHandleTypes &
					 VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT);
				++props->texture_mod_count;

				found = true;
				wlr_drm_format_set_add(&dev->dmabuf_texture_formats,
					props->format.drm_format, m.drmFormatModifier);

				wlr_log(WLR_DEBUG, "    >> dmatex: supported");
			} else {
				wlr_log(WLR_DEBUG, "    >> dmatex: importing not supported");
			}
		} else {
			wlr_log(WLR_DEBUG, "    >> dmatex: format features not supported");
		}
	}

	free(modp.pDrmFormatModifierProperties);
	return found;
}

void vulkan_format_props_query(struct wlr_vk_device *dev,
		const struct wlr_vk_format *format) {

	wlr_log(WLR_DEBUG, "vulkan: Checking support for format %.4s (0x%" PRIx32 ")",
		(const char *)&format->drm_format, format->drm_format);
	VkResult res;

	// get general features and modifiers
	VkFormatProperties2 fmtp = {0};
	fmtp.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2;

	VkDrmFormatModifierPropertiesListEXT modp = {0};
	modp.sType = VK_STRUCTURE_TYPE_DRM_FORMAT_MODIFIER_PROPERTIES_LIST_EXT;
	fmtp.pNext = &modp;

	vkGetPhysicalDeviceFormatProperties2(dev->phdev,
		format->vk_format, &fmtp);

	// detailed check
	VkPhysicalDeviceImageFormatInfo2 fmti = {0};
	fmti.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	fmti.type = VK_IMAGE_TYPE_2D;
	fmti.format = format->vk_format;

	VkImageFormatProperties2 ifmtp = {0};
	ifmtp.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

	bool add_fmt_props = false;
	struct wlr_vk_format_props props = {0};
	props.format = *format;

	wlr_log(WLR_DEBUG, "  drmFormatModifierCount: %d", modp.drmFormatModifierCount);
	if (modp.drmFormatModifierCount > 0) {
		add_fmt_props |= query_modifier_support(dev, &props,
			modp.drmFormatModifierCount, fmti);
	}

	// non-dmabuf texture properties
	if (fmtp.formatProperties.optimalTilingFeatures & tex_features) {
		fmti.pNext = NULL;
		ifmtp.pNext = NULL;
		fmti.tiling = VK_IMAGE_TILING_OPTIMAL;
		fmti.usage = tex_usage;

		res = vkGetPhysicalDeviceImageFormatProperties2(
			dev->phdev, &fmti, &ifmtp);
		if (res != VK_SUCCESS) {
			if (res != VK_ERROR_FORMAT_NOT_SUPPORTED) {
				wlr_vk_error("vkGetPhysicalDeviceImageFormatProperties2",
					res);
			}

			wlr_log(WLR_DEBUG, " >> shmtex: format not supported");
		} else {
			VkExtent3D me = ifmtp.imageFormatProperties.maxExtent;
			props.max_extent.width = me.width;
			props.max_extent.height = me.height;
			props.features = fmtp.formatProperties.optimalTilingFeatures;

			wlr_log(WLR_DEBUG, " >> shmtex: supported");

			dev->shm_formats[dev->shm_format_count] = format->drm_format;
			++dev->shm_format_count;

			add_fmt_props = true;
		}
	} else {
		wlr_log(WLR_DEBUG, " >> shmtex: format features not supported");
	}

	if (add_fmt_props) {
		dev->format_props[dev->format_prop_count] = props;
		++dev->format_prop_count;
	}
}

void vulkan_format_props_finish(struct wlr_vk_format_props *props) {
	free(props->texture_mods);
	free(props->render_mods);
}

struct wlr_vk_format_modifier_props *vulkan_format_props_find_modifier(
		struct wlr_vk_format_props *props, uint64_t mod, bool render) {
	if (render) {
		for (unsigned i = 0u; i < props->render_mod_count; ++i) {
			if (props->render_mods[i].props.drmFormatModifier == mod) {
				return &props->render_mods[i];
			}
		}
	} else {
		for (unsigned i = 0u; i < props->texture_mod_count; ++i) {
			if (props->texture_mods[i].props.drmFormatModifier == mod) {
				return &props->texture_mods[i];
			}
		}
	}

	return NULL;
}

