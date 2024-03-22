// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::ir::*;

use std::collections::HashMap;

struct CopyNode {
    num_reads: usize,
    src: isize,
}

struct CopyGraph {
    nodes: Vec<CopyNode>,
}

impl CopyGraph {
    pub fn new() -> CopyGraph {
        CopyGraph { nodes: Vec::new() }
    }

    fn add_node(&mut self) -> usize {
        let node_idx = self.nodes.len();
        self.nodes.push(CopyNode {
            num_reads: 0,
            src: -1,
        });
        node_idx
    }

    fn num_reads(&self, node_idx: usize) -> usize {
        self.nodes[node_idx].num_reads
    }

    fn src(&self, node_idx: usize) -> Option<usize> {
        if self.nodes[node_idx].src < 0 {
            None
        } else {
            Some(self.nodes[node_idx].src.try_into().unwrap())
        }
    }

    fn add_edge(&mut self, dst_idx: usize, src_idx: usize) {
        // Disallow self-loops
        assert!(dst_idx != src_idx);

        // Each node has in-degree at most 1
        assert!(self.nodes[dst_idx].src == -1);
        self.nodes[dst_idx].src = src_idx.try_into().unwrap();
        self.nodes[src_idx].num_reads += 1;
    }

    fn del_edge(&mut self, dst_idx: usize, src_idx: usize) -> bool {
        assert!(self.nodes[dst_idx].src >= 0);
        self.nodes[dst_idx].src = -1;
        self.nodes[src_idx].num_reads -= 1;
        self.nodes[src_idx].num_reads == 0
    }
}

fn copy_needs_tmp(dst: &RegRef, src: &SrcRef) -> bool {
    if let Some(src_reg) = src.as_reg() {
        (dst.file() == RegFile::Mem && src_reg.file() == RegFile::Mem)
            || (dst.file() == RegFile::Bar && src_reg.file() == RegFile::Bar)
    } else {
        false
    }
}

fn cycle_use_swap(pc: &OpParCopy, file: RegFile) -> bool {
    match file {
        RegFile::GPR => {
            if let Some(tmp) = &pc.tmp {
                debug_assert!(tmp.file() == RegFile::GPR);
                false
            } else {
                true
            }
        }
        RegFile::Bar | RegFile::Mem => {
            let tmp = &pc.tmp.expect("This copy needs a temporary");
            assert!(tmp.comps() >= 2, "Memory cycles need 2 temporaries");
            false
        }
        _ => true,
    }
}

fn lower_par_copy(pc: OpParCopy, sm: u8) -> MappedInstrs {
    let mut graph = CopyGraph::new();
    let mut vals = Vec::new();
    let mut reg_to_idx = HashMap::new();

    for (i, (dst, _)) in pc.dsts_srcs.iter().enumerate() {
        // Destinations must be pairwise unique
        let reg = dst.as_reg().unwrap();
        assert!(reg_to_idx.get(reg).is_none());

        // Everything must be scalar
        assert!(reg.comps() == 1);

        let node_idx = graph.add_node();
        assert!(node_idx == i && vals.len() == i);
        vals.push(SrcRef::from(*reg));
        reg_to_idx.insert(*reg, i);
    }

    for (dst_idx, (_, src)) in pc.dsts_srcs.iter().enumerate() {
        assert!(src.src_mod.is_none());
        let src = src.src_ref;

        let src_idx = if let SrcRef::Reg(reg) = src {
            // Everything must be scalar
            assert!(reg.comps() == 1);

            *reg_to_idx.entry(reg).or_insert_with(|| {
                let node_idx = graph.add_node();
                assert!(node_idx == vals.len());
                vals.push(src);
                node_idx
            })
        } else {
            // We can't have bindless CBufs because we can't resolve cycles
            // containing one.
            assert!(src.get_reg().is_none());

            let node_idx = graph.add_node();
            assert!(node_idx == vals.len());
            vals.push(src);
            node_idx
        };

        if dst_idx != src_idx {
            graph.add_edge(dst_idx, src_idx);
        }
    }

    let mut b = InstrBuilder::new(sm);

    let mut ready = Vec::new();
    for i in 0..pc.dsts_srcs.len() {
        if graph.num_reads(i) == 0 {
            ready.push(i);
        }
    }
    while !ready.is_empty() {
        let dst_idx = ready.pop().unwrap();
        if let Some(src_idx) = graph.src(dst_idx) {
            let dst = *vals[dst_idx].as_reg().unwrap();
            let src = vals[src_idx];
            if copy_needs_tmp(&dst, &src) {
                let tmp = pc.tmp.expect("This copy needs a temporary").comp(0);
                b.copy_to(tmp.into(), src.into());
                b.copy_to(dst.into(), tmp.into());
            } else {
                b.copy_to(dst.into(), src.into());
            }
            if graph.del_edge(dst_idx, src_idx) {
                ready.push(src_idx);
            }
        }
    }

    // At this point, all we are left with in the graph are isolated nodes
    // (no edges) and cycles.
    //
    // Proof:
    //
    // Without loss of generality, we can assume that there are no isolated
    // nodes in the graph.  By construction, no node has in-degree more than 1
    // (that would indicate a duplicate destination).  The loop above ensures
    // that there are no nodes with an in-degree of 1 and an out-degree of 0.
    //
    // Suppose that there were a node with an in-degree of 0.  Then, because no
    // node has an in-degree greater than 1, the number of edges must be less
    // than the number of nodes.  This implies that there is some node N with
    // with out-degree of 0.  If N has an in-degree of 0 then it is isolated,
    // which is a contradiction.  If N has an in-degree of 1 then it is a node
    // with in-degree of 1 and out-degree of 0 which is also a contradiction.
    // Therefore, there are no nodes with in-degree of 0 and all nodes have an
    // in-degree of 1.
    //
    // Since all nodes have an in-degree of 1, no node has an out-degree of 0
    // and, because the sum of all in-degrees equals the sum of all out-degrees
    // (they both equal the number of edges), every node must also have an
    // out-degree of 1.  Therefore, the graph only contains cycles.
    //
    // QED
    for i in 0..pc.dsts_srcs.len() {
        if graph.src(i).is_none() {
            debug_assert!(graph.num_reads(i) == 0);
            continue;
        }

        let file = vals[i].as_reg().unwrap().file();
        if cycle_use_swap(&pc, file) {
            loop {
                let j = graph.src(i).unwrap();
                // We're part of a cycle so j also has a source
                let k = graph.src(j).unwrap();
                let j_reg = vals[j].as_reg().unwrap();
                let k_reg = vals[k].as_reg().unwrap();
                b.swap(*j_reg, *k_reg);
                graph.del_edge(i, j);
                graph.del_edge(j, k);
                if i == k {
                    // This was our last swap
                    break;
                } else {
                    graph.add_edge(i, k);
                }
            }
        } else {
            let pc_tmp = pc.tmp.expect("This copy needs a temporary");
            let tmp = pc_tmp.comp(0);

            let mut p = i;
            let mut p_reg = *vals[p].as_reg().unwrap();
            b.copy_to(tmp.into(), p_reg.into());

            loop {
                let j = graph.src(p).unwrap();
                if j == i {
                    break;
                }

                let j_reg = *vals[j].as_reg().unwrap();
                debug_assert!(j_reg.file() == file);

                if file == RegFile::Bar || file == RegFile::Mem {
                    let copy_tmp = pc_tmp.comp(1);
                    b.copy_to(copy_tmp.into(), j_reg.into());
                    b.copy_to(p_reg.into(), copy_tmp.into());
                } else {
                    b.copy_to(p_reg.into(), j_reg.into());
                }
                graph.del_edge(p, j);

                p = j;
                p_reg = j_reg;
            }

            b.copy_to(p_reg.into(), tmp.into());
            graph.del_edge(p, i);
        }
    }

    b.as_mapped_instrs()
}

impl Shader {
    pub fn lower_par_copies(&mut self) {
        let sm = self.info.sm;
        self.map_instrs(|instr, _| -> MappedInstrs {
            match instr.op {
                Op::ParCopy(pc) => {
                    assert!(instr.pred.is_true());
                    lower_par_copy(pc, sm)
                }
                _ => MappedInstrs::One(instr),
            }
        });
    }
}
