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
#include <stdbool.h>

static bool bin_debug = false;
#define BIN_DEBUG bin_debug

#include "freedreno_gmem.c"

/* NOTE, non-interesting gmem keys (ie. things that are small enough to fit
 * in a single bin) are commented out, but retained for posterity.
 */
/* clang-format off */
static const struct gmem_key keys[] = {
   { .minx=0, .miny=0, .width=1536, .height=2048, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {1,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   /* manhattan: */
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
// { .minx=0, .miny=0, .width=64, .height=64, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
// { .minx=0, .miny=0, .width=32, .height=32, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
// { .minx=0, .miny=0, .width=16, .height=16, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
// { .minx=0, .miny=0, .width=8, .height=8, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
// { .minx=0, .miny=0, .width=4, .height=4, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
// { .minx=0, .miny=0, .width=2, .height=2, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
// { .minx=0, .miny=0, .width=1, .height=1, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=4, .cbuf_cpp = {4,4,4,4,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
// { .minx=0, .miny=0, .width=64, .height=64, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {2,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=1024, .height=1024, .gmem_page_align=1, .nr_cbufs=0, .cbuf_cpp = {0,0,0,0,0,0,0,0,}, .zsbuf_cpp = {2,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=0, .cbuf_cpp = {0,0,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
   { .minx=0, .miny=0, .width=960, .height=540, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=480, .height=270, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
// { .minx=0, .miny=0, .width=240, .height=135, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
// { .minx=0, .miny=0, .width=120, .height=67, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},

   /* trex: */
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {2,0,}},
   { .minx=0, .miny=0, .width=960, .height=540, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {2,0,0,0,0,0,0,0,}, .zsbuf_cpp = {2,0,}},
   { .minx=0, .miny=0, .width=1024, .height=1024, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {2,0,}},
// { .minx=0, .miny=0, .width=64, .height=64, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {2,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},

   /* supertuxkart: */
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {2,0,}},
   { .minx=0, .miny=0, .width=810, .height=810, .gmem_page_align=1, .nr_cbufs=2, .cbuf_cpp = {4,4,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
// { .minx=0, .miny=0, .width=405, .height=405, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {2,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=405, .height=405, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {8,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=810, .height=810, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {8,0,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
   { .minx=0, .miny=0, .width=810, .height=810, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
   { .minx=0, .miny=0, .width=810, .height=810, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=960, .height=540, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {2,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {8,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=960, .height=540, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {8,0,0,0,0,0,0,0,}, .zsbuf_cpp = {0,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=2, .cbuf_cpp = {4,4,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {8,0,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
   { .minx=0, .miny=0, .width=1920, .height=1080, .gmem_page_align=1, .nr_cbufs=1, .cbuf_cpp = {4,0,0,0,0,0,0,0,}, .zsbuf_cpp = {4,0,}},
};
/* clang-format on */

struct gpu_info {
   const char *name;
   uint32_t gpu_id;
   uint8_t gmem_page_align;
   uint32_t gmemsize_bytes;
};

#define SZ_128K 0x00020000
#define SZ_256K 0x00040000
#define SZ_512K 0x00080000
#define SZ_1M   0x00100000

/* keep sorted by gpu name: */
static const struct gpu_info gpu_infos[] = {
   {"a306", 307, 4, SZ_128K},
   {"a405", 405, 4, SZ_256K},
   {"a530", 530, 4, SZ_1M},
   {"a618", 618, 1, SZ_512K},
   {"a630", 630, 1, SZ_1M},
   {"a650", 630, 1, SZ_1M + SZ_128K},
};

static const struct option opts[] = {
   {.name = "gpu",     .has_arg = 1, NULL, 'g'},
   {.name = "help",    .has_arg = 0, NULL, 'h'},
   {.name = "verbose", .has_arg = 0, NULL, 'v'},
   {}};

static void
usage(void)
{
   fprintf(stderr, "Usage:\n\n"
                   "\tgmemtool [-hv] [-g GPU]\n\n"
                   "Options:\n"
                   "\t-g, --gpu=GPU   - use GMEM size/alignment/etc settings "
                   "for the specified GPU\n"
                   "\t-h, --help      - this usage message\n"
                   "\t-v, --verbose   - dump more verbose output\n"
                   "\n");
   fprintf(stderr, "Where GPU is one of:\n");
   for (int i = 0; i < ARRAY_SIZE(gpu_infos); i++)
      fprintf(stderr, "\t%s\n", gpu_infos[i].name);
   exit(2);
}

int
main(int argc, char **argv)
{
   const char *gpu_name = "a630";
   int c;

   while ((c = getopt_long(argc, argv, "g:hv", opts, NULL)) != -1) {
      switch (c) {
      case 'g':
         gpu_name = optarg;
         break;
      case 'v':
         bin_debug = true;
         break;
      case 'h':
      default:
         usage();
      }
   }

   const struct gpu_info *gpu_info = NULL;

   for (int i = 0; i < ARRAY_SIZE(gpu_infos); i++) {
      if (strcmp(gpu_name, gpu_infos[i].name) == 0) {
         gpu_info = &gpu_infos[i];
         break;
      }
   }

   if (!gpu_info) {
      printf("unrecognized gpu name: %s\n", gpu_name);
      usage();
   }

   struct fd_dev_id dev_id = {
         .gpu_id = gpu_info->gpu_id,
   };
   /* Setup a fake screen with enough GMEM related configuration
    * to make gmem_stateobj_init() happy:
    */
   struct fd_screen screen = {
      .dev_id = &dev_id,
      .gmemsize_bytes = gpu_info->gmemsize_bytes,
   };

   screen.info = fd_dev_info_raw(&dev_id);

   /* And finally run thru all the GMEM keys: */
   for (int i = 0; i < ARRAY_SIZE(keys); i++) {
      struct gmem_key key = keys[i];
      key.gmem_page_align = gpu_info->gmem_page_align;
      struct fd_gmem_stateobj *gmem = gmem_stateobj_init(&screen, &key);
      dump_gmem_state(gmem);

      assert((gmem->bin_w * gmem->nbins_x) >= key.width);
      assert((gmem->bin_h * gmem->nbins_y) >= key.height);
      assert(gmem->bin_w < screen.info->tile_max_w);
      assert(gmem->bin_h < screen.info->tile_max_h);

      ralloc_free(gmem);
   }

   return 0;
}
