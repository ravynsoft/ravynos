/*
 * Copyright © 2020 Google, Inc.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <getopt.h>
#include <inttypes.h>
#include <locale.h>
#include <stdlib.h>
#include <xf86drm.h>

#include "util/u_math.h"

#include "perfcntrs/freedreno_perfcntr.h"

#include "main.h"

static void
dump_float(void *buf, int sz)
{
   uint8_t *ptr = (uint8_t *)buf;
   uint8_t *end = ptr + sz - 3;
   int i = 0;

   while (ptr < end) {
      uint32_t d = 0;

      printf((i % 8) ? " " : "\t");

      d |= *(ptr++) << 0;
      d |= *(ptr++) << 8;
      d |= *(ptr++) << 16;
      d |= *(ptr++) << 24;

      printf("%8f", uif(d));

      if ((i % 8) == 7) {
         printf("\n");
      }

      i++;
   }

   if (i % 8) {
      printf("\n");
   }
}

static void
dump_hex(void *buf, int sz)
{
   uint8_t *ptr = (uint8_t *)buf;
   uint8_t *end = ptr + sz;
   int i = 0;

   while (ptr < end) {
      uint32_t d = 0;

      printf((i % 8) ? " " : "\t");

      d |= *(ptr++) << 0;
      d |= *(ptr++) << 8;
      d |= *(ptr++) << 16;
      d |= *(ptr++) << 24;

      printf("%08x", d);

      if ((i % 8) == 7) {
         printf("\n");
      }

      i++;
   }

   if (i % 8) {
      printf("\n");
   }
}

static const char *shortopts = "df:g:hp:";

static const struct option longopts[] = {
   {"disasm", no_argument, 0, 'd'},         {"file", required_argument, 0, 'f'},
   {"groups", required_argument, 0, 'g'},   {"help", no_argument, 0, 'h'},
   {"perfcntr", required_argument, 0, 'p'}, {0, 0, 0, 0}};

static void
usage(const char *name)
{
   printf(
      "Usage: %s [-dfgh]\n"
      "\n"
      "options:\n"
      "    -d, --disasm             print disassembled shader\n"
      "    -f, --file=FILE          read shader from file (instead of stdin)\n"
      "    -g, --groups=X,Y,Z       use specified group size\n"
      "    -h, --help               show this message\n"
      "    -p, --perfcntr=LIST      sample specified performance counters "
      "(comma\n"
      "                             separated list)\n",
      name);
}

/* performance counter description: */
static unsigned num_groups;
static const struct fd_perfcntr_group *groups;

/* Track enabled counters per group: */
static unsigned *enabled_counters;

static void
setup_counter(const char *name, struct perfcntr *c)
{
   for (int i = 0; i < num_groups; i++) {
      const struct fd_perfcntr_group *group = &groups[i];

      for (int j = 0; j < group->num_countables; j++) {
         const struct fd_perfcntr_countable *countable = &group->countables[j];

         if (strcmp(name, countable->name) != 0)
            continue;

         /*
          * Allocate a counter to use to monitor the requested countable:
          */
         if (enabled_counters[i] >= group->num_counters) {
            errx(-1, "Too many counters selected in group: %s", group->name);
         }

         unsigned idx = enabled_counters[i]++;
         const struct fd_perfcntr_counter *counter = &group->counters[idx];

         /*
          * And initialize the perfcntr struct, pulling together the info
          * about selected counter and countable, to simplify life for the
          * backend:
          */
         c->name = name;
         c->select_reg = counter->select_reg;
         c->counter_reg_lo = counter->counter_reg_lo;
         c->counter_reg_hi = counter->counter_reg_hi;
         c->selector = countable->selector;

         return;
      }
   }

   errx(-1, "could not find countable: %s", name);
}

static struct perfcntr *
parse_perfcntrs(const struct fd_dev_id *dev_id, const char *perfcntrstr,
                unsigned *num_perfcntrs)
{
   struct perfcntr *counters = NULL;
   char *cnames, *s;
   unsigned cnt = 0;

   groups = fd_perfcntrs(dev_id, &num_groups);
   enabled_counters = (uint32_t *) calloc(num_groups, sizeof(enabled_counters[0]));

   cnames = strdup(perfcntrstr);
   while ((s = strstr(cnames, ","))) {
      char *name = cnames;
      s[0] = '\0';
      cnames = &s[1];

      counters =
         (struct perfcntr *)realloc(counters, ++cnt * sizeof(counters[0]));
      setup_counter(name, &counters[cnt - 1]);
   }

   char *name = cnames;
   counters = (struct perfcntr *)realloc(counters, ++cnt * sizeof(counters[0]));
   setup_counter(name, &counters[cnt - 1]);

   *num_perfcntrs = cnt;

   return counters;
}

int
main(int argc, char **argv)
{
   FILE *in = stdin;
   const char *perfcntrstr = NULL;
   struct perfcntr *perfcntrs = NULL;
   unsigned num_perfcntrs = 0;
   bool disasm = false;
   uint32_t grid[3] = {0};
   int opt, ret;

   setlocale(LC_NUMERIC, "en_US.UTF-8");

   while ((opt = getopt_long_only(argc, argv, shortopts, longopts, NULL)) !=
          -1) {
      switch (opt) {
      case 'd':
         disasm = true;
         break;
      case 'f':
         in = fopen(optarg, "r");
         if (!in)
            err(1, "could not open '%s'", optarg);
         break;
      case 'g':
         ret = sscanf(optarg, "%u,%u,%u", &grid[0], &grid[1], &grid[2]);
         if (ret != 3) {
            usage(argv[0]);
            return -1;
         }
         break;
      case 'h':
         usage(argv[0]);
         return -1;
      case 'p':
         perfcntrstr = optarg;
         break;
      default:
         printf("unrecognized arg: %c\n", opt);
         usage(argv[0]);
         return -1;
      }
   }

   struct fd_device *dev = fd_device_open();
   struct fd_pipe *pipe = fd_pipe_new(dev, FD_PIPE_3D);

   const struct fd_dev_id *dev_id = fd_pipe_dev_id(pipe);

   printf("got gpu: %s\n", fd_dev_name(dev_id));

   struct backend *backend;
   switch (fd_dev_gen(dev_id)) {
   case 4:
      backend = a4xx_init(dev, dev_id);
      break;
   case 6:
      backend = a6xx_init<A6XX>(dev, dev_id);
      break;
   case 7:
      backend = a6xx_init<A7XX>(dev, dev_id);
      break;
   default:
      err(1, "unsupported gpu generation: a%uxx", fd_dev_gen(dev_id));
   }

   struct kernel *kernel = backend->assemble(backend, in);
   printf("localsize: %dx%dx%d\n", kernel->local_size[0], kernel->local_size[1],
          kernel->local_size[2]);
   for (int i = 0; i < kernel->num_bufs; i++) {
      printf("buf[%d]: size=%u\n", i, kernel->buf_sizes[i]);
      kernel->bufs[i] = fd_bo_new(dev, kernel->buf_sizes[i] * 4, 0, "buf[%d]", i);
   }

   if (disasm)
      backend->disassemble(kernel, stdout);

   if (grid[0] == 0)
      return 0;

   struct fd_submit *submit = fd_submit_new(pipe);

   if (perfcntrstr) {
      if (!backend->set_perfcntrs) {
         err(1, "performance counters not supported");
      }
      perfcntrs = parse_perfcntrs(dev_id, perfcntrstr, &num_perfcntrs);
      backend->set_perfcntrs(backend, perfcntrs, num_perfcntrs);
   }

   backend->emit_grid(kernel, grid, submit);

   struct fd_fence *fence = fd_submit_flush(submit, -1, false);

   fd_fence_flush(fence);
   fd_fence_del(fence);

   for (int i = 0; i < kernel->num_bufs; i++) {
      fd_bo_cpu_prep(kernel->bufs[i], pipe, FD_BO_PREP_READ);
      void *map = fd_bo_map(kernel->bufs[i]);

      printf("buf[%d]:\n", i);
      dump_hex(map, kernel->buf_sizes[i] * 4);
      dump_float(map, kernel->buf_sizes[i] * 4);
   }

   if (perfcntrstr) {
      uint64_t results[num_perfcntrs];
      backend->read_perfcntrs(backend, results);

      for (unsigned i = 0; i < num_perfcntrs; i++) {
         printf("%s:\t%'" PRIu64 "\n", perfcntrs[i].name, results[i]);
      }
   }

   return 0;
}
