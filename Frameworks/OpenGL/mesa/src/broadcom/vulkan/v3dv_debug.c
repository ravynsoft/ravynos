/*
 * Copyright © 2019 Raspberry Pi Ltd
 *
 * based in part on radv_debug.h which is:
 * Copyright © 2017 Google.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include "v3dv_debug.h"
#include "unistd.h"

#include "v3dv_debug.h"
#include <unistd.h>

void
v3dv_print_spirv(const char *data, uint32_t size, FILE *fp)
{
   char path[] = "/tmp/fileXXXXXX";
   char line[2048], command[128];
   FILE *p;
   int fd;

   /* Dump the binary into a temporary file. */
   fd = mkstemp(path);
   if (fd < 0)
      return;

   if (write(fd, data, size) == -1)
      goto fail;

   sprintf(command, "spirv-dis %s", path);

   /* Disassemble using spirv-dis if installed. */
   p = popen(command, "r");
   if (p) {
      while (fgets(line, sizeof(line), p))
         fprintf(fp, "%s", line);
      pclose(p);
   }

 fail:
   close(fd);
   unlink(path);
}
