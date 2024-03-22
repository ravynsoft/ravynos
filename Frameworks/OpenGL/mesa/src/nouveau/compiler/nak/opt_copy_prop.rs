// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::ir::*;

use std::collections::HashMap;

struct CopyEntry {
    src_type: SrcType,
    src: Src,
}

struct CopyPropPass {
    ssa_map: HashMap<SSAValue, CopyEntry>,
}

impl CopyPropPass {
    pub fn new() -> CopyPropPass {
        CopyPropPass {
            ssa_map: HashMap::new(),
        }
    }

    fn add_copy(&mut self, dst: SSAValue, src_type: SrcType, src: Src) {
        assert!(src.src_ref.get_reg().is_none());
        self.ssa_map.insert(
            dst,
            CopyEntry {
                src_type: src_type,
                src: src,
            },
        );
    }

    fn add_fp64_copy(&mut self, dst: &SSARef, src: Src) {
        assert!(dst.comps() == 2);
        match src.src_ref {
            SrcRef::Zero | SrcRef::Imm32(_) => {
                self.add_copy(dst[0], SrcType::ALU, Src::new_zero());
                self.add_copy(dst[1], SrcType::F64, src);
            }
            SrcRef::CBuf(cb) => {
                let lo32 = Src::from(SrcRef::CBuf(cb));
                let hi32 = Src {
                    src_ref: SrcRef::CBuf(cb.offset(4)),
                    src_mod: src.src_mod,
                };
                self.add_copy(dst[0], SrcType::ALU, lo32);
                self.add_copy(dst[1], SrcType::F64, hi32);
            }
            SrcRef::SSA(ssa) => {
                assert!(ssa.comps() == 2);
                let lo32 = Src::from(ssa[0]);
                let hi32 = Src {
                    src_ref: ssa[1].into(),
                    src_mod: src.src_mod,
                };
                self.add_copy(dst[0], SrcType::ALU, lo32);
                self.add_copy(dst[1], SrcType::F64, hi32);
            }
            _ => (),
        }
    }

    fn get_copy(&self, dst: &SSAValue) -> Option<&CopyEntry> {
        self.ssa_map.get(dst)
    }

    fn prop_to_pred(&self, pred: &mut Pred) {
        loop {
            let src_ssa = match &pred.pred_ref {
                PredRef::SSA(ssa) => ssa,
                _ => return,
            };

            let entry = match self.get_copy(&src_ssa) {
                Some(e) => e,
                None => return,
            };

            match entry.src.src_ref {
                SrcRef::True => {
                    pred.pred_ref = PredRef::None;
                }
                SrcRef::False => {
                    pred.pred_ref = PredRef::None;
                    pred.pred_inv = !pred.pred_inv;
                }
                SrcRef::SSA(ssa) => {
                    assert!(ssa.comps() == 1);
                    pred.pred_ref = PredRef::SSA(ssa[0]);
                }
                _ => return,
            }

            match entry.src.src_mod {
                SrcMod::None => (),
                SrcMod::BNot => {
                    pred.pred_inv = !pred.pred_inv;
                }
                _ => panic!("Invalid predicate modifier"),
            }
        }
    }

    fn prop_to_ssa_ref(&self, src_ssa: &mut SSARef) -> bool {
        let mut progress = false;

        for c in 0..src_ssa.comps() {
            let c_ssa = &mut src_ssa[usize::from(c)];
            let entry = match self.get_copy(c_ssa) {
                Some(e) => e,
                None => continue,
            };

            if entry.src.src_mod.is_none() {
                if let SrcRef::SSA(entry_ssa) = entry.src.src_ref {
                    assert!(entry_ssa.comps() == 1);
                    *c_ssa = entry_ssa[0];
                    progress = true;
                }
            }
        }

        progress
    }

    fn prop_to_ssa_src(&self, src: &mut Src) {
        assert!(src.src_mod.is_none());
        if let SrcRef::SSA(src_ssa) = &mut src.src_ref {
            loop {
                if !self.prop_to_ssa_ref(src_ssa) {
                    break;
                }
            }
        }
    }

    fn prop_to_gpr_src(&self, src: &mut Src) {
        loop {
            let src_ssa = match &mut src.src_ref {
                SrcRef::SSA(ssa) => {
                    // First, try to propagate SSA components
                    if self.prop_to_ssa_ref(ssa) {
                        continue;
                    }
                    ssa
                }
                _ => return,
            };

            for c in 0..usize::from(src_ssa.comps()) {
                let entry = match self.get_copy(&src_ssa[c]) {
                    Some(e) => e,
                    None => return,
                };

                match entry.src.src_ref {
                    SrcRef::Zero | SrcRef::Imm32(0) => (),
                    _ => return,
                }
            }

            // If we got here, all the components are zero
            src.src_ref = SrcRef::Zero;
        }
    }

    fn prop_to_scalar_src(&self, src_type: SrcType, src: &mut Src) {
        loop {
            let src_ssa = match &src.src_ref {
                SrcRef::SSA(ssa) => ssa,
                _ => return,
            };

            assert!(src_ssa.comps() == 1);
            let entry = match self.get_copy(&src_ssa[0]) {
                Some(e) => e,
                None => return,
            };

            // If there are modifiers, the source types have to match
            if !entry.src.src_mod.is_none() && entry.src_type != src_type {
                return;
            }

            src.src_ref = entry.src.src_ref;
            src.src_mod = entry.src.src_mod.modify(src.src_mod);
        }
    }

    fn prop_to_f64_src(&self, src: &mut Src) {
        loop {
            let src_ssa = match &mut src.src_ref {
                SrcRef::SSA(ssa) => ssa,
                _ => return,
            };

            assert!(src_ssa.comps() == 2);

            // First, try to propagate the two halves individually.  Source
            // modifiers only apply to the high 32 bits so we have to reject
            // any copies with source modifiers in the low bits and apply
            // source modifiers as needed when propagating the high bits.
            let lo_entry_or_none = self.get_copy(&src_ssa[0]);
            if let Some(lo_entry) = lo_entry_or_none {
                if lo_entry.src.src_mod.is_none() {
                    if let SrcRef::SSA(lo_entry_ssa) = lo_entry.src.src_ref {
                        src_ssa[0] = lo_entry_ssa[0];
                        continue;
                    }
                }
            }

            let hi_entry_or_none = self.get_copy(&src_ssa[1]);
            if let Some(hi_entry) = hi_entry_or_none {
                if hi_entry.src.src_mod.is_none()
                    || hi_entry.src_type == SrcType::F64
                {
                    if let SrcRef::SSA(hi_entry_ssa) = hi_entry.src.src_ref {
                        src_ssa[1] = hi_entry_ssa[0];
                        src.src_mod = hi_entry.src.src_mod.modify(src.src_mod);
                        continue;
                    }
                }
            }

            let Some(lo_entry) = lo_entry_or_none else {
                return;
            };

            let Some(hi_entry) = hi_entry_or_none else {
                return;
            };

            if !lo_entry.src.src_mod.is_none() {
                return;
            }

            if !hi_entry.src.src_mod.is_none()
                && hi_entry.src_type != SrcType::F64
            {
                return;
            }

            let new_src_ref = match hi_entry.src.src_ref {
                SrcRef::Zero => match lo_entry.src.src_ref {
                    SrcRef::Zero | SrcRef::Imm32(0) => SrcRef::Zero,
                    _ => return,
                },
                SrcRef::Imm32(i) => {
                    // 32-bit immediates for f64 srouces are the top 32 bits
                    // with zero in the lower 32.
                    match lo_entry.src.src_ref {
                        SrcRef::Zero | SrcRef::Imm32(0) => SrcRef::Imm32(i),
                        _ => return,
                    }
                }
                SrcRef::CBuf(hi_cb) => match lo_entry.src.src_ref {
                    SrcRef::CBuf(lo_cb) => {
                        if hi_cb.buf != lo_cb.buf {
                            return;
                        }
                        if lo_cb.offset % 8 != 0 {
                            return;
                        }
                        if hi_cb.offset != lo_cb.offset + 4 {
                            return;
                        }
                        SrcRef::CBuf(lo_cb)
                    }
                    _ => return,
                },
                // SrcRef::SSA is already handled above
                _ => return,
            };

            src.src_ref = new_src_ref;
            src.src_mod = hi_entry.src.src_mod.modify(src.src_mod);
        }
    }

    fn prop_to_src(&self, src_type: SrcType, src: &mut Src) {
        match src_type {
            SrcType::SSA => {
                self.prop_to_ssa_src(src);
            }
            SrcType::GPR => {
                self.prop_to_gpr_src(src);
            }
            SrcType::ALU
            | SrcType::F32
            | SrcType::I32
            | SrcType::B32
            | SrcType::Pred => {
                self.prop_to_scalar_src(src_type, src);
            }
            SrcType::F64 => {
                self.prop_to_f64_src(src);
            }
            SrcType::Bar => (),
        }
    }

    fn try_add_instr(&mut self, instr: &Instr) {
        match &instr.op {
            Op::FAdd(add) => {
                let dst = add.dst.as_ssa().unwrap();
                assert!(dst.comps() == 1);
                let dst = dst[0];

                if !add.saturate {
                    if add.srcs[0].is_fneg_zero(SrcType::F32) {
                        self.add_copy(dst, SrcType::F32, add.srcs[1]);
                    } else if add.srcs[1].is_fneg_zero(SrcType::F32) {
                        self.add_copy(dst, SrcType::F32, add.srcs[0]);
                    }
                }
            }
            Op::DAdd(add) => {
                let dst = add.dst.as_ssa().unwrap();
                if add.srcs[0].is_fneg_zero(SrcType::F64) {
                    self.add_fp64_copy(dst, add.srcs[1]);
                } else if add.srcs[1].is_fneg_zero(SrcType::F64) {
                    self.add_fp64_copy(dst, add.srcs[0]);
                }
            }
            Op::Lop3(lop) => {
                let dst = lop.dst.as_ssa().unwrap();
                assert!(dst.comps() == 1);
                let dst = dst[0];

                let op = lop.op;
                if op.lut == 0 {
                    self.add_copy(dst, SrcType::ALU, SrcRef::Zero.into());
                } else if op.lut == !0 {
                    self.add_copy(
                        dst,
                        SrcType::ALU,
                        SrcRef::Imm32(u32::MAX).into(),
                    );
                } else {
                    for s in 0..3 {
                        if op.lut == LogicOp3::SRC_MASKS[s] {
                            self.add_copy(dst, SrcType::ALU, lop.srcs[s]);
                        }
                    }
                }
            }
            Op::PLop3(lop) => {
                for i in 0..2 {
                    let dst = match lop.dsts[i] {
                        Dst::SSA(vec) => {
                            assert!(vec.comps() == 1);
                            vec[0]
                        }
                        _ => continue,
                    };

                    let op = lop.ops[i];
                    if op.lut == 0 {
                        self.add_copy(dst, SrcType::Pred, SrcRef::False.into());
                    } else if op.lut == !0 {
                        self.add_copy(dst, SrcType::Pred, SrcRef::True.into());
                    } else {
                        for s in 0..3 {
                            if op.lut == LogicOp3::SRC_MASKS[s] {
                                self.add_copy(dst, SrcType::Pred, lop.srcs[s]);
                            } else if op.lut == !LogicOp3::SRC_MASKS[s] {
                                self.add_copy(
                                    dst,
                                    SrcType::Pred,
                                    lop.srcs[s].bnot(),
                                );
                            }
                        }
                    }
                }
            }
            Op::INeg(neg) => {
                let dst = neg.dst.as_ssa().unwrap();
                assert!(dst.comps() == 1);
                self.add_copy(dst[0], SrcType::I32, neg.src.ineg());
            }
            Op::Prmt(prmt) => {
                let dst = prmt.dst.as_ssa().unwrap();
                assert!(dst.comps() == 1);
                if prmt.mode != PrmtMode::Index {
                    return;
                }
                let SrcRef::Imm32(sel) = prmt.sel.src_ref else {
                    return;
                };

                if sel == 0x3210 {
                    self.add_copy(dst[0], SrcType::GPR, prmt.srcs[0]);
                } else if sel == 0x7654 {
                    self.add_copy(dst[0], SrcType::GPR, prmt.srcs[1]);
                } else {
                    let mut is_imm = true;
                    let mut imm = 0_u32;
                    for d in 0..4 {
                        let s = ((sel >> d * 4) & 0x7) as usize;
                        let sign = (sel >> d * 4) & 0x8 != 0;
                        if let Some(u) = prmt.srcs[s / 4].as_u32() {
                            let mut sb = (u >> (s * 8)) as u8;
                            if sign {
                                sb = ((sb as i8) >> 7) as u8;
                            }
                            imm |= (sb as u32) << (d * 8);
                        } else {
                            is_imm = false;
                            break;
                        }
                    }
                    if is_imm {
                        self.add_copy(dst[0], SrcType::GPR, imm.into());
                    }
                }
            }
            Op::Copy(copy) => {
                let dst = copy.dst.as_ssa().unwrap();
                assert!(dst.comps() == 1);
                self.add_copy(dst[0], SrcType::GPR, copy.src);
            }
            Op::ParCopy(pcopy) => {
                for (dst, src) in pcopy.dsts_srcs.iter() {
                    let dst = dst.as_ssa().unwrap();
                    assert!(dst.comps() == 1);
                    self.add_copy(dst[0], SrcType::GPR, *src);
                }
            }
            _ => (),
        }
    }

    pub fn run(&mut self, f: &mut Function) {
        for b in &mut f.blocks {
            for instr in &mut b.instrs {
                self.try_add_instr(instr);

                self.prop_to_pred(&mut instr.pred);

                let src_types = instr.src_types();
                for (i, src) in instr.srcs_mut().iter_mut().enumerate() {
                    self.prop_to_src(src_types[i], src);
                }
            }
        }
    }
}

impl Shader {
    pub fn opt_copy_prop(&mut self) {
        for f in &mut self.functions {
            CopyPropPass::new().run(f);
        }
    }
}
