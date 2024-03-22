#include "../sfn_liverangeevaluator.h"
#include "../sfn_shader.h"
#include "sfn_test_shaders.h"

#include "gtest/gtest.h"
#include <sstream>

namespace r600 {

using std::ostringstream;

class LiveRangeTests : public TestShader {

protected:
   void check(const char *shader, LiveRangeMap& expect);
};

using SimpleTest = testing::Test;

TEST_F(SimpleTest, SimpleLiveRangeMapTest)
{
   LiveRangeMap a;
   LiveRangeMap b;

   EXPECT_EQ(a, b);

   Register r1x(1, 0, pin_none);
   a.append_register(&r1x);
   r1x.set_index(0);
   a.set_life_range(r1x, 0, 1);

   EXPECT_NE(a, b);

   b.append_register(&r1x);
   b.set_life_range(r1x, 0, 1);
   EXPECT_EQ(a, b);

   Register r2x(2, 0, pin_none);
   a.append_register(&r2x);
   r2x.set_index(0);
   a.set_life_range(r2x, 0, 2);

   EXPECT_NE(a, b);

   b.append_register(&r2x);
   b.set_life_range(r2x, 0, 2);
   EXPECT_EQ(a, b);

   a.set_life_range(r2x, 1, 2);
   EXPECT_NE(a, b);

   b.set_life_range(r2x, 1, 2);
   EXPECT_EQ(a, b);

   a.set_life_range(r2x, 0, 1);
   EXPECT_NE(a, b);
}

TEST_F(LiveRangeTests, SimpleAssignments)
{
   RegisterVec4::Swizzle dummy;

   ValueFactory vf;

   Register *r1x = vf.dest_from_string("S1.x@free");
   RegisterVec4 r0 = vf.dest_vec4_from_string("S0.xyzw", dummy, pin_group);

   LiveRangeMap expect = vf.prepare_live_range_map();

   expect.set_life_range(*r1x, 2, 3);
   for (int i = 0; i < 4; ++i)
      expect.set_life_range(*r0[i], 1, 3);

   check(red_triangle_fs_expect_from_nir, expect);
}

TEST_F(LiveRangeTests, SimpleAdd)
{
   RegisterVec4::Swizzle dummy;

   ValueFactory vf;
   Register *r0x = vf.dest_from_string("S0.x@free");
   Register *r1x = vf.dest_from_string("S1.x@free");
   RegisterVec4 r2 = vf.dest_vec4_from_string("S2.xyzw", dummy, pin_none);
   Register *r3x = vf.dest_from_string("S3.x@free");
   RegisterVec4 r4 = vf.dest_vec4_from_string("S4.xyzw", dummy, pin_group);

   LiveRangeMap expect = vf.prepare_live_range_map();

   expect.set_life_range(*r0x, 1, 4);
   expect.set_life_range(*r1x, 2, 3);
   expect.set_life_range(*r3x, 4, 5);

   expect.set_life_range(*r2[0], 3, 4);
   for (int i = 1; i < 4; ++i)
      expect.set_life_range(*r2[i], 3, 5);

   for (int i = 0; i < 4; ++i)
      expect.set_life_range(*r4[i], 5, 6);

   check(add_add_1_expect_from_nir, expect);
}

TEST_F(LiveRangeTests, SimpleAInterpolation)
{
   RegisterVec4::Swizzle dummy;

   ValueFactory vf;
   Register *r0x = vf.dest_from_string("R0.x@fully");
   r0x->set_flag(Register::pin_start);
   Register *r0y = vf.dest_from_string("R0.y@fully");
   r0y->set_flag(Register::pin_start);

   Register *r1x = vf.dest_from_string("S1.x@free");
   RegisterVec4 r2 = vf.dest_vec4_from_string("S2.xyzw", dummy, pin_chan);

   Register *r3x = vf.dest_from_string("S3.x");
   Register *r3y = vf.dest_from_string("S3.y");
   Register *r3z = vf.dest_from_string("S3.z");

   Register *r4x = vf.dest_from_string("S4.x");
   Register *r4y = vf.dest_from_string("S4.y");

   RegisterVec4 r5 = vf.dest_vec4_from_string("S5.xy_w", dummy, pin_group);
   RegisterVec4 r6 = vf.dest_vec4_from_string("S6.xyzw", dummy, pin_group);

   LiveRangeMap expect = vf.prepare_live_range_map();

   expect.set_life_range(*r0x, 0, 3);
   expect.set_life_range(*r0y, 0, 3);

   expect.set_life_range(*r1x, 1, 2);

   expect.set_life_range(*r2[0], 3, 4);
   expect.set_life_range(*r2[1], 3, 4);
   expect.set_life_range(*r2[2], 2, 3);
   expect.set_life_range(*r2[3], 2, 4);

   expect.set_life_range(*r3x, 4, 5);
   expect.set_life_range(*r3y, 4, 5);
   expect.set_life_range(*r3z, 4, 6);

   expect.set_life_range(*r4x, 5, 6);
   expect.set_life_range(*r4y, 5, 6);

   expect.set_life_range(*r5[0], 6, 7);
   expect.set_life_range(*r5[1], 6, 7);
   expect.set_life_range(*r5[3], 6, 7);

   expect.set_life_range(*r6[0], 7, 8);
   expect.set_life_range(*r6[1], 7, 8);
   expect.set_life_range(*r6[2], 7, 8);
   expect.set_life_range(*r6[3], 7, 8);

   check(basic_interpolation_translated_1, expect);
}

TEST_F(LiveRangeTests, SimpleArrayAccess)
{
   RegisterVec4::Swizzle dummy;

   ValueFactory vf;

   auto array = vf.array_from_string("A0[2].xy");

   auto s1 = vf.dest_from_string("S1.x");
   auto s2x = vf.dest_from_string("S2.x");
   auto s2y = vf.dest_from_string("S2.y");

   auto s3 = vf.dest_vec4_from_string("S3.xy01", dummy, pin_chgr);

   LiveRangeMap expect = vf.prepare_live_range_map();

   expect.set_life_range(*array->element(0, nullptr, 0), 1, 5);
   expect.set_life_range(*array->element(0, nullptr, 1), 1, 5);
   expect.set_life_range(*array->element(1, nullptr, 0), 1, 4);

   expect.set_life_range(*array->element(1, nullptr, 1), 2, 4);

   expect.set_life_range(*s1, 2, 3);

   expect.set_life_range(*s2x, 5, 6);
   expect.set_life_range(*s2y, 5, 6);

   expect.set_life_range(*s3[0], 6, 7);
   expect.set_life_range(*s3[1], 6, 7);

   check(shader_with_dest_array2_scheduled, expect);
}

void
LiveRangeTests::check(const char *shader, LiveRangeMap& expect)
{
   auto sh = from_string(shader);
   ASSERT_TRUE(sh);

   LiveRangeEvaluator eval;

   LiveRangeMap eval_map = eval.run(*sh);

   ostringstream eval_str;
   eval_str << eval_map;

   ostringstream expect_str;
   expect_str << expect;

   EXPECT_EQ(eval_str.str(), expect_str.str());
}

} // namespace r600
