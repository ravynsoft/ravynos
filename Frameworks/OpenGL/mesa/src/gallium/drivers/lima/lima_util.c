/*
 * Copyright (C) 2018-2019 Lima Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <pipe/p_defines.h>

#include "util/u_debug.h"
#include "util/u_memory.h"
#include "util/u_box.h"

#include "lima_util.h"
#include "lima_parser.h"
#include "lima_screen.h"

struct lima_dump {
   FILE *fp;
   int id;
};

static void
lima_dump_blob(FILE *fp, void *data, int size, bool is_float)
{
   fprintf(fp, "{\n");
   for (int i = 0; i * 4 < size; i++) {
      if (i % 4 == 0)
         fprintf(fp, "\t");

      if (is_float)
         fprintf(fp, "%f, ", ((float *)data)[i]);
      else
         fprintf(fp, "0x%08x, ", ((uint32_t *)data)[i]);

      if ((i % 4 == 3) || (i == size / 4 - 1)) {
         fprintf(fp, "/* 0x%08x */", MAX2((i - 3) * 4, 0));
         if (i) fprintf(fp, "\n");
      }
   }
   fprintf(fp, "}\n");
}

void
lima_dump_shader(struct lima_dump *dump, void *data, int size, bool is_frag)
{
   if (dump)
      lima_parse_shader(dump->fp, (uint32_t *)data, size, is_frag);
}

void
lima_dump_vs_command_stream_print(struct lima_dump *dump, void *data,
                                  int size, uint32_t start)
{
   if (dump)
      lima_parse_vs(dump->fp, (uint32_t *)data, size, start);
}

void
lima_dump_plbu_command_stream_print(struct lima_dump *dump, void *data,
                                    int size, uint32_t start)
{
   if (dump)
      lima_parse_plbu(dump->fp, (uint32_t *)data, size, start);
}

void
lima_dump_rsw_command_stream_print(struct lima_dump *dump, void *data,
                                   int size, uint32_t start)
{
   if (dump)
      lima_parse_render_state(dump->fp, (uint32_t *)data, size, start);
}

void
lima_dump_texture_descriptor(struct lima_dump *dump, void *data,
                             int size, uint32_t start, uint32_t offset)
{
   if (dump)
      lima_parse_texture_descriptor(dump->fp, (uint32_t *)data, size, start, offset);
}

struct lima_dump *
lima_dump_create(void)
{
   static int dump_id = 0;

   if (!(lima_debug & LIMA_DEBUG_DUMP))
      return NULL;

   struct lima_dump *ret = MALLOC_STRUCT(lima_dump);
   if (!ret)
      return NULL;

   ret->id = dump_id++;

   char buffer[PATH_MAX];
   const char *dump_command = debug_get_option("LIMA_DUMP_FILE", "lima.dump");
   snprintf(buffer, sizeof(buffer), "%s.staging.%04d", dump_command, ret->id);

   ret->fp = fopen(buffer, "w");
   if (!ret->fp) {
      fprintf(stderr, "lima: failed to open command stream log file %s\n", buffer);
      FREE(ret);
      return NULL;
   }

   return ret;
}

void
lima_dump_free(struct lima_dump *dump)
{
   static int frame_count = 0;

   if (!dump)
      return;

   fclose(dump->fp);

   /* we only know the frame count when flush, not on dump create, so first dump to a
    * stage file, then move to the final file with frame count as surfix when flush.
    */

   char stage_name[PATH_MAX];
   char final_name[PATH_MAX];
   const char *dump_command = debug_get_option("LIMA_DUMP_FILE", "lima.dump");
   snprintf(stage_name, sizeof(stage_name), "%s.staging.%04d", dump_command, dump->id);
   snprintf(final_name, sizeof(final_name), "%s.%04d", dump_command, frame_count++);

   if (rename(stage_name, final_name))
      fprintf(stderr, "lima: failed to rename log %s to %s\n", stage_name, final_name);

   FREE(dump);
}

void
_lima_dump_command_stream_print(struct lima_dump *dump, void *data,
                                int size, bool is_float, const char *fmt, ...)
{
   va_list ap;
   va_start(ap, fmt);
   vfprintf(dump->fp, fmt, ap);
   va_end(ap);

   lima_dump_blob(dump->fp, data, size, is_float);
}

void
lima_damage_rect_union(struct pipe_scissor_state *rect,
                       unsigned minx, unsigned maxx,
                       unsigned miny, unsigned maxy)
{
   rect->minx = MIN2(rect->minx, minx);
   rect->miny = MIN2(rect->miny, miny);
   rect->maxx = MAX2(rect->maxx, maxx);
   rect->maxy = MAX2(rect->maxy, maxy);
}
