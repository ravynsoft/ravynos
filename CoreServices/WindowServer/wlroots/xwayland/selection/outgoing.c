#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wayland-util.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_primary_selection.h>
#include <wlr/util/log.h>
#include <xcb/xfixes.h>
#include "xwayland/selection.h"
#include "xwayland/xwm.h"

static void xwm_selection_send_notify(struct wlr_xwm *xwm,
		xcb_selection_request_event_t *req, bool success) {
	xcb_selection_notify_event_t selection_notify = {
		.response_type = XCB_SELECTION_NOTIFY,
		.sequence = 0,
		.time = req->time,
		.requestor = req->requestor,
		.selection = req->selection,
		.target = req->target,
		.property = success ? req->property : XCB_ATOM_NONE,
	};

	wlr_log(WLR_DEBUG, "SendEvent destination=%" PRIu32 " SelectionNotify(31) time=%" PRIu32
		" requestor=%" PRIu32 " selection=%" PRIu32 " target=%" PRIu32 " property=%" PRIu32,
		req->requestor, req->time, req->requestor, req->selection, req->target,
		selection_notify.property);
	xcb_send_event(xwm->xcb_conn,
		0, // propagate
		req->requestor,
		XCB_EVENT_MASK_NO_EVENT,
		(const char *)&selection_notify);
	xcb_flush(xwm->xcb_conn);
}

static int xwm_selection_flush_source_data(
		struct wlr_xwm_selection_transfer *transfer) {
	xcb_change_property(transfer->selection->xwm->xcb_conn,
		XCB_PROP_MODE_REPLACE,
		transfer->request.requestor,
		transfer->request.property,
		transfer->request.target,
		8, // format
		transfer->source_data.size,
		transfer->source_data.data);
	xcb_flush(transfer->selection->xwm->xcb_conn);
	transfer->property_set = true;
	size_t length = transfer->source_data.size;
	transfer->source_data.size = 0;
	return length;
}

static void xwm_selection_transfer_start_outgoing(
		struct wlr_xwm_selection_transfer *transfer);

void xwm_selection_transfer_destroy_outgoing(
		struct wlr_xwm_selection_transfer *transfer) {
	wl_list_remove(&transfer->link);
	wlr_log(WLR_DEBUG, "Destroying transfer %p", transfer);

	xwm_selection_transfer_remove_event_source(transfer);
	xwm_selection_transfer_close_wl_client_fd(transfer);
	wl_array_release(&transfer->source_data);
	free(transfer);
}

static int xwm_data_source_read(int fd, uint32_t mask, void *data) {
	struct wlr_xwm_selection_transfer *transfer = data;
	struct wlr_xwm *xwm = transfer->selection->xwm;

	void *p;
	size_t current = transfer->source_data.size;
	if (transfer->source_data.size < INCR_CHUNK_SIZE) {
		p = wl_array_add(&transfer->source_data, INCR_CHUNK_SIZE);
		if (p == NULL) {
			wlr_log(WLR_ERROR, "Could not allocate selection source_data");
			goto error_out;
		}
	} else {
		p = (char *)transfer->source_data.data + transfer->source_data.size;
	}

	size_t available = transfer->source_data.alloc - current;
	ssize_t len = read(fd, p, available);
	if (len == -1) {
		wlr_log_errno(WLR_ERROR, "read error from data source");
		goto error_out;
	}

	wlr_log(WLR_DEBUG, "read %zd bytes (available %zu, mask 0x%x)", len,
		available, mask);

	transfer->source_data.size = current + len;
	if (transfer->source_data.size >= INCR_CHUNK_SIZE) {
		if (!transfer->incr) {
			wlr_log(WLR_DEBUG, "got %zu bytes, starting incr",
				transfer->source_data.size);

			size_t incr_chunk_size = INCR_CHUNK_SIZE;
			xcb_change_property(xwm->xcb_conn,
				XCB_PROP_MODE_REPLACE,
				transfer->request.requestor,
				transfer->request.property,
				xwm->atoms[INCR],
				32, /* format */
				1, &incr_chunk_size);
			transfer->incr = true;
			transfer->property_set = true;
			transfer->flush_property_on_delete = true;
			xwm_selection_transfer_remove_event_source(transfer);
			xwm_selection_send_notify(xwm, &transfer->request, true);
		} else if (transfer->property_set) {
			wlr_log(WLR_DEBUG, "got %zu bytes, waiting for property delete",
				transfer->source_data.size);

			transfer->flush_property_on_delete = true;
			xwm_selection_transfer_remove_event_source(transfer);
		} else {
			wlr_log(WLR_DEBUG, "got %zu bytes, property deleted, setting new "
				"property", transfer->source_data.size);
			xwm_selection_flush_source_data(transfer);
		}
	} else if (len == 0 && !transfer->incr) {
		wlr_log(WLR_DEBUG, "non-incr transfer complete");
		xwm_selection_flush_source_data(transfer);
		xwm_selection_send_notify(xwm, &transfer->request, true);
		xwm_selection_transfer_destroy_outgoing(transfer);
	} else if (len == 0 && transfer->incr) {
		wlr_log(WLR_DEBUG, "incr transfer complete");

		transfer->flush_property_on_delete = true;
		if (transfer->property_set) {
			wlr_log(WLR_DEBUG, "got %zu bytes, waiting for property delete",
				transfer->source_data.size);
		} else {
			wlr_log(WLR_DEBUG, "got %zu bytes, property deleted, setting new "
				"property", transfer->source_data.size);
			xwm_selection_flush_source_data(transfer);
		}
		xwm_selection_transfer_remove_event_source(transfer);
		xwm_selection_transfer_close_wl_client_fd(transfer);
	} else {
		wlr_log(WLR_DEBUG, "nothing happened, buffered the bytes");
	}

	return 1;

error_out:
	xwm_selection_send_notify(xwm, &transfer->request, false);
	xwm_selection_transfer_destroy_outgoing(transfer);
	return 0;
}

void xwm_send_incr_chunk(struct wlr_xwm_selection_transfer *transfer) {
	wlr_log(WLR_DEBUG, "property deleted");

	transfer->property_set = false;
	if (transfer->flush_property_on_delete) {
		wlr_log(WLR_DEBUG, "setting new property, %zu bytes",
			transfer->source_data.size);
		transfer->flush_property_on_delete = false;
		int length = xwm_selection_flush_source_data(transfer);

		if (transfer->wl_client_fd >= 0) {
			xwm_selection_transfer_start_outgoing(transfer);
		} else if (length > 0) {
			/* Transfer is all done, but queue a flush for
			 * the delete of the last chunk so we can set
			 * the 0 sized property to signal the end of
			 * the transfer. */
			transfer->flush_property_on_delete = true;
			wl_array_release(&transfer->source_data);
			wl_array_init(&transfer->source_data);
		} else {
			xwm_selection_transfer_destroy_outgoing(transfer);
		}
	}
}

static void xwm_selection_source_send(struct wlr_xwm_selection *selection,
		const char *mime_type, int32_t fd) {
	if (selection == &selection->xwm->clipboard_selection) {
		struct wlr_data_source *source =
			selection->xwm->seat->selection_source;
		if (source != NULL) {
			wlr_data_source_send(source, mime_type, fd);
			return;
		}
	} else if (selection == &selection->xwm->primary_selection) {
		struct wlr_primary_selection_source *source =
			selection->xwm->seat->primary_selection_source;
		if (source != NULL) {
			wlr_primary_selection_source_send(source, mime_type, fd);
			return;
		}
	} else if (selection == &selection->xwm->dnd_selection) {
		struct wlr_data_source *source =
			selection->xwm->seat->drag_source;
		if (source != NULL) {
			wlr_data_source_send(source, mime_type, fd);
			return;
		}
	}

	wlr_log(WLR_DEBUG, "not sending selection: no selection source available");
}

static void xwm_selection_transfer_start_outgoing(
		struct wlr_xwm_selection_transfer *transfer) {
	struct wlr_xwm *xwm = transfer->selection->xwm;
	struct wl_event_loop *loop =
		wl_display_get_event_loop(xwm->xwayland->wl_display);
	wlr_log(WLR_DEBUG, "Starting transfer %p", transfer);
	transfer->event_source = wl_event_loop_add_fd(loop, transfer->wl_client_fd,
		WL_EVENT_READABLE, xwm_data_source_read, transfer);
}

static struct wl_array *xwm_selection_source_get_mime_types(
		struct wlr_xwm_selection *selection) {
	if (selection == &selection->xwm->clipboard_selection) {
		struct wlr_data_source *source =
			selection->xwm->seat->selection_source;
		if (source != NULL) {
			return &source->mime_types;
		}
	} else if (selection == &selection->xwm->primary_selection) {
		struct wlr_primary_selection_source *source =
			selection->xwm->seat->primary_selection_source;
		if (source != NULL) {
			return &source->mime_types;
		}
	} else if (selection == &selection->xwm->dnd_selection) {
		struct wlr_data_source *source =
			selection->xwm->seat->drag_source;
		if (source != NULL) {
			return &source->mime_types;
		}
	}
	return NULL;
}

/**
 * Read the Wayland selection and send it to an Xwayland client.
 */
static bool xwm_selection_send_data(struct wlr_xwm_selection *selection,
		xcb_selection_request_event_t *req, const char *mime_type) {
	// Check MIME type
	struct wl_array *mime_types =
		xwm_selection_source_get_mime_types(selection);
	if (mime_types == NULL) {
		wlr_log(WLR_ERROR, "not sending selection: no MIME type list available");
		return false;
	}

	bool found = false;
	char **mime_type_ptr;
	wl_array_for_each(mime_type_ptr, mime_types) {
		char *t = *mime_type_ptr;
		if (strcmp(t, mime_type) == 0) {
			found = true;
			break;
		}
	}
	if (!found) {
		wlr_log(WLR_ERROR, "not sending selection: "
			"requested an unsupported MIME type %s", mime_type);
		return false;
	}

	struct wlr_xwm_selection_transfer *transfer =
		calloc(1, sizeof(struct wlr_xwm_selection_transfer));
	if (transfer == NULL) {
		wlr_log(WLR_ERROR, "Allocation failed");
		return false;
	}

	xwm_selection_transfer_init(transfer, selection);
	transfer->request = *req;
	wl_array_init(&transfer->source_data);

	int p[2];
	if (pipe(p) == -1) {
		wlr_log_errno(WLR_ERROR, "pipe() failed");
		return false;
	}

	fcntl(p[0], F_SETFD, FD_CLOEXEC);
	fcntl(p[0], F_SETFL, O_NONBLOCK);
	fcntl(p[1], F_SETFD, FD_CLOEXEC);
	fcntl(p[1], F_SETFL, O_NONBLOCK);

	transfer->wl_client_fd = p[0];

	wlr_log(WLR_DEBUG, "Sending Wayland selection %u to Xwayland window with "
		"MIME type %s, target %u, transfer %p", req->target, mime_type,
		req->target, transfer);
	xwm_selection_source_send(selection, mime_type, p[1]);

	// It seems that if we ever try to reply to a selection request after
	// another has been sent by the same requestor, the requestor never reads
	// from it. It appears to only ever read from the latest, so purge stale
	// transfers to prevent clipboard hangs.
	struct wlr_xwm_selection_transfer *outgoing, *tmp;
	wl_list_for_each_safe(outgoing, tmp, &selection->outgoing, link) {
		if (transfer->request.requestor == outgoing->request.requestor) {
			wlr_log(WLR_DEBUG, "Destroying stale transfer %p", outgoing);
			xwm_selection_send_notify(selection->xwm, &outgoing->request, false);
			xwm_selection_transfer_destroy_outgoing(outgoing);
		} else {
			wlr_log(WLR_DEBUG, "Transfer %p still running", outgoing);
		}
	}

	wl_list_insert(&selection->outgoing, &transfer->link);

	xwm_selection_transfer_start_outgoing(transfer);

	return true;
}

static void xwm_selection_send_targets(struct wlr_xwm_selection *selection,
		xcb_selection_request_event_t *req) {
	struct wlr_xwm *xwm = selection->xwm;

	struct wl_array *mime_types =
		xwm_selection_source_get_mime_types(selection);
	if (mime_types == NULL) {
		wlr_log(WLR_ERROR, "not sending selection targets: "
			"no selection source available");
		xwm_selection_send_notify(selection->xwm, req, false);
		return;
	}

	size_t n = 2 + mime_types->size / sizeof(char *);
	xcb_atom_t targets[n];
	targets[0] = xwm->atoms[TIMESTAMP];
	targets[1] = xwm->atoms[TARGETS];

	size_t i = 0;
	char **mime_type_ptr;
	wl_array_for_each(mime_type_ptr, mime_types) {
		char *mime_type = *mime_type_ptr;
		targets[2+i] = xwm_mime_type_to_atom(xwm, mime_type);
		++i;
	}

	xcb_change_property(xwm->xcb_conn,
		XCB_PROP_MODE_REPLACE,
		req->requestor,
		req->property,
		XCB_ATOM_ATOM,
		32, // format
		n, targets);

	xwm_selection_send_notify(selection->xwm, req, true);
}

static void xwm_selection_send_timestamp(struct wlr_xwm_selection *selection,
		xcb_selection_request_event_t *req) {
	xcb_change_property(selection->xwm->xcb_conn,
		XCB_PROP_MODE_REPLACE,
		req->requestor,
		req->property,
		XCB_ATOM_INTEGER,
		32, // format
		1, &selection->timestamp);

	xwm_selection_send_notify(selection->xwm, req, true);
}

void xwm_handle_selection_request(struct wlr_xwm *xwm,
		xcb_selection_request_event_t *req) {
	wlr_log(WLR_DEBUG, "XCB_SELECTION_REQUEST (time=%u owner=%u, requestor=%u "
		"selection=%u, target=%u, property=%u)",
		req->time, req->owner, req->requestor, req->selection, req->target,
		req->property);

	if (req->selection == xwm->atoms[CLIPBOARD_MANAGER]) {
		// The wlroots clipboard should already have grabbed the first target,
		// so just send selection notify now. This isn't synchronized with the
		// clipboard finishing getting the data, so there's a race here.
		xwm_selection_send_notify(xwm, req, true);
		return;
	}

	struct wlr_xwm_selection *selection =
		xwm_get_selection(xwm, req->selection);
	if (selection == NULL) {
		wlr_log(WLR_DEBUG, "received selection request for unknown selection");
		goto fail_notify_requestor;
	}

	if (req->requestor == selection->window) {
		wlr_log(WLR_ERROR, "selection request should have been caught before");
		goto fail_notify_requestor;
	}

	if (selection->window != req->owner) {
		if (req->time != XCB_CURRENT_TIME && req->time < selection->timestamp) {
			wlr_log(WLR_DEBUG, "ignored old request from timestamp %d; expected > %d",
					req->time, selection->timestamp);
			goto fail_notify_requestor;
		}

		wlr_log(WLR_DEBUG, "received selection request with invalid owner");
		// Don't fail (`goto fail_notify_requestor`) the selection request if we're
		// no longer the selection owner.
		return;
	}

	// No xwayland surface focused, deny access to clipboard
	if (xwm->focus_surface == NULL && xwm->drag_focus == NULL) {
		char *selection_name = xwm_get_atom_name(xwm, selection->atom);
		wlr_log(WLR_DEBUG, "denying read access to selection %u (%s): "
			"no xwayland surface focused", selection->atom, selection_name);
		free(selection_name);
		goto fail_notify_requestor;
	}

	if (req->target == xwm->atoms[TARGETS]) {
		xwm_selection_send_targets(selection, req);
	} else if (req->target == xwm->atoms[TIMESTAMP]) {
		xwm_selection_send_timestamp(selection, req);
	} else if (req->target == xwm->atoms[DELETE]) {
		xwm_selection_send_notify(selection->xwm, req, true);
	} else {
		// Send data
		char *mime_type = xwm_mime_type_from_atom(xwm, req->target);
		if (mime_type == NULL) {
			wlr_log(WLR_ERROR, "ignoring selection request: unknown atom %u",
				req->target);
			goto fail_notify_requestor;
		}

		bool send_success = xwm_selection_send_data(selection, req, mime_type);
		free(mime_type);
		if (!send_success) {
			goto fail_notify_requestor;
		}
	}

	return;

fail_notify_requestor:
	// Something went wrong, and there won't be any data being sent to the
	// requestor, so let them know.
	xwm_selection_send_notify(xwm, req, false);
}

void xwm_handle_selection_destroy_notify(struct wlr_xwm *xwm,
		xcb_destroy_notify_event_t *event) {
	struct wlr_xwm_selection *selections[] = {
		&xwm->clipboard_selection,
		&xwm->primary_selection,
		&xwm->dnd_selection,
	};

	for (size_t i = 0; i < sizeof(selections)/sizeof(selections[0]); ++i) {
		struct wlr_xwm_selection *selection = selections[i];

		struct wlr_xwm_selection_transfer *outgoing, *tmp;
		wl_list_for_each_safe(outgoing, tmp, &selection->outgoing, link) {
			if (event->window == outgoing->request.requestor) {
				xwm_selection_transfer_destroy_outgoing(outgoing);
			}
		}
	}
}
