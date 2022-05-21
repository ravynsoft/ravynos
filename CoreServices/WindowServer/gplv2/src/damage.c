// SPDX-License-Identifier: GPL-2.0-only
#include "labwc.h"

void
damage_all_outputs(struct server *server)
{
	struct output *output;
	wl_list_for_each(output, &server->outputs, link) {
		if (output && output->wlr_output && output->damage) {
			wlr_output_damage_add_whole(output->damage);
		}
	}
}

void
damage_view_part(struct view *view)
{
	struct output *output;
	wl_list_for_each (output, &view->server->outputs, link) {
		output_damage_surface(output, view->surface, view->x, view->y,
			false);
	}
}

void
damage_view_whole(struct view *view)
{
	struct output *output;
	wl_list_for_each (output, &view->server->outputs, link) {
		output_damage_surface(output, view->surface, view->x, view->y,
			true);
	}
}
