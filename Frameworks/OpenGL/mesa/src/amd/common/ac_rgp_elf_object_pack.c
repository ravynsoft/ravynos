/*
 * Copyright 2021 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * \file ac_rgp_elf_object_pack.c
 *
 * This file provides functions to create elf object for rgp profiling.
 * The functions in this file create 64bit elf code object irrespective
 * of if the driver is compiled as 32 or 64 bit.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libelf.h>
#include "ac_msgpack.h"
#include "ac_rgp.h"

#include "util/bitscan.h"
#include "util/u_math.h"

#ifndef EM_AMDGPU
// Old distributions may not have this enum constant
#define EM_AMDGPU 224
#endif

char hw_stage_string[RGP_HW_STAGE_MAX][4] = {
   ".vs",
   ".ls",
   ".hs",
   ".es",
   ".gs",
   ".ps",
   ".cs"
};

char hw_stage_symbol_string[RGP_HW_STAGE_MAX][16] = {
   "_amdgpu_vs_main",
   "_amdgpu_ls_main",
   "_amdgpu_hs_main",
   "_amdgpu_es_main",
   "_amdgpu_gs_main",
   "_amdgpu_ps_main",
   "_amdgpu_cs_main"
};

static const char *
get_api_stage_string(gl_shader_stage stage)
{
   switch (stage) {
   case MESA_SHADER_VERTEX:
      return".vertex";
   case MESA_SHADER_TESS_CTRL:
      return".hull";
   case MESA_SHADER_TESS_EVAL:
      return".domain";
   case MESA_SHADER_GEOMETRY:
      return".geometry";
   case MESA_SHADER_FRAGMENT:
      return".pixel";
   case MESA_SHADER_MESH:
      return ".mesh";
   case MESA_SHADER_TASK:
      return ".task";
   default:
      /* RT shaders are implemented using compute HW stages, so use ".compute"
         for any stage other than graphics stages */
      return".compute";
   }
}

static const char *
get_hw_stage_symbol(struct rgp_code_object_record *record, unsigned index)
{
   if (record->is_rt)
      return record->shader_data[index].rt_shader_name;
   else
      return hw_stage_symbol_string[record->shader_data[index].hw_stage];
}

static const char *
rt_subtype_from_stage(gl_shader_stage stage)
{
   switch (stage) {
   case MESA_SHADER_RAYGEN:
      return "RayGeneration";
   case MESA_SHADER_MISS:
      return "Miss";
   case MESA_SHADER_CLOSEST_HIT:
      return "ClosestHit";
   case MESA_SHADER_CALLABLE:
      return "Callable";
   case MESA_SHADER_INTERSECTION:
      return "Traversal";
   /* There are also AnyHit and Intersection subtypes, but on RADV
    * these are inlined into the traversal shader */
   default:
      return "Unknown";
   }
}

/**
 * rgp profiler requires data for few variables stored in msgpack format
 * in notes section. This function writes the data from
 * struct rgp_code_object_record to elf object in msgpack format.
 * for msgpack specification refer to
 * github.com/msgpack/msgpack/blob/master/spec.md
 */
static void
ac_rgp_write_msgpack(FILE *output,
                     struct rgp_code_object_record *record,
                     uint32_t *written_size)
{
   struct ac_msgpack msgpack;
   uint32_t num_shaders;
   uint32_t i;
   uint32_t mask;

   num_shaders = util_bitcount(record->shader_stages_mask);

   ac_msgpack_init(&msgpack);

   ac_msgpack_add_fixmap_op(&msgpack, 2);
      ac_msgpack_add_fixstr(&msgpack, "amdpal.version");
      ac_msgpack_add_fixarray_op(&msgpack, 2);
         ac_msgpack_add_uint(&msgpack, 2);
         ac_msgpack_add_uint(&msgpack, 1);

      ac_msgpack_add_fixstr(&msgpack, "amdpal.pipelines");
      ac_msgpack_add_fixarray_op(&msgpack, 1);
         ac_msgpack_add_fixmap_op(&msgpack, 6 + record->is_rt);

            /* 1
             * This not used in RGP but data needs to be present
             */
            ac_msgpack_add_fixstr(&msgpack, ".spill_threshold");
            ac_msgpack_add_uint(&msgpack, 0xffff);

            /* 2
             * This not used in RGP but data needs to be present
             */
            ac_msgpack_add_fixstr(&msgpack, ".user_data_limit");
            ac_msgpack_add_uint(&msgpack, 32);

            /* 3 */
            ac_msgpack_add_fixstr(&msgpack, ".shaders");
            ac_msgpack_add_fixmap_op(&msgpack, num_shaders);
               mask = record->shader_stages_mask;
               while(mask) {
                  i = u_bit_scan(&mask);
                  ac_msgpack_add_fixstr(&msgpack, get_api_stage_string(i));
                  ac_msgpack_add_fixmap_op(&msgpack, 2);
                  ac_msgpack_add_fixstr(&msgpack, ".api_shader_hash");
                  ac_msgpack_add_fixarray_op(&msgpack, 2);
                     ac_msgpack_add_uint(&msgpack,
                                         record->shader_data[i].hash[0]);
                     ac_msgpack_add_uint(&msgpack, 0);
                  ac_msgpack_add_fixstr(&msgpack, ".hardware_mapping");
                  ac_msgpack_add_fixarray_op(&msgpack, 1);
                     ac_msgpack_add_fixstr(&msgpack, hw_stage_string[
                                           record->shader_data[i].hw_stage]);
               }

            /* 4 */
            ac_msgpack_add_fixstr(&msgpack, ".hardware_stages");
            ac_msgpack_add_fixmap_op(&msgpack,
                                     record->num_shaders_combined);
               mask = record->shader_stages_mask;
               while(mask) {
                  i = u_bit_scan(&mask);

                  if (record->shader_data[i].is_combined)
                     continue;

                  ac_msgpack_add_fixstr(&msgpack, hw_stage_string[
                                        record->shader_data[i].hw_stage]);
                  ac_msgpack_add_fixmap_op(&msgpack, 6);
                     ac_msgpack_add_fixstr(&msgpack, ".entry_point");
                     ac_msgpack_add_fixstr(&msgpack, get_hw_stage_symbol(record, i));

                     ac_msgpack_add_fixstr(&msgpack, ".sgpr_count");
                     ac_msgpack_add_uint(&msgpack,
                                         record->shader_data[i].sgpr_count);

                     ac_msgpack_add_fixstr(&msgpack, ".vgpr_count");
                     ac_msgpack_add_uint(&msgpack,
                                         record->shader_data[i].vgpr_count);

                     ac_msgpack_add_fixstr(&msgpack, ".scratch_memory_size");
                     ac_msgpack_add_uint(&msgpack,
                                         record->shader_data[i].scratch_memory_size);

                     ac_msgpack_add_fixstr(&msgpack, ".wavefront_size");
                     ac_msgpack_add_uint(&msgpack,
                                         record->shader_data[i].wavefront_size);

                     ac_msgpack_add_fixstr(&msgpack, ".lds_size");
                     ac_msgpack_add_uint(&msgpack, record->shader_data[i].lds_size);
               }

            /* 5 */
            ac_msgpack_add_fixstr(&msgpack, ".internal_pipeline_hash");
            ac_msgpack_add_fixarray_op(&msgpack, 2);
               ac_msgpack_add_uint(&msgpack, record->pipeline_hash[0]);
               ac_msgpack_add_uint(&msgpack, record->pipeline_hash[1]);

            /* 6 */
            ac_msgpack_add_fixstr(&msgpack, ".api");
            ac_msgpack_add_fixstr(&msgpack, "Vulkan");

            if (record->is_rt) {
                  /* 7 */
                  ac_msgpack_add_fixstr(&msgpack, ".shader_functions");
                  ac_msgpack_add_fixmap_op(&msgpack, num_shaders);
                     mask = record->shader_stages_mask;
                     while (mask) {
                        i = u_bit_scan(&mask);
                        ac_msgpack_add_fixstr(&msgpack, record->shader_data[i].rt_shader_name);
                        ac_msgpack_add_fixmap_op(&msgpack, 7);
                           ac_msgpack_add_fixstr(&msgpack, ".stack_frame_size_in_bytes");
                           ac_msgpack_add_uint(&msgpack, record->shader_data[i].rt_stack_size);

                           ac_msgpack_add_fixstr(&msgpack, ".shader_subtype");
                           ac_msgpack_add_fixstr(&msgpack, rt_subtype_from_stage(i));
                           ac_msgpack_add_fixstr(&msgpack, ".api_shader_hash");
                           ac_msgpack_add_fixarray_op(&msgpack, 2);
                              ac_msgpack_add_uint(&msgpack, record->pipeline_hash[0]);
                              ac_msgpack_add_uint(&msgpack, record->pipeline_hash[1]);

                           ac_msgpack_add_fixstr(&msgpack, ".sgpr_count");
                           ac_msgpack_add_uint(&msgpack, record->shader_data[i].sgpr_count);

                           ac_msgpack_add_fixstr(&msgpack, ".vgpr_count");
                           ac_msgpack_add_uint(&msgpack, record->shader_data[i].vgpr_count);

                           ac_msgpack_add_fixstr(&msgpack, ".lds_size");
                           ac_msgpack_add_uint(&msgpack, record->shader_data[i].lds_size);

                           ac_msgpack_add_fixstr(&msgpack, ".scratch_memory_size");
                           ac_msgpack_add_uint(&msgpack, 
                                               record->shader_data[i].scratch_memory_size);
                     }
            }
   ac_msgpack_resize_if_required(&msgpack, 4 - (msgpack.offset % 4));
   msgpack.offset = ALIGN(msgpack.offset, 4);
   fwrite(msgpack.mem, 1, msgpack.offset, output);
   *written_size = msgpack.offset;
   ac_msgpack_destroy(&msgpack);
}


static uint32_t
get_lowest_shader(uint32_t *shader_stages_mask,
                  struct rgp_code_object_record *record,
                  struct rgp_shader_data **rgp_shader_data)
{
   uint32_t i, lowest = 0;
   uint32_t mask;
   uint64_t base_address = -1;

   if (*shader_stages_mask == 0)
      return false;

   mask = *shader_stages_mask;
   while(mask) {
      i = u_bit_scan(&mask);
      if (record->shader_data[i].is_combined) {
         *shader_stages_mask = *shader_stages_mask & ~((uint32_t)1 << i);
         continue;
      }
      if (base_address > record->shader_data[i].base_address) {
         lowest = i;
         base_address = record->shader_data[i].base_address;
      }
   }

   *shader_stages_mask = *shader_stages_mask & ~((uint32_t)1 << lowest);
   *rgp_shader_data = &record->shader_data[lowest];
   return true;
}

/**
 *  write the shader code into elf object in text section
 */
static void
ac_rgp_file_write_elf_text(FILE *output, uint32_t *elf_size_calc,
                           struct rgp_code_object_record *record,
                           uint32_t *text_size)
{
   struct rgp_shader_data *rgp_shader_data = NULL;
   struct rgp_shader_data *prev_rgp_shader_data = NULL;
   uint32_t symbol_offset = 0;
   uint32_t mask = record->shader_stages_mask;
   static bool warn_once = true;

   while(get_lowest_shader(&mask, record, &rgp_shader_data)) {
      if (prev_rgp_shader_data) {
         uint32_t code_offset = rgp_shader_data->base_address -
                                prev_rgp_shader_data->base_address;
         uint32_t gap_between_code = code_offset -
                                     prev_rgp_shader_data->code_size;
         symbol_offset += code_offset;
         if (gap_between_code > 0x10000 && warn_once) {
            fprintf(stderr, "Warning: shader code far from previous "
                            "(%d bytes apart). The rgp capture file "
                            "might be very large.\n", gap_between_code);
            warn_once = false;
         }

         fseek(output, gap_between_code, SEEK_CUR);
         *elf_size_calc += gap_between_code;
      }

      rgp_shader_data->elf_symbol_offset = symbol_offset;
      fwrite(rgp_shader_data->code, 1, rgp_shader_data->code_size, output);
      *elf_size_calc += rgp_shader_data->code_size;
      prev_rgp_shader_data = rgp_shader_data;
   }

   symbol_offset += rgp_shader_data->code_size;
   uint32_t aligned = ALIGN(symbol_offset, 256) - symbol_offset;
   fseek(output, aligned, SEEK_CUR);
   *elf_size_calc += aligned;
   *text_size = symbol_offset + aligned;
}

/*
 * hardcoded index for string table and text section in elf object.
 * While populating section header table, the index order should
 * be strictly followed.
 */
#define RGP_ELF_STRING_TBL_SEC_HEADER_INDEX 1
#define RGP_ELF_TEXT_SEC_HEADER_INDEX 2

/*
 * hardcode the string table so that is a single write to output.
 * the strings are in a structure so that it is easy to get the offset
 * of given string in string table.
 */
struct ac_rgp_elf_string_table {
   char null[sizeof("")];
   char strtab[sizeof(".strtab")];
   char text[sizeof(".text")];
   char symtab[sizeof(".symtab")];
   char note[sizeof(".note")];
   char vs_main[sizeof("_amdgpu_vs_main")];
   char ls_main[sizeof("_amdgpu_ls_main")];
   char hs_main[sizeof("_amdgpu_hs_main")];
   char es_main[sizeof("_amdgpu_es_main")];
   char gs_main[sizeof("_amdgpu_gs_main")];
   char ps_main[sizeof("_amdgpu_ps_main")];
   char cs_main[sizeof("_amdgpu_cs_main")];
};

struct ac_rgp_elf_string_table rgp_elf_strtab = {
   .null = "",
   .strtab = ".strtab",
   .text = ".text",
   .symtab = ".symtab",
   .note = ".note",
   .vs_main = "_amdgpu_vs_main",
   .ls_main = "_amdgpu_ls_main",
   .hs_main = "_amdgpu_hs_main",
   .es_main = "_amdgpu_es_main",
   .gs_main = "_amdgpu_gs_main",
   .ps_main = "_amdgpu_ps_main",
   .cs_main = "_amdgpu_cs_main",
};

uint32_t rgp_elf_hw_stage_string_offset[RGP_HW_STAGE_MAX] = {
   (uintptr_t)((struct ac_rgp_elf_string_table*)0)->vs_main,
   (uintptr_t)((struct ac_rgp_elf_string_table*)0)->ls_main,
   (uintptr_t)((struct ac_rgp_elf_string_table*)0)->hs_main,
   (uintptr_t)((struct ac_rgp_elf_string_table*)0)->es_main,
   (uintptr_t)((struct ac_rgp_elf_string_table*)0)->gs_main,
   (uintptr_t)((struct ac_rgp_elf_string_table*)0)->ps_main,
   (uintptr_t)((struct ac_rgp_elf_string_table*)0)->cs_main,
};


static void
ac_rgp_file_write_elf_symbol_table(FILE *output, uint32_t *elf_size_calc,
                                   struct rgp_code_object_record *record,
                                   uint32_t *symbol_table_size)
{
   Elf64_Sym elf_sym;
   uint32_t i;
   uint32_t mask = record->shader_stages_mask;

   memset(&elf_sym, 0x00, sizeof(elf_sym));
   fwrite(&elf_sym, 1, sizeof(elf_sym), output);

   uint32_t rt_name_offset = 0;

   while(mask) {
      i = u_bit_scan(&mask);
      if (record->shader_data[i].is_combined)
         continue;

      if (record->is_rt) {
         elf_sym.st_name = sizeof(rgp_elf_strtab) + rt_name_offset;
         rt_name_offset += strlen(record->shader_data[i].rt_shader_name) + 1;
      } else
         elf_sym.st_name = rgp_elf_hw_stage_string_offset[record->shader_data[i].hw_stage];
      elf_sym.st_info = STT_FUNC;
      elf_sym.st_other = 0x0;
      elf_sym.st_shndx = RGP_ELF_TEXT_SEC_HEADER_INDEX;
      elf_sym.st_value = record->shader_data[i].elf_symbol_offset;
      elf_sym.st_size = record->shader_data[i].code_size;
      fwrite(&elf_sym, 1, sizeof(elf_sym), output);
   }

   *symbol_table_size = (record->num_shaders_combined + 1)
                        * sizeof(elf_sym);
   *elf_size_calc += *symbol_table_size;
}


/* Below defines from from llvm project
 * llvm/includel/llvm/BinaryFormat/ELF.h
 */
#define ELFOSABI_AMDGPU_PAL 65
#define NT_AMDGPU_METADATA 32

uint8_t elf_ident[EI_NIDENT] = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3,
                                 ELFCLASS64, ELFDATA2LSB, EV_CURRENT,
                                 ELFOSABI_AMDGPU_PAL,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00 };

#define NOTE_MSGPACK_NAME "AMDGPU"
struct ac_rgp_elf_note_msgpack_hdr {
   Elf64_Nhdr  hdr;
   char name[sizeof(NOTE_MSGPACK_NAME)];
};

void
ac_rgp_file_write_elf_object(FILE *output, size_t file_elf_start,
                             struct rgp_code_object_record *record,
                             uint32_t *written_size, uint32_t flags)
{
   Elf64_Ehdr elf_hdr;
   Elf64_Shdr sec_hdr[5];
   uint32_t elf_size_calc;
   struct ac_rgp_elf_note_msgpack_hdr note_hdr;
   uint32_t text_size = 0;
   uint32_t symbol_table_size = 0;
   uint32_t msgpack_size = 0;
   size_t note_sec_start;
   uint32_t sh_offset;
   uint32_t strtab_size = sizeof(rgp_elf_strtab);

   /* Give space for header in file. It will be written to file at the end */
   fseek(output, sizeof(Elf64_Ehdr), SEEK_CUR);

   elf_size_calc = sizeof(Elf64_Ehdr);

   /* Initialize elf header */
   memcpy(&elf_hdr.e_ident, &elf_ident, EI_NIDENT);
   elf_hdr.e_type = ET_REL;
   elf_hdr.e_machine = EM_AMDGPU;
   elf_hdr.e_version = EV_CURRENT;
   elf_hdr.e_entry = 0;
   elf_hdr.e_flags = flags;
   elf_hdr.e_shstrndx = 1; /* string table entry is hardcoded to 1*/
   elf_hdr.e_phoff = 0;
   elf_hdr.e_shentsize = sizeof(Elf64_Shdr);
   elf_hdr.e_ehsize = sizeof(Elf64_Ehdr);
   elf_hdr.e_phentsize = 0;
   elf_hdr.e_phnum = 0;

   /* write hardcoded string table */
   fwrite(&rgp_elf_strtab, 1, sizeof(rgp_elf_strtab), output);
   if (record->is_rt) {
      uint32_t mask = record->shader_stages_mask;
      while (mask) {
         int i = u_bit_scan(&mask);

         char *name = record->shader_data[i].rt_shader_name;
         uint32_t name_len = strlen(name);

         fwrite(name, 1, name_len + 1, output);
         strtab_size += name_len + 1;
      }
   }
   elf_size_calc += strtab_size;

   /* write shader code as .text code */
   ac_rgp_file_write_elf_text(output, &elf_size_calc, record, &text_size);

   /* write symbol table */
   ac_rgp_file_write_elf_symbol_table(output, &elf_size_calc, record,
                                      &symbol_table_size);

   /* write .note */
   /* the .note section contains msgpack which stores variables */
   note_sec_start = file_elf_start + elf_size_calc;
   fseek(output, sizeof(struct ac_rgp_elf_note_msgpack_hdr), SEEK_CUR);
   ac_rgp_write_msgpack(output, record, &msgpack_size);
   note_hdr.hdr.n_namesz = sizeof(NOTE_MSGPACK_NAME);
   note_hdr.hdr.n_descsz = msgpack_size;
   note_hdr.hdr.n_type = NT_AMDGPU_METADATA;
   memcpy(note_hdr.name, NOTE_MSGPACK_NAME "\0",
          sizeof(NOTE_MSGPACK_NAME) + 1);
   fseek(output, note_sec_start, SEEK_SET);
   fwrite(&note_hdr, 1, sizeof(struct ac_rgp_elf_note_msgpack_hdr), output);
   fseek(output, 0, SEEK_END);
   elf_size_calc += (msgpack_size +
                     sizeof(struct ac_rgp_elf_note_msgpack_hdr));

   /* write section headers */
   sh_offset = elf_size_calc;
   memset(&sec_hdr[0], 0x00, sizeof(Elf64_Shdr) * 5);

   /* string table must be at index 1 as used in other places*/
   sec_hdr[1].sh_name = (uintptr_t)((struct ac_rgp_elf_string_table*)0)->strtab;
   sec_hdr[1].sh_type = SHT_STRTAB;
   sec_hdr[1].sh_offset = sizeof(Elf64_Ehdr);
   sec_hdr[1].sh_size = strtab_size;

   /* text must be at index 2 as used in other places*/
   sec_hdr[2].sh_name = (uintptr_t)((struct ac_rgp_elf_string_table*)0)->text;
   sec_hdr[2].sh_type = SHT_PROGBITS;
   sec_hdr[2].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
   sec_hdr[2].sh_offset = sec_hdr[1].sh_offset + sec_hdr[1].sh_size;
   sec_hdr[2].sh_size = text_size;
   sec_hdr[2].sh_addralign = 256;

   sec_hdr[3].sh_name = (uintptr_t)((struct ac_rgp_elf_string_table*)0)->symtab;
   sec_hdr[3].sh_type = SHT_SYMTAB;
   sec_hdr[3].sh_offset = sec_hdr[2].sh_offset +
                          align64(sec_hdr[2].sh_size, 256);
   sec_hdr[3].sh_size = symbol_table_size;
   sec_hdr[3].sh_link = RGP_ELF_STRING_TBL_SEC_HEADER_INDEX;
   sec_hdr[3].sh_addralign = 8;
   sec_hdr[3].sh_entsize = sizeof(Elf64_Sym);

   sec_hdr[4].sh_name = (uintptr_t)((struct ac_rgp_elf_string_table*)0)->note;
   sec_hdr[4].sh_type = SHT_NOTE;
   sec_hdr[4].sh_offset = sec_hdr[3].sh_offset + sec_hdr[3].sh_size;
   sec_hdr[4].sh_size = msgpack_size +
                        sizeof(struct ac_rgp_elf_note_msgpack_hdr);
   sec_hdr[4].sh_addralign = 4;
   fwrite(&sec_hdr, 1, sizeof(Elf64_Shdr) * 5, output);
   elf_size_calc += (sizeof(Elf64_Shdr) * 5);

   /* update and write elf header */
   elf_hdr.e_shnum = 5;
   elf_hdr.e_shoff = sh_offset;

   fseek(output, file_elf_start, SEEK_SET);
   fwrite(&elf_hdr, 1, sizeof(Elf64_Ehdr), output);
   fseek(output, 0, SEEK_END);

   *written_size = elf_size_calc;
}
