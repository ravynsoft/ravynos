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

#include "nv50_ir.h"
#include "nv50_ir_build_util.h"

/* On nvc0, surface info is obtained via the surface binding points passed
 * to the SULD/SUST instructions.
 * On nve4, surface info is stored in c[] and is used by various special
 * instructions, e.g. for clamping coordinates or generating an address.
 * They couldn't just have added an equivalent to TIC now, couldn't they ?
 */
#define NVC0_SU_INFO_ADDR   0x00
#define NVC0_SU_INFO_FMT    0x04
#define NVC0_SU_INFO_DIM_X  0x08
#define NVC0_SU_INFO_PITCH  0x0c
#define NVC0_SU_INFO_DIM_Y  0x10
#define NVC0_SU_INFO_ARRAY  0x14
#define NVC0_SU_INFO_DIM_Z  0x18
#define NVC0_SU_INFO_UNK1C  0x1c
#define NVC0_SU_INFO_WIDTH  0x20
#define NVC0_SU_INFO_HEIGHT 0x24
#define NVC0_SU_INFO_DEPTH  0x28
#define NVC0_SU_INFO_TARGET 0x2c
#define NVC0_SU_INFO_BSIZE  0x30
#define NVC0_SU_INFO_RAW_X  0x34
#define NVC0_SU_INFO_MS_X   0x38
#define NVC0_SU_INFO_MS_Y   0x3c

#define NVC0_SU_INFO__STRIDE 0x40

#define NVC0_SU_INFO_DIM(i)  (0x08 + (i) * 8)
#define NVC0_SU_INFO_SIZE(i) (0x20 + (i) * 4)
#define NVC0_SU_INFO_MS(i)   (0x38 + (i) * 4)

namespace nv50_ir {

class NVC0LegalizeSSA : public Pass
{
protected:
   using Pass::visit;
   bool visit(BasicBlock *) override;
   bool visit(Function *) override;

   // we want to insert calls to the builtin library only after optimization
   void handleDIV(Instruction *); // integer division, modulus
   void handleRCPRSQLib(Instruction *, Value *[]);
   void handleRCPRSQ(Instruction *); // double precision float recip/rsqrt
   void handleSET(CmpInstruction *);
   void handleTEXLOD(TexInstruction *);
   void handleShift(Instruction *);
   void handleBREV(Instruction *);

   void handleFTZ(Instruction *);

   BuildUtil bld;
};

class NVC0LegalizePostRA : public Pass
{
public:
   NVC0LegalizePostRA(const Program *);

protected:
   using Pass::visit;
   bool visit(Function *) override;
   bool visit(BasicBlock *) override;

private:
   void replaceCvt(Instruction *);
   void replaceZero(Instruction *);
   bool tryReplaceContWithBra(BasicBlock *);
   void propagateJoin(BasicBlock *);

   struct TexUse
   {
      TexUse(Instruction *use, const Instruction *tex, bool after)
         : insn(use), tex(tex), after(after), level(-1) { }
      Instruction *insn;
      const Instruction *tex; // or split / mov
      bool after;
      int level;
   };
   struct Limits
   {
      Limits() : min(0), max(0) { }
      Limits(int min, int max) : min(min), max(max) { }
      int min, max;
   };
   bool insertTextureBarriers(Function *);
   inline bool insnDominatedBy(const Instruction *, const Instruction *) const;
   void findFirstUses(Instruction *texi, std::list<TexUse> &uses);
   void findFirstUsesBB(int minGPR, int maxGPR, Instruction *start,
                        const Instruction *texi, std::list<TexUse> &uses,
                        std::unordered_set<const BasicBlock *> &visited);
   void addTexUse(std::list<TexUse>&, Instruction *, const Instruction *);
   const Instruction *recurseDef(const Instruction *);

private:
   LValue *rZero;
   LValue *carry;
   LValue *pOne;
   const bool needTexBar;
};

class NVC0LoweringPass : public Pass
{
public:
   NVC0LoweringPass(Program *);

protected:
   bool handleRDSV(Instruction *);
   bool handleEXPORT(Instruction *);
   bool handleOUT(Instruction *);
   bool handleDIV(Instruction *);
   bool handleMOD(Instruction *);
   bool handleSQRT(Instruction *);
   bool handleTEX(TexInstruction *);
   bool handleTXD(TexInstruction *);
   bool handleTXQ(TexInstruction *);
   virtual bool handleManualTXD(TexInstruction *);
   bool handleTXLQ(TexInstruction *);
   bool handleSUQ(TexInstruction *);
   bool handleATOM(Instruction *);
   bool handleATOMCctl(Instruction *);
   bool handleCasExch(Instruction *);
   void handleSurfaceOpGM107(TexInstruction *);
   void handleSurfaceOpNVE4(TexInstruction *);
   void handleSurfaceOpNVC0(TexInstruction *);
   void handleSharedATOM(Instruction *);
   void handleSharedATOMNVE4(Instruction *);
   void handleLDST(Instruction *);
   bool handleBUFQ(Instruction *);
   void handlePIXLD(Instruction *);

   void checkPredicate(Instruction *);
   Value *loadMsAdjInfo32(TexInstruction::Target targ, uint32_t index, int slot, Value *ind, bool bindless);

   bool visit(Instruction *) override;
   bool visit(Function *) override;
   bool visit(BasicBlock *) override;

private:
   void readTessCoord(LValue *dst, int c);

   Value *loadResInfo32(Value *ptr, uint32_t off, uint16_t base);
   Value *loadResInfo64(Value *ptr, uint32_t off, uint16_t base);
   Value *loadResLength32(Value *ptr, uint32_t off, uint16_t base);
   Value *loadSuInfo32(Value *ptr, int slot, uint32_t off, bool bindless);
   Value *loadBufInfo64(Value *ptr, uint32_t off);
   Value *loadBufLength32(Value *ptr, uint32_t off);
   Value *loadUboInfo64(Value *ptr, uint32_t off);
   Value *loadUboLength32(Value *ptr, uint32_t off);
   Value *loadMsInfo32(Value *ptr, uint32_t off);

   void adjustCoordinatesMS(TexInstruction *);
   TexInstruction *processSurfaceCoordsGM107(TexInstruction *, Instruction *[4]);
   void processSurfaceCoordsNVE4(TexInstruction *);
   void processSurfaceCoordsNVC0(TexInstruction *);
   void convertSurfaceFormat(TexInstruction *, Instruction **);
   void insertOOBSurfaceOpResult(TexInstruction *);
   Value *calculateSampleOffset(Value *sampleID);

protected:
   Value *loadTexHandle(Value *ptr, unsigned int slot);

   BuildUtil bld;

private:
   const Target *const targ;

   LValue *gpEmitAddress;
};

} // namespace nv50_ir
