/**************************************************************************
 * 
 * Copyright 2008 VMware, Inc.
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

#ifndef TGSI_INFO_H
#define TGSI_INFO_H

#include "compiler/shader_enums.h"
#include "util/compiler.h"
#include "pipe/p_shader_tokens.h"
#include "util/format/u_format.h"

#if defined __cplusplus
extern "C" {
#endif

/* This enum describes how an opcode calculates its result. */
enum tgsi_output_mode {
   /** The opcode produces no result. */
   TGSI_OUTPUT_NONE            = 0,

   /** When this opcode writes to a channel of the destination register,
    *  it takes as arguments values from the same channel of the source
    *  register(s).
    *
    *  Example: TGSI_OPCODE_ADD
    */
   TGSI_OUTPUT_COMPONENTWISE   = 1,

   /** This opcode writes the same value to all enabled channels of the
    * destination register.
    *
    *  Example: TGSI_OPCODE_RSQ
    */
   TGSI_OUTPUT_REPLICATE       = 2,

   /** The operation performed by this opcode is dependent on which channel
    *  of the destination register is being written.
    *
    *  Example: TGSI_OPCODE_LOG
    */
   TGSI_OUTPUT_CHAN_DEPENDENT  = 3,

   /**
    * Example: TGSI_OPCODE_TEX
    */
   TGSI_OUTPUT_OTHER           = 4
};

struct tgsi_opcode_info
{
   unsigned num_dst:3;
   unsigned num_src:3;
   unsigned is_tex:1;
   unsigned is_store:1;
   unsigned is_branch:1;
   unsigned pre_dedent:1;
   unsigned post_indent:1;
   enum tgsi_output_mode output_mode:4;
   enum tgsi_opcode opcode:10;
};

const struct tgsi_opcode_info *
tgsi_get_opcode_info(enum tgsi_opcode opcode);

const char *
tgsi_get_opcode_name(enum tgsi_opcode opcode);

enum tgsi_opcode_type {
   TGSI_TYPE_UNTYPED, /* for MOV */
   TGSI_TYPE_VOID,
   TGSI_TYPE_UNSIGNED,
   TGSI_TYPE_SIGNED,
   TGSI_TYPE_FLOAT,
   TGSI_TYPE_DOUBLE,
   TGSI_TYPE_UNSIGNED64,
   TGSI_TYPE_SIGNED64,
};

static inline bool tgsi_type_is_64bit(enum tgsi_opcode_type type)
{
   if (type == TGSI_TYPE_DOUBLE || type == TGSI_TYPE_UNSIGNED64 ||
       type == TGSI_TYPE_SIGNED64)
      return true;
   return false;
}

enum tgsi_opcode_type
tgsi_opcode_infer_src_type(enum tgsi_opcode opcode, unsigned src_idx);

enum tgsi_opcode_type
tgsi_opcode_infer_dst_type(enum tgsi_opcode opcode, unsigned dst_idx);

#if defined __cplusplus
}
#endif

#endif /* TGSI_INFO_H */
