#include <stdlib.h>
#include <stdio.h>

#include "util/u_prim.h"

struct test_info {
   enum mesa_prim prim_type;
   uint32_t count;
   uint32_t expected;
};

struct test_info tests[] = {
   { MESA_PRIM_POINTS, 0, 0 },
   { MESA_PRIM_POINTS, 1, 1 },
   { MESA_PRIM_POINTS, 2, 2 },

   { MESA_PRIM_LINES, 0, 0 },
   { MESA_PRIM_LINES, 1, 2 },
   { MESA_PRIM_LINES, 2, 4 },

   { MESA_PRIM_TRIANGLES, 0, 0 },
   { MESA_PRIM_TRIANGLES, 1, 3 },
   { MESA_PRIM_TRIANGLES, 2, 6 },

   { MESA_PRIM_QUADS, 0, 0 },
   { MESA_PRIM_QUADS, 1, 4 },
   { MESA_PRIM_QUADS, 2, 8 },
};

int
main(int argc, char **argv)
{
   for(int i = 0; i < ARRAY_SIZE(tests); i++) {
      struct test_info *info = &tests[i];
      uint32_t n = u_vertices_for_prims(info->prim_type, info->count);
      if (n != info->expected) {
         printf("Failure! Expected %u vertices for %u x %s, but got %u.\n",
                info->expected, info->count, u_prim_name(info->prim_type), n);
         return 1;
      }
   }

   printf("Success!\n");
   return 0;
}
