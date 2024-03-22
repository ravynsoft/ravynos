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

#include <memory>
#include <string>
#include <vector>

#include "util/list.h"
#include "util/macros.h"

#include "common/intel_disasm.h"
#include "common/intel_hang_dump.h"

/* Data */

struct hang_bo {
   void     *map    = NULL;
   uint64_t  offset = 0;
   uint64_t  size   = 0;
};

struct hang_map {
   uint64_t  offset = 0;
   uint64_t  size   = 0;
};

struct hang_exec {
   uint64_t  offset = 0;
};

/* UI */

#include <epoxy/gl.h>

#include "imgui/imgui.h"
#include "imgui/imgui_memory_editor.h"
#include "imgui_impl_gtk3.h"
#include "imgui_impl_opengl3.h"

#include "aubinator_viewer.h"

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

class window {
public:
   virtual void display() = 0;
   virtual void destroy() = 0;

   virtual ~window() {}

   const char *name() const { return m_name; }

   bool m_opened = true;

   ImVec2 m_position = ImVec2(-1, -1);
   ImVec2 m_size     = ImVec2(700, 300);

protected:
   window() {}

   char m_name[128];
};

static struct Context {
   /* Hang file descriptor */
   int file_fd = -1;
   void *file_map = NULL;

   /* Map hang file in RW for edition */
   bool edit = false;

   struct intel_device_info devinfo;
   struct intel_spec *spec = NULL;
   struct brw_isa_info isa;

   /* Result of parsing the hang file */
   std::vector<hang_bo>   bos;
   std::vector<hang_map>  maps;
   std::vector<hang_exec> execs;

   hang_bo hw_image;

   GtkWidget *gtk_window;

   /* UI state*/
   bool show_commands_window;
   bool show_registers_window;

   struct aub_viewer_cfg cfg;

   std::vector<std::shared_ptr<window>> windows;
} context;

thread_local ImGuiContext* __MesaImGui;

/**/

static uint8_t
read_edit_window(const uint8_t *data, size_t off)
{
   return data[off];
}

static void
write_edit_window(uint8_t *data, size_t off, uint8_t d)
{
   data[off] = d;
}

class edit_window : public window {
public:
   struct hang_bo m_bo;

   struct intel_batch_decode_bo m_aub_bo;
   uint64_t m_aub_offset;

   struct intel_batch_decode_bo m_gtt_bo;
   uint64_t m_gtt_offset;

   struct MemoryEditor m_editor;

   edit_window(const struct hang_bo &bo)
      : m_bo(bo) {
      m_editor.OptShowDataPreview = true;
      m_editor.OptShowAscii = false;
      m_editor.ReadFn = read_edit_window;
      m_editor.WriteFn = write_edit_window;

      snprintf(m_name, sizeof(m_name), "Memory view 0x%016" PRIx64 "##%p",
               bo.offset, this);
   }

   void display() {
      if (m_bo.map) {
         ImGui::BeginChild(ImGui::GetID("##block"));
         m_editor.DrawContents((uint8_t *) m_bo.map, m_bo.size, m_bo.offset);
         ImGui::EndChild();
      } else {
         ImGui::Text("Memory view at 0x%" PRIx64 " not available", m_bo.offset);
      }
   }

   void destroy() {}
};

class shader_window : public window {
public:
   std::string m_description;
   uint64_t m_address;
   std::string m_shader;

   shader_window(const char *description, uint64_t address)
      : m_description(description)
      , m_address(address) {
      snprintf(m_name, sizeof(m_name),
               "%s (0x%" PRIx64 ")##%p", m_description.c_str(), m_address, this);

      for (auto &bo : context.bos) {
         if (address >= bo.offset &&
             address < (bo.offset + bo.size)) {
            char *shader_txt = NULL;
            size_t shader_txt_size = 0;
            FILE *f = open_memstream(&shader_txt, &shader_txt_size);
            if (f) {
               intel_disassemble(&context.isa,
                                 (const uint8_t *) bo.map +
                                 (address - bo.offset), 0, f);
               fclose(f);
            }

            m_shader = std::string(shader_txt);
         }
      }
   }

   void display() {
      ImGui::InputTextMultiline("Assembly",
                                (char *) m_shader.c_str(), m_shader.size(),
                                ImGui::GetContentRegionAvail(),
                                ImGuiInputTextFlags_ReadOnly);
   }

   void destroy() {}
};

static struct intel_batch_decode_bo
batch_get_bo(void *user_data, bool ppgtt, uint64_t address)
{
   intel_batch_decode_bo ret_bo;
   ret_bo.map = NULL;
   ret_bo.addr = 0;

   if (!ppgtt)
      return ret_bo;

   for (const auto &bo : context.bos) {
      if (address >= bo.offset &&
          address < (bo.offset + bo.size)) {
         ret_bo.map = bo.map;
         ret_bo.addr = bo.offset;
         ret_bo.size = bo.size;
      }
   }

   return ret_bo;
}

static void
batch_display_shader(void *user_data, const char *shader_desc, uint64_t address)
{
   context.windows.push_back(std::shared_ptr<window>(new shader_window(shader_desc, address)));
}

class batch_window : public window {
public:
   batch_window(const struct hang_bo &bo)
      : m_bo(bo)
      , m_collapsed(true) {
      aub_viewer_decode_ctx_init(&m_decode_ctx,
                                 &context.cfg,
                                 &m_decode_cfg,
                                 &context.devinfo,
                                 context.spec,
                                 batch_get_bo,
                                 NULL,
                                 NULL);
      m_decode_ctx.display_shader = batch_display_shader;
      // window->decode_ctx.display_urb = batch_display_urb;
      // window->decode_ctx.edit_address = batch_edit_address;

      snprintf(m_name, sizeof(m_name), "Batch view 0x%016" PRIx64 "##%p",
               bo.offset, this);
   }
   ~batch_window() {}

   void display() {
         ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / (2 * 2));
         decode_options();
         if (ImGui::Button("Edit commands"))
            context.windows.push_back(std::shared_ptr<window>(new edit_window(m_bo)));
         ImGui::PopItemWidth();

         ImGui::BeginChild(ImGui::GetID("##block"));

         aub_viewer_render_batch(&m_decode_ctx,
                                 m_bo.map,
                                 m_bo.size,
                                 m_bo.offset,
                                 false /* from_ring */);

         ImGui::EndChild();
   }

   void destroy() {}

private:

   void decode_options() {
      char name[40];
      snprintf(name, sizeof(name), "command filter##%p", &m_decode_cfg.command_filter);
      m_decode_cfg.command_filter.Draw(name); ImGui::SameLine();
      snprintf(name, sizeof(name), "field filter##%p", &m_decode_cfg.field_filter);
      m_decode_cfg.field_filter.Draw(name); ImGui::SameLine();
      if (ImGui::Button("Dwords")) m_decode_cfg.show_dwords ^= 1;
   }

   struct hang_bo m_bo;

   bool m_collapsed;

   struct aub_viewer_decode_cfg m_decode_cfg;
   struct aub_viewer_decode_ctx m_decode_ctx;

   char edit_address[20];
};

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
display_hang_stats()
{
   ImGui::Begin("Hang stats");

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

   if (ImGui::Button("Help") || has_ctrl_key('h')) { ImGui::OpenPopup("Help"); }

   ImGui::Text("BOs:        %zu", context.bos.size());
   ImGui::Text("Execs       %zu", context.execs.size());
   ImGui::Text("Maps:       %zu", context.maps.size());
   ImGui::Text("PCI ID:    0x%x", context.devinfo.pci_device_id);

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

   uint64_t exec_buf_addr = 0;
   if (!context.execs.empty())
      exec_buf_addr = context.execs.front().offset;

   ImGui::BeginChild(ImGui::GetID("BO list:"));
   for (const auto &bo : context.bos) {
      char bo_name[80];
      snprintf(bo_name, sizeof(bo_name), "BO 0x%012" PRIx64 " size=%" PRIu64 "(%s) %s",
               bo.offset, bo.size, human_size(bo.size),
               bo.offset == exec_buf_addr ? "BATCH BUFFER" : "");

      if (ImGui::Selectable(bo_name, false))
         context.windows.push_back(std::shared_ptr<window>(new batch_window(bo)));
   }
   if (context.hw_image.size != 0 && ImGui::Selectable("HW IMAGE", false))
      context.windows.push_back(std::shared_ptr<window>(new batch_window(context.hw_image)));
   ImGui::EndChild();

   ImGui::End();
}

/* Main redrawing */

static void
display_windows(void)
{
   display_hang_stats();

   /* Start by disposing closed windows, we don't want to destroy windows that
    * have already been scheduled to be painted. So destroy always happens on
    * the next draw cycle, prior to any drawing.
    */
   auto it = context.windows.begin();
   while (it != context.windows.end()) {
      if (!(*it)->m_opened) {
         (*it)->destroy();
         it = context.windows.erase(it);
      } else {
         it++;
      }
   }

   for (uint32_t i = 0; i < context.windows.size(); i++) {
      std::shared_ptr<window> window = context.windows[i];
      ImGui::SetNextWindowPos(window->m_position, ImGuiCond_FirstUseEver);
      ImGui::SetNextWindowSize(window->m_size, ImGuiCond_FirstUseEver);
      if (ImGui::Begin(window->name(), &window->m_opened)) {
         window->display();
         window->m_position = ImGui::GetWindowPos();
         window->m_size = ImGui::GetWindowSize();
      }
      if (window_has_ctrl_key('w'))
         window->m_opened = false;
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
   // TODO
}

static void
print_help(const char *progname, FILE *file)
{
   fprintf(file,
           "Usage: %s -p platform HANG_FILE\n"
           "\n"
           "    -p, --platform platform    platform to use for decoding\n"
           "    -e, --edit                 map the hang file read/write for edition\n"
           , progname);
}

static void
add_bo(void *map, uint64_t addr, uint64_t size)
{
   hang_bo bo;
   bo.map    = map;
   bo.offset = addr;
   bo.size   = size;
   context.bos.push_back(bo);
}

static void
add_map(uint64_t addr, uint64_t size)
{
   hang_map map;
   map.offset = addr;
   map.size   = size;
   context.maps.push_back(map);
}

static void
add_exec(uint64_t addr)
{
   hang_exec exec;
   exec.offset = addr;
   context.execs.push_back(exec);
}

static size_t
get_block_size(uint32_t type)
{
   switch (type) {
   case INTEL_HANG_DUMP_BLOCK_TYPE_HEADER:   return sizeof(struct intel_hang_dump_block_header);
   case INTEL_HANG_DUMP_BLOCK_TYPE_BO:       return sizeof(struct intel_hang_dump_block_bo);
   case INTEL_HANG_DUMP_BLOCK_TYPE_MAP:      return sizeof(struct intel_hang_dump_block_map);
   case INTEL_HANG_DUMP_BLOCK_TYPE_EXEC:     return sizeof(struct intel_hang_dump_block_exec);
   case INTEL_HANG_DUMP_BLOCK_TYPE_HW_IMAGE: return sizeof(struct intel_hang_dump_block_hw_image);
   default:                                  unreachable("invalid block");
   }
}

static void
parse_hang_file(const char *filename)
{
   context.file_fd = open(filename, context.edit ? O_RDWR : O_RDONLY);
   if (context.file_fd < 0)
      exit(EXIT_FAILURE);

   struct stat file_stats;
   if (fstat(context.file_fd, &file_stats) != 0)
      exit(EXIT_FAILURE);

   context.file_map = mmap(NULL, file_stats.st_size,
                           PROT_READ | PROT_WRITE,
                           context.edit ? MAP_SHARED : MAP_PRIVATE,
                           context.file_fd, 0);
   if (context.file_map == MAP_FAILED)
      exit(EXIT_FAILURE);

   uint8_t *current_file_ptr = (uint8_t *) context.file_map;
   uint8_t *last_file_ptr = current_file_ptr + file_stats.st_size;

   while (current_file_ptr < last_file_ptr) {
      union intel_hang_dump_block_all *block_header =
         (union intel_hang_dump_block_all *)current_file_ptr;
      size_t block_size = get_block_size(block_header->base.type);

      switch (block_header->base.type) {
      case INTEL_HANG_DUMP_BLOCK_TYPE_HEADER:
         assert(block_header->header.magic == INTEL_HANG_DUMP_MAGIC);
         assert(block_header->header.version == INTEL_HANG_DUMP_VERSION);
         break;

      case INTEL_HANG_DUMP_BLOCK_TYPE_BO: {
         add_bo((uint8_t *) current_file_ptr + block_size,
                block_header->bo.offset,
                block_header->bo.size);
         current_file_ptr = (uint8_t *) current_file_ptr + block_size + block_header->bo.size;
         break;
      }

      case INTEL_HANG_DUMP_BLOCK_TYPE_HW_IMAGE: {
         context.hw_image.offset = block_header->bo.offset;
         context.hw_image.size = block_header->hw_img.size;
         context.hw_image.map = (uint8_t *) current_file_ptr + block_size;
         current_file_ptr = (uint8_t *) current_file_ptr + block_size + block_header->hw_img.size;
         break;
      }

      case INTEL_HANG_DUMP_BLOCK_TYPE_MAP: {
         add_map(block_header->map.offset,
                 block_header->map.size);
         current_file_ptr = (uint8_t *) current_file_ptr + block_size;
         break;
      }

      case INTEL_HANG_DUMP_BLOCK_TYPE_EXEC: {
         add_exec(block_header->exec.offset);
         current_file_ptr = (uint8_t *) current_file_ptr + block_size;
         break;
      }

      default:
         unreachable("Invalid block type");
      }
   }
}

int
main(int argc, char *argv[])
{
   int c, i;
   bool help = false, edit = false;
   const char *platform = NULL;
   const struct option aubinator_opts[] = {
      { "platform",      required_argument, NULL,                          0    },
      { "edit",          no_argument,       (int *) &edit,                 true },
      { "help",          no_argument,       (int *) &help,                 true },
      { NULL,            0,                 NULL,                          0    },
   };

   i = 0;
   while ((c = getopt_long(argc, argv, "p:e", aubinator_opts, &i)) != -1) {
      switch (c) {
      case 'p':
         platform = optarg;
         break;
      case 'e':
         edit = true;
         break;
      default:
         break;
      }
   }

   context = {};
   context.edit = edit;

   const char *filename = NULL;
   if (optind < argc)
      filename = argv[optind];

   if (help || !platform || !filename) {
      print_help(argv[0], stderr);
      exit(0);
   }

   intel_get_device_info_from_pci_id(
      intel_device_name_to_pci_device_id(platform),
      &context.devinfo);

   brw_init_isa_info(&context.isa, &context.devinfo);
   context.spec = intel_spec_load(&context.devinfo);

   parse_hang_file(filename);

   gtk_init(NULL, NULL);

   context.gtk_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(context.gtk_window), "Hang Viewer");
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

   return EXIT_SUCCESS;
}
