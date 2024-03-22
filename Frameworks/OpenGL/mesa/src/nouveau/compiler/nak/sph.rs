// Copyright © 2023 Collabora, Ltd.
// SPDX-License-Identifier: MIT

extern crate bitview;

use crate::ir::{ShaderInfo, ShaderIoInfo, ShaderStageInfo};
use bitview::{
    BitMutView, BitMutViewable, BitView, BitViewable, SetBit, SetField,
    SetFieldU64,
};
use nak_bindings::*;
use std::ops::Range;

pub const _FERMI_SHADER_HEADER_SIZE: usize = 20;
pub const TURING_SHADER_HEADER_SIZE: usize = 32;
pub const CURRENT_MAX_SHADER_HEADER_SIZE: usize = TURING_SHADER_HEADER_SIZE;

type SubSPHView<'a> = BitMutView<'a, [u32; CURRENT_MAX_SHADER_HEADER_SIZE]>;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum ShaderType {
    Vertex,
    TessellationInit,
    Tessellation,
    Geometry,
    Fragment,
}

impl From<&ShaderStageInfo> for ShaderType {
    fn from(value: &ShaderStageInfo) -> Self {
        match value {
            ShaderStageInfo::Vertex => ShaderType::Vertex,
            ShaderStageInfo::Fragment => ShaderType::Fragment,
            ShaderStageInfo::Geometry(_) => ShaderType::Geometry,
            ShaderStageInfo::TessellationInit(_) => {
                ShaderType::TessellationInit
            }
            ShaderStageInfo::Tessellation => ShaderType::Tessellation,
            _ => panic!("Invalid ShaderStageInfo {:?}", value),
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum OutputTopology {
    PointList,
    LineStrip,
    TriangleStrip,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum PixelImap {
    Unused,
    Constant,
    Perspective,
    ScreenLinear,
}

impl From<PixelImap> for u8 {
    fn from(value: PixelImap) -> u8 {
        match value {
            PixelImap::Unused => 0,
            PixelImap::Constant => 1,
            PixelImap::Perspective => 2,
            PixelImap::ScreenLinear => 3,
        }
    }
}

#[derive(Debug)]
pub struct ShaderProgramHeader {
    pub data: [u32; CURRENT_MAX_SHADER_HEADER_SIZE],
    shader_type: ShaderType,
    sm: u8,
}

impl BitViewable for ShaderProgramHeader {
    fn bits(&self) -> usize {
        BitView::new(&self.data).bits()
    }

    fn get_bit_range_u64(&self, range: Range<usize>) -> u64 {
        BitView::new(&self.data).get_bit_range_u64(range)
    }
}

impl BitMutViewable for ShaderProgramHeader {
    fn set_bit_range_u64(&mut self, range: Range<usize>, val: u64) {
        BitMutView::new(&mut self.data).set_bit_range_u64(range, val);
    }
}

impl SetFieldU64 for ShaderProgramHeader {
    fn set_field_u64(&mut self, range: Range<usize>, val: u64) {
        BitMutView::new(&mut self.data).set_field_u64(range, val);
    }
}

impl ShaderProgramHeader {
    pub fn new(shader_type: ShaderType, sm: u8) -> Self {
        let mut res = Self {
            data: [0; CURRENT_MAX_SHADER_HEADER_SIZE],
            shader_type,
            sm,
        };

        let sph_type = if shader_type == ShaderType::Fragment {
            2
        } else {
            1
        };

        let sph_version = 3;
        res.set_sph_type(sph_type, sph_version);
        res.set_shader_type(shader_type);

        res
    }

    #[inline]
    fn common_word0(&mut self) -> SubSPHView<'_> {
        BitMutView::new_subset(&mut self.data, 0..32)
    }

    #[inline]
    fn common_word1(&mut self) -> SubSPHView<'_> {
        BitMutView::new_subset(&mut self.data, 32..64)
    }

    #[inline]
    fn common_word2(&mut self) -> SubSPHView<'_> {
        BitMutView::new_subset(&mut self.data, 64..96)
    }

    #[inline]
    fn common_word3<'a>(&mut self) -> SubSPHView<'_> {
        BitMutView::new_subset(&mut self.data, 96..128)
    }

    #[inline]
    fn common_word4(&mut self) -> SubSPHView<'_> {
        BitMutView::new_subset(&mut self.data, 128..160)
    }

    #[inline]
    fn imap_system_values_ab(&mut self) -> SubSPHView<'_> {
        BitMutView::new_subset(&mut self.data, 160..192)
    }

    #[inline]
    fn imap_g_vtg(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type != ShaderType::Fragment);

        BitMutView::new_subset(&mut self.data, 192..320)
    }

    #[inline]
    fn imap_g_ps(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type == ShaderType::Fragment);

        BitMutView::new_subset(&mut self.data, 192..448)
    }

    #[inline]
    fn imap_system_values_c(&mut self) -> SubSPHView<'_> {
        if self.shader_type == ShaderType::Fragment {
            BitMutView::new_subset(&mut self.data, 464..480)
        } else {
            BitMutView::new_subset(&mut self.data, 336..352)
        }
    }

    #[inline]
    fn imap_system_values_d_vtg(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type != ShaderType::Fragment);
        BitMutView::new_subset(&mut self.data, 392..400)
    }

    #[inline]
    fn omap_system_values_ab(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type != ShaderType::Fragment);
        BitMutView::new_subset(&mut self.data, 400..432)
    }

    #[inline]
    fn omap_g(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type != ShaderType::Fragment);

        BitMutView::new_subset(&mut self.data, 432..560)
    }

    #[inline]
    fn omap_system_values_c(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type != ShaderType::Fragment);
        BitMutView::new_subset(&mut self.data, 576..592)
    }

    #[inline]
    fn imap_system_values_d_ps(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type == ShaderType::Fragment);
        BitMutView::new_subset(&mut self.data, 560..576)
    }

    #[inline]
    fn omap_target(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type == ShaderType::Fragment);

        BitMutView::new_subset(&mut self.data, 576..608)
    }

    #[inline]
    fn omap_system_values_d_vtg(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type != ShaderType::Fragment);
        BitMutView::new_subset(&mut self.data, 632..640)
    }

    #[inline]
    fn set_sph_type(&mut self, sph_type: u8, sph_version: u8) {
        let mut common_word0 = self.common_word0();

        common_word0.set_field(0..5, sph_type);
        common_word0.set_field(5..10, sph_version);
    }

    #[inline]
    fn set_shader_type(&mut self, shader_type: ShaderType) {
        self.common_word0().set_field(
            10..14,
            match shader_type {
                ShaderType::Vertex => 1_u8,
                ShaderType::TessellationInit => 2_u8,
                ShaderType::Tessellation => 3_u8,
                ShaderType::Geometry => 4_u8,
                ShaderType::Fragment => 5_u8,
            },
        );
    }

    #[inline]
    pub fn set_multiple_render_target_enable(&mut self, mrt_enable: bool) {
        self.common_word0().set_bit(14, mrt_enable);
    }

    #[inline]
    pub fn set_kills_pixels(&mut self, kills_pixels: bool) {
        self.common_word0().set_bit(15, kills_pixels);
    }

    #[inline]
    pub fn set_does_global_store(&mut self, does_global_store: bool) {
        self.common_word0().set_bit(16, does_global_store);
    }

    #[inline]
    pub fn set_sass_version(&mut self, sass_version: u8) {
        self.common_word0().set_field(17..21, sass_version);
    }

    #[inline]
    pub fn set_gs_passthrough_enable(&mut self, gs_passthrough_enable: bool) {
        assert!(self.shader_type == ShaderType::Geometry);
        self.common_word0().set_bit(24, gs_passthrough_enable);
    }

    #[inline]
    pub fn set_does_load_or_store(&mut self, does_load_or_store: bool) {
        self.common_word0().set_bit(26, does_load_or_store);
    }

    #[inline]
    pub fn set_does_fp64(&mut self, does_fp64: bool) {
        self.common_word0().set_bit(27, does_fp64);
    }

    #[inline]
    pub fn set_stream_out_mask(&mut self, stream_out_mask: u8) {
        self.common_word0().set_field(28..32, stream_out_mask);
    }

    #[inline]
    pub fn set_shader_local_memory_size(
        &mut self,
        shader_local_memory_size: u64,
    ) {
        assert!(shader_local_memory_size <= 0xffffffffffff);
        assert!(shader_local_memory_size % 0x10 == 0);

        let low = (shader_local_memory_size & 0xffffff) as u32;
        let high = ((shader_local_memory_size >> 32) & 0xffffff) as u32;

        self.common_word1().set_field(0..24, low);
        self.common_word2().set_field(0..24, high);
    }

    #[inline]
    pub fn set_per_patch_attribute_count(
        &mut self,
        per_patch_attribute_count: u8,
    ) {
        assert!(self.shader_type == ShaderType::TessellationInit);

        self.common_word1()
            .set_field(24..32, per_patch_attribute_count);

        // Maxwell changed that encoding.
        if self.sm > 35 {
            self.common_word3()
                .set_field(28..32, per_patch_attribute_count & 0xf);
            self.common_word4()
                .set_field(20..24, per_patch_attribute_count >> 4);
        }
    }

    #[inline]
    pub fn set_threads_per_input_primitive(
        &mut self,
        threads_per_input_primitive: u8,
    ) {
        self.common_word2()
            .set_field(24..32, threads_per_input_primitive);
    }

    #[inline]
    #[allow(dead_code)]
    pub fn set_shader_local_memory_crs_size(
        &mut self,
        shader_local_memory_crs_size: u32,
    ) {
        assert!(shader_local_memory_crs_size <= 0xffffff);
        self.common_word3()
            .set_field(0..24, shader_local_memory_crs_size);
    }

    #[inline]
    pub fn set_output_topology(&mut self, output_topology: OutputTopology) {
        self.common_word3().set_field(
            24..28,
            match output_topology {
                OutputTopology::PointList => 1_u8,
                OutputTopology::LineStrip => 6_u8,
                OutputTopology::TriangleStrip => 7_u8,
            },
        );
    }

    #[inline]
    pub fn set_max_output_vertex_count(
        &mut self,
        max_output_vertex_count: u16,
    ) {
        assert!(max_output_vertex_count <= 0xfff);
        self.common_word4()
            .set_field(0..12, max_output_vertex_count);
    }

    #[inline]
    pub fn set_store_req_start(&mut self, store_req_start: u8) {
        self.common_word4().set_field(12..20, store_req_start);
    }

    #[inline]
    pub fn set_store_req_end(&mut self, store_req_end: u8) {
        self.common_word4().set_field(24..32, store_req_end);
    }

    pub fn set_imap_system_values_ab(&mut self, val: u32) {
        self.imap_system_values_ab().set_field(0..32, val);
    }

    pub fn set_imap_system_values_c(&mut self, val: u16) {
        self.imap_system_values_c().set_field(0..16, val);
    }

    pub fn set_imap_system_values_d_vtg(&mut self, val: u8) {
        assert!(self.shader_type != ShaderType::Fragment);
        self.imap_system_values_d_vtg().set_field(0..8, val);
    }

    #[inline]
    pub fn set_imap_vector_ps(&mut self, index: usize, value: PixelImap) {
        assert!(index < 128);
        assert!(self.shader_type == ShaderType::Fragment);

        self.imap_g_ps()
            .set_field(index * 2..(index + 1) * 2, u8::from(value));
    }

    #[inline]
    pub fn set_imap_system_values_d_ps(
        &mut self,
        index: usize,
        value: PixelImap,
    ) {
        assert!(index < 8);
        assert!(self.shader_type == ShaderType::Fragment);

        self.imap_system_values_d_ps()
            .set_field(index * 2..(index + 1) * 2, u8::from(value));
    }

    #[inline]
    pub fn set_imap_vector_vtg(&mut self, index: usize, value: u32) {
        assert!(index < 4);
        assert!(self.shader_type != ShaderType::Fragment);

        self.imap_g_vtg()
            .set_field(index * 32..(index + 1) * 32, value);
    }

    #[inline]
    pub fn set_omap_system_values_ab(&mut self, val: u32) {
        self.omap_system_values_ab().set_field(0..32, val);
    }

    #[inline]
    pub fn set_omap_system_values_c(&mut self, val: u16) {
        self.omap_system_values_c().set_field(0..16, val);
    }

    pub fn set_omap_system_values_d_vtg(&mut self, val: u8) {
        assert!(self.shader_type != ShaderType::Fragment);
        self.omap_system_values_d_vtg().set_field(0..8, val);
    }

    #[inline]
    pub fn set_omap_vector(&mut self, index: usize, value: u32) {
        assert!(index < 4);
        assert!(self.shader_type != ShaderType::Fragment);

        self.omap_g().set_field(index * 32..(index + 1) * 32, value);
    }

    #[inline]
    pub fn set_omap_targets(&mut self, value: u32) {
        self.omap_target().set_field(0..32, value)
    }

    #[inline]
    pub fn set_omap_sample_mask(&mut self, sample_mask: bool) {
        assert!(self.shader_type == ShaderType::Fragment);
        self.set_bit(608, sample_mask);
    }

    #[inline]
    pub fn set_omap_depth(&mut self, depth: bool) {
        assert!(self.shader_type == ShaderType::Fragment);
        self.set_bit(609, depth);
    }

    #[inline]
    pub fn set_does_interlock(&mut self, does_interlock: bool) {
        assert!(self.shader_type == ShaderType::Fragment);
        self.set_bit(610, does_interlock);
    }

    // TODO: This seems always set on fragment shaders, figure out what this is for.
    #[inline]
    pub fn set_unknown_bit611(&mut self, value: bool) {
        assert!(self.shader_type == ShaderType::Fragment);
        self.set_bit(611, value);
    }

    #[inline]
    fn pervertex_imap_vector_ps(&mut self) -> SubSPHView<'_> {
        assert!(self.shader_type == ShaderType::Fragment);

        BitMutView::new_subset(&mut self.data, 672..800)
    }

    #[inline]
    pub fn set_pervertex_imap_vector(&mut self, index: usize, value: u32) {
        assert!(index < 4);
        assert!(self.shader_type == ShaderType::Fragment);

        self.pervertex_imap_vector_ps()
            .set_field(index * 32..(index + 1) * 32, value);
    }
}

pub fn encode_header(
    shader_info: &ShaderInfo,
    fs_key: Option<&nak_fs_key>,
) -> [u32; CURRENT_MAX_SHADER_HEADER_SIZE] {
    if let ShaderStageInfo::Compute(_) = shader_info.stage {
        return [0_u32; CURRENT_MAX_SHADER_HEADER_SIZE];
    }

    let mut sph = ShaderProgramHeader::new(
        ShaderType::from(&shader_info.stage),
        shader_info.sm,
    );

    sph.set_sass_version(1);
    sph.set_does_load_or_store(shader_info.uses_global_mem);
    sph.set_does_global_store(shader_info.writes_global_mem);
    sph.set_does_fp64(shader_info.uses_fp64);

    let slm_size = shader_info.slm_size.next_multiple_of(16);
    sph.set_shader_local_memory_size(slm_size.into());

    match &shader_info.io {
        ShaderIoInfo::Vtg(io) => {
            sph.set_imap_system_values_ab(io.sysvals_in.ab);
            sph.set_imap_system_values_c(io.sysvals_in.c);
            sph.set_imap_system_values_d_vtg(io.sysvals_in_d);

            for (index, value) in io.attr_in.iter().enumerate() {
                sph.set_imap_vector_vtg(index, *value);
            }

            for (index, value) in io.attr_out.iter().enumerate() {
                sph.set_omap_vector(index, *value);
            }

            sph.set_store_req_start(io.store_req_start);
            sph.set_store_req_end(io.store_req_end);

            sph.set_omap_system_values_ab(io.sysvals_out.ab);
            sph.set_omap_system_values_c(io.sysvals_out.c);
            sph.set_omap_system_values_d_vtg(io.sysvals_out_d);
        }
        ShaderIoInfo::Fragment(io) => {
            sph.set_imap_system_values_ab(io.sysvals_in.ab);
            sph.set_imap_system_values_c(io.sysvals_in.c);

            for (index, imap) in io.sysvals_in_d.iter().enumerate() {
                sph.set_imap_system_values_d_ps(index, *imap);
            }

            for (index, imap) in io.attr_in.iter().enumerate() {
                sph.set_imap_vector_ps(index, *imap);
            }

            let zs_self_dep = fs_key.map_or(false, |key| key.zs_self_dep);

            sph.set_multiple_render_target_enable(io.writes_color > 0xf);
            sph.set_kills_pixels(io.uses_kill || zs_self_dep);
            sph.set_omap_sample_mask(io.writes_sample_mask);
            sph.set_omap_depth(io.writes_depth);
            sph.set_omap_targets(io.writes_color);
            sph.set_does_interlock(io.does_interlock);

            for (index, value) in io.barycentric_attr_in.iter().enumerate() {
                sph.set_pervertex_imap_vector(index, *value);
            }
        }
        _ => {}
    }

    match &shader_info.stage {
        ShaderStageInfo::Geometry(stage) => {
            sph.set_gs_passthrough_enable(stage.passthrough_enable);
            sph.set_stream_out_mask(stage.stream_out_mask);
            sph.set_threads_per_input_primitive(
                stage.threads_per_input_primitive,
            );
            sph.set_output_topology(stage.output_topology);
            sph.set_max_output_vertex_count(stage.max_output_vertex_count);
        }
        ShaderStageInfo::TessellationInit(stage) => {
            sph.set_per_patch_attribute_count(stage.per_patch_attribute_count);
            sph.set_threads_per_input_primitive(stage.threads_per_patch);
        }
        ShaderStageInfo::Compute(_) => {
            panic!("Compute shaders don't have a SPH!")
        }
        _ => {}
    };

    sph.data
}
