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

#include "util/u_debug.h"
#include "util/u_memory.h"
#include "tgsi_info.h"

#define NONE TGSI_OUTPUT_NONE
#define COMP TGSI_OUTPUT_COMPONENTWISE
#define REPL TGSI_OUTPUT_REPLICATE
#define CHAN TGSI_OUTPUT_CHAN_DEPENDENT
#define OTHR TGSI_OUTPUT_OTHER

#define OPCODE(_num_dst, _num_src, _output_mode, name, ...) \
   { .opcode = TGSI_OPCODE_ ## name, \
     .output_mode = _output_mode, .num_dst = _num_dst, .num_src = _num_src, \
     ##__VA_ARGS__ },

#define OPCODE_GAP(opc) { .opcode = opc },

static const struct tgsi_opcode_info opcode_info[TGSI_OPCODE_LAST] =
{
#include "tgsi_info_opcodes.h"
};

#undef OPCODE
#undef OPCODE_GAP

const struct tgsi_opcode_info *
tgsi_get_opcode_info(enum tgsi_opcode opcode)
{
   static bool firsttime = 1;

   ASSERT_BITFIELD_SIZE(struct tgsi_opcode_info, opcode, TGSI_OPCODE_LAST - 1);
   ASSERT_BITFIELD_SIZE(struct tgsi_opcode_info, output_mode,
                        TGSI_OUTPUT_OTHER);

   if (firsttime) {
      unsigned i;
      firsttime = 0;
      for (i = 0; i < ARRAY_SIZE(opcode_info); i++)
         assert(opcode_info[i].opcode == i);
   }
   
   if (opcode < TGSI_OPCODE_LAST)
      return &opcode_info[opcode];

   assert( 0 );
   return NULL;
}

#define OPCODE(_num_dst, _num_src, _output_mode, name, ...) #name,
#define OPCODE_GAP(opc) "UNK" #opc,

static const char * const opcode_names[TGSI_OPCODE_LAST] =
{
#include "tgsi_info_opcodes.h"
};

#undef OPCODE
#undef OPCODE_GAP

const char *
tgsi_get_opcode_name(enum tgsi_opcode opcode)
{
   if (opcode >= ARRAY_SIZE(opcode_names))
      return "UNK_OOB";
   return opcode_names[opcode];
}


/**
 * Infer the type (of the dst) of the opcode.
 *
 * MOV and UCMP is special so return VOID
 */
static inline enum tgsi_opcode_type
tgsi_opcode_infer_type(enum tgsi_opcode opcode)
{
   switch (opcode) {
   case TGSI_OPCODE_MOV:
   case TGSI_OPCODE_UCMP:
      return TGSI_TYPE_UNTYPED;
   case TGSI_OPCODE_NOT:
   case TGSI_OPCODE_SHL:
   case TGSI_OPCODE_AND:
   case TGSI_OPCODE_OR:
   case TGSI_OPCODE_XOR:
   case TGSI_OPCODE_TXQ:
   case TGSI_OPCODE_TXQS:
   case TGSI_OPCODE_F2U:
   case TGSI_OPCODE_UDIV:
   case TGSI_OPCODE_UMAD:
   case TGSI_OPCODE_UMAX:
   case TGSI_OPCODE_UMIN:
   case TGSI_OPCODE_UMOD:
   case TGSI_OPCODE_UMUL:
   case TGSI_OPCODE_USEQ:
   case TGSI_OPCODE_USGE:
   case TGSI_OPCODE_USHR:
   case TGSI_OPCODE_USLT:
   case TGSI_OPCODE_USNE:
   case TGSI_OPCODE_SVIEWINFO:
   case TGSI_OPCODE_UMUL_HI:
   case TGSI_OPCODE_UBFE:
   case TGSI_OPCODE_BFI:
   case TGSI_OPCODE_BREV:
   case TGSI_OPCODE_IMG2HND:
   case TGSI_OPCODE_SAMP2HND:
      return TGSI_TYPE_UNSIGNED;
   case TGSI_OPCODE_ARL:
   case TGSI_OPCODE_ARR:
   case TGSI_OPCODE_MOD:
   case TGSI_OPCODE_F2I:
   case TGSI_OPCODE_FSEQ:
   case TGSI_OPCODE_FSGE:
   case TGSI_OPCODE_FSLT:
   case TGSI_OPCODE_FSNE:
   case TGSI_OPCODE_IDIV:
   case TGSI_OPCODE_IMAX:
   case TGSI_OPCODE_IMIN:
   case TGSI_OPCODE_INEG:
   case TGSI_OPCODE_ISGE:
   case TGSI_OPCODE_ISHR:
   case TGSI_OPCODE_ISLT:
   case TGSI_OPCODE_UADD:
   case TGSI_OPCODE_UARL:
   case TGSI_OPCODE_IABS:
   case TGSI_OPCODE_ISSG:
   case TGSI_OPCODE_IMUL_HI:
   case TGSI_OPCODE_IBFE:
   case TGSI_OPCODE_IMSB:
   case TGSI_OPCODE_DSEQ:
   case TGSI_OPCODE_DSGE:
   case TGSI_OPCODE_DSLT:
   case TGSI_OPCODE_DSNE:
   case TGSI_OPCODE_U64SEQ:
   case TGSI_OPCODE_U64SNE:
   case TGSI_OPCODE_U64SLT:
   case TGSI_OPCODE_U64SGE:
   case TGSI_OPCODE_I64SLT:
   case TGSI_OPCODE_I64SGE:
   case TGSI_OPCODE_LSB:
   case TGSI_OPCODE_POPC:
   case TGSI_OPCODE_UMSB:
      return TGSI_TYPE_SIGNED;
   case TGSI_OPCODE_DADD:
   case TGSI_OPCODE_DABS:
   case TGSI_OPCODE_DFMA:
   case TGSI_OPCODE_DNEG:
   case TGSI_OPCODE_DMUL:
   case TGSI_OPCODE_DMAX:
   case TGSI_OPCODE_DDIV:
   case TGSI_OPCODE_DMIN:
   case TGSI_OPCODE_DRCP:
   case TGSI_OPCODE_DSQRT:
   case TGSI_OPCODE_DMAD:
   case TGSI_OPCODE_DLDEXP:
   case TGSI_OPCODE_DFRAC:
   case TGSI_OPCODE_DRSQ:
   case TGSI_OPCODE_DTRUNC:
   case TGSI_OPCODE_DCEIL:
   case TGSI_OPCODE_DFLR:
   case TGSI_OPCODE_DROUND:
   case TGSI_OPCODE_DSSG:
   case TGSI_OPCODE_F2D:
   case TGSI_OPCODE_I2D:
   case TGSI_OPCODE_U2D:
   case TGSI_OPCODE_U642D:
   case TGSI_OPCODE_I642D:
      return TGSI_TYPE_DOUBLE;
   case TGSI_OPCODE_U64MAX:
   case TGSI_OPCODE_U64MIN:
   case TGSI_OPCODE_U64ADD:
   case TGSI_OPCODE_U64MUL:
   case TGSI_OPCODE_U64DIV:
   case TGSI_OPCODE_U64MOD:
   case TGSI_OPCODE_U64SHL:
   case TGSI_OPCODE_U64SHR:
   case TGSI_OPCODE_F2U64:
   case TGSI_OPCODE_D2U64:
      return TGSI_TYPE_UNSIGNED64;
   case TGSI_OPCODE_I64MAX:
   case TGSI_OPCODE_I64MIN:
   case TGSI_OPCODE_I64ABS:
   case TGSI_OPCODE_I64SSG:
   case TGSI_OPCODE_I64NEG:
   case TGSI_OPCODE_I64SHR:
   case TGSI_OPCODE_I64DIV:
   case TGSI_OPCODE_I64MOD:
   case TGSI_OPCODE_F2I64:
   case TGSI_OPCODE_U2I64:
   case TGSI_OPCODE_I2I64:
   case TGSI_OPCODE_D2I64:
      return TGSI_TYPE_SIGNED64;
   default:
      return TGSI_TYPE_FLOAT;
   }
}

/*
 * infer the source type of a TGSI opcode.
 */
enum tgsi_opcode_type
tgsi_opcode_infer_src_type(enum tgsi_opcode opcode, unsigned src_idx)
{
   if (src_idx == 1 &&
       (opcode == TGSI_OPCODE_DLDEXP || opcode == TGSI_OPCODE_LDEXP))
      return TGSI_TYPE_SIGNED;

   if (src_idx == 1 &&
       (opcode == TGSI_OPCODE_LOAD))
      return TGSI_TYPE_UNSIGNED;

   if (src_idx == 0 &&
       (opcode == TGSI_OPCODE_STORE))
      return TGSI_TYPE_UNSIGNED;

   if (src_idx == 1 &&
       ((opcode >= TGSI_OPCODE_ATOMUADD && opcode <= TGSI_OPCODE_ATOMIMAX) ||
       opcode == TGSI_OPCODE_ATOMINC_WRAP || opcode == TGSI_OPCODE_ATOMDEC_WRAP))
      return TGSI_TYPE_UNSIGNED;

   switch (opcode) {
   case TGSI_OPCODE_UIF:
   case TGSI_OPCODE_TXF:
   case TGSI_OPCODE_TXF_LZ:
   case TGSI_OPCODE_U2F:
   case TGSI_OPCODE_U2D:
   case TGSI_OPCODE_UADD:
   case TGSI_OPCODE_SWITCH:
   case TGSI_OPCODE_CASE:
   case TGSI_OPCODE_SAMPLE_I:
   case TGSI_OPCODE_SAMPLE_I_MS:
   case TGSI_OPCODE_UMUL_HI:
   case TGSI_OPCODE_UP2H:
   case TGSI_OPCODE_U2I64:
   case TGSI_OPCODE_MEMBAR:
   case TGSI_OPCODE_UMSB:
      return TGSI_TYPE_UNSIGNED;
   case TGSI_OPCODE_IMUL_HI:
   case TGSI_OPCODE_I2F:
   case TGSI_OPCODE_I2D:
   case TGSI_OPCODE_I2I64:
      return TGSI_TYPE_SIGNED;
   case TGSI_OPCODE_ARL:
   case TGSI_OPCODE_ARR:
   case TGSI_OPCODE_F2D:
   case TGSI_OPCODE_F2I:
   case TGSI_OPCODE_F2U:
   case TGSI_OPCODE_FSEQ:
   case TGSI_OPCODE_FSGE:
   case TGSI_OPCODE_FSLT:
   case TGSI_OPCODE_FSNE:
   case TGSI_OPCODE_UCMP:
   case TGSI_OPCODE_F2U64:
   case TGSI_OPCODE_F2I64:
      return TGSI_TYPE_FLOAT;
   case TGSI_OPCODE_D2F:
   case TGSI_OPCODE_D2U:
   case TGSI_OPCODE_D2I:
   case TGSI_OPCODE_DSEQ:
   case TGSI_OPCODE_DSGE:
   case TGSI_OPCODE_DSLT:
   case TGSI_OPCODE_DSNE:
   case TGSI_OPCODE_D2U64:
   case TGSI_OPCODE_D2I64:
      return TGSI_TYPE_DOUBLE;
   case TGSI_OPCODE_U64SEQ:
   case TGSI_OPCODE_U64SNE:
   case TGSI_OPCODE_U64SLT:
   case TGSI_OPCODE_U64SGE:
   case TGSI_OPCODE_U642F:
   case TGSI_OPCODE_U642D:
      return TGSI_TYPE_UNSIGNED64;
   case TGSI_OPCODE_I64SLT:
   case TGSI_OPCODE_I64SGE:
   case TGSI_OPCODE_I642F:
   case TGSI_OPCODE_I642D:
            return TGSI_TYPE_SIGNED64;
   default:
      return tgsi_opcode_infer_type(opcode);
   }
}

/*
 * infer the destination type of a TGSI opcode.
 */
enum tgsi_opcode_type
tgsi_opcode_infer_dst_type(enum tgsi_opcode opcode, unsigned dst_idx)
{
   return tgsi_opcode_infer_type(opcode);
}
