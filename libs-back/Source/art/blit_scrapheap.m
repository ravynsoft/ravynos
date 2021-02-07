/*
   Copyright (C) 2004 Free Software Foundation, Inc.

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
This file is never compiled. It just contains alternate and/or partial
implementations of some functions in blit.m that are interesting but not
(in their current form) better than the implementations in blit.m .
*/

/**** blit_alpha_opaque for 16/15 bpp ****/

/*
Do two pixels at once. Turned out to be slower than a straight loop.
*/

  const unsigned char *src = asrc;
  BLEND_TYPE *dst = (BLEND_TYPE *)adst;
  unsigned int nr, ng, nb, a;

/*

                      bbbb bggg gggr rrrr a0
                    b bbbb gggg ggrr rrr  a1
                   bb bbbg gggg grrr rr   a2
                  bbb bbgg gggg rrrr r    a3
                 bbbb bggg gggr rrrr      a4
               b bbbb gggg ggrr rrr       a5
              bb bbbg gggg grrr rr        a6
             bbb bbgg gggg rrrr r         a7

                         R rrrr rrrr rrrr
                  Ggg gggg gggg ggg
            Bbbb bbbb bbbb b

                                         bbbb b000 0000 0000 bbbb b000 0000 0000  a0
                                      b  bbbb 0000 0000 000b bbbb 0000 0000 000   a1
                                     bb  bbb0 0000 0000 00bb bbb0 0000 0000 00    a2
                                    bbb  bb00 0000 0000 0bbb bb00 0000 0000 0     a3
                                   bbbb  b000 0000 0000 bbbb b000 0000 0000       a4
                                 b bbbb  0000 0000 000b bbbb 0000 0000 000        a5
                                bb bbb0  0000 0000 00bb bbb0 0000 0000 00         a6
                               bbb bb00  0000 0000 0bbb bb00 0000 0000 0          a7
          bbbb b000 0000 0000 bbbb b000  0000 0000                                b0
        b bbbb 0000 0000 000b bbbb 0000  0000 000                                 b1
       bb bbb0 0000 0000 00bb bbb0 0000  0000 00                                  b2
      bbb bb00 0000 0000 0bbb bb00 0000  0000 0                                   b3
     bbbb b000 0000 0000 bbbb b000 0000  0000                                     b4
   b bbbb 0000 0000 000b bbbb 0000 0000  000                                      b5
  bb bbb0 0000 0000 00bb bbb0 0000 0000  00                                       b6
 bbb bb00 0000 0000 0bbb bb00 0000 0000  0                                        b7
BBBB BBBB BBBB B000 XXXX XXXX XXXX XXXX  XXXX X000 AAAA AAAA AAAA A000 0000 0000


bbbb bbbb 0000 0000 0000 0000 aaaa aaaa

*/

//printf("call with color=%02x %02x %02x\n",r,g,b);
  while (num >= 2)
    {
      unsigned int v1,v2;
      unsigned int a1;
      unsigned int temp;

      a = *((unsigned short *)src);
//      printf("alpha=%04x\n",a);
      a = a&0xffff;
/*      if (!a)
	{
	  num -= 2;
	  src += 2;
	  dst += 2;
	  continue;
	}*/
      a = (a|(a<<8))&0xff00ff;
//      printf("unpack to %08x\n",a);

      v1 = b*a;
      v1 = (v1>>11)&0x001f001f;
//      printf("blue: %08x\n",v1);

      v2 = g*a;
      v2 = (v2>> 5)&0x07e007e0;
//      printf("green: %08x\n",v2);
      v1 = v1|v2;

      v2 = r*a;
      v2 =  v2     &0xf800f800;
//      printf("red: %08x\n",v2);
      v1 = v1|v2;
//      printf("result: %08x\n",v1);

      a = 0xff00ff - a;

      temp = dst[0];
//      printf("p1: %04x\n",temp);
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
	: "0" (temp), "c" (a&0xff)
	: "eax", "edx");
//      printf("to: %04x\n",temp);
      v1 += temp;
//      printf("add in gives: %08x\n",v1);

      temp = dst[1];
//      printf("p2: %04x\n",temp);
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
	: "0" (temp), "c" (a>>16)
	: "eax", "edx");
//      printf("to: %04x\n",temp);
      v1 += (temp)<<16;
//      printf("add in gives: %08x\n",v1);

      *((unsigned int *)dst)=v1;

      num -= 2;
      dst += 2;
      src += 2;
    }

  if (num)
    {
      a = *src;
//      if (a)
	{
	  BLEND_READ(dst, nr, ng, nb)
/*	  nr = inv_gamma_table[nr];
	  ng = inv_gamma_table[ng];
	  nb = inv_gamma_table[nb];*/
	  nr = (r * a + nr * (255 - a) + 0xff) >> 8;
	  ng = (g * a + ng * (255 - a) + 0xff) >> 8;
	  nb = (b * a + nb * (255 - a) + 0xff) >> 8;
/*	  nr = gamma_table[nr];
	  ng = gamma_table[ng];
	  nb = gamma_table[nb];*/
	  BLEND_WRITE(dst, nr, ng, nb)
	  BLEND_INC(dst)
	}
    }

/**** blit_alpha_opaque for 16/15 bpp ****/

/* use mmx. didn't help */

#if 0 && FORMAT_HOW == DI_32_BGRA
static unsigned char constants[24]={0,0,0,0,0,0,0,0, 255,0,255,0,255,0,255,0, 0,0,0,0,0,0,0,0};
  constants[16]=b;
  constants[18]=g;
  constants[20]=r;
  asm volatile (
/*  "  movq (%%eax), %%mm0\n"  // hoistable
  "  movq 8(%%eax), %%mm1\n"*/ // hoistable
  "  movq 16(%%eax), %%mm2\n"
  "1:\n"

                          "movzbl (%%edi), %%eax\n"
  "testl %%eax,%%eax\n"
  "jz 2f\n"
                                                                              "movd (%%esi), %%mm4\n"
                          "movl %%eax, %%edx\n"
                          "shll $8, %%edx\n"
                          "orl %%edx, %%eax\n"
                                                                              "punpcklbw %%mm0, %%mm4\n"
                                                        "movq %%mm1, %%mm5\n"
                          "shll $8, %%edx\n"
                          "orl %%edx, %%eax\n"
                          "movd %%eax, %%mm3\n"
                          "punpcklbw %%mm0, %%mm3\n"
                                          "psubw %%mm3, %%mm5\n"
                          "pmullw %%mm2, %%mm3\n"
                                                               "pmullw %%mm5, %%mm4\n"
                                            "paddw %%mm3, %%mm4\n"
                                            "paddw %%mm1, %%mm4\n"
                                            "psrlw $8,%%mm4\n"
                                            "packuswb %%mm0,%%mm4\n"
                                            "movd %%mm4,(%%esi)\n"
  "2:\n"
  "incl %%edi\n"
  "addl $4, %%esi\n"
  "decl %%ecx\n"
  "jnz 1b\n"
//  "  emms\n" // hoistable
  :
  : "a" (&constants), "S" (adst), "D" (asrc), "c" (num));
#else

/**** ****/

