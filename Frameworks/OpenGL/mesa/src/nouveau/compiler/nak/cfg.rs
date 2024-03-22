// Copyright © 2023 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::bitset::BitSet;

use std::collections::HashMap;
use std::hash::Hash;
use std::ops::{Deref, DerefMut, Index, IndexMut};
use std::slice;

pub struct CFGNode<N> {
    node: N,
    dom: usize,
    dom_pre_idx: usize,
    dom_post_idx: usize,
    lph: usize,
    pred: Vec<usize>,
    succ: Vec<usize>,
}

impl<N> Deref for CFGNode<N> {
    type Target = N;

    fn deref(&self) -> &N {
        &self.node
    }
}

impl<N> DerefMut for CFGNode<N> {
    fn deref_mut(&mut self) -> &mut N {
        &mut self.node
    }
}

fn graph_post_dfs<N>(
    nodes: &Vec<CFGNode<N>>,
    id: usize,
    seen: &mut BitSet,
    post_idx: &mut Vec<usize>,
    count: &mut usize,
) {
    if seen.get(id) {
        return;
    }
    seen.insert(id);

    // Reverse the order of the successors so that any successors which are
    // forward edges get descending indices.  This ensures that, in the reverse
    // post order, successors (and their dominated children) come in-order.
    // In particular, as long as fall-through edges are only ever used for
    // forward edges and the fall-through edge comes first, we guarantee that
    // the fallthrough block comes immediately after its predecessor.
    for s in nodes[id].succ.iter().rev() {
        graph_post_dfs(nodes, *s, seen, post_idx, count);
    }

    post_idx[id] = *count;
    *count += 1;
}

fn rev_post_order_sort<N>(nodes: &mut Vec<CFGNode<N>>) {
    let mut seen = BitSet::new();
    let mut post_idx = Vec::new();
    post_idx.resize(nodes.len(), usize::MAX);
    let mut count = 0;

    graph_post_dfs(nodes, 0, &mut seen, &mut post_idx, &mut count);

    assert!(count <= nodes.len());

    let remap_idx = |i: usize| {
        let pid = post_idx[i];
        if pid == usize::MAX {
            None
        } else {
            assert!(pid < count);
            Some((count - 1) - pid)
        }
    };
    assert!(remap_idx(0) == Some(0));

    // Re-map edges to use post-index numbering
    for n in nodes.iter_mut() {
        let remap_filter_idx = |i: &mut usize| {
            if let Some(r) = remap_idx(*i) {
                *i = r;
                true
            } else {
                false
            }
        };
        n.pred.retain_mut(remap_filter_idx);
        n.succ.retain_mut(remap_filter_idx);
    }

    // We know a priori that each non-MAX post_idx is unique so we can sort the
    // nodes by inserting them into a new array by index.
    let mut sorted: Vec<CFGNode<N>> = Vec::with_capacity(count);
    for (i, n) in nodes.drain(..).enumerate() {
        if let Some(r) = remap_idx(i) {
            unsafe { sorted.as_mut_ptr().add(r).write(n) };
        }
    }
    unsafe { sorted.set_len(count) };

    std::mem::swap(nodes, &mut sorted);
}

fn find_common_dom<N>(
    nodes: &Vec<CFGNode<N>>,
    mut a: usize,
    mut b: usize,
) -> usize {
    while a != b {
        while a > b {
            a = nodes[a].dom;
        }
        while b > a {
            b = nodes[b].dom;
        }
    }
    a
}

fn dom_idx_dfs<N>(
    nodes: &mut Vec<CFGNode<N>>,
    dom_children: &Vec<Vec<usize>>,
    id: usize,
    count: &mut usize,
) {
    nodes[id].dom_pre_idx = *count;
    *count += 1;

    for c in dom_children[id].iter() {
        dom_idx_dfs(nodes, dom_children, *c, count);
    }

    nodes[id].dom_post_idx = *count;
    *count += 1;
}

fn calc_dominance<N>(nodes: &mut Vec<CFGNode<N>>) {
    nodes[0].dom = 0;
    loop {
        let mut changed = false;
        for i in 1..nodes.len() {
            let mut dom = nodes[i].pred[0];
            for p in &nodes[i].pred[1..] {
                if nodes[*p].dom != usize::MAX {
                    dom = find_common_dom(nodes, dom, *p);
                }
            }
            assert!(dom != usize::MAX);
            if nodes[i].dom != dom {
                nodes[i].dom = dom;
                changed = true;
            }
        }

        if !changed {
            break;
        }
    }

    let mut dom_children = Vec::new();
    dom_children.resize(nodes.len(), Vec::new());

    for i in 1..nodes.len() {
        let p = nodes[i].dom;
        if p != i {
            dom_children[p].push(i);
        }
    }

    let mut count = 0_usize;
    dom_idx_dfs(nodes, &dom_children, 0, &mut count);
    debug_assert!(count == nodes.len() * 2);
}

fn loop_detect_dfs<N>(
    nodes: &Vec<CFGNode<N>>,
    id: usize,
    pre: &mut BitSet,
    post: &mut BitSet,
    loops: &mut BitSet,
) {
    if pre.get(id) {
        if !post.get(id) {
            loops.insert(id);
        }
        return;
    }

    pre.insert(id);

    for s in nodes[id].succ.iter() {
        loop_detect_dfs(nodes, *s, pre, post, loops);
    }

    post.insert(id);
}

fn detect_loops<N>(nodes: &mut Vec<CFGNode<N>>) -> bool {
    let mut dfs_pre = BitSet::new();
    let mut dfs_post = BitSet::new();
    let mut loops = BitSet::new();
    loop_detect_dfs(nodes, 0, &mut dfs_pre, &mut dfs_post, &mut loops);

    let mut has_loop = false;
    nodes[0].lph = usize::MAX;
    for i in 1..nodes.len() {
        if loops.get(i) {
            // This is a loop header
            nodes[i].lph = i;
            has_loop = true;
        } else {
            // Otherwise, we have the same loop header as our dominator
            let dom = nodes[i].dom;
            let dom_lph = nodes[dom].lph;
            nodes[i].lph = dom_lph;
        }
    }

    has_loop
}

pub struct CFG<N> {
    has_loop: bool,
    nodes: Vec<CFGNode<N>>,
}

impl<N> CFG<N> {
    pub fn from_blocks_edges(
        nodes: impl IntoIterator<Item = N>,
        edges: impl IntoIterator<Item = (usize, usize)>,
    ) -> Self {
        let mut nodes = Vec::from_iter(nodes.into_iter().map(|n| CFGNode {
            node: n,
            dom: usize::MAX,
            dom_pre_idx: usize::MAX,
            dom_post_idx: 0,
            lph: usize::MAX,
            pred: Vec::new(),
            succ: Vec::new(),
        }));

        for (p, s) in edges {
            nodes[s].pred.push(p);
            nodes[p].succ.push(s);
        }

        rev_post_order_sort(&mut nodes);
        calc_dominance(&mut nodes);
        let has_loop = detect_loops(&mut nodes);

        CFG {
            has_loop: has_loop,
            nodes: nodes,
        }
    }

    pub fn get(&self, idx: usize) -> Option<&N> {
        self.nodes.get(idx).map(|n| &n.node)
    }

    pub fn get_mut(&mut self, idx: usize) -> Option<&mut N> {
        self.nodes.get_mut(idx).map(|n| &mut n.node)
    }

    pub fn iter(&self) -> slice::Iter<CFGNode<N>> {
        self.nodes.iter()
    }

    pub fn iter_mut(&mut self) -> slice::IterMut<CFGNode<N>> {
        self.nodes.iter_mut()
    }

    pub fn len(&self) -> usize {
        self.nodes.len()
    }

    pub fn dom_dfs_pre_index(&self, idx: usize) -> usize {
        self.nodes[idx].dom_pre_idx
    }

    pub fn dom_dfs_post_index(&self, idx: usize) -> usize {
        self.nodes[idx].dom_post_idx
    }

    pub fn dom_parent_index(&self, idx: usize) -> Option<usize> {
        if idx == 0 {
            None
        } else {
            Some(self.nodes[idx].dom)
        }
    }

    pub fn dominates(&self, parent: usize, child: usize) -> bool {
        // If a block is unreachable, then dom_pre_idx == usize::MAX and
        // dom_post_idx == 0.  This allows us to trivially handle unreachable
        // blocks here with zero extra work.
        self.dom_dfs_pre_index(child) >= self.dom_dfs_pre_index(parent)
            && self.dom_dfs_post_index(child) <= self.dom_dfs_post_index(parent)
    }

    pub fn has_loop(&self) -> bool {
        self.has_loop
    }

    pub fn is_loop_header(&self, idx: usize) -> bool {
        self.nodes[idx].lph == idx
    }

    pub fn loop_header_index(&self, idx: usize) -> Option<usize> {
        let lph = self.nodes[idx].lph;
        if lph == usize::MAX {
            None
        } else {
            debug_assert!(self.is_loop_header(lph));
            Some(lph)
        }
    }

    pub fn succ_indices(&self, idx: usize) -> &[usize] {
        &self.nodes[idx].succ[..]
    }

    pub fn pred_indices(&self, idx: usize) -> &[usize] {
        &self.nodes[idx].pred[..]
    }

    pub fn drain<'a>(&'a mut self) -> impl Iterator<Item = N> + 'a {
        self.has_loop = false;
        self.nodes.drain(..).map(|n| n.node)
    }
}

impl<N> Index<usize> for CFG<N> {
    type Output = N;

    fn index(&self, idx: usize) -> &N {
        &self.nodes[idx].node
    }
}

impl<N> IndexMut<usize> for CFG<N> {
    fn index_mut(&mut self, idx: usize) -> &mut N {
        &mut self.nodes[idx].node
    }
}

impl<'a, N> IntoIterator for &'a CFG<N> {
    type Item = &'a CFGNode<N>;
    type IntoIter = slice::Iter<'a, CFGNode<N>>;

    fn into_iter(self) -> slice::Iter<'a, CFGNode<N>> {
        self.iter()
    }
}

impl<'a, N> IntoIterator for &'a mut CFG<N> {
    type Item = &'a mut CFGNode<N>;
    type IntoIter = slice::IterMut<'a, CFGNode<N>>;

    fn into_iter(self) -> slice::IterMut<'a, CFGNode<N>> {
        self.iter_mut()
    }
}

pub struct CFGBuilder<K, N> {
    nodes: Vec<N>,
    edges: Vec<(K, K)>,
    key_map: HashMap<K, usize>,
}

impl<K, N> CFGBuilder<K, N> {
    pub fn new() -> CFGBuilder<K, N> {
        CFGBuilder {
            nodes: Vec::new(),
            edges: Vec::new(),
            key_map: HashMap::new(),
        }
    }
}

impl<K: Eq + Hash, N> CFGBuilder<K, N> {
    pub fn add_node(&mut self, k: K, n: N) {
        self.key_map.insert(k, self.nodes.len());
        self.nodes.push(n);
    }

    pub fn add_edge(&mut self, s: K, p: K) {
        self.edges.push((s, p));
    }

    pub fn as_cfg(mut self) -> CFG<N> {
        let edges = self.edges.drain(..).map(|(s, p)| {
            let s = *self.key_map.get(&s).unwrap();
            let p = *self.key_map.get(&p).unwrap();
            (s, p)
        });
        CFG::from_blocks_edges(self.nodes, edges)
    }
}

impl<K, N> Default for CFGBuilder<K, N> {
    fn default() -> Self {
        CFGBuilder::new()
    }
}
