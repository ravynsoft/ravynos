// SPDX-License-Identifier: GPL-2.0-only
#include <strings.h>
#include <wlr/util/log.h>
#include "common/spawn.h"
#include "common/zfree.h"
#include "labwc.h"
#include "menu/menu.h"
#include "ssd.h"
#include "action.h"

enum action_type {
	ACTION_TYPE_NONE = 0,
	ACTION_TYPE_CLOSE,
	ACTION_TYPE_DEBUG,
	ACTION_TYPE_EXECUTE,
	ACTION_TYPE_EXIT,
	ACTION_TYPE_MOVE_TO_EDGE,
	ACTION_TYPE_SNAP_TO_EDGE,
	ACTION_TYPE_NEXT_WINDOW,
	ACTION_TYPE_PREVIOUS_WINDOW,
	ACTION_TYPE_RECONFIGURE,
	ACTION_TYPE_SHOW_MENU,
	ACTION_TYPE_TOGGLE_MAXIMIZE,
	ACTION_TYPE_TOGGLE_FULLSCREEN,
	ACTION_TYPE_TOGGLE_DECORATIONS,
	ACTION_TYPE_FOCUS,
	ACTION_TYPE_ICONIFY,
	ACTION_TYPE_MOVE,
	ACTION_TYPE_RAISE,
	ACTION_TYPE_RESIZE,
};

const char *action_names[] = {
	"NoOp",
	"Close",
	"Debug",
	"Execute",
	"Exit",
	"MoveToEdge",
	"SnapToEdge",
	"NextWindow",
	"PreviousWindow",
	"Reconfigure",
	"ShowMenu",
	"ToggleMaximize",
	"ToggleFullscreen",
	"ToggleDecorations",
	"Focus",
	"Iconify",
	"Move",
	"Raise",
	"Resize",
	NULL
};


static enum action_type
action_type_from_str(const char *action_name)
{
	for (size_t i = 1; action_names[i] != NULL; i++) {
		if (!strcasecmp(action_name, action_names[i])) {
			return i;
		}
	}
	wlr_log(WLR_ERROR, "Invalid action: %s", action_name);
	return ACTION_TYPE_NONE;
}

struct action *
action_create(const char *action_name)
{
	if (!action_name) {
		wlr_log(WLR_ERROR, "action name not specified");
		return NULL;
	}
	struct action *action = calloc(1, sizeof(struct action));
	action->type = action_type_from_str(action_name);
	return action;
}

void action_list_free(struct wl_list *action_list) {
	struct action *action, *action_tmp;
	wl_list_for_each_safe(action, action_tmp, action_list, link) {
		wl_list_remove(&action->link);
		zfree(action->arg);
		zfree(action);
	}
}

static void
show_menu(struct server *server, struct view *view, const char *menu_name)
{
	struct menu *menu = NULL;
	bool force_menu_top_left = false;

	if (!menu_name) {
		return;
	}

	if (!strcasecmp(menu_name, "root-menu")) {
		menu = server->rootmenu;
		server->windowmenu->visible = false;
	} else if (!strcasecmp(menu_name, "client-menu") && view) {
		menu = server->windowmenu;
		server->rootmenu->visible = false;
		enum ssd_part_type type = ssd_at(view, server->seat.cursor->x,
			server->seat.cursor->y);
		if (type == LAB_SSD_BUTTON_WINDOW_MENU) {
			force_menu_top_left = true;
		} else if (ssd_part_contains(LAB_SSD_PART_TITLEBAR, type)) {
			force_menu_top_left = false;
		} else {
			force_menu_top_left = true;
		}
	} else {
		return;
	}

	menu->visible = true;
	server->input_mode = LAB_INPUT_STATE_MENU;

	int x, y;
	if (force_menu_top_left) {
		x = view->x;
		y = view->y;
	} else {
		x = server->seat.cursor->x;
		y = server->seat.cursor->y;
	}
	menu_move(menu, x, y);
	damage_all_outputs(server);
}

static struct view *
activator_or_focused_view(struct view *activator, struct server *server)
{
	return activator ? activator : desktop_focused_view(server);
}

void
action(struct view *activator, struct server *server, struct wl_list *actions, uint32_t resize_edges)
{
	if (!actions) {
		wlr_log(WLR_ERROR, "empty actions");
		return;
	}

	struct view *view;
	struct action *action;
	wl_list_for_each(action, actions, link) {
		wlr_log(WLR_DEBUG, "Handling action %s (%u) with arg %s",
			 action_names[action->type], action->type, action->arg);

		/* Refetch view because it may have been changed due to the previous action */
		view = activator_or_focused_view(activator, server);

		switch (action->type) {
		case ACTION_TYPE_CLOSE:
			if (view) {
				view_close(view);
			}
			break;
		case ACTION_TYPE_DEBUG:
			/* nothing */
			break;
		case ACTION_TYPE_EXECUTE:
			{
				struct buf cmd;
				buf_init(&cmd);
				buf_add(&cmd, action->arg);
				buf_expand_shell_variables(&cmd);
				spawn_async_no_shell(cmd.buf);
				free(cmd.buf);
			}
			break;
		case ACTION_TYPE_EXIT:
			wl_display_terminate(server->wl_display);
			break;
		case ACTION_TYPE_MOVE_TO_EDGE:
			view_move_to_edge(view, action->arg);
			break;
		case ACTION_TYPE_SNAP_TO_EDGE:
			view_snap_to_edge(view, action->arg);
			break;
		case ACTION_TYPE_NEXT_WINDOW:
			server->cycle_view =
				desktop_cycle_view(server, server->cycle_view, LAB_CYCLE_DIR_FORWARD);
			osd_update(server);
			break;
		case ACTION_TYPE_PREVIOUS_WINDOW:
			server->cycle_view =
				desktop_cycle_view(server, server->cycle_view, LAB_CYCLE_DIR_BACKWARD);
			osd_update(server);
			break;
		case ACTION_TYPE_RECONFIGURE:
			/* Should be changed to signal() */
			spawn_async_no_shell("killall -SIGHUP labwc");
			break;
		case ACTION_TYPE_SHOW_MENU:
			show_menu(server, view, action->arg);
			break;
		case ACTION_TYPE_TOGGLE_MAXIMIZE:
			if (view) {
				view_toggle_maximize(view);
			}
			break;
		case ACTION_TYPE_TOGGLE_FULLSCREEN:
			if (view) {
				view_toggle_fullscreen(view);
			}
			break;
		case ACTION_TYPE_TOGGLE_DECORATIONS:
			if (view) {
				view_toggle_decorations(view);
			}
			break;
		case ACTION_TYPE_FOCUS:
			view = desktop_view_at_cursor(server);
			if (view) {
				desktop_focus_and_activate_view(&server->seat, view);
				damage_all_outputs(server);
			}
			break;
		case ACTION_TYPE_ICONIFY:
			if (view) {
				view_minimize(view, true);
			}
			break;
		case ACTION_TYPE_MOVE:
			view = desktop_view_at_cursor(server);
			if (view) {
				interactive_begin(view, LAB_INPUT_STATE_MOVE, 0);
			}
			break;
		case ACTION_TYPE_RAISE:
			if (view) {
				desktop_move_to_front(view);
				damage_all_outputs(server);
			}
			break;
		case ACTION_TYPE_RESIZE:
			view = desktop_view_at_cursor(server);
			if (view) {
				interactive_begin(view, LAB_INPUT_STATE_RESIZE, resize_edges);
			}
			break;
		case ACTION_TYPE_NONE:
			wlr_log(WLR_ERROR, "Not executing unknown action with arg %s", action->arg);
			break;
		default:
			/*
			 * If we get here it must be a BUG caused most likely by
			 * action_names and action_type being out of sync or by
			 * adding a new action without installing a handler here.
			 */
			wlr_log(WLR_ERROR, "Not executing invalid action (%u) with arg %s"
				" This is a BUG. Please report.", action->type, action->arg);
		}
	}
}
