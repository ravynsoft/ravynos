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
#ifndef __NV50_IR_TARGET_GV100_H__
#define __NV50_IR_TARGET_GV100_H__
#include "nv50_ir_target_gm107.h"

namespace nv50_ir {

class TargetGV100 : public TargetGM107 {
public:
   TargetGV100(unsigned int chipset);

   virtual CodeEmitter *getCodeEmitter(Program::Type);

   virtual bool runLegalizePass(Program *, CGStage stage) const;

   virtual void getBuiltinCode(const uint32_t **code, uint32_t *size) const;
   virtual uint32_t getBuiltinOffset(int builtin) const;

   virtual bool insnCanLoad(const Instruction *, int, const Instruction *) const;
   virtual bool isOpSupported(operation, DataType) const;
   virtual bool isModSupported(const Instruction *, int s, Modifier) const;
   virtual bool isSatSupported(const Instruction *) const;

   virtual bool isBarrierRequired(const Instruction *) const;

private:
   void initOpInfo();
   void initProps(const struct opProperties *, int);
};

};
#endif
