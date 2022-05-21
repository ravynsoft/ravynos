// SPDX-License-Identifier: GPL-2.0-only
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wlr/util/log.h>
#include "common/buf.h"
#include "common/dir.h"
#include "common/font.h"
#include "common/nodename.h"
#include "common/string-helpers.h"
#include "common/zfree.h"
#include "labwc.h"
#include "menu/menu.h"
#include "theme.h"
#include "action.h"

#define MENUWIDTH (110)
#define MENU_ITEM_PADDING_Y (4)
#define MENU_ITEM_PADDING_X (7)

/* state-machine variables for processing <item></item> */
static bool in_item;
static struct menuitem *current_item;
static struct action *current_item_action;

static int menu_level;
static struct menu *current_menu;

/* vector for <menu id="" label=""> elements */
static struct menu *menus;
static int nr_menus, alloc_menus;

static struct menu *
menu_create(struct server *server, const char *id, const char *label)
{
	if (nr_menus == alloc_menus) {
		alloc_menus = (alloc_menus + 16) * 2;
		menus = realloc(menus, alloc_menus * sizeof(struct menu));
	}
	struct menu *menu = menus + nr_menus;
	memset(menu, 0, sizeof(*menu));
	nr_menus++;
	wl_list_init(&menu->menuitems);
	menu->id = strdup(id);
	menu->label = strdup(label);
	menu->parent = current_menu;
	menu->server = server;
	return menu;
}

static struct menu *
get_menu_by_id(const char *id)
{
	struct menu *menu;
	for (int i = 0; i < nr_menus; ++i) {
		menu = menus + i;
		if (!strcmp(menu->id, id)) {
			return menu;
		}
	}
	return NULL;
}

static struct menuitem *
item_create(struct menu *menu, const char *text)
{
	struct menuitem *menuitem = calloc(1, sizeof(struct menuitem));
	if (!menuitem) {
		return NULL;
	}
	struct server *server = menu->server;
	struct theme *theme = server->theme;
	struct font font = {
		.name = rc.font_name_menuitem,
		.size = rc.font_size_menuitem,
	};

	menuitem->box.width = MENUWIDTH;
	menuitem->box.height = font_height(&font) + 2 * MENU_ITEM_PADDING_Y;

	int item_max_width = MENUWIDTH - 2 * MENU_ITEM_PADDING_X;
	font_texture_create(server, &menuitem->texture.active, item_max_width,
		text, &font, theme->menu_items_active_text_color);
	font_texture_create(server, &menuitem->texture.inactive, item_max_width,
		text, &font, theme->menu_items_text_color);

	/* center align vertically */
	menuitem->texture.offset_y =
		(menuitem->box.height - menuitem->texture.active->height) / 2;
	menuitem->texture.offset_x = MENU_ITEM_PADDING_X;

	wl_list_insert(&menu->menuitems, &menuitem->link);
	wl_list_init(&menuitem->actions);
	return menuitem;
}

/*
 * Handle the following:
 * <item label="">
 *   <action name="">
 *     <command></command>
 *   </action>
 * </item>
 */
static void
fill_item(char *nodename, char *content)
{
	string_truncate_at_pattern(nodename, ".item.menu");

	/* <item label=""> defines the start of a new item */
	if (!strcmp(nodename, "label")) {
		current_item = item_create(current_menu, content);
		current_item_action = NULL;
	} else if (!current_item) {
		wlr_log(WLR_ERROR, "expect <item label=\"\"> element first. "
			"nodename: '%s' content: '%s'", nodename, content);
	} else if (!strcmp(nodename, "name.action")) {
		current_item_action = action_create(content);
		wl_list_insert(current_item->actions.prev, &current_item_action->link);
	} else if (!current_item_action) {
		wlr_log(WLR_ERROR, "expect <action name=\"\"> element first. "
			"nodename: '%s' content: '%s'", nodename, content);
	} else if (!strcmp(nodename, "command.action")) {
		current_item_action->arg = strdup(content);
	}
}

static void
entry(xmlNode *node, char *nodename, char *content)
{
	if (!nodename || !content) {
		return;
	}
	string_truncate_at_pattern(nodename, ".openbox_menu");
	if (in_item) {
		fill_item(nodename, content);
	}
}

static void
process_node(xmlNode *node)
{
	static char buffer[256];

	char *content = (char *)node->content;
	if (xmlIsBlankNode(node)) {
		return;
	}
	char *name = nodename(node, buffer, sizeof(buffer));
	entry(node, name, content);
}

static void xml_tree_walk(xmlNode *node, struct server *server);

static void
traverse(xmlNode *n, struct server *server)
{
	xmlAttr *attr;

	process_node(n);
	for (attr = n->properties; attr; attr = attr->next) {
		xml_tree_walk(attr->children, server);
	}
	xml_tree_walk(n->children, server);
}

/*
 * <menu> elements have three different roles:
 *  * Definition of (sub)menu - has ID, LABEL and CONTENT
 *  * Menuitem of pipemenu type - has EXECUTE and LABEL
 *  * Menuitem of submenu type - has ID only
 */
static void
handle_menu_element(xmlNode *n, struct server *server)
{
	char *label = (char *)xmlGetProp(n, (const xmlChar *)"label");
	char *execute = (char *)xmlGetProp(n, (const xmlChar *)"execute");
	char *id = (char *)xmlGetProp(n, (const xmlChar *)"id");

	if (execute) {
		wlr_log(WLR_ERROR, "we do not support pipemenus");
	} else if (label && id) {
		struct menu **submenu = NULL;
		if (menu_level > 0) {
			/*
			 * In a nested (inline) menu definition we need to
			 * create an item pointing to the new submenu
			 */
			current_item = item_create(current_menu, label);
			submenu = &current_item->submenu;
		}
		++menu_level;
		current_menu = menu_create(server, id, label);
		if (submenu) {
			*submenu = current_menu;
		}
		traverse(n, server);
		current_menu = current_menu->parent;
		--menu_level;
	} else if (id) {
		struct menu *menu = get_menu_by_id(id);
		if (menu) {
			current_item = item_create(current_menu, menu->label);
			current_item->submenu = menu;
		} else {
			wlr_log(WLR_ERROR, "no menu with id '%s'", id);
		}
	}
	zfree(label);
	zfree(execute);
	zfree(id);
}

static void
xml_tree_walk(xmlNode *node, struct server *server)
{
	for (xmlNode *n = node; n && n->name; n = n->next) {
		if (!strcasecmp((char *)n->name, "comment")) {
			continue;
		}
		if (!strcasecmp((char *)n->name, "menu")) {
			handle_menu_element(n, server);
			continue;
		}
		if (!strcasecmp((char *)n->name, "item")) {
			in_item = true;
			traverse(n, server);
			in_item = false;
			continue;
		}
		traverse(n, server);
	}
}

static void
parse_xml(const char *filename, struct server *server)
{
	FILE *stream;
	char *line = NULL;
	size_t len = 0;
	struct buf b;
	static char menuxml[4096] = { 0 };

	if (!strlen(config_dir())) {
		return;
	}
	snprintf(menuxml, sizeof(menuxml), "%s/%s", config_dir(), filename);

	stream = fopen(menuxml, "r");
	if (!stream) {
		wlr_log(WLR_ERROR, "cannot read %s", menuxml);
		return;
	}
	wlr_log(WLR_INFO, "read menu file %s", menuxml);
	buf_init(&b);
	while (getline(&line, &len, stream) != -1) {
		char *p = strrchr(line, '\n');
		if (p) {
			*p = '\0';
		}
		buf_add(&b, line);
	}
	free(line);
	fclose(stream);
	xmlDoc *d = xmlParseMemory(b.buf, b.len);
	if (!d) {
		wlr_log(WLR_ERROR, "xmlParseMemory()");
		goto err;
	}
	xml_tree_walk(xmlDocGetRootElement(d), server);
	xmlFreeDoc(d);
	xmlCleanupParser();
err:
	free(b.buf);
}

static void
menu_configure(struct menu *menu, int x, int y)
{
	struct theme *theme = menu->server->theme;

	menu->box.x = x;
	menu->box.y = y;

	int offset = 0;
	struct menuitem *menuitem;
	wl_list_for_each_reverse (menuitem, &menu->menuitems, link) {
		menuitem->box.x = menu->box.x;
		menuitem->box.y = menu->box.y + offset;
		offset += menuitem->box.height;
		if (menuitem->submenu) {
			menu_configure(menuitem->submenu, menuitem->box.x
				+ MENUWIDTH - theme->menu_overlap_x,
				menuitem->box.y + theme->menu_overlap_y);
		}
	}

	menu->box.width = MENUWIDTH;
	menu->box.height = offset;
}

void
menu_init_rootmenu(struct server *server)
{
	parse_xml("menu.xml", server);
	server->rootmenu = get_menu_by_id("root-menu");

	/* Default menu if no menu.xml found */
	if (!server->rootmenu) {
		current_menu = NULL;
		server->rootmenu = menu_create(server, "root-menu", "");
	}
	if (wl_list_empty(&server->rootmenu->menuitems)) {
		current_item = item_create(server->rootmenu, "Reconfigure");
		fill_item("name.action", "Reconfigure");
		current_item = item_create(server->rootmenu, "Exit");
		fill_item("name.action", "Exit");
	}

	server->rootmenu->visible = true;
	menu_configure(server->rootmenu, 100, 100);
}

void
menu_init_windowmenu(struct server *server)
{
	server->windowmenu = get_menu_by_id("client-menu");

	/* Default menu if no menu.xml found */
	if (!server->windowmenu) {
		current_menu = NULL;
		server->windowmenu = menu_create(server, "client-menu", "");
	}
	if (wl_list_empty(&server->windowmenu->menuitems)) {
		current_item = item_create(server->windowmenu, "Minimize");
		fill_item("name.action", "Iconify");
		current_item = item_create(server->windowmenu, "Maximize");
		fill_item("name.action", "ToggleMaximize");
		current_item = item_create(server->windowmenu, "Fullscreen");
		fill_item("name.action", "ToggleFullscreen");
		current_item = item_create(server->windowmenu, "Decorations");
		fill_item("name.action", "ToggleDecorations");
		current_item = item_create(server->windowmenu, "Close");
		fill_item("name.action", "Close");
	}

	server->windowmenu->visible = true;
	menu_configure(server->windowmenu, 100, 100);
}

void
menu_finish(void)
{
	struct menu *menu;
	for (int i = 0; i < nr_menus; ++i) {
		menu = menus + i;
		struct menuitem *item, *next;
		wl_list_for_each_safe(item, next, &menu->menuitems, link) {
			wl_list_remove(&item->link);
			action_list_free(&item->actions);
			free(item);
		}
	}
	zfree(menus);
	alloc_menus = 0;
	nr_menus = 0;
}

static void
close_all_submenus(struct menu *menu)
{
	struct menuitem *item;
	wl_list_for_each (item, &menu->menuitems, link) {
		if (item->submenu) {
			item->submenu->visible = false;
			close_all_submenus(item->submenu);
		}
	}
}

void
menu_move(struct menu *menu, int x, int y)
{
	assert(menu);
	close_all_submenus(menu);
	menu_configure(menu, x, y);
}

/* TODO: consider renaming function to menu_process_cursor_motion */
void
menu_set_selected(struct menu *menu, int x, int y)
{
	if (!menu->visible) {
		return;
	}

	struct menuitem *item;
	wl_list_for_each (item, &menu->menuitems, link) {
		item->selected = wlr_box_contains_point(&item->box, x, y);

		if (!item->selected) {
			if (item->submenu && item->submenu->visible) {
				/*
				 * Handle the case where a submenu is already
				 * open.
				 */
				item->selected = true;
				menu_set_selected(item->submenu, x, y);
			}
			continue;
		}

		/* We're now on an item that has mouse-focus */
		if (item->submenu) {
			if (item->submenu->visible) {
				/* do nothing - submenu already open */
			} else {
				/* open submenu */
				close_all_submenus(menu);
				item->submenu->visible = true;
				menu_set_selected(item->submenu, x, y);
			}
		} else {
			close_all_submenus(menu);
		}
	}
}

static void
menu_clear_selection(struct menu *menu)
{
	struct menuitem *item;
	wl_list_for_each (item, &menu->menuitems, link) {
		item->selected = false;
		if (item->submenu) {
			menu_clear_selection(item->submenu);
		}
	}
}

void
menu_action_selected(struct server *server, struct menu *menu)
{
	struct menuitem *menuitem;
	wl_list_for_each (menuitem, &menu->menuitems, link) {
		if (menuitem->selected && !menuitem->submenu) {
			action(NULL, server, &menuitem->actions, 0);
			break;
		}
		if (menuitem->submenu) {
			menu_action_selected(server, menuitem->submenu);
		}
	}
	menu_clear_selection(menu);
}

void
menu_reconfigure(struct server *server, struct menu *menu)
{
	menu_finish();
	menu_init_rootmenu(server);
	menu_init_windowmenu(server);
}
