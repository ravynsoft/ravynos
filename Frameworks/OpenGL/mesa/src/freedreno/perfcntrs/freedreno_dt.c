/*
 * Copyright © 2021 Google, Inc.
 * All Rights Reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <fcntl.h>
#include <ftw.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util/macros.h"
#include "util/os_file.h"

#include "freedreno_dt.h"

static struct {
   char *dtnode;
   int address_cells, size_cells;
   uint64_t base;
   uint32_t size;
   uint32_t min_freq;
   uint32_t max_freq;
} dev;

/*
 * code to find stuff in /proc/device-tree:
 *
 * NOTE: if we sampled the counters from the cmdstream, we could avoid needing
 * /dev/mem and /proc/device-tree crawling.  OTOH when the GPU is heavily loaded
 * we would be competing with whatever else is using the GPU.
 */

static void *
readdt(const char *node)
{
   char *path;
   void *buf;
   size_t sz;

   (void)asprintf(&path, "%s/%s", dev.dtnode, node);
   buf = os_read_file(path, &sz);
   free(path);

   return buf;
}

static int
find_freqs_fn(const char *fpath, const struct stat *sb, int typeflag,
              struct FTW *ftwbuf)
{
   const char *fname = fpath + ftwbuf->base;
   size_t sz;

   if (strcmp(fname, "qcom,gpu-freq") == 0) {
      uint32_t *buf = (uint32_t *)os_read_file(fpath, &sz);
      uint32_t freq = ntohl(buf[0]);
      free(buf);
      dev.max_freq = MAX2(dev.max_freq, freq);
      dev.min_freq = MIN2(dev.min_freq, freq);
   }

   return 0;
}

static void
find_freqs(void)
{
   char *path;

   dev.min_freq = ~0;
   dev.max_freq = 0;

   (void)asprintf(&path, "%s/%s", dev.dtnode, "qcom,gpu-pwrlevels");

   nftw(path, find_freqs_fn, 64, 0);

   free(path);
}

static const char *compatibles[] = {
   "qcom,adreno-3xx",
   "qcom,kgsl-3d0",
   "amd,imageon",
   "qcom,adreno",
};

/**
 * compatstrs is a list of compatible strings separated by null, ie.
 *
 *       compatible = "qcom,adreno-630.2", "qcom,adreno";
 *
 * would result in "qcom,adreno-630.2\0qcom,adreno\0"
 */
static bool
match_compatible(char *compatstrs, int sz)
{
   while (sz > 0) {
      char *compatible = compatstrs;

      for (unsigned i = 0; i < ARRAY_SIZE(compatibles); i++) {
         if (strcmp(compatible, compatibles[i]) == 0) {
            return true;
         }
      }

      compatstrs += strlen(compatible) + 1;
      sz -= strlen(compatible) + 1;
   }
   return false;
}

static int
find_device_fn(const char *fpath, const struct stat *sb, int typeflag,
               struct FTW *ftwbuf)
{
   const char *fname = fpath + ftwbuf->base;
   size_t sz;

   if (strcmp(fname, "compatible") == 0) {
      char *str = os_read_file(fpath, &sz);
      if (match_compatible(str, sz)) {
         int dlen = strlen(fpath) - strlen("/compatible");
         dev.dtnode = malloc(dlen + 1);
         memcpy(dev.dtnode, fpath, dlen);
         dev.dtnode[dlen] = '\0';
         printf("found dt node: %s\n", dev.dtnode);

         char buf[dlen + sizeof("/../#address-cells") + 1];
         size_t sz;
         int *val;

         sprintf(buf, "%s/../#address-cells", dev.dtnode);
         val = (int *)os_read_file(buf, &sz);
         dev.address_cells = ntohl(*val);
         free(val);

         sprintf(buf, "%s/../#size-cells", dev.dtnode);
         val = (int *)os_read_file(buf, &sz);
         dev.size_cells = ntohl(*val);
         free(val);

         printf("#address-cells=%d, #size-cells=%d\n", dev.address_cells,
                dev.size_cells);
      }
      free(str);
   }
   if (dev.dtnode) {
      /* we found it! */
      return 1;
   }
   return 0;
}

static bool
find_device(void)
{
   int ret;
   uint32_t *buf, *b;

   if (dev.dtnode)
      return true;

   ret = nftw("/proc/device-tree/", find_device_fn, 64, 0);
   if (ret < 0)
      return false;

   if (!dev.dtnode)
      return false;

   b = buf = readdt("reg");

   if (dev.address_cells == 2) {
      uint32_t u[2] = {ntohl(buf[0]), ntohl(buf[1])};
      dev.base = (((uint64_t)u[0]) << 32) | u[1];
      buf += 2;
   } else {
      dev.base = ntohl(buf[0]);
      buf += 1;
   }

   if (dev.size_cells == 2) {
      uint32_t u[2] = {ntohl(buf[0]), ntohl(buf[1])};
      dev.size = (((uint64_t)u[0]) << 32) | u[1];
      buf += 2;
   } else {
      dev.size = ntohl(buf[0]);
      buf += 1;
   }

   free(b);

   printf("i/o region at %08" PRIx64 " (size: %x)\n", dev.base, dev.size);

   find_freqs();

   printf("min_freq=%u, max_freq=%u\n", dev.min_freq, dev.max_freq);

   return true;
}

bool
fd_dt_find_freqs(uint32_t *min_freq, uint32_t *max_freq)
{
   if (!find_device())
      return false;

   *min_freq = dev.min_freq;
   *max_freq = dev.max_freq;

   return true;
}

void *
fd_dt_find_io(void)
{
   if (!find_device())
      return NULL;

   int fd = open("/dev/mem", O_RDWR | O_SYNC);
   if (fd < 0)
      return NULL;

   void *io =
      mmap(0, dev.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, dev.base);
   close(fd);
   if (io == MAP_FAILED)
      return NULL;

   return io;
}
