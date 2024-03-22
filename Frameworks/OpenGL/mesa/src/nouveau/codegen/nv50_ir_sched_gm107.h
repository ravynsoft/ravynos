#ifndef __NV50_IR_SCHED_GM107_H__
#define __NV50_IR_SCHED_GM107_H__
namespace nv50_ir {

class SchedDataCalculatorGM107 : public Pass
{
public:
   SchedDataCalculatorGM107(const TargetGM107 *targ) : score(NULL), targ(targ) {}

private:
   struct RegScores
   {
      struct ScoreData {
         int r[256];
         int p[8];
         int c;
      } rd, wr;
      int base;

      void rebase(const int base)
      {
         const int delta = this->base - base;
         if (!delta)
            return;
         this->base = 0;

         for (int i = 0; i < 256; ++i) {
            rd.r[i] += delta;
            wr.r[i] += delta;
         }
         for (int i = 0; i < 8; ++i) {
            rd.p[i] += delta;
            wr.p[i] += delta;
         }
         rd.c += delta;
         wr.c += delta;
      }
      void wipe()
      {
         memset(&rd, 0, sizeof(rd));
         memset(&wr, 0, sizeof(wr));
      }
      int getLatest(const ScoreData& d) const
      {
         int max = 0;
         for (int i = 0; i < 256; ++i)
            if (d.r[i] > max)
               max = d.r[i];
         for (int i = 0; i < 8; ++i)
            if (d.p[i] > max)
               max = d.p[i];
         if (d.c > max)
            max = d.c;
         return max;
      }
      inline int getLatestRd() const
      {
         return getLatest(rd);
      }
      inline int getLatestWr() const
      {
         return getLatest(wr);
      }
      inline int getLatest() const
      {
         return MAX2(getLatestRd(), getLatestWr());
      }
      void setMax(const RegScores *that)
      {
         for (int i = 0; i < 256; ++i) {
            rd.r[i] = MAX2(rd.r[i], that->rd.r[i]);
            wr.r[i] = MAX2(wr.r[i], that->wr.r[i]);
         }
         for (int i = 0; i < 8; ++i) {
            rd.p[i] = MAX2(rd.p[i], that->rd.p[i]);
            wr.p[i] = MAX2(wr.p[i], that->wr.p[i]);
         }
         rd.c = MAX2(rd.c, that->rd.c);
         wr.c = MAX2(wr.c, that->wr.c);
      }
      void print(int cycle)
      {
         for (int i = 0; i < 256; ++i) {
            if (rd.r[i] > cycle)
               INFO("rd $r%i @ %i\n", i, rd.r[i]);
            if (wr.r[i] > cycle)
               INFO("wr $r%i @ %i\n", i, wr.r[i]);
         }
         for (int i = 0; i < 8; ++i) {
            if (rd.p[i] > cycle)
               INFO("rd $p%i @ %i\n", i, rd.p[i]);
            if (wr.p[i] > cycle)
               INFO("wr $p%i @ %i\n", i, wr.p[i]);
         }
         if (rd.c > cycle)
            INFO("rd $c @ %i\n", rd.c);
         if (wr.c > cycle)
            INFO("wr $c @ %i\n", wr.c);
      }
   };

   RegScores *score; // for current BB
   std::vector<RegScores> scoreBoards;

   const TargetGM107 *targ;
   bool visit(Function *);
   bool visit(BasicBlock *);

   void commitInsn(const Instruction *, int);
   int calcDelay(const Instruction *, int) const;
   void setDelay(Instruction *, int, const Instruction *);
   void recordWr(const Value *, int, int);
   void checkRd(const Value *, int, int&) const;

   inline void emitYield(Instruction *);
   inline void emitStall(Instruction *, uint8_t);
   inline void emitReuse(Instruction *, uint8_t);
   inline void emitWrDepBar(Instruction *, uint8_t);
   inline void emitRdDepBar(Instruction *, uint8_t);
   inline void emitWtDepBar(Instruction *, uint8_t);

   inline int getStall(const Instruction *) const;
   inline int getWrDepBar(const Instruction *) const;
   inline int getRdDepBar(const Instruction *) const;
   inline int getWtDepBar(const Instruction *) const;

   void setReuseFlag(Instruction *);

   inline void printSchedInfo(int, const Instruction *) const;

   struct LiveBarUse {
      LiveBarUse(Instruction *insn, Instruction *usei)
         : insn(insn), usei(usei) { }
      Instruction *insn;
      Instruction *usei;
   };

   struct LiveBarDef {
      LiveBarDef(Instruction *insn, Instruction *defi)
         : insn(insn), defi(defi) { }
      Instruction *insn;
      Instruction *defi;
   };

   bool insertBarriers(BasicBlock *);

   bool doesInsnWriteTo(const Instruction *insn, const Value *val) const;
   Instruction *findFirstUse(const Instruction *) const;
   Instruction *findFirstDef(const Instruction *) const;

   bool needRdDepBar(const Instruction *) const;
   bool needWrDepBar(const Instruction *) const;
};

}; // namespace nv50_ir
#endif
