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

#include "colors.h"

const struct envy_colors envy_null_colors = {
	.reset	= "",
	.iname	= "",
	.rname	= "",
	.mod	= "",
	.sym	= "",
	.reg	= "",
	.regsp	= "",
	.num	= "",
	.mem	= "",
	.btarg	= "",
	.ctarg	= "",
	.bctarg	= "",
	.eval	= "",
	.comm	= "",
	.err	= "",
};

const struct envy_colors envy_def_colors = {
	.reset	= "\x1b[0m",
	.iname	= "\x1b[0;32m",
	.rname	= "\x1b[0;32m",
	.mod	= "\x1b[0;36m",
	.sym	= "\x1b[0;36m",
	.reg	= "\x1b[0;31m",
	.regsp	= "\x1b[0;35m",
	.num	= "\x1b[0;33m",
	.mem	= "\x1b[0;35m",
	.btarg	= "\x1b[0;35m",
	.ctarg	= "\x1b[0;1;37m",
	.bctarg	= "\x1b[0;1;35m",
	.eval	= "\x1b[0;35m",
	.comm	= "\x1b[0;34m",
	.err	= "\x1b[0;1;31m",
};
