/* TILEPro opcode information.

   Copyright (C) 2011-2023 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"

/* This define is BFD_RELOC_##x for real bfd, or -1 for everyone else.  */
#define BFD_RELOC(x) BFD_RELOC_##x
#include "bfd.h"

/* Special registers.  */
#define TREG_LR 55
#define TREG_SN 56
#define TREG_ZERO 63

#if defined(__KERNEL__) || defined(_LIBC)
/* FIXME: Rename this. */
#include <asm/opcode-tile.h>
#define DISASM_ONLY
#else
#include "opcode/tilepro.h"
#endif

#ifdef __KERNEL__
#include <linux/stddef.h>
#else
#include <stddef.h>
#endif

const struct tilepro_opcode tilepro_opcodes[397] =
{
 { "bpt", TILEPRO_OPC_BPT, 0x2, 0, TREG_ZERO, 0,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbffffff80000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b3cae00000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "info", TILEPRO_OPC_INFO, 0xf, 1, TREG_ZERO, 1,
    { { 0 }, { 1 }, { 2 }, { 3 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00fffULL,
      0xfff807ff80000000ULL,
      0x8000000078000fffULL,
      0xf80007ff80000000ULL,
      0ULL
    },
    {
      0x0000000050100fffULL,
      0x302007ff80000000ULL,
      0x8000000050000fffULL,
      0xc00007ff80000000ULL,
      -1ULL
    }
#endif
  },
  { "infol", TILEPRO_OPC_INFOL, 0x3, 1, TREG_ZERO, 1,
    { { 4 }, { 5 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000fffULL,
      0xf80007ff80000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000030000fffULL,
      0x200007ff80000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "j", TILEPRO_OPC_J, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 6 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xf000000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x5000000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jal", TILEPRO_OPC_JAL, 0x2, 1, TREG_LR, 1,
    { { 0, }, { 6 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xf000000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x6000000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lw_tls", TILEPRO_OPC_LW_TLS, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30d0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lw_tls.sn", TILEPRO_OPC_LW_TLS_SN, 0x2, 3, TREG_SN, 1,
    { { 0, }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x34d0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "move", TILEPRO_OPC_MOVE, 0xf, 2, TREG_ZERO, 1,
    { { 9, 10 }, { 7, 8 }, { 11, 12 }, { 13, 14 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0xfffff80000000000ULL,
      0x80000000780ff000ULL,
      0xf807f80000000000ULL,
      0ULL
    },
    {
      0x0000000000cff000ULL,
      0x0833f80000000000ULL,
      0x80000000180bf000ULL,
      0x9805f80000000000ULL,
      -1ULL
    }
#endif
  },
  { "move.sn", TILEPRO_OPC_MOVE_SN, 0x3, 2, TREG_SN, 1,
    { { 9, 10 }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008cff000ULL,
      0x0c33f80000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "movei", TILEPRO_OPC_MOVEI, 0xf, 2, TREG_ZERO, 1,
    { { 9, 0 }, { 7, 1 }, { 11, 2 }, { 13, 3 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00fc0ULL,
      0xfff807e000000000ULL,
      0x8000000078000fc0ULL,
      0xf80007e000000000ULL,
      0ULL
    },
    {
      0x0000000040800fc0ULL,
      0x305807e000000000ULL,
      0x8000000058000fc0ULL,
      0xc80007e000000000ULL,
      -1ULL
    }
#endif
  },
  { "movei.sn", TILEPRO_OPC_MOVEI_SN, 0x3, 2, TREG_SN, 1,
    { { 9, 0 }, { 7, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00fc0ULL,
      0xfff807e000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048800fc0ULL,
      0x345807e000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "moveli", TILEPRO_OPC_MOVELI, 0x3, 2, TREG_ZERO, 1,
    { { 9, 4 }, { 7, 5 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000fc0ULL,
      0xf80007e000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000020000fc0ULL,
      0x180007e000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "moveli.sn", TILEPRO_OPC_MOVELI_SN, 0x3, 2, TREG_SN, 1,
    { { 9, 4 }, { 7, 5 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000fc0ULL,
      0xf80007e000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000010000fc0ULL,
      0x100007e000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "movelis", TILEPRO_OPC_MOVELIS, 0x3, 2, TREG_SN, 1,
    { { 9, 4 }, { 7, 5 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000fc0ULL,
      0xf80007e000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000010000fc0ULL,
      0x100007e000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "prefetch", TILEPRO_OPC_PREFETCH, 0x12, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 15 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff81f80000000ULL,
      0ULL,
      0ULL,
      0x8700000003f00000ULL
    },
    {
      -1ULL,
      0x400b501f80000000ULL,
      -1ULL,
      -1ULL,
      0x8000000003f00000ULL
    }
#endif
  },
  { "raise", TILEPRO_OPC_RAISE, 0x2, 0, TREG_ZERO, 1,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbffffff80000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b3cae80000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "add", TILEPRO_OPC_ADD, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x00000000000c0000ULL,
      0x0806000000000000ULL,
      0x8000000008000000ULL,
      0x8800000000000000ULL,
      -1ULL
    }
#endif
  },
  { "add.sn", TILEPRO_OPC_ADD_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000080c0000ULL,
      0x0c06000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addb", TILEPRO_OPC_ADDB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000040000ULL,
      0x0802000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addb.sn", TILEPRO_OPC_ADDB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008040000ULL,
      0x0c02000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addbs_u", TILEPRO_OPC_ADDBS_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001880000ULL,
      0x0888000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addbs_u.sn", TILEPRO_OPC_ADDBS_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009880000ULL,
      0x0c88000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addh", TILEPRO_OPC_ADDH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000080000ULL,
      0x0804000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addh.sn", TILEPRO_OPC_ADDH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008080000ULL,
      0x0c04000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addhs", TILEPRO_OPC_ADDHS, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000018c0000ULL,
      0x088a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addhs.sn", TILEPRO_OPC_ADDHS_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000098c0000ULL,
      0x0c8a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addi", TILEPRO_OPC_ADDI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 11, 12, 2 }, { 13, 14, 3 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0x8000000078000000ULL,
      0xf800000000000000ULL,
      0ULL
    },
    {
      0x0000000040300000ULL,
      0x3018000000000000ULL,
      0x8000000048000000ULL,
      0xb800000000000000ULL,
      -1ULL
    }
#endif
  },
  { "addi.sn", TILEPRO_OPC_ADDI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048300000ULL,
      0x3418000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addib", TILEPRO_OPC_ADDIB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040100000ULL,
      0x3008000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addib.sn", TILEPRO_OPC_ADDIB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048100000ULL,
      0x3408000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addih", TILEPRO_OPC_ADDIH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040200000ULL,
      0x3010000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addih.sn", TILEPRO_OPC_ADDIH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048200000ULL,
      0x3410000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addli", TILEPRO_OPC_ADDLI, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 4 }, { 7, 8, 5 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000000ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000020000000ULL,
      0x1800000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addli.sn", TILEPRO_OPC_ADDLI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 4 }, { 7, 8, 5 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000000ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000010000000ULL,
      0x1000000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "addlis", TILEPRO_OPC_ADDLIS, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 4 }, { 7, 8, 5 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000000ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000010000000ULL,
      0x1000000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "adds", TILEPRO_OPC_ADDS, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001800000ULL,
      0x0884000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "adds.sn", TILEPRO_OPC_ADDS_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009800000ULL,
      0x0c84000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "adiffb_u", TILEPRO_OPC_ADIFFB_U, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000100000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "adiffb_u.sn", TILEPRO_OPC_ADIFFB_U_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008100000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "adiffh", TILEPRO_OPC_ADIFFH, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000140000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "adiffh.sn", TILEPRO_OPC_ADIFFH_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008140000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "and", TILEPRO_OPC_AND, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000180000ULL,
      0x0808000000000000ULL,
      0x8000000018000000ULL,
      0x9800000000000000ULL,
      -1ULL
    }
#endif
  },
  { "and.sn", TILEPRO_OPC_AND_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008180000ULL,
      0x0c08000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "andi", TILEPRO_OPC_ANDI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 11, 12, 2 }, { 13, 14, 3 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0x8000000078000000ULL,
      0xf800000000000000ULL,
      0ULL
    },
    {
      0x0000000050100000ULL,
      0x3020000000000000ULL,
      0x8000000050000000ULL,
      0xc000000000000000ULL,
      -1ULL
    }
#endif
  },
  { "andi.sn", TILEPRO_OPC_ANDI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000058100000ULL,
      0x3420000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "auli", TILEPRO_OPC_AULI, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 4 }, { 7, 8, 5 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000000ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000030000000ULL,
      0x2000000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "avgb_u", TILEPRO_OPC_AVGB_U, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000001c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "avgb_u.sn", TILEPRO_OPC_AVGB_U_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000081c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "avgh", TILEPRO_OPC_AVGH, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000200000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "avgh.sn", TILEPRO_OPC_AVGH_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008200000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bbns", TILEPRO_OPC_BBNS, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000700000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bbns.sn", TILEPRO_OPC_BBNS_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000700000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bbnst", TILEPRO_OPC_BBNST, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000780000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bbnst.sn", TILEPRO_OPC_BBNST_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000780000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bbs", TILEPRO_OPC_BBS, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000600000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bbs.sn", TILEPRO_OPC_BBS_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000600000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bbst", TILEPRO_OPC_BBST, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000680000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bbst.sn", TILEPRO_OPC_BBST_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000680000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bgez", TILEPRO_OPC_BGEZ, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000300000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bgez.sn", TILEPRO_OPC_BGEZ_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000300000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bgezt", TILEPRO_OPC_BGEZT, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000380000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bgezt.sn", TILEPRO_OPC_BGEZT_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000380000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bgz", TILEPRO_OPC_BGZ, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000200000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bgz.sn", TILEPRO_OPC_BGZ_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000200000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bgzt", TILEPRO_OPC_BGZT, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000280000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bgzt.sn", TILEPRO_OPC_BGZT_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000280000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bitx", TILEPRO_OPC_BITX, 0x5, 2, TREG_ZERO, 1,
    { { 9, 10 }, { 0, }, { 11, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070161000ULL,
      -1ULL,
      0x80000000680a1000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bitx.sn", TILEPRO_OPC_BITX_SN, 0x1, 2, TREG_SN, 1,
    { { 9, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078161000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "blez", TILEPRO_OPC_BLEZ, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000500000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "blez.sn", TILEPRO_OPC_BLEZ_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000500000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "blezt", TILEPRO_OPC_BLEZT, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000580000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "blezt.sn", TILEPRO_OPC_BLEZT_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000580000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "blz", TILEPRO_OPC_BLZ, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000400000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "blz.sn", TILEPRO_OPC_BLZ_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000400000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "blzt", TILEPRO_OPC_BLZT, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000480000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "blzt.sn", TILEPRO_OPC_BLZT_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000480000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bnz", TILEPRO_OPC_BNZ, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000100000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bnz.sn", TILEPRO_OPC_BNZ_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000100000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bnzt", TILEPRO_OPC_BNZT, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000180000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bnzt.sn", TILEPRO_OPC_BNZT_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000180000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bytex", TILEPRO_OPC_BYTEX, 0x5, 2, TREG_ZERO, 1,
    { { 9, 10 }, { 0, }, { 11, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070162000ULL,
      -1ULL,
      0x80000000680a2000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bytex.sn", TILEPRO_OPC_BYTEX_SN, 0x1, 2, TREG_SN, 1,
    { { 9, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078162000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bz", TILEPRO_OPC_BZ, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bz.sn", TILEPRO_OPC_BZ_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bzt", TILEPRO_OPC_BZT, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2800000080000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "bzt.sn", TILEPRO_OPC_BZT_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 8, 20 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfc00000780000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x2c00000080000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "clz", TILEPRO_OPC_CLZ, 0x5, 2, TREG_ZERO, 1,
    { { 9, 10 }, { 0, }, { 11, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070163000ULL,
      -1ULL,
      0x80000000680a3000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "clz.sn", TILEPRO_OPC_CLZ_SN, 0x1, 2, TREG_SN, 1,
    { { 9, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078163000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "crc32_32", TILEPRO_OPC_CRC32_32, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000240000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "crc32_32.sn", TILEPRO_OPC_CRC32_32_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008240000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "crc32_8", TILEPRO_OPC_CRC32_8, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000280000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "crc32_8.sn", TILEPRO_OPC_CRC32_8_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008280000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "ctz", TILEPRO_OPC_CTZ, 0x5, 2, TREG_ZERO, 1,
    { { 9, 10 }, { 0, }, { 11, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070164000ULL,
      -1ULL,
      0x80000000680a4000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "ctz.sn", TILEPRO_OPC_CTZ_SN, 0x1, 2, TREG_SN, 1,
    { { 9, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078164000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "drain", TILEPRO_OPC_DRAIN, 0x2, 0, TREG_ZERO, 0,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b080000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "dtlbpr", TILEPRO_OPC_DTLBPR, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b100000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "dword_align", TILEPRO_OPC_DWORD_ALIGN, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000017c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "dword_align.sn", TILEPRO_OPC_DWORD_ALIGN_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000097c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "finv", TILEPRO_OPC_FINV, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b180000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "flush", TILEPRO_OPC_FLUSH, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b200000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "fnop", TILEPRO_OPC_FNOP, 0xf, 0, TREG_ZERO, 1,
    { {  }, {  }, {  }, {  }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000077fff000ULL,
      0xfbfff80000000000ULL,
      0x80000000780ff000ULL,
      0xf807f80000000000ULL,
      0ULL
    },
    {
      0x0000000070165000ULL,
      0x400b280000000000ULL,
      0x80000000680a5000ULL,
      0xd805080000000000ULL,
      -1ULL
    }
#endif
  },
  { "icoh", TILEPRO_OPC_ICOH, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b300000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "ill", TILEPRO_OPC_ILL, 0xa, 0, TREG_ZERO, 1,
    { { 0, }, {  }, { 0, }, {  }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0xf807f80000000000ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b380000000000ULL,
      -1ULL,
      0xd805100000000000ULL,
      -1ULL
    }
#endif
  },
  { "inthb", TILEPRO_OPC_INTHB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000002c0000ULL,
      0x080a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "inthb.sn", TILEPRO_OPC_INTHB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000082c0000ULL,
      0x0c0a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "inthh", TILEPRO_OPC_INTHH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000300000ULL,
      0x080c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "inthh.sn", TILEPRO_OPC_INTHH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008300000ULL,
      0x0c0c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "intlb", TILEPRO_OPC_INTLB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000340000ULL,
      0x080e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "intlb.sn", TILEPRO_OPC_INTLB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008340000ULL,
      0x0c0e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "intlh", TILEPRO_OPC_INTLH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000380000ULL,
      0x0810000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "intlh.sn", TILEPRO_OPC_INTLH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008380000ULL,
      0x0c10000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "inv", TILEPRO_OPC_INV, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b400000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "iret", TILEPRO_OPC_IRET, 0x2, 0, TREG_ZERO, 1,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b480000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jalb", TILEPRO_OPC_JALB, 0x2, 1, TREG_LR, 1,
    { { 0, }, { 22 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x6800000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jalf", TILEPRO_OPC_JALF, 0x2, 1, TREG_LR, 1,
    { { 0, }, { 22 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x6000000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jalr", TILEPRO_OPC_JALR, 0x2, 1, TREG_LR, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x0814000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jalrp", TILEPRO_OPC_JALRP, 0x2, 1, TREG_LR, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x0812000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jb", TILEPRO_OPC_JB, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 22 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x5800000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jf", TILEPRO_OPC_JF, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 22 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x5000000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jr", TILEPRO_OPC_JR, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x0818000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "jrp", TILEPRO_OPC_JRP, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x0816000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lb", TILEPRO_OPC_LB, 0x12, 2, TREG_ZERO, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 23, 15 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0x8700000000000000ULL
    },
    {
      -1ULL,
      0x400b500000000000ULL,
      -1ULL,
      -1ULL,
      0x8000000000000000ULL
    }
#endif
  },
  { "lb.sn", TILEPRO_OPC_LB_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x440b500000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lb_u", TILEPRO_OPC_LB_U, 0x12, 2, TREG_ZERO, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 23, 15 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0x8700000000000000ULL
    },
    {
      -1ULL,
      0x400b580000000000ULL,
      -1ULL,
      -1ULL,
      0x8100000000000000ULL
    }
#endif
  },
  { "lb_u.sn", TILEPRO_OPC_LB_U_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x440b580000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lbadd", TILEPRO_OPC_LBADD, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30b0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lbadd.sn", TILEPRO_OPC_LBADD_SN, 0x2, 3, TREG_SN, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x34b0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lbadd_u", TILEPRO_OPC_LBADD_U, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30b8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lbadd_u.sn", TILEPRO_OPC_LBADD_U_SN, 0x2, 3, TREG_SN, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x34b8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lh", TILEPRO_OPC_LH, 0x12, 2, TREG_ZERO, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 23, 15 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0x8700000000000000ULL
    },
    {
      -1ULL,
      0x400b600000000000ULL,
      -1ULL,
      -1ULL,
      0x8200000000000000ULL
    }
#endif
  },
  { "lh.sn", TILEPRO_OPC_LH_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x440b600000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lh_u", TILEPRO_OPC_LH_U, 0x12, 2, TREG_ZERO, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 23, 15 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0x8700000000000000ULL
    },
    {
      -1ULL,
      0x400b680000000000ULL,
      -1ULL,
      -1ULL,
      0x8300000000000000ULL
    }
#endif
  },
  { "lh_u.sn", TILEPRO_OPC_LH_U_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x440b680000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lhadd", TILEPRO_OPC_LHADD, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30c0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lhadd.sn", TILEPRO_OPC_LHADD_SN, 0x2, 3, TREG_SN, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x34c0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lhadd_u", TILEPRO_OPC_LHADD_U, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30c8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lhadd_u.sn", TILEPRO_OPC_LHADD_U_SN, 0x2, 3, TREG_SN, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x34c8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lnk", TILEPRO_OPC_LNK, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 7 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x081a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lnk.sn", TILEPRO_OPC_LNK_SN, 0x2, 1, TREG_SN, 1,
    { { 0, }, { 7 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x0c1a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lw", TILEPRO_OPC_LW, 0x12, 2, TREG_ZERO, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 23, 15 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0x8700000000000000ULL
    },
    {
      -1ULL,
      0x400b700000000000ULL,
      -1ULL,
      -1ULL,
      0x8400000000000000ULL
    }
#endif
  },
  { "lw.sn", TILEPRO_OPC_LW_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x440b700000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lw_na", TILEPRO_OPC_LW_NA, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400bc00000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lw_na.sn", TILEPRO_OPC_LW_NA_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x440bc00000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lwadd", TILEPRO_OPC_LWADD, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30d0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lwadd.sn", TILEPRO_OPC_LWADD_SN, 0x2, 3, TREG_SN, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x34d0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lwadd_na", TILEPRO_OPC_LWADD_NA, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30d8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "lwadd_na.sn", TILEPRO_OPC_LWADD_NA_SN, 0x2, 3, TREG_SN, 1,
    { { 0, }, { 7, 24, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x34d8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "maxb_u", TILEPRO_OPC_MAXB_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000003c0000ULL,
      0x081c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "maxb_u.sn", TILEPRO_OPC_MAXB_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000083c0000ULL,
      0x0c1c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "maxh", TILEPRO_OPC_MAXH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000400000ULL,
      0x081e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "maxh.sn", TILEPRO_OPC_MAXH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008400000ULL,
      0x0c1e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "maxib_u", TILEPRO_OPC_MAXIB_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040400000ULL,
      0x3028000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "maxib_u.sn", TILEPRO_OPC_MAXIB_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048400000ULL,
      0x3428000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "maxih", TILEPRO_OPC_MAXIH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040500000ULL,
      0x3030000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "maxih.sn", TILEPRO_OPC_MAXIH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048500000ULL,
      0x3430000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mf", TILEPRO_OPC_MF, 0x2, 0, TREG_ZERO, 1,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b780000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mfspr", TILEPRO_OPC_MFSPR, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 7, 25 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbf8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x3038000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "minb_u", TILEPRO_OPC_MINB_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000440000ULL,
      0x0820000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "minb_u.sn", TILEPRO_OPC_MINB_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008440000ULL,
      0x0c20000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "minh", TILEPRO_OPC_MINH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000480000ULL,
      0x0822000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "minh.sn", TILEPRO_OPC_MINH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008480000ULL,
      0x0c22000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "minib_u", TILEPRO_OPC_MINIB_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040600000ULL,
      0x3040000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "minib_u.sn", TILEPRO_OPC_MINIB_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048600000ULL,
      0x3440000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "minih", TILEPRO_OPC_MINIH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040700000ULL,
      0x3048000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "minih.sn", TILEPRO_OPC_MINIH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048700000ULL,
      0x3448000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mm", TILEPRO_OPC_MM, 0x3, 5, TREG_ZERO, 1,
    { { 9, 10, 16, 26, 27 }, { 7, 8, 17, 28, 29 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000070000000ULL,
      0xf800000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000060000000ULL,
      0x3800000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mnz", TILEPRO_OPC_MNZ, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000540000ULL,
      0x0828000000000000ULL,
      0x8000000010000000ULL,
      0x9002000000000000ULL,
      -1ULL
    }
#endif
  },
  { "mnz.sn", TILEPRO_OPC_MNZ_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008540000ULL,
      0x0c28000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mnzb", TILEPRO_OPC_MNZB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000004c0000ULL,
      0x0824000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mnzb.sn", TILEPRO_OPC_MNZB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000084c0000ULL,
      0x0c24000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mnzh", TILEPRO_OPC_MNZH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000500000ULL,
      0x0826000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mnzh.sn", TILEPRO_OPC_MNZH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008500000ULL,
      0x0c26000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mtspr", TILEPRO_OPC_MTSPR, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 30, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbf8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x3050000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhh_ss", TILEPRO_OPC_MULHH_SS, 0x5, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 11, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000680000ULL,
      -1ULL,
      0x8000000038000000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhh_ss.sn", TILEPRO_OPC_MULHH_SS_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008680000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhh_su", TILEPRO_OPC_MULHH_SU, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000006c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhh_su.sn", TILEPRO_OPC_MULHH_SU_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000086c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhh_uu", TILEPRO_OPC_MULHH_UU, 0x5, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 11, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000700000ULL,
      -1ULL,
      0x8000000038040000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhh_uu.sn", TILEPRO_OPC_MULHH_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008700000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhha_ss", TILEPRO_OPC_MULHHA_SS, 0x5, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 31, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000580000ULL,
      -1ULL,
      0x8000000040000000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhha_ss.sn", TILEPRO_OPC_MULHHA_SS_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008580000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhha_su", TILEPRO_OPC_MULHHA_SU, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000005c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhha_su.sn", TILEPRO_OPC_MULHHA_SU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000085c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhha_uu", TILEPRO_OPC_MULHHA_UU, 0x5, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 31, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000600000ULL,
      -1ULL,
      0x8000000040040000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhha_uu.sn", TILEPRO_OPC_MULHHA_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008600000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhhsa_uu", TILEPRO_OPC_MULHHSA_UU, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000640000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhhsa_uu.sn", TILEPRO_OPC_MULHHSA_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008640000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhl_ss", TILEPRO_OPC_MULHL_SS, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000880000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhl_ss.sn", TILEPRO_OPC_MULHL_SS_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008880000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhl_su", TILEPRO_OPC_MULHL_SU, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000008c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhl_su.sn", TILEPRO_OPC_MULHL_SU_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000088c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhl_us", TILEPRO_OPC_MULHL_US, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000900000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhl_us.sn", TILEPRO_OPC_MULHL_US_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008900000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhl_uu", TILEPRO_OPC_MULHL_UU, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000940000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhl_uu.sn", TILEPRO_OPC_MULHL_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008940000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhla_ss", TILEPRO_OPC_MULHLA_SS, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000740000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhla_ss.sn", TILEPRO_OPC_MULHLA_SS_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008740000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhla_su", TILEPRO_OPC_MULHLA_SU, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000780000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhla_su.sn", TILEPRO_OPC_MULHLA_SU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008780000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhla_us", TILEPRO_OPC_MULHLA_US, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000007c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhla_us.sn", TILEPRO_OPC_MULHLA_US_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000087c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhla_uu", TILEPRO_OPC_MULHLA_UU, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000800000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhla_uu.sn", TILEPRO_OPC_MULHLA_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008800000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhlsa_uu", TILEPRO_OPC_MULHLSA_UU, 0x5, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 31, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000840000ULL,
      -1ULL,
      0x8000000030000000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulhlsa_uu.sn", TILEPRO_OPC_MULHLSA_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008840000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulll_ss", TILEPRO_OPC_MULLL_SS, 0x5, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 11, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000a80000ULL,
      -1ULL,
      0x8000000038080000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulll_ss.sn", TILEPRO_OPC_MULLL_SS_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008a80000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulll_su", TILEPRO_OPC_MULLL_SU, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000ac0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulll_su.sn", TILEPRO_OPC_MULLL_SU_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008ac0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulll_uu", TILEPRO_OPC_MULLL_UU, 0x5, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 11, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000b00000ULL,
      -1ULL,
      0x80000000380c0000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulll_uu.sn", TILEPRO_OPC_MULLL_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008b00000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mullla_ss", TILEPRO_OPC_MULLLA_SS, 0x5, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 31, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000980000ULL,
      -1ULL,
      0x8000000040080000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mullla_ss.sn", TILEPRO_OPC_MULLLA_SS_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008980000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mullla_su", TILEPRO_OPC_MULLLA_SU, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000009c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mullla_su.sn", TILEPRO_OPC_MULLLA_SU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000089c0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mullla_uu", TILEPRO_OPC_MULLLA_UU, 0x5, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 31, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000a00000ULL,
      -1ULL,
      0x80000000400c0000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mullla_uu.sn", TILEPRO_OPC_MULLLA_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008a00000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulllsa_uu", TILEPRO_OPC_MULLLSA_UU, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000a40000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mulllsa_uu.sn", TILEPRO_OPC_MULLLSA_UU_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008a40000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mvnz", TILEPRO_OPC_MVNZ, 0x5, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 31, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000b40000ULL,
      -1ULL,
      0x8000000010040000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mvnz.sn", TILEPRO_OPC_MVNZ_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008b40000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mvz", TILEPRO_OPC_MVZ, 0x5, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 31, 12, 18 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0x80000000780c0000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000b80000ULL,
      -1ULL,
      0x8000000010080000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mvz.sn", TILEPRO_OPC_MVZ_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008b80000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mz", TILEPRO_OPC_MZ, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000c40000ULL,
      0x082e000000000000ULL,
      0x80000000100c0000ULL,
      0x9004000000000000ULL,
      -1ULL
    }
#endif
  },
  { "mz.sn", TILEPRO_OPC_MZ_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008c40000ULL,
      0x0c2e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mzb", TILEPRO_OPC_MZB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000bc0000ULL,
      0x082a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mzb.sn", TILEPRO_OPC_MZB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008bc0000ULL,
      0x0c2a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mzh", TILEPRO_OPC_MZH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000c00000ULL,
      0x082c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "mzh.sn", TILEPRO_OPC_MZH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008c00000ULL,
      0x0c2c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "nap", TILEPRO_OPC_NAP, 0x2, 0, TREG_ZERO, 0,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b800000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "nop", TILEPRO_OPC_NOP, 0xf, 0, TREG_ZERO, 1,
    { {  }, {  }, {  }, {  }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x8000000077fff000ULL,
      0xfbfff80000000000ULL,
      0x80000000780ff000ULL,
      0xf807f80000000000ULL,
      0ULL
    },
    {
      0x0000000070166000ULL,
      0x400b880000000000ULL,
      0x80000000680a6000ULL,
      0xd805180000000000ULL,
      -1ULL
    }
#endif
  },
  { "nor", TILEPRO_OPC_NOR, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000c80000ULL,
      0x0830000000000000ULL,
      0x8000000018040000ULL,
      0x9802000000000000ULL,
      -1ULL
    }
#endif
  },
  { "nor.sn", TILEPRO_OPC_NOR_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008c80000ULL,
      0x0c30000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "or", TILEPRO_OPC_OR, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000cc0000ULL,
      0x0832000000000000ULL,
      0x8000000018080000ULL,
      0x9804000000000000ULL,
      -1ULL
    }
#endif
  },
  { "or.sn", TILEPRO_OPC_OR_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008cc0000ULL,
      0x0c32000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "ori", TILEPRO_OPC_ORI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 11, 12, 2 }, { 13, 14, 3 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0x8000000078000000ULL,
      0xf800000000000000ULL,
      0ULL
    },
    {
      0x0000000040800000ULL,
      0x3058000000000000ULL,
      0x8000000058000000ULL,
      0xc800000000000000ULL,
      -1ULL
    }
#endif
  },
  { "ori.sn", TILEPRO_OPC_ORI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048800000ULL,
      0x3458000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "packbs_u", TILEPRO_OPC_PACKBS_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000019c0000ULL,
      0x0892000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "packbs_u.sn", TILEPRO_OPC_PACKBS_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000099c0000ULL,
      0x0c92000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "packhb", TILEPRO_OPC_PACKHB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000d00000ULL,
      0x0834000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "packhb.sn", TILEPRO_OPC_PACKHB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008d00000ULL,
      0x0c34000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "packhs", TILEPRO_OPC_PACKHS, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001980000ULL,
      0x0890000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "packhs.sn", TILEPRO_OPC_PACKHS_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009980000ULL,
      0x0c90000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "packlb", TILEPRO_OPC_PACKLB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000d40000ULL,
      0x0836000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "packlb.sn", TILEPRO_OPC_PACKLB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008d40000ULL,
      0x0c36000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "pcnt", TILEPRO_OPC_PCNT, 0x5, 2, TREG_ZERO, 1,
    { { 9, 10 }, { 0, }, { 11, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070167000ULL,
      -1ULL,
      0x80000000680a7000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "pcnt.sn", TILEPRO_OPC_PCNT_SN, 0x1, 2, TREG_SN, 1,
    { { 9, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078167000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "rl", TILEPRO_OPC_RL, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000d80000ULL,
      0x0838000000000000ULL,
      0x8000000020000000ULL,
      0xa000000000000000ULL,
      -1ULL
    }
#endif
  },
  { "rl.sn", TILEPRO_OPC_RL_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008d80000ULL,
      0x0c38000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "rli", TILEPRO_OPC_RLI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 11, 12, 34 }, { 13, 14, 35 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0x80000000780e0000ULL,
      0xf807000000000000ULL,
      0ULL
    },
    {
      0x0000000070020000ULL,
      0x4001000000000000ULL,
      0x8000000068020000ULL,
      0xd801000000000000ULL,
      -1ULL
    }
#endif
  },
  { "rli.sn", TILEPRO_OPC_RLI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078020000ULL,
      0x4401000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "s1a", TILEPRO_OPC_S1A, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000dc0000ULL,
      0x083a000000000000ULL,
      0x8000000008040000ULL,
      0x8802000000000000ULL,
      -1ULL
    }
#endif
  },
  { "s1a.sn", TILEPRO_OPC_S1A_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008dc0000ULL,
      0x0c3a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "s2a", TILEPRO_OPC_S2A, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000e00000ULL,
      0x083c000000000000ULL,
      0x8000000008080000ULL,
      0x8804000000000000ULL,
      -1ULL
    }
#endif
  },
  { "s2a.sn", TILEPRO_OPC_S2A_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008e00000ULL,
      0x0c3c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "s3a", TILEPRO_OPC_S3A, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000000e40000ULL,
      0x083e000000000000ULL,
      0x8000000030040000ULL,
      0xb002000000000000ULL,
      -1ULL
    }
#endif
  },
  { "s3a.sn", TILEPRO_OPC_S3A_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008e40000ULL,
      0x0c3e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadab_u", TILEPRO_OPC_SADAB_U, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000e80000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadab_u.sn", TILEPRO_OPC_SADAB_U_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008e80000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadah", TILEPRO_OPC_SADAH, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000ec0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadah.sn", TILEPRO_OPC_SADAH_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008ec0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadah_u", TILEPRO_OPC_SADAH_U, 0x1, 3, TREG_ZERO, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000f00000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadah_u.sn", TILEPRO_OPC_SADAH_U_SN, 0x1, 3, TREG_SN, 1,
    { { 21, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008f00000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadb_u", TILEPRO_OPC_SADB_U, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000f40000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadb_u.sn", TILEPRO_OPC_SADB_U_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008f40000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadh", TILEPRO_OPC_SADH, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000f80000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadh.sn", TILEPRO_OPC_SADH_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008f80000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadh_u", TILEPRO_OPC_SADH_U, 0x1, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000000fc0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sadh_u.sn", TILEPRO_OPC_SADH_U_SN, 0x1, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000008fc0000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sb", TILEPRO_OPC_SB, 0x12, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 17 }, { 0, }, { 0, }, { 15, 36 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfe000000000000ULL,
      0ULL,
      0ULL,
      0x8700000000000000ULL
    },
    {
      -1ULL,
      0x0840000000000000ULL,
      -1ULL,
      -1ULL,
      0x8500000000000000ULL
    }
#endif
  },
  { "sbadd", TILEPRO_OPC_SBADD, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 24, 17, 37 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbf8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30e0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seq", TILEPRO_OPC_SEQ, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000001080000ULL,
      0x0846000000000000ULL,
      0x8000000030080000ULL,
      0xb004000000000000ULL,
      -1ULL
    }
#endif
  },
  { "seq.sn", TILEPRO_OPC_SEQ_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009080000ULL,
      0x0c46000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqb", TILEPRO_OPC_SEQB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001000000ULL,
      0x0842000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqb.sn", TILEPRO_OPC_SEQB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009000000ULL,
      0x0c42000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqh", TILEPRO_OPC_SEQH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001040000ULL,
      0x0844000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqh.sn", TILEPRO_OPC_SEQH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009040000ULL,
      0x0c44000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqi", TILEPRO_OPC_SEQI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 11, 12, 2 }, { 13, 14, 3 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0x8000000078000000ULL,
      0xf800000000000000ULL,
      0ULL
    },
    {
      0x0000000040b00000ULL,
      0x3070000000000000ULL,
      0x8000000060000000ULL,
      0xd000000000000000ULL,
      -1ULL
    }
#endif
  },
  { "seqi.sn", TILEPRO_OPC_SEQI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048b00000ULL,
      0x3470000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqib", TILEPRO_OPC_SEQIB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040900000ULL,
      0x3060000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqib.sn", TILEPRO_OPC_SEQIB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048900000ULL,
      0x3460000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqih", TILEPRO_OPC_SEQIH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040a00000ULL,
      0x3068000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "seqih.sn", TILEPRO_OPC_SEQIH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048a00000ULL,
      0x3468000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sh", TILEPRO_OPC_SH, 0x12, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 17 }, { 0, }, { 0, }, { 15, 36 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfe000000000000ULL,
      0ULL,
      0ULL,
      0x8700000000000000ULL
    },
    {
      -1ULL,
      0x0854000000000000ULL,
      -1ULL,
      -1ULL,
      0x8600000000000000ULL
    }
#endif
  },
  { "shadd", TILEPRO_OPC_SHADD, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 24, 17, 37 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbf8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30e8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shl", TILEPRO_OPC_SHL, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000001140000ULL,
      0x084c000000000000ULL,
      0x8000000020040000ULL,
      0xa002000000000000ULL,
      -1ULL
    }
#endif
  },
  { "shl.sn", TILEPRO_OPC_SHL_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009140000ULL,
      0x0c4c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shlb", TILEPRO_OPC_SHLB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000010c0000ULL,
      0x0848000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shlb.sn", TILEPRO_OPC_SHLB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000090c0000ULL,
      0x0c48000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shlh", TILEPRO_OPC_SHLH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001100000ULL,
      0x084a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shlh.sn", TILEPRO_OPC_SHLH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009100000ULL,
      0x0c4a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shli", TILEPRO_OPC_SHLI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 11, 12, 34 }, { 13, 14, 35 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0x80000000780e0000ULL,
      0xf807000000000000ULL,
      0ULL
    },
    {
      0x0000000070080000ULL,
      0x4004000000000000ULL,
      0x8000000068040000ULL,
      0xd802000000000000ULL,
      -1ULL
    }
#endif
  },
  { "shli.sn", TILEPRO_OPC_SHLI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078080000ULL,
      0x4404000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shlib", TILEPRO_OPC_SHLIB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070040000ULL,
      0x4002000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shlib.sn", TILEPRO_OPC_SHLIB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078040000ULL,
      0x4402000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shlih", TILEPRO_OPC_SHLIH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070060000ULL,
      0x4003000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shlih.sn", TILEPRO_OPC_SHLIH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078060000ULL,
      0x4403000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shr", TILEPRO_OPC_SHR, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000001200000ULL,
      0x0852000000000000ULL,
      0x8000000020080000ULL,
      0xa004000000000000ULL,
      -1ULL
    }
#endif
  },
  { "shr.sn", TILEPRO_OPC_SHR_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009200000ULL,
      0x0c52000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shrb", TILEPRO_OPC_SHRB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001180000ULL,
      0x084e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shrb.sn", TILEPRO_OPC_SHRB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009180000ULL,
      0x0c4e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shrh", TILEPRO_OPC_SHRH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000011c0000ULL,
      0x0850000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shrh.sn", TILEPRO_OPC_SHRH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000091c0000ULL,
      0x0c50000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shri", TILEPRO_OPC_SHRI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 11, 12, 34 }, { 13, 14, 35 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0x80000000780e0000ULL,
      0xf807000000000000ULL,
      0ULL
    },
    {
      0x00000000700e0000ULL,
      0x4007000000000000ULL,
      0x8000000068060000ULL,
      0xd803000000000000ULL,
      -1ULL
    }
#endif
  },
  { "shri.sn", TILEPRO_OPC_SHRI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000780e0000ULL,
      0x4407000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shrib", TILEPRO_OPC_SHRIB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000700a0000ULL,
      0x4005000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shrib.sn", TILEPRO_OPC_SHRIB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000780a0000ULL,
      0x4405000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shrih", TILEPRO_OPC_SHRIH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000700c0000ULL,
      0x4006000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "shrih.sn", TILEPRO_OPC_SHRIH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000780c0000ULL,
      0x4406000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slt", TILEPRO_OPC_SLT, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x00000000014c0000ULL,
      0x086a000000000000ULL,
      0x8000000028080000ULL,
      0xa804000000000000ULL,
      -1ULL
    }
#endif
  },
  { "slt.sn", TILEPRO_OPC_SLT_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000094c0000ULL,
      0x0c6a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slt_u", TILEPRO_OPC_SLT_U, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000001500000ULL,
      0x086c000000000000ULL,
      0x80000000280c0000ULL,
      0xa806000000000000ULL,
      -1ULL
    }
#endif
  },
  { "slt_u.sn", TILEPRO_OPC_SLT_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009500000ULL,
      0x0c6c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltb", TILEPRO_OPC_SLTB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001240000ULL,
      0x0856000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltb.sn", TILEPRO_OPC_SLTB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009240000ULL,
      0x0c56000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltb_u", TILEPRO_OPC_SLTB_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001280000ULL,
      0x0858000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltb_u.sn", TILEPRO_OPC_SLTB_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009280000ULL,
      0x0c58000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slte", TILEPRO_OPC_SLTE, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x00000000013c0000ULL,
      0x0862000000000000ULL,
      0x8000000028000000ULL,
      0xa800000000000000ULL,
      -1ULL
    }
#endif
  },
  { "slte.sn", TILEPRO_OPC_SLTE_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000093c0000ULL,
      0x0c62000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slte_u", TILEPRO_OPC_SLTE_U, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000001400000ULL,
      0x0864000000000000ULL,
      0x8000000028040000ULL,
      0xa802000000000000ULL,
      -1ULL
    }
#endif
  },
  { "slte_u.sn", TILEPRO_OPC_SLTE_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009400000ULL,
      0x0c64000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slteb", TILEPRO_OPC_SLTEB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000012c0000ULL,
      0x085a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slteb.sn", TILEPRO_OPC_SLTEB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000092c0000ULL,
      0x0c5a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slteb_u", TILEPRO_OPC_SLTEB_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001300000ULL,
      0x085c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slteb_u.sn", TILEPRO_OPC_SLTEB_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009300000ULL,
      0x0c5c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slteh", TILEPRO_OPC_SLTEH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001340000ULL,
      0x085e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slteh.sn", TILEPRO_OPC_SLTEH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009340000ULL,
      0x0c5e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slteh_u", TILEPRO_OPC_SLTEH_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001380000ULL,
      0x0860000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slteh_u.sn", TILEPRO_OPC_SLTEH_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009380000ULL,
      0x0c60000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slth", TILEPRO_OPC_SLTH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001440000ULL,
      0x0866000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slth.sn", TILEPRO_OPC_SLTH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009440000ULL,
      0x0c66000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slth_u", TILEPRO_OPC_SLTH_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001480000ULL,
      0x0868000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slth_u.sn", TILEPRO_OPC_SLTH_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009480000ULL,
      0x0c68000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slti", TILEPRO_OPC_SLTI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 11, 12, 2 }, { 13, 14, 3 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0x8000000078000000ULL,
      0xf800000000000000ULL,
      0ULL
    },
    {
      0x0000000041000000ULL,
      0x3098000000000000ULL,
      0x8000000070000000ULL,
      0xe000000000000000ULL,
      -1ULL
    }
#endif
  },
  { "slti.sn", TILEPRO_OPC_SLTI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000049000000ULL,
      0x3498000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "slti_u", TILEPRO_OPC_SLTI_U, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 11, 12, 2 }, { 13, 14, 3 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0x8000000078000000ULL,
      0xf800000000000000ULL,
      0ULL
    },
    {
      0x0000000041100000ULL,
      0x30a0000000000000ULL,
      0x8000000078000000ULL,
      0xe800000000000000ULL,
      -1ULL
    }
#endif
  },
  { "slti_u.sn", TILEPRO_OPC_SLTI_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000049100000ULL,
      0x34a0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltib", TILEPRO_OPC_SLTIB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040c00000ULL,
      0x3078000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltib.sn", TILEPRO_OPC_SLTIB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048c00000ULL,
      0x3478000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltib_u", TILEPRO_OPC_SLTIB_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040d00000ULL,
      0x3080000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltib_u.sn", TILEPRO_OPC_SLTIB_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048d00000ULL,
      0x3480000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltih", TILEPRO_OPC_SLTIH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040e00000ULL,
      0x3088000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltih.sn", TILEPRO_OPC_SLTIH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048e00000ULL,
      0x3488000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltih_u", TILEPRO_OPC_SLTIH_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000040f00000ULL,
      0x3090000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sltih_u.sn", TILEPRO_OPC_SLTIH_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000048f00000ULL,
      0x3490000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sne", TILEPRO_OPC_SNE, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x00000000015c0000ULL,
      0x0872000000000000ULL,
      0x80000000300c0000ULL,
      0xb006000000000000ULL,
      -1ULL
    }
#endif
  },
  { "sne.sn", TILEPRO_OPC_SNE_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000095c0000ULL,
      0x0c72000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sneb", TILEPRO_OPC_SNEB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001540000ULL,
      0x086e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sneb.sn", TILEPRO_OPC_SNEB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009540000ULL,
      0x0c6e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sneh", TILEPRO_OPC_SNEH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001580000ULL,
      0x0870000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sneh.sn", TILEPRO_OPC_SNEH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009580000ULL,
      0x0c70000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sra", TILEPRO_OPC_SRA, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000001680000ULL,
      0x0878000000000000ULL,
      0x80000000200c0000ULL,
      0xa006000000000000ULL,
      -1ULL
    }
#endif
  },
  { "sra.sn", TILEPRO_OPC_SRA_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009680000ULL,
      0x0c78000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "srab", TILEPRO_OPC_SRAB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001600000ULL,
      0x0874000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "srab.sn", TILEPRO_OPC_SRAB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009600000ULL,
      0x0c74000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "srah", TILEPRO_OPC_SRAH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001640000ULL,
      0x0876000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "srah.sn", TILEPRO_OPC_SRAH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009640000ULL,
      0x0c76000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "srai", TILEPRO_OPC_SRAI, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 11, 12, 34 }, { 13, 14, 35 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0x80000000780e0000ULL,
      0xf807000000000000ULL,
      0ULL
    },
    {
      0x0000000070140000ULL,
      0x400a000000000000ULL,
      0x8000000068080000ULL,
      0xd804000000000000ULL,
      -1ULL
    }
#endif
  },
  { "srai.sn", TILEPRO_OPC_SRAI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078140000ULL,
      0x440a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sraib", TILEPRO_OPC_SRAIB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070100000ULL,
      0x4008000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sraib.sn", TILEPRO_OPC_SRAIB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078100000ULL,
      0x4408000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sraih", TILEPRO_OPC_SRAIH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070120000ULL,
      0x4009000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sraih.sn", TILEPRO_OPC_SRAIH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 32 }, { 7, 8, 33 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffe0000ULL,
      0xffff000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078120000ULL,
      0x4409000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sub", TILEPRO_OPC_SUB, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000001740000ULL,
      0x087e000000000000ULL,
      0x80000000080c0000ULL,
      0x8806000000000000ULL,
      -1ULL
    }
#endif
  },
  { "sub.sn", TILEPRO_OPC_SUB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009740000ULL,
      0x0c7e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subb", TILEPRO_OPC_SUBB, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000016c0000ULL,
      0x087a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subb.sn", TILEPRO_OPC_SUBB_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x00000000096c0000ULL,
      0x0c7a000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subbs_u", TILEPRO_OPC_SUBBS_U, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001900000ULL,
      0x088c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subbs_u.sn", TILEPRO_OPC_SUBBS_U_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009900000ULL,
      0x0c8c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subh", TILEPRO_OPC_SUBH, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001700000ULL,
      0x087c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subh.sn", TILEPRO_OPC_SUBH_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009700000ULL,
      0x0c7c000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subhs", TILEPRO_OPC_SUBHS, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001940000ULL,
      0x088e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subhs.sn", TILEPRO_OPC_SUBHS_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009940000ULL,
      0x0c8e000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subs", TILEPRO_OPC_SUBS, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000001840000ULL,
      0x0886000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "subs.sn", TILEPRO_OPC_SUBS_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009840000ULL,
      0x0c86000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "sw", TILEPRO_OPC_SW, 0x12, 2, TREG_ZERO, 1,
    { { 0, }, { 8, 17 }, { 0, }, { 0, }, { 15, 36 } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfe000000000000ULL,
      0ULL,
      0ULL,
      0x8700000000000000ULL
    },
    {
      -1ULL,
      0x0880000000000000ULL,
      -1ULL,
      -1ULL,
      0x8700000000000000ULL
    }
#endif
  },
  { "swadd", TILEPRO_OPC_SWADD, 0x2, 3, TREG_ZERO, 1,
    { { 0, }, { 24, 17, 37 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbf8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x30f0000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "swint0", TILEPRO_OPC_SWINT0, 0x2, 0, TREG_ZERO, 0,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b900000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "swint1", TILEPRO_OPC_SWINT1, 0x2, 0, TREG_ZERO, 0,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400b980000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "swint2", TILEPRO_OPC_SWINT2, 0x2, 0, TREG_ZERO, 0,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400ba00000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "swint3", TILEPRO_OPC_SWINT3, 0x2, 0, TREG_ZERO, 0,
    { { 0, }, {  }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400ba80000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tblidxb0", TILEPRO_OPC_TBLIDXB0, 0x5, 2, TREG_ZERO, 1,
    { { 21, 10 }, { 0, }, { 31, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070168000ULL,
      -1ULL,
      0x80000000680a8000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tblidxb0.sn", TILEPRO_OPC_TBLIDXB0_SN, 0x1, 2, TREG_SN, 1,
    { { 21, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078168000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tblidxb1", TILEPRO_OPC_TBLIDXB1, 0x5, 2, TREG_ZERO, 1,
    { { 21, 10 }, { 0, }, { 31, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000070169000ULL,
      -1ULL,
      0x80000000680a9000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tblidxb1.sn", TILEPRO_OPC_TBLIDXB1_SN, 0x1, 2, TREG_SN, 1,
    { { 21, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000078169000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tblidxb2", TILEPRO_OPC_TBLIDXB2, 0x5, 2, TREG_ZERO, 1,
    { { 21, 10 }, { 0, }, { 31, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x000000007016a000ULL,
      -1ULL,
      0x80000000680aa000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tblidxb2.sn", TILEPRO_OPC_TBLIDXB2_SN, 0x1, 2, TREG_SN, 1,
    { { 21, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x000000007816a000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tblidxb3", TILEPRO_OPC_TBLIDXB3, 0x5, 2, TREG_ZERO, 1,
    { { 21, 10 }, { 0, }, { 31, 12 }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0x80000000780ff000ULL,
      0ULL,
      0ULL
    },
    {
      0x000000007016b000ULL,
      -1ULL,
      0x80000000680ab000ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tblidxb3.sn", TILEPRO_OPC_TBLIDXB3_SN, 0x1, 2, TREG_SN, 1,
    { { 21, 10 }, { 0, }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffff000ULL,
      0ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x000000007816b000ULL,
      -1ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tns", TILEPRO_OPC_TNS, 0x2, 2, TREG_ZERO, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400bb00000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "tns.sn", TILEPRO_OPC_TNS_SN, 0x2, 2, TREG_SN, 1,
    { { 0, }, { 7, 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfffff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x440bb00000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "wh64", TILEPRO_OPC_WH64, 0x2, 1, TREG_ZERO, 1,
    { { 0, }, { 8 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0ULL,
      0xfbfff80000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      -1ULL,
      0x400bb80000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "xor", TILEPRO_OPC_XOR, 0xf, 3, TREG_ZERO, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 11, 12, 18 }, { 13, 14, 19 }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0x80000000780c0000ULL,
      0xf806000000000000ULL,
      0ULL
    },
    {
      0x0000000001780000ULL,
      0x0882000000000000ULL,
      0x80000000180c0000ULL,
      0x9806000000000000ULL,
      -1ULL
    }
#endif
  },
  { "xor.sn", TILEPRO_OPC_XOR_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 16 }, { 7, 8, 17 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ffc0000ULL,
      0xfffe000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000009780000ULL,
      0x0c82000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "xori", TILEPRO_OPC_XORI, 0x3, 3, TREG_ZERO, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000050200000ULL,
      0x30a8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { "xori.sn", TILEPRO_OPC_XORI_SN, 0x3, 3, TREG_SN, 1,
    { { 9, 10, 0 }, { 7, 8, 1 }, { 0, }, { 0, }, { 0, } },
#ifndef DISASM_ONLY
    {
      0x800000007ff00000ULL,
      0xfff8000000000000ULL,
      0ULL,
      0ULL,
      0ULL
    },
    {
      0x0000000058200000ULL,
      0x34a8000000000000ULL,
      -1ULL,
      -1ULL,
      -1ULL
    }
#endif
  },
  { NULL, TILEPRO_OPC_NONE, 0, 0, TREG_ZERO, 0, { { 0, } },
#ifndef DISASM_ONLY
    { 0, }, { 0, }
#endif
  }
};

#define BITFIELD(start, size) ((start) | (((1 << (size)) - 1) << 6))
#define CHILD(array_index) (TILEPRO_OPC_NONE + (array_index))

static const unsigned short decode_X0_fsm[1153] =
{
  BITFIELD(22, 9) /* index 0 */,
  CHILD(513), CHILD(530), CHILD(547), CHILD(564), CHILD(596), CHILD(613),
  CHILD(630), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, CHILD(663), CHILD(680), CHILD(697),
  CHILD(714), CHILD(746), CHILD(763), CHILD(780), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(813),
  CHILD(813), CHILD(813), CHILD(813), CHILD(813), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828), CHILD(828),
  CHILD(828), CHILD(828), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(873), CHILD(878), CHILD(883), CHILD(903), CHILD(908),
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, CHILD(913),
  CHILD(918), CHILD(923), CHILD(943), CHILD(948), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, CHILD(953), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, CHILD(988), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, CHILD(993), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, CHILD(1076), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(18, 4) /* index 513 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ADDB, TILEPRO_OPC_ADDH, TILEPRO_OPC_ADD,
  TILEPRO_OPC_ADIFFB_U, TILEPRO_OPC_ADIFFH, TILEPRO_OPC_AND,
  TILEPRO_OPC_AVGB_U, TILEPRO_OPC_AVGH, TILEPRO_OPC_CRC32_32,
  TILEPRO_OPC_CRC32_8, TILEPRO_OPC_INTHB, TILEPRO_OPC_INTHH,
  TILEPRO_OPC_INTLB, TILEPRO_OPC_INTLH, TILEPRO_OPC_MAXB_U,
  BITFIELD(18, 4) /* index 530 */,
  TILEPRO_OPC_MAXH, TILEPRO_OPC_MINB_U, TILEPRO_OPC_MINH, TILEPRO_OPC_MNZB,
  TILEPRO_OPC_MNZH, TILEPRO_OPC_MNZ, TILEPRO_OPC_MULHHA_SS,
  TILEPRO_OPC_MULHHA_SU, TILEPRO_OPC_MULHHA_UU, TILEPRO_OPC_MULHHSA_UU,
  TILEPRO_OPC_MULHH_SS, TILEPRO_OPC_MULHH_SU, TILEPRO_OPC_MULHH_UU,
  TILEPRO_OPC_MULHLA_SS, TILEPRO_OPC_MULHLA_SU, TILEPRO_OPC_MULHLA_US,
  BITFIELD(18, 4) /* index 547 */,
  TILEPRO_OPC_MULHLA_UU, TILEPRO_OPC_MULHLSA_UU, TILEPRO_OPC_MULHL_SS,
  TILEPRO_OPC_MULHL_SU, TILEPRO_OPC_MULHL_US, TILEPRO_OPC_MULHL_UU,
  TILEPRO_OPC_MULLLA_SS, TILEPRO_OPC_MULLLA_SU, TILEPRO_OPC_MULLLA_UU,
  TILEPRO_OPC_MULLLSA_UU, TILEPRO_OPC_MULLL_SS, TILEPRO_OPC_MULLL_SU,
  TILEPRO_OPC_MULLL_UU, TILEPRO_OPC_MVNZ, TILEPRO_OPC_MVZ, TILEPRO_OPC_MZB,
  BITFIELD(18, 4) /* index 564 */,
  TILEPRO_OPC_MZH, TILEPRO_OPC_MZ, TILEPRO_OPC_NOR, CHILD(581),
  TILEPRO_OPC_PACKHB, TILEPRO_OPC_PACKLB, TILEPRO_OPC_RL, TILEPRO_OPC_S1A,
  TILEPRO_OPC_S2A, TILEPRO_OPC_S3A, TILEPRO_OPC_SADAB_U, TILEPRO_OPC_SADAH,
  TILEPRO_OPC_SADAH_U, TILEPRO_OPC_SADB_U, TILEPRO_OPC_SADH,
  TILEPRO_OPC_SADH_U,
  BITFIELD(12, 2) /* index 581 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, CHILD(586),
  BITFIELD(14, 2) /* index 586 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, CHILD(591),
  BITFIELD(16, 2) /* index 591 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_MOVE,
  BITFIELD(18, 4) /* index 596 */,
  TILEPRO_OPC_SEQB, TILEPRO_OPC_SEQH, TILEPRO_OPC_SEQ, TILEPRO_OPC_SHLB,
  TILEPRO_OPC_SHLH, TILEPRO_OPC_SHL, TILEPRO_OPC_SHRB, TILEPRO_OPC_SHRH,
  TILEPRO_OPC_SHR, TILEPRO_OPC_SLTB, TILEPRO_OPC_SLTB_U, TILEPRO_OPC_SLTEB,
  TILEPRO_OPC_SLTEB_U, TILEPRO_OPC_SLTEH, TILEPRO_OPC_SLTEH_U,
  TILEPRO_OPC_SLTE,
  BITFIELD(18, 4) /* index 613 */,
  TILEPRO_OPC_SLTE_U, TILEPRO_OPC_SLTH, TILEPRO_OPC_SLTH_U, TILEPRO_OPC_SLT,
  TILEPRO_OPC_SLT_U, TILEPRO_OPC_SNEB, TILEPRO_OPC_SNEH, TILEPRO_OPC_SNE,
  TILEPRO_OPC_SRAB, TILEPRO_OPC_SRAH, TILEPRO_OPC_SRA, TILEPRO_OPC_SUBB,
  TILEPRO_OPC_SUBH, TILEPRO_OPC_SUB, TILEPRO_OPC_XOR, TILEPRO_OPC_DWORD_ALIGN,
  BITFIELD(18, 3) /* index 630 */,
  CHILD(639), CHILD(642), CHILD(645), CHILD(648), CHILD(651), CHILD(654),
  CHILD(657), CHILD(660),
  BITFIELD(21, 1) /* index 639 */,
  TILEPRO_OPC_ADDS, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 642 */,
  TILEPRO_OPC_SUBS, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 645 */,
  TILEPRO_OPC_ADDBS_U, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 648 */,
  TILEPRO_OPC_ADDHS, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 651 */,
  TILEPRO_OPC_SUBBS_U, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 654 */,
  TILEPRO_OPC_SUBHS, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 657 */,
  TILEPRO_OPC_PACKHS, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 660 */,
  TILEPRO_OPC_PACKBS_U, TILEPRO_OPC_NONE,
  BITFIELD(18, 4) /* index 663 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ADDB_SN, TILEPRO_OPC_ADDH_SN,
  TILEPRO_OPC_ADD_SN, TILEPRO_OPC_ADIFFB_U_SN, TILEPRO_OPC_ADIFFH_SN,
  TILEPRO_OPC_AND_SN, TILEPRO_OPC_AVGB_U_SN, TILEPRO_OPC_AVGH_SN,
  TILEPRO_OPC_CRC32_32_SN, TILEPRO_OPC_CRC32_8_SN, TILEPRO_OPC_INTHB_SN,
  TILEPRO_OPC_INTHH_SN, TILEPRO_OPC_INTLB_SN, TILEPRO_OPC_INTLH_SN,
  TILEPRO_OPC_MAXB_U_SN,
  BITFIELD(18, 4) /* index 680 */,
  TILEPRO_OPC_MAXH_SN, TILEPRO_OPC_MINB_U_SN, TILEPRO_OPC_MINH_SN,
  TILEPRO_OPC_MNZB_SN, TILEPRO_OPC_MNZH_SN, TILEPRO_OPC_MNZ_SN,
  TILEPRO_OPC_MULHHA_SS_SN, TILEPRO_OPC_MULHHA_SU_SN,
  TILEPRO_OPC_MULHHA_UU_SN, TILEPRO_OPC_MULHHSA_UU_SN,
  TILEPRO_OPC_MULHH_SS_SN, TILEPRO_OPC_MULHH_SU_SN, TILEPRO_OPC_MULHH_UU_SN,
  TILEPRO_OPC_MULHLA_SS_SN, TILEPRO_OPC_MULHLA_SU_SN,
  TILEPRO_OPC_MULHLA_US_SN,
  BITFIELD(18, 4) /* index 697 */,
  TILEPRO_OPC_MULHLA_UU_SN, TILEPRO_OPC_MULHLSA_UU_SN,
  TILEPRO_OPC_MULHL_SS_SN, TILEPRO_OPC_MULHL_SU_SN, TILEPRO_OPC_MULHL_US_SN,
  TILEPRO_OPC_MULHL_UU_SN, TILEPRO_OPC_MULLLA_SS_SN, TILEPRO_OPC_MULLLA_SU_SN,
  TILEPRO_OPC_MULLLA_UU_SN, TILEPRO_OPC_MULLLSA_UU_SN,
  TILEPRO_OPC_MULLL_SS_SN, TILEPRO_OPC_MULLL_SU_SN, TILEPRO_OPC_MULLL_UU_SN,
  TILEPRO_OPC_MVNZ_SN, TILEPRO_OPC_MVZ_SN, TILEPRO_OPC_MZB_SN,
  BITFIELD(18, 4) /* index 714 */,
  TILEPRO_OPC_MZH_SN, TILEPRO_OPC_MZ_SN, TILEPRO_OPC_NOR_SN, CHILD(731),
  TILEPRO_OPC_PACKHB_SN, TILEPRO_OPC_PACKLB_SN, TILEPRO_OPC_RL_SN,
  TILEPRO_OPC_S1A_SN, TILEPRO_OPC_S2A_SN, TILEPRO_OPC_S3A_SN,
  TILEPRO_OPC_SADAB_U_SN, TILEPRO_OPC_SADAH_SN, TILEPRO_OPC_SADAH_U_SN,
  TILEPRO_OPC_SADB_U_SN, TILEPRO_OPC_SADH_SN, TILEPRO_OPC_SADH_U_SN,
  BITFIELD(12, 2) /* index 731 */,
  TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, CHILD(736),
  BITFIELD(14, 2) /* index 736 */,
  TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, CHILD(741),
  BITFIELD(16, 2) /* index 741 */,
  TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN,
  TILEPRO_OPC_MOVE_SN,
  BITFIELD(18, 4) /* index 746 */,
  TILEPRO_OPC_SEQB_SN, TILEPRO_OPC_SEQH_SN, TILEPRO_OPC_SEQ_SN,
  TILEPRO_OPC_SHLB_SN, TILEPRO_OPC_SHLH_SN, TILEPRO_OPC_SHL_SN,
  TILEPRO_OPC_SHRB_SN, TILEPRO_OPC_SHRH_SN, TILEPRO_OPC_SHR_SN,
  TILEPRO_OPC_SLTB_SN, TILEPRO_OPC_SLTB_U_SN, TILEPRO_OPC_SLTEB_SN,
  TILEPRO_OPC_SLTEB_U_SN, TILEPRO_OPC_SLTEH_SN, TILEPRO_OPC_SLTEH_U_SN,
  TILEPRO_OPC_SLTE_SN,
  BITFIELD(18, 4) /* index 763 */,
  TILEPRO_OPC_SLTE_U_SN, TILEPRO_OPC_SLTH_SN, TILEPRO_OPC_SLTH_U_SN,
  TILEPRO_OPC_SLT_SN, TILEPRO_OPC_SLT_U_SN, TILEPRO_OPC_SNEB_SN,
  TILEPRO_OPC_SNEH_SN, TILEPRO_OPC_SNE_SN, TILEPRO_OPC_SRAB_SN,
  TILEPRO_OPC_SRAH_SN, TILEPRO_OPC_SRA_SN, TILEPRO_OPC_SUBB_SN,
  TILEPRO_OPC_SUBH_SN, TILEPRO_OPC_SUB_SN, TILEPRO_OPC_XOR_SN,
  TILEPRO_OPC_DWORD_ALIGN_SN,
  BITFIELD(18, 3) /* index 780 */,
  CHILD(789), CHILD(792), CHILD(795), CHILD(798), CHILD(801), CHILD(804),
  CHILD(807), CHILD(810),
  BITFIELD(21, 1) /* index 789 */,
  TILEPRO_OPC_ADDS_SN, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 792 */,
  TILEPRO_OPC_SUBS_SN, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 795 */,
  TILEPRO_OPC_ADDBS_U_SN, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 798 */,
  TILEPRO_OPC_ADDHS_SN, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 801 */,
  TILEPRO_OPC_SUBBS_U_SN, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 804 */,
  TILEPRO_OPC_SUBHS_SN, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 807 */,
  TILEPRO_OPC_PACKHS_SN, TILEPRO_OPC_NONE,
  BITFIELD(21, 1) /* index 810 */,
  TILEPRO_OPC_PACKBS_U_SN, TILEPRO_OPC_NONE,
  BITFIELD(6, 2) /* index 813 */,
  TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN,
  CHILD(818),
  BITFIELD(8, 2) /* index 818 */,
  TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN,
  CHILD(823),
  BITFIELD(10, 2) /* index 823 */,
  TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN,
  TILEPRO_OPC_MOVELI_SN,
  BITFIELD(6, 2) /* index 828 */,
  TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, CHILD(833),
  BITFIELD(8, 2) /* index 833 */,
  TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, CHILD(838),
  BITFIELD(10, 2) /* index 838 */,
  TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, TILEPRO_OPC_MOVELI,
  BITFIELD(0, 2) /* index 843 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(848),
  BITFIELD(2, 2) /* index 848 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(853),
  BITFIELD(4, 2) /* index 853 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(858),
  BITFIELD(6, 2) /* index 858 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(863),
  BITFIELD(8, 2) /* index 863 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(868),
  BITFIELD(10, 2) /* index 868 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_INFOL,
  BITFIELD(20, 2) /* index 873 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ADDIB, TILEPRO_OPC_ADDIH, TILEPRO_OPC_ADDI,
  BITFIELD(20, 2) /* index 878 */,
  TILEPRO_OPC_MAXIB_U, TILEPRO_OPC_MAXIH, TILEPRO_OPC_MINIB_U,
  TILEPRO_OPC_MINIH,
  BITFIELD(20, 2) /* index 883 */,
  CHILD(888), TILEPRO_OPC_SEQIB, TILEPRO_OPC_SEQIH, TILEPRO_OPC_SEQI,
  BITFIELD(6, 2) /* index 888 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, CHILD(893),
  BITFIELD(8, 2) /* index 893 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, CHILD(898),
  BITFIELD(10, 2) /* index 898 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_MOVEI,
  BITFIELD(20, 2) /* index 903 */,
  TILEPRO_OPC_SLTIB, TILEPRO_OPC_SLTIB_U, TILEPRO_OPC_SLTIH,
  TILEPRO_OPC_SLTIH_U,
  BITFIELD(20, 2) /* index 908 */,
  TILEPRO_OPC_SLTI, TILEPRO_OPC_SLTI_U, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(20, 2) /* index 913 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ADDIB_SN, TILEPRO_OPC_ADDIH_SN,
  TILEPRO_OPC_ADDI_SN,
  BITFIELD(20, 2) /* index 918 */,
  TILEPRO_OPC_MAXIB_U_SN, TILEPRO_OPC_MAXIH_SN, TILEPRO_OPC_MINIB_U_SN,
  TILEPRO_OPC_MINIH_SN,
  BITFIELD(20, 2) /* index 923 */,
  CHILD(928), TILEPRO_OPC_SEQIB_SN, TILEPRO_OPC_SEQIH_SN, TILEPRO_OPC_SEQI_SN,
  BITFIELD(6, 2) /* index 928 */,
  TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, CHILD(933),
  BITFIELD(8, 2) /* index 933 */,
  TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, CHILD(938),
  BITFIELD(10, 2) /* index 938 */,
  TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN,
  TILEPRO_OPC_MOVEI_SN,
  BITFIELD(20, 2) /* index 943 */,
  TILEPRO_OPC_SLTIB_SN, TILEPRO_OPC_SLTIB_U_SN, TILEPRO_OPC_SLTIH_SN,
  TILEPRO_OPC_SLTIH_U_SN,
  BITFIELD(20, 2) /* index 948 */,
  TILEPRO_OPC_SLTI_SN, TILEPRO_OPC_SLTI_U_SN, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE,
  BITFIELD(20, 2) /* index 953 */,
  TILEPRO_OPC_NONE, CHILD(958), TILEPRO_OPC_XORI, TILEPRO_OPC_NONE,
  BITFIELD(0, 2) /* index 958 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(963),
  BITFIELD(2, 2) /* index 963 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(968),
  BITFIELD(4, 2) /* index 968 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(973),
  BITFIELD(6, 2) /* index 973 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(978),
  BITFIELD(8, 2) /* index 978 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(983),
  BITFIELD(10, 2) /* index 983 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_INFO,
  BITFIELD(20, 2) /* index 988 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ANDI_SN, TILEPRO_OPC_XORI_SN,
  TILEPRO_OPC_NONE,
  BITFIELD(17, 5) /* index 993 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_RLI, TILEPRO_OPC_SHLIB, TILEPRO_OPC_SHLIH,
  TILEPRO_OPC_SHLI, TILEPRO_OPC_SHRIB, TILEPRO_OPC_SHRIH, TILEPRO_OPC_SHRI,
  TILEPRO_OPC_SRAIB, TILEPRO_OPC_SRAIH, TILEPRO_OPC_SRAI, CHILD(1026),
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(12, 4) /* index 1026 */,
  TILEPRO_OPC_NONE, CHILD(1043), CHILD(1046), CHILD(1049), CHILD(1052),
  CHILD(1055), CHILD(1058), CHILD(1061), CHILD(1064), CHILD(1067),
  CHILD(1070), CHILD(1073), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1043 */,
  TILEPRO_OPC_BITX, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1046 */,
  TILEPRO_OPC_BYTEX, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1049 */,
  TILEPRO_OPC_CLZ, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1052 */,
  TILEPRO_OPC_CTZ, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1055 */,
  TILEPRO_OPC_FNOP, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1058 */,
  TILEPRO_OPC_NOP, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1061 */,
  TILEPRO_OPC_PCNT, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1064 */,
  TILEPRO_OPC_TBLIDXB0, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1067 */,
  TILEPRO_OPC_TBLIDXB1, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1070 */,
  TILEPRO_OPC_TBLIDXB2, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1073 */,
  TILEPRO_OPC_TBLIDXB3, TILEPRO_OPC_NONE,
  BITFIELD(17, 5) /* index 1076 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_RLI_SN, TILEPRO_OPC_SHLIB_SN,
  TILEPRO_OPC_SHLIH_SN, TILEPRO_OPC_SHLI_SN, TILEPRO_OPC_SHRIB_SN,
  TILEPRO_OPC_SHRIH_SN, TILEPRO_OPC_SHRI_SN, TILEPRO_OPC_SRAIB_SN,
  TILEPRO_OPC_SRAIH_SN, TILEPRO_OPC_SRAI_SN, CHILD(1109), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(12, 4) /* index 1109 */,
  TILEPRO_OPC_NONE, CHILD(1126), CHILD(1129), CHILD(1132), CHILD(1135),
  CHILD(1055), CHILD(1058), CHILD(1138), CHILD(1141), CHILD(1144),
  CHILD(1147), CHILD(1150), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1126 */,
  TILEPRO_OPC_BITX_SN, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1129 */,
  TILEPRO_OPC_BYTEX_SN, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1132 */,
  TILEPRO_OPC_CLZ_SN, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1135 */,
  TILEPRO_OPC_CTZ_SN, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1138 */,
  TILEPRO_OPC_PCNT_SN, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1141 */,
  TILEPRO_OPC_TBLIDXB0_SN, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1144 */,
  TILEPRO_OPC_TBLIDXB1_SN, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1147 */,
  TILEPRO_OPC_TBLIDXB2_SN, TILEPRO_OPC_NONE,
  BITFIELD(16, 1) /* index 1150 */,
  TILEPRO_OPC_TBLIDXB3_SN, TILEPRO_OPC_NONE,
};

static const unsigned short decode_X1_fsm[1580] =
{
  BITFIELD(54, 9) /* index 0 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  CHILD(513), CHILD(561), CHILD(594), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, CHILD(641),
  CHILD(689), CHILD(722), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, CHILD(766),
  CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766),
  CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766),
  CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766),
  CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766),
  CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766), CHILD(766),
  CHILD(766), CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781),
  CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781),
  CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781),
  CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781),
  CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781), CHILD(781),
  CHILD(781), CHILD(781), CHILD(781), CHILD(796), CHILD(796), CHILD(796),
  CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796),
  CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796),
  CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796),
  CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796),
  CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(796), CHILD(826),
  CHILD(826), CHILD(826), CHILD(826), CHILD(826), CHILD(826), CHILD(826),
  CHILD(826), CHILD(826), CHILD(826), CHILD(826), CHILD(826), CHILD(826),
  CHILD(826), CHILD(826), CHILD(826), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843), CHILD(843),
  CHILD(843), CHILD(860), CHILD(899), CHILD(923), CHILD(932),
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  CHILD(961), CHILD(970), CHILD(994), CHILD(1003), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM,
  TILEPRO_OPC_MM, TILEPRO_OPC_MM, TILEPRO_OPC_MM, CHILD(1032),
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, CHILD(1374),
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J,
  TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_J, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL,
  TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_JAL, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(49, 5) /* index 513 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ADDB, TILEPRO_OPC_ADDH, TILEPRO_OPC_ADD,
  TILEPRO_OPC_AND, TILEPRO_OPC_INTHB, TILEPRO_OPC_INTHH, TILEPRO_OPC_INTLB,
  TILEPRO_OPC_INTLH, TILEPRO_OPC_JALRP, TILEPRO_OPC_JALR, TILEPRO_OPC_JRP,
  TILEPRO_OPC_JR, TILEPRO_OPC_LNK, TILEPRO_OPC_MAXB_U, TILEPRO_OPC_MAXH,
  TILEPRO_OPC_MINB_U, TILEPRO_OPC_MINH, TILEPRO_OPC_MNZB, TILEPRO_OPC_MNZH,
  TILEPRO_OPC_MNZ, TILEPRO_OPC_MZB, TILEPRO_OPC_MZH, TILEPRO_OPC_MZ,
  TILEPRO_OPC_NOR, CHILD(546), TILEPRO_OPC_PACKHB, TILEPRO_OPC_PACKLB,
  TILEPRO_OPC_RL, TILEPRO_OPC_S1A, TILEPRO_OPC_S2A, TILEPRO_OPC_S3A,
  BITFIELD(43, 2) /* index 546 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, CHILD(551),
  BITFIELD(45, 2) /* index 551 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, CHILD(556),
  BITFIELD(47, 2) /* index 556 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_MOVE,
  BITFIELD(49, 5) /* index 561 */,
  TILEPRO_OPC_SB, TILEPRO_OPC_SEQB, TILEPRO_OPC_SEQH, TILEPRO_OPC_SEQ,
  TILEPRO_OPC_SHLB, TILEPRO_OPC_SHLH, TILEPRO_OPC_SHL, TILEPRO_OPC_SHRB,
  TILEPRO_OPC_SHRH, TILEPRO_OPC_SHR, TILEPRO_OPC_SH, TILEPRO_OPC_SLTB,
  TILEPRO_OPC_SLTB_U, TILEPRO_OPC_SLTEB, TILEPRO_OPC_SLTEB_U,
  TILEPRO_OPC_SLTEH, TILEPRO_OPC_SLTEH_U, TILEPRO_OPC_SLTE,
  TILEPRO_OPC_SLTE_U, TILEPRO_OPC_SLTH, TILEPRO_OPC_SLTH_U, TILEPRO_OPC_SLT,
  TILEPRO_OPC_SLT_U, TILEPRO_OPC_SNEB, TILEPRO_OPC_SNEH, TILEPRO_OPC_SNE,
  TILEPRO_OPC_SRAB, TILEPRO_OPC_SRAH, TILEPRO_OPC_SRA, TILEPRO_OPC_SUBB,
  TILEPRO_OPC_SUBH, TILEPRO_OPC_SUB,
  BITFIELD(49, 4) /* index 594 */,
  CHILD(611), CHILD(614), CHILD(617), CHILD(620), CHILD(623), CHILD(626),
  CHILD(629), CHILD(632), CHILD(635), CHILD(638), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 611 */,
  TILEPRO_OPC_SW, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 614 */,
  TILEPRO_OPC_XOR, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 617 */,
  TILEPRO_OPC_ADDS, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 620 */,
  TILEPRO_OPC_SUBS, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 623 */,
  TILEPRO_OPC_ADDBS_U, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 626 */,
  TILEPRO_OPC_ADDHS, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 629 */,
  TILEPRO_OPC_SUBBS_U, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 632 */,
  TILEPRO_OPC_SUBHS, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 635 */,
  TILEPRO_OPC_PACKHS, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 638 */,
  TILEPRO_OPC_PACKBS_U, TILEPRO_OPC_NONE,
  BITFIELD(49, 5) /* index 641 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ADDB_SN, TILEPRO_OPC_ADDH_SN,
  TILEPRO_OPC_ADD_SN, TILEPRO_OPC_AND_SN, TILEPRO_OPC_INTHB_SN,
  TILEPRO_OPC_INTHH_SN, TILEPRO_OPC_INTLB_SN, TILEPRO_OPC_INTLH_SN,
  TILEPRO_OPC_JALRP, TILEPRO_OPC_JALR, TILEPRO_OPC_JRP, TILEPRO_OPC_JR,
  TILEPRO_OPC_LNK_SN, TILEPRO_OPC_MAXB_U_SN, TILEPRO_OPC_MAXH_SN,
  TILEPRO_OPC_MINB_U_SN, TILEPRO_OPC_MINH_SN, TILEPRO_OPC_MNZB_SN,
  TILEPRO_OPC_MNZH_SN, TILEPRO_OPC_MNZ_SN, TILEPRO_OPC_MZB_SN,
  TILEPRO_OPC_MZH_SN, TILEPRO_OPC_MZ_SN, TILEPRO_OPC_NOR_SN, CHILD(674),
  TILEPRO_OPC_PACKHB_SN, TILEPRO_OPC_PACKLB_SN, TILEPRO_OPC_RL_SN,
  TILEPRO_OPC_S1A_SN, TILEPRO_OPC_S2A_SN, TILEPRO_OPC_S3A_SN,
  BITFIELD(43, 2) /* index 674 */,
  TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, CHILD(679),
  BITFIELD(45, 2) /* index 679 */,
  TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, CHILD(684),
  BITFIELD(47, 2) /* index 684 */,
  TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN, TILEPRO_OPC_OR_SN,
  TILEPRO_OPC_MOVE_SN,
  BITFIELD(49, 5) /* index 689 */,
  TILEPRO_OPC_SB, TILEPRO_OPC_SEQB_SN, TILEPRO_OPC_SEQH_SN,
  TILEPRO_OPC_SEQ_SN, TILEPRO_OPC_SHLB_SN, TILEPRO_OPC_SHLH_SN,
  TILEPRO_OPC_SHL_SN, TILEPRO_OPC_SHRB_SN, TILEPRO_OPC_SHRH_SN,
  TILEPRO_OPC_SHR_SN, TILEPRO_OPC_SH, TILEPRO_OPC_SLTB_SN,
  TILEPRO_OPC_SLTB_U_SN, TILEPRO_OPC_SLTEB_SN, TILEPRO_OPC_SLTEB_U_SN,
  TILEPRO_OPC_SLTEH_SN, TILEPRO_OPC_SLTEH_U_SN, TILEPRO_OPC_SLTE_SN,
  TILEPRO_OPC_SLTE_U_SN, TILEPRO_OPC_SLTH_SN, TILEPRO_OPC_SLTH_U_SN,
  TILEPRO_OPC_SLT_SN, TILEPRO_OPC_SLT_U_SN, TILEPRO_OPC_SNEB_SN,
  TILEPRO_OPC_SNEH_SN, TILEPRO_OPC_SNE_SN, TILEPRO_OPC_SRAB_SN,
  TILEPRO_OPC_SRAH_SN, TILEPRO_OPC_SRA_SN, TILEPRO_OPC_SUBB_SN,
  TILEPRO_OPC_SUBH_SN, TILEPRO_OPC_SUB_SN,
  BITFIELD(49, 4) /* index 722 */,
  CHILD(611), CHILD(739), CHILD(742), CHILD(745), CHILD(748), CHILD(751),
  CHILD(754), CHILD(757), CHILD(760), CHILD(763), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 739 */,
  TILEPRO_OPC_XOR_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 742 */,
  TILEPRO_OPC_ADDS_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 745 */,
  TILEPRO_OPC_SUBS_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 748 */,
  TILEPRO_OPC_ADDBS_U_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 751 */,
  TILEPRO_OPC_ADDHS_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 754 */,
  TILEPRO_OPC_SUBBS_U_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 757 */,
  TILEPRO_OPC_SUBHS_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 760 */,
  TILEPRO_OPC_PACKHS_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 763 */,
  TILEPRO_OPC_PACKBS_U_SN, TILEPRO_OPC_NONE,
  BITFIELD(37, 2) /* index 766 */,
  TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN,
  CHILD(771),
  BITFIELD(39, 2) /* index 771 */,
  TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN,
  CHILD(776),
  BITFIELD(41, 2) /* index 776 */,
  TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN, TILEPRO_OPC_ADDLI_SN,
  TILEPRO_OPC_MOVELI_SN,
  BITFIELD(37, 2) /* index 781 */,
  TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, CHILD(786),
  BITFIELD(39, 2) /* index 786 */,
  TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, CHILD(791),
  BITFIELD(41, 2) /* index 791 */,
  TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, TILEPRO_OPC_ADDLI, TILEPRO_OPC_MOVELI,
  BITFIELD(31, 2) /* index 796 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(801),
  BITFIELD(33, 2) /* index 801 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(806),
  BITFIELD(35, 2) /* index 806 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(811),
  BITFIELD(37, 2) /* index 811 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(816),
  BITFIELD(39, 2) /* index 816 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, CHILD(821),
  BITFIELD(41, 2) /* index 821 */,
  TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_AULI, TILEPRO_OPC_INFOL,
  BITFIELD(31, 4) /* index 826 */,
  TILEPRO_OPC_BZ, TILEPRO_OPC_BZT, TILEPRO_OPC_BNZ, TILEPRO_OPC_BNZT,
  TILEPRO_OPC_BGZ, TILEPRO_OPC_BGZT, TILEPRO_OPC_BGEZ, TILEPRO_OPC_BGEZT,
  TILEPRO_OPC_BLZ, TILEPRO_OPC_BLZT, TILEPRO_OPC_BLEZ, TILEPRO_OPC_BLEZT,
  TILEPRO_OPC_BBS, TILEPRO_OPC_BBST, TILEPRO_OPC_BBNS, TILEPRO_OPC_BBNST,
  BITFIELD(31, 4) /* index 843 */,
  TILEPRO_OPC_BZ_SN, TILEPRO_OPC_BZT_SN, TILEPRO_OPC_BNZ_SN,
  TILEPRO_OPC_BNZT_SN, TILEPRO_OPC_BGZ_SN, TILEPRO_OPC_BGZT_SN,
  TILEPRO_OPC_BGEZ_SN, TILEPRO_OPC_BGEZT_SN, TILEPRO_OPC_BLZ_SN,
  TILEPRO_OPC_BLZT_SN, TILEPRO_OPC_BLEZ_SN, TILEPRO_OPC_BLEZT_SN,
  TILEPRO_OPC_BBS_SN, TILEPRO_OPC_BBST_SN, TILEPRO_OPC_BBNS_SN,
  TILEPRO_OPC_BBNST_SN,
  BITFIELD(51, 3) /* index 860 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ADDIB, TILEPRO_OPC_ADDIH, TILEPRO_OPC_ADDI,
  CHILD(869), TILEPRO_OPC_MAXIB_U, TILEPRO_OPC_MAXIH, TILEPRO_OPC_MFSPR,
  BITFIELD(31, 2) /* index 869 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(874),
  BITFIELD(33, 2) /* index 874 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(879),
  BITFIELD(35, 2) /* index 879 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(884),
  BITFIELD(37, 2) /* index 884 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(889),
  BITFIELD(39, 2) /* index 889 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(894),
  BITFIELD(41, 2) /* index 894 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_INFO,
  BITFIELD(51, 3) /* index 899 */,
  TILEPRO_OPC_MINIB_U, TILEPRO_OPC_MINIH, TILEPRO_OPC_MTSPR, CHILD(908),
  TILEPRO_OPC_SEQIB, TILEPRO_OPC_SEQIH, TILEPRO_OPC_SEQI, TILEPRO_OPC_SLTIB,
  BITFIELD(37, 2) /* index 908 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, CHILD(913),
  BITFIELD(39, 2) /* index 913 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, CHILD(918),
  BITFIELD(41, 2) /* index 918 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_MOVEI,
  BITFIELD(51, 3) /* index 923 */,
  TILEPRO_OPC_SLTIB_U, TILEPRO_OPC_SLTIH, TILEPRO_OPC_SLTIH_U,
  TILEPRO_OPC_SLTI, TILEPRO_OPC_SLTI_U, TILEPRO_OPC_XORI, TILEPRO_OPC_LBADD,
  TILEPRO_OPC_LBADD_U,
  BITFIELD(51, 3) /* index 932 */,
  TILEPRO_OPC_LHADD, TILEPRO_OPC_LHADD_U, CHILD(941), TILEPRO_OPC_LWADD_NA,
  TILEPRO_OPC_SBADD, TILEPRO_OPC_SHADD, TILEPRO_OPC_SWADD, TILEPRO_OPC_NONE,
  BITFIELD(43, 2) /* index 941 */,
  CHILD(946), TILEPRO_OPC_LWADD, TILEPRO_OPC_LWADD, TILEPRO_OPC_LWADD,
  BITFIELD(45, 2) /* index 946 */,
  CHILD(951), TILEPRO_OPC_LWADD, TILEPRO_OPC_LWADD, TILEPRO_OPC_LWADD,
  BITFIELD(47, 2) /* index 951 */,
  CHILD(956), TILEPRO_OPC_LWADD, TILEPRO_OPC_LWADD, TILEPRO_OPC_LWADD,
  BITFIELD(49, 2) /* index 956 */,
  TILEPRO_OPC_LW_TLS, TILEPRO_OPC_LWADD, TILEPRO_OPC_LWADD, TILEPRO_OPC_LWADD,
  BITFIELD(51, 3) /* index 961 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_ADDIB_SN, TILEPRO_OPC_ADDIH_SN,
  TILEPRO_OPC_ADDI_SN, TILEPRO_OPC_ANDI_SN, TILEPRO_OPC_MAXIB_U_SN,
  TILEPRO_OPC_MAXIH_SN, TILEPRO_OPC_MFSPR,
  BITFIELD(51, 3) /* index 970 */,
  TILEPRO_OPC_MINIB_U_SN, TILEPRO_OPC_MINIH_SN, TILEPRO_OPC_MTSPR, CHILD(979),
  TILEPRO_OPC_SEQIB_SN, TILEPRO_OPC_SEQIH_SN, TILEPRO_OPC_SEQI_SN,
  TILEPRO_OPC_SLTIB_SN,
  BITFIELD(37, 2) /* index 979 */,
  TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, CHILD(984),
  BITFIELD(39, 2) /* index 984 */,
  TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, CHILD(989),
  BITFIELD(41, 2) /* index 989 */,
  TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN, TILEPRO_OPC_ORI_SN,
  TILEPRO_OPC_MOVEI_SN,
  BITFIELD(51, 3) /* index 994 */,
  TILEPRO_OPC_SLTIB_U_SN, TILEPRO_OPC_SLTIH_SN, TILEPRO_OPC_SLTIH_U_SN,
  TILEPRO_OPC_SLTI_SN, TILEPRO_OPC_SLTI_U_SN, TILEPRO_OPC_XORI_SN,
  TILEPRO_OPC_LBADD_SN, TILEPRO_OPC_LBADD_U_SN,
  BITFIELD(51, 3) /* index 1003 */,
  TILEPRO_OPC_LHADD_SN, TILEPRO_OPC_LHADD_U_SN, CHILD(1012),
  TILEPRO_OPC_LWADD_NA_SN, TILEPRO_OPC_SBADD, TILEPRO_OPC_SHADD,
  TILEPRO_OPC_SWADD, TILEPRO_OPC_NONE,
  BITFIELD(43, 2) /* index 1012 */,
  CHILD(1017), TILEPRO_OPC_LWADD_SN, TILEPRO_OPC_LWADD_SN,
  TILEPRO_OPC_LWADD_SN,
  BITFIELD(45, 2) /* index 1017 */,
  CHILD(1022), TILEPRO_OPC_LWADD_SN, TILEPRO_OPC_LWADD_SN,
  TILEPRO_OPC_LWADD_SN,
  BITFIELD(47, 2) /* index 1022 */,
  CHILD(1027), TILEPRO_OPC_LWADD_SN, TILEPRO_OPC_LWADD_SN,
  TILEPRO_OPC_LWADD_SN,
  BITFIELD(49, 2) /* index 1027 */,
  TILEPRO_OPC_LW_TLS_SN, TILEPRO_OPC_LWADD_SN, TILEPRO_OPC_LWADD_SN,
  TILEPRO_OPC_LWADD_SN,
  BITFIELD(46, 7) /* index 1032 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  CHILD(1161), CHILD(1161), CHILD(1161), CHILD(1161), CHILD(1164),
  CHILD(1164), CHILD(1164), CHILD(1164), CHILD(1167), CHILD(1167),
  CHILD(1167), CHILD(1167), CHILD(1170), CHILD(1170), CHILD(1170),
  CHILD(1170), CHILD(1173), CHILD(1173), CHILD(1173), CHILD(1173),
  CHILD(1176), CHILD(1176), CHILD(1176), CHILD(1176), CHILD(1179),
  CHILD(1179), CHILD(1179), CHILD(1179), CHILD(1182), CHILD(1182),
  CHILD(1182), CHILD(1182), CHILD(1185), CHILD(1185), CHILD(1185),
  CHILD(1185), CHILD(1188), CHILD(1188), CHILD(1188), CHILD(1188),
  CHILD(1191), CHILD(1282), CHILD(1330), CHILD(1363), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1161 */,
  TILEPRO_OPC_RLI, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1164 */,
  TILEPRO_OPC_SHLIB, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1167 */,
  TILEPRO_OPC_SHLIH, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1170 */,
  TILEPRO_OPC_SHLI, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1173 */,
  TILEPRO_OPC_SHRIB, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1176 */,
  TILEPRO_OPC_SHRIH, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1179 */,
  TILEPRO_OPC_SHRI, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1182 */,
  TILEPRO_OPC_SRAIB, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1185 */,
  TILEPRO_OPC_SRAIH, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1188 */,
  TILEPRO_OPC_SRAI, TILEPRO_OPC_NONE,
  BITFIELD(43, 3) /* index 1191 */,
  TILEPRO_OPC_NONE, CHILD(1200), CHILD(1203), CHILD(1206), CHILD(1209),
  CHILD(1212), CHILD(1215), CHILD(1218),
  BITFIELD(53, 1) /* index 1200 */,
  TILEPRO_OPC_DRAIN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1203 */,
  TILEPRO_OPC_DTLBPR, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1206 */,
  TILEPRO_OPC_FINV, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1209 */,
  TILEPRO_OPC_FLUSH, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1212 */,
  TILEPRO_OPC_FNOP, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1215 */,
  TILEPRO_OPC_ICOH, TILEPRO_OPC_NONE,
  BITFIELD(31, 2) /* index 1218 */,
  CHILD(1223), CHILD(1251), CHILD(1279), CHILD(1279),
  BITFIELD(53, 1) /* index 1223 */,
  CHILD(1226), TILEPRO_OPC_NONE,
  BITFIELD(33, 2) /* index 1226 */,
  TILEPRO_OPC_ILL, TILEPRO_OPC_ILL, TILEPRO_OPC_ILL, CHILD(1231),
  BITFIELD(35, 2) /* index 1231 */,
  TILEPRO_OPC_ILL, CHILD(1236), TILEPRO_OPC_ILL, TILEPRO_OPC_ILL,
  BITFIELD(37, 2) /* index 1236 */,
  TILEPRO_OPC_ILL, CHILD(1241), TILEPRO_OPC_ILL, TILEPRO_OPC_ILL,
  BITFIELD(39, 2) /* index 1241 */,
  TILEPRO_OPC_ILL, CHILD(1246), TILEPRO_OPC_ILL, TILEPRO_OPC_ILL,
  BITFIELD(41, 2) /* index 1246 */,
  TILEPRO_OPC_ILL, TILEPRO_OPC_ILL, TILEPRO_OPC_BPT, TILEPRO_OPC_ILL,
  BITFIELD(53, 1) /* index 1251 */,
  CHILD(1254), TILEPRO_OPC_NONE,
  BITFIELD(33, 2) /* index 1254 */,
  TILEPRO_OPC_ILL, TILEPRO_OPC_ILL, TILEPRO_OPC_ILL, CHILD(1259),
  BITFIELD(35, 2) /* index 1259 */,
  TILEPRO_OPC_ILL, CHILD(1264), TILEPRO_OPC_ILL, TILEPRO_OPC_ILL,
  BITFIELD(37, 2) /* index 1264 */,
  TILEPRO_OPC_ILL, CHILD(1269), TILEPRO_OPC_ILL, TILEPRO_OPC_ILL,
  BITFIELD(39, 2) /* index 1269 */,
  TILEPRO_OPC_ILL, CHILD(1274), TILEPRO_OPC_ILL, TILEPRO_OPC_ILL,
  BITFIELD(41, 2) /* index 1274 */,
  TILEPRO_OPC_ILL, TILEPRO_OPC_ILL, TILEPRO_OPC_RAISE, TILEPRO_OPC_ILL,
  BITFIELD(53, 1) /* index 1279 */,
  TILEPRO_OPC_ILL, TILEPRO_OPC_NONE,
  BITFIELD(43, 3) /* index 1282 */,
  CHILD(1291), CHILD(1294), CHILD(1297), CHILD(1315), CHILD(1318),
  CHILD(1321), CHILD(1324), CHILD(1327),
  BITFIELD(53, 1) /* index 1291 */,
  TILEPRO_OPC_INV, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1294 */,
  TILEPRO_OPC_IRET, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1297 */,
  CHILD(1300), TILEPRO_OPC_NONE,
  BITFIELD(31, 2) /* index 1300 */,
  TILEPRO_OPC_LB, TILEPRO_OPC_LB, TILEPRO_OPC_LB, CHILD(1305),
  BITFIELD(33, 2) /* index 1305 */,
  TILEPRO_OPC_LB, TILEPRO_OPC_LB, TILEPRO_OPC_LB, CHILD(1310),
  BITFIELD(35, 2) /* index 1310 */,
  TILEPRO_OPC_LB, TILEPRO_OPC_LB, TILEPRO_OPC_LB, TILEPRO_OPC_PREFETCH,
  BITFIELD(53, 1) /* index 1315 */,
  TILEPRO_OPC_LB_U, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1318 */,
  TILEPRO_OPC_LH, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1321 */,
  TILEPRO_OPC_LH_U, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1324 */,
  TILEPRO_OPC_LW, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1327 */,
  TILEPRO_OPC_MF, TILEPRO_OPC_NONE,
  BITFIELD(43, 3) /* index 1330 */,
  CHILD(1339), CHILD(1342), CHILD(1345), CHILD(1348), CHILD(1351),
  CHILD(1354), CHILD(1357), CHILD(1360),
  BITFIELD(53, 1) /* index 1339 */,
  TILEPRO_OPC_NAP, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1342 */,
  TILEPRO_OPC_NOP, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1345 */,
  TILEPRO_OPC_SWINT0, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1348 */,
  TILEPRO_OPC_SWINT1, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1351 */,
  TILEPRO_OPC_SWINT2, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1354 */,
  TILEPRO_OPC_SWINT3, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1357 */,
  TILEPRO_OPC_TNS, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1360 */,
  TILEPRO_OPC_WH64, TILEPRO_OPC_NONE,
  BITFIELD(43, 2) /* index 1363 */,
  CHILD(1368), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(45, 1) /* index 1368 */,
  CHILD(1371), TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1371 */,
  TILEPRO_OPC_LW_NA, TILEPRO_OPC_NONE,
  BITFIELD(46, 7) /* index 1374 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  CHILD(1503), CHILD(1503), CHILD(1503), CHILD(1503), CHILD(1506),
  CHILD(1506), CHILD(1506), CHILD(1506), CHILD(1509), CHILD(1509),
  CHILD(1509), CHILD(1509), CHILD(1512), CHILD(1512), CHILD(1512),
  CHILD(1512), CHILD(1515), CHILD(1515), CHILD(1515), CHILD(1515),
  CHILD(1518), CHILD(1518), CHILD(1518), CHILD(1518), CHILD(1521),
  CHILD(1521), CHILD(1521), CHILD(1521), CHILD(1524), CHILD(1524),
  CHILD(1524), CHILD(1524), CHILD(1527), CHILD(1527), CHILD(1527),
  CHILD(1527), CHILD(1530), CHILD(1530), CHILD(1530), CHILD(1530),
  CHILD(1191), CHILD(1533), CHILD(1557), CHILD(1569), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1503 */,
  TILEPRO_OPC_RLI_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1506 */,
  TILEPRO_OPC_SHLIB_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1509 */,
  TILEPRO_OPC_SHLIH_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1512 */,
  TILEPRO_OPC_SHLI_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1515 */,
  TILEPRO_OPC_SHRIB_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1518 */,
  TILEPRO_OPC_SHRIH_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1521 */,
  TILEPRO_OPC_SHRI_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1524 */,
  TILEPRO_OPC_SRAIB_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1527 */,
  TILEPRO_OPC_SRAIH_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1530 */,
  TILEPRO_OPC_SRAI_SN, TILEPRO_OPC_NONE,
  BITFIELD(43, 3) /* index 1533 */,
  CHILD(1291), CHILD(1294), CHILD(1542), CHILD(1545), CHILD(1548),
  CHILD(1551), CHILD(1554), CHILD(1327),
  BITFIELD(53, 1) /* index 1542 */,
  TILEPRO_OPC_LB_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1545 */,
  TILEPRO_OPC_LB_U_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1548 */,
  TILEPRO_OPC_LH_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1551 */,
  TILEPRO_OPC_LH_U_SN, TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1554 */,
  TILEPRO_OPC_LW_SN, TILEPRO_OPC_NONE,
  BITFIELD(43, 3) /* index 1557 */,
  CHILD(1339), CHILD(1342), CHILD(1345), CHILD(1348), CHILD(1351),
  CHILD(1354), CHILD(1566), CHILD(1360),
  BITFIELD(53, 1) /* index 1566 */,
  TILEPRO_OPC_TNS_SN, TILEPRO_OPC_NONE,
  BITFIELD(43, 2) /* index 1569 */,
  CHILD(1574), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(45, 1) /* index 1574 */,
  CHILD(1577), TILEPRO_OPC_NONE,
  BITFIELD(53, 1) /* index 1577 */,
  TILEPRO_OPC_LW_NA_SN, TILEPRO_OPC_NONE,
};

static const unsigned short decode_Y0_fsm[168] =
{
  BITFIELD(27, 4) /* index 0 */,
  TILEPRO_OPC_NONE, CHILD(17), CHILD(22), CHILD(27), CHILD(47), CHILD(52),
  CHILD(57), CHILD(62), CHILD(67), TILEPRO_OPC_ADDI, CHILD(72), CHILD(102),
  TILEPRO_OPC_SEQI, CHILD(117), TILEPRO_OPC_SLTI, TILEPRO_OPC_SLTI_U,
  BITFIELD(18, 2) /* index 17 */,
  TILEPRO_OPC_ADD, TILEPRO_OPC_S1A, TILEPRO_OPC_S2A, TILEPRO_OPC_SUB,
  BITFIELD(18, 2) /* index 22 */,
  TILEPRO_OPC_MNZ, TILEPRO_OPC_MVNZ, TILEPRO_OPC_MVZ, TILEPRO_OPC_MZ,
  BITFIELD(18, 2) /* index 27 */,
  TILEPRO_OPC_AND, TILEPRO_OPC_NOR, CHILD(32), TILEPRO_OPC_XOR,
  BITFIELD(12, 2) /* index 32 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, CHILD(37),
  BITFIELD(14, 2) /* index 37 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, CHILD(42),
  BITFIELD(16, 2) /* index 42 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_MOVE,
  BITFIELD(18, 2) /* index 47 */,
  TILEPRO_OPC_RL, TILEPRO_OPC_SHL, TILEPRO_OPC_SHR, TILEPRO_OPC_SRA,
  BITFIELD(18, 2) /* index 52 */,
  TILEPRO_OPC_SLTE, TILEPRO_OPC_SLTE_U, TILEPRO_OPC_SLT, TILEPRO_OPC_SLT_U,
  BITFIELD(18, 2) /* index 57 */,
  TILEPRO_OPC_MULHLSA_UU, TILEPRO_OPC_S3A, TILEPRO_OPC_SEQ, TILEPRO_OPC_SNE,
  BITFIELD(18, 2) /* index 62 */,
  TILEPRO_OPC_MULHH_SS, TILEPRO_OPC_MULHH_UU, TILEPRO_OPC_MULLL_SS,
  TILEPRO_OPC_MULLL_UU,
  BITFIELD(18, 2) /* index 67 */,
  TILEPRO_OPC_MULHHA_SS, TILEPRO_OPC_MULHHA_UU, TILEPRO_OPC_MULLLA_SS,
  TILEPRO_OPC_MULLLA_UU,
  BITFIELD(0, 2) /* index 72 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(77),
  BITFIELD(2, 2) /* index 77 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(82),
  BITFIELD(4, 2) /* index 82 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(87),
  BITFIELD(6, 2) /* index 87 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(92),
  BITFIELD(8, 2) /* index 92 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(97),
  BITFIELD(10, 2) /* index 97 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_INFO,
  BITFIELD(6, 2) /* index 102 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, CHILD(107),
  BITFIELD(8, 2) /* index 107 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, CHILD(112),
  BITFIELD(10, 2) /* index 112 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_MOVEI,
  BITFIELD(15, 5) /* index 117 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_RLI, TILEPRO_OPC_RLI, TILEPRO_OPC_RLI, TILEPRO_OPC_RLI,
  TILEPRO_OPC_SHLI, TILEPRO_OPC_SHLI, TILEPRO_OPC_SHLI, TILEPRO_OPC_SHLI,
  TILEPRO_OPC_SHRI, TILEPRO_OPC_SHRI, TILEPRO_OPC_SHRI, TILEPRO_OPC_SHRI,
  TILEPRO_OPC_SRAI, TILEPRO_OPC_SRAI, TILEPRO_OPC_SRAI, TILEPRO_OPC_SRAI,
  CHILD(150), CHILD(159), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(12, 3) /* index 150 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_BITX, TILEPRO_OPC_BYTEX, TILEPRO_OPC_CLZ,
  TILEPRO_OPC_CTZ, TILEPRO_OPC_FNOP, TILEPRO_OPC_NOP, TILEPRO_OPC_PCNT,
  BITFIELD(12, 3) /* index 159 */,
  TILEPRO_OPC_TBLIDXB0, TILEPRO_OPC_TBLIDXB1, TILEPRO_OPC_TBLIDXB2,
  TILEPRO_OPC_TBLIDXB3, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE,
};

static const unsigned short decode_Y1_fsm[140] =
{
  BITFIELD(59, 4) /* index 0 */,
  TILEPRO_OPC_NONE, CHILD(17), CHILD(22), CHILD(27), CHILD(47), CHILD(52),
  CHILD(57), TILEPRO_OPC_ADDI, CHILD(62), CHILD(92), TILEPRO_OPC_SEQI,
  CHILD(107), TILEPRO_OPC_SLTI, TILEPRO_OPC_SLTI_U, TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE,
  BITFIELD(49, 2) /* index 17 */,
  TILEPRO_OPC_ADD, TILEPRO_OPC_S1A, TILEPRO_OPC_S2A, TILEPRO_OPC_SUB,
  BITFIELD(49, 2) /* index 22 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_MNZ, TILEPRO_OPC_MZ, TILEPRO_OPC_NONE,
  BITFIELD(49, 2) /* index 27 */,
  TILEPRO_OPC_AND, TILEPRO_OPC_NOR, CHILD(32), TILEPRO_OPC_XOR,
  BITFIELD(43, 2) /* index 32 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, CHILD(37),
  BITFIELD(45, 2) /* index 37 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, CHILD(42),
  BITFIELD(47, 2) /* index 42 */,
  TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_OR, TILEPRO_OPC_MOVE,
  BITFIELD(49, 2) /* index 47 */,
  TILEPRO_OPC_RL, TILEPRO_OPC_SHL, TILEPRO_OPC_SHR, TILEPRO_OPC_SRA,
  BITFIELD(49, 2) /* index 52 */,
  TILEPRO_OPC_SLTE, TILEPRO_OPC_SLTE_U, TILEPRO_OPC_SLT, TILEPRO_OPC_SLT_U,
  BITFIELD(49, 2) /* index 57 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_S3A, TILEPRO_OPC_SEQ, TILEPRO_OPC_SNE,
  BITFIELD(31, 2) /* index 62 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(67),
  BITFIELD(33, 2) /* index 67 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(72),
  BITFIELD(35, 2) /* index 72 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(77),
  BITFIELD(37, 2) /* index 77 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(82),
  BITFIELD(39, 2) /* index 82 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, CHILD(87),
  BITFIELD(41, 2) /* index 87 */,
  TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_ANDI, TILEPRO_OPC_INFO,
  BITFIELD(37, 2) /* index 92 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, CHILD(97),
  BITFIELD(39, 2) /* index 97 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, CHILD(102),
  BITFIELD(41, 2) /* index 102 */,
  TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_ORI, TILEPRO_OPC_MOVEI,
  BITFIELD(48, 3) /* index 107 */,
  TILEPRO_OPC_NONE, TILEPRO_OPC_RLI, TILEPRO_OPC_SHLI, TILEPRO_OPC_SHRI,
  TILEPRO_OPC_SRAI, CHILD(116), TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(43, 3) /* index 116 */,
  TILEPRO_OPC_NONE, CHILD(125), CHILD(130), CHILD(135), TILEPRO_OPC_NONE,
  TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(46, 2) /* index 125 */,
  TILEPRO_OPC_FNOP, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(46, 2) /* index 130 */,
  TILEPRO_OPC_ILL, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
  BITFIELD(46, 2) /* index 135 */,
  TILEPRO_OPC_NOP, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE, TILEPRO_OPC_NONE,
};

static const unsigned short decode_Y2_fsm[24] =
{
  BITFIELD(56, 3) /* index 0 */,
  CHILD(9), TILEPRO_OPC_LB_U, TILEPRO_OPC_LH, TILEPRO_OPC_LH_U,
  TILEPRO_OPC_LW, TILEPRO_OPC_SB, TILEPRO_OPC_SH, TILEPRO_OPC_SW,
  BITFIELD(20, 2) /* index 9 */,
  TILEPRO_OPC_LB, TILEPRO_OPC_LB, TILEPRO_OPC_LB, CHILD(14),
  BITFIELD(22, 2) /* index 14 */,
  TILEPRO_OPC_LB, TILEPRO_OPC_LB, TILEPRO_OPC_LB, CHILD(19),
  BITFIELD(24, 2) /* index 19 */,
  TILEPRO_OPC_LB, TILEPRO_OPC_LB, TILEPRO_OPC_LB, TILEPRO_OPC_PREFETCH,
};

#undef BITFIELD
#undef CHILD

const unsigned short * const
tilepro_bundle_decoder_fsms[TILEPRO_NUM_PIPELINE_ENCODINGS] =
{
  decode_X0_fsm,
  decode_X1_fsm,
  decode_Y0_fsm,
  decode_Y1_fsm,
  decode_Y2_fsm
};

#ifndef DISASM_ONLY
const struct tilepro_sn_opcode tilepro_sn_opcodes[23] =
{
 { "bz", TILEPRO_SN_OPC_BZ,
    1 /* num_operands */,
    /* operands */
    { 38 },
    /* fixed_bit_mask */
    0xfc00,
    /* fixed_bit_value */
    0xe000
  },
  { "bnz", TILEPRO_SN_OPC_BNZ,
    1 /* num_operands */,
    /* operands */
    { 38 },
    /* fixed_bit_mask */
    0xfc00,
    /* fixed_bit_value */
    0xe400
  },
  { "jrr", TILEPRO_SN_OPC_JRR,
    1 /* num_operands */,
    /* operands */
    { 39 },
    /* fixed_bit_mask */
    0xff00,
    /* fixed_bit_value */
    0x0600
  },
  { "fnop", TILEPRO_SN_OPC_FNOP,
    0 /* num_operands */,
    /* operands */
    { 0, },
    /* fixed_bit_mask */
    0xffff,
    /* fixed_bit_value */
    0x0003
  },
  { "blz", TILEPRO_SN_OPC_BLZ,
    1 /* num_operands */,
    /* operands */
    { 38 },
    /* fixed_bit_mask */
    0xfc00,
    /* fixed_bit_value */
    0xf000
  },
  { "nop", TILEPRO_SN_OPC_NOP,
    0 /* num_operands */,
    /* operands */
    { 0, },
    /* fixed_bit_mask */
    0xffff,
    /* fixed_bit_value */
    0x0002
  },
  { "movei", TILEPRO_SN_OPC_MOVEI,
    1 /* num_operands */,
    /* operands */
    { 40 },
    /* fixed_bit_mask */
    0xff00,
    /* fixed_bit_value */
    0x0400
  },
  { "move", TILEPRO_SN_OPC_MOVE,
    2 /* num_operands */,
    /* operands */
    { 41, 42 },
    /* fixed_bit_mask */
    0xfff0,
    /* fixed_bit_value */
    0x0080
  },
  { "bgez", TILEPRO_SN_OPC_BGEZ,
    1 /* num_operands */,
    /* operands */
    { 38 },
    /* fixed_bit_mask */
    0xfc00,
    /* fixed_bit_value */
    0xf400
  },
  { "jr", TILEPRO_SN_OPC_JR,
    1 /* num_operands */,
    /* operands */
    { 42 },
    /* fixed_bit_mask */
    0xfff0,
    /* fixed_bit_value */
    0x0040
  },
  { "blez", TILEPRO_SN_OPC_BLEZ,
    1 /* num_operands */,
    /* operands */
    { 38 },
    /* fixed_bit_mask */
    0xfc00,
    /* fixed_bit_value */
    0xec00
  },
  { "bbns", TILEPRO_SN_OPC_BBNS,
    1 /* num_operands */,
    /* operands */
    { 38 },
    /* fixed_bit_mask */
    0xfc00,
    /* fixed_bit_value */
    0xfc00
  },
  { "jalrr", TILEPRO_SN_OPC_JALRR,
    1 /* num_operands */,
    /* operands */
    { 39 },
    /* fixed_bit_mask */
    0xff00,
    /* fixed_bit_value */
    0x0700
  },
  { "bpt", TILEPRO_SN_OPC_BPT,
    0 /* num_operands */,
    /* operands */
    { 0, },
    /* fixed_bit_mask */
    0xffff,
    /* fixed_bit_value */
    0x0001
  },
  { "jalr", TILEPRO_SN_OPC_JALR,
    1 /* num_operands */,
    /* operands */
    { 42 },
    /* fixed_bit_mask */
    0xfff0,
    /* fixed_bit_value */
    0x0050
  },
  { "shr1", TILEPRO_SN_OPC_SHR1,
    2 /* num_operands */,
    /* operands */
    { 41, 42 },
    /* fixed_bit_mask */
    0xfff0,
    /* fixed_bit_value */
    0x0090
  },
  { "bgz", TILEPRO_SN_OPC_BGZ,
    1 /* num_operands */,
    /* operands */
    { 38 },
    /* fixed_bit_mask */
    0xfc00,
    /* fixed_bit_value */
    0xe800
  },
  { "bbs", TILEPRO_SN_OPC_BBS,
    1 /* num_operands */,
    /* operands */
    { 38 },
    /* fixed_bit_mask */
    0xfc00,
    /* fixed_bit_value */
    0xf800
  },
  { "shl8ii", TILEPRO_SN_OPC_SHL8II,
    1 /* num_operands */,
    /* operands */
    { 39 },
    /* fixed_bit_mask */
    0xff00,
    /* fixed_bit_value */
    0x0300
  },
  { "addi", TILEPRO_SN_OPC_ADDI,
    1 /* num_operands */,
    /* operands */
    { 40 },
    /* fixed_bit_mask */
    0xff00,
    /* fixed_bit_value */
    0x0500
  },
  { "halt", TILEPRO_SN_OPC_HALT,
    0 /* num_operands */,
    /* operands */
    { 0, },
    /* fixed_bit_mask */
    0xffff,
    /* fixed_bit_value */
    0x0000
  },
  { "route", TILEPRO_SN_OPC_ROUTE, 0, { 0, }, 0, 0,
  },
  { 0, TILEPRO_SN_OPC_NONE, 0, { 0, }, 0, 0,
  }
};

const unsigned char tilepro_sn_route_encode[6 * 6 * 6] =
{
  0xdf,
  0xde,
  0xdd,
  0xdc,
  0xdb,
  0xda,
  0xb9,
  0xb8,
  0xa1,
  0xa0,
  0x11,
  0x10,
  0x9f,
  0x9e,
  0x9d,
  0x9c,
  0x9b,
  0x9a,
  0x79,
  0x78,
  0x61,
  0x60,
  0xb,
  0xa,
  0x5f,
  0x5e,
  0x5d,
  0x5c,
  0x5b,
  0x5a,
  0x1f,
  0x1e,
  0x1d,
  0x1c,
  0x1b,
  0x1a,
  0xd7,
  0xd6,
  0xd5,
  0xd4,
  0xd3,
  0xd2,
  0xa7,
  0xa6,
  0xb1,
  0xb0,
  0x13,
  0x12,
  0x97,
  0x96,
  0x95,
  0x94,
  0x93,
  0x92,
  0x67,
  0x66,
  0x71,
  0x70,
  0x9,
  0x8,
  0x57,
  0x56,
  0x55,
  0x54,
  0x53,
  0x52,
  0x17,
  0x16,
  0x15,
  0x14,
  0x19,
  0x18,
  0xcf,
  0xce,
  0xcd,
  0xcc,
  0xcb,
  0xca,
  0xaf,
  0xae,
  0xad,
  0xac,
  0xab,
  0xaa,
  0x8f,
  0x8e,
  0x8d,
  0x8c,
  0x8b,
  0x8a,
  0x6f,
  0x6e,
  0x6d,
  0x6c,
  0x6b,
  0x6a,
  0x4f,
  0x4e,
  0x4d,
  0x4c,
  0x4b,
  0x4a,
  0x2f,
  0x2e,
  0x2d,
  0x2c,
  0x2b,
  0x2a,
  0xc9,
  0xc8,
  0xc5,
  0xc4,
  0xc3,
  0xc2,
  0xa9,
  0xa8,
  0xa5,
  0xa4,
  0xa3,
  0xa2,
  0x89,
  0x88,
  0x85,
  0x84,
  0x83,
  0x82,
  0x69,
  0x68,
  0x65,
  0x64,
  0x63,
  0x62,
  0x47,
  0x46,
  0x45,
  0x44,
  0x43,
  0x42,
  0x27,
  0x26,
  0x25,
  0x24,
  0x23,
  0x22,
  0xd9,
  0xd8,
  0xc1,
  0xc0,
  0x3b,
  0x3a,
  0xbf,
  0xbe,
  0xbd,
  0xbc,
  0xbb,
  0xba,
  0x99,
  0x98,
  0x81,
  0x80,
  0x31,
  0x30,
  0x7f,
  0x7e,
  0x7d,
  0x7c,
  0x7b,
  0x7a,
  0x59,
  0x58,
  0x3d,
  0x3c,
  0x49,
  0x48,
  0xf,
  0xe,
  0xd,
  0xc,
  0x29,
  0x28,
  0xc7,
  0xc6,
  0xd1,
  0xd0,
  0x39,
  0x38,
  0xb7,
  0xb6,
  0xb5,
  0xb4,
  0xb3,
  0xb2,
  0x87,
  0x86,
  0x91,
  0x90,
  0x33,
  0x32,
  0x77,
  0x76,
  0x75,
  0x74,
  0x73,
  0x72,
  0x3f,
  0x3e,
  0x51,
  0x50,
  0x41,
  0x40,
  0x37,
  0x36,
  0x35,
  0x34,
  0x21,
  0x20
};

const signed char tilepro_sn_route_decode[256][3] =
{
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { 5, 3, 1 },
  { 4, 3, 1 },
  { 5, 3, 0 },
  { 4, 3, 0 },
  { 3, 5, 4 },
  { 2, 5, 4 },
  { 1, 5, 4 },
  { 0, 5, 4 },
  { 5, 1, 0 },
  { 4, 1, 0 },
  { 5, 1, 1 },
  { 4, 1, 1 },
  { 3, 5, 1 },
  { 2, 5, 1 },
  { 1, 5, 1 },
  { 0, 5, 1 },
  { 5, 5, 1 },
  { 4, 5, 1 },
  { 5, 5, 0 },
  { 4, 5, 0 },
  { 3, 5, 0 },
  { 2, 5, 0 },
  { 1, 5, 0 },
  { 0, 5, 0 },
  { 5, 5, 5 },
  { 4, 5, 5 },
  { 5, 5, 3 },
  { 4, 5, 3 },
  { 3, 5, 3 },
  { 2, 5, 3 },
  { 1, 5, 3 },
  { 0, 5, 3 },
  { 5, 5, 4 },
  { 4, 5, 4 },
  { 5, 5, 2 },
  { 4, 5, 2 },
  { 3, 5, 2 },
  { 2, 5, 2 },
  { 1, 5, 2 },
  { 0, 5, 2 },
  { 5, 2, 4 },
  { 4, 2, 4 },
  { 5, 2, 5 },
  { 4, 2, 5 },
  { 3, 5, 5 },
  { 2, 5, 5 },
  { 1, 5, 5 },
  { 0, 5, 5 },
  { 5, 0, 5 },
  { 4, 0, 5 },
  { 5, 0, 4 },
  { 4, 0, 4 },
  { 3, 4, 4 },
  { 2, 4, 4 },
  { 1, 4, 5 },
  { 0, 4, 5 },
  { 5, 4, 5 },
  { 4, 4, 5 },
  { 5, 4, 3 },
  { 4, 4, 3 },
  { 3, 4, 3 },
  { 2, 4, 3 },
  { 1, 4, 3 },
  { 0, 4, 3 },
  { 5, 4, 4 },
  { 4, 4, 4 },
  { 5, 4, 2 },
  { 4, 4, 2 },
  { 3, 4, 2 },
  { 2, 4, 2 },
  { 1, 4, 2 },
  { 0, 4, 2 },
  { 3, 4, 5 },
  { 2, 4, 5 },
  { 5, 4, 1 },
  { 4, 4, 1 },
  { 3, 4, 1 },
  { 2, 4, 1 },
  { 1, 4, 1 },
  { 0, 4, 1 },
  { 1, 4, 4 },
  { 0, 4, 4 },
  { 5, 4, 0 },
  { 4, 4, 0 },
  { 3, 4, 0 },
  { 2, 4, 0 },
  { 1, 4, 0 },
  { 0, 4, 0 },
  { 3, 3, 0 },
  { 2, 3, 0 },
  { 5, 3, 3 },
  { 4, 3, 3 },
  { 3, 3, 3 },
  { 2, 3, 3 },
  { 1, 3, 1 },
  { 0, 3, 1 },
  { 1, 3, 3 },
  { 0, 3, 3 },
  { 5, 3, 2 },
  { 4, 3, 2 },
  { 3, 3, 2 },
  { 2, 3, 2 },
  { 1, 3, 2 },
  { 0, 3, 2 },
  { 3, 3, 1 },
  { 2, 3, 1 },
  { 5, 3, 5 },
  { 4, 3, 5 },
  { 3, 3, 5 },
  { 2, 3, 5 },
  { 1, 3, 5 },
  { 0, 3, 5 },
  { 1, 3, 0 },
  { 0, 3, 0 },
  { 5, 3, 4 },
  { 4, 3, 4 },
  { 3, 3, 4 },
  { 2, 3, 4 },
  { 1, 3, 4 },
  { 0, 3, 4 },
  { 3, 2, 4 },
  { 2, 2, 4 },
  { 5, 2, 3 },
  { 4, 2, 3 },
  { 3, 2, 3 },
  { 2, 2, 3 },
  { 1, 2, 5 },
  { 0, 2, 5 },
  { 1, 2, 3 },
  { 0, 2, 3 },
  { 5, 2, 2 },
  { 4, 2, 2 },
  { 3, 2, 2 },
  { 2, 2, 2 },
  { 1, 2, 2 },
  { 0, 2, 2 },
  { 3, 2, 5 },
  { 2, 2, 5 },
  { 5, 2, 1 },
  { 4, 2, 1 },
  { 3, 2, 1 },
  { 2, 2, 1 },
  { 1, 2, 1 },
  { 0, 2, 1 },
  { 1, 2, 4 },
  { 0, 2, 4 },
  { 5, 2, 0 },
  { 4, 2, 0 },
  { 3, 2, 0 },
  { 2, 2, 0 },
  { 1, 2, 0 },
  { 0, 2, 0 },
  { 3, 1, 0 },
  { 2, 1, 0 },
  { 5, 1, 3 },
  { 4, 1, 3 },
  { 3, 1, 3 },
  { 2, 1, 3 },
  { 1, 1, 1 },
  { 0, 1, 1 },
  { 1, 1, 3 },
  { 0, 1, 3 },
  { 5, 1, 2 },
  { 4, 1, 2 },
  { 3, 1, 2 },
  { 2, 1, 2 },
  { 1, 1, 2 },
  { 0, 1, 2 },
  { 3, 1, 1 },
  { 2, 1, 1 },
  { 5, 1, 5 },
  { 4, 1, 5 },
  { 3, 1, 5 },
  { 2, 1, 5 },
  { 1, 1, 5 },
  { 0, 1, 5 },
  { 1, 1, 0 },
  { 0, 1, 0 },
  { 5, 1, 4 },
  { 4, 1, 4 },
  { 3, 1, 4 },
  { 2, 1, 4 },
  { 1, 1, 4 },
  { 0, 1, 4 },
  { 3, 0, 4 },
  { 2, 0, 4 },
  { 5, 0, 3 },
  { 4, 0, 3 },
  { 3, 0, 3 },
  { 2, 0, 3 },
  { 1, 0, 5 },
  { 0, 0, 5 },
  { 1, 0, 3 },
  { 0, 0, 3 },
  { 5, 0, 2 },
  { 4, 0, 2 },
  { 3, 0, 2 },
  { 2, 0, 2 },
  { 1, 0, 2 },
  { 0, 0, 2 },
  { 3, 0, 5 },
  { 2, 0, 5 },
  { 5, 0, 1 },
  { 4, 0, 1 },
  { 3, 0, 1 },
  { 2, 0, 1 },
  { 1, 0, 1 },
  { 0, 0, 1 },
  { 1, 0, 4 },
  { 0, 0, 4 },
  { 5, 0, 0 },
  { 4, 0, 0 },
  { 3, 0, 0 },
  { 2, 0, 0 },
  { 1, 0, 0 },
  { 0, 0, 0 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 },
  { -1, -1, -1 }
};

const char tilepro_sn_direction_names[6][5] =
{
  "w",
  "c",
  "acc",
  "n",
  "e",
  "s"
};

const signed char tilepro_sn_dest_map[6][6] =
{
  { -1, 3, 4, 5, 1, 2 } /* val -> w */,
  { -1, 3, 4, 5, 0, 2 } /* val -> c */,
  { -1, 3, 4, 5, 0, 1 } /* val -> acc */,
  { -1, 4, 5, 0, 1, 2 } /* val -> n */,
  { -1, 3, 5, 0, 1, 2 } /* val -> e */,
  { -1, 3, 4, 0, 1, 2 } /* val -> s */
};
#endif /* DISASM_ONLY */

const struct tilepro_operand tilepro_operands[43] =
{
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_IMM8_X0),
    8, 1, 0, 0, 0, 0,
    create_Imm8_X0, get_Imm8_X0
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_IMM8_X1),
    8, 1, 0, 0, 0, 0,
    create_Imm8_X1, get_Imm8_X1
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_IMM8_Y0),
    8, 1, 0, 0, 0, 0,
    create_Imm8_Y0, get_Imm8_Y0
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_IMM8_Y1),
    8, 1, 0, 0, 0, 0,
    create_Imm8_Y1, get_Imm8_Y1
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_IMM16_X0),
    16, 1, 0, 0, 0, 0,
    create_Imm16_X0, get_Imm16_X0
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_IMM16_X1),
    16, 1, 0, 0, 0, 0,
    create_Imm16_X1, get_Imm16_X1
  },
  {
    TILEPRO_OP_TYPE_ADDRESS, BFD_RELOC(TILEPRO_JOFFLONG_X1),
    29, 1, 0, 0, 1, TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES,
    create_JOffLong_X1, get_JOffLong_X1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 0, 1, 0, 0,
    create_Dest_X1, get_Dest_X1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcA_X1, get_SrcA_X1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 0, 1, 0, 0,
    create_Dest_X0, get_Dest_X0
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcA_X0, get_SrcA_X0
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 0, 1, 0, 0,
    create_Dest_Y0, get_Dest_Y0
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcA_Y0, get_SrcA_Y0
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 0, 1, 0, 0,
    create_Dest_Y1, get_Dest_Y1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcA_Y1, get_SrcA_Y1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcA_Y2, get_SrcA_Y2
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcB_X0, get_SrcB_X0
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcB_X1, get_SrcB_X1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcB_Y0, get_SrcB_Y0
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcB_Y1, get_SrcB_Y1
  },
  {
    TILEPRO_OP_TYPE_ADDRESS, BFD_RELOC(TILEPRO_BROFF_X1),
    17, 1, 0, 0, 1, TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES,
    create_BrOff_X1, get_BrOff_X1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 1, 0, 0,
    create_Dest_X0, get_Dest_X0
  },
  {
    TILEPRO_OP_TYPE_ADDRESS, BFD_RELOC(NONE),
    28, 1, 0, 0, 1, TILEPRO_LOG2_BUNDLE_ALIGNMENT_IN_BYTES,
    create_JOff_X1, get_JOff_X1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 0, 1, 0, 0,
    create_SrcBDest_Y2, get_SrcBDest_Y2
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 1, 0, 0,
    create_SrcA_X1, get_SrcA_X1
  },
  {
    TILEPRO_OP_TYPE_SPR, BFD_RELOC(TILEPRO_MF_IMM15_X1),
    15, 0, 0, 0, 0, 0,
    create_MF_Imm15_X1, get_MF_Imm15_X1
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_MMSTART_X0),
    5, 0, 0, 0, 0, 0,
    create_MMStart_X0, get_MMStart_X0
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_MMEND_X0),
    5, 0, 0, 0, 0, 0,
    create_MMEnd_X0, get_MMEnd_X0
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_MMSTART_X1),
    5, 0, 0, 0, 0, 0,
    create_MMStart_X1, get_MMStart_X1
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_MMEND_X1),
    5, 0, 0, 0, 0, 0,
    create_MMEnd_X1, get_MMEnd_X1
  },
  {
    TILEPRO_OP_TYPE_SPR, BFD_RELOC(TILEPRO_MT_IMM15_X1),
    15, 0, 0, 0, 0, 0,
    create_MT_Imm15_X1, get_MT_Imm15_X1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 1, 0, 0,
    create_Dest_Y0, get_Dest_Y0
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_SHAMT_X0),
    5, 0, 0, 0, 0, 0,
    create_ShAmt_X0, get_ShAmt_X0
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_SHAMT_X1),
    5, 0, 0, 0, 0, 0,
    create_ShAmt_X1, get_ShAmt_X1
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_SHAMT_Y0),
    5, 0, 0, 0, 0, 0,
    create_ShAmt_Y0, get_ShAmt_Y0
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_SHAMT_Y1),
    5, 0, 0, 0, 0, 0,
    create_ShAmt_Y1, get_ShAmt_Y1
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    6, 0, 1, 0, 0, 0,
    create_SrcBDest_Y2, get_SrcBDest_Y2
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(TILEPRO_DEST_IMM8_X1),
    8, 1, 0, 0, 0, 0,
    create_Dest_Imm8_X1, get_Dest_Imm8_X1
  },
  {
    TILEPRO_OP_TYPE_ADDRESS, BFD_RELOC(NONE),
    10, 1, 0, 0, 1, TILEPRO_LOG2_SN_INSTRUCTION_SIZE_IN_BYTES,
    create_BrOff_SN, get_BrOff_SN
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(NONE),
    8, 0, 0, 0, 0, 0,
    create_Imm8_SN, get_Imm8_SN
  },
  {
    TILEPRO_OP_TYPE_IMMEDIATE, BFD_RELOC(NONE),
    8, 1, 0, 0, 0, 0,
    create_Imm8_SN, get_Imm8_SN
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    2, 0, 0, 1, 0, 0,
    create_Dest_SN, get_Dest_SN
  },
  {
    TILEPRO_OP_TYPE_REGISTER, BFD_RELOC(NONE),
    2, 0, 1, 0, 0, 0,
    create_Src_SN, get_Src_SN
  }
};

#ifndef DISASM_ONLY
const struct tilepro_spr tilepro_sprs[] =
{
  { 0, "MPL_ITLB_MISS_SET_0" },
  { 1, "MPL_ITLB_MISS_SET_1" },
  { 2, "MPL_ITLB_MISS_SET_2" },
  { 3, "MPL_ITLB_MISS_SET_3" },
  { 4, "MPL_ITLB_MISS" },
  { 256, "ITLB_CURRENT_0" },
  { 257, "ITLB_CURRENT_1" },
  { 258, "ITLB_CURRENT_2" },
  { 259, "ITLB_CURRENT_3" },
  { 260, "ITLB_INDEX" },
  { 261, "ITLB_MATCH_0" },
  { 262, "ITLB_PR" },
  { 263, "NUMBER_ITLB" },
  { 264, "REPLACEMENT_ITLB" },
  { 265, "WIRED_ITLB" },
  { 266, "ITLB_PERF" },
  { 512, "MPL_MEM_ERROR_SET_0" },
  { 513, "MPL_MEM_ERROR_SET_1" },
  { 514, "MPL_MEM_ERROR_SET_2" },
  { 515, "MPL_MEM_ERROR_SET_3" },
  { 516, "MPL_MEM_ERROR" },
  { 517, "L1_I_ERROR" },
  { 518, "MEM_ERROR_CBOX_ADDR" },
  { 519, "MEM_ERROR_CBOX_STATUS" },
  { 520, "MEM_ERROR_ENABLE" },
  { 521, "MEM_ERROR_MBOX_ADDR" },
  { 522, "MEM_ERROR_MBOX_STATUS" },
  { 523, "SNIC_ERROR_LOG_STATUS" },
  { 524, "SNIC_ERROR_LOG_VA" },
  { 525, "XDN_DEMUX_ERROR" },
  { 1024, "MPL_ILL_SET_0" },
  { 1025, "MPL_ILL_SET_1" },
  { 1026, "MPL_ILL_SET_2" },
  { 1027, "MPL_ILL_SET_3" },
  { 1028, "MPL_ILL" },
  { 1536, "MPL_GPV_SET_0" },
  { 1537, "MPL_GPV_SET_1" },
  { 1538, "MPL_GPV_SET_2" },
  { 1539, "MPL_GPV_SET_3" },
  { 1540, "MPL_GPV" },
  { 1541, "GPV_REASON" },
  { 2048, "MPL_SN_ACCESS_SET_0" },
  { 2049, "MPL_SN_ACCESS_SET_1" },
  { 2050, "MPL_SN_ACCESS_SET_2" },
  { 2051, "MPL_SN_ACCESS_SET_3" },
  { 2052, "MPL_SN_ACCESS" },
  { 2053, "SNCTL" },
  { 2054, "SNFIFO_DATA" },
  { 2055, "SNFIFO_SEL" },
  { 2056, "SNIC_INVADDR" },
  { 2057, "SNISTATE" },
  { 2058, "SNOSTATE" },
  { 2059, "SNPC" },
  { 2060, "SNSTATIC" },
  { 2304, "SN_DATA_AVAIL" },
  { 2560, "MPL_IDN_ACCESS_SET_0" },
  { 2561, "MPL_IDN_ACCESS_SET_1" },
  { 2562, "MPL_IDN_ACCESS_SET_2" },
  { 2563, "MPL_IDN_ACCESS_SET_3" },
  { 2564, "MPL_IDN_ACCESS" },
  { 2565, "IDN_DEMUX_CA_COUNT" },
  { 2566, "IDN_DEMUX_COUNT_0" },
  { 2567, "IDN_DEMUX_COUNT_1" },
  { 2568, "IDN_DEMUX_CTL" },
  { 2569, "IDN_DEMUX_CURR_TAG" },
  { 2570, "IDN_DEMUX_QUEUE_SEL" },
  { 2571, "IDN_DEMUX_STATUS" },
  { 2572, "IDN_DEMUX_WRITE_FIFO" },
  { 2573, "IDN_DEMUX_WRITE_QUEUE" },
  { 2574, "IDN_PENDING" },
  { 2575, "IDN_SP_FIFO_DATA" },
  { 2576, "IDN_SP_FIFO_SEL" },
  { 2577, "IDN_SP_FREEZE" },
  { 2578, "IDN_SP_STATE" },
  { 2579, "IDN_TAG_0" },
  { 2580, "IDN_TAG_1" },
  { 2581, "IDN_TAG_VALID" },
  { 2582, "IDN_TILE_COORD" },
  { 2816, "IDN_CA_DATA" },
  { 2817, "IDN_CA_REM" },
  { 2818, "IDN_CA_TAG" },
  { 2819, "IDN_DATA_AVAIL" },
  { 3072, "MPL_UDN_ACCESS_SET_0" },
  { 3073, "MPL_UDN_ACCESS_SET_1" },
  { 3074, "MPL_UDN_ACCESS_SET_2" },
  { 3075, "MPL_UDN_ACCESS_SET_3" },
  { 3076, "MPL_UDN_ACCESS" },
  { 3077, "UDN_DEMUX_CA_COUNT" },
  { 3078, "UDN_DEMUX_COUNT_0" },
  { 3079, "UDN_DEMUX_COUNT_1" },
  { 3080, "UDN_DEMUX_COUNT_2" },
  { 3081, "UDN_DEMUX_COUNT_3" },
  { 3082, "UDN_DEMUX_CTL" },
  { 3083, "UDN_DEMUX_CURR_TAG" },
  { 3084, "UDN_DEMUX_QUEUE_SEL" },
  { 3085, "UDN_DEMUX_STATUS" },
  { 3086, "UDN_DEMUX_WRITE_FIFO" },
  { 3087, "UDN_DEMUX_WRITE_QUEUE" },
  { 3088, "UDN_PENDING" },
  { 3089, "UDN_SP_FIFO_DATA" },
  { 3090, "UDN_SP_FIFO_SEL" },
  { 3091, "UDN_SP_FREEZE" },
  { 3092, "UDN_SP_STATE" },
  { 3093, "UDN_TAG_0" },
  { 3094, "UDN_TAG_1" },
  { 3095, "UDN_TAG_2" },
  { 3096, "UDN_TAG_3" },
  { 3097, "UDN_TAG_VALID" },
  { 3098, "UDN_TILE_COORD" },
  { 3328, "UDN_CA_DATA" },
  { 3329, "UDN_CA_REM" },
  { 3330, "UDN_CA_TAG" },
  { 3331, "UDN_DATA_AVAIL" },
  { 3584, "MPL_IDN_REFILL_SET_0" },
  { 3585, "MPL_IDN_REFILL_SET_1" },
  { 3586, "MPL_IDN_REFILL_SET_2" },
  { 3587, "MPL_IDN_REFILL_SET_3" },
  { 3588, "MPL_IDN_REFILL" },
  { 3589, "IDN_REFILL_EN" },
  { 4096, "MPL_UDN_REFILL_SET_0" },
  { 4097, "MPL_UDN_REFILL_SET_1" },
  { 4098, "MPL_UDN_REFILL_SET_2" },
  { 4099, "MPL_UDN_REFILL_SET_3" },
  { 4100, "MPL_UDN_REFILL" },
  { 4101, "UDN_REFILL_EN" },
  { 4608, "MPL_IDN_COMPLETE_SET_0" },
  { 4609, "MPL_IDN_COMPLETE_SET_1" },
  { 4610, "MPL_IDN_COMPLETE_SET_2" },
  { 4611, "MPL_IDN_COMPLETE_SET_3" },
  { 4612, "MPL_IDN_COMPLETE" },
  { 4613, "IDN_REMAINING" },
  { 5120, "MPL_UDN_COMPLETE_SET_0" },
  { 5121, "MPL_UDN_COMPLETE_SET_1" },
  { 5122, "MPL_UDN_COMPLETE_SET_2" },
  { 5123, "MPL_UDN_COMPLETE_SET_3" },
  { 5124, "MPL_UDN_COMPLETE" },
  { 5125, "UDN_REMAINING" },
  { 5632, "MPL_SWINT_3_SET_0" },
  { 5633, "MPL_SWINT_3_SET_1" },
  { 5634, "MPL_SWINT_3_SET_2" },
  { 5635, "MPL_SWINT_3_SET_3" },
  { 5636, "MPL_SWINT_3" },
  { 6144, "MPL_SWINT_2_SET_0" },
  { 6145, "MPL_SWINT_2_SET_1" },
  { 6146, "MPL_SWINT_2_SET_2" },
  { 6147, "MPL_SWINT_2_SET_3" },
  { 6148, "MPL_SWINT_2" },
  { 6656, "MPL_SWINT_1_SET_0" },
  { 6657, "MPL_SWINT_1_SET_1" },
  { 6658, "MPL_SWINT_1_SET_2" },
  { 6659, "MPL_SWINT_1_SET_3" },
  { 6660, "MPL_SWINT_1" },
  { 7168, "MPL_SWINT_0_SET_0" },
  { 7169, "MPL_SWINT_0_SET_1" },
  { 7170, "MPL_SWINT_0_SET_2" },
  { 7171, "MPL_SWINT_0_SET_3" },
  { 7172, "MPL_SWINT_0" },
  { 7680, "MPL_UNALIGN_DATA_SET_0" },
  { 7681, "MPL_UNALIGN_DATA_SET_1" },
  { 7682, "MPL_UNALIGN_DATA_SET_2" },
  { 7683, "MPL_UNALIGN_DATA_SET_3" },
  { 7684, "MPL_UNALIGN_DATA" },
  { 8192, "MPL_DTLB_MISS_SET_0" },
  { 8193, "MPL_DTLB_MISS_SET_1" },
  { 8194, "MPL_DTLB_MISS_SET_2" },
  { 8195, "MPL_DTLB_MISS_SET_3" },
  { 8196, "MPL_DTLB_MISS" },
  { 8448, "AER_0" },
  { 8449, "AER_1" },
  { 8450, "DTLB_BAD_ADDR" },
  { 8451, "DTLB_BAD_ADDR_REASON" },
  { 8452, "DTLB_CURRENT_0" },
  { 8453, "DTLB_CURRENT_1" },
  { 8454, "DTLB_CURRENT_2" },
  { 8455, "DTLB_CURRENT_3" },
  { 8456, "DTLB_INDEX" },
  { 8457, "DTLB_MATCH_0" },
  { 8458, "NUMBER_DTLB" },
  { 8459, "PHYSICAL_MEMORY_MODE" },
  { 8460, "REPLACEMENT_DTLB" },
  { 8461, "WIRED_DTLB" },
  { 8462, "CACHE_RED_WAY_OVERRIDDEN" },
  { 8463, "DTLB_PERF" },
  { 8704, "MPL_DTLB_ACCESS_SET_0" },
  { 8705, "MPL_DTLB_ACCESS_SET_1" },
  { 8706, "MPL_DTLB_ACCESS_SET_2" },
  { 8707, "MPL_DTLB_ACCESS_SET_3" },
  { 8708, "MPL_DTLB_ACCESS" },
  { 9216, "MPL_DMATLB_MISS_SET_0" },
  { 9217, "MPL_DMATLB_MISS_SET_1" },
  { 9218, "MPL_DMATLB_MISS_SET_2" },
  { 9219, "MPL_DMATLB_MISS_SET_3" },
  { 9220, "MPL_DMATLB_MISS" },
  { 9472, "DMA_BAD_ADDR" },
  { 9473, "DMA_STATUS" },
  { 9728, "MPL_DMATLB_ACCESS_SET_0" },
  { 9729, "MPL_DMATLB_ACCESS_SET_1" },
  { 9730, "MPL_DMATLB_ACCESS_SET_2" },
  { 9731, "MPL_DMATLB_ACCESS_SET_3" },
  { 9732, "MPL_DMATLB_ACCESS" },
  { 10240, "MPL_SNITLB_MISS_SET_0" },
  { 10241, "MPL_SNITLB_MISS_SET_1" },
  { 10242, "MPL_SNITLB_MISS_SET_2" },
  { 10243, "MPL_SNITLB_MISS_SET_3" },
  { 10244, "MPL_SNITLB_MISS" },
  { 10245, "NUMBER_SNITLB" },
  { 10246, "REPLACEMENT_SNITLB" },
  { 10247, "SNITLB_CURRENT_0" },
  { 10248, "SNITLB_CURRENT_1" },
  { 10249, "SNITLB_CURRENT_2" },
  { 10250, "SNITLB_CURRENT_3" },
  { 10251, "SNITLB_INDEX" },
  { 10252, "SNITLB_MATCH_0" },
  { 10253, "SNITLB_PR" },
  { 10254, "WIRED_SNITLB" },
  { 10255, "SNITLB_STATUS" },
  { 10752, "MPL_SN_NOTIFY_SET_0" },
  { 10753, "MPL_SN_NOTIFY_SET_1" },
  { 10754, "MPL_SN_NOTIFY_SET_2" },
  { 10755, "MPL_SN_NOTIFY_SET_3" },
  { 10756, "MPL_SN_NOTIFY" },
  { 10757, "SN_NOTIFY_STATUS" },
  { 11264, "MPL_SN_FIREWALL_SET_0" },
  { 11265, "MPL_SN_FIREWALL_SET_1" },
  { 11266, "MPL_SN_FIREWALL_SET_2" },
  { 11267, "MPL_SN_FIREWALL_SET_3" },
  { 11268, "MPL_SN_FIREWALL" },
  { 11269, "SN_DIRECTION_PROTECT" },
  { 11776, "MPL_IDN_FIREWALL_SET_0" },
  { 11777, "MPL_IDN_FIREWALL_SET_1" },
  { 11778, "MPL_IDN_FIREWALL_SET_2" },
  { 11779, "MPL_IDN_FIREWALL_SET_3" },
  { 11780, "MPL_IDN_FIREWALL" },
  { 11781, "IDN_DIRECTION_PROTECT" },
  { 12288, "MPL_UDN_FIREWALL_SET_0" },
  { 12289, "MPL_UDN_FIREWALL_SET_1" },
  { 12290, "MPL_UDN_FIREWALL_SET_2" },
  { 12291, "MPL_UDN_FIREWALL_SET_3" },
  { 12292, "MPL_UDN_FIREWALL" },
  { 12293, "UDN_DIRECTION_PROTECT" },
  { 12800, "MPL_TILE_TIMER_SET_0" },
  { 12801, "MPL_TILE_TIMER_SET_1" },
  { 12802, "MPL_TILE_TIMER_SET_2" },
  { 12803, "MPL_TILE_TIMER_SET_3" },
  { 12804, "MPL_TILE_TIMER" },
  { 12805, "TILE_TIMER_CONTROL" },
  { 13312, "MPL_IDN_TIMER_SET_0" },
  { 13313, "MPL_IDN_TIMER_SET_1" },
  { 13314, "MPL_IDN_TIMER_SET_2" },
  { 13315, "MPL_IDN_TIMER_SET_3" },
  { 13316, "MPL_IDN_TIMER" },
  { 13317, "IDN_DEADLOCK_COUNT" },
  { 13318, "IDN_DEADLOCK_TIMEOUT" },
  { 13824, "MPL_UDN_TIMER_SET_0" },
  { 13825, "MPL_UDN_TIMER_SET_1" },
  { 13826, "MPL_UDN_TIMER_SET_2" },
  { 13827, "MPL_UDN_TIMER_SET_3" },
  { 13828, "MPL_UDN_TIMER" },
  { 13829, "UDN_DEADLOCK_COUNT" },
  { 13830, "UDN_DEADLOCK_TIMEOUT" },
  { 14336, "MPL_DMA_NOTIFY_SET_0" },
  { 14337, "MPL_DMA_NOTIFY_SET_1" },
  { 14338, "MPL_DMA_NOTIFY_SET_2" },
  { 14339, "MPL_DMA_NOTIFY_SET_3" },
  { 14340, "MPL_DMA_NOTIFY" },
  { 14592, "DMA_BYTE" },
  { 14593, "DMA_CHUNK_SIZE" },
  { 14594, "DMA_CTR" },
  { 14595, "DMA_DST_ADDR" },
  { 14596, "DMA_DST_CHUNK_ADDR" },
  { 14597, "DMA_SRC_ADDR" },
  { 14598, "DMA_SRC_CHUNK_ADDR" },
  { 14599, "DMA_STRIDE" },
  { 14600, "DMA_USER_STATUS" },
  { 14848, "MPL_IDN_CA_SET_0" },
  { 14849, "MPL_IDN_CA_SET_1" },
  { 14850, "MPL_IDN_CA_SET_2" },
  { 14851, "MPL_IDN_CA_SET_3" },
  { 14852, "MPL_IDN_CA" },
  { 15360, "MPL_UDN_CA_SET_0" },
  { 15361, "MPL_UDN_CA_SET_1" },
  { 15362, "MPL_UDN_CA_SET_2" },
  { 15363, "MPL_UDN_CA_SET_3" },
  { 15364, "MPL_UDN_CA" },
  { 15872, "MPL_IDN_AVAIL_SET_0" },
  { 15873, "MPL_IDN_AVAIL_SET_1" },
  { 15874, "MPL_IDN_AVAIL_SET_2" },
  { 15875, "MPL_IDN_AVAIL_SET_3" },
  { 15876, "MPL_IDN_AVAIL" },
  { 15877, "IDN_AVAIL_EN" },
  { 16384, "MPL_UDN_AVAIL_SET_0" },
  { 16385, "MPL_UDN_AVAIL_SET_1" },
  { 16386, "MPL_UDN_AVAIL_SET_2" },
  { 16387, "MPL_UDN_AVAIL_SET_3" },
  { 16388, "MPL_UDN_AVAIL" },
  { 16389, "UDN_AVAIL_EN" },
  { 16896, "MPL_PERF_COUNT_SET_0" },
  { 16897, "MPL_PERF_COUNT_SET_1" },
  { 16898, "MPL_PERF_COUNT_SET_2" },
  { 16899, "MPL_PERF_COUNT_SET_3" },
  { 16900, "MPL_PERF_COUNT" },
  { 16901, "PERF_COUNT_0" },
  { 16902, "PERF_COUNT_1" },
  { 16903, "PERF_COUNT_CTL" },
  { 16904, "PERF_COUNT_STS" },
  { 16905, "WATCH_CTL" },
  { 16906, "WATCH_MASK" },
  { 16907, "WATCH_VAL" },
  { 16912, "PERF_COUNT_DN_CTL" },
  { 17408, "MPL_INTCTRL_3_SET_0" },
  { 17409, "MPL_INTCTRL_3_SET_1" },
  { 17410, "MPL_INTCTRL_3_SET_2" },
  { 17411, "MPL_INTCTRL_3_SET_3" },
  { 17412, "MPL_INTCTRL_3" },
  { 17413, "EX_CONTEXT_3_0" },
  { 17414, "EX_CONTEXT_3_1" },
  { 17415, "INTERRUPT_MASK_3_0" },
  { 17416, "INTERRUPT_MASK_3_1" },
  { 17417, "INTERRUPT_MASK_RESET_3_0" },
  { 17418, "INTERRUPT_MASK_RESET_3_1" },
  { 17419, "INTERRUPT_MASK_SET_3_0" },
  { 17420, "INTERRUPT_MASK_SET_3_1" },
  { 17432, "INTCTRL_3_STATUS" },
  { 17664, "SYSTEM_SAVE_3_0" },
  { 17665, "SYSTEM_SAVE_3_1" },
  { 17666, "SYSTEM_SAVE_3_2" },
  { 17667, "SYSTEM_SAVE_3_3" },
  { 17920, "MPL_INTCTRL_2_SET_0" },
  { 17921, "MPL_INTCTRL_2_SET_1" },
  { 17922, "MPL_INTCTRL_2_SET_2" },
  { 17923, "MPL_INTCTRL_2_SET_3" },
  { 17924, "MPL_INTCTRL_2" },
  { 17925, "EX_CONTEXT_2_0" },
  { 17926, "EX_CONTEXT_2_1" },
  { 17927, "INTCTRL_2_STATUS" },
  { 17928, "INTERRUPT_MASK_2_0" },
  { 17929, "INTERRUPT_MASK_2_1" },
  { 17930, "INTERRUPT_MASK_RESET_2_0" },
  { 17931, "INTERRUPT_MASK_RESET_2_1" },
  { 17932, "INTERRUPT_MASK_SET_2_0" },
  { 17933, "INTERRUPT_MASK_SET_2_1" },
  { 18176, "SYSTEM_SAVE_2_0" },
  { 18177, "SYSTEM_SAVE_2_1" },
  { 18178, "SYSTEM_SAVE_2_2" },
  { 18179, "SYSTEM_SAVE_2_3" },
  { 18432, "MPL_INTCTRL_1_SET_0" },
  { 18433, "MPL_INTCTRL_1_SET_1" },
  { 18434, "MPL_INTCTRL_1_SET_2" },
  { 18435, "MPL_INTCTRL_1_SET_3" },
  { 18436, "MPL_INTCTRL_1" },
  { 18437, "EX_CONTEXT_1_0" },
  { 18438, "EX_CONTEXT_1_1" },
  { 18439, "INTCTRL_1_STATUS" },
  { 18440, "INTCTRL_3_STATUS_REV0" },
  { 18441, "INTERRUPT_MASK_1_0" },
  { 18442, "INTERRUPT_MASK_1_1" },
  { 18443, "INTERRUPT_MASK_RESET_1_0" },
  { 18444, "INTERRUPT_MASK_RESET_1_1" },
  { 18445, "INTERRUPT_MASK_SET_1_0" },
  { 18446, "INTERRUPT_MASK_SET_1_1" },
  { 18688, "SYSTEM_SAVE_1_0" },
  { 18689, "SYSTEM_SAVE_1_1" },
  { 18690, "SYSTEM_SAVE_1_2" },
  { 18691, "SYSTEM_SAVE_1_3" },
  { 18944, "MPL_INTCTRL_0_SET_0" },
  { 18945, "MPL_INTCTRL_0_SET_1" },
  { 18946, "MPL_INTCTRL_0_SET_2" },
  { 18947, "MPL_INTCTRL_0_SET_3" },
  { 18948, "MPL_INTCTRL_0" },
  { 18949, "EX_CONTEXT_0_0" },
  { 18950, "EX_CONTEXT_0_1" },
  { 18951, "INTCTRL_0_STATUS" },
  { 18952, "INTERRUPT_MASK_0_0" },
  { 18953, "INTERRUPT_MASK_0_1" },
  { 18954, "INTERRUPT_MASK_RESET_0_0" },
  { 18955, "INTERRUPT_MASK_RESET_0_1" },
  { 18956, "INTERRUPT_MASK_SET_0_0" },
  { 18957, "INTERRUPT_MASK_SET_0_1" },
  { 19200, "SYSTEM_SAVE_0_0" },
  { 19201, "SYSTEM_SAVE_0_1" },
  { 19202, "SYSTEM_SAVE_0_2" },
  { 19203, "SYSTEM_SAVE_0_3" },
  { 19456, "MPL_BOOT_ACCESS_SET_0" },
  { 19457, "MPL_BOOT_ACCESS_SET_1" },
  { 19458, "MPL_BOOT_ACCESS_SET_2" },
  { 19459, "MPL_BOOT_ACCESS_SET_3" },
  { 19460, "MPL_BOOT_ACCESS" },
  { 19461, "CBOX_CACHEASRAM_CONFIG" },
  { 19462, "CBOX_CACHE_CONFIG" },
  { 19463, "CBOX_MMAP_0" },
  { 19464, "CBOX_MMAP_1" },
  { 19465, "CBOX_MMAP_2" },
  { 19466, "CBOX_MMAP_3" },
  { 19467, "CBOX_MSR" },
  { 19468, "CBOX_SRC_ID" },
  { 19469, "CYCLE_HIGH_MODIFY" },
  { 19470, "CYCLE_LOW_MODIFY" },
  { 19471, "DIAG_BCST_CTL" },
  { 19472, "DIAG_BCST_MASK" },
  { 19473, "DIAG_BCST_TRIGGER" },
  { 19474, "DIAG_MUX_CTL" },
  { 19475, "DIAG_TRACE_CTL" },
  { 19476, "DIAG_TRACE_STS" },
  { 19477, "IDN_DEMUX_BUF_THRESH" },
  { 19478, "SBOX_CONFIG" },
  { 19479, "TILE_COORD" },
  { 19480, "UDN_DEMUX_BUF_THRESH" },
  { 19481, "CBOX_HOME_MAP_ADDR" },
  { 19482, "CBOX_HOME_MAP_DATA" },
  { 19483, "CBOX_MSR1" },
  { 19484, "BIG_ENDIAN_CONFIG" },
  { 19485, "MEM_STRIPE_CONFIG" },
  { 19486, "DIAG_TRACE_WAY" },
  { 19487, "VDN_SNOOP_SHIM_CTL" },
  { 19488, "PERF_COUNT_PLS" },
  { 19489, "DIAG_TRACE_DATA" },
  { 19712, "I_AER_0" },
  { 19713, "I_AER_1" },
  { 19714, "I_PHYSICAL_MEMORY_MODE" },
  { 19968, "MPL_WORLD_ACCESS_SET_0" },
  { 19969, "MPL_WORLD_ACCESS_SET_1" },
  { 19970, "MPL_WORLD_ACCESS_SET_2" },
  { 19971, "MPL_WORLD_ACCESS_SET_3" },
  { 19972, "MPL_WORLD_ACCESS" },
  { 19973, "SIM_SOCKET" },
  { 19974, "CYCLE_HIGH" },
  { 19975, "CYCLE_LOW" },
  { 19976, "DONE" },
  { 19977, "FAIL" },
  { 19978, "INTERRUPT_CRITICAL_SECTION" },
  { 19979, "PASS" },
  { 19980, "SIM_CONTROL" },
  { 19981, "EVENT_BEGIN" },
  { 19982, "EVENT_END" },
  { 19983, "TILE_WRITE_PENDING" },
  { 19984, "TILE_RTF_HWM" },
  { 20224, "PROC_STATUS" },
  { 20225, "STATUS_SATURATE" },
  { 20480, "MPL_I_ASID_SET_0" },
  { 20481, "MPL_I_ASID_SET_1" },
  { 20482, "MPL_I_ASID_SET_2" },
  { 20483, "MPL_I_ASID_SET_3" },
  { 20484, "MPL_I_ASID" },
  { 20485, "I_ASID" },
  { 20992, "MPL_D_ASID_SET_0" },
  { 20993, "MPL_D_ASID_SET_1" },
  { 20994, "MPL_D_ASID_SET_2" },
  { 20995, "MPL_D_ASID_SET_3" },
  { 20996, "MPL_D_ASID" },
  { 20997, "D_ASID" },
  { 21504, "MPL_DMA_ASID_SET_0" },
  { 21505, "MPL_DMA_ASID_SET_1" },
  { 21506, "MPL_DMA_ASID_SET_2" },
  { 21507, "MPL_DMA_ASID_SET_3" },
  { 21508, "MPL_DMA_ASID" },
  { 21509, "DMA_ASID" },
  { 22016, "MPL_SNI_ASID_SET_0" },
  { 22017, "MPL_SNI_ASID_SET_1" },
  { 22018, "MPL_SNI_ASID_SET_2" },
  { 22019, "MPL_SNI_ASID_SET_3" },
  { 22020, "MPL_SNI_ASID" },
  { 22021, "SNI_ASID" },
  { 22528, "MPL_DMA_CPL_SET_0" },
  { 22529, "MPL_DMA_CPL_SET_1" },
  { 22530, "MPL_DMA_CPL_SET_2" },
  { 22531, "MPL_DMA_CPL_SET_3" },
  { 22532, "MPL_DMA_CPL" },
  { 23040, "MPL_SN_CPL_SET_0" },
  { 23041, "MPL_SN_CPL_SET_1" },
  { 23042, "MPL_SN_CPL_SET_2" },
  { 23043, "MPL_SN_CPL_SET_3" },
  { 23044, "MPL_SN_CPL" },
  { 23552, "MPL_DOUBLE_FAULT_SET_0" },
  { 23553, "MPL_DOUBLE_FAULT_SET_1" },
  { 23554, "MPL_DOUBLE_FAULT_SET_2" },
  { 23555, "MPL_DOUBLE_FAULT_SET_3" },
  { 23556, "MPL_DOUBLE_FAULT" },
  { 23557, "LAST_INTERRUPT_REASON" },
  { 24064, "MPL_SN_STATIC_ACCESS_SET_0" },
  { 24065, "MPL_SN_STATIC_ACCESS_SET_1" },
  { 24066, "MPL_SN_STATIC_ACCESS_SET_2" },
  { 24067, "MPL_SN_STATIC_ACCESS_SET_3" },
  { 24068, "MPL_SN_STATIC_ACCESS" },
  { 24069, "SN_STATIC_CTL" },
  { 24070, "SN_STATIC_FIFO_DATA" },
  { 24071, "SN_STATIC_FIFO_SEL" },
  { 24073, "SN_STATIC_ISTATE" },
  { 24074, "SN_STATIC_OSTATE" },
  { 24076, "SN_STATIC_STATIC" },
  { 24320, "SN_STATIC_DATA_AVAIL" },
  { 24576, "MPL_AUX_PERF_COUNT_SET_0" },
  { 24577, "MPL_AUX_PERF_COUNT_SET_1" },
  { 24578, "MPL_AUX_PERF_COUNT_SET_2" },
  { 24579, "MPL_AUX_PERF_COUNT_SET_3" },
  { 24580, "MPL_AUX_PERF_COUNT" },
  { 24581, "AUX_PERF_COUNT_0" },
  { 24582, "AUX_PERF_COUNT_1" },
  { 24583, "AUX_PERF_COUNT_CTL" },
  { 24584, "AUX_PERF_COUNT_STS" },
};

const int tilepro_num_sprs = 499;

#endif /* DISASM_ONLY */

#ifndef DISASM_ONLY

#include <stdlib.h>

static int
tilepro_spr_compare (const void *a_ptr, const void *b_ptr)
{
  const struct tilepro_spr *a = (const struct tilepro_spr *) a_ptr;
  const struct tilepro_spr *b = (const struct tilepro_spr *) b_ptr;

  return a->number - b->number;
}

const char *
get_tilepro_spr_name (int num)
{
  void *result;
  struct tilepro_spr key;

  key.number = num;
  result = bsearch ((const void *) &key, (const void *) tilepro_sprs,
		    tilepro_num_sprs, sizeof (struct tilepro_spr),
		    tilepro_spr_compare);

  if (result == NULL)
    return NULL;

  {
    struct tilepro_spr *result_ptr = (struct tilepro_spr *) result;

    return result_ptr->name;
  }
}


/* Canonical name of each register. */
const char * const tilepro_register_names[] =
{
  "r0",   "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
  "r8",   "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
  "r16",  "r17", "r18", "r19", "r20", "r21", "r22", "r23",
  "r24",  "r25", "r26", "r27", "r28", "r29", "r30", "r31",
  "r32",  "r33", "r34", "r35", "r36", "r37", "r38", "r39",
  "r40",  "r41", "r42", "r43", "r44", "r45", "r46", "r47",
  "r48",  "r49", "r50", "r51", "r52", "tp",  "sp",  "lr",
  "sn",  "idn0", "idn1", "udn0", "udn1", "udn2", "udn3", "zero"
};

#endif /* not DISASM_ONLY */


/* Given a set of bundle bits and a specific pipe, returns which
   instruction the bundle contains in that pipe.  */

const struct tilepro_opcode *
find_opcode (tilepro_bundle_bits bits, tilepro_pipeline pipe)
{
  const unsigned short *table = tilepro_bundle_decoder_fsms[pipe];
  int i = 0;

  while (1)
    {
      unsigned short bitspec = table[i];
      unsigned int bitfield =
	((unsigned int) (bits >> (bitspec & 63))) & (bitspec >> 6);
      unsigned short next = table[i + 1 + bitfield];

      if (next <= TILEPRO_OPC_NONE)
	return &tilepro_opcodes[next];

      i = next - TILEPRO_OPC_NONE;
    }
}


int
parse_insn_tilepro (tilepro_bundle_bits bits,
                    unsigned int pc,
                    struct tilepro_decoded_instruction
                    decoded[TILEPRO_MAX_INSTRUCTIONS_PER_BUNDLE])
{
  int num_instructions = 0;
  int pipe;
  int min_pipe, max_pipe;

  if ((bits & TILEPRO_BUNDLE_Y_ENCODING_MASK) == 0)
    {
      min_pipe = TILEPRO_PIPELINE_X0;
      max_pipe = TILEPRO_PIPELINE_X1;
    }
  else
    {
      min_pipe = TILEPRO_PIPELINE_Y0;
      max_pipe = TILEPRO_PIPELINE_Y2;
    }

  /* For each pipe, find an instruction that fits.  */
  for (pipe = min_pipe; pipe <= max_pipe; pipe++)
    {
      const struct tilepro_opcode *opc;
      struct tilepro_decoded_instruction *d;
      int i;

      d = &decoded[num_instructions++];
      opc = find_opcode (bits, (tilepro_pipeline)pipe);
      d->opcode = opc;

      /* Decode each operand, sign extending, etc. as appropriate.  */
      for (i = 0; i < opc->num_operands; i++)
	{
	  const struct tilepro_operand *op =
	    &tilepro_operands[opc->operands[pipe][i]];
	  unsigned int opval = op->extract (bits);

	  if (op->is_signed)
	    {
	      /* Sign-extend the operand.  */
	      unsigned int sign = 1u << (op->num_bits - 1);
	      opval = ((opval & (sign + sign - 1)) ^ sign) - sign;
	    }

	  /* Adjust PC-relative scaled branch offsets.  */
	  if (op->type == TILEPRO_OP_TYPE_ADDRESS)
	    opval = opval * TILEPRO_BUNDLE_SIZE_IN_BYTES + pc;

	  /* Record the final value.  */
	  d->operands[i] = op;
	  d->operand_values[i] = opval;
	}
    }

  return num_instructions;
}
