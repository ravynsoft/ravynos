/*
 * Copyright (C) 2010 Marek Olšák <maraeo@gmail.com>
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

#include <stdlib.h>
#include "radeon_remove_constants.h"
#include "radeon_dataflow.h"

struct mark_used_data {
	unsigned char * const_used;
	unsigned * has_rel_addr;
};

static void remap_regs(void * userdata, struct rc_instruction * inst,
			rc_register_file * pfile, unsigned int * pindex)
{
	unsigned *inv_remap_table = userdata;

	if (*pfile == RC_FILE_CONSTANT) {
		*pindex = inv_remap_table[*pindex];
	}
}

static void mark_used(void * userdata, struct rc_instruction * inst,
						struct rc_src_register * src)
{
	struct mark_used_data * d = userdata;

	if (src->File == RC_FILE_CONSTANT) {
		if (src->RelAddr) {
			*d->has_rel_addr = 1;
		} else {
			d->const_used[src->Index] = 1;
		}
	}
}

void rc_remove_unused_constants(struct radeon_compiler *c, void *user)
{
	unsigned **out_remap_table = (unsigned**)user;
	unsigned char *const_used;
	unsigned *remap_table;
	unsigned *inv_remap_table;
	unsigned has_rel_addr = 0;
	unsigned is_identity = 1;
	unsigned are_externals_remapped = 0;
	struct rc_constant *constants = c->Program.Constants.Constants;
	struct mark_used_data d;
	unsigned new_count;

	if (!c->Program.Constants.Count) {
		*out_remap_table = NULL;
		return;
	}

	const_used = malloc(c->Program.Constants.Count);
	memset(const_used, 0, c->Program.Constants.Count);

	d.const_used = const_used;
	d.has_rel_addr = &has_rel_addr;

	/* Pass 1: Mark used constants. */
	for (struct rc_instruction *inst = c->Program.Instructions.Next;
	     inst != &c->Program.Instructions; inst = inst->Next) {
		rc_for_all_reads_src(inst, mark_used, &d);
	}

	/* Pass 2: If there is relative addressing or dead constant elimination
	 * is disabled, mark all externals as used. */
	if (has_rel_addr || !c->remove_unused_constants) {
		for (unsigned i = 0; i < c->Program.Constants.Count; i++)
			if (constants[i].Type == RC_CONSTANT_EXTERNAL)
				const_used[i] = 1;
	}

	/* Pass 3: Make the remapping table and remap constants.
	 * This pass removes unused constants simply by overwriting them by other constants. */
	remap_table = malloc(c->Program.Constants.Count * sizeof(unsigned));
	inv_remap_table = malloc(c->Program.Constants.Count * sizeof(unsigned));
	new_count = 0;

	for (unsigned i = 0; i < c->Program.Constants.Count; i++) {
		if (const_used[i]) {
			remap_table[new_count] = i;
			inv_remap_table[i] = new_count;

			if (i != new_count) {
				if (constants[i].Type == RC_CONSTANT_EXTERNAL)
					are_externals_remapped = 1;

				constants[new_count] = constants[i];
				is_identity = 0;
			}
			new_count++;
		}
	}

	/*  is_identity ==> new_count == old_count
	 * !is_identity ==> new_count <  old_count */
	assert( is_identity || new_count <  c->Program.Constants.Count);
	assert(!((has_rel_addr || !c->remove_unused_constants) && are_externals_remapped));

	/* Pass 4: Redirect reads of all constants to their new locations. */
	if (!is_identity) {
		for (struct rc_instruction *inst = c->Program.Instructions.Next;
		     inst != &c->Program.Instructions; inst = inst->Next) {
			rc_remap_registers(inst, remap_regs, inv_remap_table);
		}
	}

	/* Set the new constant count. Note that new_count may be less than
	 * Count even though the remapping function is identity. In that case,
	 * the constants have been removed at the end of the array. */
	c->Program.Constants.Count = new_count;

	if (are_externals_remapped) {
		*out_remap_table = remap_table;
	} else {
		*out_remap_table = NULL;
		free(remap_table);
	}

	free(const_used);
	free(inv_remap_table);

	if (c->Debug & RC_DBG_LOG)
		rc_constants_print(&c->Program.Constants);
}
