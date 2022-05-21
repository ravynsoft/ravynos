// SPDX-License-Identifier: GPL-2.0-only
/* view-impl.c: common code for shell view->impl functions */
#include <stdio.h>
#include <strings.h>
#include "labwc.h"

void
view_impl_map(struct view *view)
{
	desktop_focus_and_activate_view(&view->server->seat, view);
	desktop_move_to_front(view);

	view_update_title(view);
	view_update_app_id(view);

	damage_all_outputs(view->server);
}
