/* Alpha VMS external format of Libraries.

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

#ifndef _VMS_LBR_H
#define _VMS_LBR_H

/* Libray HeaDer.  */

/* Magic numbers.  Should match the major version.  */

#define LHD_SANEID_DCX 319232342
#define LHD_SANEID3 233579905
#define LHD_SANEID6 233579911

/* Library type.  */
#define LBR__C_TYP_UNK    0	/* Unknown / unspecified.  */
#define LBR__C_TYP_OBJ    1	/* Vax object.  */
#define LBR__C_TYP_MLB    2	/* Macro.  */
#define LBR__C_TYP_HLP    3	/* Help.  */
#define LBR__C_TYP_TXT    4	/* Text.  */
#define LBR__C_TYP_SHSTB  5	/* Vax shareable image.  */
#define LBR__C_TYP_NCS    6	/* NCS.  */
#define LBR__C_TYP_EOBJ   7	/* Alpha object.  */
#define LBR__C_TYP_ESHSTB 8	/* Alpha shareable image.  */
#define LBR__C_TYP_IOBJ   9	/* IA-64 object.  */
#define LBR__C_TYP_ISHSTB 10	/* IA-64 shareable image.  */

struct vms_lhd
{
  /* Type of the library.  See above.  */
  unsigned char type;

  /* Number of indexes.  Generally 1, 2 for object libraries.  */
  unsigned char nindex;

  unsigned char fill_1[2];

  /* Sanity Id.  */
  unsigned char sanity[4];

  /* Version.  */
  unsigned char majorid[2];
  unsigned char minorid[2];

  /* Tool name.  */
  unsigned char lbrver[32];

  /* Create time.  */
  unsigned char credat[8];

  /* Update time.  */
  unsigned char updtim[8];

  /* Size of the MHD.  */
  unsigned char mhdusz;

  unsigned char idxblkf[2];	/* Unused.  */
  unsigned char fill_2;
  unsigned char closerror[2];

  unsigned char spareword[2];

  /* First free block, and number of free blocks.  */
  unsigned char freevbn[4];
  unsigned char freeblk[4];

  unsigned char nextrfa[6];
  unsigned char nextvbn[4];

  /* Free pre-allocated index block.  */
  /* Number of free blocks.  */
  unsigned char freidxblk[4];
  /* VBN of a simply linked list of free blocks.  The list is terminated by a
     nul VBN.  */
  unsigned char freeidx[4];

  /* Highest pre-allocated index block and in use.  */
  unsigned char hipreal[4];
  unsigned char hiprusd[4];

  /* Number of index blocks in use.  */
  unsigned char idxblks[4];

  /* Number of index entries.  */
  unsigned char idxcnt[4];

  /* Number of modules entries.  */
  unsigned char modcnt[4];

  unsigned char fill_3[2];

  /* Number of module headers.  */
  unsigned char modhdrs[4];

  /* Overhead index pointers.  */
  unsigned char idxovh[4];

  /* Update history records.  */
  unsigned char maxluhrec[2];
  unsigned char numluhrec[2];
  unsigned char begluhrfa[6];
  unsigned char endluhrfa[6];

  /* DCX map.  */
  unsigned char dcxmapvbn[4];

  unsigned char fill_4[4 * 13];
};

/* Known major ids.  */
#define LBR_MAJORID 3		/* Alpha libraries.  */
#define LBR_ELFMAJORID 6	/* Elf libraries (new index, new data).  */

/* Offset of the first IDD.  */
#define LHD_IDXDESC 196

/* InDex Description.  */
struct vms_idd
{
  unsigned char flags[2];

  /* Max length of the key.  */
  unsigned char keylen[2];

  /* First index block.  */
  unsigned char vbn[4];
};

/* IDD flags.  */
#define IDD__FLAGS_ASCII 1
#define IDD__FLAGS_LOCKED 2
#define IDD__FLAGS_VARLENIDX 4
#define IDD__FLAGS_NOCASECMP 8
#define IDD__FLAGS_NOCASENTR 16
#define IDD__FLAGS_UPCASNTRY 32

#define IDD_LENGTH 8

/* Index block.  */
#define INDEXDEF__LENGTH 512
#define INDEXDEF__BLKSIZ 500

struct vms_indexdef
{
  /* Number of bytes used.  */
  unsigned char used[2];

  /* VBN of the parent.  */
  unsigned char parent[4];

  unsigned char fill_1[6];

  /* The key field contains vms_idx/vms_elfidx structures, which are
     simply a key (= a string) and a rfa.  */
  unsigned char keys[INDEXDEF__BLKSIZ];
};

/* An offset in a file.  */

struct vms_rfa
{
  /* Logical block number, 1 based.
     0 means that the field is absent.  Block size is 512.  */
  unsigned char vbn[4];

  /* Offset within the block.  */
  unsigned char offset[2];
};

/* Index keys.  For version 3.  */

struct vms_idx
{
  /* Offset from the start of the vbn, so minimum should be
     DATA__DATA (ie 6).  */
  struct vms_rfa rfa;

  unsigned char keylen;
  /* The length of this field is in fact keylen.  */
  unsigned char keyname[256];
};

/* Index keys, for version 4 and later.  */

struct vms_elfidx
{
  struct vms_rfa rfa;

  unsigned char keylen[2];
  unsigned char flags;
  unsigned char keyname[256];
};

/* Flags of elfidx.  */

#define ELFIDX__WEAK 0x01	/* Weak symbol.  */
#define ELFIDX__GROUP 0x02	/* Group symbol.  */
#define ELFIDX__LISTRFA 0x04	/* RFA field points to an LHS.  */
#define ELFIDX__SYMESC 0x08	/* Long symbol.  */

#define RFADEF__C_INDEX 0xffff

/* List head structure.  That's what is pointed by rfa when LISTRFA flag
   is set in elfidx.  */

struct vms_lhs
{
  struct vms_rfa ng_g_rfa;	/* Non-group global.  */
  struct vms_rfa ng_wk_rfa;	/* Non-group weak.  */
  struct vms_rfa g_g_rfa;	/* Group global.  */
  struct vms_rfa g_wk_rfa;	/* Group weak.  */
  unsigned char flags;
};

/* List node structure.  Fields of LHS point to this structure.  */

struct vms_lns
{
  /* Next node in the list.  */
  struct vms_rfa nxtrfa;

  /* Module associated with the key.  */
  struct vms_rfa modrfa;
};

struct vms_datadef
{
  /* Number of records in this block.  */
  unsigned char recs;
  unsigned char fill_1;

  /* Next vbn.  */
  unsigned char link[4];

  /* Data.  The first word is the record length, followed by record
     data and a possible pad byte so that record length is always aligned.  */
  unsigned char data[506];
};
#define DATA__LENGTH 512
#define DATA__DATA 6

/* Key name block.  This is used for keys longer than 128 bytes.  */

struct vms_kbn
{
  /* Length of the key chunk.  */
  unsigned char keylen[2];

  /* RFA of the next chunk.  */
  struct vms_rfa rfa;

  /* Followed by the key chunk.  */
};

/* Module header.  */
struct vms_mhd
{
  /* Fixed part.  */
  unsigned char lbrflag;
  unsigned char id;
  unsigned char fill_1[2];
  unsigned char refcnt[4];
  unsigned char datim[8];

  unsigned char objstat;
  /* Ident or GSMATCH.  */
  unsigned char objidlng;
  unsigned char objid[31];

  unsigned char pad1[3];
  unsigned char otherefcnt[4];
  unsigned char modsize[4];
  unsigned char pad2[4];
};

#define MHD__C_MHDID 0xad	/* Value for id.  */
#define MHD__C_MHDLEN 16	/* Fixed part length.  */
#define MHD__C_USRDAT 16

/* Flags for objstat.  */
#define MHD__M_SELSRC 0x1	/* Selective search.  */
#define MHD__M_OBJTIR 0x2
#define MHD__M_WKSYM  0x4

struct vms_luh
{
  unsigned char nxtluhblk[4];
  unsigned char spare[2];
  unsigned char data[506];
};

struct vms_luhdef
{
  unsigned char rechdr[2];
  unsigned char reclen[2];
};
#define LUH__RECHDRLEN 4
#define LUH__RECHDRMRK 0xabba
#define LUH__DATAFLDLEN 506

/* Entry in the history.  */

struct vms_leh
{
  unsigned char date[8];
  unsigned char nbr_units[2];
  unsigned char action[2]; /* 1: delete, 2: insert, 3: replaced.  */
  unsigned char idlen;
  /* username
     modules... */
};

#endif /* _VMS_LBR_H */
