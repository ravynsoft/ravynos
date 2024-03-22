/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2023 Collabora LTD
 *
 * Author: Gert Wollny <gert.wollny@collabora.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "sfn_test_shaders.h"
#include "../sfn_split_address_loads.h"
#include "../sfn_optimizer.h"
#include "../sfn_scheduler.h"


using namespace r600;

TEST_F(TestShaderFromNir, SimpleLoadAddress)
{
   const char *input =
R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.x__
ARRAYS A1[4].x
REGISTERS AR
SHADER
ALU ADD A1[R0.x].x : L[0xbf000000] KC0[0].x {WL}
ALU MOV S1.x@group : A1[0].x {WL}
EXPORT_DONE PIXEL 0 S1.xxxx
)";

   const char *expect =
R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.x__
ARRAYS A1[4].x
REGISTERS AR
SHADER
ALU MOVA_INT AR : R0.x
ALU ADD A1[AR].x : L[0xbf000000] KC0[0].x {WL}
ALU MOV S1.x@group : A1[0].x {WL}
EXPORT_DONE PIXEL 0 S1.xxxx
)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   check(sh, expect);
}


TEST_F(TestShaderFromNir, DestIndirectAddress)
{
   const char *input =
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
ALU MOV S46.x@group : A2[0].x {W}
ALU MOV S46.y@group : A2[0].y {W}
ALU MOV S46.z@group : A2[1].x {W}
ALU MOV S46.w@group : A2[1].y {WL}
EXPORT PARAM 0 S46.xyzw
ALU MOV S47.x@group : A2[2].x {W}
ALU MOV S47.y@group : A2[2].y {W}
ALU MOV S47.z@group : A2[3].x {W}
ALU MOV S47.w@group : A2[3].y {WL}
EXPORT PARAM 1 S47.xyzw
ALU MOV S48.x@group : A2[0].z {W}
ALU MOV S48.y@group : A2[0].w {W}
ALU MOV S48.z@group : A2[1].z {W}
ALU MOV S48.w@group : A2[1].w {WL}
EXPORT PARAM 2 S48.xyzw
ALU MOV S49.x@group : A2[2].z {W}
ALU MOV S49.y@group : A2[2].w {W}
ALU MOV S49.z@group : A2[3].z {W}
ALU MOV S49.w@group : A2[3].w {WL}
EXPORT_DONE PARAM 3 S49.xyzw
)";


   const char *expect =
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
     ALU MOVA_INT AR : S34.x
     ALU MOV A2[AR].z : I[0] {W}
     ALU MOV A2[AR].w : L[0x3dcccccd] {WL}
ELSE
     ALU MOV S37.x : KC0[0].x {WL}
     ALU MOVA_INT AR : S37.x
     ALU MOV A2[AR].x : I[0] {W}
     ALU MOV A2[AR].y : L[0x3dcccccd] {WL}
ENDIF
EXPORT_DONE POS 0 S19.xyzw
ALU MOV S46.x@group : A2[0].x {W}
ALU MOV S46.y@group : A2[0].y {W}
ALU MOV S46.z@group : A2[1].x {W}
ALU MOV S46.w@group : A2[1].y {WL}
EXPORT PARAM 0 S46.xyzw
ALU MOV S47.x@group : A2[2].x {W}
ALU MOV S47.y@group : A2[2].y {W}
ALU MOV S47.z@group : A2[3].x {W}
ALU MOV S47.w@group : A2[3].y {WL}
EXPORT PARAM 1 S47.xyzw
ALU MOV S48.x@group : A2[0].z {W}
ALU MOV S48.y@group : A2[0].w {W}
ALU MOV S48.z@group : A2[1].z {W}
ALU MOV S48.w@group : A2[1].w {WL}
EXPORT PARAM 2 S48.xyzw
ALU MOV S49.x@group : A2[2].z {W}
ALU MOV S49.y@group : A2[2].w {W}
ALU MOV S49.z@group : A2[3].z {W}
ALU MOV S49.w@group : A2[3].w {WL}
EXPORT_DONE PARAM 3 S49.xyzw
)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   check(sh, expect);
}




TEST_F(TestShaderFromNir, SimpleLoadIndexEG)
{
   const char *input =
R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.x
ARRAYS A1[4].x
REGISTERS AR
SHADER
ALU ADD S1.x : L[0xbf000000] KC0[R0.x][0].x {WL}
EXPORT_DONE PIXEL 0 S1.xxxx
)";

   const char *expect =
R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.x
ARRAYS A1[4].x
REGISTERS AR
SHADER
ALU MOVA_INT AR : R0.x
ALU SET_CF_IDX0 IDX0 : AR
ALU ADD S1.x@group : L[0xbf000000] KC0[IDX0][0].x {WL}
EXPORT_DONE PIXEL 0 S1.xxxx
)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   check(sh, expect);
}

TEST_F(TestShaderFromNir, SimpleLoadIndexCA)
{
   const char *input =
R"(
FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.x
ARRAYS A1[4].x
REGISTERS AR
SHADER
ALU ADD S1.x : L[0xbf000000] KC0[R0.x][0].x {WL}
EXPORT_DONE PIXEL 0 S1.xxxx
)";

   const char *expect =
R"(
FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.x
ARRAYS A1[4].x
REGISTERS AR
SHADER
ALU MOVA_INT IDX0 : R0.x
ALU ADD S1.x@group : L[0xbf000000] KC0[IDX0][0].x {WL}
EXPORT_DONE PIXEL 0 S1.xxxx
)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   check(sh, expect);
}


TEST_F(TestShaderFromNir, SimpleLoadIndexBuf)
{
   const char *input =
R"(
FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.x R0.y
REGISTERS AR
SHADER
LOAD_BUF S1.xyzw : R0.x + 16b RID:10 + R0.y
EXPORT_DONE PIXEL 0 S1.xyzw
)";

   const char *expect =
R"(
FS
CHIPCLASS CAYMAN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SYSVALUES R0.x R0.y
SHADER
ALU MOVA_INT IDX0 : R0.y
LOAD_BUF S1.xyzw : R0.x + 16b RID:10 + IDX0
EXPORT_DONE PIXEL 0 S1.xyzw
)";

   auto sh = from_string(input);
   split_address_loads(*sh);
   check(sh, expect);
}


TEST_F(TestShaderFromNir, SplitLoadIndexConst)
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
  ALU MOV S4.x@group{s} : KC1[S3.w@free{s}][0].x {W}
  ALU MOV S4.y@group{s} : KC1[S3.w@free{s}][0].y {W}
  ALU MOV S4.z@group{s} : KC1[S3.w@free{s}][0].z {W}
  ALU MOV S4.w@group{s} : KC1[S3.w@free{s}][0].w {WL}
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
  ALU MIN_UINT S3.w@free{s} : KC0[0].x L[0x2] {WL}
  ALU MOVA_INT IDX0 : S3.w@free{s} {}
  ALU MOV S4.x@group{s} : KC1[IDX0][0].x {W}
  ALU MOV S4.y@group{s} : KC1[IDX0][0].y {W}
  ALU MOV S4.z@group{s} : KC1[IDX0][0].z {W}
  ALU MOV S4.w@group{s} : KC1[IDX0][0].w {WL}
  EXPORT_DONE PIXEL 0 S4.xyzw
BLOCK_END
)";
   auto sh = from_string(input);
   split_address_loads(*sh);
   check(sh, expect);
}


TEST_F(TestShaderFromNir, SplitLoadIndexConstOptAndSchedule)
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
  ALU MIN_UINT S3.w@free : KC0[0].x L[0x2] {WL}
  ALU MOV S4.x@group : KC1[S3.w@free{s}][0].x {W}
  ALU MOV S4.y@group : KC1[S3.w@free{s}][0].y {W}
  ALU MOV S4.z@group : KC1[S3.w@free{s}][0].z {W}
  ALU MOV S4.w@group : KC1[S3.w@free{s}][0].w {WL}
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
  ALU MIN_UINT S3.w@free : KC0[0].x L[0x2] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOVA_INT IDX0 : S3.w@free {L}
ALU_GROUP_END
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV S4.x@chgr : KC1[IDX0][0].x {W}
  ALU MOV S4.y@chgr : KC1[IDX0][0].y {W}
  ALU MOV S4.z@chgr : KC1[IDX0][0].z {W}
  ALU MOV S4.w@chgr : KC1[IDX0][0].w {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S4.xyzw
BLOCK_END
)";
   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);
}


TEST_F(TestShaderFromNir, SplitLoadWithNonAlu)
{
   const char *input =
R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
BLOCK_START
  ALU MOV S0.x@free : KC0[1].x {W}
  ALU MOV S0.y@free : KC0[1].y {W}
  ALU MOV S2.w@free : KC0[0].x {WL}
  TEX SAMPLE S3.xyzw : S0.xy__ RID:0 SID:0 NNNN
  ALU ADD S4.x@group : KC1[S2.w@free{s}][0].x S3.x {W}
  ALU ADD S4.y@group : KC1[S2.w@free{s}][0].y S3.y {W}
  ALU ADD S4.z@group : KC1[S2.w@free{s}][0].z S3.z {W}
  ALU ADD S4.w@group : KC1[S2.w@free{s}][0].w S3.w {WL}
  EXPORT_DONE PIXEL 0 S4.xyzw
BLOCK_END
)";

   const char *expect =
R"(
FS
CHIPCLASS EVERGREEN
PROP MAX_COLOR_EXPORTS:1
PROP COLOR_EXPORTS:1
PROP COLOR_EXPORT_MASK:15
PROP WRITE_ALL_COLORS:0
OUTPUT LOC:0 FRAG_RESULT:2 MASK:15
SHADER
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOVA_INT AR : KC0[0].x {}
  ALU MOV S0.y@free : KC0[1].y {W}
  ALU MOV S0.z@free : KC0[1].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU SET_CF_IDX0 IDX0 : AR {L}
ALU_GROUP_END
BLOCK_END
BLOCK_START
  TEX SAMPLE S3.xyzw : S0.zy__ RID:0 SID:0 NNNN
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
  ALU ADD S4.x@chgr : KC1[IDX0][0].x S3.x {W}
  ALU ADD S4.y@chgr : KC1[IDX0][0].y S3.y {W}
  ALU ADD S4.z@chgr : KC1[IDX0][0].z S3.z {W}
  ALU ADD S4.w@chgr : KC1[IDX0][0].w S3.w {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S4.xyzw
BLOCK_END
)";
   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);
}

TEST_F(TestShaderFromNir, SplitLoadIndexTwoTimesOptAndSchedule)
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
  ALU MIN_UINT S3.x@free : KC0[0].x L[0x2] {W}
  ALU MIN_UINT S3.y@free : KC0[0].y L[0x2] {W}
  ALU MIN_UINT S3.z@free : KC0[0].z L[0x2] {W}
  ALU MIN_UINT S3.w@free : KC0[0].w L[0x2] {WL}
  ALU MOV S4.x@group : KC1[S3.x@free{s}][0].x {W}
  ALU MOV S4.y@group : KC1[S3.y@free{s}][0].y {W}
  ALU MOV S4.z@group : KC1[S3.z@free{s}][0].z {W}
  ALU MOV S4.w@group : KC1[S3.w@free{s}][0].w {WL}
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
  ALU MIN_UINT S3.x@free : KC0[0].x L[0x2] {W}
  ALU MIN_UINT S3.y@free : KC0[0].y L[0x2] {W}
  ALU MIN_UINT S3.z@free : KC0[0].z L[0x2] {W}
  ALU MIN_UINT S3.w@free : KC0[0].w L[0x2] {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOVA_INT IDX0 : S3.x@free {L}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOVA_INT IDX1 : S3.y@free {L}
ALU_GROUP_END
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV S4.x@chgr : KC1[IDX0][0].x {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOVA_INT IDX0 : S3.z@free {L}
ALU_GROUP_END
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV S4.z@chgr : KC1[IDX0][0].z {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV S4.y@chgr : KC1[IDX1][0].y {WL}
ALU_GROUP_END
ALU_GROUP_BEGIN
  ALU MOVA_INT IDX1 : S3.w@free {L}
ALU_GROUP_END
BLOCK_END
BLOCK_START
ALU_GROUP_BEGIN
  ALU MOV S4.w@chgr : KC1[IDX1][0].w {WL}
ALU_GROUP_END
BLOCK_END
BLOCK_START
EXPORT_DONE PIXEL 0 S4.xyzw
BLOCK_END
)";
   auto sh = from_string(input);
   split_address_loads(*sh);
   optimize(*sh);
   check(schedule(sh), expect);
}

