/*
 * Copyright © 2016 Intel Corporation
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
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <ctype.h>

#include "util/macros.h"

#include "aub_read.h"
#include "aub_mem.h"

#include "common/intel_disasm.h"

#define xtzalloc(name) ((decltype(&name)) calloc(1, sizeof(name)))
#define xtalloc(name) ((decltype(&name)) malloc(sizeof(name)))

struct aub_file {
   uint8_t *map, *end, *cursor;

   uint16_t pci_id;
   char app_name[33];

   /* List of batch buffers to process */
   struct {
      const uint8_t *start;
      const uint8_t *end;
   } *execs;
   int n_execs;
   int n_allocated_execs;

   uint32_t idx_reg_write;

   /* Device state */
   struct intel_device_info devinfo;
   struct brw_isa_info isa;
   struct intel_spec *spec;
};

static void
store_exec_begin(struct aub_file *file)
{
   if (unlikely(file->n_execs >= file->n_allocated_execs)) {
      file->n_allocated_execs = MAX2(2U * file->n_allocated_execs,
                                     4096 / sizeof(file->execs[0]));
      file->execs = (decltype(file->execs))
         realloc(static_cast<void *>(file->execs),
                 file->n_allocated_execs * sizeof(file->execs[0]));
   }

   file->execs[file->n_execs++].start = file->cursor;
}

static void
store_exec_end(struct aub_file *file)
{
   if (file->n_execs > 0 && file->execs[file->n_execs - 1].end == NULL)
      file->execs[file->n_execs - 1].end = file->cursor;
}

static void
handle_mem_write(void *user_data, uint64_t phys_addr,
                 const void *data, uint32_t data_len)
{
   struct aub_file *file = (struct aub_file *) user_data;
   file->idx_reg_write = 0;
   store_exec_end(file);
}

static void
handle_ring_write(void *user_data, enum intel_engine_class engine,
                  const void *ring_data, uint32_t ring_data_len)
{
   struct aub_file *file = (struct aub_file *) user_data;
   file->idx_reg_write = 0;
   store_exec_begin(file);
}

static void
handle_reg_write(void *user_data, uint32_t reg_offset, uint32_t reg_value)
{
   struct aub_file *file = (struct aub_file *) user_data;

   /* Only store the first register write of a series (execlist writes take
    * involve 2 dwords).
    */
   if (file->idx_reg_write++ == 0)
      store_exec_begin(file);
}

static void
handle_info(void *user_data, int pci_id, const char *app_name)
{
   struct aub_file *file = (struct aub_file *) user_data;
   store_exec_end(file);

   file->pci_id = pci_id;
   snprintf(file->app_name, sizeof(app_name), "%s", app_name);

   if (!intel_get_device_info_from_pci_id(file->pci_id, &file->devinfo)) {
      fprintf(stderr, "can't find device information: pci_id=0x%x\n", file->pci_id);
      exit(EXIT_FAILURE);
   }
   brw_init_isa_info(&file->isa, &file->devinfo);
   file->spec = intel_spec_load(&file->devinfo);
}

static void
handle_error(void *user_data, const void *aub_data, const char *msg)
{
   fprintf(stderr, "ERROR: %s", msg);
}

static struct aub_file *
aub_file_open(const char *filename)
{
   struct aub_file *file;
   struct stat sb;
   int fd;

   file = xtzalloc(*file);
   fd = open(filename, O_RDWR);
   if (fd == -1) {
      fprintf(stderr, "open %s failed: %s\n", filename, strerror(errno));
      exit(EXIT_FAILURE);
   }

   if (fstat(fd, &sb) == -1) {
      fprintf(stderr, "stat failed: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
   }

   file->map = (uint8_t *) mmap(NULL, sb.st_size,
                                PROT_READ, MAP_SHARED, fd, 0);
   if (file->map == MAP_FAILED) {
      fprintf(stderr, "mmap failed: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
   }

   close(fd);

   file->cursor = file->map;
   file->end = file->map + sb.st_size;

   struct aub_read aub_read = {};
   aub_read.user_data = file;
   aub_read.info = handle_info;
   aub_read.error = handle_error;
   aub_read.reg_write = handle_reg_write;
   aub_read.ring_write = handle_ring_write;
   aub_read.local_write = handle_mem_write;
   aub_read.phys_write = handle_mem_write;
   aub_read.ggtt_write = handle_mem_write;
   aub_read.ggtt_entry_write = handle_mem_write;

   int consumed;
   while (file->cursor < file->end &&
          (consumed = aub_read_command(&aub_read, file->cursor,
                                       file->end - file->cursor)) > 0) {
      file->cursor += consumed;
   }

   /* Ensure we have an end on the last register write. */
   if (file->n_execs > 0 && file->execs[file->n_execs - 1].end == NULL)
      file->execs[file->n_execs - 1].end = file->end;

   return file;
}

/**/

static void
update_mem_for_exec(struct aub_mem *mem, struct aub_file *file, int exec_idx)
{
   struct aub_read read = {};
   read.user_data = mem;
   read.local_write = aub_mem_local_write;
   read.phys_write = aub_mem_phys_write;
   read.ggtt_write = aub_mem_ggtt_write;
   read.ggtt_entry_write = aub_mem_ggtt_entry_write;

   /* Replay the aub file from the beginning up to just before the
    * commands we want to read. where the context setup happens.
    */
   const uint8_t *iter = file->map;
   while (iter < file->execs[exec_idx].start) {
      iter += aub_read_command(&read, iter, file->execs[exec_idx].start - iter);
   }
}

/* UI */

#include <epoxy/gl.h>

#include "imgui/imgui.h"
#include "imgui/imgui_memory_editor.h"
#include "imgui_impl_gtk3.h"
#include "imgui_impl_opengl3.h"

#include "aubinator_viewer.h"
#include "aubinator_viewer_urb.h"

struct window {
   struct list_head link; /* link in the global list of windows */
   struct list_head parent_link; /* link in parent window list of children */

   struct list_head children_windows; /* list of children windows */

   char name[128];
   bool opened;

   ImVec2 position;
   ImVec2 size;

   void (*display)(struct window*);
   void (*destroy)(struct window*);
};

struct edit_window {
   struct window base;

   struct aub_mem *mem;
   uint64_t address;
   uint32_t len;

   struct intel_batch_decode_bo aub_bo;
   uint64_t aub_offset;

   struct intel_batch_decode_bo gtt_bo;
   uint64_t gtt_offset;

   struct MemoryEditor editor;
};

struct pml4_window {
   struct window base;

   struct aub_mem *mem;
};

struct shader_window {
   struct window base;

   uint64_t address;
   char *shader;
   size_t shader_size;
};

struct urb_window {
   struct window base;

   uint32_t end_urb_offset;
   struct aub_decode_urb_stage_state urb_stages[AUB_DECODE_N_STAGE];

   AubinatorViewerUrb urb_view;
};

struct batch_window {
   struct window base;

   struct aub_mem mem;
   struct aub_read read;

   bool uses_ppgtt;

   bool collapsed;
   int exec_idx;

   struct aub_viewer_decode_cfg decode_cfg;
   struct aub_viewer_decode_ctx decode_ctx;

   struct pml4_window pml4_window;

   char edit_address[20];
};

static struct Context {
   struct aub_file *file;
   char *input_file;
   char *xml_path;

   GtkWidget *gtk_window;

   /* UI state*/
   bool show_commands_window;
   bool show_registers_window;

   struct aub_viewer_cfg cfg;

   struct list_head windows;

   struct window file_window;
   struct window commands_window;
   struct window registers_window;
} context;

thread_local ImGuiContext* __MesaImGui;

static int
map_key(int k)
{
   return ImGuiKey_COUNT + k;
}

static bool
has_ctrl_key(int key)
{
   return ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(map_key(key));
}

static bool
window_has_ctrl_key(int key)
{
   return ImGui::IsRootWindowOrAnyChildFocused() && has_ctrl_key(key);
}

static void
destroy_window_noop(struct window *win)
{
}

/* Shader windows */

static void
display_shader_window(struct window *win)
{
   struct shader_window *window = (struct shader_window *) win;

   if (window->shader) {
      ImGui::InputTextMultiline("Assembly",
                                window->shader, window->shader_size,
                                ImGui::GetContentRegionAvail(),
                                ImGuiInputTextFlags_ReadOnly);
   } else {
      ImGui::Text("Shader not available");
   }
}

static void
destroy_shader_window(struct window *win)
{
   struct shader_window *window = (struct shader_window *) win;

   free(window->shader);
   free(window);
}

static struct shader_window *
new_shader_window(struct aub_mem *mem, uint64_t address, const char *desc)
{
   struct shader_window *window = xtzalloc(*window);

   snprintf(window->base.name, sizeof(window->base.name),
            "%s (0x%" PRIx64 ")##%p", desc, address, window);

   list_inithead(&window->base.parent_link);
   window->base.position = ImVec2(-1, -1);
   window->base.size = ImVec2(700, 300);
   window->base.opened = true;
   window->base.display = display_shader_window;
   window->base.destroy = destroy_shader_window;

   struct intel_batch_decode_bo shader_bo =
      aub_mem_get_ppgtt_bo(mem, address);
   if (shader_bo.map) {
      FILE *f = open_memstream(&window->shader, &window->shader_size);
      if (f) {
         intel_disassemble(&context.file->isa,
                           (const uint8_t *) shader_bo.map +
                           (address - shader_bo.addr), 0, f);
         fclose(f);
      }
   }

   list_addtail(&window->base.link, &context.windows);

   return window;
}

/* URB windows */

static void
display_urb_window(struct window *win)
{
   struct urb_window *window = (struct urb_window *) win;
   static const char *stages[] = {
      [AUB_DECODE_STAGE_VS] = "VS",
      [AUB_DECODE_STAGE_HS] = "HS",
      [AUB_DECODE_STAGE_DS] = "DS",
      [AUB_DECODE_STAGE_GS] = "GS",
      [AUB_DECODE_STAGE_PS] = "PS",
      [AUB_DECODE_STAGE_CS] = "CS",
   };

   ImGui::Text("URB allocation:");
   window->urb_view.DrawAllocation("##urb",
                                   ARRAY_SIZE(window->urb_stages),
                                   window->end_urb_offset,
                                   stages,
                                   &window->urb_stages[0]);
}

static void
destroy_urb_window(struct window *win)
{
   struct urb_window *window = (struct urb_window *) win;

   free(window);
}

static struct urb_window *
new_urb_window(struct aub_viewer_decode_ctx *decode_ctx, uint64_t address)
{
   struct urb_window *window = xtzalloc(*window);

   snprintf(window->base.name, sizeof(window->base.name),
            "URB view (0x%" PRIx64 ")##%p", address, window);

   list_inithead(&window->base.parent_link);
   window->base.position = ImVec2(-1, -1);
   window->base.size = ImVec2(700, 300);
   window->base.opened = true;
   window->base.display = display_urb_window;
   window->base.destroy = destroy_urb_window;

   window->end_urb_offset = decode_ctx->end_urb_offset;
   memcpy(window->urb_stages, decode_ctx->urb_stages, sizeof(window->urb_stages));
   window->urb_view = AubinatorViewerUrb();

   list_addtail(&window->base.link, &context.windows);

   return window;
}

/* Memory editor windows */

static uint8_t
read_edit_window(const uint8_t *data, size_t off)
{
   struct edit_window *window = (struct edit_window *) data;

   return *((const uint8_t *) window->gtt_bo.map + window->gtt_offset + off);
}

static void
write_edit_window(uint8_t *data, size_t off, uint8_t d)
{
   struct edit_window *window = (struct edit_window *) data;
   uint8_t *gtt = (uint8_t *) window->gtt_bo.map + window->gtt_offset + off;
   uint8_t *aub = (uint8_t *) window->aub_bo.map + window->aub_offset + off;

   *gtt = *aub = d;
}

static void
display_edit_window(struct window *win)
{
   struct edit_window *window = (struct edit_window *) win;

   if (window->aub_bo.map && window->gtt_bo.map) {
      ImGui::BeginChild(ImGui::GetID("##block"));
      window->editor.DrawContents((uint8_t *) window,
                                  MIN3(window->len,
                                       window->gtt_bo.size - window->gtt_offset,
                                       window->aub_bo.size - window->aub_offset),
                                  window->address);
      ImGui::EndChild();
   } else {
      ImGui::Text("Memory view at 0x%" PRIx64 " not available", window->address);
   }
}

static void
destroy_edit_window(struct window *win)
{
   struct edit_window *window = (struct edit_window *) win;

   if (window->aub_bo.map)
      mprotect((void *) window->aub_bo.map, 4096, PROT_READ);
   free(window);
}

static struct edit_window *
new_edit_window(struct aub_mem *mem, uint64_t address, uint32_t len)
{
   struct edit_window *window = xtzalloc(*window);

   snprintf(window->base.name, sizeof(window->base.name),
            "Editing aub at 0x%" PRIx64 "##%p", address, window);

   list_inithead(&window->base.parent_link);
   window->base.position = ImVec2(-1, -1);
   window->base.size = ImVec2(500, 600);
   window->base.opened = true;
   window->base.display = display_edit_window;
   window->base.destroy = destroy_edit_window;

   window->mem = mem;
   window->address = address;
   window->aub_bo = aub_mem_get_ppgtt_addr_aub_data(mem, address);
   window->gtt_bo = aub_mem_get_ppgtt_addr_data(mem, address);
   window->len = len;
   window->editor = MemoryEditor();
   window->editor.OptShowDataPreview = true;
   window->editor.OptShowAscii = false;
   window->editor.ReadFn = read_edit_window;
   window->editor.WriteFn = write_edit_window;

   if (window->aub_bo.map) {
      uint64_t unaligned_map = (uint64_t) window->aub_bo.map;
      window->aub_bo.map = (const void *)(unaligned_map & ~0xffful);
      window->aub_offset = unaligned_map - (uint64_t) window->aub_bo.map;

      if (mprotect((void *) window->aub_bo.map, window->aub_bo.size, PROT_READ | PROT_WRITE) != 0) {
         window->aub_bo.map = NULL;
      }
   }

   window->gtt_offset = address - window->gtt_bo.addr;

   list_addtail(&window->base.link, &context.windows);

   return window;
}

/* 4 level page table walk windows */

static void
display_pml4_level(struct aub_mem *mem, uint64_t table_addr, uint64_t table_virt_addr, int level)
{
   if (level == 0)
      return;

   struct intel_batch_decode_bo table_bo =
      aub_mem_get_phys_addr_data(mem, table_addr);
   const uint64_t *table = (const uint64_t *) ((const uint8_t *) table_bo.map +
                                               table_addr - table_bo.addr);
   if (!table) {
      ImGui::TextColored(context.cfg.missing_color, "Page not available");
      return;
   }

   uint64_t addr_increment = 1ULL << (12 + 9 * (level - 1));

   if (level == 1) {
      for (int e = 0; e < 512; e++) {
         bool available = (table[e] & 1) != 0;
         uint64_t entry_virt_addr = table_virt_addr + e * addr_increment;
         if (!available)
            continue;
         ImGui::Text("Entry%03i - phys_addr=0x%" PRIx64 " - virt_addr=0x%" PRIx64,
                     e, table[e], entry_virt_addr);
      }
   } else {
      for (int e = 0; e < 512; e++) {
         bool available = (table[e] & 1) != 0;
         uint64_t entry_virt_addr = table_virt_addr + e * addr_increment;
         if (available &&
             ImGui::TreeNodeEx(&table[e],
                               available ? ImGuiTreeNodeFlags_Framed : 0,
                               "Entry%03i - phys_addr=0x%" PRIx64 " - virt_addr=0x%" PRIx64,
                               e, table[e], entry_virt_addr)) {
            display_pml4_level(mem, table[e] & ~0xffful, entry_virt_addr, level -1);
            ImGui::TreePop();
         }
      }
   }
}

static void
display_pml4_window(struct window *win)
{
   struct pml4_window *window = (struct pml4_window *) win;

   ImGui::Text("pml4: %" PRIx64, window->mem->pml4);
   ImGui::BeginChild(ImGui::GetID("##block"));
   display_pml4_level(window->mem, window->mem->pml4, 0, 4);
   ImGui::EndChild();
}

static void
show_pml4_window(struct pml4_window *window, struct aub_mem *mem)
{
   if (window->base.opened) {
      window->base.opened = false;
      return;
   }

   snprintf(window->base.name, sizeof(window->base.name),
            "4-Level page tables##%p", window);

   list_inithead(&window->base.parent_link);
   window->base.position = ImVec2(-1, -1);
   window->base.size = ImVec2(500, 600);
   window->base.opened = true;
   window->base.display = display_pml4_window;
   window->base.destroy = destroy_window_noop;

   window->mem = mem;

   list_addtail(&window->base.link, &context.windows);
}

/* Batch decoding windows */

static void
display_decode_options(struct aub_viewer_decode_cfg *cfg)
{
   char name[40];
   snprintf(name, sizeof(name), "command filter##%p", &cfg->command_filter);
   cfg->command_filter.Draw(name); ImGui::SameLine();
   snprintf(name, sizeof(name), "field filter##%p", &cfg->field_filter);
   cfg->field_filter.Draw(name); ImGui::SameLine();
   if (ImGui::Button("Dwords")) cfg->show_dwords ^= 1;
}

static void
batch_display_shader(void *user_data, const char *shader_desc, uint64_t address)
{
   struct batch_window *window = (struct batch_window *) user_data;
   struct shader_window *shader_window =
      new_shader_window(&window->mem, address, shader_desc);

   list_add(&shader_window->base.parent_link, &window->base.children_windows);
}

static void
batch_display_urb(void *user_data, const struct aub_decode_urb_stage_state *stages)
{
   struct batch_window *window = (struct batch_window *) user_data;
   struct urb_window *urb_window = new_urb_window(&window->decode_ctx, 0);

   list_add(&urb_window->base.parent_link, &window->base.children_windows);
}

static void
batch_edit_address(void *user_data, uint64_t address, uint32_t len)
{
   struct batch_window *window = (struct batch_window *) user_data;
   struct edit_window *edit_window =
      new_edit_window(&window->mem, address, len);

   list_add(&edit_window->base.parent_link, &window->base.children_windows);
}

static struct intel_batch_decode_bo
batch_get_bo(void *user_data, bool ppgtt, uint64_t address)
{
   struct batch_window *window = (struct batch_window *) user_data;

   if (window->uses_ppgtt && ppgtt)
      return aub_mem_get_ppgtt_bo(&window->mem, address);
   else
      return aub_mem_get_ggtt_bo(&window->mem, address);
}

static void
update_batch_window(struct batch_window *window, bool reset, int exec_idx)
{
   if (reset)
      aub_mem_fini(&window->mem);
   aub_mem_init(&window->mem);

   window->exec_idx = MAX2(MIN2(context.file->n_execs - 1, exec_idx), 0);
   update_mem_for_exec(&window->mem, context.file, window->exec_idx);
}

static void
display_batch_ring_write(void *user_data, enum intel_engine_class engine,
                         const void *data, uint32_t data_len)
{
   struct batch_window *window = (struct batch_window *) user_data;

   window->uses_ppgtt = false;

   aub_viewer_render_batch(&window->decode_ctx, data, data_len, 0, false);
}

static void
display_batch_execlist_write(void *user_data,
                             enum intel_engine_class engine,
                             uint64_t context_descriptor)
{
   struct batch_window *window = (struct batch_window *) user_data;

   const uint32_t pphwsp_size = 4096;
   uint32_t pphwsp_addr = context_descriptor & 0xfffff000;
   struct intel_batch_decode_bo pphwsp_bo =
      aub_mem_get_ggtt_bo(&window->mem, pphwsp_addr);
   uint32_t *context_img = (uint32_t *)((uint8_t *)pphwsp_bo.map +
                                        (pphwsp_addr - pphwsp_bo.addr) +
                                        pphwsp_size);

   uint32_t ring_buffer_head = context_img[5];
   uint32_t ring_buffer_tail = context_img[7];
   uint32_t ring_buffer_start = context_img[9];
   uint32_t ring_buffer_length = (context_img[11] & 0x1ff000) + 4096;

   window->mem.pml4 = (uint64_t)context_img[49] << 32 | context_img[51];

   struct intel_batch_decode_bo ring_bo =
      aub_mem_get_ggtt_bo(&window->mem, ring_buffer_start);
   assert(ring_bo.size > 0);
   void *commands = (uint8_t *)ring_bo.map + (ring_buffer_start - ring_bo.addr) + ring_buffer_head;

   window->uses_ppgtt = true;

   window->decode_ctx.engine = engine;
   aub_viewer_render_batch(&window->decode_ctx, commands,
                           MIN2(ring_buffer_tail - ring_buffer_head, ring_buffer_length),
                           ring_buffer_start + ring_buffer_head, true);
}

static void
display_batch_window(struct window *win)
{
   struct batch_window *window = (struct batch_window *) win;

   ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / (2 * 2));
   if (window_has_ctrl_key('f')) ImGui::SetKeyboardFocusHere();
   display_decode_options(&window->decode_cfg);
   ImGui::PopItemWidth();

   if (ImGui::InputInt("Execbuf", &window->exec_idx))
      update_batch_window(window, true, window->exec_idx);

   if (window_has_ctrl_key('p'))
      update_batch_window(window, true, window->exec_idx - 1);
   if (window_has_ctrl_key('n'))
      update_batch_window(window, true, window->exec_idx + 1);

   ImGui::Text("execbuf %i", window->exec_idx);
   if (ImGui::Button("Show PPGTT")) { show_pml4_window(&window->pml4_window, &window->mem); }

   ImGui::BeginChild(ImGui::GetID("##block"));

   struct aub_read read = {};
   read.user_data = window;
   read.ring_write = display_batch_ring_write;
   read.execlist_write = display_batch_execlist_write;

   const uint8_t *iter = context.file->execs[window->exec_idx].start;
   while (iter < context.file->execs[window->exec_idx].end) {
      iter += aub_read_command(&read, iter,
                               context.file->execs[window->exec_idx].end - iter);
   }

   ImGui::EndChild();
}

static void
destroy_batch_window(struct window *win)
{
   struct batch_window *window = (struct batch_window *) win;

   aub_mem_fini(&window->mem);

   /* This works because children windows are inserted at the back of the
    * list, ensuring the deletion loop goes through the children after calling
    * this function.
    */
   list_for_each_entry(struct window, child_window,
                       &window->base.children_windows, parent_link)
      child_window->opened = false;
   window->pml4_window.base.opened = false;

   free(window);
}

static void
new_batch_window(int exec_idx)
{
   struct batch_window *window = xtzalloc(*window);

   snprintf(window->base.name, sizeof(window->base.name),
            "Batch view##%p", window);

   list_inithead(&window->base.parent_link);
   list_inithead(&window->base.children_windows);
   window->base.position = ImVec2(-1, -1);
   window->base.size = ImVec2(600, 700);
   window->base.opened = true;
   window->base.display = display_batch_window;
   window->base.destroy = destroy_batch_window;

   window->collapsed = true;
   window->decode_cfg = aub_viewer_decode_cfg();

   aub_viewer_decode_ctx_init(&window->decode_ctx,
                              &context.cfg,
                              &window->decode_cfg,
                              &context.file->devinfo,
                              context.file->spec,
                              batch_get_bo,
                              NULL,
                              window);
   window->decode_ctx.display_shader = batch_display_shader;
   window->decode_ctx.display_urb = batch_display_urb;
   window->decode_ctx.edit_address = batch_edit_address;

   update_batch_window(window, false, exec_idx);

   list_addtail(&window->base.link, &context.windows);
}

/**/

static void
display_registers_window(struct window *win)
{
   static struct ImGuiTextFilter filter;
   if (window_has_ctrl_key('f')) ImGui::SetKeyboardFocusHere();
   filter.Draw();

   ImGui::BeginChild(ImGui::GetID("##block"));
   hash_table_foreach(context.file->spec->registers_by_name, entry) {
      struct intel_group *reg = (struct intel_group *) entry->data;
      if (filter.PassFilter(reg->name) &&
          ImGui::CollapsingHeader(reg->name)) {
         const struct intel_field *field = reg->fields;
         while (field) {
            ImGui::Text("%s : %i -> %i\n", field->name, field->start, field->end);
            field = field->next;
         }
      }
   }
   ImGui::EndChild();
}

static void
show_register_window(void)
{
   struct window *window = &context.registers_window;

   if (window->opened) {
      window->opened = false;
      return;
   }

   snprintf(window->name, sizeof(window->name), "Registers");

   list_inithead(&window->parent_link);
   window->position = ImVec2(-1, -1);
   window->size = ImVec2(200, 400);
   window->opened = true;
   window->display = display_registers_window;
   window->destroy = destroy_window_noop;

   list_addtail(&window->link, &context.windows);
}

static void
display_commands_window(struct window *win)
{
   static struct ImGuiTextFilter cmd_filter;
   if (window_has_ctrl_key('f')) ImGui::SetKeyboardFocusHere();
   cmd_filter.Draw("name filter");
   static struct ImGuiTextFilter field_filter;
   field_filter.Draw("field filter");

   static char opcode_str[9] = { 0, };
   ImGui::InputText("opcode filter", opcode_str, sizeof(opcode_str),
                    ImGuiInputTextFlags_CharsHexadecimal);
   size_t opcode_len = strlen(opcode_str);
   uint64_t opcode = strtol(opcode_str, NULL, 16);

   static bool show_dwords = true;
   if (ImGui::Button("Dwords")) show_dwords ^= 1;

   ImGui::BeginChild(ImGui::GetID("##block"));
   hash_table_foreach(context.file->spec->commands, entry) {
      struct intel_group *cmd = (struct intel_group *) entry->data;
      if ((cmd_filter.PassFilter(cmd->name) &&
           (opcode_len == 0 || (opcode & cmd->opcode_mask) == cmd->opcode)) &&
          ImGui::CollapsingHeader(cmd->name)) {
         const struct intel_field *field = cmd->fields;
         int32_t last_dword = -1;
         while (field) {
            if (show_dwords && field->start / 32 != last_dword) {
               for (last_dword = MAX2(0, last_dword + 1);
                    last_dword < field->start / 32; last_dword++) {
                  ImGui::TextColored(context.cfg.dwords_color,
                                     "Dword %d", last_dword);
               }
               ImGui::TextColored(context.cfg.dwords_color, "Dword %d", last_dword);
            }
            if (field_filter.PassFilter(field->name))
               ImGui::Text("%s : %i -> %i\n", field->name, field->start, field->end);
            field = field->next;
         }
      }
   }
   hash_table_foreach(context.file->spec->structs, entry) {
      struct intel_group *cmd = (struct intel_group *) entry->data;
      if (cmd_filter.PassFilter(cmd->name) && opcode_len == 0 &&
          ImGui::CollapsingHeader(cmd->name)) {
         const struct intel_field *field = cmd->fields;
         int32_t last_dword = -1;
         while (field) {
            if (show_dwords && field->start / 32 != last_dword) {
               last_dword = field->start / 32;
               ImGui::TextColored(context.cfg.dwords_color,
                                  "Dword %d", last_dword);
            }
            if (field_filter.PassFilter(field->name))
               ImGui::Text("%s : %i -> %i\n", field->name, field->start, field->end);
            field = field->next;
         }
      }
   }
   ImGui::EndChild();
}

static void
show_commands_window(void)
{
   struct window *window = &context.commands_window;

   if (window->opened) {
      window->opened = false;
      return;
   }

   snprintf(window->name, sizeof(window->name), "Commands & structs");

   list_inithead(&window->parent_link);
   window->position = ImVec2(-1, -1);
   window->size = ImVec2(300, 400);
   window->opened = true;
   window->display = display_commands_window;
   window->destroy = destroy_window_noop;

   list_addtail(&window->link, &context.windows);
}

/* Main window */

static const char *
human_size(size_t size)
{
   unsigned divisions = 0;
   double v = size;
   double divider = 1024;
   while (v >= divider) {
      v /= divider;
      divisions++;
   }

   static const char *units[] = { "Bytes", "Kilobytes", "Megabytes", "Gigabytes" };
   static char result[20];
   snprintf(result, sizeof(result), "%.2f %s",
            v, divisions >= ARRAY_SIZE(units) ? "Too much!" : units[divisions]);
   return result;
}

static void
display_aubfile_window(struct window *win)
{
   ImGuiColorEditFlags cflags = (ImGuiColorEditFlags_NoAlpha |
                                 ImGuiColorEditFlags_NoLabel |
                                 ImGuiColorEditFlags_NoInputs);
   struct aub_viewer_cfg *cfg = &context.cfg;

   ImGui::ColorEdit3("background", (float *)&cfg->clear_color, cflags); ImGui::SameLine();
   ImGui::ColorEdit3("missing", (float *)&cfg->missing_color, cflags); ImGui::SameLine();
   ImGui::ColorEdit3("error", (float *)&cfg->error_color, cflags); ImGui::SameLine();
   ImGui::ColorEdit3("highlight", (float *)&cfg->highlight_color, cflags); ImGui::SameLine();
   ImGui::ColorEdit3("dwords", (float *)&cfg->dwords_color, cflags); ImGui::SameLine();
   ImGui::ColorEdit3("booleans", (float *)&cfg->boolean_color, cflags); ImGui::SameLine();

   if (ImGui::Button("Commands list") || has_ctrl_key('c')) { show_commands_window(); } ImGui::SameLine();
   if (ImGui::Button("Registers list") || has_ctrl_key('r')) { show_register_window(); } ImGui::SameLine();
   if (ImGui::Button("Help") || has_ctrl_key('h')) { ImGui::OpenPopup("Help"); }

   if (ImGui::Button("New batch window") || has_ctrl_key('b')) { new_batch_window(0); }

   ImGui::Text("File name:        %s", context.input_file);
   ImGui::Text("File size:        %s", human_size(context.file->end - context.file->map));
   ImGui::Text("Execbufs          %u", context.file->n_execs);
   ImGui::Text("PCI ID:           0x%x", context.file->pci_id);
   ImGui::Text("Application name: %s", context.file->app_name);
   ImGui::Text("%s", context.file->devinfo.name);

   ImGui::SetNextWindowContentWidth(500);
   if (ImGui::BeginPopupModal("Help", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::Text("Some global keybindings:");
      ImGui::Separator();

      static const char *texts[] = {
         "Ctrl-h",          "show this screen",
         "Ctrl-c",          "show commands list",
         "Ctrl-r",          "show registers list",
         "Ctrl-b",          "new batch window",
         "Ctrl-p/n",        "switch to previous/next batch buffer",
         "Ctrl-Tab",        "switch focus between window",
         "Ctrl-left/right", "align window to the side of the screen",
      };
      float align = 0.0f;
      for (uint32_t i = 0; i < ARRAY_SIZE(texts); i += 2)
         align = MAX2(align, ImGui::CalcTextSize(texts[i]).x);
      align += ImGui::GetStyle().WindowPadding.x + 10;

      for (uint32_t i = 0; i < ARRAY_SIZE(texts); i += 2) {
         ImGui::Text("%s", texts[i]); ImGui::SameLine(align); ImGui::Text("%s", texts[i + 1]);
      }

      if (ImGui::Button("Done") || ImGui::IsKeyPressed(ImGuiKey_Escape))
         ImGui::CloseCurrentPopup();
      ImGui::EndPopup();
   }
}

static void
show_aubfile_window(void)
{
   struct window *window = &context.file_window;

   if (window->opened)
      return;

   snprintf(window->name, sizeof(window->name),
            "Aubinator Viewer: Intel AUB file decoder/editor");

   list_inithead(&window->parent_link);
   window->size = ImVec2(-1, 250);
   window->position = ImVec2(0, 0);
   window->opened = true;
   window->display = display_aubfile_window;
   window->destroy = NULL;

   list_addtail(&window->link, &context.windows);
}

/* Main redrawing */

static void
display_windows(void)
{
   /* Start by disposing closed windows, we don't want to destroy windows that
    * have already been scheduled to be painted. So destroy always happens on
    * the next draw cycle, prior to any drawing.
    */
   list_for_each_entry_safe(struct window, window, &context.windows, link) {
      if (window->opened)
         continue;

      /* Can't close this one. */
      if (window == &context.file_window) {
         window->opened = true;
         continue;
      }

      list_del(&window->link);
      list_del(&window->parent_link);
      if (window->destroy)
         window->destroy(window);
   }

   list_for_each_entry_safe(struct window, window, &context.windows, link) {
      ImGui::SetNextWindowPos(window->position, ImGuiCond_FirstUseEver);
      ImGui::SetNextWindowSize(window->size, ImGuiCond_FirstUseEver);
      if (ImGui::Begin(window->name, &window->opened)) {
         window->display(window);
         window->position = ImGui::GetWindowPos();
         window->size = ImGui::GetWindowSize();
      }
      if (window_has_ctrl_key('w'))
         window->opened = false;
      ImGui::End();
   }
}

static void
repaint_area(GtkGLArea *area, GdkGLContext *gdk_gl_context)
{
   ImGui_ImplOpenGL3_NewFrame();
   ImGui_ImplGtk3_NewFrame();
   ImGui::NewFrame();

   display_windows();

   ImGui::EndFrame();
   ImGui::Render();

   glClearColor(context.cfg.clear_color.Value.x,
                context.cfg.clear_color.Value.y,
                context.cfg.clear_color.Value.z, 1.0);
   glClear(GL_COLOR_BUFFER_BIT);
   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void
realize_area(GtkGLArea *area)
{
   ImGui::CreateContext();
   ImGui_ImplGtk3_Init(GTK_WIDGET(area), true);
   ImGui_ImplOpenGL3_Init("#version 130");

   list_inithead(&context.windows);

   ImGui::StyleColorsDark();
   context.cfg = aub_viewer_cfg();

   ImGuiIO& io = ImGui::GetIO();
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

static void
unrealize_area(GtkGLArea *area)
{
   gtk_gl_area_make_current(area);

   ImGui_ImplOpenGL3_Shutdown();
   ImGui_ImplGtk3_Shutdown();
   ImGui::DestroyContext();
}

static void
size_allocate_area(GtkGLArea *area,
                   GdkRectangle *allocation,
                   gpointer user_data)
{
   if (!gtk_widget_get_realized(GTK_WIDGET(area)))
      return;

   /* We want to catch only initial size allocate. */
   g_signal_handlers_disconnect_by_func(area,
                                        (gpointer) size_allocate_area,
                                        user_data);
   show_aubfile_window();
}

static void
print_help(const char *progname, FILE *file)
{
   fprintf(file,
           "Usage: %s [OPTION]... FILE\n"
           "Decode aub file contents from FILE.\n\n"
           "      --help             display this help and exit\n"
           "  -x, --xml=DIR          load hardware xml description from directory DIR\n",
           progname);
}

int main(int argc, char *argv[])
{
   int c, i;
   bool help = false;
   const struct option aubinator_opts[] = {
      { "help",          no_argument,       (int *) &help,                 true },
      { "xml",           required_argument, NULL,                          'x' },
      { NULL,            0,                 NULL,                          0 }
   };

   context = {};

   i = 0;
   while ((c = getopt_long(argc, argv, "x:s:", aubinator_opts, &i)) != -1) {
      switch (c) {
      case 'x':
         context.xml_path = strdup(optarg);
         break;
      default:
         break;
      }
   }

   if (optind < argc)
      context.input_file = argv[optind];

   if (help || !context.input_file) {
      print_help(argv[0], stderr);
      exit(0);
   }

   context.file = aub_file_open(context.input_file);

   gtk_init(NULL, NULL);

   context.gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(context.gtk_window), "Aubinator Viewer");
   g_signal_connect(context.gtk_window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
   gtk_window_resize(GTK_WINDOW(context.gtk_window), 1280, 720);

   GtkWidget* gl_area = gtk_gl_area_new();
   g_signal_connect(gl_area, "render", G_CALLBACK(repaint_area), NULL);
   g_signal_connect(gl_area, "realize", G_CALLBACK(realize_area), NULL);
   g_signal_connect(gl_area, "unrealize", G_CALLBACK(unrealize_area), NULL);
   g_signal_connect(gl_area, "size_allocate", G_CALLBACK(size_allocate_area), NULL);
   gtk_container_add(GTK_CONTAINER(context.gtk_window), gl_area);

   gtk_widget_show_all(context.gtk_window);

   gtk_main();

   free(context.xml_path);

   return EXIT_SUCCESS;
}
