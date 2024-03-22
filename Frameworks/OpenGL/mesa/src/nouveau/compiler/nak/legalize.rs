// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::ir::*;
use crate::liveness::{BlockLiveness, Liveness, SimpleLiveness};

use std::collections::{HashMap, HashSet};

fn src_is_reg(src: &Src) -> bool {
    match src.src_ref {
        SrcRef::Zero | SrcRef::True | SrcRef::False | SrcRef::SSA(_) => true,
        SrcRef::Imm32(_) | SrcRef::CBuf(_) => false,
        SrcRef::Reg(_) => panic!("Not in SSA form"),
    }
}

fn src_as_lop_imm(src: &Src) -> Option<bool> {
    let x = match src.src_ref {
        SrcRef::Zero => false,
        SrcRef::True => true,
        SrcRef::False => false,
        SrcRef::Imm32(i) => {
            if i == 0 {
                false
            } else if i == !0 {
                true
            } else {
                return None;
            }
        }
        _ => return None,
    };
    Some(x ^ src.src_mod.is_bnot())
}

fn fold_lop_src(src: &Src, x: &mut u8) {
    if let Some(i) = src_as_lop_imm(src) {
        *x = if i { !0 } else { 0 };
    }
    if src.src_mod.is_bnot() {
        *x = !*x;
    }
}

fn copy_alu_src(b: &mut impl SSABuilder, src: &mut Src, src_type: SrcType) {
    let val = match src_type {
        SrcType::GPR
        | SrcType::ALU
        | SrcType::F32
        | SrcType::I32
        | SrcType::B32 => b.alloc_ssa(RegFile::GPR, 1),
        SrcType::F64 => b.alloc_ssa(RegFile::GPR, 2),
        SrcType::Pred => b.alloc_ssa(RegFile::Pred, 1),
        _ => panic!("Unknown source type"),
    };

    if val.comps() == 1 {
        b.copy_to(val.into(), src.src_ref.into());
    } else {
        match src.src_ref {
            SrcRef::Imm32(u) => {
                // Immediates go in the top bits
                b.copy_to(val[0].into(), 0.into());
                b.copy_to(val[1].into(), u.into());
            }
            SrcRef::CBuf(cb) => {
                // CBufs load 8B
                b.copy_to(val[0].into(), cb.into());
                b.copy_to(val[1].into(), cb.offset(4).into());
            }
            SrcRef::SSA(vec) => {
                assert!(vec.comps() == 2);
                b.copy_to(val[0].into(), vec[0].into());
                b.copy_to(val[1].into(), vec[1].into());
            }
            _ => panic!("Invalid 64-bit SrcRef"),
        }
    }

    src.src_ref = val.into();
}

fn copy_alu_src_if_cbuf(
    b: &mut impl SSABuilder,
    src: &mut Src,
    src_type: SrcType,
) {
    match src.src_ref {
        SrcRef::CBuf(_) => copy_alu_src(b, src, src_type),
        _ => (),
    }
}

fn copy_alu_src_if_not_reg(
    b: &mut impl SSABuilder,
    src: &mut Src,
    src_type: SrcType,
) {
    if !src_is_reg(&src) {
        copy_alu_src(b, src, src_type);
    }
}

fn copy_alu_src_if_both_not_reg(
    b: &mut impl SSABuilder,
    src1: &Src,
    src2: &mut Src,
    src_type: SrcType,
) {
    if !src_is_reg(&src1) && !src_is_reg(&src2) {
        copy_alu_src(b, src2, src_type);
    }
}

fn swap_srcs_if_not_reg(x: &mut Src, y: &mut Src) -> bool {
    if !src_is_reg(x) && src_is_reg(y) {
        std::mem::swap(x, y);
        true
    } else {
        false
    }
}

fn copy_alu_src_if_i20_overflow(
    b: &mut impl SSABuilder,
    src: &mut Src,
    src_type: SrcType,
) {
    if src.as_imm_not_i20().is_some() {
        copy_alu_src(b, src, src_type);
    }
}

fn copy_alu_src_if_f20_overflow(
    b: &mut impl SSABuilder,
    src: &mut Src,
    src_type: SrcType,
) {
    if src.as_imm_not_f20().is_some() {
        copy_alu_src(b, src, src_type);
    }
}

fn copy_alu_src_if_fabs(
    b: &mut impl SSABuilder,
    src: &mut Src,
    src_type: SrcType,
) {
    if src.src_mod.has_fabs() {
        match src_type {
            SrcType::F32 => {
                let val = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpFAdd {
                    dst: val.into(),
                    srcs: [Src::new_zero().fneg(), *src],
                    saturate: false,
                    rnd_mode: FRndMode::NearestEven,
                    ftz: false,
                });
                *src = val.into();
            }
            SrcType::F64 => {
                let val = b.alloc_ssa(RegFile::GPR, 2);
                b.push_op(OpDAdd {
                    dst: val.into(),
                    srcs: [Src::new_zero().fneg(), *src],
                    rnd_mode: FRndMode::NearestEven,
                });
                *src = val.into();
            }
            _ => panic!("Invalid ffabs srouce type"),
        }
    }
}

fn legalize_sm50_instr(
    b: &mut impl SSABuilder,
    _bl: &impl BlockLiveness,
    _ip: usize,
    instr: &mut Instr,
) {
    match &mut instr.op {
        Op::Shf(op) => {
            copy_alu_src_if_not_reg(b, &mut op.shift, SrcType::GPR);
            copy_alu_src_if_not_reg(b, &mut op.high, SrcType::GPR);
        }
        Op::Shl(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
            copy_alu_src_if_i20_overflow(b, &mut op.shift, SrcType::ALU);
        }
        Op::Shr(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
            copy_alu_src_if_i20_overflow(b, &mut op.shift, SrcType::ALU);
        }
        Op::FAdd(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src1, SrcType::F32);
        }
        Op::FMul(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::F32);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::F32);
        }
        Op::FSet(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::F32);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::F32);
        }
        Op::FSetP(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::F32);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::F32);
        }
        Op::ISetP(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::ALU);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::ALU);
        }
        Op::Lop2(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::ALU);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::ALU);
        }
        Op::PSetP(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::Pred);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::Pred);
            copy_alu_src_if_not_reg(b, &mut op.srcs[2], SrcType::Pred);
        }
        Op::MuFu(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
        }
        Op::DAdd(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
            copy_alu_src_if_f20_overflow(b, src1, SrcType::F64);
        }
        Op::DFma(op) => {
            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            copy_alu_src_if_fabs(b, src0, SrcType::F64);
            copy_alu_src_if_fabs(b, src1, SrcType::F64);
            copy_alu_src_if_fabs(b, src2, SrcType::F64);
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
            copy_alu_src_if_f20_overflow(b, src1, SrcType::F64);
            copy_alu_src_if_not_reg(b, src2, SrcType::F64);
        }
        Op::DMnMx(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
            copy_alu_src_if_f20_overflow(b, src1, SrcType::F64);
        }
        Op::DMul(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            copy_alu_src_if_fabs(b, src0, SrcType::F64);
            copy_alu_src_if_fabs(b, src1, SrcType::F64);
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
            copy_alu_src_if_f20_overflow(b, src1, SrcType::F64);
        }
        Op::DSetP(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            if swap_srcs_if_not_reg(src0, src1) {
                op.cmp_op = op.cmp_op.flip();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
            copy_alu_src_if_f20_overflow(b, src1, SrcType::F64);
        }
        Op::IAbs(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
        }
        Op::Sel(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            if swap_srcs_if_not_reg(src0, src1) {
                op.cond = op.cond.bnot();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
            copy_alu_src_if_i20_overflow(b, src1, SrcType::ALU);
        }
        Op::Shfl(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
            copy_alu_src_if_cbuf(b, &mut op.lane, SrcType::ALU);
            copy_alu_src_if_cbuf(b, &mut op.c, SrcType::ALU);
        }
        Op::Vote(_) => (),
        Op::IAdd2(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::I32);
        }
        Op::I2F(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
        }
        Op::F2F(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
        }
        Op::IMad(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::ALU);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::ALU);
            copy_alu_src_if_not_reg(b, &mut op.srcs[2], SrcType::ALU);
        }
        Op::IMul(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            if swap_srcs_if_not_reg(src0, src1) {
                op.signed.swap(0, 1);
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
        }
        Op::F2I(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
        }
        Op::IMnMx(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::ALU);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::ALU);
        }
        Op::Ipa(op) => {
            copy_alu_src_if_not_reg(b, &mut op.offset, SrcType::GPR);
        }
        Op::PopC(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::ALU);
        }
        Op::Brev(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::ALU);
        }
        Op::FMnMx(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::F32);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::F32);
        }
        Op::Prmt(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::GPR);
            copy_alu_src_if_not_reg(b, &mut op.sel, SrcType::ALU);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::GPR);
        }
        Op::FFma(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::F32);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::F32);
            copy_alu_src_if_not_reg(b, &mut op.srcs[2], SrcType::F32);
        }
        Op::Ldc(_) => (),  // Nothing to do
        Op::Copy(_) => (), // Nothing to do
        Op::INeg(_) => (), /* we unconditionally lower this */
        _ => {
            let src_types = instr.src_types();
            for (i, src) in instr.srcs_mut().iter_mut().enumerate() {
                match src_types[i] {
                    SrcType::SSA => {
                        assert!(src.as_ssa().is_some());
                    }
                    SrcType::GPR => {
                        assert!(src_is_reg(src));
                    }
                    SrcType::ALU
                    | SrcType::F32
                    | SrcType::F64
                    | SrcType::I32
                    | SrcType::B32 => {
                        panic!("ALU srcs must be legalized explicitly");
                    }
                    SrcType::Pred => {
                        panic!("Predicates must be legalized explicitly");
                    }
                    SrcType::Bar => panic!("Barrier regs are Volta+"),
                }
            }
        }
    }
}

fn legalize_sm70_instr(
    b: &mut impl SSABuilder,
    bl: &impl BlockLiveness,
    ip: usize,
    instr: &mut Instr,
) {
    match &mut instr.op {
        Op::FAdd(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F32);
        }
        Op::FFma(op) => {
            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F32);
            copy_alu_src_if_both_not_reg(b, src1, src2, SrcType::F32);
        }
        Op::FMnMx(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F32);
        }
        Op::FMul(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F32);
        }
        Op::FSet(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            if !src_is_reg(src0) && src_is_reg(src1) {
                std::mem::swap(src0, src1);
                op.cmp_op = op.cmp_op.flip();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::F32);
        }
        Op::FSetP(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            if !src_is_reg(src0) && src_is_reg(src1) {
                std::mem::swap(src0, src1);
                op.cmp_op = op.cmp_op.flip();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::F32);
        }
        Op::MuFu(_) => (), // Nothing to do
        Op::DAdd(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
        }
        Op::DFma(op) => {
            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
            copy_alu_src_if_both_not_reg(b, src1, src2, SrcType::F64);
        }
        Op::DMul(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
        }
        Op::DSetP(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            if !src_is_reg(src0) && src_is_reg(src1) {
                std::mem::swap(src0, src1);
                op.cmp_op = op.cmp_op.flip();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::F64);
        }
        Op::Brev(_) | Op::Flo(_) | Op::IAbs(_) | Op::INeg(_) => (),
        Op::IAdd3(op) => {
            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            swap_srcs_if_not_reg(src2, src1);
            if !src0.src_mod.is_none() && !src1.src_mod.is_none() {
                let val = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpIAdd3 {
                    srcs: [Src::new_zero(), *src0, Src::new_zero()],
                    overflow: [Dst::None; 2],
                    dst: val.into(),
                });
                *src0 = val.into();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::I32);
            copy_alu_src_if_both_not_reg(b, src1, src2, SrcType::I32);
        }
        Op::IAdd3X(op) => {
            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            swap_srcs_if_not_reg(src2, src1);
            if !src0.src_mod.is_none() && !src1.src_mod.is_none() {
                let val = b.alloc_ssa(RegFile::GPR, 1);
                b.push_op(OpIAdd3X {
                    srcs: [Src::new_zero(), *src0, Src::new_zero()],
                    overflow: [Dst::None; 2],
                    dst: val.into(),
                    carry: [false.into(); 2],
                });
                *src0 = val.into();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::B32);
            copy_alu_src_if_both_not_reg(b, src1, src2, SrcType::B32);
        }
        Op::IDp4(op) => {
            let [ref mut src_type0, ref mut src_type1] = op.src_types;
            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            if swap_srcs_if_not_reg(src0, src1) {
                std::mem::swap(src_type0, src_type1);
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
            copy_alu_src_if_not_reg(b, src2, SrcType::ALU);
        }
        Op::IMad(op) => {
            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
            copy_alu_src_if_both_not_reg(b, src1, src2, SrcType::ALU);
        }
        Op::IMad64(op) => {
            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
            copy_alu_src_if_both_not_reg(b, src1, src2, SrcType::ALU);
        }
        Op::IMnMx(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            swap_srcs_if_not_reg(src0, src1);
            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
        }
        Op::ISetP(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            if !src_is_reg(src0) && src_is_reg(src1) {
                std::mem::swap(src0, src1);
                op.cmp_op = op.cmp_op.flip();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
        }
        Op::Lop3(op) => {
            // Fold constants and modifiers if we can
            op.op = LogicOp3::new_lut(&|mut x, mut y, mut z| {
                fold_lop_src(&op.srcs[0], &mut x);
                fold_lop_src(&op.srcs[1], &mut y);
                fold_lop_src(&op.srcs[2], &mut z);
                op.op.eval(x, y, z)
            });
            for src in &mut op.srcs {
                src.src_mod = SrcMod::None;
                if src_as_lop_imm(src).is_some() {
                    src.src_ref = SrcRef::Zero;
                }
            }

            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            if !src_is_reg(src0) && src_is_reg(src1) {
                std::mem::swap(src0, src1);
                op.op = LogicOp3::new_lut(&|x, y, z| op.op.eval(y, x, z))
            }
            if !src_is_reg(src2) && src_is_reg(src1) {
                std::mem::swap(src2, src1);
                op.op = LogicOp3::new_lut(&|x, y, z| op.op.eval(x, z, y))
            }

            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
            copy_alu_src_if_not_reg(b, src2, SrcType::ALU);
        }
        Op::PopC(_) => (),
        Op::Shf(op) => {
            copy_alu_src_if_not_reg(b, &mut op.low, SrcType::ALU);
            copy_alu_src_if_not_reg(b, &mut op.high, SrcType::ALU);
        }
        Op::F2F(_) | Op::F2I(_) | Op::I2F(_) | Op::Mov(_) | Op::FRnd(_) => (),
        Op::Prmt(op) => {
            copy_alu_src_if_not_reg(b, &mut op.srcs[0], SrcType::ALU);
            copy_alu_src_if_not_reg(b, &mut op.srcs[1], SrcType::ALU);
        }
        Op::Sel(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            if swap_srcs_if_not_reg(src0, src1) {
                op.cond = op.cond.bnot();
            }
            copy_alu_src_if_not_reg(b, src0, SrcType::ALU);
        }
        Op::PLop3(op) => {
            // Fold constants and modifiers if we can
            for lop in &mut op.ops {
                *lop = LogicOp3::new_lut(&|mut x, mut y, mut z| {
                    fold_lop_src(&op.srcs[0], &mut x);
                    fold_lop_src(&op.srcs[1], &mut y);
                    fold_lop_src(&op.srcs[2], &mut z);
                    lop.eval(x, y, z)
                });
            }
            for src in &mut op.srcs {
                src.src_mod = SrcMod::None;
                if src_as_lop_imm(src).is_some() {
                    src.src_ref = SrcRef::True;
                }
            }

            let [ref mut src0, ref mut src1, ref mut src2] = op.srcs;
            if !src_is_reg(src0) && src_is_reg(src1) {
                std::mem::swap(src0, src1);
                for lop in &mut op.ops {
                    *lop = LogicOp3::new_lut(&|x, y, z| lop.eval(y, x, z));
                }
            }
            if !src_is_reg(src2) && src_is_reg(src1) {
                std::mem::swap(src2, src1);
                for lop in &mut op.ops {
                    *lop = LogicOp3::new_lut(&|x, y, z| lop.eval(x, z, y));
                }
            }

            copy_alu_src_if_not_reg(b, src0, SrcType::Pred);
            copy_alu_src_if_not_reg(b, src2, SrcType::Pred);
        }
        Op::FSwzAdd(op) => {
            let [ref mut src0, ref mut src1] = op.srcs;
            copy_alu_src_if_not_reg(b, src0, SrcType::F32);
            copy_alu_src_if_not_reg(b, src1, SrcType::F32);
        }
        Op::Shfl(op) => {
            copy_alu_src_if_not_reg(b, &mut op.src, SrcType::GPR);
            copy_alu_src_if_cbuf(b, &mut op.lane, SrcType::ALU);
            copy_alu_src_if_cbuf(b, &mut op.c, SrcType::ALU);
        }
        Op::Out(op) => {
            copy_alu_src_if_not_reg(b, &mut op.handle, SrcType::GPR);
            copy_alu_src_if_cbuf(b, &mut op.stream, SrcType::ALU);
        }
        Op::Break(op) => {
            let bar_in = op.bar_in.src_ref.as_ssa().unwrap();
            if !op.bar_out.is_none() && bl.is_live_after_ip(&bar_in[0], ip) {
                let gpr = b.bmov_to_gpr(op.bar_in);
                let tmp = b.bmov_to_bar(gpr.into());
                op.bar_in = tmp.into();
            }
        }
        Op::BSSy(op) => {
            let bar_in = op.bar_in.src_ref.as_ssa().unwrap();
            if !op.bar_out.is_none() && bl.is_live_after_ip(&bar_in[0], ip) {
                let gpr = b.bmov_to_gpr(op.bar_in);
                let tmp = b.bmov_to_bar(gpr.into());
                op.bar_in = tmp.into();
            }
        }
        Op::OutFinal(op) => {
            copy_alu_src_if_not_reg(b, &mut op.handle, SrcType::GPR);
        }
        Op::Ldc(_) => (), // Nothing to do
        Op::BSync(_) => (),
        Op::Vote(_) => (), // Nothing to do
        Op::Copy(_) => (), // Nothing to do
        _ => {
            let src_types = instr.src_types();
            for (i, src) in instr.srcs_mut().iter_mut().enumerate() {
                match src_types[i] {
                    SrcType::SSA => {
                        assert!(src.as_ssa().is_some());
                    }
                    SrcType::GPR => {
                        assert!(src_is_reg(src));
                    }
                    SrcType::ALU
                    | SrcType::F32
                    | SrcType::F64
                    | SrcType::I32
                    | SrcType::B32 => {
                        panic!("ALU srcs must be legalized explicitly");
                    }
                    SrcType::Pred => {
                        panic!("Predicates must be legalized explicitly");
                    }
                    SrcType::Bar => (),
                }
            }
        }
    }
}

fn legalize_instr(
    b: &mut impl SSABuilder,
    bl: &impl BlockLiveness,
    ip: usize,
    instr: &mut Instr,
) {
    if b.sm() >= 70 {
        legalize_sm70_instr(b, bl, ip, instr);
    } else if b.sm() >= 50 {
        legalize_sm50_instr(b, bl, ip, instr);
    } else {
        panic!("Unknown shader model SM{}", b.sm());
    }

    let src_types = instr.src_types();
    for (i, src) in instr.srcs_mut().iter_mut().enumerate() {
        if let SrcRef::Imm32(u) = &mut src.src_ref {
            *u = match src_types[i] {
                SrcType::F32 | SrcType::F64 => match src.src_mod {
                    SrcMod::None => *u,
                    SrcMod::FAbs => *u & !(1_u32 << 31),
                    SrcMod::FNeg => *u ^ !(1_u32 << 31),
                    SrcMod::FNegAbs => *u | !(1_u32 << 31),
                    _ => panic!("Not a float source modifier"),
                },
                SrcType::I32 => match src.src_mod {
                    SrcMod::None => *u,
                    SrcMod::INeg => -(*u as i32) as u32,
                    _ => panic!("Not an integer source modifier"),
                },
                SrcType::B32 => match src.src_mod {
                    SrcMod::None => *u,
                    SrcMod::BNot => !*u,
                    _ => panic!("Not a bitwise source modifier"),
                },
                _ => {
                    assert!(src.src_mod.is_none());
                    *u
                }
            };
            src.src_mod = SrcMod::None;
        }
    }

    let mut vec_src_map: HashMap<SSARef, SSARef> = HashMap::new();
    let mut vec_comps = HashSet::new();
    for src in instr.srcs_mut() {
        if let SrcRef::SSA(vec) = &src.src_ref {
            if vec.comps() == 1 {
                continue;
            }

            // If the same vector shows up twice in one instruction, that's
            // okay. Just make it look the same as the previous source we
            // fixed up.
            if let Some(new_vec) = vec_src_map.get(&vec) {
                src.src_ref = (*new_vec).into();
                continue;
            }

            let mut new_vec = *vec;
            for c in 0..vec.comps() {
                let ssa = vec[usize::from(c)];
                // If the same SSA value shows up in multiple non-identical
                // vector sources or as multiple components in the same
                // source, we need to make a copy so it can get assigned to
                // multiple different registers.
                if vec_comps.get(&ssa).is_some() {
                    let copy = b.alloc_ssa(ssa.file(), 1)[0];
                    b.copy_to(copy.into(), ssa.into());
                    new_vec[usize::from(c)] = copy;
                } else {
                    vec_comps.insert(ssa);
                }
            }

            vec_src_map.insert(*vec, new_vec);
            src.src_ref = new_vec.into();
        }
    }
}

impl Shader {
    pub fn legalize(&mut self) {
        let sm = self.info.sm;
        for f in &mut self.functions {
            let live = SimpleLiveness::for_function(f);

            for (bi, b) in f.blocks.iter_mut().enumerate() {
                let bl = live.block_live(bi);

                let mut instrs = Vec::new();
                for (ip, mut instr) in b.instrs.drain(..).enumerate() {
                    let mut b = SSAInstrBuilder::new(sm, &mut f.ssa_alloc);
                    legalize_instr(&mut b, bl, ip, &mut instr);
                    b.push_instr(instr);
                    instrs.append(&mut b.as_vec());
                }
                b.instrs = instrs;
            }
        }
    }
}
