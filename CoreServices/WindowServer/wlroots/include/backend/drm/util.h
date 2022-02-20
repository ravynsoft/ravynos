#ifndef BACKEND_DRM_UTIL_H
#define BACKEND_DRM_UTIL_H

#include <stdint.h>
#include <wlr/types/wlr_output.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

// Calculates a more accurate refresh rate (mHz) than what mode itself provides
int32_t calculate_refresh_rate(const drmModeModeInfo *mode);
// Populates the make/model/phys_{width,height} of output from the edid data
void parse_edid(struct wlr_output *restrict output, size_t len,
	const uint8_t *data);
// Returns the string representation of a DRM output type
const char *conn_get_name(uint32_t type_id);

// Part of match_obj
enum {
	UNMATCHED = (uint32_t)-1,
	SKIP = (uint32_t)-2,
};

/*
 * Tries to match some DRM objects with some other DRM resource.
 * e.g. Match CRTCs with Encoders, CRTCs with Planes.
 *
 * objs contains a bit array which resources it can be matched with.
 * e.g. Bit 0 set means can be matched with res[0]
 *
 * res contains an index of which objs it is matched with or UNMATCHED.
 *
 * This solution is left in out.
 * Returns the total number of matched solutions.
 */
size_t match_obj(size_t num_objs, const uint32_t objs[static restrict num_objs],
		size_t num_res, const uint32_t res[static restrict num_res],
		uint32_t out[static restrict num_res]);

#endif
