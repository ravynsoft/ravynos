/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_TEXT_INPUT_V3_H
#define WLR_TYPES_WLR_TEXT_INPUT_V3_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/box.h>

enum wlr_text_input_v3_features {
	WLR_TEXT_INPUT_V3_FEATURE_SURROUNDING_TEXT = 1 << 0,
	WLR_TEXT_INPUT_V3_FEATURE_CONTENT_TYPE = 1 << 1,
	WLR_TEXT_INPUT_V3_FEATURE_CURSOR_RECTANGLE = 1 << 2,
};

struct wlr_text_input_v3_state {
	struct {
		char *text; // NULL is allowed and equivalent to empty string
		uint32_t cursor;
		uint32_t anchor;
	} surrounding;

	uint32_t text_change_cause;

	struct {
		uint32_t hint;
		uint32_t purpose;
	} content_type;

	struct wlr_box cursor_rectangle;

	// Tracks which features were used in the current commit.
	// Useful in the enabling commit, where usage means support.
	uint32_t features; // OR'ed wlr_text_input_v3_features
};

struct wlr_text_input_v3 {
	struct wlr_seat *seat; // becomes null when seat destroyed
	struct wl_resource *resource;
	struct wlr_surface *focused_surface;
	struct wlr_text_input_v3_state pending;
	struct wlr_text_input_v3_state current;
	uint32_t current_serial; // next in line to send
	bool pending_enabled;
	bool current_enabled;
	// supported in the current text input, more granular than surface
	uint32_t active_features; // OR'ed wlr_text_input_v3_features

	struct wl_list link;

	struct wl_listener surface_destroy;
	struct wl_listener seat_destroy;

	struct {
		struct wl_signal enable; // (struct wlr_text_input_v3*)
		struct wl_signal commit; // (struct wlr_text_input_v3*)
		struct wl_signal disable; // (struct wlr_text_input_v3*)
		struct wl_signal destroy; // (struct wlr_text_input_v3*)
	} events;
};

struct wlr_text_input_manager_v3 {
	struct wl_global *global;
	struct wl_list text_inputs; // struct wlr_text_input_v3::resource::link

	struct wl_listener display_destroy;

	struct {
		struct wl_signal text_input; // (struct wlr_text_input_v3*)
		struct wl_signal destroy; // (struct wlr_input_method_manager_v3*)
	} events;
};

struct wlr_text_input_manager_v3 *wlr_text_input_manager_v3_create(
	struct wl_display *wl_display);

// Sends enter to the surface and saves it
void wlr_text_input_v3_send_enter(struct wlr_text_input_v3 *text_input,
	struct wlr_surface *wlr_surface);
// Sends leave to the currently focused surface and clears it
void wlr_text_input_v3_send_leave(struct wlr_text_input_v3 *text_input);
void wlr_text_input_v3_send_preedit_string(struct wlr_text_input_v3 *text_input,
	const char *text, int32_t cursor_begin, int32_t cursor_end);
void wlr_text_input_v3_send_commit_string(struct wlr_text_input_v3 *text_input,
	const char *text);
void wlr_text_input_v3_send_delete_surrounding_text(
	struct wlr_text_input_v3 *text_input, uint32_t before_length,
	uint32_t after_length);
void wlr_text_input_v3_send_done(struct wlr_text_input_v3 *text_input);

#endif
