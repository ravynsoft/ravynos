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

#include "radeon_dataflow.h"

#include "radeon_compiler.h"
#include "radeon_compiler_util.h"
#include "radeon_program.h"

struct read_write_mask_data {
	void * UserData;
	rc_read_write_mask_fn Cb;
};

static void reads_normal_callback(
	void * userdata,
	struct rc_instruction * fullinst,
	struct rc_src_register * src)
{
	struct read_write_mask_data * cb_data = userdata;
	unsigned int refmask = 0;
	unsigned int chan;
	for(chan = 0; chan < 4; chan++) {
		refmask |= 1 << GET_SWZ(src->Swizzle, chan);
	}
	refmask &= RC_MASK_XYZW;

	if (refmask) {
		cb_data->Cb(cb_data->UserData, fullinst, src->File,
							src->Index, refmask);
	}

	if (refmask && src->RelAddr) {
		cb_data->Cb(cb_data->UserData, fullinst, RC_FILE_ADDRESS, 0,
								RC_MASK_X);
	}
}

static void pair_get_src_refmasks(unsigned int * refmasks,
					struct rc_pair_instruction * inst,
					unsigned int swz, unsigned int src)
{
	if (swz >= 4)
		return;

	if (swz == RC_SWIZZLE_X || swz == RC_SWIZZLE_Y || swz == RC_SWIZZLE_Z) {
		if(src == RC_PAIR_PRESUB_SRC) {
			unsigned int i;
			int srcp_regs =
				rc_presubtract_src_reg_count(
				inst->RGB.Src[src].Index);
			for(i = 0; i < srcp_regs; i++) {
				refmasks[i] |= 1 << swz;
			}
		}
		else {
			refmasks[src] |= 1 << swz;
		}
	}

	if (swz == RC_SWIZZLE_W) {
		if (src == RC_PAIR_PRESUB_SRC) {
			unsigned int i;
			int srcp_regs = rc_presubtract_src_reg_count(
					inst->Alpha.Src[src].Index);
			for(i = 0; i < srcp_regs; i++) {
				refmasks[i] |= 1 << swz;
			}
		}
		else {
			refmasks[src] |= 1 << swz;
		}
	}
}

static void reads_pair(struct rc_instruction * fullinst, rc_read_write_mask_fn cb, void * userdata)
{
	struct rc_pair_instruction * inst = &fullinst->U.P;
	unsigned int refmasks[3] = { 0, 0, 0 };

	unsigned int arg;

	for(arg = 0; arg < 3; ++arg) {
		unsigned int chan;
		for(chan = 0; chan < 3; ++chan) {
			unsigned int swz_rgb =
				GET_SWZ(inst->RGB.Arg[arg].Swizzle, chan);
			unsigned int swz_alpha =
				GET_SWZ(inst->Alpha.Arg[arg].Swizzle, chan);
			pair_get_src_refmasks(refmasks, inst, swz_rgb,
						inst->RGB.Arg[arg].Source);
			pair_get_src_refmasks(refmasks, inst, swz_alpha,
						inst->Alpha.Arg[arg].Source);
		}
	}

	for(unsigned int src = 0; src < 3; ++src) {
		if (inst->RGB.Src[src].Used && (refmasks[src] & RC_MASK_XYZ))
			cb(userdata, fullinst, inst->RGB.Src[src].File, inst->RGB.Src[src].Index,
			   refmasks[src] & RC_MASK_XYZ);

		if (inst->Alpha.Src[src].Used && (refmasks[src] & RC_MASK_W))
			cb(userdata, fullinst, inst->Alpha.Src[src].File, inst->Alpha.Src[src].Index, RC_MASK_W);
	}
}

static void pair_sub_for_all_args(
	struct rc_instruction * fullinst,
	struct rc_pair_sub_instruction * sub,
	rc_pair_read_arg_fn cb,
	void * userdata)
{
	int i;
	const struct rc_opcode_info * info = rc_get_opcode_info(sub->Opcode);

	for(i = 0; i < info->NumSrcRegs; i++) {
		unsigned int src_type;

		src_type = rc_source_type_swz(sub->Arg[i].Swizzle);

		if (src_type == RC_SOURCE_NONE)
			continue;

		if (sub->Arg[i].Source == RC_PAIR_PRESUB_SRC) {
			unsigned int presub_type;
			unsigned int presub_src_count;
			struct rc_pair_instruction_source * src_array;
			unsigned int j;

			if (src_type & RC_SOURCE_RGB) {
				presub_type = fullinst->
					U.P.RGB.Src[RC_PAIR_PRESUB_SRC].Index;
				src_array = fullinst->U.P.RGB.Src;
			} else {
				presub_type = fullinst->
					U.P.Alpha.Src[RC_PAIR_PRESUB_SRC].Index;
				src_array = fullinst->U.P.Alpha.Src;
			}
			presub_src_count
				= rc_presubtract_src_reg_count(presub_type);
			for(j = 0; j < presub_src_count; j++) {
				cb(userdata, fullinst, &sub->Arg[i],
								&src_array[j]);
			}
		} else {
			struct rc_pair_instruction_source * src =
				rc_pair_get_src(&fullinst->U.P, &sub->Arg[i]);
			if (src) {
				cb(userdata, fullinst, &sub->Arg[i], src);
			}
		}
	}
}

/* This function calls the callback function (cb) for each source used by
 * the instruction.
 * */
void rc_for_all_reads_src(
	struct rc_instruction * inst,
	rc_read_src_fn cb,
	void * userdata)
{
	const struct rc_opcode_info * opcode =
					rc_get_opcode_info(inst->U.I.Opcode);

	/* This function only works with normal instructions. */
	if (inst->Type != RC_INSTRUCTION_NORMAL) {
		assert(0);
		return;
	}

	for(unsigned int src = 0; src < opcode->NumSrcRegs; ++src) {

		if (inst->U.I.SrcReg[src].File == RC_FILE_PRESUB) {
			unsigned int i;
			unsigned int srcp_regs = rc_presubtract_src_reg_count(
						inst->U.I.PreSub.Opcode);
			for( i = 0; i < srcp_regs; i++) {
				cb(userdata, inst, &inst->U.I.PreSub.SrcReg[i]);
			}
		} else {
			cb(userdata, inst, &inst->U.I.SrcReg[src]);
		}
	}
}

/**
 * This function calls the callback function (cb) for each arg of the RGB and
 * alpha components.
 */
void rc_pair_for_all_reads_arg(
	struct rc_instruction * inst,
	rc_pair_read_arg_fn cb,
	void * userdata)
{
	/* This function only works with pair instructions. */
	if (inst->Type != RC_INSTRUCTION_PAIR) {
		assert(0);
		return;
	}

	pair_sub_for_all_args(inst, &inst->U.P.RGB, cb, userdata);
	pair_sub_for_all_args(inst, &inst->U.P.Alpha, cb, userdata);
}

/**
 * Calls a callback function for all register reads.
 *
 * This is conservative, i.e. if the same register is referenced multiple times,
 * the callback may also be called multiple times.
 * Also, the writemask of the instruction is not taken into account.
 */
void rc_for_all_reads_mask(struct rc_instruction * inst, rc_read_write_mask_fn cb, void * userdata)
{
	if (inst->Type == RC_INSTRUCTION_NORMAL) {
		struct read_write_mask_data cb_data;
		cb_data.UserData = userdata;
		cb_data.Cb = cb;

		rc_for_all_reads_src(inst, reads_normal_callback, &cb_data);
	} else {
		reads_pair(inst, cb, userdata);
	}
}



static void writes_normal(struct rc_instruction * fullinst, rc_read_write_mask_fn cb, void * userdata)
{
	struct rc_sub_instruction * inst = &fullinst->U.I;
	const struct rc_opcode_info * opcode = rc_get_opcode_info(inst->Opcode);

	if (opcode->HasDstReg && inst->DstReg.WriteMask)
		cb(userdata, fullinst, inst->DstReg.File, inst->DstReg.Index, inst->DstReg.WriteMask);

	if (inst->WriteALUResult)
		cb(userdata, fullinst, RC_FILE_SPECIAL, RC_SPECIAL_ALU_RESULT, RC_MASK_X);
}

static void writes_pair(struct rc_instruction * fullinst, rc_read_write_mask_fn cb, void * userdata)
{
	struct rc_pair_instruction * inst = &fullinst->U.P;

	if (inst->RGB.WriteMask)
		cb(userdata, fullinst, RC_FILE_TEMPORARY, inst->RGB.DestIndex, inst->RGB.WriteMask);

	if (inst->Alpha.WriteMask)
		cb(userdata, fullinst, RC_FILE_TEMPORARY, inst->Alpha.DestIndex, RC_MASK_W);

	if (inst->WriteALUResult)
		cb(userdata, fullinst, RC_FILE_SPECIAL, RC_SPECIAL_ALU_RESULT, RC_MASK_X);
}

/**
 * Calls a callback function for all register writes in the instruction,
 * reporting writemasks to the callback function.
 *
 * \warning Does not report output registers for paired instructions!
 */
void rc_for_all_writes_mask(struct rc_instruction * inst, rc_read_write_mask_fn cb, void * userdata)
{
	if (inst->Type == RC_INSTRUCTION_NORMAL) {
		writes_normal(inst, cb, userdata);
	} else {
		writes_pair(inst, cb, userdata);
	}
}


struct mask_to_chan_data {
	void * UserData;
	rc_read_write_chan_fn Fn;
};

static void mask_to_chan_cb(void * data, struct rc_instruction * inst,
		rc_register_file file, unsigned int index, unsigned int mask)
{
	struct mask_to_chan_data * d = data;
	for(unsigned int chan = 0; chan < 4; ++chan) {
		if (GET_BIT(mask, chan))
			d->Fn(d->UserData, inst, file, index, chan);
	}
}

/**
 * Calls a callback function for all sourced register channels.
 *
 * This is conservative, i.e. channels may be called multiple times,
 * and the writemask of the instruction is not taken into account.
 */
void rc_for_all_reads_chan(struct rc_instruction * inst, rc_read_write_chan_fn cb, void * userdata)
{
	struct mask_to_chan_data d;
	d.UserData = userdata;
	d.Fn = cb;
	rc_for_all_reads_mask(inst, &mask_to_chan_cb, &d);
}

/**
 * Calls a callback function for all written register channels.
 *
 * \warning Does not report output registers for paired instructions!
 */
void rc_for_all_writes_chan(struct rc_instruction * inst, rc_read_write_chan_fn cb, void * userdata)
{
	struct mask_to_chan_data d;
	d.UserData = userdata;
	d.Fn = cb;
	rc_for_all_writes_mask(inst, &mask_to_chan_cb, &d);
}

static void remap_normal_instruction(struct rc_instruction * fullinst,
		rc_remap_register_fn cb, void * userdata)
{
	struct rc_sub_instruction * inst = &fullinst->U.I;
	const struct rc_opcode_info * opcode = rc_get_opcode_info(inst->Opcode);
	unsigned int remapped_presub = 0;

	if (opcode->HasDstReg) {
		rc_register_file file = inst->DstReg.File;
		unsigned int index = inst->DstReg.Index;

		cb(userdata, fullinst, &file, &index);

		inst->DstReg.File = file;
		inst->DstReg.Index = index;
	}

	for(unsigned int src = 0; src < opcode->NumSrcRegs; ++src) {
		rc_register_file file = inst->SrcReg[src].File;
		unsigned int index = inst->SrcReg[src].Index;

		if (file == RC_FILE_PRESUB) {
			unsigned int i;
			unsigned int srcp_srcs = rc_presubtract_src_reg_count(
						inst->PreSub.Opcode);
			/* Make sure we only remap presubtract sources once in
			 * case more than one source register reads the
			 * presubtract result. */
			if (remapped_presub)
				continue;

			for(i = 0; i < srcp_srcs; i++) {
				file = inst->PreSub.SrcReg[i].File;
				index = inst->PreSub.SrcReg[i].Index;
				cb(userdata, fullinst, &file, &index);
				inst->PreSub.SrcReg[i].File = file;
				inst->PreSub.SrcReg[i].Index = index;
			}
			remapped_presub = 1;
		}
		else {
			cb(userdata, fullinst, &file, &index);

			inst->SrcReg[src].File = file;
			inst->SrcReg[src].Index = index;
		}
	}
}

static void remap_pair_instruction(struct rc_instruction * fullinst,
		rc_remap_register_fn cb, void * userdata)
{
	struct rc_pair_instruction * inst = &fullinst->U.P;

	if (inst->RGB.WriteMask) {
		rc_register_file file = RC_FILE_TEMPORARY;
		unsigned int index = inst->RGB.DestIndex;

		cb(userdata, fullinst, &file, &index);

		inst->RGB.DestIndex = index;
	}

	if (inst->Alpha.WriteMask) {
		rc_register_file file = RC_FILE_TEMPORARY;
		unsigned int index = inst->Alpha.DestIndex;

		cb(userdata, fullinst, &file, &index);

		inst->Alpha.DestIndex = index;
	}

	for(unsigned int src = 0; src < 3; ++src) {
		if (inst->RGB.Src[src].Used) {
			rc_register_file file = inst->RGB.Src[src].File;
			unsigned int index = inst->RGB.Src[src].Index;

			cb(userdata, fullinst, &file, &index);

			inst->RGB.Src[src].File = file;
			inst->RGB.Src[src].Index = index;
		}

		if (inst->Alpha.Src[src].Used) {
			rc_register_file file = inst->Alpha.Src[src].File;
			unsigned int index = inst->Alpha.Src[src].Index;

			cb(userdata, fullinst, &file, &index);

			inst->Alpha.Src[src].File = file;
			inst->Alpha.Src[src].Index = index;
		}
	}
}


/**
 * Remap all register accesses according to the given function.
 * That is, call the function \p cb for each referenced register (both read and written)
 * and update the given instruction \p inst accordingly
 * if it modifies its \ref pfile and \ref pindex contents.
 */
void rc_remap_registers(struct rc_instruction * inst, rc_remap_register_fn cb, void * userdata)
{
	if (inst->Type == RC_INSTRUCTION_NORMAL)
		remap_normal_instruction(inst, cb, userdata);
	else
		remap_pair_instruction(inst, cb, userdata);
}

struct branch_write_mask {
	unsigned int IfWriteMask:4;
	unsigned int ElseWriteMask:4;
	unsigned int HasElse:1;
};

union get_readers_read_cb {
	rc_read_src_fn I;
	rc_pair_read_arg_fn P;
};

struct get_readers_callback_data {
	struct radeon_compiler * C;
	struct rc_reader_data * ReaderData;
	rc_read_src_fn ReadNormalCB;
	rc_pair_read_arg_fn ReadPairCB;
	rc_read_write_mask_fn WriteCB;
	rc_register_file DstFile;
	unsigned int DstIndex;
	unsigned int DstMask;
	unsigned int AliveWriteMask;
	/*  For convenience, this is indexed starting at 1 */
	struct branch_write_mask BranchMasks[R500_PFS_MAX_BRANCH_DEPTH_FULL + 1];
};

static struct rc_reader * add_reader(
	struct memory_pool * pool,
	struct rc_reader_data * data,
	struct rc_instruction * inst,
	unsigned int mask)
{
	struct rc_reader * new;
	memory_pool_array_reserve(pool, struct rc_reader, data->Readers,
				data->ReaderCount, data->ReadersReserved, 1);
	new = &data->Readers[data->ReaderCount++];
	new->Inst = inst;
	new->WriteMask = mask;
	return new;
}

static void add_reader_normal(
	struct memory_pool * pool,
	struct rc_reader_data * data,
	struct rc_instruction * inst,
	unsigned int mask,
	struct rc_src_register * src)
{
	struct rc_reader * new = add_reader(pool, data, inst, mask);
	new->U.I.Src = src;
}


static void add_reader_pair(
	struct memory_pool * pool,
	struct rc_reader_data * data,
	struct rc_instruction * inst,
	unsigned int mask,
	struct rc_pair_instruction_arg * arg,
	struct rc_pair_instruction_source * src)
{
	struct rc_reader * new = add_reader(pool, data, inst, mask);
	new->U.P.Src = src;
	new->U.P.Arg = arg;
}

static unsigned int get_readers_read_callback(
	struct get_readers_callback_data * cb_data,
	rc_register_file file,
	unsigned int index,
	unsigned int swizzle)
{
	unsigned int shared_mask, read_mask;

	shared_mask = rc_src_reads_dst_mask(file, index, swizzle,
		cb_data->DstFile, cb_data->DstIndex, cb_data->AliveWriteMask);

	if (shared_mask == RC_MASK_NONE)
		return shared_mask;

	/* If we make it this far, it means that this source reads from the
	 * same register written to by d->ReaderData->Writer. */

	read_mask = rc_swizzle_to_writemask(swizzle);
	if (cb_data->ReaderData->AbortOnRead & read_mask) {
		cb_data->ReaderData->Abort = 1;
		return shared_mask;
	}

	if (cb_data->ReaderData->LoopDepth > 0) {
		cb_data->ReaderData->AbortOnWrite |=
				(read_mask & cb_data->AliveWriteMask);
	}

	/* XXX The behavior in this case should be configurable. */
	if ((read_mask & cb_data->AliveWriteMask) != read_mask) {
		cb_data->ReaderData->Abort = 1;
		return shared_mask;
	}

	return shared_mask;
}

static void get_readers_pair_read_callback(
	void * userdata,
	struct rc_instruction * inst,
	struct rc_pair_instruction_arg * arg,
	struct rc_pair_instruction_source * src)
{
	unsigned int shared_mask;
	struct get_readers_callback_data * d = userdata;

	shared_mask = get_readers_read_callback(d,
				src->File, src->Index, arg->Swizzle);

	if (shared_mask == RC_MASK_NONE)
		return;

	if (d->ReadPairCB)
		d->ReadPairCB(d->ReaderData, inst, arg, src);

	if (d->ReaderData->ExitOnAbort && d->ReaderData->Abort)
		return;

	add_reader_pair(&d->C->Pool, d->ReaderData, inst, shared_mask, arg, src);
}

/**
 * This function is used by rc_get_readers_normal() to determine whether inst
 * is a reader of userdata->ReaderData->Writer
 */
static void get_readers_normal_read_callback(
	void * userdata,
	struct rc_instruction * inst,
	struct rc_src_register * src)
{
	struct get_readers_callback_data * d = userdata;
	unsigned int shared_mask;

	shared_mask = get_readers_read_callback(d,
			src->File, src->Index, src->Swizzle);

	if (shared_mask == RC_MASK_NONE)
		return;
	/* The callback function could potentially clear d->ReaderData->Abort,
	 * so we need to call it before we return. */
	if (d->ReadNormalCB)
		d->ReadNormalCB(d->ReaderData, inst, src);

	if (d->ReaderData->ExitOnAbort && d->ReaderData->Abort)
		return;

	add_reader_normal(&d->C->Pool, d->ReaderData, inst, shared_mask, src);
}

/**
 * This function is used by rc_get_readers_normal() to determine when
 * userdata->ReaderData->Writer is dead (i. e. All components of its
 * destination register have been overwritten by other instructions).
 */
static void get_readers_write_callback(
	void *userdata,
	struct rc_instruction * inst,
	rc_register_file file,
	unsigned int index,
	unsigned int mask)
{
	struct get_readers_callback_data * d = userdata;

	if (index == d->DstIndex && file == d->DstFile) {
		unsigned int shared_mask = mask & d->DstMask;
		d->ReaderData->AbortOnRead &= ~shared_mask;
		d->AliveWriteMask &= ~shared_mask;
		if (d->ReaderData->AbortOnWrite & shared_mask) {
			d->ReaderData->Abort = 1;
		}
	}

	if(d->WriteCB)
		d->WriteCB(d->ReaderData, inst, file, index, mask);
}

static void push_branch_mask(
	struct get_readers_callback_data * d,
	unsigned int * branch_depth)
{
	(*branch_depth)++;
	if (*branch_depth > R500_PFS_MAX_BRANCH_DEPTH_FULL) {
		d->ReaderData->Abort = 1;
		return;
	}
	d->BranchMasks[*branch_depth].IfWriteMask =
					d->AliveWriteMask;
}

static void pop_branch_mask(
	struct get_readers_callback_data * d,
	unsigned int * branch_depth)
{
	struct branch_write_mask * masks = &d->BranchMasks[*branch_depth];

	if (masks->HasElse) {
		/* Abort on read for components that were written in the IF
		 * block. */
		d->ReaderData->AbortOnRead |=
				masks->IfWriteMask & ~masks->ElseWriteMask;
		/* Abort on read for components that were written in the ELSE
		 * block. */
		d->ReaderData->AbortOnRead |=
				masks->ElseWriteMask & ~d->AliveWriteMask;

		d->AliveWriteMask = masks->IfWriteMask
			^ ((masks->IfWriteMask ^ masks->ElseWriteMask)
			& (masks->IfWriteMask ^ d->AliveWriteMask));
	} else {
		d->ReaderData->AbortOnRead |=
				masks->IfWriteMask & ~d->AliveWriteMask;
		d->AliveWriteMask = masks->IfWriteMask;

	}
	memset(masks, 0, sizeof(struct branch_write_mask));
	(*branch_depth)--;
}

static void get_readers_for_single_write(
	void * userdata,
	struct rc_instruction * writer,
	rc_register_file dst_file,
	unsigned int dst_index,
	unsigned int dst_mask)
{
	struct rc_instruction * tmp;
	unsigned int branch_depth = 0;
	struct rc_instruction * endloop = NULL;
	unsigned int abort_on_read_at_endloop = 0;
	unsigned int abort_on_read_at_break = 0;
	unsigned int alive_write_mask_at_breaks = 0;
	struct get_readers_callback_data * d = userdata;

	d->ReaderData->Writer = writer;
	d->ReaderData->AbortOnRead = 0;
	d->ReaderData->AbortOnWrite = 0;
	d->ReaderData->LoopDepth = 0;
	d->ReaderData->InElse = 0;
	d->DstFile = dst_file;
	d->DstIndex = dst_index;
	d->DstMask = dst_mask;
	d->AliveWriteMask = dst_mask;
	memset(d->BranchMasks, 0, sizeof(d->BranchMasks));

	if (!dst_mask)
		return;

	for(tmp = writer->Next; tmp != &d->C->Program.Instructions;
							tmp = tmp->Next){
		rc_opcode opcode = rc_get_flow_control_inst(tmp);
		switch(opcode) {
		case RC_OPCODE_BGNLOOP:
			d->ReaderData->LoopDepth++;
			push_branch_mask(d, &branch_depth);
			break;
		case RC_OPCODE_ENDLOOP:
			if (d->ReaderData->LoopDepth > 0) {
				d->ReaderData->LoopDepth--;
				if (d->ReaderData->LoopDepth == 0) {
					d->ReaderData->AbortOnWrite = 0;
				}
				pop_branch_mask(d, &branch_depth);
			} else {
				/* Here we have reached an ENDLOOP without
				 * seeing its BGNLOOP.  These means that
				 * the writer was written inside of a loop,
				 * so it could have readers that are above it
				 * (i.e. they have a lower IP).  To find these
				 * readers we jump to the BGNLOOP instruction
				 * and check each instruction until we get
				 * back to the writer.
				 */
				endloop = tmp;
				tmp = rc_match_endloop(tmp);
				if (!tmp) {
					rc_error(d->C, "Failed to match endloop.\n");
					d->ReaderData->Abort = 1;
					return;
				}
				abort_on_read_at_endloop = d->ReaderData->AbortOnRead;
				d->ReaderData->AbortOnRead |= d->AliveWriteMask;
				continue;
			}
			break;
		case RC_OPCODE_BRK:
			if (branch_depth == 0 && d->ReaderData->LoopDepth == 0) {
				tmp = rc_match_bgnloop(tmp);
				d->ReaderData->AbortOnRead = d->AliveWriteMask;
			} else {
				struct branch_write_mask * masks = &d->BranchMasks[branch_depth];
				alive_write_mask_at_breaks |= d->AliveWriteMask;
				if (masks->HasElse) {
					/* Abort on read for components that were written in the IF
					 * block. */
					abort_on_read_at_break |=
						masks->IfWriteMask & ~masks->ElseWriteMask;
					/* Abort on read for components that were written in the ELSE
					 * block. */
					abort_on_read_at_break |=
						masks->ElseWriteMask & ~d->AliveWriteMask;
				} else {
					abort_on_read_at_break |=
						masks->IfWriteMask & ~d->AliveWriteMask;
				}
			}
			break;
		case RC_OPCODE_IF:
			push_branch_mask(d, &branch_depth);
			break;
		case RC_OPCODE_ELSE:
			if (branch_depth == 0) {
				d->ReaderData->InElse = 1;
			} else {
				unsigned int temp_mask = d->AliveWriteMask;
				d->AliveWriteMask =
					d->BranchMasks[branch_depth].IfWriteMask;
				d->BranchMasks[branch_depth].ElseWriteMask =
								temp_mask;
				d->BranchMasks[branch_depth].HasElse = 1;
			}
			break;
		case RC_OPCODE_ENDIF:
			if (branch_depth == 0) {
				d->ReaderData->AbortOnRead = d->AliveWriteMask;
				d->ReaderData->InElse = 0;
			}
			else {
				pop_branch_mask(d, &branch_depth);
			}
			break;
		default:
			break;
		}

		if (d->ReaderData->InElse)
			continue;

		if (tmp->Type == RC_INSTRUCTION_NORMAL) {
			rc_for_all_reads_src(tmp,
				get_readers_normal_read_callback, d);
		} else {
			rc_pair_for_all_reads_arg(tmp,
				get_readers_pair_read_callback, d);
		}

		/* This can happen when we jump from an ENDLOOP to BGNLOOP */
		if (tmp == writer) {
			tmp = endloop;
			endloop = NULL;
			d->ReaderData->AbortOnRead = abort_on_read_at_endloop
							| abort_on_read_at_break;
			/* Restore the AliveWriteMask to account for all possible
			 * exits from the loop. */
			d->AliveWriteMask = alive_write_mask_at_breaks;
			alive_write_mask_at_breaks = 0;
			continue;
		}
		rc_for_all_writes_mask(tmp, get_readers_write_callback, d);

		if (d->ReaderData->ExitOnAbort && d->ReaderData->Abort)
			return;

		/* The check for !endloop in needed for the following scenario:
		 *
		 * 0 MOV TEMP[0] none.0
		 * 1 BGNLOOP
		 * 2   IF some exit condition
		 * 3      BRK
		 * 4   ENDIF
		 * 5 ADD TEMP[0], TEMP[0], CONST[0]
		 * 6 ADD TEMP[0], TEMP[0], none.1
		 * 7 ENDLOOP
		 * 8 MOV OUT[0] TEMP[0]
		 *
		 * When we search for the readers of instruction 6, we encounter the ENDLOOP
		 * and continue searching at BGNLOOP. At instruction 5 the AliveWriteMask
		 * becomes 0 and we would stop the search. However we still need to continue
		 * back to 6 from which we jump after the endloop, restore the AliveWriteMask
		 * according to the possible states at breaks and continue after the loop.
                 */
		if (branch_depth == 0 && !d->AliveWriteMask && !endloop)
			return;
	}
}

static void init_get_readers_callback_data(
	struct get_readers_callback_data * d,
	struct rc_reader_data * reader_data,
	struct radeon_compiler * c,
	rc_read_src_fn read_normal_cb,
	rc_pair_read_arg_fn read_pair_cb,
	rc_read_write_mask_fn write_cb)
{
	reader_data->C = c;
	reader_data->Abort = 0;
	reader_data->ReaderCount = 0;
	reader_data->ReadersReserved = 0;
	reader_data->Readers = NULL;

	d->C = c;
	d->ReaderData = reader_data;
	d->ReadNormalCB = read_normal_cb;
	d->ReadPairCB = read_pair_cb;
	d->WriteCB = write_cb;
}

/**
 * This function will create a list of readers via the rc_reader_data struct.
 * This function will abort (set the flag data->Abort) and return if it
 * encounters an instruction that reads from @param writer and also a different
 * instruction.  Here are some examples:
 *
 * writer = instruction 0;
 * 0 MOV TEMP[0].xy, TEMP[1].xy
 * 1 MOV TEMP[0].zw, TEMP[2].xy
 * 2 MOV TEMP[3], TEMP[0]
 * The Abort flag will be set on instruction 2, because it reads values written
 * by instructions 0 and 1.
 *
 * writer = instruction 1;
 * 0 IF TEMP[0].x
 * 1 MOV TEMP[1], TEMP[2]
 * 2 ELSE
 * 3 MOV TEMP[1], TEMP[2]
 * 4 ENDIF
 * 5 MOV TEMP[3], TEMP[1]
 * The Abort flag will be set on instruction 5, because it could read from the
 * value written by either instruction 1 or 3, depending on the jump decision
 * made at instruction 0.
 *
 * writer = instruction 0;
 * 0 MOV TEMP[0], TEMP[1]
 * 2 BGNLOOP
 * 3 ADD TEMP[0], TEMP[0], none.1
 * 4 ENDLOOP
 * The Abort flag will be set on instruction 3, because in the first iteration
 * of the loop it reads the value written by instruction 0 and in all other
 * iterations it reads the value written by instruction 3.
 *
 * @param read_cb This function will be called for every instruction that
 * has been determined to be a reader of writer.
 * @param write_cb This function will be called for every instruction after
 * writer.
 */
void rc_get_readers(
	struct radeon_compiler * c,
	struct rc_instruction * writer,
	struct rc_reader_data * data,
	rc_read_src_fn read_normal_cb,
	rc_pair_read_arg_fn read_pair_cb,
	rc_read_write_mask_fn write_cb)
{
	struct get_readers_callback_data d;

	init_get_readers_callback_data(&d, data, c, read_normal_cb,
						read_pair_cb, write_cb);

	rc_for_all_writes_mask(writer, get_readers_for_single_write, &d);
}

void rc_get_readers_sub(
	struct radeon_compiler * c,
	struct rc_instruction * writer,
	struct rc_pair_sub_instruction * sub_writer,
	struct rc_reader_data * data,
	rc_read_src_fn read_normal_cb,
	rc_pair_read_arg_fn read_pair_cb,
	rc_read_write_mask_fn write_cb)
{
	struct get_readers_callback_data d;

	init_get_readers_callback_data(&d, data, c, read_normal_cb,
						read_pair_cb, write_cb);

	if (sub_writer->WriteMask) {
		get_readers_for_single_write(&d, writer, RC_FILE_TEMPORARY,
			sub_writer->DestIndex, sub_writer->WriteMask);
	}
}
