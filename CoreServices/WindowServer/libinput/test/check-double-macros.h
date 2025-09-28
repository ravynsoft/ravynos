/*
 * Copyright © 2019 Red Hat, Inc.
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

#include <check.h>

#undef ck_assert_double_eq
#undef ck_assert_double_ne
#undef ck_assert_double_lt
#undef ck_assert_double_le
#undef ck_assert_double_gt
#undef ck_assert_double_ge
#undef ck_assert_double_eq_tol
#undef ck_assert_double_ne_tol

#define CK_DOUBLE_EQ_EPSILON 1E-3
#define _ck_assert_double_eq(X,Y, epsilon)  \
	do { \
		double _ck_x = X; \
		double _ck_y = Y; \
		ck_assert_msg(fabs(_ck_x - _ck_y) < epsilon, \
			      "Assertion '" #X " == " #Y \
			      "' failed: "#X"==%f, "#Y"==%f", \
			      _ck_x, \
			      _ck_y); \
	} while (0)

#define _ck_assert_double_ne(X,Y, epsilon)  \
	do { \
		double _ck_x = X; \
		double _ck_y = Y; \
		ck_assert_msg(fabs(_ck_x - _ck_y) > epsilon, \
			      "Assertion '" #X " != " #Y \
			      "' failed: "#X"==%f, "#Y"==%f", \
			      _ck_x, \
			      _ck_y); \
	} while (0)

#define ck_assert_double_eq(X, Y) _ck_assert_double_eq(X, Y, CK_DOUBLE_EQ_EPSILON)
#define ck_assert_double_eq_tol(X, Y, tol) _ck_assert_double_eq(X, Y, tol)
#define ck_assert_double_ne(X, Y) _ck_assert_double_ne(X, Y, CK_DOUBLE_EQ_EPSILON)
#define ck_assert_double_ne_tol(X, Y, tol) _ck_assert_double_ne(X, Y, tol)

#define _ck_assert_double_eq_op(X, OP, Y)  \
	do { \
		double _ck_x = X; \
		double _ck_y = Y; \
		ck_assert_msg(_ck_x OP _ck_y || \
			      fabs(_ck_x - _ck_y) < CK_DOUBLE_EQ_EPSILON, \
			      "Assertion '" #X#OP#Y \
			      "' failed: "#X"==%f, "#Y"==%f", \
			      _ck_x, \
			      _ck_y); \
	} while (0)

#define _ck_assert_double_ne_op(X, OP,Y) \
	do { \
		double _ck_x = X; \
		double _ck_y = Y; \
		ck_assert_msg(_ck_x OP _ck_y && \
			      fabs(_ck_x - _ck_y) > CK_DOUBLE_EQ_EPSILON, \
			      "Assertion '" #X#OP#Y \
			      "' failed: "#X"==%f, "#Y"==%f", \
			      _ck_x, \
			      _ck_y); \
	} while (0)

#define ck_assert_double_lt(X, Y) _ck_assert_double_ne_op(X, <, Y)
#define ck_assert_double_le(X, Y) _ck_assert_double_eq_op(X, <=, Y)
#define ck_assert_double_gt(X, Y) _ck_assert_double_ne_op(X, >, Y)
#define ck_assert_double_ge(X, Y) _ck_assert_double_eq_op(X, >=, Y)

