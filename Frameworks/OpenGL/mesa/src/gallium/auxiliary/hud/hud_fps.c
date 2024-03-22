/**************************************************************************
 *
 * Copyright 2013 Marek Olšák <maraeo@gmail.com>
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/* This file contains code for calculating framerate for displaying on the HUD.
 */

#include "hud/hud_private.h"
#include "util/os_time.h"
#include "util/u_memory.h"

struct fps_info {
   bool frametime;
   int frames;
   uint64_t last_time;
};

static void
query_fps(struct hud_graph *gr, struct pipe_context *pipe)
{
   struct fps_info *info = gr->query_data;
   uint64_t now = os_time_get();

   info->frames++;

   if (info->last_time) {
      if (info->frametime) {
         double frametime = ((double)now - (double)info->last_time) / 1000.0;
         hud_graph_add_value(gr, frametime);
         info->last_time = now;
      }
      else if (info->last_time + gr->pane->period <= now) {
         double fps = ((uint64_t)info->frames) * 1000000 /
                      (double)(now - info->last_time);
         info->frames = 0;
         info->last_time = now;

         hud_graph_add_value(gr, fps);
      }
   }
   else {
      info->last_time = now;
   }
}

static void
free_query_data(void *p, struct pipe_context *pipe)
{
   FREE(p);
}

void
hud_fps_graph_install(struct hud_pane *pane)
{
   struct hud_graph *gr = CALLOC_STRUCT(hud_graph);

   if (!gr)
      return;

   strcpy(gr->name, "fps");
   gr->query_data = CALLOC_STRUCT(fps_info);
   if (!gr->query_data) {
      FREE(gr);
      return;
   }
   struct fps_info *info = gr->query_data;
   info->frametime = false;

   gr->query_new_value = query_fps;

   /* Don't use free() as our callback as that messes up Gallium's
    * memory debugger.  Use simple free_query_data() wrapper.
    */
   gr->free_query_data = free_query_data;

   hud_pane_add_graph(pane, gr);
}

void
hud_frametime_graph_install(struct hud_pane *pane)
{
   struct hud_graph *gr = CALLOC_STRUCT(hud_graph);

   if (!gr)
      return;

   strcpy(gr->name, "frametime (ms)");
   gr->query_data = CALLOC_STRUCT(fps_info);
   if (!gr->query_data) {
      FREE(gr);
      return;
   }
   struct fps_info *info = gr->query_data;
   info->frametime = true;

   gr->query_new_value = query_fps;

   gr->free_query_data = free_query_data;

   hud_pane_add_graph(pane, gr);
}
