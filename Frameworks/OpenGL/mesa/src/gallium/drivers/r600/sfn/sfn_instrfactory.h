/* -*- mesa-c++  -*-
 *
 * Copyright (c) 2022 Collabora LTD
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

#ifndef INSTRFACTORY_H
#define INSTRFACTORY_H

#include "sfn_instr.h"
#include "sfn_valuefactory.h"

#include <iosfwd>

namespace r600 {

class Shader;
class InstrFactory : public Allocate {
public:
   InstrFactory();

   PInst from_string(const std::string& s, int nesting_depth, bool is_cayman);
   bool from_nir(nir_instr *instr, Shader& shader);
   auto& value_factory() { return m_value_factory; }

private:
   bool load_const(nir_load_const_instr *lc, Shader& shader);
   bool process_jump(nir_jump_instr *instr, Shader& shader);
   bool process_undef(nir_undef_instr *undef, Shader& shader);

   Instr::Pointer export_from_string(std::istream& is, bool is_last);

   ValueFactory m_value_factory;
   AluGroup *group;
};

} // namespace r600

#endif // INSTRFACTORY_H
