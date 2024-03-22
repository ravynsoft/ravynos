
#include "../sfn_instr_alu.h"
#include "../sfn_instr_export.h"
#include "../sfn_instr_tex.h"
#include "../sfn_instrfactory.h"

#include "gtest/gtest.h"
#include <sstream>

using namespace r600;

using std::istringstream;
using std::string;
using std::vector;

class TestShaderFromString : public ::testing::Test {
public:
   void SetUp() override
   {
      init_pool();
      m_instr_factory = new InstrFactory();
   }

   void TearDown() override { release_pool(); }

   TestShaderFromString();

   std::vector<PInst> from_string(const std::string& s);

protected:
   void check(const vector<PInst>& eval,
              const std::vector<PInst, Allocator<PInst>>& expect);

private:
   InstrFactory *m_instr_factory = nullptr;
};

TEST_F(TestShaderFromString, test_simple_fs)
{
   auto init_str =
      R"(

# load constant color
ALU MOV R2000.x@group : L[0x38000000] {W}
ALU MOV R2000.y@group : L[0x0] {W}
ALU MOV R2000.z@group : L[0x0] {W}
ALU MOV R2000.w@group : L[0x38F00000] {WL}

# write output
EXPORT_DONE PIXEL 0 R2000.xyzw
)";

   auto shader = from_string(init_str);

   std::vector<PInst, Allocator<PInst>> expect;

   expect.push_back(new AluInstr(op1_mov,
                                 new Register(2000, 0, pin_group),
                                 new LiteralConstant(0x38000000),
                                 {alu_write}));

   expect.push_back(new AluInstr(
      op1_mov, new Register(2000, 1, pin_group), new LiteralConstant(0x0), {alu_write}));

   expect.push_back(new AluInstr(
      op1_mov, new Register(2000, 2, pin_group), new LiteralConstant(0x0), {alu_write}));

   expect.push_back(new AluInstr(op1_mov,
                                 new Register(2000, 3, pin_group),
                                 new LiteralConstant(0x38F00000),
                                 {alu_write, alu_last_instr}));

   auto exp = new ExportInstr(ExportInstr::pixel, 0, RegisterVec4(2000, false));
   exp->set_is_last_export(true);
   expect.push_back(exp);

   check(shader, expect);
}

TestShaderFromString::TestShaderFromString() {}

std::vector<PInst>
TestShaderFromString::from_string(const std::string& s)
{
   istringstream is(s);
   string line;

   std::vector<PInst> shader;

   while (std::getline(is, line)) {
      if (line.find_first_not_of(" \t") == std::string::npos)
         continue;
      if (line[0] == '#')
         continue;

      shader.push_back(m_instr_factory->from_string(line, 0, false));
   }

   return shader;
}

void
TestShaderFromString::check(const vector<PInst>& eval,
                            const std::vector<PInst, Allocator<PInst>>& expect)
{
   ASSERT_EQ(eval.size(), expect.size());

   for (unsigned i = 0; i < eval.size(); ++i) {
      EXPECT_EQ(*eval[i], *expect[i]);
   }
}
