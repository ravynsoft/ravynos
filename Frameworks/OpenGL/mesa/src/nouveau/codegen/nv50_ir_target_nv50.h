/*
 * Copyright 2011 Christoph Bumiller
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
 */

#include "nv50_ir_target.h"

namespace nv50_ir {


class TargetNV50 : public Target
{
public:
   TargetNV50(unsigned int chipset);

   virtual CodeEmitter *getCodeEmitter(Program::Type);

   virtual bool runLegalizePass(Program *, CGStage stage) const;

   virtual void getBuiltinCode(const uint32_t **code, uint32_t *size) const;

   virtual void parseDriverInfo(const struct nv50_ir_prog_info *,
                                const struct nv50_ir_prog_info_out *);

   virtual bool insnCanLoad(const Instruction *insn, int s,
                            const Instruction *ld) const;
   virtual bool insnCanLoadOffset(const Instruction *insn, int s,
                                  int offset) const;
   virtual bool isOpSupported(operation, DataType) const;
   virtual bool isAccessSupported(DataFile, DataType) const;
   virtual bool isModSupported(const Instruction *, int s, Modifier) const;
   virtual bool isSatSupported(const Instruction *) const;
   virtual bool mayPredicate(const Instruction *, const Value *) const;

   virtual int getLatency(const Instruction *) const;
   virtual int getThroughput(const Instruction *) const;

   virtual unsigned int getFileSize(DataFile) const;
   virtual unsigned int getFileUnit(DataFile) const;

   virtual uint32_t getSVAddress(DataFile shaderFile, const Symbol *sv) const;

   uint32_t getBuiltinOffset(int builtin) const;

private:
   void initOpInfo();

   uint16_t sysvalLocation[SV_LAST + 1];
   uint8_t wposMask;
};

} // namespace nv50_ir
