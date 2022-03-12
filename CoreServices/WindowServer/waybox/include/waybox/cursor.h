#ifndef _WB_CURSOR_H
#define _WB_CURSOR_H
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>

struct wb_server;

enum wb_cursor_mode {
	WB_CURSOR_PASSTHROUGH,
	WB_CURSOR_MOVE,
	WB_CURSOR_RESIZE,
};

struct wb_cursor {
	struct wlr_cursor *cursor;
	struct wlr_xcursor_manager *xcursor_manager;

	struct wb_server *server;

	enum wb_cursor_mode cursor_mode;
	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;

	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;

	struct wl_listener request_cursor;
};

struct wb_cursor *wb_cursor_create(struct wb_server *server);
void wb_cursor_destroy(struct wb_cursor *cursor);

#endif /* cursor.h */
