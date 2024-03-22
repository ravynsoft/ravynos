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
#include "helpers.h"

#include "common/amd_family.h"
#include "vulkan/vk_format.h"

#include <llvm-c/Target.h>

#include <mutex>
#include <sstream>
#include <stdio.h>

using namespace aco;

extern "C" {
PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName);
}

ac_shader_config config;
aco_shader_info info;
std::unique_ptr<Program> program;
Builder bld(NULL);
Temp inputs[16];

static VkInstance instance_cache[CHIP_LAST] = {VK_NULL_HANDLE};
static VkDevice device_cache[CHIP_LAST] = {VK_NULL_HANDLE};
static std::mutex create_device_mutex;

#define FUNCTION_LIST                                                                              \
   ITEM(CreateInstance)                                                                            \
   ITEM(DestroyInstance)                                                                           \
   ITEM(EnumeratePhysicalDevices)                                                                  \
   ITEM(GetPhysicalDeviceProperties2)                                                              \
   ITEM(CreateDevice)                                                                              \
   ITEM(DestroyDevice)                                                                             \
   ITEM(CreateShaderModule)                                                                        \
   ITEM(DestroyShaderModule)                                                                       \
   ITEM(CreateGraphicsPipelines)                                                                   \
   ITEM(CreateComputePipelines)                                                                    \
   ITEM(DestroyPipeline)                                                                           \
   ITEM(CreateDescriptorSetLayout)                                                                 \
   ITEM(DestroyDescriptorSetLayout)                                                                \
   ITEM(CreatePipelineLayout)                                                                      \
   ITEM(DestroyPipelineLayout)                                                                     \
   ITEM(CreateRenderPass)                                                                          \
   ITEM(DestroyRenderPass)                                                                         \
   ITEM(GetPipelineExecutablePropertiesKHR)                                                        \
   ITEM(GetPipelineExecutableInternalRepresentationsKHR)

#define ITEM(n) PFN_vk##n n;
FUNCTION_LIST
#undef ITEM

void
create_program(enum amd_gfx_level gfx_level, Stage stage, unsigned wave_size,
               enum radeon_family family)
{
   memset(&config, 0, sizeof(config));
   info.wave_size = wave_size;

   program.reset(new Program);
   aco::init_program(program.get(), stage, &info, gfx_level, family, false, &config);
   program->workgroup_size = UINT_MAX;
   calc_min_waves(program.get());

   program->debug.func = nullptr;
   program->debug.private_data = nullptr;

   program->debug.output = output;
   program->debug.shorten_messages = true;
   program->debug.func = nullptr;
   program->debug.private_data = nullptr;

   Block* block = program->create_and_insert_block();
   block->kind = block_kind_top_level;

   bld = Builder(program.get(), &program->blocks[0]);

   config.float_mode = program->blocks[0].fp_mode.val;
}

bool
setup_cs(const char* input_spec, enum amd_gfx_level gfx_level, enum radeon_family family,
         const char* subvariant, unsigned wave_size)
{
   if (!set_variant(gfx_level, subvariant))
      return false;

   memset(&info, 0, sizeof(info));
   create_program(gfx_level, compute_cs, wave_size, family);

   if (input_spec) {
      std::vector<RegClass> input_classes;
      while (input_spec[0]) {
         RegType type = input_spec[0] == 'v' ? RegType::vgpr : RegType::sgpr;
         unsigned size = input_spec[1] - '0';
         bool in_bytes = input_spec[2] == 'b';
         input_classes.push_back(RegClass::get(type, size * (in_bytes ? 1 : 4)));

         input_spec += 2 + in_bytes;
         while (input_spec[0] == ' ')
            input_spec++;
      }

      aco_ptr<Instruction> startpgm{create_instruction<Pseudo_instruction>(
         aco_opcode::p_startpgm, Format::PSEUDO, 0, input_classes.size())};
      for (unsigned i = 0; i < input_classes.size(); i++) {
         inputs[i] = bld.tmp(input_classes[i]);
         startpgm->definitions[i] = Definition(inputs[i]);
      }
      bld.insert(std::move(startpgm));
   }

   return true;
}

void
finish_program(Program* prog, bool endpgm)
{
   for (Block& BB : prog->blocks) {
      for (unsigned idx : BB.linear_preds)
         prog->blocks[idx].linear_succs.emplace_back(BB.index);
      for (unsigned idx : BB.logical_preds)
         prog->blocks[idx].logical_succs.emplace_back(BB.index);
   }

   if (endpgm) {
      for (Block& block : prog->blocks) {
         if (block.linear_succs.size() == 0) {
            block.kind |= block_kind_uniform;
            Builder(prog, &block).sopp(aco_opcode::s_endpgm);
         }
      }
   }
}

void
finish_validator_test()
{
   finish_program(program.get());
   aco_print_program(program.get(), output);
   fprintf(output, "Validation results:\n");
   if (aco::validate_ir(program.get()))
      fprintf(output, "Validation passed\n");
   else
      fprintf(output, "Validation failed\n");
}

void
finish_opt_test()
{
   finish_program(program.get());
   if (!aco::validate_ir(program.get())) {
      fail_test("Validation before optimization failed");
      return;
   }
   aco::optimize(program.get());
   if (!aco::validate_ir(program.get())) {
      fail_test("Validation after optimization failed");
      return;
   }
   aco_print_program(program.get(), output);
}

void
finish_setup_reduce_temp_test()
{
   finish_program(program.get());
   if (!aco::validate_ir(program.get())) {
      fail_test("Validation before setup_reduce_temp failed");
      return;
   }
   aco::setup_reduce_temp(program.get());
   if (!aco::validate_ir(program.get())) {
      fail_test("Validation after setup_reduce_temp failed");
      return;
   }
   aco_print_program(program.get(), output);
}

void
finish_ra_test(ra_test_policy policy, bool lower)
{
   finish_program(program.get());
   if (!aco::validate_ir(program.get())) {
      fail_test("Validation before register allocation failed");
      return;
   }

   program->workgroup_size = program->wave_size;
   aco::live live_vars = aco::live_var_analysis(program.get());
   aco::register_allocation(program.get(), live_vars.live_out, policy);

   if (aco::validate_ra(program.get())) {
      fail_test("Validation after register allocation failed");
      return;
   }

   if (lower) {
      aco::ssa_elimination(program.get());
      aco::lower_to_hw_instr(program.get());
   }

   aco_print_program(program.get(), output);
}

void
finish_optimizer_postRA_test()
{
   finish_program(program.get());
   aco::optimize_postRA(program.get());
   aco_print_program(program.get(), output);
}

void
finish_to_hw_instr_test()
{
   finish_program(program.get());
   aco::lower_to_hw_instr(program.get());
   aco_print_program(program.get(), output);
}

void
finish_waitcnt_test()
{
   finish_program(program.get());
   aco::insert_wait_states(program.get());
   aco_print_program(program.get(), output);
}

void
finish_insert_nops_test(bool endpgm)
{
   finish_program(program.get(), endpgm);
   aco::insert_NOPs(program.get());
   aco_print_program(program.get(), output);
}

void
finish_form_hard_clause_test()
{
   finish_program(program.get());
   aco::form_hard_clauses(program.get());
   aco_print_program(program.get(), output);
}

void
finish_assembler_test()
{
   finish_program(program.get());
   std::vector<uint32_t> binary;
   unsigned exec_size = emit_program(program.get(), binary);

   /* we could use CLRX for disassembly but that would require it to be
    * installed */
   if (program->gfx_level >= GFX8) {
      print_asm(program.get(), binary, exec_size / 4u, output);
   } else {
      // TODO: maybe we should use CLRX and skip this test if it's not available?
      for (uint32_t dword : binary)
         fprintf(output, "%.8x\n", dword);
   }
}

void
writeout(unsigned i, Temp tmp)
{
   if (tmp.id())
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(i), tmp);
   else
      bld.pseudo(aco_opcode::p_unit_test, Operand::c32(i));
}

void
writeout(unsigned i, aco::Builder::Result res)
{
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(i), res);
}

void
writeout(unsigned i, Operand op)
{
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(i), op);
}

void
writeout(unsigned i, Operand op0, Operand op1)
{
   bld.pseudo(aco_opcode::p_unit_test, Operand::c32(i), op0, op1);
}

Temp
fneg(Temp src, Builder b)
{
   if (src.bytes() == 2)
      return b.vop2(aco_opcode::v_mul_f16, b.def(v2b), Operand::c16(0xbc00u), src);
   else
      return b.vop2(aco_opcode::v_mul_f32, b.def(v1), Operand::c32(0xbf800000u), src);
}

Temp
fabs(Temp src, Builder b)
{
   if (src.bytes() == 2) {
      Builder::Result res =
         b.vop2_e64(aco_opcode::v_mul_f16, b.def(v2b), Operand::c16(0x3c00), src);
      res->valu().abs[1] = true;
      return res;
   } else {
      Builder::Result res =
         b.vop2_e64(aco_opcode::v_mul_f32, b.def(v1), Operand::c32(0x3f800000u), src);
      res->valu().abs[1] = true;
      return res;
   }
}

Temp
f2f32(Temp src, Builder b)
{
   return b.vop1(aco_opcode::v_cvt_f32_f16, b.def(v1), src);
}

Temp
f2f16(Temp src, Builder b)
{
   return b.vop1(aco_opcode::v_cvt_f16_f32, b.def(v2b), src);
}

Temp
u2u16(Temp src, Builder b)
{
   return b.pseudo(aco_opcode::p_extract_vector, b.def(v2b), src, Operand::zero());
}

Temp
fadd(Temp src0, Temp src1, Builder b)
{
   if (src0.bytes() == 2)
      return b.vop2(aco_opcode::v_add_f16, b.def(v2b), src0, src1);
   else
      return b.vop2(aco_opcode::v_add_f32, b.def(v1), src0, src1);
}

Temp
fmul(Temp src0, Temp src1, Builder b)
{
   if (src0.bytes() == 2)
      return b.vop2(aco_opcode::v_mul_f16, b.def(v2b), src0, src1);
   else
      return b.vop2(aco_opcode::v_mul_f32, b.def(v1), src0, src1);
}

Temp
fma(Temp src0, Temp src1, Temp src2, Builder b)
{
   if (src0.bytes() == 2)
      return b.vop3(aco_opcode::v_fma_f16, b.def(v2b), src0, src1, src2);
   else
      return b.vop3(aco_opcode::v_fma_f32, b.def(v1), src0, src1, src2);
}

Temp
fsat(Temp src, Builder b)
{
   if (src.bytes() == 2)
      return b.vop3(aco_opcode::v_med3_f16, b.def(v2b), Operand::c16(0u), Operand::c16(0x3c00u),
                    src);
   else
      return b.vop3(aco_opcode::v_med3_f32, b.def(v1), Operand::zero(), Operand::c32(0x3f800000u),
                    src);
}

Temp
fmin(Temp src0, Temp src1, Builder b)
{
   return b.vop2(aco_opcode::v_min_f32, b.def(v1), src0, src1);
}

Temp
fmax(Temp src0, Temp src1, Builder b)
{
   return b.vop2(aco_opcode::v_max_f32, b.def(v1), src0, src1);
}

Temp
ext_ushort(Temp src, unsigned idx, Builder b)
{
   return b.pseudo(aco_opcode::p_extract, b.def(src.regClass()), src, Operand::c32(idx),
                   Operand::c32(16u), Operand::c32(false));
}

Temp
ext_ubyte(Temp src, unsigned idx, Builder b)
{
   return b.pseudo(aco_opcode::p_extract, b.def(src.regClass()), src, Operand::c32(idx),
                   Operand::c32(8u), Operand::c32(false));
}

void
emit_divergent_if_else(Program* prog, aco::Builder& b, Operand cond, std::function<void()> then,
                       std::function<void()> els)
{
   prog->blocks.reserve(prog->blocks.size() + 6);

   Block* if_block = &prog->blocks.back();
   Block* then_logical = prog->create_and_insert_block();
   Block* then_linear = prog->create_and_insert_block();
   Block* invert = prog->create_and_insert_block();
   Block* else_logical = prog->create_and_insert_block();
   Block* else_linear = prog->create_and_insert_block();
   Block* endif_block = prog->create_and_insert_block();

   if_block->kind |= block_kind_branch;
   invert->kind |= block_kind_invert;
   endif_block->kind |= block_kind_merge | (if_block->kind & block_kind_top_level);

   /* Set up logical CF */
   then_logical->logical_preds.push_back(if_block->index);
   else_logical->logical_preds.push_back(if_block->index);
   endif_block->logical_preds.push_back(then_logical->index);
   endif_block->logical_preds.push_back(else_logical->index);

   /* Set up linear CF */
   then_logical->linear_preds.push_back(if_block->index);
   then_linear->linear_preds.push_back(if_block->index);
   invert->linear_preds.push_back(then_logical->index);
   invert->linear_preds.push_back(then_linear->index);
   else_logical->linear_preds.push_back(invert->index);
   else_linear->linear_preds.push_back(invert->index);
   endif_block->linear_preds.push_back(else_logical->index);
   endif_block->linear_preds.push_back(else_linear->index);

   PhysReg saved_exec_reg(84);

   b.reset(if_block);
   Temp saved_exec = b.sop1(Builder::s_and_saveexec, b.def(b.lm, saved_exec_reg),
                            Definition(scc, s1), Definition(exec, b.lm), cond, Operand(exec, b.lm));
   b.branch(aco_opcode::p_cbranch_nz, Definition(vcc, bld.lm), then_logical->index,
            then_linear->index);

   b.reset(then_logical);
   b.pseudo(aco_opcode::p_logical_start);
   then();
   b.pseudo(aco_opcode::p_logical_end);
   b.branch(aco_opcode::p_branch, Definition(vcc, bld.lm), invert->index);

   b.reset(then_linear);
   b.branch(aco_opcode::p_branch, Definition(vcc, bld.lm), invert->index);

   b.reset(invert);
   b.sop2(Builder::s_andn2, Definition(exec, bld.lm), Definition(scc, s1),
          Operand(saved_exec, saved_exec_reg), Operand(exec, bld.lm));
   b.branch(aco_opcode::p_cbranch_nz, Definition(vcc, bld.lm), else_logical->index,
            else_linear->index);

   b.reset(else_logical);
   b.pseudo(aco_opcode::p_logical_start);
   els();
   b.pseudo(aco_opcode::p_logical_end);
   b.branch(aco_opcode::p_branch, Definition(vcc, bld.lm), endif_block->index);

   b.reset(else_linear);
   b.branch(aco_opcode::p_branch, Definition(vcc, bld.lm), endif_block->index);

   b.reset(endif_block);
   b.pseudo(aco_opcode::p_parallelcopy, Definition(exec, bld.lm),
            Operand(saved_exec, saved_exec_reg));
}

VkDevice
get_vk_device(enum amd_gfx_level gfx_level)
{
   enum radeon_family family;
   switch (gfx_level) {
   case GFX6: family = CHIP_TAHITI; break;
   case GFX7: family = CHIP_BONAIRE; break;
   case GFX8: family = CHIP_POLARIS10; break;
   case GFX9: family = CHIP_VEGA10; break;
   case GFX10: family = CHIP_NAVI10; break;
   case GFX10_3: family = CHIP_NAVI21; break;
   case GFX11: family = CHIP_NAVI31; break;
   default: family = CHIP_UNKNOWN; break;
   }
   return get_vk_device(family);
}

VkDevice
get_vk_device(enum radeon_family family)
{
   assert(family != CHIP_UNKNOWN);

   std::lock_guard<std::mutex> guard(create_device_mutex);

   if (device_cache[family])
      return device_cache[family];

   setenv("RADV_FORCE_FAMILY", ac_get_family_name(family), 1);

   VkApplicationInfo app_info = {};
   app_info.pApplicationName = "aco_tests";
   app_info.apiVersion = VK_API_VERSION_1_2;
   VkInstanceCreateInfo instance_create_info = {};
   instance_create_info.pApplicationInfo = &app_info;
   instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   ASSERTED VkResult result = ((PFN_vkCreateInstance)vk_icdGetInstanceProcAddr(
      NULL, "vkCreateInstance"))(&instance_create_info, NULL, &instance_cache[family]);
   assert(result == VK_SUCCESS);

#define ITEM(n) n = (PFN_vk##n)vk_icdGetInstanceProcAddr(instance_cache[family], "vk" #n);
   FUNCTION_LIST
#undef ITEM

   uint32_t device_count = 1;
   VkPhysicalDevice device = VK_NULL_HANDLE;
   result = EnumeratePhysicalDevices(instance_cache[family], &device_count, &device);
   assert(result == VK_SUCCESS);
   assert(device != VK_NULL_HANDLE);

   VkDeviceCreateInfo device_create_info = {};
   device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   static const char* extensions[] = {"VK_KHR_pipeline_executable_properties"};
   device_create_info.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);
   device_create_info.ppEnabledExtensionNames = extensions;
   result = CreateDevice(device, &device_create_info, NULL, &device_cache[family]);

   return device_cache[family];
}

static struct DestroyDevices {
   ~DestroyDevices()
   {
      for (unsigned i = 0; i < CHIP_LAST; i++) {
         if (!device_cache[i])
            continue;
         DestroyDevice(device_cache[i], NULL);
         DestroyInstance(instance_cache[i], NULL);
      }
   }
} destroy_devices;

void
print_pipeline_ir(VkDevice device, VkPipeline pipeline, VkShaderStageFlagBits stages,
                  const char* name, bool remove_encoding)
{
   uint32_t executable_count = 16;
   VkPipelineExecutablePropertiesKHR executables[16];
   VkPipelineInfoKHR pipeline_info;
   pipeline_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR;
   pipeline_info.pNext = NULL;
   pipeline_info.pipeline = pipeline;
   ASSERTED VkResult result =
      GetPipelineExecutablePropertiesKHR(device, &pipeline_info, &executable_count, executables);
   assert(result == VK_SUCCESS);

   uint32_t executable = 0;
   for (; executable < executable_count; executable++) {
      if (executables[executable].stages == stages)
         break;
   }
   assert(executable != executable_count);

   VkPipelineExecutableInfoKHR exec_info;
   exec_info.sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR;
   exec_info.pNext = NULL;
   exec_info.pipeline = pipeline;
   exec_info.executableIndex = executable;

   uint32_t ir_count = 16;
   VkPipelineExecutableInternalRepresentationKHR ir[16];
   memset(ir, 0, sizeof(ir));
   result = GetPipelineExecutableInternalRepresentationsKHR(device, &exec_info, &ir_count, ir);
   assert(result == VK_SUCCESS);

   VkPipelineExecutableInternalRepresentationKHR* requested_ir = nullptr;
   for (unsigned i = 0; i < ir_count; ++i) {
      if (strcmp(ir[i].name, name) == 0) {
         requested_ir = &ir[i];
         break;
      }
   }
   assert(requested_ir && "Could not find requested IR");

   char* data = (char*)malloc(requested_ir->dataSize);
   requested_ir->pData = data;
   result = GetPipelineExecutableInternalRepresentationsKHR(device, &exec_info, &ir_count, ir);
   assert(result == VK_SUCCESS);

   if (remove_encoding) {
      for (char* c = data; *c; c++) {
         if (*c == ';') {
            for (; *c && *c != '\n'; c++)
               *c = ' ';
         }
      }
   }

   fprintf(output, "%s", data);
   free(data);
}

VkShaderModule
__qoCreateShaderModule(VkDevice dev, const QoShaderModuleCreateInfo* module_info)
{
   VkShaderModuleCreateInfo vk_module_info;
   vk_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   vk_module_info.pNext = NULL;
   vk_module_info.flags = 0;
   vk_module_info.codeSize = module_info->spirvSize;
   vk_module_info.pCode = (const uint32_t*)module_info->pSpirv;

   VkShaderModule module;
   ASSERTED VkResult result = CreateShaderModule(dev, &vk_module_info, NULL, &module);
   assert(result == VK_SUCCESS);

   return module;
}

PipelineBuilder::PipelineBuilder(VkDevice dev)
{
   memset(this, 0, sizeof(*this));
   topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   device = dev;
}

PipelineBuilder::~PipelineBuilder()
{
   DestroyPipeline(device, pipeline, NULL);

   for (unsigned i = 0; i < (is_compute() ? 1 : gfx_pipeline_info.stageCount); i++) {
      VkPipelineShaderStageCreateInfo* stage_info = &stages[i];
      if (owned_stages & stage_info->stage)
         DestroyShaderModule(device, stage_info->module, NULL);
   }

   DestroyPipelineLayout(device, pipeline_layout, NULL);

   for (unsigned i = 0; i < util_bitcount64(desc_layouts_used); i++)
      DestroyDescriptorSetLayout(device, desc_layouts[i], NULL);

   DestroyRenderPass(device, render_pass, NULL);
}

void
PipelineBuilder::add_desc_binding(VkShaderStageFlags stage_flags, uint32_t layout, uint32_t binding,
                                  VkDescriptorType type, uint32_t count)
{
   desc_layouts_used |= 1ull << layout;
   desc_bindings[layout][num_desc_bindings[layout]++] = {binding, type, count, stage_flags, NULL};
}

void
PipelineBuilder::add_vertex_binding(uint32_t binding, uint32_t stride, VkVertexInputRate rate)
{
   vs_bindings[vs_input.vertexBindingDescriptionCount++] = {binding, stride, rate};
}

void
PipelineBuilder::add_vertex_attribute(uint32_t location, uint32_t binding, VkFormat format,
                                      uint32_t offset)
{
   vs_attributes[vs_input.vertexAttributeDescriptionCount++] = {location, binding, format, offset};
}

void
PipelineBuilder::add_resource_decls(QoShaderModuleCreateInfo* module)
{
   for (unsigned i = 0; i < module->declarationCount; i++) {
      const QoShaderDecl* decl = &module->pDeclarations[i];
      switch (decl->decl_type) {
      case QoShaderDeclType_ubo:
         add_desc_binding(module->stage, decl->set, decl->binding,
                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
         break;
      case QoShaderDeclType_ssbo:
         add_desc_binding(module->stage, decl->set, decl->binding,
                          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
         break;
      case QoShaderDeclType_img_buf:
         add_desc_binding(module->stage, decl->set, decl->binding,
                          VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
         break;
      case QoShaderDeclType_img:
         add_desc_binding(module->stage, decl->set, decl->binding,
                          VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
         break;
      case QoShaderDeclType_tex_buf:
         add_desc_binding(module->stage, decl->set, decl->binding,
                          VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
         break;
      case QoShaderDeclType_combined:
         add_desc_binding(module->stage, decl->set, decl->binding,
                          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
         break;
      case QoShaderDeclType_tex:
         add_desc_binding(module->stage, decl->set, decl->binding,
                          VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
         break;
      case QoShaderDeclType_samp:
         add_desc_binding(module->stage, decl->set, decl->binding, VK_DESCRIPTOR_TYPE_SAMPLER);
         break;
      default: break;
      }
   }
}

void
PipelineBuilder::add_io_decls(QoShaderModuleCreateInfo* module)
{
   unsigned next_vtx_offset = 0;
   for (unsigned i = 0; i < module->declarationCount; i++) {
      const QoShaderDecl* decl = &module->pDeclarations[i];
      switch (decl->decl_type) {
      case QoShaderDeclType_in:
         if (module->stage == VK_SHADER_STAGE_VERTEX_BIT) {
            if (!strcmp(decl->type, "float") || decl->type[0] == 'v')
               add_vertex_attribute(decl->location, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
                                    next_vtx_offset);
            else if (decl->type[0] == 'u')
               add_vertex_attribute(decl->location, 0, VK_FORMAT_R32G32B32A32_UINT,
                                    next_vtx_offset);
            else if (decl->type[0] == 'i')
               add_vertex_attribute(decl->location, 0, VK_FORMAT_R32G32B32A32_SINT,
                                    next_vtx_offset);
            next_vtx_offset += 16;
         }
         break;
      case QoShaderDeclType_out:
         if (module->stage == VK_SHADER_STAGE_FRAGMENT_BIT) {
            if (!strcmp(decl->type, "float") || decl->type[0] == 'v')
               color_outputs[decl->location] = VK_FORMAT_R32G32B32A32_SFLOAT;
            else if (decl->type[0] == 'u')
               color_outputs[decl->location] = VK_FORMAT_R32G32B32A32_UINT;
            else if (decl->type[0] == 'i')
               color_outputs[decl->location] = VK_FORMAT_R32G32B32A32_SINT;
         }
         break;
      default: break;
      }
   }
   if (next_vtx_offset)
      add_vertex_binding(0, next_vtx_offset);
}

void
PipelineBuilder::add_stage(VkShaderStageFlagBits stage, VkShaderModule module, const char* name)
{
   VkPipelineShaderStageCreateInfo* stage_info;
   if (stage == VK_SHADER_STAGE_COMPUTE_BIT)
      stage_info = &stages[0];
   else
      stage_info = &stages[gfx_pipeline_info.stageCount++];
   stage_info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   stage_info->pNext = NULL;
   stage_info->flags = 0;
   stage_info->stage = stage;
   stage_info->module = module;
   stage_info->pName = name;
   stage_info->pSpecializationInfo = NULL;
   owned_stages |= stage;
}

void
PipelineBuilder::add_stage(VkShaderStageFlagBits stage, QoShaderModuleCreateInfo module,
                           const char* name)
{
   add_stage(stage, __qoCreateShaderModule(device, &module), name);
   add_resource_decls(&module);
   add_io_decls(&module);
}

void
PipelineBuilder::add_vsfs(VkShaderModule vs, VkShaderModule fs)
{
   add_stage(VK_SHADER_STAGE_VERTEX_BIT, vs);
   add_stage(VK_SHADER_STAGE_FRAGMENT_BIT, fs);
}

void
PipelineBuilder::add_vsfs(QoShaderModuleCreateInfo vs, QoShaderModuleCreateInfo fs)
{
   add_stage(VK_SHADER_STAGE_VERTEX_BIT, vs);
   add_stage(VK_SHADER_STAGE_FRAGMENT_BIT, fs);
}

void
PipelineBuilder::add_cs(VkShaderModule cs)
{
   add_stage(VK_SHADER_STAGE_COMPUTE_BIT, cs);
}

void
PipelineBuilder::add_cs(QoShaderModuleCreateInfo cs)
{
   add_stage(VK_SHADER_STAGE_COMPUTE_BIT, cs);
}

bool
PipelineBuilder::is_compute()
{
   return gfx_pipeline_info.stageCount == 0;
}

void
PipelineBuilder::create_compute_pipeline()
{
   VkComputePipelineCreateInfo create_info;
   create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
   create_info.pNext = NULL;
   create_info.flags = VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
   create_info.stage = stages[0];
   create_info.layout = pipeline_layout;
   create_info.basePipelineHandle = VK_NULL_HANDLE;
   create_info.basePipelineIndex = 0;

   ASSERTED VkResult result =
      CreateComputePipelines(device, VK_NULL_HANDLE, 1, &create_info, NULL, &pipeline);
   assert(result == VK_SUCCESS);
}

void
PipelineBuilder::create_graphics_pipeline()
{
   /* create the create infos */
   if (!samples)
      samples = VK_SAMPLE_COUNT_1_BIT;

   unsigned num_color_attachments = 0;
   VkPipelineColorBlendAttachmentState blend_attachment_states[16];
   VkAttachmentReference color_attachments[16];
   VkAttachmentDescription attachment_descs[17];
   for (unsigned i = 0; i < 16; i++) {
      if (color_outputs[i] == VK_FORMAT_UNDEFINED)
         continue;

      VkAttachmentDescription* desc = &attachment_descs[num_color_attachments];
      desc->flags = 0;
      desc->format = color_outputs[i];
      desc->samples = samples;
      desc->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      desc->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc->initialLayout = VK_IMAGE_LAYOUT_GENERAL;
      desc->finalLayout = VK_IMAGE_LAYOUT_GENERAL;

      VkAttachmentReference* ref = &color_attachments[num_color_attachments];
      ref->attachment = num_color_attachments;
      ref->layout = VK_IMAGE_LAYOUT_GENERAL;

      VkPipelineColorBlendAttachmentState* blend = &blend_attachment_states[num_color_attachments];
      blend->blendEnable = false;
      blend->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

      num_color_attachments++;
   }

   unsigned num_attachments = num_color_attachments;
   VkAttachmentReference ds_attachment;
   if (ds_output != VK_FORMAT_UNDEFINED) {
      VkAttachmentDescription* desc = &attachment_descs[num_attachments];
      desc->flags = 0;
      desc->format = ds_output;
      desc->samples = samples;
      desc->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      desc->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
      desc->stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      desc->initialLayout = VK_IMAGE_LAYOUT_GENERAL;
      desc->finalLayout = VK_IMAGE_LAYOUT_GENERAL;

      ds_attachment.attachment = num_color_attachments;
      ds_attachment.layout = VK_IMAGE_LAYOUT_GENERAL;

      num_attachments++;
   }

   vs_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   vs_input.pNext = NULL;
   vs_input.flags = 0;
   vs_input.pVertexBindingDescriptions = vs_bindings;
   vs_input.pVertexAttributeDescriptions = vs_attributes;

   VkPipelineInputAssemblyStateCreateInfo assembly_state;
   assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   assembly_state.pNext = NULL;
   assembly_state.flags = 0;
   assembly_state.topology = topology;
   assembly_state.primitiveRestartEnable = false;

   VkPipelineTessellationStateCreateInfo tess_state;
   tess_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
   tess_state.pNext = NULL;
   tess_state.flags = 0;
   tess_state.patchControlPoints = patch_size;

   VkPipelineViewportStateCreateInfo viewport_state;
   viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewport_state.pNext = NULL;
   viewport_state.flags = 0;
   viewport_state.viewportCount = 1;
   viewport_state.pViewports = NULL;
   viewport_state.scissorCount = 1;
   viewport_state.pScissors = NULL;

   VkPipelineRasterizationStateCreateInfo rasterization_state;
   rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   rasterization_state.pNext = NULL;
   rasterization_state.flags = 0;
   rasterization_state.depthClampEnable = false;
   rasterization_state.rasterizerDiscardEnable = false;
   rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
   rasterization_state.cullMode = VK_CULL_MODE_NONE;
   rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   rasterization_state.depthBiasEnable = false;
   rasterization_state.lineWidth = 1.0;

   VkPipelineMultisampleStateCreateInfo ms_state;
   ms_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   ms_state.pNext = NULL;
   ms_state.flags = 0;
   ms_state.rasterizationSamples = samples;
   ms_state.sampleShadingEnable = sample_shading_enable;
   ms_state.minSampleShading = min_sample_shading;
   VkSampleMask sample_mask = 0xffffffff;
   ms_state.pSampleMask = &sample_mask;
   ms_state.alphaToCoverageEnable = false;
   ms_state.alphaToOneEnable = false;

   VkPipelineDepthStencilStateCreateInfo ds_state;
   ds_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   ds_state.pNext = NULL;
   ds_state.flags = 0;
   ds_state.depthTestEnable = ds_output != VK_FORMAT_UNDEFINED;
   ds_state.depthWriteEnable = true;
   ds_state.depthCompareOp = VK_COMPARE_OP_ALWAYS;
   ds_state.depthBoundsTestEnable = false;
   ds_state.stencilTestEnable = true;
   ds_state.front.failOp = VK_STENCIL_OP_KEEP;
   ds_state.front.passOp = VK_STENCIL_OP_REPLACE;
   ds_state.front.depthFailOp = VK_STENCIL_OP_REPLACE;
   ds_state.front.compareOp = VK_COMPARE_OP_ALWAYS;
   ds_state.front.compareMask = 0xffffffff, ds_state.front.writeMask = 0;
   ds_state.front.reference = 0;
   ds_state.back = ds_state.front;

   VkPipelineColorBlendStateCreateInfo color_blend_state;
   color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   color_blend_state.pNext = NULL;
   color_blend_state.flags = 0;
   color_blend_state.logicOpEnable = false;
   color_blend_state.attachmentCount = num_color_attachments;
   color_blend_state.pAttachments = blend_attachment_states;

   VkDynamicState dynamic_states[9] = {VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_SCISSOR,
                                       VK_DYNAMIC_STATE_LINE_WIDTH,
                                       VK_DYNAMIC_STATE_DEPTH_BIAS,
                                       VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                                       VK_DYNAMIC_STATE_DEPTH_BOUNDS,
                                       VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
                                       VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                                       VK_DYNAMIC_STATE_STENCIL_REFERENCE};

   VkPipelineDynamicStateCreateInfo dynamic_state;
   dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   dynamic_state.pNext = NULL;
   dynamic_state.flags = 0;
   dynamic_state.dynamicStateCount = sizeof(dynamic_states) / sizeof(VkDynamicState);
   dynamic_state.pDynamicStates = dynamic_states;

   gfx_pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   gfx_pipeline_info.pNext = NULL;
   gfx_pipeline_info.flags = VK_PIPELINE_CREATE_CAPTURE_INTERNAL_REPRESENTATIONS_BIT_KHR;
   gfx_pipeline_info.pVertexInputState = &vs_input;
   gfx_pipeline_info.pInputAssemblyState = &assembly_state;
   gfx_pipeline_info.pTessellationState = &tess_state;
   gfx_pipeline_info.pViewportState = &viewport_state;
   gfx_pipeline_info.pRasterizationState = &rasterization_state;
   gfx_pipeline_info.pMultisampleState = &ms_state;
   gfx_pipeline_info.pDepthStencilState = &ds_state;
   gfx_pipeline_info.pColorBlendState = &color_blend_state;
   gfx_pipeline_info.pDynamicState = &dynamic_state;
   gfx_pipeline_info.subpass = 0;

   /* create the objects used to create the pipeline */
   VkSubpassDescription subpass;
   subpass.flags = 0;
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.inputAttachmentCount = 0;
   subpass.pInputAttachments = NULL;
   subpass.colorAttachmentCount = num_color_attachments;
   subpass.pColorAttachments = color_attachments;
   subpass.pResolveAttachments = NULL;
   subpass.pDepthStencilAttachment = ds_output == VK_FORMAT_UNDEFINED ? NULL : &ds_attachment;
   subpass.preserveAttachmentCount = 0;
   subpass.pPreserveAttachments = NULL;

   VkRenderPassCreateInfo renderpass_info;
   renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderpass_info.pNext = NULL;
   renderpass_info.flags = 0;
   renderpass_info.attachmentCount = num_attachments;
   renderpass_info.pAttachments = attachment_descs;
   renderpass_info.subpassCount = 1;
   renderpass_info.pSubpasses = &subpass;
   renderpass_info.dependencyCount = 0;
   renderpass_info.pDependencies = NULL;

   ASSERTED VkResult result = CreateRenderPass(device, &renderpass_info, NULL, &render_pass);
   assert(result == VK_SUCCESS);

   gfx_pipeline_info.layout = pipeline_layout;
   gfx_pipeline_info.renderPass = render_pass;

   /* create the pipeline */
   gfx_pipeline_info.pStages = stages;

   result = CreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &gfx_pipeline_info, NULL, &pipeline);
   assert(result == VK_SUCCESS);
}

void
PipelineBuilder::create_pipeline()
{
   unsigned num_desc_layouts = 0;
   for (unsigned i = 0; i < 64; i++) {
      if (!(desc_layouts_used & (1ull << i)))
         continue;

      VkDescriptorSetLayoutCreateInfo desc_layout_info;
      desc_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      desc_layout_info.pNext = NULL;
      desc_layout_info.flags = 0;
      desc_layout_info.bindingCount = num_desc_bindings[i];
      desc_layout_info.pBindings = desc_bindings[i];

      ASSERTED VkResult result = CreateDescriptorSetLayout(device, &desc_layout_info, NULL,
                                                           &desc_layouts[num_desc_layouts]);
      assert(result == VK_SUCCESS);
      num_desc_layouts++;
   }

   VkPipelineLayoutCreateInfo pipeline_layout_info;
   pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipeline_layout_info.pNext = NULL;
   pipeline_layout_info.flags = 0;
   pipeline_layout_info.pushConstantRangeCount = 1;
   pipeline_layout_info.pPushConstantRanges = &push_constant_range;
   pipeline_layout_info.setLayoutCount = num_desc_layouts;
   pipeline_layout_info.pSetLayouts = desc_layouts;

   ASSERTED VkResult result =
      CreatePipelineLayout(device, &pipeline_layout_info, NULL, &pipeline_layout);
   assert(result == VK_SUCCESS);

   if (is_compute())
      create_compute_pipeline();
   else
      create_graphics_pipeline();
}

void
PipelineBuilder::print_ir(VkShaderStageFlagBits stage_flags, const char* name, bool remove_encoding)
{
   if (!pipeline)
      create_pipeline();
   print_pipeline_ir(device, pipeline, stage_flags, name, remove_encoding);
}
