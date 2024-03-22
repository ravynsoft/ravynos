#ifndef AUBINATOR_VIEWER_URB_H
#define AUBINATOR_VIEWER_URB_H

#include "aubinator_viewer.h"

#include "imgui/imgui.h"

struct AubinatorViewerUrb {

   float RowHeight;

   AubinatorViewerUrb() {
      RowHeight = 10.0f;
   }

   bool _Hovered(const ImVec2& mouse, bool window_hovered,
                 const ImVec2& tl, const ImVec2& br) {
      return window_hovered &&
         tl.x <= mouse.x && tl.y <= mouse.y &&
         br.x > mouse.x && br.y > mouse.y;
   }

   void DrawAllocation(const char *label,
                       int n_stages,
                       int end_urb_offset,
                       const char *stage_names[],
                       const struct aub_decode_urb_stage_state *stages) {
      const ImVec2 label_size = ImGui::CalcTextSize("VS entry:  ", NULL, true);
      ImVec2 graph_size(ImGui::CalcItemWidth(), 2 * n_stages * label_size.y);

      ImGui::BeginChild(label, ImVec2(0, graph_size.y), false);

      ImDrawList* draw_list = ImGui::GetWindowDrawList();

      const float row_height = MAX2(RowHeight, label_size.y);
      const float width = ImGui::GetContentRegionAvailWidth() - label_size.x;
      const float alloc_delta = width / end_urb_offset;
      const ImVec2 window_pos = ImGui::GetWindowPos();
      const ImVec2 mouse_pos = ImGui::GetMousePos();
      const bool window_hovered = ImGui::IsWindowHovered();

      int const_idx = 0;
      for (int s = 0; s < n_stages; s++) {
         const float x = window_pos.x + label_size.x;
         const float y = window_pos.y + s * row_height;

         ImVec2 alloc_pos(window_pos.x, y);
         ImVec2 alloc_tl(x + stages[s].start * alloc_delta, y);
         ImVec2 alloc_br(x + (stages[s].start +
                              stages[s].n_entries * stages[s].size) * alloc_delta,
                         y + row_height);
         ImVec2 const_tl(x + const_idx * alloc_delta, y);
         ImVec2 const_br(x + (const_idx + stages[s].const_rd_length) * alloc_delta,
                         y + row_height);

         char label[40];
         snprintf(label, sizeof(label), "%s: ", stage_names[s]);
         draw_list->AddText(alloc_pos, ImGui::GetColorU32(ImGuiCol_Text), label);

         float r, g, b;
         bool hovered;

         /* URB allocation */
         hovered = _Hovered(mouse_pos, window_hovered, alloc_tl, alloc_br);
         ImGui::ColorConvertHSVtoRGB((2 * s) * 1.0f / (2 * n_stages),
                                     1.0f, hovered ? 1.0f : 0.8f,
                                     r, g, b);
         draw_list->AddRectFilled(alloc_tl, alloc_br, ImColor(r, g, b));
         if (hovered) {
            ImGui::SetTooltip("%s: start=%u end=%u",
                              stage_names[s],
                              stages[s].start,
                              stages[s].start + stages[s].n_entries * stages[s].size);
         }

         /* Constant URB input */
         hovered = _Hovered(mouse_pos, window_hovered, const_tl, const_br);
         ImGui::ColorConvertHSVtoRGB((2 * s + 1) * 1.0f / (2 * n_stages),
                                     1.0f, hovered ? 1.0f : 0.8f,
                                     r, g, b);
         draw_list->AddRectFilled(const_tl, const_br, ImColor(r, g, b));
         if (hovered) {
            ImGui::SetTooltip("%s constant: start=%u end=%u",
                              stage_names[s],
                              stages[s].rd_offset,
                              stages[s].rd_offset + stages[s].rd_length);
         }

         const_idx += stages[s].const_rd_length;
      }

      ImGui::EndChild();
   }
};

#endif /* AUBINATOR_VIEWER_URB_H */
