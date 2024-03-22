/*
 * Copyright 2015 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "ac_debug.h"
#include "sid.h"
#include "sid_tables.h"

#include "util/compiler.h"
#include "util/u_debug.h"
#include "util/u_math.h"
#include "util/memstream.h"
#include "util/u_string.h"

#include <stdlib.h>

#ifdef HAVE_VALGRIND
#include <memcheck.h>
#include <valgrind.h>
#endif

DEBUG_GET_ONCE_BOOL_OPTION(color, "AMD_COLOR", true);

/* Parsed IBs are difficult to read without colors. Use "less -R file" to
 * read them, or use "aha -b -f file" to convert them to html.
 */
#define COLOR_RESET  "\033[0m"
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN   "\033[1;36m"
#define COLOR_PURPLE "\033[1;35m"

#define O_COLOR_RESET  (debug_get_option_color() ? COLOR_RESET : "")
#define O_COLOR_RED    (debug_get_option_color() ? COLOR_RED : "")
#define O_COLOR_GREEN  (debug_get_option_color() ? COLOR_GREEN : "")
#define O_COLOR_YELLOW (debug_get_option_color() ? COLOR_YELLOW : "")
#define O_COLOR_CYAN   (debug_get_option_color() ? COLOR_CYAN : "")
#define O_COLOR_PURPLE (debug_get_option_color() ? COLOR_PURPLE : "")

#define INDENT_PKT 8

struct ac_ib_parser {
   FILE *f;
   uint32_t *ib;
   unsigned num_dw;
   const int *trace_ids;
   unsigned trace_id_count;
   enum amd_gfx_level gfx_level;
   enum radeon_family family;
   ac_debug_addr_callback addr_callback;
   void *addr_callback_data;

   unsigned cur_dw;
};

static void parse_gfx_compute_ib(FILE *f, struct ac_ib_parser *ib);

static void print_spaces(FILE *f, unsigned num)
{
   fprintf(f, "%*s", num, "");
}

static void print_value(FILE *file, uint32_t value, int bits)
{
   /* Guess if it's int or float */
   if (value <= (1 << 15)) {
      if (value <= 9)
         fprintf(file, "%u\n", value);
      else
         fprintf(file, "%u (0x%0*x)\n", value, bits / 4, value);
   } else {
      float f = uif(value);

      if (fabs(f) < 100000 && f * 10 == floor(f * 10))
         fprintf(file, "%.1ff (0x%0*x)\n", f, bits / 4, value);
      else
         /* Don't print more leading zeros than there are bits. */
         fprintf(file, "0x%0*x\n", bits / 4, value);
   }
}

static void print_data_dword(FILE *file, uint32_t value, const char *comment)
{
   print_spaces(file, INDENT_PKT);
   fprintf(file, "(%s)\n", comment);
}

static void print_named_value(FILE *file, const char *name, uint32_t value, int bits)
{
   print_spaces(file, INDENT_PKT);
   fprintf(file, "%s%s%s <- ",
           O_COLOR_YELLOW, name,
           O_COLOR_RESET);
   print_value(file, value, bits);
}

static void print_string_value(FILE *file, const char *name, const char *value)
{
   print_spaces(file, INDENT_PKT);
   fprintf(file, "%s%s%s <- ",
           O_COLOR_YELLOW, name,
           O_COLOR_RESET);
   fprintf(file, "%s\n", value);
}

void ac_dump_reg(FILE *file, enum amd_gfx_level gfx_level, enum radeon_family family,
                 unsigned offset, uint32_t value, uint32_t field_mask)
{
   const struct si_reg *reg = ac_find_register(gfx_level, family, offset);

   if (reg) {
      const char *reg_name = sid_strings + reg->name_offset;
      bool first_field = true;

      print_spaces(file, INDENT_PKT);
      fprintf(file, "%s%s%s <- ",
              O_COLOR_YELLOW, reg_name,
              O_COLOR_RESET);

      if (!reg->num_fields) {
         print_value(file, value, 32);
         return;
      }

      for (unsigned f = 0; f < reg->num_fields; f++) {
         const struct si_field *field = sid_fields_table + reg->fields_offset + f;
         const int *values_offsets = sid_strings_offsets + field->values_offset;
         uint32_t val = (value & field->mask) >> (ffs(field->mask) - 1);

         if (!(field->mask & field_mask))
            continue;

         /* Indent the field. */
         if (!first_field)
            print_spaces(file, INDENT_PKT + strlen(reg_name) + 4);

         /* Print the field. */
         fprintf(file, "%s = ", sid_strings + field->name_offset);

         if (val < field->num_values && values_offsets[val] >= 0)
            fprintf(file, "%s\n", sid_strings + values_offsets[val]);
         else
            print_value(file, val, util_bitcount(field->mask));

         first_field = false;
      }
      return;
   }

   print_spaces(file, INDENT_PKT);
   fprintf(file, "%s0x%05x%s <- 0x%08x\n",
           O_COLOR_YELLOW, offset,
           O_COLOR_RESET, value);
}

static uint32_t ac_ib_get(struct ac_ib_parser *ib)
{
   uint32_t v = 0;

   if (ib->cur_dw < ib->num_dw) {
      v = ib->ib[ib->cur_dw];
#ifdef HAVE_VALGRIND
      /* Help figure out where garbage data is written to IBs.
       *
       * Arguably we should do this already when the IBs are written,
       * see RADEON_VALGRIND. The problem is that client-requests to
       * Valgrind have an overhead even when Valgrind isn't running,
       * and radeon_emit is performance sensitive...
       */
      if (VALGRIND_CHECK_VALUE_IS_DEFINED(v))
         fprintf(ib->f, "%sValgrind: The next DWORD is garbage%s\n",
                 debug_get_option_color() ? COLOR_RED : "", O_COLOR_RESET);
#endif
      fprintf(ib->f, "\n\035#%08x ", v);
   } else {
      fprintf(ib->f, "\n\035#???????? ");
   }

   ib->cur_dw++;
   return v;
}

static void ac_parse_set_reg_packet(FILE *f, unsigned count, unsigned reg_offset,
                                    struct ac_ib_parser *ib)
{
   unsigned reg_dw = ac_ib_get(ib);
   unsigned reg = ((reg_dw & 0xFFFF) << 2) + reg_offset;
   unsigned index = reg_dw >> 28;
   int i;

   if (index != 0)
      print_named_value(f, "INDEX", index, 32);

   for (i = 0; i < count; i++)
      ac_dump_reg(f, ib->gfx_level, ib->family, reg + i * 4, ac_ib_get(ib), ~0);
}

static void ac_parse_set_reg_pairs_packet(FILE *f, unsigned count, unsigned reg_base,
                                          struct ac_ib_parser *ib)
{
   for (unsigned i = 0; i < (count + 1) / 2; i++) {
      unsigned reg_offset = (ac_ib_get(ib) << 2) + reg_base;
      ac_dump_reg(f, ib->gfx_level, ib->family, reg_offset, ac_ib_get(ib), ~0);
   }
}

static void ac_parse_set_reg_pairs_packed_packet(FILE *f, unsigned count, unsigned reg_base,
                                                 struct ac_ib_parser *ib)
{
   unsigned reg_offset0 = 0, reg_offset1 = 0;

   print_named_value(f, "REG_COUNT", ac_ib_get(ib), 32);

   for (unsigned i = 0; i < count; i++) {
      if (i % 3 == 0) {
         unsigned tmp = ac_ib_get(ib);
         reg_offset0 = ((tmp & 0xffff) << 2) + reg_base;
         reg_offset1 = ((tmp >> 16) << 2) + reg_base;
      } else if (i % 3 == 1) {
         ac_dump_reg(f, ib->gfx_level, ib->family, reg_offset0, ac_ib_get(ib), ~0);
      } else {
         ac_dump_reg(f, ib->gfx_level, ib->family, reg_offset1, ac_ib_get(ib), ~0);
      }
   }
}

static void ac_parse_packet3(FILE *f, uint32_t header, struct ac_ib_parser *ib,
                             int *current_trace_id)
{
   unsigned first_dw = ib->cur_dw;
   int count = PKT_COUNT_G(header);
   unsigned op = PKT3_IT_OPCODE_G(header);
   const char *shader_type = PKT3_SHADER_TYPE_G(header) ? "(shader_type=compute)" : "";
   const char *predicated = PKT3_PREDICATE(header) ? "(predicated)" : "";
   const char *reset_filter_cam = PKT3_RESET_FILTER_CAM_G(header) ? "(reset_filter_cam)" : "";
   int i;
   unsigned tmp;

   /* Print the name first. */
   for (i = 0; i < ARRAY_SIZE(packet3_table); i++)
      if (packet3_table[i].op == op)
         break;

   char unknown_name[32];
   const char *pkt_name;

   if (i < ARRAY_SIZE(packet3_table)) {
      pkt_name = sid_strings + packet3_table[i].name_offset;
   } else {
      snprintf(unknown_name, sizeof(unknown_name), "UNKNOWN(0x%02X)", op);
      pkt_name = unknown_name;
   }
   const char *color;

   if (strstr(pkt_name, "DRAW") || strstr(pkt_name, "DISPATCH"))
      color = O_COLOR_PURPLE;
   else if (strstr(pkt_name, "SET") == pkt_name && strstr(pkt_name, "REG"))
      color = O_COLOR_CYAN;
   else if (i >= ARRAY_SIZE(packet3_table))
      color = O_COLOR_RED;
   else
      color = O_COLOR_GREEN;

   fprintf(f, "%s%s%s%s%s%s:\n", color, pkt_name, O_COLOR_RESET,
           shader_type, predicated, reset_filter_cam);

   /* Print the contents. */
   switch (op) {
   case PKT3_SET_CONTEXT_REG:
      ac_parse_set_reg_packet(f, count, SI_CONTEXT_REG_OFFSET, ib);
      break;
   case PKT3_SET_CONFIG_REG:
      ac_parse_set_reg_packet(f, count, SI_CONFIG_REG_OFFSET, ib);
      break;
   case PKT3_SET_UCONFIG_REG:
   case PKT3_SET_UCONFIG_REG_INDEX:
      ac_parse_set_reg_packet(f, count, CIK_UCONFIG_REG_OFFSET, ib);
      break;
   case PKT3_SET_SH_REG:
   case PKT3_SET_SH_REG_INDEX:
      ac_parse_set_reg_packet(f, count, SI_SH_REG_OFFSET, ib);
      break;
   case PKT3_SET_CONTEXT_REG_PAIRS:
      ac_parse_set_reg_pairs_packet(f, count, SI_CONTEXT_REG_OFFSET, ib);
      break;
   case PKT3_SET_SH_REG_PAIRS:
      ac_parse_set_reg_pairs_packet(f, count, SI_SH_REG_OFFSET, ib);
      break;
   case PKT3_SET_CONTEXT_REG_PAIRS_PACKED:
      ac_parse_set_reg_pairs_packed_packet(f, count, SI_CONTEXT_REG_OFFSET, ib);
      break;
   case PKT3_SET_SH_REG_PAIRS_PACKED:
   case PKT3_SET_SH_REG_PAIRS_PACKED_N:
      ac_parse_set_reg_pairs_packed_packet(f, count, SI_SH_REG_OFFSET, ib);
      break;
   case PKT3_ACQUIRE_MEM:
      if (ib->gfx_level >= GFX11) {
         if (G_585_PWS_ENA(ib->ib[ib->cur_dw + 5])) {
            ac_dump_reg(f, ib->gfx_level, ib->family, R_580_ACQUIRE_MEM_PWS_2, ac_ib_get(ib), ~0);
            print_named_value(f, "GCR_SIZE", ac_ib_get(ib), 32);
            print_named_value(f, "GCR_SIZE_HI", ac_ib_get(ib), 25);
            print_named_value(f, "GCR_BASE_LO", ac_ib_get(ib), 32);
            print_named_value(f, "GCR_BASE_HI", ac_ib_get(ib), 32);
            ac_dump_reg(f, ib->gfx_level, ib->family, R_585_ACQUIRE_MEM_PWS_7, ac_ib_get(ib), ~0);
            ac_dump_reg(f, ib->gfx_level, ib->family, R_586_GCR_CNTL, ac_ib_get(ib), ~0);
         } else {
            print_string_value(f, "ENGINE_SEL", ac_ib_get(ib) & 0x80000000 ? "ME" : "PFP");
            print_named_value(f, "GCR_SIZE", ac_ib_get(ib), 32);
            print_named_value(f, "GCR_SIZE_HI", ac_ib_get(ib), 25);
            print_named_value(f, "GCR_BASE_LO", ac_ib_get(ib), 32);
            print_named_value(f, "GCR_BASE_HI", ac_ib_get(ib), 32);
            print_named_value(f, "POLL_INTERVAL", ac_ib_get(ib), 16);
            ac_dump_reg(f, ib->gfx_level, ib->family, R_586_GCR_CNTL, ac_ib_get(ib), ~0);
         }
      } else {
         tmp = ac_ib_get(ib);
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0301F0_CP_COHER_CNTL, tmp, 0x7fffffff);
         print_string_value(f, "ENGINE_SEL", tmp & 0x80000000 ? "ME" : "PFP");
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0301F4_CP_COHER_SIZE, ac_ib_get(ib), ~0);
         ac_dump_reg(f, ib->gfx_level, ib->family, R_030230_CP_COHER_SIZE_HI, ac_ib_get(ib), ~0);
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0301F8_CP_COHER_BASE, ac_ib_get(ib), ~0);
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0301E4_CP_COHER_BASE_HI, ac_ib_get(ib), ~0);
         print_named_value(f, "POLL_INTERVAL", ac_ib_get(ib), 16);
         if (ib->gfx_level >= GFX10)
            ac_dump_reg(f, ib->gfx_level, ib->family, R_586_GCR_CNTL, ac_ib_get(ib), ~0);
      }
      break;
   case PKT3_SURFACE_SYNC:
      if (ib->gfx_level >= GFX7) {
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0301F0_CP_COHER_CNTL, ac_ib_get(ib), ~0);
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0301F4_CP_COHER_SIZE, ac_ib_get(ib), ~0);
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0301F8_CP_COHER_BASE, ac_ib_get(ib), ~0);
      } else {
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0085F0_CP_COHER_CNTL, ac_ib_get(ib), ~0);
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0085F4_CP_COHER_SIZE, ac_ib_get(ib), ~0);
         ac_dump_reg(f, ib->gfx_level, ib->family, R_0085F8_CP_COHER_BASE, ac_ib_get(ib), ~0);
      }
      print_named_value(f, "POLL_INTERVAL", ac_ib_get(ib), 16);
      break;
   case PKT3_EVENT_WRITE: {
      uint32_t event_dw = ac_ib_get(ib);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_028A90_VGT_EVENT_INITIATOR, event_dw,
                  S_028A90_EVENT_TYPE(~0));
      print_named_value(f, "EVENT_INDEX", (event_dw >> 8) & 0xf, 4);
      print_named_value(f, "INV_L2", (event_dw >> 20) & 0x1, 1);
      if (count > 0) {
         print_named_value(f, "ADDRESS_LO", ac_ib_get(ib), 32);
         print_named_value(f, "ADDRESS_HI", ac_ib_get(ib), 16);
      }
      break;
   }
   case PKT3_EVENT_WRITE_EOP: {
      uint32_t event_dw = ac_ib_get(ib);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_028A90_VGT_EVENT_INITIATOR, event_dw,
                  S_028A90_EVENT_TYPE(~0));
      print_named_value(f, "EVENT_INDEX", (event_dw >> 8) & 0xf, 4);
      print_named_value(f, "TCL1_VOL_ACTION_ENA", (event_dw >> 12) & 0x1, 1);
      print_named_value(f, "TC_VOL_ACTION_ENA", (event_dw >> 13) & 0x1, 1);
      print_named_value(f, "TC_WB_ACTION_ENA", (event_dw >> 15) & 0x1, 1);
      print_named_value(f, "TCL1_ACTION_ENA", (event_dw >> 16) & 0x1, 1);
      print_named_value(f, "TC_ACTION_ENA", (event_dw >> 17) & 0x1, 1);
      print_named_value(f, "ADDRESS_LO", ac_ib_get(ib), 32);
      uint32_t addr_hi_dw = ac_ib_get(ib);
      print_named_value(f, "ADDRESS_HI", addr_hi_dw, 16);
      print_named_value(f, "DST_SEL", (addr_hi_dw >> 16) & 0x3, 2);
      print_named_value(f, "INT_SEL", (addr_hi_dw >> 24) & 0x7, 3);
      print_named_value(f, "DATA_SEL", addr_hi_dw >> 29, 3);
      print_named_value(f, "DATA_LO", ac_ib_get(ib), 32);
      print_named_value(f, "DATA_HI", ac_ib_get(ib), 32);
      break;
   }
   case PKT3_RELEASE_MEM: {
      uint32_t event_dw = ac_ib_get(ib);
      if (ib->gfx_level >= GFX10) {
         ac_dump_reg(f, ib->gfx_level, ib->family, R_490_RELEASE_MEM_OP, event_dw, ~0u);
      } else {
         ac_dump_reg(f, ib->gfx_level, ib->family, R_028A90_VGT_EVENT_INITIATOR, event_dw,
                     S_028A90_EVENT_TYPE(~0));
         print_named_value(f, "EVENT_INDEX", (event_dw >> 8) & 0xf, 4);
         print_named_value(f, "TCL1_VOL_ACTION_ENA", (event_dw >> 12) & 0x1, 1);
         print_named_value(f, "TC_VOL_ACTION_ENA", (event_dw >> 13) & 0x1, 1);
         print_named_value(f, "TC_WB_ACTION_ENA", (event_dw >> 15) & 0x1, 1);
         print_named_value(f, "TCL1_ACTION_ENA", (event_dw >> 16) & 0x1, 1);
         print_named_value(f, "TC_ACTION_ENA", (event_dw >> 17) & 0x1, 1);
         print_named_value(f, "TC_NC_ACTION_ENA", (event_dw >> 19) & 0x1, 1);
         print_named_value(f, "TC_WC_ACTION_ENA", (event_dw >> 20) & 0x1, 1);
         print_named_value(f, "TC_MD_ACTION_ENA", (event_dw >> 21) & 0x1, 1);
      }
      uint32_t sel_dw = ac_ib_get(ib);
      print_named_value(f, "DST_SEL", (sel_dw >> 16) & 0x3, 2);
      print_named_value(f, "INT_SEL", (sel_dw >> 24) & 0x7, 3);
      print_named_value(f, "DATA_SEL", sel_dw >> 29, 3);
      print_named_value(f, "ADDRESS_LO", ac_ib_get(ib), 32);
      print_named_value(f, "ADDRESS_HI", ac_ib_get(ib), 32);
      print_named_value(f, "DATA_LO", ac_ib_get(ib), 32);
      print_named_value(f, "DATA_HI", ac_ib_get(ib), 32);
      print_named_value(f, "CTXID", ac_ib_get(ib), 32);
      break;
   }
   case PKT3_WAIT_REG_MEM:
      print_named_value(f, "OP", ac_ib_get(ib), 32);
      print_named_value(f, "ADDRESS_LO", ac_ib_get(ib), 32);
      print_named_value(f, "ADDRESS_HI", ac_ib_get(ib), 32);
      print_named_value(f, "REF", ac_ib_get(ib), 32);
      print_named_value(f, "MASK", ac_ib_get(ib), 32);
      print_named_value(f, "POLL_INTERVAL", ac_ib_get(ib), 16);
      break;
   case PKT3_DRAW_INDEX_AUTO:
      ac_dump_reg(f, ib->gfx_level, ib->family, R_030930_VGT_NUM_INDICES, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_0287F0_VGT_DRAW_INITIATOR, ac_ib_get(ib), ~0);
      break;
   case PKT3_DRAW_INDEX_2:
      ac_dump_reg(f, ib->gfx_level, ib->family, R_028A78_VGT_DMA_MAX_SIZE, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_0287E8_VGT_DMA_BASE, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_0287E4_VGT_DMA_BASE_HI, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_030930_VGT_NUM_INDICES, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_0287F0_VGT_DRAW_INITIATOR, ac_ib_get(ib), ~0);
      break;
   case PKT3_INDEX_TYPE:
      ac_dump_reg(f, ib->gfx_level, ib->family, R_028A7C_VGT_DMA_INDEX_TYPE, ac_ib_get(ib), ~0);
      break;
   case PKT3_NUM_INSTANCES:
      ac_dump_reg(f, ib->gfx_level, ib->family, R_030934_VGT_NUM_INSTANCES, ac_ib_get(ib), ~0);
      break;
   case PKT3_WRITE_DATA:
      ac_dump_reg(f, ib->gfx_level, ib->family, R_370_CONTROL, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_371_DST_ADDR_LO, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_372_DST_ADDR_HI, ac_ib_get(ib), ~0);
      while (ib->cur_dw <= first_dw + count)
          print_data_dword(f, ac_ib_get(ib), "data");
      break;
   case PKT3_CP_DMA:
      ac_dump_reg(f, ib->gfx_level, ib->family, R_410_CP_DMA_WORD0, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_411_CP_DMA_WORD1, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_412_CP_DMA_WORD2, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_413_CP_DMA_WORD3, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_415_COMMAND, ac_ib_get(ib), ~0);
      break;
   case PKT3_DMA_DATA:
      ac_dump_reg(f, ib->gfx_level, ib->family, R_501_DMA_DATA_WORD0, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_502_SRC_ADDR_LO, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_503_SRC_ADDR_HI, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_505_DST_ADDR_LO, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_506_DST_ADDR_HI, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_415_COMMAND, ac_ib_get(ib), ~0);
      break;
   case PKT3_INDIRECT_BUFFER_SI:
   case PKT3_INDIRECT_BUFFER_CONST:
   case PKT3_INDIRECT_BUFFER: {
      uint32_t base_lo_dw = ac_ib_get(ib);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_3F0_IB_BASE_LO, base_lo_dw, ~0);
      uint32_t base_hi_dw = ac_ib_get(ib);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_3F1_IB_BASE_HI, base_hi_dw, ~0);
      uint32_t control_dw = ac_ib_get(ib);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_3F2_IB_CONTROL, control_dw, ~0);

      if (!ib->addr_callback)
         break;

      uint64_t addr = ((uint64_t)base_hi_dw << 32) | base_lo_dw;
      void *data = ib->addr_callback(ib->addr_callback_data, addr);
      if (!data)
         break;

      if (G_3F2_CHAIN(control_dw)) {
         ib->ib = data;
         ib->num_dw = G_3F2_IB_SIZE(control_dw);
         ib->cur_dw = 0;
         return;
      }

      struct ac_ib_parser ib_recurse;
      memcpy(&ib_recurse, ib, sizeof(ib_recurse));
      ib_recurse.ib = data;
      ib_recurse.num_dw = G_3F2_IB_SIZE(control_dw);
      ib_recurse.cur_dw = 0;
      if (ib_recurse.trace_id_count) {
         if (*current_trace_id == *ib->trace_ids) {
            ++ib_recurse.trace_ids;
            --ib_recurse.trace_id_count;
         } else {
            ib_recurse.trace_id_count = 0;
         }
      }

      fprintf(f, "\n\035>------------------ nested begin ------------------\n");
      parse_gfx_compute_ib(f, &ib_recurse);
      fprintf(f, "\n\035<------------------- nested end -------------------\n");
      break;
   }
   case PKT3_CLEAR_STATE:
   case PKT3_INCREMENT_DE_COUNTER:
   case PKT3_PFP_SYNC_ME:
      print_data_dword(f, ac_ib_get(ib), "reserved");
      break;
   case PKT3_NOP:
      if (header == PKT3_NOP_PAD) {
         count = -1; /* One dword NOP. */
      } else if (count == 0 && ib->cur_dw < ib->num_dw && AC_IS_TRACE_POINT(ib->ib[ib->cur_dw])) {
         unsigned packet_id = AC_GET_TRACE_POINT_ID(ib->ib[ib->cur_dw]);

         print_spaces(f, INDENT_PKT);
         fprintf(f, "%sTrace point ID: %u%s\n", O_COLOR_RED, packet_id, O_COLOR_RESET);

         if (!ib->trace_id_count)
            break; /* tracing was disabled */

         *current_trace_id = packet_id;

         print_spaces(f, INDENT_PKT);
         if (packet_id < *ib->trace_ids) {
            fprintf(f, "%sThis trace point was reached by the CP.%s\n",
                    O_COLOR_RED, O_COLOR_RESET);
         } else if (packet_id == *ib->trace_ids) {
            fprintf(f, "%s!!!!! This is the last trace point that "
                                 "was reached by the CP !!!!!%s\n",
                    O_COLOR_RED, O_COLOR_RESET);
         } else if (packet_id + 1 == *ib->trace_ids) {
            fprintf(f, "%s!!!!! This is the first trace point that "
                                 "was NOT been reached by the CP !!!!!%s\n",
                    O_COLOR_RED, O_COLOR_RESET);
         } else {
            fprintf(f, "%s!!!!! This trace point was NOT reached "
                                 "by the CP !!!!!%s\n",
                    O_COLOR_RED, O_COLOR_RESET);
         }
      } else {
         while (ib->cur_dw <= first_dw + count)
             print_data_dword(f, ac_ib_get(ib), "unused");
      }
      break;
   case PKT3_DISPATCH_DIRECT:
      ac_dump_reg(f, ib->gfx_level, ib->family, R_00B804_COMPUTE_DIM_X, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_00B808_COMPUTE_DIM_Y, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_00B80C_COMPUTE_DIM_Z, ac_ib_get(ib), ~0);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_00B800_COMPUTE_DISPATCH_INITIATOR,
                  ac_ib_get(ib), ~0);
      break;
   case PKT3_DISPATCH_INDIRECT:
      print_named_value(f, "DATA_OFFSET", ac_ib_get(ib), 32);
      ac_dump_reg(f, ib->gfx_level, ib->family, R_00B800_COMPUTE_DISPATCH_INITIATOR,
                  ac_ib_get(ib), ~0);
      break;
   case PKT3_SET_BASE:
      tmp = ac_ib_get(ib);
      print_string_value(f, "BASE_INDEX", tmp == 1 ? "INDIRECT_BASE" : COLOR_RED "UNKNOWN" COLOR_RESET);
      break;
   case PKT3_PRIME_UTCL2:
      tmp = ac_ib_get(ib);
      print_named_value(f, "CACHE_PERM[rwx]", tmp & 0x7, 3);
      print_string_value(f, "PRIME_MODE", tmp & 0x8 ? "WAIT_FOR_XACK" : "DONT_WAIT_FOR_XACK");
      print_named_value(f, "ENGINE_SEL", tmp >> 30, 2);
      print_named_value(f, "ADDR_LO", ac_ib_get(ib), 32);
      print_named_value(f, "ADDR_HI", ac_ib_get(ib), 32);
      print_named_value(f, "REQUESTED_PAGES", ac_ib_get(ib), 14);
      break;
   case PKT3_ATOMIC_MEM:
      tmp = ac_ib_get(ib);
      print_named_value(f, "ATOMIC", tmp & 0x7f, 7);
      print_named_value(f, "COMMAND", (tmp >> 8) & 0xf, 4);
      print_named_value(f, "CACHE_POLICY", (tmp >> 25) & 0x3, 2);
      print_named_value(f, "ENGINE_SEL", tmp >> 30, 2);
      print_named_value(f, "ADDR_LO", ac_ib_get(ib), 32);
      print_named_value(f, "ADDR_HI", ac_ib_get(ib), 32);
      print_named_value(f, "SRC_DATA_LO", ac_ib_get(ib), 32);
      print_named_value(f, "SRC_DATA_HI", ac_ib_get(ib), 32);
      print_named_value(f, "CMP_DATA_LO", ac_ib_get(ib), 32);
      print_named_value(f, "CMP_DATA_HI", ac_ib_get(ib), 32);
      print_named_value(f, "LOOP_INTERVAL", ac_ib_get(ib) & 0x1fff, 13);
      break;
   }

   /* print additional dwords */
   while (ib->cur_dw <= first_dw + count)
      ac_ib_get(ib);

   if (ib->cur_dw > first_dw + count + 1)
      fprintf(f, "%s !!!!! count in header too low !!!!!%s\n",
              O_COLOR_RED, O_COLOR_RESET);
}

/**
 * Parse and print an IB into a file.
 */
static void parse_gfx_compute_ib(FILE *f, struct ac_ib_parser *ib)
{
   int current_trace_id = -1;

   while (ib->cur_dw < ib->num_dw) {
      uint32_t header = ac_ib_get(ib);
      unsigned type = PKT_TYPE_G(header);

      switch (type) {
      case 3:
         ac_parse_packet3(f, header, ib, &current_trace_id);
         break;
      case 2:
         /* type-2 nop */
         if (header == 0x80000000) {
            fprintf(f, "%sNOP (type 2)%s\n",
                    O_COLOR_GREEN, O_COLOR_RESET);
            break;
         }
         FALLTHROUGH;
      default:
         fprintf(f, "Unknown packet type %i\n", type);
         break;
      }
   }
}

static void format_ib_output(FILE *f, char *out)
{
   unsigned depth = 0;

   for (;;) {
      char op = 0;

      if (out[0] == '\n' && out[1] == '\035')
         out++;
      if (out[0] == '\035') {
         op = out[1];
         out += 2;
      }

      if (op == '<')
         depth--;

      unsigned indent = 4 * depth;
      if (op != '#')
         indent += 9;

      if (indent)
         print_spaces(f, indent);

      char *end = strchrnul(out, '\n');
      fwrite(out, end - out, 1, f);
      fputc('\n', f); /* always end with a new line */
      if (!*end)
         break;

      out = end + 1;

      if (op == '>')
         depth++;
   }
}

static void parse_sdma_ib(FILE *f, struct ac_ib_parser *ib)
{
   while (ib->cur_dw < ib->num_dw) {
      const uint32_t header = ac_ib_get(ib);
      const uint32_t opcode = header & 0xff;
      const uint32_t sub_op = (header >> 8) & 0xff;

      switch (opcode) {
      case SDMA_OPCODE_NOP: {
         fprintf(f, "NOP\n");

         const uint32_t count = header >> 16;
         for (unsigned i = 0; i < count; ++i) {
            ac_ib_get(ib);
            fprintf(f, "\n");
         }
         break;
      }
      case SDMA_OPCODE_CONSTANT_FILL: {
         fprintf(f, "CONSTANT_FILL\n");
         ac_ib_get(ib);
         fprintf(f, "\n");
         ac_ib_get(ib);
         fprintf(f, "\n");
         uint32_t value = ac_ib_get(ib);
         fprintf(f, "    fill value = %u\n", value);
         uint32_t byte_count = ac_ib_get(ib) + 1;
         fprintf(f, "    fill byte count = %u\n", byte_count);

         unsigned dwords = byte_count / 4;
         for (unsigned i = 0; i < dwords; ++i) {
            ac_ib_get(ib);
            fprintf(f, "\n");
         }

         break;
      }
      case SDMA_OPCODE_WRITE: {
         fprintf(f, "WRITE\n");

         /* VA */
         ac_ib_get(ib);
         fprintf(f, "\n");
         ac_ib_get(ib);
         fprintf(f, "\n");

         uint32_t dwords = ac_ib_get(ib) + 1;
         fprintf(f, "    written dword count = %u\n", dwords);

         for (unsigned i = 0; i < dwords; ++i) {
            ac_ib_get(ib);
            fprintf(f, "\n");
         }

         break;
      }
      case SDMA_OPCODE_COPY: {
         switch (sub_op) {
         case SDMA_COPY_SUB_OPCODE_LINEAR: {
            fprintf(f, "COPY LINEAR\n");

            uint32_t copy_bytes = ac_ib_get(ib) + (ib->gfx_level >= GFX9 ? 1 : 0);
            fprintf(f, "    copy bytes: %u\n", copy_bytes);
            ac_ib_get(ib);
            fprintf(f, "\n");
            ac_ib_get(ib);
            fprintf(f, "    src VA low\n");
            ac_ib_get(ib);
            fprintf(f, "    src VA high\n");
            ac_ib_get(ib);
            fprintf(f, "    dst VA low\n");
            ac_ib_get(ib);
            fprintf(f, "    dst VA high\n");

            break;
         }
         case SDMA_COPY_SUB_OPCODE_LINEAR_SUB_WINDOW: {
            fprintf(f, "COPY LINEAR_SUB_WINDOW\n");

            for (unsigned i = 0; i < 12; ++i) {
               ac_ib_get(ib);
               fprintf(f, "\n");
            }
            break;
         }
         case SDMA_COPY_SUB_OPCODE_TILED_SUB_WINDOW: {
            fprintf(f, "COPY TILED_SUB_WINDOW %s\n", header >> 31 ? "t2l" : "l2t");
            uint32_t dcc = (header >> 19) & 1;

            /* Tiled VA */
            ac_ib_get(ib);
            fprintf(f, "    tiled VA low\n");
            ac_ib_get(ib);
            fprintf(f, "    tiled VA high\n");

            uint32_t dw3 = ac_ib_get(ib);
            fprintf(f, "    tiled offset x = %u, y=%u\n", dw3 & 0xffff, dw3 >> 16);
            uint32_t dw4 = ac_ib_get(ib);
            fprintf(f, "    tiled offset z = %u, tiled width = %u\n", dw4 & 0xffff, (dw4 >> 16) + 1);
            uint32_t dw5 = ac_ib_get(ib);
            fprintf(f, "    tiled height = %u, tiled depth = %u\n", (dw5 & 0xffff) + 1, (dw5 >> 16) + 1);

            /* Tiled image info */
            ac_ib_get(ib);
            fprintf(f, "    (tiled image info)\n");

            /* Linear VA */
            ac_ib_get(ib);
            fprintf(f, "    linear VA low\n");
            ac_ib_get(ib);
            fprintf(f, "    linear VA high\n");

            uint32_t dw9 = ac_ib_get(ib);
            fprintf(f, "    linear offset x = %u, y=%u\n", dw9 & 0xffff, dw9 >> 16);
            uint32_t dw10 = ac_ib_get(ib);
            fprintf(f, "    linear offset z = %u, linear pitch = %u\n", dw10 & 0xffff, (dw10 >> 16) + 1);
            uint32_t dw11 = ac_ib_get(ib);
            fprintf(f, "    linear slice pitch = %u\n", dw11 + 1);
            uint32_t dw12 = ac_ib_get(ib);
            fprintf(f, "    copy width = %u, copy height = %u\n", (dw12 & 0xffff) + 1, (dw12 >> 16) + 1);
            uint32_t dw13 = ac_ib_get(ib);
            fprintf(f, "    copy depth = %u\n", dw13 + 1);

            if (dcc) {
               ac_ib_get(ib);
               fprintf(f, "    metadata VA low\n");
               ac_ib_get(ib);
               fprintf(f, "    metadata VA high\n");
               ac_ib_get(ib);
               fprintf(f, "    (metadata config)\n");
            }
            break;
         }
         case SDMA_COPY_SUB_OPCODE_T2T_SUB_WINDOW: {
            fprintf(f, "COPY T2T_SUB_WINDOW\n");
            uint32_t dcc = (header >> 19) & 1;

            for (unsigned i = 0; i < 14; ++i) {
               ac_ib_get(ib);
               fprintf(f, "\n");
            }

            if (dcc) {
               ac_ib_get(ib);
               fprintf(f, "    metadata VA low\n");
               ac_ib_get(ib);
               fprintf(f, "    metadata VA high\n");
               ac_ib_get(ib);
               fprintf(f, "    (metadata config)\n");
            }
            break;
         }
         default:
            fprintf(f, "(unrecognized COPY sub op)\n");
            break;
         }
         break;
      }
      default:
         fprintf(f, " (unrecognized opcode)\n");
         break;
      }
   }
}

/**
 * Parse and print an IB into a file.
 *
 * \param f            file
 * \param ib_ptr       IB
 * \param num_dw       size of the IB
 * \param gfx_level   gfx level
 * \param family	chip family
 * \param ip_type IP type
 * \param trace_ids	the last trace IDs that are known to have been reached
 *			and executed by the CP, typically read from a buffer
 * \param trace_id_count The number of entries in the trace_ids array.
 * \param addr_callback Get a mapped pointer of the IB at a given address. Can
 *                      be NULL.
 * \param addr_callback_data user data for addr_callback
 */
void ac_parse_ib_chunk(FILE *f, uint32_t *ib_ptr, int num_dw, const int *trace_ids,
                       unsigned trace_id_count, enum amd_gfx_level gfx_level,
                       enum radeon_family family, enum amd_ip_type ip_type,
                       ac_debug_addr_callback addr_callback, void *addr_callback_data)
{
   struct ac_ib_parser ib = {0};
   ib.ib = ib_ptr;
   ib.num_dw = num_dw;
   ib.trace_ids = trace_ids;
   ib.trace_id_count = trace_id_count;
   ib.gfx_level = gfx_level;
   ib.family = family;
   ib.addr_callback = addr_callback;
   ib.addr_callback_data = addr_callback_data;

   char *out;
   size_t outsize;
   struct u_memstream mem;
   u_memstream_open(&mem, &out, &outsize);
   FILE *const memf = u_memstream_get(&mem);
   ib.f = memf;

   if (ip_type == AMD_IP_GFX || ip_type == AMD_IP_COMPUTE)
      parse_gfx_compute_ib(memf, &ib);
   else if (ip_type == AMD_IP_SDMA)
      parse_sdma_ib(memf, &ib);
   else
      unreachable("unsupported IP type");

   u_memstream_close(&mem);

   if (out) {
      format_ib_output(f, out);
      free(out);
   }

   if (ib.cur_dw > ib.num_dw) {
      printf("\nPacket ends after the end of IB.\n");
      exit(1);
   }
}

static const char *ip_name(const enum amd_ip_type ip)
{
   switch (ip) {
   case AMD_IP_GFX:
      return "GFX";
   case AMD_IP_COMPUTE:
      return "COMPUTE";
   case AMD_IP_SDMA:
      return "SDMA";
   default:
      return "Unknown";
   }
}

/**
 * Parse and print an IB into a file.
 *
 * \param f		file
 * \param ib		IB
 * \param num_dw	size of the IB
 * \param gfx_level	gfx level
 * \param family	chip family
 * \param ip_type IP type
 * \param trace_ids	the last trace IDs that are known to have been reached
 *			and executed by the CP, typically read from a buffer
 * \param trace_id_count The number of entries in the trace_ids array.
 * \param addr_callback Get a mapped pointer of the IB at a given address. Can
 *                      be NULL.
 * \param addr_callback_data user data for addr_callback
 */
void ac_parse_ib(FILE *f, uint32_t *ib, int num_dw, const int *trace_ids, unsigned trace_id_count,
                 const char *name, enum amd_gfx_level gfx_level, enum radeon_family family,
                 enum amd_ip_type ip_type, ac_debug_addr_callback addr_callback, void *addr_callback_data)
{
   fprintf(f, "------------------ %s begin - %s ------------------\n", name, ip_name(ip_type));

   ac_parse_ib_chunk(f, ib, num_dw, trace_ids, trace_id_count, gfx_level, family, ip_type,
                     addr_callback, addr_callback_data);

   fprintf(f, "------------------- %s end - %s -------------------\n\n", name, ip_name(ip_type));
}
