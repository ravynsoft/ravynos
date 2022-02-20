#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>
#include <wlr/types/wlr_xcursor_manager.h>

struct wlr_xcursor_manager *wlr_xcursor_manager_create(const char *name,
		uint32_t size) {
	struct wlr_xcursor_manager *manager =
		calloc(1, sizeof(struct wlr_xcursor_manager));
	if (manager == NULL) {
		return NULL;
	}
	if (name != NULL) {
		manager->name = strdup(name);
	}
	manager->size = size;
	wl_list_init(&manager->scaled_themes);
	return manager;
}

void wlr_xcursor_manager_destroy(struct wlr_xcursor_manager *manager) {
	if (manager == NULL) {
		return;
	}
	struct wlr_xcursor_manager_theme *theme, *tmp;
	wl_list_for_each_safe(theme, tmp, &manager->scaled_themes, link) {
		wl_list_remove(&theme->link);
		wlr_xcursor_theme_destroy(theme->theme);
		free(theme);
	}
	free(manager->name);
	free(manager);
}

bool wlr_xcursor_manager_load(struct wlr_xcursor_manager *manager,
		float scale) {
	struct wlr_xcursor_manager_theme *theme;
	wl_list_for_each(theme, &manager->scaled_themes, link) {
		if (theme->scale == scale) {
			return true;
		}
	}

	theme = calloc(1, sizeof(struct wlr_xcursor_manager_theme));
	if (theme == NULL) {
		return false;
	}
	theme->scale = scale;
	theme->theme = wlr_xcursor_theme_load(manager->name, manager->size * scale);
	if (theme->theme == NULL) {
		free(theme);
		return false;
	}
	wl_list_insert(&manager->scaled_themes, &theme->link);
	return true;
}

struct wlr_xcursor *wlr_xcursor_manager_get_xcursor(
		struct wlr_xcursor_manager *manager, const char *name, float scale) {
	struct wlr_xcursor_manager_theme *theme;
	wl_list_for_each(theme, &manager->scaled_themes, link) {
		if (theme->scale == scale) {
			return wlr_xcursor_theme_get_cursor(theme->theme, name);
		}
	}
	return NULL;
}

void wlr_xcursor_manager_set_cursor_image(struct wlr_xcursor_manager *manager,
		const char *name, struct wlr_cursor *cursor) {
	struct wlr_xcursor_manager_theme *theme;
	wl_list_for_each(theme, &manager->scaled_themes, link) {
		struct wlr_xcursor *xcursor =
			wlr_xcursor_theme_get_cursor(theme->theme, name);
		if (xcursor == NULL) {
			continue;
		}

		struct wlr_xcursor_image *image = xcursor->images[0];
		wlr_cursor_set_image(cursor, image->buffer, image->width * 4,
			image->width, image->height, image->hotspot_x, image->hotspot_y,
			theme->scale);
	}
}
