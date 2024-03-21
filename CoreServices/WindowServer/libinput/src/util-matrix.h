/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2013-2015 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "config.h"

#include <string.h>
#include <stdbool.h>
#include <math.h>

struct matrix {
	float val[3][3]; /* [row][col] */
};

static inline double
deg2rad(int degree)
{
	return M_PI * degree / 180.0;
}

static inline void
matrix_init_identity(struct matrix *m)
{
	memset(m, 0, sizeof(*m));
	m->val[0][0] = 1;
	m->val[1][1] = 1;
	m->val[2][2] = 1;
}

static inline void
matrix_from_farray6(struct matrix *m, const float values[6])
{
	matrix_init_identity(m);
	m->val[0][0] = values[0];
	m->val[0][1] = values[1];
	m->val[0][2] = values[2];
	m->val[1][0] = values[3];
	m->val[1][1] = values[4];
	m->val[1][2] = values[5];
}

static inline void
matrix_init_scale(struct matrix *m, float sx, float sy)
{
	matrix_init_identity(m);
	m->val[0][0] = sx;
	m->val[1][1] = sy;
}

static inline void
matrix_init_translate(struct matrix *m, float x, float y)
{
	matrix_init_identity(m);
	m->val[0][2] = x;
	m->val[1][2] = y;
}

static inline void
matrix_init_rotate(struct matrix *m, int degrees)
{
	double s, c;

	s = sin(deg2rad(degrees));
	c = cos(deg2rad(degrees));

	matrix_init_identity(m);
	m->val[0][0] = c;
	m->val[0][1] = -s;
	m->val[1][0] = s;
	m->val[1][1] = c;
}

static inline bool
matrix_is_identity(const struct matrix *m)
{
	return (m->val[0][0] == 1 &&
		m->val[0][1] == 0 &&
		m->val[0][2] == 0 &&
		m->val[1][0] == 0 &&
		m->val[1][1] == 1 &&
		m->val[1][2] == 0 &&
		m->val[2][0] == 0 &&
		m->val[2][1] == 0 &&
		m->val[2][2] == 1);
}

static inline void
matrix_mult(struct matrix *dest,
	    const struct matrix *m1,
	    const struct matrix *m2)
{
	struct matrix m; /* allow for dest == m1 or dest == m2 */
	int row, col, i;

	for (row = 0; row < 3; row++) {
		for (col = 0; col < 3; col++) {
			double v = 0;
			for (i = 0; i < 3; i++) {
				v += m1->val[row][i] * m2->val[i][col];
			}
			m.val[row][col] = v;
		}
	}

	memcpy(dest, &m, sizeof(m));
}

static inline void
matrix_mult_vec(const struct matrix *m, int *x, int *y)
{
	int tx, ty;

	tx = *x * m->val[0][0] + *y * m->val[0][1] + m->val[0][2];
	ty = *x * m->val[1][0] + *y * m->val[1][1] + m->val[1][2];

	*x = tx;
	*y = ty;
}

static inline void
matrix_mult_vec_double(const struct matrix *m, double *x, double *y)
{
	double tx, ty;

	tx = *x * m->val[0][0] + *y * m->val[0][1] + m->val[0][2];
	ty = *x * m->val[1][0] + *y * m->val[1][1] + m->val[1][2];

	*x = tx;
	*y = ty;
}

static inline void
matrix_to_farray6(const struct matrix *m, float out[6])
{
	out[0] = m->val[0][0];
	out[1] = m->val[0][1];
	out[2] = m->val[0][2];
	out[3] = m->val[1][0];
	out[4] = m->val[1][1];
	out[5] = m->val[1][2];
}

static inline void
matrix_to_relative(struct matrix *dest, const struct matrix *src)
{
	matrix_init_identity(dest);
	dest->val[0][0] = src->val[0][0];
	dest->val[0][1] = src->val[0][1];
	dest->val[1][0] = src->val[1][0];
	dest->val[1][1] = src->val[1][1];
}
