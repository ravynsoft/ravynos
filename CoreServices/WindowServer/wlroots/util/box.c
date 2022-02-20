#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

void wlr_box_closest_point(const struct wlr_box *box, double x, double y,
		double *dest_x, double *dest_y) {
	// if box is empty, then it contains no points, so no closest point either
	if (box->width <= 0 || box->height <= 0) {
		*dest_x = NAN;
		*dest_y = NAN;
		return;
	}

	// find the closest x point
	if (x < box->x) {
		*dest_x = box->x;
	} else if (x >= box->x + box->width) {
		*dest_x = box->x + box->width - 1;
	} else {
		*dest_x = x;
	}

	// find closest y point
	if (y < box->y) {
		*dest_y = box->y;
	} else if (y >= box->y + box->height) {
		*dest_y = box->y + box->height - 1;
	} else {
		*dest_y = y;
	}
}

bool wlr_box_empty(const struct wlr_box *box) {
	return box == NULL || box->width <= 0 || box->height <= 0;
}

bool wlr_box_intersection(struct wlr_box *dest, const struct wlr_box *box_a,
		const struct wlr_box *box_b) {
	bool a_empty = wlr_box_empty(box_a);
	bool b_empty = wlr_box_empty(box_b);

	if (a_empty || b_empty) {
		dest->x = 0;
		dest->y = 0;
		dest->width = -100;
		dest->height = -100;
		return false;
	}

	int x1 = fmax(box_a->x, box_b->x);
	int y1 = fmax(box_a->y, box_b->y);
	int x2 = fmin(box_a->x + box_a->width, box_b->x + box_b->width);
	int y2 = fmin(box_a->y + box_a->height, box_b->y + box_b->height);

	dest->x = x1;
	dest->y = y1;
	dest->width = x2 - x1;
	dest->height = y2 - y1;

	return !wlr_box_empty(dest);
}

bool wlr_box_contains_point(const struct wlr_box *box, double x, double y) {
	if (wlr_box_empty(box)) {
		return false;
	} else {
		return x >= box->x && x < box->x + box->width &&
			y >= box->y && y < box->y + box->height;
	}
}

void wlr_box_transform(struct wlr_box *dest, const struct wlr_box *box,
		enum wl_output_transform transform, int width, int height) {
	struct wlr_box src = *box;

	if (transform % 2 == 0) {
		dest->width = src.width;
		dest->height = src.height;
	} else {
		dest->width = src.height;
		dest->height = src.width;
	}

	switch (transform) {
	case WL_OUTPUT_TRANSFORM_NORMAL:
		dest->x = src.x;
		dest->y = src.y;
		break;
	case WL_OUTPUT_TRANSFORM_90:
		dest->x = height - src.y - src.height;
		dest->y = src.x;
		break;
	case WL_OUTPUT_TRANSFORM_180:
		dest->x = width - src.x - src.width;
		dest->y = height - src.y - src.height;
		break;
	case WL_OUTPUT_TRANSFORM_270:
		dest->x = src.y;
		dest->y = width - src.x - src.width;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED:
		dest->x = width - src.x - src.width;
		dest->y = src.y;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_90:
		dest->x = src.y;
		dest->y = src.x;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_180:
		dest->x = src.x;
		dest->y = height - src.y - src.height;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_270:
		dest->x = height - src.y - src.height;
		dest->y = width - src.x - src.width;
		break;
	}
}

bool wlr_fbox_empty(const struct wlr_fbox *box) {
	return box == NULL || box->width <= 0 || box->height <= 0;
}

void wlr_fbox_transform(struct wlr_fbox *dest, const struct wlr_fbox *box,
		enum wl_output_transform transform, double width, double height) {
	struct wlr_fbox src = *box;

	if (transform % 2 == 0) {
		dest->width = src.width;
		dest->height = src.height;
	} else {
		dest->width = src.height;
		dest->height = src.width;
	}

	switch (transform) {
	case WL_OUTPUT_TRANSFORM_NORMAL:
		dest->x = src.x;
		dest->y = src.y;
		break;
	case WL_OUTPUT_TRANSFORM_90:
		dest->x = height - src.y - src.height;
		dest->y = src.x;
		break;
	case WL_OUTPUT_TRANSFORM_180:
		dest->x = width - src.x - src.width;
		dest->y = height - src.y - src.height;
		break;
	case WL_OUTPUT_TRANSFORM_270:
		dest->x = src.y;
		dest->y = width - src.x - src.width;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED:
		dest->x = width - src.x - src.width;
		dest->y = src.y;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_90:
		dest->x = src.y;
		dest->y = src.x;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_180:
		dest->x = src.x;
		dest->y = height - src.y - src.height;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_270:
		dest->x = height - src.y - src.height;
		dest->y = width - src.x - src.width;
		break;
	}
}
