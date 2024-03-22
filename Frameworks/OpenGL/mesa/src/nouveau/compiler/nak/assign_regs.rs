// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::bitset::BitSet;
use crate::ir::*;
use crate::liveness::{BlockLiveness, Liveness, SimpleLiveness};

use std::cmp::{max, Ordering};
use std::collections::{HashMap, HashSet};

struct KillSet {
    set: HashSet<SSAValue>,
    vec: Vec<SSAValue>,
}

impl KillSet {
    pub fn new() -> KillSet {
        KillSet {
            set: HashSet::new(),
            vec: Vec::new(),
        }
    }

    pub fn clear(&mut self) {
        self.set.clear();
        self.vec.clear();
    }

    pub fn insert(&mut self, ssa: SSAValue) {
        if self.set.insert(ssa) {
            self.vec.push(ssa);
        }
    }

    pub fn iter(&self) -> std::slice::Iter<'_, SSAValue> {
        self.vec.iter()
    }

    pub fn is_empty(&self) -> bool {
        self.vec.is_empty()
    }
}

enum SSAUse {
    FixedReg(u32),
    Vec(SSARef),
}

struct SSAUseMap {
    ssa_map: HashMap<SSAValue, Vec<(usize, SSAUse)>>,
}

impl SSAUseMap {
    fn add_fixed_reg_use(&mut self, ip: usize, ssa: SSAValue, reg: u32) {
        let v = self.ssa_map.entry(ssa).or_insert_with(|| Vec::new());
        v.push((ip, SSAUse::FixedReg(reg)));
    }

    fn add_vec_use(&mut self, ip: usize, vec: SSARef) {
        if vec.comps() == 1 {
            return;
        }

        for ssa in vec.iter() {
            let v = self.ssa_map.entry(*ssa).or_insert_with(|| Vec::new());
            v.push((ip, SSAUse::Vec(vec)));
        }
    }

    fn find_vec_use_after(&self, ssa: SSAValue, ip: usize) -> Option<&SSAUse> {
        if let Some(v) = self.ssa_map.get(&ssa) {
            let p = v.partition_point(|(uip, _)| *uip <= ip);
            if p == v.len() {
                None
            } else {
                let (_, u) = &v[p];
                Some(u)
            }
        } else {
            None
        }
    }

    pub fn add_block(&mut self, b: &BasicBlock) {
        for (ip, instr) in b.instrs.iter().enumerate() {
            match &instr.op {
                Op::FSOut(op) => {
                    for (i, src) in op.srcs.iter().enumerate() {
                        let out_reg = u32::try_from(i).unwrap();
                        if let SrcRef::SSA(ssa) = src.src_ref {
                            assert!(ssa.comps() == 1);
                            self.add_fixed_reg_use(ip, ssa[0], out_reg);
                        }
                    }
                }
                _ => {
                    // We don't care about predicates because they're scalar
                    for src in instr.srcs() {
                        if let SrcRef::SSA(ssa) = src.src_ref {
                            self.add_vec_use(ip, ssa);
                        }
                    }
                }
            }
        }
    }

    pub fn for_block(b: &BasicBlock) -> SSAUseMap {
        let mut am = SSAUseMap {
            ssa_map: HashMap::new(),
        };
        am.add_block(b);
        am
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
enum LiveRef {
    SSA(SSAValue),
    Phi(u32),
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
struct LiveValue {
    pub live_ref: LiveRef,
    pub reg_ref: RegRef,
}

// We need a stable ordering of live values so that RA is deterministic
impl Ord for LiveValue {
    fn cmp(&self, other: &Self) -> Ordering {
        let s_file = u8::from(self.reg_ref.file());
        let o_file = u8::from(other.reg_ref.file());
        match s_file.cmp(&o_file) {
            Ordering::Equal => {
                let s_idx = self.reg_ref.base_idx();
                let o_idx = other.reg_ref.base_idx();
                s_idx.cmp(&o_idx)
            }
            ord => ord,
        }
    }
}

impl PartialOrd for LiveValue {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

#[derive(Clone)]
struct RegAllocator {
    file: RegFile,
    num_regs: u32,
    used: BitSet,
    reg_ssa: Vec<SSAValue>,
    ssa_reg: HashMap<SSAValue, u32>,
}

impl RegAllocator {
    pub fn new(file: RegFile, num_regs: u32) -> Self {
        Self {
            file: file,
            num_regs: num_regs,
            used: BitSet::new(),
            reg_ssa: Vec::new(),
            ssa_reg: HashMap::new(),
        }
    }

    fn file(&self) -> RegFile {
        self.file
    }

    pub fn num_regs_used(&self) -> u32 {
        self.ssa_reg.len().try_into().unwrap()
    }

    pub fn reg_is_used(&self, reg: u32) -> bool {
        self.used.get(reg.try_into().unwrap())
    }

    fn reg_range_is_unused(&self, reg: u32, comps: u8) -> bool {
        for c in 0..u32::from(comps) {
            if self.reg_is_used(reg + c) {
                return false;
            }
        }
        true
    }

    pub fn try_get_reg(&self, ssa: SSAValue) -> Option<u32> {
        self.ssa_reg.get(&ssa).cloned()
    }

    pub fn try_get_ssa(&self, reg: u32) -> Option<SSAValue> {
        if self.reg_is_used(reg) {
            Some(self.reg_ssa[usize::try_from(reg).unwrap()])
        } else {
            None
        }
    }

    pub fn try_get_vec_reg(&self, vec: &SSARef) -> Option<u32> {
        let Some(reg) = self.try_get_reg(vec[0]) else {
            return None;
        };

        let align = u32::from(vec.comps()).next_power_of_two();
        if reg % align != 0 {
            return None;
        }

        for c in 1..vec.comps() {
            let ssa = vec[usize::from(c)];
            if self.try_get_reg(ssa) != Some(reg + u32::from(c)) {
                return None;
            }
        }
        Some(reg)
    }

    pub fn free_ssa(&mut self, ssa: SSAValue) -> u32 {
        assert!(ssa.file() == self.file);
        let reg = self.ssa_reg.remove(&ssa).unwrap();
        assert!(self.reg_is_used(reg));
        let reg_usize = usize::try_from(reg).unwrap();
        assert!(self.reg_ssa[reg_usize] == ssa);
        self.used.remove(reg_usize);
        reg
    }

    pub fn assign_reg(&mut self, ssa: SSAValue, reg: u32) {
        assert!(ssa.file() == self.file);
        assert!(reg < self.num_regs);
        assert!(!self.reg_is_used(reg));

        let reg_usize = usize::try_from(reg).unwrap();
        if reg_usize >= self.reg_ssa.len() {
            self.reg_ssa.resize(reg_usize + 1, SSAValue::NONE);
        }
        self.reg_ssa[reg_usize] = ssa;
        let old = self.ssa_reg.insert(ssa, reg);
        assert!(old.is_none());
        self.used.insert(reg_usize);
    }

    pub fn try_find_unused_reg_range(
        &self,
        start_reg: u32,
        align: u32,
        comps: u8,
    ) -> Option<u32> {
        assert!(comps > 0 && u32::from(comps) <= self.num_regs);

        let mut next_reg = start_reg;
        loop {
            let reg: u32 = self
                .used
                .next_unset(usize::try_from(next_reg).unwrap())
                .try_into()
                .unwrap();

            // Ensure we're properly aligned
            let reg = reg.next_multiple_of(align);

            // Ensure we're in-bounds. This also serves as a check to ensure
            // that u8::try_from(reg + i) will succeed.
            if reg > self.num_regs - u32::from(comps) {
                return None;
            }

            if self.reg_range_is_unused(reg, comps) {
                return Some(reg);
            }

            next_reg = reg + align;
        }
    }

    pub fn alloc_scalar(
        &mut self,
        ip: usize,
        sum: &SSAUseMap,
        ssa: SSAValue,
    ) -> u32 {
        if let Some(u) = sum.find_vec_use_after(ssa, ip) {
            match u {
                SSAUse::FixedReg(reg) => {
                    if !self.reg_is_used(*reg) {
                        self.assign_reg(ssa, *reg);
                        return *reg;
                    }
                }
                SSAUse::Vec(vec) => {
                    let mut comp = u8::MAX;
                    for c in 0..vec.comps() {
                        if vec[usize::from(c)] == ssa {
                            comp = c;
                            break;
                        }
                    }
                    assert!(comp < vec.comps());

                    let align = u32::from(vec.comps()).next_power_of_two();
                    for c in 0..vec.comps() {
                        if c == comp {
                            continue;
                        }

                        let other = vec[usize::from(c)];
                        let Some(other_reg) = self.try_get_reg(other) else {
                            continue;
                        };

                        let vec_reg = other_reg & !(align - 1);
                        if other_reg != vec_reg + u32::from(c) {
                            continue;
                        }

                        let reg = vec_reg + u32::from(comp);
                        if reg < self.num_regs && !self.reg_is_used(reg) {
                            self.assign_reg(ssa, reg);
                            return reg;
                        }
                    }

                    // We weren't able to pair it with an already allocated
                    // register but maybe we can at least find an aligned one.
                    if let Some(reg) =
                        self.try_find_unused_reg_range(0, align, 1)
                    {
                        self.assign_reg(ssa, reg);
                        return reg;
                    }
                }
            }
        }

        let reg = self
            .try_find_unused_reg_range(0, 1, 1)
            .expect("Failed to find free register");
        self.assign_reg(ssa, reg);
        reg
    }
}

struct PinnedRegAllocator<'a> {
    ra: &'a mut RegAllocator,
    pcopy: OpParCopy,
    pinned: BitSet,
    evicted: HashMap<SSAValue, u32>,
}

impl<'a> PinnedRegAllocator<'a> {
    fn new(ra: &'a mut RegAllocator) -> Self {
        PinnedRegAllocator {
            ra: ra,
            pcopy: OpParCopy::new(),
            pinned: Default::default(),
            evicted: HashMap::new(),
        }
    }

    fn file(&self) -> RegFile {
        self.ra.file()
    }

    fn pin_reg(&mut self, reg: u32) {
        self.pinned.insert(reg.try_into().unwrap());
    }

    fn pin_reg_range(&mut self, reg: u32, comps: u8) {
        for c in 0..u32::from(comps) {
            self.pin_reg(reg + c);
        }
    }

    fn reg_is_pinned(&self, reg: u32) -> bool {
        self.pinned.get(reg.try_into().unwrap())
    }

    fn reg_range_is_unpinned(&self, reg: u32, comps: u8) -> bool {
        for c in 0..u32::from(comps) {
            if self.reg_is_pinned(reg + c) {
                return false;
            }
        }
        true
    }

    fn assign_pin_reg(&mut self, ssa: SSAValue, reg: u32) -> RegRef {
        self.pin_reg(reg);
        self.ra.assign_reg(ssa, reg);
        RegRef::new(self.file(), reg, 1)
    }

    pub fn assign_pin_vec_reg(&mut self, vec: SSARef, reg: u32) -> RegRef {
        for c in 0..vec.comps() {
            let ssa = vec[usize::from(c)];
            self.assign_pin_reg(ssa, reg + u32::from(c));
        }
        RegRef::new(self.file(), reg, vec.comps())
    }

    fn try_find_unpinned_reg_range(
        &self,
        start_reg: u32,
        align: u32,
        comps: u8,
    ) -> Option<u32> {
        let align = align;

        let mut next_reg = start_reg;
        loop {
            let reg: u32 = self
                .pinned
                .next_unset(usize::try_from(next_reg).unwrap())
                .try_into()
                .unwrap();

            // Ensure we're properly aligned
            let reg = reg.next_multiple_of(align);

            // Ensure we're in-bounds. This also serves as a check to ensure
            // that u8::try_from(reg + i) will succeed.
            if reg > self.ra.num_regs - u32::from(comps) {
                return None;
            }

            if self.reg_range_is_unpinned(reg, comps) {
                return Some(reg);
            }

            next_reg = reg + align;
        }
    }

    pub fn evict_ssa(&mut self, ssa: SSAValue, old_reg: u32) {
        assert!(ssa.file() == self.file());
        assert!(!self.reg_is_pinned(old_reg));
        self.evicted.insert(ssa, old_reg);
    }

    pub fn evict_reg_if_used(&mut self, reg: u32) {
        assert!(!self.reg_is_pinned(reg));

        if let Some(ssa) = self.ra.try_get_ssa(reg) {
            self.ra.free_ssa(ssa);
            self.evict_ssa(ssa, reg);
        }
    }

    fn move_ssa_to_reg(&mut self, ssa: SSAValue, new_reg: u32) {
        if let Some(old_reg) = self.ra.try_get_reg(ssa) {
            assert!(self.evicted.get(&ssa).is_none());
            assert!(!self.reg_is_pinned(old_reg));

            if new_reg == old_reg {
                self.pin_reg(new_reg);
            } else {
                self.ra.free_ssa(ssa);
                self.evict_reg_if_used(new_reg);

                self.pcopy.push(
                    RegRef::new(self.file(), new_reg, 1).into(),
                    RegRef::new(self.file(), old_reg, 1).into(),
                );

                self.assign_pin_reg(ssa, new_reg);
            }
        } else if let Some(old_reg) = self.evicted.remove(&ssa) {
            self.evict_reg_if_used(new_reg);

            self.pcopy.push(
                RegRef::new(self.file(), new_reg, 1).into(),
                RegRef::new(self.file(), old_reg, 1).into(),
            );

            self.assign_pin_reg(ssa, new_reg);
        } else {
            panic!("Unknown SSA value");
        }
    }

    fn finish(mut self, pcopy: &mut OpParCopy) {
        pcopy.dsts_srcs.append(&mut self.pcopy.dsts_srcs);

        if !self.evicted.is_empty() {
            // Sort so we get determinism, even if the hash map order changes
            // from one run to another or due to rust compiler updates.
            let mut evicted: Vec<_> = self.evicted.drain().collect();
            evicted.sort_by_key(|(_, reg)| *reg);

            for (ssa, old_reg) in evicted {
                let mut next_reg = 0;
                let new_reg = loop {
                    let reg = self
                        .ra
                        .try_find_unused_reg_range(next_reg, 1, 1)
                        .expect("Failed to find free register");
                    if !self.reg_is_pinned(reg) {
                        break reg;
                    }
                    next_reg = reg + 1;
                };

                pcopy.push(
                    RegRef::new(self.file(), new_reg, 1).into(),
                    RegRef::new(self.file(), old_reg, 1).into(),
                );
                self.assign_pin_reg(ssa, new_reg);
            }
        }
    }

    pub fn try_get_vec_reg(&self, vec: &SSARef) -> Option<u32> {
        self.ra.try_get_vec_reg(vec)
    }

    pub fn collect_vector(&mut self, vec: &SSARef) -> RegRef {
        if let Some(reg) = self.try_get_vec_reg(vec) {
            self.pin_reg_range(reg, vec.comps());
            return RegRef::new(self.file(), reg, vec.comps());
        }

        let comps = vec.comps();
        let align = u32::from(comps).next_power_of_two();

        let reg = self
            .ra
            .try_find_unused_reg_range(0, align, comps)
            .or_else(|| {
                for c in 0..comps {
                    let ssa = vec[usize::from(c)];
                    let Some(comp_reg) = self.ra.try_get_reg(ssa) else {
                        continue;
                    };

                    let vec_reg = comp_reg & !(align - 1);
                    if comp_reg != vec_reg + u32::from(c) {
                        continue;
                    }

                    if vec_reg + u32::from(comps) > self.ra.num_regs {
                        continue;
                    }

                    if self.reg_range_is_unpinned(vec_reg, comps) {
                        return Some(vec_reg);
                    }
                }
                None
            })
            .or_else(|| self.try_find_unpinned_reg_range(0, align, comps))
            .expect("Failed to find an unpinned register range");

        for c in 0..comps {
            let ssa = vec[usize::from(c)];
            self.move_ssa_to_reg(ssa, reg + u32::from(c));
        }

        RegRef::new(self.file(), reg, comps)
    }

    pub fn alloc_vector(&mut self, vec: SSARef) -> RegRef {
        let comps = vec.comps();
        let align = u32::from(comps).next_power_of_two();

        if let Some(reg) = self.ra.try_find_unused_reg_range(0, align, comps) {
            return self.assign_pin_vec_reg(vec, reg);
        }

        let reg = self
            .try_find_unpinned_reg_range(0, align, comps)
            .expect("Failed to find an unpinned register range");

        for c in 0..comps {
            self.evict_reg_if_used(reg + u32::from(c));
        }
        self.assign_pin_vec_reg(vec, reg)
    }

    pub fn free_killed(&mut self, killed: &KillSet) {
        for ssa in killed.iter() {
            if ssa.file() == self.file() {
                self.ra.free_ssa(*ssa);
            }
        }
    }
}

impl Drop for PinnedRegAllocator<'_> {
    fn drop(&mut self) {
        assert!(self.evicted.is_empty());
    }
}

fn instr_remap_srcs_file(instr: &mut Instr, ra: &mut PinnedRegAllocator) {
    // Collect vector sources first since those may silently pin some of our
    // scalar sources.
    for src in instr.srcs_mut() {
        if let SrcRef::SSA(ssa) = &src.src_ref {
            if ssa.file() == ra.file() && ssa.comps() > 1 {
                src.src_ref = ra.collect_vector(ssa).into();
            }
        }
    }

    if let PredRef::SSA(pred) = instr.pred.pred_ref {
        if pred.file() == ra.file() {
            instr.pred.pred_ref = ra.collect_vector(&pred.into()).into();
        }
    }

    for src in instr.srcs_mut() {
        if let SrcRef::SSA(ssa) = &src.src_ref {
            if ssa.file() == ra.file() && ssa.comps() == 1 {
                src.src_ref = ra.collect_vector(ssa).into();
            }
        }
    }
}

fn instr_alloc_scalar_dsts_file(
    instr: &mut Instr,
    ip: usize,
    sum: &SSAUseMap,
    ra: &mut RegAllocator,
) {
    for dst in instr.dsts_mut() {
        if let Dst::SSA(ssa) = dst {
            assert!(ssa.comps() == 1);
            if ssa.file() == ra.file() {
                let reg = ra.alloc_scalar(ip, sum, ssa[0]);
                *dst = RegRef::new(ra.file(), reg, 1).into();
            }
        }
    }
}

fn instr_assign_regs_file(
    instr: &mut Instr,
    ip: usize,
    sum: &SSAUseMap,
    killed: &KillSet,
    pcopy: &mut OpParCopy,
    ra: &mut RegAllocator,
) {
    struct VecDst {
        dst_idx: usize,
        comps: u8,
        killed: Option<SSARef>,
        reg: u32,
    }

    let mut vec_dsts = Vec::new();
    let mut vec_dst_comps = 0;
    for (i, dst) in instr.dsts().iter().enumerate() {
        if let Dst::SSA(ssa) = dst {
            if ssa.file() == ra.file() && ssa.comps() > 1 {
                vec_dsts.push(VecDst {
                    dst_idx: i,
                    comps: ssa.comps(),
                    killed: None,
                    reg: u32::MAX,
                });
                vec_dst_comps += ssa.comps();
            }
        }
    }

    // No vector destinations is the easy case
    if vec_dst_comps == 0 {
        let mut pra = PinnedRegAllocator::new(ra);
        instr_remap_srcs_file(instr, &mut pra);
        pra.free_killed(killed);
        pra.finish(pcopy);
        instr_alloc_scalar_dsts_file(instr, ip, sum, ra);
        return;
    }

    // Predicates can't be vectors.  This lets us ignore instr.pred in our
    // analysis for the cases below. Only the easy case above needs to care
    // about them.
    assert!(!ra.file().is_predicate());

    let mut avail = killed.set.clone();
    let mut killed_vecs = Vec::new();
    for src in instr.srcs() {
        if let SrcRef::SSA(vec) = src.src_ref {
            if vec.comps() > 1 {
                let mut vec_killed = true;
                for ssa in vec.iter() {
                    if ssa.file() != ra.file() || !avail.contains(ssa) {
                        vec_killed = false;
                        break;
                    }
                }
                if vec_killed {
                    for ssa in vec.iter() {
                        avail.remove(ssa);
                    }
                    killed_vecs.push(vec);
                }
            }
        }
    }

    vec_dsts.sort_by_key(|v| v.comps);
    killed_vecs.sort_by_key(|v| v.comps());

    let mut next_dst_reg = 0;
    let mut vec_dsts_map_to_killed_srcs = true;
    let mut could_trivially_allocate = true;
    for vec_dst in vec_dsts.iter_mut().rev() {
        while !killed_vecs.is_empty() {
            let src = killed_vecs.pop().unwrap();
            if src.comps() >= vec_dst.comps {
                vec_dst.killed = Some(src);
                break;
            }
        }
        if vec_dst.killed.is_none() {
            vec_dsts_map_to_killed_srcs = false;
        }

        let align = u32::from(vec_dst.comps).next_power_of_two();
        if let Some(reg) =
            ra.try_find_unused_reg_range(next_dst_reg, align, vec_dst.comps)
        {
            vec_dst.reg = reg;
            next_dst_reg = reg + u32::from(vec_dst.comps);
        } else {
            could_trivially_allocate = false;
        }
    }

    if vec_dsts_map_to_killed_srcs {
        let mut pra = PinnedRegAllocator::new(ra);
        instr_remap_srcs_file(instr, &mut pra);

        for vec_dst in &mut vec_dsts {
            let src_vec = vec_dst.killed.as_ref().unwrap();
            vec_dst.reg = pra.try_get_vec_reg(src_vec).unwrap();
        }

        pra.free_killed(killed);

        for vec_dst in vec_dsts {
            let dst = &mut instr.dsts_mut()[vec_dst.dst_idx];
            *dst = pra
                .assign_pin_vec_reg(*dst.as_ssa().unwrap(), vec_dst.reg)
                .into();
        }

        pra.finish(pcopy);

        instr_alloc_scalar_dsts_file(instr, ip, sum, ra);
    } else if could_trivially_allocate {
        let mut pra = PinnedRegAllocator::new(ra);
        for vec_dst in vec_dsts {
            let dst = &mut instr.dsts_mut()[vec_dst.dst_idx];
            *dst = pra
                .assign_pin_vec_reg(*dst.as_ssa().unwrap(), vec_dst.reg)
                .into();
        }

        instr_remap_srcs_file(instr, &mut pra);
        pra.free_killed(killed);
        pra.finish(pcopy);
        instr_alloc_scalar_dsts_file(instr, ip, sum, ra);
    } else {
        let mut pra = PinnedRegAllocator::new(ra);
        instr_remap_srcs_file(instr, &mut pra);

        // Allocate vector destinations first so we have the most freedom.
        // Scalar destinations can fill in holes.
        for dst in instr.dsts_mut() {
            if let Dst::SSA(ssa) = dst {
                if ssa.file() == pra.file() && ssa.comps() > 1 {
                    *dst = pra.alloc_vector(*ssa).into();
                }
            }
        }

        pra.free_killed(killed);
        pra.finish(pcopy);

        instr_alloc_scalar_dsts_file(instr, ip, sum, ra);
    }
}

impl PerRegFile<RegAllocator> {
    pub fn assign_reg(&mut self, ssa: SSAValue, reg: RegRef) {
        assert!(reg.file() == ssa.file());
        assert!(reg.comps() == 1);
        self[ssa.file()].assign_reg(ssa, reg.base_idx());
    }

    pub fn free_killed(&mut self, killed: &KillSet) {
        for ssa in killed.iter() {
            self[ssa.file()].free_ssa(*ssa);
        }
    }
}

struct AssignRegsBlock {
    ra: PerRegFile<RegAllocator>,
    pcopy_tmp_gprs: u8,
    live_in: Vec<LiveValue>,
    phi_out: HashMap<u32, SrcRef>,
}

impl AssignRegsBlock {
    fn new(num_regs: &PerRegFile<u32>, pcopy_tmp_gprs: u8) -> AssignRegsBlock {
        AssignRegsBlock {
            ra: PerRegFile::new_with(|file| {
                RegAllocator::new(file, num_regs[file])
            }),
            pcopy_tmp_gprs: pcopy_tmp_gprs,
            live_in: Vec::new(),
            phi_out: HashMap::new(),
        }
    }

    fn get_scalar(&self, ssa: SSAValue) -> RegRef {
        let ra = &self.ra[ssa.file()];
        let reg = ra.try_get_reg(ssa).expect("Unknown SSA value");
        RegRef::new(ssa.file(), reg, 1)
    }

    fn alloc_scalar(
        &mut self,
        ip: usize,
        sum: &SSAUseMap,
        ssa: SSAValue,
    ) -> RegRef {
        let ra = &mut self.ra[ssa.file()];
        let reg = ra.alloc_scalar(ip, sum, ssa);
        RegRef::new(ssa.file(), reg, 1)
    }

    fn try_coalesce(&mut self, ssa: SSAValue, src: &Src) -> bool {
        debug_assert!(src.src_mod.is_none());
        let SrcRef::Reg(src_reg) = src.src_ref else {
            return false;
        };
        debug_assert!(src_reg.comps() == 1);

        if src_reg.file() != ssa.file() {
            return false;
        }

        let ra = &mut self.ra[src_reg.file()];
        if ra.reg_is_used(src_reg.base_idx()) {
            return false;
        }

        ra.assign_reg(ssa, src_reg.base_idx());
        true
    }

    fn pcopy_tmp(&self) -> Option<RegRef> {
        if self.pcopy_tmp_gprs > 0 {
            Some(RegRef::new(
                RegFile::GPR,
                self.ra[RegFile::GPR].num_regs,
                self.pcopy_tmp_gprs,
            ))
        } else {
            None
        }
    }

    fn assign_regs_instr(
        &mut self,
        mut instr: Box<Instr>,
        ip: usize,
        sum: &SSAUseMap,
        srcs_killed: &KillSet,
        dsts_killed: &KillSet,
        pcopy: &mut OpParCopy,
    ) -> Option<Box<Instr>> {
        match &mut instr.op {
            Op::Undef(undef) => {
                if let Dst::SSA(ssa) = undef.dst {
                    assert!(ssa.comps() == 1);
                    self.alloc_scalar(ip, sum, ssa[0]);
                }
                assert!(srcs_killed.is_empty());
                self.ra.free_killed(dsts_killed);
                None
            }
            Op::PhiSrcs(phi) => {
                for (id, src) in phi.srcs.iter() {
                    assert!(src.src_mod.is_none());
                    if let SrcRef::SSA(ssa) = src.src_ref {
                        assert!(ssa.comps() == 1);
                        let reg = self.get_scalar(ssa[0]);
                        self.phi_out.insert(*id, reg.into());
                    } else {
                        self.phi_out.insert(*id, src.src_ref);
                    }
                }
                assert!(dsts_killed.is_empty());
                None
            }
            Op::PhiDsts(phi) => {
                assert!(instr.pred.is_true());

                for (id, dst) in phi.dsts.iter() {
                    if let Dst::SSA(ssa) = dst {
                        assert!(ssa.comps() == 1);
                        let reg = self.alloc_scalar(ip, sum, ssa[0]);
                        self.live_in.push(LiveValue {
                            live_ref: LiveRef::Phi(*id),
                            reg_ref: reg,
                        });
                    }
                }
                assert!(srcs_killed.is_empty());
                self.ra.free_killed(dsts_killed);

                None
            }
            Op::Break(op) => {
                for src in op.srcs_as_mut_slice() {
                    if let SrcRef::SSA(ssa) = src.src_ref {
                        assert!(ssa.comps() == 1);
                        let reg = self.get_scalar(ssa[0]);
                        src.src_ref = reg.into();
                    }
                }

                self.ra.free_killed(srcs_killed);

                if let Dst::SSA(ssa) = &op.bar_out {
                    let reg = *op.bar_in.src_ref.as_reg().unwrap();
                    self.ra.assign_reg(ssa[0], reg);
                    op.bar_out = reg.into();
                }

                self.ra.free_killed(dsts_killed);

                Some(instr)
            }
            Op::BSSy(op) => {
                for src in op.srcs_as_mut_slice() {
                    if let SrcRef::SSA(ssa) = src.src_ref {
                        assert!(ssa.comps() == 1);
                        let reg = self.get_scalar(ssa[0]);
                        src.src_ref = reg.into();
                    }
                }

                self.ra.free_killed(srcs_killed);

                if let Dst::SSA(ssa) = &op.bar_out {
                    let reg = *op.bar_in.src_ref.as_reg().unwrap();
                    self.ra.assign_reg(ssa[0], reg);
                    op.bar_out = reg.into();
                }

                self.ra.free_killed(dsts_killed);

                Some(instr)
            }
            Op::Copy(copy) => {
                if let SrcRef::SSA(src_vec) = &copy.src.src_ref {
                    debug_assert!(src_vec.comps() == 1);
                    let src_ssa = &src_vec[0];
                    copy.src.src_ref = self.get_scalar(*src_ssa).into();
                }

                self.ra.free_killed(srcs_killed);

                let mut del_copy = false;
                if let Dst::SSA(dst_vec) = &mut copy.dst {
                    debug_assert!(dst_vec.comps() == 1);
                    let dst_ssa = &dst_vec[0];

                    if self.try_coalesce(*dst_ssa, &copy.src) {
                        del_copy = true;
                    } else {
                        copy.dst = self.alloc_scalar(ip, sum, *dst_ssa).into();
                    }
                }

                self.ra.free_killed(dsts_killed);

                if del_copy {
                    None
                } else {
                    Some(instr)
                }
            }
            Op::ParCopy(pcopy) => {
                for (_, src) in pcopy.dsts_srcs.iter_mut() {
                    if let SrcRef::SSA(src_vec) = src.src_ref {
                        debug_assert!(src_vec.comps() == 1);
                        let src_ssa = &src_vec[0];
                        src.src_ref = self.get_scalar(*src_ssa).into();
                    }
                }

                self.ra.free_killed(srcs_killed);

                // Try to coalesce destinations into sources, if possible
                pcopy.dsts_srcs.retain(|dst, src| match dst {
                    Dst::None => false,
                    Dst::SSA(dst_vec) => {
                        debug_assert!(dst_vec.comps() == 1);
                        !self.try_coalesce(dst_vec[0], src)
                    }
                    Dst::Reg(_) => true,
                });

                for (dst, _) in pcopy.dsts_srcs.iter_mut() {
                    if let Dst::SSA(dst_vec) = dst {
                        debug_assert!(dst_vec.comps() == 1);
                        *dst = self.alloc_scalar(ip, sum, dst_vec[0]).into();
                    }
                }

                self.ra.free_killed(dsts_killed);

                pcopy.tmp = self.pcopy_tmp();
                if pcopy.is_empty() {
                    None
                } else {
                    Some(instr)
                }
            }
            Op::FSOut(out) => {
                for src in out.srcs.iter_mut() {
                    if let SrcRef::SSA(src_vec) = src.src_ref {
                        debug_assert!(src_vec.comps() == 1);
                        let src_ssa = &src_vec[0];
                        src.src_ref = self.get_scalar(*src_ssa).into();
                    }
                }

                self.ra.free_killed(srcs_killed);
                assert!(dsts_killed.is_empty());

                // This should be the last instruction and its sources should
                // be the last free GPRs.
                debug_assert!(self.ra[RegFile::GPR].num_regs_used() == 0);

                for (i, src) in out.srcs.iter().enumerate() {
                    let reg = u32::try_from(i).unwrap();
                    let dst = RegRef::new(RegFile::GPR, reg, 1);
                    pcopy.push(dst.into(), *src);
                }

                None
            }
            _ => {
                for file in self.ra.values_mut() {
                    instr_assign_regs_file(
                        &mut instr,
                        ip,
                        sum,
                        srcs_killed,
                        pcopy,
                        file,
                    );
                }
                self.ra.free_killed(dsts_killed);
                Some(instr)
            }
        }
    }

    fn first_pass<BL: BlockLiveness>(
        &mut self,
        b: &mut BasicBlock,
        bl: &BL,
        pred_ra: Option<&PerRegFile<RegAllocator>>,
    ) {
        // Populate live in from the register file we're handed.  We'll add more
        // live in when we process the OpPhiDst, if any.
        if let Some(pred_ra) = pred_ra {
            for (raf, pred_raf) in self.ra.values_mut().zip(pred_ra.values()) {
                for (ssa, reg) in &pred_raf.ssa_reg {
                    if bl.is_live_in(ssa) {
                        raf.assign_reg(*ssa, *reg);
                        self.live_in.push(LiveValue {
                            live_ref: LiveRef::SSA(*ssa),
                            reg_ref: RegRef::new(raf.file(), *reg, 1),
                        });
                    }
                }
            }
        }

        let sum = SSAUseMap::for_block(b);

        let mut instrs = Vec::new();
        let mut srcs_killed = KillSet::new();
        let mut dsts_killed = KillSet::new();

        for (ip, instr) in b.instrs.drain(..).enumerate() {
            // Build up the kill set
            srcs_killed.clear();
            if let PredRef::SSA(ssa) = &instr.pred.pred_ref {
                if !bl.is_live_after_ip(ssa, ip) {
                    srcs_killed.insert(*ssa);
                }
            }
            for src in instr.srcs() {
                for ssa in src.iter_ssa() {
                    if !bl.is_live_after_ip(ssa, ip) {
                        srcs_killed.insert(*ssa);
                    }
                }
            }

            dsts_killed.clear();
            for dst in instr.dsts() {
                if let Dst::SSA(vec) = dst {
                    for ssa in vec.iter() {
                        if !bl.is_live_after_ip(ssa, ip) {
                            dsts_killed.insert(*ssa);
                        }
                    }
                }
            }

            let mut pcopy = OpParCopy::new();
            pcopy.tmp = self.pcopy_tmp();

            let instr = self.assign_regs_instr(
                instr,
                ip,
                &sum,
                &srcs_killed,
                &dsts_killed,
                &mut pcopy,
            );

            if !pcopy.is_empty() {
                instrs.push(Instr::new_boxed(pcopy));
            }

            if let Some(instr) = instr {
                instrs.push(instr);
            }
        }

        // Sort live-in to maintain determinism
        self.live_in.sort();

        b.instrs = instrs;
    }

    fn second_pass(&self, target: &AssignRegsBlock, b: &mut BasicBlock) {
        let mut pcopy = OpParCopy::new();
        pcopy.tmp = self.pcopy_tmp();

        for lv in &target.live_in {
            let src = match lv.live_ref {
                LiveRef::SSA(ssa) => SrcRef::from(self.get_scalar(ssa)),
                LiveRef::Phi(phi) => *self.phi_out.get(&phi).unwrap(),
            };
            let dst = lv.reg_ref;
            if let SrcRef::Reg(src_reg) = src {
                if dst == src_reg {
                    continue;
                }
            }
            pcopy.push(dst.into(), src.into());
        }

        if b.branch().is_some() {
            b.instrs.insert(b.instrs.len() - 1, Instr::new_boxed(pcopy));
        } else {
            b.instrs.push(Instr::new_boxed(pcopy));
        }
    }
}

impl Shader {
    pub fn assign_regs(&mut self) {
        assert!(self.functions.len() == 1);
        let f = &mut self.functions[0];

        // Convert to CSSA before we spill or assign registers
        f.to_cssa();

        let mut live = SimpleLiveness::for_function(f);
        let mut max_live = live.calc_max_live(f);

        // We want at least one temporary GPR reserved for parallel copies.
        let mut tmp_gprs = 1_u8;

        let spill_files = [RegFile::Pred, RegFile::Bar];
        for file in spill_files {
            let num_regs = file.num_regs(self.info.sm);
            if max_live[file] > num_regs {
                f.spill_values(file, num_regs);

                // Re-calculate liveness after we spill
                live = SimpleLiveness::for_function(f);
                max_live = live.calc_max_live(f);

                match file {
                    RegFile::Bar => {
                        tmp_gprs = max(tmp_gprs, 2);
                    }
                    _ => (),
                }
            }
        }

        // An instruction can have at most 4 vector sources/destinations.  In
        // order to ensure we always succeed at allocation, regardless of
        // arbitrary choices, we need at least 16 GPRs.
        let mut gpr_limit = max(max_live[RegFile::GPR], 16);
        let mut total_gprs = gpr_limit + u32::from(tmp_gprs);

        let max_gprs = RegFile::GPR.num_regs(self.info.sm);
        if total_gprs > max_gprs {
            // If we're spilling GPRs, we need to reserve 2 GPRs for OpParCopy
            // lowering because it needs to be able lower Mem copies which
            // require a temporary
            tmp_gprs = max(tmp_gprs, 2);
            total_gprs = max_gprs;
            gpr_limit = total_gprs - u32::from(tmp_gprs);

            f.spill_values(RegFile::GPR, gpr_limit);

            // Re-calculate liveness one last time
            live = SimpleLiveness::for_function(f);
        }

        self.info.num_gprs = total_gprs.try_into().unwrap();

        // We do a maximum here because nak_from_nir may set num_barriers to 1
        // in the case where there is an OpBar.
        self.info.num_barriers = max(
            self.info.num_barriers,
            max_live[RegFile::Bar].try_into().unwrap(),
        );

        let limit = PerRegFile::new_with(|file| {
            if file == RegFile::GPR {
                gpr_limit
            } else {
                file.num_regs(self.info.sm)
            }
        });

        let mut blocks: Vec<AssignRegsBlock> = Vec::new();
        for b_idx in 0..f.blocks.len() {
            let pred = f.blocks.pred_indices(b_idx);
            let pred_ra = if pred.is_empty() {
                None
            } else {
                // Start with the previous block's.
                Some(&blocks[pred[0]].ra)
            };

            let bl = live.block_live(b_idx);

            let mut arb = AssignRegsBlock::new(&limit, tmp_gprs);
            arb.first_pass(&mut f.blocks[b_idx], bl, pred_ra);

            assert!(blocks.len() == b_idx);
            blocks.push(arb);
        }

        for b_idx in 0..f.blocks.len() {
            let arb = &blocks[b_idx];
            for sb_idx in f.blocks.succ_indices(b_idx).to_vec() {
                arb.second_pass(&blocks[sb_idx], &mut f.blocks[b_idx]);
            }
        }
    }
}
