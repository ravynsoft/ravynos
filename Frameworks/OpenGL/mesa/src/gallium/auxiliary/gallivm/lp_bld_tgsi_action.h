/*
 * Copyright 2011-2012 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 **************************************************************************/

/**
 *
 * @author Tom Stellard <thomas.stellard@amd.com>
 *
 */


#ifndef LP_BLD_TGSI_ACTION_H
#define LP_BLD_TGSI_ACTION_H

#include <llvm-c/Core.h>

struct lp_build_tgsi_context;

struct lp_build_emit_data {
   /** Arguments that are passed to lp_build_tgsi_action::emit.  The
    * order of the arguments should be as follows:
    * SOA: s0.x, s0.y, s0.z, s0.w, s1.x, s1.y, s1.z, s1.w, s2.x, s2.y, s2.x, s2.w
    * AOS: s0.xyzw, s1.xyzw, s2.xyzw
    * TEXTURE Instructions: coord.xyzw
    *
    * Arguments should be packed into the args array.  For example an SOA
    * instructions that reads s0.x and s1.x args should look like this:
    * args[0] = s0.x;
    * args[1] = s1.x;
    */
   LLVMValueRef args[20];

   /**
    * Number of arguments in the args array.
    */
   unsigned arg_count;

   /**
    * The type output type of the opcode.  This should be set in the
    * lp_build_tgsi_action::fetch_args function.
    */
   LLVMTypeRef dst_type;

   /** This is used by the lp_build_tgsi_action::fetch_args function to
    * determine which channel to read from the opcode arguments.  It also
    * specifies which index of the output array should be written to by
    * the lp_build_tgsi_action::emit function.  However, this value is
    * usually ignored by any opcodes that are not TGSI_OUTPUT_COMPONENTWISE.
    */
   unsigned chan;

   /**
    * This is used to specify the src channel to read from for doubles.
    */
   unsigned src_chan;

   /** The lp_build_tgsi_action::emit 'executes' the opcode and writes the
    * results to this array.
    */
   LLVMValueRef output[4];

   /**
    * Secondary output for instruction that have a second destination register.
    */
   LLVMValueRef output1[4];

   /**
    * The current instruction that is being 'executed'.
    */
   const struct tgsi_full_instruction * inst;
   const struct tgsi_opcode_info * info;
};

struct lp_build_tgsi_action
{

   /**
    * This function is responsible for doing 2-3 things:
    * 1. Fetching the instruction arguments into the emit_data->args array.
    * 2. Setting the number of arguments in emit_data->arg_count.
    * 3. Setting the destination type in emit_data->dst_type (usually only
    *    necessary for opcodes that are TGSI_OUTPUT_COMPONENTWISE).
    */
   void (*fetch_args)(struct lp_build_tgsi_context *,
                      struct lp_build_emit_data *);


   /**
    * This function is responsible for emitting LLVM IR for a TGSI opcode.
    * It should store the values it generates in the emit_data->output array
    * and for TGSI_OUTPUT_COMPONENTWISE and TGSI_OUTPUT_REPLICATE instructions
    * (and possibly others depending on the specific implementation), it should
    * make sure to store the values in the array slot indexed by emit_data->chan.
    */
   void (*emit)(const struct lp_build_tgsi_action *,
                        struct lp_build_tgsi_context *,
                        struct lp_build_emit_data *);

   /**
    * This variable can be used to store an intrinsic name, in case the TGSI
    * opcode will be replaced by a target specific intrinsic.  (There is a
    * convenience function in lp_bld_tgsi.c called lp_build_tgsi_intrinsic()
    * that can be assigned to lp_build_tgsi_action::emit and used for
    * generating intrinsics).
    */
   const char * intr_name;
};

/**
 * This function initializes the bld_base->op_actions array with some
 * generic operand actions.
 */
void
lp_set_default_actions(
   struct lp_build_tgsi_context * bld_base);

/*
 * This function initialize the bld_base->op_actions array with some
 * operand actions that are intended only for use when generating
 * instructions to be executed on a CPU.
 */
void
lp_set_default_actions_cpu(
   struct lp_build_tgsi_context * bld_base);

#endif /* LP_BLD_TGSI_ACTION_H */
