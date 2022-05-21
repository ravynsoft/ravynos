/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_ACTION_H
#define __LABWC_ACTION_H

struct server;
struct view;

struct action {
	uint32_t type;
	char *arg;
	struct wl_list link;
};

struct action *action_create(const char *action_name);
void action_list_free(struct wl_list *action_list);

void action(struct view *activator, struct server *server,
	struct wl_list *actions, uint32_t resize_edges);

#endif
