/*
   Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>

   This file is part of GNUstep.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

/*
TODO: rounding of alpha is wrong in many places, ie. an alpha of 255 is
treated as an alpha of 255/256 instead of 255/255. The
  if (alpha>127) alpha++;
hacks should take care of this with as much accuracy as we can get.
still need to fix all the remaining spots

2002-08-06: boundary cases should be correct now in most places, although
rounding is way off (but not more than 1, hopefully) for near-boundary
cases. at least pure black stays pure black and pure white stays pure white

2004-01-10: Test suite now says that all composite functions are 'correct',
in the sense that the difference from the correct result is never larger
than 0.5, and that's the best I can get with finite precision'. :)

(However, should probably check whether the results here always match the
correctly rounded correct result.)

TODO: (optional?) proper gamma handling?


TODO: more cpp magic to reduce the amount of code?
*/


/*
Define the different blitting functions.
Each blitter is defined once for each format
using the different helper macros (or specially optimized functions in
some cases).
*/


static void MPRE(blit_alpha_opaque) (unsigned char *adst,
	const unsigned char *asrc,
	unsigned char r, unsigned char g, unsigned char b, int num)
{
  const unsigned char *src = asrc;
  BLEND_TYPE *dst = (BLEND_TYPE *)adst;
  int nr, ng, nb, a;

  for (; num; num--, src++)
    {
      a = *src;
      if (!a)
	{
	  BLEND_INC(dst)
	  continue;
	}

      BLEND_READ(dst, nr, ng, nb)
      nr = inv_gamma_table[nr];
      ng = inv_gamma_table[ng];
      nb = inv_gamma_table[nb];
      nr = (r * a + nr * (255 - a) + 0xff) >> 8;
      ng = (g * a + ng * (255 - a) + 0xff) >> 8;
      nb = (b * a + nb * (255 - a) + 0xff) >> 8;
      nr = gamma_table[nr];
      ng = gamma_table[ng];
      nb = gamma_table[nb];
      BLEND_WRITE(dst, nr, ng, nb)
      BLEND_INC(dst)
    }
}

static void MPRE(blit_mono_opaque) (unsigned char *adst,
	const unsigned char *src, int src_ofs,
	unsigned char r, unsigned char g, unsigned char b,
	int num)
{
  COPY_TYPE *dst = (COPY_TYPE *)adst;
  COPY_TYPE_PIXEL(v)
  int i;
  unsigned char s;

  COPY_ASSEMBLE_PIXEL(v, r, g, b)

  s = *src++;
  i = src_ofs;
  while (src_ofs--) s <<= 1;

  for (; num; num--)
    {
      if (s&0x80)
	{
	  COPY_WRITE(dst, v)
	}
      COPY_INC(dst)
      i++;
      if (i == 8)
	{
	  s = *src++;
	  i = 0;
	}
      else
	s <<= 1;
    }
}

static void MPRE(blit_alpha) (unsigned char *adst, const unsigned char *asrc,
	unsigned char r, unsigned char g, unsigned char b, unsigned char alpha,
	int num)
{
  const unsigned char *src = asrc;
  BLEND_TYPE *dst = (BLEND_TYPE *)adst;
  int a, nr, ng, nb;

  if (alpha>127) alpha++;

  for (; num; num--, src++)
    {
      a = *src;
      if (!a)
	{
	  BLEND_INC(dst)
	  continue;
	}
      a *= alpha;
      BLEND_READ(dst, nr, ng, nb)
      nr = (r * a + nr * (65280 - a) + 0xff00) >> 16;
      ng = (g * a + ng * (65280 - a) + 0xff00) >> 16;
      nb = (b * a + nb * (65280 - a) + 0xff00) >> 16;
      BLEND_WRITE(dst, nr, ng, nb)
      BLEND_INC(dst)
    }
}

static void MPRE(blit_mono) (unsigned char *adst,
	const unsigned char *src, int src_ofs,
	unsigned char r, unsigned char g, unsigned char b, unsigned char alpha,
	int num)
{
  BLEND_TYPE *dst = (BLEND_TYPE *)adst;
  int i, nr, ng, nb;
  unsigned char s;
  int a;

  a = alpha;
  if (a>127) a++;

  s = *src++;
  i = src_ofs;
  while (src_ofs--) s <<= 1;

  for (; num; num--)
    {
      if (s&0x80)
	{
	  BLEND_READ(dst, nr, ng, nb)
	  nr = (r * a + nr * (255 - a) + 0xff) >> 8;
	  ng = (g * a + ng * (255 - a) + 0xff) >> 8;
	  nb = (b * a + nb * (255 - a) + 0xff) >> 8;
	  BLEND_WRITE(dst, nr, ng, nb)
	  BLEND_INC(dst)
	    }
      else
	{
	  BLEND_INC(dst)
	}
      i++;
      if (i == 8)
	{
	  s = *src++;
	  i = 0;
	}
      else
	s <<= 1;
    }
}


static void MPRE(blit_alpha_a) (unsigned char *adst, unsigned char *dsta,
	const unsigned char *asrc,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a_alpha,
	int num)
{
  const unsigned char *src = asrc;
  BLEND_TYPE *dst = (BLEND_TYPE *)adst;
  int a, nr, ng, nb, na;
  int alpha = a_alpha;

  if (alpha>127) alpha++;

  for (; num; num--, src++)
    {
      a = *src;
      if (!a)
	{
	  ALPHA_INC(dst, dsta)
	  continue;
	}
      a *= alpha;
      BLEND_READ_ALPHA(dst, dsta, nr, ng, nb, na)
      nr = (r * a + nr * (65280 - a) + 0xff00) >> 16;
      ng = (g * a + ng * (65280 - a) + 0xff00) >> 16;
      nb = (b * a + nb * (65280 - a) + 0xff00) >> 16;
      na = ((a << 8) + na * (65280 - a) + 0xff00) >> 16;
      BLEND_WRITE_ALPHA(dst, dsta, nr, ng, nb, na)
      ALPHA_INC(dst, dsta)
    }
}

static void MPRE(blit_mono_a) (unsigned char *adst, unsigned char *dsta,
	const unsigned char *src, int src_ofs,
	unsigned char r, unsigned char g, unsigned char b, unsigned char alpha,
	int num)
{
  BLEND_TYPE *dst = (BLEND_TYPE *)adst;
  int i, nr, ng, nb;
  unsigned char s;
  int a;

  a = alpha;
  if (a>127) a++;

  s = *src++;
  i = src_ofs;
  while (src_ofs--) s <<= 1;

  for (; num; num--)
    {
      if (s&0x80)
	{
	  BLEND_READ(dst, nr, ng, nb)
	  nr = (r * a + nr * (255 - a) + 0xff) >> 8;
	  ng = (g * a + ng * (255 - a) + 0xff) >> 8;
	  nb = (b * a + nb * (255 - a) + 0xff) >> 8;
	  BLEND_WRITE(dst, nr, ng, nb)
	  BLEND_INC(dst)
	}
      else
	{
	  BLEND_INC(dst)
	}
      i++;
      if (i == 8)
	{
	  s = *src++;
	  i = 0;
	}
      else
	s <<= 1;
    }
}


static void MPRE(blit_subpixel) (unsigned char *adst, const unsigned char *asrc,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a,
	int num)
{
  const unsigned char *src = asrc;
  BLEND_TYPE *dst = (BLEND_TYPE *)adst;
  unsigned int nr, ng, nb;
  unsigned int ar, ag, ab;
  int alpha = a;

  if (alpha>127) alpha++;

  for (; num; num--)
    {
      ar = *src++;
      ag = *src++;
      ab = *src++;

      BLEND_READ(dst, nr, ng, nb)

      nr = inv_gamma_table[nr];
      ng = inv_gamma_table[ng];
      nb = inv_gamma_table[nb];

      ar *= alpha;
      ag *= alpha;
      ab *= alpha;

      nr = (r * ar + nr * (65280 - ar) + 0xff00) >> 16;
      ng = (g * ag + ng * (65280 - ag) + 0xff00) >> 16;
      nb = (b * ab + nb * (65280 - ab) + 0xff00) >> 16;

      nr = gamma_table[nr];
      ng = gamma_table[ng];
      nb = gamma_table[nb];

      BLEND_WRITE(dst, nr, ng, nb)
      BLEND_INC(dst)
    }
}


static void MPRE(run_opaque) (render_run_t *ri, int num)
{
#if FORMAT_HOW == DI_16_B5_G5_R5_A1 || FORMAT_HOW == DI_16_B5_G6_R5
  unsigned int v;
  unsigned short *dst = (unsigned short *)ri->dst;

  COPY_ASSEMBLE_PIXEL(v, ri->r, ri->g, ri->b)
  v = v + (v << 16);
  if (((NSUInteger)dst&2) && num)
    {
      *dst++ = v;
      num--;
    }
  while (num >= 2)
    {
      *((unsigned int *)dst) = v;
      dst += 2;
      num -= 2;
    }
  if (num)
    *dst = v;
#else
  COPY_TYPE *dst = (COPY_TYPE *)ri->dst;
  COPY_TYPE_PIXEL(v)

#if FORMAT_HOW == DI_32_RGBA || FORMAT_HOW == DI_32_BGRA || \
    FORMAT_HOW == DI_32_ARGB || FORMAT_HOW == DI_32_ABGR
  if (ri->r == ri->g && ri->r == ri->b)
    {
      num *= 4;
      memset(dst, ri->r, num);
      return;
    }
#endif

  COPY_ASSEMBLE_PIXEL(v, ri->r, ri->g, ri->b)
  for (; num; num--)
    {
      COPY_WRITE(dst, v)
      COPY_INC(dst)
    }
#endif
}

static void MPRE(run_alpha) (render_run_t *ri, int num)
{
  BLEND_TYPE *dst = (BLEND_TYPE *)ri->dst;
  int nr, ng, nb;
  int r, g, b, a;
  a = ri->a;
  r = ri->r * a;
  g = ri->g * a;
  b = ri->b * a;
  a = 255 - a;
  for (; num; num--)
    {
      BLEND_READ(dst, nr, ng, nb)
      nr = (r + nr * a + 0xff) >> 8;
      ng = (g + ng * a + 0xff) >> 8;
      nb = (b + nb * a + 0xff) >> 8;
      BLEND_WRITE(dst, nr, ng, nb)
      BLEND_INC(dst)
    }
}


static void MPRE(run_alpha_a) (render_run_t *ri, int num)
{
  int nr, ng, nb, na;
  int sr, sg, sb, a;
  BLEND_TYPE *dst = (BLEND_TYPE *)ri->dst;
#ifndef INLINE_ALPHA
  unsigned char *dst_alpha = ri->dsta;
#endif

  a = ri->a;
  sr = ri->r * a;
  sg = ri->g * a;
  sb = ri->b * a;
  a = 255 - a;

  for (; num; num--)
    {
      BLEND_READ_ALPHA(dst, dst_alpha, nr, ng, nb, na)
      nr = (sr + nr * a + 0xff) >> 8;
      ng = (sg + ng * a + 0xff) >> 8;
      nb = (sb + nb * a + 0xff) >> 8;
      na = (na * a + 0xffff - (a << 8)) >> 8;
      BLEND_WRITE_ALPHA(dst, dst_alpha, nr, ng, nb, na)
      ALPHA_INC(dst, dst_alpha)
    }
}

static void MPRE(run_opaque_a) (render_run_t *ri, int num)
{
  COPY_TYPE *dst = (COPY_TYPE *)ri->dst;
  COPY_TYPE_PIXEL(v)
  int n;

#ifdef INLINE_ALPHA
  COPY_ASSEMBLE_PIXEL_ALPHA(v, ri->r, ri->g, ri->b, 0xff)
#else
  COPY_ASSEMBLE_PIXEL(v, ri->r, ri->g, ri->b)
#endif
  for (n = num; n; n--)
    {
      COPY_WRITE(dst, v)
      COPY_INC(dst)
    }
#ifndef INLINE_ALPHA
  memset(ri->dsta, 0xff, num);
#endif
}


static void MPRE(read_pixels_o) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src;
  unsigned char *dst = c->dst;
  int r, g, b;

  for (; num; num--)
    {
      BLEND_READ(s, r, g, b)
      BLEND_INC(s)
      dst[0] = r;
      dst[1] = g;
      dst[2] = b;
      dst[3] = 0xff;
      dst += 4;
    }
}

static void MPRE(read_pixels_a) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca;
#endif
  unsigned char *dst = c->dst;
  int r, g, b, a;

  for (; num; num--)
    {
      BLEND_READ_ALPHA(s, src_alpha, r, g, b, a)
      ALPHA_INC(s, src_alpha)
      dst[0] = r;
      dst[1] = g;
      dst[2] = b;
      dst[3] = a;
      dst += 4;
    }
}


/* 1 : 1 - srca */
static void MPRE(sover_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(s, src_alpha, sa)
      if (!sa)
	{
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (sa == 255)
	{
	  BLEND_READ(s, sr, sg, sb)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, 255)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ(s, sr, sg, sb)
      BLEND_READ_ALPHA(d, dst_alpha, dr, dg, db, da)

      da = sa + ((da * (255 - sa) + 0xff) >> 8);
      sa = 255 - sa;
      dr = sr + ((dr * sa + 0xff) >> 8);
      dg = sg + ((dg * sa + 0xff) >> 8);
      db = sb + ((db * sa + 0xff) >> 8);

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}

static void MPRE(sover_ao) (composite_run_t *c, int num)
{
#if FORMAT_HOW == DI_16_B5_G6_R5 || FORMAT_HOW == DI_16_B5_G5_R5_A1
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
  unsigned char *src_alpha = c->srca;
  int sr, sg, sb, sa;
#undef i386
#ifndef i386
  int dr, dg, db;
#endif

  unsigned int temp;

  for (; num; num--)
    {
      ALPHA_READ(s, src_alpha, sa)
      if (!sa)
	{
	  ALPHA_INC(s, src_alpha)
	  BLEND_INC(d)
	  continue;
	}
      if (sa == 255)
	{
	  BLEND_READ(s, sr, sg, sb)
	  BLEND_WRITE(d, sr, sg, sb)
	  ALPHA_INC(s, src_alpha)
	  BLEND_INC(d)
	  continue;
	}


#ifdef i386
      /*
      The basic idea here is to scale all components using one multiply,
      and to do so without losing any accuracy. To do this, we move the
      components around so we get 8 empty bits above each component. In
      a 32-bit word, we don't have enough space for this, but by moving
      green to the top 6 bits and using ix86 'mul', we'll get the top 32
      bits of the multiplication in another register and can later recombine
      the components relatively easily.

      Mostly equivalent c (16bpp case):

      unsigned long long int temp;

      sa = 255-sa;
      
      temp = d[0];
      temp = ((temp|(temp<<21))&0xfc00001f)|((temp&0xf800)<<2);
      temp = temp*sa;
      temp = temp+0x000000020003e01fLL;
      temp = temp>>8;
      temp = (temp&0x1f) | ((temp&0x3e000)>>2) | ((temp&0xfc000000)>>21);
      d[0]=temp + s[0];


16bpp:
original:                                 0000 0000 0000 0000  bbbb bggg gggr rrrr
after unpacking:                          gggg gg00 0000 00bb  bbb0 0000 000r rrrr
after 'mul':   ....  0000 0000 gggg gggg  gggg ggbb bbbb bbbb  bbbr rrrr rrrr rrrr

15bpp:
original:                                 0000 0000 0000 0000  0bbb bbgg gggr rrrr
after unpacking:                          gggg g000 0000 00bb  bbb0 0000 000r rrrr
after 'mul':   ....  0000 0000 gggg gggg  gggg g0bb bbbb bbbb  bbbr rrrr rrrr rrrr

      */
      temp = d[0];
      sa = 255 - sa;
      asm (
	"movl %0,%%eax\n"
#if FORMAT_HOW == DI_16_B5_G6_R5
        "shll $21,%%eax\n"
#else
        "shll $22,%%eax\n"
#endif
	"orl %0,%%eax\n"
#if FORMAT_HOW == DI_16_B5_G6_R5
	"andl $0xfc00001f,%%eax\n"
	"andl $0xf800,%0\n"
	"shll $2,%0\n"
#else
	"andl $0xf800001f,%%eax\n"
	"andl $0x7c00,%0\n"
	"shll $3,%0\n"
#endif
	"orl %0,%%eax\n"
	"mul %2\n"
	"addl $0x0003e01f,%%eax\n"
#if FORMAT_HOW == DI_16_B5_G6_R5
	"addl $0x02,%%edx\n"
	"andl $0xfc,%%edx\n"
	"shll $3,%%edx\n"
#else
	"addl $0x01,%%edx\n"
	"andl $0xf8,%%edx\n"
	"shll $2,%%edx\n"
#endif
	"shrl $8,%%eax\n"
	"movl %%eax,%0\n"
	"andl $0x1f,%0\n"
	"orl %%edx,%0\n"
#if FORMAT_HOW == DI_16_B5_G6_R5
	"shrl $2,%%eax\n"
	"andl $0xf800,%%eax\n"
#else
	"shrl $3,%%eax\n"
	"andl $0x7c00,%%eax\n"
#endif
	"orl %%eax,%0\n"
	: "=r" (temp)
	: "0" (temp), "g" (sa)
	: "eax", "edx");
      d[0] = temp + s[0];
#else
      /*
      Generic, non-ix86 code. Can't use the really optimized path, but
      we can still add in the entire source pixel instead of unpacking
      it.
      */
      BLEND_READ(d, dr, dg, db)

      sa = 255 - sa;
      dr = ((dr * sa + 0xff) >> 8);
      dg = ((dg * sa + 0xff) >> 8);
      db = ((db * sa + 0xff) >> 8);

      COPY_ASSEMBLE_PIXEL(temp, dr, dg, db)
  
      d[0] = temp + s[0];
#endif

      ALPHA_INC(s, src_alpha)
      BLEND_INC(d)
    }
#else
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca;
#endif
  int sr, sg, sb, sa, dr, dg, db;

  for (; num; num--)
    {
      ALPHA_READ(s, src_alpha, sa)
      if (!sa)
	{
	  ALPHA_INC(s, src_alpha)
	  BLEND_INC(d)
	  continue;
	}
      if (sa == 255)
	{
	  BLEND_READ(s, sr, sg, sb)
	  BLEND_WRITE(d, sr, sg, sb)
	  ALPHA_INC(s, src_alpha)
	  BLEND_INC(d)
	  continue;
	}

      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      sa = 255 - sa;
      dr = sr + ((dr * sa + 0xff) >> 8);
      dg = sg + ((dg * sa + 0xff) >> 8);
      db = sb + ((db * sa + 0xff) >> 8);

      BLEND_WRITE(d, dr, dg, db)

      ALPHA_INC(s, src_alpha)
      BLEND_INC(d)
    }
#endif
}

/* dsta : 0 */
static void MPRE(sin_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(d, dst_alpha, da)
      if (!da)
	{
	  BLEND_WRITE_ALPHA(d, dst_alpha, 0, 0, 0, 0)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (da == 255)
	{
	  BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, sa)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)

      dr = (sr * da + 0xff) >> 8;
      dg = (sg * da + 0xff) >> 8;
      db = (sb * da + 0xff) >> 8;
      da = (sa * da + 0xff) >> 8;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}

static void MPRE(sin_oa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(d, dst_alpha, da)
      if (!da)
	{
	  BLEND_WRITE_ALPHA(d, dst_alpha, 0, 0, 0, 0)
	  BLEND_INC(s)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (da == 255)
	{
	  BLEND_READ(s, sr, sg, sb)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, 255)
	  BLEND_INC(s)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ(s, sr, sg, sb)

      dr = (sr * da + 0xff) >> 8;
      dg = (sg * da + 0xff) >> 8;
      db = (sb * da + 0xff) >> 8;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      BLEND_INC(s)
      ALPHA_INC(d, dst_alpha)
    }
}

/* 1 - dsta : 0 */
static void MPRE(sout_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(d, dst_alpha, da)
      da = 255 - da;
      if (!da)
	{
	  BLEND_WRITE_ALPHA(d, dst_alpha, 0, 0, 0, 0)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (da == 255)
	{
	  BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, sa)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)

      dr = (sr * da + 0xff) >> 8;
      dg = (sg * da + 0xff) >> 8;
      db = (sb * da + 0xff) >> 8;
      da = (sa * da + 0xff) >> 8;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}

static void MPRE(sout_oa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(d, dst_alpha, da)
      da = 255 - da;
      if (!da)
	{
	  BLEND_WRITE_ALPHA(d, dst_alpha, 0, 0, 0, 0)
	  BLEND_INC(s)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (da == 255)
	{
	  BLEND_READ(s, sr, sg, sb)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, 255)
	  BLEND_INC(s)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ(s, sr, sg, sb)

      dr = (sr * da + 0x80) >> 8;
      dg = (sg * da + 0x80) >> 8;
      db = (sb * da + 0x80) >> 8;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      BLEND_INC(s)
      ALPHA_INC(d, dst_alpha)
    }
}

/* dsta : 1 - srca */

/*

0 0   0 1  noop
1 0   0 0  clear, noop
0 1   1 1  noop
1 1   1 0  copy

*/

static void MPRE(satop_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(d, dst_alpha, da)
      ALPHA_READ(s, src_alpha, sa)
      if (!da || (da == 255 && !sa))
	{
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (da == 255 && sa == 255)
	{
	  BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, sa)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      sa = 255 - sa;

      dr = (sr * da + dr * sa + 0xff) >> 8;
      dg = (sg * da + dg * sa + 0xff) >> 8;
      db = (sb * da + db * sa + 0xff) >> 8;
      /* For alpha, satop simplifies to da' = da. */

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}

/* 1 - dsta : 1 */
static void MPRE(dover_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(d, dst_alpha, da)
      if (da == 255)
	{
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (!da)
	{
	  BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, sa)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
      BLEND_READ(d, dr, dg, db)

      sa = da + ((sa * (255 - da) + 0x80) >> 8);
      da = 255 - da;
      dr = dr + ((sr * da + 0x80) >> 8);
      dg = dg + ((sg * da + 0x80) >> 8);
      db = db + ((sb * da + 0x80) >> 8);

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, sa)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}

static void MPRE(dover_oa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(d, dst_alpha, da)
      if (da == 255)
	{
	  BLEND_INC(s)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (!da)
	{
	  BLEND_READ(s, sr, sg, sb)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, 255)
	  BLEND_INC(s)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      da = 255 - da;
      dr = dr + ((sr * da + 0x80) >> 8);
      dg = dg + ((sg * da + 0x80) >> 8);
      db = db + ((sb * da + 0x80) >> 8);

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, 255)

      BLEND_INC(s)
      ALPHA_INC(d, dst_alpha)
    }
}

/* 0 : srca */
static void MPRE(din_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(s, src_alpha, sa)
      if (!sa)
	{
	  BLEND_WRITE_ALPHA(d, dst_alpha, 0, 0, 0, 0)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (sa == 255)
	{
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ_ALPHA(d, dst_alpha, dr, dg, db, da)

      dr = (dr * sa + 0x80) >> 8;
      dg = (dg * sa + 0x80) >> 8;
      db = (db * sa + 0x80) >> 8;
      da = (da * sa + 0x80) >> 8;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}

/* 0 : 1 - srca */
static void MPRE(dout_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(s, src_alpha, sa)
      sa = 255 - sa;
      if (!sa)
	{
	  BLEND_WRITE_ALPHA(d, dst_alpha, 0, 0, 0, 0)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (sa == 255)
	{
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	    continue;
	}

      BLEND_READ_ALPHA(d, dst_alpha, dr, dg, db, da)

      dr = (dr * sa + 0x80) >> 8;
      dg = (dg * sa + 0x80) >> 8;
      db = (db * sa + 0x80) >> 8;
      da = (da * sa + 0x80) >> 8;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}

/* 1 - dstA : srcA */

/*

0 0   1 0 clear, noop
1 0   1 1 copy
0 1   0 0 clear
1 1   0 1 noop

*/

static void MPRE(datop_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(d, dst_alpha, da)
      ALPHA_READ(s, src_alpha, sa)
      if ((!da && !sa) || (da == 255 && sa == 255))
	{
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (da == 255 && !sa)
	{
	  BLEND_WRITE_ALPHA(d, dst_alpha, 0, 0, 0, 0)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (sa == 255 && !da)
	{
	  BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, sa)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      da = 255 - da;

      dr = (dr * sa + sr * da + 0xff) >> 8;
      dg = (dg * sa + sg * da + 0xff) >> 8;
      db = (db * sa + sb * da + 0xff) >> 8;
      /* For alpha, datop simplifies to da' = sa. */
      da = sa;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}

/* 1 - dsta : 1 - srca */

/*

0 0  1 1 clear, noop
1 0  0 1 noop
0 1  1 0 copy
1 1  0 0 clear

*/

static void MPRE(xor_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      ALPHA_READ(s, src_alpha, sa)
      ALPHA_READ(d, dst_alpha, da)
      if (!sa && (da == 255 || !da))
	{
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (sa == 255 && !da)
	{
	  BLEND_READ(s, sr, sg, sb)
	  BLEND_WRITE_ALPHA(d, dst_alpha, sr, sg, sb, sa)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}
      if (da == 255 && sa == 255)
	{
	  BLEND_WRITE_ALPHA(d, dst_alpha, 0, 0, 0, 0)
	  ALPHA_INC(s, src_alpha)
	  ALPHA_INC(d, dst_alpha)
	  continue;
	}

      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      da = 255 - da;
      sa = 255 - sa;
      dr = ((dr * sa + sr * da + 0xff) >> 8);
      dg = ((dg * sa + sg * da + 0xff) >> 8);
      db = ((db * sa + sb * da + 0xff) >> 8);
      da = ((da * (255 - sa) + sa * (255 - da) + 0xff) >> 8);

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}


static void MPRE(plusl_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
      BLEND_READ_ALPHA(d, dst_alpha, dr, dg, db, da)

      dr += sr; if (dr>255) dr = 255;
      dg += sg; if (dg>255) dg = 255;
      db += sb; if (db>255) db = 255;
      da += sa; if (da>255) da = 255;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}
static void MPRE(plusl_oa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, dr, dg, db;

  for (; num; num--)
    {
      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      dr += sr; if (dr>255) dr = 255;
      dg += sg; if (dg>255) dg = 255;
      db += sb; if (db>255) db = 255;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, 0xff)

      BLEND_INC(s)
      ALPHA_INC(d, dst_alpha)
    }
}
static void MPRE(plusl_ao_oo) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
  int sr, sg, sb, dr, dg, db;

  for (; num; num--)
    {
      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      dr += sr; if (dr>255) dr = 255;
      dg += sg; if (dg>255) dg = 255;
      db += sb; if (db>255) db = 255;

      BLEND_WRITE(d, dr, dg, db)

      BLEND_INC(s)
      BLEND_INC(d)
    }
}


static void MPRE(plusd_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;

  for (; num; num--)
    {
      BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
      BLEND_READ_ALPHA(d, dst_alpha, dr, dg, db, da)

      dr += sr - 255; if (dr<0) dr = 0;
      dg += sg - 255; if (dg<0) dg = 0;
      db += sb - 255; if (db<0) db = 0;
      da += sa;       if (da>255) da = 255;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}
static void MPRE(plusd_oa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, dr, dg, db;

  for (; num; num--)
    {
      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      dr += sr - 255; if (dr<0) dr = 0;
      dg += sg - 255; if (dg<0) dg = 0;
      db += sb - 255; if (db<0) db = 0;

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, 0xff)

      BLEND_INC(s)
      ALPHA_INC(d, dst_alpha)
    }
}
static void MPRE(plusd_ao_oo) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
  int sr, sg, sb, dr, dg, db;

  for (; num; num--)
    {
      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      dr += sr - 255; if (dr<0) dr = 0;
      dg += sg - 255; if (dg<0) dg = 0;
      db += sb - 255; if (db<0) db = 0;

      BLEND_WRITE(d, dr, dg, db)

      BLEND_INC(s)
      BLEND_INC(d)
    }
}


static void MPRE(dissolve_aa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca,
    *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;
  int fraction = c->fraction;

  for (; num; num--)
    {
      BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
      BLEND_READ_ALPHA(d, dst_alpha, dr, dg, db, da)

      sr = (sr * fraction + 0xff) >> 8;
      sg = (sg * fraction + 0xff) >> 8;
      sb = (sb * fraction + 0xff) >> 8;
      sa = (sa * fraction + 0xff) >> 8;

      da = sa + ((da * (255 - sa) + 0xff) >> 8);
      sa = 255 - sa;
      dr = sr + ((dr * sa + 0xff) >> 8);
      dg = sg + ((dg * sa + 0xff) >> 8);
      db = sb + ((db * sa + 0xff) >> 8);

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      ALPHA_INC(s, src_alpha)
      ALPHA_INC(d, dst_alpha)
    }
}
static void MPRE(dissolve_ao) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *src_alpha = c->srca;
#endif
  int sr, sg, sb, sa, dr, dg, db;
  int fraction = c->fraction;

  for (; num; num--)
    {
      BLEND_READ_ALPHA(s, src_alpha, sr, sg, sb, sa)
      BLEND_READ(d, dr, dg, db)

      sr = (sr * fraction + 0xff) >> 8;
      sg = (sg * fraction + 0xff) >> 8;
      sb = (sb * fraction + 0xff) >> 8;
      sa = (sa * fraction + 0xff) >> 8;

      sa = 255 - sa;
      dr = sr + ((dr * sa + 0xff) >> 8);
      dg = sg + ((dg * sa + 0xff) >> 8);
      db = sb + ((db * sa + 0xff) >> 8);

      BLEND_WRITE(d, dr, dg, db)

      ALPHA_INC(s, src_alpha)
      BLEND_INC(d)
    }
}
static void MPRE(dissolve_oa) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
#ifndef INLINE_ALPHA
  unsigned char *dst_alpha = c->dsta;
#endif
  int sr, sg, sb, sa, dr, dg, db, da;
  int fraction = c->fraction;

  for (; num; num--)
    {
      BLEND_READ(s, sr, sg, sb)
      BLEND_READ_ALPHA(d, dst_alpha, dr, dg, db, da)

      sr = (sr * fraction + 0xff) >> 8;
      sg = (sg * fraction + 0xff) >> 8;
      sb = (sb * fraction + 0xff) >> 8;
      sa = fraction;

      da = sa + ((da * (255 - sa) + 0xff) >> 8);
      sa = 255 - sa;
      dr = sr + ((dr * sa + 0xff) >> 8);
      dg = sg + ((dg * sa + 0xff) >> 8);
      db = sb + ((db * sa + 0xff) >> 8);

      BLEND_WRITE_ALPHA(d, dst_alpha, dr, dg, db, da)

      BLEND_INC(s)
      ALPHA_INC(d, dst_alpha)
    }
}
static void MPRE(dissolve_oo) (composite_run_t *c, int num)
{
  BLEND_TYPE *s = (BLEND_TYPE *)c->src, *d = (BLEND_TYPE *)c->dst;
  int sr, sg, sb, sa, dr, dg, db;
  int fraction = c->fraction;

  for (; num; num--)
    {
      BLEND_READ(s, sr, sg, sb)
      BLEND_READ(d, dr, dg, db)

      sr = (sr * fraction + 0xff) >> 8;
      sg = (sg * fraction + 0xff) >> 8;
      sb = (sb * fraction + 0xff) >> 8;
      sa = fraction;

      sa = 255 - sa;
      dr = sr + ((dr * sa + 0xff) >> 8);
      dg = sg + ((dg * sa + 0xff) >> 8);
      db = sb + ((db * sa + 0xff) >> 8);

      BLEND_WRITE(d, dr, dg, db)

      BLEND_INC(s)
      BLEND_INC(d)
    }
}


#undef I_NAME
#undef BLEND_TYPE
#undef BLEND_READ
#undef BLEND_READ_ALPHA
#undef BLEND_WRITE
#undef BLEND_WRITE_ALPHA
#undef BLEND_INC
#undef ALPHA_READ
#undef ALPHA_INC
#undef COPY_TYPE
#undef COPY_TYPE_PIXEL
#undef COPY_ASSEMBLE_PIXEL
#undef COPY_ASSEMBLE_PIXEL_ALPHA
#undef COPY_WRITE
#undef COPY_INC
#undef FORMAT_HOW
#undef INLINE_ALPHA


