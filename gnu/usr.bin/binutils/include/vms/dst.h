/* Alpha VMS external format of Debug Symbol Table.

   Copyright (C) 2010-2023 Free Software Foundation, Inc.
   Written by Tristan Gingold <gingold@adacore.com>, AdaCore.

   This file is part of BFD, the Binary File Descriptor library.

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

#ifndef _VMS_DST_H
#define _VMS_DST_H

/* Also available in vms freeware v5.0 debug/alpha_dstrecrds.sdl.  */

struct vms_dst_header
{
  /* Length.  */
  unsigned char length[2];

  /* Type.  */
  unsigned char type[2];
};

/* Beginning of module.  */
#define DST__K_MODBEG 188

/* Some well known languages.  */
#define DST__K_MACRO	0
#define DST__K_BLISS	2
#define DST__K_C	7
#define DST__K_ADA	9
#define DST__K_CXX	15

struct vms_dst_modbeg
{
  unsigned char flags;
  unsigned char unused;
  unsigned char language[4];
  unsigned char major[2];
  unsigned char minor[2];
  /* Module name ASCIC.  */
  /* Ident name ASCIC.  */
};

/* Routine begin.  */
#define DST__K_RTNBEG 190

struct vms_dst_rtnbeg
{
  unsigned char flags;

  /* Address of the code.  */
  unsigned char address[4];

  /* Procedure descriptor address.  */
  unsigned char pd_address[4];

  /* Name: ASCIC  */
};

/* Line number.  */
#define DST__K_LINE_NUM 185

struct vms_dst_pcline
{
  unsigned char pcline_command;
  unsigned char field[4];
};

#define DST__K_DELTA_PC_W	1
#define DST__K_INCR_LINUM	2
#define DST__K_INCR_LINUM_W	3
#define DST__K_SET_LINUM_INCR	4
#define DST__K_SET_LINUM_INCR_W	5
#define DST__K_RESET_LINUM_INCR	6
#define DST__K_BEG_STMT_MODE	7
#define DST__K_END_STMT_MODE	8
#define DST__K_SET_LINUM	9
#define DST__K_SET_PC		10
#define DST__K_SET_PC_W		11
#define DST__K_SET_PC_L		12
#define DST__K_SET_STMTNUM	13
#define DST__K_TERM		14
#define DST__K_TERM_W		15
#define DST__K_SET_ABS_PC	16
#define DST__K_DELTA_PC_L	17
#define DST__K_INCR_LINUM_L	18
#define DST__K_SET_LINUM_B	19
#define DST__K_SET_LINUM_L	20
#define DST__K_TERM_L		21

/* Routine end.  */
#define DST__K_RTNEND 191

struct vms_dst_rtnend
{
  unsigned char unused;
  unsigned char size[4];
};

/* Prologue.  */
#define DST__K_PROLOG 162

struct vms_dst_prolog
{
  unsigned char bkpt_addr[4];
};

/* Epilog.  */
#define DST__K_EPILOG 127

struct vms_dst_epilog
{
  unsigned char flags;
  unsigned char count[4];
};

/* Module end.  */
#define DST__K_MODEND 189

/* Block begin.  */
#define DST__K_BLKBEG 176

struct vms_dst_blkbeg
{
  unsigned char unused;
  unsigned char address[4];
  /* Name ASCIC.  */
};

/* Block end.  */
#define DST__K_BLKEND 177

struct vms_dst_blkend
{
  unsigned char unused;
  unsigned char size[4];
};

/* Source correlation.  */
#define DST__K_SOURCE 155

#define DST__K_SRC_DECLFILE    1
#define DST__K_SRC_SETFILE     2
#define DST__K_SRC_SETREC_L    3
#define DST__K_SRC_SETREC_W    4
#define DST__K_SRC_SETLNUM_L   5
#define DST__K_SRC_SETLNUM_W   6
#define DST__K_SRC_INCRLNUM_B  7
#define DST__K_SRC_DEFLINES_W 10
#define DST__K_SRC_DEFLINES_B 11
#define DST__K_SRC_FORMFEED   16

struct vms_dst_src_decl_src
{
  unsigned char length;
  unsigned char flags;
  unsigned char fileid[2];
  unsigned char rms_cdt[8];
  unsigned char rms_ebk[4];
  unsigned char rms_ffb[2];
  unsigned char rms_rfo;
  /* Filename ASCIC.  */
};

/* Record begin.  */
#define DST__K_RECBEG 171

struct vms_dst_recbeg
{
  unsigned char vflags;
  unsigned char value[4];
  /* Filename ASCIC.  */
};

/* Record end.  */
#define DST__K_RECEND 172

/* Enumeration begin.  */
#define DST__K_ENUMBEG 165

/* Enumeration element.  */
#define DST__K_ENUMELT 164

/* Enumeration end.  */
#define DST__K_ENUMEND 166

/* Separate type specification.  */
#define DST__K_SEPTYP 163

/* Type specification.  */
#define DST__K_TYPSPEC 175

#define DST__K_TS_ATOM          1	/* Atomic.  */
#define DST__K_TS_DSC           2	/* VMS Standard descriptor.  */
#define DST__K_TS_IND           3	/* Indirect.  */
#define DST__K_TS_TPTR          4	/* Typed pointer.  */
#define DST__K_TS_PTR           5	/* Pointer.  */
#define DST__K_TS_PIC           6	/* Pictured.  */
#define DST__K_TS_ARRAY         7
#define DST__K_TS_SET           8
#define DST__K_TS_SUBRANGE      9	/* Subrange.  */
#define DST__K_TS_ADA_DSC      10	/* Ada descriptor.  */
#define DST__K_TS_FILE         11
#define DST__K_TS_AREA         12	/* Area (PL/I).  */
#define DST__K_TS_OFFSET       13	/* Offset (PL/I).  */
#define DST__K_TS_NOV_LENG     14	/* Novel Length.  */
#define DST__K_TS_IND_TSPEC    15	/* Internal to debugger.  */
#define DST__K_TS_SELF_REL_LABEL 16	/* Self-relative label (PL/I).  */
#define DST__K_TS_RFA          17	/* (Basic).  */
#define DST__K_TS_TASK         18	/* (Ada).  */
#define DST__K_TS_ADA_ARRAY    19
#define DST__K_TS_XMOD_IND     20	/* Cross-module indirect type spec.  */
#define DST__K_TS_CONSTRAINED  21	/* (Ada).  */
#define DST__K_TS_MAYBE_CONSTR 22	/* Might-be-constrained (Ada).  */
#define DST__K_TS_DYN_LOV_LENG 23
#define DST__K_TS_TPTR_D       24	/* Typed pointer to descriptor.  */
#define DST__K_TS_SCAN_TREE    25
#define DST__K_TS_SCAN_TREEPTR 26
#define DST__K_TS_INCOMPLETE   27
#define DST__K_TS_BLISS_BLOCK  28
#define DST__K_TS_TPTR_64      29
#define DST__K_TS_PTR_64       30
#define DST__K_TS_REF          31	/* C++ referenced type.  */
#define DST__K_TS_REF_64       32

/* Value Specification.  */
#define DST__K_VFLAGS_NOVAL	128 /* No value.  */
#define DST__K_VFLAGS_NOTACTIVE	248 /* Not active at current PC.  */
#define DST__K_VFLAGS_UNALLOC	249 /* Not allocated.  */
#define DST__K_VFLAGS_DSC	250 /* Descriptor format.  */
#define DST__K_VFLAGS_TVS	251 /* Trailing value spec.  */
#define DST__K_VS_FOLLOWS	253 /* Value specification follow.  */
#define DST__K_VFLAGS_BITOFFS	255 /* Value is a bit offset.  */

/* Vflags fields.  */
#define DST__K_VALKIND_MASK 0x03
#define DST__K_INDIR        0x04
#define DST__K_DISP         0x08
#define DST__K_REGNUM_MASK  0xf0
#define DST__K_REGNUM_SHIFT  4

#define DST__K_VALKIND_LITERAL 0
#define DST__K_VALKIND_ADDR    1
#define DST__K_VALKIND_DESC    2
#define DST__K_VALKIND_REG     3

/* Label.  */
#define DST__K_LABEL 187

struct vms_dst_label
{
  unsigned char unused;

  unsigned char value[4];
  unsigned char name[1];
};

/* Discontiguous range.  */
#define DST__K_DIS_RANGE 118
#endif /* _VMS_DST_H */
