#ifndef TYPES_WLR_DATA_DEVICE_H
#define TYPES_WLR_DATA_DEVICE_H

#include <wayland-server-core.h>

#define DATA_DEVICE_ALL_ACTIONS (WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY | \
	WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE | \
	WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK)

struct wlr_client_data_source {
	struct wlr_data_source source;
	struct wlr_data_source_impl impl;
	struct wl_resource *resource;
	bool finalized;
};

extern const struct wlr_surface_role drag_icon_surface_role;

struct wlr_data_offer *data_offer_create(struct wl_resource *device_resource,
	struct wlr_data_source *source, enum wlr_data_offer_type type);
void data_offer_update_action(struct wlr_data_offer *offer);
void data_offer_destroy(struct wlr_data_offer *offer);

struct wlr_client_data_source *client_data_source_create(
	struct wl_client *client, uint32_t version, uint32_t id,
	struct wl_list *resource_list);
struct wlr_client_data_source *client_data_source_from_resource(
	struct wl_resource *resource);
void data_source_notify_finish(struct wlr_data_source *source);

struct wlr_seat_client *seat_client_from_data_device_resource(
	struct wl_resource *resource);
/**
 * Creates a new wl_data_offer if there is a wl_data_source currently set as
 * the seat selection and sends it to the seat client, followed by the
 * wl_data_device.selection() event.  If there is no current selection, the
 * wl_data_device.selection() event will carry a NULL wl_data_offer.  If the
 * client does not have a wl_data_device for the seat nothing will be done.
 */
void seat_client_send_selection(struct wlr_seat_client *seat_client);

#endif
