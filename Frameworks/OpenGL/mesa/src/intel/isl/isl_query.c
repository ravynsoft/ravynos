/*
 * Copyright © 2023 Intel Corporation
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
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#include <getopt.h>

#include "dev/intel_debug.h"
#include "dev/intel_device_info.h"
#include "isl/isl.h"

const struct debug_control usages_control[] = {
   { "rt",          ISL_SURF_USAGE_RENDER_TARGET_BIT, },
   { "depth",       ISL_SURF_USAGE_DEPTH_BIT, },
   { "stencil",     ISL_SURF_USAGE_STENCIL_BIT, },
   { "texture",     ISL_SURF_USAGE_TEXTURE_BIT, },
   { "cube",        ISL_SURF_USAGE_CUBE_BIT, },
   { "disable-aux", ISL_SURF_USAGE_DISABLE_AUX_BIT, },
   { "display",     ISL_SURF_USAGE_DISPLAY_BIT, },
   { "storage",     ISL_SURF_USAGE_STORAGE_BIT, },
   { "hiz",         ISL_SURF_USAGE_HIZ_BIT, },
   { "mcs",         ISL_SURF_USAGE_MCS_BIT, },
   { "ccs",         ISL_SURF_USAGE_CCS_BIT, },
   { "vertex",      ISL_SURF_USAGE_VERTEX_BUFFER_BIT, },
   { "index",       ISL_SURF_USAGE_INDEX_BUFFER_BIT, },
   { "constant",    ISL_SURF_USAGE_CONSTANT_BUFFER_BIT, },
   { "cpb",         ISL_SURF_USAGE_CPB_BIT, },
};

static void
print_help(const char *name)
{
   printf("Usage: %s [OPTION] ...\n", name);
   printf("Prints out ISL surface parameters.\n");
   printf("    --help                          Display this help and exit\n");
   printf("    -p, --platform NAME             Platform to use\n");
   printf("    -D, --dimension DIM             Dimension of the surface (1, 2, 3)\n");
   printf("    -f, --format FORMAT             Format of the surface (--list-formats to list them)\n");
   printf("    -w, --width WIDTH               Width of the surface\n");
   printf("    -h, --height HEIGHT             Height of the surface\n");
   printf("    -d, --depth DEPTH               Depth of the surface\n");
   printf("    -a, --array ARRAY               Array length of the surface\n");
   printf("    -s, --samples SAMPLES           Sample count of the surface\n");
   printf("    -l, --levels LEVELS             Miplevel count of the surface\n");
   printf("    -P, --pitch PITCH               Row pitch of the surface\n");
   printf("    -u, --usages USAGE1,USAGE2,...  Usage list of the surface\n");
   printf("        usages: ");
   for (uint32_t i = 0; i < ARRAY_SIZE(usages_control); i++)
      printf("%s%s", i == 0 ? "" : ", ", usages_control[i].string);
   printf("\n");
   printf("    -F, --list-formats              List format names\n");
   printf("\n");
}

int
main(int argc, char *argv[])
{
   int c, i;
   bool help = false;
   const struct option isl_opts[] = {
      { "help",         no_argument,       (int *) &help,               true },
      { "platform",     required_argument, NULL,                        'p' },
      { "dimension",    required_argument, NULL,                        'D' },
      { "format",       required_argument, NULL,                        'f' },
      { "width",        required_argument, NULL,                        'w' },
      { "height",       required_argument, NULL,                        'h' },
      { "depth",        required_argument, NULL,                        'd' },
      { "array",        required_argument, NULL,                        'a' },
      { "samples",      required_argument, NULL,                        's' },
      { "levels",       required_argument, NULL,                        'l' },
      { "pitch",        required_argument, NULL,                        'P' },
      { "usages",       required_argument, NULL,                        'u' },
      { "list-formats",       no_argument, NULL,                        'F' },
      { NULL,         0,                 NULL,                          0 }
   };

   const char *platform          = "tgl";
   enum isl_surf_dim dimension   = ISL_SURF_DIM_2D;
   enum isl_format format        = ISL_FORMAT_R8G8B8A8_UNORM;
   uint32_t width                = 1;
   uint32_t height               = 1;
   uint32_t depth                = 1;
   uint32_t array                = 1;
   uint32_t samples              = 1;
   uint32_t levels               = 1;
   uint32_t row_pitch            = 0;
   isl_surf_usage_flags_t usages = ISL_SURF_USAGE_RENDER_TARGET_BIT | ISL_SURF_USAGE_TEXTURE_BIT;

   i = 0;
   while ((c = getopt_long(argc, argv, "p:D:f:w:h:d:a:s:l:P:u:F", isl_opts, &i)) != -1) {
      switch (c) {
      case 'p':
         platform = optarg;
         break;
      case 'D':
         dimension = atoi(optarg) - 1;
         break;
      case 'f':
         for (uint32_t i = 0; i < ISL_NUM_FORMATS; i++) {
            if (isl_format_get_layout(i)->bpb != 0 &&
                strcmp(optarg, isl_format_get_short_name(i)) == 0) {
               format = i;
               break;
            }
         }
         break;
      case 'w':
         width = atoi(optarg);
         break;
      case 'h':
         height = atoi(optarg);
         break;
      case 'd':
         depth = atoi(optarg);
         break;
      case 'a':
         array = atoi(optarg);
         break;
      case 's':
         samples = atoi(optarg);
         break;
      case 'l':
         levels = atoi(optarg);
         break;
      case 'P':
         row_pitch = atoi(optarg);
         break;
      case 'u': {
         usages = parse_debug_string(optarg, usages_control);
         break;
      }
      case 'F':
         printf("Format list:\n");
         for (uint32_t i = 0; i < ISL_NUM_FORMATS; i++) {
            if (isl_format_get_layout(i)->bpb != 0)
               printf("   %s\n", isl_format_get_short_name(i));
         }
         exit(EXIT_SUCCESS);
         break;
      case '?':
         print_help(argv[0]);
         exit(EXIT_FAILURE);
      default:
         break;
      }
   }

   brw_process_intel_debug_variable();

   int pci_id = intel_device_name_to_pci_device_id(platform);
   if (pci_id == -1) {
      printf("Unknown platform: %s\n", platform);
      exit(EXIT_FAILURE);
   }

   struct intel_device_info devinfo;
   if (!intel_get_device_info_from_pci_id(pci_id, &devinfo)) {
      printf("Unable to identify devid=0x%x\n", pci_id);
      exit(EXIT_FAILURE);
   }

   struct isl_device isl_dev;
   isl_device_init(&isl_dev, &devinfo);

   struct isl_surf surf;
   bool ok = isl_surf_init(&isl_dev, &surf,
                           .format = format,
                           .dim = dimension,
                           .width = width,
                           .height = height,
                           .depth = depth,
                           .levels = levels,
                           .array_len = array,
                           .samples = samples,
                           .row_pitch_B = row_pitch,
                           .usage = usages,
                           .tiling_flags = ISL_TILING_ANY_MASK);
   if (!ok) {
      printf("Surface creation failed\n");
      exit(EXIT_FAILURE);
   }

#define _2d_vals(name) \
   name.w, name.h
#define _3d_vals(name) \
   name.w, name.h, name.d
#define _4d_vals(name) \
   name.w, name.h, name.d, name.a

   printf("Surface parameters:\n");
   printf("  dim:                 %ud\n", surf.dim + 1);
   printf("  dim_layout:          %u\n", surf.dim_layout);
   printf("  msaa_layout:         %u\n", surf.msaa_layout);
   printf("  tiling:              %s\n", isl_tiling_to_name(surf.tiling));
   printf("  format:              %s\n", isl_format_get_short_name(surf.format));
   printf("  img_align_el:        %ux%ux%u\n", _3d_vals(surf.image_alignment_el));
   printf("  logical_level0_px:   %ux%ux%ux%u\n", _4d_vals(surf.logical_level0_px));
   printf("  phys_level0_sa:      %ux%ux%ux%u\n", _4d_vals(surf.phys_level0_sa));
   printf("  levels:              %u\n", surf.levels);
   printf("  samples:             %ux\n", surf.samples);
   printf("  size_B:              %"PRIu64"\n", surf.size_B);
   printf("  alignment_B:         %u\n", surf.alignment_B);
   printf("  row_pitch_B:         %u\n", surf.row_pitch_B);
   printf("  array_pitch_el_rows: %u\n", surf.array_pitch_el_rows);

   struct isl_tile_info tile_info;
   isl_surf_get_tile_info(&surf, &tile_info);
   printf("  tile_info:\n");
   printf("    tiling:             %s\n", isl_tiling_to_name(tile_info.tiling));
   printf("    format_bpb:         %u\n", tile_info.format_bpb);
   printf("    logical_extent_el:  %ux%ux%ux%u\n", _4d_vals(tile_info.logical_extent_el));
   printf("    phys_extent_B:      %ux%u = %u\n",
          _2d_vals(tile_info.phys_extent_B),
          tile_info.phys_extent_B.w * tile_info.phys_extent_B.h);

   return EXIT_SUCCESS;
}
