#define _XOPEN_SOURCE 700
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include "types/wlr_data_device.h"
#include "util/signal.h"

static const struct wl_data_offer_interface data_offer_impl;

static struct wlr_data_offer *data_offer_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &wl_data_offer_interface,
		&data_offer_impl));
	return wl_resource_get_user_data(resource);
}

static uint32_t data_offer_choose_action(struct wlr_data_offer *offer) {
	uint32_t offer_actions, preferred_action = 0;
	if (wl_resource_get_version(offer->resource) >=
			WL_DATA_OFFER_ACTION_SINCE_VERSION) {
		offer_actions = offer->actions;
		preferred_action = offer->preferred_action;
	} else {
		offer_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
	}

	uint32_t source_actions;
	if (offer->source->actions >= 0) {
		source_actions = offer->source->actions;
	} else {
		source_actions = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
	}

	uint32_t available_actions = offer_actions & source_actions;
	if (!available_actions) {
		return WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
	}

	if (offer->source->compositor_action & available_actions) {
		return offer->source->compositor_action;
	}

	// If the dest side has a preferred DnD action, use it
	if ((preferred_action & available_actions) != 0) {
		return preferred_action;
	}

	// Use the first found action, in bit order
	return 1 << (ffs(available_actions) - 1);
}

void data_offer_update_action(struct wlr_data_offer *offer) {
	assert(offer->type == WLR_DATA_OFFER_DRAG);

	uint32_t action = data_offer_choose_action(offer);
	if (offer->source->current_dnd_action == action) {
		return;
	}
	offer->source->current_dnd_action = action;

	if (offer->in_ask) {
		return;
	}

	wlr_data_source_dnd_action(offer->source, action);

	if (wl_resource_get_version(offer->resource) >=
			WL_DATA_OFFER_ACTION_SINCE_VERSION) {
		wl_data_offer_send_action(offer->resource, action);
	}
}

static void data_offer_handle_accept(struct wl_client *client,
		struct wl_resource *resource, uint32_t serial, const char *mime_type) {
	struct wlr_data_offer *offer = data_offer_from_resource(resource);
	if (offer == NULL) {
		return;
	}

	if (offer->type != WLR_DATA_OFFER_DRAG) {
		wlr_log(WLR_DEBUG, "Ignoring wl_data_offer.accept request on a "
			"non-drag-and-drop offer");
		return;
	}

	wlr_data_source_accept(offer->source, serial, mime_type);
}

static void data_offer_handle_receive(struct wl_client *client,
		struct wl_resource *resource, const char *mime_type, int32_t fd) {
	struct wlr_data_offer *offer = data_offer_from_resource(resource);
	if (offer == NULL) {
		close(fd);
		return;
	}

	wlr_data_source_send(offer->source, mime_type, fd);
}

static void data_offer_source_dnd_finish(struct wlr_data_offer *offer) {
	struct wlr_data_source *source = offer->source;
	if (source->actions < 0) {
		return;
	}

	if (offer->in_ask) {
		wlr_data_source_dnd_action(source, source->current_dnd_action);
	}

	wlr_data_source_dnd_finish(source);
}

static void data_offer_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void data_offer_handle_finish(struct wl_client *client,
		struct wl_resource *resource) {
	struct wlr_data_offer *offer = data_offer_from_resource(resource);
	if (offer == NULL) {
		return;
	}

	// TODO: also fail while we have a drag-and-drop grab
	if (offer->type != WLR_DATA_OFFER_DRAG) {
		wl_resource_post_error(offer->resource,
			WL_DATA_OFFER_ERROR_INVALID_FINISH, "Offer is not drag-and-drop");
		return;
	}
	if (!offer->source->accepted) {
		wl_resource_post_error(offer->resource,
			WL_DATA_OFFER_ERROR_INVALID_FINISH, "Premature finish request");
		return;
	}
	enum wl_data_device_manager_dnd_action action =
		offer->source->current_dnd_action;
	if (action == WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE ||
			action == WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
		wl_resource_post_error(offer->resource,
			WL_DATA_OFFER_ERROR_INVALID_FINISH,
			"Offer finished with an invalid action");
		return;
	}

	data_offer_source_dnd_finish(offer);
	data_offer_destroy(offer);
}

static void data_offer_handle_set_actions(struct wl_client *client,
		struct wl_resource *resource, uint32_t actions,
		uint32_t preferred_action) {
	struct wlr_data_offer *offer = data_offer_from_resource(resource);
	if (offer == NULL) {
		return;
	}

	if (actions & ~DATA_DEVICE_ALL_ACTIONS) {
		wl_resource_post_error(offer->resource,
			WL_DATA_OFFER_ERROR_INVALID_ACTION_MASK,
			"invalid action mask %x", actions);
		return;
	}

	if (preferred_action && (!(preferred_action & actions) ||
			__builtin_popcount(preferred_action) > 1)) {
		wl_resource_post_error(offer->resource,
			WL_DATA_OFFER_ERROR_INVALID_ACTION,
			"invalid action %x", preferred_action);
		return;
	}

	if (offer->type != WLR_DATA_OFFER_DRAG) {
		wl_resource_post_error(offer->resource,
			WL_DATA_OFFER_ERROR_INVALID_OFFER,
			"set_action can only be sent to drag-and-drop offers");
		return;
	}

	offer->actions = actions;
	offer->preferred_action = preferred_action;

	data_offer_update_action(offer);
}

void data_offer_destroy(struct wlr_data_offer *offer) {
	if (offer == NULL) {
		return;
	}

	wl_list_remove(&offer->source_destroy.link);
	wl_list_remove(&offer->link);

	if (offer->type == WLR_DATA_OFFER_DRAG && offer->source) {
		// If the drag destination has version < 3, wl_data_offer.finish
		// won't be called, so do this here as a safety net, because
		// we still want the version >= 3 drag source to be happy.
		if (wl_resource_get_version(offer->resource) <
				WL_DATA_OFFER_ACTION_SINCE_VERSION) {
			data_offer_source_dnd_finish(offer);
		} else if (offer->source->impl->dnd_finish) {
			wlr_data_source_destroy(offer->source);
		}
	}

	// Make the resource inert
	wl_resource_set_user_data(offer->resource, NULL);

	free(offer);
}

static const struct wl_data_offer_interface data_offer_impl = {
	.accept = data_offer_handle_accept,
	.receive = data_offer_handle_receive,
	.destroy = data_offer_handle_destroy,
	.finish = data_offer_handle_finish,
	.set_actions = data_offer_handle_set_actions,
};

static void data_offer_handle_resource_destroy(struct wl_resource *resource) {
	struct wlr_data_offer *offer = data_offer_from_resource(resource);
	data_offer_destroy(offer);
}

static void data_offer_handle_source_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_data_offer *offer =
		wl_container_of(listener, offer, source_destroy);
	// Prevent data_offer_destroy from destroying the source again
	offer->source = NULL;
	data_offer_destroy(offer);
}

struct wlr_data_offer *data_offer_create(struct wl_resource *device_resource,
		struct wlr_data_source *source, enum wlr_data_offer_type type) {
	struct wlr_seat_client *seat_client =
		seat_client_from_data_device_resource(device_resource);
	assert(seat_client != NULL);
	assert(source != NULL); // a NULL source means no selection

	struct wlr_data_offer *offer = calloc(1, sizeof(struct wlr_data_offer));
	if (offer == NULL) {
		return NULL;
	}
	offer->source = source;
	offer->type = type;

	struct wl_client *client = wl_resource_get_client(device_resource);
	uint32_t version = wl_resource_get_version(device_resource);
	offer->resource =
		wl_resource_create(client, &wl_data_offer_interface, version, 0);
	if (offer->resource == NULL) {
		free(offer);
		return NULL;
	}
	wl_resource_set_implementation(offer->resource, &data_offer_impl, offer,
		data_offer_handle_resource_destroy);

	switch (type) {
	case WLR_DATA_OFFER_SELECTION:
		wl_list_insert(&seat_client->seat->selection_offers, &offer->link);
		break;
	case WLR_DATA_OFFER_DRAG:
		wl_list_insert(&seat_client->seat->drag_offers, &offer->link);
		break;
	}

	offer->source_destroy.notify = data_offer_handle_source_destroy;
	wl_signal_add(&source->events.destroy, &offer->source_destroy);

	wl_data_device_send_data_offer(device_resource, offer->resource);

	char **p;
	wl_array_for_each(p, &source->mime_types) {
		wl_data_offer_send_offer(offer->resource, *p);
	}

	return offer;
}
