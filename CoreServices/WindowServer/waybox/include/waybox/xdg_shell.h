#ifndef _WB_XDG_SHELL_H
#define _WB_XDG_SHELL_H

#include "waybox/server.h"

void init_xdg_shell(struct wb_server *server);
void focus_view(struct wb_view *view, struct wlr_surface *surface);
struct wlr_output *get_active_output(struct wb_view *view);
struct wb_view *get_view_at(
		struct wb_server *server, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy);
#endif
