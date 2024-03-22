/**************************************************************************
 *
 * Copyright 2003 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "i915_debug.h"
#include "util/log.h"
#include "util/ralloc.h"
#include "util/u_debug.h"
#include "i915_batch.h"
#include "i915_context.h"
#include "i915_debug_private.h"
#include "i915_reg.h"
#include "i915_screen.h"

static const struct debug_named_value i915_debug_options[] = {
   {"blit", DBG_BLIT, "Print when using the 2d blitter"},
   {"emit", DBG_EMIT, "State emit information"},
   {"atoms", DBG_ATOMS, "Print dirty state atoms"},
   {"flush", DBG_FLUSH, "Flushing information"},
   {"texture", DBG_TEXTURE, "Texture information"},
   {"constants", DBG_CONSTANTS, "Constant buffers"},
   {"fs", DBG_FS, "Dump fragment shaders"},
   {"vbuf", DBG_VBUF, "Use the WIP vbuf code path"},
   DEBUG_NAMED_VALUE_END};

unsigned i915_debug = 0;

DEBUG_GET_ONCE_FLAGS_OPTION(i915_debug, "I915_DEBUG", i915_debug_options, 0)
DEBUG_GET_ONCE_BOOL_OPTION(i915_no_tiling, "I915_NO_TILING", false)
DEBUG_GET_ONCE_BOOL_OPTION(i915_use_blitter, "I915_USE_BLITTER", true)

void
i915_debug_init(struct i915_screen *is)
{
   i915_debug = debug_get_option_i915_debug();
   is->debug.tiling = !debug_get_option_i915_no_tiling();
   is->debug.use_blitter = debug_get_option_i915_use_blitter();
}

/***********************************************************************
 * Batchbuffer dumping
 */

static bool
debug(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned i;
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);

   if (len == 0) {
      mesa_logi("Error - zero length packet (0x%08x)", stream->ptr[0]);
      assert(0);
      return false;
   }

   if (stream->print_addresses)
      mesa_logi("%08x:  ", stream->offset);

   mesa_logi("%s (%d dwords):", name, len);
   for (i = 0; i < len; i++)
      mesa_logi("\t0x%08x", ptr[i]);
   mesa_logi("%s", "");

   stream->offset += len * sizeof(unsigned);

   return true;
}

static const char *
get_prim_name(unsigned val)
{
   switch (val & PRIM3D_MASK) {
   case PRIM3D_TRILIST:
      return "TRILIST";
      break;
   case PRIM3D_TRISTRIP:
      return "TRISTRIP";
      break;
   case PRIM3D_TRISTRIP_RVRSE:
      return "TRISTRIP_RVRSE";
      break;
   case PRIM3D_TRIFAN:
      return "TRIFAN";
      break;
   case PRIM3D_POLY:
      return "POLY";
      break;
   case PRIM3D_LINELIST:
      return "LINELIST";
      break;
   case PRIM3D_LINESTRIP:
      return "LINESTRIP";
      break;
   case PRIM3D_RECTLIST:
      return "RECTLIST";
      break;
   case PRIM3D_POINTLIST:
      return "POINTLIST";
      break;
   case PRIM3D_DIB:
      return "DIB";
      break;
   case PRIM3D_CLEAR_RECT:
      return "CLEAR_RECT";
      break;
   case PRIM3D_ZONE_INIT:
      return "ZONE_INIT";
      break;
   default:
      return "????";
      break;
   }
}

static bool
debug_prim(struct debug_stream *stream, const char *name, bool dump_floats,
           unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   const char *prim = get_prim_name(ptr[0]);
   unsigned i;

   mesa_logi("%s %s (%d dwords):", name, prim, len);
   mesa_logi("\t0x%08x", ptr[0]);
   for (i = 1; i < len; i++) {
      if (dump_floats)
         mesa_logi("\t0x%08x // %f", ptr[i], *(float *)&ptr[i]);
      else
         mesa_logi("\t0x%08x", ptr[i]);
   }

   mesa_logi("%s", "");

   stream->offset += len * sizeof(unsigned);

   return true;
}

static bool
debug_program(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);

   if (len == 0) {
      mesa_logi("Error - zero length packet (0x%08x)", stream->ptr[0]);
      assert(0);
      return false;
   }

   if (stream->print_addresses)
      mesa_logi("%08x:  ", stream->offset);

   mesa_logi("%s (%d dwords):", name, len);
   i915_disassemble_program(ptr, len);

   stream->offset += len * sizeof(unsigned);
   return true;
}

static bool
debug_chain(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   unsigned old_offset = stream->offset + len * sizeof(unsigned);
   unsigned i;

   mesa_logi("%s (%d dwords):", name, len);
   for (i = 0; i < len; i++)
      mesa_logi("\t0x%08x", ptr[i]);

   stream->offset = ptr[1] & ~0x3;

   if (stream->offset < old_offset)
      mesa_logi("... skipping backwards from 0x%x --> 0x%x ...", old_offset,
                stream->offset);
   else
      mesa_logi("... skipping from 0x%x --> 0x%x ...", old_offset,
                stream->offset);

   return true;
}

static bool
debug_variable_length_prim(struct debug_stream *stream)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   const char *prim = get_prim_name(ptr[0]);
   unsigned i, len;

   uint16_t *idx = (uint16_t *)(ptr + 1);
   for (i = 0; idx[i] != 0xffff; i++)
      ;

   len = 1 + (i + 2) / 2;

   mesa_logi("3DPRIM, %s variable length %d indicies (%d dwords):", prim, i,
             len);
   for (i = 0; i < len; i++)
      mesa_logi("\t0x%08x", ptr[i]);
   mesa_logi("%s", "");

   stream->offset += len * sizeof(unsigned);
   return true;
}

static void
BITS(struct debug_stream *stream, unsigned dw, unsigned hi, unsigned lo,
     const char *fmt, ...)
{
   va_list args;
   unsigned himask = 0xFFFFFFFFUL >> (31 - (hi));

   va_start(args, fmt);
   char *out = ralloc_vasprintf(NULL, fmt, args);
   va_end(args);

   mesa_logi("\t\t %s : 0x%x", out, ((dw)&himask) >> (lo));

   ralloc_free(out);
}

#define MBZ(dw, hi, lo)                                                        \
   do {                                                                        \
      ASSERTED unsigned x = (dw) >> (lo);                                      \
      ASSERTED unsigned lomask = (1 << (lo)) - 1;                              \
      ASSERTED unsigned himask;                                                \
      himask = (1UL << (hi)) - 1;                                              \
      assert((x & himask & ~lomask) == 0);                                     \
   } while (0)

static void
FLAG(struct debug_stream *stream, unsigned dw, unsigned bit, const char *fmt,
     ...)
{
   if (((dw) >> (bit)) & 1) {
      va_list args;
      va_start(args, fmt);
      char *out = ralloc_vasprintf(NULL, fmt, args);
      va_end(args);

      mesa_logi("\t\t %s", out);

      ralloc_free(out);
   }
}

static bool
debug_load_immediate(struct debug_stream *stream, const char *name,
                     unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   unsigned bits = (ptr[0] >> 4) & 0xff;
   unsigned j = 0;

   mesa_logi("%s (%d dwords, flags: %x):", name, len, bits);
   mesa_logi("\t0x%08x", ptr[j++]);

   if (bits & (1 << 0)) {
      mesa_logi("\t  LIS0: 0x%08x", ptr[j]);
      mesa_logi("\t vb address: 0x%08x", (ptr[j] & ~0x3));
      BITS(stream, ptr[j], 0, 0, "vb invalidate disable");
      j++;
   }
   if (bits & (1 << 1)) {
      mesa_logi("\t  LIS1: 0x%08x", ptr[j]);
      BITS(stream, ptr[j], 29, 24, "vb dword width");
      BITS(stream, ptr[j], 21, 16, "vb dword pitch");
      BITS(stream, ptr[j], 15, 0, "vb max index");
      j++;
   }
   if (bits & (1 << 2)) {
      int i;
      mesa_logi("\t  LIS2: 0x%08x", ptr[j]);
      for (i = 0; i < 8; i++) {
         unsigned tc = (ptr[j] >> (i * 4)) & 0xf;
         if (tc != 0xf)
            BITS(stream, tc, 3, 0, "tex coord %d", i);
      }
      j++;
   }
   if (bits & (1 << 3)) {
      mesa_logi("\t  LIS3: 0x%08x", ptr[j]);
      j++;
   }
   if (bits & (1 << 4)) {
      mesa_logi("\t  LIS4: 0x%08x", ptr[j]);
      BITS(stream, ptr[j], 31, 23, "point width");
      BITS(stream, ptr[j], 22, 19, "line width");
      FLAG(stream, ptr[j], 18, "alpha flatshade");
      FLAG(stream, ptr[j], 17, "fog flatshade");
      FLAG(stream, ptr[j], 16, "spec flatshade");
      FLAG(stream, ptr[j], 15, "rgb flatshade");
      BITS(stream, ptr[j], 14, 13, "cull mode");
      FLAG(stream, ptr[j], 12, "vfmt: point width");
      FLAG(stream, ptr[j], 11, "vfmt: specular/fog");
      FLAG(stream, ptr[j], 10, "vfmt: rgba");
      FLAG(stream, ptr[j], 9, "vfmt: depth offset");
      BITS(stream, ptr[j], 8, 6, "vfmt: position (2==xyzw)");
      FLAG(stream, ptr[j], 5, "force dflt diffuse");
      FLAG(stream, ptr[j], 4, "force dflt specular");
      FLAG(stream, ptr[j], 3, "local depth offset enable");
      FLAG(stream, ptr[j], 2, "vfmt: fp32 fog coord");
      FLAG(stream, ptr[j], 1, "sprite point");
      FLAG(stream, ptr[j], 0, "antialiasing");
      j++;
   }
   if (bits & (1 << 5)) {
      mesa_logi("\t  LIS5: 0x%08x", ptr[j]);
      BITS(stream, ptr[j], 31, 28, "rgba write disables");
      FLAG(stream, ptr[j], 27, "force dflt point width");
      FLAG(stream, ptr[j], 26, "last pixel enable");
      FLAG(stream, ptr[j], 25, "global z offset enable");
      FLAG(stream, ptr[j], 24, "fog enable");
      BITS(stream, ptr[j], 23, 16, "stencil ref");
      BITS(stream, ptr[j], 15, 13, "stencil test");
      BITS(stream, ptr[j], 12, 10, "stencil fail op");
      BITS(stream, ptr[j], 9, 7, "stencil pass z fail op");
      BITS(stream, ptr[j], 6, 4, "stencil pass z pass op");
      FLAG(stream, ptr[j], 3, "stencil write enable");
      FLAG(stream, ptr[j], 2, "stencil test enable");
      FLAG(stream, ptr[j], 1, "color dither enable");
      FLAG(stream, ptr[j], 0, "logiop enable");
      j++;
   }
   if (bits & (1 << 6)) {
      mesa_logi("\t  LIS6: 0x%08x", ptr[j]);
      FLAG(stream, ptr[j], 31, "alpha test enable");
      BITS(stream, ptr[j], 30, 28, "alpha func");
      BITS(stream, ptr[j], 27, 20, "alpha ref");
      FLAG(stream, ptr[j], 19, "depth test enable");
      BITS(stream, ptr[j], 18, 16, "depth func");
      FLAG(stream, ptr[j], 15, "blend enable");
      BITS(stream, ptr[j], 14, 12, "blend func");
      BITS(stream, ptr[j], 11, 8, "blend src factor");
      BITS(stream, ptr[j], 7, 4, "blend dst factor");
      FLAG(stream, ptr[j], 3, "depth write enable");
      FLAG(stream, ptr[j], 2, "color write enable");
      BITS(stream, ptr[j], 1, 0, "provoking vertex");
      j++;
   }

   mesa_logi("%s", "");

   assert(j == len);

   stream->offset += len * sizeof(unsigned);

   return true;
}

static bool
debug_load_indirect(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   unsigned bits = (ptr[0] >> 8) & 0x3f;
   unsigned i, j = 0;

   mesa_logi("%s (%d dwords):", name, len);
   mesa_logi("\t0x%08x", ptr[j++]);

   for (i = 0; i < 6; i++) {
      if (bits & (1 << i)) {
         switch (1 << (8 + i)) {
         case LI0_STATE_STATIC_INDIRECT:
            mesa_logi("        STATIC: 0x%08x | %x", ptr[j] & ~3, ptr[j] & 3);
            j++;
            mesa_logi("                0x%08x", ptr[j++]);
            break;
         case LI0_STATE_DYNAMIC_INDIRECT:
            mesa_logi("       DYNAMIC: 0x%08x | %x", ptr[j] & ~3, ptr[j] & 3);
            j++;
            break;
         case LI0_STATE_SAMPLER:
            mesa_logi("       SAMPLER: 0x%08x | %x", ptr[j] & ~3, ptr[j] & 3);
            j++;
            mesa_logi("                0x%08x", ptr[j++]);
            break;
         case LI0_STATE_MAP:
            mesa_logi("           MAP: 0x%08x | %x", ptr[j] & ~3, ptr[j] & 3);
            j++;
            mesa_logi("                0x%08x", ptr[j++]);
            break;
         case LI0_STATE_PROGRAM:
            mesa_logi("       PROGRAM: 0x%08x | %x", ptr[j] & ~3, ptr[j] & 3);
            j++;
            mesa_logi("                0x%08x", ptr[j++]);
            break;
         case LI0_STATE_CONSTANTS:
            mesa_logi("     CONSTANTS: 0x%08x | %x", ptr[j] & ~3, ptr[j] & 3);
            j++;
            mesa_logi("                0x%08x", ptr[j++]);
            break;
         default:
            assert(0);
            break;
         }
      }
   }

   if (bits == 0) {
      mesa_logi("\t  DUMMY: 0x%08x", ptr[j++]);
   }

   mesa_logi("%s", "");

   assert(j == len);

   stream->offset += len * sizeof(unsigned);

   return true;
}

static void
BR13(struct debug_stream *stream, unsigned val)
{
   mesa_logi("\t0x%08x", val);
   FLAG(stream, val, 30, "clipping enable");
   BITS(stream, val, 25, 24, "color depth (3==32bpp)");
   BITS(stream, val, 23, 16, "raster op");
   BITS(stream, val, 15, 0, "dest pitch");
}

static void
BR22(struct debug_stream *stream, unsigned val)
{
   mesa_logi("\t0x%08x", val);
   BITS(stream, val, 31, 16, "dest y1");
   BITS(stream, val, 15, 0, "dest x1");
}

static void
BR23(struct debug_stream *stream, unsigned val)
{
   mesa_logi("\t0x%08x", val);
   BITS(stream, val, 31, 16, "dest y2");
   BITS(stream, val, 15, 0, "dest x2");
}

static void
BR09(struct debug_stream *stream, unsigned val)
{
   mesa_logi("\t0x%08x -- dest address", val);
}

static void
BR26(struct debug_stream *stream, unsigned val)
{
   mesa_logi("\t0x%08x", val);
   BITS(stream, val, 31, 16, "src y1");
   BITS(stream, val, 15, 0, "src x1");
}

static void
BR11(struct debug_stream *stream, unsigned val)
{
   mesa_logi("\t0x%08x", val);
   BITS(stream, val, 15, 0, "src pitch");
}

static void
BR12(struct debug_stream *stream, unsigned val)
{
   mesa_logi("\t0x%08x -- src address", val);
}

static void
BR16(struct debug_stream *stream, unsigned val)
{
   mesa_logi("\t0x%08x -- color", val);
}

static bool
debug_copy_blit(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   int j = 0;

   mesa_logi("%s (%d dwords):", name, len);
   mesa_logi("\t0x%08x", ptr[j++]);

   BR13(stream, ptr[j++]);
   BR22(stream, ptr[j++]);
   BR23(stream, ptr[j++]);
   BR09(stream, ptr[j++]);
   BR26(stream, ptr[j++]);
   BR11(stream, ptr[j++]);
   BR12(stream, ptr[j++]);

   stream->offset += len * sizeof(unsigned);
   assert(j == len);
   return true;
}

static bool
debug_color_blit(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   int j = 0;

   mesa_logi("%s (%d dwords):", name, len);
   mesa_logi("\t0x%08x", ptr[j++]);

   BR13(stream, ptr[j++]);
   BR22(stream, ptr[j++]);
   BR23(stream, ptr[j++]);
   BR09(stream, ptr[j++]);
   BR16(stream, ptr[j++]);

   stream->offset += len * sizeof(unsigned);
   assert(j == len);
   return true;
}

static bool
debug_modes4(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   int j = 0;

   mesa_logi("%s (%d dwords):", name, len);
   mesa_logi("\t0x%08x", ptr[j]);
   BITS(stream, ptr[j], 21, 18, "logicop func");
   FLAG(stream, ptr[j], 17, "stencil test mask modify-enable");
   FLAG(stream, ptr[j], 16, "stencil write mask modify-enable");
   BITS(stream, ptr[j], 15, 8, "stencil test mask");
   BITS(stream, ptr[j], 7, 0, "stencil write mask");
   j++;

   stream->offset += len * sizeof(unsigned);
   assert(j == len);
   return true;
}

static bool
debug_map_state(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   unsigned j = 0;

   mesa_logi("%s (%d dwords):", name, len);
   mesa_logi("\t0x%08x", ptr[j++]);

   {
      mesa_logi("\t0x%08x", ptr[j]);
      BITS(stream, ptr[j], 15, 0, "map mask");
      j++;
   }

   while (j < len) {
      {
         mesa_logi("\t  TMn.0: 0x%08x", ptr[j]);
         mesa_logi("\t map address: 0x%08x", (ptr[j] & ~0x3));
         FLAG(stream, ptr[j], 1, "vertical line stride");
         FLAG(stream, ptr[j], 0, "vertical line stride offset");
         j++;
      }

      {
         mesa_logi("\t  TMn.1: 0x%08x", ptr[j]);
         BITS(stream, ptr[j], 31, 21, "height");
         BITS(stream, ptr[j], 20, 10, "width");
         BITS(stream, ptr[j], 9, 7, "surface format");
         BITS(stream, ptr[j], 6, 3, "texel format");
         FLAG(stream, ptr[j], 2, "use fence regs");
         FLAG(stream, ptr[j], 1, "tiled surface");
         FLAG(stream, ptr[j], 0, "tile walk ymajor");
         j++;
      }
      {
         mesa_logi("\t  TMn.2: 0x%08x", ptr[j]);
         BITS(stream, ptr[j], 31, 21, "dword pitch");
         BITS(stream, ptr[j], 20, 15, "cube face enables");
         BITS(stream, ptr[j], 14, 9, "max lod");
         FLAG(stream, ptr[j], 8, "mip layout right");
         BITS(stream, ptr[j], 7, 0, "depth");
         j++;
      }
   }

   stream->offset += len * sizeof(unsigned);
   assert(j == len);
   return true;
}

static bool
debug_sampler_state(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   unsigned j = 0;

   mesa_logi("%s (%d dwords):", name, len);
   mesa_logi("\t0x%08x", ptr[j++]);

   {
      mesa_logi("\t0x%08x", ptr[j]);
      BITS(stream, ptr[j], 15, 0, "sampler mask");
      j++;
   }

   while (j < len) {
      {
         mesa_logi("\t  TSn.0: 0x%08x", ptr[j]);
         FLAG(stream, ptr[j], 31, "reverse gamma");
         FLAG(stream, ptr[j], 30, "planar to packed");
         FLAG(stream, ptr[j], 29, "yuv->rgb");
         BITS(stream, ptr[j], 28, 27, "chromakey index");
         BITS(stream, ptr[j], 26, 22, "base mip level");
         BITS(stream, ptr[j], 21, 20, "mip mode filter");
         BITS(stream, ptr[j], 19, 17, "mag mode filter");
         BITS(stream, ptr[j], 16, 14, "min mode filter");
         BITS(stream, ptr[j], 13, 5, "lod bias (s4.4)");
         FLAG(stream, ptr[j], 4, "shadow enable");
         FLAG(stream, ptr[j], 3, "max-aniso-4");
         BITS(stream, ptr[j], 2, 0, "shadow func");
         j++;
      }

      {
         mesa_logi("\t  TSn.1: 0x%08x", ptr[j]);
         BITS(stream, ptr[j], 31, 24, "min lod");
         MBZ(ptr[j], 23, 18);
         FLAG(stream, ptr[j], 17, "kill pixel enable");
         FLAG(stream, ptr[j], 16, "keyed tex filter mode");
         FLAG(stream, ptr[j], 15, "chromakey enable");
         BITS(stream, ptr[j], 14, 12, "tcx wrap mode");
         BITS(stream, ptr[j], 11, 9, "tcy wrap mode");
         BITS(stream, ptr[j], 8, 6, "tcz wrap mode");
         FLAG(stream, ptr[j], 5, "normalized coords");
         BITS(stream, ptr[j], 4, 1, "map (surface) index");
         FLAG(stream, ptr[j], 0, "EAST deinterlacer enable");
         j++;
      }
      {
         mesa_logi("\t  TSn.2: 0x%08x  (default color)", ptr[j]);
         j++;
      }
   }

   stream->offset += len * sizeof(unsigned);
   assert(j == len);
   return true;
}

static bool
debug_dest_vars(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   int j = 0;

   mesa_logi("%s (%d dwords):", name, len);
   mesa_logi("\t0x%08x", ptr[j++]);

   {
      mesa_logi("\t0x%08x", ptr[j]);
      FLAG(stream, ptr[j], 31, "early classic ztest");
      FLAG(stream, ptr[j], 30, "opengl tex default color");
      FLAG(stream, ptr[j], 29, "bypass iz");
      FLAG(stream, ptr[j], 28, "lod preclamp");
      BITS(stream, ptr[j], 27, 26, "dither pattern");
      FLAG(stream, ptr[j], 25, "linear gamma blend");
      FLAG(stream, ptr[j], 24, "debug dither");
      BITS(stream, ptr[j], 23, 20, "dstorg x");
      BITS(stream, ptr[j], 19, 16, "dstorg y");
      MBZ(ptr[j], 15, 15);
      BITS(stream, ptr[j], 14, 12, "422 write select");
      BITS(stream, ptr[j], 11, 8, "cbuf format");
      BITS(stream, ptr[j], 3, 2, "zbuf format");
      FLAG(stream, ptr[j], 1, "vert line stride");
      FLAG(stream, ptr[j], 1, "vert line stride offset");
      j++;
   }

   stream->offset += len * sizeof(unsigned);
   assert(j == len);
   return true;
}

static bool
debug_buf_info(struct debug_stream *stream, const char *name, unsigned len)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   int j = 0;

   mesa_logi("%s (%d dwords):", name, len);
   mesa_logi("\t0x%08x", ptr[j++]);

   {
      mesa_logi("\t0x%08x", ptr[j]);
      BITS(stream, ptr[j], 28, 28, "aux buffer id");
      BITS(stream, ptr[j], 27, 24, "buffer id (7=depth, 3=back)");
      FLAG(stream, ptr[j], 23, "use fence regs");
      FLAG(stream, ptr[j], 22, "tiled surface");
      FLAG(stream, ptr[j], 21, "tile walk ymajor");
      MBZ(ptr[j], 20, 14);
      BITS(stream, ptr[j], 13, 2, "dword pitch");
      MBZ(ptr[j], 2, 0);
      j++;
   }

   mesa_logi("\t0x%08x -- buffer base address", ptr[j++]);

   stream->offset += len * sizeof(unsigned);
   assert(j == len);
   return true;
}

static bool
i915_debug_packet(struct debug_stream *stream)
{
   unsigned *ptr = (unsigned *)(stream->ptr + stream->offset);
   unsigned cmd = *ptr;

   switch (((cmd >> 29) & 0x7)) {
   case 0x0:
      switch ((cmd >> 23) & 0x3f) {
      case 0x0:
         return debug(stream, "MI_NOOP", 1);
      case 0x3:
         return debug(stream, "MI_WAIT_FOR_EVENT", 1);
      case 0x4:
         return debug(stream, "MI_FLUSH", 1);
      case 0xA:
         debug(stream, "MI_BATCH_BUFFER_END", 1);
         return false;
      case 0x22:
         return debug(stream, "MI_LOAD_REGISTER_IMM", 3);
      case 0x31:
         return debug_chain(stream, "MI_BATCH_BUFFER_START", 2);
      default:
         (void)debug(stream, "UNKNOWN 0x0 case!", 1);
         assert(0);
         break;
      }
      break;
   case 0x1:
      (void)debug(stream, "UNKNOWN 0x1 case!", 1);
      assert(0);
      break;
   case 0x2:
      switch ((cmd >> 22) & 0xff) {
      case 0x50:
         return debug_color_blit(stream, "XY_COLOR_BLT", (cmd & 0xff) + 2);
      case 0x53:
         return debug_copy_blit(stream, "XY_SRC_COPY_BLT", (cmd & 0xff) + 2);
      default:
         return debug(stream, "blit command", (cmd & 0xff) + 2);
      }
      break;
   case 0x3:
      switch ((cmd >> 24) & 0x1f) {
      case 0x6:
         return debug(stream, "3DSTATE_ANTI_ALIASING", 1);
      case 0x7:
         return debug(stream, "3DSTATE_RASTERIZATION_RULES", 1);
      case 0x8:
         return debug(stream, "3DSTATE_BACKFACE_STENCIL_OPS", 1);
      case 0x9:
         return debug(stream, "3DSTATE_BACKFACE_STENCIL_MASKS", 1);
      case 0xb:
         return debug(stream, "3DSTATE_INDEPENDENT_ALPHA_BLEND", 1);
      case 0xc:
         return debug(stream, "3DSTATE_MODES5", 1);
      case 0xd:
         return debug_modes4(stream, "3DSTATE_MODES4", 1);
      case 0x15:
         return debug(stream, "3DSTATE_FOG_COLOR", 1);
      case 0x16:
         return debug(stream, "3DSTATE_COORD_SET_BINDINGS", 1);
      case 0x1c:
         /* 3DState16NP */
         switch ((cmd >> 19) & 0x1f) {
         case 0x10:
            return debug(stream, "3DSTATE_SCISSOR_ENABLE", 1);
         case 0x11:
            return debug(stream, "3DSTATE_DEPTH_SUBRECTANGLE_DISABLE", 1);
         default:
            (void)debug(stream, "UNKNOWN 0x1c case!", 1);
            assert(0);
            break;
         }
         break;
      case 0x1d:
         /* 3DStateMW */
         switch ((cmd >> 16) & 0xff) {
         case 0x0:
            return debug_map_state(stream, "3DSTATE_MAP_STATE",
                                   (cmd & 0x1f) + 2);
         case 0x1:
            return debug_sampler_state(stream, "3DSTATE_SAMPLER_STATE",
                                       (cmd & 0x1f) + 2);
         case 0x4:
            return debug_load_immediate(stream, "3DSTATE_LOAD_STATE_IMMEDIATE",
                                        (cmd & 0xf) + 2);
         case 0x5:
            return debug_program(stream, "3DSTATE_PIXEL_SHADER_PROGRAM",
                                 (cmd & 0x1ff) + 2);
         case 0x6:
            return debug(stream, "3DSTATE_PIXEL_SHADER_CONSTANTS",
                         (cmd & 0xff) + 2);
         case 0x7:
            return debug_load_indirect(stream, "3DSTATE_LOAD_INDIRECT",
                                       (cmd & 0xff) + 2);
         case 0x80:
            return debug(stream, "3DSTATE_DRAWING_RECTANGLE",
                         (cmd & 0xffff) + 2);
         case 0x81:
            return debug(stream, "3DSTATE_SCISSOR_RECTANGLE",
                         (cmd & 0xffff) + 2);
         case 0x83:
            return debug(stream, "3DSTATE_SPAN_STIPPLE", (cmd & 0xffff) + 2);
         case 0x85:
            return debug_dest_vars(stream, "3DSTATE_DEST_BUFFER_VARS",
                                   (cmd & 0xffff) + 2);
         case 0x88:
            return debug(stream, "3DSTATE_CONSTANT_BLEND_COLOR",
                         (cmd & 0xffff) + 2);
         case 0x89:
            return debug(stream, "3DSTATE_FOG_MODE", (cmd & 0xffff) + 2);
         case 0x8e:
            return debug_buf_info(stream, "3DSTATE_BUFFER_INFO",
                                  (cmd & 0xffff) + 2);
         case 0x97:
            return debug(stream, "3DSTATE_DEPTH_OFFSET_SCALE",
                         (cmd & 0xffff) + 2);
         case 0x98:
            return debug(stream, "3DSTATE_DEFAULT_Z", (cmd & 0xffff) + 2);
         case 0x99:
            return debug(stream, "3DSTATE_DEFAULT_DIFFUSE", (cmd & 0xffff) + 2);
         case 0x9a:
            return debug(stream, "3DSTATE_DEFAULT_SPECULAR",
                         (cmd & 0xffff) + 2);
         case 0x9c:
            return debug(stream, "3DSTATE_CLEAR_PARAMETERS",
                         (cmd & 0xffff) + 2);
         default:
            assert(0);
            return 0;
         }
         break;
      case 0x1e:
         if (cmd & (1 << 23))
            return debug(stream, "???", (cmd & 0xffff) + 1);
         else
            return debug(stream, "", 1);
         break;
      case 0x1f:
         if ((cmd & (1 << 23)) == 0)
            return debug_prim(stream, "3DPRIM (inline)", 1,
                              (cmd & 0x1ffff) + 2);
         else if (cmd & (1 << 17)) {
            if ((cmd & 0xffff) == 0)
               return debug_variable_length_prim(stream);
            else
               return debug_prim(stream, "3DPRIM (indexed)", 0,
                                 (((cmd & 0xffff) + 1) / 2) + 1);
         } else
            return debug_prim(stream, "3DPRIM  (indirect sequential)", 0, 2);
         break;
      default:
         return debug(stream, "", 0);
      }
      break;
   default:
      assert(0);
      return 0;
   }

   assert(0);
   return 0;
}

void
i915_dump_batchbuffer(struct i915_winsys_batchbuffer *batch)
{
   struct debug_stream stream;
   unsigned *start = (unsigned *)batch->map;
   unsigned *end = (unsigned *)batch->ptr;
   unsigned long bytes = (unsigned long)(end - start) * 4;
   bool done = false;

   stream.offset = 0;
   stream.ptr = (char *)start;
   stream.print_addresses = 0;

   if (!start || !end) {
      mesa_logi("BATCH: ???");
      return;
   }

   mesa_logi("BATCH: (%d)", (int)bytes / 4);

   while (!done && stream.offset < bytes) {
      if (!i915_debug_packet(&stream))
         break;

      assert(stream.offset <= bytes);
   }

   mesa_logi("END-BATCH");
}

/***********************************************************************
 * Dirty state atom dumping
 */

void
i915_dump_dirty(struct i915_context *i915, const char *func)
{
   struct {
      unsigned dirty;
      const char *name;
   } l[] = {
      {I915_NEW_VIEWPORT, "viewport"},
      {I915_NEW_RASTERIZER, "rasterizer"},
      {I915_NEW_FS, "fs"},
      {I915_NEW_BLEND, "blend"},
      {I915_NEW_CLIP, "clip"},
      {I915_NEW_SCISSOR, "scissor"},
      {I915_NEW_STIPPLE, "stipple"},
      {I915_NEW_FRAMEBUFFER, "framebuffer"},
      {I915_NEW_ALPHA_TEST, "alpha_test"},
      {I915_NEW_DEPTH_STENCIL, "depth_stencil"},
      {I915_NEW_SAMPLER, "sampler"},
      {I915_NEW_SAMPLER_VIEW, "sampler_view"},
      {I915_NEW_VS_CONSTANTS, "vs_const"},
      {I915_NEW_FS_CONSTANTS, "fs_const"},
      {I915_NEW_VBO, "vbo"},
      {I915_NEW_VS, "vs"},
      {0, NULL},
   };
   int i;

   mesa_logi("%s: ", func);
   for (i = 0; l[i].name; i++)
      if (i915->dirty & l[i].dirty)
         mesa_logi("%s ", l[i].name);
   mesa_logi("%s", "");
}

void
i915_dump_hardware_dirty(struct i915_context *i915, const char *func)
{
   struct {
      unsigned dirty;
      const char *name;
   } l[] = {
      {I915_HW_STATIC, "static"},
      {I915_HW_DYNAMIC, "dynamic"},
      {I915_HW_SAMPLER, "sampler"},
      {I915_HW_MAP, "map"},
      {I915_HW_PROGRAM, "program"},
      {I915_HW_CONSTANTS, "constants"},
      {I915_HW_IMMEDIATE, "immediate"},
      {I915_HW_INVARIANT, "invariant"},
      {0, NULL},
   };
   int i;

   mesa_logi("%s: ", func);
   for (i = 0; l[i].name; i++)
      if (i915->hardware_dirty & l[i].dirty)
         mesa_logi("%s ", l[i].name);
   mesa_logi("%s", "");
}
