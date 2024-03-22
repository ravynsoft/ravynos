// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::ir::*;

use std::collections::HashMap;
use std::slice;

struct LopEntry {
    op: LogicOp3,
    srcs_used: u8,
    srcs: [Src; 3],
}

struct LopPass {
    use_counts: HashMap<SSAValue, u32>,
    ssa_lop: HashMap<SSAValue, LopEntry>,
}

fn src_as_bool(src: &Src) -> Option<bool> {
    assert!(src.src_mod.is_none());
    match src.src_ref {
        SrcRef::Zero | SrcRef::False | SrcRef::Imm32(0) => Some(false),
        SrcRef::True | SrcRef::Imm32(u32::MAX) => Some(true),
        _ => return None,
    }
}

impl LopPass {
    fn new(f: &Function) -> LopPass {
        let mut use_counts = HashMap::new();
        for b in &f.blocks {
            for instr in &b.instrs {
                if let PredRef::SSA(ssa) = instr.pred.pred_ref {
                    use_counts.entry(ssa).and_modify(|e| *e += 1).or_insert(1);
                }

                for src in instr.srcs() {
                    if let SrcRef::SSA(vec) = src.src_ref {
                        for ssa in vec.iter() {
                            use_counts
                                .entry(*ssa)
                                .and_modify(|e| *e += 1)
                                .or_insert(1);
                        }
                    }
                }
            }
        }
        LopPass {
            use_counts: use_counts,
            ssa_lop: HashMap::new(),
        }
    }

    fn add_lop(&mut self, ssa: SSAValue, op: LogicOp3, srcs: [Src; 3]) {
        let mut srcs_used = 0;
        for i in 0..3 {
            if op.src_used(i) {
                srcs_used |= 1 << i;
                assert!(src_as_bool(&srcs[i]).is_none());
            }
        }
        let entry = LopEntry {
            op: op,
            srcs_used: srcs_used,
            srcs: srcs,
        };
        self.ssa_lop.insert(ssa, entry);
    }

    fn dedup_srcs(&self, op: &mut LogicOp3, srcs: &[Src; 3]) {
        for i in 0..2 {
            for j in (i + 1)..3 {
                if srcs[i].src_ref == srcs[j].src_ref {
                    *op = LogicOp3::new_lut(&|x, y, z| {
                        let dup = [x, y, z][i];
                        let si = match srcs[i].src_mod {
                            SrcMod::None => dup,
                            SrcMod::BNot => !dup,
                            _ => panic!("Not a bitwise modifer"),
                        };
                        let sj = match srcs[j].src_mod {
                            SrcMod::None => dup,
                            SrcMod::BNot => !dup,
                            _ => panic!("Not a bitwise modifer"),
                        };

                        let mut s = [x, y, z];
                        s[i] = si;
                        s[j] = sj;

                        op.eval(s[0], s[1], s[2])
                    });
                }
            }
        }
    }

    fn try_prop_to_src(
        &self,
        ops: &mut [LogicOp3],
        srcs: &mut [Src; 3],
        src_idx: usize,
    ) {
        loop {
            assert!(srcs[src_idx].src_mod.is_none());
            let ssa = match srcs[src_idx].src_ref {
                SrcRef::SSA(vec) => {
                    assert!(vec.comps() == 1);
                    vec[0]
                }
                _ => return,
            };

            let Some(entry) = self.ssa_lop.get(&ssa) else {
                return;
            };

            let entry_use_count = *self.use_counts.get(&ssa).unwrap();
            if entry.srcs_used.count_ones() > 1 && entry_use_count > 1 {
                return;
            }

            let mut entry_srcs = [usize::MAX; 3];
            let mut next_src = 0_usize;
            for i in 0..3 {
                if entry.srcs_used & (1 << i) == 0 {
                    continue;
                }

                let mut found = false;
                for j in 0..3 {
                    if entry.srcs[i].src_ref == srcs[j].src_ref {
                        entry_srcs[i] = j;
                        found = true;
                        break;
                    }
                }
                if found {
                    continue;
                }

                loop {
                    if next_src >= srcs.len() {
                        return;
                    }

                    // All callers of this function need to ensure that
                    // constant sources are already folded so we know we
                    // can always re-use them.
                    if next_src == src_idx
                        || src_as_bool(&srcs[next_src]).is_some()
                    {
                        entry_srcs[i] = next_src;
                        next_src += 1;
                        break;
                    }
                    next_src += 1;
                }
            }

            // Clear out the propagated source. What we put here doesn't matter
            // since it's no longer used.  It may be overwritten by one of the
            // entry sources but there is no guarantee of this.
            srcs[src_idx] = match ssa.file() {
                RegFile::GPR | RegFile::UGPR => SrcRef::Zero.into(),
                RegFile::Pred | RegFile::UPred => SrcRef::True.into(),
                RegFile::Carry | RegFile::Bar | RegFile::Mem => {
                    panic!("Not a normal register");
                }
            };

            for i in 0..3 {
                if entry_srcs[i] != usize::MAX {
                    srcs[entry_srcs[i]] = entry.srcs[i];
                }
            }
            for op in ops.iter_mut() {
                *op = LogicOp3::new_lut(&|x, y, z| {
                    let mut s = [x, y, z];
                    let mut es = [0; 3];
                    for i in 0..3 {
                        if entry_srcs[i] != usize::MAX {
                            es[i] = s[entry_srcs[i]];
                        }
                    }
                    let e = entry.op.eval(es[0], es[1], es[2]);
                    s[src_idx] = e;
                    op.eval(s[0], s[1], s[2])
                });
            }
        }
    }

    fn opt_lop3(&mut self, op: &mut OpLop3) {
        self.dedup_srcs(&mut op.op, &op.srcs);

        for (i, src) in op.srcs.iter_mut().enumerate() {
            assert!(src.src_mod.is_none());

            if let Some(b) = src_as_bool(src) {
                op.op.fix_src(i, b);
            }

            if !op.op.src_used(i) {
                // Replace unused sources with RZ
                *src = SrcRef::Zero.into();
            }
        }

        for i in 0..3 {
            self.try_prop_to_src(slice::from_mut(&mut op.op), &mut op.srcs, i);
        }

        if let Dst::SSA(ssa) = op.dst {
            assert!(ssa.comps() == 1);
            self.add_lop(ssa[0], op.op, op.srcs);
        }
    }

    fn opt_plop3(&mut self, op: &mut OpPLop3) {
        self.dedup_srcs(&mut op.ops[0], &op.srcs);
        self.dedup_srcs(&mut op.ops[1], &op.srcs);

        // Replace unused sources with PT
        for (i, src) in op.srcs.iter_mut().enumerate() {
            if src.src_mod.is_bnot() {
                op.ops[0].invert_src(i);
                op.ops[1].invert_src(i);
                src.src_mod = SrcMod::None;
            }

            if let Some(b) = src_as_bool(src) {
                op.ops[0].fix_src(i, b);
                op.ops[1].fix_src(i, b);
            }

            if !op.ops[0].src_used(i) && !op.ops[1].src_used(i) {
                *src = SrcRef::True.into();
            }
        }

        for i in 0..3 {
            self.try_prop_to_src(&mut op.ops, &mut op.srcs, i);
        }

        for i in 0..2 {
            if let Dst::SSA(ssa) = op.dsts[i] {
                assert!(ssa.comps() == 1);
                self.add_lop(ssa[0], op.ops[i], op.srcs);
            }
        }
    }

    fn run(&mut self, f: &mut Function) {
        for b in &mut f.blocks {
            for instr in &mut b.instrs {
                match &mut instr.op {
                    Op::Lop3(op) => self.opt_lop3(op),
                    Op::PLop3(op) => self.opt_plop3(op),
                    _ => (),
                }
            }
        }
    }
}

impl Shader {
    pub fn opt_lop(&mut self) {
        for f in &mut self.functions {
            let mut pass = LopPass::new(f);
            pass.run(f);
        }
    }
}
