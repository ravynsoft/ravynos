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
 * SVGA Shader Token Opcode Info
 * 
 * @author Michal Krol <michal@vmware.com>
 */

#include "util/u_debug.h"
#include "svga_shader_op.h"

#include "../svga_hw_reg.h"
#include "svga3d_shaderdefs.h"

#define SVGA3DOP_INVALID SVGA3DOP_END
#define TGSI_OPCODE_INVALID TGSI_OPCODE_LAST

static struct sh_opcode_info opcode_info[] =
{
   { "nop",          0, 0, 0, 0, SVGA3DOP_NOP          },
   { "mov",          1, 1, 0, 0, SVGA3DOP_MOV,         },
   { "add",          1, 2, 0, 0, SVGA3DOP_ADD,         },
   { "sub",          1, 2, 0, 0, SVGA3DOP_SUB,         },
   { "mad",          1, 3, 0, 0, SVGA3DOP_MAD,         },
   { "mul",          1, 2, 0, 0, SVGA3DOP_MUL,         },
   { "rcp",          1, 1, 0, 0, SVGA3DOP_RCP,         },
   { "rsq",          1, 1, 0, 0, SVGA3DOP_RSQ,         },
   { "dp3",          1, 2, 0, 0, SVGA3DOP_DP3,         },
   { "dp4",          1, 2, 0, 0, SVGA3DOP_DP4,         },
   { "min",          1, 2, 0, 0, SVGA3DOP_MIN,         },
   { "max",          1, 2, 0, 0, SVGA3DOP_MAX,         },
   { "slt",          1, 2, 0, 0, SVGA3DOP_SLT,         },
   { "sge",          1, 2, 0, 0, SVGA3DOP_SGE,         },
   { "exp",          1, 1, 0, 0, SVGA3DOP_EXP,         },
   { "log",          1, 1, 0, 0, SVGA3DOP_LOG,         },
   { "lit",          1, 1, 0, 0, SVGA3DOP_LIT,         },
   { "dst",          1, 2, 0, 0, SVGA3DOP_DST,         },
   { "lrp",          1, 3, 0, 0, SVGA3DOP_LRP,         },
   { "frc",          1, 1, 0, 0, SVGA3DOP_FRC,         },
   { "m4x4",         1, 2, 0, 0, SVGA3DOP_M4x4,        },
   { "m4x3",         1, 2, 0, 0, SVGA3DOP_M4x3,        },
   { "m3x4",         1, 2, 0, 0, SVGA3DOP_M3x4,        },
   { "m3x3",         1, 2, 0, 0, SVGA3DOP_M3x3,        },
   { "m3x2",         1, 2, 0, 0, SVGA3DOP_M3x2,        },
   { "call",         0, 1, 0, 0, SVGA3DOP_CALL,        },
   { "callnz",       0, 2, 0, 0, SVGA3DOP_CALLNZ,      },
   { "loop",         0, 2, 0, 1, SVGA3DOP_LOOP,        },
   { "ret",          0, 0, 0, 0, SVGA3DOP_RET,         },
   { "endloop",      0, 0, 1, 0, SVGA3DOP_ENDLOOP,     },
   { "label",        0, 1, 0, 0, SVGA3DOP_LABEL,       },
   { "dcl",          0, 0, 0, 0, SVGA3DOP_DCL,         },
   { "pow",          1, 2, 0, 0, SVGA3DOP_POW,         },
   { "crs",          1, 2, 0, 0, SVGA3DOP_CRS,         },
   { "sgn",          1, 3, 0, 0, SVGA3DOP_SGN,         },
   { "abs",          1, 1, 0, 0, SVGA3DOP_ABS,         },
   { "nrm",          1, 1, 0, 0, SVGA3DOP_NRM,         }, /* 3-componenet normalization */
   { "sincos",       1, 3, 0, 0, SVGA3DOP_SINCOS,      },
   { "rep",          0, 1, 0, 1, SVGA3DOP_REP,         },
   { "endrep",       0, 0, 1, 0, SVGA3DOP_ENDREP,      },
   { "if",           0, 1, 0, 1, SVGA3DOP_IF,          },
   { "ifc",          0, 2, 0, 1, SVGA3DOP_IFC,         },
   { "else",         0, 0, 1, 1, SVGA3DOP_ELSE,        },
   { "endif",        0, 0, 1, 0, SVGA3DOP_ENDIF,       },
   { "break",        0, 0, 0, 0, SVGA3DOP_BREAK,       },
   { "breakc",       0, 2, 0, 0, SVGA3DOP_BREAKC,      },
   { "mova",         1, 1, 0, 0, SVGA3DOP_MOVA,        },
   { "defb",         0, 0, 0, 0, SVGA3DOP_DEFB,        },
   { "defi",         0, 0, 0, 0, SVGA3DOP_DEFI,        },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "???",          0, 0, 0, 0, SVGA3DOP_INVALID,     },
   { "texcoord",     1, 0, 0, 0, SVGA3DOP_TEXCOORD,    },
   { "texkill",      1, 0, 0, 0, SVGA3DOP_TEXKILL,     },
   { "tex",          1, 0, 0, 0, SVGA3DOP_TEX,         },
   { "texbem",       1, 1, 0, 0, SVGA3DOP_TEXBEM,      },
   { "texbeml",      1, 1, 0, 0, SVGA3DOP_TEXBEML,     },
   { "texreg2ar",    1, 1, 0, 0, SVGA3DOP_TEXREG2AR,   },
   { "texreg2gb",    1, 1, 0, 0, SVGA3DOP_TEXREG2GB,   },
   { "texm3x2pad",   1, 1, 0, 0, SVGA3DOP_TEXM3x2PAD,  },
   { "texm3x2tex",   1, 1, 0, 0, SVGA3DOP_TEXM3x2TEX,  },
   { "texm3x3pad",   1, 1, 0, 0, SVGA3DOP_TEXM3x3PAD,  },
   { "texm3x3tex",   1, 1, 0, 0, SVGA3DOP_TEXM3x3TEX,  },
   { "reserved0",    0, 0, 0, 0, SVGA3DOP_RESERVED0,   },
   { "texm3x3spec",  1, 2, 0, 0, SVGA3DOP_TEXM3x3SPEC, },
   { "texm3x3vspec", 1, 1, 0, 0, SVGA3DOP_TEXM3x3VSPEC,},
   { "expp",         1, 1, 0, 0, SVGA3DOP_EXPP,        },
   { "logp",         1, 1, 0, 0, SVGA3DOP_LOGP,        },
   { "cnd",          1, 3, 0, 0, SVGA3DOP_CND,         },
   { "def",          0, 0, 0, 0, SVGA3DOP_DEF,         },
   { "texreg2rgb",   1, 1, 0, 0, SVGA3DOP_TEXREG2RGB,  },
   { "texdp3tex",    1, 1, 0, 0, SVGA3DOP_TEXDP3TEX,   },
   { "texm3x2depth", 1, 1, 0, 0, SVGA3DOP_TEXM3x2DEPTH,},
   { "texdp3",       1, 1, 0, 0, SVGA3DOP_TEXDP3,      },
   { "texm3x3",      1, 1, 0, 0, SVGA3DOP_TEXM3x3,     },
   { "texdepth",     1, 0, 0, 0, SVGA3DOP_TEXDEPTH,    },
   { "cmp",          1, 3, 0, 0, SVGA3DOP_CMP,         },
   { "bem",          1, 2, 0, 0, SVGA3DOP_BEM,         },
   { "dp2add",       1, 3, 0, 0, SVGA3DOP_DP2ADD,      },
   { "dsx",          1, 1, 0, 0, SVGA3DOP_INVALID,     },
   { "dsy",          1, 1, 0, 0, SVGA3DOP_INVALID,     },
   { "texldd",       1, 4, 0, 0, SVGA3DOP_INVALID,     },
   { "setp",         1, 2, 0, 0, SVGA3DOP_SETP,        },
   { "texldl",       1, 2, 0, 0, SVGA3DOP_TEXLDL,      },
   { "breakp",       0, 1, 0, 0, SVGA3DOP_INVALID,     },
};

const struct sh_opcode_info *svga_opcode_info( uint op )
{
   struct sh_opcode_info *info;

   if (op >= ARRAY_SIZE(opcode_info)) {
      /* The opcode is either PHASE, COMMENT, END or out of range.
       */
      assert( 0 );
      return NULL;
   }

   info = &opcode_info[op];

   if (info->svga_opcode == SVGA3DOP_INVALID) {
      /* No valid information. Please provide number of dst/src registers.
       */
      _debug_printf("Missing information for opcode %u, '%s'\n", op,
                    opcode_info[op].mnemonic);
      assert( 0 );
      return NULL;
   }

   /* Sanity check.
    */
   assert( op == info->svga_opcode );

   return info;
}
