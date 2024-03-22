// Copyright © 2022 Collabora, Ltd.
// SPDX-License-Identifier: MIT

use nak_bindings::*;

use std::ffi::{c_void, CStr};
use std::marker::PhantomData;
use std::ptr::NonNull;
use std::str;

// from https://internals.rust-lang.org/t/discussion-on-offset-of/7440/2
macro_rules! offset_of {
    ($Struct:path, $field:ident) => {{
        // Using a separate function to minimize unhygienic hazards
        // (e.g. unsafety of #[repr(packed)] field borrows).
        // Uncomment `const` when `const fn`s can juggle pointers.

        // const
        fn offset() -> usize {
            let u = std::mem::MaybeUninit::<$Struct>::uninit();
            // Use pattern-matching to avoid accidentally going through Deref.
            let &$Struct { $field: ref f, .. } = unsafe { &*u.as_ptr() };
            let o =
                (f as *const _ as usize).wrapping_sub(&u as *const _ as usize);
            // Triple check that we are within `u` still.
            assert!((0..=std::mem::size_of_val(&u)).contains(&o));
            o
        }
        offset()
    }};
}

pub struct ExecListIter<'a, T> {
    n: &'a exec_node,
    offset: usize,
    _marker: PhantomData<T>,
}

impl<'a, T> ExecListIter<'a, T> {
    fn new(l: &'a exec_list, offset: usize) -> Self {
        Self {
            n: &l.head_sentinel,
            offset: offset,
            _marker: PhantomData,
        }
    }
}

impl<'a, T: 'a> Iterator for ExecListIter<'a, T> {
    type Item = &'a T;

    fn next(&mut self) -> Option<Self::Item> {
        self.n = unsafe { &*self.n.next };
        if self.n.next.is_null() {
            None
        } else {
            let t: *const c_void = (self.n as *const exec_node).cast();
            Some(unsafe { &*(t.sub(self.offset).cast()) })
        }
    }
}

pub trait NirDef {
    fn parent_instr(&self) -> &nir_instr;
    fn components_read(&self) -> nir_component_mask_t;
    fn all_uses_are_fsat(&self) -> bool;
}

impl NirDef for nir_def {
    fn parent_instr(&self) -> &nir_instr {
        unsafe { NonNull::new(self.parent_instr).unwrap().as_ref() }
    }

    fn components_read(&self) -> nir_component_mask_t {
        unsafe { nir_def_components_read(self as *const _) }
    }

    fn all_uses_are_fsat(&self) -> bool {
        unsafe { nir_def_all_uses_are_fsat(self as *const _) }
    }
}

pub trait AsConst: NirValue {
    fn as_load_const(&self) -> Option<&nir_load_const_instr>;

    fn is_const(&self) -> bool {
        self.as_load_const().is_some()
    }

    fn comp_as_int(&self, comp: u8) -> Option<i64> {
        if let Some(load) = self.as_load_const() {
            assert!(comp < load.def.num_components);
            Some(unsafe {
                let comp = usize::from(comp);
                match self.bit_size() {
                    8 => load.values()[comp].i8_ as i64,
                    16 => load.values()[comp].i16_ as i64,
                    32 => load.values()[comp].i32_ as i64,
                    64 => load.values()[comp].i64_,
                    _ => panic!("Invalid bit size"),
                }
            })
        } else {
            None
        }
    }

    fn comp_as_uint(&self, comp: u8) -> Option<u64> {
        if let Some(load) = self.as_load_const() {
            assert!(comp < load.def.num_components);
            Some(unsafe {
                let comp = usize::from(comp);
                match self.bit_size() {
                    8 => load.values()[comp].u8_ as u64,
                    16 => load.values()[comp].u16_ as u64,
                    32 => load.values()[comp].u32_ as u64,
                    64 => load.values()[comp].u64_,
                    _ => panic!("Invalid bit size"),
                }
            })
        } else {
            None
        }
    }

    fn as_int(&self) -> Option<i64> {
        assert!(self.num_components() == 1);
        self.comp_as_int(0)
    }

    fn as_uint(&self) -> Option<u64> {
        assert!(self.num_components() == 1);
        self.comp_as_uint(0)
    }

    fn is_zero(&self) -> bool {
        self.num_components() == 1 && self.as_uint() == Some(0)
    }
}

impl AsConst for nir_def {
    fn as_load_const(&self) -> Option<&nir_load_const_instr> {
        self.parent_instr().as_load_const()
    }
}

impl AsConst for nir_src {
    fn as_load_const(&self) -> Option<&nir_load_const_instr> {
        self.as_def().parent_instr().as_load_const()
    }
}

pub trait AsDef {
    fn as_def<'a>(&'a self) -> &'a nir_def;
}

impl AsDef for nir_def {
    fn as_def<'a>(&'a self) -> &'a nir_def {
        self
    }
}

pub trait NirValue {
    fn bit_size(&self) -> u8;
    fn num_components(&self) -> u8;
}

impl<T: AsDef> NirValue for T {
    fn bit_size(&self) -> u8 {
        self.as_def().bit_size
    }

    fn num_components(&self) -> u8 {
        self.as_def().num_components
    }
}

impl AsDef for nir_src {
    fn as_def<'a>(&'a self) -> &'a nir_def {
        unsafe { &*self.ssa }
    }
}

pub trait NirSrcsAsSlice<S> {
    fn srcs_as_slice(&self) -> &[S];

    fn get_src(&self, idx: usize) -> &S {
        &self.srcs_as_slice()[idx]
    }
}

pub trait NirAluInstr {
    fn info(&self) -> &nir_op_info;
    fn src_components(&self, src_idx: u8) -> u8;
}

impl NirAluInstr for nir_alu_instr {
    fn info(&self) -> &nir_op_info {
        let info_idx: usize = self.op.try_into().unwrap();
        unsafe { &nir_op_infos[info_idx] }
    }

    fn src_components(&self, src_idx: u8) -> u8 {
        assert!(src_idx < self.info().num_inputs);
        unsafe {
            nir_ssa_alu_instr_src_components(self as *const _, src_idx.into())
                .try_into()
                .unwrap()
        }
    }
}

impl NirSrcsAsSlice<nir_alu_src> for nir_alu_instr {
    fn srcs_as_slice(&self) -> &[nir_alu_src] {
        unsafe {
            self.src
                .as_slice(self.info().num_inputs.try_into().unwrap())
        }
    }
}

impl AsDef for nir_alu_src {
    fn as_def<'a>(&'a self) -> &'a nir_def {
        self.src.as_def()
    }
}

pub trait NirAluInfo {
    fn name(&self) -> &'static str;
}

impl NirAluInfo for nir_op_info {
    fn name(&self) -> &'static str {
        unsafe { CStr::from_ptr(self.name).to_str().expect("Invalid UTF-8") }
    }
}

impl NirSrcsAsSlice<nir_tex_src> for nir_tex_instr {
    fn srcs_as_slice(&self) -> &[nir_tex_src] {
        unsafe { std::slice::from_raw_parts(self.src, self.num_srcs as usize) }
    }
}

pub trait NirIntrinsicInstr {
    fn info(&self) -> &nir_intrinsic_info;
    fn get_const_index(&self, name: u32) -> u32;
    fn base(&self) -> i32;
    fn range_base(&self) -> i32;
    fn range(&self) -> i32;
    fn write_mask(&self) -> u32;
    fn stream_id(&self) -> u32;
    fn component(&self) -> u32;
    fn interp_mode(&self) -> u32;
    fn reduction_op(&self) -> nir_op;
    fn cluster_size(&self) -> u32;
    fn image_dim(&self) -> glsl_sampler_dim;
    fn image_array(&self) -> bool;
    fn access(&self) -> gl_access_qualifier;
    fn align(&self) -> u32;
    fn align_mul(&self) -> u32;
    fn align_offset(&self) -> u32;
    fn execution_scope(&self) -> mesa_scope;
    fn memory_scope(&self) -> mesa_scope;
    fn memory_semantics(&self) -> nir_memory_semantics;
    fn memory_modes(&self) -> nir_variable_mode;
    fn flags(&self) -> u32;
    fn atomic_op(&self) -> nir_atomic_op;
}

impl NirIntrinsicInstr for nir_intrinsic_instr {
    fn info(&self) -> &nir_intrinsic_info {
        let info_idx: usize = self.intrinsic.try_into().unwrap();
        unsafe { &nir_intrinsic_infos[info_idx] }
    }

    fn get_const_index(&self, name: u32) -> u32 {
        let name: usize = name.try_into().unwrap();
        let idx = self.info().index_map[name];
        assert!(idx > 0);
        self.const_index[usize::from(idx - 1)] as u32
    }

    fn base(&self) -> i32 {
        self.get_const_index(NIR_INTRINSIC_BASE) as i32
    }

    fn range_base(&self) -> i32 {
        self.get_const_index(NIR_INTRINSIC_RANGE_BASE) as i32
    }

    fn range(&self) -> i32 {
        self.get_const_index(NIR_INTRINSIC_RANGE) as i32
    }

    fn write_mask(&self) -> u32 {
        self.get_const_index(NIR_INTRINSIC_WRITE_MASK)
    }

    fn stream_id(&self) -> u32 {
        self.get_const_index(NIR_INTRINSIC_STREAM_ID)
    }

    fn component(&self) -> u32 {
        self.get_const_index(NIR_INTRINSIC_COMPONENT)
    }

    fn interp_mode(&self) -> u32 {
        self.get_const_index(NIR_INTRINSIC_INTERP_MODE)
    }

    fn reduction_op(&self) -> nir_op {
        self.get_const_index(NIR_INTRINSIC_REDUCTION_OP) as nir_op
    }

    fn cluster_size(&self) -> u32 {
        self.get_const_index(NIR_INTRINSIC_CLUSTER_SIZE)
    }

    fn image_dim(&self) -> glsl_sampler_dim {
        self.get_const_index(NIR_INTRINSIC_IMAGE_DIM) as glsl_sampler_dim
    }

    fn image_array(&self) -> bool {
        self.get_const_index(NIR_INTRINSIC_IMAGE_ARRAY) != 0
    }

    fn access(&self) -> gl_access_qualifier {
        self.get_const_index(NIR_INTRINSIC_ACCESS) as gl_access_qualifier
    }

    fn align(&self) -> u32 {
        let mul = self.align_mul();
        let offset = self.align_offset();
        assert!(offset < mul);
        if offset > 0 {
            1 << offset.trailing_zeros()
        } else {
            mul
        }
    }

    fn align_mul(&self) -> u32 {
        self.get_const_index(NIR_INTRINSIC_ALIGN_MUL)
    }

    fn align_offset(&self) -> u32 {
        self.get_const_index(NIR_INTRINSIC_ALIGN_OFFSET)
    }

    fn execution_scope(&self) -> mesa_scope {
        self.get_const_index(NIR_INTRINSIC_EXECUTION_SCOPE)
    }

    fn memory_scope(&self) -> mesa_scope {
        self.get_const_index(NIR_INTRINSIC_MEMORY_SCOPE)
    }

    fn memory_semantics(&self) -> nir_memory_semantics {
        self.get_const_index(NIR_INTRINSIC_MEMORY_SEMANTICS)
    }

    fn memory_modes(&self) -> nir_variable_mode {
        self.get_const_index(NIR_INTRINSIC_MEMORY_MODES)
    }

    fn flags(&self) -> u32 {
        self.get_const_index(NIR_INTRINSIC_FLAGS)
    }

    fn atomic_op(&self) -> nir_atomic_op {
        self.get_const_index(NIR_INTRINSIC_ATOMIC_OP) as nir_atomic_op
    }
}

impl NirSrcsAsSlice<nir_src> for nir_intrinsic_instr {
    fn srcs_as_slice(&self) -> &[nir_src] {
        unsafe {
            let info = nir_intrinsic_infos[self.intrinsic as usize];
            self.src.as_slice(info.num_srcs as usize)
        }
    }
}

pub trait NirIntrinsicInfo {
    fn name(&self) -> &'static str;
}

impl NirIntrinsicInfo for nir_intrinsic_info {
    fn name(&self) -> &'static str {
        unsafe { CStr::from_ptr(self.name).to_str().expect("Invalid UTF-8") }
    }
}

pub trait NirLoadConstInstr {
    fn values(&self) -> &[nir_const_value];
}

impl NirLoadConstInstr for nir_load_const_instr {
    fn values(&self) -> &[nir_const_value] {
        unsafe { self.value.as_slice(self.def.num_components as usize) }
    }
}

pub trait NirPhiSrc {
    fn pred(&self) -> &nir_block;
}

impl NirPhiSrc for nir_phi_src {
    fn pred(&self) -> &nir_block {
        unsafe { NonNull::new(self.pred).unwrap().as_ref() }
    }
}

pub trait NirPhiInstr {
    fn iter_srcs(&self) -> ExecListIter<nir_phi_src>;
}

impl NirPhiInstr for nir_phi_instr {
    fn iter_srcs(&self) -> ExecListIter<nir_phi_src> {
        ExecListIter::new(&self.srcs, offset_of!(nir_phi_src, node))
    }
}

pub trait NirInstr {
    fn as_alu(&self) -> Option<&nir_alu_instr>;
    fn as_jump(&self) -> Option<&nir_jump_instr>;
    fn as_tex(&self) -> Option<&nir_tex_instr>;
    fn as_intrinsic(&self) -> Option<&nir_intrinsic_instr>;
    fn as_load_const(&self) -> Option<&nir_load_const_instr>;
    fn as_undef(&self) -> Option<&nir_undef_instr>;
    fn as_phi(&self) -> Option<&nir_phi_instr>;
}

impl NirInstr for nir_instr {
    fn as_alu(&self) -> Option<&nir_alu_instr> {
        if self.type_ == nir_instr_type_alu {
            let p = self as *const nir_instr;
            Some(unsafe { &*(p as *const nir_alu_instr) })
        } else {
            None
        }
    }

    fn as_jump(&self) -> Option<&nir_jump_instr> {
        if self.type_ == nir_instr_type_jump {
            let p = self as *const nir_instr;
            Some(unsafe { &*(p as *const nir_jump_instr) })
        } else {
            None
        }
    }

    fn as_tex(&self) -> Option<&nir_tex_instr> {
        if self.type_ == nir_instr_type_tex {
            let p = self as *const nir_instr;
            Some(unsafe { &*(p as *const nir_tex_instr) })
        } else {
            None
        }
    }

    fn as_intrinsic(&self) -> Option<&nir_intrinsic_instr> {
        if self.type_ == nir_instr_type_intrinsic {
            let p = self as *const nir_instr;
            Some(unsafe { &*(p as *const nir_intrinsic_instr) })
        } else {
            None
        }
    }

    fn as_load_const(&self) -> Option<&nir_load_const_instr> {
        if self.type_ == nir_instr_type_load_const {
            let p = self as *const nir_instr;
            Some(unsafe { &*(p as *const nir_load_const_instr) })
        } else {
            None
        }
    }

    fn as_undef(&self) -> Option<&nir_undef_instr> {
        if self.type_ == nir_instr_type_undef {
            let p = self as *const nir_instr;
            Some(unsafe { &*(p as *const nir_undef_instr) })
        } else {
            None
        }
    }

    fn as_phi(&self) -> Option<&nir_phi_instr> {
        if self.type_ == nir_instr_type_phi {
            let p = self as *const nir_instr;
            Some(unsafe { &*(p as *const nir_phi_instr) })
        } else {
            None
        }
    }
}

pub trait NirBlock {
    fn iter_instr_list(&self) -> ExecListIter<nir_instr>;
    fn successors(&self) -> [Option<&nir_block>; 2];
    fn following_if(&self) -> Option<&nir_if>;
}

impl NirBlock for nir_block {
    fn iter_instr_list(&self) -> ExecListIter<nir_instr> {
        ExecListIter::new(&self.instr_list, offset_of!(nir_instr, node))
    }

    fn successors(&self) -> [Option<&nir_block>; 2] {
        [
            NonNull::new(self.successors[0]).map(|b| unsafe { b.as_ref() }),
            NonNull::new(self.successors[1]).map(|b| unsafe { b.as_ref() }),
        ]
    }

    fn following_if(&self) -> Option<&nir_if> {
        let self_ptr = self as *const _ as *mut _;
        unsafe { nir_block_get_following_if(self_ptr).as_ref() }
    }
}

pub trait NirIf {
    fn first_then_block(&self) -> &nir_block;
    fn first_else_block(&self) -> &nir_block;
    fn iter_then_list(&self) -> ExecListIter<nir_cf_node>;
    fn iter_else_list(&self) -> ExecListIter<nir_cf_node>;
}

impl NirIf for nir_if {
    fn first_then_block(&self) -> &nir_block {
        self.iter_then_list().next().unwrap().as_block().unwrap()
    }
    fn first_else_block(&self) -> &nir_block {
        self.iter_else_list().next().unwrap().as_block().unwrap()
    }
    fn iter_then_list(&self) -> ExecListIter<nir_cf_node> {
        ExecListIter::new(&self.then_list, offset_of!(nir_cf_node, node))
    }
    fn iter_else_list(&self) -> ExecListIter<nir_cf_node> {
        ExecListIter::new(&self.else_list, offset_of!(nir_cf_node, node))
    }
}

pub trait NirLoop {
    fn iter_body(&self) -> ExecListIter<nir_cf_node>;
}

impl NirLoop for nir_loop {
    fn iter_body(&self) -> ExecListIter<nir_cf_node> {
        ExecListIter::new(&self.body, offset_of!(nir_cf_node, node))
    }
}

pub trait NirCfNode {
    fn as_block(&self) -> Option<&nir_block>;
    fn as_if(&self) -> Option<&nir_if>;
    fn as_loop(&self) -> Option<&nir_loop>;
}

impl NirCfNode for nir_cf_node {
    fn as_block(&self) -> Option<&nir_block> {
        if self.type_ == nir_cf_node_block {
            Some(unsafe { &*(self as *const nir_cf_node as *const nir_block) })
        } else {
            None
        }
    }

    fn as_if(&self) -> Option<&nir_if> {
        if self.type_ == nir_cf_node_if {
            Some(unsafe { &*(self as *const nir_cf_node as *const nir_if) })
        } else {
            None
        }
    }

    fn as_loop(&self) -> Option<&nir_loop> {
        if self.type_ == nir_cf_node_loop {
            Some(unsafe { &*(self as *const nir_cf_node as *const nir_loop) })
        } else {
            None
        }
    }
}

pub trait NirFunctionImpl {
    fn iter_body(&self) -> ExecListIter<nir_cf_node>;
    fn end_block(&self) -> &nir_block;
    fn function(&self) -> &nir_function;
}

impl NirFunctionImpl for nir_function_impl {
    fn iter_body(&self) -> ExecListIter<nir_cf_node> {
        ExecListIter::new(&self.body, offset_of!(nir_cf_node, node))
    }

    fn end_block(&self) -> &nir_block {
        unsafe { NonNull::new(self.end_block).unwrap().as_ref() }
    }

    fn function(&self) -> &nir_function {
        unsafe { self.function.as_ref() }.unwrap()
    }
}

pub trait NirFunction {
    fn get_impl(&self) -> Option<&nir_function_impl>;
}

impl NirFunction for nir_function {
    fn get_impl(&self) -> Option<&nir_function_impl> {
        unsafe { self.impl_.as_ref() }
    }
}

pub trait NirShader {
    fn iter_functions(&self) -> ExecListIter<nir_function>;
    fn iter_variables(&self) -> ExecListIter<nir_variable>;
}

impl NirShader for nir_shader {
    fn iter_functions(&self) -> ExecListIter<nir_function> {
        ExecListIter::new(&self.functions, offset_of!(nir_function, node))
    }

    fn iter_variables(&self) -> ExecListIter<nir_variable> {
        ExecListIter::new(&self.variables, offset_of!(nir_variable, node))
    }
}
