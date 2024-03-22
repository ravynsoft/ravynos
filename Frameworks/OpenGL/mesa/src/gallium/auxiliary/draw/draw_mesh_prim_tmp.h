#define FUNC_VARS                               \
   struct draw_mesh_prim *asmblr,              \
   const struct draw_prim_info *input_prims,    \
   const struct draw_vertex_info *input_verts,  \
   unsigned start,                              \
   unsigned count

#define FUNC_ENTER                                                  \
   /* declare more local vars */                                    \
   const enum mesa_prim prim = input_prims->prim;                   \
   const unsigned prim_flags = input_prims->flags;                  \
   const bool last_vertex_last = false;                             \
   switch (prim) {                                                  \
   case MESA_PRIM_POINTS:                                           \
   case MESA_PRIM_LINES:                                            \
   case MESA_PRIM_TRIANGLES:                                        \
      break;                                                        \
   default:                                                         \
      assert(!"unexpected primitive type in prim assembler");       \
      return;                                                       \
   }


#define POINT(i0)                             prim_point(asmblr, i0)
#define LINE(flags, i0, i1)                   prim_line(asmblr, i0, i1)
#define TRIANGLE(flags, i0, i1, i2)           prim_tri(asmblr, i0, i1, i2)
#define QUAD(flags, i0, i1, i2, i3)
#define PASS_QUADS

#include "draw_decompose_tmp.h"
