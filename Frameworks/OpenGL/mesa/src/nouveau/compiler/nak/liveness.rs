// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::bitset::BitSet;
use crate::ir::*;

use std::cell::RefCell;
use std::cmp::{max, Ord, Ordering};
use std::collections::{hash_set, HashMap, HashSet};

#[derive(Clone)]
pub struct LiveSet {
    live: PerRegFile<u32>,
    set: HashSet<SSAValue>,
}

impl LiveSet {
    pub fn new() -> LiveSet {
        LiveSet {
            live: Default::default(),
            set: HashSet::new(),
        }
    }

    pub fn contains(&self, ssa: &SSAValue) -> bool {
        self.set.contains(ssa)
    }

    pub fn count(&self, file: RegFile) -> u32 {
        self.live[file]
    }

    pub fn insert(&mut self, ssa: SSAValue) -> bool {
        if self.set.insert(ssa) {
            self.live[ssa.file()] += 1;
            true
        } else {
            false
        }
    }

    pub fn iter(&self) -> hash_set::Iter<SSAValue> {
        self.set.iter()
    }

    pub fn remove(&mut self, ssa: &SSAValue) -> bool {
        if self.set.remove(ssa) {
            self.live[ssa.file()] -= 1;
            true
        } else {
            false
        }
    }

    pub fn insert_instr_top_down<L: BlockLiveness>(
        &mut self,
        ip: usize,
        instr: &Instr,
        bl: &L,
    ) -> PerRegFile<u32> {
        // Vector destinations go live before sources are killed.  Even
        // in the case where the destination is immediately killed, it
        // still may contribute to pressure temporarily.
        for dst in instr.dsts() {
            if let Dst::SSA(vec) = dst {
                if vec.comps() > 1 {
                    for ssa in vec.iter() {
                        self.insert(*ssa);
                    }
                }
            }
        }

        let after_dsts_live = self.live;

        instr.for_each_ssa_use(|ssa| {
            if !bl.is_live_after_ip(ssa, ip) {
                self.remove(ssa);
            }
        });

        // Scalar destinations are allocated last
        for dst in instr.dsts() {
            if let Dst::SSA(vec) = dst {
                if vec.comps() == 1 {
                    self.insert(vec[0]);
                }
            }
        }

        let max_live = PerRegFile::new_with(|file| {
            max(self.live[file], after_dsts_live[file])
        });

        // It's possible (but unlikely) that a destination is immediately
        // killed. Remove any which are killed by this instruction.
        instr.for_each_ssa_def(|ssa| {
            debug_assert!(self.contains(ssa));
            if !bl.is_live_after_ip(ssa, ip) {
                self.remove(ssa);
            }
        });

        max_live
    }
}

impl FromIterator<SSAValue> for LiveSet {
    fn from_iter<T: IntoIterator<Item = SSAValue>>(iter: T) -> Self {
        let mut set = LiveSet::new();
        for ssa in iter {
            set.insert(ssa);
        }
        set
    }
}

pub trait BlockLiveness {
    /// Returns true if @val is still live after @ip
    fn is_live_after_ip(&self, val: &SSAValue, ip: usize) -> bool;

    /// Returns true if @val is live-in to this block
    fn is_live_in(&self, val: &SSAValue) -> bool;

    /// Returns true if @val is live-out of this block
    fn is_live_out(&self, val: &SSAValue) -> bool;

    fn get_instr_pressure(&self, ip: usize, instr: &Instr) -> PerRegFile<u8> {
        let mut live = PerRegFile::new_with(|_| 0_i8);

        // Vector destinations go live before sources are killed.
        for dst in instr.dsts() {
            if let Dst::SSA(vec) = dst {
                if vec.comps() > 1 {
                    for ssa in vec.iter() {
                        live[ssa.file()] += 1;
                    }
                }
            }
        }

        // This is the first high point
        let vec_dst_live = live.clone();

        // Use a hash set because sources may occur more than once
        let mut killed = HashSet::new();
        instr.for_each_ssa_use(|ssa| {
            if !self.is_live_after_ip(ssa, ip) {
                killed.insert(*ssa);
            }
        });
        for ssa in killed.drain() {
            live[ssa.file()] -= 1;
        }

        // Scalar destinations are allocated last
        for dst in instr.dsts() {
            if let Dst::SSA(vec) = dst {
                if vec.comps() == 1 {
                    live[vec[0].file()] += 1;
                }
            }
        }

        PerRegFile::new_with(|file| {
            max(0, max(vec_dst_live[file], live[file]))
                .try_into()
                .unwrap()
        })
    }
}

pub trait Liveness {
    type PerBlock: BlockLiveness;

    fn block_live(&self, idx: usize) -> &Self::PerBlock;

    fn calc_max_live(&self, f: &Function) -> PerRegFile<u32> {
        let mut max_live: PerRegFile<u32> = Default::default();
        let mut block_live_out: Vec<LiveSet> = Vec::new();

        for (bb_idx, bb) in f.blocks.iter().enumerate() {
            let bl = self.block_live(bb_idx);

            let mut live = LiveSet::new();

            // Predecessors are added block order so we can just grab the first
            // one (if any) and it will be a block we've processed.
            if let Some(pred_idx) = f.blocks.pred_indices(bb_idx).first() {
                let pred_out = &block_live_out[*pred_idx];
                for ssa in pred_out.iter() {
                    if bl.is_live_in(ssa) {
                        live.insert(*ssa);
                    }
                }
            }

            for (ip, instr) in bb.instrs.iter().enumerate() {
                let live_at_instr = live.insert_instr_top_down(ip, instr, bl);
                max_live = PerRegFile::new_with(|file| {
                    max(max_live[file], live_at_instr[file])
                });

                if let Op::FSOut(fs_out) = &instr.op {
                    // This should be the last instruction.  Everything should
                    // be dead once we've processed it.
                    debug_assert!(live.count(RegFile::GPR) == 0);
                    let num_gprs_out = fs_out.srcs.len().try_into().unwrap();
                    max_live[RegFile::GPR] =
                        max(max_live[RegFile::GPR], num_gprs_out);
                }
            }

            assert!(block_live_out.len() == bb_idx);
            block_live_out.push(live);
        }

        max_live
    }
}

pub struct SimpleBlockLiveness {
    defs: BitSet,
    uses: BitSet,
    last_use: HashMap<u32, usize>,
    live_in: BitSet,
    live_out: BitSet,
}

impl SimpleBlockLiveness {
    fn new() -> Self {
        Self {
            defs: BitSet::new(),
            uses: BitSet::new(),
            last_use: HashMap::new(),
            live_in: BitSet::new(),
            live_out: BitSet::new(),
        }
    }

    fn add_def(&mut self, ssa: SSAValue) {
        self.defs.insert(ssa.idx().try_into().unwrap());
    }

    fn add_use(&mut self, ssa: SSAValue, ip: usize) {
        self.uses.insert(ssa.idx().try_into().unwrap());
        self.last_use.insert(ssa.idx(), ip);
    }
}

impl BlockLiveness for SimpleBlockLiveness {
    fn is_live_after_ip(&self, val: &SSAValue, ip: usize) -> bool {
        if self.live_out.get(val.idx().try_into().unwrap()) {
            true
        } else {
            if let Some(last_use_ip) = self.last_use.get(&val.idx()) {
                *last_use_ip > ip
            } else {
                false
            }
        }
    }

    fn is_live_in(&self, val: &SSAValue) -> bool {
        self.live_in.get(val.idx().try_into().unwrap())
    }

    fn is_live_out(&self, val: &SSAValue) -> bool {
        self.live_out.get(val.idx().try_into().unwrap())
    }
}

pub struct SimpleLiveness {
    ssa_block_ip: HashMap<SSAValue, (usize, usize)>,
    blocks: Vec<SimpleBlockLiveness>,
}

impl SimpleLiveness {
    pub fn for_function(func: &Function) -> SimpleLiveness {
        let mut l = SimpleLiveness {
            ssa_block_ip: HashMap::new(),
            blocks: Vec::new(),
        };
        let mut live_in = Vec::new();

        for (bi, b) in func.blocks.iter().enumerate() {
            let mut bl = SimpleBlockLiveness::new();

            for (ip, instr) in b.instrs.iter().enumerate() {
                instr.for_each_ssa_use(|ssa| {
                    bl.add_use(*ssa, ip);
                });
                instr.for_each_ssa_def(|ssa| {
                    l.ssa_block_ip.insert(*ssa, (bi, ip));
                    bl.add_def(*ssa);
                });
            }

            l.blocks.push(bl);
            live_in.push(BitSet::new());
        }
        assert!(l.blocks.len() == func.blocks.len());
        assert!(live_in.len() == func.blocks.len());

        let num_ssa = usize::try_from(func.ssa_alloc.max_idx() + 1).unwrap();
        let mut tmp = BitSet::new();
        tmp.reserve(num_ssa);

        let mut to_do = true;
        while to_do {
            to_do = false;
            for (b_idx, bl) in l.blocks.iter_mut().enumerate().rev() {
                // Compute live-out
                for sb_idx in func.blocks.succ_indices(b_idx) {
                    to_do |= bl.live_out.union_with(&live_in[*sb_idx]);
                }

                tmp.clear();
                tmp.set_words(0..num_ssa, |w| {
                    (bl.live_out.get_word(w) | bl.uses.get_word(w))
                        & !bl.defs.get_word(w)
                });

                to_do |= live_in[b_idx].union_with(&tmp);
            }
        }

        for (bl, b_live_in) in l.blocks.iter_mut().zip(live_in.into_iter()) {
            bl.live_in = b_live_in;
        }

        l
    }
}

impl SimpleLiveness {
    pub fn def_block_ip(&self, ssa: &SSAValue) -> (usize, usize) {
        *self.ssa_block_ip.get(ssa).unwrap()
    }

    pub fn interferes(&self, a: &SSAValue, b: &SSAValue) -> bool {
        let (ab, ai) = self.def_block_ip(a);
        let (bb, bi) = self.def_block_ip(b);

        match ab.cmp(&bb).then(ai.cmp(&bi)) {
            Ordering::Equal => true,
            Ordering::Less => self.block_live(bb).is_live_after_ip(a, bi),
            Ordering::Greater => self.block_live(ab).is_live_after_ip(b, ai),
        }
    }
}

impl Liveness for SimpleLiveness {
    type PerBlock = SimpleBlockLiveness;

    fn block_live(&self, idx: usize) -> &SimpleBlockLiveness {
        &self.blocks[idx]
    }
}

struct SSAUseDef {
    defined: bool,
    uses: Vec<usize>,
}

impl SSAUseDef {
    fn add_def(&mut self) {
        self.defined = true;
    }

    fn add_in_block_use(&mut self, use_ip: usize) {
        self.uses.push(use_ip);
    }

    fn add_successor_use(
        &mut self,
        num_block_instrs: usize,
        use_ip: usize,
    ) -> bool {
        // IPs are relative to the start of their block
        let use_ip = num_block_instrs + use_ip;

        if let Some(last_use_ip) = self.uses.last_mut() {
            if *last_use_ip < num_block_instrs {
                // We've never seen a successor use before
                self.uses.push(use_ip);
                true
            } else if *last_use_ip > use_ip {
                // Otherwise, we want the minimum next use
                *last_use_ip = use_ip;
                true
            } else {
                false
            }
        } else {
            self.uses.push(use_ip);
            true
        }
    }
}

pub struct NextUseBlockLiveness {
    num_instrs: usize,
    ssa_map: HashMap<SSAValue, SSAUseDef>,
}

impl NextUseBlockLiveness {
    fn new(num_instrs: usize) -> Self {
        Self {
            num_instrs: num_instrs,
            ssa_map: HashMap::new(),
        }
    }

    fn entry_mut(&mut self, ssa: SSAValue) -> &mut SSAUseDef {
        self.ssa_map.entry(ssa).or_insert_with(|| SSAUseDef {
            defined: false,
            uses: Vec::new(),
        })
    }

    fn add_def(&mut self, ssa: SSAValue) {
        self.entry_mut(ssa).add_def();
    }

    fn add_use(&mut self, ssa: SSAValue, ip: usize) {
        self.entry_mut(ssa).add_in_block_use(ip);
    }

    /// Returns an iterator over all the values which are live-in to this block
    pub fn iter_live_in<'a>(&'a self) -> impl Iterator<Item = &'a SSAValue> {
        self.ssa_map.iter().filter_map(|(ssa, entry)| {
            if entry.defined || entry.uses.is_empty() {
                None
            } else {
                Some(ssa)
            }
        })
    }

    /// Returns the IP of the first use of @val
    ///
    /// The returned IP is relative to the start of this block.  If the next use
    /// is in some successor block, the returned IP is relative to the start of
    /// this block.  If @val is not used in this block and is not live-out, None
    /// is returned.
    pub fn first_use(&self, val: &SSAValue) -> Option<usize> {
        if let Some(entry) = self.ssa_map.get(val) {
            entry.uses.first().cloned()
        } else {
            None
        }
    }

    /// Returns the IP of the first use of @val which is greater than or equal
    /// to @ip
    ///
    /// All IPs are relative to the start of the block.  If the next use is some
    /// successor block, the returned IP is relative to the start of this block.
    pub fn next_use_after_or_at_ip(
        &self,
        val: &SSAValue,
        ip: usize,
    ) -> Option<usize> {
        if let Some(entry) = self.ssa_map.get(val) {
            let i = entry.uses.partition_point(|u| *u < ip);
            if i < entry.uses.len() {
                Some(entry.uses[i])
            } else {
                None
            }
        } else {
            None
        }
    }
}

impl BlockLiveness for NextUseBlockLiveness {
    fn is_live_after_ip(&self, val: &SSAValue, ip: usize) -> bool {
        if let Some(entry) = self.ssa_map.get(val) {
            if let Some(last_use_ip) = entry.uses.last() {
                *last_use_ip > ip
            } else {
                false
            }
        } else {
            false
        }
    }

    fn is_live_in(&self, val: &SSAValue) -> bool {
        if let Some(entry) = self.ssa_map.get(val) {
            !entry.defined && !entry.uses.is_empty()
        } else {
            false
        }
    }

    fn is_live_out(&self, val: &SSAValue) -> bool {
        if let Some(entry) = self.ssa_map.get(val) {
            if let Some(last_use_ip) = entry.uses.last() {
                *last_use_ip >= self.num_instrs
            } else {
                false
            }
        } else {
            false
        }
    }
}

/// An implementation of Liveness that tracks next-use IPs for each SSAValue
///
/// Along with the usual liveness information, this tracks next-use IPs for each
/// SSAValue.  Cross-block next-use IPs computed are as per the global next-use
/// distance algorithm described in "Register Spilling and Live-Range Splitting
/// for SSA-Form Programs" by Braun and Hack.
pub struct NextUseLiveness {
    blocks: Vec<NextUseBlockLiveness>,
}

impl NextUseLiveness {
    pub fn for_function(
        func: &Function,
        files: &RegFileSet,
    ) -> NextUseLiveness {
        let mut blocks = Vec::new();
        for (bi, b) in func.blocks.iter().enumerate() {
            let mut bl = NextUseBlockLiveness::new(b.instrs.len());

            for (ip, instr) in b.instrs.iter().enumerate() {
                instr.for_each_ssa_use(|ssa| {
                    if files.contains(ssa.file()) {
                        bl.add_use(*ssa, ip);
                    }
                });

                instr.for_each_ssa_def(|ssa| {
                    if files.contains(ssa.file()) {
                        bl.add_def(*ssa);
                    }
                });
            }

            debug_assert!(bi == blocks.len());
            blocks.push(RefCell::new(bl));
        }

        let mut to_do = true;
        while to_do {
            to_do = false;
            for (b_idx, b) in func.blocks.iter().enumerate().rev() {
                let num_instrs = b.instrs.len();
                let mut bl = blocks[b_idx].borrow_mut();

                // Compute live-out
                for sb_idx in func.blocks.succ_indices(b_idx) {
                    if *sb_idx == b_idx {
                        for entry in bl.ssa_map.values_mut() {
                            if entry.defined {
                                continue;
                            }

                            let Some(first_use_ip) = entry.uses.first() else {
                                continue;
                            };

                            to_do |= entry
                                .add_successor_use(num_instrs, *first_use_ip);
                        }
                    } else {
                        let sbl = blocks[*sb_idx].borrow();
                        for (ssa, entry) in sbl.ssa_map.iter() {
                            if entry.defined {
                                continue;
                            }

                            let Some(first_use_ip) = entry.uses.first() else {
                                continue;
                            };

                            to_do |= bl
                                .entry_mut(*ssa)
                                .add_successor_use(num_instrs, *first_use_ip);
                        }
                    }
                }
            }
        }

        NextUseLiveness {
            blocks: blocks.into_iter().map(|bl| bl.into_inner()).collect(),
        }
    }
}

impl Liveness for NextUseLiveness {
    type PerBlock = NextUseBlockLiveness;

    fn block_live(&self, idx: usize) -> &NextUseBlockLiveness {
        &self.blocks[idx]
    }
}
