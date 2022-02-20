#ifndef XWAYLAND_SELECTION_H
#define XWAYLAND_SELECTION_H

#include <xcb/xfixes.h>

#define INCR_CHUNK_SIZE (64 * 1024)

#define XDND_VERSION 5

struct wlr_primary_selection_source;

struct wlr_xwm_selection;

struct wlr_xwm_selection_transfer {
	struct wlr_xwm_selection *selection;

	bool incr;
	bool flush_property_on_delete;
	bool property_set;
	struct wl_array source_data;
	int wl_client_fd;
	struct wl_event_source *event_source;
	struct wl_list link;

	// when sending to x11
	xcb_selection_request_event_t request;

	// when receiving from x11
	int property_start;
	xcb_get_property_reply_t *property_reply;
	xcb_window_t incoming_window;
};

struct wlr_xwm_selection {
	struct wlr_xwm *xwm;

	xcb_atom_t atom;
	xcb_window_t window;
	xcb_window_t owner;
	xcb_timestamp_t timestamp;

	struct wl_list incoming;
	struct wl_list outgoing;
};

struct wlr_xwm_selection_transfer *
xwm_selection_find_incoming_transfer_by_window(
	struct wlr_xwm_selection *selection, xcb_window_t window);

void xwm_selection_transfer_remove_event_source(
	struct wlr_xwm_selection_transfer *transfer);
void xwm_selection_transfer_close_wl_client_fd(
	struct wlr_xwm_selection_transfer *transfer);
void xwm_selection_transfer_destroy_property_reply(
	struct wlr_xwm_selection_transfer *transfer);
void xwm_selection_transfer_init(struct wlr_xwm_selection_transfer *transfer,
	struct wlr_xwm_selection *selection);
void xwm_selection_transfer_destroy(
	struct wlr_xwm_selection_transfer *transfer);

void xwm_selection_transfer_destroy_outgoing(
	struct wlr_xwm_selection_transfer *transfer);

xcb_atom_t xwm_mime_type_to_atom(struct wlr_xwm *xwm, char *mime_type);
char *xwm_mime_type_from_atom(struct wlr_xwm *xwm, xcb_atom_t atom);
struct wlr_xwm_selection *xwm_get_selection(struct wlr_xwm *xwm,
	xcb_atom_t selection_atom);

void xwm_send_incr_chunk(struct wlr_xwm_selection_transfer *transfer);
void xwm_handle_selection_request(struct wlr_xwm *xwm,
	xcb_selection_request_event_t *req);
void xwm_handle_selection_destroy_notify(struct wlr_xwm *xwm,
		xcb_destroy_notify_event_t *event);

void xwm_get_incr_chunk(struct wlr_xwm_selection_transfer *transfer);
void xwm_handle_selection_notify(struct wlr_xwm *xwm,
	xcb_selection_notify_event_t *event);
int xwm_handle_xfixes_selection_notify(struct wlr_xwm *xwm,
	xcb_xfixes_selection_notify_event_t *event);
bool data_source_is_xwayland(struct wlr_data_source *wlr_source);
bool primary_selection_source_is_xwayland(
	struct wlr_primary_selection_source *wlr_source);

void xwm_seat_handle_start_drag(struct wlr_xwm *xwm, struct wlr_drag *drag);

void xwm_selection_init(struct wlr_xwm_selection *selection,
	struct wlr_xwm *xwm, xcb_atom_t atom);
void xwm_selection_finish(struct wlr_xwm_selection *selection);

#endif
