/*
 * Copyright (C) 2021 Collabora, Ltd.
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
 *
 */

/*
 * Debug dump analyser for panfrost.  In case of a gpu crash/hang,
 * the coredump should be found in:
 *
 *    /sys/class/devcoredump/devcd<n>/data
 *
 * The crashdump will hang around for 5min, it can be cleared by writing to
 * the file, ie:
 *
 *    echo 1 > /sys/class/devcoredump/devcd<n>/data
 *
 * (the driver won't log any new crashdumps until the previous one is cleared
 * or times out after 5min)
 */

#include <endian.h>
#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <drm-uapi/panfrost_drm.h>

#include "decode.h"

/* Same as panfrost_dump_object_header, but with field
 * entries in host byte order
 */
struct panfrost_dump_object_header_ho {
   uint32_t magic;
   uint32_t type;
   uint32_t file_size;
   uint32_t file_offset;

   union {
      struct pan_reg_hdr_ho {
         uint64_t jc;
         uint32_t gpu_id;
         uint32_t major;
         uint32_t minor;
         uint64_t nbos;
      } reghdr;

      struct pan_bomap_hdr_ho {
         uint32_t valid;
         uint64_t iova;
         uint32_t data[2];
      } bomap;

      uint32_t sizer[496];
   };
};

#define MAX_BODUMP_FILENAME 32
#define GPU_PAGE_SIZE       4096

static bool
read_header(FILE *fp, struct panfrost_dump_object_header_ho *pdoh)
{
   /* Fields in the coredump file header structures
    * are found in little-endian order
    */
   struct panfrost_dump_object_header doh_le;
   size_t nr;

   nr = fread(&doh_le, 1, sizeof(struct panfrost_dump_object_header), fp);
   if (nr < sizeof(struct panfrost_dump_object_header)) {
      fprintf(stderr, "Wrong header read\n");
      return false;
   }

   /* Convert from little-endian to host byte order */
   pdoh->magic = le32toh(doh_le.magic);
   if (pdoh->magic != PANFROSTDUMP_MAGIC) {
      fprintf(stderr, "Wrong header magic\n");
      return false;
   }

   pdoh->type = le32toh(doh_le.type);
   pdoh->file_offset = le32toh(doh_le.file_offset);
   pdoh->file_size = le32toh(doh_le.file_size);

   switch (pdoh->type) {
   case PANFROSTDUMP_BUF_REG:
      pdoh->reghdr.jc = le64toh(doh_le.reghdr.jc);
      pdoh->reghdr.gpu_id = le32toh(doh_le.reghdr.gpu_id);
      pdoh->reghdr.major = le32toh(doh_le.reghdr.major);
      pdoh->reghdr.minor = le32toh(doh_le.reghdr.minor);
      pdoh->reghdr.nbos = le64toh(doh_le.reghdr.nbos);
      break;
   case PANFROSTDUMP_BUF_BO:
      pdoh->bomap.iova = le64toh(doh_le.bomap.iova);
      pdoh->bomap.valid = le32toh(doh_le.bomap.valid);
      pdoh->bomap.data[0] = le32toh(doh_le.bomap.data[0]);
      pdoh->bomap.data[1] = le32toh(doh_le.bomap.data[1]);
   }

   return true;
}

static bool
read_register(uint32_t *ro, uint32_t *rv, FILE *fp)
{
   /* Register pair we read form memory is
    * laid out in little-endian order
    */
   struct panfrost_dump_registers reg_le;
   size_t nr;

   nr = fread(&reg_le, 1, sizeof(reg_le), fp);
   if (nr < sizeof(reg_le)) {
      fprintf(stderr, "Wrong register read\n");
      return false;
   }

   *ro = le32toh(reg_le.reg);
   *rv = le32toh(reg_le.value);

   return true;
}

static bool
read_page_addr(uint64_t *phys_page, FILE *fp)
{
   uint64_t phys_addr_le;
   size_t nr;

   nr = fread(&phys_addr_le, 1, sizeof(uint64_t), fp);
   if (nr < sizeof(uint64_t)) {
      fprintf(stderr, "Wrong page address read\n");

      /* Skip over to the next address */
      if (fseek(fp, sizeof(uint64_t) - nr, SEEK_CUR)) {
         perror("fseek error");
         return false;
      }

      return false;
   }

   *phys_page = le64toh(phys_addr_le);

   return true;
}

/* Keeping these definitions as global/static shouldn't be
 * an issue because this tool will always be single-threaded
 */
static FILE *hdr_fp;
static FILE *data_fp;
static char **bos;
static uint32_t bo_num;

static void
cleanup(void)
{
   if (hdr_fp != NULL)
      fclose(hdr_fp);
   if (data_fp != NULL)
      fclose(data_fp);
   if (bos != NULL) {
      for (int k = 0; k < bo_num; k++)
         free(bos[k]);
      free(bos);
   }
}

static void
print_help(const char *progname, FILE *file)
{
   fprintf(file,
           "Usage: %s [OPTION] inputfile\n"
           "Decode Panfrost coredump file.\n\n"
           "    -h, --help             display this help and exit\n"
           "    -a, --addr             print BO physical addresses\n"
           "    -r, --regs             print Panfrost HW registers\n"
           "Example:\n"
           "    panfrostdump -a -r coredump.bin\n",
           progname);
}

int
main(int argc, char *argv[])
{
   struct panfrost_dump_object_header_ho doh;
   bool print_addr = false;
   bool print_reg = false;
   uint32_t gpu_id = 0;
   uint64_t jc = 0;
   size_t nbytes;
   int i, j, k, c;

   if (argc < 2) {
      printf("Pass a coredump file\n");
      return EXIT_FAILURE;
   }

   /* clang-format off */
   const struct option longopts[] = {
      { "addr", no_argument, (int *) &print_addr, true },
      { "regs", no_argument, (int *) &print_reg, true },
      { "help", no_argument, NULL, 'h' },
      { NULL, 0, NULL, 0 }
   };
   /* clang-format on */

   while ((c = getopt_long(argc, argv, "arh", longopts, NULL)) != -1) {
      switch (c) {
      case 'h':
         print_help(argv[0], stderr);
         return EXIT_SUCCESS;
      case 'a':
         print_addr = true;
         break;
      case 'r':
         print_reg = true;
         break;
      default:
         fprintf(stderr, "Unknown option\n");
         print_help(argv[0], stderr);
         return EXIT_FAILURE;
      }
   }

   i = j = k = 0;

   atexit(cleanup);
   struct pandecode_context *ctx = pandecode_create_context(false);

   hdr_fp = fopen(argv[optind], "r");
   if (!hdr_fp) {
      perror("failed to open file");
      return EXIT_FAILURE;
   }

   data_fp = fopen(argv[optind], "r");
   if (!data_fp) {
      perror("failed to open file");
      return EXIT_FAILURE;
   }

   /* Read register header */
   if (!read_header(hdr_fp, &doh))
      return EXIT_FAILURE;

   if (fseek(data_fp, doh.file_offset, SEEK_SET)) {
      perror("fseek error");
      return EXIT_FAILURE;
   }

   if (doh.type == PANFROSTDUMP_BUF_REG) {
      jc = doh.reghdr.jc;
      gpu_id = doh.reghdr.gpu_id;
      bo_num = doh.reghdr.nbos;

      bos = calloc(bo_num, sizeof(char *));
      if (!bos) {
         fprintf(stderr, "Failed to allocate memory for BO pointer array\n");
         return EXIT_FAILURE;
      }

      printf("JC: %" PRIX64 ", GPU_ID: %" PRIX32 "\n", jc, gpu_id);

      if (print_reg) {
         puts("GPU registers:");
         for (i = 0;
              i < (doh.file_size / sizeof(struct panfrost_dump_registers));
              i++) {
            uint32_t reg_offset;
            uint32_t reg_val;

            if (read_register(&reg_offset, &reg_val, data_fp))
               printf("0x%04X : 0x%08X\n", reg_offset, reg_val);
         }
      }
   }

   if (!read_header(hdr_fp, &doh))
      return EXIT_FAILURE;

   if (doh.type == PANFROSTDUMP_BUF_BOMAP) {
      uint32_t bomap_offset = doh.file_offset;

      if (!jc || !gpu_id) {
         fprintf(stderr, "Missing initial dump header\n");
         return EXIT_FAILURE;
      }

      if (!read_header(hdr_fp, &doh))
         return EXIT_FAILURE;

      while (doh.type != PANFROSTDUMP_BUF_TRAILER) {
         if (doh.bomap.valid) {
            if (fseek(data_fp, bomap_offset + doh.bomap.data[0], SEEK_SET)) {
               perror("fseek error");
               return EXIT_FAILURE;
            }

            if (print_addr) {
               printf("BO(%u) VA(%" PRIX64 ") SZ(%" PRIX32
                      ") page addresses:\n",
                      j, doh.bomap.iova, doh.file_size);

               for (k = 0; k < (doh.file_size / GPU_PAGE_SIZE); k++) {
                  uint64_t phys_addr;

                  if (!read_page_addr(&phys_addr, data_fp))
                     continue;

                  printf("%u: %" PRIX64 "\n", k, phys_addr);
               }
            }

            /* Copy the BO into external file */
            char bodump_filename[MAX_BODUMP_FILENAME];
            FILE *bodump;

            snprintf(bodump_filename, MAX_BODUMP_FILENAME, "bodump-%u.dump", j);

            if ((bodump = fopen(bodump_filename, "wb"))) {
               if (fseek(data_fp, doh.file_offset, SEEK_SET)) {
                  perror("fseek error");
                  return EXIT_FAILURE;
               }

               bos[j] = malloc(doh.file_size);
               if (!bos[j]) {
                  fprintf(stderr, "Failed to allocate memory for BO\n");
                  return EXIT_FAILURE;
               }

               fseek(data_fp, doh.file_offset, SEEK_SET);

               nbytes = fread(bos[j], 1, doh.file_size, data_fp);
               if (nbytes < doh.file_size) {
                  fprintf(stderr, "Read less than BO size: %u\n", errno);
                  return EXIT_FAILURE;
               }
               nbytes = fwrite(bos[j], 1, doh.file_size, bodump);
               if (nbytes < doh.file_size) {
                  fprintf(stderr, "Failed to write BO contents into file: %u\n",
                          errno);
                  return EXIT_FAILURE;
               }

               fclose(bodump);

               pandecode_inject_mmap(ctx, doh.bomap.iova, bos[j], doh.file_size,
                                     NULL);

            } else {
               perror("failed to open BO dump file");
            }
         } else {
            fprintf(stderr, "BO(%u) isn't valid\n", j);
         }

         if (!read_header(hdr_fp, &doh))
            return EXIT_FAILURE;

         j++;
      }
   } else {
      if (!read_header(hdr_fp, &doh))
         return EXIT_FAILURE;
   }

   if (doh.type != PANFROSTDUMP_BUF_TRAILER)
      fprintf(stderr, "Trailing header isn't right\n");

   pandecode_jc(ctx, jc, gpu_id);
   pandecode_destroy_context(ctx);

   fclose(data_fp);
   fclose(hdr_fp);
   data_fp = hdr_fp = NULL;

   return EXIT_SUCCESS;
}
