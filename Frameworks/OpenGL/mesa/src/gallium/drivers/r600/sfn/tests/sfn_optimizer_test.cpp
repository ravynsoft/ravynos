
#include "../sfn_optimizer.h"
#include "../sfn_ra.h"
#include "../sfn_scheduler.h"
#include "../sfn_shader.h"
#include "../sfn_split_address_loads.h"
#include "sfn_test_shaders.h"

using namespace r600;
using std::ostringstream;

TEST_F(TestShaderFromNir, SimpleDCE)
{
   auto sh = from_string(red_triangle_fs_expect_from_nir);
   dead_code_elimination(*sh);

   check(sh, red_triangle_fs_expect_from_nir_dce);
}

TEST_F(TestShaderFromNir, CopyPropagationForwardBackward)
{
   auto sh = from_string(add_add_1_expect_from_nir);
   copy_propagation_fwd(*sh);
   check(sh, add_add_1_expect_from_nir_copy_prop_fwd);
}

TEST_F(TestShaderFromNir, CopyPropagationForwardDCE)
{
   auto sh = from_string(add_add_1_expect_from_nir);
   copy_propagation_fwd(*sh);
   dead_code_elimination(*sh);
   check(sh, add_add_1_expect_from_nir_copy_prop_fwd_dce);
}

TEST_F(TestShaderFromNir, CopyPropagationBackwardDCE)
{
   auto sh = from_string(add_add_1_expect_from_nir_copy_prop_fwd_dce);
   copy_propagation_backward(*sh);
   dead_code_elimination(*sh);
   check(sh, add_add_1_expect_from_nir_copy_prop_fwd_dce_bwd);
}

TEST_F(TestShaderFromNir, FullOPtimize)
{
   auto sh = from_string(basic_interpolation_orig);

   bool progress;

   do {
      progress = false;
      progress |= copy_propagation_fwd(*sh);
      progress |= dead_code_elimination(*sh);
      progress |= copy_propagation_backward(*sh);
      progress |= dead_code_elimination(*sh);
   } while (progress);

   check(sh, basic_interpolation_expect_from_nir_opt);
}

TEST_F(TestShaderFromNir, CombinePinFlags)
{
   auto sh = from_string(shader_group_chan_pin_to_combine);

   bool progress;

   do {
      progress = false;
      progress |= copy_propagation_fwd(*sh);
      progress |= dead_code_elimination(*sh);
      progress |= copy_propagation_backward(*sh);
      progress |= dead_code_elimination(*sh);
   } while (progress);

   check(sh, shader_group_chan_pin_combined);
}

TEST_F(TestShaderFromNir, FullOPtimizeLoop)
{
   auto sh = from_string(vs_nexted_loop_from_nir_expect);

   optimize(*sh);

   check(sh, vs_nexted_loop_from_nir_expect_opt);
}

TEST_F(TestShaderFromNir, CombineRegisterToTexSrc)
{
const char *shader_input =
   R"(FS
CHIPCLASS EVERGREEN
REGISTERS R0.x R1.x R2.x R3.x
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU ADD R2.x : R0.x R2.x {W}
ALU MUL R3.x : R0.x R3.x {WL}
ALU MOV S1.x@group : R2.x {W}
ALU MOV S1.y@group : R3.x {WL}
TEX SAMPLE S2.xyzw : S1.xy__ RID:18 SID:0 NNNN
EXPORT_DONE PIXEL 0 S2.xyzw
)";

const char *shader_expect =
   R"(FS
CHIPCLASS EVERGREEN
REGISTERS R1024.x@group R1024.y@group R0.x
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER

ALU ADD R1024.x@group : R0.x R1024.x@group {W}
ALU MUL R1024.y@group : R0.x R1024.y@group {WL}
TEX SAMPLE S2.xyzw : R1024.xy__ RID:18 SID:0 NNNN
EXPORT_DONE PIXEL 0 S2.xyzw
)";

   auto sh = from_string(shader_input);

   optimize(*sh);

   check(sh, shader_expect);
}

TEST_F(TestShaderFromNir, CopyPropRegDest)
{
const char *shader_input =
   R"(FS
CHIPCLASS EVERGREEN
REGISTERS R0.x
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S2.x : R0.x {W}
ALU MUL S3.x : S2.x S2.x {W}
EXPORT_DONE PIXEL 0 S3.xxxx
)";

const char *shader_expect =
   R"(FS
CHIPCLASS EVERGREEN
REGISTERS R0.x
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MUL S3.x : R0.x R0.x {W}
EXPORT_DONE PIXEL 0 S3.xxxx
)";

   auto sh = from_string(shader_input);

   optimize(*sh);

   check(sh, shader_expect);
}

TEST_F(TestShaderFromNir, CopyPropRegDestAndOverwrite)
{
const char *shader_input =
   R"(FS
CHIPCLASS EVERGREEN
REGISTERS R0.x
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S2.x : R0.x {W}
ALU MOV R0.x : L[2.0] {W}
ALU MUL S3.x : S2.x R0.x {W}
EXPORT_DONE PIXEL 0 S3.xxxx
)";

const char *shader_expect =
   R"(FS
CHIPCLASS EVERGREEN
REGISTERS R0.x
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S2.x : R0.x {W}
ALU MOV R0.x : L[2.0] {W}
ALU MUL S3.x : S2.x L[2.0] {W}
EXPORT_DONE PIXEL 0 S3.xxxx
)";

   auto sh = from_string(shader_input);

   optimize(*sh);

   check(sh, shader_expect);
}

TEST_F(TestShaderFromNir, CopyPropRegDestAndIndirectOverwrite)
{
const char *shader_input =
   R"(FS
CHIPCLASS EVERGREEN
REGISTERS R0.x
ARRAYS A0[2].x
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S2.x : A0[0].x {W}
ALU MOV A0[R0.x].x : L[2.0] {W}
ALU MUL S3.x : S2.x A0[1].x {W}
EXPORT_DONE PIXEL 0 S3.xxxx
)";

const char *shader_expect =
   R"(FS
CHIPCLASS EVERGREEN
REGISTERS R0.x
ARRAYS A0[2].x
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S2.x : A0[0].x {W}
ALU MOV A0[R0.x].x : L[2.0] {W}
ALU MUL S3.x : S2.x A0[1].x {W}
EXPORT_DONE PIXEL 0 S3.xxxx
)";

   auto sh = from_string(shader_input);

   optimize(*sh);

   check(sh, shader_expect);
}


TEST_F(TestShaderFromNir, CopyPropAndIndirectReadOrder)
{
const char *shader_input =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A0[4].x
REGISTERS R0.xy
SHADER
BLOCK_START
  ALU MOV A0[0].x : I[0] {W}
  ALU MOV A0[1].x : I[1] {W}
  ALU MOV A0[2].x : I[0] {W}
  ALU MOV A0[3].x : I[0] {W}
  ALU MOV S2.x{s} : A0[R0.x].x {W}
  ALU MOV A0[1].x : L[0x2] {W}
  ALU MOV A0[2].x : L[0x2] {W}
  ALU MOV A0[3].x : L[0x3] {W}
  ALU MOV A0[R0.y].x : I[1] {W}
  ALU MOV S3.x : A0[0].x {W}
  ALU MOV S3.y : A0[1].x {W}
  ALU MOV S3.z : A0[2].x {W}
  ALU MOV S3.w : A0[3].x {W}
  EXPORT_DONE PIXEL 0 S3.xyzw
BLOCK_END
)";

const char *shader_expect =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A0[4].x
REGISTERS R0.xy
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOVA_INT AR : R0.x {}
  ALU MOV A0[0].x : I[0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV A0[1].x : I[1] {W}
  ALU MOV A0[2].x : I[0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV A0[3].x : I[0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU MOV S2.x : A0[AR].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOVA_INT AR : R0.y {}
  ALU MOV A0[1].x : L[0x2] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV A0[2].x : L[0x2] {W}
  ALU MOV A0[3].x : L[0x3] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV A0[AR].x : I[1] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV S3.x@chgr : A0[0].x {W}
  ALU MOV S3.y@chgr : A0[1].x {W}
  ALU MOV S3.z@chgr : A0[2].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV S3.w@chgr : A0[3].x {WL}
ALU_GROUP_END
BLOCK_START
BLOCK_END
  EXPORT_DONE PIXEL 0 S3.xyzw
BLOCK_END
)";

   auto sh = from_string(shader_input);
   split_address_loads(*sh);
   check(schedule(sh), shader_expect);
}

TEST_F(TestShaderFromNir, OptimizeWithDestArrayValue)
{
   auto sh = from_string(shader_with_dest_array);

   optimize(*sh);

   check(sh, shader_with_dest_array_opt_expect);
}

TEST_F(TestShaderFromNir, ScheduleOPtimizedWithDestArrayValue)
{
   auto sh = from_string(shader_with_dest_array_opt_expect);
   split_address_loads(*sh);
   check(schedule(sh), shader_with_dest_array_opt_scheduled);
}

TEST_F(TestShaderFromNir, ScheduleWithArrayWriteAndRead)
{
   auto sh = from_string(shader_with_dest_array2);
   split_address_loads(*sh);
   check(schedule(sh), shader_with_dest_array2_scheduled);
}

TEST_F(TestShaderFromNir, RA_with_dest_array)
{
   auto sh = from_string(shader_with_dest_array2_scheduled);

   auto lrm = r600::LiveRangeEvaluator().run(*sh);
   EXPECT_TRUE(r600::register_allocation(lrm));

   ra_check(sh, shader_with_dest_array2_scheduled_ra);
}

TEST_F(TestShaderFromNir, RA_with_chan_group)
{
   auto sh = from_string(shader_group_chan_pin_combined_scheduled);

   auto lrm = r600::LiveRangeEvaluator().run(*sh);
   EXPECT_TRUE(r600::register_allocation(lrm));
   ra_check(sh, shader_group_chan_pin_combined_scheduled_ra);
}

TEST_F(TestShaderFromNir, TES_opt)
{
   auto sh = from_string(tes_pre_op);

   optimize(*sh);

   check(sh, tes_optimized);
}

TEST_F(TestShaderFromNir, TES_scheduled)
{
   auto sh = from_string(tes_optimized_pre_sched);

   check(schedule(sh), tes_optimized_sched);
}

/*
TEST_F(TestShaderFromNir, ShaderClone)
{
   auto sh = from_string(red_triangle_fs_expect_from_nir);

   auto sh_cloned = sh->clone();

   MemoryPool::instance().push();
   dead_code_elimination(*sh);

   check(sh, red_triangle_fs_expect_from_nir_dce);

   check(sh_cloned, red_triangle_fs_expect_from_nir);

   MemoryPool::instance().pop();
   check(sh, red_triangle_fs_expect_from_nir_dce);
}
*/

TEST_F(TestShaderFromNir, ShaderSchedule)
{
   auto sh = from_string(basic_interpolation_orig);

   check(schedule(sh), basic_interpolation_expect_from_nir_sched);
}

TEST_F(TestShaderFromNir, ShaderScheduleCayman)
{
   auto sh = from_string(basic_interpolation_orig_cayman);

   check(schedule(sh), basic_interpolation_expect_from_nir_sched_cayman);
}

TEST_F(TestShaderFromNir, ShaderScheduleOptimizedCayman)
{
   auto sh = from_string(basic_interpolation_orig_cayman);

   optimize(*sh);

   check(schedule(sh), basic_interpolation_expect_opt_sched_cayman);
}

TEST_F(TestShaderFromNir, CopyPropLegalConst)
{
   auto sh = from_string(dot4_pre);

   copy_propagation_fwd(*sh);
   dead_code_elimination(*sh);

   check(sh, dot4_copy_prop_dce);
}

TEST_F(TestShaderFromNir, FullOPtimize_glxgears_vs2)
{

   auto sh = from_string(glxgears_vs2_from_nir_expect);
   optimize(*sh);
   check(sh, glxgears_vs2_from_nir_expect_optimized);
}

TEST_F(TestShaderFromNir, test_schedule_group)
{

   auto sh = from_string(test_schedule_group);
   optimize(*sh);
   check(schedule(sh), test_schedule_group_expect);
}

TEST_F(TestShaderFromNir, test_dont_kill_dual_use)
{
   auto sh = from_string(shader_copy_prop_dont_kill_double_use);
   optimize(*sh);
   check(schedule(sh), shader_copy_prop_dont_kill_double_use_expect);
}

TEST_F(TestShaderFromNir, test_schedule_with_bany)
{

   auto sh = from_string(shader_with_bany_expect_eg);
   optimize(*sh);
   check(schedule(sh), shader_with_bany_expect_opt_sched_eg);
}

TEST_F(TestShaderFromNir, GroupAndChanCombine)
{
   auto sh = from_string(shader_group_chan_pin_to_combine_2);
   optimize(*sh);
   check(sh, shader_group_chan_pin_to_combine_2_opt);
}

TEST_F(TestShaderFromNir, RemoveUseAfterSplitgroup)
{
   auto sh = from_string(fs_with_loop_multislot_reuse);
   check(schedule(sh), fs_with_loop_multislot_reuse_scheduled);
}

TEST_F(TestShaderFromNir, OptimizeVSforTCS)
{
   auto sh = from_string(vtx_for_tcs_inp);
   optimize(*sh);
   check(sh, vtx_for_tcs_opt);
}

TEST_F(TestShaderFromNir, ScheduleVSforTCS)
{
   auto sh = from_string(vtx_for_tcs_pre_sched);
   check(schedule(sh), vtx_for_tcs_sched);
}

TEST_F(TestShaderFromNir, fs_opt_tex_coord)
{
   auto sh = from_string(fs_opt_tex_coord_init);

   optimize(*sh);

   check(sh, fs_opt_tex_coord_expect);
}

TEST_F(TestShaderFromNir, fs_shed_tex_coord)
{
   auto sh = from_string(fs_sched_tex_coord_init);
   check(schedule(sh), fs_sched_tex_coord_expect);
}

TEST_F(TestShaderFromNir, OptimizeAddWChanetoTrans)
{
   const char *input =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
SHADER
BLOCK_START
  ALU MOV S2.x@free{s} : I[0] {WL}
  ALU MOV S3.y@free{s} : L[0x40c00000] {WL}
  ALU MOV S4.z@free{s} : L[0xc1140000] {WL}
  ALU MOV S5.w@free{s} : L[0xbfe00000] {WL}
  ALU MOV S6.x@free{s} : L[0x3fa00000] {WL}
  ALU MOV S7.x{s} : KC0[0].x {W}
  ALU MOV S7.y{s} : KC0[0].y {W}
  ALU MOV S7.z{s} : KC0[0].z {W}
  ALU MOV S7.w{s} : KC0[0].w {WL}
  ALU ADD S8.y@free{s} : S3.y@free{s} -S7.x{s} {WL}
  ALU ADD S9.z@free{s} : S4.z@free{s} -S7.y{s} {WL}
  ALU ADD S10.w@free{s} : S5.w@free{s} -S7.z{s} {WL}
  ALU ADD S11.x@free{s} : S6.x@free{s} -S7.w{s} {WL}
  ALU EXP_IEEE S12.y@free{s} : S8.y@free{s} + S8.y@free{s} + S8.y@free{s} {WL}
  ALU EXP_IEEE S13.z@free{s} : S9.z@free{s} + S9.z@free{s} + S9.z@free{s} {WL}
  ALU EXP_IEEE S14.x@free{s} : S10.w@free{s} + S10.w@free{s} + S10.w@free{s} {WL}
  ALU EXP_IEEE S15.y@free{s} : S11.x@free{s} + S11.x@free{s} + S11.x@free{s} {WL}
  ALU MOV S17.x{s} : S12.y@free{s} {W}
  ALU MOV S17.y{s} : S13.z@free{s} {W}
  ALU MOV S17.z{s} : S14.x@free{s} {W}
  ALU MOV S17.w{s} : S15.y@free{s} {WL}
  ALU MOV S18.x@group{s} : S17.x{s} {W}
  ALU MOV S18.y@group{s} : S17.y{s} {W}
  ALU MOV S18.z@group{s} : S17.z{s} {W}
  ALU MOV S18.w@group{s} : S17.w{s} {WL}
  EXPORT_DONE PARAM 0 S18.xyzw
BLOCK_END)";

   const char *expect =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
SHADER
BLOCK_START
  ALU ADD S8.y@free{s} : L[0x40c00000] -KC0[0].x {WL}
  ALU ADD S9.z@free{s} : L[0xc1140000] -KC0[0].y {WL}
  ALU ADD S10.w@free{s} : L[0xbfe00000] -KC0[0].z {WL}
  ALU ADD S11.x@free{s} : L[0x3fa00000] -KC0[0].w {WL}
  ALU EXP_IEEE S18.x@group{s} : S8.y@free{s} + S8.y@free{s} + S8.y@free{s} {W}
  ALU EXP_IEEE S18.y@group{s} : S9.z@free{s} + S9.z@free{s} + S9.z@free{s} {W}
  ALU EXP_IEEE S18.z@group{s} : S10.w@free{s} + S10.w@free{s} + S10.w@free{s} {W}
  ALU EXP_IEEE S18.w@group{s} : S11.x@free{s} + S11.x@free{s} + S11.x@free{s} + S11.x@free{s} {WL}
  EXPORT_DONE PARAM 0 S18.xyzw
BLOCK_END
)";
   auto sh = from_string(input);
   optimize(*sh);
   check(sh, expect);
};

TEST_F(TestShaderFromNir, PeeholeSoureModsSimple)
{
   const char *input =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
SHADER
BLOCK_START
  ALU MOV S2.x@free{s} : I[0] {WL}
  ALU MOV S3.y@free{s} : L[0x40c00000] {WL}
  ALU MOV S4.z@free{s} : L[0xc1140000] {WL}
  ALU MOV S5.w@free{s} : L[0xbfe00000] {WL}
  ALU MOV S6.x@free{s} : L[0x3fa00000] {WL}
  ALU MOV S7.x{s} : |KC0[0].x| {W}
  ALU MOV S7.y{s} : -KC0[0].y {W}
  ALU MOV S7.z{s} : -|KC0[0].z| {W}
  ALU MOV S7.w{s} : KC0[0].w {WL}
  ALU ADD S8.y@free{s} : S3.y@free{s} S7.x{s} {WL}
  ALU ADD S9.z@free{s} : S4.z@free{s} S7.y{s} {WL}
  ALU ADD S10.w@free{s} : S5.w@free{s} S7.z{s} {WL}
  ALU ADD S11.x@free{s} : S6.x@free{s} S7.w{s} {WL}
  ALU EXP_IEEE S12.y@free{s} : S8.y@free{s} + S8.y@free{s} + S8.y@free{s} {WL}
  ALU EXP_IEEE S13.z@free{s} : S9.z@free{s} + S9.z@free{s} + S9.z@free{s} {WL}
  ALU EXP_IEEE S14.x@free{s} : S10.w@free{s} + S10.w@free{s} + S10.w@free{s} {WL}
  ALU EXP_IEEE S15.y@free{s} : S11.x@free{s} + S11.x@free{s} + S11.x@free{s} {WL}
  ALU MOV S17.x{s} : S12.y@free{s} {W}
  ALU MOV S17.y{s} : S13.z@free{s} {W}
  ALU MOV S17.z{s} : S14.x@free{s} {W}
  ALU MOV S17.w{s} : S15.y@free{s} {WL}
  ALU MOV S18.x@group{s} : S17.x{s} {W}
  ALU MOV S18.y@group{s} : S17.y{s} {W}
  ALU MOV S18.z@group{s} : S17.z{s} {W}
  ALU MOV S18.w@group{s} : S17.w{s} {WL}
  EXPORT_DONE PARAM 0 S18.xyzw
BLOCK_END)";

   const char *expect =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
SHADER
BLOCK_START
  ALU ADD S8.y@free{s} : L[0x40c00000] |KC0[0].x| {WL}
  ALU ADD S9.z@free{s} : L[0xc1140000] -KC0[0].y {WL}
  ALU ADD S10.w@free{s} : L[0xbfe00000] -|KC0[0].z| {WL}
  ALU ADD S11.x@free{s} : L[0x3fa00000] KC0[0].w {WL}
  ALU EXP_IEEE S18.x@group{s} : S8.y@free{s} + S8.y@free{s} + S8.y@free{s} {W}
  ALU EXP_IEEE S18.y@group{s} : S9.z@free{s} + S9.z@free{s} + S9.z@free{s} {W}
  ALU EXP_IEEE S18.z@group{s} : S10.w@free{s} + S10.w@free{s} + S10.w@free{s} {W}
  ALU EXP_IEEE S18.w@group{s} : S11.x@free{s} + S11.x@free{s} + S11.x@free{s} + S11.x@free{s} {WL}
  EXPORT_DONE PARAM 0 S18.xyzw
BLOCK_END
)";
   auto sh = from_string(input);
   optimize(*sh);
   check(sh, expect);
};

TEST_F(TestShaderFromNir, PeeholeSoureModsAbsNegTwice)
{
   const char *input =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
SHADER
BLOCK_START
  ALU MOV S2.x@free{s} : I[0] {WL}
  ALU MOV S3.y@free{s} : L[0x40c00000] {WL}
  ALU MOV S4.z@free{s} : L[0xc1140000] {WL}
  ALU MOV S5.w@free{s} : L[0xbfe00000] {WL}
  ALU MOV S6.x@free{s} : L[0x3fa00000] {WL}
  ALU MOV S7.x{s} : |KC0[0].x| {W}
  ALU MOV S7.y{s} : -KC0[0].y {W}
  ALU MOV S7.z{s} : -|KC0[0].z| {W}
  ALU MOV S7.w{s} : KC0[0].w {WL}
  ALU MOV S8.x : |S7.x| {W}
  ALU MOV S8.y : -S7.y {W}
  ALU MOV S8.z : -|S7.z| {W}
  ALU MOV S8.w : -|S7.x| {WL}
  ALU ADD S19.y@free{s} : S3.y@free{s} S8.x {WL}
  ALU ADD S9.z@free{s} : S4.z@free{s} S8.y {WL}
  ALU ADD S10.w@free{s} : S5.w@free{s} S8.z {WL}
  ALU ADD S11.x@free{s} : S6.x@free{s} S8.w {WL}
  ALU EXP_IEEE S12.y@free{s} : S19.y@free{s} + S19.y@free{s} + S19.y@free{s} {WL}
  ALU EXP_IEEE S13.z@free{s} : S9.z@free{s} + S9.z@free{s} + S9.z@free{s} {WL}
  ALU EXP_IEEE S14.x@free{s} : S10.w@free{s} + S10.w@free{s} + S10.w@free{s} {WL}
  ALU EXP_IEEE S15.y@free{s} : S11.x@free{s} + S11.x@free{s} + S11.x@free{s} {WL}
  ALU MOV S17.x{s} : S12.y@free{s} {W}
  ALU MOV S17.y{s} : S13.z@free{s} {W}
  ALU MOV S17.z{s} : S14.x@free{s} {W}
  ALU MOV S17.w{s} : S15.y@free{s} {WL}
  ALU MOV S18.x@group{s} : S17.x{s} {W}
  ALU MOV S18.y@group{s} : S17.y{s} {W}
  ALU MOV S18.z@group{s} : S17.z{s} {W}
  ALU MOV S18.w@group{s} : S17.w{s} {WL}
  EXPORT_DONE PARAM 0 S18.xyzw
BLOCK_END)";

   const char *expect =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
SHADER
BLOCK_START
  ALU ADD S19.y@free{s} : L[0x40c00000] |KC0[0].x| {WL}
  ALU ADD S9.z@free{s} : L[0xc1140000] KC0[0].y {WL}
  ALU ADD S10.w@free{s} : L[0xbfe00000] |KC0[0].z| {WL}
  ALU ADD S11.x@free{s} : L[0x3fa00000] -|KC0[0].x| {WL}
  ALU EXP_IEEE S18.x@group{s} : S19.y@free{s} + S19.y@free{s} + S19.y@free{s} {W}
  ALU EXP_IEEE S18.y@group{s} : S9.z@free{s} + S9.z@free{s} + S9.z@free{s} {W}
  ALU EXP_IEEE S18.z@group{s} : S10.w@free{s} + S10.w@free{s} + S10.w@free{s} {W}
  ALU EXP_IEEE S18.w@group{s} : S11.x@free{s} + S11.x@free{s} + S11.x@free{s} + S11.x@free{s} {WL}
  EXPORT_DONE PARAM 0 S18.xyzw
BLOCK_END
)";
   auto sh = from_string(input);
   optimize(*sh);
   check(sh, expect);
};

TEST_F(TestShaderFromNir, PeeholeSoureModsClamp)
{
   const char *input =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
SHADER
BLOCK_START
  ALU MOV S1.x{s} : |KC0[0].x| {W}
  ALU MOV S2.y{s} : -KC0[0].y {W}
  ALU ADD S3.x : S1.x S2.y {W}
  ALU MOV CLAMP S4.x : S3.x {W}
  EXPORT_DONE PARAM 0 S4.xxxx
BLOCK_END)";

   const char *expect =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
SHADER
BLOCK_START
  ALU ADD CLAMP S3.x : |KC0[0].x| -KC0[0].y {W}
  EXPORT_DONE PARAM 0 S3.xxxx
BLOCK_END
)";
   auto sh = from_string(input);
   optimize(*sh);
   check(sh, expect);
};

TEST_F(TestShaderFromNir, PeeholeSoureModsMuliSlot)
{
   const char *input =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R1.xyzw
SHADER
BLOCK_START
  ALU MOV S1.x{s} : |KC0[0].x| {W}
  ALU MOV S1.y{s} : -KC0[0].y {W}
  ALU MOV S1.z{s} : |KC0[0].z| {W}
  ALU MOV S1.w{s} : KC0[0].w {W}
  ALU MOV S2.x{s} : |R1.x| {W}
  ALU MOV S2.y{s} : R1.y {W}
  ALU MOV S2.z{s} : -R1.z {W}
  ALU MOV S2.w{s} : -R1.w {W}
  ALU DOT4 S5.x : S1.x S2.x + S1.y S2.y  + S1.z S2.z + S1.w S2.w {W}
  EXPORT_DONE PARAM 0 S5.xxxx
BLOCK_END)";

   const char *expect =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R1.xyzw
SHADER
BLOCK_START
  ALU DOT4 S5.x : |KC0[0].x| |R1.x| + -KC0[0].y R1.y  + |KC0[0].z| -R1.z + KC0[0].w -R1.w {W}
  EXPORT_DONE PARAM 0 S5.xxxx
BLOCK_END
)";
   auto sh = from_string(input);
   optimize(*sh);
   check(sh, expect);
};


TEST_F(TestShaderFromNir, OptimizeIntoGroup)
{
   const char *input =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R0.x R1.x R2.x
SHADER
BLOCK_START
  ALU MOV S4.x : R0.x {W}
  ALU MOV S5.y : R1.x {W}
  ALU MOV S6.z : R2.x {WL}
  ALU_GROUP_BEGIN
    ALU CUBE S7.x@chgr{s} : S6.z S5.y {W}
    ALU CUBE S7.y@chgr{s} : S6.z S4.x {W}
    ALU CUBE S7.z@chgr{s} : S4.x S6.z {W}
    ALU CUBE S7.w@chgr{s} : S5.y S6.z {WL}
  ALU_GROUP_END
  EXPORT_DONE PARAM 0 S7.xyzw
BLOCK_END)";

   const char *expect =
R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R0.x R1.x R2.x
SHADER
BLOCK_START
  ALU_GROUP_BEGIN
    ALU CUBE S7.x@chgr{s} : R2.x@chan R1.x@chan {W}
    ALU CUBE S7.y@chgr{s} : R2.x@chan R0.x@chan {W}
    ALU CUBE S7.z@chgr{s} : R0.x@chan R2.x@chan {W}
    ALU CUBE S7.w@chgr{s} : R1.x@chan R2.x@chan {WL}
  ALU_GROUP_END
  EXPORT_DONE PARAM 0 S7.xyzw
BLOCK_END
)";
   auto sh = from_string(input);
   optimize(*sh);
   check(sh, expect);
};

TEST_F(TestShaderFromNir, ScheduleSplitLoadIndexConst)
{
   const char *input =
R"(
FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
BLOCK_START
  ALU MIN_UINT S3.w@free{s} : KC0[0].x L[0x2] {WL}
  ALU MOVA_INT IDX0 : S3.w@free{s} {}
  ALU MOV S4.x@group{s} : KC1[IDX0][0].x {W}
  ALU MOV S4.y@group{s} : KC1[IDX0][0].y {W}
  ALU MOV S4.z@group{s} : KC1[IDX0][0].z {W}
  ALU MOV S4.w@group{s} : KC1[IDX0][0].w {WL}
  EXPORT_DONE PIXEL 0 S4.xyzw
BLOCK_END
)";

   const char *expect =
R"(
FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
BLOCK_START
ALU_GROUP_BEGIN
     ALU MIN_UINT S3.w@free{s} : KC0[0].x L[0x2] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
     ALU MOVA_INT IDX0 : S3.w@free{s} {L}
ALU_GROUP_END
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
     ALU MOV S4.x@chgr : KC1[IDX0][0].x {W}
     ALU MOV S4.y@chgr : KC1[IDX0][0].y {W}
     ALU MOV S4.z@chgr : KC1[IDX0][0].z {W}
     ALU MOV S4.w@chgr : KC1[IDX0][0].w {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
BLOCK_END
BLOCK_START
     EXPORT_DONE PIXEL 0 S4.xyzw
BLOCK_END
)";

   auto sh = from_string(input);
   check(schedule(sh), expect);
}

TEST_F(TestShaderFromNir, ScheduleSplitLoadAddrAndNOPAfterIndirectDest)
{
   const char *input =
R"(FS
CHIPCLASS R600
FAMILY R600
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
  ALU MOV S1.x : KC0[0].x {WL}
  ALU MOV A1[S1.x].x : KC0[0].y {WL}
  ALU ADD S2.x : A1[1].x KC0[1].x {WL}
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";


   const char *expect =
R"(FS
CHIPCLASS R600
FAMILY R600
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
ALU_GROUP_BEGIN
   ALU MOVA_INT AR : KC0[0].x {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU MOV A1[AR].x : KC0[0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU NOP __.x : {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU ADD S2.x@chgr : A1[1].x KC0[1].x {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);

}

TEST_F(TestShaderFromNir, ScheduleSplitLoadAddrAndNOPBeforIndirectSrc)
{
   const char *input =
R"(FS
CHIPCLASS R600
FAMILY R600
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
  ALU MOV S1.x : KC0[0].x {WL}
  ALU MOV A1[0].x : KC0[0].y {WL}
  ALU ADD S2.x : A1[S1.x].x KC0[1].x {WL}
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";


   const char *expect =
R"(FS
CHIPCLASS R600
FAMILY R600
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
ALU_GROUP_BEGIN
   ALU MOVA_INT AR : KC0[0].x {}
   ALU MOV A1[0].x : KC0[0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU NOP __.x : {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU ADD S2.x@chgr : A1[AR].x KC0[1].x {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);

}


TEST_F(TestShaderFromNir, ScheduleSplitLoadAddrAndNOPAfterIndirectDestRV670)
{
   const char *input =
R"(FS
CHIPCLASS R600
FAMILY RV670
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
  ALU MOV S1.x : KC0[0].x {WL}
  ALU MOV A1[S1.x].x : KC0[0].y {WL}
  ALU ADD S2.x : A1[1].x KC0[1].x {WL}
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";


   const char *expect =
R"(FS
CHIPCLASS R600
FAMILY RV670
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
ALU_GROUP_BEGIN
   ALU MOVA_INT AR : KC0[0].x {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU MOV A1[AR].x : KC0[0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU ADD S2.x@chgr : A1[1].x KC0[1].x {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);

}

TEST_F(TestShaderFromNir, ScheduleSplitLoadAddrAndNOPAfterIndirectDestEG)
{
   const char *input =
R"(FS
CHIPCLASS EVERGREEN
FAMILY BARTS
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
  ALU MOV S1.x : KC0[0].x {WL}
  ALU MOV A1[S1.x].x : KC0[0].y {WL}
  ALU ADD S2.x : A1[1].x KC0[1].x {WL}
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";


   const char *expect =
R"(FS
CHIPCLASS EVERGREEN
FAMILY BARTS
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
ALU_GROUP_BEGIN
   ALU MOVA_INT AR : KC0[0].x {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU MOV A1[AR].x : KC0[0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU ADD S2.x@chgr : A1[1].x KC0[1].x {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);

}

TEST_F(TestShaderFromNir, ScheduleSplitLoadAddrAndNOPAfterIndirectDestRV770)
{
   const char *input =
R"(FS
CHIPCLASS R700
FAMILY RV770
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
  ALU MOV S1.x : KC0[0].x {WL}
  ALU MOV A1[S1.x].x : KC0[0].y {WL}
  ALU ADD S2.x : A1[1].x KC0[1].x {WL}
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";


   const char *expect =
R"(FS
CHIPCLASS R700
FAMILY RV770
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
ALU_GROUP_BEGIN
   ALU MOVA_INT AR : KC0[0].x {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU MOV A1[AR].x : KC0[0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU NOP __.x : {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU ADD S2.x@chgr : A1[1].x KC0[1].x {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
  EXPORT_DONE PIXEL 0 S2.xxxx
BLOCK_END)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);

}

TEST_F(TestShaderFromNir, ScheduleSplitLoadAddrAndNOPAfterIndirectDestRV770_2)
{
   const char *input =
R"(FS
CHIPCLASS R700
FAMILY RV770
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
  ALU MOV S1.x : KC0[0].x {W}
  ALU MOV S1.y : KC0[0].y {W}
  ALU ADD S1.z : KC0[0].z KC0[2].z {W}
  ALU ADD S1.w : KC0[0].w KC0[2].w {W}
  ALU MOV A1[S1.x].x : KC0[1].y {WL}
  ALU ADD S2.x : A1[1].x S1.y {W}
  ALU ADD S2.y : KC0[1].y S1.z {W}
  ALU ADD S2.z : KC0[1].z S1.w {W}
  ALU ADD S2.w : KC0[1].w S1.w {WL}
  EXPORT_DONE PIXEL 0 S2.xyzw
BLOCK_END)";


   const char *expect =
R"(FS
CHIPCLASS R700
FAMILY RV770
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A1[2].x
SHADER
BLOCK_START
ALU_GROUP_BEGIN
   ALU MOVA_INT AR : KC0[0].x {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
    ALU MOV A1[AR].x : KC0[1].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU ADD S1.z : KC0[0].z KC0[2].z {W}
   ALU ADD S1.w : KC0[0].w KC0[2].w {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU ADD S2.x@chgr : A1[1].x KC0[0].y {W}
   ALU ADD S2.y@chgr : KC0[1].y S1.z{s} {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU ADD S2.z@chgr : KC0[1].z S1.w{s} {W}
   ALU ADD S2.w@chgr : KC0[1].w S1.w{s} {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
  EXPORT_DONE PIXEL 0 S2.xyzw
BLOCK_END)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);

}
