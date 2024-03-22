/*
 * Copyright © 2020 Valve Corporation
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
 *
 */
#ifndef ACO_TEST_HELPERS_H
#define ACO_TEST_HELPERS_H

#include "vulkan/vulkan.h"

#include "framework.h"
#include <functional>

enum QoShaderDeclType {
   QoShaderDeclType_ubo,
   QoShaderDeclType_ssbo,
   QoShaderDeclType_img_buf,
   QoShaderDeclType_img,
   QoShaderDeclType_tex_buf,
   QoShaderDeclType_combined,
   QoShaderDeclType_tex,
   QoShaderDeclType_samp,
   QoShaderDeclType_in,
   QoShaderDeclType_out,
};

struct QoShaderDecl {
   const char* name;
   const char* type;
   QoShaderDeclType decl_type;
   // TODO: array size?
   unsigned location;
   unsigned component;
   unsigned binding;
   unsigned set;
};

struct QoShaderModuleCreateInfo {
   void* pNext;
   size_t spirvSize;
   const void* pSpirv;
   uint32_t declarationCount;
   const QoShaderDecl* pDeclarations;
   VkShaderStageFlagBits stage;
};

extern ac_shader_config config;
extern aco_shader_info info;
extern std::unique_ptr<aco::Program> program;
extern aco::Builder bld;
extern aco::Temp inputs[16];

namespace aco {
struct ra_test_policy;
}

void create_program(enum amd_gfx_level gfx_level, aco::Stage stage, unsigned wave_size = 64,
                    enum radeon_family family = CHIP_UNKNOWN);
bool setup_cs(const char* input_spec, enum amd_gfx_level gfx_level,
              enum radeon_family family = CHIP_UNKNOWN, const char* subvariant = "",
              unsigned wave_size = 64);

void finish_program(aco::Program* program, bool endpgm = true);
void finish_validator_test();
void finish_opt_test();
void finish_setup_reduce_temp_test();
void finish_ra_test(aco::ra_test_policy, bool lower = false);
void finish_optimizer_postRA_test();
void finish_to_hw_instr_test();
void finish_waitcnt_test();
void finish_insert_nops_test(bool endpgm = true);
void finish_form_hard_clause_test();
void finish_assembler_test();

void writeout(unsigned i, aco::Temp tmp = aco::Temp(0, aco::s1));
void writeout(unsigned i, aco::Builder::Result res);
void writeout(unsigned i, aco::Operand op);
void writeout(unsigned i, aco::Operand op0, aco::Operand op1);

aco::Temp fneg(aco::Temp src, aco::Builder b = bld);
aco::Temp fabs(aco::Temp src, aco::Builder b = bld);
aco::Temp f2f32(aco::Temp src, aco::Builder b = bld);
aco::Temp f2f16(aco::Temp src, aco::Builder b = bld);
aco::Temp u2u16(aco::Temp src, aco::Builder b = bld);
aco::Temp fadd(aco::Temp src0, aco::Temp src1, aco::Builder b = bld);
aco::Temp fmul(aco::Temp src0, aco::Temp src1, aco::Builder b = bld);
aco::Temp fma(aco::Temp src0, aco::Temp src1, aco::Temp src2, aco::Builder b = bld);
aco::Temp fsat(aco::Temp src, aco::Builder b = bld);
aco::Temp fmin(aco::Temp src0, aco::Temp src1, aco::Builder b = bld);
aco::Temp fmax(aco::Temp src0, aco::Temp src1, aco::Builder b = bld);
aco::Temp ext_ushort(aco::Temp src, unsigned idx, aco::Builder b = bld);
aco::Temp ext_ubyte(aco::Temp src, unsigned idx, aco::Builder b = bld);
void emit_divergent_if_else(aco::Program* prog, aco::Builder& b, aco::Operand cond,
                            std::function<void()> then, std::function<void()> els);

/* vulkan helpers */
VkDevice get_vk_device(enum amd_gfx_level gfx_level);
VkDevice get_vk_device(enum radeon_family family);

void print_pipeline_ir(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits stages,
                       const char* name, bool remove_encoding = false);

VkShaderModule __qoCreateShaderModule(VkDevice dev, const QoShaderModuleCreateInfo* info);

class PipelineBuilder {
public:
   /* inputs */
   VkDevice device;
   VkFormat color_outputs[16];
   VkFormat ds_output;
   VkPrimitiveTopology topology;
   VkSampleCountFlagBits samples;
   bool sample_shading_enable;
   float min_sample_shading;
   uint32_t patch_size;
   VkPipelineVertexInputStateCreateInfo vs_input;
   VkVertexInputBindingDescription vs_bindings[16];
   VkVertexInputAttributeDescription vs_attributes[16];
   VkPushConstantRange push_constant_range;
   uint64_t desc_layouts_used;
   unsigned num_desc_bindings[64];
   VkDescriptorSetLayoutBinding desc_bindings[64][64];
   VkPipelineShaderStageCreateInfo stages[5];
   VkShaderStageFlags owned_stages;

   /* outputs */
   VkGraphicsPipelineCreateInfo gfx_pipeline_info;
   VkComputePipelineCreateInfo cs_pipeline_info;
   VkDescriptorSetLayout desc_layouts[64];
   VkPipelineLayout pipeline_layout;
   VkRenderPass render_pass;
   VkPipeline pipeline;

   PipelineBuilder(VkDevice dev);
   ~PipelineBuilder();

   PipelineBuilder(const PipelineBuilder&) = delete;
   PipelineBuilder& operator=(const PipelineBuilder&) = delete;

   void add_desc_binding(VkShaderStageFlags stage_flags, uint32_t layout, uint32_t binding,
                         VkDescriptorType type, uint32_t count = 1);

   void add_vertex_binding(uint32_t binding, uint32_t stride,
                           VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX);
   void add_vertex_attribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);

   void add_resource_decls(QoShaderModuleCreateInfo* module);
   void add_io_decls(QoShaderModuleCreateInfo* module);

   void add_stage(VkShaderStageFlagBits stage, VkShaderModule module, const char* name = "main");
   void add_stage(VkShaderStageFlagBits stage, QoShaderModuleCreateInfo module,
                  const char* name = "main");
   void add_vsfs(VkShaderModule vs, VkShaderModule fs);
   void add_vsfs(QoShaderModuleCreateInfo vs, QoShaderModuleCreateInfo fs);
   void add_cs(VkShaderModule cs);
   void add_cs(QoShaderModuleCreateInfo cs);

   bool is_compute();

   void create_pipeline();

   void print_ir(VkShaderStageFlagBits stages, const char* name, bool remove_encoding = false);

private:
   void create_compute_pipeline();
   void create_graphics_pipeline();
};

#endif /* ACO_TEST_HELPERS_H */
