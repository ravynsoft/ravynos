/*
 * Copyright (C) 2021 Collabora, Ltd.
 * Copyright (C) 2019 Ryan Houdek <Sonicadvance1@gmail.com>
 * Copyright (C) 2014 Rob Clark <robclark@freedesktop.org>
 * Copyright © 2015 Red Hat
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
#include <string.h>
#include "bifrost/disassemble.h"
#include "util/macros.h"
#include "valhall/disassemble.h"

unsigned gpu_id = 0x7212;
int verbose = 0;

#define BI_FOURCC(ch0, ch1, ch2, ch3)                                          \
   ((uint32_t)(ch0) | (uint32_t)(ch1) << 8 | (uint32_t)(ch2) << 16 |           \
    (uint32_t)(ch3) << 24)

static void
disassemble(const char *filename)
{
   FILE *fp = fopen(filename, "rb");
   assert(fp);

   fseek(fp, 0, SEEK_END);
   unsigned filesize = ftell(fp);
   rewind(fp);

   uint32_t *code = malloc(filesize);
   unsigned res = fread(code, 1, filesize, fp);
   if (res != filesize) {
      printf("Couldn't read full file\n");
   }

   fclose(fp);

   void *entrypoint = code;

   if (filesize && code[0] == BI_FOURCC('M', 'B', 'S', '2')) {
      for (int i = 0; i < filesize / 4; ++i) {
         if (code[i] != BI_FOURCC('O', 'B', 'J', 'C'))
            continue;

         unsigned size = code[i + 1];
         unsigned offset = i + 2;

         entrypoint = code + offset;
         filesize = size;
      }
   }

   if ((gpu_id >> 12) >= 9)
      disassemble_valhall(stdout, entrypoint, filesize, verbose);
   else
      disassemble_bifrost(stdout, entrypoint, filesize, verbose);

   free(code);
}

int
main(int argc, char **argv)
{
   int c;

   if (argc < 2) {
      printf("Pass a command\n");
      exit(1);
   }

   static struct option longopts[] = {
      {"id", optional_argument, NULL, 'i'},
      {"gpu", optional_argument, NULL, 'g'},
      {"verbose", no_argument, &verbose, 'v'},
      {NULL, 0, NULL, 0},
   };

   static struct {
      const char *name;
      unsigned major, minor;
   } gpus[] = {
      {"G71", 6, 0}, {"G72", 6, 2}, {"G51", 7, 0}, {"G76", 7, 1},
      {"G52", 7, 2}, {"G31", 7, 3}, {"G77", 9, 0}, {"G57", 9, 1},
      {"G78", 9, 2}, {"G57", 9, 3}, {"G68", 9, 4}, {"G78AE", 9, 5},
   };

   while ((c = getopt_long(argc, argv, "v:", longopts, NULL)) != -1) {

      switch (c) {
      case 'i':
         gpu_id = atoi(optarg);

         if (!gpu_id) {
            fprintf(stderr, "Expected GPU ID, got %s\n", optarg);
            return 1;
         }

         break;
      case 'g':
         gpu_id = 0;

         /* Compatibility with the Arm compiler */
         if (strncmp(optarg, "Mali-", 5) == 0)
            optarg += 5;

         for (unsigned i = 0; i < ARRAY_SIZE(gpus); ++i) {
            if (strcmp(gpus[i].name, optarg))
               continue;

            unsigned major = gpus[i].major;
            unsigned minor = gpus[i].minor;

            gpu_id = (major << 12) | (minor << 8);
            break;
         }

         if (!gpu_id) {
            fprintf(stderr, "Unknown GPU %s\n", optarg);
            return 1;
         }

         break;
      default:
         break;
      }
   }

   disassemble(argv[optind + 1]);
   return 0;
}
