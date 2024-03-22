/*
 * Copyright © 2021 Google, Inc.
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

#include <assert.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "freedreno_pm4.h"

#include "emu.h"
#include "util.h"

/*
 * Emulator User Interface:
 *
 * Handles the user prompts and input parsing.
 */

static void
clear_line(void)
{
   if (!isatty(STDOUT_FILENO))
      return;
   printf("\r                                                           \r");
}

static int
readchar(void)
{
   static struct termios saved_termios, unbuffered_termios;
   int c;

   fflush(stdout);

   tcgetattr(STDIN_FILENO, &saved_termios);
   unbuffered_termios = saved_termios;
   cfmakeraw(&unbuffered_termios);

   tcsetattr(STDIN_FILENO, TCSANOW, &unbuffered_termios);
   do {
      c = getchar();
   } while (isspace(c));
   tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);

   /* TODO, read from script until EOF and then read from stdin: */
   if (c == -1)
      exit(0);

   return c;
}

static const char *
extract_string(char **buf)
{
   char *p = *buf;

   /* eat any leading whitespace: */
   while (*p && isspace(*p))
      p++;

   if (!*p)
      return NULL;

   char *ret = p;

   /* skip to next whitespace: */
   while (*p && !isspace(*p))
      p++;

   if (*p)
      *p = '\0';

   *buf = ++p;

   return ret;
}

static size_t
readline(char **p)
{
   static char *buf;
   static size_t n;

   ssize_t ret = getline(&buf, &n, stdin);
   if (ret < 0)
      return ret;

   *p = buf;
   return 0;
}

static ssize_t
read_two_values(const char **val1, const char **val2)
{
   char *p;

   ssize_t ret = readline(&p);
   if (ret < 0)
      return ret;

   *val1 = extract_string(&p);
   *val2 = extract_string(&p);

   return 0;
}

static ssize_t
read_one_value(const char **val)
{
   char *p;

   ssize_t ret = readline(&p);
   if (ret < 0)
      return ret;

   *val = extract_string(&p);

   return 0;
}

static void
print_dst(unsigned reg)
{
   if (reg == REG_REM)
      printf("$rem"); /* remainding dwords in packet */
   else if (reg == REG_ADDR)
      printf("$addr");
   else if (reg == REG_USRADDR)
      printf("$usraddr");
   else if (reg == REG_DATA)
      printf("$data");
   else
      printf("$%02x", reg);
}

static void
dump_gpr_register(struct emu *emu, unsigned n)
{
   printf("              GPR:  ");
   print_dst(n);
   printf(": ");
   if (BITSET_TEST(emu->gpr_regs.written, n)) {
      printdelta("%08x\n", emu->gpr_regs.val[n]);
   } else {
      printf("%08x\n", emu->gpr_regs.val[n]);
   }
}

static void
dump_gpr_registers(struct emu *emu)
{
   for (unsigned i = 0; i < ARRAY_SIZE(emu->gpr_regs.val); i++) {
      dump_gpr_register(emu, i);
   }
}

static void
dump_gpu_register(struct emu *emu, unsigned n)
{
   printf("              GPU:  ");
   char *name = afuc_gpu_reg_name(n);
   if (name) {
      printf("%s", name);
      free(name);
   } else {
      printf("0x%04x", n);
   }
   printf(": ");
   if (BITSET_TEST(emu->gpu_regs.written, n)) {
      printdelta("%08x\n", emu->gpu_regs.val[n]);
   } else {
      printf("%08x\n", emu->gpu_regs.val[n]);
   }
}

static void
dump_pipe_register(struct emu *emu, unsigned n)
{
   printf("              PIPE: ");
   print_pipe_reg(n);
   printf(": ");
   if (BITSET_TEST(emu->pipe_regs.written, n)) {
      printdelta("%08x\n", emu->pipe_regs.val[n]);
   } else {
      printf("%08x\n", emu->pipe_regs.val[n]);
   }
}

static void
dump_control_register(struct emu *emu, unsigned n)
{
   printf("              CTRL: ");
   print_control_reg(n);
   printf(": ");
   if (BITSET_TEST(emu->control_regs.written, n)) {
      printdelta("%08x\n", emu->control_regs.val[n]);
   } else {
      printf("%08x\n", emu->control_regs.val[n]);
   }
}

static void
dump_sqe_register(struct emu *emu, unsigned n)
{
   printf("              SQE: ");
   print_sqe_reg(n);
   printf(": ");
   if (BITSET_TEST(emu->sqe_regs.written, n)) {
      printdelta("%08x\n", emu->sqe_regs.val[n]);
   } else {
      printf("%08x\n", emu->sqe_regs.val[n]);
   }
}

static void
dump_sqe_registers(struct emu *emu)
{
   for (unsigned i = 0; i < ARRAY_SIZE(emu->sqe_regs.val); i++) {
      dump_sqe_register(emu, i);
   }
}


static void
dump_gpumem(struct emu *emu, uintptr_t addr)
{
   uint32_t val = emu_mem_read_dword(emu, addr);

   printf("              MEM:  0x%016"PRIxPTR": ", addr);
   if (addr == emu->gpumem_written) {
      printdelta("0x%08x\n", val);
   } else {
      printf("0x%08x\n", val);
   }
}

static void
emu_write_gpr_prompt(struct emu *emu)
{
   clear_line();
   printf("    GPR register (name or offset) and value: ");

   const char *name;
   const char *value;

   if (read_two_values(&name, &value))
      return;

   unsigned offset = afuc_gpr_reg(name);
   uint32_t val = strtoul(value, NULL, 0);

   emu_set_gpr_reg(emu, offset, val);
}

static void
emu_write_control_prompt(struct emu *emu)
{
   clear_line();
   printf("    Control register (name or offset) and value: ");

   const char *name;
   const char *value;

   if (read_two_values(&name, &value))
      return;

   unsigned offset = afuc_control_reg(name);
   uint32_t val = strtoul(value, NULL, 0);

   emu_set_control_reg(emu, offset, val);
}

static void
emu_dump_control_prompt(struct emu *emu)
{
   clear_line();
   printf("    Control register (name or offset): ");

   const char *name;

   if (read_one_value(&name))
      return;

   printf("\n");

   unsigned offset = afuc_control_reg(name);
   dump_control_register(emu, offset);
}

static void
emu_write_sqe_prompt(struct emu *emu)
{
   clear_line();
   printf("    SQE register (name or offset) and value: ");

   const char *name;
   const char *value;

   if (read_two_values(&name, &value))
      return;

   unsigned offset = afuc_sqe_reg(name);
   uint32_t val = strtoul(value, NULL, 0);

   emu_set_sqe_reg(emu, offset, val);
}

static void
emu_write_gpu_prompt(struct emu *emu)
{
   clear_line();
   printf("    GPU register (name or offset) and value: ");

   const char *name;
   const char *value;

   if (read_two_values(&name, &value))
      return;

   unsigned offset = afuc_gpu_reg(name);
   uint32_t val = strtoul(value, NULL, 0);

   emu_set_gpu_reg(emu, offset, val);
}

static void
emu_dump_gpu_prompt(struct emu *emu)
{
   clear_line();
   printf("    GPU register (name or offset): ");

   const char *name;

   if (read_one_value(&name))
      return;

   printf("\n");

   unsigned offset = afuc_gpu_reg(name);
   dump_gpu_register(emu, offset);
}

static void
emu_write_mem_prompt(struct emu *emu)
{
   clear_line();
   printf("    GPU memory offset and value: ");

   const char *offset;
   const char *value;

   if (read_two_values(&offset, &value))
      return;

   uintptr_t addr = strtoull(offset, NULL, 0);
   uint32_t val = strtoul(value, NULL, 0);

   emu_mem_write_dword(emu, addr, val);
}

static void
emu_dump_mem_prompt(struct emu *emu)
{
   clear_line();
   printf("    GPU memory offset: ");

   const char *offset;

   if (read_one_value(&offset))
      return;

   printf("\n");

   uintptr_t addr = strtoull(offset, NULL, 0);
   dump_gpumem(emu, addr);
}

static void
emu_dump_prompt(struct emu *emu)
{
   do {
      clear_line();
      printf("  dump: GPR (r)egisters, (c)ontrol register, (s)qe registers, (g)pu register, (m)emory: ");

      int c = readchar();
      printf("%c\n", c);

      if (c == 'r') {
         /* Since there aren't too many GPR registers, just dump
          * them all:
          */
         dump_gpr_registers(emu);
         break;
      } else if (c == 's') {
         /* Similarly, just dump all the SQE registers */
         dump_sqe_registers(emu);
         break;
      } else if (c == 'c') {
         emu_dump_control_prompt(emu);
         break;
      } else if (c == 'g') {
         emu_dump_gpu_prompt(emu);
         break;
      } else if (c == 'm') {
         emu_dump_mem_prompt(emu);
         break;
      } else {
         printf("invalid option: '%c'\n", c);
         break;
      }
   } while (true);
}

static void
emu_write_prompt(struct emu *emu)
{
   do {
      clear_line();
      printf("  write: GPR (r)egister, (c)ontrol register, (s)sqe register, (g)pu register, (m)emory: ");

      int c = readchar();
      printf("%c\n", c);

      if (c == 'r') {
         emu_write_gpr_prompt(emu);
         break;
      } else if (c == 's') {
         emu_write_sqe_prompt(emu);
         break;
      } else if (c == 'c') {
         emu_write_control_prompt(emu);
         break;
      } else if (c == 'g') {
         emu_write_gpu_prompt(emu);
         break;
      } else if (c == 'm') {
         emu_write_mem_prompt(emu);
         break;
      } else {
         printf("invalid option: '%c'\n", c);
         break;
      }
   } while (true);
}

static void
emu_packet_prompt(struct emu *emu)
{
   clear_line();
   printf("  Enter packet (opc or register name), followed by payload: ");
   fflush(stdout);

   char *p;
   if (readline(&p) < 0)
      return;

   printf("\n");

   const char *name = extract_string(&p);

   /* Read the payload, so we can know the size to generate correct header: */
   uint32_t payload[0x7f];
   unsigned cnt = 0;

   do {
      const char *val = extract_string(&p);
      if (!val)
         break;

      assert(cnt < ARRAY_SIZE(payload));
      payload[cnt++] = strtoul(val, NULL, 0);
   } while (true);

   uint32_t hdr;
   if (afuc_pm4_id(name) >= 0) {
      unsigned opcode = afuc_pm4_id(name);
      hdr = pm4_pkt7_hdr(opcode, cnt);
   } else {
      unsigned regindx = afuc_gpu_reg(name);
      hdr = pm4_pkt4_hdr(regindx, cnt);
   }

   ASSERTED bool ret = emu_queue_push(&emu->roq, hdr);
   assert(ret);

   for (unsigned i = 0; i < cnt; i++) {
      ASSERTED bool ret = emu_queue_push(&emu->roq, payload[i]);
      assert(ret);
   }
}

void
emu_main_prompt(struct emu *emu)
{
   if (emu->run_mode)
      return;

   do {
      clear_line();
      printf("(s)tep, (r)un, (d)ump, (w)rite, (p)acket, (h)elp, (q)uit: ");

      int c = readchar();

      printf("%c\n", c);

      if (c == 's') {
         break;
      } else if (c == 'r') {
         emu->run_mode = true;
         break;
      } else if (c == 'd') {
         emu_dump_prompt(emu);
      } else if (c == 'w') {
         emu_write_prompt(emu);
      } else if (c == 'p') {
         emu_packet_prompt(emu);
      } else if (c == 'h') {
         printf("  (s)tep   - single step to next instruction\n");
         printf("  (r)un    - run until next waitin\n");
         printf("  (d)ump   - dump memory/register menu\n");
         printf("  (w)rite  - write memory/register menu\n");
         printf("  (p)acket - inject a pm4 packet\n");
         printf("  (h)elp   - show this usage message\n");
         printf("  (q)uit   - exit emulator\n");
      } else if (c == 'q') {
         printf("\n");
         exit(0);
      } else {
         printf("invalid option: '%c'\n", c);
      }
   } while (true);
}

void
emu_clear_state_change(struct emu *emu)
{
   memset(emu->control_regs.written, 0, sizeof(emu->control_regs.written));
   memset(emu->sqe_regs.written,     0, sizeof(emu->sqe_regs.written));
   memset(emu->pipe_regs.written,    0, sizeof(emu->pipe_regs.written));
   memset(emu->gpu_regs.written,     0, sizeof(emu->gpu_regs.written));
   memset(emu->gpr_regs.written,     0, sizeof(emu->gpr_regs.written));
   emu->gpumem_written = ~0;
}

void
emu_dump_state_change(struct emu *emu)
{
   unsigned i;

   if (emu->quiet)
      return;

   /* Print the GPRs that changed: */
   BITSET_FOREACH_SET (i, emu->gpr_regs.written, EMU_NUM_GPR_REGS) {
      dump_gpr_register(emu, i);
   }

   BITSET_FOREACH_SET (i, emu->gpu_regs.written, EMU_NUM_GPU_REGS) {
      dump_gpu_register(emu, i);
   }

   BITSET_FOREACH_SET (i, emu->pipe_regs.written, EMU_NUM_PIPE_REGS) {
      dump_pipe_register(emu, i);
   }

   BITSET_FOREACH_SET (i, emu->control_regs.written, EMU_NUM_CONTROL_REGS) {
      dump_control_register(emu, i);
   }

   BITSET_FOREACH_SET (i, emu->sqe_regs.written, EMU_NUM_SQE_REGS) {
      dump_sqe_register(emu, i);
   }

   if (emu->gpumem_written != ~0) {
      dump_gpumem(emu, emu->gpumem_written);
   }
}
