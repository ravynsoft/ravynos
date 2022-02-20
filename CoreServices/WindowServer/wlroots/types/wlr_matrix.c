#include <math.h>
#include <string.h>
#include <wayland-server-protocol.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/box.h>

void wlr_matrix_identity(float mat[static 9]) {
	static const float identity[9] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};
	memcpy(mat, identity, sizeof(identity));
}

void wlr_matrix_multiply(float mat[static 9], const float a[static 9],
		const float b[static 9]) {
	float product[9];

	product[0] = a[0]*b[0] + a[1]*b[3] + a[2]*b[6];
	product[1] = a[0]*b[1] + a[1]*b[4] + a[2]*b[7];
	product[2] = a[0]*b[2] + a[1]*b[5] + a[2]*b[8];

	product[3] = a[3]*b[0] + a[4]*b[3] + a[5]*b[6];
	product[4] = a[3]*b[1] + a[4]*b[4] + a[5]*b[7];
	product[5] = a[3]*b[2] + a[4]*b[5] + a[5]*b[8];

	product[6] = a[6]*b[0] + a[7]*b[3] + a[8]*b[6];
	product[7] = a[6]*b[1] + a[7]*b[4] + a[8]*b[7];
	product[8] = a[6]*b[2] + a[7]*b[5] + a[8]*b[8];

	memcpy(mat, product, sizeof(product));
}

void wlr_matrix_transpose(float mat[static 9], const float a[static 9]) {
	float transposition[9] = {
		a[0], a[3], a[6],
		a[1], a[4], a[7],
		a[2], a[5], a[8],
	};
	memcpy(mat, transposition, sizeof(transposition));
}

void wlr_matrix_translate(float mat[static 9], float x, float y) {
	float translate[9] = {
		1.0f, 0.0f, x,
		0.0f, 1.0f, y,
		0.0f, 0.0f, 1.0f,
	};
	wlr_matrix_multiply(mat, mat, translate);
}

void wlr_matrix_scale(float mat[static 9], float x, float y) {
	float scale[9] = {
		x,    0.0f, 0.0f,
		0.0f, y,    0.0f,
		0.0f, 0.0f, 1.0f,
	};
	wlr_matrix_multiply(mat, mat, scale);
}

void wlr_matrix_rotate(float mat[static 9], float rad) {
	float rotate[9] = {
		cos(rad), -sin(rad), 0.0f,
		sin(rad),  cos(rad), 0.0f,
		0.0f,      0.0f,     1.0f,
	};
	wlr_matrix_multiply(mat, mat, rotate);
}

static const float transforms[][9] = {
	[WL_OUTPUT_TRANSFORM_NORMAL] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	},
	[WL_OUTPUT_TRANSFORM_90] = {
		0.0f, 1.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	},
	[WL_OUTPUT_TRANSFORM_180] = {
		-1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	},
	[WL_OUTPUT_TRANSFORM_270] = {
		0.0f, -1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	},
	[WL_OUTPUT_TRANSFORM_FLIPPED] = {
		-1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	},
	[WL_OUTPUT_TRANSFORM_FLIPPED_90] = {
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	},
	[WL_OUTPUT_TRANSFORM_FLIPPED_180] = {
		1.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	},
	[WL_OUTPUT_TRANSFORM_FLIPPED_270] = {
		0.0f, -1.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	},
};

void wlr_matrix_transform(float mat[static 9],
		enum wl_output_transform transform) {
	wlr_matrix_multiply(mat, mat, transforms[transform]);
}

// Equivalent to glOrtho(0, width, 0, height, 1, -1) with the transform applied
void wlr_matrix_projection(float mat[static 9], int width, int height,
		enum wl_output_transform transform) {
	memset(mat, 0, sizeof(*mat) * 9);

	const float *t = transforms[transform];
	float x = 2.0f / width;
	float y = 2.0f / height;

	// Rotation + reflection
	mat[0] = x * t[0];
	mat[1] = x * t[1];
	mat[3] = y * -t[3];
	mat[4] = y * -t[4];

	// Translation
	mat[2] = -copysign(1.0f, mat[0] + mat[1]);
	mat[5] = -copysign(1.0f, mat[3] + mat[4]);

	// Identity
	mat[8] = 1.0f;
}

void wlr_matrix_project_box(float mat[static 9], const struct wlr_box *box,
		enum wl_output_transform transform, float rotation,
		const float projection[static 9]) {
	int x = box->x;
	int y = box->y;
	int width = box->width;
	int height = box->height;

	wlr_matrix_identity(mat);
	wlr_matrix_translate(mat, x, y);

	if (rotation != 0) {
		wlr_matrix_translate(mat, width/2, height/2);
		wlr_matrix_rotate(mat, rotation);
		wlr_matrix_translate(mat, -width/2, -height/2);
	}

	wlr_matrix_scale(mat, width, height);

	if (transform != WL_OUTPUT_TRANSFORM_NORMAL) {
		wlr_matrix_translate(mat, 0.5, 0.5);
		wlr_matrix_transform(mat, transform);
		wlr_matrix_translate(mat, -0.5, -0.5);
	}

	wlr_matrix_multiply(mat, projection, mat);
}
