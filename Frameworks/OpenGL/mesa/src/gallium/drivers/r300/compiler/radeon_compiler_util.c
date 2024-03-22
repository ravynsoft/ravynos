/*
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

/**
 * \file
 */

#include "radeon_compiler_util.h"

#include "radeon_compiler.h"
#include "radeon_dataflow.h"
#include "r300_fragprog_swizzle.h"
/**
 */
unsigned int rc_swizzle_to_writemask(unsigned int swz)
{
	unsigned int mask = 0;
	unsigned int i;

	for(i = 0; i < 4; i++) {
		mask |= 1 << GET_SWZ(swz, i);
	}
	mask &= RC_MASK_XYZW;

	return mask;
}

rc_swizzle get_swz(unsigned int swz, rc_swizzle idx)
{
	if (idx & 0x4)
		return idx;
	return GET_SWZ(swz, idx);
}

/**
 * The purpose of this function is to standardize the number channels used by
 * swizzles.  All swizzles regardless of what instruction they are a part of
 * should have 4 channels initialized with values.
 * @param channels The number of channels in initial_value that have a
 * meaningful value.
 * @return An initialized swizzle that has all of the unused channels set to
 * RC_SWIZZLE_UNUSED.
 */
unsigned int rc_init_swizzle(unsigned int initial_value, unsigned int channels)
{
	unsigned int i;
	for (i = channels; i < 4; i++) {
		SET_SWZ(initial_value, i, RC_SWIZZLE_UNUSED);
	}
	return initial_value;
}

unsigned int combine_swizzles4(unsigned int src,
		rc_swizzle swz_x, rc_swizzle swz_y, rc_swizzle swz_z, rc_swizzle swz_w)
{
	unsigned int ret = 0;

	ret |= get_swz(src, swz_x);
	ret |= get_swz(src, swz_y) << 3;
	ret |= get_swz(src, swz_z) << 6;
	ret |= get_swz(src, swz_w) << 9;

	return ret;
}

unsigned int combine_swizzles(unsigned int src, unsigned int swz)
{
	unsigned int ret = 0;

	ret |= get_swz(src, GET_SWZ(swz, RC_SWIZZLE_X));
	ret |= get_swz(src, GET_SWZ(swz, RC_SWIZZLE_Y)) << 3;
	ret |= get_swz(src, GET_SWZ(swz, RC_SWIZZLE_Z)) << 6;
	ret |= get_swz(src, GET_SWZ(swz, RC_SWIZZLE_W)) << 9;

	return ret;
}

/**
 * @param mask Must be either RC_MASK_X, RC_MASK_Y, RC_MASK_Z, or RC_MASK_W
 */
rc_swizzle rc_mask_to_swizzle(unsigned int mask)
{
	switch (mask) {
	case RC_MASK_X: return RC_SWIZZLE_X;
	case RC_MASK_Y: return RC_SWIZZLE_Y;
	case RC_MASK_Z: return RC_SWIZZLE_Z;
	case RC_MASK_W: return RC_SWIZZLE_W;
	}
	return RC_SWIZZLE_UNUSED;
}

/* Reorder mask bits according to swizzle. */
unsigned swizzle_mask(unsigned swizzle, unsigned mask)
{
	unsigned ret = 0;
	for (unsigned chan = 0; chan < 4; ++chan) {
		unsigned swz = GET_SWZ(swizzle, chan);
		if (swz < 4)
			ret |= GET_BIT(mask, swz) << chan;
	}
	return ret;
}

static unsigned int srcs_need_rewrite(const struct rc_opcode_info * info)
{
	if (info->HasTexture) {
		return 0;
	}
	switch (info->Opcode) {
		case RC_OPCODE_DP2:
		case RC_OPCODE_DP3:
		case RC_OPCODE_DP4:
		case RC_OPCODE_DDX:
		case RC_OPCODE_DDY:
			return 0;
		default:
			return 1;
	}
}

/**
 * This function moves the old swizzles to new channels using the values
 * in the conversion swizzle. For example if the instruction writemask is
 * changed from x to y, then conversion_swizzle should be y___ and this
 * function will adjust the old argument swizzles (of the same instruction)
 * to the new channels, so x___ will become _x__, etc...
 *
 * @param old_swizzle The swizzle to change
 * @param conversion_swizzle Describes the conversion to perform on the swizzle
 * @return A new swizzle
 */
unsigned int rc_adjust_channels(
	unsigned int old_swizzle,
	unsigned int conversion_swizzle)
{
	unsigned int i;
	unsigned int new_swizzle = rc_init_swizzle(RC_SWIZZLE_UNUSED, 0);
	for (i = 0; i < 4; i++) {
		unsigned int new_chan = get_swz(conversion_swizzle, i);
		if (new_chan == RC_SWIZZLE_UNUSED) {
			continue;
		}
		SET_SWZ(new_swizzle, new_chan, GET_SWZ(old_swizzle, i));
	}
	return new_swizzle;
}

static unsigned int rewrite_writemask(
	unsigned int old_mask,
	unsigned int conversion_swizzle)
{
	unsigned int new_mask = 0;
	unsigned int i;

	for (i = 0; i < 4; i++) {
		if (!GET_BIT(old_mask, i)
		   || GET_SWZ(conversion_swizzle, i) == RC_SWIZZLE_UNUSED) {
			continue;
		}
		new_mask |= (1 << GET_SWZ(conversion_swizzle, i));
	}

	return new_mask;
}

/**
 * This function rewrites the writemask of sub and adjusts the swizzles
 * of all its source registers based on the conversion_swizzle.
 * conversion_swizzle represents a mapping of the old writemask to the
 * new writemask.  For a detailed description of how conversion swizzles
 * work see rc_rewrite_swizzle().
 */
void rc_pair_rewrite_writemask(
	struct rc_pair_sub_instruction * sub,
	unsigned int conversion_swizzle)
{
	const struct rc_opcode_info * info = rc_get_opcode_info(sub->Opcode);
	unsigned int i;

	sub->WriteMask = rewrite_writemask(sub->WriteMask, conversion_swizzle);

	if (!srcs_need_rewrite(info)) {
		return ;
	}

	for (i = 0; i < info->NumSrcRegs; i++) {
		sub->Arg[i].Swizzle =
			rc_adjust_channels(sub->Arg[i].Swizzle,
						conversion_swizzle);
	}
}

static void normal_rewrite_writemask_cb(
	void * userdata,
	struct rc_instruction * inst,
	struct rc_src_register * src)
{
	unsigned int * conversion_swizzle = (unsigned int *)userdata;
	src->Swizzle = rc_adjust_channels(src->Swizzle, *conversion_swizzle);

	/* Per-channel negates are possible in vertex shaders,
	 * so we need to rewrite it properly as well. */
	unsigned int new_negate = 0;
	for (unsigned int i = 0; i < 4; i++) {
		unsigned int new_chan = get_swz(*conversion_swizzle, i);

		if (new_chan == RC_SWIZZLE_UNUSED)
			continue;

		if ((1 << i) & src->Negate)
			new_negate |= 1 << new_chan;
	}
	src->Negate = new_negate;
}

/**
 * This function is the same as rc_pair_rewrite_writemask() except it
 * operates on normal instructions.
 */
void rc_normal_rewrite_writemask(
	struct rc_instruction * inst,
	unsigned int conversion_swizzle)
{
	struct rc_sub_instruction * sub = &inst->U.I;
	const struct rc_opcode_info * info = rc_get_opcode_info(sub->Opcode);
	sub->DstReg.WriteMask =
		rewrite_writemask(sub->DstReg.WriteMask, conversion_swizzle);

	if (info->HasTexture) {
		unsigned int i;
		assert(sub->TexSwizzle == RC_SWIZZLE_XYZW);
		for (i = 0; i < 4; i++) {
			unsigned int swz = GET_SWZ(conversion_swizzle, i);
			if (swz > 3)
				continue;
			SET_SWZ(sub->TexSwizzle, swz, i);
		}
	}

	if (!srcs_need_rewrite(info)) {
		return;
	}

	rc_for_all_reads_src(inst, normal_rewrite_writemask_cb,
							&conversion_swizzle);
}

/**
 * This function replaces each value 'swz' in swizzle with the value of
 * GET_SWZ(conversion_swizzle, swz).  So, if you want to change all the X's
 * in swizzle to Y, then conversion_swizzle should be Y___ (0xff9).  If you want
 * to change all the Y's in swizzle to X, then conversion_swizzle should be
 * _X__ (0xfc7).  If you want to change the Y's to X and the X's to Y, then
 * conversion swizzle should be YX__ (0xfc1).
 * @param swizzle The swizzle to change
 * @param conversion_swizzle Describes the conversion to perform on the swizzle
 * @return A converted swizzle
 */
unsigned int rc_rewrite_swizzle(
	unsigned int swizzle,
	unsigned int conversion_swizzle)
{
	unsigned int chan;
	unsigned int out_swizzle = swizzle;

	for (chan = 0; chan < 4; chan++) {
		unsigned int swz = GET_SWZ(swizzle, chan);
		unsigned int new_swz;
		if (swz > 3) {
			SET_SWZ(out_swizzle, chan, swz);
		} else {
			new_swz = GET_SWZ(conversion_swizzle, swz);
			if (new_swz != RC_SWIZZLE_UNUSED) {
				SET_SWZ(out_swizzle, chan, new_swz);
			} else {
				SET_SWZ(out_swizzle, chan, swz);
			}
		}
	}
	return out_swizzle;
}

/**
 * Left multiplication of a register with a swizzle
 */
struct rc_src_register lmul_swizzle(unsigned int swizzle, struct rc_src_register srcreg)
{
	struct rc_src_register tmp = srcreg;
	int i;
	tmp.Swizzle = 0;
	tmp.Negate = 0;
	for(i = 0; i < 4; ++i) {
		rc_swizzle swz = GET_SWZ(swizzle, i);
		if (swz < 4) {
			tmp.Swizzle |= GET_SWZ(srcreg.Swizzle, swz) << (i*3);
			tmp.Negate |= GET_BIT(srcreg.Negate, swz) << i;
		} else {
			tmp.Swizzle |= swz << (i*3);
		}
	}
	return tmp;
}

void reset_srcreg(struct rc_src_register* reg)
{
	memset(reg, 0, sizeof(struct rc_src_register));
	reg->Swizzle = RC_SWIZZLE_XYZW;
}

unsigned int rc_src_reads_dst_mask(
		rc_register_file src_file,
		unsigned int src_idx,
		unsigned int src_swz,
		rc_register_file dst_file,
		unsigned int dst_idx,
		unsigned int dst_mask)
{
	if (src_file != dst_file || src_idx != dst_idx) {
		return RC_MASK_NONE;
	}
	return dst_mask & rc_swizzle_to_writemask(src_swz);
}

/**
 * @return A bit mask specifying whether this swizzle will select from an RGB
 * source, an Alpha source, or both.
 */
unsigned int rc_source_type_swz(unsigned int swizzle)
{
	unsigned int chan;
	unsigned int swz = RC_SWIZZLE_UNUSED;
	unsigned int ret = RC_SOURCE_NONE;

	for(chan = 0; chan < 4; chan++) {
		swz = GET_SWZ(swizzle, chan);
		if (swz == RC_SWIZZLE_W) {
			ret |= RC_SOURCE_ALPHA;
		} else if (swz == RC_SWIZZLE_X || swz == RC_SWIZZLE_Y
						|| swz == RC_SWIZZLE_Z) {
			ret |= RC_SOURCE_RGB;
		}
	}
	return ret;
}

unsigned int rc_source_type_mask(unsigned int mask)
{
	unsigned int ret = RC_SOURCE_NONE;

	if (mask & RC_MASK_XYZ)
		ret |= RC_SOURCE_RGB;

	if (mask & RC_MASK_W)
		ret |= RC_SOURCE_ALPHA;

	return ret;
}

struct src_select {
	rc_register_file File;
	int Index;
	unsigned int SrcType;
	unsigned int Swizzle;
};

struct can_use_presub_data {
	struct src_select Selects[5];
	unsigned int SelectCount;
	const struct rc_src_register * ReplaceReg;
	unsigned int ReplaceRemoved;
};

static void can_use_presub_data_add_select(
	struct can_use_presub_data * data,
	rc_register_file file,
	unsigned int index,
	unsigned int swizzle)
{
	struct src_select * select;

	select = &data->Selects[data->SelectCount++];
	select->File = file;
	select->Index = index;
	select->SrcType = rc_source_type_swz(swizzle);
	select->Swizzle = swizzle;
}

/**
 * This callback function counts the number of sources in inst that are
 * different from the sources in can_use_presub_data->RemoveSrcs.
 */
static void can_use_presub_read_cb(
	void * userdata,
	struct rc_instruction * inst,
	struct rc_src_register * src)
{
	struct can_use_presub_data * d = userdata;

	if (!d->ReplaceRemoved && src == d->ReplaceReg) {
		d->ReplaceRemoved = 1;
		return;
	}

	if (src->File == RC_FILE_NONE)
		return;

	can_use_presub_data_add_select(d, src->File, src->Index,
					src->Swizzle);
}

unsigned int rc_inst_can_use_presub(
	struct radeon_compiler * c,
	struct rc_instruction * inst,
	rc_presubtract_op presub_op,
	unsigned int presub_writemask,
	const struct rc_src_register * replace_reg,
	const struct rc_src_register * presub_src0,
	const struct rc_src_register * presub_src1)
{
	struct can_use_presub_data d;
	unsigned int num_presub_srcs;
	unsigned int i;
	const struct rc_opcode_info * info =
					rc_get_opcode_info(inst->U.I.Opcode);
	int rgb_count = 0, alpha_count = 0;
	unsigned int src_type0, src_type1;

	if (presub_op == RC_PRESUB_NONE) {
		return 1;
	}

	if (info->HasTexture) {
		return 0;
	}

	/* We can't allow constant swizzles from presubtract, because it is not possible
	 * to rewrite it to a native swizzle later. */
	if (!c->is_r500) {
		for (i = 0; i < 4; i++) {
			rc_swizzle swz = GET_SWZ(replace_reg->Swizzle, i);
			if (swz > RC_SWIZZLE_W && swz < RC_SWIZZLE_UNUSED)
				return 0;
		}
	}

	/* We can't use more than one presubtract value in an
	 * instruction, unless the two prsubtract operations
	 * are the same and read from the same registers.
	 * XXX For now we will limit instructions to only one presubtract
	 * value.*/
	if (inst->U.I.PreSub.Opcode != RC_PRESUB_NONE) {
		return 0;
	}

	memset(&d, 0, sizeof(d));
	d.ReplaceReg = replace_reg;

	rc_for_all_reads_src(inst, can_use_presub_read_cb, &d);

	num_presub_srcs = rc_presubtract_src_reg_count(presub_op);

	src_type0 = rc_source_type_swz(presub_src0->Swizzle);
	can_use_presub_data_add_select(&d,
		presub_src0->File,
		presub_src0->Index,
		presub_src0->Swizzle);

	if (num_presub_srcs > 1) {
		src_type1 = rc_source_type_swz(presub_src1->Swizzle);
		can_use_presub_data_add_select(&d,
			presub_src1->File,
			presub_src1->Index,
			presub_src1->Swizzle);

		/* Even if both of the presub sources read from the same
		 * register, we still need to use 2 different source selects
		 * for them, so we need to increment the count to compensate.
		 */
		if (presub_src0->File == presub_src1->File
		    && presub_src0->Index == presub_src1->Index) {
			if (src_type0 & src_type1 & RC_SOURCE_RGB) {
				rgb_count++;
			}
			if (src_type0 & src_type1 & RC_SOURCE_ALPHA) {
				alpha_count++;
			}
		}
	}

	/* Count the number of source selects for Alpha and RGB.  If we
	 * encounter two of the same source selects then we can ignore the
	 * first one. */
	for (i = 0; i < d.SelectCount; i++) {
		unsigned int j;
		unsigned int src_type = d.Selects[i].SrcType;
		for (j = i + 1; j < d.SelectCount; j++) {
			/* Even if the sources are the same now, they will not be the
			 * same later, if we have to rewrite some non-native swizzle. */
			if(!c->is_r500 && (
				!r300_swizzle_is_native_basic(d.Selects[i].Swizzle) ||
				!r300_swizzle_is_native_basic(d.Selects[j].Swizzle)))
				continue;
			if (d.Selects[i].File == d.Selects[j].File
			    && d.Selects[i].Index == d.Selects[j].Index) {
				src_type &= ~d.Selects[j].SrcType;
			}
		}
		if (src_type & RC_SOURCE_RGB) {
			rgb_count++;
		}

		if (src_type & RC_SOURCE_ALPHA) {
			alpha_count++;
		}
	}

	if (rgb_count > 3 || alpha_count > 3) {
		return 0;
	}

	return 1;
}

struct max_data {
	unsigned int Max;
	unsigned int HasFileType;
	rc_register_file File;
};

static void max_callback(
	void * userdata,
	struct rc_instruction * inst,
	rc_register_file file,
	unsigned int index,
	unsigned int mask)
{
	struct max_data * d = (struct max_data*)userdata;
	if (file == d->File && (!d->HasFileType || index > d->Max)) {
		d->Max = index;
		d->HasFileType = 1;
	}
}

/**
 * @return The maximum index of the specified register file used by the
 * program.
 */
int rc_get_max_index(
	struct radeon_compiler * c,
	rc_register_file file)
{
	struct max_data data;
	struct rc_instruction * inst;
	data.Max = 0;
	data.HasFileType = 0;
	data.File = file;
	for (inst = c->Program.Instructions.Next;
					inst != &c->Program.Instructions;
					inst = inst->Next) {
		rc_for_all_reads_mask(inst, max_callback, &data);
		rc_for_all_writes_mask(inst, max_callback, &data);
	}
	if (!data.HasFileType) {
		return -1;
	} else {
		return data.Max;
	}
}

/**
 * This function removes a source from a pair instructions.
 * @param inst
 * @param src_type RC_SOURCE_RGB, RC_SOURCE_ALPHA, or both bitwise or'd
 * @param source The index of the source to remove

 */
void rc_pair_remove_src(
	struct rc_instruction * inst,
	unsigned int src_type,
	unsigned int source)
{
	if (src_type & RC_SOURCE_RGB) {
		memset(&inst->U.P.RGB.Src[source], 0,
			sizeof(struct rc_pair_instruction_source));
	}

	if (src_type & RC_SOURCE_ALPHA) {
		memset(&inst->U.P.Alpha.Src[source], 0,
			sizeof(struct rc_pair_instruction_source));
	}
}

/**
 * @return RC_OPCODE_NOOP if inst is not a flow control instruction.
 * @return The opcode of inst if it is a flow control instruction.
 */
rc_opcode rc_get_flow_control_inst(struct rc_instruction * inst)
{
	const struct rc_opcode_info * info;
	if (inst->Type == RC_INSTRUCTION_NORMAL) {
		info = rc_get_opcode_info(inst->U.I.Opcode);
	} else {
		info = rc_get_opcode_info(inst->U.P.RGB.Opcode);
		/*A flow control instruction shouldn't have an alpha
		 * instruction.*/
		assert(!info->IsFlowControl ||
				inst->U.P.Alpha.Opcode == RC_OPCODE_NOP);
	}

	if (info->IsFlowControl)
		return info->Opcode;
	else
		return RC_OPCODE_NOP;

}

/**
 * @return The BGNLOOP instruction that starts the loop ended by endloop.
 */
struct rc_instruction * rc_match_endloop(struct rc_instruction * endloop)
{
	unsigned int endloop_count = 0;
	struct rc_instruction * inst;
	for (inst = endloop->Prev; inst != endloop; inst = inst->Prev) {
		rc_opcode op = rc_get_flow_control_inst(inst);
		if (op == RC_OPCODE_ENDLOOP) {
			endloop_count++;
		} else if (op == RC_OPCODE_BGNLOOP) {
			if (endloop_count == 0) {
				return inst;
			} else {
				endloop_count--;
			}
		}
	}
	return NULL;
}

/**
 * @return The ENDLOOP instruction that ends the loop started by bgnloop.
 */
struct rc_instruction * rc_match_bgnloop(struct rc_instruction * bgnloop)
{
	unsigned int bgnloop_count = 0;
	struct rc_instruction * inst;
	for (inst = bgnloop->Next; inst!=bgnloop; inst = inst->Next) {
		rc_opcode op = rc_get_flow_control_inst(inst);
		if (op == RC_OPCODE_BGNLOOP) {
			bgnloop_count++;
		} else if (op == RC_OPCODE_ENDLOOP) {
			if (bgnloop_count == 0) {
				return inst;
			} else {
				bgnloop_count--;
			}
		}
	}
	return NULL;
}

/**
 * @return A conversion swizzle for converting from old_mask->new_mask
 */
unsigned int rc_make_conversion_swizzle(
	unsigned int old_mask,
	unsigned int new_mask)
{
	unsigned int conversion_swizzle = rc_init_swizzle(RC_SWIZZLE_UNUSED, 0);
	unsigned int old_idx;
	unsigned int new_idx = 0;
	for (old_idx = 0; old_idx < 4; old_idx++) {
		if (!GET_BIT(old_mask, old_idx))
			continue;
		for ( ; new_idx < 4; new_idx++) {
			if (GET_BIT(new_mask, new_idx)) {
				SET_SWZ(conversion_swizzle, old_idx, new_idx);
				new_idx++;
				break;
			}
		}
	}
	return conversion_swizzle;
}

/**
 * @return 1 if the register contains an immediate value, 0 otherwise.
 */
unsigned int rc_src_reg_is_immediate(
	struct radeon_compiler * c,
	unsigned int file,
	unsigned int index)
{
	return file == RC_FILE_CONSTANT &&
	c->Program.Constants.Constants[index].Type == RC_CONSTANT_IMMEDIATE;
}

/**
 * @return The immediate value in the specified register.
 */
float rc_get_constant_value(
	struct radeon_compiler * c,
	unsigned int index,
	unsigned int swizzle,
	unsigned int negate,
	unsigned int chan)
{
	float base = 1.0f;
	int swz = GET_SWZ(swizzle, chan);
	if(swz >= 4 || index >= c->Program.Constants.Count ){
		rc_error(c, "get_constant_value: Can't find a value.\n");
		return 0.0f;
	}
	if(GET_BIT(negate, chan)){
		base = -1.0f;
	}
	return base *
		c->Program.Constants.Constants[index].u.Immediate[swz];
}

/**
 * This function returns the component value (RC_SWIZZLE_*) of the first used
 * channel in the swizzle.  This is only useful for scalar instructions that are
 * known to use only one channel of the swizzle.
 */
unsigned int rc_get_scalar_src_swz(unsigned int swizzle)
{
	unsigned int swz, chan;
	for (chan = 0; chan < 4; chan++) {
		swz = GET_SWZ(swizzle, chan);
		if (swz != RC_SWIZZLE_UNUSED) {
			break;
		}
	}
	assert(swz != RC_SWIZZLE_UNUSED);
	return swz;
}

bool rc_inst_has_three_diff_temp_srcs(struct rc_instruction *inst)
{
	return (inst->U.I.SrcReg[0].File == RC_FILE_TEMPORARY &&
		inst->U.I.SrcReg[1].File == RC_FILE_TEMPORARY &&
		inst->U.I.SrcReg[2].File == RC_FILE_TEMPORARY &&
		inst->U.I.SrcReg[0].Index != inst->U.I.SrcReg[1].Index &&
		inst->U.I.SrcReg[1].Index != inst->U.I.SrcReg[2].Index &&
		inst->U.I.SrcReg[0].Index != inst->U.I.SrcReg[2].Index);
}
