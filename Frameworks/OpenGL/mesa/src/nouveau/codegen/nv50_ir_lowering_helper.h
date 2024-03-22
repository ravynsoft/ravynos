/*
 * Copyright 2018 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Karol Herbst <kherbst@redhat.com>
 */

#ifndef __NV50_IR_LOWERING_HELPER__
#define __NV50_IR_LOWERING_HELPER__

#include "nv50_ir.h"
#include "nv50_ir_build_util.h"

namespace nv50_ir {

class LoweringHelper : public Pass
{
private:
   virtual bool visit(Instruction *);

   BuildUtil bld;
public:
   bool handleABS(Instruction *);
   bool handleCVT(Instruction *);
   bool handleMAXMIN(Instruction *);
   bool handleMOV(Instruction *);
   bool handleNEG(Instruction *);
   bool handleSAT(Instruction *);
   bool handleSLCT(CmpInstruction *);

   bool handleLogOp(Instruction *);
};

} // namespace nv50_ir

#endif // __NV50_IR_LOWERING_HELPER__
