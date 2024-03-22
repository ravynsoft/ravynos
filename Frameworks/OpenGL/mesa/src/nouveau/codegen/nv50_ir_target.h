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

#ifndef __NV50_IR_TARGET_H__
#define __NV50_IR_TARGET_H__

#include "nv50_ir.h"

namespace nv50_ir {

struct RelocInfo;

struct RelocEntry
{
   enum Type
   {
      TYPE_CODE,
      TYPE_BUILTIN,
      TYPE_DATA
   };

   uint32_t data;
   uint32_t mask;
   uint32_t offset;
   int8_t bitPos;
   Type type;

   inline void apply(uint32_t *binary, const RelocInfo *info) const;
};

struct RelocInfo
{
   uint32_t codePos;
   uint32_t libPos;
   uint32_t dataPos;

   uint32_t count;

   RelocEntry entry[0];
};

struct FixupData {
   FixupData(bool force, bool flat, uint8_t alphatest, bool msaa) :
      force_persample_interp(force), flatshade(flat), alphatest(alphatest), msaa(msaa) {}
   bool force_persample_interp;
   bool flatshade;
   uint8_t alphatest;
   bool msaa;
};

struct FixupEntry;
typedef void (*FixupApply)(const FixupEntry*, uint32_t*, const FixupData&);

struct FixupEntry
{
   FixupEntry(FixupApply apply, int ipa, int reg, int loc) :
      apply(apply), ipa(ipa), reg(reg), loc(loc) {}

   FixupApply apply;
   union {
      struct {
         uint32_t ipa:4; // SC mode used to identify colors
         uint32_t reg:8; // The reg used for perspective division
         uint32_t loc:20; // Let's hope we don't have more than 1M-sized shaders
      };
      uint32_t val;
   };
};

struct FixupInfo
{
   uint32_t count;
   FixupEntry entry[0];
};

class CodeEmitter
{
public:
   CodeEmitter(const Target *);
   virtual ~CodeEmitter() { }

   // returns whether the instruction was encodable and written
   virtual bool emitInstruction(Instruction *) = 0;

   virtual uint32_t getMinEncodingSize(const Instruction *) const = 0;

   void setCodeLocation(void *, uint32_t size);
   inline void *getCodeLocation() const { return code; }
   inline uint32_t getCodeSize() const { return codeSize; }

   bool addReloc(RelocEntry::Type, int w, uint32_t data, uint32_t m,
                 int s);

   inline void *getRelocInfo() const { return relocInfo; }

   bool addInterp(int ipa, int reg, FixupApply apply);
   inline void *getFixupInfo() const { return fixupInfo; }

   virtual void prepareEmission(Program *);
   virtual void prepareEmission(Function *);
   virtual void prepareEmission(BasicBlock *);

   void printBinary() const;

protected:
   const Target *targ;

   uint32_t *code;
   uint32_t codeSize;
   uint32_t codeSizeLimit;

   RelocInfo *relocInfo;
   FixupInfo *fixupInfo;
};


enum OpClass
{
   OPCLASS_MOVE          = 0,
   OPCLASS_LOAD          = 1,
   OPCLASS_STORE         = 2,
   OPCLASS_ARITH         = 3,
   OPCLASS_SHIFT         = 4,
   OPCLASS_SFU           = 5,
   OPCLASS_LOGIC         = 6,
   OPCLASS_COMPARE       = 7,
   OPCLASS_CONVERT       = 8,
   OPCLASS_ATOMIC        = 9,
   OPCLASS_TEXTURE       = 10,
   OPCLASS_SURFACE       = 11,
   OPCLASS_FLOW          = 12,
   OPCLASS_PSEUDO        = 14,
   OPCLASS_VECTOR        = 15,
   OPCLASS_BITFIELD      = 16,
   OPCLASS_CONTROL       = 17,
   OPCLASS_OTHER         = 18
};

class Target
{
public:
   Target(bool m, bool j, bool s) : hasJoin(m), joinAnterior(j), hasSWSched(s) { }
   virtual ~Target() { }

   static Target *create(uint32_t chipset);
   static void destroy(Target *);

   // 0x50 and 0x84 to 0xaf for nv50
   // 0xc0 to 0xdf for nvc0
   inline uint32_t getChipset() const { return chipset; }

   virtual CodeEmitter *getCodeEmitter(Program::Type) = 0;

   // Drivers should upload this so we can use it from all programs.
   // The address chosen is supplied to the relocation routine.
   virtual void getBuiltinCode(const uint32_t **code, uint32_t *size) const = 0;

   virtual void parseDriverInfo(const struct nv50_ir_prog_info *info,
                                const struct nv50_ir_prog_info_out *info_out) {
      if (info_out->type == PIPE_SHADER_COMPUTE) {
         threads = info->prop.cp.numThreads[0] *
            info->prop.cp.numThreads[1] *
            info->prop.cp.numThreads[2];
         if (threads == 0)
            threads = info->target >= NVISA_GK104_CHIPSET ? 1024 : 512;
      } else {
         threads = 32; // doesn't matter, just not too big.
      }
   }

   virtual bool runLegalizePass(Program *, CGStage stage) const = 0;

public:
   struct OpInfo
   {
      OpInfo *variants;
      operation op;
      uint16_t srcTypes;
      uint16_t dstTypes;
      uint32_t immdBits;
      uint8_t srcNr;
      uint8_t srcMods[3];
      uint8_t dstMods;
      uint16_t srcFiles[3];
      uint16_t dstFiles;
      unsigned int minEncSize  : 5;
      unsigned int vector      : 1;
      unsigned int predicate   : 1;
      unsigned int commutative : 1;
      unsigned int pseudo      : 1;
      unsigned int flow        : 1;
      unsigned int hasDest     : 1;
      unsigned int terminator  : 1;
   };

   inline const OpInfo& getOpInfo(const Instruction *) const;
   inline const OpInfo& getOpInfo(const operation) const;

   inline DataFile nativeFile(DataFile f) const;

   virtual bool insnCanLoad(const Instruction *insn, int s,
                            const Instruction *ld) const = 0;
   virtual bool insnCanLoadOffset(const Instruction *insn, int s,
                                  int offset) const = 0;
   virtual bool isOpSupported(operation, DataType) const = 0;
   virtual bool isAccessSupported(DataFile, DataType) const = 0;
   virtual bool isModSupported(const Instruction *,
                               int s, Modifier) const = 0;
   virtual bool isSatSupported(const Instruction *) const = 0;
   virtual bool isPostMultiplySupported(operation op, float f,
                                        int& e) const { return false; }
   virtual bool mayPredicate(const Instruction *,
                             const Value *) const = 0;

   // whether @insn can be issued together with @next (order matters)
   virtual bool canDualIssue(const Instruction *insn,
                             const Instruction *next) const { return false; }
   virtual int getLatency(const Instruction *) const { return 1; }
   virtual int getThroughput(const Instruction *) const { return 1; }

   virtual unsigned int getFileSize(DataFile) const = 0;
   virtual unsigned int getFileUnit(DataFile) const = 0;

   virtual uint32_t getSVAddress(DataFile, const Symbol *) const = 0;

public:
   const bool hasJoin;      // true if instructions have a join modifier
   const bool joinAnterior; // true if join is executed before the op
   const bool hasSWSched;   // true if code should provide scheduling data

   static const uint8_t operationSrcNr[];
   static const OpClass operationClass[];

   static inline uint8_t getOpSrcNr(operation op)
   {
      return operationSrcNr[op];
   }
   static inline OpClass getOpClass(operation op)
   {
      return operationClass[op];
   }

protected:
   uint32_t chipset;
   uint32_t threads;

   DataFile nativeFileMap[DATA_FILE_COUNT];

   OpInfo opInfo[OP_LAST + 1];
};

const Target::OpInfo& Target::getOpInfo(const Instruction *insn) const
{
   return opInfo[MIN2(insn->op, OP_LAST)];
}

const Target::OpInfo& Target::getOpInfo(const operation op) const
{
   return opInfo[op];
}

inline DataFile Target::nativeFile(DataFile f) const
{
   return nativeFileMap[f];
}

} // namespace nv50_ir

#endif // __NV50_IR_TARGET_H__
