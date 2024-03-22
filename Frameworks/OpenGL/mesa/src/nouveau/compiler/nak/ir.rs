// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

extern crate bitview;
extern crate nak_ir_proc;

use bitview::BitMutView;

use crate::api::{GetDebugFlags, DEBUG};
pub use crate::builder::{Builder, InstrBuilder, SSABuilder, SSAInstrBuilder};
use crate::cfg::CFG;
use crate::sph::{OutputTopology, PixelImap};
use nak_ir_proc::*;
use std::cmp::{max, min};
use std::fmt;
use std::fmt::Write;
use std::iter::Zip;
use std::ops::{BitAnd, BitOr, Deref, DerefMut, Index, IndexMut, Not, Range};
use std::slice;

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub struct Label {
    idx: u32,
}

impl fmt::Display for Label {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "L{}", self.idx)
    }
}

pub struct LabelAllocator {
    count: u32,
}

impl LabelAllocator {
    pub fn new() -> LabelAllocator {
        LabelAllocator { count: 0 }
    }

    pub fn alloc(&mut self) -> Label {
        let idx = self.count;
        self.count += 1;
        Label { idx: idx }
    }
}

/// Represents a register file
#[repr(u8)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum RegFile {
    /// The general-purpose register file
    ///
    /// General-purpose registers are 32 bits per SIMT channel.
    GPR = 0,

    /// The general-purpose uniform register file
    ///
    /// General-purpose uniform registers are 32 bits each and uniform across a
    /// wave.
    UGPR = 1,

    /// The predicate reigster file
    ///
    /// Predicate registers are 1 bit per SIMT channel.
    Pred = 2,

    /// The uniform predicate reigster file
    ///
    /// Uniform predicate registers are 1 bit and uniform across a wave.
    UPred = 3,

    /// The carry flag register file
    ///
    /// Only one carry flag register exists in hardware, but representing it as
    /// a reg file simplifies dependency tracking.
    ///
    /// This is used only on SM50.
    Carry = 4,

    /// The barrier register file
    ///
    /// This is a lane mask used for wave re-convergence instructions.
    Bar = 5,

    /// The memory register file
    ///
    /// This is a virtual register file for things which will get spilled to
    /// local memory.  Each memory location is 32 bits per SIMT channel.
    Mem = 6,
}

const NUM_REG_FILES: usize = 7;

impl RegFile {
    /// Returns true if the register file is uniform across a wave
    pub fn is_uniform(&self) -> bool {
        match self {
            RegFile::GPR
            | RegFile::Pred
            | RegFile::Carry
            | RegFile::Bar
            | RegFile::Mem => false,
            RegFile::UGPR | RegFile::UPred => true,
        }
    }

    /// Returns true if the register file is general-purpose
    pub fn is_gpr(&self) -> bool {
        match self {
            RegFile::GPR | RegFile::UGPR => true,
            RegFile::Pred
            | RegFile::UPred
            | RegFile::Carry
            | RegFile::Bar
            | RegFile::Mem => false,
        }
    }

    /// Returns true if the register file is a predicate register file
    pub fn is_predicate(&self) -> bool {
        match self {
            RegFile::GPR
            | RegFile::UGPR
            | RegFile::Carry
            | RegFile::Bar
            | RegFile::Mem => false,
            RegFile::Pred | RegFile::UPred => true,
        }
    }

    pub fn num_regs(&self, sm: u8) -> u32 {
        match self {
            RegFile::GPR => {
                if DEBUG.spill() {
                    // We need at least 16 registers to satisfy RA constraints
                    // for texture ops and another 2 for parallel copy lowering
                    18
                } else if sm >= 70 {
                    // Volta+ has a maximum of 253 registers.  Presumably
                    // because two registers get burned for UGPRs? Unclear
                    // on why we need it on Volta though.
                    253
                } else {
                    255
                }
            }
            RegFile::UGPR => {
                if sm >= 75 {
                    63
                } else {
                    0
                }
            }
            RegFile::Pred => 7,
            RegFile::UPred => {
                if sm >= 75 {
                    7
                } else {
                    0
                }
            }
            RegFile::Carry => {
                if sm >= 70 {
                    0
                } else {
                    1
                }
            }
            RegFile::Bar => {
                if sm >= 70 {
                    16
                } else {
                    0
                }
            }
            RegFile::Mem => 1 << 24,
        }
    }

    fn fmt_prefix(&self) -> &'static str {
        match self {
            RegFile::GPR => "r",
            RegFile::UGPR => "ur",
            RegFile::Pred => "p",
            RegFile::UPred => "up",
            RegFile::Carry => "c",
            RegFile::Bar => "b",
            RegFile::Mem => "m",
        }
    }
}

impl fmt::Display for RegFile {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            RegFile::GPR => write!(f, "GPR"),
            RegFile::UGPR => write!(f, "UGPR"),
            RegFile::Pred => write!(f, "Pred"),
            RegFile::UPred => write!(f, "UPred"),
            RegFile::Carry => write!(f, "Carry"),
            RegFile::Bar => write!(f, "Bar"),
            RegFile::Mem => write!(f, "Mem"),
        }
    }
}

impl From<RegFile> for u8 {
    fn from(value: RegFile) -> u8 {
        value as u8
    }
}

impl TryFrom<u32> for RegFile {
    type Error = &'static str;

    fn try_from(value: u32) -> Result<Self, Self::Error> {
        match value {
            0 => Ok(RegFile::GPR),
            1 => Ok(RegFile::UGPR),
            2 => Ok(RegFile::Pred),
            3 => Ok(RegFile::UPred),
            4 => Ok(RegFile::Carry),
            5 => Ok(RegFile::Bar),
            6 => Ok(RegFile::Mem),
            _ => Err("Invalid register file number"),
        }
    }
}

impl TryFrom<u16> for RegFile {
    type Error = &'static str;

    fn try_from(value: u16) -> Result<Self, Self::Error> {
        RegFile::try_from(u32::from(value))
    }
}

impl TryFrom<u8> for RegFile {
    type Error = &'static str;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        RegFile::try_from(u32::from(value))
    }
}

/// A trait for things which have an associated register file
pub trait HasRegFile {
    fn file(&self) -> RegFile;

    fn is_uniform(&self) -> bool {
        self.file().is_uniform()
    }

    fn is_gpr(&self) -> bool {
        self.file().is_gpr()
    }

    fn is_predicate(&self) -> bool {
        self.file().is_predicate()
    }
}

#[derive(Clone)]
pub struct RegFileSet {
    bits: u8,
}

impl RegFileSet {
    pub fn new() -> RegFileSet {
        RegFileSet { bits: 0 }
    }

    pub fn len(&self) -> usize {
        self.bits.count_ones() as usize
    }

    pub fn contains(&self, file: RegFile) -> bool {
        self.bits & (1 << (file as u8)) != 0
    }

    pub fn insert(&mut self, file: RegFile) -> bool {
        let has_file = self.contains(file);
        self.bits |= 1 << (file as u8);
        !has_file
    }

    pub fn is_empty(&self) -> bool {
        self.bits == 0
    }

    #[allow(dead_code)]
    pub fn iter(&self) -> RegFileSet {
        self.clone()
    }

    pub fn remove(&mut self, file: RegFile) -> bool {
        let has_file = self.contains(file);
        self.bits &= !(1 << (file as u8));
        has_file
    }
}

impl FromIterator<RegFile> for RegFileSet {
    fn from_iter<T: IntoIterator<Item = RegFile>>(iter: T) -> Self {
        let mut set = RegFileSet::new();
        for file in iter {
            set.insert(file);
        }
        set
    }
}

impl Iterator for RegFileSet {
    type Item = RegFile;

    fn next(&mut self) -> Option<RegFile> {
        if self.is_empty() {
            None
        } else {
            let file = self.bits.trailing_zeros().try_into().unwrap();
            self.remove(file);
            Some(file)
        }
    }

    fn size_hint(&self) -> (usize, Option<usize>) {
        let len = self.len();
        (len, Some(len))
    }
}

#[derive(Clone, Copy)]
pub struct PerRegFile<T> {
    per_file: [T; NUM_REG_FILES],
}

impl<T> PerRegFile<T> {
    pub fn new_with<F: Fn(RegFile) -> T>(f: F) -> Self {
        PerRegFile {
            per_file: [
                f(RegFile::GPR),
                f(RegFile::UGPR),
                f(RegFile::Pred),
                f(RegFile::UPred),
                f(RegFile::Carry),
                f(RegFile::Bar),
                f(RegFile::Mem),
            ],
        }
    }

    #[allow(dead_code)]
    pub fn values(&self) -> slice::Iter<T> {
        self.per_file.iter()
    }

    #[allow(dead_code)]
    pub fn values_mut(&mut self) -> slice::IterMut<T> {
        self.per_file.iter_mut()
    }
}

impl<T: Default> Default for PerRegFile<T> {
    fn default() -> Self {
        PerRegFile {
            per_file: Default::default(),
        }
    }
}

impl<T> Index<RegFile> for PerRegFile<T> {
    type Output = T;

    fn index(&self, idx: RegFile) -> &T {
        &self.per_file[idx as u8 as usize]
    }
}

impl<T> IndexMut<RegFile> for PerRegFile<T> {
    fn index_mut(&mut self, idx: RegFile) -> &mut T {
        &mut self.per_file[idx as u8 as usize]
    }
}

/// An SSA value
///
/// Each SSA in NAK represents a single 32-bit or 1-bit (if a predicate) value
/// which must either be spilled to memory or allocated space in the specified
/// register file.  Whenever more data is required such as a 64-bit memory
/// address, double-precision float, or a vec4 texture result, multiple SSA
/// values are used.
///
/// Each SSA value logically contains two things: an index and a register file.
/// It is required that each index refers to a unique SSA value, regardless of
/// register file.  This way the index can be used to index tightly-packed data
/// structures such as bitsets without having to determine separate ranges for
/// each register file.
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub struct SSAValue {
    packed: u32,
}

impl SSAValue {
    /// A special SSA value which is always invalid
    pub const NONE: Self = SSAValue { packed: 0 };

    /// Returns an SSA value with the given register file and index
    pub fn new(file: RegFile, idx: u32) -> SSAValue {
        assert!(idx > 0 && idx < (1 << 29) - 2);
        let mut packed = idx;
        assert!(u8::from(file) < 8);
        packed |= u32::from(u8::from(file)) << 29;
        SSAValue { packed: packed }
    }

    /// Returns the index of this SSA value
    pub fn idx(&self) -> u32 {
        self.packed & 0x1fffffff
    }

    /// Returns true if this SSA value is equal to SSAValue::NONE
    #[allow(dead_code)]
    pub fn is_none(&self) -> bool {
        self.packed == 0
    }

    fn fmt_plain(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}{}", self.file().fmt_prefix(), self.idx())
    }
}

impl HasRegFile for SSAValue {
    /// Returns the register file of this SSA value
    fn file(&self) -> RegFile {
        RegFile::try_from(self.packed >> 29).unwrap()
    }
}

impl fmt::Display for SSAValue {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "%")?;
        self.fmt_plain(f)
    }
}

/// A reference to one or more SSA values
///
/// Because each SSA value represents a single 1 or 32-bit scalar, we need a way
/// to reference multiple SSA values for instructions which read or write
/// multiple registers in the same source.  When the register allocator runs,
/// all the SSA values in a given SSA ref will be placed in consecutive
/// registers, with the base register aligned to the number of values, aligned
/// to the next power of two.
///
/// An SSA reference can reference between 1 and 4 SSA values.  It dereferences
/// to a slice for easy access to individual SSA values.  The structure is
/// designed so that is always 16B, regardless of how many SSA values are
/// referenced so it's easy and fairly cheap to copy around and embed in other
/// structures.
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub struct SSARef {
    v: [SSAValue; 4],
}

impl SSARef {
    /// Returns a new SSA reference
    #[inline]
    fn new(comps: &[SSAValue]) -> SSARef {
        assert!(comps.len() > 0 && comps.len() <= 4);
        let mut r = SSARef {
            v: [SSAValue::NONE; 4],
        };
        for i in 0..comps.len() {
            r.v[i] = comps[i];
        }
        if comps.len() < 4 {
            r.v[3].packed = (comps.len() as u32).wrapping_neg();
        }
        r
    }

    /// Returns the number of components in this SSA reference
    pub fn comps(&self) -> u8 {
        if self.v[3].packed >= u32::MAX - 2 {
            self.v[3].packed.wrapping_neg() as u8
        } else {
            4
        }
    }
}

impl HasRegFile for SSARef {
    fn file(&self) -> RegFile {
        let comps = usize::from(self.comps());
        for i in 1..comps {
            assert!(self.v[i].file() == self.v[0].file());
        }
        self.v[0].file()
    }
}

impl Deref for SSARef {
    type Target = [SSAValue];

    fn deref(&self) -> &[SSAValue] {
        let comps = usize::from(self.comps());
        &self.v[..comps]
    }
}

impl DerefMut for SSARef {
    fn deref_mut(&mut self) -> &mut [SSAValue] {
        let comps = usize::from(self.comps());
        &mut self.v[..comps]
    }
}

impl TryFrom<&[SSAValue]> for SSARef {
    type Error = &'static str;

    fn try_from(comps: &[SSAValue]) -> Result<Self, Self::Error> {
        if comps.len() == 0 {
            Err("Empty vector")
        } else if comps.len() > 4 {
            Err("Too many vector components")
        } else {
            Ok(SSARef::new(comps))
        }
    }
}

impl TryFrom<Vec<SSAValue>> for SSARef {
    type Error = &'static str;

    fn try_from(comps: Vec<SSAValue>) -> Result<Self, Self::Error> {
        SSARef::try_from(&comps[..])
    }
}

macro_rules! impl_ssa_ref_from_arr {
    ($n: expr) => {
        impl From<[SSAValue; $n]> for SSARef {
            fn from(comps: [SSAValue; $n]) -> Self {
                SSARef::new(&comps[..])
            }
        }
    };
}
impl_ssa_ref_from_arr!(1);
impl_ssa_ref_from_arr!(2);
impl_ssa_ref_from_arr!(3);
impl_ssa_ref_from_arr!(4);

impl From<SSAValue> for SSARef {
    fn from(val: SSAValue) -> Self {
        [val].into()
    }
}

impl fmt::Display for SSARef {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.comps() == 1 {
            write!(f, "{}", self[0])
        } else {
            write!(f, "{{")?;
            for (i, v) in self.iter().enumerate() {
                if i != 0 {
                    write!(f, " ")?;
                }
                write!(f, "{}", v)?;
            }
            write!(f, "}}")
        }
    }
}

pub struct SSAValueAllocator {
    count: u32,
}

impl SSAValueAllocator {
    pub fn new() -> SSAValueAllocator {
        SSAValueAllocator { count: 0 }
    }

    pub fn max_idx(&self) -> u32 {
        self.count
    }

    pub fn alloc(&mut self, file: RegFile) -> SSAValue {
        self.count += 1;
        SSAValue::new(file, self.count)
    }

    pub fn alloc_vec(&mut self, file: RegFile, comps: u8) -> SSARef {
        assert!(comps >= 1 && comps <= 4);
        let mut vec = [SSAValue::NONE; 4];
        for c in 0..comps {
            vec[usize::from(c)] = self.alloc(file);
        }
        vec[0..usize::from(comps)].try_into().unwrap()
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub struct RegRef {
    packed: u32,
}

impl RegRef {
    fn zero_idx(file: RegFile) -> u32 {
        match file {
            RegFile::GPR => 255,
            RegFile::UGPR => 63,
            RegFile::Pred => 7,
            RegFile::UPred => 7,
            RegFile::Carry => panic!("Carry has no zero index"),
            RegFile::Bar => panic!("Bar has no zero index"),
            RegFile::Mem => panic!("Mem has no zero index"),
        }
    }

    pub fn new(file: RegFile, base_idx: u32, comps: u8) -> RegRef {
        assert!(base_idx < (1 << 26));
        let mut packed = base_idx;
        assert!(comps > 0 && comps <= 8);
        packed |= u32::from(comps - 1) << 26;
        assert!(u8::from(file) < 8);
        packed |= u32::from(u8::from(file)) << 29;
        RegRef { packed: packed }
    }

    pub fn zero(file: RegFile, comps: u8) -> RegRef {
        RegRef::new(file, RegRef::zero_idx(file), comps)
    }

    pub fn base_idx(&self) -> u32 {
        self.packed & 0x03ffffff
    }

    pub fn idx_range(&self) -> Range<u32> {
        let start = self.base_idx();
        let end = start + u32::from(self.comps());
        start..end
    }

    pub fn comps(&self) -> u8 {
        (((self.packed >> 26) & 0x7) + 1).try_into().unwrap()
    }

    pub fn comp(&self, c: u8) -> RegRef {
        assert!(c < self.comps());
        RegRef::new(self.file(), self.base_idx() + u32::from(c), 1)
    }
}

impl HasRegFile for RegRef {
    fn file(&self) -> RegFile {
        ((self.packed >> 29) & 0x7).try_into().unwrap()
    }
}

impl fmt::Display for RegRef {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}{}", self.file().fmt_prefix(), self.base_idx())?;
        if self.comps() > 1 {
            write!(f, "..{}", self.idx_range().end)?;
        }
        Ok(())
    }
}

#[derive(Clone, Copy)]
pub enum Dst {
    None,
    SSA(SSARef),
    Reg(RegRef),
}

impl Dst {
    pub fn is_none(&self) -> bool {
        match self {
            Dst::None => true,
            _ => false,
        }
    }

    pub fn as_reg(&self) -> Option<&RegRef> {
        match self {
            Dst::Reg(r) => Some(r),
            _ => None,
        }
    }

    pub fn as_ssa(&self) -> Option<&SSARef> {
        match self {
            Dst::SSA(r) => Some(r),
            _ => None,
        }
    }

    pub fn iter_ssa(&self) -> slice::Iter<'_, SSAValue> {
        match self {
            Dst::None | Dst::Reg(_) => &[],
            Dst::SSA(ssa) => ssa.deref(),
        }
        .iter()
    }

    pub fn iter_ssa_mut(&mut self) -> slice::IterMut<'_, SSAValue> {
        match self {
            Dst::None | Dst::Reg(_) => &mut [],
            Dst::SSA(ssa) => ssa.deref_mut(),
        }
        .iter_mut()
    }
}

impl From<RegRef> for Dst {
    fn from(reg: RegRef) -> Dst {
        Dst::Reg(reg)
    }
}

impl<T: Into<SSARef>> From<T> for Dst {
    fn from(ssa: T) -> Dst {
        Dst::SSA(ssa.into())
    }
}

impl fmt::Display for Dst {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Dst::None => write!(f, "null")?,
            Dst::SSA(v) => v.fmt(f)?,
            Dst::Reg(r) => r.fmt(f)?,
        }
        Ok(())
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum CBuf {
    Binding(u8),

    #[allow(dead_code)]
    BindlessSSA(SSAValue),

    #[allow(dead_code)]
    BindlessGPR(RegRef),
}

impl fmt::Display for CBuf {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            CBuf::Binding(idx) => write!(f, "c[{:#x}]", idx),
            CBuf::BindlessSSA(v) => write!(f, "cx[{}]", v),
            CBuf::BindlessGPR(r) => write!(f, "cx[{}]", r),
        }
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub struct CBufRef {
    pub buf: CBuf,
    pub offset: u16,
}

impl CBufRef {
    pub fn offset(self, offset: u16) -> CBufRef {
        CBufRef {
            buf: self.buf,
            offset: self.offset + offset,
        }
    }
}

impl fmt::Display for CBufRef {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}[{:#x}]", self.buf, self.offset)
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum SrcRef {
    Zero,
    True,
    False,
    Imm32(u32),
    CBuf(CBufRef),
    SSA(SSARef),
    Reg(RegRef),
}

impl SrcRef {
    pub fn is_alu(&self) -> bool {
        match self {
            SrcRef::Zero | SrcRef::Imm32(_) | SrcRef::CBuf(_) => true,
            SrcRef::SSA(ssa) => ssa.is_gpr(),
            SrcRef::Reg(reg) => reg.is_gpr(),
            SrcRef::True | SrcRef::False => false,
        }
    }

    pub fn is_predicate(&self) -> bool {
        match self {
            SrcRef::Zero | SrcRef::Imm32(_) | SrcRef::CBuf(_) => false,
            SrcRef::True | SrcRef::False => true,
            SrcRef::SSA(ssa) => ssa.is_predicate(),
            SrcRef::Reg(reg) => reg.is_predicate(),
        }
    }

    pub fn is_barrier(&self) -> bool {
        match self {
            SrcRef::SSA(ssa) => ssa.file() == RegFile::Bar,
            SrcRef::Reg(reg) => reg.file() == RegFile::Bar,
            _ => false,
        }
    }

    pub fn as_reg(&self) -> Option<&RegRef> {
        match self {
            SrcRef::Reg(r) => Some(r),
            _ => None,
        }
    }

    pub fn as_ssa(&self) -> Option<&SSARef> {
        match self {
            SrcRef::SSA(r) => Some(r),
            _ => None,
        }
    }

    pub fn get_reg(&self) -> Option<&RegRef> {
        match self {
            SrcRef::Zero
            | SrcRef::True
            | SrcRef::False
            | SrcRef::Imm32(_)
            | SrcRef::SSA(_) => None,
            SrcRef::CBuf(cb) => match &cb.buf {
                CBuf::Binding(_) | CBuf::BindlessSSA(_) => None,
                CBuf::BindlessGPR(reg) => Some(reg),
            },
            SrcRef::Reg(reg) => Some(reg),
        }
    }

    pub fn iter_ssa(&self) -> slice::Iter<'_, SSAValue> {
        match self {
            SrcRef::Zero
            | SrcRef::True
            | SrcRef::False
            | SrcRef::Imm32(_)
            | SrcRef::Reg(_) => &[],
            SrcRef::CBuf(cb) => match &cb.buf {
                CBuf::Binding(_) | CBuf::BindlessGPR(_) => &[],
                CBuf::BindlessSSA(ssa) => slice::from_ref(ssa),
            },
            SrcRef::SSA(ssa) => ssa.deref(),
        }
        .iter()
    }

    pub fn iter_ssa_mut(&mut self) -> slice::IterMut<'_, SSAValue> {
        match self {
            SrcRef::Zero
            | SrcRef::True
            | SrcRef::False
            | SrcRef::Imm32(_)
            | SrcRef::Reg(_) => &mut [],
            SrcRef::CBuf(cb) => match &mut cb.buf {
                CBuf::Binding(_) | CBuf::BindlessGPR(_) => &mut [],
                CBuf::BindlessSSA(ssa) => slice::from_mut(ssa),
            },
            SrcRef::SSA(ssa) => ssa.deref_mut(),
        }
        .iter_mut()
    }
}

impl From<bool> for SrcRef {
    fn from(b: bool) -> SrcRef {
        if b {
            SrcRef::True
        } else {
            SrcRef::False
        }
    }
}

impl From<u32> for SrcRef {
    fn from(u: u32) -> SrcRef {
        if u == 0 {
            SrcRef::Zero
        } else {
            SrcRef::Imm32(u)
        }
    }
}

impl From<f32> for SrcRef {
    fn from(f: f32) -> SrcRef {
        f.to_bits().into()
    }
}

impl From<CBufRef> for SrcRef {
    fn from(cb: CBufRef) -> SrcRef {
        SrcRef::CBuf(cb)
    }
}

impl From<RegRef> for SrcRef {
    fn from(reg: RegRef) -> SrcRef {
        SrcRef::Reg(reg)
    }
}

impl<T: Into<SSARef>> From<T> for SrcRef {
    fn from(ssa: T) -> SrcRef {
        SrcRef::SSA(ssa.into())
    }
}

impl fmt::Display for SrcRef {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            SrcRef::Zero => write!(f, "rZ"),
            SrcRef::True => write!(f, "pT"),
            SrcRef::False => write!(f, "pF"),
            SrcRef::Imm32(u) => write!(f, "{:#x}", u),
            SrcRef::CBuf(c) => c.fmt(f),
            SrcRef::SSA(v) => v.fmt(f),
            SrcRef::Reg(r) => r.fmt(f),
        }
    }
}

#[derive(Clone, Copy, PartialEq)]
pub enum SrcMod {
    None,
    FAbs,
    FNeg,
    FNegAbs,
    INeg,
    BNot,
}

impl SrcMod {
    pub fn is_none(&self) -> bool {
        match self {
            SrcMod::None => true,
            _ => false,
        }
    }

    pub fn has_fabs(&self) -> bool {
        match self {
            SrcMod::None | SrcMod::FNeg => false,
            SrcMod::FAbs | SrcMod::FNegAbs => true,
            _ => panic!("Not a float modifier"),
        }
    }

    pub fn has_fneg(&self) -> bool {
        match self {
            SrcMod::None | SrcMod::FAbs => false,
            SrcMod::FNeg | SrcMod::FNegAbs => true,
            _ => panic!("Not a float modifier"),
        }
    }

    pub fn is_ineg(&self) -> bool {
        match self {
            SrcMod::None => false,
            SrcMod::INeg => true,
            _ => panic!("Not an integer modifier"),
        }
    }

    pub fn is_bnot(&self) -> bool {
        match self {
            SrcMod::None => false,
            SrcMod::BNot => true,
            _ => panic!("Not a bitwise modifier"),
        }
    }

    pub fn fabs(self) -> SrcMod {
        match self {
            SrcMod::None | SrcMod::FAbs | SrcMod::FNeg | SrcMod::FNegAbs => {
                SrcMod::FAbs
            }
            _ => panic!("Not a float source modifier"),
        }
    }

    pub fn fneg(self) -> SrcMod {
        match self {
            SrcMod::None => SrcMod::FNeg,
            SrcMod::FAbs => SrcMod::FNegAbs,
            SrcMod::FNeg => SrcMod::None,
            SrcMod::FNegAbs => SrcMod::FAbs,
            _ => panic!("Not a float source modifier"),
        }
    }

    pub fn ineg(self) -> SrcMod {
        match self {
            SrcMod::None => SrcMod::INeg,
            SrcMod::INeg => SrcMod::None,
            _ => panic!("Not an integer source modifier"),
        }
    }

    pub fn bnot(self) -> SrcMod {
        match self {
            SrcMod::None => SrcMod::BNot,
            SrcMod::BNot => SrcMod::None,
            _ => panic!("Not a boolean source modifier"),
        }
    }

    pub fn modify(self, other: SrcMod) -> SrcMod {
        match other {
            SrcMod::None => self,
            SrcMod::FAbs => self.fabs(),
            SrcMod::FNeg => self.fneg(),
            SrcMod::FNegAbs => self.fabs().fneg(),
            SrcMod::INeg => self.ineg(),
            SrcMod::BNot => self.bnot(),
        }
    }
}

#[repr(u8)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum SrcType {
    SSA,
    GPR,
    ALU,
    F32,
    F64,
    I32,
    B32,
    Pred,
    Bar,
}

#[derive(Clone, Copy, PartialEq)]
pub struct Src {
    pub src_ref: SrcRef,
    pub src_mod: SrcMod,
}

impl Src {
    pub fn new_zero() -> Src {
        SrcRef::Zero.into()
    }

    pub fn new_imm_u32(u: u32) -> Src {
        u.into()
    }

    pub fn new_imm_bool(b: bool) -> Src {
        b.into()
    }

    pub fn fabs(&self) -> Src {
        Src {
            src_ref: self.src_ref,
            src_mod: self.src_mod.fabs(),
        }
    }

    pub fn fneg(&self) -> Src {
        Src {
            src_ref: self.src_ref,
            src_mod: self.src_mod.fneg(),
        }
    }

    pub fn ineg(&self) -> Src {
        Src {
            src_ref: self.src_ref,
            src_mod: self.src_mod.ineg(),
        }
    }

    pub fn bnot(&self) -> Src {
        Src {
            src_ref: self.src_ref,
            src_mod: self.src_mod.bnot(),
        }
    }

    pub fn as_ssa(&self) -> Option<&SSARef> {
        if self.src_mod.is_none() {
            self.src_ref.as_ssa()
        } else {
            None
        }
    }

    pub fn as_bool(&self) -> Option<bool> {
        match self.src_ref {
            SrcRef::True => Some(!self.src_mod.is_bnot()),
            SrcRef::False => Some(self.src_mod.is_bnot()),
            SrcRef::SSA(vec) => {
                assert!(vec.is_predicate() && vec.comps() == 1);
                None
            }
            SrcRef::Reg(reg) => {
                assert!(reg.is_predicate() && reg.comps() == 1);
                None
            }
            _ => panic!("Not a boolean source"),
        }
    }

    pub fn as_u32(&self) -> Option<u32> {
        if self.src_mod.is_none() {
            match self.src_ref {
                SrcRef::Zero => Some(0),
                SrcRef::Imm32(u) => Some(u),
                SrcRef::CBuf(_) | SrcRef::SSA(_) | SrcRef::Reg(_) => None,
                _ => panic!("Invalid integer source"),
            }
        } else {
            None
        }
    }

    pub fn as_imm_not_i20(&self) -> Option<u32> {
        match self.src_ref {
            SrcRef::Imm32(i) => {
                assert!(self.src_mod.is_none());
                let top = i & 0xfff80000;
                if top == 0 || top == 0xfff80000 {
                    None
                } else {
                    Some(i)
                }
            }
            _ => None,
        }
    }

    pub fn as_imm_not_f20(&self) -> Option<u32> {
        match self.src_ref {
            SrcRef::Imm32(i) => {
                assert!(self.src_mod.is_none());
                if (i & 0xfff) == 0 {
                    None
                } else {
                    Some(i)
                }
            }
            _ => None,
        }
    }

    pub fn iter_ssa(&self) -> slice::Iter<'_, SSAValue> {
        self.src_ref.iter_ssa()
    }

    pub fn iter_ssa_mut(&mut self) -> slice::IterMut<'_, SSAValue> {
        self.src_ref.iter_ssa_mut()
    }

    #[allow(dead_code)]
    pub fn is_uniform(&self) -> bool {
        match self.src_ref {
            SrcRef::Zero
            | SrcRef::True
            | SrcRef::False
            | SrcRef::Imm32(_)
            | SrcRef::CBuf(_) => true,
            SrcRef::SSA(ssa) => ssa.is_uniform(),
            SrcRef::Reg(reg) => reg.is_uniform(),
        }
    }

    pub fn is_predicate(&self) -> bool {
        self.src_ref.is_predicate()
    }

    pub fn is_zero(&self) -> bool {
        match self.src_ref {
            SrcRef::Zero | SrcRef::Imm32(0) => match self.src_mod {
                SrcMod::None | SrcMod::FAbs | SrcMod::INeg => true,
                SrcMod::FNeg | SrcMod::FNegAbs | SrcMod::BNot => false,
            },
            _ => false,
        }
    }

    pub fn is_fneg_zero(&self, src_type: SrcType) -> bool {
        match self.src_ref {
            SrcRef::Zero | SrcRef::Imm32(0) => match self.src_mod {
                SrcMod::FNeg | SrcMod::FNegAbs => true,
                _ => false,
            },
            SrcRef::Imm32(0x80000000) => {
                src_type == SrcType::F32 && self.src_mod.is_none()
            }
            _ => false,
        }
    }

    #[allow(dead_code)]
    pub fn supports_type(&self, src_type: &SrcType) -> bool {
        match src_type {
            SrcType::SSA => {
                if !self.src_mod.is_none() {
                    return false;
                }

                match self.src_ref {
                    SrcRef::SSA(_) | SrcRef::Reg(_) => true,
                    _ => false,
                }
            }
            SrcType::GPR => {
                if !self.src_mod.is_none() {
                    return false;
                }

                match self.src_ref {
                    SrcRef::Zero | SrcRef::SSA(_) | SrcRef::Reg(_) => true,
                    _ => false,
                }
            }
            SrcType::ALU => self.src_mod.is_none() && self.src_ref.is_alu(),
            SrcType::F32 | SrcType::F64 => {
                match self.src_mod {
                    SrcMod::None
                    | SrcMod::FAbs
                    | SrcMod::FNeg
                    | SrcMod::FNegAbs => (),
                    _ => return false,
                }

                self.src_ref.is_alu()
            }
            SrcType::I32 => {
                match self.src_mod {
                    SrcMod::None | SrcMod::INeg => (),
                    _ => return false,
                }

                self.src_ref.is_alu()
            }
            SrcType::B32 => {
                match self.src_mod {
                    SrcMod::None | SrcMod::BNot => (),
                    _ => return false,
                }

                self.src_ref.is_alu()
            }
            SrcType::Pred => {
                match self.src_mod {
                    SrcMod::None | SrcMod::BNot => (),
                    _ => return false,
                }

                self.src_ref.is_predicate()
            }
            SrcType::Bar => self.src_mod.is_none() && self.src_ref.is_barrier(),
        }
    }
}

impl<T: Into<SrcRef>> From<T> for Src {
    fn from(value: T) -> Src {
        Src {
            src_ref: value.into(),
            src_mod: SrcMod::None,
        }
    }
}

impl fmt::Display for Src {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.src_mod {
            SrcMod::None => write!(f, "{}", self.src_ref),
            SrcMod::FAbs => write!(f, "|{}|", self.src_ref),
            SrcMod::FNeg => write!(f, "-{}", self.src_ref),
            SrcMod::FNegAbs => write!(f, "-|{}|", self.src_ref),
            SrcMod::INeg => write!(f, "-{}", self.src_ref),
            SrcMod::BNot => write!(f, "!{}", self.src_ref),
        }
    }
}

impl SrcType {
    const DEFAULT: SrcType = SrcType::GPR;
}

pub enum SrcTypeList {
    Array(&'static [SrcType]),
    Uniform(SrcType),
}

impl Index<usize> for SrcTypeList {
    type Output = SrcType;

    fn index(&self, idx: usize) -> &SrcType {
        match self {
            SrcTypeList::Array(arr) => &arr[idx],
            SrcTypeList::Uniform(typ) => &typ,
        }
    }
}

pub trait SrcsAsSlice {
    fn srcs_as_slice(&self) -> &[Src];
    fn srcs_as_mut_slice(&mut self) -> &mut [Src];
    fn src_types(&self) -> SrcTypeList;
}

pub trait DstsAsSlice {
    fn dsts_as_slice(&self) -> &[Dst];
    fn dsts_as_mut_slice(&mut self) -> &mut [Dst];
}

fn fmt_dst_slice(f: &mut fmt::Formatter<'_>, dsts: &[Dst]) -> fmt::Result {
    if dsts.is_empty() {
        return Ok(());
    }

    // Figure out the last non-null dst
    //
    // Note: By making the top inclusive and starting at 0, we ensure that
    // at least one dst always gets printed.
    let mut last_dst = 0;
    for (i, dst) in dsts.iter().enumerate() {
        if !dst.is_none() {
            last_dst = i;
        }
    }

    for i in 0..(last_dst + 1) {
        if i != 0 {
            write!(f, " ")?;
        }
        write!(f, "{}", &dsts[i])?;
    }
    Ok(())
}

pub trait DisplayOp: DstsAsSlice {
    fn fmt_dsts(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        fmt_dst_slice(f, self.dsts_as_slice())
    }

    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result;
}

// Hack struct so we can re-use Formatters.  Shamelessly stolen from
// https://users.rust-lang.org/t/reusing-an-fmt-formatter/8531/4
pub struct Fmt<F>(pub F)
where
    F: Fn(&mut fmt::Formatter) -> fmt::Result;

impl<F> fmt::Display for Fmt<F>
where
    F: Fn(&mut fmt::Formatter) -> fmt::Result,
{
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        (self.0)(f)
    }
}

macro_rules! impl_display_for_op {
    ($op: ident) => {
        impl fmt::Display for $op {
            fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
                let mut s = String::new();
                write!(s, "{}", Fmt(|f| self.fmt_dsts(f)))?;
                if !s.is_empty() {
                    write!(f, "{} = ", s)?;
                }
                self.fmt_op(f)
            }
        }
    };
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum PredSetOp {
    And,
    Or,
    Xor,
}

impl PredSetOp {
    pub fn is_trivial(&self, accum: &Src) -> bool {
        if let Some(b) = accum.as_bool() {
            match self {
                PredSetOp::And => b,
                PredSetOp::Or => !b,
                PredSetOp::Xor => !b,
            }
        } else {
            false
        }
    }
}

impl fmt::Display for PredSetOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            PredSetOp::And => write!(f, ".and"),
            PredSetOp::Or => write!(f, ".or"),
            PredSetOp::Xor => write!(f, ".xor"),
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum FloatCmpOp {
    OrdEq,
    OrdNe,
    OrdLt,
    OrdLe,
    OrdGt,
    OrdGe,
    UnordEq,
    UnordNe,
    UnordLt,
    UnordLe,
    UnordGt,
    UnordGe,
    IsNum,
    IsNan,
}

impl FloatCmpOp {
    pub fn flip(self) -> FloatCmpOp {
        match self {
            FloatCmpOp::OrdEq | FloatCmpOp::OrdNe => self,
            FloatCmpOp::OrdLt => FloatCmpOp::OrdGt,
            FloatCmpOp::OrdLe => FloatCmpOp::OrdGe,
            FloatCmpOp::OrdGt => FloatCmpOp::OrdLt,
            FloatCmpOp::OrdGe => FloatCmpOp::OrdLe,
            FloatCmpOp::UnordEq | FloatCmpOp::UnordNe => self,
            FloatCmpOp::UnordLt => FloatCmpOp::UnordGt,
            FloatCmpOp::UnordLe => FloatCmpOp::UnordGe,
            FloatCmpOp::UnordGt => FloatCmpOp::UnordLt,
            FloatCmpOp::UnordGe => FloatCmpOp::UnordLe,
            FloatCmpOp::IsNum | FloatCmpOp::IsNan => panic!("Cannot flip unop"),
        }
    }
}

impl fmt::Display for FloatCmpOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            FloatCmpOp::OrdEq => write!(f, ".eq"),
            FloatCmpOp::OrdNe => write!(f, ".ne"),
            FloatCmpOp::OrdLt => write!(f, ".lt"),
            FloatCmpOp::OrdLe => write!(f, ".le"),
            FloatCmpOp::OrdGt => write!(f, ".gt"),
            FloatCmpOp::OrdGe => write!(f, ".ge"),
            FloatCmpOp::UnordEq => write!(f, ".equ"),
            FloatCmpOp::UnordNe => write!(f, ".neu"),
            FloatCmpOp::UnordLt => write!(f, ".ltu"),
            FloatCmpOp::UnordLe => write!(f, ".leu"),
            FloatCmpOp::UnordGt => write!(f, ".gtu"),
            FloatCmpOp::UnordGe => write!(f, ".geu"),
            FloatCmpOp::IsNum => write!(f, ".num"),
            FloatCmpOp::IsNan => write!(f, ".nan"),
        }
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum IntCmpOp {
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,
}

impl IntCmpOp {
    pub fn flip(self) -> IntCmpOp {
        match self {
            IntCmpOp::Eq | IntCmpOp::Ne => self,
            IntCmpOp::Lt => IntCmpOp::Gt,
            IntCmpOp::Le => IntCmpOp::Ge,
            IntCmpOp::Gt => IntCmpOp::Lt,
            IntCmpOp::Ge => IntCmpOp::Le,
        }
    }
}

impl fmt::Display for IntCmpOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IntCmpOp::Eq => write!(f, ".eq"),
            IntCmpOp::Ne => write!(f, ".ne"),
            IntCmpOp::Lt => write!(f, ".lt"),
            IntCmpOp::Le => write!(f, ".le"),
            IntCmpOp::Gt => write!(f, ".gt"),
            IntCmpOp::Ge => write!(f, ".ge"),
        }
    }
}

pub enum IntCmpType {
    U32,
    I32,
}

impl fmt::Display for IntCmpType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IntCmpType::U32 => write!(f, ".u32"),
            IntCmpType::I32 => write!(f, ".i32"),
        }
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum LogicOp2 {
    And,
    Or,
    Xor,
    PassB,
}

impl fmt::Display for LogicOp2 {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            LogicOp2::And => write!(f, "and"),
            LogicOp2::Or => write!(f, "or"),
            LogicOp2::Xor => write!(f, "xor"),
            LogicOp2::PassB => write!(f, "pass_b"),
        }
    }
}

impl LogicOp2 {
    pub fn to_lut(self) -> LogicOp3 {
        match self {
            LogicOp2::And => LogicOp3::new_lut(&|x, y, _| x & y),
            LogicOp2::Or => LogicOp3::new_lut(&|x, y, _| x | y),
            LogicOp2::Xor => LogicOp3::new_lut(&|x, y, _| x ^ y),
            LogicOp2::PassB => LogicOp3::new_lut(&|_, b, _| b),
        }
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub struct LogicOp3 {
    pub lut: u8,
}

impl LogicOp3 {
    pub const SRC_MASKS: [u8; 3] = [0xf0, 0xcc, 0xaa];

    #[inline]
    pub fn new_lut<F: Fn(u8, u8, u8) -> u8>(f: &F) -> LogicOp3 {
        LogicOp3 {
            lut: f(
                LogicOp3::SRC_MASKS[0],
                LogicOp3::SRC_MASKS[1],
                LogicOp3::SRC_MASKS[2],
            ),
        }
    }

    pub fn new_const(val: bool) -> LogicOp3 {
        LogicOp3 {
            lut: if val { !0 } else { 0 },
        }
    }

    pub fn src_used(&self, src_idx: usize) -> bool {
        let mask = LogicOp3::SRC_MASKS[src_idx];
        let shift = LogicOp3::SRC_MASKS[src_idx].trailing_zeros();
        self.lut & !mask != (self.lut >> shift) & !mask
    }

    pub fn fix_src(&mut self, src_idx: usize, val: bool) {
        let mask = LogicOp3::SRC_MASKS[src_idx];
        let shift = LogicOp3::SRC_MASKS[src_idx].trailing_zeros();
        if val {
            let t_bits = self.lut & mask;
            self.lut = t_bits | (t_bits >> shift)
        } else {
            let f_bits = self.lut & !mask;
            self.lut = (f_bits << shift) | f_bits
        };
    }

    pub fn invert_src(&mut self, src_idx: usize) {
        let mask = LogicOp3::SRC_MASKS[src_idx];
        let shift = LogicOp3::SRC_MASKS[src_idx].trailing_zeros();
        let t_bits = self.lut & mask;
        let f_bits = self.lut & !mask;
        self.lut = (f_bits << shift) | (t_bits >> shift);
    }

    pub fn eval<
        T: BitAnd<Output = T> + BitOr<Output = T> + Copy + Not<Output = T>,
    >(
        &self,
        x: T,
        y: T,
        z: T,
    ) -> T {
        let mut res = x & !x; // zero
        if (self.lut & (1 << 0)) != 0 {
            res = res | (!x & !y & !z);
        }
        if (self.lut & (1 << 1)) != 0 {
            res = res | (!x & !y & z);
        }
        if (self.lut & (1 << 2)) != 0 {
            res = res | (!x & y & !z);
        }
        if (self.lut & (1 << 3)) != 0 {
            res = res | (!x & y & z);
        }
        if (self.lut & (1 << 4)) != 0 {
            res = res | (x & !y & !z);
        }
        if (self.lut & (1 << 5)) != 0 {
            res = res | (x & !y & z);
        }
        if (self.lut & (1 << 6)) != 0 {
            res = res | (x & y & !z);
        }
        if (self.lut & (1 << 7)) != 0 {
            res = res | (x & y & z);
        }
        res
    }
}

impl fmt::Display for LogicOp3 {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "LUT[{:#x}]", self.lut)
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum FloatType {
    F16,
    F32,
    F64,
}

impl FloatType {
    pub fn from_bits(bytes: usize) -> FloatType {
        match bytes {
            16 => FloatType::F16,
            32 => FloatType::F32,
            64 => FloatType::F64,
            _ => panic!("Invalid float type size"),
        }
    }

    pub fn bits(&self) -> usize {
        match self {
            FloatType::F16 => 16,
            FloatType::F32 => 32,
            FloatType::F64 => 64,
        }
    }
}

impl fmt::Display for FloatType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            FloatType::F16 => write!(f, ".f16"),
            FloatType::F32 => write!(f, ".f32"),
            FloatType::F64 => write!(f, ".f64"),
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum FRndMode {
    NearestEven,
    NegInf,
    PosInf,
    Zero,
}

impl fmt::Display for FRndMode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            FRndMode::NearestEven => write!(f, ".re"),
            FRndMode::NegInf => write!(f, ".rm"),
            FRndMode::PosInf => write!(f, ".rp"),
            FRndMode::Zero => write!(f, ".rz"),
        }
    }
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub enum TexDim {
    _1D,
    Array1D,
    _2D,
    Array2D,
    _3D,
    Cube,
    ArrayCube,
}

impl fmt::Display for TexDim {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            TexDim::_1D => write!(f, ".1d"),
            TexDim::Array1D => write!(f, ".a1d"),
            TexDim::_2D => write!(f, ".2d"),
            TexDim::Array2D => write!(f, ".a2d"),
            TexDim::_3D => write!(f, ".3d"),
            TexDim::Cube => write!(f, ".cube"),
            TexDim::ArrayCube => write!(f, ".acube"),
        }
    }
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub enum TexLodMode {
    Auto,
    Zero,
    Bias,
    Lod,
    Clamp,
    BiasClamp,
}

impl fmt::Display for TexLodMode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            TexLodMode::Auto => write!(f, "la"),
            TexLodMode::Zero => write!(f, "lz"),
            TexLodMode::Bias => write!(f, "lb"),
            TexLodMode::Lod => write!(f, "ll"),
            TexLodMode::Clamp => write!(f, "lc"),
            TexLodMode::BiasClamp => write!(f, "lb.lc"),
        }
    }
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub enum Tld4OffsetMode {
    None,
    AddOffI,
    PerPx,
}

impl fmt::Display for Tld4OffsetMode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Tld4OffsetMode::None => write!(f, "no_off"),
            Tld4OffsetMode::AddOffI => write!(f, "aoffi"),
            Tld4OffsetMode::PerPx => write!(f, "ptp"),
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq)]
pub enum TexQuery {
    Dimension,
    TextureType,
    SamplerPos,
}

impl fmt::Display for TexQuery {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            TexQuery::Dimension => write!(f, "dimension"),
            TexQuery::TextureType => write!(f, "texture_type"),
            TexQuery::SamplerPos => write!(f, "sampler_pos"),
        }
    }
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub enum ImageDim {
    _1D,
    _1DBuffer,
    _1DArray,
    _2D,
    _2DArray,
    _3D,
}

impl ImageDim {
    pub fn coord_comps(&self) -> u8 {
        match self {
            ImageDim::_1D => 1,
            ImageDim::_1DBuffer => 1,
            ImageDim::_1DArray => 2,
            ImageDim::_2D => 2,
            ImageDim::_2DArray => 3,
            ImageDim::_3D => 3,
        }
    }
}

impl fmt::Display for ImageDim {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ImageDim::_1D => write!(f, ".1d"),
            ImageDim::_1DBuffer => write!(f, ".buf"),
            ImageDim::_1DArray => write!(f, ".a1d"),
            ImageDim::_2D => write!(f, ".2d"),
            ImageDim::_2DArray => write!(f, ".a2d"),
            ImageDim::_3D => write!(f, ".3d"),
        }
    }
}

pub enum IntType {
    U8,
    I8,
    U16,
    I16,
    U32,
    I32,
    U64,
    I64,
}

impl IntType {
    pub fn from_bits(bits: usize, is_signed: bool) -> IntType {
        match bits {
            8 => {
                if is_signed {
                    IntType::I8
                } else {
                    IntType::U8
                }
            }
            16 => {
                if is_signed {
                    IntType::I16
                } else {
                    IntType::U16
                }
            }
            32 => {
                if is_signed {
                    IntType::I32
                } else {
                    IntType::U32
                }
            }
            64 => {
                if is_signed {
                    IntType::I64
                } else {
                    IntType::U64
                }
            }
            _ => panic!("Invalid integer type size"),
        }
    }

    pub fn is_signed(&self) -> bool {
        match self {
            IntType::U8 | IntType::U16 | IntType::U32 | IntType::U64 => false,
            IntType::I8 | IntType::I16 | IntType::I32 | IntType::I64 => true,
        }
    }

    pub fn bits(&self) -> usize {
        match self {
            IntType::U8 | IntType::I8 => 8,
            IntType::U16 | IntType::I16 => 16,
            IntType::U32 | IntType::I32 => 32,
            IntType::U64 | IntType::I64 => 64,
        }
    }
}

impl fmt::Display for IntType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            IntType::U8 => write!(f, ".u8"),
            IntType::I8 => write!(f, ".i8"),
            IntType::U16 => write!(f, ".u16"),
            IntType::I16 => write!(f, ".i16"),
            IntType::U32 => write!(f, ".u32"),
            IntType::I32 => write!(f, ".i32"),
            IntType::U64 => write!(f, ".u64"),
            IntType::I64 => write!(f, ".i64"),
        }
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum MemAddrType {
    A32,
    A64,
}

impl fmt::Display for MemAddrType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MemAddrType::A32 => write!(f, ".a32"),
            MemAddrType::A64 => write!(f, ".a64"),
        }
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum MemType {
    U8,
    I8,
    U16,
    I16,
    B32,
    B64,
    B128,
}

impl MemType {
    pub fn from_size(size: u8, is_signed: bool) -> MemType {
        match size {
            1 => {
                if is_signed {
                    MemType::I8
                } else {
                    MemType::U8
                }
            }
            2 => {
                if is_signed {
                    MemType::I16
                } else {
                    MemType::U16
                }
            }
            4 => MemType::B32,
            8 => MemType::B64,
            16 => MemType::B128,
            _ => panic!("Invalid memory load/store size"),
        }
    }
}

impl fmt::Display for MemType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MemType::U8 => write!(f, ".u8"),
            MemType::I8 => write!(f, ".i8"),
            MemType::U16 => write!(f, ".u16"),
            MemType::I16 => write!(f, ".i16"),
            MemType::B32 => write!(f, ".b32"),
            MemType::B64 => write!(f, ".b64"),
            MemType::B128 => write!(f, ".b128"),
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum MemOrder {
    Constant,
    Weak,
    Strong(MemScope),
}

impl fmt::Display for MemOrder {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MemOrder::Constant => write!(f, ".constant"),
            MemOrder::Weak => write!(f, ".weak"),
            MemOrder::Strong(scope) => write!(f, ".strong{}", scope),
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum MemScope {
    CTA,
    GPU,
    System,
}

impl fmt::Display for MemScope {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MemScope::CTA => write!(f, ".cta"),
            MemScope::GPU => write!(f, ".gpu"),
            MemScope::System => write!(f, ".sys"),
        }
    }
}

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum MemSpace {
    Global(MemAddrType),
    Local,
    Shared,
}

impl MemSpace {
    pub fn addr_type(&self) -> MemAddrType {
        match self {
            MemSpace::Global(t) => *t,
            MemSpace::Local => MemAddrType::A32,
            MemSpace::Shared => MemAddrType::A32,
        }
    }
}

impl fmt::Display for MemSpace {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MemSpace::Global(t) => write!(f, ".global{t}"),
            MemSpace::Local => write!(f, ".local"),
            MemSpace::Shared => write!(f, ".shared"),
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum MemEvictionPriority {
    First,
    Normal,
    Last,
    Unchanged,
}

impl fmt::Display for MemEvictionPriority {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MemEvictionPriority::First => write!(f, ".ef"),
            MemEvictionPriority::Normal => Ok(()),
            MemEvictionPriority::Last => write!(f, ".el"),
            MemEvictionPriority::Unchanged => write!(f, ".lu"),
        }
    }
}

#[derive(Clone)]
pub struct MemAccess {
    pub mem_type: MemType,
    pub space: MemSpace,
    pub order: MemOrder,
    pub eviction_priority: MemEvictionPriority,
}

impl fmt::Display for MemAccess {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{}{}{}{}",
            self.space, self.order, self.eviction_priority, self.mem_type,
        )
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum AtomType {
    F16x2,
    U32,
    I32,
    F32,
    U64,
    I64,
    F64,
}

impl AtomType {
    pub fn F(bits: u8) -> AtomType {
        match bits {
            16 => panic!("16-bit float atomics not yet supported"),
            32 => AtomType::F32,
            64 => AtomType::F64,
            _ => panic!("Invalid float atomic type"),
        }
    }

    pub fn U(bits: u8) -> AtomType {
        match bits {
            32 => AtomType::U32,
            64 => AtomType::U64,
            _ => panic!("Invalid uint atomic type"),
        }
    }

    pub fn I(bits: u8) -> AtomType {
        match bits {
            32 => AtomType::I32,
            64 => AtomType::I64,
            _ => panic!("Invalid int atomic type"),
        }
    }
}

impl fmt::Display for AtomType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            AtomType::F16x2 => write!(f, ".f16x2"),
            AtomType::U32 => write!(f, ".u32"),
            AtomType::I32 => write!(f, ".i32"),
            AtomType::F32 => write!(f, ".f32"),
            AtomType::U64 => write!(f, ".u64"),
            AtomType::I64 => write!(f, ".i64"),
            AtomType::F64 => write!(f, ".f64"),
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum AtomOp {
    Add,
    Min,
    Max,
    Inc,
    Dec,
    And,
    Or,
    Xor,
    Exch,
    CmpExch,
}

impl fmt::Display for AtomOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            AtomOp::Add => write!(f, ".add"),
            AtomOp::Min => write!(f, ".min"),
            AtomOp::Max => write!(f, ".max"),
            AtomOp::Inc => write!(f, ".inc"),
            AtomOp::Dec => write!(f, ".dec"),
            AtomOp::And => write!(f, ".and"),
            AtomOp::Or => write!(f, ".or"),
            AtomOp::Xor => write!(f, ".xor"),
            AtomOp::Exch => write!(f, ".exch"),
            AtomOp::CmpExch => write!(f, ".cmpexch"),
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq)]
pub enum InterpFreq {
    Pass,
    PassMulW,
    Constant,
    State,
}

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq)]
pub enum InterpLoc {
    Default,
    Centroid,
    Offset,
}

pub struct AttrAccess {
    pub addr: u16,
    pub comps: u8,
    pub patch: bool,
    pub output: bool,
    pub phys: bool,
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpFAdd {
    pub dst: Dst,

    #[src_type(F32)]
    pub srcs: [Src; 2],

    pub saturate: bool,
    pub rnd_mode: FRndMode,
    pub ftz: bool,
}

impl DisplayOp for OpFAdd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let sat = if self.saturate { ".sat" } else { "" };
        write!(f, "fadd{sat}")?;
        if self.rnd_mode != FRndMode::NearestEven {
            write!(f, "{}", self.rnd_mode)?;
        }
        if self.ftz {
            write!(f, ".ftz")?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1],)
    }
}
impl_display_for_op!(OpFAdd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpFFma {
    pub dst: Dst,

    #[src_type(F32)]
    pub srcs: [Src; 3],

    pub saturate: bool,
    pub rnd_mode: FRndMode,
    pub ftz: bool,
    pub dnz: bool,
}

impl DisplayOp for OpFFma {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let sat = if self.saturate { ".sat" } else { "" };
        write!(f, "ffma{sat}")?;
        if self.rnd_mode != FRndMode::NearestEven {
            write!(f, "{}", self.rnd_mode)?;
        }
        if self.dnz {
            write!(f, ".dnz")?;
        } else if self.ftz {
            write!(f, ".ftz")?;
        }
        write!(f, " {} {} {}", self.srcs[0], self.srcs[1], self.srcs[2])
    }
}
impl_display_for_op!(OpFFma);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpFMnMx {
    pub dst: Dst,

    #[src_type(F32)]
    pub srcs: [Src; 2],

    #[src_type(Pred)]
    pub min: Src,

    pub ftz: bool,
}

impl DisplayOp for OpFMnMx {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let ftz = if self.ftz { ".ftz" } else { "" };
        write!(
            f,
            "fmnmx{ftz} {} {} {}",
            self.srcs[0], self.srcs[1], self.min
        )
    }
}
impl_display_for_op!(OpFMnMx);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpFMul {
    pub dst: Dst,

    #[src_type(F32)]
    pub srcs: [Src; 2],

    pub saturate: bool,
    pub rnd_mode: FRndMode,
    pub ftz: bool,
    pub dnz: bool,
}

impl DisplayOp for OpFMul {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let sat = if self.saturate { ".sat" } else { "" };
        write!(f, "fmul{sat}")?;
        if self.rnd_mode != FRndMode::NearestEven {
            write!(f, "{}", self.rnd_mode)?;
        }
        if self.dnz {
            write!(f, ".dnz")?;
        } else if self.ftz {
            write!(f, ".ftz")?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1],)
    }
}
impl_display_for_op!(OpFMul);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpFSet {
    pub dst: Dst,
    pub cmp_op: FloatCmpOp,

    #[src_type(F32)]
    pub srcs: [Src; 2],

    pub ftz: bool,
}

impl DisplayOp for OpFSet {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let ftz = if self.ftz { ".ftz" } else { "" };
        write!(
            f,
            "fset{}{ftz} {} {}",
            self.cmp_op, self.srcs[0], self.srcs[1]
        )
    }
}
impl_display_for_op!(OpFSet);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpFSetP {
    pub dst: Dst,

    pub set_op: PredSetOp,
    pub cmp_op: FloatCmpOp,

    #[src_type(F32)]
    pub srcs: [Src; 2],

    #[src_type(Pred)]
    pub accum: Src,

    pub ftz: bool,
}

impl DisplayOp for OpFSetP {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let ftz = if self.ftz { ".ftz" } else { "" };
        write!(f, "fsetp{}{ftz}", self.cmp_op)?;
        if !self.set_op.is_trivial(&self.accum) {
            write!(f, "{}", self.set_op)?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1])?;
        if !self.set_op.is_trivial(&self.accum) {
            write!(f, " {}", self.accum)?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpFSetP);

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq)]
pub enum FSwzAddOp {
    Add,
    SubRight,
    SubLeft,
    MoveLeft,
}

impl fmt::Display for FSwzAddOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            FSwzAddOp::Add => write!(f, "add"),
            FSwzAddOp::SubRight => write!(f, "subr"),
            FSwzAddOp::SubLeft => write!(f, "sub"),
            FSwzAddOp::MoveLeft => write!(f, "mov2"),
        }
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpFSwzAdd {
    pub dst: Dst,

    #[src_type(GPR)]
    pub srcs: [Src; 2],

    pub rnd_mode: FRndMode,
    pub ftz: bool,

    pub ops: [FSwzAddOp; 4],
}

impl DisplayOp for OpFSwzAdd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "fswzadd",)?;
        if self.rnd_mode != FRndMode::NearestEven {
            write!(f, "{}", self.rnd_mode)?;
        }
        if self.ftz {
            write!(f, ".ftz")?;
        }
        write!(
            f,
            " {} {} [{}, {}, {}, {}]",
            self.srcs[0],
            self.srcs[1],
            self.ops[0],
            self.ops[1],
            self.ops[2],
            self.ops[3],
        )
    }
}
impl_display_for_op!(OpFSwzAdd);

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq)]
pub enum MuFuOp {
    Cos,
    Sin,
    Exp2,
    Log2,
    Rcp,
    Rsq,
    Rcp64H,
    Rsq64H,
    Sqrt,
    Tanh,
}

impl fmt::Display for MuFuOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            MuFuOp::Cos => write!(f, "cos"),
            MuFuOp::Sin => write!(f, "sin"),
            MuFuOp::Exp2 => write!(f, "exp2"),
            MuFuOp::Log2 => write!(f, "log2"),
            MuFuOp::Rcp => write!(f, "rcp"),
            MuFuOp::Rsq => write!(f, "rsq"),
            MuFuOp::Rcp64H => write!(f, "rcp64h"),
            MuFuOp::Rsq64H => write!(f, "rsq64h"),
            MuFuOp::Sqrt => write!(f, "sqrt"),
            MuFuOp::Tanh => write!(f, "tanh"),
        }
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpMuFu {
    pub dst: Dst,
    pub op: MuFuOp,

    #[src_type(F32)]
    pub src: Src,
}

impl DisplayOp for OpMuFu {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "mufu.{} {}", self.op, self.src)
    }
}
impl_display_for_op!(OpMuFu);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpDAdd {
    pub dst: Dst,

    #[src_type(F64)]
    pub srcs: [Src; 2],

    pub rnd_mode: FRndMode,
}

impl DisplayOp for OpDAdd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "dadd")?;
        if self.rnd_mode != FRndMode::NearestEven {
            write!(f, "{}", self.rnd_mode)?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1],)
    }
}
impl_display_for_op!(OpDAdd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpDMul {
    pub dst: Dst,

    #[src_type(F64)]
    pub srcs: [Src; 2],

    pub rnd_mode: FRndMode,
}

impl DisplayOp for OpDMul {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "dmul")?;
        if self.rnd_mode != FRndMode::NearestEven {
            write!(f, "{}", self.rnd_mode)?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1],)
    }
}
impl_display_for_op!(OpDMul);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpDFma {
    pub dst: Dst,

    #[src_type(F64)]
    pub srcs: [Src; 3],

    pub rnd_mode: FRndMode,
}

impl DisplayOp for OpDFma {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "dfma")?;
        if self.rnd_mode != FRndMode::NearestEven {
            write!(f, "{}", self.rnd_mode)?;
        }
        write!(f, " {} {} {}", self.srcs[0], self.srcs[1], self.srcs[2])
    }
}
impl_display_for_op!(OpDFma);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpDMnMx {
    pub dst: Dst,

    #[src_type(F64)]
    pub srcs: [Src; 2],

    #[src_type(Pred)]
    pub min: Src,
}

impl DisplayOp for OpDMnMx {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "dmnmx {} {} {}", self.srcs[0], self.srcs[1], self.min)
    }
}
impl_display_for_op!(OpDMnMx);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpDSetP {
    pub dst: Dst,

    pub set_op: PredSetOp,
    pub cmp_op: FloatCmpOp,

    #[src_type(F64)]
    pub srcs: [Src; 2],

    #[src_type(Pred)]
    pub accum: Src,
}

impl DisplayOp for OpDSetP {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "dsetp{}", self.cmp_op)?;
        if !self.set_op.is_trivial(&self.accum) {
            write!(f, "{}", self.set_op)?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1])?;
        if !self.set_op.is_trivial(&self.accum) {
            write!(f, " {}", self.accum)?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpDSetP);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpBrev {
    pub dst: Dst,

    #[src_type(ALU)]
    pub src: Src,
}

impl DisplayOp for OpBrev {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "brev {}", self.src,)
    }
}
impl_display_for_op!(OpBrev);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpFlo {
    pub dst: Dst,

    #[src_type(ALU)]
    pub src: Src,

    pub signed: bool,
    pub return_shift_amount: bool,
}

impl DisplayOp for OpFlo {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "flo")?;
        if self.return_shift_amount {
            write!(f, ".samt")?;
        }
        write!(f, " {}", self.src)
    }
}
impl_display_for_op!(OpFlo);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIAbs {
    pub dst: Dst,

    #[src_type(ALU)]
    pub src: Src,
}

impl DisplayOp for OpIAbs {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "iabs {}", self.src)
    }
}
impl_display_for_op!(OpIAbs);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpINeg {
    pub dst: Dst,

    #[src_type(ALU)]
    pub src: Src,
}

impl DisplayOp for OpINeg {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "ineg {}", self.src)
    }
}
impl_display_for_op!(OpINeg);

/// Only used on SM50
#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIAdd2 {
    pub dst: Dst,
    pub carry_out: Dst,

    #[src_type(ALU)]
    pub srcs: [Src; 2],
    pub carry_in: Src,
}

impl DisplayOp for OpIAdd2 {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "iadd2 {} {}", self.srcs[0], self.srcs[1])?;
        if !self.carry_in.is_zero() {
            write!(f, " {}", self.carry_in)?;
        }
        Ok(())
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIAdd3 {
    pub dst: Dst,
    pub overflow: [Dst; 2],

    #[src_type(I32)]
    pub srcs: [Src; 3],
}

impl DisplayOp for OpIAdd3 {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "iadd3 {} {} {}",
            self.srcs[0], self.srcs[1], self.srcs[2],
        )
    }
}
impl_display_for_op!(OpIAdd3);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIAdd3X {
    pub dst: Dst,
    pub overflow: [Dst; 2],

    #[src_type(B32)]
    pub srcs: [Src; 3],

    #[src_type(Pred)]
    pub carry: [Src; 2],
}

impl DisplayOp for OpIAdd3X {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "iadd3.x {} {} {} {} {}",
            self.srcs[0],
            self.srcs[1],
            self.srcs[2],
            self.carry[0],
            self.carry[1]
        )
    }
}
impl_display_for_op!(OpIAdd3X);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIDp4 {
    pub dst: Dst,

    pub src_types: [IntType; 2],

    #[src_type(I32)]
    pub srcs: [Src; 3],
}

impl DisplayOp for OpIDp4 {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "idp4{}{} {} {} {}",
            self.src_types[0],
            self.src_types[1],
            self.srcs[0],
            self.srcs[1],
            self.srcs[2],
        )
    }
}
impl_display_for_op!(OpIDp4);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIMad {
    pub dst: Dst,

    #[src_type(ALU)]
    pub srcs: [Src; 3],

    pub signed: bool,
}

impl DisplayOp for OpIMad {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "imad {} {} {}", self.srcs[0], self.srcs[1], self.srcs[2],)
    }
}
impl_display_for_op!(OpIMad);

/// Only used on SM50
#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIMul {
    pub dst: Dst,

    #[src_type(ALU)]
    pub srcs: [Src; 2],

    pub signed: [bool; 2],
    pub high: bool,
}

impl DisplayOp for OpIMul {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "imul")?;
        if self.high {
            write!(f, ".hi")?;
        }
        let src_type = |signed| if signed { ".s32" } else { ".u32" };
        write!(
            f,
            "{}{}",
            src_type(self.signed[0]),
            src_type(self.signed[1])
        )?;
        write!(f, " {} {}", self.srcs[0], self.srcs[1])
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIMad64 {
    pub dst: Dst,

    #[src_type(ALU)]
    pub srcs: [Src; 3],

    pub signed: bool,
}

impl DisplayOp for OpIMad64 {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "imad64 {} {} {}",
            self.srcs[0], self.srcs[1], self.srcs[2],
        )
    }
}
impl_display_for_op!(OpIMad64);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIMnMx {
    pub dst: Dst,
    pub cmp_type: IntCmpType,

    #[src_type(ALU)]
    pub srcs: [Src; 2],

    #[src_type(Pred)]
    pub min: Src,
}

impl DisplayOp for OpIMnMx {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "imnmx{} {} {} {}",
            self.cmp_type, self.srcs[0], self.srcs[1], self.min
        )
    }
}
impl_display_for_op!(OpIMnMx);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpISetP {
    pub dst: Dst,

    pub set_op: PredSetOp,
    pub cmp_op: IntCmpOp,
    pub cmp_type: IntCmpType,
    pub ex: bool,

    #[src_type(ALU)]
    pub srcs: [Src; 2],

    #[src_type(Pred)]
    pub accum: Src,

    #[src_type(Pred)]
    pub low_cmp: Src,
}

impl DisplayOp for OpISetP {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "isetp{}{}", self.cmp_op, self.cmp_type)?;
        if !self.set_op.is_trivial(&self.accum) {
            write!(f, "{}", self.set_op)?;
        }
        if self.ex {
            write!(f, ".ex")?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1])?;
        if !self.set_op.is_trivial(&self.accum) {
            write!(f, " {}", self.accum)?;
        }
        if self.ex {
            write!(f, " {}", self.low_cmp)?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpISetP);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpLop2 {
    pub dst: Dst,

    #[src_type(ALU)]
    pub srcs: [Src; 2],

    pub op: LogicOp2,
}

impl DisplayOp for OpLop2 {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "lop2.{} {} {}", self.op, self.srcs[0], self.srcs[1],)
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpLop3 {
    pub dst: Dst,

    #[src_type(ALU)]
    pub srcs: [Src; 3],

    pub op: LogicOp3,
}

impl DisplayOp for OpLop3 {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "lop3.{} {} {} {}",
            self.op, self.srcs[0], self.srcs[1], self.srcs[2],
        )
    }
}
impl_display_for_op!(OpLop3);

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, PartialEq)]
pub enum ShflOp {
    Idx,
    Up,
    Down,
    Bfly,
}

impl fmt::Display for ShflOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ShflOp::Idx => write!(f, "idx"),
            ShflOp::Up => write!(f, "up"),
            ShflOp::Down => write!(f, "down"),
            ShflOp::Bfly => write!(f, "bfly"),
        }
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpShf {
    pub dst: Dst,

    #[src_type(GPR)]
    pub low: Src,

    #[src_type(ALU)]
    pub high: Src,

    #[src_type(GPR)]
    pub shift: Src,

    pub right: bool,
    pub wrap: bool,
    pub data_type: IntType,
    pub dst_high: bool,
}

impl DisplayOp for OpShf {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "shf")?;
        if self.right {
            write!(f, ".r")?;
        } else {
            write!(f, ".l")?;
        }
        if self.wrap {
            write!(f, ".w")?;
        }
        write!(f, "{}", self.data_type)?;
        if self.dst_high {
            write!(f, ".hi")?;
        }
        write!(f, " {} {} {}", self.low, self.high, self.shift)
    }
}
impl_display_for_op!(OpShf);

/// Only used on SM50
#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpShl {
    pub dst: Dst,

    #[src_type(GPR)]
    pub src: Src,

    #[src_type(ALU)]
    pub shift: Src,

    pub wrap: bool,
}

impl DisplayOp for OpShl {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "shl")?;
        if self.wrap {
            write!(f, ".w")?;
        }
        write!(f, " {} {}", self.src, self.shift)
    }
}

/// Only used on SM50
#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpShr {
    pub dst: Dst,

    #[src_type(GPR)]
    pub src: Src,

    #[src_type(ALU)]
    pub shift: Src,

    pub wrap: bool,
    pub signed: bool,
}

impl DisplayOp for OpShr {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "shr")?;
        if self.wrap {
            write!(f, ".w")?;
        }
        if !self.signed {
            write!(f, ".u32")?;
        }
        write!(f, " {} {}", self.src, self.shift)
    }
}

#[repr(C)]
#[derive(DstsAsSlice)]
pub struct OpF2F {
    pub dst: Dst,

    pub src: Src,

    pub src_type: FloatType,
    pub dst_type: FloatType,
    pub rnd_mode: FRndMode,
    pub ftz: bool,
    /// Place the result into the upper 16 bits of the destination register
    pub high: bool,
}

impl SrcsAsSlice for OpF2F {
    fn srcs_as_slice(&self) -> &[Src] {
        std::slice::from_ref(&self.src)
    }

    fn srcs_as_mut_slice(&mut self) -> &mut [Src] {
        std::slice::from_mut(&mut self.src)
    }

    fn src_types(&self) -> SrcTypeList {
        let src_type = match self.src_type {
            FloatType::F16 => SrcType::ALU,
            FloatType::F32 => SrcType::F32,
            FloatType::F64 => SrcType::F64,
        };
        SrcTypeList::Uniform(src_type)
    }
}

impl DisplayOp for OpF2F {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "f2f")?;
        if self.ftz {
            write!(f, ".ftz")?;
        }
        write!(
            f,
            "{}{}{} {}",
            self.dst_type, self.src_type, self.rnd_mode, self.src,
        )
    }
}
impl_display_for_op!(OpF2F);

#[repr(C)]
#[derive(DstsAsSlice)]
pub struct OpF2I {
    pub dst: Dst,

    pub src: Src,

    pub src_type: FloatType,
    pub dst_type: IntType,
    pub rnd_mode: FRndMode,
    pub ftz: bool,
}

impl SrcsAsSlice for OpF2I {
    fn srcs_as_slice(&self) -> &[Src] {
        std::slice::from_ref(&self.src)
    }

    fn srcs_as_mut_slice(&mut self) -> &mut [Src] {
        std::slice::from_mut(&mut self.src)
    }

    fn src_types(&self) -> SrcTypeList {
        let src_type = match self.src_type {
            FloatType::F16 => SrcType::ALU,
            FloatType::F32 => SrcType::F32,
            FloatType::F64 => SrcType::F64,
        };
        SrcTypeList::Uniform(src_type)
    }
}

impl DisplayOp for OpF2I {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let ftz = if self.ftz { ".ftz" } else { "" };
        write!(
            f,
            "f2i{}{}{}{ftz} {}",
            self.dst_type, self.src_type, self.rnd_mode, self.src,
        )
    }
}
impl_display_for_op!(OpF2I);

#[repr(C)]
#[derive(DstsAsSlice)]
pub struct OpI2F {
    pub dst: Dst,

    pub src: Src,

    pub dst_type: FloatType,
    pub src_type: IntType,
    pub rnd_mode: FRndMode,
}

impl SrcsAsSlice for OpI2F {
    fn srcs_as_slice(&self) -> &[Src] {
        std::slice::from_ref(&self.src)
    }

    fn srcs_as_mut_slice(&mut self) -> &mut [Src] {
        std::slice::from_mut(&mut self.src)
    }

    fn src_types(&self) -> SrcTypeList {
        if self.src_type.bits() <= 32 {
            SrcTypeList::Uniform(SrcType::ALU)
        } else {
            SrcTypeList::Uniform(SrcType::GPR)
        }
    }
}

impl DisplayOp for OpI2F {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "i2f{}{}{} {}",
            self.dst_type, self.src_type, self.rnd_mode, self.src,
        )
    }
}
impl_display_for_op!(OpI2F);

#[repr(C)]
#[derive(DstsAsSlice)]
pub struct OpFRnd {
    pub dst: Dst,

    pub src: Src,

    pub dst_type: FloatType,
    pub src_type: FloatType,
    pub rnd_mode: FRndMode,
    pub ftz: bool,
}

impl SrcsAsSlice for OpFRnd {
    fn srcs_as_slice(&self) -> &[Src] {
        std::slice::from_ref(&self.src)
    }

    fn srcs_as_mut_slice(&mut self) -> &mut [Src] {
        std::slice::from_mut(&mut self.src)
    }

    fn src_types(&self) -> SrcTypeList {
        let src_type = match self.src_type {
            FloatType::F16 => SrcType::ALU,
            FloatType::F32 => SrcType::F32,
            FloatType::F64 => SrcType::F64,
        };
        SrcTypeList::Uniform(src_type)
    }
}

impl DisplayOp for OpFRnd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let ftz = if self.ftz { ".ftz" } else { "" };
        write!(
            f,
            "frnd{}{}{}{ftz} {}",
            self.dst_type, self.src_type, self.rnd_mode, self.src,
        )
    }
}
impl_display_for_op!(OpFRnd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpMov {
    pub dst: Dst,

    #[src_type(ALU)]
    pub src: Src,

    pub quad_lanes: u8,
}

impl DisplayOp for OpMov {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.quad_lanes == 0xf {
            write!(f, "mov {}", self.src)
        } else {
            write!(f, "mov[{:#x}] {}", self.quad_lanes, self.src)
        }
    }
}
impl_display_for_op!(OpMov);

#[allow(dead_code)]
#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum PrmtMode {
    Index,
    Forward4Extract,
    Backward4Extract,
    Replicate8,
    EdgeClampLeft,
    EdgeClampRight,
    Replicate16,
}

impl fmt::Display for PrmtMode {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            PrmtMode::Index => Ok(()),
            PrmtMode::Forward4Extract => write!(f, ".f4e"),
            PrmtMode::Backward4Extract => write!(f, ".b4e"),
            PrmtMode::Replicate8 => write!(f, ".rc8"),
            PrmtMode::EdgeClampLeft => write!(f, ".ecl"),
            PrmtMode::EdgeClampRight => write!(f, ".ecl"),
            PrmtMode::Replicate16 => write!(f, ".rc16"),
        }
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
/// Permutes `srcs` into `dst` using `selection`.
pub struct OpPrmt {
    pub dst: Dst,

    #[src_type(ALU)]
    pub srcs: [Src; 2],

    #[src_type(ALU)]
    pub sel: Src,

    pub mode: PrmtMode,
}

impl DisplayOp for OpPrmt {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "prmt{} {} [{}] {}",
            self.mode, self.srcs[0], self.sel, self.srcs[1],
        )
    }
}
impl_display_for_op!(OpPrmt);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpSel {
    pub dst: Dst,

    #[src_type(Pred)]
    pub cond: Src,

    #[src_type(ALU)]
    pub srcs: [Src; 2],
}

impl DisplayOp for OpSel {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "sel {} {} {}", self.cond, self.srcs[0], self.srcs[1],)
    }
}
impl_display_for_op!(OpSel);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpShfl {
    pub dst: Dst,
    pub in_bounds: Dst,

    #[src_type(SSA)]
    pub src: Src,

    #[src_type(ALU)]
    pub lane: Src,

    #[src_type(ALU)]
    pub c: Src,

    pub op: ShflOp,
}

impl DisplayOp for OpShfl {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "shfl.{} {} {} {}", self.op, self.src, self.lane, self.c)
    }
}
impl_display_for_op!(OpShfl);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpPLop3 {
    pub dsts: [Dst; 2],

    #[src_type(Pred)]
    pub srcs: [Src; 3],

    pub ops: [LogicOp3; 2],
}

impl DisplayOp for OpPLop3 {
    fn fmt_dsts(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {}", self.dsts[0], self.dsts[1])
    }

    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "plop3 {} {} {} {} {}",
            self.srcs[0], self.srcs[1], self.srcs[2], self.ops[0], self.ops[1],
        )
    }
}
impl_display_for_op!(OpPLop3);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpPSetP {
    pub dsts: [Dst; 2],

    pub ops: [PredSetOp; 2],

    #[src_type(Pred)]
    pub srcs: [Src; 3],
}

impl DisplayOp for OpPSetP {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "psetp{}{} {} {} {}",
            self.ops[0], self.ops[1], self.srcs[0], self.srcs[1], self.srcs[2],
        )
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpPopC {
    pub dst: Dst,

    #[src_type(ALU)]
    pub src: Src,
}

impl DisplayOp for OpPopC {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "popc {}", self.src,)
    }
}
impl_display_for_op!(OpPopC);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpTex {
    pub dsts: [Dst; 2],
    pub resident: Dst,

    #[src_type(SSA)]
    pub srcs: [Src; 2],

    pub dim: TexDim,
    pub lod_mode: TexLodMode,
    pub z_cmpr: bool,
    pub offset: bool,
    pub mask: u8,
}

impl DisplayOp for OpTex {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "tex.b{}", self.dim)?;
        if self.lod_mode != TexLodMode::Auto {
            write!(f, ".{}", self.lod_mode)?;
        }
        if self.offset {
            write!(f, ".aoffi")?;
        }
        if self.z_cmpr {
            write!(f, ".dc")?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1])
    }
}
impl_display_for_op!(OpTex);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpTld {
    pub dsts: [Dst; 2],
    pub resident: Dst,

    #[src_type(SSA)]
    pub srcs: [Src; 2],

    pub dim: TexDim,
    pub is_ms: bool,
    pub lod_mode: TexLodMode,
    pub offset: bool,
    pub mask: u8,
}

impl DisplayOp for OpTld {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "tld.b{}", self.dim)?;
        if self.lod_mode != TexLodMode::Auto {
            write!(f, ".{}", self.lod_mode)?;
        }
        if self.offset {
            write!(f, ".aoffi")?;
        }
        if self.is_ms {
            write!(f, ".ms")?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1])
    }
}
impl_display_for_op!(OpTld);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpTld4 {
    pub dsts: [Dst; 2],
    pub resident: Dst,

    #[src_type(SSA)]
    pub srcs: [Src; 2],

    pub dim: TexDim,
    pub comp: u8,
    pub offset_mode: Tld4OffsetMode,
    pub z_cmpr: bool,
    pub mask: u8,
}

impl DisplayOp for OpTld4 {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "tld4.g.b{}", self.dim)?;
        if self.offset_mode != Tld4OffsetMode::None {
            write!(f, ".{}", self.offset_mode)?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1])
    }
}
impl_display_for_op!(OpTld4);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpTmml {
    pub dsts: [Dst; 2],

    #[src_type(SSA)]
    pub srcs: [Src; 2],

    pub dim: TexDim,
    pub mask: u8,
}

impl DisplayOp for OpTmml {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "tmml.b.lod{} {} {}",
            self.dim, self.srcs[0], self.srcs[1]
        )
    }
}
impl_display_for_op!(OpTmml);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpTxd {
    pub dsts: [Dst; 2],
    pub resident: Dst,

    #[src_type(SSA)]
    pub srcs: [Src; 2],

    pub dim: TexDim,
    pub offset: bool,
    pub mask: u8,
}

impl DisplayOp for OpTxd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "txd.b{}", self.dim)?;
        if self.offset {
            write!(f, ".aoffi")?;
        }
        write!(f, " {} {}", self.srcs[0], self.srcs[1])
    }
}
impl_display_for_op!(OpTxd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpTxq {
    pub dsts: [Dst; 2],

    #[src_type(SSA)]
    pub src: Src,

    pub query: TexQuery,
    pub mask: u8,
}

impl DisplayOp for OpTxq {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "txq.b {} {}", self.src, self.query)
    }
}
impl_display_for_op!(OpTxq);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpSuLd {
    pub dst: Dst,
    pub resident: Dst,

    pub image_dim: ImageDim,
    pub mem_order: MemOrder,
    pub mem_eviction_priority: MemEvictionPriority,
    pub mask: u8,

    #[src_type(GPR)]
    pub handle: Src,

    #[src_type(SSA)]
    pub coord: Src,
}

impl DisplayOp for OpSuLd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "suld.p{}{}{} [{}] {}",
            self.image_dim,
            self.mem_order,
            self.mem_eviction_priority,
            self.coord,
            self.handle,
        )
    }
}
impl_display_for_op!(OpSuLd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpSuSt {
    pub image_dim: ImageDim,
    pub mem_order: MemOrder,
    pub mem_eviction_priority: MemEvictionPriority,
    pub mask: u8,

    #[src_type(GPR)]
    pub handle: Src,

    #[src_type(SSA)]
    pub coord: Src,

    #[src_type(SSA)]
    pub data: Src,
}

impl DisplayOp for OpSuSt {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "sust.p{}{}{} [{}] {} {}",
            self.image_dim,
            self.mem_order,
            self.mem_eviction_priority,
            self.coord,
            self.data,
            self.handle,
        )
    }
}
impl_display_for_op!(OpSuSt);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpSuAtom {
    pub dst: Dst,
    pub resident: Dst,

    pub image_dim: ImageDim,

    pub atom_op: AtomOp,
    pub atom_type: AtomType,

    pub mem_order: MemOrder,
    pub mem_eviction_priority: MemEvictionPriority,

    #[src_type(GPR)]
    pub handle: Src,

    #[src_type(SSA)]
    pub coord: Src,

    #[src_type(SSA)]
    pub data: Src,
}

impl DisplayOp for OpSuAtom {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "suatom.p{}{}{}{}{} [{}] {} {}",
            self.image_dim,
            self.atom_op,
            self.atom_type,
            self.mem_order,
            self.mem_eviction_priority,
            self.coord,
            self.data,
            self.handle,
        )
    }
}
impl_display_for_op!(OpSuAtom);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpLd {
    pub dst: Dst,

    #[src_type(GPR)]
    pub addr: Src,

    pub offset: i32,
    pub access: MemAccess,
}

impl DisplayOp for OpLd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "ld{} [{}", self.access, self.addr)?;
        if self.offset > 0 {
            write!(f, "+{:#x}", self.offset)?;
        }
        write!(f, "]")
    }
}
impl_display_for_op!(OpLd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpLdc {
    pub dst: Dst,

    #[src_type(ALU)]
    pub cb: Src,

    #[src_type(GPR)]
    pub offset: Src,

    pub mem_type: MemType,
}

impl DisplayOp for OpLdc {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let SrcRef::CBuf(cb) = self.cb.src_ref else {
            panic!("Not a cbuf");
        };
        write!(f, "ldc{} {}[", self.mem_type, cb.buf)?;
        if self.offset.is_zero() {
            write!(f, "+{:#x}", cb.offset)?;
        } else if cb.offset == 0 {
            write!(f, "{}", self.offset)?;
        } else {
            write!(f, "{}+{:#x}", self.offset, cb.offset)?;
        }
        write!(f, "]")
    }
}
impl_display_for_op!(OpLdc);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpSt {
    #[src_type(GPR)]
    pub addr: Src,

    #[src_type(SSA)]
    pub data: Src,

    pub offset: i32,
    pub access: MemAccess,
}

impl DisplayOp for OpSt {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "st{} [{}", self.access, self.addr)?;
        if self.offset > 0 {
            write!(f, "+{:#x}", self.offset)?;
        }
        write!(f, "] {}", self.data)
    }
}
impl_display_for_op!(OpSt);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpAtom {
    pub dst: Dst,

    #[src_type(GPR)]
    pub addr: Src,

    #[src_type(GPR)]
    pub cmpr: Src,

    #[src_type(SSA)]
    pub data: Src,

    pub atom_op: AtomOp,
    pub atom_type: AtomType,

    pub addr_offset: i32,

    pub mem_space: MemSpace,
    pub mem_order: MemOrder,
    pub mem_eviction_priority: MemEvictionPriority,
}

impl DisplayOp for OpAtom {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "atom{}{}{}{}{}",
            self.atom_op,
            self.atom_type,
            self.mem_space,
            self.mem_order,
            self.mem_eviction_priority,
        )?;
        write!(f, " [")?;
        if !self.addr.is_zero() {
            write!(f, "{}", self.addr)?;
        }
        if self.addr_offset > 0 {
            if !self.addr.is_zero() {
                write!(f, "+")?;
            }
            write!(f, "{:#x}", self.addr_offset)?;
        }
        write!(f, "] {}", self.data)
    }
}
impl_display_for_op!(OpAtom);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpAL2P {
    pub dst: Dst,

    #[src_type(GPR)]
    pub offset: Src,

    pub access: AttrAccess,
}

impl DisplayOp for OpAL2P {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "al2p")?;
        if self.access.output {
            write!(f, ".o")?;
        }
        if self.access.patch {
            write!(f, ".p")?;
        }
        write!(f, " a[{:#x}", self.access.addr)?;
        if !self.offset.is_zero() {
            write!(f, "+{}", self.offset)?;
        }
        write!(f, "]")
    }
}
impl_display_for_op!(OpAL2P);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpALd {
    pub dst: Dst,

    #[src_type(GPR)]
    pub vtx: Src,

    #[src_type(GPR)]
    pub offset: Src,

    pub access: AttrAccess,
}

impl DisplayOp for OpALd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "ald")?;
        if self.access.output {
            write!(f, ".o")?;
        }
        if self.access.patch {
            write!(f, ".p")?;
        }
        if self.access.phys {
            write!(f, ".phys")?;
        }
        write!(f, " a")?;
        if !self.vtx.is_zero() {
            write!(f, "[{}]", self.vtx)?;
        }
        write!(f, "[{:#x}", self.access.addr)?;
        if !self.offset.is_zero() {
            write!(f, "+{}", self.offset)?;
        }
        write!(f, "]")
    }
}
impl_display_for_op!(OpALd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpASt {
    #[src_type(GPR)]
    pub vtx: Src,

    #[src_type(GPR)]
    pub offset: Src,

    #[src_type(SSA)]
    pub data: Src,

    pub access: AttrAccess,
}

impl DisplayOp for OpASt {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "ast")?;
        if self.access.patch {
            write!(f, ".p")?;
        }
        if self.access.phys {
            write!(f, ".phys")?;
        }
        write!(f, " a")?;
        if !self.vtx.is_zero() {
            write!(f, "[{}]", self.vtx)?;
        }
        write!(f, "[{:#x}", self.access.addr)?;
        if !self.offset.is_zero() {
            write!(f, "+{}", self.offset)?;
        }
        write!(f, "] {}", self.data)
    }
}
impl_display_for_op!(OpASt);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIpa {
    pub dst: Dst,
    pub addr: u16,
    pub freq: InterpFreq,
    pub loc: InterpLoc,
    pub inv_w: Src,
    pub offset: Src,
}

impl DisplayOp for OpIpa {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "ipa")?;
        match self.freq {
            InterpFreq::Pass => write!(f, ".pass")?,
            InterpFreq::PassMulW => write!(f, ".pass_mul_w")?,
            InterpFreq::Constant => write!(f, ".constant")?,
            InterpFreq::State => write!(f, ".state")?,
        }
        match self.loc {
            InterpLoc::Default => (),
            InterpLoc::Centroid => write!(f, ".centroid")?,
            InterpLoc::Offset => write!(f, ".offset")?,
        }

        write!(f, " {} a[{:#x}] {}", self.dst, self.addr, self.inv_w)?;
        if self.loc == InterpLoc::Offset {
            write!(f, " {}", self.offset)?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpIpa);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpLdTram {
    pub dst: Dst,
    pub addr: u16,
    pub use_c: bool,
}

impl DisplayOp for OpLdTram {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "ldtram")?;
        if self.use_c {
            write!(f, ".c")?;
        } else {
            write!(f, ".ab")?;
        }
        write!(f, " a[{:#x}]", self.addr)?;
        Ok(())
    }
}
impl_display_for_op!(OpLdTram);

#[allow(dead_code)]
pub enum CCtlOp {
    PF1,
    PF2,
    WB,
    IV,
    IVAll,
    RS,
    IVAllP,
    WBAll,
    WBAllP,
}

impl CCtlOp {
    pub fn is_all(&self) -> bool {
        match self {
            CCtlOp::PF1
            | CCtlOp::PF2
            | CCtlOp::WB
            | CCtlOp::IV
            | CCtlOp::RS => false,
            CCtlOp::IVAll | CCtlOp::IVAllP | CCtlOp::WBAll | CCtlOp::WBAllP => {
                true
            }
        }
    }
}

impl fmt::Display for CCtlOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            CCtlOp::PF1 => write!(f, "pf1"),
            CCtlOp::PF2 => write!(f, "pf2"),
            CCtlOp::WB => write!(f, "wb"),
            CCtlOp::IV => write!(f, "iv"),
            CCtlOp::IVAll => write!(f, "ivall"),
            CCtlOp::RS => write!(f, "rs"),
            CCtlOp::IVAllP => write!(f, "ivallp"),
            CCtlOp::WBAll => write!(f, "wball"),
            CCtlOp::WBAllP => write!(f, "wballp"),
        }
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpCCtl {
    pub op: CCtlOp,

    pub mem_space: MemSpace,

    #[src_type(GPR)]
    pub addr: Src,

    pub addr_offset: i32,
}

impl DisplayOp for OpCCtl {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "cctl{}", self.mem_space)?;
        if !self.op.is_all() {
            write!(f, " [{}", self.addr)?;
            if self.addr_offset > 0 {
                write!(f, "+{:#x}", self.addr_offset)?;
            }
            write!(f, "]")?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpCCtl);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpMemBar {
    pub scope: MemScope,
}

impl DisplayOp for OpMemBar {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "membar.sc.{}", self.scope)
    }
}
impl_display_for_op!(OpMemBar);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpBClear {
    pub dst: Dst,
}

impl DisplayOp for OpBClear {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "bclear")
    }
}
impl_display_for_op!(OpBClear);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpBMov {
    pub dst: Dst,
    pub src: Src,
    pub clear: bool,
}

impl DisplayOp for OpBMov {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "bmov.32")?;
        if self.clear {
            write!(f, ".clear")?;
        }
        write!(f, " {}", self.src)
    }
}
impl_display_for_op!(OpBMov);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpBreak {
    pub bar_out: Dst,

    #[src_type(Bar)]
    pub bar_in: Src,

    #[src_type(Pred)]
    pub cond: Src,
}

impl DisplayOp for OpBreak {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "break {} {}", self.bar_in, self.cond)
    }
}
impl_display_for_op!(OpBreak);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpBSSy {
    pub bar_out: Dst,

    #[src_type(Pred)]
    pub bar_in: Src,

    #[src_type(Pred)]
    pub cond: Src,

    pub target: Label,
}

impl DisplayOp for OpBSSy {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "bssy {} {} {}", self.bar_in, self.cond, self.target)
    }
}
impl_display_for_op!(OpBSSy);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpBSync {
    #[src_type(Bar)]
    pub bar: Src,

    #[src_type(Pred)]
    pub cond: Src,
}

impl DisplayOp for OpBSync {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "bsync {} {}", self.bar, self.cond)
    }
}
impl_display_for_op!(OpBSync);

#[repr(C)]
#[derive(Clone, SrcsAsSlice, DstsAsSlice)]
pub struct OpBra {
    pub target: Label,
}

impl DisplayOp for OpBra {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "bra {}", self.target)
    }
}
impl_display_for_op!(OpBra);

#[repr(C)]
#[derive(Clone, SrcsAsSlice, DstsAsSlice)]
pub struct OpExit {}

impl DisplayOp for OpExit {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "exit")
    }
}
impl_display_for_op!(OpExit);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpWarpSync {
    pub mask: u32,
}

impl DisplayOp for OpWarpSync {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "warpsync 0x{:x}", self.mask)
    }
}
impl_display_for_op!(OpWarpSync);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpBar {}

impl DisplayOp for OpBar {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "bar.sync")
    }
}
impl_display_for_op!(OpBar);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpCS2R {
    pub dst: Dst,
    pub idx: u8,
}

impl DisplayOp for OpCS2R {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "cs2r sr[{:#x}]", self.idx)
    }
}
impl_display_for_op!(OpCS2R);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpIsberd {
    pub dst: Dst,

    #[src_type(SSA)]
    pub idx: Src,
}

impl DisplayOp for OpIsberd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "isberd {} [{}]", self.dst, self.idx)
    }
}
impl_display_for_op!(OpIsberd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpKill {}

impl DisplayOp for OpKill {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "kill")
    }
}
impl_display_for_op!(OpKill);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpNop {
    pub label: Option<Label>,
}

impl DisplayOp for OpNop {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "nop")?;
        if let Some(label) = &self.label {
            write!(f, " {}", label)?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpNop);

#[allow(dead_code)]
pub enum PixVal {
    MsCount,
    CovMask,
    CentroidOffset,
    MyIndex,
    InnerCoverage,
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpPixLd {
    pub dst: Dst,
    pub val: PixVal,
}

impl DisplayOp for OpPixLd {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "pixld")?;
        match self.val {
            PixVal::MsCount => write!(f, ".mscount"),
            PixVal::CovMask => write!(f, ".covmask"),
            PixVal::CentroidOffset => write!(f, ".centroid_offset"),
            PixVal::MyIndex => write!(f, ".my_index"),
            PixVal::InnerCoverage => write!(f, ".inner_coverage"),
        }
    }
}
impl_display_for_op!(OpPixLd);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpS2R {
    pub dst: Dst,
    pub idx: u8,
}

impl DisplayOp for OpS2R {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "s2r sr[{:#x}]", self.idx)
    }
}
impl_display_for_op!(OpS2R);

pub enum VoteOp {
    Any,
    All,
    Eq,
}

impl fmt::Display for VoteOp {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            VoteOp::Any => write!(f, "any"),
            VoteOp::All => write!(f, "all"),
            VoteOp::Eq => write!(f, "eq"),
        }
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpVote {
    pub op: VoteOp,

    pub ballot: Dst,
    pub vote: Dst,

    #[src_type(Pred)]
    pub pred: Src,
}

impl DisplayOp for OpVote {
    fn fmt_dsts(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.ballot.is_none() && self.vote.is_none() {
            write!(f, "none")
        } else {
            if !self.ballot.is_none() {
                write!(f, "{}", self.ballot)?;
            }
            if !self.vote.is_none() {
                write!(f, "{}", self.vote)?;
            }
            Ok(())
        }
    }

    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "vote.{} {}", self.op, self.pred)
    }
}
impl_display_for_op!(OpVote);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpUndef {
    pub dst: Dst,
}

impl DisplayOp for OpUndef {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "undef {}", self.dst)
    }
}
impl_display_for_op!(OpUndef);

pub struct VecPair<A, B> {
    a: Vec<A>,
    b: Vec<B>,
}

impl<A, B> VecPair<A, B> {
    pub fn append(&mut self, other: &mut VecPair<A, B>) {
        self.a.append(&mut other.a);
        self.b.append(&mut other.b);
    }

    pub fn is_empty(&self) -> bool {
        debug_assert!(self.a.len() == self.b.len());
        self.a.is_empty()
    }

    pub fn iter(&self) -> Zip<slice::Iter<'_, A>, slice::Iter<'_, B>> {
        debug_assert!(self.a.len() == self.b.len());
        self.a.iter().zip(self.b.iter())
    }

    pub fn iter_mut(
        &mut self,
    ) -> Zip<slice::IterMut<'_, A>, slice::IterMut<'_, B>> {
        debug_assert!(self.a.len() == self.b.len());
        self.a.iter_mut().zip(self.b.iter_mut())
    }

    pub fn len(&self) -> usize {
        debug_assert!(self.a.len() == self.b.len());
        self.a.len()
    }

    pub fn new() -> Self {
        Self {
            a: Vec::new(),
            b: Vec::new(),
        }
    }

    pub fn push(&mut self, a: A, b: B) {
        debug_assert!(self.a.len() == self.b.len());
        self.a.push(a);
        self.b.push(b);
    }
}

impl<A: Clone, B: Clone> VecPair<A, B> {
    pub fn retain(&mut self, mut f: impl FnMut(&A, &B) -> bool) {
        debug_assert!(self.a.len() == self.b.len());
        let len = self.a.len();
        let mut i = 0_usize;
        while i < len {
            if !f(&self.a[i], &self.b[i]) {
                break;
            }
            i += 1;
        }

        let mut new_len = i;

        // Don't check this one twice.
        i += 1;

        while i < len {
            // This could be more efficient but it's good enough for our
            // purposes since everything we're storing is small and has a
            // trivial Drop.
            if f(&self.a[i], &self.b[i]) {
                self.a[new_len] = self.a[i].clone();
                self.b[new_len] = self.b[i].clone();
                new_len += 1;
            }
            i += 1;
        }

        if new_len < len {
            self.a.truncate(new_len);
            self.b.truncate(new_len);
        }
    }
}

pub struct PhiAllocator {
    count: u32,
}

impl PhiAllocator {
    pub fn new() -> PhiAllocator {
        PhiAllocator { count: 0 }
    }

    pub fn alloc(&mut self) -> u32 {
        let idx = self.count;
        self.count = idx + 1;
        idx
    }
}

#[repr(C)]
#[derive(DstsAsSlice)]
pub struct OpPhiSrcs {
    pub srcs: VecPair<u32, Src>,
}

impl OpPhiSrcs {
    pub fn new() -> OpPhiSrcs {
        OpPhiSrcs {
            srcs: VecPair::new(),
        }
    }
}

impl SrcsAsSlice for OpPhiSrcs {
    fn srcs_as_slice(&self) -> &[Src] {
        &self.srcs.b
    }

    fn srcs_as_mut_slice(&mut self) -> &mut [Src] {
        &mut self.srcs.b
    }

    fn src_types(&self) -> SrcTypeList {
        SrcTypeList::Uniform(SrcType::GPR)
    }
}

impl DisplayOp for OpPhiSrcs {
    fn fmt_dsts(&self, _f: &mut fmt::Formatter<'_>) -> fmt::Result {
        Ok(())
    }

    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "phi_src ")?;
        for (i, (id, src)) in self.srcs.iter().enumerate() {
            if i > 0 {
                write!(f, ", ")?;
            }
            write!(f, "φ{} = {}", id, src)?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpPhiSrcs);

#[repr(C)]
#[derive(SrcsAsSlice)]
pub struct OpPhiDsts {
    pub dsts: VecPair<u32, Dst>,
}

impl OpPhiDsts {
    pub fn new() -> OpPhiDsts {
        OpPhiDsts {
            dsts: VecPair::new(),
        }
    }
}

impl DstsAsSlice for OpPhiDsts {
    fn dsts_as_slice(&self) -> &[Dst] {
        &self.dsts.b
    }

    fn dsts_as_mut_slice(&mut self) -> &mut [Dst] {
        &mut self.dsts.b
    }
}

impl DisplayOp for OpPhiDsts {
    fn fmt_dsts(&self, _f: &mut fmt::Formatter<'_>) -> fmt::Result {
        Ok(())
    }

    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "phi_dst ")?;
        for (i, (id, dst)) in self.dsts.iter().enumerate() {
            if i > 0 {
                write!(f, ", ")?;
            }
            write!(f, "{} = φ{}", dst, id)?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpPhiDsts);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpCopy {
    pub dst: Dst,
    pub src: Src,
}

impl DisplayOp for OpCopy {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "copy {}", self.src)
    }
}
impl_display_for_op!(OpCopy);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpSwap {
    pub dsts: [Dst; 2],
    pub srcs: [Src; 2],
}

impl DisplayOp for OpSwap {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "swap {} {}", self.srcs[0], self.srcs[1])
    }
}
impl_display_for_op!(OpSwap);

#[repr(C)]
pub struct OpParCopy {
    pub dsts_srcs: VecPair<Dst, Src>,
    pub tmp: Option<RegRef>,
}

impl OpParCopy {
    pub fn new() -> OpParCopy {
        OpParCopy {
            dsts_srcs: VecPair::new(),
            tmp: None,
        }
    }

    pub fn is_empty(&self) -> bool {
        self.dsts_srcs.is_empty()
    }

    pub fn push(&mut self, dst: Dst, src: Src) {
        self.dsts_srcs.push(dst, src);
    }
}

impl SrcsAsSlice for OpParCopy {
    fn srcs_as_slice(&self) -> &[Src] {
        &self.dsts_srcs.b
    }

    fn srcs_as_mut_slice(&mut self) -> &mut [Src] {
        &mut self.dsts_srcs.b
    }

    fn src_types(&self) -> SrcTypeList {
        SrcTypeList::Uniform(SrcType::GPR)
    }
}

impl DstsAsSlice for OpParCopy {
    fn dsts_as_slice(&self) -> &[Dst] {
        &self.dsts_srcs.a
    }

    fn dsts_as_mut_slice(&mut self) -> &mut [Dst] {
        &mut self.dsts_srcs.a
    }
}

impl DisplayOp for OpParCopy {
    fn fmt_dsts(&self, _f: &mut fmt::Formatter<'_>) -> fmt::Result {
        Ok(())
    }

    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "par_copy")?;
        for (i, (dst, src)) in self.dsts_srcs.iter().enumerate() {
            if i > 0 {
                write!(f, ",")?;
            }
            write!(f, " {} = {}", dst, src)?;
        }
        Ok(())
    }
}
impl_display_for_op!(OpParCopy);

#[repr(C)]
#[derive(DstsAsSlice)]
pub struct OpFSOut {
    pub srcs: Vec<Src>,
}

impl SrcsAsSlice for OpFSOut {
    fn srcs_as_slice(&self) -> &[Src] {
        &self.srcs
    }

    fn srcs_as_mut_slice(&mut self) -> &mut [Src] {
        &mut self.srcs
    }

    fn src_types(&self) -> SrcTypeList {
        SrcTypeList::Uniform(SrcType::GPR)
    }
}

impl DisplayOp for OpFSOut {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "fs_out {{")?;
        for (i, src) in self.srcs.iter().enumerate() {
            if i > 0 {
                write!(f, ",")?;
            }
            write!(f, " {}", src)?;
        }
        write!(f, " }}")
    }
}
impl_display_for_op!(OpFSOut);

#[derive(Copy, Clone, Debug, PartialEq)]
pub enum OutType {
    Emit,
    Cut,
    EmitThenCut,
}

impl fmt::Display for OutType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            OutType::Emit => write!(f, "emit"),
            OutType::Cut => write!(f, "cut"),
            OutType::EmitThenCut => write!(f, "emit_then_cut"),
        }
    }
}

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpOut {
    pub dst: Dst,

    #[src_type(SSA)]
    pub handle: Src,

    #[src_type(ALU)]
    pub stream: Src,

    pub out_type: OutType,
}

impl DisplayOp for OpOut {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "out.{} {} {}", self.out_type, self.handle, self.stream)
    }
}
impl_display_for_op!(OpOut);

#[repr(C)]
#[derive(SrcsAsSlice, DstsAsSlice)]
pub struct OpOutFinal {
    #[src_type(SSA)]
    pub handle: Src,
}

impl DisplayOp for OpOutFinal {
    fn fmt_op(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "out.final {{ {} }}", self.handle)
    }
}
impl_display_for_op!(OpOutFinal);

#[derive(DisplayOp, DstsAsSlice, SrcsAsSlice, FromVariants)]
pub enum Op {
    FAdd(OpFAdd),
    FFma(OpFFma),
    FMnMx(OpFMnMx),
    FMul(OpFMul),
    MuFu(OpMuFu),
    FSet(OpFSet),
    FSetP(OpFSetP),
    FSwzAdd(OpFSwzAdd),
    DAdd(OpDAdd),
    DFma(OpDFma),
    DMnMx(OpDMnMx),
    DMul(OpDMul),
    DSetP(OpDSetP),
    Brev(OpBrev),
    Flo(OpFlo),
    IAbs(OpIAbs),
    INeg(OpINeg),
    IAdd2(OpIAdd2),
    IAdd3(OpIAdd3),
    IAdd3X(OpIAdd3X),
    IDp4(OpIDp4),
    IMad(OpIMad),
    IMad64(OpIMad64),
    IMul(OpIMul),
    IMnMx(OpIMnMx),
    ISetP(OpISetP),
    Lop2(OpLop2),
    Lop3(OpLop3),
    PopC(OpPopC),
    Shf(OpShf),
    Shl(OpShl),
    Shr(OpShr),
    F2F(OpF2F),
    F2I(OpF2I),
    I2F(OpI2F),
    FRnd(OpFRnd),
    Mov(OpMov),
    Prmt(OpPrmt),
    Sel(OpSel),
    Shfl(OpShfl),
    PLop3(OpPLop3),
    PSetP(OpPSetP),
    Tex(OpTex),
    Tld(OpTld),
    Tld4(OpTld4),
    Tmml(OpTmml),
    Txd(OpTxd),
    Txq(OpTxq),
    SuLd(OpSuLd),
    SuSt(OpSuSt),
    SuAtom(OpSuAtom),
    Ld(OpLd),
    Ldc(OpLdc),
    St(OpSt),
    Atom(OpAtom),
    AL2P(OpAL2P),
    ALd(OpALd),
    ASt(OpASt),
    Ipa(OpIpa),
    LdTram(OpLdTram),
    CCtl(OpCCtl),
    MemBar(OpMemBar),
    BClear(OpBClear),
    BMov(OpBMov),
    Break(OpBreak),
    BSSy(OpBSSy),
    BSync(OpBSync),
    Bra(OpBra),
    Exit(OpExit),
    WarpSync(OpWarpSync),
    Bar(OpBar),
    CS2R(OpCS2R),
    Isberd(OpIsberd),
    Kill(OpKill),
    Nop(OpNop),
    PixLd(OpPixLd),
    S2R(OpS2R),
    Vote(OpVote),
    Undef(OpUndef),
    PhiSrcs(OpPhiSrcs),
    PhiDsts(OpPhiDsts),
    Copy(OpCopy),
    Swap(OpSwap),
    ParCopy(OpParCopy),
    FSOut(OpFSOut),
    Out(OpOut),
    OutFinal(OpOutFinal),
}
impl_display_for_op!(Op);

#[derive(Clone, Copy, Eq, Hash, PartialEq)]
pub enum PredRef {
    None,
    SSA(SSAValue),
    Reg(RegRef),
}

impl PredRef {
    #[allow(dead_code)]
    pub fn as_reg(&self) -> Option<&RegRef> {
        match self {
            PredRef::Reg(r) => Some(r),
            _ => None,
        }
    }

    #[allow(dead_code)]
    pub fn as_ssa(&self) -> Option<&SSAValue> {
        match self {
            PredRef::SSA(r) => Some(r),
            _ => None,
        }
    }

    pub fn is_none(&self) -> bool {
        match self {
            PredRef::None => true,
            _ => false,
        }
    }

    pub fn iter_ssa(&self) -> slice::Iter<'_, SSAValue> {
        match self {
            PredRef::None | PredRef::Reg(_) => &[],
            PredRef::SSA(ssa) => slice::from_ref(ssa),
        }
        .iter()
    }

    pub fn iter_ssa_mut(&mut self) -> slice::IterMut<'_, SSAValue> {
        match self {
            PredRef::None | PredRef::Reg(_) => &mut [],
            PredRef::SSA(ssa) => slice::from_mut(ssa),
        }
        .iter_mut()
    }
}

impl From<RegRef> for PredRef {
    fn from(reg: RegRef) -> PredRef {
        PredRef::Reg(reg)
    }
}

impl From<SSAValue> for PredRef {
    fn from(ssa: SSAValue) -> PredRef {
        PredRef::SSA(ssa)
    }
}

impl fmt::Display for PredRef {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            PredRef::None => write!(f, "pT"),
            PredRef::SSA(ssa) => ssa.fmt_plain(f),
            PredRef::Reg(reg) => reg.fmt(f),
        }
    }
}

#[derive(Clone, Copy)]
pub struct Pred {
    pub pred_ref: PredRef,
    pub pred_inv: bool,
}

impl Pred {
    pub fn is_true(&self) -> bool {
        self.pred_ref.is_none() && !self.pred_inv
    }

    pub fn is_false(&self) -> bool {
        self.pred_ref.is_none() && self.pred_inv
    }

    pub fn iter_ssa(&self) -> slice::Iter<'_, SSAValue> {
        self.pred_ref.iter_ssa()
    }

    pub fn iter_ssa_mut(&mut self) -> slice::IterMut<'_, SSAValue> {
        self.pred_ref.iter_ssa_mut()
    }
}

impl<T: Into<PredRef>> From<T> for Pred {
    fn from(p: T) -> Self {
        Pred {
            pred_ref: p.into(),
            pred_inv: false,
        }
    }
}

impl fmt::Display for Pred {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.pred_inv {
            write!(f, "!")?;
        }
        self.pred_ref.fmt(f)
    }
}

pub const MIN_INSTR_DELAY: u8 = 1;
pub const MAX_INSTR_DELAY: u8 = 15;

pub struct InstrDeps {
    pub delay: u8,
    pub yld: bool,
    wr_bar: i8,
    rd_bar: i8,
    pub wt_bar_mask: u8,
    pub reuse_mask: u8,
}

impl InstrDeps {
    pub fn new() -> InstrDeps {
        InstrDeps {
            delay: 0,
            yld: false,
            wr_bar: -1,
            rd_bar: -1,
            wt_bar_mask: 0,
            reuse_mask: 0,
        }
    }

    pub fn rd_bar(&self) -> Option<u8> {
        if self.rd_bar < 0 {
            None
        } else {
            Some(self.rd_bar.try_into().unwrap())
        }
    }

    pub fn wr_bar(&self) -> Option<u8> {
        if self.wr_bar < 0 {
            None
        } else {
            Some(self.wr_bar.try_into().unwrap())
        }
    }

    pub fn set_delay(&mut self, delay: u8) {
        assert!(delay <= MAX_INSTR_DELAY);
        self.delay = delay;
    }

    pub fn set_yield(&mut self, yld: bool) {
        self.yld = yld;
    }

    pub fn set_rd_bar(&mut self, idx: u8) {
        assert!(idx < 6);
        self.rd_bar = idx.try_into().unwrap();
    }

    pub fn set_wr_bar(&mut self, idx: u8) {
        assert!(idx < 6);
        self.wr_bar = idx.try_into().unwrap();
    }

    pub fn add_wt_bar(&mut self, idx: u8) {
        self.add_wt_bar_mask(1 << idx);
    }

    pub fn add_wt_bar_mask(&mut self, bar_mask: u8) {
        assert!(bar_mask < 1 << 6);
        self.wt_bar_mask |= bar_mask;
    }

    #[allow(dead_code)]
    pub fn add_reuse(&mut self, idx: u8) {
        assert!(idx < 6);
        self.reuse_mask |= 1_u8 << idx;
    }
}

impl fmt::Display for InstrDeps {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.delay > 0 {
            write!(f, " delay={}", self.delay)?;
        }
        if self.wt_bar_mask != 0 {
            write!(f, " wt={:06b}", self.wt_bar_mask)?;
        }
        if self.rd_bar >= 0 {
            write!(f, " rd:{}", self.rd_bar)?;
        }
        if self.wr_bar >= 0 {
            write!(f, " wr:{}", self.wr_bar)?;
        }
        if self.reuse_mask != 0 {
            write!(f, " reuse={:06b}", self.reuse_mask)?;
        }
        if self.yld {
            write!(f, " yld")?;
        }
        Ok(())
    }
}

pub struct Instr {
    pub pred: Pred,
    pub op: Op,
    pub deps: InstrDeps,
}

impl Instr {
    pub fn new(op: impl Into<Op>) -> Instr {
        Instr {
            op: op.into(),
            pred: PredRef::None.into(),
            deps: InstrDeps::new(),
        }
    }

    pub fn new_boxed(op: impl Into<Op>) -> Box<Self> {
        Box::new(Instr::new(op))
    }

    pub fn dsts(&self) -> &[Dst] {
        self.op.dsts_as_slice()
    }

    pub fn dsts_mut(&mut self) -> &mut [Dst] {
        self.op.dsts_as_mut_slice()
    }

    pub fn srcs(&self) -> &[Src] {
        self.op.srcs_as_slice()
    }

    pub fn srcs_mut(&mut self) -> &mut [Src] {
        self.op.srcs_as_mut_slice()
    }

    pub fn src_types(&self) -> SrcTypeList {
        self.op.src_types()
    }

    pub fn for_each_ssa_use(&self, mut f: impl FnMut(&SSAValue)) {
        for ssa in self.pred.iter_ssa() {
            f(ssa);
        }
        for src in self.srcs() {
            for ssa in src.iter_ssa() {
                f(ssa);
            }
        }
    }

    pub fn for_each_ssa_use_mut(&mut self, mut f: impl FnMut(&mut SSAValue)) {
        for ssa in self.pred.iter_ssa_mut() {
            f(ssa);
        }
        for src in self.srcs_mut() {
            for ssa in src.iter_ssa_mut() {
                f(ssa);
            }
        }
    }

    pub fn for_each_ssa_def(&self, mut f: impl FnMut(&SSAValue)) {
        for dst in self.dsts() {
            for ssa in dst.iter_ssa() {
                f(ssa);
            }
        }
    }

    pub fn for_each_ssa_def_mut(&mut self, mut f: impl FnMut(&mut SSAValue)) {
        for dst in self.dsts_mut() {
            for ssa in dst.iter_ssa_mut() {
                f(ssa);
            }
        }
    }

    pub fn is_branch(&self) -> bool {
        match self.op {
            Op::Bra(_) | Op::Exit(_) => true,
            _ => false,
        }
    }

    pub fn is_barrier(&self) -> bool {
        match self.op {
            Op::Bar(_) => true,
            _ => false,
        }
    }

    pub fn uses_global_mem(&self) -> bool {
        match &self.op {
            Op::Atom(op) => op.mem_space != MemSpace::Local,
            Op::Ld(op) => op.access.space != MemSpace::Local,
            Op::St(op) => op.access.space != MemSpace::Local,
            Op::SuAtom(_) | Op::SuLd(_) | Op::SuSt(_) => true,
            _ => false,
        }
    }

    pub fn writes_global_mem(&self) -> bool {
        match &self.op {
            Op::Atom(op) => matches!(op.mem_space, MemSpace::Global(_)),
            Op::St(op) => matches!(op.access.space, MemSpace::Global(_)),
            Op::SuAtom(_) | Op::SuSt(_) => true,
            _ => false,
        }
    }

    pub fn can_eliminate(&self) -> bool {
        match &self.op {
            Op::ASt(_)
            | Op::SuSt(_)
            | Op::SuAtom(_)
            | Op::St(_)
            | Op::Atom(_)
            | Op::CCtl(_)
            | Op::MemBar(_)
            | Op::Kill(_)
            | Op::Nop(_)
            | Op::BSync(_)
            | Op::Bra(_)
            | Op::Exit(_)
            | Op::WarpSync(_)
            | Op::Bar(_)
            | Op::FSOut(_)
            | Op::Out(_)
            | Op::OutFinal(_) => false,
            Op::BMov(op) => !op.clear,
            _ => true,
        }
    }

    pub fn has_fixed_latency(&self, _sm: u8) -> bool {
        match &self.op {
            // Float ALU
            Op::FAdd(_)
            | Op::FFma(_)
            | Op::FMnMx(_)
            | Op::FMul(_)
            | Op::FSet(_)
            | Op::FSetP(_)
            | Op::FSwzAdd(_) => true,

            // Multi-function unit is variable latency
            Op::MuFu(_) => false,

            // Double-precision float ALU
            Op::DAdd(_)
            | Op::DFma(_)
            | Op::DMnMx(_)
            | Op::DMul(_)
            | Op::DSetP(_) => false,

            // Integer ALU
            Op::Brev(_) | Op::Flo(_) | Op::PopC(_) => false,
            Op::IAbs(_)
            | Op::INeg(_)
            | Op::IAdd2(_)
            | Op::IAdd3(_)
            | Op::IAdd3X(_)
            | Op::IDp4(_)
            | Op::IMad(_)
            | Op::IMad64(_)
            | Op::IMul(_)
            | Op::IMnMx(_)
            | Op::ISetP(_)
            | Op::Lop2(_)
            | Op::Lop3(_)
            | Op::Shf(_)
            | Op::Shl(_)
            | Op::Shr(_) => true,

            // Conversions are variable latency?!?
            Op::F2F(_) | Op::F2I(_) | Op::I2F(_) | Op::FRnd(_) => false,

            // Move ops
            Op::Mov(_) | Op::Prmt(_) | Op::Sel(_) => true,
            Op::Shfl(_) => false,

            // Predicate ops
            Op::PLop3(_) | Op::PSetP(_) => true,

            // Texture ops
            Op::Tex(_)
            | Op::Tld(_)
            | Op::Tld4(_)
            | Op::Tmml(_)
            | Op::Txd(_)
            | Op::Txq(_) => false,

            // Surface ops
            Op::SuLd(_) | Op::SuSt(_) | Op::SuAtom(_) => false,

            // Memory ops
            Op::Ld(_)
            | Op::Ldc(_)
            | Op::St(_)
            | Op::Atom(_)
            | Op::AL2P(_)
            | Op::ALd(_)
            | Op::ASt(_)
            | Op::Ipa(_)
            | Op::CCtl(_)
            | Op::LdTram(_)
            | Op::MemBar(_) => false,

            // Control-flow ops
            Op::BClear(_) | Op::Break(_) | Op::BSSy(_) | Op::BSync(_) => true,
            Op::Bra(_) | Op::Exit(_) => true,
            Op::WarpSync(_) => false,

            // BMOV: barriers only when using gprs (and only valid for the gpr),
            // no barriers for the others.
            Op::BMov(op) => match &op.dst {
                Dst::None => true,
                Dst::SSA(vec) => vec.file() == RegFile::Bar,
                Dst::Reg(reg) => reg.file() == RegFile::Bar,
            },

            // Geometry ops
            Op::Out(_) | Op::OutFinal(_) => false,

            // Miscellaneous ops
            Op::Bar(_)
            | Op::CS2R(_)
            | Op::Isberd(_)
            | Op::Kill(_)
            | Op::PixLd(_)
            | Op::S2R(_) => false,
            Op::Nop(_) | Op::Vote(_) => true,

            // Virtual ops
            Op::Undef(_)
            | Op::PhiSrcs(_)
            | Op::PhiDsts(_)
            | Op::Copy(_)
            | Op::Swap(_)
            | Op::ParCopy(_)
            | Op::FSOut(_) => {
                panic!("Not a hardware opcode")
            }
        }
    }

    /// Minimum latency before another instruction can execute
    pub fn get_exec_latency(&self, sm: u8) -> u32 {
        match &self.op {
            Op::Bar(_) | Op::MemBar(_) => {
                if sm >= 80 {
                    6
                } else {
                    5
                }
            }
            Op::CCtl(_op) => {
                // CCTL.C needs 8, CCTL.I needs 11
                11
            }
            // Op::DepBar(_) => 4,
            _ => 1, // TODO: co-issue
        }
    }

    pub fn get_dst_latency(&self, sm: u8, dst_idx: usize) -> u32 {
        debug_assert!(self.has_fixed_latency(sm));
        let file = match self.dsts()[dst_idx] {
            Dst::None => return 0,
            Dst::SSA(vec) => vec.file(),
            Dst::Reg(reg) => reg.file(),
        };
        if file.is_predicate() {
            13
        } else {
            6
        }
    }

    pub fn needs_yield(&self) -> bool {
        match &self.op {
            Op::Bar(_) | Op::BSync(_) => true,
            _ => false,
        }
    }

    fn fmt_pred(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if !self.pred.is_true() {
            write!(f, "@{} ", self.pred)?;
        }
        Ok(())
    }
}

impl fmt::Display for Instr {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} {}{}", Fmt(|f| self.fmt_pred(f)), self.op, self.deps)
    }
}

impl<T: Into<Op>> From<T> for Instr {
    fn from(value: T) -> Self {
        Self::new(value)
    }
}

/// The result of map() done on a Box<Instr>. A Vec is only allocated if the
/// mapping results in multiple instructions. This helps to reduce the amount of
/// Vec's allocated in the optimization passes.
pub enum MappedInstrs {
    None,
    One(Box<Instr>),
    Many(Vec<Box<Instr>>),
}

impl MappedInstrs {
    pub fn push(&mut self, i: Box<Instr>) {
        match self {
            MappedInstrs::None => {
                *self = MappedInstrs::One(i);
            }
            MappedInstrs::One(_) => {
                *self = match std::mem::replace(self, MappedInstrs::None) {
                    MappedInstrs::One(o) => MappedInstrs::Many(vec![o, i]),
                    _ => panic!("Not a One"),
                };
            }
            MappedInstrs::Many(v) => {
                v.push(i);
            }
        }
    }

    pub fn last_mut(&mut self) -> Option<&mut Box<Instr>> {
        match self {
            MappedInstrs::None => None,
            MappedInstrs::One(instr) => Some(instr),
            MappedInstrs::Many(v) => v.last_mut(),
        }
    }
}

pub struct BasicBlock {
    pub label: Label,
    pub instrs: Vec<Box<Instr>>,
}

impl BasicBlock {
    pub fn new(label: Label) -> BasicBlock {
        BasicBlock {
            label: label,
            instrs: Vec::new(),
        }
    }

    fn map_instrs_priv(
        &mut self,
        map: &mut impl FnMut(Box<Instr>, &mut SSAValueAllocator) -> MappedInstrs,
        ssa_alloc: &mut SSAValueAllocator,
    ) {
        let mut instrs = Vec::new();
        for i in self.instrs.drain(..) {
            match map(i, ssa_alloc) {
                MappedInstrs::None => (),
                MappedInstrs::One(i) => {
                    instrs.push(i);
                }
                MappedInstrs::Many(mut v) => {
                    instrs.append(&mut v);
                }
            }
        }
        self.instrs = instrs;
    }

    pub fn phi_dsts(&self) -> Option<&OpPhiDsts> {
        for instr in self.instrs.iter() {
            match &instr.op {
                Op::PhiDsts(phi) => return Some(phi),
                _ => break,
            }
        }
        None
    }

    #[allow(dead_code)]
    pub fn phi_dsts_mut(&mut self) -> Option<&mut OpPhiDsts> {
        for instr in self.instrs.iter_mut() {
            match &mut instr.op {
                Op::PhiDsts(phi) => return Some(phi),
                _ => break,
            }
        }
        None
    }

    pub fn phi_srcs(&self) -> Option<&OpPhiSrcs> {
        for instr in self.instrs.iter().rev() {
            if instr.is_branch() {
                continue;
            }

            match &instr.op {
                Op::PhiSrcs(phi) => return Some(phi),
                _ => break,
            }
        }
        None
    }

    pub fn phi_srcs_mut(&mut self) -> Option<&mut OpPhiSrcs> {
        for instr in self.instrs.iter_mut().rev() {
            if instr.is_branch() {
                continue;
            }

            match &mut instr.op {
                Op::PhiSrcs(phi) => return Some(phi),
                _ => break,
            }
        }
        None
    }

    pub fn branch(&self) -> Option<&Instr> {
        if let Some(i) = self.instrs.last() {
            if i.is_branch() {
                Some(i)
            } else {
                None
            }
        } else {
            None
        }
    }

    #[allow(dead_code)]
    pub fn branch_mut(&mut self) -> Option<&mut Instr> {
        if let Some(i) = self.instrs.last_mut() {
            if i.is_branch() {
                Some(i)
            } else {
                None
            }
        } else {
            None
        }
    }

    pub fn falls_through(&self) -> bool {
        if let Some(i) = self.branch() {
            !i.pred.is_true()
        } else {
            true
        }
    }
}

pub struct Function {
    pub ssa_alloc: SSAValueAllocator,
    pub phi_alloc: PhiAllocator,
    pub blocks: CFG<BasicBlock>,
}

impl Function {
    fn map_instrs_priv(
        &mut self,
        map: &mut impl FnMut(Box<Instr>, &mut SSAValueAllocator) -> MappedInstrs,
    ) {
        for b in &mut self.blocks {
            b.map_instrs_priv(map, &mut self.ssa_alloc);
        }
    }

    pub fn map_instrs(
        &mut self,
        mut map: impl FnMut(Box<Instr>, &mut SSAValueAllocator) -> MappedInstrs,
    ) {
        self.map_instrs_priv(&mut map);
    }
}

impl fmt::Display for Function {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let mut pred_width = 0;
        let mut dsts_width = 0;
        let mut op_width = 0;

        let mut blocks = Vec::new();
        for b in &self.blocks {
            let mut instrs = Vec::new();
            for i in &b.instrs {
                let mut pred = String::new();
                write!(pred, "{}", Fmt(|f| i.fmt_pred(f)))?;
                let mut dsts = String::new();
                write!(dsts, "{}", Fmt(|f| i.op.fmt_dsts(f)))?;
                let mut op = String::new();
                write!(op, "{}", Fmt(|f| i.op.fmt_op(f)))?;
                let mut deps = String::new();
                write!(deps, "{}", i.deps)?;

                pred_width = max(pred_width, pred.len());
                dsts_width = max(dsts_width, dsts.len());
                op_width = max(op_width, op.len());

                instrs.push((pred, dsts, op, deps));
            }
            blocks.push(instrs);
        }

        for (i, mut b) in blocks.drain(..).enumerate() {
            write!(f, "block {} {} [", i, self.blocks[i].label)?;
            for (pi, p) in self.blocks.pred_indices(i).iter().enumerate() {
                if pi > 0 {
                    write!(f, ", ")?;
                }
                write!(f, "{}", p)?;
            }
            write!(f, "] -> {{\n")?;

            for (pred, dsts, op, deps) in b.drain(..) {
                let eq_sym = if dsts.is_empty() { " " } else { "=" };
                if deps.is_empty() {
                    write!(
                        f,
                        "{:<pred_width$} {:<dsts_width$} {} {}\n",
                        pred, dsts, eq_sym, op,
                    )?;
                } else {
                    write!(
                        f,
                        "{:<pred_width$} {:<dsts_width$} {} \
                         {:<op_width$} //{}\n",
                        pred, dsts, eq_sym, op, deps,
                    )?;
                }
            }

            write!(f, "}} -> [")?;
            for (si, s) in self.blocks.succ_indices(i).iter().enumerate() {
                if si > 0 {
                    write!(f, ", ")?;
                }
                write!(f, "{}", s)?;
            }
            write!(f, "]\n")?;
        }
        Ok(())
    }
}

#[derive(Debug)]
pub struct ComputeShaderInfo {
    pub local_size: [u16; 3],
    pub smem_size: u16,
}

#[derive(Debug)]
pub struct GeometryShaderInfo {
    pub passthrough_enable: bool,
    pub stream_out_mask: u8,
    pub threads_per_input_primitive: u8,
    pub output_topology: OutputTopology,
    pub max_output_vertex_count: u16,
}

impl Default for GeometryShaderInfo {
    fn default() -> Self {
        Self {
            passthrough_enable: false,
            stream_out_mask: 0,
            threads_per_input_primitive: 0,
            output_topology: OutputTopology::LineStrip,
            max_output_vertex_count: 0,
        }
    }
}

#[derive(Debug)]
pub struct TessellationInitShaderInfo {
    pub per_patch_attribute_count: u8,
    pub threads_per_patch: u8,
}

#[derive(Debug)]
pub enum ShaderStageInfo {
    Compute(ComputeShaderInfo),
    Vertex,
    Fragment,
    Geometry(GeometryShaderInfo),
    TessellationInit(TessellationInitShaderInfo),
    Tessellation,
}

#[derive(Debug, Default)]
pub struct SysValInfo {
    pub ab: u32,
    pub c: u16,
}

#[derive(Debug)]
pub struct VtgIoInfo {
    pub sysvals_in: SysValInfo,
    pub sysvals_in_d: u8,
    pub sysvals_out: SysValInfo,
    pub sysvals_out_d: u8,
    pub attr_in: [u32; 4],
    pub attr_out: [u32; 4],
    pub store_req_start: u8,
    pub store_req_end: u8,
}

impl VtgIoInfo {
    fn mark_attrs(&mut self, addrs: Range<u16>, written: bool) {
        let sysvals = if written {
            &mut self.sysvals_out
        } else {
            &mut self.sysvals_in
        };

        let sysvals_d = if written {
            &mut self.sysvals_out_d
        } else {
            &mut self.sysvals_in_d
        };

        let mut attr = BitMutView::new(if written {
            &mut self.attr_out
        } else {
            &mut self.attr_in
        });

        let mut addrs = addrs;
        addrs.start &= !3;
        for addr in addrs.step_by(4) {
            if addr < 0x080 {
                sysvals.ab |= 1 << (addr / 4);
            } else if addr < 0x280 {
                let attr_idx = (addr - 0x080) as usize / 4;
                attr.set_bit(attr_idx, true);
            } else if addr < 0x2c0 {
                panic!("FF color I/O not supported");
            } else if addr < 0x300 {
                sysvals.c |= 1 << ((addr - 0x2c0) / 4);
            } else if addr >= 0x3a0 && addr < 0x3c0 {
                *sysvals_d |= 1 << ((addr - 0x3a0) / 4);
            }
        }
    }

    pub fn mark_attrs_read(&mut self, addrs: Range<u16>) {
        self.mark_attrs(addrs, false);
    }

    pub fn mark_attrs_written(&mut self, addrs: Range<u16>) {
        self.mark_attrs(addrs, true);
    }

    pub fn mark_store_req(&mut self, addrs: Range<u16>) {
        let start = (addrs.start / 4).try_into().unwrap();
        let end = ((addrs.end - 1) / 4).try_into().unwrap();
        self.store_req_start = min(self.store_req_start, start);
        self.store_req_end = max(self.store_req_end, end);
    }
}

#[derive(Debug)]
pub struct FragmentIoInfo {
    pub sysvals_in: SysValInfo,
    pub sysvals_in_d: [PixelImap; 8],
    pub attr_in: [PixelImap; 128],
    pub barycentric_attr_in: [u32; 4],

    pub reads_sample_mask: bool,
    pub uses_kill: bool,
    pub writes_color: u32,
    pub writes_sample_mask: bool,
    pub writes_depth: bool,
    pub does_interlock: bool,
}

impl FragmentIoInfo {
    pub fn mark_attr_read(&mut self, addr: u16, interp: PixelImap) {
        if addr < 0x080 {
            self.sysvals_in.ab |= 1 << (addr / 4);
        } else if addr < 0x280 {
            let attr_idx = (addr - 0x080) as usize / 4;
            self.attr_in[attr_idx] = interp;
        } else if addr < 0x2c0 {
            panic!("FF color I/O not supported");
        } else if addr < 0x300 {
            self.sysvals_in.c |= 1 << ((addr - 0x2c0) / 4);
        } else if addr >= 0x3a0 && addr < 0x3c0 {
            let attr_idx = (addr - 0x3a0) as usize / 4;
            self.sysvals_in_d[attr_idx] = interp;
        }
    }

    pub fn mark_barycentric_attr_in(&mut self, addr: u16) {
        assert!(addr >= 0x80 && addr < 0x280);

        let mut attr = BitMutView::new(&mut self.barycentric_attr_in);

        let attr_idx = (addr - 0x080) as usize / 4;
        attr.set_bit(attr_idx, true);
    }
}

#[derive(Debug)]
pub enum ShaderIoInfo {
    None,
    Vtg(VtgIoInfo),
    Fragment(FragmentIoInfo),
}

#[derive(Debug)]
pub struct ShaderInfo {
    pub sm: u8,
    pub num_gprs: u8,
    pub num_barriers: u8,
    pub slm_size: u32,
    pub uses_global_mem: bool,
    pub writes_global_mem: bool,
    pub uses_fp64: bool,
    pub stage: ShaderStageInfo,
    pub io: ShaderIoInfo,
}

pub struct Shader {
    pub info: ShaderInfo,
    pub functions: Vec<Function>,
}

impl Shader {
    pub fn for_each_instr(&self, f: &mut impl FnMut(&Instr)) {
        for func in &self.functions {
            for b in &func.blocks {
                for i in &b.instrs {
                    f(i);
                }
            }
        }
    }

    pub fn map_instrs(
        &mut self,
        mut map: impl FnMut(Box<Instr>, &mut SSAValueAllocator) -> MappedInstrs,
    ) {
        for f in &mut self.functions {
            f.map_instrs_priv(&mut map);
        }
    }

    pub fn lower_ineg(&mut self) {
        let sm = self.info.sm;
        self.map_instrs(|mut instr: Box<Instr>, _| -> MappedInstrs {
            match instr.op {
                Op::INeg(neg) => {
                    if sm >= 70 {
                        instr.op = Op::IAdd3(OpIAdd3 {
                            dst: neg.dst,
                            overflow: [Dst::None; 2],
                            srcs: [0.into(), neg.src.ineg(), 0.into()],
                        });
                    } else {
                        instr.op = Op::IAdd2(OpIAdd2 {
                            dst: neg.dst,
                            srcs: [0.into(), neg.src.ineg()],
                            carry_in: 0.into(),
                            carry_out: Dst::None,
                        });
                    }
                    MappedInstrs::One(instr)
                }
                _ => MappedInstrs::One(instr),
            }
        })
    }

    pub fn gather_global_mem_usage(&mut self) {
        if let ShaderStageInfo::Compute(_) = self.info.stage {
            return;
        }

        let mut uses_global_mem = false;
        let mut writes_global_mem = false;

        self.for_each_instr(&mut |instr| {
            if !uses_global_mem {
                uses_global_mem = instr.uses_global_mem();
            }

            if !writes_global_mem {
                writes_global_mem = instr.writes_global_mem();
            }
        });

        self.info.uses_global_mem = uses_global_mem;
        self.info.writes_global_mem = writes_global_mem;
    }
}

impl fmt::Display for Shader {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        for func in &self.functions {
            write!(f, "{}", func)?;
        }
        Ok(())
    }
}
