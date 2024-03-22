#ifndef SFN_TEST_SHADERS_H
#define SFN_TEST_SHADERS_H
#include <gtest/gtest.h>

#include "../sfn_shader.h"

namespace r600 {

extern const char *red_triangle_fs_nir;
extern const char *red_triangle_fs_expect_from_nir;
extern const char *red_triangle_fs_expect_from_nir_dce;

extern const char *add_add_1_nir;
extern const char *add_add_1_expect_from_nir;
extern const char *add_add_1_expect_from_nir_copy_prop_fwd;
extern const char *add_add_1_expect_from_nir_copy_prop_fwd_dce;
extern const char *add_add_1_expect_from_nir_copy_prop_fwd_dce_bwd;

extern const char *basic_interpolation_nir;
extern const char *basic_interpolation_orig;
extern const char *basic_interpolation_translated_1;
extern const char *basic_interpolation_expect_from_nir;
extern const char *basic_interpolation_expect_from_nir_opt;
extern const char *basic_interpolation_expect_from_nir_sched;

extern const char *glxgears_vs2_nir;
extern const char *glxgears_vs2_from_nir_expect;
extern const char *glxgears_vs2_from_nir_expect_optimized;

extern const char *dot4_pre;
extern const char *dot4_copy_prop_dce;

extern const char *glxgears_vs2_from_nir_expect_cayman;
extern const char *basic_interpolation_orig_cayman;
extern const char *basic_interpolation_expect_from_nir_sched_cayman;
extern const char *basic_interpolation_expect_opt_sched_cayman;

extern const char *vs_nexted_loop_nir;
extern const char *vs_nexted_loop_from_nir_expect;
extern const char *vs_nexted_loop_from_nir_expect_opt;

extern const char *shader_with_local_array_nir;
extern const char *shader_with_local_array_expect;

extern const char *test_schedule_group;
extern const char *test_schedule_group_expect;

extern const char *shader_with_bany_nir;
extern const char *shader_with_bany_expect_eg;
extern const char *shader_with_bany_expect_opt_sched_eg;

extern const char *shader_copy_prop_dont_kill_double_use;
extern const char *shader_copy_prop_dont_kill_double_use_expect;

extern const char *shader_with_dest_array;
extern const char *shader_with_dest_array_opt_expect;
extern const char *shader_with_dest_array_opt_scheduled;

extern const char *shader_with_dest_array2;
extern const char *shader_with_dest_array2_scheduled;

extern const char *shader_with_dest_array2_scheduled_ra;

extern const char *shader_group_chan_pin_to_combine;
extern const char *shader_group_chan_pin_combined;

extern const char *shader_group_chan_pin_combined_scheduled;
extern const char *shader_group_chan_pin_combined_scheduled_ra;

extern const char *shader_group_chan_pin_to_combine_2;
extern const char *shader_group_chan_pin_to_combine_2_opt;

extern const char *fs_with_loop_multislot_reuse;
extern const char *fs_with_loop_multislot_reuse_scheduled;

extern const char *gs_abs_float_nir;
extern const char *gs_abs_float_expect;

extern const char *vtx_for_tcs_nir;
extern const char *vtx_for_tcs_from_nir_expect;

extern const char *tcs_nir;
extern const char *tcs_from_nir_expect;

extern const char *tes_nir;
extern const char *tes_from_nir_expect;

extern const char *tes_pre_op;
extern const char *tes_optimized;
extern const char *tes_optimized_pre_sched;
extern const char *tes_optimized_sched;

extern const char *vtx_for_tcs_inp;
extern const char *vtx_for_tcs_opt;
extern const char *vtx_for_tcs_pre_sched;
extern const char *vtx_for_tcs_sched;

extern const char *fs_opt_tex_coord_init;
extern const char *fs_opt_tex_coord_expect;

extern const char *fs_sched_tex_coord_init;
extern const char *fs_sched_tex_coord_expect;

class TestShader : public ::testing::Test {

   void SetUp() override;
   void TearDown() override;

   virtual void SetUpMore();
   virtual void TearDownMore();

protected:
   Shader *from_string(const std::string& s);
};

class TestShaderFromNir : public TestShader {

protected:
   void check(Shader *s, const char *expect_str);
   void ra_check(Shader *s, const char *expect_str);
};

} // namespace r600

#endif // SFN_TEST_SHADERS_H
