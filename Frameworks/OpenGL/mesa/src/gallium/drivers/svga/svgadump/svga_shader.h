/**********************************************************
 * Copyright 2007-2009 VMware, Inc.  All rights reserved.
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
 * SVGA Shader Token Definitions
 * 
 * @author Michal Krol <michal@vmware.com>
 */

#ifndef ST_SHADER_SVGA_H
#define ST_SHADER_SVGA_H

#include "util/compiler.h"

struct sh_op
{
   unsigned opcode:16;
   unsigned control:8;
   unsigned length:4;
   unsigned predicated:1;
   unsigned unused:1;
   unsigned coissue:1;
   unsigned is_reg:1;
};

struct sh_reg
{
   unsigned number:11;
   unsigned type_hi:2;
   unsigned relative:1;
   unsigned unused:14;
   unsigned type_lo:3;
   unsigned is_reg:1;
};

static inline unsigned
sh_reg_type( struct sh_reg reg )
{
   return reg.type_lo | (reg.type_hi << 3);
}

struct sh_cdata
{
   float xyzw[4];
};

struct sh_def
{
   struct sh_op op;
   struct sh_reg reg;
   struct sh_cdata cdata;
};

struct sh_defb
{
   struct sh_op op;
   struct sh_reg reg;
   uint data;
};

struct sh_idata
{
   int xyzw[4];
};

struct sh_defi
{
   struct sh_op op;
   struct sh_reg reg;
   struct sh_idata idata;
};

#define PS_TEXTURETYPE_UNKNOWN   SVGA3DSAMP_UNKNOWN
#define PS_TEXTURETYPE_2D        SVGA3DSAMP_2D
#define PS_TEXTURETYPE_CUBE      SVGA3DSAMP_CUBE
#define PS_TEXTURETYPE_VOLUME    SVGA3DSAMP_VOLUME

struct sh_sampleinfo
{
   unsigned unused:27;
   unsigned texture_type:4;
   unsigned is_reg:1;
};

struct sh_semantic
{
   unsigned usage:4;
   unsigned unused1:12;
   unsigned usage_index:4;
   unsigned unused2:11;
   unsigned is_reg:1;
};

#define SH_WRITEMASK_0              0x1
#define SH_WRITEMASK_1              0x2
#define SH_WRITEMASK_2              0x4
#define SH_WRITEMASK_3              0x8
#define SH_WRITEMASK_ALL            0xf

#define SH_DSTMOD_NONE              0x0
#define SH_DSTMOD_SATURATE          0x1
#define SH_DSTMOD_PARTIALPRECISION  0x2
#define SH_DSTMOD_MSAMPCENTROID     0x4

struct sh_dstreg
{
   unsigned number:11;
   unsigned type_hi:2;
   unsigned relative:1;
   unsigned unused:2;
   unsigned write_mask:4;
   unsigned modifier:4;
   unsigned shift_scale:4;
   unsigned type_lo:3;
   unsigned is_reg:1;
};

static inline unsigned
sh_dstreg_type( struct sh_dstreg reg )
{
   return reg.type_lo | (reg.type_hi << 3);
}

struct sh_dcl
{
   struct sh_op op;
   union {
      struct sh_sampleinfo sampleinfo;
      struct sh_semantic semantic;
   } u;
   struct sh_dstreg reg;
};

struct sh_srcreg
{
   unsigned number:11;
   unsigned type_hi:2;
   unsigned relative:1;
   unsigned unused:2;
   unsigned swizzle_x:2;
   unsigned swizzle_y:2;
   unsigned swizzle_z:2;
   unsigned swizzle_w:2;
   unsigned modifier:4;
   unsigned type_lo:3;
   unsigned is_reg:1;
};

static inline unsigned
sh_srcreg_type( struct sh_srcreg reg )
{
   return reg.type_lo | (reg.type_hi << 3);
}

struct sh_dstop
{
   struct sh_op op;
   struct sh_dstreg dst;
};

struct sh_srcop
{
   struct sh_op op;
   struct sh_srcreg src;
};

struct sh_src2op
{
   struct sh_op op;
   struct sh_srcreg src0;
   struct sh_srcreg src1;
};

struct sh_unaryop
{
   struct sh_op op;
   struct sh_dstreg dst;
   struct sh_srcreg src;
};

struct sh_binaryop
{
   struct sh_op op;
   struct sh_dstreg dst;
   struct sh_srcreg src0;
   struct sh_srcreg src1;
};

struct sh_trinaryop
{
   struct sh_op op;
   struct sh_dstreg dst;
   struct sh_srcreg src0;
   struct sh_srcreg src1;
   struct sh_srcreg src2;
};

struct sh_comment
{
   unsigned opcode:16;
   unsigned size:16;
};

#endif /* ST_SHADER_SVGA_H */
