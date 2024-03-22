/*
 * Copyright © 2018 Intel Corporation
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "compiler/brw_eu.h"
#include "dev/intel_device_info.h"
#include "util/u_dynarray.h"

enum opt_input_type {
   OPT_INPUT_BINARY,
   OPT_INPUT_C_LITERAL,
};

static enum opt_input_type input_type = OPT_INPUT_BINARY;

/* Return size of file in bytes pointed by fp */
static long
i965_disasm_get_file_size(FILE *fp)
{
   long size;

   fseek(fp, 0L, SEEK_END);
   size = ftell(fp);
   fseek(fp, 0L, SEEK_SET);

   return size;
}

/* Read hex file which should be in following format:
 * for example :
 *    { 0x00000000, 0x00000000, 0x00000000, 0x00000000 }
 */
static void *
i965_disasm_read_c_literal_file(FILE *fp, size_t *end)
{
   struct util_dynarray assembly = {};
   uint32_t temp[2];

   if (fscanf(fp, " { ") == EOF) {
      fprintf(stderr, "Couldn't find opening `{`\n");
      return NULL;
   }

   if (fscanf(fp, "0x%x , 0x%x", &temp[0], &temp[1]) == 2) {
      util_dynarray_append(&assembly, uint32_t, temp[0]);
      util_dynarray_append(&assembly, uint32_t, temp[1]);
   } else {
      fprintf(stderr, "Couldn't read hex values\n");
      return NULL;
   }

   while (fscanf(fp, " , 0x%x , 0x%x ", &temp[0], &temp[1]) == 2) {
      util_dynarray_append(&assembly, uint32_t, temp[0]);
      util_dynarray_append(&assembly, uint32_t, temp[1]);
   }

   if (fscanf(fp, "}") == EOF) {
      fprintf(stderr, "Couldn't find closing `}`\n");
      return NULL;
   }

   *end = assembly.size;
   return assembly.data;
}

static void *
i965_disasm_read_binary(FILE *fp, size_t *end)
{
   size_t size;
   void *assembly;

   long sz = i965_disasm_get_file_size(fp);
   if (sz < 0)
      return NULL;

   *end = (size_t)sz;
   if (!*end)
      return NULL;

   assembly = malloc(*end + 1);
   if (assembly == NULL)
      return NULL;

   size = fread(assembly, *end, 1, fp);
   if (!size) {
      free(assembly);
      return NULL;
   }
   return assembly;
}

static void
print_help(const char *progname, FILE *file)
{
   fprintf(file,
           "Usage: %s [OPTION]...\n"
           "Disassemble i965 instructions from binary file.\n\n"
           "      --help             display this help and exit\n"
           "      --input-path=PATH  read binary file from binary file PATH\n"
           "      --type=INPUT_TYPE  INPUT_TYPE can be 'bin' (default if omitted),\n"
           "                         'c_literal'.\n"
           "      --gen=platform     disassemble instructions for given \n"
           "                         platform (3 letter platform name)\n",
           progname);
}

int main(int argc, char *argv[])
{
   FILE *fp = NULL;
   void *assembly = NULL;
   char *file_path = NULL;
   size_t start = 0, end = 0;
   uint16_t pci_id = 0;
   int c;
   int result = EXIT_FAILURE;

   bool help = false;
   const struct option i965_disasm_opts[] = {
      { "help",          no_argument,       (int *) &help,      true },
      { "input-path",    required_argument, NULL,               'i' },
      { "type",          required_argument, NULL,               't' },
      { "gen",           required_argument, NULL,               'g'},
      { NULL,            0,                 NULL,                0 }
   };

   while ((c = getopt_long(argc, argv, ":i:t:g:h", i965_disasm_opts, NULL)) != -1) {
      switch (c) {
      case 'g': {
         const int id = intel_device_name_to_pci_device_id(optarg);
         if (id < 0) {
            fprintf(stderr, "can't parse gen: '%s', expected 3 letter "
                            "platform name\n", optarg);
            goto end;
         } else {
            pci_id = id;
         }
         break;
      }
      case 'i':
         file_path = strdup(optarg);
         fp = fopen(file_path, "r");
         if (!fp) {
            fprintf(stderr, "Unable to read input file : %s\n",
                    file_path);
            goto end;
         }
         break;
      case 't':
         if (strcmp(optarg, "c_literal") == 0) {
            input_type = OPT_INPUT_C_LITERAL;
         } else if (strcmp(optarg, "bin") == 0) {
            input_type = OPT_INPUT_BINARY;
         } else {
            fprintf(stderr, "invalid value for --type: %s\n", optarg);
            goto end;
         }
         break;
      case 'h':
         help = true;
         print_help(argv[0], stderr);
         goto end;
      case 0:
         break;
      case ':':
         fprintf(stderr, "%s: option `-%c' requires an argument\n",
                 argv[0], optopt);
         goto end;
      case '?':
      default:
         fprintf(stderr, "%s: option `-%c' is invalid: ignored\n",
                 argv[0], optopt);
         goto end;
      }
   }

   if (help || !file_path || !pci_id) {
      print_help(argv[0], stderr);
      exit(0);
   }

   struct intel_device_info devinfo;
   if (!intel_get_device_info_from_pci_id(pci_id, &devinfo)) {
      fprintf(stderr, "can't find device information: pci_id=0x%x\n", pci_id);
      exit(EXIT_FAILURE);
   }

   struct brw_isa_info isa;
   brw_init_isa_info(&isa, &devinfo);

   if (input_type == OPT_INPUT_BINARY)
      assembly = i965_disasm_read_binary(fp, &end);
   else if (input_type == OPT_INPUT_C_LITERAL)
      assembly = i965_disasm_read_c_literal_file(fp, &end);

   if (!assembly) {
      if (end)
        fprintf(stderr, "Unable to allocate buffer to read input file\n");
      else
        fprintf(stderr, "Failed to read input file\n");

      goto end;
   }

   /* Disassemble i965 instructions from buffer assembly */
   brw_disassemble_with_labels(&isa, assembly, start, end, stdout);

   result = EXIT_SUCCESS;

end:
   if (fp)
      fclose(fp);

   free(file_path);
   free(assembly);

   exit(result);
}
