#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#include "util/u_math.h"
#include "util/half_float.h"
#include "util/u_cpu_detect.h"

static void
test(void)
{
   unsigned i;
   unsigned roundtrip_fails = 0;

   for(i = 0; i < 1 << 16; ++i)
   {
      uint16_t h = (uint16_t) i;
      union fi f;
      uint16_t rh;

      f.f = _mesa_half_to_float(h);
      rh = _mesa_float_to_half(f.f);

      if (h != rh && !(util_is_half_nan(h) && util_is_half_nan(rh))) {
         printf("Roundtrip failed: %x -> %x = %f -> %x\n", h, f.ui, f.f, rh);
         ++roundtrip_fails;
      }
   }

   if(roundtrip_fails) {
      printf("Failure! %u/65536 half floats failed a conversion to float and back.\n", roundtrip_fails);
      exit(1);
   }
}

int
main(int argc, char **argv)
{
   test();

   /* Test non-f16c. */
   if (util_get_cpu_caps()->has_f16c) {
      ((struct util_cpu_caps_t *)util_get_cpu_caps())->has_f16c = false;
      test();
   }

   printf("Success!\n");
   return 0;
}
