/*
 * Copyright (C) 2009 Nicolai Haehnle.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "radeon_code.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "radeon_program.h"

void rc_constants_init(struct rc_constant_list * c)
{
	memset(c, 0, sizeof(*c));
}

/**
 * Copy a constants structure, assuming that the destination structure
 * is not initialized.
 */
void rc_constants_copy(struct rc_constant_list * dst, struct rc_constant_list * src)
{
	dst->Constants = malloc(sizeof(struct rc_constant) * src->Count);
	memcpy(dst->Constants, src->Constants, sizeof(struct rc_constant) * src->Count);
	dst->Count = src->Count;
	dst->_Reserved = src->Count;
}

void rc_constants_destroy(struct rc_constant_list * c)
{
	free(c->Constants);
	memset(c, 0, sizeof(*c));
}

unsigned rc_constants_add(struct rc_constant_list * c, struct rc_constant * constant)
{
	unsigned index = c->Count;

	if (c->Count >= c->_Reserved) {
		struct rc_constant * newlist;

		c->_Reserved = c->_Reserved * 2;
		if (!c->_Reserved)
			c->_Reserved = 16;

		newlist = malloc(sizeof(struct rc_constant) * c->_Reserved);
		memcpy(newlist, c->Constants, sizeof(struct rc_constant) * c->Count);

		free(c->Constants);
		c->Constants = newlist;
	}

	c->Constants[index] = *constant;
	c->Count++;

	return index;
}


/**
 * Add a state vector to the constant list, while trying to avoid duplicates.
 */
unsigned rc_constants_add_state(struct rc_constant_list * c, unsigned state0, unsigned state1)
{
	unsigned index;
	struct rc_constant constant;

	for(index = 0; index < c->Count; ++index) {
		if (c->Constants[index].Type == RC_CONSTANT_STATE) {
			if (c->Constants[index].u.State[0] == state0 &&
			    c->Constants[index].u.State[1] == state1)
				return index;
		}
	}

	memset(&constant, 0, sizeof(constant));
	constant.Type = RC_CONSTANT_STATE;
	constant.Size = 4;
	constant.u.State[0] = state0;
	constant.u.State[1] = state1;

	return rc_constants_add(c, &constant);
}


/**
 * Add an immediate vector to the constant list, while trying to avoid
 * duplicates.
 */
unsigned rc_constants_add_immediate_vec4(struct rc_constant_list * c, const float * data)
{
	unsigned index;
	struct rc_constant constant;

	for(index = 0; index < c->Count; ++index) {
		if (c->Constants[index].Type == RC_CONSTANT_IMMEDIATE) {
			if (!memcmp(c->Constants[index].u.Immediate, data, sizeof(float)*4))
				return index;
		}
	}

	memset(&constant, 0, sizeof(constant));
	constant.Type = RC_CONSTANT_IMMEDIATE;
	constant.Size = 4;
	memcpy(constant.u.Immediate, data, sizeof(float) * 4);

	return rc_constants_add(c, &constant);
}


/**
 * Add an immediate scalar to the constant list, while trying to avoid
 * duplicates.
 */
unsigned rc_constants_add_immediate_scalar(struct rc_constant_list * c, float data, unsigned * swizzle)
{
	unsigned index;
	int free_index = -1;
	struct rc_constant constant;

	for(index = 0; index < c->Count; ++index) {
		if (c->Constants[index].Type == RC_CONSTANT_IMMEDIATE) {
			unsigned comp;
			for(comp = 0; comp < c->Constants[index].Size; ++comp) {
				if (c->Constants[index].u.Immediate[comp] == data) {
					*swizzle = RC_MAKE_SWIZZLE_SMEAR(comp);
					return index;
				}
			}

			if (c->Constants[index].Size < 4)
				free_index = index;
		}
	}

	if (free_index >= 0) {
		unsigned comp = c->Constants[free_index].Size++;
		c->Constants[free_index].u.Immediate[comp] = data;
		*swizzle = RC_MAKE_SWIZZLE_SMEAR(comp);
		return free_index;
	}

	memset(&constant, 0, sizeof(constant));
	constant.Type = RC_CONSTANT_IMMEDIATE;
	constant.Size = 1;
	constant.u.Immediate[0] = data;
	*swizzle = RC_SWIZZLE_XXXX;

	return rc_constants_add(c, &constant);
}

void rc_constants_print(struct rc_constant_list * c)
{
	unsigned int i;
	for(i = 0; i < c->Count; i++) {
		if (c->Constants[i].Type == RC_CONSTANT_IMMEDIATE) {
			float * values = c->Constants[i].u.Immediate;
			fprintf(stderr, "CONST[%u] = "
				"{ %10.4f %10.4f %10.4f %10.4f }\n",
				i, values[0],values[1], values[2], values[3]);
		}
	}
}
