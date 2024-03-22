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
 */
#include "helpers.h"

class Volatile : public spirv_test {};

TEST_F(Volatile, opload_volatile)
{
   /*
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
  %_struct_7 = OpTypeStruct %uint
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
          %9 = OpVariable %_ptr_Uniform__struct_7 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Uniform_uint %9 %int_0
         %14 = OpLoad %uint %13 Volatile
               OpStore %13 %14
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010300, 0x00070000, 0x0000000f, 0x00000000, 0x00020011,
      0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
      0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0005000f, 0x00000005,
      0x00000002, 0x6e69616d, 0x00000000, 0x00060010, 0x00000002, 0x00000011,
      0x00000001, 0x00000001, 0x00000001, 0x00050048, 0x00000003, 0x00000000,
      0x00000023, 0x00000000, 0x00030047, 0x00000003, 0x00000003, 0x00040047,
      0x00000004, 0x00000022, 0x00000000, 0x00040047, 0x00000004, 0x00000021,
      0x00000000, 0x00020013, 0x00000005, 0x00030021, 0x00000006, 0x00000005,
      0x00040015, 0x00000007, 0x00000020, 0x00000000, 0x0003001e, 0x00000003,
      0x00000007, 0x00040020, 0x00000008, 0x00000002, 0x00000003, 0x0004003b,
      0x00000008, 0x00000004, 0x00000002, 0x00040015, 0x00000009, 0x00000020,
      0x00000001, 0x0004002b, 0x00000009, 0x0000000a, 0x00000000, 0x00040020,
      0x0000000b, 0x00000002, 0x00000007, 0x00050036, 0x00000005, 0x00000002,
      0x00000000, 0x00000006, 0x000200f8, 0x0000000c, 0x00050041, 0x0000000b,
      0x0000000d, 0x00000004, 0x0000000a, 0x0005003d, 0x00000007, 0x0000000e,
      0x0000000d, 0x00000001, 0x0003003e, 0x0000000d, 0x0000000e, 0x000100fd,
      0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_load_deref);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}

TEST_F(Volatile, opstore_volatile)
{
   /*
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
  %_struct_7 = OpTypeStruct %uint
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
          %9 = OpVariable %_ptr_Uniform__struct_7 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Uniform_uint %9 %int_0
         %14 = OpLoad %uint %13
               OpStore %13 %14 Volatile
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010300, 0x00070000, 0x0000000f, 0x00000000, 0x00020011,
      0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
      0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0005000f, 0x00000005,
      0x00000002, 0x6e69616d, 0x00000000, 0x00060010, 0x00000002, 0x00000011,
      0x00000001, 0x00000001, 0x00000001, 0x00050048, 0x00000003, 0x00000000,
      0x00000023, 0x00000000, 0x00030047, 0x00000003, 0x00000003, 0x00040047,
      0x00000004, 0x00000022, 0x00000000, 0x00040047, 0x00000004, 0x00000021,
      0x00000000, 0x00020013, 0x00000005, 0x00030021, 0x00000006, 0x00000005,
      0x00040015, 0x00000007, 0x00000020, 0x00000000, 0x0003001e, 0x00000003,
      0x00000007, 0x00040020, 0x00000008, 0x00000002, 0x00000003, 0x0004003b,
      0x00000008, 0x00000004, 0x00000002, 0x00040015, 0x00000009, 0x00000020,
      0x00000001, 0x0004002b, 0x00000009, 0x0000000a, 0x00000000, 0x00040020,
      0x0000000b, 0x00000002, 0x00000007, 0x00050036, 0x00000005, 0x00000002,
      0x00000000, 0x00000006, 0x000200f8, 0x0000000c, 0x00050041, 0x0000000b,
      0x0000000d, 0x00000004, 0x0000000a, 0x0004003d, 0x00000007, 0x0000000e,
      0x0000000d, 0x0004003e, 0x0000000d, 0x0000000e, 0x00000001, 0x000100fd,
      0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_store_deref);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}

TEST_F(Volatile, opcopymemory_volatile_both)
{
   /*
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main"
               OpExecutionMode %4 LocalSize 1 1 1
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 BufferBlock
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
  %_struct_7 = OpTypeStruct %uint
%_ptr_Uniform__struct_7 = OpTypePointer Uniform %_struct_7
          %9 = OpVariable %_ptr_Uniform__struct_7 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_Uniform_uint %9 %int_0
               OpCopyMemory %13 %13 Volatile
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010300, 0x00070000, 0x0000000e, 0x00000000, 0x00020011,
      0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
      0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0005000f, 0x00000005,
      0x00000002, 0x6e69616d, 0x00000000, 0x00060010, 0x00000002, 0x00000011,
      0x00000001, 0x00000001, 0x00000001, 0x00050048, 0x00000003, 0x00000000,
      0x00000023, 0x00000000, 0x00030047, 0x00000003, 0x00000003, 0x00040047,
      0x00000004, 0x00000022, 0x00000000, 0x00040047, 0x00000004, 0x00000021,
      0x00000000, 0x00020013, 0x00000005, 0x00030021, 0x00000006, 0x00000005,
      0x00040015, 0x00000007, 0x00000020, 0x00000000, 0x0003001e, 0x00000003,
      0x00000007, 0x00040020, 0x00000008, 0x00000002, 0x00000003, 0x0004003b,
      0x00000008, 0x00000004, 0x00000002, 0x00040015, 0x00000009, 0x00000020,
      0x00000001, 0x0004002b, 0x00000009, 0x0000000a, 0x00000000, 0x00040020,
      0x0000000b, 0x00000002, 0x00000007, 0x00050036, 0x00000005, 0x00000002,
      0x00000000, 0x00000006, 0x000200f8, 0x0000000c, 0x00050041, 0x0000000b,
      0x0000000d, 0x00000004, 0x0000000a, 0x0004003f, 0x0000000d, 0x0000000d,
      0x00000001, 0x000100fd, 0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_load_deref);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);

   intrinsic = find_intrinsic(nir_intrinsic_store_deref);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}

TEST_F(Volatile, opcopymemory_volatile_target)
{
   /*
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main" %9
               OpExecutionMode %4 LocalSize 1 1 1
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 Block
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
  %_struct_7 = OpTypeStruct %uint
%_ptr_StorageBuffer__struct_7 = OpTypePointer StorageBuffer %_struct_7
          %9 = OpVariable %_ptr_StorageBuffer__struct_7 StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_StorageBuffer_uint %9 %int_0
               OpCopyMemory %13 %13 Volatile None
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010500, 0x00070000, 0x0000000e, 0x00000000, 0x00020011,
      0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
      0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0006000f, 0x00000005,
      0x00000002, 0x6e69616d, 0x00000000, 0x00000003, 0x00060010, 0x00000002,
      0x00000011, 0x00000001, 0x00000001, 0x00000001, 0x00050048, 0x00000004,
      0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x00000004, 0x00000002,
      0x00040047, 0x00000003, 0x00000022, 0x00000000, 0x00040047, 0x00000003,
      0x00000021, 0x00000000, 0x00020013, 0x00000005, 0x00030021, 0x00000006,
      0x00000005, 0x00040015, 0x00000007, 0x00000020, 0x00000000, 0x0003001e,
      0x00000004, 0x00000007, 0x00040020, 0x00000008, 0x0000000c, 0x00000004,
      0x0004003b, 0x00000008, 0x00000003, 0x0000000c, 0x00040015, 0x00000009,
      0x00000020, 0x00000001, 0x0004002b, 0x00000009, 0x0000000a, 0x00000000,
      0x00040020, 0x0000000b, 0x0000000c, 0x00000007, 0x00050036, 0x00000005,
      0x00000002, 0x00000000, 0x00000006, 0x000200f8, 0x0000000c, 0x00050041,
      0x0000000b, 0x0000000d, 0x00000003, 0x0000000a, 0x0005003f, 0x0000000d,
      0x0000000d, 0x00000001, 0x00000000, 0x000100fd, 0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_load_deref);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_EQ(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);

   intrinsic = find_intrinsic(nir_intrinsic_store_deref);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}

TEST_F(Volatile, opcopymemory_volatile_source)
{
   /*
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %4 "main" %9
               OpExecutionMode %4 LocalSize 1 1 1
               OpMemberDecorate %_struct_7 0 Offset 0
               OpDecorate %_struct_7 Block
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
  %_struct_7 = OpTypeStruct %uint
%_ptr_StorageBuffer__struct_7 = OpTypePointer StorageBuffer %_struct_7
          %9 = OpVariable %_ptr_StorageBuffer__struct_7 StorageBuffer
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %13 = OpAccessChain %_ptr_StorageBuffer_uint %9 %int_0
               OpCopyMemory %13 %13 None Volatile
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010500, 0x00070000, 0x0000000e, 0x00000000, 0x00020011,
      0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
      0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0006000f, 0x00000005,
      0x00000002, 0x6e69616d, 0x00000000, 0x00000003, 0x00060010, 0x00000002,
      0x00000011, 0x00000001, 0x00000001, 0x00000001, 0x00050048, 0x00000004,
      0x00000000, 0x00000023, 0x00000000, 0x00030047, 0x00000004, 0x00000002,
      0x00040047, 0x00000003, 0x00000022, 0x00000000, 0x00040047, 0x00000003,
      0x00000021, 0x00000000, 0x00020013, 0x00000005, 0x00030021, 0x00000006,
      0x00000005, 0x00040015, 0x00000007, 0x00000020, 0x00000000, 0x0003001e,
      0x00000004, 0x00000007, 0x00040020, 0x00000008, 0x0000000c, 0x00000004,
      0x0004003b, 0x00000008, 0x00000003, 0x0000000c, 0x00040015, 0x00000009,
      0x00000020, 0x00000001, 0x0004002b, 0x00000009, 0x0000000a, 0x00000000,
      0x00040020, 0x0000000b, 0x0000000c, 0x00000007, 0x00050036, 0x00000005,
      0x00000002, 0x00000000, 0x00000006, 0x000200f8, 0x0000000c, 0x00050041,
      0x0000000b, 0x0000000d, 0x00000003, 0x0000000a, 0x0005003f, 0x0000000d,
      0x0000000d, 0x00000000, 0x00000001, 0x000100fd, 0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_load_deref);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);

   intrinsic = find_intrinsic(nir_intrinsic_store_deref);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_EQ(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}

TEST_F(Volatile, opimageread_volatile)
{
   /*
               OpCapability Shader
               OpCapability VulkanMemoryModel
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %4 "main" %9
               OpExecutionMode %4 LocalSize 1 1 1
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
          %7 = OpTypeImage %uint 2D 0 0 0 2 R32ui
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
          %9 = OpVariable %_ptr_UniformConstant_7 UniformConstant
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %14 = OpConstantComposite %v2int %int_0 %int_0
     %v4uint = OpTypeVector %uint 4
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %10 = OpLoad %7 %9
         %15 = OpLoad %7 %9
         %17 = OpImageRead %v4uint %15 %14 VolatileTexel
               OpImageWrite %10 %14 %17
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010500, 0x00070000, 0x00000014, 0x00000000, 0x00020011,
      0x00000001, 0x00020011, 0x000014e1, 0x0006000b, 0x00000001, 0x4c534c47,
      0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000003,
      0x0006000f, 0x00000005, 0x00000002, 0x6e69616d, 0x00000000, 0x00000003,
      0x00060010, 0x00000002, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
      0x00040047, 0x00000003, 0x00000022, 0x00000000, 0x00040047, 0x00000003,
      0x00000021, 0x00000001, 0x00020013, 0x00000004, 0x00030021, 0x00000005,
      0x00000004, 0x00040015, 0x00000006, 0x00000020, 0x00000000, 0x00090019,
      0x00000007, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
      0x00000002, 0x00000021, 0x00040020, 0x00000008, 0x00000000, 0x00000007,
      0x0004003b, 0x00000008, 0x00000003, 0x00000000, 0x00040015, 0x00000009,
      0x00000020, 0x00000001, 0x00040017, 0x0000000a, 0x00000009, 0x00000002,
      0x0004002b, 0x00000009, 0x0000000b, 0x00000000, 0x0005002c, 0x0000000a,
      0x0000000c, 0x0000000b, 0x0000000b, 0x00040017, 0x0000000d, 0x00000006,
      0x00000004, 0x00040017, 0x0000000e, 0x00000006, 0x00000003, 0x0004002b,
      0x00000006, 0x0000000f, 0x00000001, 0x00050036, 0x00000004, 0x00000002,
      0x00000000, 0x00000005, 0x000200f8, 0x00000010, 0x0004003d, 0x00000007,
      0x00000011, 0x00000003, 0x0004003d, 0x00000007, 0x00000012, 0x00000003,
      0x00060062, 0x0000000d, 0x00000013, 0x00000012, 0x0000000c, 0x00000800,
      0x00040063, 0x00000011, 0x0000000c, 0x00000013, 0x000100fd, 0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_image_deref_load, 0);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}

TEST_F(Volatile, opimagewrite_volatile)
{
   /*
               OpCapability Shader
               OpCapability VulkanMemoryModel
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %4 "main" %9
               OpExecutionMode %4 LocalSize 1 1 1
               OpDecorate %9 DescriptorSet 0
               OpDecorate %9 Binding 1
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
          %7 = OpTypeImage %uint 2D 0 0 0 2 R32ui
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
          %9 = OpVariable %_ptr_UniformConstant_7 UniformConstant
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %14 = OpConstantComposite %v2int %int_0 %int_0
     %v4uint = OpTypeVector %uint 4
     %v3uint = OpTypeVector %uint 3
     %uint_1 = OpConstant %uint 1
          %4 = OpFunction %void None %3
          %5 = OpLabel
         %10 = OpLoad %7 %9
         %15 = OpLoad %7 %9
         %17 = OpImageRead %v4uint %15 %14
               OpImageWrite %10 %14 %17 VolatileTexel
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010500, 0x00070000, 0x00000014, 0x00000000, 0x00020011,
      0x00000001, 0x00020011, 0x000014e1, 0x0006000b, 0x00000001, 0x4c534c47,
      0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000003,
      0x0006000f, 0x00000005, 0x00000002, 0x6e69616d, 0x00000000, 0x00000003,
      0x00060010, 0x00000002, 0x00000011, 0x00000001, 0x00000001, 0x00000001,
      0x00040047, 0x00000003, 0x00000022, 0x00000000, 0x00040047, 0x00000003,
      0x00000021, 0x00000001, 0x00020013, 0x00000004, 0x00030021, 0x00000005,
      0x00000004, 0x00040015, 0x00000006, 0x00000020, 0x00000000, 0x00090019,
      0x00000007, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000,
      0x00000002, 0x00000021, 0x00040020, 0x00000008, 0x00000000, 0x00000007,
      0x0004003b, 0x00000008, 0x00000003, 0x00000000, 0x00040015, 0x00000009,
      0x00000020, 0x00000001, 0x00040017, 0x0000000a, 0x00000009, 0x00000002,
      0x0004002b, 0x00000009, 0x0000000b, 0x00000000, 0x0005002c, 0x0000000a,
      0x0000000c, 0x0000000b, 0x0000000b, 0x00040017, 0x0000000d, 0x00000006,
      0x00000004, 0x00040017, 0x0000000e, 0x00000006, 0x00000003, 0x0004002b,
      0x00000006, 0x0000000f, 0x00000001, 0x00050036, 0x00000004, 0x00000002,
      0x00000000, 0x00000005, 0x000200f8, 0x00000010, 0x0004003d, 0x00000007,
      0x00000011, 0x00000003, 0x0004003d, 0x00000007, 0x00000012, 0x00000003,
      0x00050062, 0x0000000d, 0x00000013, 0x00000012, 0x0000000c, 0x00050063,
      0x00000011, 0x0000000c, 0x00000013, 0x00000800, 0x000100fd, 0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_image_deref_store, 0);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}

TEST_F(Volatile, opatomicload_image_volatile)
{
   /*
               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpCapability VulkanMemoryModelDeviceScope
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %2 "main" %3
               OpExecutionMode %2 LocalSize 1 1 1
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 1
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
         %10 = OpTypeImage %uint 2D 0 0 0 2 R32ui
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
          %3 = OpVariable %_ptr_UniformConstant_10 UniformConstant
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %15 = OpConstantComposite %v2int %int_0 %int_0
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
%_ptr_Image_uint = OpTypePointer Image %uint
     %uint_1 = OpConstant %uint 1
  %uint_2048 = OpConstant %uint 2048
 %uint_34816 = OpConstant %uint 34816
     %v3uint = OpTypeVector %uint 3
          %2 = OpFunction %void None %7
         %29 = OpLabel
         %30 = OpImageTexelPointer %_ptr_Image_uint %3 %15 %uint_0
         %31 = OpAtomicLoad %uint %30 %int_1 %uint_34816
               OpAtomicStore %30 %int_1 %uint_2048 %31
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010500, 0x00070000, 0x00000017, 0x00000000, 0x00020011,
      0x00000001, 0x00020011, 0x000014e1, 0x00020011, 0x000014e2, 0x0006000b,
      0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e,
      0x00000000, 0x00000003, 0x0006000f, 0x00000005, 0x00000002, 0x6e69616d,
      0x00000000, 0x00000003, 0x00060010, 0x00000002, 0x00000011, 0x00000001,
      0x00000001, 0x00000001, 0x00040047, 0x00000003, 0x00000022, 0x00000000,
      0x00040047, 0x00000003, 0x00000021, 0x00000001, 0x00020013, 0x00000004,
      0x00030021, 0x00000005, 0x00000004, 0x00040015, 0x00000006, 0x00000020,
      0x00000000, 0x00090019, 0x00000007, 0x00000006, 0x00000001, 0x00000000,
      0x00000000, 0x00000000, 0x00000002, 0x00000021, 0x00040020, 0x00000008,
      0x00000000, 0x00000007, 0x0004003b, 0x00000008, 0x00000003, 0x00000000,
      0x00040015, 0x00000009, 0x00000020, 0x00000001, 0x00040017, 0x0000000a,
      0x00000009, 0x00000002, 0x0004002b, 0x00000009, 0x0000000b, 0x00000000,
      0x0005002c, 0x0000000a, 0x0000000c, 0x0000000b, 0x0000000b, 0x0004002b,
      0x00000009, 0x0000000d, 0x00000001, 0x0004002b, 0x00000006, 0x0000000e,
      0x00000000, 0x00040020, 0x0000000f, 0x0000000b, 0x00000006, 0x0004002b,
      0x00000006, 0x00000010, 0x00000001, 0x0004002b, 0x00000006, 0x00000011,
      0x00000800, 0x0004002b, 0x00000006, 0x00000012, 0x00008800, 0x00040017,
      0x00000013, 0x00000006, 0x00000003, 0x00050036, 0x00000004, 0x00000002,
      0x00000000, 0x00000005, 0x000200f8, 0x00000014, 0x0006003c, 0x0000000f,
      0x00000015, 0x00000003, 0x0000000c, 0x0000000e, 0x000600e3, 0x00000006,
      0x00000016, 0x00000015, 0x0000000d, 0x00000012, 0x000500e4, 0x00000015,
      0x0000000d, 0x00000011, 0x00000016, 0x000100fd, 0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_image_deref_load, 0);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}

TEST_F(Volatile, opatomicstore_image_volatile)
{
   /*
               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpCapability VulkanMemoryModelDeviceScope
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical Vulkan
               OpEntryPoint GLCompute %2 "main" %3
               OpExecutionMode %2 LocalSize 1 1 1
               OpDecorate %3 DescriptorSet 0
               OpDecorate %3 Binding 1
       %void = OpTypeVoid
          %7 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
         %10 = OpTypeImage %uint 2D 0 0 0 2 R32ui
%_ptr_UniformConstant_10 = OpTypePointer UniformConstant %10
          %3 = OpVariable %_ptr_UniformConstant_10 UniformConstant
        %int = OpTypeInt 32 1
      %v2int = OpTypeVector %int 2
      %int_0 = OpConstant %int 0
         %15 = OpConstantComposite %v2int %int_0 %int_0
      %int_1 = OpConstant %int 1
     %uint_0 = OpConstant %uint 0
%_ptr_Image_uint = OpTypePointer Image %uint
     %uint_1 = OpConstant %uint 1
  %uint_2048 = OpConstant %uint 2048
 %uint_34816 = OpConstant %uint 34816
     %v3uint = OpTypeVector %uint 3
          %2 = OpFunction %void None %7
         %29 = OpLabel
         %30 = OpImageTexelPointer %_ptr_Image_uint %3 %15 %uint_0
         %31 = OpAtomicLoad %uint %30 %int_1 %uint_2048
               OpAtomicStore %30 %int_1 %uint_34816 %31
               OpReturn
               OpFunctionEnd
   */
   static const uint32_t words[] = {
      0x07230203, 0x00010500, 0x00070000, 0x00000017, 0x00000000, 0x00020011,
      0x00000001, 0x00020011, 0x000014e1, 0x00020011, 0x000014e2, 0x0006000b,
      0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e,
      0x00000000, 0x00000003, 0x0006000f, 0x00000005, 0x00000002, 0x6e69616d,
      0x00000000, 0x00000003, 0x00060010, 0x00000002, 0x00000011, 0x00000001,
      0x00000001, 0x00000001, 0x00040047, 0x00000003, 0x00000022, 0x00000000,
      0x00040047, 0x00000003, 0x00000021, 0x00000001, 0x00020013, 0x00000004,
      0x00030021, 0x00000005, 0x00000004, 0x00040015, 0x00000006, 0x00000020,
      0x00000000, 0x00090019, 0x00000007, 0x00000006, 0x00000001, 0x00000000,
      0x00000000, 0x00000000, 0x00000002, 0x00000021, 0x00040020, 0x00000008,
      0x00000000, 0x00000007, 0x0004003b, 0x00000008, 0x00000003, 0x00000000,
      0x00040015, 0x00000009, 0x00000020, 0x00000001, 0x00040017, 0x0000000a,
      0x00000009, 0x00000002, 0x0004002b, 0x00000009, 0x0000000b, 0x00000000,
      0x0005002c, 0x0000000a, 0x0000000c, 0x0000000b, 0x0000000b, 0x0004002b,
      0x00000009, 0x0000000d, 0x00000001, 0x0004002b, 0x00000006, 0x0000000e,
      0x00000000, 0x00040020, 0x0000000f, 0x0000000b, 0x00000006, 0x0004002b,
      0x00000006, 0x00000010, 0x00000001, 0x0004002b, 0x00000006, 0x00000011,
      0x00000800, 0x0004002b, 0x00000006, 0x00000012, 0x00008800, 0x00040017,
      0x00000013, 0x00000006, 0x00000003, 0x00050036, 0x00000004, 0x00000002,
      0x00000000, 0x00000005, 0x000200f8, 0x00000014, 0x0006003c, 0x0000000f,
      0x00000015, 0x00000003, 0x0000000c, 0x0000000e, 0x000600e3, 0x00000006,
      0x00000016, 0x00000015, 0x0000000d, 0x00000011, 0x000500e4, 0x00000015,
      0x0000000d, 0x00000012, 0x00000016, 0x000100fd, 0x00010038,
   };

   get_nir(sizeof(words) / sizeof(words[0]), words);

   nir_intrinsic_instr *intrinsic = find_intrinsic(nir_intrinsic_image_deref_store, 0);
   ASSERT_NE(intrinsic, nullptr);
   EXPECT_NE(nir_intrinsic_access(intrinsic) & ACCESS_VOLATILE, 0);
}
