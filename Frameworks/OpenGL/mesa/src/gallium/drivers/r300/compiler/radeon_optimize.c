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

#include "util/u_math.h"

#include "radeon_dataflow.h"

#include "radeon_compiler.h"
#include "radeon_compiler_util.h"
#include "radeon_list.h"
#include "radeon_swizzle.h"
#include "radeon_variable.h"

struct src_clobbered_reads_cb_data {
	rc_register_file File;
	unsigned int Index;
	unsigned int Mask;
	struct rc_reader_data * ReaderData;
};

typedef void (*rc_presub_replace_fn)(struct rc_instruction *,
						struct rc_instruction *,
						unsigned int);

static struct rc_src_register chain_srcregs(struct rc_src_register outer, struct rc_src_register inner)
{
	struct rc_src_register combine;
	combine.File = inner.File;
	combine.Index = inner.Index;
	combine.RelAddr = inner.RelAddr;
	if (outer.Abs) {
		combine.Abs = 1;
		combine.Negate = outer.Negate;
	} else {
		combine.Abs = inner.Abs;
		combine.Negate = swizzle_mask(outer.Swizzle, inner.Negate);
		combine.Negate ^= outer.Negate;
	}
	combine.Swizzle = combine_swizzles(inner.Swizzle, outer.Swizzle);
	return combine;
}

static void copy_propagate_scan_read(void * data, struct rc_instruction * inst,
						struct rc_src_register * src)
{
	rc_register_file file = src->File;
	struct rc_reader_data * reader_data = data;

	if(!rc_inst_can_use_presub(reader_data->C,
				inst,
				reader_data->Writer->U.I.PreSub.Opcode,
				rc_swizzle_to_writemask(src->Swizzle),
				src,
				&reader_data->Writer->U.I.PreSub.SrcReg[0],
				&reader_data->Writer->U.I.PreSub.SrcReg[1])) {
		reader_data->Abort = 1;
		return;
	}

	/* XXX This could probably be handled better. */
	if (file == RC_FILE_ADDRESS) {
		reader_data->Abort = 1;
		return;
	}

	/* R300/R400 is unhappy about propagating
	 *  0: MOV temp[1], -none.1111;
	 *  1: KIL temp[1];
	 * to
	 *  0: KIL -none.1111;
	 *
	 * R500 is fine with it.
	 */
	if (!reader_data->C->is_r500 && inst->U.I.Opcode == RC_OPCODE_KIL &&
		reader_data->Writer->U.I.SrcReg[0].File == RC_FILE_NONE) {
		reader_data->Abort = 1;
		return;
	}

	/* These instructions cannot read from the constants file.
	 * see radeonTransformTEX()
	 */
	if(reader_data->Writer->U.I.SrcReg[0].File != RC_FILE_TEMPORARY &&
			reader_data->Writer->U.I.SrcReg[0].File != RC_FILE_INPUT &&
			reader_data->Writer->U.I.SrcReg[0].File != RC_FILE_NONE &&
				(inst->U.I.Opcode == RC_OPCODE_TEX ||
				inst->U.I.Opcode == RC_OPCODE_TXB ||
				inst->U.I.Opcode == RC_OPCODE_TXP ||
				inst->U.I.Opcode == RC_OPCODE_TXD ||
				inst->U.I.Opcode == RC_OPCODE_TXL ||
				inst->U.I.Opcode == RC_OPCODE_KIL)){
		reader_data->Abort = 1;
		return;
	}
}

static void src_clobbered_reads_cb(
	void * data,
	struct rc_instruction * inst,
	struct rc_src_register * src)
{
	struct src_clobbered_reads_cb_data * sc_data = data;

	if (src->File == sc_data->File
	    && src->Index == sc_data->Index
	    && (rc_swizzle_to_writemask(src->Swizzle) & sc_data->Mask)) {

		sc_data->ReaderData->AbortOnRead = RC_MASK_XYZW;
	}

	if (src->RelAddr && sc_data->File == RC_FILE_ADDRESS) {
		sc_data->ReaderData->AbortOnRead = RC_MASK_XYZW;
	}
}

static void is_src_clobbered_scan_write(
	void * data,
	struct rc_instruction * inst,
	rc_register_file file,
	unsigned int index,
	unsigned int mask)
{
	struct src_clobbered_reads_cb_data sc_data;
	struct rc_reader_data * reader_data = data;
	sc_data.File = file;
	sc_data.Index = index;
	sc_data.Mask = mask;
	sc_data.ReaderData = reader_data;
	rc_for_all_reads_src(reader_data->Writer,
					src_clobbered_reads_cb, &sc_data);
}

static void copy_propagate(struct radeon_compiler * c, struct rc_instruction * inst_mov)
{
	struct rc_reader_data reader_data;
	unsigned int i;

	if (inst_mov->U.I.DstReg.File != RC_FILE_TEMPORARY ||
	    inst_mov->U.I.WriteALUResult)
		return;

	/* Get a list of all the readers of this MOV instruction. */
	reader_data.ExitOnAbort = 1;
	rc_get_readers(c, inst_mov, &reader_data,
		       copy_propagate_scan_read, NULL,
		       is_src_clobbered_scan_write);

	if (reader_data.Abort || reader_data.ReaderCount == 0)
		return;

	/* We can propagate SaturateMode if all the readers are MOV instructions
	 * without a presubtract operation, source negation and absolute.
	 * In that case, we just move SaturateMode to all readers. */
        if (inst_mov->U.I.SaturateMode) {
		for (i = 0; i < reader_data.ReaderCount; i++) {
			struct rc_instruction * inst = reader_data.Readers[i].Inst;

			if (inst->U.I.Opcode != RC_OPCODE_MOV ||
			    inst->U.I.SrcReg[0].File == RC_FILE_PRESUB ||
			    inst->U.I.SrcReg[0].Abs ||
			    inst->U.I.SrcReg[0].Negate) {
				return;
			}
		}
	}

	/* Propagate the MOV instruction. */
	for (i = 0; i < reader_data.ReaderCount; i++) {
		struct rc_instruction * inst = reader_data.Readers[i].Inst;
		*reader_data.Readers[i].U.I.Src = chain_srcregs(*reader_data.Readers[i].U.I.Src, inst_mov->U.I.SrcReg[0]);

		if (inst_mov->U.I.SrcReg[0].File == RC_FILE_PRESUB)
			inst->U.I.PreSub = inst_mov->U.I.PreSub;
		if (!inst->U.I.SaturateMode)
			inst->U.I.SaturateMode = inst_mov->U.I.SaturateMode;
	}

	/* Finally, remove the original MOV instruction */
	rc_remove_instruction(inst_mov);
}

/**
 * Check if a source register is actually always the same
 * swizzle constant.
 */
static int is_src_uniform_constant(struct rc_src_register src,
		rc_swizzle * pswz, unsigned int * pnegate)
{
	int have_used = 0;

	if (src.File != RC_FILE_NONE) {
		*pswz = 0;
		return 0;
	}

	for(unsigned int chan = 0; chan < 4; ++chan) {
		unsigned int swz = GET_SWZ(src.Swizzle, chan);
		if (swz < 4) {
			*pswz = 0;
			return 0;
		}
		if (swz == RC_SWIZZLE_UNUSED)
			continue;

		if (!have_used) {
			*pswz = swz;
			*pnegate = GET_BIT(src.Negate, chan);
			have_used = 1;
		} else {
			if (swz != *pswz || *pnegate != GET_BIT(src.Negate, chan)) {
				*pswz = 0;
				return 0;
			}
		}
	}

	return 1;
}

static void constant_folding_add(struct rc_instruction * inst)
{
	rc_swizzle swz = 0;
	unsigned int negate = 0;

	if (is_src_uniform_constant(inst->U.I.SrcReg[0], &swz, &negate)) {
		if (swz == RC_SWIZZLE_ZERO) {
			inst->U.I.Opcode = RC_OPCODE_MOV;
			inst->U.I.SrcReg[0] = inst->U.I.SrcReg[1];
			return;
		}
	}

	if (is_src_uniform_constant(inst->U.I.SrcReg[1], &swz, &negate)) {
		if (swz == RC_SWIZZLE_ZERO) {
			inst->U.I.Opcode = RC_OPCODE_MOV;
			return;
		}
	}
}

/**
 * Replace 0.0, 1.0 and 0.5 immediate constants by their
 * respective swizzles. Simplify instructions like ADD dst, src, 0;
 */
static void constant_folding(struct radeon_compiler * c, struct rc_instruction * inst)
{
	const struct rc_opcode_info * opcode = rc_get_opcode_info(inst->U.I.Opcode);
	unsigned int i;

	/* Replace 0.0, 1.0 and 0.5 immediates by their explicit swizzles */
	for(unsigned int src = 0; src < opcode->NumSrcRegs; ++src) {
		struct rc_constant * constant;
		struct rc_src_register newsrc;
		int have_real_reference;
		unsigned int chan;

		/* If there are only 0, 0.5, 1, or _ swizzles, mark the source as a constant. */
		for (chan = 0; chan < 4; ++chan)
			if (GET_SWZ(inst->U.I.SrcReg[src].Swizzle, chan) <= 3)
				break;
		if (chan == 4) {
			inst->U.I.SrcReg[src].File = RC_FILE_NONE;
			continue;
		}

		/* Convert immediates to swizzles. */
		if (inst->U.I.SrcReg[src].File != RC_FILE_CONSTANT ||
		    inst->U.I.SrcReg[src].RelAddr ||
		    inst->U.I.SrcReg[src].Index >= c->Program.Constants.Count)
			continue;

		constant =
			&c->Program.Constants.Constants[inst->U.I.SrcReg[src].Index];

		if (constant->Type != RC_CONSTANT_IMMEDIATE)
			continue;

		newsrc = inst->U.I.SrcReg[src];
		have_real_reference = 0;
		for (chan = 0; chan < 4; ++chan) {
			unsigned int swz = GET_SWZ(newsrc.Swizzle, chan);
			unsigned int newswz;
			float imm;
			float baseimm;

			if (swz >= 4)
				continue;

			imm = constant->u.Immediate[swz];
			baseimm = imm;
			if (imm < 0.0)
				baseimm = -baseimm;

			if (baseimm == 0.0) {
				newswz = RC_SWIZZLE_ZERO;
			} else if (baseimm == 1.0) {
				newswz = RC_SWIZZLE_ONE;
			} else if (baseimm == 0.5 && c->has_half_swizzles) {
				newswz = RC_SWIZZLE_HALF;
			} else {
				have_real_reference = 1;
				continue;
			}

			SET_SWZ(newsrc.Swizzle, chan, newswz);
			if (imm < 0.0 && !newsrc.Abs)
				newsrc.Negate ^= 1 << chan;
		}

		if (!have_real_reference) {
			newsrc.File = RC_FILE_NONE;
			newsrc.Index = 0;
		}

		/* don't make the swizzle worse */
		if (!c->SwizzleCaps->IsNative(inst->U.I.Opcode, newsrc))
			continue;

		inst->U.I.SrcReg[src] = newsrc;
	}

	if (c->type == RC_FRAGMENT_PROGRAM &&
		inst->U.I.Opcode == RC_OPCODE_ADD)
		constant_folding_add(inst);

	/* In case this instruction has been converted, make sure all of the
	 * registers that are no longer used are empty. */
	opcode = rc_get_opcode_info(inst->U.I.Opcode);
	for(i = opcode->NumSrcRegs; i < 3; i++) {
		memset(&inst->U.I.SrcReg[i], 0, sizeof(struct rc_src_register));
	}
}

/**
 * If src and dst use the same register, this function returns a writemask that
 * indicates which components are read by src.  Otherwise zero is returned.
 */
static unsigned int src_reads_dst_mask(struct rc_src_register src,
						struct rc_dst_register dst)
{
	if (dst.File != src.File || dst.Index != src.Index) {
		return 0;
	}
	return rc_swizzle_to_writemask(src.Swizzle);
}

/* Return 1 if the source registers has a constant swizzle (e.g. 0, 0.5, 1.0)
 * in any of its channels.  Return 0 otherwise. */
static int src_has_const_swz(struct rc_src_register src) {
	int chan;
	for(chan = 0; chan < 4; chan++) {
		unsigned int swz = GET_SWZ(src.Swizzle, chan);
		if (swz == RC_SWIZZLE_ZERO || swz == RC_SWIZZLE_HALF
						|| swz == RC_SWIZZLE_ONE) {
			return 1;
		}
	}
	return 0;
}

static void presub_scan_read(
	void * data,
	struct rc_instruction * inst,
	struct rc_src_register * src)
{
	struct rc_reader_data * reader_data = data;
	rc_presubtract_op * presub_opcode = reader_data->CbData;

	if (!rc_inst_can_use_presub(reader_data->C,
			inst,
			*presub_opcode,
			reader_data->Writer->U.I.DstReg.WriteMask,
			src,
			&reader_data->Writer->U.I.SrcReg[0],
			&reader_data->Writer->U.I.SrcReg[1])) {
		reader_data->Abort = 1;
		return;
	}
}

static int presub_helper(
	struct radeon_compiler * c,
	struct rc_instruction * inst_add,
	rc_presubtract_op presub_opcode,
	rc_presub_replace_fn presub_replace)
{
	struct rc_reader_data reader_data;
	unsigned int i;
	rc_presubtract_op cb_op = presub_opcode;

	reader_data.CbData = &cb_op;
	reader_data.ExitOnAbort = 1;
	rc_get_readers(c, inst_add, &reader_data, presub_scan_read, NULL,
						is_src_clobbered_scan_write);

	if (reader_data.Abort || reader_data.ReaderCount == 0)
		return 0;

	for(i = 0; i < reader_data.ReaderCount; i++) {
		unsigned int src_index;
		struct rc_reader reader = reader_data.Readers[i];
		const struct rc_opcode_info * info =
				rc_get_opcode_info(reader.Inst->U.I.Opcode);

		for (src_index = 0; src_index < info->NumSrcRegs; src_index++) {
			if (&reader.Inst->U.I.SrcReg[src_index] == reader.U.I.Src)
				presub_replace(inst_add, reader.Inst, src_index);
		}
	}
	return 1;
}

static void presub_replace_add(
	struct rc_instruction * inst_add,
	struct rc_instruction * inst_reader,
	unsigned int src_index)
{
	rc_presubtract_op presub_opcode;

	unsigned int negates = 0;
	if (inst_add->U.I.SrcReg[0].Negate)
		negates++;
	if (inst_add->U.I.SrcReg[1].Negate)
		negates++;
	assert(negates != 2 || inst_add->U.I.SrcReg[1].Negate == inst_add->U.I.SrcReg[0].Negate);

	if (negates == 1)
		presub_opcode = RC_PRESUB_SUB;
	else
		presub_opcode = RC_PRESUB_ADD;

	if (inst_add->U.I.SrcReg[1].Negate && negates == 1) {
		inst_reader->U.I.PreSub.SrcReg[0] = inst_add->U.I.SrcReg[1];
		inst_reader->U.I.PreSub.SrcReg[1] = inst_add->U.I.SrcReg[0];
	} else {
		inst_reader->U.I.PreSub.SrcReg[0] = inst_add->U.I.SrcReg[0];
		inst_reader->U.I.PreSub.SrcReg[1] = inst_add->U.I.SrcReg[1];
	}
	/* If both sources are negative we can move the negate to the presub. */
	unsigned negate_mask = negates == 1 ? 0 : inst_add->U.I.SrcReg[0].Negate;
	inst_reader->U.I.PreSub.SrcReg[0].Negate = negate_mask;
	inst_reader->U.I.PreSub.SrcReg[1].Negate = negate_mask;
	inst_reader->U.I.PreSub.Opcode = presub_opcode;
	inst_reader->U.I.SrcReg[src_index] =
			chain_srcregs(inst_reader->U.I.SrcReg[src_index],
					inst_reader->U.I.PreSub.SrcReg[0]);
	inst_reader->U.I.SrcReg[src_index].File = RC_FILE_PRESUB;
	inst_reader->U.I.SrcReg[src_index].Index = presub_opcode;
}

static int is_presub_candidate(
	struct radeon_compiler * c,
	struct rc_instruction * inst)
{
	const struct rc_opcode_info * info = rc_get_opcode_info(inst->U.I.Opcode);
	unsigned int i;
	unsigned int is_constant[2] = {0, 0};

	assert(inst->U.I.Opcode == RC_OPCODE_ADD || inst->U.I.Opcode == RC_OPCODE_MAD);

	if (inst->U.I.PreSub.Opcode != RC_PRESUB_NONE
			|| inst->U.I.SaturateMode
			|| inst->U.I.WriteALUResult
			|| inst->U.I.Omod) {
		return 0;
	}

	/* If first two sources use a constant swizzle, then we can't convert it to
	 * a presubtract operation.  In fact for the ADD and SUB presubtract
	 * operations neither source can contain a constant swizzle.  This
	 * specific case is checked in peephole_add_presub_add() when
	 * we make sure the swizzles for both sources are equal, so we
	 * don't need to worry about it here. */
	for (i = 0; i < 2; i++) {
		int chan;
		for (chan = 0; chan < 4; chan++) {
			rc_swizzle swz =
				get_swz(inst->U.I.SrcReg[i].Swizzle, chan);
			if (swz == RC_SWIZZLE_ONE
					|| swz == RC_SWIZZLE_ZERO
					|| swz == RC_SWIZZLE_HALF) {
				is_constant[i] = 1;
			}
		}
	}
	if (is_constant[0] && is_constant[1])
		return 0;

	for(i = 0; i < info->NumSrcRegs; i++) {
		struct rc_src_register src = inst->U.I.SrcReg[i];
		if (src_reads_dst_mask(src, inst->U.I.DstReg))
			return 0;

		src.File = RC_FILE_PRESUB;
		if (!c->SwizzleCaps->IsNative(inst->U.I.Opcode, src))
			return 0;
	}
	return 1;
}

static int peephole_add_presub_add(
	struct radeon_compiler * c,
	struct rc_instruction * inst_add)
{
	unsigned dstmask = inst_add->U.I.DstReg.WriteMask;
        unsigned src0_neg = inst_add->U.I.SrcReg[0].Negate & dstmask;
        unsigned src1_neg = inst_add->U.I.SrcReg[1].Negate & dstmask;

	if (inst_add->U.I.SrcReg[0].Swizzle != inst_add->U.I.SrcReg[1].Swizzle)
		return 0;

	/* src0 and src1 can't have absolute values */
	if (inst_add->U.I.SrcReg[0].Abs || inst_add->U.I.SrcReg[1].Abs)
	        return 0;

        /* if src0 is negative, at least all bits of dstmask have to be set */
        if (inst_add->U.I.SrcReg[0].Negate && src0_neg != dstmask)
	        return 0;

        /* if src1 is negative, at least all bits of dstmask have to be set */
        if (inst_add->U.I.SrcReg[1].Negate && src1_neg != dstmask)
	        return 0;

	if (!is_presub_candidate(c, inst_add))
		return 0;

	if (presub_helper(c, inst_add, RC_PRESUB_ADD, presub_replace_add)) {
		rc_remove_instruction(inst_add);
		return 1;
	}
	return 0;
}

static void presub_replace_inv(
	struct rc_instruction * inst_add,
	struct rc_instruction * inst_reader,
	unsigned int src_index)
{
	/* We must be careful not to modify inst_add, since it
	 * is possible it will remain part of the program.*/
	inst_reader->U.I.PreSub.SrcReg[0] = inst_add->U.I.SrcReg[1];
	inst_reader->U.I.PreSub.SrcReg[0].Negate = 0;
	inst_reader->U.I.PreSub.Opcode = RC_PRESUB_INV;
	inst_reader->U.I.SrcReg[src_index] = chain_srcregs(inst_reader->U.I.SrcReg[src_index],
						inst_reader->U.I.PreSub.SrcReg[0]);

	inst_reader->U.I.SrcReg[src_index].File = RC_FILE_PRESUB;
	inst_reader->U.I.SrcReg[src_index].Index = RC_PRESUB_INV;
}

static void presub_replace_bias(
	struct rc_instruction * inst_mad,
	struct rc_instruction * inst_reader,
	unsigned int src_index)
{
	/* We must be careful not to modify inst_mad, since it
	 * is possible it will remain part of the program.*/
	inst_reader->U.I.PreSub.SrcReg[0] = inst_mad->U.I.SrcReg[0];
	inst_reader->U.I.PreSub.SrcReg[0].Negate = 0;
	inst_reader->U.I.PreSub.Opcode = RC_PRESUB_BIAS;
	inst_reader->U.I.SrcReg[src_index] = chain_srcregs(inst_reader->U.I.SrcReg[src_index],
						inst_reader->U.I.PreSub.SrcReg[0]);

	inst_reader->U.I.SrcReg[src_index].File = RC_FILE_PRESUB;
	inst_reader->U.I.SrcReg[src_index].Index = RC_PRESUB_BIAS;
}

/**
 * PRESUB_INV: ADD TEMP[0], none.1, -TEMP[1]
 * Use the presubtract 1 - src0 for all readers of TEMP[0].  The first source
 * of the add instruction must have the constant 1 swizzle.  This function
 * does not check const registers to see if their value is 1.0, so it should
 * be called after the constant_folding optimization.
 * @return
 * 	0 if the ADD instruction is still part of the program.
 * 	1 if the ADD instruction is no longer part of the program.
 */
static int peephole_add_presub_inv(
	struct radeon_compiler * c,
	struct rc_instruction * inst_add)
{
	unsigned int i, swz;

	if (!is_presub_candidate(c, inst_add))
		return 0;

	/* Check if src0 is 1. */
	/* XXX It would be nice to use is_src_uniform_constant here, but that
	 * function only works if the register's file is RC_FILE_NONE */
	for(i = 0; i < 4; i++ ) {
		if (!(inst_add->U.I.DstReg.WriteMask & (1 << i)))
			continue;

		swz = GET_SWZ(inst_add->U.I.SrcReg[0].Swizzle, i);
		if (swz != RC_SWIZZLE_ONE || inst_add->U.I.SrcReg[0].Negate & (1 << i))
			return 0;
	}

	/* Check src1. */
	if ((inst_add->U.I.SrcReg[1].Negate & inst_add->U.I.DstReg.WriteMask) !=
						inst_add->U.I.DstReg.WriteMask
		|| inst_add->U.I.SrcReg[1].Abs
		|| src_has_const_swz(inst_add->U.I.SrcReg[1])) {

		return 0;
	}

	if (presub_helper(c, inst_add, RC_PRESUB_INV, presub_replace_inv)) {
		rc_remove_instruction(inst_add);
		return 1;
	}
	return 0;
}

/**
 * PRESUB_BIAD: MAD -TEMP[0], 2.0, 1.0
 * Use the presubtract 1 - 2*src0 for all readers of TEMP[0].  The first source
 * of the add instruction must have the constant 1 swizzle.  This function
 * does not check const registers to see if their value is 1.0, so it should
 * be called after the constant_folding optimization.
 * @return
 * 	0 if the MAD instruction is still part of the program.
 * 	1 if the MAD instruction is no longer part of the program.
 */
static int peephole_mad_presub_bias(
	struct radeon_compiler * c,
	struct rc_instruction * inst_mad)
{
	unsigned int i, swz;

	if (!is_presub_candidate(c, inst_mad))
		return 0;

	/* Check if src2 is 1. */
	for(i = 0; i < 4; i++ ) {
		if (!(inst_mad->U.I.DstReg.WriteMask & (1 << i)))
			continue;

		swz = GET_SWZ(inst_mad->U.I.SrcReg[2].Swizzle, i);
		if (swz != RC_SWIZZLE_ONE || inst_mad->U.I.SrcReg[2].Negate & (1 << i))
			return 0;
	}

	/* Check if src1 is 2. */
	struct rc_src_register src1_reg = inst_mad->U.I.SrcReg[1];
	if ((src1_reg.Negate & inst_mad->U.I.DstReg.WriteMask) != 0 || src1_reg.Abs)
		return 0;
        struct rc_constant *constant = &c->Program.Constants.Constants[src1_reg.Index];
	if (constant->Type != RC_CONSTANT_IMMEDIATE)
		return 0;
        for (i = 0; i < 4; i++) {
		if (!(inst_mad->U.I.DstReg.WriteMask & (1 << i)))
			continue;
		swz = GET_SWZ(src1_reg.Swizzle, i);
		if (swz >= RC_SWIZZLE_ZERO || constant->u.Immediate[swz] != 2.0)
			return 0;
	}

	/* Check src0. */
	if ((inst_mad->U.I.SrcReg[0].Negate & inst_mad->U.I.DstReg.WriteMask) !=
						inst_mad->U.I.DstReg.WriteMask
		|| inst_mad->U.I.SrcReg[0].Abs
		|| src_has_const_swz(inst_mad->U.I.SrcReg[0])) {

		return 0;
	}

	if (presub_helper(c, inst_mad, RC_PRESUB_BIAS, presub_replace_bias)) {
		rc_remove_instruction(inst_mad);
		return 1;
	}
	return 0;
}

struct peephole_mul_cb_data {
	struct rc_dst_register * Writer;
	unsigned int Clobbered;
};

static void omod_filter_reader_cb(
	void * userdata,
	struct rc_instruction * inst,
	rc_register_file file,
	unsigned int index,
	unsigned int mask)
{
	struct peephole_mul_cb_data * d = userdata;
	if (rc_src_reads_dst_mask(file, mask, index,
		d->Writer->File, d->Writer->Index, d->Writer->WriteMask)) {

		d->Clobbered = 1;
	}
}

static void omod_filter_writer_cb(
	void * userdata,
	struct rc_instruction * inst,
	rc_register_file file,
	unsigned int index,
	unsigned int mask)
{
	struct peephole_mul_cb_data * d = userdata;
	if (file == d->Writer->File && index == d->Writer->Index &&
					(mask & d->Writer->WriteMask)) {
		d->Clobbered = 1;
	}
}

static int peephole_mul_omod(
	struct radeon_compiler * c,
	struct rc_instruction * inst_mul,
	struct rc_list * var_list)
{
	unsigned int chan = 0, swz, i;
	int const_index = -1;
	int temp_index = -1;
	float const_value;
	rc_omod_op omod_op = RC_OMOD_DISABLE;
	struct rc_list * writer_list;
	struct rc_variable * var;
	struct peephole_mul_cb_data cb_data;
	unsigned writemask_sum;

	for (i = 0; i < 2; i++) {
		unsigned int j;
		if (inst_mul->U.I.SrcReg[i].File != RC_FILE_CONSTANT
			&& inst_mul->U.I.SrcReg[i].File != RC_FILE_TEMPORARY) {
			return 0;
		}
		if (inst_mul->U.I.SrcReg[i].File == RC_FILE_TEMPORARY) {
			if (temp_index != -1) {
				/* The instruction has two temp sources */
				return 0;
			} else {
				temp_index = i;
				continue;
			}
		}
		/* If we get this far Src[i] must be a constant src */
		if (inst_mul->U.I.SrcReg[i].Negate) {
			return 0;
		}
		/* The constant src needs to read from the same swizzle */
		swz = RC_SWIZZLE_UNUSED;
		chan = 0;
		for (j = 0; j < 4; j++) {
			unsigned int j_swz =
				GET_SWZ(inst_mul->U.I.SrcReg[i].Swizzle, j);
			if (j_swz == RC_SWIZZLE_UNUSED) {
				continue;
			}
			if (swz == RC_SWIZZLE_UNUSED) {
				swz = j_swz;
				chan = j;
			} else if (j_swz != swz) {
				return 0;
			}
		}

		if (const_index != -1) {
			/* The instruction has two constant sources */
			return 0;
		} else {
			const_index = i;
		}
	}

	if (!rc_src_reg_is_immediate(c, inst_mul->U.I.SrcReg[const_index].File,
				inst_mul->U.I.SrcReg[const_index].Index)) {
		return 0;
	}
	const_value = rc_get_constant_value(c,
			inst_mul->U.I.SrcReg[const_index].Index,
			inst_mul->U.I.SrcReg[const_index].Swizzle,
			inst_mul->U.I.SrcReg[const_index].Negate,
			chan);

	if (const_value == 2.0f) {
		omod_op = RC_OMOD_MUL_2;
	} else if (const_value == 4.0f) {
		omod_op = RC_OMOD_MUL_4;
	} else if (const_value == 8.0f) {
		omod_op = RC_OMOD_MUL_8;
	} else if (const_value == (1.0f / 2.0f)) {
		omod_op = RC_OMOD_DIV_2;
	} else if (const_value == (1.0f / 4.0f)) {
		omod_op = RC_OMOD_DIV_4;
	} else if (const_value == (1.0f / 8.0f)) {
		omod_op = RC_OMOD_DIV_8;
	} else {
		return 0;
	}

	writer_list = rc_variable_list_get_writers_one_reader(var_list,
		RC_INSTRUCTION_NORMAL, &inst_mul->U.I.SrcReg[temp_index]);

	if (!writer_list) {
		return 0;
	}

	cb_data.Clobbered = 0;
	cb_data.Writer = &inst_mul->U.I.DstReg;
	for (var = writer_list->Item; var; var = var->Friend) {
		struct rc_instruction * inst;
		const struct rc_opcode_info * info = rc_get_opcode_info(
				var->Inst->U.I.Opcode);
		if (info->HasTexture) {
			return 0;
		}
		if (var->Inst->U.I.SaturateMode != RC_SATURATE_NONE) {
			return 0;
		}
		for (inst = inst_mul->Prev; inst != var->Inst;
							inst = inst->Prev) {
			rc_for_all_reads_mask(inst, omod_filter_reader_cb,
								&cb_data);
			rc_for_all_writes_mask(inst, omod_filter_writer_cb,
								&cb_data);
			if (cb_data.Clobbered) {
				break;
			}
		}
	}

	if (cb_data.Clobbered) {
		return 0;
	}

	writemask_sum = rc_variable_writemask_sum(writer_list->Item);

	/* rc_normal_rewrite_writemask can't expand a previous writemask to store
	 * more channels replicated.
	 */
	if (util_bitcount(writemask_sum) < util_bitcount(inst_mul->U.I.DstReg.WriteMask))
		return 0;

	/* Rewrite the instructions */
	for (var = writer_list->Item; var; var = var->Friend) {
		struct rc_variable * writer = var;
		unsigned conversion_swizzle = rc_make_conversion_swizzle(
					writemask_sum,
					inst_mul->U.I.DstReg.WriteMask);
		writer->Inst->U.I.Omod = omod_op;
		writer->Inst->U.I.DstReg.File = inst_mul->U.I.DstReg.File;
		writer->Inst->U.I.DstReg.Index = inst_mul->U.I.DstReg.Index;
		rc_normal_rewrite_writemask(writer->Inst, conversion_swizzle);
		writer->Inst->U.I.SaturateMode = inst_mul->U.I.SaturateMode;
	}

	rc_remove_instruction(inst_mul);

	return 1;
}

/**
 * @return
 * 	0 if inst is still part of the program.
 * 	1 if inst is no longer part of the program.
 */
static int peephole(struct radeon_compiler * c, struct rc_instruction * inst)
{
	if (!c->has_presub)
		return 0;

	switch(inst->U.I.Opcode) {
	case RC_OPCODE_ADD:
	{
		if (peephole_add_presub_inv(c, inst))
			return 1;
		if (peephole_add_presub_add(c, inst))
			return 1;
		break;
	}
	case RC_OPCODE_MAD:
	{
		if (peephole_mad_presub_bias(c, inst))
			return 1;
		break;
	}
	default:
		break;
	}
	return 0;
}

static unsigned int merge_swizzles(unsigned int swz1, unsigned int swz2)
{
	unsigned int new_swz = rc_init_swizzle(RC_SWIZZLE_UNUSED, 0);
	for (unsigned int chan = 0; chan < 4; chan++) {
		unsigned int swz = GET_SWZ(swz1, chan);
		if (swz != RC_SWIZZLE_UNUSED) {
			SET_SWZ(new_swz, chan, swz);
			continue;
		}
		swz = GET_SWZ(swz2, chan);
		SET_SWZ(new_swz, chan, swz);
	}
	return new_swz;
}

/* Sets negate to 0 for unused channels. */
static unsigned int clean_negate(struct rc_src_register src)
{
	unsigned int new_negate = 0;
	for (unsigned int chan = 0; chan < 4; chan++) {
		unsigned int swz = GET_SWZ(src.Swizzle, chan);
		if (swz != RC_SWIZZLE_UNUSED)
			new_negate |= src.Negate & (1 << chan);
	}
	return new_negate;
}

static unsigned int merge_negates(struct rc_src_register src1, struct rc_src_register src2)
{
	return clean_negate(src1) | clean_negate(src2);
}

static unsigned int fill_swizzle(unsigned int orig_swz, unsigned int wmask, unsigned int const_swz)
{
	for (unsigned int chan = 0; chan < 4; chan++) {
		unsigned int swz = GET_SWZ(orig_swz, chan);
		if (swz == RC_SWIZZLE_UNUSED && (wmask & (1 << chan))) {
			SET_SWZ(orig_swz, chan, const_swz);
		}
	}
	return orig_swz;
}

static int have_shared_source(struct rc_instruction * inst1, struct rc_instruction * inst2)
{
	int shared_src = -1;
	const struct rc_opcode_info * opcode1 = rc_get_opcode_info(inst1->U.I.Opcode);
	const struct rc_opcode_info * opcode2 = rc_get_opcode_info(inst2->U.I.Opcode);
	for (unsigned i = 0; i < opcode1->NumSrcRegs; i++) {
		for (unsigned j = 0; j < opcode2->NumSrcRegs; j++) {
			if (inst1->U.I.SrcReg[i].File == inst2->U.I.SrcReg[j].File &&
				inst1->U.I.SrcReg[i].Index == inst2->U.I.SrcReg[j].Index &&
				inst1->U.I.SrcReg[i].RelAddr == inst2->U.I.SrcReg[j].RelAddr &&
				inst1->U.I.SrcReg[i].Abs == inst2->U.I.SrcReg[j].Abs)
				shared_src = i;
		}
	}
	return shared_src;
}

/**
 * Merges two MOVs writing different channels of the same destination register
 * with the use of the constant swizzles.
 */
static bool merge_movs(
	struct radeon_compiler * c,
	struct rc_instruction * inst,
	struct rc_instruction * cur)
{
	/* We can merge two MOVs into MOV if one of them is from inline constant,
	 * i.e., constant swizzles and RC_FILE_NONE).
	 *
	 * For example
	 *   MOV temp[0].x none.1___
	 *   MOV temp[0].y input[0]._x__
	 *
	 * becomes
	 *   MOV temp[0].xy input[0].1x__
	 */
	unsigned int orig_dst_wmask = inst->U.I.DstReg.WriteMask;
	if (cur->U.I.SrcReg[0].File == RC_FILE_NONE ||
		inst->U.I.SrcReg[0].File == RC_FILE_NONE) {
		struct rc_src_register src;
		if (cur->U.I.SrcReg[0].File == RC_FILE_NONE)
			src = inst->U.I.SrcReg[0];
		else
			src = cur->U.I.SrcReg[0];
		src.Swizzle = merge_swizzles(cur->U.I.SrcReg[0].Swizzle,
						inst->U.I.SrcReg[0].Swizzle);
		src.Negate = merge_negates(inst->U.I.SrcReg[0], cur->U.I.SrcReg[0]);
		if (c->SwizzleCaps->IsNative(RC_OPCODE_MOV, src)) {
			cur->U.I.DstReg.WriteMask |= orig_dst_wmask;
			cur->U.I.SrcReg[0] = src;
			rc_remove_instruction(inst);
			return true;
		}
	}

	/* Handle the trivial case where the MOVs share a source.
	 *
	 * For example
	 *   MOV temp[0].x const[0].x
	 *   MOV temp[0].y const[0].z
	 *
	 * becomes
	 *   MOV temp[0].xy const[0].xz
	 */
	if (have_shared_source(inst, cur) == 0) {
		struct rc_src_register src = cur->U.I.SrcReg[0];
		src.Negate = merge_negates(inst->U.I.SrcReg[0], cur->U.I.SrcReg[0]);
		src.Swizzle = merge_swizzles(cur->U.I.SrcReg[0].Swizzle,
						inst->U.I.SrcReg[0].Swizzle);

                if (c->SwizzleCaps->IsNative(RC_OPCODE_MOV, src)) {
                        cur->U.I.DstReg.WriteMask |= orig_dst_wmask;
                        cur->U.I.SrcReg[0] = src;
                        rc_remove_instruction(inst);
                        return true;
                }
	}

	/* Otherwise, we can convert the MOVs into ADD.
	 *
	 * For example
	 *   MOV temp[0].x const[0].x
	 *   MOV temp[0].y input[0].y
	 *
	 * becomes
	 *   ADD temp[0].xy const[0].x0 input[0].0y
	 */
	unsigned wmask = cur->U.I.DstReg.WriteMask | orig_dst_wmask;
	struct rc_src_register src0 = inst->U.I.SrcReg[0];
	struct rc_src_register src1 = cur->U.I.SrcReg[0];

	src0.Swizzle = fill_swizzle(src0.Swizzle,
				wmask, RC_SWIZZLE_ZERO);
	src1.Swizzle = fill_swizzle(src1.Swizzle,
				wmask, RC_SWIZZLE_ZERO);
	if (!c->SwizzleCaps->IsNative(RC_OPCODE_ADD, src0) ||
		!c->SwizzleCaps->IsNative(RC_OPCODE_ADD, src1))
		return false;

	cur->U.I.DstReg.WriteMask = wmask;
	cur->U.I.Opcode = RC_OPCODE_ADD;
	cur->U.I.SrcReg[0] = src0;
	cur->U.I.SrcReg[1] = src1;

	/* finally delete the original mov */
	rc_remove_instruction(inst);
	return true;
}

/**
 * This function will try to merge MOV and ADD/MUL instructions with the same
 * destination, making use of the constant swizzles.
 *
 * For example:
 *   MOV temp[0].x const[0].x
 *   MUL temp[0].yz const[1].yz const[2].yz
 *
 * becomes
 *   MAD temp[0].xyz const[1].0yz const[2].0yz const[0].x00
 */
static int merge_mov_add_mul(
	struct radeon_compiler * c,
	struct rc_instruction * inst1,
	struct rc_instruction * inst2)
{
	struct rc_instruction * inst, * mov;
	if (inst1->U.I.Opcode == RC_OPCODE_MOV) {
		mov = inst1;
		inst = inst2;
	} else {
		mov = inst2;
		inst = inst1;
	}

	const bool is_mul = inst->U.I.Opcode == RC_OPCODE_MUL;
	int shared_index = have_shared_source(inst, mov);
	unsigned wmask = mov->U.I.DstReg.WriteMask | inst->U.I.DstReg.WriteMask;

	/* If there is a shared source, just merge the swizzles and be done with it. */
	if (shared_index != -1) {
		struct rc_src_register shared_src = inst->U.I.SrcReg[shared_index];
		struct rc_src_register other_src = inst->U.I.SrcReg[1 - shared_index];

		shared_src.Negate = merge_negates(mov->U.I.SrcReg[0], shared_src);
		shared_src.Swizzle = merge_swizzles(shared_src.Swizzle,
					mov->U.I.SrcReg[0].Swizzle);
		other_src.Negate = clean_negate(other_src);
		unsigned int swz = is_mul ? RC_SWIZZLE_ONE : RC_SWIZZLE_ZERO;
		other_src.Swizzle = fill_swizzle(other_src.Swizzle, wmask, swz);

		if (!c->SwizzleCaps->IsNative(RC_OPCODE_ADD, shared_src) ||
			!c->SwizzleCaps->IsNative(RC_OPCODE_ADD, other_src))
			return 0;

		inst2->U.I.Opcode = inst->U.I.Opcode;
		inst2->U.I.SrcReg[0] = shared_src;
		inst2->U.I.SrcReg[1] = other_src;

	/* TODO: we can do a bit better in the special case when one of the sources is none.
	 * Convert to MAD otherwise.
	 */
	} else {
		struct rc_src_register src0, src1, src2;
		if (is_mul) {
			src2 = mov->U.I.SrcReg[0];
			src0 = inst->U.I.SrcReg[0];
			src1 = inst->U.I.SrcReg[1];
		} else {
			src0 = mov->U.I.SrcReg[0];
			src1 = inst->U.I.SrcReg[0];
			src2 = inst->U.I.SrcReg[1];
		}
		/* The following login expects that the unused channels have empty negate bits. */
		src0.Negate = clean_negate(src0);
		src1.Negate = clean_negate(src1);
		src2.Negate = clean_negate(src2);

		src0.Swizzle = fill_swizzle(src0.Swizzle,
					wmask, RC_SWIZZLE_ONE);
		src1.Swizzle = fill_swizzle(src1.Swizzle,
					wmask, is_mul ? RC_SWIZZLE_ZERO : RC_SWIZZLE_ONE);
		src2.Swizzle = fill_swizzle(src2.Swizzle,
					wmask, RC_SWIZZLE_ZERO);
		if (!c->SwizzleCaps->IsNative(RC_OPCODE_MAD, src0) ||
			!c->SwizzleCaps->IsNative(RC_OPCODE_MAD, src1) ||
			!c->SwizzleCaps->IsNative(RC_OPCODE_MAD, src2))
			return 0;

		inst2->U.I.Opcode = RC_OPCODE_MAD;
		inst2->U.I.SrcReg[0] = src0;
		inst2->U.I.SrcReg[1] = src1;
		inst2->U.I.SrcReg[2] = src2;
	}
	inst2->U.I.DstReg.WriteMask = wmask;
	/* finally delete the original instruction */
	rc_remove_instruction(inst1);

	return 1;
}

/**
 * This function will try to merge MOV and MAD instructions with the same
 * destination, making use of the constant swizzles. This only works
 * if there is a shared source or one of the sources is RC_FILE_NONE.
 *
 * For example:
 *   MOV temp[0].x const[0].x
 *   MAD temp[0].yz const[0].yz const[1].yz input[0].xw
 *
 * becomes
 *   MAD temp[0].xyz const[0].xyz const[2].1yz input[0].0xw
 */
static bool merge_mov_mad(
	struct radeon_compiler * c,
	struct rc_instruction * inst1,
	struct rc_instruction * inst2)
{
	struct rc_instruction * mov, * mad;
	if (inst1->U.I.Opcode == RC_OPCODE_MOV) {
		mov = inst1;
		mad = inst2;
	} else {
		mov = inst2;
		mad = inst1;
	}

	int shared_index = have_shared_source(mad, mov);
	unsigned wmask = mov->U.I.DstReg.WriteMask | mad->U.I.DstReg.WriteMask;
	struct rc_src_register src[3];
	src[0] = mad->U.I.SrcReg[0];
	src[1] = mad->U.I.SrcReg[1];
	src[2] = mad->U.I.SrcReg[2];

	/* Shared source is the one for multiplication. */
	if (shared_index == 0 || shared_index == 1) {
		src[shared_index].Negate = merge_negates(src[shared_index], mov->U.I.SrcReg[0]);
		src[1 - shared_index].Negate = clean_negate(src[1 - shared_index]);
		src[shared_index].Swizzle = merge_swizzles(src[shared_index].Swizzle,
				mov->U.I.SrcReg[0].Swizzle);
		src[1 - shared_index].Swizzle = fill_swizzle(
				src[1 - shared_index].Swizzle, wmask, RC_SWIZZLE_ONE);
		src[2].Swizzle =  fill_swizzle(src[2].Swizzle, wmask, RC_SWIZZLE_ZERO);

	/* Shared source is the one for used for addition, or it is none. Additionally,
	 * if the mov SrcReg is none, we merge it with the addition (third) reg as well
	 * because than we have the highest change the swizzles will be legal.
	 */
	} else if (shared_index == 2 || mov->U.I.SrcReg[0].File == RC_FILE_NONE ||
			src[2].File == RC_FILE_NONE) {
		src[2].Negate = merge_negates(src[2], mov->U.I.SrcReg[0]);
		src[2].Swizzle = merge_swizzles(src[2].Swizzle, mov->U.I.SrcReg[0].Swizzle);
		src[0].Swizzle = fill_swizzle(src[0].Swizzle, wmask, RC_SWIZZLE_ZERO);
		src[1].Swizzle = fill_swizzle(src[1].Swizzle, wmask, RC_SWIZZLE_ZERO);
		if (src[2].File == RC_FILE_NONE) {
			src[2].File = mov->U.I.SrcReg[0].File;
			src[2].Index = mov->U.I.SrcReg[0].Index;
			src[2].RelAddr = mov->U.I.SrcReg[0].RelAddr;
			src[2].Abs = mov->U.I.SrcReg[0].Abs;
		}

	/* First or the second MAD source is RC_FILE_NONE, we merge the mov into it,
	 * fill the other one with ones and the reg for addition with zeros.
	 */
	} else if (src[0].File == RC_FILE_NONE || src[1].File == RC_FILE_NONE) {
		unsigned none_src = src[0].File == RC_FILE_NONE ? 0 : 1;
		src[none_src] = mov->U.I.SrcReg[0];
		src[none_src].Negate = merge_negates(src[none_src], mad->U.I.SrcReg[none_src]);
		src[none_src].Swizzle = merge_swizzles(src[none_src].Swizzle,
				mad->U.I.SrcReg[none_src].Swizzle);
		src[1 - none_src].Negate = clean_negate(src[1 - none_src]);
		src[1 - none_src].Swizzle = fill_swizzle(src[1 - none_src].Swizzle,
				wmask, RC_SWIZZLE_ONE);
		src[2].Swizzle =  fill_swizzle(src[2].Swizzle, wmask, RC_SWIZZLE_ZERO);
	} else {
		return false;
	}

	if (!c->SwizzleCaps->IsNative(RC_OPCODE_MAD, src[0]) ||
		!c->SwizzleCaps->IsNative(RC_OPCODE_MAD, src[1]) ||
		!c->SwizzleCaps->IsNative(RC_OPCODE_MAD, src[2]))
		return false;

	inst2->U.I.Opcode = RC_OPCODE_MAD;
	inst2->U.I.SrcReg[0] = src[0];
	inst2->U.I.SrcReg[1] = src[1];
	inst2->U.I.SrcReg[2] = src[2];
	inst2->U.I.DstReg.WriteMask = wmask;
	rc_remove_instruction(inst1);
	return true;
}

static bool inst_combination(
	struct rc_instruction * inst1,
	struct rc_instruction * inst2,
	rc_opcode opcode1,
	rc_opcode opcode2)
{
	return ((inst1->U.I.Opcode == opcode1 && inst2->U.I.Opcode == opcode2) ||
		(inst2->U.I.Opcode == opcode1 && inst1->U.I.Opcode == opcode2));
}

/**
 * Searches for instructions writing different channels of the same register that could
 * be merged together with the use of constant swizzles.
 *
 * The potential candidates are combinations of MOVs, ADDs, MULs and MADs.
 */
static void merge_channels(struct radeon_compiler * c, struct rc_instruction * inst)
{
	unsigned int orig_dst_reg = inst->U.I.DstReg.Index;
	unsigned int orig_dst_file = inst->U.I.DstReg.File;
	unsigned int orig_dst_wmask = inst->U.I.DstReg.WriteMask;
	const struct rc_opcode_info * orig_opcode = rc_get_opcode_info(inst->U.I.Opcode);

	struct rc_instruction * cur = inst;
	while (cur!= &c->Program.Instructions) {
		cur = cur->Next;
		const struct rc_opcode_info * opcode = rc_get_opcode_info(cur->U.I.Opcode);

		/* Keep it simple for now and stop when encountering any
		 * control flow.
		 */
		if (opcode->IsFlowControl)
			return;

		/* Stop when the original destination is overwritten */
		if (orig_dst_reg == cur->U.I.DstReg.Index &&
			orig_dst_file == cur->U.I.DstReg.File &&
			(orig_dst_wmask & cur->U.I.DstReg.WriteMask) != 0)
			return;

		/* Stop the search when the original instruction destination
		 * is used as a source for anything.
		 */
		for (unsigned i = 0; i < opcode->NumSrcRegs; i++) {
			if (cur->U.I.SrcReg[i].File == orig_dst_file &&
				cur->U.I.SrcReg[i].Index == orig_dst_reg)
				return;
		}

		/* Stop the search when some of the original sources are touched. */
		for (unsigned i = 0; i < orig_opcode->NumSrcRegs; i++) {
			if (inst->U.I.SrcReg[i].File == cur->U.I.DstReg.File &&
				inst->U.I.SrcReg[i].Index == cur->U.I.DstReg.Index)
				return;
		}

		if (cur->U.I.DstReg.File == orig_dst_file &&
			cur->U.I.DstReg.Index == orig_dst_reg &&
			cur->U.I.SaturateMode == inst->U.I.SaturateMode &&
			(cur->U.I.DstReg.WriteMask & orig_dst_wmask) == 0) {

			if (inst_combination(cur, inst, RC_OPCODE_MOV, RC_OPCODE_MOV)) {
				if (merge_movs(c, inst, cur))
					return;
			}

			/* Skip the merge if one of the instructions writes just w channel
			 * and we are compiling a fragment shader. We can pair-schedule it together
			 * later anyway and it will also give the scheduler a bit more flexibility.
			 * Only check this after merging MOVs as when we manage to merge two MOVs
			 * into another MOV we can still copy propagate it away. So it is a win in
			 * that case.
			 */
			if (c->has_omod && (cur->U.I.DstReg.WriteMask == RC_MASK_W ||
				inst->U.I.DstReg.WriteMask == RC_MASK_W))
				continue;

			if (inst_combination(cur, inst, RC_OPCODE_MOV, RC_OPCODE_ADD) ||
				inst_combination(cur, inst, RC_OPCODE_MOV, RC_OPCODE_MUL)) {
				if (merge_mov_add_mul(c, inst, cur))
					return;
			}

			if (inst_combination(cur, inst, RC_OPCODE_MOV, RC_OPCODE_MAD)) {
				if (merge_mov_mad(c, inst, cur))
					return;
			}
		}
	}
}

/**
 * Searches for duplicate ARLs/ARRs
 *
 * Only a very trivial case is now optimized where if a second one is detected which reads from
 * the same register as the first one and source is the same, just remove the second one.
 */
static void merge_A0_loads(
	struct radeon_compiler * c,
	struct rc_instruction * inst,
	bool is_ARL)
{
	unsigned int A0_src_reg = inst->U.I.SrcReg[0].Index;
	unsigned int A0_src_file = inst->U.I.SrcReg[0].File;
	unsigned int A0_src_swizzle = inst->U.I.SrcReg[0].Swizzle;
	int cf_depth = 0;

	struct rc_instruction * cur = inst;
	while (cur != &c->Program.Instructions) {
		cur = cur->Next;
		const struct rc_opcode_info * opcode = rc_get_opcode_info(cur->U.I.Opcode);

		/* Keep it simple for now and stop when encountering any
		 * control flow besides simple ifs.
		 */
		if (opcode->IsFlowControl) {
			switch (cur->U.I.Opcode) {
			case RC_OPCODE_IF:
			{
				cf_depth++;
				break;
			}
			case RC_OPCODE_ELSE:
			{
				if (cf_depth < 1)
					return;
				break;
			}
			case RC_OPCODE_ENDIF:
			{
                                cf_depth--;
                                break;
			}
			default:
				return;
			}
		}

		/* Stop when the original source is overwritten */
		if (A0_src_reg == cur->U.I.DstReg.Index &&
			A0_src_file == cur->U.I.DstReg.File &&
			cur->U.I.DstReg.WriteMask | rc_swizzle_to_writemask(A0_src_swizzle))
			return;

		/* Wrong A0 load type. */
		if ((is_ARL && cur->U.I.Opcode == RC_OPCODE_ARR) ||
		    (!is_ARL && cur->U.I.Opcode == RC_OPCODE_ARL))
			return;

		if (cur->U.I.Opcode == RC_OPCODE_ARL || cur->U.I.Opcode == RC_OPCODE_ARR) {
			if (A0_src_reg == cur->U.I.SrcReg[0].Index &&
			    A0_src_file == cur->U.I.SrcReg[0].File &&
			    A0_src_swizzle == cur->U.I.SrcReg[0].Swizzle) {
				struct rc_instruction * next = cur->Next;
				rc_remove_instruction(cur);
				cur = next;
			} else {
				return;
			}
		}
	}
}

/**
 * According to the GLSL spec, round is only 1.30 and up
 * so the only reason why we should ever see round is if it actually
 * is lowered ARR (from nine->ttn). In that case we want to reconstruct
 * the ARR instead of lowering the round.
 */
static void transform_vertex_ROUND(struct radeon_compiler* c,
	struct rc_instruction* inst)
{
	struct rc_reader_data readers;
	rc_get_readers(c, inst, &readers, NULL, NULL, NULL);

	assert(readers.ReaderCount > 0);
	for (unsigned i = 0; i < readers.ReaderCount; i++) {
		struct rc_instruction *reader = readers.Readers[i].Inst;
		if (reader->U.I.Opcode != RC_OPCODE_ARL) {
			assert(!"Unable to convert ROUND+ARL to ARR\n");
			return;
		}
	}

	/* Only ARL readers, convert all to ARR */
	for (unsigned i = 0; i < readers.ReaderCount; i++) {
		readers.Readers[i].Inst->U.I.Opcode = RC_OPCODE_ARR;
	}
	/* Switch ROUND to MOV and let copy propagate sort it out later. */
	inst->U.I.Opcode = RC_OPCODE_MOV;
}

/**
 * Apply various optimizations specific to the A0 adress register loads.
 */
static void optimize_A0_loads(struct radeon_compiler * c) {
	struct rc_instruction * inst = c->Program.Instructions.Next;

	while (inst != &c->Program.Instructions) {
		struct rc_instruction * cur = inst;
		inst = inst->Next;
		if (cur->U.I.Opcode == RC_OPCODE_ARL) {
			merge_A0_loads(c, cur, true);
		} else if (cur->U.I.Opcode == RC_OPCODE_ARR) {
			merge_A0_loads(c, cur, false);
		} else if (cur->U.I.Opcode == RC_OPCODE_ROUND) {
			transform_vertex_ROUND(c, cur);
		}
	}
}

void rc_optimize(struct radeon_compiler * c, void *user)
{
	struct rc_instruction * inst = c->Program.Instructions.Next;
	while(inst != &c->Program.Instructions) {
		struct rc_instruction * cur = inst;
		inst = inst->Next;
		constant_folding(c, cur);
	}

	/* Copy propagate simple movs away. */
	inst = c->Program.Instructions.Next;
	while(inst != &c->Program.Instructions) {
		struct rc_instruction * cur = inst;
		inst = inst->Next;
		if (cur->U.I.Opcode == RC_OPCODE_MOV) {
			copy_propagate(c, cur);
		}
	}

	if (c->type == RC_VERTEX_PROGRAM) {
		optimize_A0_loads(c);
	}

	/* Merge MOVs to same source in different channels using the constant
	 * swizzle.
	 */
	if (c->is_r500 || c->type == RC_VERTEX_PROGRAM) {
		inst = c->Program.Instructions.Next;
		while(inst != &c->Program.Instructions) {
			struct rc_instruction * cur = inst;
			inst = inst->Next;
			if (cur->U.I.Opcode == RC_OPCODE_MOV ||
				cur->U.I.Opcode == RC_OPCODE_ADD ||
				cur->U.I.Opcode == RC_OPCODE_MAD ||
				cur->U.I.Opcode == RC_OPCODE_MUL)
				merge_channels(c, cur);
		}
	}

	/* Copy propagate few extra movs from the merge_channels pass. */
	inst = c->Program.Instructions.Next;
	while(inst != &c->Program.Instructions) {
		struct rc_instruction * cur = inst;
		inst = inst->Next;
		if (cur->U.I.Opcode == RC_OPCODE_MOV) {
			copy_propagate(c, cur);
		}
	}

	if (c->type != RC_FRAGMENT_PROGRAM) {
		return;
	}

	/* Presubtract operations. */
	inst = c->Program.Instructions.Next;
	while(inst != &c->Program.Instructions) {
		struct rc_instruction * cur = inst;
		inst = inst->Next;
		peephole(c, cur);
	}

	/* Output modifiers. */
	inst = c->Program.Instructions.Next;
	struct rc_list * var_list = NULL;
	while(inst != &c->Program.Instructions) {
		struct rc_instruction * cur = inst;
		inst = inst->Next;
		if (cur->U.I.Opcode == RC_OPCODE_MUL) {
			if (!var_list)
				var_list = rc_get_variables(c);
			if (peephole_mul_omod(c, cur, var_list))
				var_list = NULL;
		}
	}
}
