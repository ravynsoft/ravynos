/*
 * Copyright 2020 Red Hat Inc.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __NV50_IR_LOWERING_GV100_H__
#define __NV50_IR_LOWERING_GV100_H__
#include "nv50_ir_lowering_gm107.h"

namespace nv50_ir {

class GV100LoweringPass : public Pass
{
public:
   GV100LoweringPass(Program *p) {
      bld.setProgram(p);
   }

private:
   BuildUtil bld;

   virtual bool visit(Instruction *);

   bool handleDMNMX(Instruction *);
   bool handleEXTBF(Instruction *);
   bool handleFLOW(Instruction *);
   bool handleI2I(Instruction *);
   bool handleINSBF(Instruction *);
   bool handlePINTERP(Instruction *);
   bool handlePREFLOW(Instruction *);
   bool handlePRESIN(Instruction *);
};

class GV100LegalizeSSA : public GM107LegalizeSSA
{
public:
   GV100LegalizeSSA(Program *p) {
      bld.setProgram(p);
   }

private:
   virtual bool visit(Function *) { return true; }
   virtual bool visit(BasicBlock *) { return true; }
   virtual bool visit(Instruction *);

   bool handleCMP(Instruction *);
   bool handleIADD64(Instruction *);
   bool handleIMAD_HIGH(Instruction *);
   bool handleIMNMX(Instruction *);
   bool handleIMUL(Instruction *);
   bool handleLOP2(Instruction *);
   bool handleNOT(Instruction *);
   bool handlePREEX2(Instruction *);
   bool handleQUADON(Instruction *);
   bool handleQUADPOP(Instruction *);
   bool handleSET(Instruction *);
   bool handleSHFL(Instruction *);
   bool handleShift(Instruction *);
   bool handleSUB(Instruction *);
};
}
#endif
