// Copyright © 2023 Collabora, Ltd.
// SPDX-License-Identifier: MIT

mod api;
mod assign_regs;
mod bitset;
mod builder;
mod calc_instr_deps;
mod cfg;
mod encode_sm50;
mod encode_sm70;
mod from_nir;
mod ir;
mod legalize;
mod liveness;
mod lower_copy_swap;
mod lower_par_copies;
mod nir;
mod opt_bar_prop;
mod opt_copy_prop;
mod opt_dce;
mod opt_jump_thread;
mod opt_lop;
mod opt_out;
mod repair_ssa;
mod sph;
mod spill_values;
mod to_cssa;
