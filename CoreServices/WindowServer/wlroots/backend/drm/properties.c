#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <wlr/util/log.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "backend/drm/properties.h"

/*
 * Creates a mapping between property names and an array index where to store
 * the ids.  The prop_info arrays must be sorted by name, as bsearch is used to
 * search them.
 */
struct prop_info {
	const char *name;
	size_t index;
};

static const struct prop_info connector_info[] = {
#define INDEX(name) (offsetof(union wlr_drm_connector_props, name) / sizeof(uint32_t))
	{ "CRTC_ID", INDEX(crtc_id) },
	{ "DPMS", INDEX(dpms) },
	{ "EDID", INDEX(edid) },
	{ "PATH", INDEX(path) },
	{ "link-status", INDEX(link_status) },
	{ "non-desktop", INDEX(non_desktop) },
	{ "panel orientation", INDEX(panel_orientation) },
	{ "subconnector", INDEX(subconnector) },
	{ "vrr_capable", INDEX(vrr_capable) },
#undef INDEX
};

static const struct prop_info crtc_info[] = {
#define INDEX(name) (offsetof(union wlr_drm_crtc_props, name) / sizeof(uint32_t))
	{ "ACTIVE", INDEX(active) },
	{ "GAMMA_LUT", INDEX(gamma_lut) },
	{ "GAMMA_LUT_SIZE", INDEX(gamma_lut_size) },
	{ "MODE_ID", INDEX(mode_id) },
	{ "VRR_ENABLED", INDEX(vrr_enabled) },
#undef INDEX
};

static const struct prop_info plane_info[] = {
#define INDEX(name) (offsetof(union wlr_drm_plane_props, name) / sizeof(uint32_t))
	{ "CRTC_H", INDEX(crtc_h) },
	{ "CRTC_ID", INDEX(crtc_id) },
	{ "CRTC_W", INDEX(crtc_w) },
	{ "CRTC_X", INDEX(crtc_x) },
	{ "CRTC_Y", INDEX(crtc_y) },
	{ "FB_DAMAGE_CLIPS", INDEX(fb_damage_clips) },
	{ "FB_ID", INDEX(fb_id) },
	{ "IN_FORMATS", INDEX(in_formats) },
	{ "SRC_H", INDEX(src_h) },
	{ "SRC_W", INDEX(src_w) },
	{ "SRC_X", INDEX(src_x) },
	{ "SRC_Y", INDEX(src_y) },
	{ "rotation", INDEX(rotation) },
	{ "type", INDEX(type) },
#undef INDEX
};

static int cmp_prop_info(const void *arg1, const void *arg2) {
	const char *key = arg1;
	const struct prop_info *elem = arg2;

	return strcmp(key, elem->name);
}

static bool scan_properties(int fd, uint32_t id, uint32_t type, uint32_t *result,
		const struct prop_info *info, size_t info_len) {
	drmModeObjectProperties *props = drmModeObjectGetProperties(fd, id, type);
	if (!props) {
		wlr_log_errno(WLR_ERROR, "Failed to get DRM object properties");
		return false;
	}

	for (uint32_t i = 0; i < props->count_props; ++i) {
		drmModePropertyRes *prop = drmModeGetProperty(fd, props->props[i]);
		if (!prop) {
			wlr_log_errno(WLR_ERROR, "Failed to get DRM object property");
			continue;
		}

		const struct prop_info *p =
			bsearch(prop->name, info, info_len, sizeof(info[0]), cmp_prop_info);
		if (p) {
			result[p->index] = prop->prop_id;
		}

		drmModeFreeProperty(prop);
	}

	drmModeFreeObjectProperties(props);
	return true;
}

bool get_drm_connector_props(int fd, uint32_t id,
		union wlr_drm_connector_props *out) {
	return scan_properties(fd, id, DRM_MODE_OBJECT_CONNECTOR, out->props,
		connector_info, sizeof(connector_info) / sizeof(connector_info[0]));
}

bool get_drm_crtc_props(int fd, uint32_t id, union wlr_drm_crtc_props *out) {
	return scan_properties(fd, id, DRM_MODE_OBJECT_CRTC, out->props,
		crtc_info, sizeof(crtc_info) / sizeof(crtc_info[0]));
}

bool get_drm_plane_props(int fd, uint32_t id, union wlr_drm_plane_props *out) {
	return scan_properties(fd, id, DRM_MODE_OBJECT_PLANE, out->props,
		plane_info, sizeof(plane_info) / sizeof(plane_info[0]));
}

bool get_drm_prop(int fd, uint32_t obj, uint32_t prop, uint64_t *ret) {
	drmModeObjectProperties *props =
		drmModeObjectGetProperties(fd, obj, DRM_MODE_OBJECT_ANY);
	if (!props) {
		return false;
	}

	bool found = false;

	for (uint32_t i = 0; i < props->count_props; ++i) {
		if (props->props[i] == prop) {
			*ret = props->prop_values[i];
			found = true;
			break;
		}
	}

	drmModeFreeObjectProperties(props);
	return found;
}

void *get_drm_prop_blob(int fd, uint32_t obj, uint32_t prop, size_t *ret_len) {
	uint64_t blob_id;
	if (!get_drm_prop(fd, obj, prop, &blob_id)) {
		return NULL;
	}

	drmModePropertyBlobRes *blob = drmModeGetPropertyBlob(fd, blob_id);
	if (!blob) {
		return NULL;
	}

	void *ptr = malloc(blob->length);
	if (!ptr) {
		drmModeFreePropertyBlob(blob);
		return NULL;
	}

	memcpy(ptr, blob->data, blob->length);
	*ret_len = blob->length;

	drmModeFreePropertyBlob(blob);
	return ptr;
}

char *get_drm_prop_enum(int fd, uint32_t obj, uint32_t prop_id) {
	uint64_t value;
	if (!get_drm_prop(fd, obj, prop_id, &value)) {
		return NULL;
	}

	drmModePropertyRes *prop = drmModeGetProperty(fd, prop_id);
	if (!prop) {
		return NULL;
	}

	char *str = NULL;
	for (int i = 0; i < prop->count_enums; i++) {
		if (prop->enums[i].value == value) {
			str = strdup(prop->enums[i].name);
			break;
		}
	}

	drmModeFreeProperty(prop);

	return str;
}
