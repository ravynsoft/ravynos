#ifndef TYPES_WLR_OUTPUT_H
#define TYPES_WLR_OUTPUT_H

#include <wlr/render/drm_format_set.h>
#include <wlr/types/wlr_output.h>

void output_pending_resolution(struct wlr_output *output, int *width,
	int *height);

struct wlr_drm_format *output_pick_format(struct wlr_output *output,
	const struct wlr_drm_format_set *display_formats, uint32_t format);
void output_clear_back_buffer(struct wlr_output *output);
bool output_ensure_buffer(struct wlr_output *output);

#endif
