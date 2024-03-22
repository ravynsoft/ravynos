// Copyright © 2023 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use crate::ir::*;

fn try_combine_outs(emit: &mut Instr, cut: &Instr) -> bool {
    let Op::Out(emit) = &mut emit.op else {
        return false;
    };

    let Op::Out(cut) = &cut.op else {
        return false;
    };

    if emit.out_type != OutType::Emit || cut.out_type != OutType::Cut {
        return false;
    }

    let Some(handle) = emit.dst.as_ssa() else {
        return false;
    };

    if cut.handle.as_ssa() != Some(handle) {
        return false;
    }

    if emit.stream != cut.stream {
        return false;
    }

    emit.dst = cut.dst;
    emit.out_type = OutType::EmitThenCut;

    true
}

impl Shader {
    pub fn opt_out(&mut self) {
        if !matches!(self.info.stage, ShaderStageInfo::Geometry(_)) {
            return;
        }

        for f in &mut self.functions {
            for b in &mut f.blocks {
                let mut instrs: Vec<Box<Instr>> = Vec::new();
                for instr in b.instrs.drain(..) {
                    if let Some(prev) = instrs.last_mut() {
                        if try_combine_outs(prev, &instr) {
                            continue;
                        }
                    }
                    instrs.push(instr);
                }
                b.instrs = instrs;
            }
        }
    }
}
