#ifndef TYPES_WLR_XDG_SHELL_H
#define TYPES_WLR_XDG_SHELL_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_xdg_shell.h>
#include "xdg-shell-protocol.h"

struct wlr_xdg_positioner_resource {
	struct wl_resource *resource;
	struct wlr_xdg_positioner attrs;
};

extern const struct wlr_surface_role xdg_toplevel_surface_role;
extern const struct wlr_surface_role xdg_popup_surface_role;

struct wlr_xdg_surface *create_xdg_surface(
	struct wlr_xdg_client *client, struct wlr_surface *wlr_surface,
	uint32_t id);
void unmap_xdg_surface(struct wlr_xdg_surface *surface);
void reset_xdg_surface(struct wlr_xdg_surface *surface);
void destroy_xdg_surface(struct wlr_xdg_surface *surface);
void xdg_surface_role_commit(struct wlr_surface *wlr_surface);
void xdg_surface_role_precommit(struct wlr_surface *wlr_surface,
	const struct wlr_surface_state *state);

void create_xdg_positioner(struct wlr_xdg_client *client, uint32_t id);
struct wlr_xdg_positioner_resource *get_xdg_positioner_from_resource(
	struct wl_resource *resource);

void create_xdg_popup(struct wlr_xdg_surface *surface,
	struct wlr_xdg_surface *parent,
	struct wlr_xdg_positioner_resource *positioner, uint32_t id);
void unmap_xdg_popup(struct wlr_xdg_popup *popup);
void handle_xdg_popup_committed(struct wlr_xdg_popup *popup);

void create_xdg_toplevel(struct wlr_xdg_surface *surface,
	uint32_t id);
void unmap_xdg_toplevel(struct wlr_xdg_toplevel *toplevel);
void handle_xdg_toplevel_committed(struct wlr_xdg_toplevel *toplevel);
struct wlr_xdg_toplevel_configure *send_xdg_toplevel_configure(
	struct wlr_xdg_toplevel *toplevel);
void handle_xdg_toplevel_ack_configure(struct wlr_xdg_toplevel *toplevel,
	struct wlr_xdg_toplevel_configure *configure);

#endif
