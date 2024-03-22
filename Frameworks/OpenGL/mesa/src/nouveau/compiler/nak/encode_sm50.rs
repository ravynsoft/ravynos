// Copyright © 2023 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::ir::*;
use bitview::*;

use std::collections::HashMap;
use std::ops::Range;

impl Src {
    fn is_reg_or_zero(&self) -> bool {
        match self.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => true,
            _ => false,
        }
    }
}

fn align_down(value: usize, align: usize) -> usize {
    value / align * align
}

fn align_up(value: usize, align: usize) -> usize {
    align_down(value + (align - 1), align)
}

struct SM50Instr {
    inst: [u32; 2],
    sched: u32,
    sm: u8,
}

impl BitViewable for SM50Instr {
    fn bits(&self) -> usize {
        BitView::new(&self.inst).bits()
    }

    fn get_bit_range_u64(&self, range: Range<usize>) -> u64 {
        BitView::new(&self.inst).get_bit_range_u64(range)
    }
}

impl BitMutViewable for SM50Instr {
    fn set_bit_range_u64(&mut self, range: Range<usize>, val: u64) {
        BitMutView::new(&mut self.inst).set_bit_range_u64(range, val);
    }
}

impl SetFieldU64 for SM50Instr {
    fn set_field_u64(&mut self, range: Range<usize>, val: u64) {
        BitMutView::new(&mut self.inst).set_field_u64(range, val);
    }
}

impl SM50Instr {
    fn new(sm: u8) -> Self {
        Self {
            inst: [0x0; 2],
            sched: 0x7e0,
            sm,
        }
    }

    fn nop(sm: u8) -> Self {
        let mut res = Self::new(sm);

        res.encode_nop();

        res.set_instr_deps(&InstrDeps::new());

        res
    }

    fn set_bit(&mut self, bit: usize, val: bool) {
        BitMutView::new(&mut self.inst).set_bit(bit, val);
    }

    fn set_opcode(&mut self, opcode: u16) {
        self.set_field(48..64, opcode);
    }

    fn set_pred_reg(&mut self, range: Range<usize>, reg: RegRef) {
        assert!(range.len() == 3);
        assert!(reg.file() == RegFile::Pred);
        assert!(reg.base_idx() <= 7);
        assert!(reg.comps() == 1);
        self.set_field(range, reg.base_idx());
    }

    fn set_pred(&mut self, pred: &Pred) {
        assert!(!pred.is_false());
        self.set_pred_reg(
            16..19,
            match pred.pred_ref {
                PredRef::None => RegRef::zero(RegFile::Pred, 1),
                PredRef::Reg(reg) => reg,
                PredRef::SSA(_) => panic!("SSA values must be lowered"),
            },
        );
        self.set_bit(19, pred.pred_inv);
    }

    fn set_instr_deps(&mut self, deps: &InstrDeps) {
        let mut sched = BitMutView::new(&mut self.sched);

        sched.set_field(0..4, deps.delay);
        sched.set_bit(4, deps.yld);
        sched.set_field(5..8, deps.wr_bar().unwrap_or(7));
        sched.set_field(8..11, deps.rd_bar().unwrap_or(7));
        sched.set_field(11..17, deps.wt_bar_mask);
        sched.set_field(17..21, deps.reuse_mask);
    }

    fn set_reg(&mut self, range: Range<usize>, reg: RegRef) {
        assert!(range.len() == 8);
        assert!(reg.file() == RegFile::GPR);
        self.set_field(range, reg.base_idx());
    }

    fn set_reg_src_ref(&mut self, range: Range<usize>, src_ref: SrcRef) {
        match src_ref {
            SrcRef::Zero => self.set_reg(range, RegRef::zero(RegFile::GPR, 1)),
            SrcRef::Reg(reg) => self.set_reg(range, reg),
            _ => panic!("Not a register"),
        }
    }

    fn set_reg_src(&mut self, range: Range<usize>, src: Src) {
        assert!(src.src_mod.is_none());
        self.set_reg_src_ref(range, src.src_ref);
    }

    fn set_reg_fmod_src(
        &mut self,
        range: Range<usize>,
        abs_bit: usize,
        neg_bit: usize,
        src: Src,
    ) {
        self.set_reg_src_ref(range, src.src_ref);
        self.set_bit(abs_bit, src.src_mod.has_fabs());
        self.set_bit(neg_bit, src.src_mod.has_fneg());
    }

    fn set_reg_ineg_src(
        &mut self,
        range: Range<usize>,
        neg_bit: usize,
        src: Src,
    ) {
        self.set_reg_src_ref(range, src.src_ref);
        self.set_bit(neg_bit, src.src_mod.is_ineg());
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
        self.set_bit(not_bit, not ^ src.src_mod.is_bnot());
    }

    fn set_dst(&mut self, dst: Dst) {
        let reg = match dst {
            Dst::None => RegRef::zero(RegFile::GPR, 1),
            Dst::Reg(reg) => reg,
            _ => panic!("invalid dst {dst}"),
        };
        self.set_reg(0..8, reg);
    }

    fn set_src_imm32(&mut self, range: Range<usize>, u: u32) {
        assert!(range.len() == 32);
        self.set_field(range, u);
    }

    fn set_src_imm_i20(
        &mut self,
        range: Range<usize>,
        sign_bit: usize,
        i: u32,
    ) {
        assert!(range.len() == 19);
        assert!((i & 0xfff80000) == 0 || (i & 0xfff80000) == 0xfff80000);

        self.set_field(range, i & 0x7ffff);
        self.set_field(sign_bit..sign_bit + 1, (i & 0x80000) >> 19);
    }

    fn set_src_imm_f20(
        &mut self,
        range: Range<usize>,
        sign_bit: usize,
        f: u32,
    ) {
        assert!(range.len() == 19);
        assert!((f & 0x00000fff) == 0);

        self.set_field(range, (f >> 12) & 0x7ffff);
        self.set_field(sign_bit..sign_bit + 1, f >> 31);
    }

    fn set_src_cb(&mut self, range: Range<usize>, cb: &CBufRef) {
        let mut v = BitMutView::new_subset(self, range);

        assert!(cb.offset % 4 == 0);

        v.set_field(0..14, cb.offset >> 2);
        if let CBuf::Binding(idx) = cb.buf {
            v.set_field(14..19, idx);
        } else {
            panic!("Must be a bound constant buffer");
        }
    }

    fn set_cb_fmod_src(
        &mut self,
        range: Range<usize>,
        abs_bit: usize,
        neg_bit: usize,
        src: Src,
    ) {
        if let SrcRef::CBuf(cb) = &src.src_ref {
            self.set_src_cb(range, cb);
        } else {
            panic!("Not a CBuf source");
        }

        self.set_bit(abs_bit, src.src_mod.has_fabs());
        self.set_bit(neg_bit, src.src_mod.has_fneg());
    }

    fn set_cb_ineg_src(
        &mut self,
        range: Range<usize>,
        neg_bit: usize,
        src: Src,
    ) {
        if let SrcRef::CBuf(cb) = &src.src_ref {
            self.set_src_cb(range, cb);
        } else {
            panic!("Not a CBuf source");
        }

        self.set_bit(neg_bit, src.src_mod.is_ineg());
    }

    fn encode_mov(&mut self, op: &OpMov) {
        match &op.src.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5c98);
                self.set_reg_src(20..28, op.src);
                self.set_field(39..43, op.quad_lanes);
            }
            SrcRef::Imm32(i) => {
                self.set_opcode(0x0100);
                self.set_src_imm32(20..52, *i);
                self.set_field(12..16, op.quad_lanes);
            }
            SrcRef::CBuf(cb) => {
                self.set_opcode(0x4c98);
                self.set_src_cb(20..39, cb);
                self.set_field(39..43, op.quad_lanes);
            }
            src => panic!("Unsupported src type for MOV: {src}"),
        }

        self.set_dst(op.dst);
    }

    fn encode_sel(&mut self, op: &OpSel) {
        match &op.srcs[1].src_ref {
            SrcRef::Imm32(imm32) => {
                self.set_opcode(0x38a0);
                self.set_src_imm_i20(20..39, 56, *imm32);
            }
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5ca0);
                self.set_reg_src_ref(20..28, op.srcs[1].src_ref);
            }
            SrcRef::CBuf(cbuf) => {
                self.set_opcode(0x4ca0);
                self.set_src_cb(20..39, &cbuf);
            }
            src => panic!("Unsupported src type for SEL: {src}"),
        }

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.srcs[0]);
        self.set_pred_src(39..42, 42, op.cond);
    }

    fn encode_shfl(&mut self, op: &OpShfl) {
        self.set_opcode(0xef10);

        self.set_dst(op.dst);
        self.set_pred_dst(48..51, op.in_bounds);
        self.set_reg_src(8..16, op.src);

        match op.lane.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_bit(28, false);
                self.set_reg_src(20..28, op.lane);
            }
            SrcRef::Imm32(imm) => {
                self.set_bit(28, true);
                self.set_field(20..25, imm & 0x1f);
            }
            lane => panic!("unsupported lane src type for SHFL: {lane}"),
        }
        match op.c.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_bit(29, false);
                self.set_reg_src(39..47, op.c);
            }
            SrcRef::Imm32(imm) => {
                self.set_bit(29, true);
                self.set_field(34..47, imm & 0x1f1f);
            }
            c => panic!("unsupported c src type for SHFL: {c}"),
        }

        self.set_field(
            30..32,
            match op.op {
                ShflOp::Idx => 0u8,
                ShflOp::Up => 1u8,
                ShflOp::Down => 2u8,
                ShflOp::Bfly => 3u8,
            },
        );
    }

    fn encode_vote(&mut self, op: &OpVote) {
        self.set_opcode(0x50d8);

        self.set_dst(op.ballot);
        self.set_pred_dst(45..48, op.vote);
        self.set_pred_src(39..42, 42, op.pred);

        self.set_field(
            48..50,
            match op.op {
                VoteOp::All => 0u8,
                VoteOp::Any => 1u8,
                VoteOp::Eq => 2u8,
            },
        );
    }

    fn encode_psetp(&mut self, op: &OpPSetP) {
        self.set_opcode(0x5090);

        self.set_pred_dst(3..6, op.dsts[0]);
        self.set_pred_dst(0..3, op.dsts[1]);

        self.set_pred_src(12..15, 15, op.srcs[0]);
        self.set_pred_src(29..32, 32, op.srcs[1]);
        self.set_pred_src(39..42, 42, op.srcs[2]);

        self.set_pred_set_op(24..26, op.ops[0]);
        self.set_pred_set_op(45..47, op.ops[1]);
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

    fn set_mem_order(&mut self, _order: &MemOrder) {
        // TODO: order and scope aren't present before SM70, what should we do?
    }

    fn set_mem_access(&mut self, access: &MemAccess) {
        self.set_field(
            45..46,
            match access.space.addr_type() {
                MemAddrType::A32 => 0_u8,
                MemAddrType::A64 => 1_u8,
            },
        );
        self.set_mem_type(48..51, access.mem_type);
        self.set_mem_order(&access.order);
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

    fn encode_ldg(&mut self, op: &OpLd) {
        self.set_opcode(0xeed0);

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.addr);
        self.set_field(20..44, op.offset);

        self.set_mem_access(&op.access);
    }

    fn encode_ldl(&mut self, op: &OpLd) {
        self.set_opcode(0xef40);

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.addr);
        self.set_field(20..44, op.offset);

        self.set_mem_access(&op.access);
    }

    fn encode_lds(&mut self, op: &OpLd) {
        self.set_opcode(0xef48);

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.addr);
        self.set_field(20..44, op.offset);

        self.set_mem_access(&op.access);
    }

    fn encode_ld(&mut self, op: &OpLd) {
        match op.access.space {
            MemSpace::Global(_) => self.encode_ldg(op),
            MemSpace::Local => self.encode_ldl(op),
            MemSpace::Shared => self.encode_lds(op),
        }
    }

    fn encode_ldc(&mut self, op: &OpLdc) {
        assert!(op.cb.src_mod.is_none());
        let SrcRef::CBuf(cb) = &op.cb.src_ref else {
            panic!("Not a CBuf source");
        };
        let CBuf::Binding(cb_idx) = cb.buf else {
            panic!("Must be a bound constant buffer");
        };

        self.set_opcode(0xef90);

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.offset);
        self.set_field(20..36, cb.offset);
        self.set_field(36..41, cb_idx);
        self.set_field(44..46, 0_u8); // TODO: subop
        self.set_mem_type(48..51, op.mem_type);
    }

    fn encode_stg(&mut self, op: &OpSt) {
        self.set_opcode(0xeed8);

        self.set_reg_src(0..8, op.data);
        self.set_reg_src(8..16, op.addr);
        self.set_field(20..44, op.offset);
        self.set_mem_access(&op.access);
    }

    fn encode_stl(&mut self, op: &OpSt) {
        self.set_opcode(0xef50);

        self.set_reg_src(0..8, op.data);
        self.set_reg_src(8..16, op.addr);
        self.set_field(20..44, op.offset);
        self.set_mem_access(&op.access);
    }

    fn encode_sts(&mut self, op: &OpSt) {
        self.set_opcode(0xef58);

        self.set_reg_src(0..8, op.data);
        self.set_reg_src(8..16, op.addr);
        self.set_field(20..44, op.offset);
        self.set_mem_access(&op.access);
    }

    fn encode_st(&mut self, op: &OpSt) {
        match op.access.space {
            MemSpace::Global(_) => self.encode_stg(op),
            MemSpace::Local => self.encode_stl(op),
            MemSpace::Shared => self.encode_sts(op),
        }
    }

    fn encode_lop2(&mut self, op: &OpLop2) {
        if let Some(imm32) = op.srcs[1].as_imm_not_i20() {
            self.set_opcode(0x0400);

            self.set_dst(op.dst);
            self.set_reg_src_ref(8..16, op.srcs[0].src_ref);
            self.set_bit(55, op.srcs[0].src_mod.is_bnot());
            self.set_src_imm32(20..52, imm32);

            self.set_field(
                53..55,
                match op.op {
                    LogicOp2::And => 0_u8,
                    LogicOp2::Or => 1_u8,
                    LogicOp2::Xor => 2_u8,
                    LogicOp2::PassB => {
                        panic!("PASS_B is not supported for LOP32I");
                    }
                },
            );
        } else {
            match &op.srcs[1].src_ref {
                SrcRef::Zero | SrcRef::Reg(_) => {
                    self.set_opcode(0x5c40);
                    self.set_reg_src_ref(20..28, op.srcs[1].src_ref);
                }
                SrcRef::Imm32(i) => {
                    self.set_opcode(0x3840);
                    self.set_src_imm_i20(20..39, 56, *i);
                }
                SrcRef::CBuf(cb) => {
                    self.set_opcode(0x4c40);
                    self.set_src_cb(20..39, cb);
                }
                src1 => panic!("unsupported src1 type for IMUL: {src1}"),
            }

            self.set_dst(op.dst);
            self.set_reg_src_ref(8..16, op.srcs[0].src_ref);

            self.set_bit(39, op.srcs[0].src_mod.is_bnot());
            self.set_bit(40, op.srcs[1].src_mod.is_bnot());

            self.set_field(
                41..43,
                match op.op {
                    LogicOp2::And => 0_u8,
                    LogicOp2::Or => 1_u8,
                    LogicOp2::Xor => 2_u8,
                    LogicOp2::PassB => 3_u8,
                },
            );

            self.set_pred_dst(48..51, Dst::None);
        }
    }

    fn encode_shf(&mut self, op: &OpShf) {
        match &op.shift.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5cf8);
                self.set_reg_src(20..28, op.shift);
            }
            SrcRef::Imm32(i) => {
                self.set_opcode(0x38f8);
                assert!(op.shift.src_mod.is_none());
                self.set_src_imm_i20(20..39, 56, *i);
            }
            src1 => panic!("unsupported src1 type for SHF: {src1}"),
        }

        self.set_field(
            37..39,
            match op.data_type {
                IntType::I32 => 0_u8,
                IntType::U32 => 0_u8,
                IntType::U64 => 2_u8,
                IntType::I64 => 3_u8,
                _ => panic!("Invalid shift data type"),
            },
        );

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.low);
        self.set_reg_src(39..47, op.high);

        self.set_bit(47, false); // .CC
        self.set_bit(48, op.dst_high);
        self.set_bit(49, false); // .X
        self.set_bit(50, op.wrap);
    }

    fn encode_shl(&mut self, op: &OpShl) {
        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.src);
        match op.shift.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5c48);
                self.set_reg_src(20..28, op.shift);
            }
            SrcRef::Imm32(i) => {
                self.set_opcode(0x3848);
                self.set_src_imm_i20(20..39, 56, i);
            }
            SrcRef::CBuf(cb) => {
                self.set_opcode(0x4c48);
                self.set_src_cb(20..39, &cb);
            }
            src1 => panic!("unsupported src1 type for SHL: {src1}"),
        }

        self.set_bit(39, op.wrap);
    }

    fn encode_shr(&mut self, op: &OpShr) {
        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.src);
        match op.shift.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5c28);
                self.set_reg_src(20..28, op.shift);
            }
            SrcRef::Imm32(i) => {
                self.set_opcode(0x3828);
                self.set_src_imm_i20(20..39, 56, i);
            }
            SrcRef::CBuf(cb) => {
                self.set_opcode(0x4c28);
                self.set_src_cb(20..39, &cb);
            }
            src1 => panic!("unsupported src1 type for SHL: {src1}"),
        }

        self.set_bit(39, op.wrap);
        self.set_bit(48, op.signed);
    }

    fn encode_i2f(&mut self, op: &OpI2F) {
        let abs_bit = 49;
        let neg_bit = 45;

        match &op.src.src_ref {
            SrcRef::Imm32(imm) => {
                self.set_opcode(0x38b8);
                self.set_src_imm_i20(20..39, 56, *imm);
            }
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5cb8);
                self.set_reg_fmod_src(20..28, abs_bit, neg_bit, op.src);
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4cb8);
                self.set_cb_fmod_src(20..39, abs_bit, neg_bit, op.src);
            }
            src => panic!("Unsupported src type for I2F: {src}"),
        }

        self.set_field(41..43, 0_u8); // TODO: subop
        self.set_bit(13, op.src_type.is_signed());
        self.set_field(8..10, (op.dst_type.bits() / 8).ilog2());
        self.set_rnd_mode(39..41, op.rnd_mode);
        self.set_field(10..12, (op.src_type.bits() / 8).ilog2());

        self.set_dst(op.dst);
    }

    fn encode_f2f(&mut self, op: &OpF2F) {
        assert!(op.src.is_reg_or_zero());

        let abs_bit = 49;
        let neg_bit = 45;

        match &op.src.src_ref {
            SrcRef::Imm32(imm) => {
                self.set_opcode(0x38a8);
                self.set_src_imm_i20(20..39, 56, *imm);
            }
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5ca8);
                self.set_reg_fmod_src(20..28, abs_bit, neg_bit, op.src);
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4ca8);
                self.set_cb_fmod_src(20..39, abs_bit, neg_bit, op.src);
            }
            src => panic!("Unsupported src type for F2F: {src}"),
        }

        // no saturation in the IR, would be bit 50
        self.set_field(8..10, (op.dst_type.bits() / 8).ilog2());
        self.set_field(10..12, (op.src_type.bits() / 8).ilog2());
        self.set_rnd_mode(39..41, op.rnd_mode);
        self.set_field(41..43, 0_u8); // TODO: subop
        self.set_bit(44, op.ftz);

        self.set_dst(op.dst);
    }

    fn encode_frnd(&mut self, op: &OpFRnd) {
        // FRND doesn't exist on SM50, remap it to F2F.

        self.encode_f2f(&OpF2F {
            dst: op.dst,
            src: op.src,
            src_type: op.src_type,
            dst_type: op.dst_type,
            rnd_mode: op.rnd_mode,
            ftz: op.ftz,
            high: false,
        });
    }

    fn encode_imad(&mut self, op: &OpIMad) {
        assert!(op.srcs[0].is_reg_or_zero());
        assert!(op.srcs[1].is_reg_or_zero());
        assert!(op.srcs[2].is_reg_or_zero());

        let neg_1_bit = 51;
        let neg_2_bit = 52;

        match &op.srcs[2].src_ref {
            SrcRef::Imm32(imm) => {
                panic!("Invalid immediate src2 for IMAD {}", *imm)
            }
            SrcRef::Reg(_) => match &op.srcs[1].src_ref {
                SrcRef::Imm32(imm) => {
                    self.set_opcode(0x3400);
                    self.set_src_imm_i20(20..39, 56, *imm);
                }
                SrcRef::Zero | SrcRef::Reg(_) => {
                    self.set_opcode(0x5a00);
                    self.set_reg_ineg_src(20..28, neg_1_bit, op.srcs[1]);
                }
                SrcRef::CBuf(_) => {
                    self.set_opcode(0x4a00);
                    self.set_cb_ineg_src(20..39, neg_1_bit, op.srcs[1]);
                }

                src => panic!("Invalid src1 for IMAD {src}"),
            },
            SrcRef::CBuf(_) => {
                self.set_opcode(0x5200);
                self.set_reg_ineg_src(39..47, neg_1_bit, op.srcs[1]);
                self.set_cb_ineg_src(20..39, neg_2_bit, op.srcs[2]);
            }
            src => panic!("Unsupported src2 type for F2F: {src}"),
        }

        self.set_bit(48, op.signed); // src0 signed
        self.set_bit(
            51,
            op.srcs[0].src_mod.is_ineg() ^ op.srcs[1].src_mod.is_ineg(),
        );
        self.set_bit(53, op.signed); // src1 signed

        self.set_reg_src(8..16, op.srcs[0]);
        self.set_dst(op.dst);
    }

    fn encode_imul(&mut self, op: &OpIMul) {
        assert!(op.srcs[0].src_mod.is_none());
        assert!(op.srcs[1].src_mod.is_none());

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.srcs[0]);

        if let Some(i) = op.srcs[1].as_imm_not_i20() {
            self.set_opcode(0x1fc0);
            self.set_src_imm32(20..52, i);

            self.set_bit(53, op.high);
            self.set_bit(54, op.signed[0]);
            self.set_bit(55, op.signed[1]);
        } else {
            match op.srcs[1].src_ref {
                SrcRef::Zero | SrcRef::Reg(_) => {
                    self.set_opcode(0x5c38);
                    self.set_reg_src(20..28, op.srcs[1]);
                }
                SrcRef::Imm32(i) => {
                    self.set_opcode(0x3838);
                    self.set_src_imm_i20(20..39, 56, i);
                }
                SrcRef::CBuf(cb) => {
                    self.set_opcode(0x4c38);
                    self.set_src_cb(20..39, &cb);
                }
                src1 => panic!("unsupported src1 type for IMUL: {src1}"),
            };

            self.set_bit(39, op.high);
            self.set_bit(40, op.signed[0]);
            self.set_bit(41, op.signed[1]);
        }
    }

    fn encode_f2i(&mut self, op: &OpF2I) {
        match &op.src.src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5cb0);
                self.set_reg_fmod_src(20..28, 49, 45, op.src);
            }
            SrcRef::Imm32(i) => {
                self.set_opcode(0x38b0);
                self.set_src_imm_f20(20..39, 56, *i);
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4cb0);
                self.set_cb_fmod_src(20..39, 49, 45, op.src);
            }
            src => panic!("Unsupported src type for F2I: {src}"),
        }

        self.set_dst(op.dst);

        self.set_field(8..10, (op.dst_type.bits() / 8).ilog2());
        self.set_field(10..12, (op.src_type.bits() / 8).ilog2());
        self.set_bit(12, op.dst_type.is_signed());
        self.set_rnd_mode(39..41, op.rnd_mode);
        self.set_bit(44, op.ftz);
        self.set_bit(47, false); // .CC
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

    fn encode_imnmx(&mut self, op: &OpIMnMx) {
        match &op.srcs[1].src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5c20);
                self.set_reg_src(20..28, op.srcs[1]);
            }
            SrcRef::Imm32(i) => {
                self.set_opcode(0x3820);
                self.set_src_imm_f20(20..39, 56, *i);
            }
            SrcRef::CBuf(cb) => {
                self.set_opcode(0x4c20);
                self.set_src_cb(20..39, cb);
            }
            src1 => panic!("unsupported src1 type for IMNMX: {src1}"),
        }

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.srcs[0]);
        self.set_pred_src(39..42, 42, op.min);
        self.set_bit(47, false); // .CC
        self.set_bit(
            48,
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
        assert!(op.srcs[0].src_mod.is_none());
        assert!(op.srcs[1].src_mod.is_none());

        match &op.srcs[1].src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5b60);
                self.set_reg_src(20..28, op.srcs[1]);
            }
            SrcRef::Imm32(i) => {
                self.set_opcode(0x3660);
                self.set_src_imm_i20(20..39, 56, *i);
            }
            SrcRef::CBuf(cb) => {
                self.set_opcode(0x4b60);
                self.set_src_cb(20..39, cb);
            }
            _ => panic!("Unsupported src type"),
        }

        self.set_pred_dst(0..3, Dst::None); // dst1
        self.set_pred_dst(3..6, op.dst);
        self.set_reg_src(8..16, op.srcs[0]);
        self.set_pred_src(39..42, 42, op.accum);

        self.set_bit(43, false); // .X
        self.set_pred_set_op(45..47, op.set_op);

        self.set_field(
            48..49,
            match op.cmp_type {
                IntCmpType::U32 => 0_u32,
                IntCmpType::I32 => 1_u32,
            },
        );
        self.set_int_cmp_op(49..52, op.cmp_op);
    }

    fn encode_sust(&mut self, op: &OpSuSt) {
        self.set_opcode(0xeb20);

        self.set_reg_src(8..16, op.coord);
        self.set_reg_src(0..8, op.data);
        self.set_reg_src(39..47, op.handle);

        self.set_image_dim(33..36, op.image_dim);
        self.set_mem_order(&op.mem_order);

        assert!(op.mask == 0x1 || op.mask == 0x3 || op.mask == 0xf);
        self.set_field(20..24, op.mask);
    }

    fn set_atom_op(&mut self, range: Range<usize>, atom_op: AtomOp) {
        assert!(range.len() == 4);
        self.set_field(
            range,
            match atom_op {
                AtomOp::Add => 0_u8,
                AtomOp::Min => 1_u8,
                AtomOp::Max => 2_u8,
                AtomOp::Inc => 3_u8,
                AtomOp::Dec => 4_u8,
                AtomOp::And => 5_u8,
                AtomOp::Or => 6_u8,
                AtomOp::Xor => 7_u8,
                AtomOp::Exch => 8_u8,
                AtomOp::CmpExch => panic!("CmpXchg not yet supported"),
            },
        );
    }

    fn encode_atomg(&mut self, op: &OpAtom) {
        self.set_opcode(0xed00);
        self.set_mem_order(&op.mem_order);

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.addr);
        self.set_reg_src(20..28, op.data);
        self.set_field(28..48, op.addr_offset);
        self.set_field(
            48..49,
            match op.mem_space.addr_type() {
                MemAddrType::A32 => 0_u8,
                MemAddrType::A64 => 1_u8,
            },
        );
        self.set_field(
            49..52,
            match op.atom_type {
                AtomType::U32 => 0_u8,
                AtomType::I32 => 1_u8,
                AtomType::U64 => 2_u8,
                AtomType::F32 => 3_u8,
                // NOTE: U128 => 4_u8,
                AtomType::I64 => 5_u8,
                // TODO: do something about ATOMG.F64
                other => panic!("ATOMG.{other} not supported on SM50"),
            },
        );
        self.set_atom_op(52..56, op.atom_op);
    }

    fn encode_atoms(&mut self, op: &OpAtom) {
        self.set_opcode(0xec00);
        self.set_mem_order(&op.mem_order);

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.addr);
        self.set_reg_src(20..28, op.data);
        self.set_field(
            28..30,
            match op.atom_type {
                AtomType::U32 => 0_u8,
                AtomType::I32 => 1_u8,
                AtomType::U64 => 2_u8,
                AtomType::I64 => 3_u8,
                // TODO: do something about ATOMS.F{32,64}
                other => panic!("ATOMS.{other} not supported on SM50"),
            },
        );
        assert_eq!(op.addr_offset % 4, 0);
        self.set_field(30..52, op.addr_offset / 4);
        self.set_atom_op(52..56, op.atom_op);
    }

    fn encode_atom(&mut self, op: &OpAtom) {
        match op.mem_space {
            MemSpace::Global(_) => self.encode_atomg(op),
            MemSpace::Local => panic!("Atomics do not support local"),
            MemSpace::Shared => self.encode_atoms(op),
        }
    }

    fn set_tex_dim(&mut self, range: Range<usize>, dim: TexDim) {
        assert!(range.len() == 3);
        self.set_field(
            range,
            match dim {
                TexDim::_1D => 0_u8,
                TexDim::Array1D => 1_u8,
                TexDim::_2D => 2_u8,
                TexDim::Array2D => 3_u8,
                TexDim::_3D => 4_u8,
                TexDim::Cube => 6_u8,
                TexDim::ArrayCube => 7_u8,
            },
        );
    }

    fn set_tex_lod_mode(&mut self, range: Range<usize>, lod_mode: TexLodMode) {
        assert!(range.len() == 2);
        self.set_field(
            range,
            match lod_mode {
                TexLodMode::Auto => 0_u8,
                TexLodMode::Zero => 1_u8,
                TexLodMode::Bias => 2_u8,
                TexLodMode::Lod => 3_u8,
                _ => panic!("Unknown LOD mode"),
            },
        );
    }

    fn encode_tex(&mut self, op: &OpTex) {
        self.set_opcode(0xdeb8);

        self.set_dst(op.dsts[0]);
        assert!(op.dsts[1].is_none());
        assert!(op.resident.is_none());
        self.set_reg_src(8..16, op.srcs[0]);
        self.set_reg_src(20..28, op.srcs[1]);

        self.set_tex_dim(28..31, op.dim);
        self.set_field(31..35, op.mask);
        self.set_bit(35, false); // ToDo: NDV
        self.set_tex_lod_mode(37..39, op.lod_mode);
        self.set_bit(49, false); // TODO: .NODEP
        self.set_bit(50, op.z_cmpr);
        self.set_bit(54, op.offset);
    }

    fn encode_tld(&mut self, op: &OpTld) {
        self.set_opcode(0xdd38);

        self.set_dst(op.dsts[0]);
        assert!(op.dsts[1].is_none());
        assert!(op.resident.is_none());
        self.set_reg_src(8..16, op.srcs[0]);
        self.set_reg_src(20..28, op.srcs[1]);

        self.set_tex_dim(28..31, op.dim);
        self.set_field(31..35, op.mask);
        self.set_bit(35, op.offset);
        self.set_bit(49, false); // TODO: .NODEP
        self.set_bit(50, op.is_ms);

        assert!(
            op.lod_mode == TexLodMode::Zero || op.lod_mode == TexLodMode::Lod
        );
        self.set_bit(55, op.lod_mode == TexLodMode::Zero);
    }

    fn encode_tld4(&mut self, op: &OpTld4) {
        self.set_opcode(0xdef8);

        self.set_dst(op.dsts[0]);
        assert!(op.dsts[1].is_none());
        assert!(op.resident.is_none());
        self.set_reg_src(8..16, op.srcs[0]);
        self.set_reg_src(20..28, op.srcs[1]);

        self.set_tex_dim(28..31, op.dim);
        self.set_field(31..35, op.mask);
        self.set_bit(35, false); // ToDo: NDV
        self.set_field(
            36..38,
            match op.offset_mode {
                Tld4OffsetMode::None => 0_u8,
                Tld4OffsetMode::AddOffI => 1_u8,
                Tld4OffsetMode::PerPx => 2_u8,
            },
        );
        self.set_field(38..40, op.comp);
        self.set_bit(49, false); // TODO: .NODEP
        self.set_bit(50, op.z_cmpr);
    }

    fn encode_tmml(&mut self, op: &OpTmml) {
        self.set_opcode(0xdf60);

        self.set_dst(op.dsts[0]);
        assert!(op.dsts[1].is_none());
        self.set_reg_src(8..16, op.srcs[0]);
        self.set_reg_src(20..28, op.srcs[1]);

        self.set_tex_dim(28..31, op.dim);
        self.set_field(31..35, op.mask);
        self.set_bit(35, false); // ToDo: NDV
        self.set_bit(49, false); // TODO: .NODEP
    }

    fn encode_txd(&mut self, op: &OpTxd) {
        self.set_opcode(0xde78);

        self.set_dst(op.dsts[0]);
        assert!(op.dsts[1].is_none());
        assert!(op.resident.is_none());
        self.set_reg_src(8..16, op.srcs[0]);
        self.set_reg_src(20..28, op.srcs[1]);

        self.set_tex_dim(28..31, op.dim);
        self.set_field(31..35, op.mask);
        self.set_bit(35, op.offset);
        self.set_bit(49, false); // TODO: .NODEP
    }

    fn encode_txq(&mut self, op: &OpTxq) {
        self.set_opcode(0xdf50);

        self.set_dst(op.dsts[0]);
        assert!(op.dsts[1].is_none());
        self.set_reg_src(8..16, op.src);

        self.set_field(
            22..28,
            match op.query {
                TexQuery::Dimension => 1_u8,
                TexQuery::TextureType => 2_u8,
                TexQuery::SamplerPos => 5_u8,
                // TexQuery::Filter => 0x10_u8,
                // TexQuery::Lod => 0x12_u8,
                // TexQuery::Wrap => 0x14_u8,
                // TexQuery::BorderColour => 0x16,
            },
        );
        self.set_field(31..35, op.mask);
        self.set_bit(49, false); // TODO: .NODEP
    }

    fn encode_ipa(&mut self, op: &OpIpa) {
        self.set_opcode(0xe000);

        self.set_dst(op.dst);
        self.set_reg_src(8..16, 0.into()); // addr
        self.set_reg_src(20..28, op.inv_w);
        self.set_reg_src(39..47, op.offset);

        assert!(op.addr % 4 == 0);
        self.set_field(28..38, op.addr);
        self.set_bit(38, false); // .IDX
        self.set_pred_dst(47..50, Dst::None); // TODO: What is this for?
        self.set_bit(51, false); // .SAT
        self.set_field(
            52..54,
            match op.loc {
                InterpLoc::Default => 0_u8,
                InterpLoc::Centroid => 1_u8,
                InterpLoc::Offset => 2_u8,
            },
        );
        self.set_field(
            54..56,
            match op.freq {
                InterpFreq::Pass => 0_u8,
                InterpFreq::PassMulW => 1_u8,
                InterpFreq::Constant => 2_u8,
                InterpFreq::State => 3_u8,
            },
        );
    }

    fn encode_ald(&mut self, op: &OpALd) {
        self.set_opcode(0xefd8);

        self.set_dst(op.dst);
        self.set_reg_src(8..16, op.offset);
        self.set_reg_src(39..47, op.vtx);

        assert!(!op.access.phys);
        self.set_field(20..30, op.access.addr);
        self.set_bit(31, op.access.patch);
        self.set_bit(32, op.access.output);
        self.set_field(47..49, op.access.comps - 1);
    }

    fn encode_ast(&mut self, op: &OpASt) {
        self.set_opcode(0xeff0);

        self.set_reg_src(0..8, op.data);
        self.set_reg_src(8..16, op.offset);
        self.set_reg_src(39..47, op.vtx);

        assert!(!op.access.phys);
        assert!(op.access.output);
        self.set_field(20..30, op.access.addr);
        self.set_bit(31, op.access.patch);
        self.set_bit(32, op.access.output);
        self.set_field(47..49, op.access.comps - 1);
    }

    fn encode_membar(&mut self, op: &OpMemBar) {
        self.set_opcode(0xef98);

        self.set_field(
            8..10,
            match op.scope {
                MemScope::CTA => 0_u8,
                MemScope::GPU => 1_u8,
                MemScope::System => 2_u8,
            },
        );
    }

    fn set_rel_offset(
        &mut self,
        range: Range<usize>,
        label: &Label,
        ip: usize,
        labels: &HashMap<Label, usize>,
    ) {
        let ip = u32::try_from(ip).unwrap();
        let ip = i32::try_from(ip).unwrap();

        let target_ip = *labels.get(label).unwrap();
        let target_ip = u32::try_from(target_ip).unwrap();
        let target_ip = i32::try_from(target_ip).unwrap();

        let rel_offset = target_ip - ip - 8;

        self.set_field(range, rel_offset);
    }

    fn encode_bra(
        &mut self,
        op: &OpBra,
        ip: usize,
        labels: &HashMap<Label, usize>,
    ) {
        self.set_opcode(0xe240);
        self.set_rel_offset(20..44, &op.target, ip, labels);
        self.set_field(0..5, 0xF_u8); // TODO: Pred?
    }

    fn encode_exit(&mut self, _op: &OpExit) {
        self.set_opcode(0xe300);

        // TODO: pred
        self.set_pred(&Pred {
            pred_ref: PredRef::None,
            pred_inv: false,
        });

        // TODO: CC flags
        self.set_field(0..4, 0xf_u8); // CC.T
    }

    fn encode_bar(&mut self, _op: &OpBar) {
        self.set_opcode(0xf0a8);

        self.set_reg_src(8..16, SrcRef::Zero.into());

        // 00: RED.POPC
        // 01: RED.AND
        // 02: RED.OR
        self.set_field(35..37, 0_u8);

        // 00: SYNC
        // 01: ARV
        // 02: RED
        // 03: SCAN
        self.set_field(32..35, 0_u8);

        self.set_pred_src(39..42, 42, SrcRef::True.into());
    }

    fn encode_nop(&mut self) {
        self.set_opcode(0x50b0);

        // TODO: pred
        self.set_pred(&Pred {
            pred_ref: PredRef::None,
            pred_inv: false,
        });

        // TODO: CC flags
        self.set_field(8..12, 0xf_u8); // CC.T
    }

    fn encode_s2r(&mut self, op: &OpS2R) {
        self.set_opcode(0xf0c8);
        self.set_dst(op.dst);
        self.set_field(20..28, op.idx);
    }

    fn encode_popc(&mut self, op: &OpPopC) {
        assert!(op.src.is_reg_or_zero());

        match &op.src.src_ref {
            SrcRef::Imm32(imm) => {
                self.set_opcode(0x3808);
                self.set_src_imm_i20(20..39, 56, *imm);
            }
            SrcRef::Reg(_) => {
                self.set_opcode(0x5c08);
                self.set_reg_src(20..28, op.src);
            }
            SrcRef::CBuf(cbuf) => {
                self.set_opcode(0x4c08);
                self.set_src_cb(20..39, cbuf);
            }
            src => panic!("Invalid source for POPC: {src}"),
        }

        let not_mod = matches!(op.src.src_mod, SrcMod::BNot);
        self.set_bit(40, not_mod);
        self.set_dst(op.dst);
    }

    fn encode_fadd(&mut self, op: &OpFAdd) {
        if let Some(imm32) = op.srcs[1].as_imm_not_f20() {
            self.set_opcode(0x0800);
            self.set_dst(op.dst);
            self.set_reg_fmod_src(8..16, 54, 56, op.srcs[0]);
            self.set_src_imm32(20..52, imm32);
            self.set_bit(55, op.ftz);
        } else {
            match &op.srcs[1].src_ref {
                SrcRef::Zero | SrcRef::Reg(_) => {
                    self.set_opcode(0x5c58);
                    self.set_reg_fmod_src(20..28, 49, 45, op.srcs[1]);
                }
                SrcRef::Imm32(imm) => {
                    self.set_opcode(0x3858);
                    self.set_src_imm_f20(20..39, 56, *imm);
                    assert!(op.srcs[1].src_mod.is_none());
                }
                SrcRef::CBuf(_) => {
                    self.set_opcode(0x4c58);
                    self.set_cb_fmod_src(20..39, 49, 45, op.srcs[1]);
                }
                _ => panic!("Unsupported src type"),
            }

            self.set_dst(op.dst);
            self.set_reg_fmod_src(8..16, 46, 48, op.srcs[0]);

            self.set_rnd_mode(39..41, op.rnd_mode);
            self.set_bit(44, op.ftz);
            self.set_bit(50, op.saturate);
        }
    }

    fn encode_fmnmx(&mut self, op: &OpFMnMx) {
        assert!(op.srcs[0].is_reg_or_zero());
        assert!(op.srcs[1].is_reg_or_zero());

        match &op.srcs[1].src_ref {
            SrcRef::Imm32(imm32) => {
                self.set_opcode(0x3860);
                self.set_src_imm_f20(20..39, 56, *imm32);
            }
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5c60);
                self.set_reg_fmod_src(20..28, 49, 45, op.srcs[1]);
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4c60);
                self.set_cb_fmod_src(20..39, 49, 45, op.srcs[1]);
            }
            src => panic!("Unsupported src type for FMNMX: {src}"),
        }

        self.set_reg_fmod_src(8..16, 46, 48, op.srcs[0]);
        self.set_dst(op.dst);
        self.set_pred_src(39..42, 42, op.min);
        self.set_bit(44, op.ftz);
    }

    fn encode_fmul(&mut self, op: &OpFMul) {
        assert!(op.srcs[0].is_reg_or_zero());
        assert!(op.srcs[1].is_reg_or_zero());

        if let Some(imm32) = op.srcs[1].as_imm_not_f20() {
            self.set_opcode(0x1e00);

            self.set_bit(53, op.ftz);
            self.set_bit(54, op.dnz);
            self.set_bit(55, op.saturate);

            self.set_src_imm32(20..52, imm32);
            self.set_bit(
                19,
                op.srcs[0].src_mod.has_fneg() ^ op.srcs[1].src_mod.has_fneg(),
            );
        } else {
            match &op.srcs[1].src_ref {
                SrcRef::Imm32(imm32) => {
                    self.set_opcode(0x3868);
                    self.set_src_imm_f20(20..39, 56, *imm32);
                }
                SrcRef::Zero | SrcRef::Reg(_) => {
                    self.set_opcode(0x5c68);
                    self.set_reg_src(20..28, op.srcs[1]);
                }
                SrcRef::CBuf(cbuf) => {
                    self.set_opcode(0x4c68);
                    self.set_src_cb(20..39, cbuf);
                }
                src => panic!("Unsupported src type for FMUL: {src}"),
            }

            self.set_rnd_mode(39..41, op.rnd_mode);
            self.set_field(41..44, 0x0_u8); // TODO: PDIV
            self.set_bit(44, op.ftz);
            self.set_bit(45, op.dnz);
            self.set_bit(
                48,
                op.srcs[0].src_mod.has_fneg() ^ op.srcs[1].src_mod.has_fneg(),
            );
            self.set_bit(50, op.saturate);
        }

        self.set_reg_fmod_src(8..16, 46, 48, op.srcs[0]);
        self.set_dst(op.dst);
    }

    fn encode_ffma(&mut self, op: &OpFFma) {
        // TODO: FFMA in the 32 bits immediate form use the dest as source 2
        assert!(op.srcs[1].as_imm_not_i20().is_none());

        // FFMA doesn't have any abs flags.
        assert!(!op.srcs[0].src_mod.has_fabs());
        assert!(!op.srcs[1].src_mod.has_fabs());
        assert!(!op.srcs[2].src_mod.has_fabs());

        match &op.srcs[1].src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5980);
                self.set_reg_src_ref(20..28, op.srcs[1].src_ref);
            }
            SrcRef::Imm32(i) => {
                self.set_opcode(0x3280);
                self.set_src_imm_i20(20..39, 56, *i);
            }
            SrcRef::CBuf(cb) => {
                self.set_opcode(0x4980);
                self.set_src_cb(20..39, cb);
            }
            src1 => panic!("unsupported src1 type for IMUL: {src1}"),
        }

        self.set_dst(op.dst);
        self.set_reg_src_ref(8..16, op.srcs[0].src_ref);
        self.set_reg_src_ref(39..47, op.srcs[2].src_ref);

        self.set_bit(
            48,
            op.srcs[0].src_mod.has_fneg() ^ op.srcs[1].src_mod.has_fneg(),
        );
        self.set_bit(49, op.srcs[2].src_mod.has_fneg());
        self.set_bit(50, op.saturate);
        self.set_rnd_mode(51..53, op.rnd_mode);

        self.set_bit(53, op.ftz);
        self.set_bit(54, op.dnz);
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
        assert!(op.srcs[0].is_reg_or_zero());
        assert!(op.srcs[1].is_reg_or_zero());

        match &op.srcs[1].src_ref {
            SrcRef::Imm32(imm32) => {
                self.set_opcode(0x3000);
                self.set_src_imm_f20(20..39, 56, *imm32);
            }
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5800);
                self.set_reg_fmod_src(20..28, 44, 53, op.srcs[1]);
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4800);
                self.set_cb_fmod_src(20..39, 44, 6, op.srcs[1]);
            }
            src => panic!("Unsupported src type for FSET: {src}"),
        }

        self.set_reg_fmod_src(8..16, 54, 43, op.srcs[0]);
        self.set_pred_src(39..42, 42, SrcRef::True.into());
        self.set_float_cmp_op(48..52, op.cmp_op);
        self.set_bit(52, true); // bool float
        self.set_bit(55, op.ftz);
        self.set_dst(op.dst);
    }

    fn encode_fsetp(&mut self, op: &OpFSetP) {
        assert!(op.srcs[0].is_reg_or_zero());

        match &op.srcs[1].src_ref {
            SrcRef::Imm32(imm32) => {
                self.set_opcode(0x36b0);
                self.set_src_imm_f20(20..39, 56, *imm32);
            }
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5bb0);
                self.set_reg_fmod_src(20..28, 44, 6, op.srcs[1]);
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4bb0);
                self.set_cb_fmod_src(20..39, 44, 6, op.srcs[1]);
            }
            src => panic!("Unsupported src type for FSETP: {src}"),
        }

        self.set_pred_dst(3..6, op.dst);
        self.set_pred_dst(0..3, Dst::None); // dst1
        self.set_pred_src(39..42, 42, op.accum);
        self.set_pred_set_op(45..47, op.set_op);
        self.set_bit(47, op.ftz);
        self.set_float_cmp_op(48..52, op.cmp_op);
        self.set_reg_fmod_src(8..16, 7, 43, op.srcs[0]);
    }

    fn encode_mufu(&mut self, op: &OpMuFu) {
        assert!(op.src.is_reg_or_zero());

        // TODO: This is following ALU encoding, figure out the correct form of this.
        self.set_opcode(0x5080);

        self.set_dst(op.dst);
        self.set_reg_fmod_src(8..16, 46, 48, op.src);

        self.set_field(
            20..24,
            match op.op {
                MuFuOp::Cos => 0_u8,
                MuFuOp::Sin => 1_u8,
                MuFuOp::Exp2 => 2_u8,
                MuFuOp::Log2 => 3_u8,
                MuFuOp::Rcp => 4_u8,
                MuFuOp::Rsq => 5_u8,
                MuFuOp::Rcp64H => 6_u8,
                MuFuOp::Rsq64H => 7_u8,
                // SQRT is only on SM52 and later
                MuFuOp::Sqrt if self.sm >= 52 => 8_u8,
                MuFuOp::Sqrt => panic!("MUFU.SQRT not supported on SM50"),
                MuFuOp::Tanh => panic!("MUFU.TANH not supported on SM50"),
            },
        );
    }

    fn encode_dadd(&mut self, op: &OpDAdd) {
        match &op.srcs[1].src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5c70);
                self.set_reg_fmod_src(20..28, 49, 45, op.srcs[1]);
            }
            SrcRef::Imm32(imm) => {
                self.set_opcode(0x3870);
                self.set_src_imm_f20(20..39, 56, *imm);
                assert!(op.srcs[1].src_mod.is_none());
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4c70);
                self.set_cb_fmod_src(20..39, 49, 45, op.srcs[1]);
            }
            _ => panic!("Unsupported src type"),
        }

        self.set_dst(op.dst);
        self.set_reg_fmod_src(8..16, 46, 48, op.srcs[0]);
        self.set_rnd_mode(39..41, op.rnd_mode);
    }

    fn encode_dfma(&mut self, op: &OpDFma) {
        match &op.srcs[2].src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                match &op.srcs[1].src_ref {
                    SrcRef::Zero | SrcRef::Reg(_) => {
                        self.set_opcode(0x5b70);
                        self.set_reg_src_ref(20..28, op.srcs[1].src_ref);
                    }
                    SrcRef::Imm32(imm) => {
                        self.set_opcode(0x3670);
                        self.set_src_imm_f20(20..39, 56, *imm);
                        assert!(op.srcs[1].src_mod.is_none());
                    }
                    SrcRef::CBuf(cb) => {
                        self.set_opcode(0x4b70);
                        self.set_src_cb(20..39, cb);
                    }
                    _ => panic!("Invalid dfma src1: {}", op.srcs[1]),
                }
                self.set_reg_src_ref(39..47, op.srcs[2].src_ref);
            }
            SrcRef::CBuf(cb) => {
                self.set_opcode(0x5370);
                self.set_reg_src_ref(39..47, op.srcs[1].src_ref);
                self.set_src_cb(20..39, cb);
            }
            _ => panic!("Invalid dfma src2: {}", op.srcs[2]),
        }

        self.set_dst(op.dst);
        self.set_reg_src_ref(8..16, op.srcs[0].src_ref);

        assert!(!op.srcs[0].src_mod.has_fabs());
        assert!(!op.srcs[1].src_mod.has_fabs());
        assert!(!op.srcs[2].src_mod.has_fabs());
        self.set_bit(
            48,
            op.srcs[0].src_mod.has_fneg() ^ op.srcs[1].src_mod.has_fneg(),
        );
        self.set_bit(49, op.srcs[2].src_mod.has_fneg());

        self.set_rnd_mode(50..52, op.rnd_mode);
    }

    fn encode_dmnmx(&mut self, op: &OpDMnMx) {
        match &op.srcs[1].src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5c50);
                self.set_reg_fmod_src(20..28, 49, 45, op.srcs[1]);
            }
            SrcRef::Imm32(imm32) => {
                self.set_opcode(0x3850);
                self.set_src_imm_f20(20..39, 56, *imm32);
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4c50);
                self.set_cb_fmod_src(20..39, 49, 45, op.srcs[1]);
            }
            src => panic!("Unsupported src type for FMNMX: {src}"),
        }

        self.set_reg_fmod_src(8..16, 46, 48, op.srcs[0]);
        self.set_dst(op.dst);
        self.set_pred_src(39..42, 42, op.min);
    }

    fn encode_dmul(&mut self, op: &OpDMul) {
        match &op.srcs[1].src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5c80);
                self.set_reg_src_ref(20..28, op.srcs[1].src_ref);
            }
            SrcRef::Imm32(imm) => {
                self.set_opcode(0x3880);
                self.set_src_imm_f20(20..39, 56, *imm);
                assert!(op.srcs[1].src_mod.is_none());
            }
            SrcRef::CBuf(cb) => {
                self.set_opcode(0x4c80);
                self.set_src_cb(20..39, cb);
            }
            _ => panic!("Invalid dmul src1: {}", op.srcs[1]),
        }

        self.set_dst(op.dst);
        self.set_reg_src_ref(8..16, op.srcs[0].src_ref);

        self.set_rnd_mode(39..41, op.rnd_mode);

        assert!(!op.srcs[0].src_mod.has_fabs());
        assert!(!op.srcs[1].src_mod.has_fabs());
        self.set_bit(
            48,
            op.srcs[0].src_mod.has_fneg() ^ op.srcs[1].src_mod.has_fneg(),
        );
    }

    fn encode_dsetp(&mut self, op: &OpDSetP) {
        match &op.srcs[1].src_ref {
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5b80);
                self.set_reg_fmod_src(20..28, 44, 6, op.srcs[1]);
            }
            SrcRef::Imm32(imm) => {
                self.set_opcode(0x3680);
                self.set_src_imm_f20(20..39, 56, *imm);
                assert!(op.srcs[1].src_mod.is_none());
            }
            SrcRef::CBuf(_) => {
                self.set_opcode(0x4b80);
                self.set_reg_fmod_src(20..39, 44, 6, op.srcs[1]);
            }
            _ => panic!("Invalid dmul src1: {}", op.srcs[1]),
        }

        self.set_pred_dst(3..6, op.dst);
        self.set_pred_dst(0..3, Dst::None); // dst1
        self.set_pred_src(39..42, 42, op.accum);
        self.set_pred_set_op(45..47, op.set_op);
        self.set_float_cmp_op(48..52, op.cmp_op);
        self.set_reg_fmod_src(8..16, 7, 43, op.srcs[0]);
    }

    fn encode_iabs(&mut self, op: &OpIAbs) {
        assert!(op.src.is_reg_or_zero());

        // IABS isn't a thing on SM50, we use I2I instead.

        // We always assume 32bits signed for now
        let src_type = IntType::I32;
        let dst_type = IntType::I32;

        match &op.src.src_ref {
            SrcRef::Imm32(imm32) => {
                self.set_opcode(0x38e0);
                self.set_src_imm_i20(20..39, 56, *imm32);
            }
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5ce0);
                self.set_reg_src(20..28, op.src);
            }
            SrcRef::CBuf(cbuf) => {
                self.set_opcode(0x4ce0);
                self.set_src_cb(20..39, cbuf);
            }
            src => panic!("Unsupported src type for IABS: {src}"),
        }
        self.set_bit(12, dst_type.is_signed());
        self.set_bit(13, src_type.is_signed());
        self.set_field(8..10, (dst_type.bits() / 8).ilog2());
        self.set_field(10..12, (src_type.bits() / 8).ilog2());
        self.set_dst(op.dst);
    }

    fn encode_iadd2(&mut self, op: &OpIAdd2) {
        let carry_in = match op.carry_in.src_ref {
            SrcRef::Reg(reg) if reg.file() == RegFile::Carry => true,
            SrcRef::Zero => false,
            other => panic!("invalid carry_in src for IADD2 {other}"),
        };
        let carry_out = match op.carry_out {
            Dst::Reg(reg) if reg.file() == RegFile::Carry => true,
            Dst::None => false,
            other => panic!("invalid carry_out dst for IADD2 {other}"),
        };

        if let Some(imm32) = op.srcs[1].as_imm_not_i20() {
            self.set_opcode(0x1c00);

            self.set_dst(op.dst);
            self.set_reg_ineg_src(8..16, 56, op.srcs[0]);
            self.set_src_imm32(20..52, imm32);

            self.set_bit(53, carry_in);
            self.set_bit(52, carry_out);
        } else {
            match &op.srcs[1].src_ref {
                SrcRef::Zero | SrcRef::Reg(_) => {
                    self.set_opcode(0x5c10);
                    self.set_reg_ineg_src(20..28, 48, op.srcs[1]);
                }
                SrcRef::Imm32(imm) => {
                    self.set_opcode(0x3810);
                    self.set_src_imm_i20(20..39, 56, *imm);
                }
                SrcRef::CBuf(_) => {
                    self.set_opcode(0x4c10);
                    self.set_cb_ineg_src(20..39, 48, op.srcs[1]);
                }
                src => panic!("Unsupported src type for IADD: {src}"),
            }

            self.set_dst(op.dst);
            self.set_reg_ineg_src(8..16, 49, op.srcs[0]);

            self.set_bit(43, carry_in);
            self.set_bit(47, carry_out);
        }
    }

    fn encode_prmt(&mut self, op: &OpPrmt) {
        assert!(op.srcs[0].is_reg_or_zero());
        assert!(op.sel.is_reg_or_zero());
        assert!(op.srcs[1].is_reg_or_zero());

        match &op.sel.src_ref {
            SrcRef::Imm32(imm) => {
                self.set_opcode(0x36c0);
                self.set_src_imm_i20(20..39, 56, *imm);
            }
            SrcRef::Zero | SrcRef::Reg(_) => {
                self.set_opcode(0x5bc0);
                self.set_reg_src(20..28, op.sel);
            }
            SrcRef::CBuf(cbuf) => {
                self.set_opcode(0x4bc0);
                self.set_src_cb(20..39, cbuf);
            }
            src => panic!("Unsupported src type for PRMT: {src}"),
        }

        self.set_reg_src(8..16, op.srcs[0]);
        self.set_reg_src(39..47, op.srcs[1]);
        self.set_dst(op.dst);
        // TODO: subop?
    }

    pub fn encode(
        instr: &Instr,
        sm: u8,
        ip: usize,
        labels: &HashMap<Label, usize>,
    ) -> Self {
        assert!(sm >= 50);

        let mut si = SM50Instr::new(sm);

        match &instr.op {
            Op::FAdd(op) => si.encode_fadd(&op),
            Op::FMnMx(op) => si.encode_fmnmx(&op),
            Op::FMul(op) => si.encode_fmul(&op),
            Op::FFma(op) => si.encode_ffma(&op),
            Op::FSet(op) => si.encode_fset(&op),
            Op::FSetP(op) => si.encode_fsetp(&op),
            Op::MuFu(op) => si.encode_mufu(&op),
            Op::DAdd(op) => si.encode_dadd(&op),
            Op::DFma(op) => si.encode_dfma(&op),
            Op::DMnMx(op) => si.encode_dmnmx(&op),
            Op::DMul(op) => si.encode_dmul(&op),
            Op::DSetP(op) => si.encode_dsetp(&op),
            Op::IAbs(op) => si.encode_iabs(&op),
            Op::IAdd2(op) => si.encode_iadd2(&op),
            Op::Mov(op) => si.encode_mov(&op),
            Op::Sel(op) => si.encode_sel(&op),
            Op::Shfl(op) => si.encode_shfl(&op),
            Op::Vote(op) => si.encode_vote(&op),
            Op::PSetP(op) => si.encode_psetp(&op),
            Op::SuSt(op) => si.encode_sust(&op),
            Op::S2R(op) => si.encode_s2r(&op),
            Op::PopC(op) => si.encode_popc(&op),
            Op::Prmt(op) => si.encode_prmt(&op),
            Op::Ld(op) => si.encode_ld(&op),
            Op::Ldc(op) => si.encode_ldc(&op),
            Op::St(op) => si.encode_st(&op),
            Op::Lop2(op) => si.encode_lop2(&op),
            Op::Shf(op) => si.encode_shf(&op),
            Op::Shl(op) => si.encode_shl(&op),
            Op::Shr(op) => si.encode_shr(&op),
            Op::F2F(op) => si.encode_f2f(&op),
            Op::F2I(op) => si.encode_f2i(&op),
            Op::I2F(op) => si.encode_i2f(&op),
            Op::FRnd(op) => si.encode_frnd(&op),
            Op::IMad(op) => si.encode_imad(&op),
            Op::IMul(op) => si.encode_imul(&op),
            Op::IMnMx(op) => si.encode_imnmx(&op),
            Op::ISetP(op) => si.encode_isetp(&op),
            Op::Tex(op) => si.encode_tex(&op),
            Op::Tld(op) => si.encode_tld(&op),
            Op::Tld4(op) => si.encode_tld4(&op),
            Op::Tmml(op) => si.encode_tmml(&op),
            Op::Txd(op) => si.encode_txd(&op),
            Op::Txq(op) => si.encode_txq(&op),
            Op::Ipa(op) => si.encode_ipa(&op),
            Op::ALd(op) => si.encode_ald(&op),
            Op::ASt(op) => si.encode_ast(&op),
            Op::MemBar(op) => si.encode_membar(&op),
            Op::Atom(op) => si.encode_atom(&op),
            Op::Bra(op) => si.encode_bra(&op, ip, labels),
            Op::Exit(op) => si.encode_exit(&op),
            Op::Bar(op) => si.encode_bar(&op),
            _ => panic!("Unhandled instruction {}", instr.op),
        }

        si.set_pred(&instr.pred);
        si.set_instr_deps(&instr.deps);

        si
    }
}

fn encode_instr(
    instr_index: usize,
    instr: Option<&Box<Instr>>,
    sm: u8,
    labels: &HashMap<Label, usize>,
    ip: &mut usize,
    sched_instr: &mut [u32; 2],
) -> [u32; 2] {
    let res = instr
        .map(|x| SM50Instr::encode(x, sm, *ip, labels))
        .unwrap_or_else(|| SM50Instr::nop(sm));

    *ip += 8;

    BitMutView::new(sched_instr)
        .set_field(21 * instr_index..21 * (instr_index + 1), res.sched);

    res.inst
}

impl Shader {
    pub fn encode_sm50(&self) -> Vec<u32> {
        assert!(self.functions.len() == 1);
        let func = &self.functions[0];

        let mut num_instrs = 0_usize;
        let mut labels = HashMap::new();
        for b in &func.blocks {
            // We ensure blocks will have groups of 3 instructions with a
            // schedule instruction before each groups.  As we should never jump
            // to a schedule instruction, we account for that here.
            labels.insert(b.label, num_instrs + 8);

            let block_num_instrs = align_up(b.instrs.len(), 3);

            // Every 3 instructions, we have a new schedule instruction so we
            // need to account for that.
            num_instrs += (block_num_instrs + (block_num_instrs / 3)) * 8;
        }

        let mut encoded = Vec::new();
        for b in &func.blocks {
            // A block is composed of groups of 3 instructions.
            let block_num_instrs = align_up(b.instrs.len(), 3);

            let mut instrs_iter = b.instrs.iter();

            for _ in 0..(block_num_instrs / 3) {
                let mut ip = ((encoded.len() / 2) + 1) * 8;

                let mut sched_instr = [0x0; 2];

                let instr0 = encode_instr(
                    0,
                    instrs_iter.next(),
                    self.info.sm,
                    &labels,
                    &mut ip,
                    &mut sched_instr,
                );
                let instr1 = encode_instr(
                    1,
                    instrs_iter.next(),
                    self.info.sm,
                    &labels,
                    &mut ip,
                    &mut sched_instr,
                );
                let instr2 = encode_instr(
                    2,
                    instrs_iter.next(),
                    self.info.sm,
                    &labels,
                    &mut ip,
                    &mut sched_instr,
                );

                encoded.extend_from_slice(&sched_instr[..]);
                encoded.extend_from_slice(&instr0[..]);
                encoded.extend_from_slice(&instr1[..]);
                encoded.extend_from_slice(&instr2[..]);
            }
        }

        encoded
    }
}
