/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_MENU_H
#define __LABWC_MENU_H

#include <wayland-server.h>
#include <wlr/render/wlr_renderer.h>

struct menuitem {
	struct wl_list actions;
	struct menu *submenu;
	struct wlr_box box;
	struct {
		struct wlr_texture *active;
		struct wlr_texture *inactive;
		int offset_x;
		int offset_y;
	} texture;
	bool selected;
	struct wl_list link; /* menu::menuitems */
};

/* This could be the root-menu or a submenu */
struct menu {
	char *id;
	char *label;
	bool visible;
	struct menu *parent;
	struct wlr_box box;
	struct wl_list menuitems;
	struct server *server;
};

void menu_init_rootmenu(struct server *server);
void menu_init_windowmenu(struct server *server);
void menu_finish(void);

/* menu_move - move to position (x, y) */
void menu_move(struct menu *menu, int x, int y);

/* menu_set_selected - select item at (x, y) */
void menu_set_selected(struct menu *menu, int x, int y);

/* menu_action_selected - select item at (x, y) */
void menu_action_selected(struct server *server, struct menu *menu);

/* menu_reconfigure - reload theme and content */
void menu_reconfigure(struct server *server, struct menu *menu);

#endif /* __LABWC_MENU_H */
