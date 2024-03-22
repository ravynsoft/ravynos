/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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

/**
 * \brief  Drawing stage for handling glPolygonMode(line/point).
 * Convert triangles to points or lines as needed.
 */

/* Authors:  Keith Whitwell <keithw@vmware.com>
 */

#include "util/u_memory.h"
#include "pipe/p_defines.h"
#include "draw_private.h"
#include "draw_pipe.h"
#include "draw_fs.h"


struct unfilled_stage {
   struct draw_stage stage;

   /** [0] = front face, [1] = back face.
    * legal values:  PIPE_POLYGON_MODE_FILL, PIPE_POLYGON_MODE_LINE,
    * and PIPE_POLYGON_MODE_POINT,
    */
   unsigned mode[2];

   int face_slot;
};


static inline struct
unfilled_stage *unfilled_stage(struct draw_stage *stage)
{
   return (struct unfilled_stage *) stage;
}


static void
inject_front_face_info(struct draw_stage *stage,
                       struct prim_header *header)
{
   struct unfilled_stage *unfilled = unfilled_stage(stage);
   bool is_front_face = (
      (stage->draw->rasterizer->front_ccw && header->det < 0.0f) ||
      (!stage->draw->rasterizer->front_ccw && header->det > 0.0f));
   int slot = unfilled->face_slot;

   /* In case the backend doesn't care about it */
   if (slot < 0) {
      return;
   }

   for (unsigned i = 0; i < 3; ++i) {
      struct vertex_header *v = header->v[i];
      v->data[slot][0] = is_front_face;
      v->data[slot][1] = is_front_face;
      v->data[slot][2] = is_front_face;
      v->data[slot][3] = is_front_face;
      v->vertex_id = UNDEFINED_VERTEX_ID;
   }
}


static void
point(struct draw_stage *stage,
      struct prim_header *header,
      struct vertex_header *v0)
{
   struct prim_header tmp;
   tmp.det = header->det;
   tmp.flags = 0;
   tmp.v[0] = v0;
   stage->next->point(stage->next, &tmp);
}


static void
line(struct draw_stage *stage,
     struct prim_header *header,
     struct vertex_header *v0,
     struct vertex_header *v1)
{
   struct prim_header tmp;
   tmp.det = header->det;
   tmp.flags = 0;
   tmp.v[0] = v0;
   tmp.v[1] = v1;
   stage->next->line(stage->next, &tmp);
}


static void
points(struct draw_stage *stage,
       struct prim_header *header)
{
   struct vertex_header *v0 = header->v[0];
   struct vertex_header *v1 = header->v[1];
   struct vertex_header *v2 = header->v[2];

   inject_front_face_info(stage, header);

   if ((header->flags & DRAW_PIPE_EDGE_FLAG_0) && v0->edgeflag)
      point(stage, header, v0);
   if ((header->flags & DRAW_PIPE_EDGE_FLAG_1) && v1->edgeflag)
      point(stage, header, v1);
   if ((header->flags & DRAW_PIPE_EDGE_FLAG_2) && v2->edgeflag)
      point(stage, header, v2);
}


static void
lines(struct draw_stage *stage,
      struct prim_header *header)
{
   struct vertex_header *v0 = header->v[0];
   struct vertex_header *v1 = header->v[1];
   struct vertex_header *v2 = header->v[2];

   if (header->flags & DRAW_PIPE_RESET_STIPPLE)
      /*
       * XXX could revisit this. The only stage which cares is the line
       * stipple stage. Could just emit correct reset flags here and not
       * bother about all the calling through reset_stipple_counter
       * stages. Though technically it is necessary if line stipple is
       * handled by the driver, but this is not actually hooked up when
       * using vbuf (vbuf stage reset_stipple_counter does nothing).
       */
      stage->next->reset_stipple_counter(stage->next);

   inject_front_face_info(stage, header);

   if ((header->flags & DRAW_PIPE_EDGE_FLAG_2) && v2->edgeflag)
      line(stage, header, v2, v0);
   if ((header->flags & DRAW_PIPE_EDGE_FLAG_0) && v0->edgeflag)
      line(stage, header, v0, v1);
   if ((header->flags & DRAW_PIPE_EDGE_FLAG_1) && v1->edgeflag)
      line(stage, header, v1, v2);
}


/** For debugging */
static void
print_header_flags(unsigned flags)
{
   debug_printf("header->flags = ");
   if (flags & DRAW_PIPE_RESET_STIPPLE)
      debug_printf("RESET_STIPPLE ");
   if (flags & DRAW_PIPE_EDGE_FLAG_0)
      debug_printf("EDGE_FLAG_0 ");
   if (flags & DRAW_PIPE_EDGE_FLAG_1)
      debug_printf("EDGE_FLAG_1 ");
   if (flags & DRAW_PIPE_EDGE_FLAG_2)
      debug_printf("EDGE_FLAG_2 ");
   debug_printf("\n");
}


/* Unfilled tri:
 *
 * Note edgeflags in the vertex struct is not sufficient as we will
 * need to manipulate them when decomposing primitives.
 *
 * We currently keep the vertex edgeflag and primitive edgeflag mask
 * separate until the last possible moment.
 */
static void
unfilled_tri(struct draw_stage *stage,
             struct prim_header *header)
{
   struct unfilled_stage *unfilled = unfilled_stage(stage);
   unsigned cw = header->det >= 0.0;
   unsigned mode = unfilled->mode[cw];

   if (0)
      print_header_flags(header->flags);

   switch (mode) {
   case PIPE_POLYGON_MODE_FILL:
      stage->next->tri(stage->next, header);
      break;
   case PIPE_POLYGON_MODE_LINE:
      lines(stage, header);
      break;
   case PIPE_POLYGON_MODE_POINT:
      points(stage, header);
      break;
   default:
      assert(0);
   }
}


static void
unfilled_first_tri(struct draw_stage *stage,
                   struct prim_header *header)
{
   struct unfilled_stage *unfilled = unfilled_stage(stage);
   const struct pipe_rasterizer_state *rast = stage->draw->rasterizer;

   unfilled->mode[0] = rast->front_ccw ? rast->fill_front : rast->fill_back;
   unfilled->mode[1] = rast->front_ccw ? rast->fill_back : rast->fill_front;

   stage->tri = unfilled_tri;
   stage->tri(stage, header);
}


static void
unfilled_flush(struct draw_stage *stage,
               unsigned flags)
{
   stage->next->flush(stage->next, flags);

   stage->tri = unfilled_first_tri;
}


static void
unfilled_reset_stipple_counter(struct draw_stage *stage)
{
   stage->next->reset_stipple_counter(stage->next);
}


static void
unfilled_destroy(struct draw_stage *stage)
{
   draw_free_temp_verts(stage);
   FREE(stage);
}


/*
 * Try to allocate an output slot which we can use
 * to preserve the front face information.
 */
void
draw_unfilled_prepare_outputs(struct draw_context *draw,
                               struct draw_stage *stage)
{
   struct unfilled_stage *unfilled = unfilled_stage(stage);
   const struct pipe_rasterizer_state *rast = draw ? draw->rasterizer : NULL;
   bool is_unfilled = (rast &&
                       (rast->fill_front != PIPE_POLYGON_MODE_FILL ||
                        rast->fill_back != PIPE_POLYGON_MODE_FILL));
   const struct draw_fragment_shader *fs = draw ? draw->fs.fragment_shader : NULL;

   if (is_unfilled && fs && fs->info.uses_frontface)  {
      unfilled->face_slot = draw_alloc_extra_vertex_attrib(
         stage->draw, TGSI_SEMANTIC_FACE, 0);
   } else {
      unfilled->face_slot = -1;
   }
}


/**
 * Create unfilled triangle stage.
 */
struct draw_stage *
draw_unfilled_stage(struct draw_context *draw)
{
   struct unfilled_stage *unfilled = CALLOC_STRUCT(unfilled_stage);
   if (!unfilled)
      goto fail;

   unfilled->stage.draw = draw;
   unfilled->stage.name = "unfilled";
   unfilled->stage.next = NULL;
   unfilled->stage.tmp = NULL;
   unfilled->stage.point = draw_pipe_passthrough_point;
   unfilled->stage.line = draw_pipe_passthrough_line;
   unfilled->stage.tri = unfilled_first_tri;
   unfilled->stage.flush = unfilled_flush;
   unfilled->stage.reset_stipple_counter = unfilled_reset_stipple_counter;
   unfilled->stage.destroy = unfilled_destroy;

   unfilled->face_slot = -1;

   if (!draw_alloc_temp_verts(&unfilled->stage, 0))
      goto fail;

   return &unfilled->stage;

 fail:
   if (unfilled)
      unfilled->stage.destroy(&unfilled->stage);

   return NULL;
}
