#ifndef AUBINATOR_VIEWER_H
#define AUBINATOR_VIEWER_H

#include "imgui/imgui.h"

#include "common/intel_decoder.h"
#include "common/intel_disasm.h"

struct aub_viewer_cfg {
   ImColor clear_color;
   ImColor dwords_color;
   ImColor highlight_color;
   ImColor error_color;
   ImColor missing_color;
   ImColor boolean_color;

  aub_viewer_cfg() :
    clear_color(114, 144, 154),
    dwords_color(29, 177, 194, 255),
    highlight_color(0, 230, 0, 255),
    error_color(236, 255, 0, 255),
    missing_color(230, 0, 230, 255),
    boolean_color(228, 75, 255) {}
};

struct aub_viewer_decode_cfg {
   struct ImGuiTextFilter command_filter;
   struct ImGuiTextFilter field_filter;

   bool drop_filtered;
   bool show_dwords;

  aub_viewer_decode_cfg() :
    drop_filtered(false),
    show_dwords(true) {}
};

enum aub_decode_stage {
   AUB_DECODE_STAGE_VS,
   AUB_DECODE_STAGE_HS,
   AUB_DECODE_STAGE_DS,
   AUB_DECODE_STAGE_GS,
   AUB_DECODE_STAGE_PS,
   AUB_DECODE_STAGE_CS,
   AUB_DECODE_N_STAGE,
};

struct aub_decode_urb_stage_state {
   uint32_t start;
   uint32_t size;
   uint32_t n_entries;

   uint32_t const_rd_length;
   uint32_t rd_offset;
   uint32_t rd_length;
   uint32_t wr_offset;
   uint32_t wr_length;
};

struct aub_viewer_decode_ctx {
   struct intel_batch_decode_bo (*get_bo)(void *user_data, bool ppgtt, uint64_t address);
   unsigned (*get_state_size)(void *user_data,
                              uint32_t offset_from_dynamic_state_base_addr);

   void (*display_shader)(void *user_data, const char *shader_desc, uint64_t address);
   void (*display_urb)(void *user_data, const struct aub_decode_urb_stage_state *stages);
   void (*edit_address)(void *user_data, uint64_t address, uint32_t length);

   void *user_data;

   const struct intel_device_info *devinfo;
   struct intel_spec *spec;
   enum intel_engine_class engine;

   struct aub_viewer_cfg *cfg;
   struct aub_viewer_decode_cfg *decode_cfg;

   uint64_t surface_base;
   uint64_t dynamic_base;
   uint64_t instruction_base;

   enum aub_decode_stage stage;
   uint32_t end_urb_offset;
   struct aub_decode_urb_stage_state urb_stages[AUB_DECODE_N_STAGE];

   int n_batch_buffer_start;
};

void aub_viewer_decode_ctx_init(struct aub_viewer_decode_ctx *ctx,
                                struct aub_viewer_cfg *cfg,
                                struct aub_viewer_decode_cfg *decode_cfg,
                                const struct intel_device_info *devinfo,
                                struct intel_spec *spec,
                                struct intel_batch_decode_bo (*get_bo)(void *, bool, uint64_t),
                                unsigned (*get_state_size)(void *, uint32_t),
                                void *user_data);

void aub_viewer_render_batch(struct aub_viewer_decode_ctx *ctx,
                             const void *batch, uint32_t batch_size,
                             uint64_t batch_addr, bool from_ring);

#endif /* AUBINATOR_VIEWER_H */
