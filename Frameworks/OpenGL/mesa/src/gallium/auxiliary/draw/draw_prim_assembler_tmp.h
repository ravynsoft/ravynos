#define FUNC_VARS                               \
   struct draw_assembler *asmblr,               \
   const struct draw_prim_info *input_prims,    \
   const struct draw_vertex_info *input_verts,  \
   unsigned start,                              \
   unsigned count

#define FUNC_ENTER                                                \
   /* declare more local vars */                                  \
   const enum mesa_prim prim = input_prims->prim;            \
   const unsigned prim_flags = input_prims->flags;                \
   const bool last_vertex_last = !asmblr->draw->rasterizer->flatshade_first;  \
   switch (prim) {                                                  \
   case MESA_PRIM_POLYGON:                                          \
      assert(!"unexpected primitive type in prim assembler"); \
      return;                                                       \
   default:                                                         \
      break;                                                        \
   }


#define PASS_QUADS
#define POINT(i0)                             prim_point(asmblr, i0)
#define LINE(flags, i0, i1)                   prim_line(asmblr, i0, i1)
#define TRIANGLE(flags, i0, i1, i2)           prim_tri(asmblr, i0, i1, i2)
#define QUAD(flags, i0, i1, i2, i3)           prim_quad(asmblr, i0, i1, i2, i3)

#include "draw_decompose_tmp.h"
