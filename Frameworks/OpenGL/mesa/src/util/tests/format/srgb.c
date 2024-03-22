#include <stdio.h>
#include <stdlib.h>

#include "util/macros.h"
#include "util/format/u_format.h"
#include "util/format/u_formats.h"

int main(void)
{
   for (enum pipe_format format = 0; format < PIPE_FORMAT_COUNT; format++)
   {
      if (!util_format_is_srgb(format)) {
         const enum pipe_format linear = util_format_linear(format);
         if (format != linear) {
            fprintf(stderr, "%s converted to linear is %s\n",
                    util_format_name(format),
                    util_format_name(linear));
            return EXIT_FAILURE;
         }
         continue;
      }

      const enum pipe_format linear = util_format_linear(format);
      if (format == linear) {
         fprintf(stderr, "%s can't be converted to a linear equivalent\n",
                 util_format_name(format));
         return EXIT_FAILURE;
      }

      const enum pipe_format srgb = util_format_srgb(linear);
      if (format != srgb) {
         fprintf(stderr, "%s converted to linear and back to srgb becomes %s\n",
                 util_format_name(format),
                 util_format_name(srgb));
         return EXIT_FAILURE;
      }
   }

   return EXIT_SUCCESS;
}
