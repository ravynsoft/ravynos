#include <assert.h>
#include <stdlib.h>
#include <wlr/util/edges.h>
#include "types/wlr_xdg_shell.h"

static const struct xdg_positioner_interface xdg_positioner_implementation;

struct wlr_xdg_positioner *wlr_xdg_positioner_from_resource(
		struct wl_resource *resource) {
	assert(wl_resource_instance_of(resource, &xdg_positioner_interface,
		&xdg_positioner_implementation));
	return wl_resource_get_user_data(resource);
}

static void xdg_positioner_handle_set_size(struct wl_client *client,
		struct wl_resource *resource, int32_t width, int32_t height) {
	struct wlr_xdg_positioner *positioner =
		wlr_xdg_positioner_from_resource(resource);

	if (width < 1 || height < 1) {
		wl_resource_post_error(resource,
			XDG_POSITIONER_ERROR_INVALID_INPUT,
			"width and height must be positive and non-zero");
		return;
	}

	positioner->rules.size.width = width;
	positioner->rules.size.height = height;
}

static void xdg_positioner_handle_set_anchor_rect(struct wl_client *client,
		struct wl_resource *resource, int32_t x, int32_t y, int32_t width,
		int32_t height) {
	struct wlr_xdg_positioner *positioner =
		wlr_xdg_positioner_from_resource(resource);

	if (width < 0 || height < 0) {
		wl_resource_post_error(resource,
			XDG_POSITIONER_ERROR_INVALID_INPUT,
			"width and height must be positive");
		return;
	}

	positioner->rules.anchor_rect.x = x;
	positioner->rules.anchor_rect.y = y;
	positioner->rules.anchor_rect.width = width;
	positioner->rules.anchor_rect.height = height;
}

static void xdg_positioner_handle_set_anchor(struct wl_client *client,
		struct wl_resource *resource, uint32_t anchor) {
	struct wlr_xdg_positioner *positioner =
		wlr_xdg_positioner_from_resource(resource);

	if (anchor > XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT) {
		wl_resource_post_error(resource,
			XDG_POSITIONER_ERROR_INVALID_INPUT,
			"invalid anchor value");
		return;
	}

	positioner->rules.anchor = anchor;
}

static void xdg_positioner_handle_set_gravity(struct wl_client *client,
		struct wl_resource *resource, uint32_t gravity) {
	struct wlr_xdg_positioner *positioner =
		wlr_xdg_positioner_from_resource(resource);

	if (gravity > XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT) {
		wl_resource_post_error(resource,
			XDG_POSITIONER_ERROR_INVALID_INPUT,
			"invalid gravity value");
		return;
	}

	positioner->rules.gravity = gravity;
}

static void xdg_positioner_handle_set_constraint_adjustment(
		struct wl_client *client, struct wl_resource *resource,
		uint32_t constraint_adjustment) {
	struct wlr_xdg_positioner *positioner =
		wlr_xdg_positioner_from_resource(resource);

	positioner->rules.constraint_adjustment = constraint_adjustment;
}

static void xdg_positioner_handle_set_offset(struct wl_client *client,
		struct wl_resource *resource, int32_t x, int32_t y) {
	struct wlr_xdg_positioner *positioner =
		wlr_xdg_positioner_from_resource(resource);

	positioner->rules.offset.x = x;
	positioner->rules.offset.y = y;
}

static void xdg_positioner_handle_destroy(struct wl_client *client,
		struct wl_resource *resource) {
	wl_resource_destroy(resource);
}

static const struct xdg_positioner_interface
		xdg_positioner_implementation = {
	.destroy = xdg_positioner_handle_destroy,
	.set_size = xdg_positioner_handle_set_size,
	.set_anchor_rect = xdg_positioner_handle_set_anchor_rect,
	.set_anchor = xdg_positioner_handle_set_anchor,
	.set_gravity = xdg_positioner_handle_set_gravity,
	.set_constraint_adjustment =
		xdg_positioner_handle_set_constraint_adjustment,
	.set_offset = xdg_positioner_handle_set_offset,
};

static void xdg_positioner_handle_resource_destroy(
		struct wl_resource *resource) {
	struct wlr_xdg_positioner *positioner =
		wlr_xdg_positioner_from_resource(resource);
	free(positioner);
}

void create_xdg_positioner(struct wlr_xdg_client *client, uint32_t id) {
	struct wlr_xdg_positioner *positioner = calloc(1, sizeof(*positioner));
	if (positioner == NULL) {
		wl_client_post_no_memory(client->client);
		return;
	}

	positioner->resource = wl_resource_create(client->client,
		&xdg_positioner_interface,
		wl_resource_get_version(client->resource),
		id);
	if (positioner->resource == NULL) {
		free(positioner);
		wl_client_post_no_memory(client->client);
		return;
	}
	wl_resource_set_implementation(positioner->resource,
		&xdg_positioner_implementation,
		positioner, xdg_positioner_handle_resource_destroy);
}

static uint32_t xdg_positioner_anchor_to_wlr_edges(
		enum xdg_positioner_anchor anchor) {
	switch (anchor) {
	case XDG_POSITIONER_ANCHOR_NONE:
		return WLR_EDGE_NONE;
	case XDG_POSITIONER_ANCHOR_TOP:
		return WLR_EDGE_TOP;
	case XDG_POSITIONER_ANCHOR_TOP_LEFT:
		return WLR_EDGE_TOP | WLR_EDGE_LEFT;
	case XDG_POSITIONER_ANCHOR_TOP_RIGHT:
		return WLR_EDGE_TOP | WLR_EDGE_RIGHT;
	case XDG_POSITIONER_ANCHOR_BOTTOM:
		return WLR_EDGE_BOTTOM;
	case XDG_POSITIONER_ANCHOR_BOTTOM_LEFT:
		return WLR_EDGE_BOTTOM | WLR_EDGE_LEFT;
	case XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT:
		return WLR_EDGE_BOTTOM | WLR_EDGE_RIGHT;
	case XDG_POSITIONER_ANCHOR_LEFT:
		return WLR_EDGE_LEFT;
	case XDG_POSITIONER_ANCHOR_RIGHT:
		return WLR_EDGE_RIGHT;
	}

	abort(); // Unreachable
}

static uint32_t xdg_positioner_gravity_to_wlr_edges(
		enum xdg_positioner_gravity gravity) {
	// Gravity and edge enums are the same
	return xdg_positioner_anchor_to_wlr_edges((enum xdg_positioner_anchor)gravity);
}

void wlr_xdg_positioner_rules_get_geometry(
		const struct wlr_xdg_positioner_rules *rules, struct wlr_box *box) {
	box->x = rules->offset.x;
	box->y = rules->offset.y;
	box->width = rules->size.width;
	box->height = rules->size.height;

	uint32_t edges = xdg_positioner_anchor_to_wlr_edges(rules->anchor);

	if (edges & WLR_EDGE_TOP) {
		box->y += rules->anchor_rect.y;
	} else if (edges & WLR_EDGE_BOTTOM) {
		box->y += rules->anchor_rect.y + rules->anchor_rect.height;
	} else {
		box->y += rules->anchor_rect.y + rules->anchor_rect.height / 2;
	}

	if (edges & WLR_EDGE_LEFT) {
		box->x += rules->anchor_rect.x;
	} else if (edges & WLR_EDGE_RIGHT) {
		box->x += rules->anchor_rect.x + rules->anchor_rect.width;
	} else {
		box->x += rules->anchor_rect.x + rules->anchor_rect.width / 2;
	}

	edges = xdg_positioner_gravity_to_wlr_edges(rules->gravity);

	if (edges & WLR_EDGE_TOP) {
		box->y -= box->height;
	} else if (~edges & WLR_EDGE_BOTTOM) {
		box->y -= box->height / 2;
	}

	if (edges & WLR_EDGE_LEFT) {
		box->x -= box->width;
	} else if (~edges & WLR_EDGE_RIGHT) {
		box->x -= box->width / 2;
	}
}

static enum xdg_positioner_anchor xdg_positioner_anchor_invert_x(
		enum xdg_positioner_anchor anchor) {
	switch (anchor) {
	case XDG_POSITIONER_ANCHOR_LEFT:
		return XDG_POSITIONER_ANCHOR_RIGHT;
	case XDG_POSITIONER_ANCHOR_RIGHT:
		return XDG_POSITIONER_ANCHOR_LEFT;
	case XDG_POSITIONER_ANCHOR_TOP_LEFT:
		return XDG_POSITIONER_ANCHOR_TOP_RIGHT;
	case XDG_POSITIONER_ANCHOR_TOP_RIGHT:
		return XDG_POSITIONER_ANCHOR_TOP_LEFT;
	case XDG_POSITIONER_ANCHOR_BOTTOM_LEFT:
		return XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT;
	case XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT:
		return XDG_POSITIONER_ANCHOR_BOTTOM_LEFT;
	default:
		return anchor;
	}
}

static enum xdg_positioner_gravity xdg_positioner_gravity_invert_x(
		enum xdg_positioner_gravity gravity) {
	// gravity and edge enums are the same
	return (enum xdg_positioner_gravity)xdg_positioner_anchor_invert_x(
		(enum xdg_positioner_anchor)gravity);
}

static enum xdg_positioner_anchor xdg_positioner_anchor_invert_y(
		enum xdg_positioner_anchor anchor) {
	switch (anchor) {
	case XDG_POSITIONER_ANCHOR_TOP:
		return XDG_POSITIONER_ANCHOR_BOTTOM;
	case XDG_POSITIONER_ANCHOR_BOTTOM:
		return XDG_POSITIONER_ANCHOR_TOP;
	case XDG_POSITIONER_ANCHOR_TOP_LEFT:
		return XDG_POSITIONER_ANCHOR_BOTTOM_LEFT;
	case XDG_POSITIONER_ANCHOR_BOTTOM_LEFT:
		return XDG_POSITIONER_ANCHOR_TOP_LEFT;
	case XDG_POSITIONER_ANCHOR_TOP_RIGHT:
		return XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT;
	case XDG_POSITIONER_ANCHOR_BOTTOM_RIGHT:
		return XDG_POSITIONER_ANCHOR_TOP_RIGHT;
	default:
		return anchor;
	}
}

static enum xdg_positioner_gravity xdg_positioner_gravity_invert_y(
		enum xdg_positioner_gravity gravity) {
	// gravity and edge enums are the same
	return (enum xdg_positioner_gravity)xdg_positioner_anchor_invert_y(
		(enum xdg_positioner_anchor)gravity);
}

/**
 * Distances from each edge of the box to the corresponding edge of
 * the anchor rect. Each distance is positive if the edge is outside
 * the anchor rect, and negative if the edge is inside it.
 */
struct constraint_offsets {
	int top;
	int bottom;
	int left;
	int right;
};

static bool is_unconstrained(const struct constraint_offsets *offsets) {
	return offsets->top <= 0 && offsets->bottom <= 0 &&
		offsets->left <= 0 && offsets->right <= 0;
}

static void get_constrained_box_offsets(const struct wlr_box *constraint,
		const struct wlr_box *box, struct constraint_offsets *offsets) {
	offsets->left = constraint->x - box->x;
	offsets->right = box->x + box->width - constraint->x - constraint->width;
	offsets->top = constraint->y - box->y;
	offsets->bottom = box->y + box->height - constraint->y - constraint->height;
}

static bool xdg_positioner_rules_unconstrain_by_flip(
		const struct wlr_xdg_positioner_rules *rules,
		const struct wlr_box *constraint, struct wlr_box *box,
		struct constraint_offsets *offsets) {
	// If none of the edges are constrained, no need to flip.
	// If both edges are constrained, the box is bigger than
	// the anchor rect and flipping won't help anyway.
	bool flip_x = ((offsets->left > 0) ^ (offsets->right > 0)) &&
		(rules->constraint_adjustment & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X);
	bool flip_y = ((offsets->top > 0) ^ (offsets->bottom > 0)) &&
		(rules->constraint_adjustment & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y);

	if (!flip_x && !flip_y) {
		return false;
	}

	struct wlr_xdg_positioner_rules flipped = *rules;
	if (flip_x) {
		flipped.anchor = xdg_positioner_anchor_invert_x(flipped.anchor);
		flipped.gravity = xdg_positioner_gravity_invert_x(flipped.gravity);
	}
	if (flip_y) {
		flipped.anchor = xdg_positioner_anchor_invert_y(flipped.anchor);
		flipped.gravity = xdg_positioner_gravity_invert_y(flipped.gravity);
	}

	struct wlr_box flipped_box;
	wlr_xdg_positioner_rules_get_geometry(&flipped, &flipped_box);
	struct constraint_offsets flipped_offsets;
	get_constrained_box_offsets(constraint, &flipped_box, &flipped_offsets);

	// Only apply flipping if it helps
	if (flipped_offsets.left <= 0 && flipped_offsets.right <= 0) {
		box->x = flipped_box.x;
		offsets->left = flipped_offsets.left;
		offsets->right = flipped_offsets.right;
	}
	if (flipped_offsets.top <= 0 && flipped_offsets.bottom <= 0) {
		box->y = flipped_box.y;
		offsets->top = flipped_offsets.top;
		offsets->bottom = flipped_offsets.bottom;
	}

	return is_unconstrained(offsets);
}

static bool xdg_positioner_rules_unconstrain_by_slide(
		const struct wlr_xdg_positioner_rules *rules,
		const struct wlr_box *constraint, struct wlr_box *box,
		struct constraint_offsets *offsets) {
	uint32_t gravity = xdg_positioner_gravity_to_wlr_edges(rules->gravity);

	// We can only slide if there is gravity on this axis
	bool slide_x = (offsets->left > 0 || offsets->right > 0) &&
		(rules->constraint_adjustment & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X) &&
		(gravity & (WLR_EDGE_LEFT | WLR_EDGE_RIGHT));
	bool slide_y = (offsets->top > 0 || offsets->bottom > 0) &&
		(rules->constraint_adjustment & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y) &&
		(gravity & (WLR_EDGE_TOP | WLR_EDGE_BOTTOM));

	if (!slide_x && !slide_y) {
		return false;
	}

	if (slide_x) {
		if (offsets->left > 0 && offsets->right > 0) {
			// The protocol states: "First try to slide towards the direction of
			// the gravity [...] Then try to slide towards the opposite direction
			// of the gravity". The only situation where this order matters is when
			// the box is bigger than the anchor rect and completely includes it.
			// In this case, the second slide will fail immediately, so simply
			// slide towards the direction of the gravity.
			if (gravity & WLR_EDGE_LEFT) {
				box->x -= offsets->right;
			} else if (gravity & WLR_EDGE_RIGHT) {
				box->x += offsets->left;
			}
		} else {
			// If at least one edge is already unconstrained, the order of slide
			// attempts doesn't matter. Slide for the minimal distance needed to
			// satisfy the requirement of constraining one edge or unconstraining
			// another.
			int abs_left = offsets->left > 0 ? offsets->left : -offsets->left;
			int abs_right = offsets->right > 0 ? offsets->right : -offsets->right;
			if (abs_left < abs_right) {
				box->x += offsets->left;
			} else {
				box->x -= offsets->right;
			}
		}
	}
	if (slide_y) {
		if (offsets->top > 0 && offsets->bottom > 0) {
			if (gravity & WLR_EDGE_TOP) {
				box->y -= offsets->bottom;
			} else if (gravity & WLR_EDGE_BOTTOM) {
				box->y += offsets->top;
			}
		} else {
			int abs_top = offsets->top > 0 ? offsets->top : -offsets->top;
			int abs_bottom = offsets->bottom > 0 ? offsets->bottom : -offsets->bottom;
			if (abs_top < abs_bottom) {
				box->y += offsets->top;
			} else {
				box->y -= offsets->bottom;
			}
		}
	}

	get_constrained_box_offsets(constraint, box, offsets);
	return is_unconstrained(offsets);
}

static bool xdg_positioner_rules_unconstrain_by_resize(
		const struct wlr_xdg_positioner_rules *rules,
		const struct wlr_box *constraint, struct wlr_box *box,
		struct constraint_offsets *offsets) {
	bool resize_x = (offsets->left > 0 || offsets->right > 0) &&
		(rules->constraint_adjustment & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X);
	bool resize_y = (offsets->top > 0 || offsets->bottom > 0) &&
		(rules->constraint_adjustment & XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y);

	if (!resize_x && !resize_y) {
		return false;
	}

	if (offsets->left < 0) {
		offsets->left = 0;
	}
	if (offsets->right < 0) {
		offsets->right = 0;
	}
	if (offsets->top < 0) {
		offsets->top = 0;
	}
	if (offsets->bottom < 0) {
		offsets->bottom = 0;
	}

	// Try to satisfy the constraints by clipping the box.
	struct wlr_box resized_box = *box;
	if (resize_x) {
		resized_box.x += offsets->left;
		resized_box.width -= offsets->left + offsets->right;
	}
	if (resize_y) {
		resized_box.y += offsets->top;
		resized_box.height -= offsets->top + offsets->bottom;
	}

	if (wlr_box_empty(&resized_box)) {
		return false;
	}

	*box = resized_box;
	get_constrained_box_offsets(constraint, box, offsets);
	return is_unconstrained(offsets);
}

void wlr_xdg_positioner_rules_unconstrain_box(
		const struct wlr_xdg_positioner_rules *rules,
		const struct wlr_box *constraint, struct wlr_box *box) {
	struct constraint_offsets offsets;
	get_constrained_box_offsets(constraint, box, &offsets);
	if (is_unconstrained(&offsets)) {
		// Already unconstrained
		return;
	}
	if (xdg_positioner_rules_unconstrain_by_flip(rules, constraint, box, &offsets)) {
		return;
	}
	if (xdg_positioner_rules_unconstrain_by_slide(rules, constraint, box, &offsets)) {
		return;
	}
	if (xdg_positioner_rules_unconstrain_by_resize(rules, constraint, box, &offsets)) {
		return;
	}
}
