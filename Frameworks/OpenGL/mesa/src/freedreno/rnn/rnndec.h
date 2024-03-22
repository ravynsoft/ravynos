/*
 * Copyright (C) 2010 Marcin Kościelnicki <koriakin@0x04.net>
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

#ifndef RNNDEC_H
#define RNNDEC_H

#include "rnn.h"
#include "colors.h"

struct rnndecvariant {
	struct rnnenum *en;
	int variant;
};

struct rnndeccontext {
	struct rnndb *db;
	struct rnndecvariant **vars;
	int varsnum;
	int varsmax;
	const struct envy_colors *colors;
};

struct rnndecaddrinfo {
	struct rnntypeinfo *typeinfo;
	int width;
	char *name;
};

struct rnndeccontext *rnndec_newcontext(struct rnndb *db);
int rnndec_varadd(struct rnndeccontext *ctx, char *varset, const char *variant);
int rnndec_varmatch(struct rnndeccontext *ctx, struct rnnvarinfo *vi);
const char *rnndec_decode_enum(struct rnndeccontext *ctx, const char *enumname, uint64_t enumval);
char *rnndec_decodeval(struct rnndeccontext *ctx, struct rnntypeinfo *ti, uint64_t value);
int rnndec_checkaddr(struct rnndeccontext *ctx, struct rnndomain *domain, uint64_t addr, int write);
struct rnndecaddrinfo *rnndec_decodeaddr(struct rnndeccontext *ctx, struct rnndomain *domain, uint64_t addr, int write);
uint64_t rnndec_decodereg(struct rnndeccontext *ctx, struct rnndomain *domain, const char *name);

#endif
