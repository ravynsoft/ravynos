// Copyright © 2023 Mel Henning
// SPDX-License-Identifier: MIT

use crate::cfg::CFGBuilder;
use crate::ir::*;
use std::collections::HashMap;

fn clone_branch(op: &Op) -> Op {
    match op {
        Op::Bra(b) => Op::Bra(b.clone()),
        Op::Exit(e) => Op::Exit(e.clone()),
        _ => unreachable!(),
    }
}

fn jump_thread(func: &mut Function) -> bool {
    // Let's call a basic block "trivial" if its only instruction is an
    // unconditional branch. If a block is trivial, we can update all of its
    // predecessors to jump to its sucessor.
    //
    // A single reverse pass over the basic blocks is enough to update all of
    // the edges we're interested in. Roughly, if we assume that all loops in
    // the shader can terminate, then loop heads are never trivial and we
    // never replace a backward edge. Therefore, in each step we only need to
    // make sure that later control flow has been replaced in order to update
    // the current block as much as possible.
    //
    // We additionally try to update a branch-to-empty-block to point to the
    // block's successor, which along with block dce/reordering can sometimes
    // enable a later optimization that converts branches to fallthrough.
    let mut progress = false;

    // A branch to label can be replaced with Op
    let mut replacements: HashMap<Label, Op> = HashMap::new();

    // Invariant 1: At the end of each loop iteration,
    //              every trivial block with an index in [i, blocks.len())
    //              is represented in replacements.keys()
    // Invariant 2: replacements.values() never contains
    //              a branch to a trivial block
    for i in (0..func.blocks.len()).rev() {
        // Replace the branch if possible
        if let Some(instr) = func.blocks[i].instrs.last_mut() {
            if let Op::Bra(OpBra { target }) = instr.op {
                if let Some(replacement) = replacements.get(&target) {
                    instr.op = clone_branch(replacement);
                    progress = true;
                }
                // If the branch target was previously a trivial block then the
                // branch was previously a forward edge (see above) and by
                // invariants 1 and 2 we just updated the branch to target
                // a nontrivial block
            }
        }

        // Is this block trivial?
        let block_label = func.blocks[i].label;
        match &func.blocks[i].instrs[..] {
            [instr] => {
                if instr.is_branch() && instr.pred.is_true() {
                    // Upholds invariant 2 because we updated the branch above
                    replacements.insert(block_label, clone_branch(&instr.op));
                }
            }
            [] => {
                // Empty block - falls through
                // Our successor might be trivial, so we need to
                // apply the rewrite map to uphold invariant 2
                let target_label = func.blocks[i + 1].label;
                let replacement = replacements
                    .get(&target_label)
                    .map(clone_branch)
                    .unwrap_or_else(|| {
                        Op::Bra(OpBra {
                            target: target_label,
                        })
                    });
                replacements.insert(block_label, replacement);
            }
            _ => (),
        }
    }

    if progress {
        // We don't update the CFG above, so rewrite it if we made progress
        rewrite_cfg(func);
    }

    return progress;
}

fn rewrite_cfg(func: &mut Function) {
    // CFGBuilder takes care of removing dead blocks for us
    // We use the basic block's label to identify it
    let mut builder = CFGBuilder::new();

    for i in 0..func.blocks.len() {
        let block = &func.blocks[i];
        // Note: fall-though must be first edge
        if block.falls_through() {
            let next_block = &func.blocks[i + 1];
            builder.add_edge(block.label, next_block.label);
        }
        if let Some(control_flow) = block.branch() {
            match &control_flow.op {
                Op::Bra(bra) => {
                    builder.add_edge(block.label, bra.target);
                }
                Op::Exit(_) => (),
                _ => unreachable!(),
            };
        }
    }

    for block in func.blocks.drain() {
        builder.add_node(block.label, block);
    }
    let _ = std::mem::replace(&mut func.blocks, builder.as_cfg());
}

/// Replace jumps to the following block with fall-through
fn opt_fall_through(func: &mut Function) {
    for i in 0..func.blocks.len() - 1 {
        let remove_last_instr = match func.blocks[i].branch() {
            Some(b) => match b.op {
                Op::Bra(OpBra { target }) => target == func.blocks[i + 1].label,
                _ => false,
            },
            None => false,
        };

        if remove_last_instr {
            func.blocks[i].instrs.pop();
        }
    }
}

impl Function {
    pub fn opt_jump_thread(&mut self) {
        if jump_thread(self) {
            opt_fall_through(self);
        }
    }
}

impl Shader {
    /// A simple jump threading pass
    ///
    /// Note that this can introduce critical edges, so it cannot be run before RA
    pub fn opt_jump_thread(&mut self) {
        for f in &mut self.functions {
            f.opt_jump_thread();
        }
    }
}
