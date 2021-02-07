/*
   Copyright (C) 2002 Free Software Foundation, Inc.

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

#ifndef blit_h
#define blit_h


/** Information about how we draw stuff **/


typedef struct render_run_s
{
  unsigned char r, g, b, a;
  unsigned char *dst, *dsta;
} render_run_t;


typedef struct
{
  unsigned char *dst, *dsta;
  unsigned char *src, *srca;
  unsigned char fraction; /* for dissolving */
} composite_run_t;


typedef struct draw_info_s
{
  int how;
#define DI_FALLBACK              0

/* counting from lsb */
#define DI_16_B5_G5_R5_A1        1
#define DI_16_B5_G6_R5           2

#define DI_24_RGB                3
#define DI_24_BGR                4
#define DI_32_RGBA               5
#define DI_32_BGRA               6
#define DI_32_ARGB               7
#define DI_32_ABGR               8

#define DI_NUM                   9

  int bytes_per_pixel;
  int drawing_depth;
  int inline_alpha, inline_alpha_ofs;


  void (*render_run_alpha)(render_run_t *ri, int num);
  void (*render_run_opaque)(render_run_t *ri, int num);
  void (*render_run_alpha_a)(render_run_t *ri, int num);
  void (*render_run_opaque_a)(render_run_t *ri, int num);

  void (*render_blit_alpha_opaque)(unsigned char *dst,
				   const unsigned char *src,
				   unsigned char r, unsigned char g,
				   unsigned char b, int num);
  void (*render_blit_mono_opaque)(unsigned char *dst,
				  const unsigned char *src, int src_ofs,
				  unsigned char r, unsigned char g,
				  unsigned char b, int num);

  void (*render_blit_alpha)(unsigned char *dst, const unsigned char *src,
			    unsigned char r, unsigned char g, unsigned char b,
			    unsigned char alpha, int num);
  void (*render_blit_mono)(unsigned char *dst,
			   const unsigned char *src, int src_ofs,
			   unsigned char r, unsigned char g, unsigned char b,
			   unsigned char alpha, int num);

  void (*render_blit_alpha_a)(unsigned char *dst, unsigned char *dsta, const unsigned char *src,
			      unsigned char r, unsigned char g, unsigned char b,
			      unsigned char alpha, int num);
  void (*render_blit_mono_a)(unsigned char *dst, unsigned char *dsta,
			     const unsigned char *src, int src_ofs,
			     unsigned char r, unsigned char g, unsigned char b,
			     unsigned char alpha, int num);

  void (*render_blit_subpixel)(unsigned char *dst, const unsigned char *src,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a,
	int num);


  /* dst should be a 32bpp RGBA buffer. */
  void (*read_pixels_o)(composite_run_t *c, int num);
  void (*read_pixels_a)(composite_run_t *c, int num);


  void (*composite_sover_aa)(composite_run_t *c, int num);
  void (*composite_sover_ao)(composite_run_t *c, int num);

  void (*composite_sin_aa)(composite_run_t *c, int num);
  void (*composite_sin_oa)(composite_run_t *c, int num);

  void (*composite_sout_aa)(composite_run_t *c, int num);
  void (*composite_sout_oa)(composite_run_t *c, int num);

  void (*composite_satop_aa)(composite_run_t *c, int num);

  void (*composite_dover_aa)(composite_run_t *c, int num);
  void (*composite_dover_oa)(composite_run_t *c, int num);

  void (*composite_din_aa)(composite_run_t *c, int num);

  void (*composite_dout_aa)(composite_run_t *c, int num);

  void (*composite_datop_aa)(composite_run_t *c, int num);

  void (*composite_xor_aa)(composite_run_t *c, int num);

  void (*composite_plusl_aa)(composite_run_t *c, int num);
  void (*composite_plusl_oa)(composite_run_t *c, int num);
  void (*composite_plusl_ao)(composite_run_t *c, int num);
  void (*composite_plusl_oo)(composite_run_t *c, int num);

  void (*composite_plusd_aa)(composite_run_t *c, int num);
  void (*composite_plusd_oa)(composite_run_t *c, int num);
  void (*composite_plusd_ao)(composite_run_t *c, int num);
  void (*composite_plusd_oo)(composite_run_t *c, int num);

  void (*dissolve_aa)(composite_run_t *c, int num);
  void (*dissolve_oa)(composite_run_t *c, int num);
  void (*dissolve_ao)(composite_run_t *c, int num);
  void (*dissolve_oo)(composite_run_t *c, int num);
} draw_info_t;

#define RENDER_RUN_ALPHA (DI.render_run_alpha)
#define RENDER_RUN_OPAQUE (DI.render_run_opaque)
#define RENDER_RUN_ALPHA_A (DI.render_run_alpha_a)
#define RENDER_RUN_OPAQUE_A (DI.render_run_opaque_a)

#define RENDER_BLIT_ALPHA_OPAQUE (DI.render_blit_alpha_opaque)
#define RENDER_BLIT_MONO_OPAQUE DI.render_blit_mono_opaque
#define RENDER_BLIT_ALPHA DI.render_blit_alpha
#define RENDER_BLIT_MONO DI.render_blit_mono
#define RENDER_BLIT_ALPHA_A DI.render_blit_alpha_a
#define RENDER_BLIT_MONO_A DI.render_blit_mono_a

void artcontext_setup_draw_info(draw_info_t *di,
	unsigned int red_mask, unsigned int green_mask, unsigned int blue_mask,
	int bpp);
void artcontext_setup_gamma(float gamma);

#endif

