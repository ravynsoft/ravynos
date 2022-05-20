#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <xf86drm.h>
#include <vulkan/vulkan.h>
#include <wlr/util/log.h>
#include <wlr/version.h>
#include <wlr/config.h>
#include "render/vulkan.h"

// Returns the name of the first extension that could not be found or NULL.
static const char *find_extensions(const VkExtensionProperties *avail,
		unsigned availc, const char **req, unsigned reqc) {
	// check if all required extensions are supported
	for (size_t i = 0; i < reqc; ++i) {
		bool found = false;
		for (size_t j = 0; j < availc; ++j) {
			if (!strcmp(avail[j].extensionName, req[i])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return req[i];
		}
	}

	return NULL;
}

static VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
		VkDebugUtilsMessageTypeFlagsEXT type,
		const VkDebugUtilsMessengerCallbackDataEXT *debug_data,
		void *data) {
	// we ignore some of the non-helpful warnings
	static const char *const ignored[] = {
		// notifies us that shader output is not consumed since
		// we use the shared vertex buffer with uv output
		"UNASSIGNED-CoreValidation-Shader-OutputNotConsumed",
	};

	if (debug_data->pMessageIdName) {
		for (unsigned i = 0; i < sizeof(ignored) / sizeof(ignored[0]); ++i) {
			if (strcmp(debug_data->pMessageIdName, ignored[i]) == 0) {
				return false;
			}
		}
	}

	enum wlr_log_importance importance;
	switch (severity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		importance = WLR_ERROR;
		break;
	default:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		importance = WLR_INFO;
		break;
	}

	wlr_log(importance, "%s (%s)", debug_data->pMessage,
		debug_data->pMessageIdName);
	if (debug_data->queueLabelCount > 0) {
		const char *name = debug_data->pQueueLabels[0].pLabelName;
		if (name) {
			wlr_log(importance, "    last label '%s'", name);
		}
	}

	for (unsigned i = 0; i < debug_data->objectCount; ++i) {
		if (debug_data->pObjects[i].pObjectName) {
			wlr_log(importance, "    involving '%s'", debug_data->pMessage);
		}
	}

	return false;
}


// instance
struct wlr_vk_instance *vulkan_instance_create(size_t ext_count,
		const char **exts, bool debug) {
	// we require vulkan 1.1
	PFN_vkEnumerateInstanceVersion pfEnumInstanceVersion =
		(PFN_vkEnumerateInstanceVersion)
		vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion");
	if (!pfEnumInstanceVersion) {
		wlr_log(WLR_ERROR, "wlroots requires vulkan 1.1 which is not available");
		return NULL;
	}

	uint32_t ini_version;
	if (pfEnumInstanceVersion(&ini_version) != VK_SUCCESS ||
			ini_version < VK_API_VERSION_1_1) {
		wlr_log(WLR_ERROR, "wlroots requires vulkan 1.1 which is not available");
		return NULL;
	}

	// query extension support
	uint32_t avail_extc = 0;
	VkResult res;
	res = vkEnumerateInstanceExtensionProperties(NULL, &avail_extc, NULL);
	if ((res != VK_SUCCESS) || (avail_extc == 0)) {
		wlr_vk_error("Could not enumerate instance extensions (1)", res);
		return NULL;
	}

	VkExtensionProperties avail_ext_props[avail_extc + 1];
	res = vkEnumerateInstanceExtensionProperties(NULL, &avail_extc,
		avail_ext_props);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Could not enumerate instance extensions (2)", res);
		return NULL;
	}

	for (size_t j = 0; j < avail_extc; ++j) {
		wlr_log(WLR_DEBUG, "Vulkan instance extension %s v%"PRIu32,
			avail_ext_props[j].extensionName, avail_ext_props[j].specVersion);
	}

	// create instance
	struct wlr_vk_instance *ini = calloc(1, sizeof(*ini));
	if (!ini) {
		wlr_log_errno(WLR_ERROR, "allocation failed");
		return NULL;
	}

	bool debug_utils_found = false;
	ini->extensions = calloc(1 + ext_count, sizeof(*ini->extensions));
	if (!ini->extensions) {
		wlr_log_errno(WLR_ERROR, "allocation failed");
		goto error;
	}

	// find extensions
	for (unsigned i = 0; i < ext_count; ++i) {
		if (find_extensions(avail_ext_props, avail_extc, &exts[i], 1)) {
			wlr_log(WLR_DEBUG, "vulkan instance extension %s not found",
				exts[i]);
			continue;
		}

		ini->extensions[ini->extension_count++] = exts[i];
	}

	if (debug) {
		const char *name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		if (find_extensions(avail_ext_props, avail_extc, &name, 1) == NULL) {
			debug_utils_found = true;
			ini->extensions[ini->extension_count++] = name;
		}
	}

	VkApplicationInfo application_info = {0};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pEngineName = "wlroots";
	application_info.engineVersion = WLR_VERSION_NUM;
	application_info.apiVersion = VK_API_VERSION_1_1;

	const char *layers[] = {
		"VK_LAYER_KHRONOS_validation",
		// "VK_LAYER_RENDERDOC_Capture",
		// "VK_LAYER_live_introspection",
	};

	unsigned layer_count = debug * (sizeof(layers) / sizeof(layers[0]));

	VkInstanceCreateInfo instance_info = {0};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &application_info;
	instance_info.enabledExtensionCount = ini->extension_count;
	instance_info.ppEnabledExtensionNames = ini->extensions;
	instance_info.enabledLayerCount = layer_count;
	instance_info.ppEnabledLayerNames = layers;

	VkDebugUtilsMessageSeverityFlagsEXT severity =
		// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	VkDebugUtilsMessageTypeFlagsEXT types =
		// VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	VkDebugUtilsMessengerCreateInfoEXT debug_info = {0};
	debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_info.messageSeverity = severity;
	debug_info.messageType = types;
	debug_info.pfnUserCallback = &debug_callback;
	debug_info.pUserData = ini;

	if (debug_utils_found) {
		// already adding the debug utils messenger extension to
		// instance creation gives us additional information during
		// instance creation and destruction, can be useful for debugging
		// layers/extensions not being found.
		instance_info.pNext = &debug_info;
	}

	res = vkCreateInstance(&instance_info, NULL, &ini->instance);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Could not create instance", res);
		goto error;
	}

	// debug callback
	if (debug_utils_found) {
		ini->api.createDebugUtilsMessengerEXT =
			(PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				ini->instance, "vkCreateDebugUtilsMessengerEXT");
		ini->api.destroyDebugUtilsMessengerEXT =
			(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
				ini->instance, "vkDestroyDebugUtilsMessengerEXT");

		if (ini->api.createDebugUtilsMessengerEXT) {
			ini->api.createDebugUtilsMessengerEXT(ini->instance,
				&debug_info, NULL, &ini->messenger);
		} else {
			wlr_log(WLR_ERROR, "vkCreateDebugUtilsMessengerEXT not found");
		}
	}

	return ini;

error:
	vulkan_instance_destroy(ini);
	return NULL;
}

void vulkan_instance_destroy(struct wlr_vk_instance *ini) {
	if (!ini) {
		return;
	}

	if (ini->messenger && ini->api.destroyDebugUtilsMessengerEXT) {
		ini->api.destroyDebugUtilsMessengerEXT(ini->instance,
			ini->messenger, NULL);
	}

	if (ini->instance) {
		vkDestroyInstance(ini->instance, NULL);
	}

	free(ini->extensions);
	free(ini);
}

// physical device matching
static void log_phdev(const VkPhysicalDeviceProperties *props) {
	uint32_t vv_major = VK_VERSION_MAJOR(props->apiVersion);
	uint32_t vv_minor = VK_VERSION_MINOR(props->apiVersion);
	uint32_t vv_patch = VK_VERSION_PATCH(props->apiVersion);

	uint32_t dv_major = VK_VERSION_MAJOR(props->driverVersion);
	uint32_t dv_minor = VK_VERSION_MINOR(props->driverVersion);
	uint32_t dv_patch = VK_VERSION_PATCH(props->driverVersion);

	const char *dev_type = "unknown";
	switch (props->deviceType) {
	case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
		dev_type = "integrated";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
		dev_type = "discrete";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_CPU:
		dev_type = "cpu";
		break;
	case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
		dev_type = "vgpu";
		break;
	default:
		break;
	}

	wlr_log(WLR_INFO, "Vulkan device: '%s'", props->deviceName);
	wlr_log(WLR_INFO, "  Device type: '%s'", dev_type);
	wlr_log(WLR_INFO, "  Supported API version: %u.%u.%u", vv_major, vv_minor, vv_patch);
	wlr_log(WLR_INFO, "  Driver version: %u.%u.%u", dv_major, dv_minor, dv_patch);
}

VkPhysicalDevice vulkan_find_drm_phdev(struct wlr_vk_instance *ini, int drm_fd) {
	VkResult res;
	uint32_t num_phdevs;

	res = vkEnumeratePhysicalDevices(ini->instance, &num_phdevs, NULL);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Could not retrieve physical devices", res);
		return VK_NULL_HANDLE;
	}

	VkPhysicalDevice phdevs[1 + num_phdevs];
	res = vkEnumeratePhysicalDevices(ini->instance, &num_phdevs, phdevs);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Could not retrieve physical devices", res);
		return VK_NULL_HANDLE;
	}

	struct stat drm_stat = {0};
	if (fstat(drm_fd, &drm_stat) != 0) {
		wlr_log_errno(WLR_ERROR, "fstat failed");
		return VK_NULL_HANDLE;
	}

	for (uint32_t i = 0; i < num_phdevs; ++i) {
		VkPhysicalDevice phdev = phdevs[i];

		// check whether device supports vulkan 1.1, needed for
		// vkGetPhysicalDeviceProperties2
		VkPhysicalDeviceProperties phdev_props;
		vkGetPhysicalDeviceProperties(phdev, &phdev_props);

		log_phdev(&phdev_props);

		if (phdev_props.apiVersion < VK_API_VERSION_1_1) {
			// NOTE: we could additionaly check whether the
			// VkPhysicalDeviceProperties2KHR extension is supported but
			// implementations not supporting 1.1 are unlikely in future
			continue;
		}

		// check for extensions
		uint32_t avail_extc = 0;
		res = vkEnumerateDeviceExtensionProperties(phdev, NULL,
			&avail_extc, NULL);
		if ((res != VK_SUCCESS) || (avail_extc == 0)) {
			wlr_vk_error("  Could not enumerate device extensions", res);
			continue;
		}

		VkExtensionProperties avail_ext_props[avail_extc + 1];
		res = vkEnumerateDeviceExtensionProperties(phdev, NULL,
			&avail_extc, avail_ext_props);
		if (res != VK_SUCCESS) {
			wlr_vk_error("  Could not enumerate device extensions", res);
			continue;
		}

		const char *name = VK_EXT_PHYSICAL_DEVICE_DRM_EXTENSION_NAME;
		bool has_drm_props = find_extensions(avail_ext_props, avail_extc, &name, 1) == NULL;
		name = VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME;
		bool has_driver_props = find_extensions(avail_ext_props, avail_extc, &name, 1) == NULL;

		VkPhysicalDeviceProperties2 props = {0};
		props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

		VkPhysicalDeviceDrmPropertiesEXT drm_props = {0};
		drm_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRM_PROPERTIES_EXT;
		if (has_drm_props) {
			drm_props.pNext = props.pNext;
			props.pNext = &drm_props;
		}

		VkPhysicalDeviceDriverPropertiesKHR driver_props = {0};
		driver_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
		if (has_driver_props) {
			driver_props.pNext = props.pNext;
			props.pNext = &driver_props;
		}

		vkGetPhysicalDeviceProperties2(phdev, &props);

		if (has_driver_props) {
			wlr_log(WLR_INFO, "  Driver name: %s (%s)", driver_props.driverName, driver_props.driverInfo);
		}

		if (!has_drm_props) {
			wlr_log(WLR_DEBUG, "  Ignoring physical device \"%s\": "
				"VK_EXT_physical_device_drm not supported",
				phdev_props.deviceName);
			continue;
		}

		dev_t primary_devid = makedev(drm_props.primaryMajor, drm_props.primaryMinor);
		dev_t render_devid = makedev(drm_props.renderMajor, drm_props.renderMinor);
		if (primary_devid == drm_stat.st_rdev ||
				render_devid == drm_stat.st_rdev) {
			wlr_log(WLR_INFO, "Found matching Vulkan physical device: %s",
				phdev_props.deviceName);
			return phdev;
		}
	}

	return VK_NULL_HANDLE;
}

struct wlr_vk_device *vulkan_device_create(struct wlr_vk_instance *ini,
		VkPhysicalDevice phdev, size_t ext_count, const char **exts) {
	VkResult res;

	// check for extensions
	uint32_t avail_extc = 0;
	res = vkEnumerateDeviceExtensionProperties(phdev, NULL,
		&avail_extc, NULL);
	if (res != VK_SUCCESS || avail_extc == 0) {
		wlr_vk_error("Could not enumerate device extensions (1)", res);
		return NULL;
	}

	VkExtensionProperties avail_ext_props[avail_extc + 1];
	res = vkEnumerateDeviceExtensionProperties(phdev, NULL,
		&avail_extc, avail_ext_props);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Could not enumerate device extensions (2)", res);
		return NULL;
	}

	for (size_t j = 0; j < avail_extc; ++j) {
		wlr_log(WLR_DEBUG, "Vulkan device extension %s v%"PRIu32,
			avail_ext_props[j].extensionName, avail_ext_props[j].specVersion);
	}

	// create device
	struct wlr_vk_device *dev = calloc(1, sizeof(*dev));
	if (!dev) {
		wlr_log_errno(WLR_ERROR, "allocation failed");
		return NULL;
	}

	dev->phdev = phdev;
	dev->instance = ini;
	dev->drm_fd = -1;
	dev->extensions = calloc(16 + ext_count, sizeof(*ini->extensions));
	if (!dev->extensions) {
		wlr_log_errno(WLR_ERROR, "allocation failed");
		goto error;
	}

	// find extensions
	for (unsigned i = 0; i < ext_count; ++i) {
		if (find_extensions(avail_ext_props, avail_extc, &exts[i], 1)) {
			wlr_log(WLR_DEBUG, "vulkan device extension %s not found",
				exts[i]);
			continue;
		}

		dev->extensions[dev->extension_count++] = exts[i];
	}

	// For dmabuf import we require at least the external_memory_fd,
	// external_memory_dma_buf, queue_family_foreign and
	// image_drm_format_modifier extensions.
	const char *names[] = {
		VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME,
		VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME, // or vulkan 1.2
		VK_EXT_EXTERNAL_MEMORY_DMA_BUF_EXTENSION_NAME,
		VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME,
		VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME,
	};

	unsigned nc = sizeof(names) / sizeof(names[0]);
	const char *not_found = find_extensions(avail_ext_props, avail_extc, names, nc);
	if (not_found) {
		wlr_log(WLR_ERROR, "vulkan: required device extension %s not found",
			not_found);
		goto error;
	}

	for (unsigned i = 0u; i < nc; ++i) {
		dev->extensions[dev->extension_count++] = names[i];
	}

	// queue families
	{
		uint32_t qfam_count;
		vkGetPhysicalDeviceQueueFamilyProperties(phdev, &qfam_count, NULL);
		assert(qfam_count > 0);
		VkQueueFamilyProperties queue_props[qfam_count];
		vkGetPhysicalDeviceQueueFamilyProperties(phdev, &qfam_count,
			queue_props);

		bool graphics_found = false;
		for (unsigned i = 0u; i < qfam_count; ++i) {
			graphics_found = queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
			if (graphics_found) {
				dev->queue_family = i;
				break;
			}
		}

		assert(graphics_found);
	}

	const float prio = 1.f;
	VkDeviceQueueCreateInfo qinfo = {};
	qinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	qinfo.queueFamilyIndex = dev->queue_family;
	qinfo.queueCount = 1;
	qinfo.pQueuePriorities = &prio;

	VkDeviceCreateInfo dev_info = {0};
	dev_info.pNext = NULL;
	dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dev_info.queueCreateInfoCount = 1u;
	dev_info.pQueueCreateInfos = &qinfo;
	dev_info.enabledExtensionCount = dev->extension_count;
	dev_info.ppEnabledExtensionNames = dev->extensions;

	res = vkCreateDevice(phdev, &dev_info, NULL, &dev->dev);
	if (res != VK_SUCCESS) {
		wlr_vk_error("Failed to create vulkan device", res);
		goto error;
	}


	vkGetDeviceQueue(dev->dev, dev->queue_family, 0, &dev->queue);

	// load api
	dev->api.getMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)
		vkGetDeviceProcAddr(dev->dev, "vkGetMemoryFdPropertiesKHR");

	if (!dev->api.getMemoryFdPropertiesKHR) {
		wlr_log(WLR_ERROR, "Failed to retrieve required dev function pointers");
		goto error;
	}

	// - check device format support -
	size_t max_fmts;
	const struct wlr_vk_format *fmts = vulkan_get_format_list(&max_fmts);
	dev->shm_formats = calloc(max_fmts, sizeof(*dev->shm_formats));
	dev->format_props = calloc(max_fmts, sizeof(*dev->format_props));
	if (!dev->shm_formats || !dev->format_props) {
		wlr_log_errno(WLR_ERROR, "allocation failed");
		goto error;
	}

	for (unsigned i = 0u; i < max_fmts; ++i) {
		vulkan_format_props_query(dev, &fmts[i]);
	}

	return dev;

error:
	vulkan_device_destroy(dev);
	return NULL;
}

void vulkan_device_destroy(struct wlr_vk_device *dev) {
	if (!dev) {
		return;
	}

	if (dev->dev) {
		vkDestroyDevice(dev->dev, NULL);
	}

	if (dev->drm_fd > 0) {
		close(dev->drm_fd);
	}

	wlr_drm_format_set_finish(&dev->dmabuf_render_formats);
	wlr_drm_format_set_finish(&dev->dmabuf_texture_formats);

	for (unsigned i = 0u; i < dev->format_prop_count; ++i) {
		vulkan_format_props_finish(&dev->format_props[i]);
	}

	free(dev->extensions);
	free(dev->shm_formats);
	free(dev->format_props);
	free(dev);
}
