#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <wlr/types/wlr_xdg_foreign_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include "util/signal.h"
#include "xdg-foreign-unstable-v1-protocol.h"

#define FOREIGN_V1_VERSION 1

static const struct zxdg_exported_v1_interface xdg_exported_impl;
static const struct zxdg_imported_v1_interface xdg_imported_impl;
static const struct zxdg_exporter_v1_interface xdg_exporter_impl;
static const struct zxdg_importer_v1_interface xdg_importer_impl;

static struct wlr_xdg_imported_v1 *xdg_imported_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zxdg_imported_v1_interface,
		&xdg_imported_impl));
	return wl_resource_get_user_data(resource);
}

static void xdg_imported_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static bool verify_is_toplevel(struct wl_resource *client_resource,
		struct wlr_surface *surface) {
	if (wlr_surface_is_xdg_surface(surface)) {
		struct wlr_xdg_surface *xdg_surface =
			wlr_xdg_surface_from_wlr_surface(surface);
		if (xdg_surface == NULL || xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
			wl_resource_post_error(client_resource, -1,
					"surface must be an xdg_toplevel");
			return false;
		}
	} else {
		wl_resource_post_error(client_resource, -1,
				"surface must be an xdg_surface");
		return false;
	}
	return true;
}

static void destroy_imported_child(struct wlr_xdg_imported_child_v1 *child) {
	wl_list_remove(&child->xdg_toplevel_set_parent.link);
	wl_list_remove(&child->xdg_surface_unmap.link);
	wl_list_remove(&child->link);
	free(child);
}

static void handle_child_xdg_surface_unmap(
		struct wl_listener *listener, void *data) {
	struct wlr_xdg_imported_child_v1 *child =
		wl_container_of(listener, child, xdg_surface_unmap);
	destroy_imported_child(child);
}

static void handle_xdg_toplevel_set_parent(
		struct wl_listener *listener, void *data) {
	struct wlr_xdg_imported_child_v1 *child =
		wl_container_of(listener, child, xdg_toplevel_set_parent);
	destroy_imported_child(child);
}

static void xdg_imported_handle_set_parent_of(struct wl_client *client,
		struct wl_resource *resource,
		struct wl_resource *child_resource) {
	struct wlr_xdg_imported_v1 *imported =
		xdg_imported_from_resource(resource);
	if (imported == NULL) {
		return;
	}
	struct wlr_surface *wlr_surface = imported->exported->surface;
	struct wlr_surface *wlr_surface_child =
		wlr_surface_from_resource(child_resource);

	if (!verify_is_toplevel(resource, wlr_surface_child)) {
		return;
	}
	struct wlr_xdg_imported_child_v1 *child;
	wl_list_for_each(child, &imported->children, link) {
		if (child->surface == wlr_surface_child) {
			return;
		}
	}

	child = calloc(1, sizeof(struct wlr_xdg_imported_child_v1));
	if (child == NULL) {
		wl_client_post_no_memory(client);
		return;
	}
	child->surface = wlr_surface_child;
	child->xdg_surface_unmap.notify = handle_child_xdg_surface_unmap;
	child->xdg_toplevel_set_parent.notify = handle_xdg_toplevel_set_parent;

	struct wlr_xdg_surface *surface =
		wlr_xdg_surface_from_wlr_surface(wlr_surface);
	struct wlr_xdg_surface *surface_child =
		wlr_xdg_surface_from_wlr_surface(wlr_surface_child);

	wlr_xdg_toplevel_set_parent(surface_child->toplevel, surface->toplevel);
	wl_signal_add(&surface_child->events.unmap,
			&child->xdg_surface_unmap);
	wl_signal_add(&surface_child->toplevel->events.set_parent,
			&child->xdg_toplevel_set_parent);

	wl_list_insert(&imported->children, &child->link);
}

static const struct zxdg_imported_v1_interface xdg_imported_impl = {
	.destroy = xdg_imported_handle_destroy,
	.set_parent_of = xdg_imported_handle_set_parent_of
};

static struct wlr_xdg_exported_v1 *xdg_exported_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zxdg_exported_v1_interface,
		&xdg_exported_impl));
	return wl_resource_get_user_data(resource);
}

static void xdg_exported_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct zxdg_exported_v1_interface xdg_exported_impl = {
	.destroy = xdg_exported_handle_destroy
};

static void xdg_exporter_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static struct wlr_xdg_foreign_v1 *xdg_foreign_from_exporter_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zxdg_exporter_v1_interface,
		&xdg_exporter_impl));
	return wl_resource_get_user_data(resource);
}

static void destroy_imported(struct wlr_xdg_imported_v1 *imported) {
	imported->exported = NULL;
	struct wlr_xdg_imported_child_v1 *child, *child_tmp;
	wl_list_for_each_safe(child, child_tmp, &imported->children, link) {
		struct wlr_xdg_surface *xdg_child =
			wlr_xdg_surface_from_wlr_surface(child->surface);

		if (xdg_child != NULL) {
			wlr_xdg_toplevel_set_parent(xdg_child->toplevel, NULL);
		}
	}

	wl_list_remove(&imported->exported_destroyed.link);
	wl_list_init(&imported->exported_destroyed.link);

	wl_list_remove(&imported->link);
	wl_list_init(&imported->link);
	wl_resource_set_user_data(imported->resource, NULL);
	free(imported);
}

static void destroy_exported(struct wlr_xdg_exported_v1 *exported) {
	wlr_xdg_foreign_exported_finish(&exported->base);

	wl_list_remove(&exported->xdg_surface_destroy.link);
	wl_list_remove(&exported->link);
	wl_resource_set_user_data(exported->resource, NULL);
	free(exported);
}

static void xdg_exported_handle_resource_destroy(
		struct wl_resource *resource) {
	struct wlr_xdg_exported_v1 *exported =
		xdg_exported_from_resource(resource);

	if (exported) {
		destroy_exported(exported);
	}
}

static void handle_xdg_surface_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_xdg_exported_v1 *exported =
		wl_container_of(listener, exported, xdg_surface_destroy);

	destroy_exported(exported);
}

static void xdg_exporter_handle_export(struct wl_client *wl_client,
		struct wl_resource *client_resource,
		uint32_t id,
		struct wl_resource *surface_resource) {
	struct wlr_xdg_foreign_v1 *foreign =
		xdg_foreign_from_exporter_resource(client_resource);
	struct wlr_surface *surface = wlr_surface_from_resource(surface_resource);

	if (!verify_is_toplevel(client_resource, surface)) {
		return;
	}

	struct wlr_xdg_exported_v1 *exported =
		calloc(1, sizeof(struct wlr_xdg_exported_v1));
	if (exported == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	if (!wlr_xdg_foreign_exported_init(&exported->base, foreign->registry)) {
		wl_client_post_no_memory(wl_client);
		free(exported);
		return;
	}

	exported->base.surface = surface;
	exported->resource = wl_resource_create(wl_client, &zxdg_exported_v1_interface,
		wl_resource_get_version(client_resource), id);
	if (exported->resource == NULL) {
		wlr_xdg_foreign_exported_finish(&exported->base);
		wl_client_post_no_memory(wl_client);
		free(exported);
		return;
	}

	wl_resource_set_implementation(exported->resource, &xdg_exported_impl,
			exported, xdg_exported_handle_resource_destroy);

	wl_list_insert(&foreign->exporter.objects, &exported->link);

	zxdg_exported_v1_send_handle(exported->resource, exported->base.handle);

	exported->xdg_surface_destroy.notify = handle_xdg_surface_destroy;
	struct wlr_xdg_surface *xdg_surface =
		wlr_xdg_surface_from_wlr_surface(surface);
	wl_signal_add(&xdg_surface->events.unmap, &exported->xdg_surface_destroy);
}

static const struct zxdg_exporter_v1_interface xdg_exporter_impl = {
	.destroy = xdg_exporter_handle_destroy,
	.export = xdg_exporter_handle_export
};

static void xdg_exporter_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_xdg_foreign_v1 *foreign = data;

	struct wl_resource *exporter_resource =
		wl_resource_create(wl_client, &zxdg_exporter_v1_interface, version, id);
	if (exporter_resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	wl_resource_set_implementation(exporter_resource, &xdg_exporter_impl,
			foreign, NULL);
}

static struct wlr_xdg_foreign_v1 *xdg_foreign_from_importer_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &zxdg_importer_v1_interface,
		&xdg_importer_impl));
	return wl_resource_get_user_data(resource);
}

static void xdg_importer_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static void xdg_imported_handle_resource_destroy(
		struct wl_resource *resource) {
	struct wlr_xdg_imported_v1 *imported = xdg_imported_from_resource(resource);
	if (!imported) {
		return;
	}

	destroy_imported(imported);
}

static void xdg_imported_handle_exported_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_xdg_imported_v1 *imported =
		wl_container_of(listener, imported, exported_destroyed);
	zxdg_imported_v1_send_destroyed(imported->resource);
	destroy_imported(imported);
}

static void xdg_importer_handle_import(struct wl_client *wl_client,
				struct wl_resource *client_resource,
				uint32_t id,
				const char *handle) {
	struct wlr_xdg_foreign_v1 *foreign =
		xdg_foreign_from_importer_resource(client_resource);

	struct wlr_xdg_imported_v1 *imported =
		calloc(1, sizeof(struct wlr_xdg_imported_v1));
	if (imported == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	imported->exported = wlr_xdg_foreign_registry_find_by_handle(
		foreign->registry, handle);
	imported->resource = wl_resource_create(wl_client, &zxdg_imported_v1_interface,
			wl_resource_get_version(client_resource), id);
	if (imported->resource == NULL) {
		wl_client_post_no_memory(wl_client);
		free(imported);
		return;
	}

	wl_resource_set_implementation(imported->resource, &xdg_imported_impl,
			imported, xdg_imported_handle_resource_destroy);

	if (imported->exported == NULL) {
		wl_resource_set_user_data(imported->resource, NULL);
		zxdg_imported_v1_send_destroyed(imported->resource);
		free(imported);
		return;
	}

	wl_list_init(&imported->children);
	wl_list_insert(&foreign->importer.objects, &imported->link);

	imported->exported_destroyed.notify = xdg_imported_handle_exported_destroy;
	wl_signal_add(&imported->exported->events.destroy, &imported->exported_destroyed);
}

static const struct zxdg_importer_v1_interface xdg_importer_impl = {
	.destroy = xdg_importer_handle_destroy,
	.import = xdg_importer_handle_import
};

static void xdg_importer_bind(struct wl_client *wl_client, void *data,
		uint32_t version, uint32_t id) {
	struct wlr_xdg_foreign_v1 *foreign = data;

	struct wl_resource *importer_resource =
		wl_resource_create(wl_client, &zxdg_importer_v1_interface, version, id);

	if (importer_resource == NULL) {
		wl_client_post_no_memory(wl_client);
		return;
	}

	wl_resource_set_implementation(importer_resource, &xdg_importer_impl,
			foreign, NULL);
}

static void xdg_foreign_destroy(struct wlr_xdg_foreign_v1 *foreign) {
	if (!foreign) {
		return;
	}

	wlr_signal_emit_safe(&foreign->events.destroy, NULL);
	wl_list_remove(&foreign->foreign_registry_destroy.link);
	wl_list_remove(&foreign->display_destroy.link);

	wl_global_destroy(foreign->exporter.global);
	wl_global_destroy(foreign->importer.global);
	free(foreign);
}

static void handle_display_destroy(struct wl_listener *listener, void *data) {
	struct wlr_xdg_foreign_v1 *foreign =
		wl_container_of(listener, foreign, display_destroy);
	xdg_foreign_destroy(foreign);
}

static void handle_foreign_registry_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_xdg_foreign_v1 *foreign =
		wl_container_of(listener, foreign, foreign_registry_destroy);
	xdg_foreign_destroy(foreign);
}

struct wlr_xdg_foreign_v1 *wlr_xdg_foreign_v1_create(
		struct wl_display *display, struct wlr_xdg_foreign_registry *registry) {
	struct wlr_xdg_foreign_v1 *foreign = calloc(1,
			sizeof(struct wlr_xdg_foreign_v1));
	if (!foreign) {
		return NULL;
	}

	foreign->exporter.global = wl_global_create(display,
			&zxdg_exporter_v1_interface,
			FOREIGN_V1_VERSION, foreign,
			xdg_exporter_bind);
	if (!foreign->exporter.global) {
		free(foreign);
		return NULL;
	}

	foreign->importer.global = wl_global_create(display,
			&zxdg_importer_v1_interface,
			FOREIGN_V1_VERSION, foreign,
			xdg_importer_bind);
	if (!foreign->importer.global) {
		wl_global_destroy(foreign->exporter.global);
		free(foreign);
		return NULL;
	}

	foreign->registry = registry;

	wl_signal_init(&foreign->events.destroy);
	wl_list_init(&foreign->exporter.objects);
	wl_list_init(&foreign->importer.objects);

	foreign->display_destroy.notify = handle_display_destroy;
	wl_display_add_destroy_listener(display, &foreign->display_destroy);

	foreign->foreign_registry_destroy.notify = handle_foreign_registry_destroy;
	wl_signal_add(&registry->events.destroy, &foreign->foreign_registry_destroy);

	return foreign;
}
