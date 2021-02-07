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

#include <math.h>
#include <string.h>

#include <Foundation/NSDebug.h>
#include <Foundation/NSString.h>

#include "blit.h"

/*
First attempt at gamma correction. Only used in text rendering (blit_*),
but that's where it's needed the most. The gamma adjustment is a large
hack, but the results are good.
*/
static unsigned char gamma_table[256],inv_gamma_table[256];


#define NPRE(r, pre) pre##_##r
#define M2PRE(a, b) NPRE(a, b)
#define MPRE(r) M2PRE(r, FORMAT_INSTANCE)


/*
For each supported pixel format we define a bunch of macros and include
ourself.
*/


/* 24-bit red green blue */
#define FORMAT_INSTANCE rgb
#define FORMAT_HOW DI_24_RGB

#define BLEND_TYPE unsigned char
#define BLEND_READ(p,nr,ng,nb) nr=p[0]; ng=p[1]; nb=p[2];
#define BLEND_READ_ALPHA(p,pa,nr,ng,nb,na) nr=p[0]; ng=p[1]; nb=p[2]; na=pa[0];
#define BLEND_WRITE(p,nr,ng,nb) p[0]=nr; p[1]=ng; p[2]=nb;
#define BLEND_WRITE_ALPHA(p,pa,nr,ng,nb,na) p[0]=nr; p[1]=ng; p[2]=nb; pa[0]=na;
#define BLEND_INC(p) p+=3;

#define ALPHA_READ(s,sa,d) d=sa[0];
#define ALPHA_INC(s,sa) s+=3; sa++;

#define COPY_TYPE unsigned char
#define COPY_TYPE_PIXEL(a) unsigned char a[3];
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v[0]=r; v[1]=g; v[2]=b;
#define COPY_WRITE(dst,v) dst[0]=v[0]; dst[1]=v[1]; dst[2]=v[2];
#define COPY_INC(dst) dst+=3;

#include "blit.m"

#undef FORMAT_INSTANCE


/* 24-bit blue green red */
#define FORMAT_INSTANCE bgr
#define FORMAT_HOW DI_24_BGR

#define BLEND_TYPE unsigned char
#define BLEND_READ(p,nr,ng,nb) nb=p[0]; ng=p[1]; nr=p[2];
#define BLEND_READ_ALPHA(p,pa,nr,ng,nb,na) nb=p[0]; ng=p[1]; nr=p[2]; na=pa[0];
#define BLEND_WRITE(p,nr,ng,nb) p[0]=nb; p[1]=ng; p[2]=nr;
#define BLEND_WRITE_ALPHA(p,pa,nr,ng,nb,na) p[0]=nb; p[1]=ng; p[2]=nr; pa[0]=na;
#define BLEND_INC(p) p+=3;

#define ALPHA_READ(s,sa,d) d=sa[0];
#define ALPHA_INC(s,sa) s+=3; sa++;

#define COPY_TYPE unsigned char
#define COPY_TYPE_PIXEL(a) unsigned char a[3];
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v[0]=b; v[1]=g; v[2]=r;
#define COPY_WRITE(dst,v) dst[0]=v[0]; dst[1]=v[1]; dst[2]=v[2];
#define COPY_INC(dst) dst+=3;

#include "blit.m"

#undef FORMAT_INSTANCE


/* 32-bit red green blue alpha */
#define FORMAT_INSTANCE rgba
#define FORMAT_HOW DI_32_RGBA

#define INLINE_ALPHA

#define BLEND_TYPE unsigned char
#define BLEND_READ(p,nr,ng,nb) nr=p[0]; ng=p[1]; nb=p[2];
#define BLEND_READ_ALPHA(p,pa,nr,ng,nb,na) nr=p[0]; ng=p[1]; nb=p[2]; na=p[3];
#define BLEND_WRITE(p,nr,ng,nb) p[0]=nr; p[1]=ng; p[2]=nb;
#define BLEND_WRITE_ALPHA(p,pa,nr,ng,nb,na) p[0]=nr; p[1]=ng; p[2]=nb; p[3]=na;
#define BLEND_INC(p) p+=4;

#define ALPHA_READ(s,sa,d) d=s[3];
#define ALPHA_INC(s,sa) s+=4;

#define COPY_TYPE unsigned int
#define COPY_TYPE_PIXEL(a) unsigned int a;
#if GS_WORDS_BIGENDIAN
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=(r<<24)|(g<<16)|(b<<8);
#define COPY_ASSEMBLE_PIXEL_ALPHA(v,r,g,b,a) v=(r<<24)|(g<<16)|(b<<8)|(a);
#else
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=(b<<16)|(g<<8)|(r<<0);
#define COPY_ASSEMBLE_PIXEL_ALPHA(v,r,g,b,a) v=(b<<16)|(g<<8)|(r<<0)|(a<<24);
#endif
#define COPY_WRITE(dst,v) dst[0]=v;
#define COPY_INC(dst) dst++;

#include "blit.m"

#undef FORMAT_INSTANCE


/* 32-bit blue green red alpha */
#define FORMAT_INSTANCE bgra
#define FORMAT_HOW DI_32_BGRA

#define INLINE_ALPHA

#define BLEND_TYPE unsigned char
#define BLEND_READ(p,nr,ng,nb) nb=p[0]; ng=p[1]; nr=p[2];
#define BLEND_READ_ALPHA(p,pa,nr,ng,nb,na) nb=p[0]; ng=p[1]; nr=p[2]; na=p[3];
#define BLEND_WRITE(p,nr,ng,nb) p[0]=nb; p[1]=ng; p[2]=nr;
#define BLEND_WRITE_ALPHA(p,pa,nr,ng,nb,na) p[0]=nb; p[1]=ng; p[2]=nr; p[3]=na;
#define BLEND_INC(p) p+=4;

#define ALPHA_READ(s,sa,d) d=s[3];
#define ALPHA_INC(s,sa) s+=4;

#define COPY_TYPE unsigned int
#define COPY_TYPE_PIXEL(a) unsigned int a;
#if GS_WORDS_BIGENDIAN
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=(b<<24)|(g<<16)|(r<<8);
#define COPY_ASSEMBLE_PIXEL_ALPHA(v,r,g,b,a) v=(b<<24)|(g<<16)|(r<<8)|(a);
#else
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=(r<<16)|(g<<8)|(b<<0);
#define COPY_ASSEMBLE_PIXEL_ALPHA(v,r,g,b,a) v=(r<<16)|(g<<8)|(b<<0)|(a<<24);
#endif
#define COPY_WRITE(dst,v) dst[0]=v;
#define COPY_INC(dst) dst++;

#include "blit.m"

#undef FORMAT_INSTANCE


/* 32-bit alpha red green blue */
#define FORMAT_INSTANCE argb
#define FORMAT_HOW DI_32_ARGB

#define INLINE_ALPHA

#define BLEND_TYPE unsigned char
#define BLEND_READ(p,nr,ng,nb) nr=p[1]; ng=p[2]; nb=p[3];
#define BLEND_READ_ALPHA(p,pa,nr,ng,nb,na) nr=p[1]; ng=p[2]; nb=p[3]; na=p[0];
#define BLEND_WRITE(p,nr,ng,nb) p[1]=nr; p[2]=ng; p[3]=nb;
#define BLEND_WRITE_ALPHA(p,pa,nr,ng,nb,na) p[1]=nr; p[2]=ng; p[3]=nb; p[0]=na;
#define BLEND_INC(p) p+=4;

#define ALPHA_READ(s,sa,d) d=s[0];
#define ALPHA_INC(s,sa) s+=4;

#define COPY_TYPE unsigned int
#define COPY_TYPE_PIXEL(a) unsigned int a;
#if GS_WORDS_BIGENDIAN
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=(r<<16)|(g<<8)|(b<<0);
#define COPY_ASSEMBLE_PIXEL_ALPHA(v,r,g,b,a) v=(r<<16)|(g<<8)|(b<<0)|(a<<24);
#else
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=(b<<24)|(g<<16)|(r<<8);
#define COPY_ASSEMBLE_PIXEL_ALPHA(v,r,g,b,a) v=(b<<24)|(g<<16)|(r<<8)|(a);
#endif
#define COPY_WRITE(dst,v) dst[0]=v;
#define COPY_INC(dst) dst++;

#include "blit.m"

#undef FORMAT_INSTANCE


/* 32-bit alpha blue green red */
#define FORMAT_INSTANCE abgr
#define FORMAT_HOW DI_32_ABGR

#define INLINE_ALPHA

#define BLEND_TYPE unsigned char
#define BLEND_READ(p,nr,ng,nb) nb=p[1]; ng=p[2]; nr=p[3];
#define BLEND_READ_ALPHA(p,pa,nr,ng,nb,na) nb=p[1]; ng=p[2]; nr=p[3]; na=p[0];
#define BLEND_WRITE(p,nr,ng,nb) p[1]=nb; p[2]=ng; p[3]=nr;
#define BLEND_WRITE_ALPHA(p,pa,nr,ng,nb,na) p[1]=nb; p[2]=ng; p[3]=nr; p[0]=na;
#define BLEND_INC(p) p+=4;

#define ALPHA_READ(s,sa,d) d=s[0];
#define ALPHA_INC(s,sa) s+=4;

#define COPY_TYPE unsigned int
#define COPY_TYPE_PIXEL(a) unsigned int a;
#if GS_WORDS_BIGENDIAN
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=(b<<16)|(g<<8)|(r<<0);
#define COPY_ASSEMBLE_PIXEL_ALPHA(v,r,g,b,a) v=(b<<16)|(g<<8)|(r<<0)|(a<<24);
#else
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=(r<<24)|(g<<16)|(b<<8);
#define COPY_ASSEMBLE_PIXEL_ALPHA(v,r,g,b,a) v=(r<<24)|(g<<16)|(b<<8)|(a);
#endif
#define COPY_WRITE(dst,v) dst[0]=v;
#define COPY_INC(dst) dst++;

#include "blit.m"

#undef FORMAT_INSTANCE


/* 16-bit  5 bits blue, 6 bits green, 5 bits red */
#define FORMAT_INSTANCE b5g6r5
#define FORMAT_HOW DI_16_B5_G6_R5

#define BLEND_TYPE unsigned short
#define BLEND_READ(p,nr,ng,nb) \
	{ \
		unsigned short _s=p[0]; \
		nr=(_s>>11)<<3; \
		ng=((_s>>5)<<2)&0xff; \
		nb=(_s<<3)&0xff; \
	}
#define BLEND_READ_ALPHA(p,pa,nr,ng,nb,na) \
	{ \
		unsigned short _s=p[0]; \
		nr=(_s>>11)<<3; \
		ng=((_s>>5)<<2)&0xff; \
		nb=(_s<<3)&0xff; \
		na=pa[0]; \
	}
#define BLEND_WRITE(p,nr,ng,nb) p[0]=((nr>>3)<<11)|((ng>>2)<<5)|(nb>>3);
#define BLEND_WRITE_ALPHA(p,pa,nr,ng,nb,na) p[0]=((nr>>3)<<11)|((ng>>2)<<5)|(nb>>3); pa[0]=na;
#define BLEND_INC(p) p++;

#define ALPHA_READ(s,sa,d) d=sa[0];
#define ALPHA_INC(s,sa) s++; sa++;

#define COPY_TYPE unsigned short
#define COPY_TYPE_PIXEL(a) unsigned short a;
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=((r>>3)<<11)|((g>>2)<<5)|(b>>3);
#define COPY_WRITE(dst,v) dst[0]=v;
#define COPY_INC(dst) dst++;

#include "blit.m"
#undef FORMAT_INSTANCE


/* 16-bit  5 bits blue, 5 bits green, 5 bits red */
#define FORMAT_INSTANCE b5g5r5a1
#define FORMAT_HOW DI_16_B5_G5_R5_A1

#define BLEND_TYPE unsigned short
#define BLEND_READ(p,nr,ng,nb) \
	{ \
		unsigned short _s=p[0]; \
		nr=(_s>>10)<<3; \
		ng=((_s>>5)<<3)&0xff; \
		nb=(_s<<3)&0xff; \
	}
#define BLEND_READ_ALPHA(p,pa,nr,ng,nb,na) \
	{ \
		unsigned short _s=p[0]; \
		nr=(_s>>10)<<3; \
		ng=((_s>>5)<<3)&0xff; \
		nb=(_s<<3)&0xff; \
		na=pa[0]; \
	}
#define BLEND_WRITE(p,nr,ng,nb) p[0]=((nr>>3)<<10)+((ng>>3)<<5)+(nb>>3);
#define BLEND_WRITE_ALPHA(p,pa,nr,ng,nb,na) p[0]=((nr>>3)<<10)+((ng>>3)<<5)+(nb>>3); pa[0]=na;
#define BLEND_INC(p) p++;

#define ALPHA_READ(s,sa,d) d=sa[0];
#define ALPHA_INC(s,sa) s++; sa++;

#define COPY_TYPE unsigned short
#define COPY_TYPE_PIXEL(a) unsigned short a;
#define COPY_ASSEMBLE_PIXEL(v,r,g,b) v=((r>>3)<<10)|((g>>3)<<5)|(b>>3);
#define COPY_WRITE(dst,v) dst[0]=v;
#define COPY_INC(dst) dst++;

#include "blit.m"
#undef FORMAT_INSTANCE

/* end of pixel formats */


static draw_info_t draw_infos[DI_NUM] = {

#define C(x) \
  NPRE(run_alpha,x), \
  NPRE(run_opaque,x), \
  NPRE(run_alpha_a,x), \
  NPRE(run_opaque_a,x), \
  NPRE(blit_alpha_opaque,x), \
  NPRE(blit_mono_opaque,x), \
  NPRE(blit_alpha,x), \
  NPRE(blit_mono,x), \
  NPRE(blit_alpha_a,x), \
  NPRE(blit_mono_a,x), \
  \
  NPRE(blit_subpixel,x), \
  \
  NPRE(read_pixels_o,x), \
  NPRE(read_pixels_a,x), \
  \
  NPRE(sover_aa,x), \
  NPRE(sover_ao,x), \
  NPRE(sin_aa,x), \
  NPRE(sin_oa,x), \
  NPRE(sout_aa,x), \
  NPRE(sout_oa,x), \
  NPRE(satop_aa,x), \
  NPRE(dover_aa,x), \
  NPRE(dover_oa,x), \
  NPRE(din_aa,x), \
  NPRE(dout_aa,x), \
  NPRE(datop_aa,x), \
  NPRE(xor_aa,x), \
  NPRE(plusl_aa,x), \
  NPRE(plusl_oa,x), \
  NPRE(plusl_ao_oo,x), \
  NPRE(plusl_ao_oo,x), \
  NPRE(plusd_aa,x), \
  NPRE(plusd_oa,x), \
  NPRE(plusd_ao_oo,x), \
  NPRE(plusd_ao_oo,x), \
  NPRE(dissolve_aa,x), \
  NPRE(dissolve_oa,x), \
  NPRE(dissolve_ao,x), \
  NPRE(dissolve_oo,x),

/* TODO: try to implement fallback versions? possible? */
{DI_FALLBACK       ,0, 0,0,-1,/*C(fallback)*/},

{DI_16_B5_G5_R5_A1 ,2,15,0,-1,C(b5g5r5a1)},
{DI_16_B5_G6_R5    ,2,16,0,-1,C(b5g6r5)},
{DI_24_RGB         ,3,24,0,-1,C(rgb)},
{DI_24_BGR         ,3,24,0,-1,C(bgr)},
/* ARTContext.m assumes that only 32-bit modes have inline alpha. this
might eventually need to be fixed */
{DI_32_RGBA        ,4,24,1, 3,C(rgba)},
{DI_32_BGRA        ,4,24,1, 3,C(bgra)},
{DI_32_ARGB        ,4,24,1, 0,C(argb)},
{DI_32_ABGR        ,4,24,1, 0,C(abgr)},
};



static int byte_ofs_of_mask(unsigned int m)
{
  union
    {
      unsigned char b[4];
      unsigned int m;
    } tmp;

  tmp.m = m;
  if (tmp.b[0] == 0xff && !tmp.b[1] && !tmp.b[2] && !tmp.b[3])
    return 0;
  else if (tmp.b[1] == 0xff && !tmp.b[0] && !tmp.b[2] && !tmp.b[3])
    return 1;
  else if (tmp.b[2] == 0xff && !tmp.b[0] && !tmp.b[1] && !tmp.b[3])
    return 2;
  else if (tmp.b[3] == 0xff && !tmp.b[0] && !tmp.b[1] && !tmp.b[2])
    return 3;
  else
    return -1;
}

void artcontext_setup_draw_info(draw_info_t *di,
	unsigned int red_mask, unsigned int green_mask, unsigned int blue_mask,
	int bpp)
{
  int t = DI_FALLBACK;

  NSDebugLLog(@"back-art", @"%s masks=(%08x %08x %08x) bpp=%i",
    __PRETTY_FUNCTION__, red_mask, green_mask, blue_mask, bpp);

  if (bpp == 16 && red_mask == 0xf800 && green_mask == 0x7e0 &&
      blue_mask == 0x1f)
    {
      t = DI_16_B5_G6_R5;
    }
  else if (bpp == 16 &&  red_mask == 0x7c00 && green_mask == 0x3e0 &&
	   blue_mask == 0x1f)
    {
      t = DI_16_B5_G5_R5_A1;
    }
  else if (bpp == 24 || bpp == 32)
    {
      int r, g, b;

      r = byte_ofs_of_mask(red_mask);
      g = byte_ofs_of_mask(green_mask);
      b = byte_ofs_of_mask(blue_mask);

      if (bpp == 24)
        {
          if (r == 0 && g == 1 && b == 2)
            t = DI_24_RGB;
          else if (r == 2 && g == 1 && b == 0)
            t = DI_24_BGR;
        }
      else if (bpp == 32)
        {
          if (r == 0 && g == 1 && b == 2)
            t = DI_32_RGBA;
          else if (r == 2 && g == 1 && b == 0)
            t = DI_32_BGRA;
          else if (r == 1 && g == 2 && b == 3)
            t = DI_32_ARGB;
          else if (r == 3 && g == 2 && b == 1)
            t = DI_32_ABGR;
        }
    }

  NSDebugLLog(@"back-art", @"got t=%i", t);

  *di = draw_infos[t];
  if (!di->render_run_alpha)
    *di = draw_infos[DI_FALLBACK];
  if (di->how == DI_FALLBACK)
    {
      NSLog(@"gnustep-back(art): Unrecognized color masks: %08x:%08x:%08x %i",
	    red_mask, green_mask, blue_mask, bpp);
      NSLog(@"Please report this along with details on your pixel format "
	    @"(ie. the four numbers above) to bug-gnustep@gnu.org."
	    @"Better: implement it and send a patch.)");
      exit(1);
    }
}

void artcontext_setup_gamma(float gamma)
{
  int i;
  
  if (!gamma)
    gamma = 1.4;

  NSDebugLLog(@"back-art",@"gamma=%g",gamma);

  gamma = 1.0 / gamma;
  
  for (i = 0; i < 256; i++)
    {
      gamma_table[i] = pow(i / 255.0, gamma) * 255 + .5;
      inv_gamma_table[i] = pow(i / 255.0, 1.0 / gamma) * 255 + .5;
    }
}



/*

compositing:                   source opaque/dest. opaque
                               00               01               10               11
Clear   0          0         +ab, clear
Copy    1          0           copy all         +ab, copy        copy             copy

Sover   1          1 - srcA    impl             impl             copy             copy
Sin     dstA       0           impl             +ab, copy        impl             copy
Sout    1 - dstA   0           impl             clear            impl             clear
Satop   dstA       1 - srcA    impl             Sover 01         Sin 10           copy

Dover   1 - dstA   1           impl             noop             impl             noop
Din     0          srcA        impl             +ab, 00          noop             noop
Dout    0          1 - srcA    impl             +ab, 00          clear            clear
Datop   1 - dstA   srcA        impl             +ab, 00          Dover 10         noop

Xor     1 - dstA   1 - srcA    impl             +ab, 00          Sout 10          clear

PlusL                          impl             impl             impl             impl
dst=dst+src, dsta=dsta+srca

PlusD                          impl             impl             impl             impl
dst=dst+src-1, dsta=dsta+srca


compositing (source transparent) dest. opaque
                               0                1
Clear   0          0           clear            clear
Copy    1          0           clear            clear

Sover   1          1 - srcA    noop             noop
Sin     dstA       0           clear            clear
Sout    1 - dstA   0           clear            clear
Satop   dstA       1 - srcA    noop             noop

Dover   1 - dstA   1           noop             noop
Din     0          srcA        clear            clear
Dout    0          1 - srcA    noop             noop
Datop   1 - dstA   srcA        clear            clear

Xor     1 - dstA   1 - srcA    noop             noop



PlusL    dst=src+dst  , clamp to 1.0; dsta=srca+dsta, clamp to 1.0
PlusD    dst=src+dst-1, clamp to 0.0; dsta=srca+dsta, clamp to 1.0

*/
