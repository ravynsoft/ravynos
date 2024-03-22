/*
 * Copyright 2011 Tom Stellard <tstellar@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include "radeon_variable.h"

#include "memory_pool.h"
#include "radeon_compiler_util.h"
#include "radeon_dataflow.h"
#include "radeon_list.h"
#include "radeon_opcodes.h"
#include "radeon_program.h"

/**
 * Rewrite the index and writemask for the destination register of var
 * and its friends to new_index and new_writemask.  This function also takes
 * care of rewriting the swizzles for the sources of var.
 */
void rc_variable_change_dst(
	struct rc_variable * var,
	unsigned int new_index,
	unsigned int new_writemask)
{
	struct rc_variable * var_ptr;
	struct rc_list * readers;
	unsigned int old_mask = rc_variable_writemask_sum(var);
	unsigned int conversion_swizzle =
			rc_make_conversion_swizzle(old_mask, new_writemask);

	for (var_ptr = var; var_ptr; var_ptr = var_ptr->Friend) {
		if (var_ptr->Inst->Type == RC_INSTRUCTION_NORMAL) {
			rc_normal_rewrite_writemask(var_ptr->Inst,
							conversion_swizzle);
			var_ptr->Inst->U.I.DstReg.Index = new_index;
		} else {
			struct rc_pair_sub_instruction * sub;
			if (var_ptr->Dst.WriteMask == RC_MASK_W) {
				assert(new_writemask & RC_MASK_W);
				sub = &var_ptr->Inst->U.P.Alpha;
			} else {
				sub = &var_ptr->Inst->U.P.RGB;
				rc_pair_rewrite_writemask(sub,
							conversion_swizzle);
			}
			sub->DestIndex = new_index;
		}
	}

	readers = rc_variable_readers_union(var);

	for ( ; readers; readers = readers->Next) {
		struct rc_reader * reader = readers->Item;
		if (reader->Inst->Type == RC_INSTRUCTION_NORMAL) {
			reader->U.I.Src->Index = new_index;
			reader->U.I.Src->Swizzle = rc_rewrite_swizzle(
				reader->U.I.Src->Swizzle, conversion_swizzle);
		} else {
			struct rc_pair_instruction * pair_inst =
							&reader->Inst->U.P;
			unsigned int src_type = rc_source_type_swz(
							reader->U.P.Arg->Swizzle);

			int src_index = reader->U.P.Arg->Source;
			if (src_index == RC_PAIR_PRESUB_SRC) {
				src_index = rc_pair_get_src_index(
						pair_inst, reader->U.P.Src);
			}
			rc_pair_remove_src(reader->Inst, src_type,
							src_index);
			/* Reuse the source index of the source that
			 * was just deleted and set its register
			 * index.  We can't use rc_pair_alloc_source
			 * for this because it might return a source
			 * index that is already being used. */
			if (src_type & RC_SOURCE_RGB) {
				pair_inst->RGB.Src[src_index]
					.Used =	1;
				pair_inst->RGB.Src[src_index]
					.Index = new_index;
				pair_inst->RGB.Src[src_index]
					.File = RC_FILE_TEMPORARY;
			}
			if (src_type & RC_SOURCE_ALPHA) {
				pair_inst->Alpha.Src[src_index]
					.Used = 1;
				pair_inst->Alpha.Src[src_index]
					.Index = new_index;
				pair_inst->Alpha.Src[src_index]
					.File = RC_FILE_TEMPORARY;
			}
			reader->U.P.Arg->Swizzle = rc_rewrite_swizzle(
				reader->U.P.Arg->Swizzle, conversion_swizzle);
			if (reader->U.P.Arg->Source != RC_PAIR_PRESUB_SRC) {
				reader->U.P.Arg->Source = src_index;
			}
		}
	}
}

/**
 * Compute the live intervals for var and its friends.
 */
void rc_variable_compute_live_intervals(struct rc_variable * var)
{
	while(var) {
		unsigned int i;
		unsigned int start = var->Inst->IP;

		for (i = 0; i < var->ReaderCount; i++) {
			unsigned int chan;
			unsigned int chan_start = start;
			unsigned int chan_end = var->Readers[i].Inst->IP;
			unsigned int mask = var->Readers[i].WriteMask;
			struct rc_instruction * inst;

			/* Extend the live interval of T0 to the start of the
			 * loop for sequences like:
			 * BGNLOOP
			 * read T0
			 * ...
			 * write T0
			 * ENDLOOP
			 */
			if (var->Readers[i].Inst->IP < start) {
				struct rc_instruction * bgnloop =
					rc_match_endloop(var->Readers[i].Inst);
				chan_start = bgnloop->IP;
			}

			/* Extend the live interval of T0 to the start of the
			 * loop in case there is a BRK instruction in the loop
			 * (we don't actually check for a BRK instruction we
			 * assume there is one somewhere in the loop, which
			 * there usually is) for sequences like:
			 * BGNLOOP
			 * ...
			 * conditional BRK
			 * ...
			 * write T0
			 * ENDLOOP
			 * read T0
			 ***************************************************
			 * Extend the live interval of T0 to the end of the
			 * loop for sequences like:
			 * write T0
			 * BGNLOOP
			 * ...
			 * read T0
			 * ENDLOOP
			 */
			for (inst = var->Inst; inst != var->Readers[i].Inst;
							inst = inst->Next) {
				rc_opcode op = rc_get_flow_control_inst(inst);
				if (op == RC_OPCODE_ENDLOOP) {
					struct rc_instruction * bgnloop =
						rc_match_endloop(inst);
					if (bgnloop->IP < chan_start) {
						chan_start = bgnloop->IP;
					}
				} else if (op == RC_OPCODE_BGNLOOP) {
					struct rc_instruction * endloop =
						rc_match_bgnloop(inst);
					if (endloop->IP > chan_end) {
						chan_end = endloop->IP;
					}
				}
			}

			for (chan = 0; chan < 4; chan++) {
				if ((mask >> chan) & 0x1) {
					if (!var->Live[chan].Used
					|| chan_start < var->Live[chan].Start) {
						var->Live[chan].Start =
								chan_start;
					}
					if (!var->Live[chan].Used
					|| chan_end > var->Live[chan].End) {
						var->Live[chan].End = chan_end;
					}
					var->Live[chan].Used = 1;
				}
			}
		}
		var = var->Friend;
	}
}

/**
 * @return 1 if a and b share a reader
 * @return 0 if they do not
 */
static unsigned int readers_intersect(
	struct rc_variable * a,
	struct rc_variable * b)
{
	unsigned int a_index, b_index;
	for (a_index = 0; a_index < a->ReaderCount; a_index++) {
		struct rc_reader reader_a = a->Readers[a_index];
		for (b_index = 0; b_index < b->ReaderCount; b_index++) {
			struct rc_reader reader_b = b->Readers[b_index];
			if (reader_a.Inst->Type == RC_INSTRUCTION_NORMAL
				&& reader_b.Inst->Type == RC_INSTRUCTION_NORMAL
				&& reader_a.U.I.Src == reader_b.U.I.Src) {

				return 1;
			}
			if (reader_a.Inst->Type == RC_INSTRUCTION_PAIR
				&& reader_b.Inst->Type == RC_INSTRUCTION_PAIR
				&& reader_a.U.P.Src == reader_b.U.P.Src) {

				return 1;
			}
		}
	}
	return 0;
}

void rc_variable_add_friend(
	struct rc_variable * var,
	struct rc_variable * friend)
{
	assert(var->Dst.Index == friend->Dst.Index);
	while(var->Friend) {
		var = var->Friend;
	}
	var->Friend = friend;
}

struct rc_variable * rc_variable(
	struct radeon_compiler * c,
	unsigned int DstFile,
	unsigned int DstIndex,
	unsigned int DstWriteMask,
	struct rc_reader_data * reader_data)
{
	struct rc_variable * new =
			memory_pool_malloc(&c->Pool, sizeof(struct rc_variable));
	memset(new, 0, sizeof(struct rc_variable));
	new->C = c;
	new->Dst.File = DstFile;
	new->Dst.Index = DstIndex;
	new->Dst.WriteMask = DstWriteMask;
	if (reader_data) {
		new->Inst = reader_data->Writer;
		new->ReaderCount = reader_data->ReaderCount;
		new->Readers = reader_data->Readers;
	}
	return new;
}

static void get_variable_helper(
	struct rc_list ** variable_list,
	struct rc_variable * variable)
{
	struct rc_list * list_ptr;
	for (list_ptr = *variable_list; list_ptr; list_ptr = list_ptr->Next) {
		struct rc_variable * var;
		for (var = list_ptr->Item; var; var = var->Friend) {
			if (readers_intersect(var, variable)) {
				rc_variable_add_friend(var, variable);
				return;
			}
		}
	}
	rc_list_add(variable_list, rc_list(&variable->C->Pool, variable));
}

static void get_variable_pair_helper(
	struct rc_list ** variable_list,
	struct radeon_compiler * c,
	struct rc_instruction * inst,
	struct rc_pair_sub_instruction * sub_inst)
{
	struct rc_reader_data reader_data;
	struct rc_variable * new_var;
	rc_register_file file;
	unsigned int writemask;

	if (sub_inst->Opcode == RC_OPCODE_NOP) {
		return;
	}
	memset(&reader_data, 0, sizeof(struct rc_reader_data));
	rc_get_readers_sub(c, inst, sub_inst, &reader_data, NULL, NULL, NULL);

	if (reader_data.ReaderCount == 0) {
		return;
	}

	if (sub_inst->WriteMask) {
		file = RC_FILE_TEMPORARY;
		writemask = sub_inst->WriteMask;
	} else if (sub_inst->OutputWriteMask) {
		file = RC_FILE_OUTPUT;
		writemask = sub_inst->OutputWriteMask;
	} else {
		writemask = 0;
		file = RC_FILE_NONE;
	}
	new_var = rc_variable(c, file, sub_inst->DestIndex, writemask,
								&reader_data);
	get_variable_helper(variable_list, new_var);
}

/**
 * Compare function for sorting variable pointers by the lowest instruction
 * IP from it and its friends.
 */
static int cmpfunc_variable_by_ip (const void * a, const void * b) {
	struct rc_variable * var_a = *(struct rc_variable **)a;
	struct rc_variable * var_b = *(struct rc_variable **)b;
	unsigned int min_ip_a = var_a->Inst->IP;
	unsigned int min_ip_b = var_b->Inst->IP;

	/* Find the minimal IP of a variable and its friends */
	while (var_a->Friend) {
		var_a = var_a->Friend;
		if (var_a->Inst->IP < min_ip_a)
			min_ip_a = var_a->Inst->IP;
	}
	while (var_b->Friend) {
		var_b = var_b->Friend;
		if (var_b->Inst->IP < min_ip_b)
			min_ip_b = var_b->Inst->IP;
	}

	return (int)min_ip_a - (int)min_ip_b;
}

/**
 * Generate a list of variables used by the shader program.  Each instruction
 * that writes to a register is considered a variable.  The struct rc_variable
 * data structure includes a list of readers and is essentially a
 * definition-use chain.  Any two variables that share a reader are considered
 * "friends" and they are linked together via the Friend attribute.
 */
struct rc_list * rc_get_variables(struct radeon_compiler * c)
{
	struct rc_instruction * inst;
	struct rc_list * variable_list = NULL;

	/* We search for the variables in two loops in order to get it right in
	 * the following specific case
	 *
	 * IF aluresult.x___;
	 *   ...
	 *   MAD temp[0].xyz, src0.000, src0.111, src0.000
	 *   MAD temp[0].w, src0.0, src0.1, src0.0
	 * ELSE;
	 *   ...
	 *   TXB temp[0], temp[1].xy_w, 2D[0] SEM_WAIT SEM_ACQUIRE;
	 * ENDIF;
	 * src0.xyz = input[0], src0.w = input[0], src1.xyz = temp[0], src1.w = temp[0] SEM_WAIT
	 * MAD temp[1].xyz, src0.xyz, src1.xyz, src0.000
	 * MAD temp[1].w, src0.w, src1.w, src0.0
	 *
	 * If we go just in one loop, we will first create two variables for the
	 * temp[0].xyz and temp[0].w. This happens because they don't share a reader
	 * as the src1.xyz and src1.w of the instruction where the value is used are
	 * in theory independent. They are not because the same register is written
	 * also by the texture instruction in the other branch and TEX can't write xyz
	 * and w separately.
	 *
	 * Therefore first search for RC_INSTRUCTION_NORMAL to create variables from
	 * the texture instruction and than the pair instructions will be properly
	 * marked as friends. So we will end with only one variable here as we should.
	 *
	 * This doesn't matter before the pair translation, because everything is
	 * RC_INSTRUCTION_NORMAL.
	 */
	for (inst = c->Program.Instructions.Next;
					inst != &c->Program.Instructions;
					inst = inst->Next) {
		if (inst->Type == RC_INSTRUCTION_NORMAL) {
			struct rc_reader_data reader_data;
			struct rc_variable * new_var;
			memset(&reader_data, 0, sizeof(reader_data));
			rc_get_readers(c, inst, &reader_data, NULL, NULL, NULL);
			if (reader_data.ReaderCount == 0) {
				/* Variable is only returned if there is both writer
				 * and reader. This means dead writes will not get
				 * register allocated as a result and can overwrite random
				 * registers. Assert on dead writes insted so we can improve
				 * the DCE.
				 */
				const struct rc_opcode_info *opcode =
					rc_get_opcode_info(inst->U.I.Opcode);
				assert(c->type == RC_FRAGMENT_PROGRAM ||
					!opcode->HasDstReg ||
					inst->U.I.DstReg.File == RC_FILE_OUTPUT ||
					inst->U.I.DstReg.File == RC_FILE_ADDRESS);
                                continue;
			}
			new_var = rc_variable(c, inst->U.I.DstReg.File,
				inst->U.I.DstReg.Index,
				inst->U.I.DstReg.WriteMask, &reader_data);
			get_variable_helper(&variable_list, new_var);
		}
	}

	bool needs_sorting = false;
	for (inst = c->Program.Instructions.Next;
					inst != &c->Program.Instructions;
					inst = inst->Next) {
		if (inst->Type != RC_INSTRUCTION_NORMAL) {
			needs_sorting = true;
			get_variable_pair_helper(&variable_list, c, inst,
							&inst->U.P.RGB);
			get_variable_pair_helper(&variable_list, c, inst,
							&inst->U.P.Alpha);
		}
	}

	if (variable_list && needs_sorting) {
		unsigned int count = rc_list_count(variable_list);
		struct rc_variable **variables = memory_pool_malloc(&c->Pool,
				sizeof(struct rc_variable *) * count);

		struct rc_list * current = variable_list;
		for(unsigned int i = 0; current; i++, current = current->Next) {
			struct rc_variable * var = current->Item;
			variables[i] = var;
		}

		qsort(variables, count, sizeof(struct rc_variable *), cmpfunc_variable_by_ip);

		current = variable_list;
		for(unsigned int i = 0; current; i++, current = current->Next) {
			current->Item = variables[i];
		}
	}

	return variable_list;
}

/**
 * @return The bitwise or of the writemasks of a variable and all of its
 * friends.
 */
unsigned int rc_variable_writemask_sum(struct rc_variable * var)
{
	unsigned int writemask = 0;
	while(var) {
		writemask |= var->Dst.WriteMask;
		var = var->Friend;
	}
	return writemask;
}

/*
 * @return A list of readers for a variable and its friends.  Readers
 * that read from two different variable friends are only included once in
 * this list.
 */
struct rc_list * rc_variable_readers_union(struct rc_variable * var)
{
	struct rc_list * list = NULL;
	while (var) {
		unsigned int i;
		for (i = 0; i < var->ReaderCount; i++) {
			struct rc_list * temp;
			struct rc_reader * a = &var->Readers[i];
			unsigned int match = 0;
			for (temp = list; temp; temp = temp->Next) {
				struct rc_reader * b = temp->Item;
				if (a->Inst->Type != b->Inst->Type) {
					continue;
				}
				if (a->Inst->Type == RC_INSTRUCTION_NORMAL) {
					if (a->U.I.Src == b->U.I.Src) {
						match = 1;
						break;
					}
				}
				if (a->Inst->Type == RC_INSTRUCTION_PAIR) {
					if (a->U.P.Arg == b->U.P.Arg
					    && a->U.P.Src == b->U.P.Src) {
						match = 1;
						break;
					}
				}
			}
			if (match) {
				continue;
			}
			rc_list_add(&list, rc_list(&var->C->Pool, a));
		}
		var = var->Friend;
	}
	return list;
}

static unsigned int reader_equals_src(
	struct rc_reader reader,
	unsigned int src_type,
	void * src)
{
	if (reader.Inst->Type != src_type) {
		return 0;
	}
	if (src_type == RC_INSTRUCTION_NORMAL) {
		return reader.U.I.Src == src;
	} else {
		return reader.U.P.Src == src;
	}
}

static unsigned int variable_writes_src(
	struct rc_variable * var,
	unsigned int src_type,
	void * src)
{
	unsigned int i;
	for (i = 0; i < var->ReaderCount; i++) {
		if (reader_equals_src(var->Readers[i], src_type, src)) {
			return 1;
		}
	}
	return 0;
}


struct rc_list * rc_variable_list_get_writers(
	struct rc_list * var_list,
	unsigned int src_type,
	void * src)
{
	struct rc_list * list_ptr;
	struct rc_list * writer_list = NULL;
	for (list_ptr = var_list; list_ptr; list_ptr = list_ptr->Next) {
		struct rc_variable * var = list_ptr->Item;
		if (variable_writes_src(var, src_type, src)) {
			struct rc_variable * friend;
			rc_list_add(&writer_list, rc_list(&var->C->Pool, var));
			for (friend = var->Friend; friend;
						friend = friend->Friend) {
				if (variable_writes_src(friend, src_type, src)) {
					rc_list_add(&writer_list,
						rc_list(&var->C->Pool, friend));
				}
			}
			/* Once we have identified the variable and its
			 * friends that write this source, we can stop
			 * stop searching, because we know none of the
			 * other variables in the list will write this source.
			 * If they did they would be friends of var.
			 */
			break;
		}
	}
	return writer_list;
}

struct rc_list * rc_variable_list_get_writers_one_reader(
	struct rc_list * var_list,
	unsigned int src_type,
	void * src)
{
	struct rc_list * writer_list =
		rc_variable_list_get_writers(var_list, src_type, src);
	struct rc_list * reader_list =
		rc_variable_readers_union(writer_list->Item);
	if (rc_list_count(reader_list) > 1) {
		return NULL;
	} else {
		return writer_list;
	}
}

void rc_variable_print(struct rc_variable * var)
{
	unsigned int i;
	while (var) {
		fprintf(stderr, "%u: TEMP[%u].%u: ",
			var->Inst->IP, var->Dst.Index, var->Dst.WriteMask);
		for (i = 0; i < 4; i++) {
			fprintf(stderr, "chan %u: start=%u end=%u ", i,
					var->Live[i].Start, var->Live[i].End);
		}
		fprintf(stderr, "%u readers\n", var->ReaderCount);
		if (var->Friend) {
			fprintf(stderr, "Friend: \n\t");
		}
		var = var->Friend;
	}
}
