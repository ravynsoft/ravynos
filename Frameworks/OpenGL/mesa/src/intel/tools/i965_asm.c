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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <stdio.h>
#include <getopt.h>
#include "i965_asm.h"

enum opt_output_type {
   OPT_OUTPUT_HEX,
   OPT_OUTPUT_C_LITERAL,
   OPT_OUTPUT_BIN,
};

extern FILE *yyin;
struct brw_codegen *p;
static enum opt_output_type output_type = OPT_OUTPUT_BIN;
char *input_filename = NULL;
int errors;

struct list_head instr_labels;
struct list_head target_labels;

static void
print_help(const char *progname, FILE *file)
{
   fprintf(file,
           "Usage: %s [OPTION] inputfile\n"
           "Assemble i965 instructions from input file.\n\n"
           "    -h, --help             display this help and exit\n"
           "    -t, --type=OUTPUT_TYPE OUTPUT_TYPE can be 'bin' (default if omitted),\n"
           "                           'c_literal', or 'hex'\n"
           "    -o, --output           specify output file\n"
           "        --compact          print compacted instructions\n"
           "    -g, --gen=platform     assemble instructions for given \n"
           "                           platform (3 letter platform name)\n"
           "Example:\n"
           "    i965_asm -g kbl input.asm -t hex -o output\n",
           progname);
}

static uint32_t
get_dword(const brw_inst *inst, int idx)
{
   uint32_t dword;
   memcpy(&dword, (char *)inst + 4 * idx, sizeof(dword));
   return dword;
}

static void
print_instruction(FILE *output, bool compact, const brw_inst *instruction)
{
   int byte_limit;

   byte_limit = (compact == true) ? 8 : 16;

   switch (output_type) {
   case OPT_OUTPUT_HEX: {
      fprintf(output, "%02x", ((unsigned char *)instruction)[0]);

      for (unsigned i = 1; i < byte_limit; i++) {
         fprintf(output, " %02x", ((unsigned char *)instruction)[i]);
      }
      break;
   }
   case OPT_OUTPUT_C_LITERAL: {
      fprintf(output, "\t0x%08x,", get_dword(instruction, 0));

      for (unsigned i = 1; i < byte_limit / 4; i++)
         fprintf(output, " 0x%08x,", get_dword(instruction, i));

      break;
   }
   case OPT_OUTPUT_BIN:
      fwrite(instruction, 1, byte_limit, output);
      break;
   }

   if (output_type != OPT_OUTPUT_BIN) {
      fprintf(output, "\n");
   }
}

static struct intel_device_info *
i965_disasm_init(uint16_t pci_id)
{
   struct intel_device_info *devinfo;

   devinfo = malloc(sizeof *devinfo);
   if (devinfo == NULL)
      return NULL;

   if (!intel_get_device_info_from_pci_id(pci_id, devinfo)) {
      fprintf(stderr, "can't find device information: pci_id=0x%x\n",
              pci_id);
      free(devinfo);
      return NULL;
   }

   return devinfo;
}

static bool
i965_postprocess_labels()
{
   if (p->devinfo->ver < 6) {
      return true;
   }

   void *store = p->store;

   struct target_label *tlabel;
   struct instr_label *ilabel, *s;

   const unsigned to_bytes_scale = brw_jump_scale(p->devinfo);

   LIST_FOR_EACH_ENTRY(tlabel, &target_labels, link) {
      LIST_FOR_EACH_ENTRY_SAFE(ilabel, s, &instr_labels, link) {
         if (!strcmp(tlabel->name, ilabel->name)) {
            brw_inst *inst = store + ilabel->offset;

            int relative_offset = (tlabel->offset - ilabel->offset) / sizeof(brw_inst);
            relative_offset *= to_bytes_scale;

            unsigned opcode = brw_inst_opcode(p->isa, inst);

            if (ilabel->type == INSTR_LABEL_JIP) {
               switch (opcode) {
               case BRW_OPCODE_IF:
               case BRW_OPCODE_ELSE:
               case BRW_OPCODE_ENDIF:
               case BRW_OPCODE_WHILE:
                  if (p->devinfo->ver >= 7) {
                     brw_inst_set_jip(p->devinfo, inst, relative_offset);
                  } else if (p->devinfo->ver == 6) {
                     brw_inst_set_gfx6_jump_count(p->devinfo, inst, relative_offset);
                  }
                  break;
               case BRW_OPCODE_BREAK:
               case BRW_OPCODE_HALT:
               case BRW_OPCODE_CONTINUE:
                  brw_inst_set_jip(p->devinfo, inst, relative_offset);
                  break;
               default:
                  fprintf(stderr, "Unknown opcode %d with JIP label\n", opcode);
                  return false;
               }
            } else {
               switch (opcode) {
               case BRW_OPCODE_IF:
               case BRW_OPCODE_ELSE:
                  if (p->devinfo->ver > 7) {
                     brw_inst_set_uip(p->devinfo, inst, relative_offset);
                  } else if (p->devinfo->ver == 7) {
                     brw_inst_set_uip(p->devinfo, inst, relative_offset);
                  } else if (p->devinfo->ver == 6) {
                     // Nothing
                  }
                  break;
               case BRW_OPCODE_WHILE:
               case BRW_OPCODE_ENDIF:
                  fprintf(stderr, "WHILE/ENDIF cannot have UIP offset\n");
                  return false;
               case BRW_OPCODE_BREAK:
               case BRW_OPCODE_CONTINUE:
               case BRW_OPCODE_HALT:
                  brw_inst_set_uip(p->devinfo, inst, relative_offset);
                  break;
               default:
                  fprintf(stderr, "Unknown opcode %d with UIP label\n", opcode);
                  return false;
               }
            }

            list_del(&ilabel->link);
         }
      }
   }

   LIST_FOR_EACH_ENTRY(ilabel, &instr_labels, link) {
      fprintf(stderr, "Unknown label '%s'\n", ilabel->name);
   }

   return list_is_empty(&instr_labels);
}

int main(int argc, char **argv)
{
   char *output_file = NULL;
   char c;
   FILE *output = stdout;
   bool help = false, compact = false;
   void *store;
   uint64_t pci_id = 0;
   int offset = 0, err;
   int start_offset = 0;
   struct disasm_info *disasm_info;
   struct intel_device_info *devinfo = NULL;
   int result = EXIT_FAILURE;
   list_inithead(&instr_labels);
   list_inithead(&target_labels);

   const struct option i965_asm_opts[] = {
      { "help",          no_argument,       (int *) &help,      true },
      { "type",          required_argument, NULL,               't' },
      { "gen",           required_argument, NULL,               'g' },
      { "output",        required_argument, NULL,               'o' },
      { "compact",       no_argument,       (int *) &compact,   true },
      { NULL,            0,                 NULL,               0 }
   };

   while ((c = getopt_long(argc, argv, ":t:g:o:h", i965_asm_opts, NULL)) != -1) {
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
      case 'h':
         help = true;
         print_help(argv[0], stderr);
         goto end;
      case 't': {
         if (strcmp(optarg, "hex") == 0) {
            output_type = OPT_OUTPUT_HEX;
         } else if (strcmp(optarg, "c_literal") == 0) {
            output_type = OPT_OUTPUT_C_LITERAL;
         } else if (strcmp(optarg, "bin") == 0) {
            output_type = OPT_OUTPUT_BIN;
         } else {
            fprintf(stderr, "invalid value for --type: %s\n", optarg);
            goto end;
         }
         break;
      }
      case 'o':
         output_file = strdup(optarg);
         break;
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

   if (help || !pci_id) {
      print_help(argv[0], stderr);
      goto end;
   }

   if (!argv[optind]) {
      fprintf(stderr, "Please specify input file\n");
      goto end;
   }

   input_filename = strdup(argv[optind]);
   yyin = fopen(input_filename, "r");
   if (!yyin) {
      fprintf(stderr, "Unable to read input file : %s\n",
              input_filename);
      goto end;
   }

   if (output_file) {
      output = fopen(output_file, "w");
      if (!output) {
         fprintf(stderr, "Couldn't open output file\n");
         goto end;
      }
   }

   devinfo = i965_disasm_init(pci_id);
   if (!devinfo) {
      fprintf(stderr, "Unable to allocate memory for "
                      "intel_device_info struct instance.\n");
      goto end;
   }

   struct brw_isa_info isa;
   brw_init_isa_info(&isa, devinfo);

   p = rzalloc(NULL, struct brw_codegen);
   brw_init_codegen(&isa, p, p);
   p->automatic_exec_sizes = false;

   err = yyparse();
   if (err || errors)
      goto end;

   if (!i965_postprocess_labels())
      goto end;

   store = p->store;

   disasm_info = disasm_initialize(p->isa, NULL);
   if (!disasm_info) {
      fprintf(stderr, "Unable to initialize disasm_info struct instance\n");
      goto end;
   }

   if (output_type == OPT_OUTPUT_C_LITERAL)
      fprintf(output, "{\n");

   brw_validate_instructions(p->isa, p->store, 0,
                             p->next_insn_offset, disasm_info);

   const int nr_insn = (p->next_insn_offset - start_offset) / 16;

   if (compact)
      brw_compact_instructions(p, start_offset, disasm_info);

   for (int i = 0; i < nr_insn; i++) {
      const brw_inst *insn = store + offset;
      bool compacted = false;

      if (compact && brw_inst_cmpt_control(p->devinfo, insn)) {
            offset += 8;
            compacted = true;
      } else {
            offset += 16;
      }

      print_instruction(output, compacted, insn);
   }

   ralloc_free(disasm_info);

   if (output_type == OPT_OUTPUT_C_LITERAL)
      fprintf(output, "}");

   result = EXIT_SUCCESS;
   goto end;

end:
   free(input_filename);
   free(output_file);

   if (yyin)
      fclose(yyin);

   if (output)
      fclose(output);

   if (p)
      ralloc_free(p);

   if (devinfo)
      free(devinfo);

   exit(result);
}
