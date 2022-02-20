#include <stdlib.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>

struct wlr_scene_output_layout {
	struct wlr_output_layout *layout;
	struct wlr_scene *scene;

	struct wl_listener layout_add;
	struct wl_listener layout_change;
	struct wl_listener layout_destroy;
	struct wl_listener scene_destroy;
};

static void scene_output_layout_destroy(struct wlr_scene_output_layout *sol) {
	wl_list_remove(&sol->layout_destroy.link);
	wl_list_remove(&sol->layout_add.link);
	wl_list_remove(&sol->layout_change.link);
	wl_list_remove(&sol->scene_destroy.link);
	free(sol);
}

static void scene_output_layout_handle_layout_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_scene_output_layout *sol =
		wl_container_of(listener, sol, layout_destroy);

	// Remove all outputs managed by the output layout
	struct wlr_scene_output *scene_output, *tmp;
	wl_list_for_each_safe(scene_output, tmp, &sol->scene->outputs, link) {
		struct wlr_output_layout_output *lo =
			wlr_output_layout_get(sol->layout, scene_output->output);
		if (lo != NULL) {
			wlr_scene_output_destroy(scene_output);
		}
	}

	scene_output_layout_destroy(sol);
}

static void scene_output_layout_handle_layout_change(
		struct wl_listener *listener, void *data) {
	struct wlr_scene_output_layout *sol =
		wl_container_of(listener, sol, layout_change);

	struct wlr_scene_output *scene_output, *tmp;
	wl_list_for_each_safe(scene_output, tmp, &sol->scene->outputs, link) {
		struct wlr_output_layout_output *lo =
			wlr_output_layout_get(sol->layout, scene_output->output);
		if (lo == NULL) {
			// Output has been removed from the layout
			wlr_scene_output_destroy(scene_output);
			continue;
		}

		wlr_scene_output_set_position(scene_output, lo->x, lo->y);
	}
}

static void scene_output_layout_handle_layout_add(
		struct wl_listener *listener, void *data) {
	struct wlr_scene_output_layout *sol =
		wl_container_of(listener, sol, layout_add);
	struct wlr_output_layout_output *lo = data;

	struct wlr_scene_output *scene_output =
		wlr_scene_output_create(sol->scene, lo->output);
	if (scene_output == NULL) {
		return;
	}

	wlr_scene_output_set_position(scene_output, lo->x, lo->y);
}

static void scene_output_layout_handle_scene_destroy(
		struct wl_listener *listener, void *data) {
	struct wlr_scene_output_layout *sol =
		wl_container_of(listener, sol, scene_destroy);
	scene_output_layout_destroy(sol);
}

bool wlr_scene_attach_output_layout(struct wlr_scene *scene,
		struct wlr_output_layout *output_layout) {
	struct wlr_scene_output_layout *sol = calloc(1, sizeof(*sol));
	if (sol == NULL) {
		return false;
	}

	sol->scene = scene;
	sol->layout = output_layout;

	sol->layout_destroy.notify = scene_output_layout_handle_layout_destroy;
	wl_signal_add(&output_layout->events.destroy, &sol->layout_destroy);

	sol->layout_change.notify = scene_output_layout_handle_layout_change;
	wl_signal_add(&output_layout->events.change, &sol->layout_change);

	sol->layout_add.notify = scene_output_layout_handle_layout_add;
	wl_signal_add(&output_layout->events.add, &sol->layout_add);

	sol->scene_destroy.notify = scene_output_layout_handle_scene_destroy;
	wl_signal_add(&output_layout->events.destroy, &sol->scene_destroy);

	return true;
}
