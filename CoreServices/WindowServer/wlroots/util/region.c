#include <assert.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <wlr/util/region.h>

void wlr_region_scale(pixman_region32_t *dst, pixman_region32_t *src,
		float scale) {
	wlr_region_scale_xy(dst, src, scale, scale);
}

void wlr_region_scale_xy(pixman_region32_t *dst, pixman_region32_t *src,
		float scale_x, float scale_y) {
	if (scale_x == 1.0 && scale_y == 1.0) {
		pixman_region32_copy(dst, src);
		return;
	}

	int nrects;
	pixman_box32_t *src_rects = pixman_region32_rectangles(src, &nrects);

	pixman_box32_t *dst_rects = malloc(nrects * sizeof(pixman_box32_t));
	if (dst_rects == NULL) {
		return;
	}

	for (int i = 0; i < nrects; ++i) {
		dst_rects[i].x1 = floor(src_rects[i].x1 * scale_x);
		dst_rects[i].x2 = ceil(src_rects[i].x2 * scale_x);
		dst_rects[i].y1 = floor(src_rects[i].y1 * scale_y);
		dst_rects[i].y2 = ceil(src_rects[i].y2 * scale_y);
	}

	pixman_region32_fini(dst);
	pixman_region32_init_rects(dst, dst_rects, nrects);
	free(dst_rects);
}

void wlr_region_transform(pixman_region32_t *dst, pixman_region32_t *src,
		enum wl_output_transform transform, int width, int height) {
	if (transform == WL_OUTPUT_TRANSFORM_NORMAL) {
		pixman_region32_copy(dst, src);
		return;
	}

	int nrects;
	pixman_box32_t *src_rects = pixman_region32_rectangles(src, &nrects);

	pixman_box32_t *dst_rects = malloc(nrects * sizeof(pixman_box32_t));
	if (dst_rects == NULL) {
		return;
	}

	for (int i = 0; i < nrects; ++i) {
		switch (transform) {
		case WL_OUTPUT_TRANSFORM_NORMAL:
			dst_rects[i].x1 = src_rects[i].x1;
			dst_rects[i].y1 = src_rects[i].y1;
			dst_rects[i].x2 = src_rects[i].x2;
			dst_rects[i].y2 = src_rects[i].y2;
			break;
		case WL_OUTPUT_TRANSFORM_90:
			dst_rects[i].x1 = height - src_rects[i].y2;
			dst_rects[i].y1 = src_rects[i].x1;
			dst_rects[i].x2 = height - src_rects[i].y1;
			dst_rects[i].y2 = src_rects[i].x2;
			break;
		case WL_OUTPUT_TRANSFORM_180:
			dst_rects[i].x1 = width - src_rects[i].x2;
			dst_rects[i].y1 = height - src_rects[i].y2;
			dst_rects[i].x2 = width - src_rects[i].x1;
			dst_rects[i].y2 = height - src_rects[i].y1;
			break;
		case WL_OUTPUT_TRANSFORM_270:
			dst_rects[i].x1 = src_rects[i].y1;
			dst_rects[i].y1 = width - src_rects[i].x2;
			dst_rects[i].x2 = src_rects[i].y2;
			dst_rects[i].y2 = width - src_rects[i].x1;
			break;
		case WL_OUTPUT_TRANSFORM_FLIPPED:
			dst_rects[i].x1 = width - src_rects[i].x2;
			dst_rects[i].y1 = src_rects[i].y1;
			dst_rects[i].x2 = width - src_rects[i].x1;
			dst_rects[i].y2 = src_rects[i].y2;
			break;
		case WL_OUTPUT_TRANSFORM_FLIPPED_90:
			dst_rects[i].x1 = src_rects[i].y1;
			dst_rects[i].y1 = src_rects[i].x1;
			dst_rects[i].x2 = src_rects[i].y2;
			dst_rects[i].y2 = src_rects[i].x2;
			break;
		case WL_OUTPUT_TRANSFORM_FLIPPED_180:
			dst_rects[i].x1 = src_rects[i].x1;
			dst_rects[i].y1 = height - src_rects[i].y2;
			dst_rects[i].x2 = src_rects[i].x2;
			dst_rects[i].y2 = height - src_rects[i].y1;
			break;
		case WL_OUTPUT_TRANSFORM_FLIPPED_270:
			dst_rects[i].x1 = height - src_rects[i].y2;
			dst_rects[i].y1 = width - src_rects[i].x2;
			dst_rects[i].x2 = height - src_rects[i].y1;
			dst_rects[i].y2 = width - src_rects[i].x1;
			break;
		}
	}

	pixman_region32_fini(dst);
	pixman_region32_init_rects(dst, dst_rects, nrects);
	free(dst_rects);
}

void wlr_region_expand(pixman_region32_t *dst, pixman_region32_t *src,
		int distance) {
	if (distance == 0) {
		pixman_region32_copy(dst, src);
		return;
	}

	int nrects;
	pixman_box32_t *src_rects = pixman_region32_rectangles(src, &nrects);

	pixman_box32_t *dst_rects = malloc(nrects * sizeof(pixman_box32_t));
	if (dst_rects == NULL) {
		return;
	}

	for (int i = 0; i < nrects; ++i) {
		dst_rects[i].x1 = src_rects[i].x1 - distance;
		dst_rects[i].x2 = src_rects[i].x2 + distance;
		dst_rects[i].y1 = src_rects[i].y1 - distance;
		dst_rects[i].y2 = src_rects[i].y2 + distance;
	}

	pixman_region32_fini(dst);
	pixman_region32_init_rects(dst, dst_rects, nrects);
	free(dst_rects);
}

void wlr_region_rotated_bounds(pixman_region32_t *dst, pixman_region32_t *src,
		float rotation, int ox, int oy) {
	if (rotation == 0) {
		pixman_region32_copy(dst, src);
		return;
	}

	int nrects;
	pixman_box32_t *src_rects = pixman_region32_rectangles(src, &nrects);

	pixman_box32_t *dst_rects = malloc(nrects * sizeof(pixman_box32_t));
	if (dst_rects == NULL) {
		return;
	}

	for (int i = 0; i < nrects; ++i) {
		double x1 = src_rects[i].x1 - ox;
		double y1 = src_rects[i].y1 - oy;
		double x2 = src_rects[i].x2 - ox;
		double y2 = src_rects[i].y2 - oy;

		double rx1 = x1 * cos(rotation) - y1 * sin(rotation);
		double ry1 = x1 * sin(rotation) + y1 * cos(rotation);

		double rx2 = x2 * cos(rotation) - y1 * sin(rotation);
		double ry2 = x2 * sin(rotation) + y1 * cos(rotation);

		double rx3 = x2 * cos(rotation) - y2 * sin(rotation);
		double ry3 = x2 * sin(rotation) + y2 * cos(rotation);

		double rx4 = x1 * cos(rotation) - y2 * sin(rotation);
		double ry4 = x1 * sin(rotation) + y2 * cos(rotation);

		x1 = fmin(fmin(rx1, rx2), fmin(rx3, rx4));
		y1 = fmin(fmin(ry1, ry2), fmin(ry3, ry4));
		x2 = fmax(fmax(rx1, rx2), fmax(rx3, rx4));
		y2 = fmax(fmax(ry1, ry2), fmax(ry3, ry4));

		dst_rects[i].x1 = floor(ox + x1);
		dst_rects[i].x2 = ceil(ox + x2);
		dst_rects[i].y1 = floor(oy + y1);
		dst_rects[i].y2 = ceil(oy + y2);
	}

	pixman_region32_fini(dst);
	pixman_region32_init_rects(dst, dst_rects, nrects);
	free(dst_rects);
}

static void region_confine(pixman_region32_t *region, double x1, double y1, double x2,
		double y2, double *x2_out, double *y2_out, pixman_box32_t box) {
	double x_clamped = fmax(fmin(x2, box.x2 - 1), box.x1);
	double y_clamped = fmax(fmin(y2, box.y2 - 1), box.y1);

	// If the target coordinates are above box.{x,y}2 - 1, but less than
	// box.{x,y}2, then they are still within the box.
	if (floor(x_clamped) == floor(x2) && floor(y_clamped) == floor(y2)) {
		*x2_out = x2;
		*y2_out = y2;
		return;
	}

	double dx = x2 - x1;
	double dy = y2 - y1;

	// We use fabs to avoid negative zeroes and thus avoid a bug
	// with negative infinity.
	double delta = fmin(fabs(x_clamped - x1) / fabs(dx), fabs(y_clamped - y1) / fabs(dy));

	// We clamp it again due to precision errors.
	double x = fmax(fmin(delta * dx + x1, box.x2 - 1), box.x1);
	double y = fmax(fmin(delta * dy + y1, box.y2 - 1), box.y1);

	// Go one unit past the boundary to find an adjacent box.
	int x_ext = floor(x) + (dx == 0 ? 0 : dx > 0 ? 1 : -1);
	int y_ext = floor(y) + (dy == 0 ? 0 : dy > 0 ? 1 : -1);

	if (pixman_region32_contains_point(region, x_ext, y_ext, &box)) {
		return region_confine(region, x1, y1, x2, y2, x2_out, y2_out, box);
	} else if (dx == 0 || dy == 0) {
		*x2_out = x;
		*y2_out = y;
	} else {
		bool bordering_x = x == box.x1 || x == box.x2 - 1;
		bool bordering_y = y == box.y1 || y == box.y2 - 1;

		if (bordering_x == bordering_y) {
			double x2_potential, y2_potential;
			double tmp1, tmp2;
			region_confine(region, x, y, x, y2, &tmp1, &y2_potential, box);
			region_confine(region, x, y, x2, y, &x2_potential, &tmp2, box);
			if (fabs(x2_potential - x) > fabs(y2_potential - y)) {
				*x2_out = x2_potential;
				*y2_out = y;
			} else {
				*x2_out = x;
				*y2_out = y2_potential;
			}
		} else if (bordering_x) {
			return region_confine(region, x, y, x, y2, x2_out, y2_out, box);
		} else if (bordering_y) {
			return region_confine(region, x, y, x2, y, x2_out, y2_out, box);
		}
	}
}

bool wlr_region_confine(pixman_region32_t *region, double x1, double y1, double x2,
		double y2, double *x2_out, double *y2_out) {
	pixman_box32_t box;
	if (pixman_region32_contains_point(region, floor(x1), floor(y1), &box)) {
		region_confine(region, x1, y1, x2, y2, x2_out, y2_out, box);
		return true;
	} else {
		return false;
	}
}
