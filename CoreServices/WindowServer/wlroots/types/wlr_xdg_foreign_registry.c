#include <wlr/types/wlr_xdg_foreign_registry.h>
#include "util/signal.h"
#include "util/token.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

bool wlr_xdg_foreign_exported_init(
		struct wlr_xdg_foreign_exported *exported,
		struct wlr_xdg_foreign_registry *registry) {
	do {
		if (!generate_token(exported->handle)) {
			return false;
		}
	} while (wlr_xdg_foreign_registry_find_by_handle(registry, exported->handle) != NULL);

	exported->registry = registry;
	wl_list_insert(&registry->exported_surfaces, &exported->link);

	wl_signal_init(&exported->events.destroy);
	return true;
}

struct wlr_xdg_foreign_exported *wlr_xdg_foreign_registry_find_by_handle(
		struct wlr_xdg_foreign_registry *registry, const char *handle) {
	if (handle == NULL || strlen(handle) >= WLR_XDG_FOREIGN_HANDLE_SIZE) {
		return NULL;
	}

	struct wlr_xdg_foreign_exported *exported;
	wl_list_for_each(exported, &registry->exported_surfaces, link) {
		if (strcmp(handle, exported->handle) == 0) {
			return exported;
		}
	}

	return NULL;
}

void wlr_xdg_foreign_exported_finish(struct wlr_xdg_foreign_exported *surface) {
	wlr_signal_emit_safe(&surface->events.destroy, NULL);
	surface->registry = NULL;
	wl_list_remove(&surface->link);
	wl_list_init(&surface->link);
}

static void foreign_registry_handle_display_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_xdg_foreign_registry *registry =
		wl_container_of(listener, registry, display_destroy);

	wlr_signal_emit_safe(&registry->events.destroy, NULL);

	// Implementations are supposed to remove all surfaces
	assert(wl_list_empty(&registry->exported_surfaces));
	free(registry);
}


struct wlr_xdg_foreign_registry *wlr_xdg_foreign_registry_create(
		struct wl_display *display) {
	struct wlr_xdg_foreign_registry *registry = calloc(1, sizeof(*registry));
	if (!registry) {
		return NULL;
	}

	registry->display_destroy.notify = foreign_registry_handle_display_destroy;
	wl_display_add_destroy_listener(display, &registry->display_destroy);

	wl_list_init(&registry->exported_surfaces);
	wl_signal_init(&registry->events.destroy);
	return registry;
}
