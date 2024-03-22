#include "sfn_test_shaders.h"

#include "../sfn_memorypool.h"
#include "../sfn_shader_fs.h"
#include "../sfn_shader_gs.h"
#include "../sfn_shader_tess.h"
#include "../sfn_shader_vs.h"

namespace r600 {

using std::istringstream;
using std::ostringstream;
using std::string;

void
TestShaderFromNir::check(Shader *s, const char *expect_orig)
{
   ostringstream test_str;
   s->print(test_str);

   auto expect = from_string(expect_orig);

   ostringstream expect_str;
   expect->print(expect_str);

   EXPECT_EQ(test_str.str(), expect_str.str());
}

void
TestShaderFromNir::ra_check(Shader *s, const char *expect_orig)
{
   s->value_factory().clear_pins();
   ostringstream test_str;
   s->print(test_str);

   auto expect = from_string(expect_orig);
   expect->value_factory().clear_pins();

   ostringstream expect_str;
   expect->print(expect_str);

   EXPECT_EQ(test_str.str(), expect_str.str());
}

const char *red_triangle_fs_nir =
   R"(shader: MESA_SHADER_FRAGMENT
name: TTN
inputs: 0
outputs: 1
uniforms: 0
shared: 0
decl_function main (0 params)

impl main {
   decl_var  INTERP_MODE_FLAT vec4 out@out_0-temp
   block block_0:
   /* preds: */
   vec4 32 ssa_0 = load_const (0x3f800000 /* 1.000000 */, 0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */, 0x3f800000 /* 1.000000 */)
   vec1 32 ssa_1 = load_const (0x00000000 /* 0.000000 */)
   intrinsic store_output (ssa_0, ssa_1) (0, 15, 0, 160, 132) /* base=0 */ /* wrmask=xyz */ /* component=0 */ /* src_type=float32 */ /* location=4 slots=1 */
   /* succs: block_1 */
   block block_1:
})";

const char *red_triangle_fs_expect_from_nir = R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S0.x@group : I[1.0] {W}
ALU MOV S0.y@group : I[0] {W}
ALU MOV S0.z@group : I[0] {W}
ALU MOV S0.w@group : I[1.0] {WL}
ALU MOV S1.x@free : I[0] {WL}
EXPORT_DONE PIXEL 0 S0.xyzw
)";

const char *red_triangle_fs_expect_from_nir_dce = R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S0.x@group : I[1.0] {W}
ALU MOV S0.y@group : I[0] {W}
ALU MOV S0.z@group : I[0] {W}
ALU MOV S0.w@group : I[1.0] {WL}
EXPORT_DONE PIXEL 0 S0.xyzw
)";

const char *add_add_1_nir =
   R"(shader: MESA_SHADER_FRAGMENT
name: GLSL3
inputs: 0
outputs: 1
uniforms: 1
shared: 0
decl_var uniform INTERP_MODE_NONE vec4 color (0, 0, 0)
decl_function main (0 params)

impl main {
     decl_var  INTERP_MODE_NONE vec4 out@gl_FragColor-temp
     block block_0:
     /* preds: */
     vec1 32 ssa_0 = load_const (0xbf000000 /* -0.500000 */)
     vec1 32 ssa_1 = load_const (0x00000000 /* 0.000000 */)
     vec4 32 ssa_2 = intrinsic load_uniform (ssa_1) (0, 1, 160) /* base=0 */ /* range=1 */ /* dest_type=float32 */   /* color */
     vec1 32 ssa_3 = fadd ssa_0, ssa_2.x
     vec4 32 ssa_4 = vec4 ssa_3, ssa_2.y, ssa_2.z, ssa_2.w
     intrinsic store_output (ssa_4, ssa_1) (0, 15, 0, 160, 130) /* base=0 */ /* wrmask=xyzw */ /* component=0 */ /* src_type=float32 */ /* location=2 slots=1 */
     /* succs: block_1 */
     block block_1:
})";

const char *add_add_1_expect_from_nir =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP WRITE_ALL_COLORS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S0.x@free : L[0xbf000000] {WL}
ALU MOV S1.x@free : I[0] {WL}
ALU MOV S2.x : KC0[0].x {W}
ALU MOV S2.y : KC0[0].y {W}
ALU MOV S2.z : KC0[0].z {W}
ALU MOV S2.w : KC0[0].w {WL}
ALU ADD S3.x@free : S0.x@free S2.x {WL}
ALU MOV S4.x@group : S3.x@free {W}
ALU MOV S4.y@group : S2.y {W}
ALU MOV S4.z@group : S2.z {W}
ALU MOV S4.w@group : S2.w {WL}
EXPORT_DONE PIXEL 0 S4.xyzw
)";

const char *add_add_1_expect_from_nir_copy_prop_fwd =
   R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP WRITE_ALL_COLORS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S0.x@free : L[0xbf000000] {WL}
ALU MOV S1.x@free : I[0] {WL}
ALU MOV S2.x : KC0[0].x {W}
ALU MOV S2.y : KC0[0].y {W}
ALU MOV S2.z : KC0[0].z {W}
ALU MOV S2.w : KC0[0].w {WL}
ALU ADD S3.x@free : L[0xbf000000] KC0[0].x {WL}
ALU MOV S4.x@group : S3.x@free {W}
ALU MOV S4.y@group : KC0[0].y {W}
ALU MOV S4.z@group : KC0[0].z {W}
ALU MOV S4.w@group : KC0[0].w {WL}
EXPORT_DONE PIXEL 0 S4.xyzw
)";

const char *add_add_1_expect_from_nir_copy_prop_fwd_dce =
   R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP WRITE_ALL_COLORS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU ADD S3.x@free : L[0xbf000000] KC0[0].x {WL}
ALU MOV S4.x@group : S3.x@free {W}
ALU MOV S4.y@group : KC0[0].y {W}
ALU MOV S4.z@group : KC0[0].z {W}
ALU MOV S4.w@group : KC0[0].w {WL}
EXPORT_DONE PIXEL 0 S4.xyzw
)";

const char *add_add_1_expect_from_nir_copy_prop_fwd_dce_bwd =
   R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP WRITE_ALL_COLORS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
ALU ADD S4.x@group : L[0xbf000000] KC0[0].x {W}
ALU MOV S4.y@group : KC0[0].y {W}
ALU MOV S4.z@group : KC0[0].z {W}
ALU MOV S4.w@group : KC0[0].w {WL}
EXPORT_DONE PIXEL 0 S4.xyzw
)";

const char *basic_interpolation_nir =
   R"(shader: MESA_SHADER_FRAGMENT
name: TTN
inputs: 1
outputs: 1
uniforms: 0
shared: 0
decl_var uniform INTERP_MODE_NONE sampler2D sampler (0, 0, 0)
decl_function main (0 params)

impl main {
        decl_var  INTERP_MODE_NOPERSPECTIVE vec4 in@in_0-temp
        decl_var  INTERP_MODE_FLAT vec4 out@out_0-temp
        block block_0:
        /* preds: */
        vec2 32 ssa_0 = intrinsic load_barycentric_pixel () (3) /* interp_mode=3 */
        vec1 32 ssa_1 = load_const (0x00000000 /* 0.000000 */)
        vec4 32 ssa_2 = intrinsic load_interpolated_input (ssa_0, ssa_1) (0, 0, 160, 160) /* base=0 */ /* component=0 */ /* dest_type=float32 */ /* location=32 slots=1 */
        vec3 32 ssa_3 = f2i32 ssa_2.xyw
        vec1 32 ssa_4 = mov ssa_3.z
        vec2 32 ssa_5 = vec2 ssa_3.x, ssa_3.y
        vec4 32 ssa_6 = (float32)txf ssa_5 (coord), ssa_4 (lod), 0 (texture), 0 (sampler)
        intrinsic store_output (ssa_6, ssa_1) (0, 15, 0, 160, 132) /* base=0 */ /* wrmask=xyzw */ /* component=0 */ /* src_type=float32 */ /* location=4 slots=1 */
        /* succs: block_1 */
        block block_1:
})";

const char *basic_interpolation_expect_from_nir =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
ALU MOV S1.x@free : I[0] {WL}
ALU_GROUP_BEGIN
ALU INTERP_ZW  __.x@chan : R0.y@fully Param0.x VEC_210 {}
ALU INTERP_ZW  __.y@chan : R0.x@fully Param0.y VEC_210 {}
ALU INTERP_ZW  S2.z@chan : R0.y@fully Param0.z VEC_210 {W}
ALU INTERP_ZW  S2.w@chan : R0.x@fully Param0.w VEC_210 {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY  S2.x@chan : R0.y@fully Param0.x VEC_210 {W}
ALU INTERP_XY  S2.y@chan : R0.x@fully Param0.y VEC_210 {W}
ALU INTERP_XY  __.z@chan : R0.y@fully Param0.z VEC_210 {}
ALU INTERP_XY  __.w@chan : R0.x@fully Param0.w VEC_210 {L}
ALU_GROUP_END

ALU TRUNC S3.x@free : S2.x@chan {WL}
ALU TRUNC S4.y@free : S2.y@chan {WL}
ALU TRUNC S5.z@free : S2.w@chan {WL}

ALU FLT_TO_INT S6.x : S3.x@free {W}
ALU FLT_TO_INT S6.y : S4.y@free {W}
ALU FLT_TO_INT S6.z : S5.z@free {WL}

ALU MOV S7.x@free : S6.z {WL}
ALU MOV S8.x : S6.x {W}
ALU MOV S8.y : S6.y {WL}
ALU MOV S9.x@group : S8.x {W}
ALU MOV S9.y@group : S8.y {W}
ALU MOV S9.w@group : S7.x@free {WL}
TEX LD S10.xyzw : S9.xy_w RID:18 SID:0 NNNN
EXPORT_DONE PIXEL 0 S10.xyzw)";

const char *basic_interpolation_translated_1 =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
ALU MOV S1.x@free : I[0] {WL}
ALU_GROUP_BEGIN
ALU INTERP_ZW  __.x@chan : R0.y@fully Param0.x VEC_210 {}
ALU INTERP_ZW  __.y@chan : R0.x@fully Param0.y VEC_210 {}
ALU INTERP_ZW  S2.z@chan : R0.y@fully Param0.z VEC_210 {W}
ALU INTERP_ZW  S2.w@chan : R0.x@fully Param0.w VEC_210 {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY  S2.x@chan : R0.y@fully Param0.x VEC_210 {W}
ALU INTERP_XY  S2.y@chan : R0.x@fully Param0.y VEC_210 {W}
ALU INTERP_XY  __.z@chan : R0.y@fully Param0.z VEC_210 {}
ALU INTERP_XY  __.w@chan : R0.x@fully Param0.w VEC_210 {L}
ALU_GROUP_END

ALU FLT_TO_INT S3.x : S2.x@free {W}
ALU FLT_TO_INT S3.y : S2.y@free {W}
ALU FLT_TO_INT S3.z : S2.w@free {WL}
ALU MOV S4.x : S3.x {W}
ALU MOV S4.y : S3.y {WL}
ALU MOV S5.x@group : S4.x {W}
ALU MOV S5.y@group : S4.y {W}
ALU MOV S5.w@group : S3.z {WL}
TEX LD S6.xyzw : S5.xy_w RID:18 SID:0 NNNN
EXPORT_DONE PIXEL 0 S6.xyzw)";

const char *basic_interpolation_2 =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
ALU_GROUP_BEGIN
ALU INTERP_ZW  __.x@chan : R0.y@fully Param0.x VEC_210 {}
ALU INTERP_ZW  __.y@chan : R0.x@fully Param0.y VEC_210 {}
ALU INTERP_ZW  S2.z@chan : R0.y@fully Param0.z VEC_210 {W}
ALU INTERP_ZW  S2.w@chan : R0.x@fully Param0.w VEC_210 {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY  S2.x@chan : R0.y@fully Param0.x VEC_210 {W}
ALU INTERP_XY  S2.y@chan : R0.x@fully Param0.y VEC_210 {W}
ALU INTERP_XY  __.z@chan : R0.y@fully Param0.z VEC_210 {}
ALU INTERP_XY  __.w@chan : R0.x@fully Param0.w VEC_210 {L}
ALU_GROUP_END
EXPORT_DONE PIXEL 0 S2.xyzw
)";

const char *basic_interpolation_orig =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
ALU MOV S1024.x : I[0] {WL}
ALU_GROUP_BEGIN
ALU INTERP_ZW  __.x@chan : R0.y@fully Param0.x VEC_210 {}
ALU INTERP_ZW  __.y@chan : R0.x@fully Param0.y VEC_210 {}
ALU INTERP_ZW  S1025.z@chan : R0.y@fully Param0.z VEC_210 {W}
ALU INTERP_ZW  S1025.w@chan : R0.x@fully Param0.w VEC_210 {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY  S1025.x@chan : R0.y@fully Param0.x VEC_210 {W}
ALU INTERP_XY  S1025.y@chan : R0.x@fully Param0.y VEC_210 {W}
ALU INTERP_XY  __.z@chan : R0.y@fully Param0.z VEC_210 {}
ALU INTERP_XY  __.w@chan : R0.x@fully Param0.w VEC_210 {L}
ALU_GROUP_END

ALU FLT_TO_INT S1026.x : S1025.x@chan {W}
ALU FLT_TO_INT S1026.y : S1025.y@chan {W}
ALU FLT_TO_INT S1026.z : S1025.w@chan {WL}
ALU MOV S1027.x : S1026.x {W}
ALU MOV S1027.y : S1026.y {WL}
ALU MOV S1028.x@group : S1027.x {W}
ALU MOV S1028.y@group : S1027.y {W}
ALU MOV S1028.w@group : S1026.z {WL}
TEX LD S1029.xyzw : S1028.xy_w RID:0 SID:18 NNNN
EXPORT_DONE PIXEL 0 S1029.xyzw
)";

const char *basic_interpolation_expect_from_nir_sched =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
BLOCK_START
ALU_GROUP_BEGIN
ALU INTERP_ZW  __.x@chan : R0.y@fully Param0.x VEC_210 {}
ALU INTERP_ZW  __.y@chan : R0.x@fully Param0.y VEC_210 {}
ALU INTERP_ZW  S1025.z@chan : R0.y@fully Param0.z VEC_210 {W}
ALU INTERP_ZW  S1025.w@chan : R0.x@fully Param0.w VEC_210 {W}
ALU MOV S1024.x : I[0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY  S1025.x@chan : R0.y@fully Param0.x VEC_210 {W}
ALU INTERP_XY  S1025.y@chan : R0.x@fully Param0.y VEC_210 {W}
ALU INTERP_XY  __.z@chan : R0.y@fully Param0.z VEC_210 {}
ALU INTERP_XY  __.w@chan : R0.x@fully Param0.w VEC_210 {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU FLT_TO_INT S1026.x : S1025.x@chan {W}
ALU FLT_TO_INT S1026.y : S1025.y@chan {W}
ALU FLT_TO_INT S1026.z : S1025.w@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU MOV S1027.x : S1026.x {W}
ALU MOV S1027.y : S1026.y {W}
ALU MOV S1028.w@group : S1026.z {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU MOV S1028.x@group : S1027.x {W}
ALU MOV S1028.y@group : S1027.y {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
TEX LD S1029.xyzw : S1028.xy_w RID:0 SID:18 NNNN
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S1029.xyzw
BLOCK_END
)";

const char *basic_interpolation_orig_cayman =
   R"(FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
ALU MOV S1024.x : I[0] {WL}
ALU_GROUP_BEGIN
ALU INTERP_ZW  __.x@chan : R0.y@fully Param0.x VEC_210 {}
ALU INTERP_ZW  __.y@chan : R0.x@fully Param0.y VEC_210 {}
ALU INTERP_ZW  S1025.z@chan : R0.y@fully Param0.z VEC_210 {W}
ALU INTERP_ZW  S1025.w@chan : R0.x@fully Param0.w VEC_210 {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY  S1025.x@chan : R0.y@fully Param0.x VEC_210 {W}
ALU INTERP_XY  S1025.y@chan : R0.x@fully Param0.y VEC_210 {W}
ALU INTERP_XY  __.z@chan : R0.y@fully Param0.z VEC_210 {}
ALU INTERP_XY  __.w@chan : R0.x@fully Param0.w VEC_210 {L}
ALU_GROUP_END

ALU FLT_TO_INT S1026.x : S1025.x@chan {W}
ALU FLT_TO_INT S1026.y : S1025.y@chan {W}
ALU FLT_TO_INT S1026.z : S1025.w@chan {WL}
ALU MOV S1027.x : S1026.x {W}
ALU MOV S1027.y : S1026.y {WL}
ALU MOV S1028.x@group : S1027.x {W}
ALU MOV S1028.y@group : S1027.y {W}
ALU MOV S1028.w@group : S1026.z {WL}
TEX LD S1029.xyzw : S1028.xy_w RID:0 SID:18 NNNN
EXPORT_DONE PIXEL 0 S1029.xyzw
)";

const char *basic_interpolation_expect_from_nir_sched_cayman =
   R"(FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
BLOCK_START
ALU_GROUP_BEGIN
ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
ALU INTERP_ZW S1025.z@chan : R0.y@fully Param0.z {W} VEC_210
ALU INTERP_ZW S1025.w@chan : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY S1025.x@chan : R0.y@fully Param0.x {W} VEC_210
ALU INTERP_XY S1025.y@chan : R0.x@fully Param0.y {W} VEC_210
ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU FLT_TO_INT S1026.x : S1025.x@chan {W}
ALU FLT_TO_INT S1026.y : S1025.y@chan {W}
ALU FLT_TO_INT S1026.z : S1025.w@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU MOV S1027.x : S1026.x {W}
ALU MOV S1027.y : S1026.y {W}
ALU MOV S1028.w@group : S1026.z {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU MOV S1028.x@group : S1027.x {W}
ALU MOV S1028.y@group : S1027.y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU MOV S1024.x : I[0] {WL}
ALU_GROUP_END
BLOCK_START
BLOCK_END
TEX LD S1029.xyzw : S1028.xy_w RID:0 SID:18 NNNN
BLOCK_START
BLOCK_END
EXPORT_DONE PIXEL 0 S1029.xyzw
BLOCK_END
)";

const char *basic_interpolation_expect_opt_sched_cayman =
   R"(FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
BLOCK_START
ALU_GROUP_BEGIN
ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
ALU INTERP_ZW S1025.z@chan : R0.y@fully Param0.z {W} VEC_210
ALU INTERP_ZW S1025.w@chan : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY S1025.x@chan : R0.y@fully Param0.x {W} VEC_210
ALU INTERP_XY S1025.y@chan : R0.x@fully Param0.y {W} VEC_210
ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU FLT_TO_INT S1026.x@group : S1025.x@chan {W}
ALU FLT_TO_INT S1026.y@group : S1025.y@chan {W}
ALU FLT_TO_INT S1026.z@group : S1025.w@chan {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
TEX LD S1029.xyzw : S1026.xy_z RID:0 SID:18 NNNN
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S1029.xyzw
BLOCK_END
)";

const char *basic_interpolation_expect_from_nir_opt =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.xy__
SHADER
ALU_GROUP_BEGIN
ALU INTERP_ZW  __.x@chan : R0.y@fully Param0.x VEC_210 {}
ALU INTERP_ZW  __.y@chan : R0.x@fully Param0.y VEC_210 {}
ALU INTERP_ZW  S1025.z@chan : R0.y@fully Param0.z VEC_210 {W}
ALU INTERP_ZW  S1025.w@chan : R0.x@fully Param0.w VEC_210 {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY  S1025.x@chan : R0.y@fully Param0.x VEC_210 {W}
ALU INTERP_XY  S1025.y@chan : R0.x@fully Param0.y VEC_210 {W}
ALU INTERP_XY  __.z@chan : R0.y@fully Param0.z VEC_210 {}
ALU INTERP_XY  __.w@chan : R0.x@fully Param0.w VEC_210 {L}
ALU_GROUP_END
ALU FLT_TO_INT S1026.x@group : S1025.x@chan {W}
ALU FLT_TO_INT S1026.y@group : S1025.y@chan {W}
ALU FLT_TO_INT S1026.z@group : S1025.w@chan {WL}
TEX LD S1029.xyzw : S1026.xy_z RID:0 SID:18 NNNN
EXPORT_DONE PIXEL 0 S1029.xyzw
)";

const char *dot4_pre =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S1.x : KC0[0].x  {W}
ALU MOV S1.y : KC0[0].y  {W}
ALU MOV S1.z : KC0[0].z  {W}
ALU MOV S1.w : KC0[0].w  {WL}
ALU MOV S2.x : KC0[1].x  {W}
ALU MOV S2.y : KC0[1].y  {W}
ALU MOV S2.z : KC0[1].z  {W}
ALU MOV S2.w : KC0[1].w  {WL}
ALU DOT4_IEEE S3.x@free : S1.x S2.x + S1.y S2.y + S1.z S2.z + S1.w S2.w  {WL}
ALU MOV S4.x : S3.x@free {W}
ALU MOV S4.y : S3.x@free {W}
ALU MOV S4.z : S3.x@free {W}
ALU MOV S4.w : S3.x@free {W}
EXPORT_DONE PIXEL 0 S4.xyzw
)";

const char *dot4_copy_prop_dce =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
#PROP RAT_BASE:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S2.x : KC0[1].x  {W}
ALU MOV S2.y : KC0[1].y  {W}
ALU MOV S2.z : KC0[1].z  {W}
ALU MOV S2.w : KC0[1].w  {WL}
ALU DOT4_IEEE S3.x@group : KC0[0].x S2.x + KC0[0].y S2.y + KC0[0].z S2.z + KC0[0].w S2.w  {WL}
EXPORT_DONE PIXEL 0 S3.xxxx
)";

const char *glxgears_vs2_nir =
   R"(shader: MESA_SHADER_VERTEX
name: ARB0
inputs: 2
outputs: 2
uniforms: 11
shared: 0
decl_var uniform INTERP_MODE_NONE vec4[11] name (0, 0, 0)
decl_function main (0 params)

impl main {
        block block_0:
        /* preds: */
        vec1 32 ssa_0 = load_const (0x00000000 /* 0.000000 */)
        vec4 32 ssa_1 = intrinsic load_input (ssa_0) (0, 0, 160, 128) /* base=0 */ /* component=0 */ /* dest_type=float32 */ /* location=0 slots=1 */
        vec1 32 ssa_2 = load_const (0x00000006 /* 0.000000 */)
        vec4 32 ssa_3 = intrinsic load_uniform (ssa_2) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */ /*  */
        vec4 32 ssa_4 = fmul ssa_1.xxxx, ssa_3
        vec1 32 ssa_5 = load_const (0x00000007 /* 0.000000 */)
        vec4 32 ssa_6 = intrinsic load_uniform (ssa_5) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */ /*  */
        vec4 32 ssa_7 = ffma ssa_1.yyyy, ssa_6, ssa_4
        vec1 32 ssa_8 = load_const (0x00000008 /* 0.000000 */)
        vec4 32 ssa_9 = intrinsic load_uniform (ssa_8) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */ /*  */
        vec4 32 ssa_10 = ffma ssa_1.zzzz, ssa_9, ssa_7
        vec1 32 ssa_11 = load_const (0x00000009 /* 0.000000 */)
        vec4 32 ssa_12 = intrinsic load_uniform (ssa_11) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */       /*  */
        vec4 32 ssa_13 = ffma ssa_1.wwww, ssa_12, ssa_10
        vec4 32 ssa_14 = intrinsic load_input (ssa_0) (1, 0, 160, 129) /* base=1 */ /* component=0 */ /* dest_type=float32 */ /* location=1 slots=1 */
        vec1 32 ssa_15 = fdot3 ssa_14.xyz, ssa_14.xyz
        vec1 32 ssa_16 = frsq abs(ssa_15)
        vec4 32 ssa_17 = fmul ssa_14, ssa_16.xxxx
        vec1 32 ssa_18 = load_const (0x00000002 /* 0.000000 */)
        vec4 32 ssa_19 = intrinsic load_uniform (ssa_18) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */       /*  */
        vec1 32 ssa_20 = load_const (0x0000000a /* 0.000000 */)
        vec4 32 ssa_21 = intrinsic load_uniform (ssa_20) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */       /*  */
        vec1 32 ssa_22 = fdot3 ssa_17.xyz, ssa_21.xyz
        vec4 32 ssa_23 = load_const (0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */, 0x3f800000 /* 1.000000 */)
        vec1 32 ssa_24 = fmax ssa_23.y, ssa_22
        vec4 32 ssa_25 = load_const (0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */)
        vec1 32 ssa_26 = slt ssa_25.z, ssa_22
        vec1 32 ssa_27 = load_const (0x00000003 /* 0.000000 */)
        vec4 32 ssa_28 = intrinsic load_uniform (ssa_27) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */       /*  */
        vec3 32 ssa_29 = fadd ssa_28.xyz, ssa_19.xyz
        vec1 32 ssa_30 = load_const (0x00000004 /* 0.000000 */)
        vec4 32 ssa_31 = intrinsic load_uniform (ssa_30) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */       /*  */
        vec3 32 ssa_32 = ffma ssa_24.xxx, ssa_31.xyz, ssa_29
        vec1 32 ssa_33 = load_const (0x00000005 /* 0.000000 */)
        vec4 32 ssa_34 = intrinsic load_uniform (ssa_33) (0, 11, 160) /* base=0 */ /* range=11 */ /* dest_type=float32 */       /*  */
        vec3 32 ssa_35 = ffma.sat ssa_26.xxx, ssa_34.xyz, ssa_32
        intrinsic store_output (ssa_13, ssa_0) (0, 15, 0, 160, 128) /* base=0 */ /* wrmask=xyzw */ /* component=0 */ /* src_type=float32 */ /* location=0 slots=1 */
        vec3 32 ssa_36 = mov ssa_35
        vec1 32 ssa_37 = fsat ssa_19.w
        vec4 32 ssa_38 = vec4 ssa_36.x, ssa_36.y, ssa_36.z, ssa_37
        intrinsic store_output (ssa_38, ssa_0) (1, 15, 0, 160, 129) /* base=1 */ /* wrmask=xyzw */ /* component=0 */ /* src_type=float32 */ /* location=1 slots=1 */
        /* succs: block_1 */
        block block_1:
})";

const char *glxgears_vs2_from_nir_expect =
   R"(VS
CHIPCLASS EVERGREEN
INPUT LOC:0
INPUT LOC:1
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:1 MASK:15
SYSVALUES R1.xyzw R2.xyzw
SHADER
ALU MOV S3.x@free : I[0] {WL}
ALU MOV S4.x@free : L[0x6] {WL}
ALU MOV S5.x : KC0[6].x {W}
ALU MOV S5.y : KC0[6].y {W}
ALU MOV S5.z : KC0[6].z {W}
ALU MOV S5.w : KC0[6].w {WL}
ALU MUL_IEEE S6.x : R1.x@fully S5.x {W}
ALU MUL_IEEE S6.y : R1.x@fully S5.y {W}
ALU MUL_IEEE S6.z : R1.x@fully S5.z {W}
ALU MUL_IEEE S6.w : R1.x@fully S5.w {WL}
ALU MOV S7.x@free : L[0x7] {WL}
ALU MOV S8.x : KC0[7].x {W}
ALU MOV S8.y : KC0[7].y {W}
ALU MOV S8.z : KC0[7].z {W}
ALU MOV S8.w : KC0[7].w {WL}
ALU MULADD_IEEE S9.x : R1.y@fully S8.x S6.x {W}
ALU MULADD_IEEE S9.y : R1.y@fully S8.y S6.y {W}
ALU MULADD_IEEE S9.z : R1.y@fully S8.z S6.z {W}
ALU MULADD_IEEE S9.w : R1.y@fully S8.w S6.w {WL}
ALU MOV S10.x@free : L[0x8] {WL}
ALU MOV S11.x : KC0[8].x {W}
ALU MOV S11.y : KC0[8].y {W}
ALU MOV S11.z : KC0[8].z {W}
ALU MOV S11.w : KC0[8].w {WL}
ALU MULADD_IEEE S12.x : R1.z@fully S11.x S9.x {W}
ALU MULADD_IEEE S12.y : R1.z@fully S11.y S9.y {W}
ALU MULADD_IEEE S12.z : R1.z@fully S11.z S9.z {W}
ALU MULADD_IEEE S12.w : R1.z@fully S11.w S9.w {WL}
ALU MOV S13.x@free : L[0x9] {WL}
ALU MOV S14.x : KC0[9].x {W}
ALU MOV S14.y : KC0[9].y {W}
ALU MOV S14.z : KC0[9].z {W}
ALU MOV S14.w : KC0[9].w {WL}
ALU MULADD_IEEE S15.x@group : R1.w@fully S14.x S12.x {W}
ALU MULADD_IEEE S15.y@group : R1.w@fully S14.y S12.y {W}
ALU MULADD_IEEE S15.z@group : R1.w@fully S14.z S12.z {W}
ALU MULADD_IEEE S15.w@group : R1.w@fully S14.w S12.w {WL}
ALU DOT4_IEEE S16.x@free : R2.x@fully R2.x@fully + R2.y@fully R2.y@fully + R2.z@fully R2.z@fully + I[0].x I[0].x {WL}
ALU RECIPSQRT_IEEE S17.x@free : |S16.x@free| {WL}
ALU MUL_IEEE S18.x : R2.x@fully S17.x@free {W}
ALU MUL_IEEE S18.y : R2.y@fully S17.x@free {W}
ALU MUL_IEEE S18.z : R2.z@fully S17.x@free {W}
ALU MUL_IEEE S18.w : R2.w@fully S17.x@free {WL}
ALU MOV S19.x@free : L[0x2] {WL}
ALU MOV S20.x : KC0[2].x {W}
ALU MOV S20.y : KC0[2].y {W}
ALU MOV S20.z : KC0[2].z {W}
ALU MOV S20.w : KC0[2].w {WL}
ALU MOV S21.x@free : L[0xa] {WL}
ALU MOV S22.x : KC0[10].x {W}
ALU MOV S22.y : KC0[10].y {W}
ALU MOV S22.z : KC0[10].z {W}
ALU MOV S22.w : KC0[10].w {WL}
ALU DOT4_IEEE S23.x@free : S18.x S22.x + S18.y S22.y + S18.z S22.z + I[0].x I[0].x {WL}
ALU MOV S24.x : I[0] {W}
ALU MOV S24.y : I[0] {W}
ALU MOV S24.z : I[0] {W}
ALU MOV S24.w : I[1.0] {WL}
ALU MAX_DX10 S25.x@free : S24.y S23.x@free {WL}
ALU MOV S26.x : I[0] {W}
ALU MOV S26.y : I[0] {W}
ALU MOV S26.z : I[0] {W}
ALU MOV S26.w : I[0] {WL}
ALU SETGT S27.x@free : S23.x@free S26.z {WL}
ALU MOV S28.x@free : L[0x3] {WL}
ALU MOV S29.x : KC0[3].x {W}
ALU MOV S29.y : KC0[3].y {W}
ALU MOV S29.z : KC0[3].z {W}
ALU MOV S29.w : KC0[3].w {WL}
ALU ADD S30.x : S29.x S20.x {W}
ALU ADD S30.y : S29.y S20.y {W}
ALU ADD S30.z : S29.z S20.z {WL}
ALU MOV S31.x@free : L[0x4] {WL}
ALU MOV S32.x : KC0[4].x {W}
ALU MOV S32.y : KC0[4].y {W}
ALU MOV S32.z : KC0[4].z {W}
ALU MOV S32.w : KC0[4].w {WL}
ALU MULADD_IEEE S33.x : S25.x@free S32.x S30.x {W}
ALU MULADD_IEEE S33.y : S25.x@free S32.y S30.y {W}
ALU MULADD_IEEE S33.z : S25.x@free S32.z S30.z {WL}
ALU MOV S34.x@free : L[0x5] {WL}
ALU MOV S35.x : KC0[5].x {W}
ALU MOV S35.y : KC0[5].y {W}
ALU MOV S35.z : KC0[5].z {W}
ALU MOV S35.w : KC0[5].w {WL}
ALU MULADD_IEEE CLAMP S36.x : S27.x@free S35.x S33.x {W}
ALU MULADD_IEEE CLAMP S36.y : S27.x@free S35.y S33.y {W}
ALU MULADD_IEEE CLAMP S36.z : S27.x@free S35.z S33.z {WL}
EXPORT_DONE POS 0 S15.xyzw
ALU MOV S38.x : S36.x {W}
ALU MOV S38.y : S36.y {W}
ALU MOV S38.z : S36.z {WL}
ALU MOV CLAMP S39.x@free : S20.w {WL}
ALU MOV S40.x@group : S38.x {W}
ALU MOV S40.y@group : S38.y {W}
ALU MOV S40.z@group : S38.z {W}
ALU MOV S40.w@group : S39.x@free {WL}
EXPORT_DONE PARAM 0 S40.xyzw)";

const char *glxgears_vs2_from_nir_expect_cayman =
   R"(VS
CHIPCLASS CAYMAN
INPUT LOC:0
INPUT LOC:1
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:1 MASK:15
SYSVALUES R1.xyzw R2.xyzw
SHADER
ALU MOV S3.x@free : I[0] {WL}
ALU MOV S4.x@free : L[0x6] {WL}
ALU MOV S5.x : KC0[6].x {W}
ALU MOV S5.y : KC0[6].y {W}
ALU MOV S5.z : KC0[6].z {W}
ALU MOV S5.w : KC0[6].w {WL}
ALU MUL_IEEE S6.x : R1.x@fully S5.x {W}
ALU MUL_IEEE S6.y : R1.x@fully S5.y {W}
ALU MUL_IEEE S6.z : R1.x@fully S5.z {W}
ALU MUL_IEEE S6.w : R1.x@fully S5.w {WL}
ALU MOV S7.x@free : L[0x7] {WL}
ALU MOV S8.x : KC0[7].x {W}
ALU MOV S8.y : KC0[7].y {W}
ALU MOV S8.z : KC0[7].z {W}
ALU MOV S8.w : KC0[7].w {WL}
ALU MULADD_IEEE S9.x : R1.y@fully S8.x S6.x {W}
ALU MULADD_IEEE S9.y : R1.y@fully S8.y S6.y {W}
ALU MULADD_IEEE S9.z : R1.y@fully S8.z S6.z {W}
ALU MULADD_IEEE S9.w : R1.y@fully S8.w S6.w {WL}
ALU MOV S10.x@free : L[0x8] {WL}
ALU MOV S11.x : KC0[8].x {W}
ALU MOV S11.y : KC0[8].y {W}
ALU MOV S11.z : KC0[8].z {W}
ALU MOV S11.w : KC0[8].w {WL}
ALU MULADD_IEEE S12.x : R1.z@fully S11.x S9.x {W}
ALU MULADD_IEEE S12.y : R1.z@fully S11.y S9.y {W}
ALU MULADD_IEEE S12.z : R1.z@fully S11.z S9.z {W}
ALU MULADD_IEEE S12.w : R1.z@fully S11.w S9.w {WL}
ALU MOV S13.x@free : L[0x9] {WL}
ALU MOV S14.x : KC0[9].x {W}
ALU MOV S14.y : KC0[9].y {W}
ALU MOV S14.z : KC0[9].z {W}
ALU MOV S14.w : KC0[9].w {WL}
ALU MULADD_IEEE S15.x@group : R1.w@fully S14.x S12.x {W}
ALU MULADD_IEEE S15.y@group : R1.w@fully S14.y S12.y {W}
ALU MULADD_IEEE S15.z@group : R1.w@fully S14.z S12.z {W}
ALU MULADD_IEEE S15.w@group : R1.w@fully S14.w S12.w {WL}
ALU DOT4_IEEE S16.x@free : R2.x@fully R2.x@fully + R2.y@fully R2.y@fully + R2.z@fully R2.z@fully + I[0].x I[0].x {WL}
ALU RECIPSQRT_IEEE S17.x@chan : |S16.x@free| + |S16.x@free| + S16.x@free {WL}
ALU MUL_IEEE S18.x : R2.x@fully S17.x@free {W}
ALU MUL_IEEE S18.y : R2.y@fully S17.x@free {W}
ALU MUL_IEEE S18.z : R2.z@fully S17.x@free {W}
ALU MUL_IEEE S18.w : R2.w@fully S17.x@free {WL}
ALU MOV S19.x@free : L[0x2] {WL}
ALU MOV S20.x : KC0[2].x {W}
ALU MOV S20.y : KC0[2].y {W}
ALU MOV S20.z : KC0[2].z {W}
ALU MOV S20.w : KC0[2].w {WL}
ALU MOV S21.x@free : L[0xa] {WL}
ALU MOV S22.x : KC0[10].x {W}
ALU MOV S22.y : KC0[10].y {W}
ALU MOV S22.z : KC0[10].z {W}
ALU MOV S22.w : KC0[10].w {WL}
ALU DOT4_IEEE S23.x@free : S18.x S22.x + S18.y S22.y + S18.z S22.z + I[0].x I[0].x {WL}
ALU MOV S24.x : I[0] {W}
ALU MOV S24.y : I[0] {W}
ALU MOV S24.z : I[0] {W}
ALU MOV S24.w : I[1.0] {WL}
ALU MAX_DX10 S25.x@free : S24.y S23.x@free {WL}
ALU MOV S26.x : I[0] {W}
ALU MOV S26.y : I[0] {W}
ALU MOV S26.z : I[0] {W}
ALU MOV S26.w : I[0] {WL}
ALU SETGT S27.x@free : S23.x@free S26.z {WL}
ALU MOV S28.x@free : L[0x3] {WL}
ALU MOV S29.x : KC0[3].x {W}
ALU MOV S29.y : KC0[3].y {W}
ALU MOV S29.z : KC0[3].z {W}
ALU MOV S29.w : KC0[3].w {WL}
ALU ADD S30.x : S29.x S20.x {W}
ALU ADD S30.y : S29.y S20.y {W}
ALU ADD S30.z : S29.z S20.z {WL}
ALU MOV S31.x@free : L[0x4] {WL}
ALU MOV S32.x : KC0[4].x {W}
ALU MOV S32.y : KC0[4].y {W}
ALU MOV S32.z : KC0[4].z {W}
ALU MOV S32.w : KC0[4].w {WL}
ALU MULADD_IEEE S33.x : S25.x@free S32.x S30.x {W}
ALU MULADD_IEEE S33.y : S25.x@free S32.y S30.y {W}
ALU MULADD_IEEE S33.z : S25.x@free S32.z S30.z {WL}
ALU MOV S34.x@free : L[0x5] {WL}
ALU MOV S35.x : KC0[5].x {W}
ALU MOV S35.y : KC0[5].y {W}
ALU MOV S35.z : KC0[5].z {W}
ALU MOV S35.w : KC0[5].w {WL}
ALU MULADD_IEEE CLAMP S36.x : S27.x@free S35.x S33.x {W}
ALU MULADD_IEEE CLAMP S36.y : S27.x@free S35.y S33.y {W}
ALU MULADD_IEEE CLAMP S36.z : S27.x@free S35.z S33.z {WL}
EXPORT_DONE POS 0 S15.xyzw
ALU MOV S38.x : S36.x {W}
ALU MOV S38.y : S36.y {W}
ALU MOV S38.z : S36.z {WL}
ALU MOV CLAMP S39.x@free : S20.w {WL}
ALU MOV S40.x@group : S38.x {W}
ALU MOV S40.y@group : S38.y {W}
ALU MOV S40.z@group : S38.z {W}
ALU MOV S40.w@group : S39.x@free {WL}
EXPORT_DONE PARAM 0 S40.xyzw)";

const char *glxgears_vs2_from_nir_expect_optimized =
   R"(VS
CHIPCLASS EVERGREEN
INPUT LOC:0
INPUT LOC:1
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:1 MASK:15
SYSVALUES R1.xyzw R2.xyzw
SHADER
ALU MUL_IEEE S6.x : R1.x@fully KC0[6].x {W}
ALU MUL_IEEE S6.y : R1.x@fully KC0[6].y {W}
ALU MUL_IEEE S6.z : R1.x@fully KC0[6].z {W}
ALU MUL_IEEE S6.w : R1.x@fully KC0[6].w {WL}
ALU MULADD_IEEE S9.x : R1.y@fully KC0[7].x S6.x {W}
ALU MULADD_IEEE S9.y : R1.y@fully KC0[7].y S6.y {W}
ALU MULADD_IEEE S9.z : R1.y@fully KC0[7].z S6.z {W}
ALU MULADD_IEEE S9.w : R1.y@fully KC0[7].w S6.w {WL}
ALU MULADD_IEEE S12.x : R1.z@fully KC0[8].x S9.x {W}
ALU MULADD_IEEE S12.y : R1.z@fully KC0[8].y S9.y {W}
ALU MULADD_IEEE S12.z : R1.z@fully KC0[8].z S9.z {W}
ALU MULADD_IEEE S12.w : R1.z@fully KC0[8].w S9.w {WL}
ALU MULADD_IEEE S15.x@group : R1.w@fully KC0[9].x S12.x {W}
ALU MULADD_IEEE S15.y@group : R1.w@fully KC0[9].y S12.y {W}
ALU MULADD_IEEE S15.z@group : R1.w@fully KC0[9].z S12.z {W}
ALU MULADD_IEEE S15.w@group : R1.w@fully KC0[9].w S12.w {WL}
ALU DOT4_IEEE S16.x@free : R2.x@fully R2.x@fully + R2.y@fully R2.y@fully + R2.z@fully R2.z@fully + I[0].x I[0].x {WL}
ALU RECIPSQRT_IEEE S17.x@free : |S16.x@free| {WL}
ALU MUL_IEEE S18.x : R2.x@fully S17.x@free {W}
ALU MUL_IEEE S18.y : R2.y@fully S17.x@free {W}
ALU MUL_IEEE S18.z : R2.z@fully S17.x@free {W}
ALU DOT4_IEEE S23.x@free : S18.x KC0[10].x + S18.y KC0[10].y + S18.z KC0[10].z + I[0].x I[0].x {WL}
ALU MAX_DX10 S25.x@free : I[0] S23.x@free {WL}
ALU SETGT S27.x@free : S23.x@free I[0] {WL}
ALU ADD S30.x : KC0[3].x KC0[2].x {W}
ALU ADD S30.y : KC0[3].y KC0[2].y {W}
ALU ADD S30.z : KC0[3].z KC0[2].z {WL}
ALU MULADD_IEEE S33.x : S25.x@free KC0[4].x S30.x {W}
ALU MULADD_IEEE S33.y : S25.x@free KC0[4].y S30.y {W}
ALU MULADD_IEEE S33.z : S25.x@free KC0[4].z S30.z {WL}
ALU MULADD_IEEE CLAMP S1024.x@group : S27.x@free KC0[5].x S33.x {W}
ALU MULADD_IEEE CLAMP S1024.y@group : S27.x@free KC0[5].y S33.y {W}
ALU MULADD_IEEE CLAMP S1024.z@group : S27.x@free KC0[5].z S33.z {WL}
EXPORT_DONE POS 0 S15.xyzw
ALU MOV CLAMP S1024.w@group : KC0[2].w {WL}
EXPORT_DONE PARAM 0 S1024.xyzw)";

const char *vs_nexted_loop_nir =
   R"(shader: MESA_SHADER_VERTEX
name: GLSL3
inputs: 1
outputs: 2
uniforms: 3
shared: 0
decl_var uniform INTERP_MODE_NONE int a (0, 0, 0)
decl_var uniform INTERP_MODE_NONE int b (1, 1, 0)
decl_var uniform INTERP_MODE_NONE int c (2, 2, 0)
decl_function main (0 params)

impl main {
   decl_var  INTERP_MODE_NONE vec4 out@gl_Position-temp
   decl_var  INTERP_MODE_NONE vec4 out@gl_FrontColor-temp
   decl_reg vec1 32 r2
   decl_reg vec1 32 r3
   decl_reg vec1 32 r4
   decl_reg vec1 32 r5
   decl_reg vec1 32 r6
   decl_reg vec1 32 r7
   decl_reg vec1 32 r8
   block block_0:
   /* preds: */
   vec1 32 ssa_0 = load_const (0x00000000 /* 0.000000 */)
   vec4 32 ssa_1 = intrinsic load_input (ssa_0) (0, 0, 160, 128) /* base=0 */ /* component=0 */ /* dest_type=float32 */ /* location=0 slots=1 */
   vec1 32 ssa_2 = load_const (0xffffffff /* -nan */)
   vec1 32 ssa_3 = load_const (0x00000000 /* 0.000000 */)
   vec1 32 ssa_4 = load_const (0x00000001 /* 0.000000 */)
   vec4 32 ssa_5 = load_const (0x3f800000 /* 1.000000 */, 0x3f800000 /* 1.000000 */, 0x00000000 /* 0.000000 */, 0x3f800000 /* 1.000000 */)
   vec1 32 ssa_6 = load_const (0x00000002 /* 0.000000 */)
   vec1 32 ssa_7 = intrinsic load_uniform (ssa_0) (0, 1, 34) /* base=0 */ /* range=1 */ /* dest_type=int32 */	/* a */
   vec1 32 ssa_8 = ieq32 ssa_7, ssa_4
   /* succs: block_1 block_10 */
   if ssa_8 {
      block block_1:
      /* preds: block_0 */
      vec1 32 ssa_9 = intrinsic load_uniform (ssa_0) (2, 1, 34) /* base=2 */ /* range=1 */ /* dest_type=int32 */	/* c */
      vec1 32 ssa_10 = ine32 ssa_9, ssa_4
      /* succs: block_2 block_8 */
      if ssa_10 {
         block block_2:
         /* preds: block_1 */
         r3 = mov ssa_4
         r2 = mov ssa_0
         /* succs: block_3 */
         loop {
            block block_3:
            /* preds: block_2 block_6 */
            r4 = i2f32 r2
            vec1 32 ssa_11 = intrinsic load_uniform (ssa_0) (1, 1, 34) /* base=1 */ /* range=1 */ /* dest_type=int32 */	/* b */
            vec1 32 ssa_12 = ine32 ssa_11, ssa_6
            /* succs: block_4 block_5 */
            if ssa_12 {
               block block_4:
               /* preds: block_3 */
               break
               /* succs: block_7 */
            } else {
               block block_5:
               /* preds: block_3 */
               /* succs: block_6 */
            }
            block block_6:
            /* preds: block_5 */
            r5 = iadd r3, ssa_4
            r2 = mov r3
            r3 = mov r5
            /* succs: block_3 */
         }
         block block_7:
         /* preds: block_4 */
         vec1 32 ssa_13 = load_const (0x3f800000 /* 1.000000 */)
         r8 = mov ssa_13
         r7 = mov r8
         r6 = mov ssa_2
         /* succs: block_9 */
      } else {
         block block_8:
         /* preds: block_1 */
         vec1 32 ssa_14 = load_const (0x3f800000 /* 1.000000 */)
         r8 = mov ssa_14
         r7 = mov ssa_0
         r4 = mov r8
         r6 = mov ssa_3
         /* succs: block_9 */
      }
      block block_9:
      /* preds: block_7 block_8 */
      /* succs: block_11 */
   } else {
      block block_10:
      /* preds: block_0 */
      vec1 32 ssa_15 = load_const (0x3f800000 /* 1.000000 */)
      r8 = mov ssa_15
      r7 = mov ssa_0
      r4 = mov r8
      r6 = mov ssa_2
      /* succs: block_11 */
   }
   block block_11:
   /* preds: block_9 block_10 */
   vec1 32 ssa_16 = b32csel r6, r4, ssa_5.x
   vec1 32 ssa_17 = b32csel r6, r7, ssa_5.y
   vec1 32 ssa_18 = b32csel r6, r8, ssa_5.w
   intrinsic store_output (ssa_1, ssa_0) (0, 15, 0, 160, 128) /* base=0 */ /* wrmask=xyzw */ /* component=0 */ /* src_type=float32 */ /* location=0 slots=1 */
   vec1 32 ssa_19 = fsat ssa_16
   vec1 32 ssa_20 = fsat ssa_17
   vec1 32 ssa_21 = fsat ssa_18
   vec4 32 ssa_22 = vec4 ssa_19, ssa_20, ssa_0, ssa_21
   intrinsic store_output (ssa_22, ssa_0) (1, 15, 0, 160, 129) /* base=1 */ /* wrmask=xyzw */ /* component=0 */ /* src_type=float32 */ /* location=1 slots=1 */
   /* succs: block_12 */
   block block_12:
})";

const char *vs_nexted_loop_from_nir_expect =
   R"(VS
CHIPCLASS EVERGREEN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:1 MASK:15
SYSVALUES R1.xyzw
REGISTERS R2.x R3.x R4.x R5.x R6.x R7.x R8.x
SHADER
ALU MOV S9.x@free : I[0] {WL}
ALU MOV S10.x@free : I[-1] {WL}
ALU MOV S11.x@free : I[0] {WL}
ALU MOV S12.x@free : I[1] {WL}
ALU MOV S13.x : I[1.0] {W}
ALU MOV S13.y : I[1.0] {W}
ALU MOV S13.z : I[0] {W}
ALU MOV S13.w : I[1.0] {WL}
ALU MOV S14.x@free : L[0x2] {WL}
ALU MOV S15.x@free : KC0[0].x {WL}
ALU SETE_INT S16.x@free : S15.x@free S12.x@free {WL}
IF (( ALU PRED_SETNE_INT __.x@free : S16.x@free I[0] {LEP} PUSH_BEFORE ))
  ALU MOV S18.x@free : KC0[2].x {WL}
  ALU SETNE_INT S19.x@free : S18.x@free S12.x {WL}
  IF (( ALU PRED_SETNE_INT __.y@free : S19.x@free I[0] {LEP} PUSH_BEFORE ))
    ALU MOV R3.x : S12.x@free {WL}
    ALU MOV R2.x : S9.x@free {WL}
    LOOP_BEGIN
      ALU INT_TO_FLT R4.x : R2.x {WL}
      ALU MOV S21.x@free : KC0[1].x {WL}
      ALU SETNE_INT S22.x@free : S21.x@free S14.x@free {WL}
      IF (( ALU PRED_SETNE_INT __.z@free : S22.x@free I[0] {LEP} PUSH_BEFORE ))
        BREAK
      ENDIF
      ALU ADD_INT R5.x@free : R3.x S12.x@free {WL}
      ALU MOV R2.x : R3.x {WL}
      ALU MOV R3.x : R5.x {WL}
    LOOP_END
    ALU MOV S24.x@free : I[1.0] {WL}
    ALU MOV R8.x : S24.x@free {WL}
    ALU MOV R7.x : R8.x {WL}
    ALU MOV R6.x : S10.x@free {WL}
  ELSE
    ALU MOV S25.x@free : I[1.0] {WL}
    ALU MOV R8.x : S25.x@free {WL}
    ALU MOV R7.x : S9.x {WL}
    ALU MOV R4.x : R8.x {WL}
    ALU MOV R6.x : S11.x@free {WL}
  ENDIF
ELSE
  ALU MOV S26.x@free : I[1.0] {WL}
  ALU MOV R8.x : S26.x@free {WL}
  ALU MOV R7.x : S9.x {WL}
  ALU MOV R4.x : R8.x {WL}
  ALU MOV R6.x : S10.x@free {WL}
ENDIF
ALU CNDE_INT S27.x@free : R6.x S13.x R4.x {WL}
ALU CNDE_INT S28.x@free : R6.x S13.y R7.x {WL}
ALU CNDE_INT S29.x@free : R6.x S13.w R8.x {WL}
EXPORT_DONE POS 0 R1.xyzw
ALU MOV CLAMP S31.x@free : S27.x@free {WL}
ALU MOV CLAMP S32.x@free : S28.x@free {WL}
ALU MOV CLAMP S33.x@free : S29.x@free {WL}
ALU MOV S34.x@group : S31.x@free {W}
ALU MOV S34.y@group : S32.x@free {W}
ALU MOV S34.z@group : S9.x@free {W}
ALU MOV S34.w@group : S33.x@free {WL}
EXPORT_DONE PARAM 0 S34.xyzw
)";

const char *vs_nexted_loop_from_nir_expect_opt =
   R"(
VS
CHIPCLASS EVERGREEN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:1 MASK:15
SYSVALUES R1.xyzw
REGISTERS R2.x@free R3.x@free R4.x@free R5.x@free R6.x@free R7.x@free R8.x@free
SHADER
IF (( ALU PREDE_INT __.x@free : KC0[0].x I[1] {LEP} PUSH_BEFORE ))
  IF (( ALU PRED_SETNE_INT __.y@free : KC0[2].x I[1]  {LEP} PUSH_BEFORE ))
    ALU MOV R3.x : I[1] {WL}
    ALU MOV R2.x : I[0] {WL}
    LOOP_BEGIN
      ALU INT_TO_FLT R4.x : R2.x {WL}
      IF (( ALU PRED_SETNE_INT __.z@free : KC0[1].x L[0x2] {LEP} PUSH_BEFORE ))
        BREAK
      ENDIF
      ALU ADD_INT R5.x : R3.x I[1] {WL}
      ALU MOV R2.x : R3.x {WL}
      ALU MOV R3.x : R5.x {WL}
    LOOP_END
    ALU MOV R8.x : I[1.0] {WL}
    ALU MOV R7.x : I[1.0] {WL}
    ALU MOV R6.x : I[-1] {WL}
  ELSE
    ALU MOV R8.x : I[1.0] {WL}
    ALU MOV R7.x : I[0] {WL}
    ALU MOV R4.x : I[1.0] {WL}
    ALU MOV R6.x : I[0] {WL}
  ENDIF
ELSE
  ALU MOV R8.x : I[1.0] {WL}
  ALU MOV R7.x : I[0] {WL}
  ALU MOV R4.x : I[1.0] {WL}
  ALU MOV R6.x : I[-1] {WL}
ENDIF
ALU CNDE_INT S27.x@free : R6.x I[1.0] R4.x {WL}
ALU CNDE_INT S28.x@free : R6.x I[1.0] R7.x {WL}
ALU CNDE_INT S29.x@free : R6.x I[1.0] R8.x {WL}
EXPORT_DONE POS 0 R1.xyzw
ALU MOV CLAMP S34.x@group : S27.x@free {W}
ALU MOV CLAMP S34.y@group : S28.x@free {W}
ALU MOV CLAMP S34.w@group : S29.x@free {WL}
EXPORT_DONE PARAM 0 S34.xy0w
)";

const char *shader_with_local_array_nir =
   R"(
shader: MESA_SHADER_FRAGMENT
name: GLSL3
inputs: 2
outputs: 1
uniforms: 2
shared: 0
decl_var uniform INTERP_MODE_NONE int index (1, 0, 0)
decl_var uniform INTERP_MODE_NONE float expect (2, 1, 0)
decl_function main (0 params)

impl main {
	decl_var  INTERP_MODE_NONE float[4] m1
	decl_var  INTERP_MODE_NONE float[4] m2
	decl_var  INTERP_MODE_NONE vec4 in@packed:m1[0],m1[1],m1[2],m1[3]-temp
	decl_var  INTERP_MODE_NONE vec4 in@packed:m2[0],m2[1],m2[2],m2[3]-temp
	decl_var  INTERP_MODE_NONE vec4 out@gl_FragColor-temp
	decl_reg vec1 32 r0[4]
	decl_reg vec1 32 r1[4]
	decl_reg vec1 32 r2
	block block_0:
	/* preds: */
	vec2 32 ssa_0 = intrinsic load_barycentric_pixel () (0) /* interp_mode=0 */
	vec1 32 ssa_1 = load_const (0x00000000 /* 0.000000 */)
	vec4 32 ssa_2 = intrinsic load_interpolated_input (ssa_0, ssa_1) (0, 0, 160, 160) /* base=0 */ /* component=0 */ /* location=32 slots=1 */
	vec4 32 ssa_3 = intrinsic load_interpolated_input (ssa_0, ssa_1) (1, 0, 160, 161) /* base=1 */ /* component=0 */ /* location=33 slots=1 */
	vec1 32 ssa_4 = load_const (0x00000004 /* 0.000000 */)
	vec1 32 ssa_5 = load_const (0xfffffffc /* -nan */)
	vec4 32 ssa_6 = load_const (0x00000000 /* 0.000000 */, 0x3f800000 /* 1.000000 */, 0x00000000 /* 0.000000 */, 0x3f800000 /* 1.000000 */)
	vec4 32 ssa_7 = load_const (0x3f800000 /* 1.000000 */, 0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */, 0x3f800000 /* 1.000000 */)
	vec1 32 ssa_8 = mov ssa_2.x
	r0[0] = mov ssa_8
	vec1 32 ssa_9 = mov ssa_2.y
	r0[1] = mov ssa_9
	vec1 32 ssa_10 = mov ssa_2.z
	r0[2] = mov ssa_10
	vec1 32 ssa_11 = mov ssa_2.w
	r0[3] = mov ssa_11
	vec1 32 ssa_12 = mov ssa_3.x
	r1[0] = mov ssa_12
	vec1 32 ssa_13 = mov ssa_3.y
	r1[1] = mov ssa_13
	vec1 32 ssa_14 = mov ssa_3.z
	r1[2] = mov ssa_14
	vec1 32 ssa_15 = mov ssa_3.w
	r1[3] = mov ssa_15
	vec1 32 ssa_16 = intrinsic load_uniform (ssa_1) (0, 1, 34) /* base=0 */ /* range=1 */ /* dest_type=int32 */	/* index */
	vec1 32 ssa_17 = ige32 ssa_16, ssa_4
	/* succs: block_1 block_2 */
	if ssa_17 {
		block block_1:
		/* preds: block_0 */
		vec1 32 ssa_18 = iadd ssa_16, ssa_5
		vec1 32 ssa_19 = load_const (0x00000000 /* 0.000000 */)
		vec1 32 ssa_20 = iadd ssa_19, ssa_18
		r2 = mov r1[0 + ssa_20]
		/* succs: block_3 */
	} else {
		block block_2:
		/* preds: block_0 */
		vec1 32 ssa_21 = load_const (0x00000000 /* 0.000000 */)
		vec1 32 ssa_22 = iadd ssa_21, ssa_16
		r2 = mov r0[0 + ssa_22]
		/* succs: block_3 */
	}
	block block_3:
	/* preds: block_1 block_2 */
	vec1 32 ssa_23 = intrinsic load_uniform (ssa_1) (1, 1, 160) /* base=1 */ /* range=1 */ /* dest_type=float32 */	/* expect */
	vec1 32 ssa_24 = feq32 r2, ssa_23
	vec1 32 ssa_25 = fneu32 r2, ssa_23
	vec1 32 ssa_26 = b2f32 ssa_25
	vec1 32 ssa_27 = b2f32 ssa_24
	vec2 32 ssa_28 = b32csel ssa_24.xx, ssa_6.zw, ssa_7.zw
	vec4 32 ssa_29 = vec4 ssa_26, ssa_27, ssa_28.x, ssa_28.y
	intrinsic store_output (ssa_29, ssa_1) (0, 15, 0, 160, 130) /* base=0 */ /* wrmask=xyzw */ /* component=0 */ /* src_type=float32 */ /* location=2 slots=1 */
	/* succs: block_4 */
	block block_4:
}
)";

const char *shader_with_local_array_expect =
   R"(FS
CHIPCLASS EVERGREEN
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
#PROP RAT_BASE:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
INPUT LOC:1 VARYING_SLOT:33 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.x@fully R0.y@fully R1.x
ARRAYS A1[4].x A1[4].y
SHADER
ALU MOV S6.x@free : I[0] {WL}
ALU_GROUP_BEGIN
  ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
  ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
  ALU INTERP_ZW S7.z@chan : R0.y@fully Param0.z {W} VEC_210
  ALU INTERP_ZW S7.w@chan : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU INTERP_XY S7.x@chan : R0.y@fully Param0.x {W} VEC_210
  ALU INTERP_XY S7.y@chan : R0.x@fully Param0.y {W} VEC_210
  ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
  ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU INTERP_ZW __.x@chan : R0.y@fully Param1.x {} VEC_210
  ALU INTERP_ZW __.y@chan : R0.x@fully Param1.y {} VEC_210
  ALU INTERP_ZW S8.z@chan : R0.y@fully Param1.z {W} VEC_210
  ALU INTERP_ZW S8.w@chan : R0.x@fully Param1.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU INTERP_XY S8.x@chan : R0.y@fully Param1.x {W} VEC_210
  ALU INTERP_XY S8.y@chan : R0.x@fully Param1.y {W} VEC_210
  ALU INTERP_XY __.z@chan : R0.y@fully Param1.z {} VEC_210
  ALU INTERP_XY __.w@chan : R0.x@fully Param1.w {L} VEC_210
ALU_GROUP_END
ALU MOV S9.x@free : L[0x4] {WL}
ALU MOV S10.x@free : L[0xfffffffc] {WL}
ALU MOV S11.x : I[0] {W}
ALU MOV S11.y : I[1.0] {W}
ALU MOV S11.z : I[0] {W}
ALU MOV S11.w : I[1.0] {WL}
ALU MOV S12.x : I[1.0] {W}
ALU MOV S12.y : I[0] {W}
ALU MOV S12.z : I[0] {W}
ALU MOV S12.w : I[1.0] {WL}
ALU MOV S13.x@free : S7.x@chan {WL}
ALU MOV A1[0].x : S13.x@free {WL}
ALU MOV S14.x@free : S7.y@chan {WL}
ALU MOV A1[1].x : S14.x@free {WL}
ALU MOV S15.x@free : S7.z@chan {WL}
ALU MOV A1[2].x : S15.x@free {WL}
ALU MOV S16.x@free : S7.w@chan {WL}
ALU MOV A1[3].x : S16.x@free {WL}
ALU MOV S17.x@free : S8.x@chan {WL}
ALU MOV A1[0].y : S17.x@free {WL}
ALU MOV S18.x@free : S8.y@chan {WL}
ALU MOV A1[1].y : S18.x@free {WL}
ALU MOV S19.x@free : S8.z@chan {WL}
ALU MOV A1[2].y : S19.x@free {WL}
ALU MOV S20.x@free : S8.w@chan {WL}
ALU MOV A1[3].y : S20.x@free {WL}
ALU MOV S21.x@free : KC0[0].x {WL}
ALU SETGE_INT S22.x@free : S21.x@free S9.x@free {WL}
IF (( ALU PRED_SETNE_INT __.x@free : S22.x@free I[0] {LEP} PUSH_BEFORE ))
  ALU ADD_INT S24.x@free : S21.x@free S10.x@free {WL}
  ALU MOV S25.x@free : I[0] {WL}
  ALU ADD_INT S26.x@free : S25.x@free S24.x@free {WL}
  ALU MOV R5.x@free : A1[S26.x@free].y {WL}
ELSE
  ALU MOV S27.x@free : I[0] {WL}
  ALU ADD_INT S28.x@free : S27.x@free S21.x@free {WL}
  ALU MOV R5.x@free : A1[S28.x@free].x {WL}
ENDIF
ALU MOV S29.x@free : KC0[1].x {WL}
ALU SETE_DX10 S30.x@free : R5.x@free S29.x@free {WL}
ALU SETNE_DX10 S31.x@free : R5.x@free S29.x@free {WL}
ALU AND_INT S32.x@free : S31.x@free I[1.0] {WL}
ALU AND_INT S33.x@free : S30.x@free I[1.0] {WL}
ALU CNDE_INT S34.x : S30.x@free S12.z S11.z {W}
ALU CNDE_INT S34.y : S30.x@free S12.w S11.w {WL}
ALU MOV S35.x@group : S32.x@free {W}
ALU MOV S35.y@group : S33.x@free {W}
ALU MOV S35.z@group : S34.x {W}
ALU MOV S35.w@group : S34.y {WL}
EXPORT_DONE PIXEL 0 S35.xyzw)";

const char *test_schedule_group =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S0.x : I[0] {WL}
ALU MOV S1.x : I[1.0] {WL}
ALU MOV S2.x : KC0[0].x {W}
ALU MOV S2.y : KC0[0].y {WL}
ALU MOV S3.x : KC0[2].x {W}
ALU MOV S3.y : KC0[2].y {WL}
ALU ADD S4.x : |S2.x| -S3.x {W}
ALU ADD S4.y : |S2.y| -S3.y {WL}
ALU DOT4_IEEE S5.x : S4.x S4.x + S4.y S4.y + I[0] I[0] + I[0] I[0] {WL}
ALU SQRT_IEEE S6.x : S5.x {WL}
ALU MOV S7.x : KC0[1].x {WL}
ALU SETGE_DX10 S8.x : S7.x S6.x {WL}
ALU NOT_INT S9.x : S8.x {WL}
ALU AND_INT S10.x : S9.x I[1.0] {WL}
ALU AND_INT S11.x : S8.x I[1.0] {WL}
ALU MOV S12.x@group : S10.x {W}
ALU MOV S12.y@group : S11.x {W}
ALU MOV S12.z@group : S0.x {W}
ALU MOV S12.w@group : S1.x {WL}
EXPORT_DONE PIXEL 0 S12.xyzw
)";

const char *test_schedule_group_expect =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU ADD S4.x@chan : |KC0[0].x| -KC0[2].x {W}
  ALU ADD S4.y@chan : |KC0[0].y| -KC0[2].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU DOT4_IEEE S5.x@chan : S4.x@chan S4.x@chan {W}
  ALU DOT4_IEEE __.y@chan : S4.y@chan S4.y@chan {}
  ALU DOT4_IEEE __.z@chan : I[0] I[0] {}
  ALU DOT4_IEEE __.w@chan : I[0] I[0] {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SQRT_IEEE S6.x : S5.x@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETGE_DX10 S8.x : KC0[1].x S6.x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU NOT_INT S9.x : S8.x {W}
  ALU AND_INT S12.y@group : S8.x I[1.0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU AND_INT S12.x@group : S9.x I[1.0] {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S12.xy01
BLOCK_END
)";

const char *shader_with_bany_nir =
   R"(shader: MESA_SHADER_FRAGMENT
source_sha1: {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000}
name: GLSL3
inputs: 0
outputs: 1
uniforms: 8
shared: 0
ray queries: 0
decl_var uniform INTERP_MODE_NONE mat4 arg0 (0, 0, 0)
decl_var uniform INTERP_MODE_NONE mat4 arg1 (1, 4, 0)
decl_function main (0 params)

impl main {
	decl_var  INTERP_MODE_NONE vec4 out@gl_FragColor-temp
	block block_0:
	/* preds: */
	vec1 32 ssa_0 = load_const (0x00000000 /* 0.000000 */)
	vec1 32 ssa_1 = load_const (0x00000001 /* 0.000000 */)
	vec1 32 ssa_2 = load_const (0x00000002 /* 0.000000 */)
	vec1 32 ssa_3 = load_const (0x00000003 /* 0.000000 */)
	vec4 32 ssa_4 = intrinsic load_uniform (ssa_0) (4, 4, 160) /* base=4 */ /* range=4 */ /* dest_type=float32 */	/* arg1 */
	vec4 32 ssa_5 = intrinsic load_uniform (ssa_0) (0, 4, 160) /* base=0 */ /* range=4 */ /* dest_type=float32 */	/* arg0 */
	vec1 32 ssa_6 = b32any_fnequal4 ssa_4, ssa_5
	vec4 32 ssa_7 = intrinsic load_uniform (ssa_1) (4, 4, 160) /* base=4 */ /* range=4 */ /* dest_type=float32 */	/* arg1 */
	vec4 32 ssa_8 = intrinsic load_uniform (ssa_1) (0, 4, 160) /* base=0 */ /* range=4 */ /* dest_type=float32 */	/* arg0 */
	vec1 32 ssa_9 = b32any_fnequal4 ssa_7, ssa_8
	vec4 32 ssa_10 = intrinsic load_uniform (ssa_2) (4, 4, 160) /* base=4 */ /* range=4 */ /* dest_type=float32 */	/* arg1 */
	vec4 32 ssa_11 = intrinsic load_uniform (ssa_2) (0, 4, 160) /* base=0 */ /* range=4 */ /* dest_type=float32 */	/* arg0 */
	vec1 32 ssa_12 = b32any_fnequal4 ssa_10, ssa_11
	vec4 32 ssa_13 = intrinsic load_uniform (ssa_3) (4, 4, 160) /* base=4 */ /* range=4 */ /* dest_type=float32 */	/* arg1 */
	vec4 32 ssa_14 = intrinsic load_uniform (ssa_3) (0, 4, 160) /* base=0 */ /* range=4 */ /* dest_type=float32 */	/* arg0 */
	vec1 32 ssa_15 = b32any_fnequal4 ssa_13, ssa_14
	vec4 32 ssa_16 = vec4 ssa_6, ssa_9, ssa_12, ssa_15
	vec4 32 ssa_17 = load_const (0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */, 0x00000000 /* 0.000000 */)
	vec1 32 ssa_18 = b32any_inequal4 ssa_16, ssa_17
	vec1 32 ssa_19 = inot ssa_18
	vec1 32 ssa_20 = b2f32 ssa_19
	vec4 32 ssa_21 = vec4 ssa_20, ssa_0, ssa_0, ssa_0
	intrinsic store_output (ssa_21, ssa_0) (0, 15, 0, 160, 130) /* base=0 */ /* wrmask=xyzw */ /* component=0 */ /* src_type=float32 */ /* location=2 slots=1 */
	/* succs: block_1 */
	block block_1:
})";

const char *shader_with_bany_expect_eg =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S0.x@free : I[0] {WL}
ALU MOV S1.x@free : I[1] {WL}
ALU MOV S2.x@free : L[0x2] {WL}
ALU MOV S3.x@free : L[0x3] {WL}
ALU MOV S4.x : KC0[4].x {W}
ALU MOV S4.y : KC0[4].y {W}
ALU MOV S4.z : KC0[4].z {W}
ALU MOV S4.w : KC0[4].w {WL}
ALU MOV S5.x : KC0[0].x {W}
ALU MOV S5.y : KC0[0].y {W}
ALU MOV S5.z : KC0[0].z {W}
ALU MOV S5.w : KC0[0].w {WL}
ALU SETNE S6.x@group : S4.x S5.x {W}
ALU SETNE S6.y@group : S4.y S5.y {W}
ALU SETNE S6.z@group : S4.z S5.z {W}
ALU SETNE S6.w@group : S4.w S5.w {WL}
ALU MAX4 S7.x@free : S6.x@group + S6.y@group + S6.z@group + S6.w@group {WL}
ALU SETE_DX10 S8.x@free : S7.x@free I[1.0] {WL}
ALU MOV S9.x : KC0[5].x {W}
ALU MOV S9.y : KC0[5].y {W}
ALU MOV S9.z : KC0[5].z {W}
ALU MOV S9.w : KC0[5].w {WL}
ALU MOV S10.x : KC0[1].x {W}
ALU MOV S10.y : KC0[1].y {W}
ALU MOV S10.z : KC0[1].z {W}
ALU MOV S10.w : KC0[1].w {WL}
ALU SETNE S11.x@group : S9.x S10.x {W}
ALU SETNE S11.y@group : S9.y S10.y {W}
ALU SETNE S11.z@group : S9.z S10.z {W}
ALU SETNE S11.w@group : S9.w S10.w {WL}
ALU MAX4 S12.y@free : S11.x@group + S11.y@group + S11.z@group + S11.w@group {WL}
ALU SETE_DX10 S13.x@free : S12.y@free I[1.0] {WL}
ALU MOV S14.x : KC0[6].x {W}
ALU MOV S14.y : KC0[6].y {W}
ALU MOV S14.z : KC0[6].z {W}
ALU MOV S14.w : KC0[6].w {WL}
ALU MOV S15.x : KC0[2].x {W}
ALU MOV S15.y : KC0[2].y {W}
ALU MOV S15.z : KC0[2].z {W}
ALU MOV S15.w : KC0[2].w {WL}
ALU SETNE S16.x@group : S14.x S15.x {W}
ALU SETNE S16.y@group : S14.y S15.y {W}
ALU SETNE S16.z@group : S14.z S15.z {W}
ALU SETNE S16.w@group : S14.w S15.w {WL}
ALU MAX4 S17.z@free : S16.x@group + S16.y@group + S16.z@group + S16.w@group {WL}
ALU SETE_DX10 S18.x@free : S17.z@free I[1.0] {WL}
ALU MOV S19.x : KC0[7].x {W}
ALU MOV S19.y : KC0[7].y {W}
ALU MOV S19.z : KC0[7].z {W}
ALU MOV S19.w : KC0[7].w {WL}
ALU MOV S20.x : KC0[3].x {W}
ALU MOV S20.y : KC0[3].y {W}
ALU MOV S20.z : KC0[3].z {W}
ALU MOV S20.w : KC0[3].w {WL}
ALU SETNE S21.x@group : S19.x S20.x {W}
ALU SETNE S21.y@group : S19.y S20.y {W}
ALU SETNE S21.z@group : S19.z S20.z {W}
ALU SETNE S21.w@group : S19.w S20.w {WL}
ALU MAX4 S22.w@free : S21.x@group + S21.y@group + S21.z@group + S21.w@group {WL}
ALU SETE_DX10 S23.x@free : S22.w@free I[1.0] {WL}
ALU MOV S24.x : S8.x@free {W}
ALU MOV S24.y : S13.x@free {W}
ALU MOV S24.z : S18.x@free {W}
ALU MOV S24.w : S23.x@free {WL}
ALU MOV S25.x : I[0] {W}
ALU MOV S25.y : I[0] {W}
ALU MOV S25.z : I[0] {W}
ALU MOV S25.w : I[0] {WL}
ALU SETNE_INT S27.x@free : S24.x S25.x {W}
ALU SETNE_INT S28.y@free : S24.y S25.y {W}
ALU SETNE_INT S29.z@free : S24.z S25.z {W}
ALU SETNE_INT S30.w@free : S24.w S25.w {WL}
ALU OR_INT S31.x@free : S27.x@free S28.y@free {W}
ALU OR_INT S32.y@free : S29.z@free S30.w@free {WL}
ALU OR_INT S26.x@free : S31.x@free S32.y@free {WL}
ALU NOT_INT S33.x@free : S26.x@free {WL}
ALU AND_INT S34.x@free : S33.x@free I[1.0] {WL}
ALU MOV S35.x@group : S34.x@free {W}
ALU MOV S35.y@group : S0.x@free {W}
ALU MOV S35.z@group : S0.x@free {W}
ALU MOV S35.w@group : S0.x@free {WL}
EXPORT_DONE PIXEL 0 S35.xyzw
)";

const char *shader_with_bany_expect_opt_sched_eg =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU SETNE S6.x@chgr : KC0[4].x KC0[0].x {W}
  ALU SETNE S6.y@chgr : KC0[4].y KC0[0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETNE S6.z@chgr : KC0[4].z KC0[0].z {W}
  ALU SETNE S6.w@chgr : KC0[4].w KC0[0].w {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MAX4 S7.x@chan : S6.x@chgr {W}
  ALU MAX4 __.y@chan : S6.y@chgr {}
  ALU MAX4 __.z@chan : S6.z@chgr {}
  ALU MAX4 __.w@chan : S6.w@chgr {}
  ALU SETNE S11.x@chgr : KC0[5].x KC0[1].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETE_DX10 S8.x@free : S7.x@chan I[1.0] {W}
  ALU SETNE S11.y@chgr : KC0[5].y KC0[1].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETNE_INT S27.x@chan : S8.x@free I[0] {W}
  ALU SETNE S11.z@chgr : KC0[5].z KC0[1].z {W}
  ALU SETNE S11.w@chgr : KC0[5].w KC0[1].w {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MAX4 __.x@chan : S11.x@chgr {}
  ALU MAX4 S12.y@chan : S11.y@chgr {W}
  ALU MAX4 __.z@chan : S11.z@chgr {}
  ALU MAX4 __.w@chan : S11.w@chgr {}
  ALU SETNE S16.x@chgr : KC0[6].x KC0[2].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETE_DX10 S13.x@free : S12.y@chan I[1.0] {W}
  ALU SETNE S16.y@chgr : KC0[6].y KC0[2].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETNE_INT S28.y@chan : S13.x@free I[0] {W}
  ALU SETNE S16.z@chgr : KC0[6].z KC0[2].z {W}
  ALU SETNE S16.w@chgr : KC0[6].w KC0[2].w {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MAX4 __.x@chan : S16.x@chgr {}
  ALU MAX4 __.y@chan : S16.y@chgr {}
  ALU MAX4 S17.z@chan : S16.z@chgr {W}
  ALU MAX4 __.w@chan : S16.w@chgr {}
  ALU SETNE S21.x@chgr : KC0[7].x KC0[3].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU OR_INT S31.x@chan : S27.x@chan S28.y@chan {W}
  ALU SETNE S21.y@chgr : KC0[7].y KC0[3].y {W}
  ALU SETE_DX10 S18.z@chan : S17.z@chan I[1.0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETNE_INT S29.x@chan : S18.z@chan I[0] {W}
  ALU SETNE S21.z@chgr : KC0[7].z KC0[3].z {W}
  ALU SETNE S21.w@chgr : KC0[7].w KC0[3].w {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MAX4 __.x@chan : S21.x@chgr {}
  ALU MAX4 __.y@chan : S21.y@chgr {}
  ALU MAX4 __.z@chan : S21.z@chgr {}
  ALU MAX4 S22.w@chan : S21.w@chgr {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETE_DX10 S23.x@free : S22.w@chan I[1.0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETNE_INT S30.w@chan : S23.x@free I[0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU OR_INT S32.y@chan : S29.x@chan S30.w@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU OR_INT S26.x@chan : S31.x@chan S32.y@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU NOT_INT S33.x@free : S26.x@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU AND_INT S35.x@group : S33.x@free I[1.0] {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S35.x000
BLOCK_END
)";

const char *shader_copy_prop_dont_kill_double_use =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S0.x : I[0] {WL}
ALU MOV S1.x : I[1] {WL}
ALU MOV S2.x : I[1.0] {WL}
ALU MOV S3.x : KC0[2].x {W}
ALU MOV S3.y : KC0[2].y {WL}
ALU MOV S4.x : KC0[0].x {W}
ALU MOV S4.y : KC0[0].y {WL}
ALU SETNE_DX10 S5.x : S3.y S4.y {W}
ALU SETNE_DX10 S5.y : S3.x S4.x {WL}
ALU OR_INT S6.x : S5.x S5.y {WL}
ALU MOV S7.x : KC0[3].x {W}
ALU MOV S7.y : KC0[3].y {WL}
ALU MOV S8.x : KC0[1].x {W}
ALU MOV S8.y : KC0[1].y {WL}
ALU SETNE_DX10 S9.x : S7.y S8.y {W}
ALU SETNE_DX10 S9.y : S7.x S8.x {WL}
ALU OR_INT S10.x : S9.x S9.y {WL}
ALU OR_INT S11.x : S10.x S6.x {WL}
ALU NOT_INT S12.x : S11.x {WL}
ALU AND_INT S13.x : S12.x I[1.0] {WL}
ALU AND_INT S14.x : S11.x I[1.0] {WL}
ALU MOV S15.x@group : S13.x {W}
ALU MOV S15.y@group : S13.x {W}
ALU MOV S15.z@group : S14.x {W}
ALU MOV S15.w@group : S2.x {WL}
EXPORT_DONE PIXEL 0 S15.xyzw
)";

const char *shader_copy_prop_dont_kill_double_use_expect =
   R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU SETNE_DX10 S5.x : KC0[2].y KC0[0].y {W}
  ALU SETNE_DX10 S5.y : KC0[2].x KC0[0].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SETNE_DX10 S9.x : KC0[3].y KC0[1].y {W}
  ALU SETNE_DX10 S9.y : KC0[3].x KC0[1].x {W}
  ALU OR_INT S6.x : S5.x S5.y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU OR_INT S10.x : S9.x S9.y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU OR_INT S11.x : S10.x S6.x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU NOT_INT S12.x : S11.x {W}
  ALU AND_INT S15.z@group : S11.x I[1.0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU AND_INT S13.x : S12.x I[1.0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV S15.x@group : S13.x {W}
  ALU MOV S15.y@group : S13.x {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S15.xyz1
BLOCK_END
)";

const char *shader_with_dest_array =
   R"(VS
CHIPCLASS EVERGREEN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
OUTPUT LOC:2 VARYING_SLOT:33 MASK:15
OUTPUT LOC:3 VARYING_SLOT:34 MASK:15
OUTPUT LOC:4 VARYING_SLOT:35 MASK:15
REGISTERS R1.xyzw
ARRAYS A2[4].xy A2[4].zw
SHADER
ALU MOV S6.x : I[0] {WL}
ALU MOV S7.x : I[1] {WL}
ALU MOV S8.x : L[0x2] {WL}
ALU MOV S9.x : L[0x3] {WL}
ALU MOV S10.x : L[0x4] {WL}
ALU MOV S11.x : L[0xfffffffc] {WL}
ALU MOV S12.x : KC0[1].x {W}
ALU MOV S12.y : KC0[1].y {W}
ALU MOV S12.z : KC0[1].z {W}
ALU MOV S12.w : KC0[1].w {WL}
ALU MOV S13.x : KC0[2].x {W}
ALU MOV S13.y : KC0[2].y {W}
ALU MOV S13.z : KC0[2].z {W}
ALU MOV S13.w : KC0[2].w {WL}
ALU MUL_IEEE S14.x : S13.x R1.y@fully {W}
ALU MUL_IEEE S14.y : S13.y R1.y@fully {W}
ALU MUL_IEEE S14.z : S13.z R1.y@fully {W}
ALU MUL_IEEE S14.w : S13.w R1.y@fully {WL}
ALU MULADD_IEEE S15.x : S12.x R1.x@fully S14.x {W}
ALU MULADD_IEEE S15.y : S12.y R1.x@fully S14.y {W}
ALU MULADD_IEEE S15.z : S12.z R1.x@fully S14.z {W}
ALU MULADD_IEEE S15.w : S12.w R1.x@fully S14.w {WL}
ALU MOV S16.x : KC0[3].x {W}
ALU MOV S16.y : KC0[3].y {W}
ALU MOV S16.z : KC0[3].z {W}
ALU MOV S16.w : KC0[3].w {WL}
ALU MULADD_IEEE S17.x : S16.x R1.z@fully S15.x {W}
ALU MULADD_IEEE S17.y : S16.y R1.z@fully S15.y {W}
ALU MULADD_IEEE S17.z : S16.z R1.z@fully S15.z {W}
ALU MULADD_IEEE S17.w : S16.w R1.z@fully S15.w {WL}
ALU MOV S18.x : KC0[4].x {W}
ALU MOV S18.y : KC0[4].y {W}
ALU MOV S18.z : KC0[4].z {W}
ALU MOV S18.w : KC0[4].w {WL}
ALU MULADD_IEEE S19.x@group : S18.x R1.w@fully S17.x {W}
ALU MULADD_IEEE S19.y@group : S18.y R1.w@fully S17.y {W}
ALU MULADD_IEEE S19.z@group : S18.z R1.w@fully S17.z {W}
ALU MULADD_IEEE S19.w@group : S18.w R1.w@fully S17.w {WL}
ALU MOV S20.x : I[1.0] {W}
ALU MOV S20.y : L[0x3f8ccccd] {WL}
ALU MOV A2[0].x : S20.x {W}
ALU MOV A2[0].y : S20.y {WL}
ALU MOV S21.x : L[0x40000000] {W}
ALU MOV S21.y : L[0x40066666] {WL}
ALU MOV A2[1].x : S21.x {W}
ALU MOV A2[1].y : S21.y {WL}
ALU MOV S22.x : L[0x40400000] {W}
ALU MOV S22.y : L[0x40466666] {WL}
ALU MOV A2[2].x : S22.x {W}
ALU MOV A2[2].y : S22.y {WL}
ALU MOV S23.x : L[0x40800000] {W}
ALU MOV S23.y : L[0x40833333] {WL}
ALU MOV A2[3].x : S23.x {W}
ALU MOV A2[3].y : S23.y {WL}
ALU MOV S24.x : L[0x40a00000] {W}
ALU MOV S24.y : L[0x40a33333] {WL}
ALU MOV A2[0].z : S24.x {W}
ALU MOV A2[0].w : S24.y {WL}
ALU MOV S25.x : L[0x40c00000] {W}
ALU MOV S25.y : L[0x40c33333] {WL}
ALU MOV A2[1].z : S25.x {W}
ALU MOV A2[1].w : S25.y {WL}
ALU MOV S26.x : L[0x40e00000] {W}
ALU MOV S26.y : L[0x40e33333] {WL}
ALU MOV A2[2].z : S26.x {W}
ALU MOV A2[2].w : S26.y {WL}
ALU MOV S27.x : L[0x41000000] {W}
ALU MOV S27.y : L[0x4101999a] {WL}
ALU MOV A2[3].z : S27.x {W}
ALU MOV A2[3].w : S27.y {WL}
ALU MOV S28.x : KC0[0].x {WL}
ALU SETGE_INT S29.x : S28.x S10.x {WL}
IF (( ALU PRED_SETNE_INT __.x@free : S29.x I[0] {LEP} PUSH_BEFORE ))
  ALU ADD_INT S31.x : S28.x S11.x {WL}
  ALU MOV S32.x : I[0] {W}
  ALU MOV S32.y : L[0x3dcccccd] {WL}
  ALU MOV S33.x : I[0] {WL}
  ALU ADD_INT S34.x : S33.x S31.x {WL}
  ALU MOV A2[S34.x].z : S32.x {W}
  ALU MOV A2[S34.x].w : S32.y {WL}
ELSE
  ALU MOV S35.x : I[0] {W}
  ALU MOV S35.y : L[0x3dcccccd] {WL}
  ALU MOV S36.x : I[0] {WL}
  ALU ADD_INT S37.x : S36.x S28.x {WL}
  ALU MOV A2[S37.x].x : S35.x {W}
  ALU MOV A2[S37.x].y : S35.y {WL}
ENDIF
ALU MOV S38.x : A2[0].x {W}
ALU MOV S38.y : A2[0].y {WL}
ALU MOV S39.x : A2[1].x {W}
ALU MOV S39.y : A2[1].y {WL}
ALU MOV S40.x : A2[2].x {W}
ALU MOV S40.y : A2[2].y {WL}
ALU MOV S41.x : A2[3].x {W}
ALU MOV S41.y : A2[3].y {WL}
ALU MOV S42.x : A2[0].z {W}
ALU MOV S42.y : A2[0].w {WL}
ALU MOV S43.x : A2[1].z {W}
ALU MOV S43.y : A2[1].w {WL}
ALU MOV S44.x : A2[2].z {W}
ALU MOV S44.y : A2[2].w {WL}
ALU MOV S45.x : A2[3].z {W}
ALU MOV S45.y : A2[3].w {WL}
EXPORT_DONE POS 0 S19.xyzw
ALU MOV S46.x@group : S38.x {W}
ALU MOV S46.y@group : S38.y {W}
ALU MOV S46.z@group : S39.x {W}
ALU MOV S46.w@group : S39.y {WL}
EXPORT PARAM 0 S46.xyzw
ALU MOV S47.x@group : S40.x {W}
ALU MOV S47.y@group : S40.y {W}
ALU MOV S47.z@group : S41.x {W}
ALU MOV S47.w@group : S41.y {WL}
EXPORT PARAM 1 S47.xyzw
ALU MOV S48.x@group : S42.x {W}
ALU MOV S48.y@group : S42.y {W}
ALU MOV S48.z@group : S43.x {W}
ALU MOV S48.w@group : S43.y {WL}
EXPORT PARAM 2 S48.xyzw
ALU MOV S49.x@group : S44.x {W}
ALU MOV S49.y@group : S44.y {W}
ALU MOV S49.z@group : S45.x {W}
ALU MOV S49.w@group : S45.y {WL}
EXPORT_DONE PARAM 3 S49.xyzw
)";

const char *shader_with_dest_array_opt_expect =
   R"(VS
CHIPCLASS EVERGREEN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
OUTPUT LOC:2 VARYING_SLOT:33 MASK:15
OUTPUT LOC:3 VARYING_SLOT:34 MASK:15
OUTPUT LOC:4 VARYING_SLOT:35 MASK:15
REGISTERS R1.xyzw
ARRAYS A2[4].xy A2[4].zw
SHADER
ALU MUL_IEEE S14.x : KC0[2].x R1.y@fully {W}
ALU MUL_IEEE S14.y : KC0[2].y R1.y@fully {W}
ALU MUL_IEEE S14.z : KC0[2].z R1.y@fully {W}
ALU MUL_IEEE S14.w : KC0[2].w R1.y@fully {WL}
ALU MULADD_IEEE S15.x : KC0[1].x R1.x@fully S14.x {W}
ALU MULADD_IEEE S15.y : KC0[1].y R1.x@fully S14.y {W}
ALU MULADD_IEEE S15.z : KC0[1].z R1.x@fully S14.z {W}
ALU MULADD_IEEE S15.w : KC0[1].w R1.x@fully S14.w {WL}
ALU MULADD_IEEE S17.x : KC0[3].x R1.z@fully S15.x {W}
ALU MULADD_IEEE S17.y : KC0[3].y R1.z@fully S15.y {W}
ALU MULADD_IEEE S17.z : KC0[3].z R1.z@fully S15.z {W}
ALU MULADD_IEEE S17.w : KC0[3].w R1.z@fully S15.w {WL}
ALU MULADD_IEEE S19.x@group : KC0[4].x R1.w@fully S17.x {W}
ALU MULADD_IEEE S19.y@group : KC0[4].y R1.w@fully S17.y {W}
ALU MULADD_IEEE S19.z@group : KC0[4].z R1.w@fully S17.z {W}
ALU MULADD_IEEE S19.w@group : KC0[4].w R1.w@fully S17.w {WL}
ALU MOV A2[0].x : I[1.0] {W}
ALU MOV A2[0].y : L[0x3f8ccccd] {WL}
ALU MOV A2[1].x : L[0x40000000] {W}
ALU MOV A2[1].y : L[0x40066666] {WL}
ALU MOV A2[2].x : L[0x40400000] {W}
ALU MOV A2[2].y : L[0x40466666] {WL}
ALU MOV A2[3].x : L[0x40800000] {W}
ALU MOV A2[3].y : L[0x40833333] {WL}
ALU MOV A2[0].z : L[0x40a00000] {W}
ALU MOV A2[0].w : L[0x40a33333] {WL}
ALU MOV A2[1].z : L[0x40c00000] {W}
ALU MOV A2[1].w : L[0x40c33333] {WL}
ALU MOV A2[2].z : L[0x40e00000] {W}
ALU MOV A2[2].w : L[0x40e33333] {WL}
ALU MOV A2[3].z : L[0x41000000] {W}
ALU MOV A2[3].w : L[0x4101999a] {WL}
IF (( ALU PRED_SETGE_INT __.x@free : KC0[0].x L[0x4] {LEP} PUSH_BEFORE ))
  ALU ADD_INT S34.x : KC0[0].x L[0xfffffffc]  {WL}
  ALU MOV A2[S34.x].z : I[0] {W}
  ALU MOV A2[S34.x].w : L[0x3dcccccd] {WL}
ELSE
     ALU MOV S37.x : KC0[0].x {WL}
     ALU MOV A2[S37.x].x : I[0] {W}
     ALU MOV A2[S37.x].y : L[0x3dcccccd] {WL}
ENDIF
EXPORT_DONE POS 0 S19.xyzw
ALU MOV S46.x@group{s} : A2[0].x {W}
ALU MOV S46.y@group{s} : A2[0].y {W}
ALU MOV S46.z@group{s} : A2[1].x {W}
ALU MOV S46.w@group{s} : A2[1].y {WL}
EXPORT PARAM 0 S46.xyzw
ALU MOV S47.x@group{s} : A2[2].x {W}
ALU MOV S47.y@group{s} : A2[2].y {W}
ALU MOV S47.z@group{s} : A2[3].x {W}
ALU MOV S47.w@group{s} : A2[3].y {WL}
EXPORT PARAM 1 S47.xyzw
ALU MOV S48.x@group{s} : A2[0].z {W}
ALU MOV S48.y@group{s} : A2[0].w {W}
ALU MOV S48.z@group{s} : A2[1].z {W}
ALU MOV S48.w@group{s} : A2[1].w {WL}
EXPORT PARAM 2 S48.xyzw
ALU MOV S49.x@group{s} : A2[2].z {W}
ALU MOV S49.y@group{s} : A2[2].w {W}
ALU MOV S49.z@group{s} : A2[3].z {W}
ALU MOV S49.w@group{s} : A2[3].w {WL}
EXPORT_DONE PARAM 3 S49.xyzw
)";

const char *shader_with_dest_array_opt_scheduled =
   R"(VS
CHIPCLASS EVERGREEN
INPUT LOC:0
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
OUTPUT LOC:2 VARYING_SLOT:33 MASK:15
OUTPUT LOC:3 VARYING_SLOT:34 MASK:15
OUTPUT LOC:4 VARYING_SLOT:35 MASK:15
REGISTERS R1.xyzw
ARRAYS A2[4].xy A2[4].zw
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV A2[0].x : I[1.0] {W}
  ALU MOV A2[0].y : L[0x3f8ccccd] {W}
  ALU MOV A2[0].z : L[0x40a00000] {W}
  ALU MOV A2[0].w : L[0x40a33333] {W}
  ALU MOV A2[1].x : L[0x40000000] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV A2[2].x : L[0x40400000] {W}
  ALU MOV A2[1].y : L[0x40066666] {W}
  ALU MOV A2[1].z : L[0x40c00000] {W}
  ALU MOV A2[1].w : L[0x40c33333] {W}
  ALU MUL_IEEE S14.x : KC0[2].x R1.y@fully {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV A2[3].x : L[0x40800000] {W}
  ALU MOV A2[2].y : L[0x40466666] {W}
  ALU MOV A2[2].z : L[0x40e00000] {W}
  ALU MOV A2[2].w : L[0x40e33333] {W}
  ALU MULADD_IEEE S15.x : KC0[1].x R1.x@fully S14.x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MULADD_IEEE S17.x : KC0[3].x R1.z@fully S15.x {W}
  ALU MOV A2[3].y : L[0x40833333] {W}
  ALU MOV A2[3].z : L[0x41000000] {W}
  ALU MOV A2[3].w : L[0x4101999a] {W}
  ALU MUL_IEEE S14.y : KC0[2].y R1.y@fully {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MULADD_IEEE S19.x@group : KC0[4].x R1.w@fully S17.x {W}
  ALU MULADD_IEEE S15.y : KC0[1].y R1.x@fully S14.y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MULADD_IEEE S17.y : KC0[3].y R1.z@fully S15.y {W}
  ALU MUL_IEEE S14.z : KC0[2].z R1.y@fully {W}
  ALU MUL_IEEE S14.w : KC0[2].w R1.y@fully {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MULADD_IEEE S19.y@group : KC0[4].y R1.w@fully S17.y {W}
  ALU MULADD_IEEE S15.z : KC0[1].z R1.x@fully S14.z {W}
  ALU MULADD_IEEE S15.w : KC0[1].w R1.x@fully S14.w {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MULADD_IEEE S17.z : KC0[3].z R1.z@fully S15.z {W}
  ALU MULADD_IEEE S17.w : KC0[3].w R1.z@fully S15.w {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MULADD_IEEE S19.z@group : KC0[4].z R1.w@fully S17.z {W}
  ALU MULADD_IEEE S19.w@group : KC0[4].w R1.w@fully S17.w {WL}
ALU_GROUP_END
IF (( ALU PRED_SETGE_INT __.x@free : KC0[0].x L[0x4] {LEP} PUSH_BEFORE ))
  ALU_GROUP_BEGIN
    ALU ADD_INT S34.x : KC0[0].x L[0xfffffffc] {WL}
  ALU_GROUP_END
  ALU_GROUP_BEGIN
     ALU MOVA_INT AR : S34.x {L}
  ALU_GROUP_END
  ALU_GROUP_BEGIN
    ALU MOV A2[AR].z : I[0] {W}
    ALU MOV A2[AR].w : L[0x3dcccccd] {WL}
  ALU_GROUP_END
ELSE
  ALU_GROUP_BEGIN
     ALU MOV S37.x : KC0[0].x {WL}
  ALU_GROUP_END
  ALU_GROUP_BEGIN
     ALU MOVA_INT AR : S37.x {L}
  ALU_GROUP_END
  ALU_GROUP_BEGIN
    ALU MOV A2[AR].x : I[0] {W}
    ALU MOV A2[AR].y : L[0x3dcccccd] {WL}
  ALU_GROUP_END
ENDIF
  ALU_GROUP_BEGIN
    ALU MOV S46.x@chgr : A2[0].x {W}
    ALU MOV S46.y@chgr : A2[0].y {W}
    ALU MOV S46.z@chgr : A2[1].x {W}
    ALU MOV S46.w@chgr : A2[1].y {W}
    ALU MOV S47.x@group : A2[2].x {WL}
 ALU_GROUP_END
 ALU_GROUP_BEGIN
   ALU MOV S48.x@chgr : A2[0].z {W}
   ALU MOV S47.y@chgr : A2[2].y {W}
   ALU MOV S47.z@chgr : A2[3].x {W}
   ALU MOV S47.w@chgr : A2[3].y {W}
   ALU MOV S48.y@group : A2[0].w {WL}
 ALU_GROUP_END
 ALU_GROUP_BEGIN
  ALU MOV S49.x@chgr : A2[2].z {W}
  ALU MOV S49.y@chgr : A2[2].w {W}
  ALU MOV S48.z@chgr : A2[1].z {W}
  ALU MOV S48.w@chgr : A2[1].w {W}
  ALU MOV S49.z@group : A2[3].z {WL}
 ALU_GROUP_END
 ALU_GROUP_BEGIN
   ALU MOV S49.w@chgr : A2[3].w {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE POS 0 S19.xyzw
EXPORT PARAM 0 S46.xyzw
EXPORT PARAM 1 S47.xyzw
EXPORT PARAM 2 S48.xyzw
EXPORT_DONE PARAM 3 S49.xyzw
BLOCK END\n
)";

const char *shader_with_dest_array2 =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A0[2].xy
SHADER
BLOCK_START
ALU MOV A0[0].x : KC0[0].x {W}
ALU MOV A0[0].y : KC0[0].y {WL}
ALU MOV A0[1].x : KC0[1].x {W}
ALU MOV A0[1].y : KC0[1].y {WL}
ALU MOV S1.x : KC0[2].x {WL}
ALU MOV A0[S1.x].x : I[1.0] {W}
ALU MOV A0[S1.x].y : L[2.0] {WL}
ALU MOV S2.x : A0[0].x {W}
ALU MOV S2.y : A0[0].y {WL}
ALU MUL_IEEE S3.x@group : S2.x KC0[2].y {W}
ALU MUL_IEEE S3.y@group : S2.y KC0[2].y {WL}
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S3.xy01
BLOCK_END
)";

const char *shader_with_dest_array2_scheduled =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A0[2].xy
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV A0[0].x : KC0[0].x {W}
  ALU MOV A0[0].y : KC0[0].y {W}
  ALU MOV A0[1].x : KC0[1].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV S1.x : KC0[2].x {W}
  ALU MOV A0[1].y : KC0[1].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOVA_INT AR : S1.x {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV A0[AR].x : I[1.0] {W}
  ALU MOV A0[AR].y : L[2.0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV S2.x : A0[0].x {W}
  ALU MOV S2.y : A0[0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MUL_IEEE S3.x@group : S2.x KC0[2].y {W}
  ALU MUL_IEEE S3.y@group : S2.y KC0[2].y {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S3.xy01
BLOCK_END
)";

const char *shader_with_dest_array2_scheduled_ra =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
ARRAYS A0[2].xy
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV A0[0].x : KC0[0].x {W}
  ALU MOV A0[0].y : KC0[0].y {W}
  ALU MOV A0[1].x : KC0[1].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV R2.x : KC0[2].x {W}
  ALU MOV A0[1].y : KC0[1].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOVA_INT AR : R2.x {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV A0[AR].x : I[1.0] {W}
  ALU MOV A0[AR].y : L[2.0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOV R1.x : A0[0].x {W}
  ALU MOV R1.y : A0[0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MUL_IEEE R0.x : R1.x KC0[2].y {W}
  ALU MUL_IEEE R0.y : R1.y KC0[2].y {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 R0.xy01
BLOCK_END
)";

const char *shader_group_chan_pin_to_combine =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.xy__
SHADER
ALU_GROUP_BEGIN
ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
ALU INTERP_ZW S1.z@chan : R0.y@fully Param0.z {W} VEC_210
ALU INTERP_ZW S1.w@chan : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY S1.x@chan : R0.y@fully Param0.x {W} VEC_210
ALU INTERP_XY S1.y@chan : R0.x@fully Param0.y {W} VEC_210
ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU MOV S2.x@group : S1.x@chan {W} VEC_210
ALU MOV S2.y@group : S1.y@chan {W} VEC_210
ALU MOV S2.z@group : S1.z@chan {W} VEC_210
ALU MOV S2.w@group : S1.w@chan {WL} VEC_210
EXPORT_DONE PIXEL 0 S2.xyzw
)";

const char *shader_group_chan_pin_combined =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.x R0.y
SHADER
ALU_GROUP_BEGIN
ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
ALU INTERP_ZW S1.z@chgr : R0.y@fully Param0.z {W} VEC_210
ALU INTERP_ZW S1.w@chgr : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY S1.x@chgr : R0.y@fully Param0.x {W} VEC_210
ALU INTERP_XY S1.y@chgr : R0.x@fully Param0.y {W} VEC_210
ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
EXPORT_DONE PIXEL 0 S1.xyzw
)";

const char *shader_group_chan_pin_combined_scheduled =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.x@fully R0.y@fully
SHADER
ALU_GROUP_BEGIN
ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
ALU INTERP_ZW S2.z@chgr : R0.y@fully Param0.z {W} VEC_210
ALU INTERP_ZW S2.w@chgr : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY S2.x@chgr : R0.y@fully Param0.x {W} VEC_210
ALU INTERP_XY S2.y@chgr : R0.x@fully Param0.y {W} VEC_210
ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
EXPORT_DONE PIXEL 0 S2.xyzw
)";

const char *shader_group_chan_pin_combined_scheduled_ra =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.x@fully R0.y@fully R1.xyzw
SHADER
ALU_GROUP_BEGIN
ALU INTERP_ZW __.x : R0.y Param0.x {} VEC_210
ALU INTERP_ZW __.y : R0.x Param0.y {} VEC_210
ALU INTERP_ZW R1.z : R0.y Param0.z {W} VEC_210
ALU INTERP_ZW R1.w : R0.x Param0.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
ALU INTERP_XY R1.x : R0.y Param0.x {W} VEC_210
ALU INTERP_XY R1.y : R0.x Param0.y {W} VEC_210
ALU INTERP_XY __.z : R0.y Param0.z {} VEC_210
ALU INTERP_XY __.w : R0.x Param0.w {L} VEC_210
ALU_GROUP_END
EXPORT_DONE PIXEL 0 R1.xyzw
)";

const char *shader_group_chan_pin_to_combine_2 =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S0.x@free : I[0] {WL}
ALU MOV S1.x : KC0[0].x {W}
ALU MOV S1.y : KC0[0].y {W}
ALU MOV S1.z : KC0[0].z {W}
ALU MOV S1.w : KC0[0].w {WL}
ALU DOT4_IEEE S2.x@free : S1.y S1.y + S1.y S1.y + I[0] I[0] + I[0] I[0] {WL}
ALU DOT4_IEEE S3.x@free : S1.x S1.z + S1.x S1.w + I[0] I[0] + I[0] I[0] {WL}
ALU DOT4_IEEE S4.x@free : S1.y S1.w + S1.w S1.y + I[0] I[0] + I[0] I[0] {WL}
ALU MOV S5.x@group : S2.x@free {W}
ALU MOV S5.y@group : S3.x@free {W}
ALU MOV S5.z@group : S3.x@free {W}
ALU MOV S5.w@group : S4.x@free {WL}
EXPORT_DONE PIXEL 0 S5.xyzw
)";

const char *shader_group_chan_pin_to_combine_2_opt =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU DOT4_IEEE S1026.x@group : KC0[0].y KC0[0].y + KC0[0].y KC0[0].y + I[0] I[0] + I[0] I[0] {WL}
ALU DOT4_IEEE S1026.z@group : KC0[0].x KC0[0].z + KC0[0].x KC0[0].w + I[0] I[0] + I[0] I[0] {WL}
ALU DOT4_IEEE S1026.w@group : KC0[0].y KC0[0].w + KC0[0].w KC0[0].y + I[0] I[0] + I[0] I[0] {WL}
EXPORT_DONE PIXEL 0 S1026.xzzw
)";

const char *fs_with_grand_and_abs =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV S1.x@free : I[0] {WL}
ALU_GROUP_BEGIN
  x: ALU INTERP_XY S2.x@chgr : R0.y@fully Param0.x {W} VEC_210
  y: ALU INTERP_XY S2.y@chan : R0.x@fully Param0.y {W} VEC_210
  z: ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
  w: ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU MOV S3.x@free : L[0xbf800000] {WL}
ALU MOV S4.x@free : I[1.0] {WL}
ALU MOV S5.x@free : L[0x41a00000] {WL}
ALU MOV S6.x@free : L[0x41200000] {WL}
ALU SETGT_DX10 S7.x : S2.x@chgr S1.x@free {W}
ALU SETGT_DX10 S7.y : S2.y@chan S1.x@free {WL}
ALU AND_INT S8.x : S7.x I[1.0] {W}
ALU AND_INT S8.y : S7.y I[1.0] {WL}
ALU SETGT_DX10 S9.x : S1.x@free S2.x@chgr {W}
ALU SETGT_DX10 S9.y : S1.x@free S2.y@chan {WL}
ALU AND_INT S10.x : S9.x I[1.0] {W}
ALU AND_INT S10.y : S9.y I[1.0] {WL}
ALU ADD S11.x : S8.x -S10.x {W}
ALU ADD S11.y : S8.y -S10.y {WL}
ALU SETE_DX10 S12.x : S11.x S3.x@free {W}
ALU SETE_DX10 S12.y : S11.y S3.x@free {WL}
ALU MOV S13.x@group : |S2.x@chgr| {WL}
TEX GET_GRADIENTS_H S14.x___ : S2.x___ RID:18 SID:0 NNNN
ALU MUL_IEEE S15.x@free : S14.x@group S5.x@free {WL}
ALU MOV S16.x@free : -S15.x@free {WL}
ALU CNDE_INT S17.x@free : S12.x S15.x@free S16.x@free {WL}
ALU MOV S18.x : KC0[0].x {W}
ALU MOV S18.y : KC0[0].y {W}
ALU MOV S18.z : KC0[0].z {W}
ALU MOV S18.w : KC0[0].w {WL}
ALU MUL_IEEE S19.x@group : |S2.y@chan| S18.x {WL}
ALU MOV S20.x@group : S19.x@group {WL}
TEX GET_GRADIENTS_V S21.x___ : S19.x___ RID:18 SID:0 NNNN
ALU MUL_IEEE S22.x@free : S21.x@group S6.x@free {WL}
ALU MOV S23.x@free : -S22.x@free {WL}
ALU CNDE_INT S24.x@free : S12.y S22.x@free S23.x@free {WL}
ALU MOV S25.x@group : S17.x@free {W}
ALU MOV S25.y@group : S24.x@free {W}
ALU MOV S25.z@group : S1.x@free {W}
ALU MOV S25.w@group : S4.x@free {WL}
EXPORT_DONE PIXEL 0 S25.xyzw
)";

const char *fs_opt_tex_coord_init =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.x@fully R0.y@fully
SHADER
ALU_GROUP_BEGIN
  ALU INTERP_XY S1.x@chan : R0.y@fully Param0.x {W} VEC_210
  ALU INTERP_XY S1.y@chan : R0.x@fully Param0.y {W} VEC_210
  ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
  ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
  ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
  ALU INTERP_ZW S1.z@chan : R0.y@fully Param0.z {W} VEC_210
  ALU INTERP_ZW S1.w@chan : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU MOV S2.x@group : S1.z@chan {W}
ALU MOV S2.y@group : S1.w@chan {WL}
TEX SAMPLE S3.xyzw : S1.xy__ RID:18 SID:0 NNNN
TEX SAMPLE S4.xyzw : S2.xy__ RID:18 SID:0 NNNN
ALU ADD S5.x@group : S3.x@group S4.x@group {W}
ALU ADD S5.y@group : S3.y@group S4.y@group {W}
ALU ADD S5.z@group : S3.z@group S4.z@group {W}
ALU ADD S5.w@group : S3.w@group S4.w@group {W}
EXPORT_DONE PIXEL 0 S5.xyzw)";

const char *fs_opt_tex_coord_expect =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.x@fully R0.y@fully
SHADER
ALU_GROUP_BEGIN
   ALU INTERP_XY S1.x@chan : R0.y@fully Param0.x {W} VEC_210
   ALU INTERP_XY S1.y@chan : R0.x@fully Param0.y {W} VEC_210
   ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
   ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
   ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
   ALU INTERP_ZW S1.z@chgr : R0.y@fully Param0.z {W} VEC_210
   ALU INTERP_ZW S1.w@chgr : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
TEX SAMPLE S3.xyzw : S1.xy__ RID:18 SID:0 NNNN
TEX SAMPLE S4.xyzw : S1.zw__ RID:18 SID:0 NNNN
ALU ADD S5.x@group : S3.x@group S4.x@group {W}
ALU ADD S5.y@group : S3.y@group S4.y@group {W}
ALU ADD S5.z@group : S3.z@group S4.z@group {W}
ALU ADD S5.w@group : S3.w@group S4.w@group {W}
EXPORT_DONE PIXEL 0 S5.xyzw)";

const char *fs_sched_tex_coord_init =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.x@fully R0.y@fully
SHADER
ALU_GROUP_BEGIN
  ALU INTERP_XY S1.x@chan : R0.y@fully Param0.x {W} VEC_210
  ALU INTERP_XY S1.y@chan : R0.x@fully Param0.y {W} VEC_210
  ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
  ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
  ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
  ALU INTERP_ZW S1.z@chan : R0.y@fully Param0.z {W} VEC_210
  ALU INTERP_ZW S1.w@chan : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU ADD S2.x@group : S1.x@chan S1.z@chan {W}
ALU ADD S2.y@group : S1.y@chan S1.w@chan {WL}
ALU MUL_IEEE S3.x@group : S1.x@chan S1.z@chan {W}
ALU MUL_IEEE S3.y@group : S1.y@chan S1.w@chan {WL}

TEX SAMPLE S4.xyzw : S2.xy__ RID:18 SID:0 NNNN
TEX SAMPLE S5.xyzw : S3.xy__ RID:18 SID:0 NNNN
ALU ADD S6.x@group : S5.x@group S4.x@group {W}
ALU ADD S6.y@group : S5.y@group S4.y@group {W}
ALU ADD S6.z@group : S5.z@group S4.z@group {W}
ALU ADD S6.w@group : S5.w@group S4.w@group {W}
EXPORT_DONE PIXEL 0 S5.xyzw)";

const char *fs_sched_tex_coord_expect =
   R"(FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
INPUT LOC:0 VARYING_SLOT:32 INTERP:2
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
REGISTERS R0.x@fully R0.y@fully
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU INTERP_XY S1.x@chan : R0.y@fully Param0.x {W} VEC_210
  ALU INTERP_XY S1.y@chan : R0.x@fully Param0.y {W} VEC_210
  ALU INTERP_XY __.z@chan : R0.y@fully Param0.z {} VEC_210
  ALU INTERP_XY __.w@chan : R0.x@fully Param0.w {L} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU INTERP_ZW __.x@chan : R0.y@fully Param0.x {} VEC_210
  ALU INTERP_ZW __.y@chan : R0.x@fully Param0.y {} VEC_210
  ALU INTERP_ZW S1.z@chan : R0.y@fully Param0.z {W} VEC_210
  ALU INTERP_ZW S1.w@chan : R0.x@fully Param0.w {WL} VEC_210
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU ADD S2.x@group : S1.x@chan S1.z@chan {W}
  ALU ADD S2.y@group : S1.y@chan S1.w@chan {W}
  ALU MUL_IEEE S3.z@chgr : S1.x@chan S1.z@chan {W}
  ALU MUL_IEEE S3.w@chgr : S1.y@chan S1.w@chan {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
TEX SAMPLE S4.xyzw : S2.xy__ RID:18 SID:0 NNNN
TEX SAMPLE S5.xyzw : S3.zw__ RID:18 SID:0 NNNN
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
ALU ADD S6.x@group : S5.x@group S4.x@group {W}
ALU ADD S6.y@group : S5.y@group S4.y@group {W}
ALU ADD S6.z@group : S5.z@group S4.z@group {W}
ALU ADD S6.w@group : S5.w@group S4.w@group {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S5.xyzw
BLOCK_END)";

const char *fs_with_loop_multislot_reuse =
   R"(FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU MOV R1.x@free : I[0] {WL}
ALU MOV S2.x@free : L[0x38f00000] {WL}
LOOP_BEGIN
ALU RECIPSQRT_IEEE S3.x@free : |R1.x@free| + |R1.x@free| + |R1.x@free| {WL}
ALU SETGT_DX10 S4.x@free : S3.x@free S2.x@free {W}
  IF (( ALU PRED_SETNE_INT __.x@free : S4.x@free I[0] {LEP} PUSH_BEFORE ))
     BREAK
  ENDIF
  ALU ADD S5.x@free : S3.x@chan  L[0x38f00000] {WL}
  ALU MUL R1.x@free : S5.x@free  L[0x38f00000] {WL}
LOOP_END
EXPORT_DONE PIXEL 0 R1.xxxx
)";

const char *fs_with_loop_multislot_reuse_scheduled =
   R"(FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:1
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
ALU_GROUP_BEGIN
  ALU MOV R1.x@free : I[0] {W}
  ALU MOV S2.y@chan : L[0x38f00000] {WL}
ALU_GROUP_END
LOOP_BEGIN
  ALU_GROUP_BEGIN
    ALU RECIPSQRT_IEEE S3.x@chan : |R1.x@free| {W}
    ALU RECIPSQRT_IEEE __.y@chan : |R1.x@free| {}
    ALU RECIPSQRT_IEEE __.z@chan : |R1.x@free| {L}
  ALU_GROUP_END
  ALU_GROUP_BEGIN
    ALU SETGT_DX10 S4.x@chan : S3.x@chgr S2.y@free {WL}
  ALU_GROUP_END
  IF (( ALU PRED_SETNE_INT __.x@free : S4.x@chan I[0] {LEP} PUSH_BEFORE ))
     BREAK
  ENDIF
  ALU_GROUP_BEGIN
    ALU ADD S5.x@free : S3.x@chan  L[0x38f00000] {WL}
  ALU_GROUP_END
  ALU_GROUP_BEGIN
    ALU MUL R1.x@free : S5.x@free  L[0x38f00000] {WL}
  ALU_GROUP_END
LOOP_END
EXPORT_DONE PIXEL 0 R1.xxxx
)";

const char *gs_abs_float_nir =
   R"(shader: MESA_SHADER_GEOMETRY
source_sha1: {0xdfd2ba73, 0x5eff5b0c, 0x577ee695, 0xb65ae49e, 0xecc34679}
name: GLSL4
inputs: 1
outputs: 2
uniforms: 3
shared: 0
ray queries: 0
invocations: 1
vertices in: 3
vertices out: 3
input primitive: TRIANGLES
output primitive: TRIANGLE_STRIP
active_stream_mask: 0x1
uses_end_primitive: 0
decl_var uniform INTERP_MODE_NONE float arg0 (0, 0, 0)
decl_var uniform INTERP_MODE_NONE float tolerance (1, 1, 0)
decl_var uniform INTERP_MODE_NONE float expected (2, 2, 0)
decl_function main (0 params)

impl main {
	block block_0:
	/* preds: */
	vec1 32 ssa_0 = load_const (0x00000000 = 0.000000)
	vec4 32 ssa_1 = intrinsic load_per_vertex_input (ssa_0, ssa_0) (0, 0, 160, 160)
	vec1 32 ssa_2 = load_const (0x00000001 = 0.000000)
	vec4 32 ssa_3 = intrinsic load_per_vertex_input (ssa_2, ssa_0) (0, 0, 160, 160)
	vec1 32 ssa_4 = load_const (0x00000002 = 0.000000)
	vec4 32 ssa_5 = intrinsic load_per_vertex_input (ssa_4, ssa_0) (0, 0, 160, 160)
	vec1 32 ssa_6 = load_const (0x3f800000 = 1.000000)
	vec1 32 ssa_7 = intrinsic load_uniform (ssa_0) (0, 1, 160)	/* arg0 */
	vec1 32 ssa_8 = intrinsic load_uniform (ssa_0) (2, 1, 160)	/* expected */
	vec1 32 ssa_9 = fsub abs(ssa_7), ssa_8
	vec1 32 ssa_10 = intrinsic load_uniform (ssa_0) (1, 1, 160)	/* tolerance */
	vec1 32 ssa_11 = fge32 ssa_10, abs(ssa_9)
	vec1 32 ssa_12 = inot ssa_11
	vec1 32 ssa_13 = b2f32 ssa_12
	vec1 32 ssa_14 = b2f32 ssa_11
	intrinsic store_output (ssa_1, ssa_0) (0, 15, 0,  160, 128)
	vec4 32 ssa_15 = vec4 ssa_13, ssa_14, ssa_0, ssa_6
	intrinsic store_output (ssa_15, ssa_0) (1, 15, 0, 160, 160)
	intrinsic emit_vertex () (0)
	intrinsic store_output (ssa_3, ssa_0) (0, 15, 0, 160, 128)
	intrinsic store_output (ssa_15, ssa_0) (1,15, 0, 160, 160)
	intrinsic emit_vertex () (0)
	intrinsic store_output (ssa_5, ssa_0) (0, 15, 0, 160, 128)
	intrinsic store_output (ssa_15, ssa_0) (1,15, 0, 160, 160)
	intrinsic emit_vertex () (0)
	/* succs: block_1 */
	block block_1:
})";

const char *gs_abs_float_expect =
   R"(GS
CHIPCLASS EVERGREEN
INPUT LOC:0 VARYING_SLOT:32
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
OUTPUT LOC:1 VARYING_SLOT:32 MASK:15
REGISTERS R0.x@fully R0.y@fully R0.w@fully
SHADER
ALU MOV S2.x@chan : I[0] {WL}
ALU MOV S3.x@chan : I[0] {WL}
ALU MOV S4.x@chan : I[0] {WL}
ALU MOV S5.x@chan : I[0] {WL}
ALU MOV S6.x@free : I[0] {WL}
LOAD_BUF S7.xyzw : R0.x@fully RID:17
ALU MOV S8.x@free : I[1] {WL}
LOAD_BUF S9.xyzw : R0.y@fully RID:17
ALU MOV S10.x@free : L[0x2] {WL}
LOAD_BUF S11.xyzw : R0.w@fully RID:17
ALU MOV S12.x@free : I[1.0] {WL}
ALU MOV S13.x@free : KC0[0].x {WL}
ALU MOV S14.x@free : KC0[2].x {WL}
ALU ADD S15.x@free : |S13.x@free| -S14.x@free {WL}
ALU MOV S16.x@free : KC0[1].x {WL}
ALU SETGE_DX10 S17.x@free : S16.x@free |S15.x@free| {WL}
ALU NOT_INT S18.x@free : S17.x@free {WL}
ALU AND_INT S19.x@free : S18.x@free I[1.0] {WL}
ALU AND_INT S20.x@free : S17.x@free I[1.0] {WL}
ALU MOV S21.x@group : S19.x@free {W}
ALU MOV S21.y@group : S20.x@free {W}
ALU MOV S21.z@group : S6.x@free {W}
ALU MOV S21.w@group : S12.x@free {WL}
MEM_RING 0 WRITE_IDX 0 S7.xyzw @S2.x@chan ES:4
MEM_RING 0 WRITE_IDX 4 S21.xyzw @S2.x@chan ES:4
EMIT_VERTEX @0
ALU ADD_INT S22.x@chan : S2.x@chan L[0x2] {WL}
MEM_RING 0 WRITE_IDX 0 S9.xyzw @S22.x@chan ES:4
MEM_RING 0 WRITE_IDX 4 S21.xyzw @S22.x@chan ES:4
EMIT_VERTEX @0
ALU ADD_INT S23.x@chan : S22.x@chan L[0x2] {WL}
MEM_RING 0 WRITE_IDX 0 S11.xyzw @S23.x@chan ES:4
MEM_RING 0 WRITE_IDX 4 S21.xyzw @S23.x@chan ES:4
EMIT_VERTEX @0
ALU ADD_INT S24.x@chan : S23.x@chan L[0x2] {WL}
)";

const char *vtx_for_tcs_nir =
   R"(shader: MESA_SHADER_VERTEX
source_sha1: {0xbd6100f2, 0xc71e7b0e, 0x74662024, 0x261073d8, 0xeae01762}
name: GLSL5
inputs: 0
outputs: 1
uniforms: 10
shared: 0
ray queries: 0
decl_var uniform INTERP_MODE_NONE int[6] constarray_1_0 (0, 0, 0) = { { 0x00000000 }, { 0x00000001 }, { 0x00000002 }, { 0x00000000 }, { 0x00000002 }, { 0x00000003 } }
decl_var uniform INTERP_MODE_NONE vec2[4] constarray_0_0 (1, 6, 0) = { { -1.000000, 1.000000 }, { -1.000000, -1.000000 }, { 1.000000, -1.000000 }, { 1.000000, 1.000000 } }
decl_function main (0 params)

impl main {
        block block_0:
        /* preds: */
        vec1 32 ssa_0 = load_const (0x00000000 = 0.000000)
        vec1 32 ssa_1 = load_const (0x3f800000 = 1.000000)
        vec1 32 ssa_2 = intrinsic load_vertex_id () ()
        vec1 32 ssa_3 = intrinsic load_uniform (ssa_2) (0, 6, 34)
        vec2 32 ssa_4 = intrinsic load_uniform (ssa_3) (6, 4, 160)
        vec4 32 ssa_5 = vec4 ssa_4.x, ssa_4.y, ssa_0, ssa_1
        vec4 32 ssa_6 = intrinsic load_tcs_in_param_base_r600 () ()
        vec1 32 ssa_7 = intrinsic load_tcs_rel_patch_id_r600 () ()
        vec1 32 ssa_8 = umul24 ssa_6.y, ssa_7
        intrinsic store_local_shared_r600 (ssa_5, ssa_8) (3)
        vec1 32 ssa_9 = load_const (0x00000008 = 0.000000)
        vec1 32 ssa_10 = iadd ssa_9, ssa_8
        intrinsic store_local_shared_r600 (ssa_5, ssa_10) (12)
        /* succs: block_1 */
        block block_1:
})";

const char *vtx_for_tcs_from_nir_expect =
   R"(VS
CHIPCLASS EVERGREEN
REGISTERS R0.x@fully R0.y@fully
SHADER
ALU MOV S1.x@free : I[0] {WL}
ALU MOV S2.x@free : I[1.0] {WL}
ALU MOV S3.x@free : R0.x@fully {WL}
LOAD_BUF S4.xyzw : S3.x@free RID:0
LOAD_BUF S5.xyzw : S4.x@group + 96b RID:0
ALU MOV S6.x : S5.x@group {W}
ALU MOV S6.y : S5.y@group {W}
ALU MOV S6.z : S1.x@free {W}
ALU MOV S6.w : S2.x@free {WL}
ALU MOV S7.x@free : I[0] {WL}
LOAD_BUF S8.xyzw : S7.x@free RID:16 SRF
ALU MOV S9.x@free : R0.y@fully {WL}
ALU MUL_UINT24 S10.x@free : S8.y@group S9.x@free {WL}
LDS WRITE_REL __.x [ S10.x@free ] : S6.x S6.y
ALU MOV S11.x@free : L[0x8] {WL}
ALU ADD_INT S12.x@free : S11.x@free S10.x@free {WL}
LDS WRITE_REL __.x [ S12.x@free ] : S6.z S6.w)";

const char *vtx_for_tcs_inp =
   R"(VS
CHIPCLASS EVERGREEN
REGISTERS R0.x@fully R0.y@fully
SHADER
ALU MOV S1.x@free : I[0] {WL}
ALU MOV S2.x@free : I[1.0] {WL}
ALU MOV S3.x@free : R0.x@fully {WL}
LOAD_BUF S4.xyzw : S3.x@free RID:0
LOAD_BUF S5.xyzw : S4.x@group + 96b RID:0
ALU MOV S6.x : S5.x@group {W}
ALU MOV S6.y : S5.y@group {W}
ALU MOV S6.z : S1.x@free {W}
ALU MOV S6.w : S2.x@free {WL}
ALU MOV S7.x@free : I[0] {WL}
LOAD_BUF S8.xyzw : S7.x@free RID:16 SRF
ALU MOV S9.x@free : R0.y@fully {WL}
ALU MUL_UINT24 S10.x@free : S8.y@group S9.x@free {WL}
LDS WRITE_REL __.x [ S10.x@free ] : S6.x S6.y
ALU MOV S11.x@free : L[0x8] {WL}
ALU ADD_INT S12.x@free : S11.x@free S10.x@free {WL}
LDS WRITE_REL __.x [ S12.x@free ] : S6.z S6.w)";

const char *vtx_for_tcs_opt =
   R"(VS
CHIPCLASS EVERGREEN
REGISTERS R0.x@fully R0.y@fully
SHADER
LOAD_BUF S4.x___ : R0.x@fully RID:0
LOAD_BUF S5.xy__ : S4.x@group + 96b RID:0
ALU MOV S7.x@free : I[0] {WL}
LOAD_BUF S8._y__ : S7.x@free RID:16 SRF
ALU MUL_UINT24 S10.x@free : S8.y@group R0.y@fully {WL}
LDS WRITE_REL __.x [ S10.x@free ] : S5.x@group S5.y@group
ALU ADD_INT S12.x@free : L[0x8] S10.x@free {WL}
LDS WRITE_REL __.x [ S12.x@free ] : I[0] I[1.0])";

const char *vtx_for_tcs_pre_sched =
   R"(VS
CHIPCLASS EVERGREEN
REGISTERS R0.x@fully R0.y@fully
SHADER
ALU MOV S3.x@free : R0.x@fully {WL}
LOAD_BUF S4.xyzw : S3.x@free RID:0
LOAD_BUF S5.xyzw : S4.x@group + 96b RID:0
ALU MOV S7.y@free : I[0] {WL}
LOAD_BUF S8.xyzw : S7.y@free RID:16 SRF
ALU MUL_UINT24 S10.x@free : S8.y@group R0.y@fully {WL}
LDS WRITE_REL __.x [ S10.x@free ] : S5.x@group S5.y@group
ALU ADD_INT S12.x@free : L[0x8] S10.x@free {WL}
LDS WRITE_REL __.x [ S12.x@free ] : I[0] I[1.0])";

const char *vtx_for_tcs_sched =
   R"(VS
CHIPCLASS EVERGREEN
REGISTERS R0.x@fully R0.y@fully
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV S3.x@free : R0.x@fully {W}
  ALU MOV S7.y@free : I[0] {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
LOAD_BUF S4.xyzw : S3.x@free RID:0
LOAD_BUF S8.xyzw : S7.y@free RID:16 SRF
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
  ALU MUL_UINT24 S10.x@free : S8.y@group R0.y@fully {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU ADD_INT S12.x@chan : L[0x8] S10.x@free {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
LOAD_BUF S5.xyzw : S4.x@group + 96b RID:0
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
  ALU LDS WRITE_REL __.x : S10.x@free S5.x@group S5.y@group {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU LDS WRITE_REL __.x : S12.x@chan I[0] I[1.0] {L}
ALU_GROUP_END
BLOCK_END)";

const char *tcs_nir =
   R"(shader: MESA_SHADER_TESS_CTRL
source_sha1: {0xc83b0de6, 0x36934b97, 0xccddb436, 0xb0952cb0, 0x07a450a1}
name: GLSL5
inputs: 1
outputs: 3
uniforms: 0
shared: 0
ray queries: 0
decl_function main (0 params)

impl main {
   block block_0:
   /* preds: */
   vec1 32 ssa_0 = undefined
   vec2 32 ssa_1 = load_const (0x3f800000, 0x3f800000)
   vec1 32 ssa_2 = load_const (0x00000000)
   vec4 32 ssa_3 = intrinsic load_tcs_out_param_base_r600 () ()
   vec1 32 ssa_4 = intrinsic load_tcs_rel_patch_id_r600 () ()
   vec2 32 ssa_5 = umad24 ssa_3.xx, ssa_4.xx, ssa_3.wz
   vec1 32 ssa_6 = mov ssa_5.x
   vec1 32 ssa_7 = load_const (0x00000010)
   vec4 32 ssa_8 = load_const (0x00000010, 0x00000000, 0x00000004, 0x00000008)
   vec4 32 ssa_9 = iadd ssa_5.xxxx, ssa_8
   vec1 32 ssa_10 = mov ssa_9.x
   vec3 32 ssa_11 = mov ssa_9.yzw
   intrinsic store_local_shared_r600 (ssa_1, ssa_10) (3)
   vec4 32 ssa_12 = load_const (0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000)
   vec4 32 ssa_13 = vec4 ssa_12.x, ssa_12.y, ssa_12.z, ssa_0
   intrinsic store_local_shared_r600 (ssa_13, ssa_6) (3)
   vec1 32 ssa_14 = load_const (0x00000008)
   vec1 32 ssa_15 = iadd ssa_14, ssa_5.x
   intrinsic store_local_shared_r600 (ssa_13, ssa_15) (12)
   vec1 32 ssa_16 = intrinsic load_invocation_id () ()
   vec4 32 ssa_17 = intrinsic load_tcs_in_param_base_r600 () ()
   vec1 32 ssa_18 = umul24 ssa_17.x, ssa_4
   vec1 32 ssa_19 = umad24 ssa_17.y, ssa_16, ssa_18
   vec4 32 ssa_20 = load_const (0x00000000, 0x00000004, 0x00000008, 0x0000000c)
   vec4 32 ssa_21 = iadd ssa_20, ssa_19.xxxx
   vec4 32 ssa_22 = intrinsic load_local_shared_r600 (ssa_21) ()
   vec1 32 ssa_23 = umad24 ssa_3.y, ssa_16, ssa_5.y
   intrinsic store_local_shared_r600 (ssa_22, ssa_23) (3)
   vec1 32 ssa_24 = iadd ssa_14, ssa_23
   intrinsic store_local_shared_r600 (ssa_22, ssa_24) (12)
   vec1 32 ssa_25 = ieq32 ssa_16, ssa_2
   /* succs: block_1 block_2 */
   if ssa_25 {
      block block_1:
      /* preds: block_0 */
      vec3 32 ssa_26 = intrinsic load_local_shared_r600 (ssa_11) ()
      vec1 32 ssa_27 = intrinsic load_tcs_tess_factor_base_r600 () ()
      vec1 32 ssa_28 = umad24 ssa_4, ssa_7, ssa_27
      vec3 32 ssa_29 = load_const (0x00000004, 0x00000008, 0x0000000c)
      vec3 32 ssa_30 = iadd ssa_28.xxx, ssa_29
      vec4 32 ssa_31 = vec4 ssa_28, ssa_26.x, ssa_30.x, ssa_26.y
      vec2 32 ssa_32 = vec2 ssa_30.y, ssa_26.z
      vec1 32 ssa_33 = intrinsic load_local_shared_r600 (ssa_10) ()
      vec2 32 ssa_34 = vec2 ssa_30.z, ssa_33
      intrinsic store_tf_r600 (ssa_31) ()
      intrinsic store_tf_r600 (ssa_32) ()
      intrinsic store_tf_r600 (ssa_34) ()
      /* succs: block_3 */
   } else {
      block block_2:
      /* preds: block_0 */
      /* succs: block_3 */
   }
   block block_3:
   /* preds: block_1 block_2 */
   /* succs: block_4 */
   block block_4:
})";

const char *tcs_from_nir_expect =
   R"(TCS
CHIPCLASS EVERGREEN
PROP TCS_PRIM_MODE:4
REGISTERS R0.x@fully R0.y@fully R0.z@fully R0.w@fully
SHADER
ALU MOV S1.x@free : I[0] {WL}
ALU MOV S2.x : I[1.0] {W}
ALU MOV S2.y : I[1.0] {WL}
ALU MOV S3.x@free : I[0] {WL}
ALU MOV S4.x@free : I[0] {WL}
LOAD_BUF S5.xyzw : S4.x@free + 16b RID:16 SRF
ALU MOV S6.x@free : R0.y@fully {WL}
ALU MULADD_UINT24 S7.x : S5.x@group S6.x@free S5.w@group {W}
ALU MULADD_UINT24 S7.y : S5.x@group S6.x@free S5.z@group {WL}
ALU MOV S8.x@free : S7.x {WL}
ALU MOV S9.x@free : L[0x10] {WL}
ALU MOV S10.x : L[0x10] {W}
ALU MOV S10.y : I[0] {W}
ALU MOV S10.z : L[0x4] {W}
ALU MOV S10.w : L[0x8] {WL}
ALU ADD_INT S11.x : S7.x S10.x {W}
ALU ADD_INT S11.y : S7.x S10.y {W}
ALU ADD_INT S11.z : S7.x S10.z {W}
ALU ADD_INT S11.w : S7.x S10.w {WL}
ALU MOV S12.x@free : S11.x {WL}
ALU MOV S13.x : S11.y {W}
ALU MOV S13.y : S11.z {W}
ALU MOV S13.z : S11.w {WL}
LDS WRITE_REL __.x [ S12.x@free ] : S2.x S2.y
ALU MOV S14.x : I[1.0] {W}
ALU MOV S14.y : I[1.0] {W}
ALU MOV S14.z : I[1.0] {W}
ALU MOV S14.w : I[1.0] {WL}
ALU MOV S15.x : S14.x {W}
ALU MOV S15.y : S14.y {W}
ALU MOV S15.z : S14.z {W}
ALU MOV S15.w : S1.x@free {WL}
LDS WRITE_REL __.x [ S8.x@free ] : S15.x S15.y
ALU MOV S16.x@free : L[0x8] {WL}
ALU ADD_INT S17.x@free : S16.x@free S7.x {WL}
LDS WRITE_REL __.x [ S17.x@free ] : S15.z S15.w
ALU MOV S18.x@free : R0.z@fully {WL}
ALU MOV S19.y@free : I[0] {WL}
LOAD_BUF S20.xyzw : S19.y@free RID:16 SRF
ALU MUL_UINT24 S21.x@free : S20.x@group S6.x@free {WL}
ALU MULADD_UINT24 S22.x@free : S20.y@group S18.x@free S21.x@free {WL}
ALU MOV S23.x : I[0] {W}
ALU MOV S23.y : L[0x4] {W}
ALU MOV S23.z : L[0x8] {W}
ALU MOV S23.w : L[0xc] {WL}
ALU ADD_INT S24.x : S23.x S22.x@free {W}
ALU ADD_INT S24.y : S23.y S22.x@free {W}
ALU ADD_INT S24.z : S23.z S22.x@free {W}
ALU ADD_INT S24.w : S23.w S22.x@free {WL}
LDS_READ [ S25.x@free S25.y@free S25.z@free S25.w@free ] : [ S24.x S24.y S24.z S24.w ]
ALU MULADD_UINT24 S26.x@free : S5.y@group S18.x@free S7.y {WL}
LDS WRITE_REL __.x [ S26.x@free ] : S25.x@free S25.y@free
ALU ADD_INT S27.x@free : S16.x@free S26.x@free {WL}
LDS WRITE_REL __.x [ S27.x@free ] : S25.z@free S25.w@free
ALU SETE_INT S28.x@free : S18.x@free S3.x@free {WL}
IF (( ALU PRED_SETNE_INT __.z@free : S28.x@free I[0] {LEP} PUSH_BEFORE ))
  LDS_READ [ S30.x@free S30.y@free S30.z@free ] : [ S13.x S13.y S13.z ]
  ALU MOV S31.x@free : R0.w@fully {WL}
  ALU MULADD_UINT24 S32.x@free : S6.x@free S9.x@free S31.x@free {WL}
  ALU MOV S33.x : L[0x4] {W}
  ALU MOV S33.y : L[0x8] {W}
  ALU MOV S33.z : L[0xc] {WL}
  ALU ADD_INT S34.x : S32.x@free S33.x {W}
  ALU ADD_INT S34.y : S32.x@free S33.y {W}
  ALU ADD_INT S34.z : S32.x@free S33.z {WL}
  ALU MOV S35.x : S32.x@free {W}
  ALU MOV S35.y : S30.x@free {W}
  ALU MOV S35.z : S34.x {W}
  ALU MOV S35.w : S30.y@free {WL}
  ALU MOV S36.x : S34.y {W}
  ALU MOV S36.y : S30.z@free {WL}
  LDS_READ [ S37.x@free ] : [ S12.x@free ]
  ALU MOV S38.x : S34.z {W}
  ALU MOV S38.y : S37.x@free {WL}
  ALU MOV S39.x@group : S35.x {W}
  ALU MOV S39.y@group : S35.y {W}
  ALU MOV S40.z@group : S35.z {W}
  ALU MOV S40.w@group : S35.w {WL}
  WRITE_TF S40.zw__
  WRITE_TF S39.xy__
  ALU MOV S41.x@group : S36.x {W}
  ALU MOV S41.y@group : S36.y {WL}
  WRITE_TF S41.xy__
  ALU MOV S42.x@group : S38.x {W}
  ALU MOV S42.y@group : S38.y {WL}
  WRITE_TF S42.xy__
ENDIF)";

const char *tes_nir =
   R"(shader: MESA_SHADER_TESS_EVAL
source_sha1: {0x2db04154, 0x4884cf59, 0x50e43ee6, 0x4bb239d7, 0x0b502229}
name: GLSL5
inputs: 1
outputs: 1
uniforms: 0
shared: 0
ray queries: 0
decl_function main (0 params)

impl main {
   block block_0:
   /* preds: */
   vec1 32 ssa_0 = load_const (0x40000000)
   vec2 32 ssa_1 = intrinsic load_tess_coord_xy () ()
   vec1 32 ssa_2 = fadd ssa_1.x, ssa_1.y
   vec1 32 ssa_3 = load_const (0x3f800000)
   vec1 32 ssa_4 = fsub ssa_3, ssa_2
   vec1 32 ssa_5 = ffma ssa_0, ssa_4, ssa_1.y
   vec1 32 ssa_6 = f2i32 ssa_5
   vec1 32 ssa_7 = load_const (0x00000000)
   vec4 32 ssa_8 = intrinsic load_tcs_out_param_base_r600 () ()
   vec1 32 ssa_9 = intrinsic load_tcs_rel_patch_id_r600 () ()
   vec1 32 ssa_10 = umad24 ssa_8.x, ssa_9, ssa_8.z
   vec1 32 ssa_11 = umad24 ssa_8.y, ssa_6, ssa_10
   vec4 32 ssa_12 = load_const (0x00000000, 0x00000004, 0x00000008, 0x0000000c)
   vec4 32 ssa_13 = iadd ssa_12, ssa_11.xxxx
   vec4 32 ssa_14 = intrinsic load_local_shared_r600 (ssa_13) ()
   intrinsic store_output (ssa_14, ssa_7) (0, 15, 0, 160, 128)
    /* succs: block_1 */
    block block_1:
})";

const char *tes_from_nir_expect =
   R"(TES
CHIPCLASS EVERGREEN
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R0.x@fully R0.y@fully R0.z@fully
SHADER
ALU MOV S1.x@free : L[0x40000000] {WL}
ALU MOV S2.x@free : R0.x@fully {WL}
ALU MOV S2.y@free : R0.y@fully {WL}
ALU ADD S3.x@free : S2.x@free S2.y@free {WL}
ALU MOV S4.x@free : I[1.0] {WL}
ALU ADD S5.x@free : S4.x@free -S3.x@free {WL}
ALU MULADD_IEEE S6.x@free : S1.x@free S5.x@free S2.y@free {WL}
ALU TRUNC S7.x@free : S6.x@free {WL}
ALU FLT_TO_INT S8.x@free : S7.x@free {WL}
ALU MOV S9.x@free : I[0] {WL}
ALU MOV S10.y@free : I[0] {WL}
LOAD_BUF S11.xyzw : S10.y@free + 16b RID:16 SRF
ALU MOV S12.x@free : R0.z@fully {WL}
ALU MULADD_UINT24 S13.x@free : S11.x@group S12.x@free S11.z@group {WL}
ALU MULADD_UINT24 S14.x@free : S11.y@group S8.x@free S13.x@free {WL}
ALU MOV S15.x : I[0] {W}
ALU MOV S15.y : L[0x4] {W}
ALU MOV S15.z : L[0x8] {W}
ALU MOV S15.w : L[0xc] {WL}
ALU ADD_INT S16.x : S15.x S14.x@free {W}
ALU ADD_INT S16.y : S15.y S14.x@free {W}
ALU ADD_INT S16.z : S15.z S14.x@free {W}
ALU ADD_INT S16.w : S15.w S14.x@free {WL}
LDS_READ [ S17.x@group S17.y@group S17.z@group S17.w@group ] : [ S16.x S16.y S16.z S16.w ]
EXPORT_DONE POS 0 S17.xyzw
EXPORT_DONE PARAM 0 R0.____)";

const char *tes_pre_op =
   R"(TES
CHIPCLASS EVERGREEN
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R0.x@fully R0.y@fully R0.z@fully
SHADER
ALU MOV S1024.x@free : L[0x40000000] {WL}
ALU MOV S1025.x@free : R0.x@fully {WL}
ALU MOV S1025.y@free : R0.y@fully {WL}
ALU ADD S1026.x@free : S1025.x@free S1025.y@free {WL}
ALU MOV S1027.x@free : I[1.0] {WL}
ALU ADD S1028.x@free : S1027.x@free -S1026.x@free {WL}
ALU MULADD_IEEE S1029.x@free : S1024.x@free S1028.x@free S1025.y@free {WL}
ALU TRUNC S1030.x@free : S1029.x@free {WL}
ALU FLT_TO_INT S1031.x@free : S1030.x@free {WL}
ALU MOV S1032.x@free : I[0] {WL}
ALU MOV S1033.y@free : I[0] {WL}
LOAD_BUF S1034.xyzw : S1033.y@free RID:16 SRF
ALU MOV S1035.x@free : R0.z@fully {WL}
ALU MULADD_UINT24 S1036.x@free : S1034.x@group S1035.x@free S1034.z@group {WL}
ALU MULADD_UINT24 S1037.x@free : S1034.y@group S1031.x@free S1036.x@free {WL}
ALU MOV S1038.x : I[0] {W}
ALU MOV S1038.y : L[0x4] {W}
ALU MOV S1038.z : L[0x8] {W}
ALU MOV S1038.w : L[0xc] {WL}
ALU ADD_INT S1039.x : S1038.x S1037.x@free {W}
ALU ADD_INT S1039.y : S1038.y S1037.x@free {W}
ALU ADD_INT S1039.z : S1038.z S1037.x@free {W}
ALU ADD_INT S1039.w : S1038.w S1037.x@free {WL}
LDS_READ [ S1040.x@group S1040.y@group S1040.z@group S1040.w@group ] : [ S1039.x S1039.y S1039.z S1039.w ]
EXPORT_DONE POS 0 S1040.xyzw
EXPORT_DONE PARAM 0 R0.____)";

const char *tes_optimized =
   R"(TES
CHIPCLASS EVERGREEN
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R0.x@fully R0.y@fully R0.z@fully
SHADER
ALU ADD S1026.x@free : R0.x@fully R0.y@fully {WL}
ALU ADD S1028.x@free : I[1.0] -S1026.x@free {WL}
ALU MULADD_IEEE S1029.x@free : L[0x40000000] S1028.x@free R0.y@fully {WL}
ALU TRUNC S1030.x@free : S1029.x@free {WL}
ALU FLT_TO_INT S1031.x@free : S1030.x@free {WL}
ALU MOV S1033.y@free : I[0] {WL}
LOAD_BUF S1034.xyz_ : S1033.y@free RID:16 SRF
ALU MULADD_UINT24 S1036.x@free : S1034.x@group R0.z@fully S1034.z@group {WL}
ALU MULADD_UINT24 S1037.x@free : S1034.y@group S1031.x@free S1036.x@free {WL}
ALU MOV S1039.x : S1037.x@free {W}
ALU ADD_INT S1039.y : L[0x4] S1037.x@free {W}
ALU ADD_INT S1039.z : L[0x8] S1037.x@free {W}
ALU ADD_INT S1039.w : L[0xc] S1037.x@free {WL}
LDS_READ [ S1040.x@group S1040.y@group S1040.z@group S1040.w@group ] : [ S1039.x S1039.y S1039.z S1039.w ]
EXPORT_DONE POS 0 S1040.xyzw
EXPORT_DONE PARAM 0 R0.____)";

const char *tes_optimized_pre_sched =
   R"(TES
CHIPCLASS EVERGREEN
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R0.x@fully R0.y@fully R0.z@fully
SHADER
ALU ADD S1026.x@free : R0.x@fully R0.y@fully {WL}
ALU ADD S1028.x@free : I[1.0] -S1026.x@free {WL}
ALU MULADD_IEEE S1029.x@free : L[0x40000000] S1028.x@free R0.y@fully {WL}
ALU TRUNC S1030.x@free : S1029.x@free {WL}
ALU FLT_TO_INT S1031.x@free : S1030.x@free {WL}
ALU MOV S1033.y@free : I[0] {WL}
LOAD_BUF S1034.xyzw : S1033.y@free RID:16 SRF
ALU MULADD_UINT24 S1036.x@free : S1034.x@group R0.z@fully S1034.z@group {WL}
ALU MULADD_UINT24 S1037.x@free : S1034.y@group S1031.x@free S1036.x@free {WL}
ALU ADD_INT S1039.x : I[0] S1037.x@free {W}
ALU ADD_INT S1039.y : L[0x4] S1037.x@free {W}
ALU ADD_INT S1039.z : L[0x8] S1037.x@free {W}
ALU ADD_INT S1039.w : L[0xc] S1037.x@free {WL}
LDS_READ [ S1040.x@group S1040.y@group S1040.z@group S1040.w@group ] : [ S1039.x S1039.y S1039.z S1039.w ]
EXPORT_DONE POS 0 S1040.xyzw
EXPORT_DONE PARAM 0 R0.____)";

const char *tes_optimized_sched =
   R"(TES
CHIPCLASS EVERGREEN
OUTPUT LOC:0 VARYING_SLOT:0 MASK:15
REGISTERS R0.x@fully R0.y@fully R0.z@fully
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU ADD S1026.x@chan : R0.x@fully R0.y@fully {W}
   ALU MOV S1033.y@chan : I[0] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU ADD S1028.x@chan : I[1.0] -S1026.x@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU MULADD_IEEE S1029.x@chan : L[0x40000000] S1028.x@chan R0.y@fully {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU TRUNC S1030.x@chan : S1029.x@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU FLT_TO_INT S1031.x@chan : S1030.x@chan {WL}
ALU_GROUP_END
BLOCK_START
BLOCK_END
LOAD_BUF S1034.xyzw : S1033.y@chan RID:16 SRF
BLOCK_START
BLOCK_END
ALU_GROUP_BEGIN
   ALU MULADD_UINT24 S1036.x@chan : S1034.x@group R0.z@fully S1034.z@group {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU MULADD_UINT24 S1037.x@chan : S1034.y@group S1031.x@chan S1036.x@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU ADD_INT S1039.x : I[0] S1037.x@chan {W}
   ALU ADD_INT S1039.y : L[0x4] S1037.x@chan {W}
   ALU ADD_INT S1039.z : L[0x8] S1037.x@chan {W}
   ALU ADD_INT S1039.w : L[0xc] S1037.x@chan {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU LDS READ_RET __.x@chan : S1039.x {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU LDS READ_RET __.x@chan : S1039.y {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU LDS READ_RET __.x@chan : S1039.z {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU LDS READ_RET __.x@chan : S1039.w {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU MOV S1040.x@group : I[LDS_OQ_A_POP] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU MOV S1040.y@group : I[LDS_OQ_A_POP] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU MOV S1040.z@group : I[LDS_OQ_A_POP] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
   ALU MOV S1040.w@group : I[LDS_OQ_A_POP] {WL}
ALU_GROUP_END
BLOCK_START
BLOCK_END
EXPORT_DONE POS 0 S1040.xyzw
EXPORT_DONE PARAM 0 R0.____
BLOCK_END)";

void
TestShader::SetUp()
{
   init_pool();
   SetUpMore();
}

void
TestShader::TearDown()
{
   TearDownMore();
   release_pool();
}

void
TestShader::SetUpMore()
{
}

void
TestShader::TearDownMore()
{
}

Shader *
TestShader::from_string(const std::string& s)
{
   istringstream is(s);
   string line;

   r600_shader_key key = {{0}};
   key.ps.nr_cbufs = 1;

   do {
      std::getline(is, line);
   } while (line.empty());

   Shader *shader = nullptr;

   if (line.substr(0, 2) == "FS")
      shader = new FragmentShaderEG(key);
   else if (line.substr(0, 2) == "VS")
      shader = new VertexShader(nullptr, nullptr, key);
   else if (line.substr(0, 2) == "GS")
      shader = new GeometryShader(key);
   else if (line.substr(0, 3) == "TCS")
      shader = new TCSShader(key);
   else if (line.substr(0, 3) == "TES")
      shader = new TESShader(nullptr, nullptr, key);
   else
      return nullptr;

   shader->reset_shader_id();

   while (std::getline(is, line)) {
      if (line.find_first_not_of(" \t") == std::string::npos)
         continue;
      if (line[0] == '#')
         continue;

      if (line.substr(0, 6) == "SHADER")
         break;

      istringstream ls(line);
      if (!shader->add_info_from_string(ls)) {
         std::cerr << "Don't understand '" << line << "\n";
         return nullptr;
      }
   }

   while (std::getline(is, line)) {
      if (line.find_first_not_of(" \t") == std::string::npos)
         continue;
      if (line[0] == '#')
         continue;

      shader->emit_instruction_from_string(line);
   }

   return shader;
}

} // namespace r600
