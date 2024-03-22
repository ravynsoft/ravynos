#include "nv50_ir_lowering_nvc0.h"

namespace nv50_ir {

class GM107LoweringPass : public NVC0LoweringPass
{
public:
   GM107LoweringPass(Program *p) : NVC0LoweringPass(p) {}
protected:
   using NVC0LoweringPass::visit;
   bool visit(Instruction *) override;

   bool handleManualTXD(TexInstruction *) override;
   bool handleDFDX(Instruction *);
   bool handlePFETCH(Instruction *);
   bool handlePOPCNT(Instruction *);
   bool handleSUQ(TexInstruction *);
};

class GM107LegalizeSSA : public NVC0LegalizeSSA
{
protected:
   using NVC0LegalizeSSA::visit;
   bool visit(Instruction *) override;

   void handlePFETCH(Instruction *);
   void handleLOAD(Instruction *);
   void handleQUADON(Instruction *);
   void handleQUADPOP(Instruction *);
};

} // namespace nv50_ir
