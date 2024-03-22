// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::ir::*;
use bitview::*;

use std::collections::HashMap;
use std::ops::Range;

struct ALURegRef {
    pub reg: RegRef,
    pub abs: bool,
    pub neg: bool,
}

struct ALUCBufRef {
    pub cb: CBufRef,
    pub abs: bool,
    pub neg: bool,
}

enum ALUSrc {
    None,
    Imm32(u32),
    Reg(ALURegRef),
    UReg(ALURegRef),
    CBuf(ALUCBufRef),
}

fn src_mod_has_abs(src_mod: SrcMod) -> bool {
    match src_mod {
        SrcMod::None | SrcMod::FNeg | SrcMod::INeg | SrcMod::BNot => false,
        SrcMod::FAbs | SrcMod::FNegAbs => true,
    }
}

fn src_mod_has_neg(src_mod: SrcMod) -> bool {
    match src_mod {
        SrcMod::None | SrcMod::FAbs => false,
        SrcMod::FNeg | SrcMod::FNegAbs | SrcMod::INeg | SrcMod::BNot => true,
    }
}

fn src_mod_is_bnot(src_mod: SrcMod) -> bool {
    match src_mod {
        SrcMod::None => false,
        SrcMod::BNot => true,
        _ => panic!("Not an predicate source modifier"),
    }
}

fn dst_is_bar(dst: Dst) -> bool {
    match dst {
        Dst::None => false,
        Dst::SSA(ssa) => ssa.file() == RegFile::Bar,
        Dst::Reg(reg) => reg.file() == RegFile::Bar,
    }
}

impl ALUSrc {
    fn from_src_file(src: &Src, file: RegFile) -> ALUSrc {
        match src.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                let reg = match src.src_ref {
                    SrcRef::Zero => RegRef::zero(file, 1),
                    SrcRef::Reg(reg) => reg,
                    _ => panic!("Invalid source ref"),
                };
                assert!(reg.comps() <= 2);
                assert!(reg.file() == file);
                let alu_ref = ALURegRef {
                    reg: reg,
                    abs: src_mod_has_abs(src.src_mod),
                    neg: src_mod_has_neg(src.src_mod),
                };
                match reg.file() {
                    RegFile::GPR => ALUSrc::Reg(alu_ref),
                    RegFile::UGPR => ALUSrc::UReg(alu_ref),
                    _ => panic!("Invalid ALU register file"),
                }
            }
            SrcRef::Imm32(i) => {
                assert!(src.src_mod.is_none());
                ALUSrc::Imm32(i)
            }
            SrcRef::CBuf(cb) => {
                let alu_ref = ALUCBufRef {
                    cb: cb,
                    abs: src_mod_has_abs(src.src_mod),
                    neg: src_mod_has_neg(src.src_mod),
                };
                ALUSrc::CBuf(alu_ref)
            }
            _ => panic!("Invalid ALU source"),
        }
    }

    pub fn from_src(src: &Src) -> ALUSrc {
        ALUSrc::from_src_file(src, RegFile::GPR)
    }

    #[allow(dead_code)]
    pub fn from_usrc(src: &Src) -> ALUSrc {
        assert!(src.is_uniform());
        ALUSrc::from_src_file(src, RegFile::UGPR)
    }
}

struct SM70Instr {
    inst: [u32; 4],
    sm: u8,
}

impl BitViewable for SM70Instr {
    fn bits(&self) -> usize {
        BitView::new(&self.inst).bits()
    }

    fn get_bit_range_u64(&self, range: Range<usize>) -> u64 {
        BitView::new(&self.inst).get_bit_range_u64(range)
    }
}

impl BitMutViewable for SM70Instr {
    fn set_bit_range_u64(&mut self, range: Range<usize>, val: u64) {
        BitMutView::new(&mut self.inst).set_bit_range_u64(range, val);
    }
}

impl SetFieldU64 for SM70Instr {
    fn set_field_u64(&mut self, range: Range<usize>, val: u64) {
        BitMutView::new(&mut self.inst).set_field_u64(range, val);
    }
}

impl SM70Instr {
    fn set_bit(&mut self, bit: usize, val: bool) {
        BitMutView::new(&mut self.inst).set_bit(bit, val);
    }

    fn set_src_imm(&mut self, range: Range<usize>, u: &u32) {
        assert!(range.len() == 32);
        self.set_field(range, *u);
    }

    fn set_reg(&mut self, range: Range<usize>, reg: RegRef) {
        assert!(range.len() == 8);
        assert!(reg.file() == RegFile::GPR);
        self.set_field(range, reg.base_idx());
    }

    fn set_ureg(&mut self, range: Range<usize>, reg: RegRef) {
        assert!(self.sm >= 75);
        assert!(range.len() == 8);
        assert!(reg.file() == RegFile::UGPR);
        assert!(reg.base_idx() <= 63);
        self.set_field(range, reg.base_idx());
    }

    fn set_pred_reg(&mut self, range: Range<usize>, reg: RegRef) {
        assert!(range.len() == 3);
        assert!(reg.file() == RegFile::Pred);
        assert!(reg.base_idx() <= 7);
        assert!(reg.comps() == 1);
        self.set_field(range, reg.base_idx());
    }

    fn set_reg_src(&mut self, range: Range<usize>, src: Src) {
        assert!(src.src_mod.is_none());
        match src.src_ref {
            SrcRef::Zero => self.set_reg(range, RegRef::zero(RegFile::GPR, 1)),
            SrcRef::Reg(reg) => self.set_reg(range, reg),
            _ => panic!("Not a register"),
        }
    }

    fn set_pred_dst(&mut self, range: Range<usize>, dst: Dst) {
        match dst {
            Dst::None => {
                self.set_pred_reg(range, RegRef::zero(RegFile::Pred, 1));
            }
            Dst::Reg(reg) => self.set_pred_reg(range, reg),
            _ => panic!("Not a register"),
        }
    }

    fn set_pred_src(&mut self, range: Range<usize>, not_bit: usize, src: Src) {
        // The default for predicates is true
        let true_reg = RegRef::new(RegFile::Pred, 7, 1);

        let (not, reg) = match src.src_ref {
            SrcRef::True => (false, true_reg),
            SrcRef::False => (true, true_reg),
            SrcRef::Reg(reg) => (false, reg),
            _ => panic!("Not a register"),
        };
        self.set_pred_reg(range, reg);
        self.set_bit(not_bit, not ^ src_mod_is_bnot(src.src_mod));
    }

    fn set_src_cb(&mut self, range: Range<usize>, cb: &CBufRef) {
        let mut v = BitMutView::new_subset(self, range);
        v.set_field(0..16, cb.offset);
        if let CBuf::Binding(idx) = cb.buf {
            v.set_field(16..21, idx);
        } else {
            panic!("Must be a bound constant buffer");
        }
    }

    #[allow(dead_code)]
    fn set_src_cx(&mut self, range: Range<usize>, cb: &CBufRef) {
        assert!(self.sm >= 75);

        let mut v = BitMutView::new_subset(self, range);
        if let CBuf::BindlessGPR(reg) = cb.buf {
            assert!(reg.base_idx() <= 63);
            assert!(reg.file() == RegFile::UGPR);
            v.set_field(0..8, reg.base_idx());
        } else {
            panic!("Must be a bound constant buffer");
        }
        assert!(cb.offset % 4 == 0);
        v.set_field(8..22, cb.offset / 4);
    }

    fn set_opcode(&mut self, opcode: u16) {
        self.set_field(0..12, opcode);
    }

    fn set_pred(&mut self, pred: &Pred) {
        assert!(!pred.is_false());
        self.set_pred_reg(
            12..15,
            match pred.pred_ref {
                PredRef::None => RegRef::zero(RegFile::Pred, 1),
                PredRef::Reg(reg) => reg,
                PredRef::SSA(_) => panic!("SSA values must be lowered"),
            },
        );
        self.set_bit(15, pred.pred_inv);
    }

    fn set_dst(&mut self, dst: Dst) {
        match dst {
            Dst::None => self.set_reg(16..24, RegRef::zero(RegFile::GPR, 1)),
            Dst::Reg(reg) => self.set_reg(16..24, reg),
            _ => panic!("Not a register"),
        }
    }

    fn set_bar_reg(&mut self, range: Range<usize>, reg: RegRef) {
        assert!(range.len() == 4);
        assert!(reg.file() == RegFile::Bar);
        assert!(reg.comps() == 1);
        self.set_field(range, reg.base_idx());
    }

    fn set_bar_dst(&mut self, range: Range<usize>, dst: Dst) {
        self.set_bar_reg(range, *dst.as_reg().unwrap());
    }

    fn set_bar_src(&mut self, range: Range<usize>, src: Src) {
        assert!(src.src_mod.is_none());
        self.set_bar_reg(range, *src.src_ref.as_reg().unwrap());
    }

    fn set_alu_reg(
        &mut self,
        range: Range<usize>,
        abs_bit: usize,
        neg_bit: usize,
        reg: &ALURegRef,
    ) {
        self.set_reg(range, reg.reg);
        self.set_bit(abs_bit, reg.abs);
        self.set_bit(neg_bit, reg.neg);
    }

    fn set_alu_ureg(
        &mut self,
        range: Range<usize>,
        abs_bit: usize,
        neg_bit: usize,
        reg: &ALURegRef,
    ) {
        self.set_ureg(range, reg.reg);
        self.set_bit(abs_bit, reg.abs);
        self.set_bit(neg_bit, reg.neg);
    }

    fn set_alu_cb(
        &mut self,
        range: Range<usize>,
        abs_bit: usize,
        neg_bit: usize,
        cb: &ALUCBufRef,
    ) {
        self.set_src_cb(range, &cb.cb);
        self.set_bit(abs_bit, cb.abs);
        self.set_bit(neg_bit, cb.neg);
    }

    fn set_alu_reg_src(
        &mut self,
        range: Range<usize>,
        abs_bit: usize,
        neg_bit: usize,
        src: &ALUSrc,
    ) {
        match src {
            ALUSrc::None => (),
            ALUSrc::Reg(reg) => self.set_alu_reg(range, abs_bit, neg_bit, reg),
            _ => panic!("Invalid ALU src0"),
        }
    }

    fn encode_alu(
        &mut self,
        opcode: u16,
        dst: Option<Dst>,
        src0: ALUSrc,
        src1: ALUSrc,
        src2: ALUSrc,
    ) {
        if let Some(dst) = dst {
            self.set_dst(dst);
        }

        self.set_alu_reg_src(24..32, 73, 72, &src0);

        let form = match &src2 {
            ALUSrc::None | ALUSrc::Reg(_) => {
                self.set_alu_reg_src(64..72, 74, 75, &src2);
                match &src1 {
                    ALUSrc::None => 1_u8, // form
                    ALUSrc::Reg(reg1) => {
                        self.set_alu_reg(32..40, 62, 63, reg1);
                        1_u8 // form
                    }
                    ALUSrc::UReg(reg1) => {
                        self.set_alu_ureg(32..40, 62, 63, reg1);
                        6_u8 // form
                    }
                    ALUSrc::Imm32(imm) => {
                        self.set_src_imm(32..64, &imm);
                        4_u8 // form
                    }
                    ALUSrc::CBuf(cb) => {
                        self.set_alu_cb(38..59, 62, 63, cb);
                        5_u8 // form
                    }
                }
            }
            ALUSrc::UReg(reg2) => {
                self.set_alu_ureg(32..40, 62, 63, reg2);
                self.set_alu_reg_src(64..72, 74, 75, &src1);
                7_u8 // form
            }
            ALUSrc::Imm32(imm) => {
                self.set_src_imm(32..64, &imm);
                self.set_alu_reg_src(64..72, 74, 75, &src1);
                2_u8 // form
            }
            ALUSrc::CBuf(cb) => {
                // TODO set_src_cx
                self.set_alu_cb(38..59, 62, 63, cb);
                self.set_alu_reg_src(64..72, 74, 75, &src1);
                3_u8 // form
            }
        };

        self.set_field(0..9, opcode);
        self.set_field(9..12, form);
    }

    fn set_instr_deps(&mut self, deps: &InstrDeps) {
        self.set_field(105..109, deps.delay);
        self.set_bit(109, deps.yld);
        self.set_field(110..113, deps.wr_bar().unwrap_or(7));
        self.set_field(113..116, deps.rd_bar().unwrap_or(7));
        self.set_field(116..122, deps.wt_bar_mask);
        self.set_field(122..126, deps.reuse_mask);
    }

    fn set_rnd_mode(&mut self, range: Range<usize>, rnd_mode: FRndMode) {
        assert!(range.len() == 2);
        self.set_field(
            range,
            match rnd_mode {
                FRndMode::NearestEven => 0_u8,
                FRndMode::NegInf => 1_u8,
                FRndMode::PosInf => 2_u8,
                FRndMode::Zero => 3_u8,
            },
        );
    }

    fn encode_fadd(&mut self, op: &OpFAdd) {
        if op.srcs[1].src_ref.as_reg().is_some() {
            self.encode_alu(
                0x021,
                Some(op.dst),
                ALUSrc::from_src(&op.srcs[0]),
                ALUSrc::from_src(&op.srcs[1]),
                ALUSrc::None,
            );
        } else {
            self.encode_alu(
                0x021,
                Some(op.dst),
                ALUSrc::from_src(&op.srcs[0]),
                ALUSrc::from_src(&Src::new_zero()),
                ALUSrc::from_src(&op.srcs[1]),
            );
        }
        self.set_bit(77, op.saturate);
        self.set_rnd_mode(78..80, op.rnd_mode);
        self.set_bit(80, op.ftz);
    }

    fn encode_ffma(&mut self, op: &OpFFma) {
        self.encode_alu(
            0x023,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&op.srcs[2]),
        );
        self.set_bit(76, op.dnz);
        self.set_bit(77, op.saturate);
        self.set_rnd_mode(78..80, op.rnd_mode);
        self.set_bit(80, op.ftz);
    }

    fn encode_fmnmx(&mut self, op: &OpFMnMx) {
        self.encode_alu(
            0x009,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&Src::new_zero()),
        );
        self.set_pred_src(87..90, 90, op.min);
        self.set_bit(80, op.ftz);
    }

    fn encode_fmul(&mut self, op: &OpFMul) {
        self.encode_alu(
            0x020,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&Src::new_zero()),
        );
        self.set_bit(76, op.dnz);
        self.set_bit(77, op.saturate);
        self.set_rnd_mode(78..80, op.rnd_mode);
        self.set_bit(80, op.ftz);
        self.set_field(84..87, 0x4_u8) // TODO: PDIV
    }

    fn set_float_cmp_op(&mut self, range: Range<usize>, op: FloatCmpOp) {
        assert!(range.len() == 4);
        self.set_field(
            range,
            match op {
                FloatCmpOp::OrdLt => 0x01_u8,
                FloatCmpOp::OrdEq => 0x02_u8,
                FloatCmpOp::OrdLe => 0x03_u8,
                FloatCmpOp::OrdGt => 0x04_u8,
                FloatCmpOp::OrdNe => 0x05_u8,
                FloatCmpOp::OrdGe => 0x06_u8,
                FloatCmpOp::UnordLt => 0x09_u8,
                FloatCmpOp::UnordEq => 0x0a_u8,
                FloatCmpOp::UnordLe => 0x0b_u8,
                FloatCmpOp::UnordGt => 0x0c_u8,
                FloatCmpOp::UnordNe => 0x0d_u8,
                FloatCmpOp::UnordGe => 0x0e_u8,
                FloatCmpOp::IsNum => 0x07_u8,
                FloatCmpOp::IsNan => 0x08_u8,
            },
        );
    }

    fn encode_fset(&mut self, op: &OpFSet) {
        self.encode_alu(
            0x00a,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::None,
        );
        self.set_float_cmp_op(76..80, op.cmp_op);
        self.set_bit(80, op.ftz);
        self.set_field(87..90, 0x7_u8); // TODO: src predicate
    }

    fn set_pred_set_op(&mut self, range: Range<usize>, op: PredSetOp) {
        assert!(range.len() == 2);
        self.set_field(
            range,
            match op {
                PredSetOp::And => 0_u8,
                PredSetOp::Or => 1_u8,
                PredSetOp::Xor => 2_u8,
            },
        );
    }

    fn encode_fsetp(&mut self, op: &OpFSetP) {
        self.encode_alu(
            0x00b,
            None,
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::None,
        );

        self.set_pred_set_op(74..76, op.set_op);
        self.set_float_cmp_op(76..80, op.cmp_op);
        self.set_bit(80, op.ftz);

        self.set_pred_dst(81..84, op.dst);
        self.set_pred_dst(84..87, Dst::None); // dst1

        self.set_pred_src(87..90, 90, op.accum);
    }

    fn encode_fswzadd(&mut self, op: &OpFSwzAdd) {
        self.set_opcode(0x822);
        self.set_dst(op.dst);

        self.set_reg_src(24..32, op.srcs[0]);
        self.set_reg_src(64..72, op.srcs[1]);

        let mut subop = 0x0_u8;

        for (i, swz_op) in op.ops.iter().enumerate() {
            let swz_op = match swz_op {
                FSwzAddOp::Add => 0,
                FSwzAddOp::SubRight => 2,
                FSwzAddOp::SubLeft => 1,
                FSwzAddOp::MoveLeft => 3,
            };

            subop |= swz_op << ((op.ops.len() - i - 1) * 2);
        }

        self.set_field(32..40, subop);

        self.set_bit(77, false); // NDV
        self.set_rnd_mode(78..80, op.rnd_mode);
        self.set_bit(80, op.ftz);
    }

    fn encode_mufu(&mut self, op: &OpMuFu) {
        self.encode_alu(
            0x108,
            Some(op.dst),
            ALUSrc::None,
            ALUSrc::from_src(&op.src),
            ALUSrc::None,
        );
        self.set_field(
            74..80,
            match op.op {
                MuFuOp::Cos => 0_u8,
                MuFuOp::Sin => 1_u8,
                MuFuOp::Exp2 => 2_u8,
                MuFuOp::Log2 => 3_u8,
                MuFuOp::Rcp => 4_u8,
                MuFuOp::Rsq => 5_u8,
                MuFuOp::Rcp64H => 6_u8,
                MuFuOp::Rsq64H => 7_u8,
                MuFuOp::Sqrt => 8_u8,
                MuFuOp::Tanh => 9_u8,
            },
        );
    }

    fn encode_dadd(&mut self, op: &OpDAdd) {
        self.encode_alu(
            0x029,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::None,
            ALUSrc::from_src(&op.srcs[1]),
        );
        self.set_rnd_mode(78..80, op.rnd_mode);
    }

    fn encode_dfma(&mut self, op: &OpDFma) {
        self.encode_alu(
            0x02b,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&op.srcs[2]),
        );
        self.set_rnd_mode(78..80, op.rnd_mode);
    }

    fn encode_dmul(&mut self, op: &OpDMul) {
        self.encode_alu(
            0x028,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::None,
        );
        self.set_rnd_mode(78..80, op.rnd_mode);
    }

    fn encode_dsetp(&mut self, op: &OpDSetP) {
        match op.srcs[1].src_ref {
            SrcRef::Reg(_) | SrcRef::Zero => {
                self.encode_alu(
                    0x02a,
                    None,
                    ALUSrc::from_src(&op.srcs[0]),
                    ALUSrc::from_src(&op.srcs[1]),
                    ALUSrc::None,
                );
            }
            _ => {
                self.encode_alu(
                    0x02a,
                    None,
                    ALUSrc::from_src(&op.srcs[0]),
                    ALUSrc::None,
                    ALUSrc::from_src(&op.srcs[1]),
                );
            }
        }

        self.set_pred_set_op(74..76, op.set_op);
        self.set_float_cmp_op(76..80, op.cmp_op);

        self.set_pred_dst(81..84, op.dst);
        self.set_pred_dst(84..87, Dst::None); /* dst1 */

        self.set_pred_src(87..90, 90, op.accum);
    }

    fn encode_brev(&mut self, op: &OpBrev) {
        self.encode_alu(
            0x101,
            Some(op.dst),
            ALUSrc::None,
            ALUSrc::from_src(&op.src),
            ALUSrc::None,
        );
    }

    fn encode_flo(&mut self, op: &OpFlo) {
        self.encode_alu(
            0x100,
            Some(op.dst),
            ALUSrc::None,
            ALUSrc::from_src(&op.src),
            ALUSrc::None,
        );
        self.set_pred_dst(81..84, Dst::None);
        self.set_field(74..75, op.return_shift_amount as u8);
        self.set_field(73..74, op.signed as u8);
        let not_mod = matches!(op.src.src_mod, SrcMod::BNot);
        self.set_field(63..64, not_mod)
    }

    fn encode_iabs(&mut self, op: &OpIAbs) {
        self.encode_alu(
            0x013,
            Some(op.dst),
            ALUSrc::None,
            ALUSrc::from_src(&op.src),
            ALUSrc::None,
        );
    }

    fn encode_iadd3(&mut self, op: &OpIAdd3) {
        // Hardware requires at least one of these be unmodified
        assert!(op.srcs[0].src_mod.is_none() || op.srcs[1].src_mod.is_none());

        self.encode_alu(
            0x010,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&op.srcs[2]),
        );

        self.set_pred_dst(81..84, op.overflow[0]);
        self.set_pred_dst(84..87, op.overflow[1]);
    }

    fn encode_iadd3x(&mut self, op: &OpIAdd3X) {
        // Hardware requires at least one of these be unmodified
        assert!(op.srcs[0].src_mod.is_none() || op.srcs[1].src_mod.is_none());

        self.encode_alu(
            0x010,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&op.srcs[2]),
        );

        self.set_bit(74, true); // .X

        self.set_pred_dst(81..84, op.overflow[0]);
        self.set_pred_dst(84..87, op.overflow[1]);

        self.set_pred_src(87..90, 90, op.carry[0]);
        self.set_pred_src(77..80, 80, op.carry[1]);
    }

    fn encode_idp4(&mut self, op: &OpIDp4) {
        self.encode_alu(
            0x026,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&op.srcs[2]),
        );

        self.set_bit(
            73,
            match op.src_types[0] {
                IntType::U8 => false,
                IntType::I8 => true,
                _ => panic!("Invalid DP4 source type"),
            },
        );
        self.set_bit(
            74,
            match op.src_types[1] {
                IntType::U8 => false,
                IntType::I8 => true,
                _ => panic!("Invalid DP4 source type"),
            },
        );
    }

    fn encode_imad(&mut self, op: &OpIMad) {
        self.encode_alu(
            0x024,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&op.srcs[2]),
        );
        self.set_pred_dst(81..84, Dst::None);
        self.set_bit(73, op.signed);
    }

    fn encode_imad64(&mut self, op: &OpIMad64) {
        self.encode_alu(
            0x025,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::from_src(&op.srcs[2]),
        );
        self.set_pred_dst(81..84, Dst::None);
        self.set_bit(73, op.signed);
    }

    fn encode_imnmx(&mut self, op: &OpIMnMx) {
        self.encode_alu(
            0x017,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.srcs[1]),
            ALUSrc::None,
        );
        self.set_pred_src(87..90, 90, op.min);
        self.set_bit(
            73,
            match op.cmp_type {
                IntCmpType::U32 => false,
                IntCmpType::I32 => true,
            },
        );
    }

    fn set_int_cmp_op(&mut self, range: Range<usize>, op: IntCmpOp) {
        assert!(range.len() == 3);
        self.set_field(
            range,
            match op {
                IntCmpOp::Eq => 2_u8,
                IntCmpOp::Ne => 5_u8,
                IntCmpOp::Lt => 1_u8,
                IntCmpOp::Le => 3_u8,
                IntCmpOp::Gt => 4_u8,
                IntCmpOp::Ge => 6_u8,
            },
        );
    }

    fn encode_isetp(&mut self, op: &OpISetP) {
        self.encode_alu(
            0x00c,
            None,
            ALUSrc::from_src(&op.srcs[0].into()),
            ALUSrc::from_src(&op.srcs[1].into()),
            ALUSrc::None,
        );

        self.set_pred_src(68..71, 71, op.low_cmp);
        self.set_bit(72, op.ex);

        self.set_field(
            73..74,
            match op.cmp_type {
                IntCmpType::U32 => 0_u32,
                IntCmpType::I32 => 1_u32,
            },
        );
        self.set_pred_set_op(74..76, op.set_op);
        self.set_int_cmp_op(76..79, op.cmp_op);

        self.set_pred_dst(81..84, op.dst);
        self.set_pred_dst(84..87, Dst::None); // dst1

        self.set_pred_src(87..90, 90, op.accum);
    }

    fn encode_lop3(&mut self, op: &OpLop3) {
        self.encode_alu(
            0x012,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0].into()),
            ALUSrc::from_src(&op.srcs[1].into()),
            ALUSrc::from_src(&op.srcs[2].into()),
        );

        self.set_field(72..80, op.op.lut);
        self.set_bit(80, false); // .PAND
        self.set_field(81..84, 7_u32); // pred
        self.set_pred_src(87..90, 90, SrcRef::False.into());
    }

    fn encode_popc(&mut self, op: &OpPopC) {
        self.encode_alu(
            0x109,
            Some(op.dst),
            ALUSrc::None,
            ALUSrc::from_src(&op.src),
            ALUSrc::None,
        );

        let not_mod = matches!(op.src.src_mod, SrcMod::BNot);
        self.set_field(63..64, not_mod)
    }

    fn encode_shf(&mut self, op: &OpShf) {
        self.encode_alu(
            0x019,
            Some(op.dst),
            ALUSrc::from_src(&op.low),
            ALUSrc::from_src(&op.shift),
            ALUSrc::from_src(&op.high),
        );

        self.set_field(
            73..75,
            match op.data_type {
                IntType::I64 => 0_u8,
                IntType::U64 => 1_u8,
                IntType::I32 => 2_u8,
                IntType::U32 => 3_u8,
                _ => panic!("Invalid shift data type"),
            },
        );
        self.set_bit(75, op.wrap);
        self.set_bit(76, op.right);
        self.set_bit(80, op.dst_high);
    }

    fn encode_f2f(&mut self, op: &OpF2F) {
        if op.src_type.bits() <= 32 && op.dst_type.bits() <= 32 {
            self.encode_alu(
                0x104,
                Some(op.dst),
                ALUSrc::None,
                ALUSrc::from_src(&op.src.into()),
                ALUSrc::None,
            );
        } else {
            self.encode_alu(
                0x110,
                Some(op.dst),
                ALUSrc::None,
                ALUSrc::from_src(&op.src.into()),
                ALUSrc::None,
            );
        }

        if op.high {
            self.set_field(60..62, 1_u8); // .H1
        }

        self.set_field(75..77, (op.dst_type.bits() / 8).ilog2());
        self.set_rnd_mode(78..80, op.rnd_mode);
        self.set_bit(80, op.ftz);
        self.set_field(84..86, (op.src_type.bits() / 8).ilog2());
    }

    fn encode_f2i(&mut self, op: &OpF2I) {
        if op.src_type.bits() <= 32 && op.dst_type.bits() <= 32 {
            self.encode_alu(
                0x105,
                Some(op.dst),
                ALUSrc::None,
                ALUSrc::from_src(&op.src.into()),
                ALUSrc::None,
            );
        } else {
            self.encode_alu(
                0x111,
                Some(op.dst),
                ALUSrc::None,
                ALUSrc::from_src(&op.src.into()),
                ALUSrc::None,
            );
        }

        self.set_bit(72, op.dst_type.is_signed());
        self.set_field(75..77, (op.dst_type.bits() / 8).ilog2());
        self.set_bit(77, false); // NTZ
        self.set_rnd_mode(78..80, op.rnd_mode);
        self.set_bit(80, op.ftz);
        self.set_field(84..86, (op.src_type.bits() / 8).ilog2());
    }

    fn encode_i2f(&mut self, op: &OpI2F) {
        if op.src_type.bits() <= 32 && op.dst_type.bits() <= 32 {
            self.encode_alu(
                0x106,
                Some(op.dst),
                ALUSrc::None,
                ALUSrc::from_src(&op.src.into()),
                ALUSrc::None,
            );
        } else {
            self.encode_alu(
                0x112,
                Some(op.dst),
                ALUSrc::None,
                ALUSrc::from_src(&op.src.into()),
                ALUSrc::None,
            );
        }

        self.set_field(60..62, 0_u8); // TODO: subop
        self.set_bit(74, op.src_type.is_signed());
        self.set_field(75..77, (op.dst_type.bits() / 8).ilog2());
        self.set_rnd_mode(78..80, op.rnd_mode);
        self.set_field(84..86, (op.src_type.bits() / 8).ilog2());
    }

    fn encode_frnd(&mut self, op: &OpFRnd) {
        if op.src_type.bits() <= 32 && op.dst_type.bits() <= 32 {
            self.encode_alu(
                0x107,
                Some(op.dst),
                ALUSrc::None,
                ALUSrc::from_src(&op.src.into()),
                ALUSrc::None,
            );
        } else {
            self.encode_alu(
                0x113,
                Some(op.dst),
                ALUSrc::None,
                ALUSrc::from_src(&op.src.into()),
                ALUSrc::None,
            );
        }

        self.set_field(84..86, (op.src_type.bits() / 8).ilog2());
        self.set_bit(80, op.ftz);
        self.set_rnd_mode(78..80, op.rnd_mode);
        self.set_field(75..77, (op.dst_type.bits() / 8).ilog2());
    }

    fn encode_mov(&mut self, op: &OpMov) {
        self.encode_alu(
            0x002,
            Some(op.dst),
            ALUSrc::None,
            ALUSrc::from_src(&op.src.into()),
            ALUSrc::None,
        );
        self.set_field(72..76, op.quad_lanes);
    }

    fn encode_prmt(&mut self, op: &OpPrmt) {
        self.encode_alu(
            0x16,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0]),
            ALUSrc::from_src(&op.sel),
            ALUSrc::from_src(&op.srcs[1]),
        );

        self.set_field(
            72..75,
            match op.mode {
                PrmtMode::Index => 0_u8,
                PrmtMode::Forward4Extract => 1_u8,
                PrmtMode::Backward4Extract => 2_u8,
                PrmtMode::Replicate8 => 3_u8,
                PrmtMode::EdgeClampLeft => 4_u8,
                PrmtMode::EdgeClampRight => 5_u8,
                PrmtMode::Replicate16 => 6_u8,
            },
        )
    }

    fn encode_sel(&mut self, op: &OpSel) {
        self.encode_alu(
            0x007,
            Some(op.dst),
            ALUSrc::from_src(&op.srcs[0].into()),
            ALUSrc::from_src(&op.srcs[1].into()),
            ALUSrc::None,
        );

        self.set_pred_src(87..90, 90, op.cond);
    }

    fn encode_shfl(&mut self, op: &OpShfl) {
        assert!(op.lane.src_mod.is_none());
        assert!(op.c.src_mod.is_none());

        match &op.lane.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => match &op.c.src_ref {
                SrcRef::Zero | SrcRef::Reg(_) => {
                    self.set_opcode(0x389);
                    self.set_reg_src(32..40, op.lane);
                    self.set_reg_src(64..72, op.c);
                }
                SrcRef::Imm32(imm_c) => {
                    self.set_opcode(0x589);
                    self.set_reg_src(32..40, op.lane);
                    self.set_field(40..53, *imm_c & 0x1f1f);
                }
                _ => panic!("Invalid instruction form"),
            },
            SrcRef::Imm32(imm_lane) => match &op.c.src_ref {
                SrcRef::Zero | SrcRef::Reg(_) => {
                    self.set_opcode(0x989);
                    self.set_field(53..58, *imm_lane & 0x1f);
                    self.set_reg_src(64..72, op.c);
                }
                SrcRef::Imm32(imm_c) => {
                    self.set_opcode(0xf89);
                    self.set_field(40..53, *imm_c & 0x1f1f);
                    self.set_field(53..58, *imm_lane & 0x1f);
                }
                _ => panic!("Invalid instruction form"),
            },
            _ => panic!("Invalid instruction form"),
        };

        self.set_dst(op.dst);
        self.set_pred_dst(81..84, op.in_bounds);
        self.set_reg_src(24..32, op.src);
        self.set_field(
            58..60,
            match op.op {
                ShflOp::Idx => 0_u8,
                ShflOp::Up => 1_u8,
                ShflOp::Down => 2_u8,
                ShflOp::Bfly => 3_u8,
            },
        );
    }

    fn encode_plop3(&mut self, op: &OpPLop3) {
        self.set_opcode(0x81c);
        self.set_field(16..24, op.ops[1].lut);
        self.set_field(64..67, op.ops[0].lut & 0x7);
        self.set_field(72..77, op.ops[0].lut >> 3);

        self.set_pred_src(68..71, 71, op.srcs[2]);

        self.set_pred_src(77..80, 80, op.srcs[1]);
        self.set_pred_dst(81..84, op.dsts[0]);
        self.set_pred_dst(84..87, op.dsts[1]);

        self.set_pred_src(87..90, 90, op.srcs[0]);
    }

    fn set_tex_dim(&mut self, range: Range<usize>, dim: TexDim) {
        assert!(range.len() == 3);
        self.set_field(
            range,
            match dim {
                TexDim::_1D => 0_u8,
                TexDim::Array1D => 4_u8,
                TexDim::_2D => 1_u8,
                TexDim::Array2D => 5_u8,
                TexDim::_3D => 2_u8,
                TexDim::Cube => 3_u8,
                TexDim::ArrayCube => 7_u8,
            },
        );
    }

    fn set_tex_lod_mode(&mut self, range: Range<usize>, lod_mode: TexLodMode) {
        assert!(range.len() == 3);
        self.set_field(
            range,
            match lod_mode {
                TexLodMode::Auto => 0_u8,
                TexLodMode::Zero => 1_u8,
                TexLodMode::Bias => 2_u8,
                TexLodMode::Lod => 3_u8,
                TexLodMode::Clamp => 4_u8,
                TexLodMode::BiasClamp => 5_u8,
            },
        );
    }

    fn encode_tex(&mut self, op: &OpTex) {
        self.set_opcode(0x361);
        self.set_bit(59, true); // .B

        self.set_dst(op.dsts[0]);
        if let Dst::Reg(reg) = op.dsts[1] {
            self.set_reg(64..72, reg);
        } else {
            self.set_field(64..72, 255_u8);
        }
        self.set_pred_dst(81..84, op.resident);

        self.set_reg_src(24..32, op.srcs[0]);
        self.set_reg_src(32..40, op.srcs[1]);

        self.set_tex_dim(61..64, op.dim);
        self.set_field(72..76, op.mask);
        self.set_bit(76, op.offset);
        self.set_bit(77, false); // ToDo: NDV
        self.set_bit(78, op.z_cmpr);
        self.set_field(84..87, 1);
        self.set_tex_lod_mode(87..90, op.lod_mode);
        self.set_bit(90, false); // TODO: .NODEP
    }

    fn encode_tld(&mut self, op: &OpTld) {
        self.set_opcode(0x367);
        self.set_bit(59, true); // .B

        self.set_dst(op.dsts[0]);
        if let Dst::Reg(reg) = op.dsts[1] {
            self.set_reg(64..72, reg);
        } else {
            self.set_field(64..72, 255_u8);
        }
        self.set_pred_dst(81..84, op.resident);

        self.set_reg_src(24..32, op.srcs[0]);
        self.set_reg_src(32..40, op.srcs[1]);

        self.set_tex_dim(61..64, op.dim);
        self.set_field(72..76, op.mask);
        self.set_bit(76, op.offset);
        // bit 77: .CL
        self.set_bit(78, op.is_ms);
        // bits 79..81: .F16
        assert!(
            op.lod_mode == TexLodMode::Zero || op.lod_mode == TexLodMode::Lod
        );
        self.set_tex_lod_mode(87..90, op.lod_mode);
        self.set_bit(90, false); // TODO: .NODEP
    }

    fn encode_tld4(&mut self, op: &OpTld4) {
        self.set_opcode(0x364);
        self.set_bit(59, true); // .B

        self.set_dst(op.dsts[0]);
        if let Dst::Reg(reg) = op.dsts[1] {
            self.set_reg(64..72, reg);
        } else {
            self.set_field(64..72, 255_u8);
        }
        self.set_pred_dst(81..84, op.resident);

        self.set_reg_src(24..32, op.srcs[0]);
        self.set_reg_src(32..40, op.srcs[1]);

        self.set_tex_dim(61..64, op.dim);
        self.set_field(72..76, op.mask);
        self.set_field(
            76..78,
            match op.offset_mode {
                Tld4OffsetMode::None => 0_u8,
                Tld4OffsetMode::AddOffI => 1_u8,
                Tld4OffsetMode::PerPx => 2_u8,
            },
        );
        // bit 77: .CL
        self.set_bit(78, op.z_cmpr);
        self.set_bit(84, true); // !.EF
        self.set_field(87..89, op.comp);
        self.set_bit(90, false); // TODO: .NODEP
    }

    fn encode_tmml(&mut self, op: &OpTmml) {
        self.set_opcode(0x36a);
        self.set_bit(59, true); // .B

        self.set_dst(op.dsts[0]);
        if let Dst::Reg(reg) = op.dsts[1] {
            self.set_reg(64..72, reg);
        } else {
            self.set_field(64..72, 255_u8);
        }

        self.set_reg_src(24..32, op.srcs[0]);
        self.set_reg_src(32..40, op.srcs[1]);

        self.set_tex_dim(61..64, op.dim);
        self.set_field(72..76, op.mask);
        self.set_bit(77, false); // ToDo: NDV
        self.set_bit(90, false); // TODO: .NODEP
    }

    fn encode_txd(&mut self, op: &OpTxd) {
        self.set_opcode(0x36d);
        self.set_bit(59, true); // .B

        self.set_dst(op.dsts[0]);
        if let Dst::Reg(reg) = op.dsts[1] {
            self.set_reg(64..72, reg);
        } else {
            self.set_field(64..72, 255_u8);
        }
        self.set_pred_dst(81..84, op.resident);

        self.set_reg_src(24..32, op.srcs[0]);
        self.set_reg_src(32..40, op.srcs[1]);

        self.set_tex_dim(61..64, op.dim);
        self.set_field(72..76, op.mask);
        self.set_bit(76, op.offset);
        self.set_bit(77, false); // ToDo: NDV
        self.set_bit(90, false); // TODO: .NODEP
    }

    fn encode_txq(&mut self, op: &OpTxq) {
        self.set_opcode(0x370);
        self.set_bit(59, true); // .B

        self.set_dst(op.dsts[0]);
        if let Dst::Reg(reg) = op.dsts[1] {
            self.set_reg(64..72, reg);
        } else {
            self.set_field(64..72, 255_u8);
        }

        self.set_reg_src(24..32, op.src);
        self.set_field(
            62..64,
            match op.query {
                TexQuery::Dimension => 0_u8,
                TexQuery::TextureType => 1_u8,
                TexQuery::SamplerPos => 2_u8,
            },
        );
        self.set_field(72..76, op.mask);
    }

    fn set_image_dim(&mut self, range: Range<usize>, dim: ImageDim) {
        assert!(range.len() == 3);
        self.set_field(
            range,
            match dim {
                ImageDim::_1D => 0_u8,
                ImageDim::_1DBuffer => 1_u8,
                ImageDim::_1DArray => 2_u8,
                ImageDim::_2D => 3_u8,
                ImageDim::_2DArray => 4_u8,
                ImageDim::_3D => 5_u8,
            },
        );
    }

    fn set_mem_order(&mut self, order: &MemOrder) {
        if self.sm < 80 {
            let scope = match order {
                MemOrder::Constant => MemScope::System,
                MemOrder::Weak => MemScope::CTA,
                MemOrder::Strong(s) => *s,
            };
            self.set_field(
                77..79,
                match scope {
                    MemScope::CTA => 0_u8,
                    // SM => 1_u8,
                    MemScope::GPU => 2_u8,
                    MemScope::System => 3_u8,
                },
            );
            self.set_field(
                79..81,
                match order {
                    MemOrder::Constant => 0_u8,
                    MemOrder::Weak => 1_u8,
                    MemOrder::Strong(_) => 2_u8,
                    // MMIO => 3_u8,
                },
            );
        } else {
            self.set_field(
                77..81,
                match order {
                    MemOrder::Constant => 0x4_u8,
                    MemOrder::Weak => 0x0_u8,
                    MemOrder::Strong(MemScope::CTA) => 0x5_u8,
                    MemOrder::Strong(MemScope::GPU) => 0x7_u8,
                    MemOrder::Strong(MemScope::System) => 0xa_u8,
                },
            );
        }
    }

    fn set_eviction_priority(&mut self, pri: &MemEvictionPriority) {
        self.set_field(
            84..86,
            match pri {
                MemEvictionPriority::First => 0_u8,
                MemEvictionPriority::Normal => 1_u8,
                MemEvictionPriority::Last => 2_u8,
                MemEvictionPriority::Unchanged => 3_u8,
            },
        );
    }

    fn encode_suld(&mut self, op: &OpSuLd) {
        self.set_opcode(0x998);

        self.set_dst(op.dst);
        self.set_reg_src(24..32, op.coord);
        self.set_reg_src(64..72, op.handle);
        self.set_pred_dst(81..84, op.resident);

        self.set_image_dim(61..64, op.image_dim);
        self.set_mem_order(&op.mem_order);
        self.set_eviction_priority(&op.mem_eviction_priority);

        assert!(op.mask == 0x1 || op.mask == 0x3 || op.mask == 0xf);
        self.set_field(72..76, op.mask);
    }

    fn encode_sust(&mut self, op: &OpSuSt) {
        self.set_opcode(0x99c);

        self.set_reg_src(24..32, op.coord);
        self.set_reg_src(32..40, op.data);
        self.set_reg_src(64..72, op.handle);

        self.set_image_dim(61..64, op.image_dim);
        self.set_mem_order(&op.mem_order);
        self.set_eviction_priority(&op.mem_eviction_priority);

        assert!(op.mask == 0x1 || op.mask == 0x3 || op.mask == 0xf);
        self.set_field(72..76, op.mask);
    }

    fn encode_suatom(&mut self, op: &OpSuAtom) {
        if matches!(op.atom_op, AtomOp::CmpExch) {
            self.set_opcode(0x396);
        } else {
            self.set_opcode(0x394);
        }

        self.set_dst(op.dst);
        self.set_reg_src(24..32, op.coord);
        self.set_reg_src(32..40, op.data);
        self.set_reg_src(64..72, op.handle);
        self.set_pred_dst(81..84, op.resident);

        self.set_image_dim(61..64, op.image_dim);
        self.set_mem_order(&op.mem_order);
        self.set_eviction_priority(&op.mem_eviction_priority);

        self.set_bit(72, false); // .BA
        self.set_atom_type(73..76, op.atom_type);
        self.set_atom_op(87..91, op.atom_op);
    }

    fn set_mem_type(&mut self, range: Range<usize>, mem_type: MemType) {
        assert!(range.len() == 3);
        self.set_field(
            range,
            match mem_type {
                MemType::U8 => 0_u8,
                MemType::I8 => 1_u8,
                MemType::U16 => 2_u8,
                MemType::I16 => 3_u8,
                MemType::B32 => 4_u8,
                MemType::B64 => 5_u8,
                MemType::B128 => 6_u8,
            },
        );
    }

    fn set_mem_access(&mut self, access: &MemAccess) {
        self.set_field(
            72..73,
            match access.space.addr_type() {
                MemAddrType::A32 => 0_u8,
                MemAddrType::A64 => 1_u8,
            },
        );
        self.set_mem_type(73..76, access.mem_type);
        self.set_mem_order(&access.order);
        self.set_eviction_priority(&access.eviction_priority);
    }

    fn encode_ldg(&mut self, op: &OpLd) {
        self.set_opcode(0x980);

        self.set_dst(op.dst);
        self.set_reg_src(24..32, op.addr);
        self.set_field(32..64, op.offset);

        self.set_mem_access(&op.access);
    }

    fn encode_ldl(&mut self, op: &OpLd) {
        self.set_opcode(0x983);
        self.set_field(84..87, 1_u8);

        self.set_dst(op.dst);
        self.set_reg_src(24..32, op.addr);
        self.set_field(40..64, op.offset);

        self.set_mem_type(73..76, op.access.mem_type);
        assert!(op.access.order == MemOrder::Strong(MemScope::CTA));
        assert!(op.access.eviction_priority == MemEvictionPriority::Normal);
    }

    fn encode_lds(&mut self, op: &OpLd) {
        self.set_opcode(0x984);

        self.set_dst(op.dst);
        self.set_reg_src(24..32, op.addr);
        self.set_field(40..64, op.offset);

        self.set_mem_type(73..76, op.access.mem_type);
        assert!(op.access.order == MemOrder::Strong(MemScope::CTA));
        assert!(op.access.eviction_priority == MemEvictionPriority::Normal);

        self.set_bit(87, false); // !.ZD - Returns a predicate?
    }

    fn encode_ld(&mut self, op: &OpLd) {
        match op.access.space {
            MemSpace::Global(_) => self.encode_ldg(op),
            MemSpace::Local => self.encode_ldl(op),
            MemSpace::Shared => self.encode_lds(op),
        }
    }

    fn encode_ldc(&mut self, op: &OpLdc) {
        self.encode_alu(
            0x182,
            Some(op.dst),
            ALUSrc::from_src(&op.offset),
            ALUSrc::from_src(&op.cb),
            ALUSrc::None,
        );

        self.set_mem_type(73..76, op.mem_type);
        self.set_field(78..80, 0_u8); // subop
    }

    fn encode_stg(&mut self, op: &OpSt) {
        self.set_opcode(0x385);

        self.set_reg_src(24..32, op.addr);
        self.set_field(32..64, op.offset);
        self.set_reg_src(64..72, op.data);

        self.set_mem_access(&op.access);
    }

    fn encode_stl(&mut self, op: &OpSt) {
        self.set_opcode(0x387);
        self.set_field(84..87, 1_u8);

        self.set_reg_src(24..32, op.addr);
        self.set_reg_src(32..40, op.data);
        self.set_field(40..64, op.offset);

        self.set_mem_type(73..76, op.access.mem_type);
        assert!(op.access.order == MemOrder::Strong(MemScope::CTA));
        assert!(op.access.eviction_priority == MemEvictionPriority::Normal);
    }

    fn encode_sts(&mut self, op: &OpSt) {
        self.set_opcode(0x388);

        self.set_reg_src(24..32, op.addr);
        self.set_reg_src(32..40, op.data);
        self.set_field(40..64, op.offset);

        self.set_mem_type(73..76, op.access.mem_type);
        assert!(op.access.order == MemOrder::Strong(MemScope::CTA));
        assert!(op.access.eviction_priority == MemEvictionPriority::Normal);
    }

    fn encode_st(&mut self, op: &OpSt) {
        match op.access.space {
            MemSpace::Global(_) => self.encode_stg(op),
            MemSpace::Local => self.encode_stl(op),
            MemSpace::Shared => self.encode_sts(op),
        }
    }

    fn set_atom_op(&mut self, range: Range<usize>, atom_op: AtomOp) {
        assert!(range.len() == 4);
        self.set_field(
            range,
            match atom_op {
                AtomOp::Add | AtomOp::CmpExch => 0_u8,
                AtomOp::Min => 1_u8,
                AtomOp::Max => 2_u8,
                AtomOp::Inc => 3_u8,
                AtomOp::Dec => 4_u8,
                AtomOp::And => 5_u8,
                AtomOp::Or => 6_u8,
                AtomOp::Xor => 7_u8,
                AtomOp::Exch => 8_u8,
            },
        );
    }

    fn set_atom_type(&mut self, range: Range<usize>, atom_type: AtomType) {
        assert!(range.len() == 3);
        self.set_field(
            range,
            match atom_type {
                AtomType::U32 => 0_u8,
                AtomType::I32 => 1_u8,
                AtomType::U64 => 2_u8,
                AtomType::F32 => 3_u8,
                AtomType::F16x2 => 4_u8,
                AtomType::I64 => 5_u8,
                AtomType::F64 => 6_u8,
            },
        );
    }

    fn encode_atomg(&mut self, op: &OpAtom) {
        if op.atom_op == AtomOp::CmpExch {
            self.set_opcode(0x38b);

            self.set_reg_src(32..40, op.cmpr);
            self.set_reg_src(64..72, op.data);
        } else {
            self.set_opcode(0x38a);

            self.set_reg_src(32..40, op.data);

            self.set_atom_op(87..91, op.atom_op);
        }

        self.set_dst(op.dst);
        self.set_pred_dst(81..84, Dst::None);

        self.set_reg_src(24..32, op.addr);
        self.set_field(40..64, op.addr_offset);

        self.set_field(
            72..73,
            match op.mem_space.addr_type() {
                MemAddrType::A32 => 0_u8,
                MemAddrType::A64 => 1_u8,
            },
        );

        self.set_atom_type(73..76, op.atom_type);
        self.set_mem_order(&op.mem_order);
        self.set_eviction_priority(&op.mem_eviction_priority);
    }

    fn encode_atoms(&mut self, op: &OpAtom) {
        if op.atom_op == AtomOp::CmpExch {
            self.set_opcode(0x38d);

            self.set_reg_src(32..40, op.cmpr);
            self.set_reg_src(64..72, op.data);
        } else {
            self.set_opcode(0x38c);

            self.set_reg_src(32..40, op.data);

            self.set_atom_op(87..91, op.atom_op);
        }

        self.set_dst(op.dst);
        self.set_reg_src(24..32, op.addr);
        self.set_field(40..64, op.addr_offset);

        assert!(op.mem_order == MemOrder::Strong(MemScope::CTA));
        assert!(op.mem_eviction_priority == MemEvictionPriority::Normal);

        self.set_atom_type(73..76, op.atom_type);
    }

    fn encode_atom(&mut self, op: &OpAtom) {
        match op.mem_space {
            MemSpace::Global(_) => self.encode_atomg(op),
            MemSpace::Local => panic!("Atomics do not support local"),
            MemSpace::Shared => self.encode_atoms(op),
        }
    }

    fn encode_al2p(&mut self, op: &OpAL2P) {
        self.set_opcode(0x920);

        self.set_dst(op.dst);
        self.set_reg_src(24..32, op.offset);

        self.set_field(40..50, op.access.addr);
        self.set_field(74..76, 0_u8); // comps
        assert!(!op.access.patch);
        self.set_bit(79, op.access.output);
    }

    fn encode_ald(&mut self, op: &OpALd) {
        self.set_opcode(0x321);

        self.set_dst(op.dst);
        self.set_reg_src(32..40, op.vtx);
        self.set_reg_src(24..32, op.offset);

        self.set_field(40..50, op.access.addr);
        self.set_field(74..76, op.access.comps - 1);
        self.set_field(76..77, op.access.patch);
        self.set_field(77..78, op.access.phys);
        self.set_field(79..80, op.access.output);
    }

    fn encode_ast(&mut self, op: &OpASt) {
        self.set_opcode(0x322);

        self.set_reg_src(32..40, op.data);
        self.set_reg_src(64..72, op.vtx);
        self.set_reg_src(24..32, op.offset);

        self.set_field(40..50, op.access.addr);
        self.set_field(74..76, op.access.comps - 1);
        self.set_field(76..77, op.access.patch);
        self.set_field(77..78, op.access.phys);
        assert!(op.access.output);
    }

    fn encode_ipa(&mut self, op: &OpIpa) {
        self.set_opcode(0x326);

        self.set_dst(op.dst);

        assert!(op.addr % 4 == 0);
        self.set_field(64..72, op.addr >> 2);

        self.set_field(
            76..78,
            match op.loc {
                InterpLoc::Default => 0_u8,
                InterpLoc::Centroid => 1_u8,
                InterpLoc::Offset => 2_u8,
            },
        );
        self.set_field(
            78..80,
            match op.freq {
                InterpFreq::Pass => 0_u8,
                InterpFreq::Constant => 1_u8,
                InterpFreq::State => 2_u8,
                InterpFreq::PassMulW => {
                    panic!("InterpFreq::PassMulW is invalid on SM70+");
                }
            },
        );

        assert!(op.inv_w.is_zero());
        self.set_reg_src(32..40, op.offset);

        // TODO: What is this for?
        self.set_pred_dst(81..84, Dst::None);
    }

    fn encode_ldtram(&mut self, op: &OpLdTram) {
        self.set_opcode(0x3ad);
        self.set_dst(op.dst);
        self.set_ureg(24..32, RegRef::zero(RegFile::UGPR, 1));

        assert!(op.addr % 4 == 0);
        self.set_field(64..72, op.addr >> 2);

        self.set_bit(72, op.use_c);

        // Unknown but required
        self.set_bit(91, true);
    }

    fn encode_cctl(&mut self, op: &OpCCtl) {
        assert!(matches!(op.mem_space, MemSpace::Global(_)));
        self.set_opcode(0x98f);

        self.set_reg_src(24..32, op.addr);
        self.set_field(32..64, op.addr_offset);

        self.set_field(
            87..91,
            match op.op {
                CCtlOp::PF1 => 0_u8,
                CCtlOp::PF2 => 1_u8,
                CCtlOp::WB => 2_u8,
                CCtlOp::IV => 3_u8,
                CCtlOp::IVAll => 4_u8,
                CCtlOp::RS => 5_u8,
                CCtlOp::IVAllP => 6_u8,
                CCtlOp::WBAll => 7_u8,
                CCtlOp::WBAllP => 8_u8,
            },
        );
    }

    fn encode_membar(&mut self, op: &OpMemBar) {
        self.set_opcode(0x992);

        self.set_bit(72, false); // !.MMIO
        self.set_field(
            76..79,
            match op.scope {
                MemScope::CTA => 0_u8,
                // SM => 1_u8,
                MemScope::GPU => 2_u8,
                MemScope::System => 3_u8,
            },
        );
        self.set_bit(80, false); // .SC
    }

    fn set_rel_offset(
        &mut self,
        range: Range<usize>,
        label: &Label,
        ip: usize,
        labels: &HashMap<Label, usize>,
    ) {
        let ip = u64::try_from(ip).unwrap();
        let ip = i64::try_from(ip).unwrap();

        let target_ip = *labels.get(label).unwrap();
        let target_ip = u64::try_from(target_ip).unwrap();
        let target_ip = i64::try_from(target_ip).unwrap();

        let rel_offset = target_ip - ip - 4;

        self.set_field(range, rel_offset);
    }

    fn encode_bclear(&mut self, op: &OpBClear) {
        self.set_opcode(0x355);

        self.set_dst(Dst::None);
        self.set_bar_dst(24..28, op.dst);

        self.set_bit(84, true); // .CLEAR
    }

    fn encode_bmov(&mut self, op: &OpBMov) {
        if dst_is_bar(op.dst) {
            self.set_opcode(0x356);

            self.set_bar_dst(24..28, op.dst);
            self.set_reg_src(32..40, op.src);

            self.set_bit(84, op.clear);
        } else {
            self.set_opcode(0x355);

            self.set_dst(op.dst);
            self.set_bar_src(24..28, op.src);

            self.set_bit(84, op.clear);
        }
    }

    fn encode_break(&mut self, op: &OpBreak) {
        self.set_opcode(0x942);
        assert!(op.bar_in.src_ref.as_reg() == op.bar_out.as_reg());
        self.set_bar_dst(16..20, op.bar_out);
        self.set_pred_src(87..90, 90, op.cond);
    }

    fn encode_bssy(
        &mut self,
        op: &OpBSSy,
        ip: usize,
        labels: &HashMap<Label, usize>,
    ) {
        self.set_opcode(0x945);
        assert!(op.bar_in.src_ref.as_reg() == op.bar_out.as_reg());
        self.set_bar_dst(16..20, op.bar_out);
        self.set_rel_offset(34..64, &op.target, ip, labels);
        self.set_pred_src(87..90, 90, op.cond);
    }

    fn encode_bsync(&mut self, op: &OpBSync) {
        self.set_opcode(0x941);
        self.set_bar_src(16..20, op.bar);
        self.set_pred_src(87..90, 90, op.cond);
    }

    fn encode_bra(
        &mut self,
        op: &OpBra,
        ip: usize,
        labels: &HashMap<Label, usize>,
    ) {
        self.set_opcode(0x947);
        self.set_rel_offset(34..82, &op.target, ip, labels);
        self.set_field(87..90, 0x7_u8); // TODO: Pred?
    }

    fn encode_exit(&mut self, _op: &OpExit) {
        self.set_opcode(0x94d);

        // ./.KEEPREFCOUNT/.PREEMPTED/.INVALID3
        self.set_field(84..85, false);
        self.set_field(85..86, false); // .NO_ATEXIT
        self.set_field(87..90, 0x7_u8); // TODO: Predicate
        self.set_field(90..91, false); // NOT
    }

    fn encode_warpsync(&mut self, op: &OpWarpSync) {
        self.encode_alu(
            0x148,
            None,
            ALUSrc::None,
            ALUSrc::Imm32(op.mask),
            ALUSrc::None,
        );
        self.set_pred_src(87..90, 90, SrcRef::True.into());
    }

    fn encode_bar(&mut self, _op: &OpBar) {
        self.set_opcode(0xb1d);

        // self.set_opcode(0x31d);

        // // src0 == src1
        // self.set_reg_src(32..40, SrcRef::Zero.into());

        // // 00: RED.POPC
        // // 01: RED.AND
        // // 02: RED.OR
        // self.set_field(74..76, 0_u8);

        // // 00: SYNC
        // // 01: ARV
        // // 02: RED
        // // 03: SCAN
        // self.set_field(77..79, 0_u8);

        // self.set_pred_src(87..90, 90, SrcRef::True.into());
    }

    fn encode_cs2r(&mut self, op: &OpCS2R) {
        self.set_opcode(0x805);
        self.set_dst(op.dst);
        self.set_field(72..80, op.idx);
        self.set_bit(80, op.dst.as_reg().unwrap().comps() == 2); // .64
    }

    fn encode_isberd(&mut self, op: &OpIsberd) {
        self.set_opcode(0x923);
        self.set_dst(op.dst);
        self.set_reg_src(24..32, op.idx);
    }

    fn encode_kill(&mut self, _op: &OpKill) {
        self.set_opcode(0x95b);
        self.set_pred_src(87..90, 90, SrcRef::True.into());
    }

    fn encode_nop(&mut self, _op: &OpNop) {
        self.set_opcode(0x918);
    }

    fn encode_pixld(&mut self, op: &OpPixLd) {
        self.set_opcode(0x925);
        self.set_dst(op.dst);
        self.set_field(
            78..81,
            match op.val {
                PixVal::MsCount => 0_u8,
                PixVal::CovMask => 1_u8,
                PixVal::CentroidOffset => 2_u8,
                PixVal::MyIndex => 3_u8,
                PixVal::InnerCoverage => 4_u8,
            },
        );
        self.set_pred_dst(81..84, Dst::None);
    }

    fn encode_s2r(&mut self, op: &OpS2R) {
        self.set_opcode(0x919);
        self.set_dst(op.dst);
        self.set_field(72..80, op.idx);
    }

    fn encode_out(&mut self, op: &OpOut) {
        self.encode_alu(
            0x124,
            Some(op.dst),
            ALUSrc::from_src(&op.handle),
            ALUSrc::from_src(&op.stream),
            ALUSrc::None,
        );

        self.set_field(
            78..80,
            match op.out_type {
                OutType::Emit => 1_u8,
                OutType::Cut => 2_u8,
                OutType::EmitThenCut => 3_u8,
            },
        );
    }

    fn encode_out_final(&mut self, op: &OpOutFinal) {
        self.encode_alu(
            0x124,
            Some(Dst::None),
            ALUSrc::from_src(&op.handle),
            ALUSrc::from_src(&Src::new_zero()),
            ALUSrc::None,
        );
    }

    fn encode_vote(&mut self, op: &OpVote) {
        self.set_opcode(0x806);
        self.set_dst(op.ballot);

        self.set_field(
            72..74,
            match op.op {
                VoteOp::All => 0_u8,
                VoteOp::Any => 1_u8,
                VoteOp::Eq => 2_u8,
            },
        );

        self.set_pred_dst(81..84, op.vote);
        self.set_pred_src(87..90, 90, op.pred);
    }

    pub fn encode(
        instr: &Instr,
        sm: u8,
        ip: usize,
        labels: &HashMap<Label, usize>,
    ) -> [u32; 4] {
        assert!(sm >= 70);

        let mut si = SM70Instr {
            inst: [0; 4],
            sm: sm,
        };

        match &instr.op {
            Op::FAdd(op) => si.encode_fadd(&op),
            Op::FFma(op) => si.encode_ffma(&op),
            Op::FMnMx(op) => si.encode_fmnmx(&op),
            Op::FMul(op) => si.encode_fmul(&op),
            Op::FSet(op) => si.encode_fset(&op),
            Op::FSetP(op) => si.encode_fsetp(&op),
            Op::FSwzAdd(op) => si.encode_fswzadd(&op),
            Op::DAdd(op) => si.encode_dadd(&op),
            Op::DFma(op) => si.encode_dfma(&op),
            Op::DMul(op) => si.encode_dmul(&op),
            Op::DSetP(op) => si.encode_dsetp(&op),
            Op::MuFu(op) => si.encode_mufu(&op),
            Op::Brev(op) => si.encode_brev(&op),
            Op::Flo(op) => si.encode_flo(&op),
            Op::IAbs(op) => si.encode_iabs(&op),
            Op::IAdd3(op) => si.encode_iadd3(&op),
            Op::IAdd3X(op) => si.encode_iadd3x(&op),
            Op::IDp4(op) => si.encode_idp4(&op),
            Op::IMad(op) => si.encode_imad(&op),
            Op::IMad64(op) => si.encode_imad64(&op),
            Op::IMnMx(op) => si.encode_imnmx(&op),
            Op::ISetP(op) => si.encode_isetp(&op),
            Op::Lop3(op) => si.encode_lop3(&op),
            Op::PopC(op) => si.encode_popc(&op),
            Op::Shf(op) => si.encode_shf(&op),
            Op::F2F(op) => si.encode_f2f(&op),
            Op::F2I(op) => si.encode_f2i(&op),
            Op::I2F(op) => si.encode_i2f(&op),
            Op::FRnd(op) => si.encode_frnd(&op),
            Op::Mov(op) => si.encode_mov(&op),
            Op::Prmt(op) => si.encode_prmt(&op),
            Op::Sel(op) => si.encode_sel(&op),
            Op::Shfl(op) => si.encode_shfl(&op),
            Op::PLop3(op) => si.encode_plop3(&op),
            Op::Tex(op) => si.encode_tex(&op),
            Op::Tld(op) => si.encode_tld(&op),
            Op::Tld4(op) => si.encode_tld4(&op),
            Op::Tmml(op) => si.encode_tmml(&op),
            Op::Txd(op) => si.encode_txd(&op),
            Op::Txq(op) => si.encode_txq(&op),
            Op::SuLd(op) => si.encode_suld(&op),
            Op::SuSt(op) => si.encode_sust(&op),
            Op::SuAtom(op) => si.encode_suatom(&op),
            Op::Ld(op) => si.encode_ld(&op),
            Op::Ldc(op) => si.encode_ldc(&op),
            Op::St(op) => si.encode_st(&op),
            Op::Atom(op) => si.encode_atom(&op),
            Op::AL2P(op) => si.encode_al2p(&op),
            Op::ALd(op) => si.encode_ald(&op),
            Op::ASt(op) => si.encode_ast(&op),
            Op::Ipa(op) => si.encode_ipa(&op),
            Op::LdTram(op) => si.encode_ldtram(&op),
            Op::CCtl(op) => si.encode_cctl(&op),
            Op::MemBar(op) => si.encode_membar(&op),
            Op::BClear(op) => si.encode_bclear(&op),
            Op::BMov(op) => si.encode_bmov(&op),
            Op::Break(op) => si.encode_break(&op),
            Op::BSSy(op) => si.encode_bssy(&op, ip, labels),
            Op::BSync(op) => si.encode_bsync(&op),
            Op::Bra(op) => si.encode_bra(&op, ip, labels),
            Op::Exit(op) => si.encode_exit(&op),
            Op::WarpSync(op) => si.encode_warpsync(&op),
            Op::Bar(op) => si.encode_bar(&op),
            Op::CS2R(op) => si.encode_cs2r(&op),
            Op::Isberd(op) => si.encode_isberd(&op),
            Op::Kill(op) => si.encode_kill(&op),
            Op::Nop(op) => si.encode_nop(&op),
            Op::PixLd(op) => si.encode_pixld(&op),
            Op::S2R(op) => si.encode_s2r(&op),
            Op::Out(op) => si.encode_out(&op),
            Op::OutFinal(op) => si.encode_out_final(&op),
            Op::Vote(op) => si.encode_vote(&op),
            _ => panic!("Unhandled instruction"),
        }

        si.set_pred(&instr.pred);
        si.set_instr_deps(&instr.deps);

        si.inst
    }
}

impl Shader {
    pub fn encode_sm70(&self) -> Vec<u32> {
        assert!(self.functions.len() == 1);
        let func = &self.functions[0];

        let mut ip = 0_usize;
        let mut labels = HashMap::new();
        for b in &func.blocks {
            labels.insert(b.label, ip);
            for instr in &b.instrs {
                match &instr.op {
                    Op::Nop(op) => {
                        if let Some(label) = op.label {
                            labels.insert(label, ip);
                        }
                    }
                    _ => (),
                }
                ip += 4;
            }
        }

        let mut encoded = Vec::new();
        for b in &func.blocks {
            for instr in &b.instrs {
                let e = SM70Instr::encode(
                    instr,
                    self.info.sm,
                    encoded.len(),
                    &labels,
                );
                encoded.extend_from_slice(&e[..]);
            }
        }
        encoded
    }
}
