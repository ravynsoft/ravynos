#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/util/log.h>
#include "backend/headless.h"
#include "util/signal.h"

static const uint32_t SUPPORTED_OUTPUT_STATE =
	WLR_OUTPUT_STATE_BACKEND_OPTIONAL |
	WLR_OUTPUT_STATE_BUFFER |
	WLR_OUTPUT_STATE_MODE;

static struct wlr_headless_output *headless_output_from_output(
		struct wlr_output *wlr_output) {
	assert(wlr_output_is_headless(wlr_output));
	return (struct wlr_headless_output *)wlr_output;
}

static bool output_set_custom_mode(struct wlr_headless_output *output,
		int32_t width, int32_t height, int32_t refresh) {
	if (refresh <= 0) {
		refresh = HEADLESS_DEFAULT_REFRESH;
	}

	output->frame_delay = 1000000 / refresh;

	wlr_output_update_custom_mode(&output->wlr_output, width, height, refresh);
	return true;
}

static bool output_test(struct wlr_output *wlr_output) {
	uint32_t unsupported =
		wlr_output->pending.committed & ~SUPPORTED_OUTPUT_STATE;
	if (unsupported != 0) {
		wlr_log(WLR_DEBUG, "Unsupported output state fields: 0x%"PRIx32,
			unsupported);
		return false;
	}

	if (wlr_output->pending.committed & WLR_OUTPUT_STATE_MODE) {
		assert(wlr_output->pending.mode_type == WLR_OUTPUT_STATE_MODE_CUSTOM);
	}

	return true;
}

static bool output_commit(struct wlr_output *wlr_output) {
	struct wlr_headless_output *output =
		headless_output_from_output(wlr_output);

	if (!output_test(wlr_output)) {
		return false;
	}

	if (wlr_output->pending.committed & WLR_OUTPUT_STATE_MODE) {
		if (!output_set_custom_mode(output,
				wlr_output->pending.custom_mode.width,
				wlr_output->pending.custom_mode.height,
				wlr_output->pending.custom_mode.refresh)) {
			return false;
		}
	}

	if (wlr_output->pending.committed & WLR_OUTPUT_STATE_BUFFER) {
		struct wlr_output_event_present present_event = {
			.commit_seq = wlr_output->commit_seq + 1,
			.presented = true,
		};
		wlr_output_send_present(wlr_output, &present_event);
	}

	return true;
}

static void output_destroy(struct wlr_output *wlr_output) {
	struct wlr_headless_output *output =
		headless_output_from_output(wlr_output);
	wl_list_remove(&output->link);
	wl_event_source_remove(output->frame_timer);
	free(output);
}

static const struct wlr_output_impl output_impl = {
	.destroy = output_destroy,
	.commit = output_commit,
};

bool wlr_output_is_headless(struct wlr_output *wlr_output) {
	return wlr_output->impl == &output_impl;
}

static int signal_frame(void *data) {
	struct wlr_headless_output *output = data;
	wlr_output_send_frame(&output->wlr_output);
	wl_event_source_timer_update(output->frame_timer, output->frame_delay);
	return 0;
}

struct wlr_output *wlr_headless_add_output(struct wlr_backend *wlr_backend,
		unsigned int width, unsigned int height) {
	struct wlr_headless_backend *backend =
		headless_backend_from_backend(wlr_backend);

	struct wlr_headless_output *output =
		calloc(1, sizeof(struct wlr_headless_output));
	if (output == NULL) {
		wlr_log(WLR_ERROR, "Failed to allocate wlr_headless_output");
		return NULL;
	}
	output->backend = backend;
	wlr_output_init(&output->wlr_output, &backend->backend, &output_impl,
		backend->display);
	struct wlr_output *wlr_output = &output->wlr_output;

	output_set_custom_mode(output, width, height, 0);
	strncpy(wlr_output->make, "headless", sizeof(wlr_output->make));
	strncpy(wlr_output->model, "headless", sizeof(wlr_output->model));

	char name[64];
	snprintf(name, sizeof(name), "HEADLESS-%zu", ++backend->last_output_num);
	wlr_output_set_name(wlr_output, name);

	char description[128];
	snprintf(description, sizeof(description),
		"Headless output %zu", backend->last_output_num);
	wlr_output_set_description(wlr_output, description);

	struct wl_event_loop *ev = wl_display_get_event_loop(backend->display);
	output->frame_timer = wl_event_loop_add_timer(ev, signal_frame, output);

	wl_list_insert(&backend->outputs, &output->link);

	if (backend->started) {
		wl_event_source_timer_update(output->frame_timer, output->frame_delay);
		wlr_output_update_enabled(wlr_output, true);
		wlr_signal_emit_safe(&backend->backend.events.new_output, wlr_output);
	}

	return wlr_output;
}
