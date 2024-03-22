// Copyright © 2023 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::cfg::CFG;
use crate::ir::*;
use crate::liveness::{BlockLiveness, Liveness, SimpleLiveness};

use std::collections::HashMap;
use std::iter::Peekable;

struct MergedIter<I: Iterator> {
    a: Peekable<I>,
    b: Peekable<I>,
}

impl<I: Iterator> MergedIter<I> {
    fn new(a: I, b: I) -> Self {
        Self {
            a: a.peekable(),
            b: b.peekable(),
        }
    }
}

impl<I: Iterator> Iterator for MergedIter<I>
where
    <I as Iterator>::Item: Ord,
{
    type Item = <I as Iterator>::Item;

    fn next(&mut self) -> Option<<I as Iterator>::Item> {
        if let Some(a) = self.a.peek() {
            if let Some(b) = self.b.peek() {
                if a <= b {
                    self.a.next()
                } else {
                    self.b.next()
                }
            } else {
                self.a.next()
            }
        } else {
            self.b.next()
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let (a_max, a_size) = self.a.size_hint();
        let (b_max, b_size) = self.b.size_hint();
        (a_max + b_max, a_size.zip(b_size).map(|(a, b)| a + b))
    }
}

enum CoalesceItem {
    SSA(SSAValue),
    Phi(u32),
}

struct CoalesceNode {
    set: usize,
    block: usize,
    ip_1: usize,
    item: CoalesceItem,
}

struct CoalesceSet {
    nodes: Vec<usize>,
}

struct CoalesceGraph<'a> {
    live: &'a SimpleLiveness,
    nodes: Vec<CoalesceNode>,
    sets: Vec<CoalesceSet>,
    ssa_node: HashMap<SSAValue, usize>,
    phi_node_file: HashMap<u32, (usize, RegFile)>,
}

impl<'a> CoalesceGraph<'a> {
    fn new(live: &'a SimpleLiveness) -> Self {
        Self {
            live: live,
            nodes: Vec::new(),
            sets: Vec::new(),
            ssa_node: HashMap::new(),
            phi_node_file: HashMap::new(),
        }
    }

    fn add_ssa(&mut self, ssa: SSAValue) {
        debug_assert!(self.sets.is_empty());

        // Set it to usize::MAX for now.  We'll update later
        if self.ssa_node.insert(ssa, usize::MAX).is_none() {
            let (block, ip) = self.live.def_block_ip(&ssa);
            self.nodes.push(CoalesceNode {
                set: usize::MAX,
                block: block,
                ip_1: ip + 1,
                item: CoalesceItem::SSA(ssa),
            });
        }
    }

    fn add_phi_dst(&mut self, phi: u32, file: RegFile, block: usize) {
        debug_assert!(self.sets.is_empty());

        // Record the register file now.  We'll set the node later
        let old = self.phi_node_file.insert(phi, (usize::MAX, file));
        debug_assert!(old.is_none());

        self.nodes.push(CoalesceNode {
            set: usize::MAX,
            block: block,
            ip_1: 0,
            item: CoalesceItem::Phi(phi),
        });
    }

    fn add_phi_src(&mut self, phi: u32, block: usize) {
        debug_assert!(self.sets.is_empty());

        self.nodes.push(CoalesceNode {
            set: usize::MAX,
            block: block,
            ip_1: usize::MAX,
            item: CoalesceItem::Phi(phi),
        });
    }

    fn init_sets<N>(&mut self, cfg: &CFG<N>) {
        // Sort the nodes by dom_dfs_pre_index followed by ip+1.  Stash the
        // dom_dfs_pre_index in the set for now.  We don't actually fill out
        // the set field until later.
        for n in self.nodes.iter_mut() {
            n.set = cfg.dom_dfs_pre_index(n.block);
        }
        self.nodes
            .sort_by(|a, b| a.set.cmp(&b.set).then(a.ip_1.cmp(&b.ip_1)));

        for ni in 0..self.nodes.len() {
            match &self.nodes[ni].item {
                CoalesceItem::SSA(ssa) => {
                    let old = self.ssa_node.insert(*ssa, ni);
                    debug_assert!(old == Some(usize::MAX));

                    self.nodes[ni].set = self.sets.len();
                    self.sets.push(CoalesceSet { nodes: vec![ni] });
                }
                CoalesceItem::Phi(phi) => {
                    let (pn, _) = self.phi_node_file.get_mut(phi).unwrap();

                    // We only want one set per phi and phi_node contains the
                    // index to any one of the nodes.
                    if *pn == usize::MAX {
                        self.nodes[ni].set = self.sets.len();
                        self.sets.push(CoalesceSet { nodes: vec![ni] });
                        *pn = ni;
                    } else {
                        let s = self.nodes[*pn].set;
                        self.nodes[ni].set = s;
                    }
                }
            }
        }
    }

    fn node_dominates<N>(&self, p: usize, c: usize, cfg: &CFG<N>) -> bool {
        if self.nodes[p].block == self.nodes[c].block {
            self.nodes[p].ip_1 <= self.nodes[c].ip_1
        } else {
            cfg.dominates(self.nodes[p].block, self.nodes[c].block)
        }
    }

    fn phi_ssa_interferes(&self, phi: &CoalesceNode, ssa: &SSAValue) -> bool {
        if phi.ip_1 == 0 {
            self.live.block_live(phi.block).is_live_in(ssa)
        } else {
            debug_assert!(phi.ip_1 == usize::MAX);
            self.live.block_live(phi.block).is_live_out(ssa)
        }
    }

    fn nodes_interfere(&self, a: usize, b: usize) -> bool {
        let a = &self.nodes[a];
        let b = &self.nodes[b];

        match &a.item {
            CoalesceItem::SSA(a_ssa) => match &b.item {
                CoalesceItem::SSA(b_ssa) => self.live.interferes(a_ssa, b_ssa),
                CoalesceItem::Phi(_) => self.phi_ssa_interferes(b, a_ssa),
            },
            CoalesceItem::Phi(_) => match &b.item {
                CoalesceItem::SSA(b_ssa) => self.phi_ssa_interferes(a, b_ssa),
                CoalesceItem::Phi(_) => {
                    // Phi nodes represent the temporary SSA value made between
                    // the parallel copy and the phi in the Boissinot algorithm
                    // so they interfere if and only if they're in the same
                    // block and both at the start or both at the end.
                    a.block == b.block && a.ip_1 == b.ip_1
                }
            },
        }
    }

    pub fn sets_interfere<N>(&self, a: usize, b: usize, cfg: &CFG<N>) -> bool {
        let a = &self.sets[a];
        let b = &self.sets[b];

        // Stack of nodes which dominate the current node
        let mut dom = Vec::new();

        for n in MergedIter::new(a.nodes.iter(), b.nodes.iter()) {
            loop {
                if let Some(p) = dom.last() {
                    if !self.node_dominates(*p, *n, cfg) {
                        dom.pop();
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }

            if let Some(p) = dom.last() {
                if self.nodes_interfere(*n, *p) {
                    return true;
                }
            }

            dom.push(*n);
        }

        false
    }

    pub fn sets_merge(&mut self, a: usize, b: usize) -> usize {
        let a_nodes = std::mem::replace(&mut self.sets[a].nodes, Vec::new());
        let b_nodes = std::mem::replace(&mut self.sets[b].nodes, Vec::new());
        let nodes = MergedIter::new(a_nodes.into_iter(), b_nodes.into_iter());

        self.sets[a].nodes = nodes
            .map(|n| {
                self.nodes[n].set = a;
                n
            })
            .collect();

        a
    }

    pub fn ssa_set(&self, ssa: &SSAValue) -> usize {
        self.nodes[*self.ssa_node.get(ssa).unwrap()].set
    }

    pub fn phi_set_file(&self, phi: &u32) -> (usize, RegFile) {
        let (n, file) = self.phi_node_file.get(phi).unwrap();
        (self.nodes[*n].set, *file)
    }
}

impl Function {
    /// Convert a function to CSSA (Conventional SSA) form
    ///
    /// In "Translating Out of Static Single Assignment Form" by Sreedhar, et.
    /// al., they define CSSA form via what they call the Phi Congruence
    /// Property:
    ///
    /// > The occurrences of all resources which belong to the same phi
    /// > congruence class in a program can be replaced by a representative
    /// > resource. After the replacement, the phi instruction can be
    /// > eliminated without violating the semantics of the original program.
    ///
    /// A more compiler-theoretic definition of CSSA form is a version of SSA
    /// form in which, for each phi, none of the SSA values involved in the phi
    /// (either as a source or destination) interfere.  While most of the papers
    /// discussing CSSA form do so in the context of out-of-SSA, this property
    /// is also useful for SSA-based spilling and register allocation.
    ///
    /// Our implementation is based on the algorithm described in "Revisiting
    /// Out-of-SSA Translation for Correctness, Code Quality, and Effciency" by
    /// Boissinot et. al.  The primary difference between this algorithm and
    /// the one in that paper is that we don't actually insert parallel copies
    /// and remove redundant entries.  Instead, we treat OpPhiSrcs and OpPhiDsts
    /// as as the parallel copies with the phi index standing in for all of the
    /// SSA values used directly by the phi.  Then, instead of removing copies
    /// where the source and destination don't interfere, we insert copies
    /// whenever the source or destination and phi index do interfere.  This
    /// lets us avoid inserting pointless instructions.
    pub fn to_cssa(&mut self) {
        let live = SimpleLiveness::for_function(self);

        let mut cg = CoalesceGraph::new(&live);
        for (bi, b) in self.blocks.iter().enumerate() {
            if let Some(phi) = b.phi_dsts() {
                for (idx, dst) in phi.dsts.iter() {
                    let vec = dst.as_ssa().unwrap();
                    debug_assert!(vec.comps() == 1);
                    cg.add_ssa(vec[0]);
                    cg.add_phi_dst(*idx, vec[0].file(), bi);
                }
            }

            if let Some(phi) = b.phi_srcs() {
                for (idx, src) in phi.srcs.iter() {
                    if let SrcRef::SSA(vec) = src.src_ref {
                        debug_assert!(vec.comps() == 1);
                        cg.add_ssa(vec[0]);
                    }
                    cg.add_phi_src(*idx, bi);
                }
            }
        }
        cg.init_sets(&self.blocks);

        for bi in 0..self.blocks.len() {
            let block_instrs =
                std::mem::replace(&mut self.blocks[bi].instrs, Vec::new());

            let mut instrs = Vec::new();
            for mut instr in block_instrs.into_iter() {
                match &mut instr.op {
                    Op::PhiDsts(phi) => {
                        let mut pcopy = OpParCopy::new();
                        for (idx, dst) in phi.dsts.iter_mut() {
                            let (ps, file) = cg.phi_set_file(idx);

                            let vec = dst.as_ssa().unwrap();
                            debug_assert!(vec.comps() == 1);
                            debug_assert!(vec[0].file() == file);
                            let ds = cg.ssa_set(&vec[0]);

                            if !cg.sets_interfere(ps, ds, &self.blocks) {
                                cg.sets_merge(ps, ds);
                                continue;
                            }

                            let tmp = self.ssa_alloc.alloc(file);
                            pcopy.push(*dst, tmp.into());
                            *dst = tmp.into();
                        }

                        instrs.push(instr);
                        if !pcopy.is_empty() {
                            instrs.push(Instr::new_boxed(pcopy));
                        }
                    }
                    Op::PhiSrcs(phi) => {
                        let mut pcopy = OpParCopy::new();
                        for (idx, src) in phi.srcs.iter_mut() {
                            let (ps, file) = cg.phi_set_file(idx);

                            debug_assert!(src.src_mod.is_none());
                            if let SrcRef::SSA(vec) = &src.src_ref {
                                debug_assert!(vec.comps() == 1);
                                let ss = cg.ssa_set(&vec[0]);
                                if cg.sets_interfere(ps, ss, &self.blocks) {
                                    let tmp = self.ssa_alloc.alloc(file);
                                    pcopy.push(tmp.into(), *src);
                                    *src = tmp.into();
                                } else {
                                    cg.sets_merge(ps, ss);
                                }
                            } else {
                                // Non-SSA sources get an actual Mov instruction
                                // and are not considered part of the parallel
                                // copy.
                                let tmp = self.ssa_alloc.alloc(file);
                                instrs.push(Instr::new_boxed(OpCopy {
                                    dst: tmp.into(),
                                    src: *src,
                                }));
                                *src = tmp.into();
                            }
                        }

                        if !pcopy.is_empty() {
                            instrs.push(Instr::new_boxed(pcopy));
                        }
                        instrs.push(instr);
                    }
                    _ => instrs.push(instr),
                }
            }
            self.blocks[bi].instrs = instrs;
        }
    }
}
