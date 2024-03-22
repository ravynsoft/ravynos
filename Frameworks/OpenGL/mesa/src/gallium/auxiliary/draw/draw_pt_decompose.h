#define LOCAL_VARS                           \
   char *verts = (char *) vertices;          \
   const bool quads_flatshade_last =         \
      draw->quads_always_flatshade_last;     \
   const bool last_vertex_last =             \
      !draw->rasterizer->flatshade_first;

#include "draw_decompose_tmp.h"
