/* Print Z80, Z180, EZ80 and R800 instructions
   Copyright (C) 2005-2023 Free Software Foundation, Inc.
   Contributed by Arnold Metselaar <arnold_m@operamail.com>

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "disassemble.h"
#include <stdio.h>

struct buffer
{
  bfd_vma base;
  int n_fetch;
  int n_used;
  signed char data[6];
  long inss; /* instruction set bit mask, taken from bfd_mach */
  int nn_len; /* address length: 2 - Z80 mode, 3 - ADL mode*/
} ;

typedef int (*func)(struct buffer *, disassemble_info *, const char *);

struct tab_elt
{
  unsigned char val;
  unsigned char mask;
  func          fp;
  const char *  text;
  unsigned      inss; /* bit mask of supported bfd_mach_* or 0 for all mach */
} ;

#define INSS_ALL 0
#define INSS_Z80 ((1 << bfd_mach_z80) | (1 << bfd_mach_z80strict) | (1 << bfd_mach_z80full))
#define INSS_R800 (1 << bfd_mach_r800)
#define INSS_GBZ80 (1 << bfd_mach_gbz80)
#define INSS_Z180 (1 << bfd_mach_z180)
#define INSS_EZ80_Z80 (1 << bfd_mach_ez80_z80)
#define INSS_EZ80_ADL (1 << bfd_mach_ez80_adl)
#define INSS_EZ80 (INSS_EZ80_ADL | INSS_EZ80_Z80)
#define INSS_Z80N (1 << bfd_mach_z80n)

#define TXTSIZ 24
/* Names of 16-bit registers.  */
static const char * rr_str[] = { "bc", "de", "hl", "sp" };
/* Names of 8-bit registers.  */
static const char * r_str[]  = { "b", "c", "d", "e", "h", "l", "(hl)", "a" };
/* Texts for condition codes.  */
static const char * cc_str[] = { "nz", "z", "nc", "c", "po", "pe", "p", "m" };
/* Instruction names for 8-bit arithmetic, operand "a" is often implicit */
static const char * arit_str[] =
{
  "add a,", "adc a,", "sub ", "sbc a,", "and ", "xor ", "or ", "cp "
} ;
static const char * arit_str_gbz80[] =
{
  "add a,", "adc a,", "sub a,", "sbc a,", "and ", "xor ", "or ", "cp "
} ;
static const char * arit_str_ez80[] =
{
  "add a,", "adc a,", "sub a,", "sbc a,", "and a,", "xor a,", "or a,", "cp a,"
} ;


static int
mach_inst (struct buffer *buf, const struct tab_elt *p)
{
  return !p->inss || (p->inss & buf->inss);
}

static int
fetch_data (struct buffer *buf, disassemble_info * info, int n)
{
  int r;

  if (buf->n_fetch + n > (int)sizeof (buf->data))
    abort ();

  r = info->read_memory_func (buf->base + buf->n_fetch,
			      (unsigned char*) buf->data + buf->n_fetch,
			      n, info);
  if (r == 0)
    buf->n_fetch += n;
  else
    info->memory_error_func (r, buf->base + buf->n_fetch, info);
  return !r;
}

static int
prt (struct buffer *buf, disassemble_info * info, const char *txt)
{
  info->fprintf_func (info->stream, "%s", txt);
  buf->n_used = buf->n_fetch;
  return 1;
}

static int
prt_e (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char e;
  int target_addr;

  if (fetch_data (buf, info, 1))
    {
      e = buf->data[1];
      target_addr = (buf->base + 2 + e) & 0xffff;
      buf->n_used = buf->n_fetch;
      info->fprintf_func (info->stream, "%s0x%04x", txt, target_addr);
    }
  else
    buf->n_used = -1;

  return buf->n_used;
}

static int
jr_cc (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];

  snprintf (mytxt, TXTSIZ, txt, cc_str[(buf->data[0] >> 3) & 3]);
  return prt_e (buf, info, mytxt);
}

static int
prt_nn (struct buffer *buf, disassemble_info * info, const char *txt)
{
  int nn;
  unsigned char *p;
  int i;

  p = (unsigned char*) buf->data + buf->n_fetch;
  if (fetch_data (buf, info, buf->nn_len))
    {
      nn = 0;
      i = buf->nn_len;
      while (i--)
        nn = nn * 0x100 + p[i];
      info->fprintf_func (info->stream, txt, nn);
      buf->n_used = buf->n_fetch;
    }
  else
    buf->n_used = -1;
  return buf->n_used;
}

static int
prt_rr_nn (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  int rr;

  rr = (buf->data[buf->n_fetch - 1] >> 4) & 3;
  snprintf (mytxt, TXTSIZ, txt, rr_str[rr]);
  return prt_nn (buf, info, mytxt);
}

static int
prt_rr (struct buffer *buf, disassemble_info * info, const char *txt)
{
  info->fprintf_func (info->stream, "%s%s", txt,
		      rr_str[(buf->data[buf->n_fetch - 1] >> 4) & 3]);
  buf->n_used = buf->n_fetch;
  return buf->n_used;
}

static int
prt_n (struct buffer *buf, disassemble_info * info, const char *txt)
{
  int n;
  unsigned char *p;

  p = (unsigned char*) buf->data + buf->n_fetch;

  if (fetch_data (buf, info, 1))
    {
      n = p[0];
      info->fprintf_func (info->stream, txt, n);
      buf->n_used = buf->n_fetch;
    }
  else
    buf->n_used = -1;

  return buf->n_used;
}

static int
prt_n_n (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  int n;
  unsigned char *p;

  p = (unsigned char*) buf->data + buf->n_fetch;

  if (fetch_data (buf, info, 1))
    {
      n = p[0];
      snprintf (mytxt, TXTSIZ, txt, n);
      buf->n_used = buf->n_fetch;
    }
  else
    buf->n_used = -1;

  return prt_n (buf, info, mytxt);
}

static int
prt_r_n (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  int r;

  r = (buf->data[buf->n_fetch - 1] >> 3) & 7;
  snprintf (mytxt, TXTSIZ, txt, r_str[r]);
  return prt_n (buf, info, mytxt);
}

static int
ld_r_n (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];

  snprintf (mytxt, TXTSIZ, txt, r_str[(buf->data[buf->n_fetch - 1] >> 3) & 7]);
  return prt_n (buf, info, mytxt);
}

static int
prt_r (struct buffer *buf, disassemble_info * info, const char *txt)
{
  info->fprintf_func (info->stream, txt,
		      r_str[(buf->data[buf->n_fetch - 1] >> 3) & 7]);
  buf->n_used = buf->n_fetch;
  return buf->n_used;
}

static int
ld_r_r (struct buffer *buf, disassemble_info * info, const char *txt)
{
  info->fprintf_func (info->stream, txt,
		      r_str[(buf->data[buf->n_fetch - 1] >> 3) & 7],
		      r_str[buf->data[buf->n_fetch - 1] & 7]);
  buf->n_used = buf->n_fetch;
  return buf->n_used;
}

static int
prt_d (struct buffer *buf, disassemble_info * info, const char *txt)
{
  int d;
  signed char *p;

  p = buf->data + buf->n_fetch;

  if (fetch_data (buf, info, 1))
    {
      d = p[0];
      info->fprintf_func (info->stream, txt, d);
      buf->n_used = buf->n_fetch;
    }
  else
    buf->n_used = -1;

  return buf->n_used;
}

static int
prt_rr_d (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  int rr;

  rr = (buf->data[buf->n_fetch - 1] >> 4) & 3;
  if (rr == 3) /* SP is not supported */
    return 0;

  snprintf (mytxt, TXTSIZ, txt, rr_str[rr]);
  return prt_d (buf, info, mytxt);
}

static int
arit_r (struct buffer *buf, disassemble_info * info, const char *txt)
{
  const char * const *arit;

  if (buf->inss & INSS_EZ80)
    arit = arit_str_ez80;
  else if (buf->inss & INSS_GBZ80)
    arit = arit_str_gbz80;
  else
    arit = arit_str;

  info->fprintf_func (info->stream, txt,
                      arit[(buf->data[buf->n_fetch - 1] >> 3) & 7],
                      r_str[buf->data[buf->n_fetch - 1] & 7]);
  buf->n_used = buf->n_fetch;
  return buf->n_used;
}

static int
prt_cc (struct buffer *buf, disassemble_info * info, const char *txt)
{
  info->fprintf_func (info->stream, "%s%s", txt,
		      cc_str[(buf->data[0] >> 3) & 7]);
  buf->n_used = buf->n_fetch;
  return buf->n_used;
}

static int
pop_rr (struct buffer *buf, disassemble_info * info, const char *txt)
{
  static char *rr_stack[] = { "bc","de","hl","af"};

  info->fprintf_func (info->stream, "%s %s", txt,
		      rr_stack[(buf->data[0] >> 4) & 3]);
  buf->n_used = buf->n_fetch;
  return buf->n_used;
}


static int
jp_cc_nn (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];

  snprintf (mytxt,TXTSIZ,
	    "%s%s,0x%%04x", txt, cc_str[(buf->data[0] >> 3) & 7]);
  return prt_nn (buf, info, mytxt);
}

static int
arit_n (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  const char * const *arit;

  if (buf->inss & INSS_EZ80)
    arit = arit_str_ez80;
  else if (buf->inss & INSS_GBZ80)
    arit = arit_str_gbz80;
  else
    arit = arit_str;

  snprintf (mytxt,TXTSIZ, txt, arit[(buf->data[0] >> 3) & 7]);
  return prt_n (buf, info, mytxt);
}

static int
rst (struct buffer *buf, disassemble_info * info, const char *txt)
{
  info->fprintf_func (info->stream, txt, buf->data[0] & 0x38);
  buf->n_used = buf->n_fetch;
  return buf->n_used;
}


static int
cis (struct buffer *buf, disassemble_info * info, const char *txt ATTRIBUTE_UNUSED)
{
  static char * opar[] = { "ld", "cp", "in", "out" };
  char * op;
  char c;

  c = buf->data[1];
  op = ((0x13 & c) == 0x13) ? "ot" : (opar[c & 3]);
  info->fprintf_func (info->stream,
		      "%s%c%s", op,
		      (c & 0x08) ? 'd' : 'i',
		      (c & 0x10) ? "r" : "");
  buf->n_used = 2;
  return buf->n_used;
}

static int
cism (struct buffer *buf, disassemble_info * info, const char *txt ATTRIBUTE_UNUSED)
{
  static char * opar[] = { "in%cm%s", "ot%cm%s" };
  char * op;
  char c;

  c = buf->data[1];
  op = opar[c & 1];
  info->fprintf_func (info->stream,
                      op,
                      (c & 0x08) ? 'd' : 'i',
                      (c & 0x10) ? "r" : "");
  buf->n_used = 2;
  return buf->n_used;
}

static int
dump (struct buffer *buf, disassemble_info * info, const char *txt)
{
  int i;

  info->fprintf_func (info->stream, "defb ");
  for (i = 0; txt[i]; ++i)
    info->fprintf_func (info->stream, i ? ", 0x%02x" : "0x%02x",
			(unsigned char) buf->data[i]);
  buf->n_used = i;
  return buf->n_used;
}

/* Table to disassemble machine codes with prefix 0xED.  */
static const struct tab_elt opc_ed[] =
{
  { 0x30, 0xFF, prt, "mul d,e", INSS_Z80N },
  { 0x31, 0xFF, prt, "add hl,a", INSS_Z80N },
  { 0x31, 0xFF, prt, "ld iy,(hl)", INSS_EZ80 },
  { 0x30, 0xFE, dump, "xx", INSS_ALL }, /* do not move this line */
  { 0x00, 0xC7, prt_r_n, "in0 %s,(0x%%02x)", INSS_Z180|INSS_EZ80 },
  { 0x01, 0xC7, prt_r_n, "out0 (0x%%02x),%s", INSS_Z180|INSS_EZ80 },
  { 0x32, 0xFF, prt_d, "lea ix,ix%+d", INSS_EZ80 },
  { 0x33, 0xFF, prt_d, "lea iy,iy%+d", INSS_EZ80 },
  { 0x02, 0xCF, prt_rr_d, "lea %s,ix%%+d", INSS_EZ80 },
  { 0x03, 0xCF, prt_rr_d, "lea %s,iy%%+d", INSS_EZ80 },
  { 0x04, 0xC7, prt_r, "tst %s", INSS_Z180},
  { 0x04, 0xC7, prt_r, "tst a,%s", INSS_EZ80 },
  { 0x07, 0xFF, prt, "ld bc,(hl)", INSS_EZ80 },
  { 0x3F, 0xFF, prt, "ld (hl),ix", INSS_EZ80 },
  { 0x0F, 0xCF, prt_rr, "ld (hl),", INSS_EZ80 },
  { 0x17, 0xFF, prt, "ld de,(hl)", INSS_EZ80 },
  { 0x23, 0xFF, prt, "swapnib", INSS_Z80N },
  { 0x24, 0xFF, prt, "mirror", INSS_Z80N },
  { 0x27, 0xFF, prt, "ld hl,(hl)", INSS_EZ80 },
  { 0x27, 0xFF, prt_n, "test 0x%02x", INSS_Z80N },
  { 0x28, 0xFF, prt, "bsla de,b", INSS_Z80N },
  { 0x29, 0xFF, prt, "bsra de,b", INSS_Z80N },
  { 0x2A, 0xFF, prt, "bsrl de,b", INSS_Z80N },
  { 0x2B, 0xFF, prt, "bsrf de,b", INSS_Z80N },
  { 0x2C, 0xFF, prt, "bslc de,b", INSS_Z80N },
  { 0x32, 0xFF, prt, "add de,a", INSS_Z80N },
  { 0x33, 0xFF, prt, "add bc,a", INSS_Z80N },
  { 0x34, 0xFF, prt_nn, "add hl,0x%04x", INSS_Z80N },
  { 0x35, 0xFF, prt_nn, "add de,0x%04x", INSS_Z80N },
  { 0x36, 0xFF, prt_nn, "add bc,0x%04x", INSS_Z80N },
  { 0x37, 0xFF, prt, "ld ix,(hl)", INSS_EZ80 },
  { 0x3E, 0xFF, prt, "ld (hl),iy", INSS_EZ80 },
  { 0x70, 0xFF, prt, "in f,(c)", INSS_Z80 | INSS_R800 | INSS_Z80N },
  { 0x70, 0xFF, dump, "xx", INSS_ALL },
  { 0x40, 0xC7, prt_r, "in %s,(bc)", INSS_EZ80 },
  { 0x40, 0xC7, prt_r, "in %s,(c)", INSS_ALL },
  { 0x71, 0xFF, prt, "out (c),0", INSS_Z80 | INSS_Z80N },
  { 0x71, 0xFF, dump, "xx", INSS_ALL },
  { 0x41, 0xC7, prt_r, "out (bc),%s", INSS_EZ80 },
  { 0x41, 0xC7, prt_r, "out (c),%s", INSS_ALL },
  { 0x42, 0xCF, prt_rr, "sbc hl,", INSS_ALL },
  { 0x43, 0xCF, prt_rr_nn, "ld (0x%%04x),%s", INSS_ALL },
  { 0x44, 0xFF, prt, "neg", INSS_ALL },
  { 0x45, 0xFF, prt, "retn", INSS_ALL },
  { 0x46, 0xFF, prt, "im 0", INSS_ALL },
  { 0x47, 0xFF, prt, "ld i,a", INSS_ALL },
  { 0x4A, 0xCF, prt_rr, "adc hl,", INSS_ALL },
  { 0x4B, 0xCF, prt_rr_nn, "ld %s,(0x%%04x)", INSS_ALL },
  { 0x4C, 0xCF, prt_rr, "mlt ", INSS_Z180|INSS_EZ80 },
  { 0x4D, 0xFF, prt, "reti", INSS_ALL },
  { 0x4F, 0xFF, prt, "ld r,a", INSS_ALL },
  { 0x54, 0xFF, prt_d, "lea ix,iy%+d", INSS_EZ80 },
  { 0x55, 0xFF, prt_d, "lea iy,ix%+d", INSS_EZ80 },
  { 0x56, 0xFF, prt, "im 1", INSS_ALL },
  { 0x57, 0xFF, prt, "ld a,i", INSS_ALL },
  { 0x5E, 0xFF, prt, "im 2", INSS_ALL },
  { 0x5F, 0xFF, prt, "ld a,r", INSS_ALL },
  { 0x64, 0xFF, prt_n, "tst 0x%02x", INSS_Z180 },
  { 0x64, 0xFF, prt_n, "tst a,0x%02x", INSS_EZ80 },
  { 0x65, 0xFF, prt_d, "pea ix%+d", INSS_EZ80 },
  { 0x66, 0xFF, prt_d, "pea iy%+d", INSS_EZ80 },
  { 0x67, 0xFF, prt, "rrd", INSS_ALL },
  { 0x6D, 0xFF, prt, "ld mb,a", INSS_EZ80 },
  { 0x6E, 0xFF, prt, "ld a,mb", INSS_EZ80 },
  { 0x6F, 0xFF, prt, "rld", INSS_ALL },
  { 0x74, 0xFF, prt_n, "tstio 0x%02x", INSS_Z180|INSS_EZ80 },
  { 0x76, 0xFF, prt, "slp", INSS_Z180|INSS_EZ80 },
  { 0x7D, 0xFF, prt, "stmix", INSS_EZ80 },
  { 0x7E, 0xFF, prt, "rsmix", INSS_EZ80 },
  { 0x82, 0xE6, cism, "", INSS_Z180|INSS_EZ80 },
  { 0x84, 0xFF, prt, "ini2", INSS_EZ80 },
  { 0x8A, 0xFF, prt_n_n, "push 0x%02x%%02x", INSS_Z80N },
  { 0x8C, 0xFF, prt, "ind2", INSS_EZ80 },
  { 0x90, 0xFF, prt, "outinb", INSS_Z80N },
  { 0x91, 0xFF, prt_n_n, "nextreg 0x%02x,0x%%02x", INSS_Z80N },
  { 0x92, 0xFF, prt_n, "nextreg 0x%02x,a", INSS_Z80N },
  { 0x93, 0xFF, prt, "pixeldn", INSS_Z80N },
  { 0x94, 0xFF, prt, "ini2r", INSS_EZ80 },
  { 0x94, 0xFF, prt, "pixelad", INSS_Z80N },
  { 0x95, 0xFF, prt, "setae", INSS_Z80N },
  { 0x98, 0xFF, prt, "jp (c)", INSS_Z80N },
  { 0x9c, 0xFF, prt, "ind2r", INSS_EZ80 },
  { 0xA0, 0xE4, cis, "", INSS_ALL },
  { 0xA4, 0xFF, prt, "outi2", INSS_EZ80 },
  { 0xA4, 0xFF, prt, "ldix", INSS_Z80N },
  { 0xAC, 0xFF, prt, "outd2", INSS_EZ80 },
  { 0xAC, 0xFF, prt, "lddx", INSS_Z80N },
  { 0xA5, 0xFF, prt, "ldws", INSS_Z80N },
  { 0xB4, 0xFF, prt, "oti2r", INSS_EZ80 },
  { 0xB4, 0xFF, prt, "ldirx", INSS_Z80N },
  { 0xB7, 0xFF, prt, "ldpirx", INSS_Z80N },
  { 0xBC, 0xFF, prt, "otd2r", INSS_EZ80 },
  { 0xBC, 0xFF, prt, "lddrx", INSS_Z80N },
  { 0xC2, 0xFF, prt, "inirx", INSS_EZ80 },
  { 0xC3, 0xFF, prt, "otirx", INSS_EZ80 },
  { 0xC7, 0xFF, prt, "ld i,hl", INSS_EZ80 },
  { 0xCA, 0xFF, prt, "indrx", INSS_EZ80 },
  { 0xCB, 0xFF, prt, "otdrx", INSS_EZ80 },
  { 0xC3, 0xFF, prt, "muluw hl,bc", INSS_R800 },
  { 0xC5, 0xE7, prt_r, "mulub a,%s", INSS_R800 },
  { 0xD7, 0xFF, prt, "ld hl,i", INSS_EZ80 },
  { 0xF3, 0xFF, prt, "muluw hl,sp", INSS_R800 },
  { 0x00, 0x00, dump, "xx", INSS_ALL }
};

static int
pref_ed (struct buffer *buf, disassemble_info *info,
         const char *txt ATTRIBUTE_UNUSED)
{
  const struct tab_elt *p;

  if (fetch_data (buf, info, 1))
    {
      for (p = opc_ed; p->val != (buf->data[1] & p->mask) || !mach_inst (buf, p); ++p)
        ;
      p->fp (buf, info, p->text);
    }
  else
    buf->n_used = -1;

  return buf->n_used;
}

/* Instruction names for the instructions addressing single bits.  */
static char *cb1_str[] = { "", "bit", "res", "set"};
/* Instruction names for shifts and rotates.  */
static char *cb2_str[] =
{
  "rlc", "rrc", "rl", "rr", "sla", "sra", "sli", "srl"
};

static int
pref_cb (struct buffer *buf, disassemble_info *info,
         const char *txt ATTRIBUTE_UNUSED)
{
  const char *op_txt;
  int idx;
  if (fetch_data (buf, info, 1))
    {
      buf->n_used = 2;
      if ((buf->data[1] & 0xc0) == 0)
        {
          idx = (buf->data[1] >> 3) & 7;
          if ((buf->inss & INSS_GBZ80) && (idx == 6))
            op_txt = "swap";
          else
            op_txt = cb2_str[idx];
          info->fprintf_func (info->stream, "%s %s",
                              op_txt,
                              r_str[buf->data[1] & 7]);
        }
      else
	info->fprintf_func (info->stream, "%s %d,%s",
			    cb1_str[(buf->data[1] >> 6) & 3],
			    (buf->data[1] >> 3) & 7,
			    r_str[buf->data[1] & 7]);
    }
  else
    buf->n_used = -1;

  return buf->n_used;
}

static int
addvv (struct buffer * buf, disassemble_info * info, const char *txt)
{
  info->fprintf_func (info->stream, "add %s,%s", txt, txt);

  return buf->n_used = buf->n_fetch;
}

static int
ld_v_v (struct buffer * buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];

  snprintf (mytxt, TXTSIZ, "ld %s%%s,%s%%s", txt, txt);
  return ld_r_r (buf, info, mytxt);
}

static int
prt_d_n (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  int d;
  signed char *p;

  p = buf->data + buf->n_fetch;

  if (fetch_data (buf, info, 1))
    {
      d = p[0];
      snprintf (mytxt, TXTSIZ, txt, d);
      return prt_n (buf, info, mytxt);
    }
  else
    buf->n_used = -1;

  return buf->n_used;
}

static int
arit_d (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  signed char c;
  const char * const *arit;

  arit = (buf->inss & INSS_EZ80) ? arit_str_ez80 : arit_str;
  c = buf->data[buf->n_fetch - 1];
  snprintf (mytxt, TXTSIZ, txt, arit[(c >> 3) & 7]);
  return prt_d (buf, info, mytxt);
}

static int
ld_r_d (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  signed char c;

  c = buf->data[buf->n_fetch - 1];
  snprintf (mytxt, TXTSIZ, txt, r_str[(c >> 3) & 7]);
  return prt_d (buf, info, mytxt);
}

static int
ld_d_r (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  signed char c;

  c = buf->data[buf->n_fetch - 1];
  snprintf (mytxt, TXTSIZ, txt, r_str[c & 7]);
  return prt_d (buf, info, mytxt);
}

static int
ld_ii_ii (struct buffer *buf, disassemble_info * info, const char *txt)
{
  char mytxt[TXTSIZ];
  signed char c;
  int p;
  static const char *ii[2] = { "ix", "iy" };

  p = (buf->data[buf->n_fetch - 2] == (signed char) 0xdd) ? 0 : 1;
  c = buf->data[buf->n_fetch - 1];
  if ((c & 0x07) != 0x07)
    p = 1 - p; /* 0 -> 1, 1 -> 0 */
  snprintf (mytxt, TXTSIZ, txt, ii[p]);
  return prt_d (buf, info, mytxt);
}

static int
pref_xd_cb (struct buffer * buf, disassemble_info * info, const char *txt)
{
  if (fetch_data (buf, info, 2))
    {
      int d;
      char arg[TXTSIZ];
      signed char *p;

      buf->n_used = 4;
      p = buf->data;
      d = p[2];

      if (((p[3] & 0xC0) == 0x40) || ((p[3] & 7) == 0x06))
	snprintf (arg, TXTSIZ, "(%s%+d)", txt, d);
      else
	snprintf (arg, TXTSIZ, "(%s%+d),%s", txt, d, r_str[p[3] & 7]);

      if ((p[3] & 0xc0) == 0)
	info->fprintf_func (info->stream, "%s %s",
			    cb2_str[(buf->data[3] >> 3) & 7],
			    arg);
      else
	info->fprintf_func (info->stream, "%s %d,%s",
			    cb1_str[(buf->data[3] >> 6) & 3],
			    (buf->data[3] >> 3) & 7,
			    arg);
    }
  else
    buf->n_used = -1;

  return buf->n_used;
}

/* Table to disassemble machine codes with prefix 0xDD or 0xFD.  */
static struct tab_elt opc_ind[] =
{
  { 0x07, 0xFF, prt_d, "ld bc,(%s%%+d)", INSS_EZ80 },
  { 0x0F, 0xFF, prt_d, "ld (%s%%+d),bc", INSS_EZ80 },
  { 0x17, 0xFF, prt_d, "ld de,(%s%%+d)", INSS_EZ80 },
  { 0x1F, 0xFF, prt_d, "ld (%s%%+d),de", INSS_EZ80 },
  { 0x24, 0xF7, prt_r, "inc %s%%s", INSS_ALL },
  { 0x25, 0xF7, prt_r, "dec %s%%s", INSS_ALL },
  { 0x26, 0xF7, ld_r_n, "ld %s%%s,0x%%%%02x", INSS_ALL },
  { 0x27, 0xFF, prt_d, "ld hl,(%s%%+d)", INSS_EZ80 },
  { 0x2F, 0xFF, prt_d, "ld (%s%%+d),hl", INSS_EZ80 },
  { 0x21, 0xFF, prt_nn, "ld %s,0x%%04x", INSS_ALL },
  { 0x22, 0xFF, prt_nn, "ld (0x%%04x),%s", INSS_ALL },
  { 0x2A, 0xFF, prt_nn, "ld %s,(0x%%04x)", INSS_ALL },
  { 0x23, 0xFF, prt, "inc %s", INSS_ALL },
  { 0x2B, 0xFF, prt, "dec %s", INSS_ALL },
  { 0x29, 0xFF, addvv, "%s", INSS_ALL },
  { 0x31, 0xFF, ld_ii_ii, "ld %%s,(%s%%%%+d)", INSS_EZ80 },
  { 0x37, 0xFF, ld_ii_ii, "ld %%s,(%s%%%%+d)", INSS_EZ80 },
  { 0x3E, 0xFE, ld_ii_ii, "ld (%s%%%%+d),%%s", INSS_EZ80 },
  { 0x09, 0xCF, prt_rr, "add %s,", INSS_ALL },
  { 0x34, 0xFF, prt_d, "inc (%s%%+d)", INSS_ALL },
  { 0x35, 0xFF, prt_d, "dec (%s%%+d)", INSS_ALL },
  { 0x36, 0xFF, prt_d_n, "ld (%s%%+d),0x%%%%02x", INSS_ALL },

  { 0x76, 0xFF, dump, "h", INSS_ALL },
  { 0x46, 0xC7, ld_r_d, "ld %%s,(%s%%%%+d)", INSS_ALL },
  { 0x70, 0xF8, ld_d_r, "ld (%s%%%%+d),%%s", INSS_ALL },
  { 0x64, 0xF6, ld_v_v, "%s", INSS_ALL },
  { 0x60, 0xF0, ld_r_r, "ld %s%%s,%%s", INSS_ALL },
  { 0x44, 0xC6, ld_r_r, "ld %%s,%s%%s", INSS_ALL },

  { 0x86, 0xC7, arit_d, "%%s(%s%%%%+d)", INSS_ALL },
  { 0x84, 0xC6, arit_r, "%%s%s%%s", INSS_ALL },

  { 0xE1, 0xFF, prt, "pop %s", INSS_ALL },
  { 0xE5, 0xFF, prt, "push %s", INSS_ALL },
  { 0xCB, 0xFF, pref_xd_cb, "%s", INSS_ALL },
  { 0xE3, 0xFF, prt, "ex (sp),%s", INSS_ALL },
  { 0xE9, 0xFF, prt, "jp (%s)", INSS_ALL },
  { 0xF9, 0xFF, prt, "ld sp,%s", INSS_ALL },
  { 0x00, 0x00, dump, "?", INSS_ALL },
} ;

static int
pref_ind (struct buffer *buf, disassemble_info *info, const char *txt)
{
  if (fetch_data (buf, info, 1))
    {
      char mytxt[TXTSIZ];
      struct tab_elt *p;

      for (p = opc_ind; p->val != (buf->data[1] & p->mask) || !mach_inst (buf, p); ++p)
        ;
      snprintf (mytxt, TXTSIZ, p->text, txt);
      p->fp (buf, info, mytxt);
    }
  else
    buf->n_used = -1;

  return buf->n_used;
}

static int
print_insn_z80_buf (struct buffer *buf, disassemble_info *info);

static int
suffix (struct buffer *buf, disassemble_info *info, const char *txt)
{
  char mybuf[TXTSIZ*4];
  fprintf_ftype old_fprintf;
  void *old_stream;
  char *p;

  switch (txt[2])
    {
    case 'l': /* SIL or LIL */
      buf->nn_len = 3;
      break;
    case 's': /* SIS or LIS */
      buf->nn_len = 2;
      break;
    default:
      abort ();
    }
  if (!fetch_data (buf, info, 1)
      || buf->data[1] == 0x40
      || buf->data[1] == 0x49
      || buf->data[1] == 0x52
      || buf->data[1] == 0x5b)
    {
      /* Double prefix, or end of data.  */
      info->fprintf_func (info->stream, ".db 0x%02x ; %s", (unsigned)buf->data[0], txt);
      buf->n_used = 1;
      return buf->n_used;
    }

  old_fprintf = info->fprintf_func;
  old_stream = info->stream;
  info->fprintf_func = (fprintf_ftype) &sprintf;
  info->stream = mybuf;
  mybuf[0] = 0;
  buf->base++;
  if (print_insn_z80_buf (buf, info) >= 0)
    buf->n_used++;
  info->fprintf_func = old_fprintf;
  info->stream = old_stream;

  for (p = mybuf; *p; ++p)
    if (*p == ' ')
      break;
  if (*p)
    {
      *p++ = '\0';
      info->fprintf_func (info->stream, "%s.%s %s", mybuf, txt, p);
    }
  else
    info->fprintf_func (info->stream, "%s.%s", mybuf, txt);
  return buf->n_used;
}

/* Table to disassemble machine codes without prefix.  */
static const struct tab_elt
opc_main[] =
{
  { 0x00, 0xFF, prt, "nop", INSS_ALL },
  { 0x01, 0xCF, prt_rr_nn, "ld %s,0x%%04x", INSS_ALL },
  { 0x02, 0xFF, prt, "ld (bc),a", INSS_ALL },
  { 0x03, 0xCF, prt_rr, "inc ", INSS_ALL },
  { 0x04, 0xC7, prt_r, "inc %s", INSS_ALL },
  { 0x05, 0xC7, prt_r, "dec %s", INSS_ALL },
  { 0x06, 0xC7, ld_r_n, "ld %s,0x%%02x", INSS_ALL },
  { 0x07, 0xFF, prt, "rlca", INSS_ALL },
  { 0x08, 0xFF, prt, "ex af,af'", ~INSS_GBZ80 },
  { 0x09, 0xCF, prt_rr, "add hl,", INSS_ALL },
  { 0x0A, 0xFF, prt, "ld a,(bc)", INSS_ALL },
  { 0x0B, 0xCF, prt_rr, "dec ", INSS_ALL },
  { 0x0F, 0xFF, prt, "rrca", INSS_ALL },
  { 0x10, 0xFF, prt_e, "djnz ", ~INSS_GBZ80 },
  { 0x12, 0xFF, prt, "ld (de),a", INSS_ALL },
  { 0x17, 0xFF, prt, "rla", INSS_ALL },
  { 0x18, 0xFF, prt_e, "jr ", INSS_ALL },
  { 0x1A, 0xFF, prt, "ld a,(de)", INSS_ALL },
  { 0x1F, 0xFF, prt, "rra", INSS_ALL },
  { 0x20, 0xE7, jr_cc, "jr %s,", INSS_ALL },
  { 0x22, 0xFF, prt_nn, "ld (0x%04x),hl", ~INSS_GBZ80 },
  { 0x27, 0xFF, prt, "daa", INSS_ALL },
  { 0x2A, 0xFF, prt_nn, "ld hl,(0x%04x)", ~INSS_GBZ80 },
  { 0x2F, 0xFF, prt, "cpl", INSS_ALL },
  { 0x32, 0xFF, prt_nn, "ld (0x%04x),a", INSS_ALL },
  { 0x37, 0xFF, prt, "scf", INSS_ALL },
  { 0x3A, 0xFF, prt_nn, "ld a,(0x%04x)", INSS_ALL },
  { 0x3F, 0xFF, prt, "ccf", INSS_ALL },

  { 0x76, 0xFF, prt, "halt", INSS_ALL },

  { 0x40, 0xFF, suffix, "sis", INSS_EZ80 },
  { 0x49, 0xFF, suffix, "lis", INSS_EZ80 },
  { 0x52, 0xFF, suffix, "sil", INSS_EZ80 },
  { 0x5B, 0xFF, suffix, "lil", INSS_EZ80 },

  { 0x40, 0xC0, ld_r_r, "ld %s,%s", INSS_ALL},

  { 0x80, 0xC0, arit_r, "%s%s", INSS_ALL },

  { 0xC0, 0xC7, prt_cc, "ret ", INSS_ALL },
  { 0xC1, 0xCF, pop_rr, "pop", INSS_ALL },
  { 0xC2, 0xC7, jp_cc_nn, "jp ", INSS_ALL },
  { 0xC3, 0xFF, prt_nn, "jp 0x%04x", INSS_ALL },
  { 0xC4, 0xC7, jp_cc_nn, "call ", INSS_ALL },
  { 0xC5, 0xCF, pop_rr, "push", INSS_ALL },
  { 0xC6, 0xC7, arit_n, "%s0x%%02x", INSS_ALL },
  { 0xC7, 0xC7, rst, "rst 0x%02x", INSS_ALL },
  { 0xC9, 0xFF, prt, "ret", INSS_ALL },
  { 0xCB, 0xFF, pref_cb, "", INSS_ALL },
  { 0xCD, 0xFF, prt_nn, "call 0x%04x", INSS_ALL },
  { 0xD3, 0xFF, prt_n, "out (0x%02x),a", ~INSS_GBZ80 },
  { 0xD9, 0xFF, prt, "exx", ~INSS_GBZ80 },
  { 0xDB, 0xFF, prt_n, "in a,(0x%02x)", ~INSS_GBZ80 },
  { 0xDD, 0xFF, pref_ind, "ix", ~INSS_GBZ80 },
  { 0xE3, 0xFF, prt, "ex (sp),hl", ~INSS_GBZ80 },
  { 0xE9, 0xFF, prt, "jp (hl)", INSS_ALL },
  { 0xEB, 0xFF, prt, "ex de,hl", ~INSS_GBZ80 },
  { 0xED, 0xFF, pref_ed, "", ~INSS_GBZ80 },
  { 0xF3, 0xFF, prt, "di", INSS_ALL },
  { 0xF9, 0xFF, prt, "ld sp,hl", INSS_ALL },
  { 0xFB, 0xFF, prt, "ei", INSS_ALL },
  { 0xFD, 0xFF, pref_ind, "iy", ~INSS_GBZ80 },
  { 0x00, 0x00, prt, "????", INSS_ALL },
} ;

/* Separate GBZ80 main opcode table due to many differences */
static const struct tab_elt
opc_main_gbz80[] =
{
  { 0x00, 0xFF, prt,"nop", INSS_ALL },
  { 0x01, 0xCF, prt_rr_nn, "ld %s,0x%%04x", INSS_ALL },
  { 0x02, 0xFF, prt, "ld (bc),a", INSS_ALL },
  { 0x03, 0xCF, prt_rr, "inc ", INSS_ALL },
  { 0x04, 0xC7, prt_r, "inc %s", INSS_ALL },
  { 0x05, 0xC7, prt_r, "dec %s", INSS_ALL },
  { 0x06, 0xC7, ld_r_n, "ld %s,0x%%02x", INSS_ALL },
  { 0x07, 0xFF, prt, "rlca", INSS_ALL },
  { 0x08, 0xFF, prt_nn, "ld (0x%04x),sp", INSS_GBZ80 },
  { 0x09, 0xCF, prt_rr, "add hl,", INSS_ALL },
  { 0x0A, 0xFF, prt, "ld a,(bc)", INSS_ALL },
  { 0x0B, 0xCF, prt_rr, "dec ", INSS_ALL },
  { 0x0F, 0xFF, prt, "rrca", INSS_ALL },
  { 0x10, 0xFF, prt, "stop", INSS_GBZ80 },
  { 0x12, 0xFF, prt, "ld (de),a", INSS_ALL },
  { 0x17, 0xFF, prt, "rla", INSS_ALL },
  { 0x18, 0xFF, prt_e, "jr ", INSS_ALL },
  { 0x1A, 0xFF, prt, "ld a,(de)", INSS_ALL },
  { 0x1F, 0xFF, prt, "rra", INSS_ALL },
  { 0x20, 0xE7, jr_cc, "jr %s,", INSS_ALL },
  { 0x22, 0xFF, prt, "ld (hl+),a", INSS_GBZ80 },
  { 0x27, 0xFF, prt, "daa", INSS_ALL },
  { 0x2A, 0xFF, prt, "ld a,(hl+)", INSS_GBZ80 },
  { 0x2F, 0xFF, prt, "cpl", INSS_ALL },
  { 0x32, 0xFF, prt, "ld (hl-),a", INSS_GBZ80 },
  { 0x37, 0xFF, prt, "scf", INSS_ALL },
  { 0x3A, 0xFF, prt, "ld a,(hl-)", INSS_GBZ80 },
  { 0x3F, 0xFF, prt, "ccf", INSS_ALL },
  { 0x76, 0xFF, prt, "halt", INSS_ALL },
  { 0x40, 0xC0, ld_r_r, "ld %s,%s", INSS_ALL},
  { 0x80, 0xC0, arit_r, "%s%s", INSS_ALL },
  { 0xC0, 0xE7, prt_cc, "ret ", INSS_ALL },
  { 0xC1, 0xCF, pop_rr, "pop", INSS_ALL },
  { 0xC2, 0xE7, jp_cc_nn, "jp ", INSS_ALL },
  { 0xC3, 0xFF, prt_nn, "jp 0x%04x", INSS_ALL },
  { 0xC4, 0xE7, jp_cc_nn, "call ", INSS_ALL },
  { 0xC5, 0xCF, pop_rr, "push", INSS_ALL },
  { 0xC6, 0xC7, arit_n, "%s0x%%02x", INSS_ALL },
  { 0xC7, 0xC7, rst, "rst 0x%02x", INSS_ALL },
  { 0xC9, 0xFF, prt, "ret", INSS_ALL },
  { 0xCB, 0xFF, pref_cb, "", INSS_ALL },
  { 0xCD, 0xFF, prt_nn, "call 0x%04x", INSS_ALL },
  { 0xD9, 0xFF, prt, "reti", INSS_GBZ80 },
  { 0xE0, 0xFF, prt_n, "ldh (0x%02x),a", INSS_GBZ80 },
  { 0xE2, 0xFF, prt, "ldh (c),a", INSS_GBZ80 },
  { 0xE8, 0xFF, prt_d, "add sp,%d", INSS_GBZ80 },
  { 0xE9, 0xFF, prt, "jp (hl)", INSS_ALL },
  { 0xEA, 0xFF, prt_nn, "ld (0x%04x),a", INSS_GBZ80 },
  { 0xF0, 0xFF, prt_n, "ldh a,(0x%02x)", INSS_GBZ80 },
  { 0xF2, 0xFF, prt, "ldh a,(c)", INSS_GBZ80 },
  { 0xF3, 0xFF, prt, "di", INSS_ALL },
  { 0xF8, 0xFF, prt_d, "ldhl sp,%d", INSS_GBZ80 },
  { 0xF9, 0xFF, prt, "ld sp,hl", INSS_ALL },
  { 0xFA, 0xFF, prt_nn, "ld a,(0x%04x)", INSS_GBZ80 },
  { 0xFB, 0xFF, prt, "ei", INSS_ALL },
  { 0x00, 0x00, dump, "?", INSS_ALL },
} ;

int
print_insn_z80 (bfd_vma addr, disassemble_info * info)
{
  struct buffer buf;

  buf.base = addr;
  buf.inss = 1 << info->mach;
  buf.nn_len = info->mach == bfd_mach_ez80_adl ? 3 : 2;
  info->bytes_per_line = (buf.inss & INSS_EZ80) ? 6 : 4; /* <ss pp oo nn mm MM> OR <pp oo nn mm> */

  return print_insn_z80_buf (&buf, info);
}

static int
print_insn_z80_buf (struct buffer *buf, disassemble_info *info)
{
  const struct tab_elt *p;

  buf->n_fetch = 0;
  buf->n_used = 0;
  if (! fetch_data (buf, info, 1))
    return -1;

  p = (buf->inss & INSS_GBZ80) ? opc_main_gbz80 : opc_main;

  for (; p->val != (buf->data[0] & p->mask) || !mach_inst (buf, p); ++p)
    ;
  p->fp (buf, info, p->text);

  return buf->n_used;
}
