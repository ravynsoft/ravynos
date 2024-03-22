/**********************************************************
 * Copyright 2008-2009 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

/**
 * @file
 * SVGA Shader Dump Facilities
 * 
 * @author Michal Krol <michal@vmware.com>
 */

#include <assert.h>
#include <string.h>

#include "svga_shader.h"
#include "svga_shader_dump.h"
#include "svga_shader_op.h"
#include "util/u_debug.h"

#include "../svga_hw_reg.h"
#include "svga3d_shaderdefs.h"

struct dump_info
{
   uint32 version;
   bool is_ps;
   int indent;
};

#define DUMP_MAX_OP_SRC 4

struct dump_op
{
   struct sh_op op;
   struct sh_dstreg dst;
   struct sh_srcreg dstind;
   struct sh_srcreg src[DUMP_MAX_OP_SRC];
   struct sh_srcreg srcind[DUMP_MAX_OP_SRC];
   struct sh_srcreg p0;
};

static void
dump_indent(int indent)
{
   int i;

   for (i = 0; i < indent; ++i) {
      _debug_printf("  ");
   }
}

static void dump_op( struct sh_op op, const char *mnemonic )
{
   assert( op.is_reg == 0 );

   if (op.predicated) {
      _debug_printf("(p0) ");
   }
   if (op.coissue)
      _debug_printf( "+" );
   _debug_printf( "%s", mnemonic );

   switch (op.opcode) {
   case SVGA3DOP_TEX:
      switch (op.control) {
      case 0:
         break;
      case 1 /* PROJECT */:
         _debug_printf("p");
         break;
      case 2 /* BIAS */:
         _debug_printf("b");
         break;
      default:
         assert(0);
      }
      break;

   case SVGA3DOP_IFC:
   case SVGA3DOP_BREAKC:
   case SVGA3DOP_SETP:
      switch (op.control) {
      case SVGA3DOPCOMP_GT:
         _debug_printf("_gt");
         break;
      case SVGA3DOPCOMP_EQ:
         _debug_printf("_eq");
         break;
      case SVGA3DOPCOMP_GE:
         _debug_printf("_ge");
         break;
      case SVGA3DOPCOMP_LT:
         _debug_printf("_lt");
         break;
      case SVGA3DOPCOMPC_NE:
         _debug_printf("_ne");
         break;
      case SVGA3DOPCOMP_LE:
         _debug_printf("_le");
         break;
      default:
         assert(0);
      }
      break;

   default:
      assert(op.control == 0);
   }
}

static void
format_reg(const char *name,
           const struct sh_reg reg,
           const struct sh_srcreg *indreg)
{
   if (reg.relative) {
      assert(indreg);

      if (sh_srcreg_type(*indreg) == SVGA3DREG_LOOP) {
         _debug_printf("%s[aL+%u]", name, reg.number);
      } else {
         _debug_printf("%s[a%u.x+%u]", name, indreg->number, reg.number);
      }
   } else {
      _debug_printf("%s%u", name, reg.number);
   }
}

static void dump_reg( struct sh_reg reg, struct sh_srcreg *indreg, const struct dump_info *di )
{
   assert( reg.is_reg == 1 );

   switch (sh_reg_type( reg )) {
   case SVGA3DREG_TEMP:
      format_reg("r", reg, NULL);
      break;

   case SVGA3DREG_INPUT:
      format_reg("v", reg, indreg);
      break;

   case SVGA3DREG_CONST:
      format_reg("c", reg, indreg);
      break;

   case SVGA3DREG_ADDR:    /* VS */
   /* SVGA3DREG_TEXTURE */ /* PS */
      assert(!reg.relative);
      if (di->is_ps) {
         format_reg("t", reg, NULL);
      } else {
         format_reg("a", reg, NULL);
      }
      break;

   case SVGA3DREG_RASTOUT:
      assert(!reg.relative);
      switch (reg.number) {
      case 0 /*POSITION*/:
         _debug_printf( "oPos" );
         break;
      case 1 /*FOG*/:
         _debug_printf( "oFog" );
         break;
      case 2 /*POINT_SIZE*/:
         _debug_printf( "oPts" );
         break;
      default:
         assert( 0 );
         _debug_printf( "???" );
      }
      break;

   case SVGA3DREG_ATTROUT:
      assert( reg.number < 2 );
      format_reg("oD", reg, NULL);
      break;

   case SVGA3DREG_TEXCRDOUT:  /* VS */
   /* SVGA3DREG_OUTPUT */     /* VS3.0+ */
      if (!di->is_ps && di->version >= SVGA3D_VS_30) {
         format_reg("o", reg, indreg);
      } else {
         format_reg("oT", reg, NULL);
      }
      break;

   case SVGA3DREG_COLOROUT:
      format_reg("oC", reg, NULL);
      break;

   case SVGA3DREG_DEPTHOUT:
      assert(!reg.relative);
      assert(reg.number == 0);
      _debug_printf("oDepth");
      break;

   case SVGA3DREG_SAMPLER:
      format_reg("s", reg, NULL);
      break;

   case SVGA3DREG_CONSTBOOL:
      format_reg("b", reg, NULL);
      break;

   case SVGA3DREG_CONSTINT:
      format_reg("i", reg, NULL);
      break;

   case SVGA3DREG_LOOP:
      assert(!reg.relative);
      assert( reg.number == 0 );
      _debug_printf( "aL" );
      break;

   case SVGA3DREG_MISCTYPE:
      assert(!reg.relative);
      switch (reg.number) {
      case SVGA3DMISCREG_POSITION:
         _debug_printf("vPos");
         break;
      case SVGA3DMISCREG_FACE:
         _debug_printf("vFace");
         break;
      default:
         assert(0);
         _debug_printf("???");
      }
      break;

   case SVGA3DREG_LABEL:
      format_reg("l", reg, NULL);
      break;

   case SVGA3DREG_PREDICATE:
      format_reg("p", reg, NULL);
      break;

   default:
      assert( 0 );
      _debug_printf( "???" );
   }
}

static void dump_cdata( struct sh_cdata cdata )
{
   _debug_printf( "%f, %f, %f, %f", cdata.xyzw[0], cdata.xyzw[1], cdata.xyzw[2], cdata.xyzw[3] );
}

static void dump_idata( struct sh_idata idata )
{
   _debug_printf( "%d, %d, %d, %d", idata.xyzw[0], idata.xyzw[1], idata.xyzw[2], idata.xyzw[3] );
}

static void dump_bdata( bool bdata )
{
   _debug_printf( bdata ? "TRUE" : "FALSE" );
}

static void
dump_sampleinfo(struct sh_sampleinfo sampleinfo)
{
   assert( sampleinfo.is_reg == 1 );

   switch (sampleinfo.texture_type) {
   case SVGA3DSAMP_2D:
      _debug_printf( "_2d" );
      break;
   case SVGA3DSAMP_2D_SHADOW:
      _debug_printf( "_2dshadow" );
      break;
   case SVGA3DSAMP_CUBE:
      _debug_printf( "_cube" );
      break;
   case SVGA3DSAMP_VOLUME:
      _debug_printf( "_volume" );
      break;
   default:
      assert( 0 );
   }
}

static void
dump_semantic(uint usage,
              uint usage_index)
{
   switch (usage) {
   case SVGA3D_DECLUSAGE_POSITION:
      _debug_printf("_position");
      break;
   case SVGA3D_DECLUSAGE_BLENDWEIGHT:
      _debug_printf("_blendweight");
      break;
   case SVGA3D_DECLUSAGE_BLENDINDICES:
      _debug_printf("_blendindices");
      break;
   case SVGA3D_DECLUSAGE_NORMAL:
      _debug_printf("_normal");
      break;
   case SVGA3D_DECLUSAGE_PSIZE:
      _debug_printf("_psize");
      break;
   case SVGA3D_DECLUSAGE_TEXCOORD:
      _debug_printf("_texcoord");
      break;
   case SVGA3D_DECLUSAGE_TANGENT:
      _debug_printf("_tangent");
      break;
   case SVGA3D_DECLUSAGE_BINORMAL:
      _debug_printf("_binormal");
      break;
   case SVGA3D_DECLUSAGE_TESSFACTOR:
      _debug_printf("_tessfactor");
      break;
   case SVGA3D_DECLUSAGE_POSITIONT:
      _debug_printf("_positiont");
      break;
   case SVGA3D_DECLUSAGE_COLOR:
      _debug_printf("_color");
      break;
   case SVGA3D_DECLUSAGE_FOG:
      _debug_printf("_fog");
      break;
   case SVGA3D_DECLUSAGE_DEPTH:
      _debug_printf("_depth");
      break;
   case SVGA3D_DECLUSAGE_SAMPLE:
      _debug_printf("_sample");
      break;
   default:
      assert(!"Unknown usage");
      _debug_printf("_???");
   }

   if (usage_index) {
      _debug_printf("%u", usage_index);
   }
}

static void
dump_dstreg(struct sh_dstreg dstreg,
            struct sh_srcreg *indreg,
            const struct dump_info *di)
{
   union {
      struct sh_reg reg;
      struct sh_dstreg dstreg;
   } u;

   memset(&u, 0, sizeof(u));

   assert( (dstreg.modifier & (SVGA3DDSTMOD_SATURATE | SVGA3DDSTMOD_PARTIALPRECISION)) == dstreg.modifier );

   if (dstreg.modifier & SVGA3DDSTMOD_SATURATE)
      _debug_printf( "_sat" );
   if (dstreg.modifier & SVGA3DDSTMOD_PARTIALPRECISION)
      _debug_printf( "_pp" );
   switch (dstreg.shift_scale) {
   case 0:
      break;
   case 1:
      _debug_printf( "_x2" );
      break;
   case 2:
      _debug_printf( "_x4" );
      break;
   case 3:
      _debug_printf( "_x8" );
      break;
   case 13:
      _debug_printf( "_d8" );
      break;
   case 14:
      _debug_printf( "_d4" );
      break;
   case 15:
      _debug_printf( "_d2" );
      break;
   default:
      assert( 0 );
   }
   _debug_printf( " " );

   u.dstreg = dstreg;
   dump_reg( u.reg, indreg, di);
   if (dstreg.write_mask != SVGA3DWRITEMASK_ALL) {
      _debug_printf( "." );
      if (dstreg.write_mask & SVGA3DWRITEMASK_0)
         _debug_printf( "x" );
      if (dstreg.write_mask & SVGA3DWRITEMASK_1)
         _debug_printf( "y" );
      if (dstreg.write_mask & SVGA3DWRITEMASK_2)
         _debug_printf( "z" );
      if (dstreg.write_mask & SVGA3DWRITEMASK_3)
         _debug_printf( "w" );
   }
}

static void dump_srcreg( struct sh_srcreg srcreg, struct sh_srcreg *indreg, const struct dump_info *di )
{
   struct sh_reg srcreg_sh = {0};
   /* bit-fields carefully aligned, ensure they stay that way. */
   STATIC_ASSERT(sizeof(struct sh_reg) == sizeof(struct sh_srcreg));
   memcpy(&srcreg_sh, &srcreg, sizeof(srcreg_sh));

   switch (srcreg.modifier) {
   case SVGA3DSRCMOD_NEG:
   case SVGA3DSRCMOD_BIASNEG:
   case SVGA3DSRCMOD_SIGNNEG:
   case SVGA3DSRCMOD_X2NEG:
   case SVGA3DSRCMOD_ABSNEG:
      _debug_printf( "-" );
      break;
   case SVGA3DSRCMOD_COMP:
      _debug_printf( "1-" );
      break;
   case SVGA3DSRCMOD_NOT:
      _debug_printf( "!" );
   }
   dump_reg(srcreg_sh, indreg, di );
   switch (srcreg.modifier) {
   case SVGA3DSRCMOD_NONE:
   case SVGA3DSRCMOD_NEG:
   case SVGA3DSRCMOD_COMP:
   case SVGA3DSRCMOD_NOT:
      break;
   case SVGA3DSRCMOD_BIAS:
   case SVGA3DSRCMOD_BIASNEG:
      _debug_printf( "_bias" );
      break;
   case SVGA3DSRCMOD_SIGN:
   case SVGA3DSRCMOD_SIGNNEG:
      _debug_printf( "_bx2" );
      break;
   case SVGA3DSRCMOD_X2:
   case SVGA3DSRCMOD_X2NEG:
      _debug_printf( "_x2" );
      break;
   case SVGA3DSRCMOD_DZ:
      _debug_printf( "_dz" );
      break;
   case SVGA3DSRCMOD_DW:
      _debug_printf( "_dw" );
      break;
   case SVGA3DSRCMOD_ABS:
   case SVGA3DSRCMOD_ABSNEG:
      _debug_printf("_abs");
      break;
   default:
      assert( 0 );
   }
   if (srcreg.swizzle_x != 0 || srcreg.swizzle_y != 1 || srcreg.swizzle_z != 2 || srcreg.swizzle_w != 3) {
      _debug_printf( "." );
      if (srcreg.swizzle_x == srcreg.swizzle_y && srcreg.swizzle_y == srcreg.swizzle_z && srcreg.swizzle_z == srcreg.swizzle_w) {
         _debug_printf( "%c", "xyzw"[srcreg.swizzle_x] );
      }
      else {
         _debug_printf( "%c", "xyzw"[srcreg.swizzle_x] );
         _debug_printf( "%c", "xyzw"[srcreg.swizzle_y] );
         _debug_printf( "%c", "xyzw"[srcreg.swizzle_z] );
         _debug_printf( "%c", "xyzw"[srcreg.swizzle_w] );
      }
   }
}

static void
parse_op(struct dump_info *di,
         const uint **token,
         struct dump_op *op,
         uint num_dst,
         uint num_src)
{
   uint i;

   assert(num_dst <= 1);
   assert(num_src <= DUMP_MAX_OP_SRC);

   op->op = *(struct sh_op *)*token;
   *token += sizeof(struct sh_op) / sizeof(uint);

   if (num_dst >= 1) {
      op->dst = *(struct sh_dstreg *)*token;
      *token += sizeof(struct sh_dstreg) / sizeof(uint);
      if (op->dst.relative &&
          (!di->is_ps && di->version >= SVGA3D_VS_30)) {
         op->dstind = *(struct sh_srcreg *)*token;
         *token += sizeof(struct sh_srcreg) / sizeof(uint);
      }
   }

   if (op->op.predicated) {
      op->p0 = *(struct sh_srcreg *)*token;
      *token += sizeof(struct sh_srcreg) / sizeof(uint);
   }

   for (i = 0; i < num_src; ++i) {
      op->src[i] = *(struct sh_srcreg *)*token;
      *token += sizeof(struct sh_srcreg) / sizeof(uint);
      if (op->src[i].relative &&
          ((!di->is_ps && di->version >= SVGA3D_VS_20) ||
          (di->is_ps && di->version >= SVGA3D_PS_30))) {
         op->srcind[i] = *(struct sh_srcreg *)*token;
         *token += sizeof(struct sh_srcreg) / sizeof(uint);
      }
   }
}

static void
dump_inst(struct dump_info *di,
          const unsigned **assem,
          struct sh_op op,
          const struct sh_opcode_info *info)
{
   struct dump_op dop;
   bool not_first_arg = false;
   uint i;

   assert(info->num_dst <= 1);

   di->indent -= info->pre_dedent;
   dump_indent(di->indent);
   di->indent += info->post_indent;

   dump_op(op, info->mnemonic);

   parse_op(di, assem, &dop, info->num_dst, info->num_src);
   if (info->num_dst > 0) {
      dump_dstreg(dop.dst, &dop.dstind, di);
      not_first_arg = true;
   }

   for (i = 0; i < info->num_src; i++) {
      if (not_first_arg) {
         _debug_printf(", ");
      } else {
         _debug_printf(" ");
      }
      dump_srcreg(dop.src[i], &dop.srcind[i], di);
      not_first_arg = true;
   }

   _debug_printf("\n");
}

void
svga_shader_dump(
   const unsigned *assem,
   unsigned dwords,
   unsigned do_binary )
{
   bool finished = false;
   struct dump_info di;

   di.version = *assem++;
   di.is_ps = (di.version & 0xFFFF0000) == 0xFFFF0000;
   di.indent = 0;

   _debug_printf(
      "%s_%u_%u\n",
      di.is_ps ? "ps" : "vs",
      (di.version >> 8) & 0xff,
      di.version & 0xff );

   while (!finished) {
      struct sh_op op = *(struct sh_op *) assem;

      switch (op.opcode) {
      case SVGA3DOP_DCL:
         {
            struct sh_dcl dcl = *(struct sh_dcl *) assem;

            _debug_printf( "dcl" );
            switch (sh_dstreg_type(dcl.reg)) {
            case SVGA3DREG_INPUT:
               if ((di.is_ps && di.version >= SVGA3D_PS_30) ||
                   (!di.is_ps && di.version >= SVGA3D_VS_30)) {
                  dump_semantic(dcl.u.semantic.usage,
                                dcl.u.semantic.usage_index);
               }
               break;
            case SVGA3DREG_TEXCRDOUT:
               if (!di.is_ps && di.version >= SVGA3D_VS_30) {
                  dump_semantic(dcl.u.semantic.usage,
                                dcl.u.semantic.usage_index);
               }
               break;
            case SVGA3DREG_SAMPLER:
               dump_sampleinfo( dcl.u.sampleinfo );
               break;
            }
            dump_dstreg(dcl.reg, NULL, &di);
            _debug_printf( "\n" );
            assem += sizeof( struct sh_dcl ) / sizeof( unsigned );
         }
         break;

      case SVGA3DOP_DEFB:
         {
            struct sh_defb defb = *(struct sh_defb *) assem;

            _debug_printf( "defb " );
            dump_reg( defb.reg, NULL, &di );
            _debug_printf( ", " );
            dump_bdata( defb.data );
            _debug_printf( "\n" );
            assem += sizeof( struct sh_defb ) / sizeof( unsigned );
         }
         break;

      case SVGA3DOP_DEFI:
         {
            struct sh_defi defi = *(struct sh_defi *) assem;

            _debug_printf( "defi " );
            dump_reg( defi.reg, NULL, &di );
            _debug_printf( ", " );
            dump_idata( defi.idata );
            _debug_printf( "\n" );
            assem += sizeof( struct sh_defi ) / sizeof( unsigned );
         }
         break;

      case SVGA3DOP_TEXCOORD:
         {
            struct sh_opcode_info info = *svga_opcode_info(op.opcode);

            assert(di.is_ps);
            if (di.version > SVGA3D_PS_13) {
               assert(info.num_src == 0);

               info.num_src = 1;
            }

            dump_inst(&di, &assem, op, &info);
         }
         break;

      case SVGA3DOP_TEX:
         {
            struct sh_opcode_info info = *svga_opcode_info(op.opcode);

            assert(di.is_ps);
            if (di.version > SVGA3D_PS_13) {
               assert(info.num_src == 0);

               if (di.version > SVGA3D_PS_14) {
                  info.num_src = 2;
                  info.mnemonic = "texld";
               } else {
                  info.num_src = 1;
               }
            }

            dump_inst(&di, &assem, op, &info);
         }
         break;

      case SVGA3DOP_DEF:
         {
            struct sh_def def = *(struct sh_def *) assem;

            _debug_printf( "def " );
            dump_reg( def.reg, NULL, &di );
            _debug_printf( ", " );
            dump_cdata( def.cdata );
            _debug_printf( "\n" );
            assem += sizeof( struct sh_def ) / sizeof( unsigned );
         }
         break;

      case SVGA3DOP_SINCOS:
         {
            struct sh_opcode_info info = *svga_opcode_info(op.opcode);

            if ((di.is_ps && di.version >= SVGA3D_PS_30) ||
                (!di.is_ps && di.version >= SVGA3D_VS_30)) {
               assert(info.num_src == 3);

               info.num_src = 1;
            }

            dump_inst(&di, &assem, op, &info);
         }
         break;

      case SVGA3DOP_PHASE:
         _debug_printf( "phase\n" );
         assem += sizeof( struct sh_op ) / sizeof( unsigned );
         break;

      case SVGA3DOP_COMMENT:
         {
            struct sh_comment comment = *(struct sh_comment *)assem;

            /* Ignore comment contents. */
            assem += sizeof(struct sh_comment) / sizeof(unsigned) + comment.size;
         }
         break;

      case SVGA3DOP_END:
         finished = true;
         break;

      default:
         {
            const struct sh_opcode_info *info = svga_opcode_info(op.opcode);

            dump_inst(&di, &assem, op, info);
         }
      }
   }
}
