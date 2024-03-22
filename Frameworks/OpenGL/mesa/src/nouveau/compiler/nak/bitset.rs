// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use std::ops::{
    BitAnd, BitAndAssign, BitOr, BitOrAssign, BitXor, BitXorAssign, Not, Range,
};

#[derive(Clone)]
pub struct BitSet {
    words: Vec<u32>,
}

impl BitSet {
    pub fn new() -> BitSet {
        BitSet { words: Vec::new() }
    }

    fn reserve_words(&mut self, words: usize) {
        if self.words.len() < words {
            self.words.resize(words, 0);
        }
    }

    #[allow(dead_code)]
    pub fn reserve(&mut self, bits: usize) {
        self.reserve_words(bits.div_ceil(32));
    }

    pub fn clear(&mut self) {
        for w in self.words.iter_mut() {
            *w = 0;
        }
    }

    pub fn get(&self, idx: usize) -> bool {
        let w = idx / 32;
        let b = idx % 32;
        if w < self.words.len() {
            self.words[w] & (1_u32 << b) != 0
        } else {
            false
        }
    }

    #[allow(dead_code)]
    pub fn is_empty(&self) -> bool {
        for w in self.words.iter() {
            if *w != 0 {
                return false;
            }
        }
        true
    }

    pub fn get_word(&self, word: usize) -> u32 {
        self.words.get(word).cloned().unwrap_or(0)
    }

    #[allow(dead_code)]
    pub fn next_set(&self, start: usize) -> Option<usize> {
        if start >= self.words.len() * 32 {
            return None;
        }

        let mut w = start / 32;
        let mut mask = u32::MAX << (start % 32);
        while w < self.words.len() {
            let b = (self.words[w] & mask).trailing_zeros();
            if b < 32 {
                return Some(w * 32 + usize::try_from(b).unwrap());
            }
            mask = u32::MAX;
            w += 1;
        }
        None
    }

    #[allow(dead_code)]
    pub fn next_unset(&self, start: usize) -> usize {
        if start >= self.words.len() * 32 {
            return start;
        }

        let mut w = start / 32;
        let mut mask = !(u32::MAX << (start % 32));
        while w < self.words.len() {
            let b = (self.words[w] | mask).trailing_ones();
            if b < 32 {
                return w * 32 + usize::try_from(b).unwrap();
            }
            mask = 0;
            w += 1;
        }
        self.words.len() * 32
    }

    pub fn insert(&mut self, idx: usize) -> bool {
        let w = idx / 32;
        let b = idx % 32;
        self.reserve_words(w + 1);
        let exists = self.words[w] & (1_u32 << b) != 0;
        self.words[w] |= 1_u32 << b;
        !exists
    }

    pub fn remove(&mut self, idx: usize) -> bool {
        let w = idx / 32;
        let b = idx % 32;
        self.reserve_words(w + 1);
        let exists = self.words[w] & (1_u32 << b) != 0;
        self.words[w] &= !(1_u32 << b);
        exists
    }

    #[inline]
    fn set_word(
        &mut self,
        w: usize,
        mask: u32,
        f: &mut impl FnMut(usize) -> u32,
    ) {
        self.words[w] = (self.words[w] & !mask) | (f(w) & mask);
    }

    pub fn set_words(
        &mut self,
        bits: Range<usize>,
        mut f: impl FnMut(usize) -> u32,
    ) {
        if bits.is_empty() {
            return;
        }

        let first_word = bits.start / 32;
        let last_word = (bits.end - 1) / 32;
        let start_mask = !0_u32 << (bits.start % 32);
        let end_mask = !0_u32 >> (31 - ((bits.end - 1) % 32));

        self.reserve(last_word + 1);

        if first_word == last_word {
            self.set_word(first_word, start_mask & end_mask, &mut f);
        } else {
            self.set_word(first_word, start_mask, &mut f);
            for w in (first_word + 1)..last_word {
                self.set_word(w, !0, &mut f);
            }
            self.set_word(last_word, end_mask, &mut f);
        }
    }

    pub fn union_with(&mut self, other: &BitSet) -> bool {
        let mut added_bits = false;
        self.reserve_words(other.words.len());
        for w in 0..other.words.len() {
            let uw = self.words[w] | other.words[w];
            if uw != self.words[w] {
                added_bits = true;
                self.words[w] = uw;
            }
        }
        added_bits
    }
}

impl Default for BitSet {
    fn default() -> BitSet {
        BitSet::new()
    }
}

impl BitAndAssign for BitSet {
    fn bitand_assign(&mut self, rhs: BitSet) {
        self.reserve_words(rhs.words.len());
        for w in 0..rhs.words.len() {
            self.words[w] &= rhs.words[w];
        }
    }
}

impl BitAnd<BitSet> for BitSet {
    type Output = BitSet;

    fn bitand(self, rhs: BitSet) -> BitSet {
        let mut res = self;
        res.bitand_assign(rhs);
        res
    }
}

impl BitOrAssign for BitSet {
    fn bitor_assign(&mut self, rhs: BitSet) {
        self.reserve_words(rhs.words.len());
        for w in 0..rhs.words.len() {
            self.words[w] |= rhs.words[w];
        }
    }
}

impl BitOr<BitSet> for BitSet {
    type Output = BitSet;

    fn bitor(self, rhs: BitSet) -> BitSet {
        let mut res = self;
        res.bitor_assign(rhs);
        res
    }
}

impl BitXorAssign for BitSet {
    fn bitxor_assign(&mut self, rhs: BitSet) {
        self.reserve_words(rhs.words.len());
        for w in 0..rhs.words.len() {
            self.words[w] ^= rhs.words[w];
        }
    }
}

impl BitXor<BitSet> for BitSet {
    type Output = BitSet;

    fn bitxor(self, rhs: BitSet) -> BitSet {
        let mut res = self;
        res.bitxor_assign(rhs);
        res
    }
}

impl Not for BitSet {
    type Output = BitSet;

    fn not(self) -> BitSet {
        let mut res = self;
        for w in 0..res.words.len() {
            res.words[w] = !res.words[w];
        }
        res
    }
}
