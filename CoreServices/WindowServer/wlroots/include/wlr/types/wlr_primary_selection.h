/*
 * This an unstable interface of wlroots. No guarantees are made regarding the
 * future consistency of this API.
 */
#ifndef WLR_USE_UNSTABLE
#error "Add -DWLR_USE_UNSTABLE to enable unstable wlroots features"
#endif

#ifndef WLR_TYPES_WLR_PRIMARY_SELECTION_H
#define WLR_TYPES_WLR_PRIMARY_SELECTION_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_seat.h>

struct wlr_primary_selection_source;

/**
 * A data source implementation. Only the `send` function is mandatory.
 */
struct wlr_primary_selection_source_impl {
	void (*send)(struct wlr_primary_selection_source *source,
		const char *mime_type, int fd);
	void (*destroy)(struct wlr_primary_selection_source *source);
};

/**
 * A source is the sending side of a selection.
 */
struct wlr_primary_selection_source {
	const struct wlr_primary_selection_source_impl *impl;

	// source metadata
	struct wl_array mime_types;

	struct {
		struct wl_signal destroy;
	} events;

	void *data;
};

void wlr_primary_selection_source_init(
	struct wlr_primary_selection_source *source,
	const struct wlr_primary_selection_source_impl *impl);
void wlr_primary_selection_source_destroy(
	struct wlr_primary_selection_source *source);
void wlr_primary_selection_source_send(
	struct wlr_primary_selection_source *source, const char *mime_type,
	int fd);

/**
 * Request setting the primary selection. If `client` is not null, then the
 * serial will be checked against the set of serials sent to the client on that
 * seat.
 */
void wlr_seat_request_set_primary_selection(struct wlr_seat *seat,
	struct wlr_seat_client *client,
	struct wlr_primary_selection_source *source, uint32_t serial);
/**
 * Sets the current primary selection for the seat. NULL can be provided to
 * clear it. This removes the previous one if there was any. In case the
 * selection doesn't come from a client, wl_display_next_serial() can be used to
 * generate a serial.
 */
void wlr_seat_set_primary_selection(struct wlr_seat *seat,
	struct wlr_primary_selection_source *source, uint32_t serial);

#endif
