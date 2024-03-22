/* Originally written by Ben Skeggs for the nv50 driver*/

#ifndef U_SPLIT_PRIM_H
#define U_SPLIT_PRIM_H

#include "pipe/p_defines.h"
#include "util/compiler.h"

#include "util/u_debug.h"

struct util_split_prim {
   void *priv;
   void (*emit)(void *priv, unsigned start, unsigned count);
   void (*edge)(void *priv, bool enabled);

   unsigned mode;
   unsigned start;
   unsigned p_start;
   unsigned p_end;

   unsigned repeat_first:1;
   unsigned close_first:1;
   unsigned edgeflag_off:1;
};

static inline void
util_split_prim_init(struct util_split_prim *s,
                  unsigned mode, unsigned start, unsigned count)
{
   if (mode == MESA_PRIM_LINE_LOOP) {
      s->mode = MESA_PRIM_LINE_STRIP;
      s->close_first = 1;
   } else {
      s->mode = mode;
      s->close_first = 0;
   }
   s->start = start;
   s->p_start = start;
   s->p_end = start + count;
   s->edgeflag_off = 0;
   s->repeat_first = 0;
}

static inline bool
util_split_prim_next(struct util_split_prim *s, unsigned max_verts)
{
   int repeat = 0;

   if (s->repeat_first) {
      s->emit(s->priv, s->start, 1);
      max_verts--;
      if (s->edgeflag_off) {
         s->edge(s->priv, true);
         s->edgeflag_off = false;
      }
   }

   if ((s->p_end - s->p_start) + s->close_first <= max_verts) {
      s->emit(s->priv, s->p_start, s->p_end - s->p_start);
      if (s->close_first)
         s->emit(s->priv, s->start, 1);
      return true;
   }

   switch (s->mode) {
   case MESA_PRIM_LINES:
      max_verts &= ~1;
      break;
   case MESA_PRIM_LINE_STRIP:
      repeat = 1;
      break;
   case MESA_PRIM_POLYGON:
      max_verts--;
      s->emit(s->priv, s->p_start, max_verts);
      s->edge(s->priv, false);
      s->emit(s->priv, s->p_start + max_verts, 1);
      s->p_start += max_verts;
      s->repeat_first = true;
      s->edgeflag_off = true;
      return false;
   case MESA_PRIM_TRIANGLES:
      max_verts = max_verts - (max_verts % 3);
      break;
   case MESA_PRIM_TRIANGLE_STRIP:
      /* to ensure winding stays correct, always split
       * on an even number of generated triangles
       */
      max_verts = max_verts & ~1;
      repeat = 2;
      break;
   case MESA_PRIM_TRIANGLE_FAN:
      s->repeat_first = true;
      repeat = 1;
      break;
   case MESA_PRIM_QUADS:
      max_verts &= ~3;
      break;
   case MESA_PRIM_QUAD_STRIP:
      max_verts &= ~1;
      repeat = 2;
      break;
   case MESA_PRIM_POINTS:
      break;
   default:
      /* TODO: implement adjacency primitives */
      assert(0);
   }

   s->emit (s->priv, s->p_start, max_verts);
   s->p_start += (max_verts - repeat);
   return false;
}

#endif /* U_SPLIT_PRIM_H */
