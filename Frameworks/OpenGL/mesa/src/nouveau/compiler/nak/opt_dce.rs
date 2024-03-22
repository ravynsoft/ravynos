// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::ir::*;

use std::collections::HashSet;

struct DeadCodePass {
    any_dead: bool,
    new_live: bool,
    live_ssa: HashSet<SSAValue>,
    live_phi: HashSet<u32>,
}

impl DeadCodePass {
    pub fn new() -> DeadCodePass {
        DeadCodePass {
            any_dead: false,
            new_live: false,
            live_ssa: HashSet::new(),
            live_phi: HashSet::new(),
        }
    }

    fn mark_ssa_live(&mut self, ssa: &SSAValue) {
        self.new_live |= self.live_ssa.insert(*ssa);
    }

    fn mark_src_live(&mut self, src: &Src) {
        if let SrcRef::SSA(ssa) = &src.src_ref {
            for val in ssa.iter() {
                self.mark_ssa_live(val);
            }
        }
    }

    fn mark_phi_live(&mut self, id: u32) {
        self.new_live |= self.live_phi.insert(id);
    }

    fn is_dst_live(&self, dst: &Dst) -> bool {
        match dst {
            Dst::SSA(ssa) => {
                for val in ssa.iter() {
                    if self.live_ssa.get(val).is_some() {
                        return true;
                    }
                }
                false
            }
            Dst::None => false,
            _ => panic!("Invalid SSA destination"),
        }
    }

    fn is_phi_live(&self, id: u32) -> bool {
        self.live_phi.get(&id).is_some()
    }

    fn is_instr_live(&self, instr: &Instr) -> bool {
        if instr.pred.is_false() {
            return false;
        }

        if !instr.can_eliminate() {
            return true;
        }

        for dst in instr.dsts() {
            if self.is_dst_live(dst) {
                return true;
            }
        }

        false
    }

    fn mark_instr(&mut self, instr: &Instr) {
        match &instr.op {
            Op::PhiSrcs(phi) => {
                assert!(instr.pred.is_true());
                for (id, src) in phi.srcs.iter() {
                    if self.is_phi_live(*id) {
                        self.mark_src_live(src);
                    } else {
                        self.any_dead = true;
                    }
                }
            }
            Op::PhiDsts(phi) => {
                assert!(instr.pred.is_true());
                for (id, dst) in phi.dsts.iter() {
                    if self.is_dst_live(dst) {
                        self.mark_phi_live(*id);
                    } else {
                        self.any_dead = true;
                    }
                }
            }
            Op::ParCopy(pcopy) => {
                assert!(instr.pred.is_true());
                for (dst, src) in pcopy.dsts_srcs.iter() {
                    if self.is_dst_live(dst) {
                        self.mark_src_live(src);
                    } else {
                        self.any_dead = true;
                    }
                }
            }
            _ => {
                if self.is_instr_live(instr) {
                    if let PredRef::SSA(ssa) = &instr.pred.pred_ref {
                        self.mark_ssa_live(ssa);
                    }

                    for src in instr.srcs() {
                        self.mark_src_live(src);
                    }
                } else {
                    self.any_dead = true;
                }
            }
        }
    }

    fn map_instr(&self, mut instr: Box<Instr>) -> MappedInstrs {
        let is_live = match &mut instr.op {
            Op::PhiSrcs(phi) => {
                phi.srcs.retain(|id, _| self.is_phi_live(*id));
                !phi.srcs.is_empty()
            }
            Op::PhiDsts(phi) => {
                phi.dsts.retain(|_, dst| self.is_dst_live(dst));
                !phi.dsts.is_empty()
            }
            Op::ParCopy(pcopy) => {
                pcopy.dsts_srcs.retain(|dst, _| self.is_dst_live(dst));
                !pcopy.dsts_srcs.is_empty()
            }
            _ => self.is_instr_live(&instr),
        };

        if is_live {
            MappedInstrs::One(instr)
        } else {
            MappedInstrs::None
        }
    }

    pub fn run(&mut self, f: &mut Function) {
        loop {
            self.new_live = false;
            self.any_dead = false;

            for b in f.blocks.iter().rev() {
                for instr in b.instrs.iter().rev() {
                    self.mark_instr(instr);
                }
            }

            if !self.new_live {
                break;
            }
        }

        if self.any_dead {
            f.map_instrs(|instr, _| self.map_instr(instr));
        }
    }
}

impl Function {
    pub fn opt_dce(&mut self) {
        DeadCodePass::new().run(self);
    }
}

impl Shader {
    pub fn opt_dce(&mut self) {
        for f in &mut self.functions {
            f.opt_dce();
        }
    }
}
