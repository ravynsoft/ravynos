#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/util/log.h>
#include <wlr/util/region.h>
#include "util/signal.h"

static struct wlr_scene *scene_root_from_node(struct wlr_scene_node *node) {
	assert(node->type == WLR_SCENE_NODE_ROOT);
	return (struct wlr_scene *)node;
}

static struct wlr_scene_tree *scene_tree_from_node(struct wlr_scene_node *node) {
	assert(node->type == WLR_SCENE_NODE_TREE);
	return (struct wlr_scene_tree *)node;
}

struct wlr_scene_surface *wlr_scene_surface_from_node(
		struct wlr_scene_node *node) {
	assert(node->type == WLR_SCENE_NODE_SURFACE);
	return (struct wlr_scene_surface *)node;
}

static struct wlr_scene_rect *scene_rect_from_node(
		struct wlr_scene_node *node) {
	assert(node->type == WLR_SCENE_NODE_RECT);
	return (struct wlr_scene_rect *)node;
}

static struct wlr_scene_buffer *scene_buffer_from_node(
		struct wlr_scene_node *node) {
	assert(node->type == WLR_SCENE_NODE_BUFFER);
	return (struct wlr_scene_buffer *)node;
}

static struct wlr_scene *scene_node_get_root(struct wlr_scene_node *node) {
	while (node->parent != NULL) {
		node = node->parent;
	}
	return scene_root_from_node(node);
}

static void scene_node_state_init(struct wlr_scene_node_state *state) {
	wl_list_init(&state->children);
	wl_list_init(&state->link);
	state->enabled = true;
}

static void scene_node_state_finish(struct wlr_scene_node_state *state) {
	wl_list_remove(&state->link);
}

static void scene_node_init(struct wlr_scene_node *node,
		enum wlr_scene_node_type type, struct wlr_scene_node *parent) {
	assert(type == WLR_SCENE_NODE_ROOT || parent != NULL);

	node->type = type;
	node->parent = parent;
	scene_node_state_init(&node->state);
	wl_signal_init(&node->events.destroy);

	if (parent != NULL) {
		wl_list_insert(parent->state.children.prev, &node->state.link);
	}
}

static void scene_node_finish(struct wlr_scene_node *node) {
	wlr_signal_emit_safe(&node->events.destroy, NULL);

	struct wlr_scene_node *child, *child_tmp;
	wl_list_for_each_safe(child, child_tmp,
			&node->state.children, state.link) {
		wlr_scene_node_destroy(child);
	}

	scene_node_state_finish(&node->state);
}

static void scene_node_damage_whole(struct wlr_scene_node *node);

void wlr_scene_node_destroy(struct wlr_scene_node *node) {
	if (node == NULL) {
		return;
	}

	scene_node_damage_whole(node);
	scene_node_finish(node);

	struct wlr_scene *scene = scene_node_get_root(node);
	struct wlr_scene_output *scene_output;
	switch (node->type) {
	case WLR_SCENE_NODE_ROOT:;
		struct wlr_scene_output *scene_output_tmp;
		wl_list_for_each_safe(scene_output, scene_output_tmp, &scene->outputs, link) {
			wlr_scene_output_destroy(scene_output);
		}

		wl_list_remove(&scene->presentation_destroy.link);

		free(scene);
		break;
	case WLR_SCENE_NODE_TREE:;
		struct wlr_scene_tree *tree = scene_tree_from_node(node);
		free(tree);
		break;
	case WLR_SCENE_NODE_SURFACE:;
		struct wlr_scene_surface *scene_surface = wlr_scene_surface_from_node(node);

		wl_list_for_each(scene_output, &scene->outputs, link) {
			// This is a noop if wlr_surface_send_enter() wasn't previously called for
			// the given output.
			wlr_surface_send_leave(scene_surface->surface, scene_output->output);
		}

		wl_list_remove(&scene_surface->surface_commit.link);
		wl_list_remove(&scene_surface->surface_destroy.link);

		free(scene_surface);
		break;
	case WLR_SCENE_NODE_RECT:;
		struct wlr_scene_rect *scene_rect = scene_rect_from_node(node);
		free(scene_rect);
		break;
	case WLR_SCENE_NODE_BUFFER:;
		struct wlr_scene_buffer *scene_buffer = scene_buffer_from_node(node);
		wl_list_remove(&scene_buffer->pending_link);
		wlr_texture_destroy(scene_buffer->texture);
		wlr_buffer_unlock(scene_buffer->buffer);
		free(scene_buffer);
		break;
	}
}

struct wlr_scene *wlr_scene_create(void) {
	struct wlr_scene *scene = calloc(1, sizeof(struct wlr_scene));
	if (scene == NULL) {
		return NULL;
	}
	scene_node_init(&scene->node, WLR_SCENE_NODE_ROOT, NULL);
	wl_list_init(&scene->outputs);
	wl_list_init(&scene->presentation_destroy.link);
	wl_list_init(&scene->pending_buffers);
	return scene;
}

struct wlr_scene_tree *wlr_scene_tree_create(struct wlr_scene_node *parent) {
	struct wlr_scene_tree *tree = calloc(1, sizeof(struct wlr_scene_tree));
	if (tree == NULL) {
		return NULL;
	}
	scene_node_init(&tree->node, WLR_SCENE_NODE_TREE, parent);

	return tree;
}

static void scene_surface_handle_surface_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_scene_surface *scene_surface =
		wl_container_of(listener, scene_surface, surface_destroy);
	wlr_scene_node_destroy(&scene_surface->node);
}

// This function must be called whenever the coordinates/dimensions of a scene
// surface or scene output change. It is not necessary to call when a scene
// surface's node is enabled/disabled or obscured by other nodes. To quote the
// protocol: "The surface might be hidden even if no leave event has been sent."
static void scene_surface_update_outputs(
		struct wlr_scene_surface *scene_surface,
		int lx, int ly, struct wlr_scene *scene) {
	struct wlr_box surface_box = {
		.x = lx,
		.y = ly,
		.width = scene_surface->surface->current.width,
		.height = scene_surface->surface->current.height,
	};

	int largest_overlap = 0;
	scene_surface->primary_output = NULL;

	struct wlr_scene_output *scene_output;
	wl_list_for_each(scene_output, &scene->outputs, link) {
		struct wlr_box output_box = {
			.x = scene_output->x,
			.y = scene_output->y,
		};
		wlr_output_effective_resolution(scene_output->output,
			&output_box.width, &output_box.height);

		struct wlr_box intersection;
		if (wlr_box_intersection(&intersection, &surface_box, &output_box)) {
			int overlap = intersection.width * intersection.height;
			if (overlap > largest_overlap) {
				largest_overlap = overlap;
				scene_surface->primary_output = scene_output->output;
			}

			// These enter/leave functions are a noop if the event has already been
			// sent for the given output.
			wlr_surface_send_enter(scene_surface->surface, scene_output->output);
		} else {
			wlr_surface_send_leave(scene_surface->surface, scene_output->output);
		}
	}
}

static void scene_node_update_surface_outputs_iterator(
		struct wlr_scene_node *node, int lx, int ly, struct wlr_scene *scene) {
	if (node->type == WLR_SCENE_NODE_SURFACE) {
		struct wlr_scene_surface *scene_surface =
			wlr_scene_surface_from_node(node);
		scene_surface_update_outputs(scene_surface, lx, ly, scene);
	}

	struct wlr_scene_node *child;
	wl_list_for_each(child, &node->state.children, state.link) {
		scene_node_update_surface_outputs_iterator(child, lx + child->state.x,
			ly + child->state.y, scene);
	}
}

static void scene_node_update_surface_outputs(struct wlr_scene_node *node) {
	struct wlr_scene *scene = scene_node_get_root(node);
	int lx, ly;
	wlr_scene_node_coords(node, &lx, &ly);
	scene_node_update_surface_outputs_iterator(node, lx, ly, scene);
}

static void scene_surface_handle_surface_commit(struct wl_listener *listener,
		void *data) {
	struct wlr_scene_surface *scene_surface =
		wl_container_of(listener, scene_surface, surface_commit);
	struct wlr_surface *surface = scene_surface->surface;

	struct wlr_scene *scene = scene_node_get_root(&scene_surface->node);

	int lx, ly;
	bool enabled = wlr_scene_node_coords(&scene_surface->node, &lx, &ly);

	if (surface->current.width != scene_surface->prev_width ||
			surface->current.height != scene_surface->prev_height) {
		scene_surface_update_outputs(scene_surface, lx, ly, scene);
		scene_surface->prev_width = surface->current.width;
		scene_surface->prev_height = surface->current.height;
	}

	if (!enabled) {
		return;
	}

	// Even if the surface hasn't submitted damage, schedule a new frame if
	// the client has requested a wl_surface.frame callback.
	if (!wl_list_empty(&surface->current.frame_callback_list) &&
			scene_surface->primary_output != NULL) {
		wlr_output_schedule_frame(scene_surface->primary_output);
	}

	if (!pixman_region32_not_empty(&surface->buffer_damage)) {
		return;
	}

	struct wlr_scene_output *scene_output;
	wl_list_for_each(scene_output, &scene->outputs, link) {
		struct wlr_output *output = scene_output->output;

		pixman_region32_t damage;
		pixman_region32_init(&damage);
		wlr_surface_get_effective_damage(surface, &damage);

		pixman_region32_translate(&damage,
			lx - scene_output->x, ly - scene_output->y);

		wlr_region_scale(&damage, &damage, output->scale);
		if (ceil(output->scale) > surface->current.scale) {
			// When scaling up a surface it'll become blurry, so we need to
			// expand the damage region.
			wlr_region_expand(&damage, &damage,
				ceil(output->scale) - surface->current.scale);
		}
		wlr_output_damage_add(scene_output->damage, &damage);
		pixman_region32_fini(&damage);
	}
}

struct wlr_scene_surface *wlr_scene_surface_create(struct wlr_scene_node *parent,
		struct wlr_surface *surface) {
	struct wlr_scene_surface *scene_surface =
		calloc(1, sizeof(struct wlr_scene_surface));
	if (scene_surface == NULL) {
		return NULL;
	}
	scene_node_init(&scene_surface->node, WLR_SCENE_NODE_SURFACE, parent);

	scene_surface->surface = surface;

	scene_surface->surface_destroy.notify = scene_surface_handle_surface_destroy;
	wl_signal_add(&surface->events.destroy, &scene_surface->surface_destroy);

	scene_surface->surface_commit.notify = scene_surface_handle_surface_commit;
	wl_signal_add(&surface->events.commit, &scene_surface->surface_commit);

	scene_node_damage_whole(&scene_surface->node);

	scene_node_update_surface_outputs(&scene_surface->node);

	return scene_surface;
}

struct wlr_scene_rect *wlr_scene_rect_create(struct wlr_scene_node *parent,
		int width, int height, const float color[static 4]) {
	struct wlr_scene_rect *scene_rect =
		calloc(1, sizeof(struct wlr_scene_rect));
	if (scene_rect == NULL) {
		return NULL;
	}
	scene_node_init(&scene_rect->node, WLR_SCENE_NODE_RECT, parent);

	scene_rect->width = width;
	scene_rect->height = height;
	memcpy(scene_rect->color, color, sizeof(scene_rect->color));

	scene_node_damage_whole(&scene_rect->node);

	return scene_rect;
}

void wlr_scene_rect_set_size(struct wlr_scene_rect *rect, int width, int height) {
	if (rect->width == width && rect->height == height) {
		return;
	}

	scene_node_damage_whole(&rect->node);
	rect->width = width;
	rect->height = height;
	scene_node_damage_whole(&rect->node);
}

void wlr_scene_rect_set_color(struct wlr_scene_rect *rect, const float color[static 4]) {
	if (memcmp(rect->color, color, sizeof(rect->color)) == 0) {
		return;
	}

	memcpy(rect->color, color, sizeof(rect->color));
	scene_node_damage_whole(&rect->node);
}

struct wlr_scene_buffer *wlr_scene_buffer_create(struct wlr_scene_node *parent,
		struct wlr_buffer *buffer) {
	struct wlr_scene_buffer *scene_buffer = calloc(1, sizeof(*scene_buffer));
	if (scene_buffer == NULL) {
		return NULL;
	}
	scene_node_init(&scene_buffer->node, WLR_SCENE_NODE_BUFFER, parent);

	scene_buffer->buffer = wlr_buffer_lock(buffer);

	scene_node_damage_whole(&scene_buffer->node);

	struct wlr_scene *scene = scene_node_get_root(parent);
	wl_list_insert(&scene->pending_buffers, &scene_buffer->pending_link);

	return scene_buffer;
}

void wlr_scene_buffer_set_source_box(struct wlr_scene_buffer *scene_buffer,
		const struct wlr_fbox *box) {
	struct wlr_fbox *cur = &scene_buffer->src_box;
	if ((wlr_fbox_empty(box) && wlr_fbox_empty(cur)) ||
			(box != NULL && memcmp(cur, box, sizeof(*box)) == 0)) {
		return;
	}

	if (box != NULL) {
		memcpy(cur, box, sizeof(*box));
	} else {
		memset(cur, 0, sizeof(*cur));
	}

	scene_node_damage_whole(&scene_buffer->node);
}

void wlr_scene_buffer_set_dest_size(struct wlr_scene_buffer *scene_buffer,
		int width, int height) {
	if (scene_buffer->dst_width == width && scene_buffer->dst_height == height) {
		return;
	}

	scene_node_damage_whole(&scene_buffer->node);
	scene_buffer->dst_width = width;
	scene_buffer->dst_height = height;
	scene_node_damage_whole(&scene_buffer->node);
}

void wlr_scene_buffer_set_transform(struct wlr_scene_buffer *scene_buffer,
		enum wl_output_transform transform) {
	if (scene_buffer->transform == transform) {
		return;
	}

	scene_node_damage_whole(&scene_buffer->node);
	scene_buffer->transform = transform;
	scene_node_damage_whole(&scene_buffer->node);
}

static struct wlr_texture *scene_buffer_get_texture(
		struct wlr_scene_buffer *scene_buffer, struct wlr_renderer *renderer) {
	struct wlr_client_buffer *client_buffer =
		wlr_client_buffer_get(scene_buffer->buffer);
	if (client_buffer != NULL) {
		return client_buffer->texture;
	}

	if (scene_buffer->texture != NULL) {
		return scene_buffer->texture;
	}

	scene_buffer->texture =
		wlr_texture_from_buffer(renderer, scene_buffer->buffer);
	return scene_buffer->texture;
}

static void scene_node_get_size(struct wlr_scene_node *node,
		int *width, int *height) {
	*width = 0;
	*height = 0;

	switch (node->type) {
	case WLR_SCENE_NODE_ROOT:
	case WLR_SCENE_NODE_TREE:
		return;
	case WLR_SCENE_NODE_SURFACE:;
		struct wlr_scene_surface *scene_surface =
			wlr_scene_surface_from_node(node);
		*width = scene_surface->surface->current.width;
		*height = scene_surface->surface->current.height;
		break;
	case WLR_SCENE_NODE_RECT:;
		struct wlr_scene_rect *scene_rect = scene_rect_from_node(node);
		*width = scene_rect->width;
		*height = scene_rect->height;
		break;
	case WLR_SCENE_NODE_BUFFER:;
		struct wlr_scene_buffer *scene_buffer = scene_buffer_from_node(node);
		if (scene_buffer->dst_width > 0 && scene_buffer->dst_height > 0) {
			*width = scene_buffer->dst_width;
			*height = scene_buffer->dst_height;
		} else {
			if (scene_buffer->transform & WL_OUTPUT_TRANSFORM_90) {
				*height = scene_buffer->buffer->width;
				*width = scene_buffer->buffer->height;
			} else {
				*width = scene_buffer->buffer->width;
				*height = scene_buffer->buffer->height;
			}
		}
		break;
	}
}

static int scale_length(int length, int offset, float scale) {
	return round((offset + length) * scale) - round(offset * scale);
}

static void scale_box(struct wlr_box *box, float scale) {
	box->width = scale_length(box->width, box->x, scale);
	box->height = scale_length(box->height, box->y, scale);
	box->x = round(box->x * scale);
	box->y = round(box->y * scale);
}

static void _scene_node_damage_whole(struct wlr_scene_node *node,
		struct wlr_scene *scene, int lx, int ly) {
	if (!node->state.enabled) {
		return;
	}

	struct wlr_scene_node *child;
	wl_list_for_each(child, &node->state.children, state.link) {
		_scene_node_damage_whole(child, scene,
			lx + child->state.x, ly + child->state.y);
	}

	int width, height;
	scene_node_get_size(node, &width, &height);

	struct wlr_scene_output *scene_output;
	wl_list_for_each(scene_output, &scene->outputs, link) {
		struct wlr_box box = {
			.x = lx - scene_output->x,
			.y = ly - scene_output->y,
			.width = width,
			.height = height,
		};

		scale_box(&box, scene_output->output->scale);

		wlr_output_damage_add_box(scene_output->damage, &box);
	}
}

static void scene_node_damage_whole(struct wlr_scene_node *node) {
	struct wlr_scene *scene = scene_node_get_root(node);
	if (wl_list_empty(&scene->outputs)) {
		return;
	}

	int lx, ly;
	if (!wlr_scene_node_coords(node, &lx, &ly)) {
		return;
	}

	_scene_node_damage_whole(node, scene, lx, ly);
}

void wlr_scene_node_set_enabled(struct wlr_scene_node *node, bool enabled) {
	if (node->state.enabled == enabled) {
		return;
	}

	// One of these damage_whole() calls will short-circuit and be a no-op
	scene_node_damage_whole(node);
	node->state.enabled = enabled;
	scene_node_damage_whole(node);
}

void wlr_scene_node_set_position(struct wlr_scene_node *node, int x, int y) {
	if (node->state.x == x && node->state.y == y) {
		return;
	}

	scene_node_damage_whole(node);
	node->state.x = x;
	node->state.y = y;
	scene_node_damage_whole(node);

	scene_node_update_surface_outputs(node);
}

void wlr_scene_node_place_above(struct wlr_scene_node *node,
		struct wlr_scene_node *sibling) {
	assert(node != sibling);
	assert(node->parent == sibling->parent);

	if (node->state.link.prev == &sibling->state.link) {
		return;
	}

	wl_list_remove(&node->state.link);
	wl_list_insert(&sibling->state.link, &node->state.link);

	scene_node_damage_whole(node);
	scene_node_damage_whole(sibling);
}

void wlr_scene_node_place_below(struct wlr_scene_node *node,
		struct wlr_scene_node *sibling) {
	assert(node != sibling);
	assert(node->parent == sibling->parent);

	if (node->state.link.next == &sibling->state.link) {
		return;
	}

	wl_list_remove(&node->state.link);
	wl_list_insert(sibling->state.link.prev, &node->state.link);

	scene_node_damage_whole(node);
	scene_node_damage_whole(sibling);
}

void wlr_scene_node_raise_to_top(struct wlr_scene_node *node) {
	struct wlr_scene_node *current_top = wl_container_of(
		node->parent->state.children.prev, current_top, state.link);
	if (node == current_top) {
		return;
	}
	wlr_scene_node_place_above(node, current_top);
}

void wlr_scene_node_lower_to_bottom(struct wlr_scene_node *node) {
	struct wlr_scene_node *current_bottom = wl_container_of(
		node->parent->state.children.next, current_bottom, state.link);
	if (node == current_bottom) {
		return;
	}
	wlr_scene_node_place_below(node, current_bottom);
}

void wlr_scene_node_reparent(struct wlr_scene_node *node,
		struct wlr_scene_node *new_parent) {
	assert(node->type != WLR_SCENE_NODE_ROOT && new_parent != NULL);

	if (node->parent == new_parent) {
		return;
	}

	/* Ensure that a node cannot become its own ancestor */
	for (struct wlr_scene_node *ancestor = new_parent; ancestor != NULL;
			ancestor = ancestor->parent) {
		assert(ancestor != node);
	}

	scene_node_damage_whole(node);

	wl_list_remove(&node->state.link);
	node->parent = new_parent;
	wl_list_insert(new_parent->state.children.prev, &node->state.link);

	scene_node_damage_whole(node);

	scene_node_update_surface_outputs(node);
}

bool wlr_scene_node_coords(struct wlr_scene_node *node,
		int *lx_ptr, int *ly_ptr) {
	int lx = 0, ly = 0;
	bool enabled = true;
	while (node != NULL) {
		lx += node->state.x;
		ly += node->state.y;
		enabled = enabled && node->state.enabled;
		node = node->parent;
	}

	*lx_ptr = lx;
	*ly_ptr = ly;
	return enabled;
}

static void scene_node_for_each_surface(struct wlr_scene_node *node,
		int lx, int ly, wlr_surface_iterator_func_t user_iterator,
		void *user_data) {
	if (!node->state.enabled) {
		return;
	}

	lx += node->state.x;
	ly += node->state.y;

	if (node->type == WLR_SCENE_NODE_SURFACE) {
		struct wlr_scene_surface *scene_surface = wlr_scene_surface_from_node(node);
		user_iterator(scene_surface->surface, lx, ly, user_data);
	}

	struct wlr_scene_node *child;
	wl_list_for_each(child, &node->state.children, state.link) {
		scene_node_for_each_surface(child, lx, ly, user_iterator, user_data);
	}
}

void wlr_scene_node_for_each_surface(struct wlr_scene_node *node,
		wlr_surface_iterator_func_t user_iterator, void *user_data) {
	scene_node_for_each_surface(node, 0, 0, user_iterator, user_data);
}

struct wlr_scene_node *wlr_scene_node_at(struct wlr_scene_node *node,
		double lx, double ly, double *nx, double *ny) {
	if (!node->state.enabled) {
		return NULL;
	}

	// TODO: optimize by storing a bounding box in each node?
	lx -= node->state.x;
	ly -= node->state.y;

	struct wlr_scene_node *child;
	wl_list_for_each_reverse(child, &node->state.children, state.link) {
		struct wlr_scene_node *node =
			wlr_scene_node_at(child, lx, ly, nx, ny);
		if (node != NULL) {
			return node;
		}
	}

	bool intersects = false;
	switch (node->type) {
	case WLR_SCENE_NODE_ROOT:
	case WLR_SCENE_NODE_TREE:
		break;
	case WLR_SCENE_NODE_SURFACE:;
		struct wlr_scene_surface *scene_surface = wlr_scene_surface_from_node(node);
		intersects = wlr_surface_point_accepts_input(scene_surface->surface, lx, ly);
		break;
	case WLR_SCENE_NODE_RECT:
	case WLR_SCENE_NODE_BUFFER:;
		int width, height;
		scene_node_get_size(node, &width, &height);
		intersects = lx >= 0 && lx < width && ly >= 0 && ly < height;
		break;
	}

	if (intersects) {
		if (nx != NULL) {
			*nx = lx;
		}
		if (ny != NULL) {
			*ny = ly;
		}
		return node;
	}

	return NULL;
}

static void scissor_output(struct wlr_output *output, pixman_box32_t *rect) {
	struct wlr_renderer *renderer = output->renderer;
	assert(renderer);

	struct wlr_box box = {
		.x = rect->x1,
		.y = rect->y1,
		.width = rect->x2 - rect->x1,
		.height = rect->y2 - rect->y1,
	};

	int ow, oh;
	wlr_output_transformed_resolution(output, &ow, &oh);

	enum wl_output_transform transform =
		wlr_output_transform_invert(output->transform);
	wlr_box_transform(&box, &box, transform, ow, oh);

	wlr_renderer_scissor(renderer, &box);
}

static void render_rect(struct wlr_output *output,
		pixman_region32_t *output_damage, const float color[static 4],
		const struct wlr_box *box, const float matrix[static 9]) {
	struct wlr_renderer *renderer = output->renderer;
	assert(renderer);

	pixman_region32_t damage;
	pixman_region32_init(&damage);
	pixman_region32_init_rect(&damage, box->x, box->y, box->width, box->height);
	pixman_region32_intersect(&damage, &damage, output_damage);

	int nrects;
	pixman_box32_t *rects = pixman_region32_rectangles(&damage, &nrects);
	for (int i = 0; i < nrects; ++i) {
		scissor_output(output, &rects[i]);
		wlr_render_rect(renderer, box, color, matrix);
	}

	pixman_region32_fini(&damage);
}

static void render_texture(struct wlr_output *output,
		pixman_region32_t *output_damage, struct wlr_texture *texture,
		const struct wlr_fbox *src_box, const struct wlr_box *dst_box,
		const float matrix[static 9]) {
	struct wlr_renderer *renderer = output->renderer;
	assert(renderer);

	struct wlr_fbox default_src_box = {0};
	if (wlr_fbox_empty(src_box)) {
		default_src_box.width = dst_box->width;
		default_src_box.height = dst_box->height;
		src_box = &default_src_box;
	}

	pixman_region32_t damage;
	pixman_region32_init(&damage);
	pixman_region32_init_rect(&damage, dst_box->x, dst_box->y,
		dst_box->width, dst_box->height);
	pixman_region32_intersect(&damage, &damage, output_damage);

	int nrects;
	pixman_box32_t *rects = pixman_region32_rectangles(&damage, &nrects);
	for (int i = 0; i < nrects; ++i) {
		scissor_output(output, &rects[i]);
		wlr_render_subtexture_with_matrix(renderer, texture, src_box, matrix, 1.0);
	}

	pixman_region32_fini(&damage);
}

struct render_data {
	struct wlr_output *output;
	pixman_region32_t *damage;

	// May be NULL
	struct wlr_presentation *presentation;
};

static void render_node_iterator(struct wlr_scene_node *node,
		int x, int y, void *_data) {
	struct render_data *data = _data;
	struct wlr_output *output = data->output;
	pixman_region32_t *output_damage = data->damage;

	struct wlr_box dst_box = {
		.x = x,
		.y = y,
	};
	scene_node_get_size(node, &dst_box.width, &dst_box.height);
	scale_box(&dst_box, output->scale);

	struct wlr_texture *texture;
	float matrix[9];
	enum wl_output_transform transform;
	switch (node->type) {
	case WLR_SCENE_NODE_ROOT:
	case WLR_SCENE_NODE_TREE:
		/* Root or tree node has nothing to render itself */
		break;
	case WLR_SCENE_NODE_SURFACE:;
		struct wlr_scene_surface *scene_surface = wlr_scene_surface_from_node(node);
		struct wlr_surface *surface = scene_surface->surface;

		texture = wlr_surface_get_texture(surface);
		if (texture == NULL) {
			return;
		}

		transform = wlr_output_transform_invert(surface->current.transform);
		wlr_matrix_project_box(matrix, &dst_box, transform, 0.0,
			output->transform_matrix);

		struct wlr_fbox src_box = {0};
		wlr_surface_get_buffer_source_box(surface, &src_box);

		render_texture(output, output_damage, texture,
			&src_box, &dst_box, matrix);

		if (data->presentation != NULL && scene_surface->primary_output == output) {
			wlr_presentation_surface_sampled_on_output(data->presentation,
				surface, output);
		}
		break;
	case WLR_SCENE_NODE_RECT:;
		struct wlr_scene_rect *scene_rect = scene_rect_from_node(node);

		render_rect(output, output_damage, scene_rect->color, &dst_box,
			output->transform_matrix);
		break;
	case WLR_SCENE_NODE_BUFFER:;
		struct wlr_scene_buffer *scene_buffer = scene_buffer_from_node(node);

		struct wlr_renderer *renderer = output->renderer;
		texture = scene_buffer_get_texture(scene_buffer, renderer);
		if (texture == NULL) {
			return;
		}

		transform = wlr_output_transform_invert(scene_buffer->transform);
		wlr_matrix_project_box(matrix, &dst_box, transform, 0.0,
			output->transform_matrix);

		render_texture(output, output_damage, texture, &scene_buffer->src_box,
			&dst_box, matrix);
		break;
	}
}

static void scene_node_for_each_node(struct wlr_scene_node *node,
		int lx, int ly, wlr_scene_node_iterator_func_t user_iterator,
		void *user_data) {
	if (!node->state.enabled) {
		return;
	}

	lx += node->state.x;
	ly += node->state.y;

	user_iterator(node, lx, ly, user_data);

	struct wlr_scene_node *child;
	wl_list_for_each(child, &node->state.children, state.link) {
		scene_node_for_each_node(child, lx, ly, user_iterator, user_data);
	}
}

void wlr_scene_render_output(struct wlr_scene *scene, struct wlr_output *output,
		int lx, int ly, pixman_region32_t *damage) {
	pixman_region32_t full_region;
	pixman_region32_init_rect(&full_region, 0, 0, output->width, output->height);
	if (damage == NULL) {
		damage = &full_region;
	}

	struct wlr_renderer *renderer = output->renderer;
	assert(renderer);

	if (output->enabled && pixman_region32_not_empty(damage)) {
		struct render_data data = {
			.output = output,
			.damage = damage,
			.presentation = scene->presentation,
		};
		scene_node_for_each_node(&scene->node, -lx, -ly,
			render_node_iterator, &data);
		wlr_renderer_scissor(renderer, NULL);
	}

	pixman_region32_fini(&full_region);
}

static void scene_handle_presentation_destroy(struct wl_listener *listener,
		void *data) {
	struct wlr_scene *scene =
		wl_container_of(listener, scene, presentation_destroy);
	wl_list_remove(&scene->presentation_destroy.link);
	wl_list_init(&scene->presentation_destroy.link);
	scene->presentation = NULL;
}

void wlr_scene_set_presentation(struct wlr_scene *scene,
		struct wlr_presentation *presentation) {
	assert(scene->presentation == NULL);
	scene->presentation = presentation;
	scene->presentation_destroy.notify = scene_handle_presentation_destroy;
	wl_signal_add(&presentation->events.destroy, &scene->presentation_destroy);
}

static void scene_output_handle_destroy(struct wlr_addon *addon) {
	struct wlr_scene_output *scene_output =
		wl_container_of(addon, scene_output, addon);
	wlr_scene_output_destroy(scene_output);
}

static const struct wlr_addon_interface output_addon_impl = {
	.name = "wlr_scene_output",
	.destroy = scene_output_handle_destroy,
};

struct wlr_scene_output *wlr_scene_output_create(struct wlr_scene *scene,
		struct wlr_output *output) {
	struct wlr_scene_output *scene_output = calloc(1, sizeof(*scene_output));
	if (scene_output == NULL) {
		return NULL;
	}

	scene_output->damage = wlr_output_damage_create(output);
	if (scene_output->damage == NULL) {
		free(scene_output);
		return NULL;
	}

	scene_output->output = output;
	scene_output->scene = scene;
	wlr_addon_init(&scene_output->addon, &output->addons, scene, &output_addon_impl);
	wl_list_insert(&scene->outputs, &scene_output->link);

	wlr_output_damage_add_whole(scene_output->damage);

	return scene_output;
}

static void scene_output_send_leave_iterator(struct wlr_surface *surface,
		int sx, int sy, void *data) {
	struct wlr_output *output = data;
	wlr_surface_send_leave(surface, output);
}

void wlr_scene_output_destroy(struct wlr_scene_output *scene_output) {
	wlr_addon_finish(&scene_output->addon);
	wl_list_remove(&scene_output->link);

	wlr_scene_output_for_each_surface(scene_output,
		scene_output_send_leave_iterator, scene_output->output);

	free(scene_output);
}

struct wlr_scene_output *wlr_scene_get_scene_output(struct wlr_scene *scene,
		struct wlr_output *output) {
	struct wlr_addon *addon =
		wlr_addon_find(&output->addons, scene, &output_addon_impl);
	if (addon == NULL) {
		return NULL;
	}
	struct wlr_scene_output *scene_output =
		wl_container_of(addon, scene_output, addon);
	return scene_output;
}

void wlr_scene_output_set_position(struct wlr_scene_output *scene_output,
		int lx, int ly) {
	if (scene_output->x == lx && scene_output->y == ly) {
		return;
	}

	scene_output->x = lx;
	scene_output->y = ly;
	wlr_output_damage_add_whole(scene_output->damage);

	scene_node_update_surface_outputs(&scene_output->scene->node);
}

struct check_scanout_data {
	// in
	struct wlr_box viewport_box;
	// out
	struct wlr_scene_node *node;
	size_t n;
};

static void check_scanout_iterator(struct wlr_scene_node *node,
		int x, int y, void *_data) {
	struct check_scanout_data *data = _data;

	struct wlr_box node_box = { .x = x, .y = y };
	scene_node_get_size(node, &node_box.width, &node_box.height);

	struct wlr_box intersection;
	if (!wlr_box_intersection(&intersection, &data->viewport_box, &node_box)) {
		return;
	}

	data->n++;

	if (data->viewport_box.x == node_box.x &&
			data->viewport_box.y == node_box.y &&
			data->viewport_box.width == node_box.width &&
			data->viewport_box.height == node_box.height) {
		data->node = node;
	}
}

static bool scene_output_scanout(struct wlr_scene_output *scene_output) {
	struct wlr_output *output = scene_output->output;

	struct wlr_box viewport_box = { .x = scene_output->x, .y = scene_output->y };
	wlr_output_effective_resolution(output,
		&viewport_box.width, &viewport_box.height);

	struct check_scanout_data check_scanout_data = {
		.viewport_box = viewport_box,
	};
	scene_node_for_each_node(&scene_output->scene->node, 0, 0,
		check_scanout_iterator, &check_scanout_data);
	if (check_scanout_data.n != 1 || check_scanout_data.node == NULL) {
		return false;
	}

	struct wlr_scene_node *node = check_scanout_data.node;
	struct wlr_buffer *buffer;
	switch (node->type) {
	case WLR_SCENE_NODE_SURFACE:;
		struct wlr_scene_surface *scene_surface = wlr_scene_surface_from_node(node);
		if (scene_surface->surface->buffer == NULL ||
				scene_surface->surface->current.viewport.has_src ||
				scene_surface->surface->current.transform != output->transform) {
			return false;
		}
		buffer = &scene_surface->surface->buffer->base;
		break;
	case WLR_SCENE_NODE_BUFFER:;
		struct wlr_scene_buffer *scene_buffer = scene_buffer_from_node(node);
		if (scene_buffer->buffer == NULL ||
				!wlr_fbox_empty(&scene_buffer->src_box) ||
				scene_buffer->transform != output->transform) {
			return false;
		}
		buffer = scene_buffer->buffer;
		break;
	default:
		return false;
	}

	wlr_output_attach_buffer(output, buffer);
	if (!wlr_output_test(output)) {
		wlr_output_rollback(output);
		return false;
	}

	struct wlr_presentation *presentation = scene_output->scene->presentation;
	if (presentation != NULL && node->type == WLR_SCENE_NODE_SURFACE) {
		struct wlr_scene_surface *scene_surface =
			wlr_scene_surface_from_node(node);
		// Since outputs may overlap, we still need to check this even though
		// we know that the surface size matches the size of this output.
		if (scene_surface->primary_output == output) {
			wlr_presentation_surface_sampled_on_output(presentation,
				scene_surface->surface, output);
		}
	}

	return wlr_output_commit(output);
}

bool wlr_scene_output_commit(struct wlr_scene_output *scene_output) {
	struct wlr_output *output = scene_output->output;

	struct wlr_renderer *renderer = output->renderer;
	assert(renderer != NULL);

	bool scanout = scene_output_scanout(scene_output);
	if (scanout != scene_output->prev_scanout) {
		wlr_log(WLR_DEBUG, "Direct scan-out %s",
			scanout ? "enabled" : "disabled");
		// When exiting direct scan-out, damage everything
		wlr_output_damage_add_whole(scene_output->damage);
	}
	scene_output->prev_scanout = scanout;
	if (scanout) {
		return true;
	}

	bool needs_frame;
	pixman_region32_t damage;
	pixman_region32_init(&damage);
	if (!wlr_output_damage_attach_render(scene_output->damage,
			&needs_frame, &damage)) {
		pixman_region32_fini(&damage);
		return false;
	}

	if (!needs_frame) {
		pixman_region32_fini(&damage);
		wlr_output_rollback(output);
		return true;
	}

	// Try to import new buffers as textures
	struct wlr_scene_buffer *scene_buffer, *scene_buffer_tmp;
	wl_list_for_each_safe(scene_buffer, scene_buffer_tmp,
			&scene_output->scene->pending_buffers, pending_link) {
		scene_buffer_get_texture(scene_buffer, renderer);
		wl_list_remove(&scene_buffer->pending_link);
		wl_list_init(&scene_buffer->pending_link);
	}

	wlr_renderer_begin(renderer, output->width, output->height);

	int nrects;
	pixman_box32_t *rects = pixman_region32_rectangles(&damage, &nrects);
	for (int i = 0; i < nrects; ++i) {
		scissor_output(output, &rects[i]);
		wlr_renderer_clear(renderer, (float[4]){ 0.0, 0.0, 0.0, 1.0 });
	}

	wlr_scene_render_output(scene_output->scene, output,
		scene_output->x, scene_output->y, &damage);
	wlr_output_render_software_cursors(output, &damage);

	wlr_renderer_end(renderer);
	pixman_region32_fini(&damage);

	int tr_width, tr_height;
	wlr_output_transformed_resolution(output, &tr_width, &tr_height);

	enum wl_output_transform transform =
		wlr_output_transform_invert(output->transform);

	pixman_region32_t frame_damage;
	pixman_region32_init(&frame_damage);
	wlr_region_transform(&frame_damage, &scene_output->damage->current,
		transform, tr_width, tr_height);
	wlr_output_set_damage(output, &frame_damage);
	pixman_region32_fini(&frame_damage);

	return wlr_output_commit(output);
}

static void scene_output_send_frame_done_iterator(struct wlr_scene_node *node,
		struct wlr_output *output, struct timespec *now) {
	if (!node->state.enabled) {
		return;
	}

	if (node->type == WLR_SCENE_NODE_SURFACE) {
		struct wlr_scene_surface *scene_surface =
			wlr_scene_surface_from_node(node);
		if (scene_surface->primary_output == output) {
			wlr_surface_send_frame_done(scene_surface->surface, now);
		}
	}

	struct wlr_scene_node *child;
	wl_list_for_each(child, &node->state.children, state.link) {
		scene_output_send_frame_done_iterator(child, output, now);
	}
}

void wlr_scene_output_send_frame_done(struct wlr_scene_output *scene_output,
		struct timespec *now) {
	scene_output_send_frame_done_iterator(&scene_output->scene->node,
		scene_output->output, now);
}

static void scene_output_for_each_surface(const struct wlr_box *output_box,
		struct wlr_scene_node *node, int lx, int ly,
		wlr_surface_iterator_func_t user_iterator, void *user_data) {
	if (!node->state.enabled) {
		return;
	}

	lx += node->state.x;
	ly += node->state.y;

	if (node->type == WLR_SCENE_NODE_SURFACE) {
		struct wlr_box node_box = { .x = lx, .y = ly };
		scene_node_get_size(node, &node_box.width, &node_box.height);

		struct wlr_box intersection;
		if (wlr_box_intersection(&intersection, output_box, &node_box)) {
			struct wlr_scene_surface *scene_surface =
				wlr_scene_surface_from_node(node);
			user_iterator(scene_surface->surface, lx, ly, user_data);
		}
	}

	struct wlr_scene_node *child;
	wl_list_for_each(child, &node->state.children, state.link) {
		scene_output_for_each_surface(output_box, child, lx, ly,
			user_iterator, user_data);
	}
}

void wlr_scene_output_for_each_surface(struct wlr_scene_output *scene_output,
		wlr_surface_iterator_func_t iterator, void *user_data) {
	struct wlr_box box = { .x = scene_output->x, .y = scene_output->y };
	wlr_output_effective_resolution(scene_output->output,
		&box.width, &box.height);
	scene_output_for_each_surface(&box, &scene_output->scene->node, 0, 0,
		iterator, user_data);
}
