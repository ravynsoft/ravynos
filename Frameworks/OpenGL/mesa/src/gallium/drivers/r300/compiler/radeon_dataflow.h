/*
 * Copyright (C) 2009 Nicolai Haehnle.
 * Copyright 2010 Tom Stellard <tstellar@gmail.com>
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

#ifndef RADEON_DATAFLOW_H
#define RADEON_DATAFLOW_H

#include "radeon_program_constants.h"

struct radeon_compiler;
struct rc_instruction;
struct rc_swizzle_caps;
struct rc_src_register;
struct rc_pair_instruction_arg;
struct rc_pair_instruction_source;
struct rc_pair_sub_instruction;
struct rc_compiler;


/**
 * Help analyze and modify the register accesses of instructions.
 */
/*@{*/
typedef void (*rc_read_write_chan_fn)(void * userdata, struct rc_instruction * inst,
			rc_register_file file, unsigned int index, unsigned int chan);
void rc_for_all_reads_chan(struct rc_instruction * inst, rc_read_write_chan_fn cb, void * userdata);
void rc_for_all_writes_chan(struct rc_instruction * inst, rc_read_write_chan_fn cb, void * userdata);

typedef void (*rc_read_write_mask_fn)(void * userdata, struct rc_instruction * inst,
			rc_register_file file, unsigned int index, unsigned int mask);
void rc_for_all_reads_mask(struct rc_instruction * inst, rc_read_write_mask_fn cb, void * userdata);
void rc_for_all_writes_mask(struct rc_instruction * inst, rc_read_write_mask_fn cb, void * userdata);

typedef void (*rc_read_src_fn)(void * userdata, struct rc_instruction * inst,
			struct rc_src_register * src);
void rc_for_all_reads_src(struct rc_instruction * inst, rc_read_src_fn cb,
			void * userdata);

typedef void (*rc_pair_read_arg_fn)(void * userdata,
	struct rc_instruction * inst, struct rc_pair_instruction_arg * arg,
	struct rc_pair_instruction_source * src);
void rc_pair_for_all_reads_arg(struct rc_instruction * inst,
					rc_pair_read_arg_fn cb, void * userdata);

typedef void (*rc_remap_register_fn)(void * userdata, struct rc_instruction * inst,
			rc_register_file * pfile, unsigned int * pindex);
void rc_remap_registers(struct rc_instruction * inst, rc_remap_register_fn cb, void * userdata);
/*@}*/

struct rc_reader {
	struct rc_instruction * Inst;
	unsigned int WriteMask;
	union {
		struct {
			struct rc_src_register * Src;
		} I;
		struct {
			struct rc_pair_instruction_arg * Arg;
			struct rc_pair_instruction_source * Src;
		} P;
	} U;
};

struct rc_reader_data {
	struct radeon_compiler * C;

	unsigned int Abort;
	unsigned int AbortOnRead;
	unsigned int AbortOnWrite;
	unsigned int LoopDepth;
	unsigned int InElse;
	struct rc_instruction * Writer;

	unsigned int ReaderCount;
	unsigned int ReadersReserved;
	struct rc_reader * Readers;

	/* If this flag is enabled, rc_get_readers will exit as soon possible
	 * after the Abort flag is set.*/
	unsigned int ExitOnAbort;
	void * CbData;
};

void rc_get_readers(
	struct radeon_compiler * c,
	struct rc_instruction * writer,
	struct rc_reader_data * data,
	rc_read_src_fn read_normal_cb,
	rc_pair_read_arg_fn read_pair_cb,
	rc_read_write_mask_fn write_cb);

void rc_get_readers_sub(
	struct radeon_compiler * c,
	struct rc_instruction * writer,
	struct rc_pair_sub_instruction * sub_writer,
	struct rc_reader_data * data,
	rc_read_src_fn read_normal_cb,
	rc_pair_read_arg_fn read_pair_cb,
	rc_read_write_mask_fn write_cb);
/**
 * Compiler passes based on dataflow analysis.
 */
/*@{*/
typedef void (*rc_dataflow_mark_outputs_fn)(void * userdata, void * data,
			void (*mark_fn)(void * data, unsigned int index, unsigned int mask));
void rc_dataflow_deadcode(struct radeon_compiler * c, void *user);
void rc_dataflow_swizzles(struct radeon_compiler * c, void *user);
/*@}*/

void rc_optimize(struct radeon_compiler * c, void *user);
void rc_inline_literals(struct radeon_compiler *c, void *user);

#endif /* RADEON_DATAFLOW_H */
