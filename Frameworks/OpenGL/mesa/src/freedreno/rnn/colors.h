/*
 * Copyright (C) 2012 Marcin Kościelnicki <koriakin@0x04.net>
 * All Rights Reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef COLORS_H
#define COLORS_H

struct envy_colors {
	const char *reset;
	const char *iname;	/* instruction name */
	const char *rname;	/* register or bitfield name */
	const char *mod;	/* instruction modifier */
	const char *sym;	/* auxiliary char like { , + */
	const char *reg;	/* ISA register */
	const char *regsp;	/* special ISA register */
	const char *num;	/* immediate number */
	const char *mem;	/* memory reference */
	const char *btarg;	/* branch target */
	const char *ctarg;	/* call target */
	const char *bctarg;	/* branch and call target */
	const char *eval;	/* enum value */
	const char *comm;	/* comment */
	const char *err;	/* error */
};

extern const struct envy_colors envy_null_colors;
extern const struct envy_colors envy_def_colors;

#endif
