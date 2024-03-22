// Copyright © 2023 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::bitset::BitSet;
use crate::ir::*;

use std::cell::RefCell;
use std::collections::HashMap;

struct Phi {
    idx: u32,
    orig: SSAValue,
    dst: SSAValue,
    srcs: HashMap<usize, SSAValue>,
}

struct DefTrackerBlock {
    pred: Vec<usize>,
    succ: Vec<usize>,
    defs: RefCell<HashMap<SSAValue, SSAValue>>,
    phis: RefCell<Vec<Phi>>,
}

fn get_ssa_or_phi(
    ssa_alloc: &mut SSAValueAllocator,
    phi_alloc: &mut PhiAllocator,
    blocks: &[DefTrackerBlock],
    needs_src: &mut BitSet,
    b_idx: usize,
    ssa: SSAValue,
) -> SSAValue {
    let b = &blocks[b_idx];
    let mut b_defs = b.defs.borrow_mut();
    if let Some(ssa) = b_defs.get(&ssa) {
        return *ssa;
    }

    let mut pred_ssa = None;
    let mut all_same = true;
    for p_idx in &b.pred {
        if *p_idx >= b_idx {
            // This is a loop back-edge, add a phi just in case.  We'll remove
            // it later if it's not needed
            all_same = false;
        } else {
            let p_ssa = get_ssa_or_phi(
                ssa_alloc, phi_alloc, blocks, needs_src, *p_idx, ssa,
            );
            if *pred_ssa.get_or_insert(p_ssa) != p_ssa {
                all_same = false;
            }
        }
    }

    if all_same {
        let pred_ssa = pred_ssa.expect("Undefined value");
        b_defs.insert(ssa, pred_ssa);
        pred_ssa
    } else {
        let phi_idx = phi_alloc.alloc();
        let phi_ssa = ssa_alloc.alloc(ssa.file());
        let mut phi = Phi {
            idx: phi_idx,
            orig: ssa,
            dst: phi_ssa,
            srcs: HashMap::new(),
        };
        for p_idx in &b.pred {
            if *p_idx >= b_idx {
                needs_src.insert(*p_idx);
                continue;
            }
            // The earlier recursive call ensured this exists
            let p_ssa = *blocks[*p_idx].defs.borrow().get(&ssa).unwrap();
            phi.srcs.insert(*p_idx, p_ssa);
        }
        b.phis.borrow_mut().push(phi);
        b_defs.insert(ssa, phi_ssa);
        phi_ssa
    }
}

fn get_or_insert_phi_dsts<'a>(bb: &'a mut BasicBlock) -> &'a mut OpPhiDsts {
    let has_phi = match &bb.instrs[0].op {
        Op::PhiDsts(_) => true,
        _ => false,
    };
    if !has_phi {
        bb.instrs.insert(0, Instr::new_boxed(OpPhiDsts::new()));
    }
    match &mut bb.instrs[0].op {
        Op::PhiDsts(phi) => phi,
        _ => panic!("Expected to find the phi we just inserted"),
    }
}

fn get_or_insert_phi_srcs<'a>(bb: &'a mut BasicBlock) -> &'a mut OpPhiSrcs {
    let mut has_phi = false;
    let mut ip = bb.instrs.len();
    for (i, instr) in bb.instrs.iter_mut().enumerate().rev() {
        match &mut instr.op {
            Op::PhiSrcs(_) => {
                ip = i;
                has_phi = true;
                break;
            }
            _ => {
                if instr.is_branch() {
                    ip = i;
                } else {
                    break;
                }
            }
        }
    }
    if !has_phi {
        bb.instrs.insert(ip, Instr::new_boxed(OpPhiSrcs::new()));
    }
    match &mut bb.instrs[ip].op {
        Op::PhiSrcs(phi) => phi,
        _ => panic!("Expected to find the phi we just inserted"),
    }
}

impl Function {
    /// Repairs SSA form
    ///
    /// Certain passes such as register spilling may produce a program that is
    /// no longer in SSA form.  This pass is able to repair SSA by inserting
    /// phis as needed.  Even though we do not require dominance or that each
    /// value be defined once we do require that, for every use of an SSAValue
    /// and for every path from the start of the program to that use, there must
    /// be some definition of the value along that path.
    ///
    /// The algorithm implemented here is based on the one in "Simple and
    /// Efficient Construction of Static Single Assignment Form" by Braun, et.
    /// al.  The primary difference between our implementation and the paper is
    /// that we can't rewrite the IR on-the-fly.  Instead, we store everything
    /// in hash tables and handle removing redundant phis with back-edges as a
    /// separate pass between figuring out where phis are needed and actually
    /// constructing the phi instructions.
    pub fn repair_ssa(&mut self) {
        // First, count the number of defs for each SSA value.  This will allow
        // us to skip any SSA values which only have a single definition in
        // later passes.
        let mut has_mult_defs = false;
        let mut num_defs = HashMap::new();
        for b in &self.blocks {
            for instr in &b.instrs {
                instr.for_each_ssa_def(|ssa| {
                    num_defs
                        .entry(*ssa)
                        .and_modify(|e| {
                            has_mult_defs = true;
                            *e += 1;
                        })
                        .or_insert(1);
                });
            }
        }

        if !has_mult_defs {
            return;
        }

        let cfg = &mut self.blocks;
        let ssa_alloc = &mut self.ssa_alloc;
        let phi_alloc = &mut self.phi_alloc;

        let mut blocks = Vec::new();
        let mut needs_src = BitSet::new();
        for b_idx in 0..cfg.len() {
            assert!(blocks.len() == b_idx);
            blocks.push(DefTrackerBlock {
                pred: cfg.pred_indices(b_idx).to_vec(),
                succ: cfg.succ_indices(b_idx).to_vec(),
                defs: RefCell::new(HashMap::new()),
                phis: RefCell::new(Vec::new()),
            });

            for instr in &mut cfg[b_idx].instrs {
                instr.for_each_ssa_use_mut(|ssa| {
                    if num_defs.get(ssa).cloned().unwrap_or(0) > 1 {
                        *ssa = get_ssa_or_phi(
                            ssa_alloc,
                            phi_alloc,
                            &blocks,
                            &mut needs_src,
                            b_idx,
                            *ssa,
                        );
                    }
                });

                instr.for_each_ssa_def_mut(|ssa| {
                    if num_defs.get(ssa).cloned().unwrap_or(0) > 1 {
                        let new_ssa = ssa_alloc.alloc(ssa.file());
                        blocks[b_idx].defs.borrow_mut().insert(*ssa, new_ssa);
                        *ssa = new_ssa;
                    }
                });
            }
        }

        // Populate phi sources for any back-edges
        loop {
            let Some(b_idx) = needs_src.next_set(0) else {
                break;
            };
            needs_src.remove(b_idx);

            for s_idx in &blocks[b_idx].succ {
                if *s_idx <= b_idx {
                    let s = &blocks[*s_idx];

                    // We do a mutable borrow here.  The algorithm is recursive
                    // and may insert phis into other blocks.  However, because
                    // this is phi exists, its destination should be in the def
                    // set for s and so no new phis should need to be added.
                    // RefCell's dynamic borrow checks will assert this.
                    for phi in s.phis.borrow_mut().iter_mut() {
                        phi.srcs.entry(b_idx).or_insert_with(|| {
                            get_ssa_or_phi(
                                ssa_alloc,
                                phi_alloc,
                                &blocks,
                                &mut needs_src,
                                b_idx,
                                phi.orig,
                            )
                        });
                    }
                }
            }
        }

        // For loop back-edges, we inserted a phi whether we need one or not.
        // We want to eliminate any redundant phis.
        let mut ssa_map = HashMap::new();
        if cfg.has_loop() {
            let mut to_do = true;
            while to_do {
                to_do = false;
                for b_idx in 0..cfg.len() {
                    let b = &blocks[b_idx];
                    b.phis.borrow_mut().retain_mut(|phi| {
                        let mut ssa = None;
                        for (_, p_ssa) in phi.srcs.iter_mut() {
                            // Apply the remap to the phi sources so that we
                            // pick up any remaps from previous loop iterations.
                            loop {
                                if let Some(new_ssa) = ssa_map.get(p_ssa) {
                                    *p_ssa = *new_ssa;
                                } else {
                                    break;
                                }
                            }

                            if *p_ssa == phi.dst {
                                continue;
                            }
                            if *ssa.get_or_insert(*p_ssa) != *p_ssa {
                                // Multiple unique sources
                                return true;
                            }
                        }

                        // All sources are identical or the phi destination so
                        // we can delete this phi and add it to the remap
                        let ssa = ssa.expect("Circular SSA def");
                        ssa_map.insert(phi.dst, ssa);
                        to_do = true;
                        false
                    });
                }
            }
        }

        // Now we apply the remap to instruction sources and place the actual
        // phis
        for b_idx in 0..cfg.len() {
            // Grab the successor index for inserting OpPhiSrc before we take a
            // mutable reference to the CFG.  There are no critical edges so we
            // can only have an OpPhiSrc if there is a single successor.
            let succ = cfg.succ_indices(b_idx);
            let s_idx = if succ.len() == 1 {
                Some(succ[0])
            } else {
                for s_idx in succ {
                    debug_assert!(blocks[*s_idx].phis.borrow().is_empty());
                }
                None
            };

            let bb = &mut cfg[b_idx];

            // First we have phi destinations
            let b_phis = blocks[b_idx].phis.borrow();
            if !b_phis.is_empty() {
                let phi_dst = get_or_insert_phi_dsts(bb);
                for phi in b_phis.iter() {
                    phi_dst.dsts.push(phi.idx, phi.dst.into());
                }
            }

            // Fix up any remapped SSA values in sources
            if !ssa_map.is_empty() {
                for instr in &mut bb.instrs {
                    instr.for_each_ssa_use_mut(|ssa| loop {
                        if let Some(new_ssa) = ssa_map.get(ssa) {
                            *ssa = *new_ssa;
                        } else {
                            break;
                        }
                    });
                }
            }

            if let Some(s_idx) = s_idx {
                let s_phis = blocks[s_idx].phis.borrow();
                if !s_phis.is_empty() {
                    let phi_src = get_or_insert_phi_srcs(bb);
                    for phi in s_phis.iter() {
                        let ssa = *phi.srcs.get(&b_idx).unwrap();
                        phi_src.srcs.push(phi.idx, ssa.into());
                    }
                }
            }
        }
    }
}
