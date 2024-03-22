use crate::api::icd::*;
use crate::api::types::*;
use crate::api::util::*;
use crate::core::context::*;
use crate::core::device::*;
use crate::core::format::*;
use crate::core::gl::*;
use crate::core::queue::*;
use crate::core::util::*;
use crate::impl_cl_type_trait;

use mesa_rust::pipe::context::*;
use mesa_rust::pipe::resource::*;
use mesa_rust::pipe::screen::ResourceType;
use mesa_rust::pipe::transfer::*;
use mesa_rust_gen::*;
use mesa_rust_util::math::*;
use mesa_rust_util::properties::Properties;
use rusticl_opencl_gen::*;

use std::cmp;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::convert::TryInto;
use std::mem;
use std::mem::size_of;
use std::ops::AddAssign;
use std::os::raw::c_void;
use std::ptr;
use std::sync::Arc;
use std::sync::Mutex;
use std::sync::MutexGuard;

struct MappingTransfer {
    tx: PipeTransfer,
    shadow: Option<PipeResource>,
    pending: u32,
}

impl MappingTransfer {
    fn new(tx: PipeTransfer, shadow: Option<PipeResource>) -> Self {
        MappingTransfer {
            tx: tx,
            shadow: shadow,
            pending: 1,
        }
    }
}

struct Mappings {
    tx: HashMap<&'static Device, MappingTransfer>,
    maps: HashMap<*mut c_void, u32>,
}

impl Mappings {
    fn new() -> Mutex<Self> {
        Mutex::new(Mappings {
            tx: HashMap::new(),
            maps: HashMap::new(),
        })
    }

    fn mark_pending(&mut self, dev: &Device) {
        self.tx.get_mut(dev).unwrap().pending += 1;
    }

    fn unmark_pending(&mut self, dev: &Device) {
        if let Some(tx) = self.tx.get_mut(dev) {
            tx.pending -= 1;
        }
    }

    fn increase_ref(&mut self, dev: &Device, ptr: *mut c_void) -> bool {
        let res = self.maps.is_empty();
        *self.maps.entry(ptr).or_default() += 1;
        self.unmark_pending(dev);
        res
    }

    fn decrease_ref(&mut self, ptr: *mut c_void, dev: &Device) -> (bool, Option<&PipeResource>) {
        if let Some(r) = self.maps.get_mut(&ptr) {
            *r -= 1;

            if *r == 0 {
                self.maps.remove(&ptr);
            }

            if self.maps.is_empty() {
                let shadow = self.tx.get(dev).and_then(|tx| tx.shadow.as_ref());
                return (true, shadow);
            }
        }
        (false, None)
    }

    fn clean_up_tx(&mut self, dev: &Device, ctx: &PipeContext) {
        if self.maps.is_empty() {
            if let Some(tx) = self.tx.get(&dev) {
                if tx.pending == 0 {
                    self.tx.remove(dev).unwrap().tx.with_ctx(ctx);
                }
            }
        }
    }
}

pub struct Mem {
    pub base: CLObjectBase<CL_INVALID_MEM_OBJECT>,
    pub context: Arc<Context>,
    pub parent: Option<Arc<Mem>>,
    pub mem_type: cl_mem_object_type,
    pub flags: cl_mem_flags,
    pub size: usize,
    pub offset: usize,
    pub host_ptr: *mut c_void,
    pub image_format: cl_image_format,
    pub pipe_format: pipe_format,
    pub image_desc: cl_image_desc,
    pub image_elem_size: u8,
    pub props: Vec<cl_mem_properties>,
    pub cbs: Mutex<Vec<MemCB>>,
    pub gl_obj: Option<GLObject>,
    res: Option<HashMap<&'static Device, Arc<PipeResource>>>,
    maps: Mutex<Mappings>,
}

impl_cl_type_trait!(cl_mem, Mem, CL_INVALID_MEM_OBJECT);

pub trait CLImageDescInfo {
    fn type_info(&self) -> (u8, bool);
    fn pixels(&self) -> usize;
    fn bx(&self) -> CLResult<pipe_box>;
    fn row_pitch(&self) -> CLResult<u32>;
    fn slice_pitch(&self) -> usize;
    fn width(&self) -> CLResult<u32>;
    fn height(&self) -> CLResult<u32>;
    fn size(&self) -> CLVec<usize>;

    fn dims(&self) -> u8 {
        self.type_info().0
    }

    fn dims_with_array(&self) -> u8 {
        let array: u8 = self.is_array().into();
        self.dims() + array
    }

    fn has_slice(&self) -> bool {
        self.dims() == 3 || self.is_array()
    }

    fn is_array(&self) -> bool {
        self.type_info().1
    }
}

impl CLImageDescInfo for cl_image_desc {
    fn type_info(&self) -> (u8, bool) {
        match self.image_type {
            CL_MEM_OBJECT_IMAGE1D | CL_MEM_OBJECT_IMAGE1D_BUFFER => (1, false),
            CL_MEM_OBJECT_IMAGE1D_ARRAY => (1, true),
            CL_MEM_OBJECT_IMAGE2D => (2, false),
            CL_MEM_OBJECT_IMAGE2D_ARRAY => (2, true),
            CL_MEM_OBJECT_IMAGE3D => (3, false),
            _ => panic!("unknown image_type {:x}", self.image_type),
        }
    }

    fn pixels(&self) -> usize {
        let mut res = self.image_width;
        let dims = self.dims();

        if dims > 1 {
            res *= self.image_height;
        }

        if dims > 2 {
            res *= self.image_depth;
        }

        if self.is_array() {
            res *= self.image_array_size;
        }

        res
    }

    fn size(&self) -> CLVec<usize> {
        let mut height = cmp::max(self.image_height, 1);
        let mut depth = cmp::max(self.image_depth, 1);

        match self.image_type {
            CL_MEM_OBJECT_IMAGE1D_ARRAY => height = self.image_array_size,
            CL_MEM_OBJECT_IMAGE2D_ARRAY => depth = self.image_array_size,
            _ => {}
        }

        CLVec::new([self.image_width, height, depth])
    }

    fn bx(&self) -> CLResult<pipe_box> {
        create_pipe_box(CLVec::default(), self.size(), self.image_type)
    }

    fn row_pitch(&self) -> CLResult<u32> {
        self.image_row_pitch
            .try_into()
            .map_err(|_| CL_OUT_OF_HOST_MEMORY)
    }

    fn slice_pitch(&self) -> usize {
        self.image_slice_pitch
    }

    fn width(&self) -> CLResult<u32> {
        self.image_width
            .try_into()
            .map_err(|_| CL_OUT_OF_HOST_MEMORY)
    }

    fn height(&self) -> CLResult<u32> {
        self.image_height
            .try_into()
            .map_err(|_| CL_OUT_OF_HOST_MEMORY)
    }
}

fn sw_copy(
    src: *const c_void,
    dst: *mut c_void,
    region: &CLVec<usize>,
    src_origin: &CLVec<usize>,
    src_row_pitch: usize,
    src_slice_pitch: usize,
    dst_origin: &CLVec<usize>,
    dst_row_pitch: usize,
    dst_slice_pitch: usize,
    pixel_size: u8,
) {
    for z in 0..region[2] {
        for y in 0..region[1] {
            unsafe {
                ptr::copy_nonoverlapping(
                    src.add(
                        (*src_origin + [0, y, z])
                            * [pixel_size as usize, src_row_pitch, src_slice_pitch],
                    ),
                    dst.add(
                        (*dst_origin + [0, y, z])
                            * [pixel_size as usize, dst_row_pitch, dst_slice_pitch],
                    ),
                    region[0] * pixel_size as usize,
                )
            };
        }
    }
}

/// helper function to determine if we can just map the resource in question or if we have to go
/// through a shdow buffer to let the CPU access the resources memory
fn can_map_directly(dev: &Device, res: &PipeResource) -> bool {
    // there are two aprts to this check:
    //   1. is the resource located in system RAM
    //   2. has the resource a linear memory layout
    // we do not want to map memory over the PCIe bus as this generally leads to bad performance.
    (dev.unified_memory() || res.is_staging() || res.is_user)
        && (res.is_buffer() || res.is_linear())
}

impl Mem {
    pub fn new_buffer(
        context: Arc<Context>,
        flags: cl_mem_flags,
        size: usize,
        host_ptr: *mut c_void,
        props: Vec<cl_mem_properties>,
    ) -> CLResult<Arc<Mem>> {
        let res_type = if bit_check(flags, CL_MEM_ALLOC_HOST_PTR) {
            ResourceType::Staging
        } else {
            ResourceType::Normal
        };

        let buffer = context.create_buffer(
            size,
            host_ptr,
            bit_check(flags, CL_MEM_COPY_HOST_PTR),
            res_type,
        )?;

        let host_ptr = if bit_check(flags, CL_MEM_USE_HOST_PTR) {
            host_ptr
        } else {
            ptr::null_mut()
        };

        Ok(Arc::new(Self {
            base: CLObjectBase::new(),
            context: context,
            parent: None,
            mem_type: CL_MEM_OBJECT_BUFFER,
            flags: flags,
            size: size,
            offset: 0,
            host_ptr: host_ptr,
            image_format: cl_image_format::default(),
            pipe_format: pipe_format::PIPE_FORMAT_NONE,
            image_desc: cl_image_desc::default(),
            image_elem_size: 0,
            props: props,
            gl_obj: None,
            cbs: Mutex::new(Vec::new()),
            res: Some(buffer),
            maps: Mappings::new(),
        }))
    }

    pub fn new_sub_buffer(
        parent: Arc<Mem>,
        flags: cl_mem_flags,
        offset: usize,
        size: usize,
    ) -> Arc<Mem> {
        let host_ptr = if parent.host_ptr.is_null() {
            ptr::null_mut()
        } else {
            unsafe { parent.host_ptr.add(offset) }
        };

        Arc::new(Self {
            base: CLObjectBase::new(),
            context: parent.context.clone(),
            parent: Some(parent),
            mem_type: CL_MEM_OBJECT_BUFFER,
            flags: flags,
            size: size,
            offset: offset,
            host_ptr: host_ptr,
            image_format: cl_image_format::default(),
            pipe_format: pipe_format::PIPE_FORMAT_NONE,
            image_desc: cl_image_desc::default(),
            image_elem_size: 0,
            props: Vec::new(),
            gl_obj: None,
            cbs: Mutex::new(Vec::new()),
            res: None,
            maps: Mappings::new(),
        })
    }

    pub fn new_image(
        context: Arc<Context>,
        parent: Option<Arc<Mem>>,
        mem_type: cl_mem_object_type,
        flags: cl_mem_flags,
        image_format: &cl_image_format,
        mut image_desc: cl_image_desc,
        image_elem_size: u8,
        host_ptr: *mut c_void,
        props: Vec<cl_mem_properties>,
    ) -> CLResult<Arc<Mem>> {
        // we have to sanitize the image_desc a little for internal use
        let api_image_desc = image_desc;
        let dims = image_desc.dims();
        let is_array = image_desc.is_array();
        if dims < 3 {
            image_desc.image_depth = 1;
        }
        if dims < 2 {
            image_desc.image_height = 1;
        }
        if !is_array {
            image_desc.image_array_size = 1;
        }

        let res_type = if bit_check(flags, CL_MEM_ALLOC_HOST_PTR) {
            ResourceType::Staging
        } else {
            ResourceType::Normal
        };

        let texture = if parent.is_none() {
            let mut texture = context.create_texture(
                &image_desc,
                image_format,
                host_ptr,
                bit_check(flags, CL_MEM_COPY_HOST_PTR),
                res_type,
            );

            // if we error allocating a Staging resource, just try with normal as
            // `CL_MEM_ALLOC_HOST_PTR` is just a performance hint.
            if res_type == ResourceType::Staging && texture.is_err() {
                texture = context.create_texture(
                    &image_desc,
                    image_format,
                    host_ptr,
                    bit_check(flags, CL_MEM_COPY_HOST_PTR),
                    ResourceType::Normal,
                )
            }

            Some(texture?)
        } else {
            None
        };

        let host_ptr = if bit_check(flags, CL_MEM_USE_HOST_PTR) {
            host_ptr
        } else {
            ptr::null_mut()
        };

        let pipe_format = image_format.to_pipe_format().unwrap();
        Ok(Arc::new(Self {
            base: CLObjectBase::new(),
            context: context,
            parent: parent,
            mem_type: mem_type,
            flags: flags,
            size: image_desc.pixels() * image_format.pixel_size().unwrap() as usize,
            offset: 0,
            host_ptr: host_ptr,
            image_format: *image_format,
            pipe_format: pipe_format,
            image_desc: api_image_desc,
            image_elem_size: image_elem_size,
            props: props,
            gl_obj: None,
            cbs: Mutex::new(Vec::new()),
            res: texture,
            maps: Mappings::new(),
        }))
    }

    pub fn from_gl(
        context: Arc<Context>,
        flags: cl_mem_flags,
        gl_export_manager: &GLExportManager,
    ) -> CLResult<Arc<Mem>> {
        let export_in = &gl_export_manager.export_in;
        let export_out = &gl_export_manager.export_out;

        let (mem_type, gl_object_type) = target_from_gl(export_in.target)?;
        let gl_mem_props = gl_export_manager.get_gl_mem_props()?;

        // Handle Buffers
        let (image_format, pipe_format) = if gl_export_manager.is_gl_buffer() {
            (cl_image_format::default(), pipe_format::PIPE_FORMAT_NONE)
        } else {
            let image_format =
                format_from_gl(export_out.internal_format).ok_or(CL_OUT_OF_HOST_MEMORY)?;
            (image_format, image_format.to_pipe_format().unwrap())
        };

        let imported_gl_tex = context.import_gl_buffer(
            export_out.dmabuf_fd as u32,
            export_out.modifier,
            mem_type,
            export_in.target,
            pipe_format,
            gl_mem_props.clone(),
        )?;

        // Cube maps faces are not linear in memory, so copy all contents
        // of desired face into a 2D image and copy it back after gl release.
        let (shadow_map, texture) = if is_cube_map_face(export_in.target) {
            let shadow = create_shadow_slice(&imported_gl_tex, image_format)?;

            let mut res_map = HashMap::new();
            shadow
                .iter()
                .map(|(k, v)| {
                    let gl_res = imported_gl_tex.get(k).unwrap().clone();
                    res_map.insert(v.clone(), gl_res);
                })
                .for_each(drop);

            (Some(res_map), shadow)
        } else {
            (None, imported_gl_tex)
        };

        let gl_obj = GLObject {
            gl_object_target: gl_export_manager.export_in.target,
            gl_object_type: gl_object_type,
            gl_object_name: export_in.obj,
            shadow_map: shadow_map,
        };

        let desc = cl_image_desc {
            image_type: mem_type,
            image_width: gl_mem_props.width as usize,
            image_height: gl_mem_props.height as usize,
            image_depth: gl_mem_props.depth as usize,
            image_array_size: gl_mem_props.array_size as usize,
            image_row_pitch: 0,
            image_slice_pitch: 0,
            num_mip_levels: 1,
            num_samples: 1,
            ..Default::default()
        };

        // it's kinda not supported, but we want to know if anything actually hits this as it's
        // certainly not tested by the CL CTS.
        if mem_type != CL_MEM_OBJECT_BUFFER {
            assert_eq!(gl_mem_props.offset, 0);
        }
        Ok(Arc::new(Self {
            base: CLObjectBase::new(),
            context: context,
            parent: None,
            mem_type: mem_type,
            flags: flags,
            size: gl_mem_props.size(),
            offset: gl_mem_props.offset as usize,
            host_ptr: ptr::null_mut(),
            image_format: image_format,
            pipe_format: pipe_format,
            image_desc: desc,
            image_elem_size: gl_mem_props.pixel_size,
            props: Vec::new(),
            gl_obj: Some(gl_obj),
            cbs: Mutex::new(Vec::new()),
            res: Some(texture),
            maps: Mappings::new(),
        }))
    }

    pub fn pixel_size(&self) -> Option<u8> {
        if self.is_buffer() {
            Some(1)
        } else {
            self.image_format.pixel_size()
        }
    }

    pub fn is_buffer(&self) -> bool {
        self.mem_type == CL_MEM_OBJECT_BUFFER
    }

    fn tx_raw(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        mut offset: usize,
        size: usize,
        rw: RWFlags,
    ) -> CLResult<PipeTransfer> {
        let b = self.to_parent(&mut offset);
        let r = b.get_res()?.get(&q.device).unwrap();

        ctx.buffer_map(
            r,
            offset.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?,
            size.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?,
            rw,
            ResourceMapType::Normal,
        )
        .ok_or(CL_OUT_OF_RESOURCES)
    }

    fn tx_raw_async(
        &self,
        dev: &Device,
        rw: RWFlags,
    ) -> CLResult<(PipeTransfer, Option<PipeResource>)> {
        let mut offset = 0;
        let b = self.to_parent(&mut offset);
        let r = b.get_res()?.get(dev).unwrap();
        let size = self.size.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
        let ctx = dev.helper_ctx();

        assert!(self.is_buffer());

        let tx = if can_map_directly(dev, r) {
            ctx.buffer_map_directly(
                r,
                offset.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?,
                size,
                rw,
            )
        } else {
            None
        };

        if let Some(tx) = tx {
            Ok((tx, None))
        } else {
            let shadow = dev
                .screen()
                .resource_create_buffer(size as u32, ResourceType::Staging, 0)
                .ok_or(CL_OUT_OF_RESOURCES)?;
            let tx = ctx
                .buffer_map_coherent(&shadow, 0, size, rw)
                .ok_or(CL_OUT_OF_RESOURCES)?;
            Ok((tx, Some(shadow)))
        }
    }

    fn tx<'a>(
        &self,
        q: &Arc<Queue>,
        ctx: &'a PipeContext,
        offset: usize,
        size: usize,
        rw: RWFlags,
    ) -> CLResult<GuardedPipeTransfer<'a>> {
        Ok(self.tx_raw(q, ctx, offset, size, rw)?.with_ctx(ctx))
    }

    fn tx_image_raw(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        bx: &pipe_box,
        rw: RWFlags,
    ) -> CLResult<PipeTransfer> {
        assert!(!self.is_buffer());

        let r = self.get_res()?.get(&q.device).unwrap();
        ctx.texture_map(r, bx, rw, ResourceMapType::Normal)
            .ok_or(CL_OUT_OF_RESOURCES)
    }

    fn tx_image_raw_async(
        &self,
        dev: &Device,
        bx: &pipe_box,
        rw: RWFlags,
    ) -> CLResult<(PipeTransfer, Option<PipeResource>)> {
        assert!(!self.is_buffer());

        let r = self.get_res()?.get(dev).unwrap();
        let ctx = dev.helper_ctx();

        let tx = if can_map_directly(dev, r) {
            ctx.texture_map_directly(r, bx, rw)
        } else {
            None
        };

        if let Some(tx) = tx {
            Ok((tx, None))
        } else {
            let shadow = dev
                .screen()
                .resource_create_texture(
                    r.width(),
                    r.height(),
                    r.depth(),
                    r.array_size(),
                    cl_mem_type_to_texture_target(self.image_desc.image_type),
                    self.pipe_format,
                    ResourceType::Staging,
                    false,
                )
                .ok_or(CL_OUT_OF_RESOURCES)?;
            let tx = ctx
                .texture_map_coherent(&shadow, bx, rw)
                .ok_or(CL_OUT_OF_RESOURCES)?;
            Ok((tx, Some(shadow)))
        }
    }

    fn tx_image<'a>(
        &self,
        q: &Arc<Queue>,
        ctx: &'a PipeContext,
        bx: &pipe_box,
        rw: RWFlags,
    ) -> CLResult<GuardedPipeTransfer<'a>> {
        Ok(self.tx_image_raw(q, ctx, bx, rw)?.with_ctx(ctx))
    }

    pub fn has_same_parent(&self, other: &Self) -> bool {
        ptr::eq(self.get_parent(), other.get_parent())
    }

    pub fn is_parent_buffer(&self) -> bool {
        self.parent.as_ref().map_or(false, |p| p.is_buffer())
    }

    pub fn is_image_from_buffer(&self) -> bool {
        self.is_parent_buffer() && self.mem_type == CL_MEM_OBJECT_IMAGE2D
    }

    // this is kinda bogus, because that won't work with system SVM, but the spec wants us to
    // implement this.
    pub fn is_svm(&self) -> bool {
        let mem = self.get_parent();
        self.context.find_svm_alloc(mem.host_ptr.cast()).is_some()
            && bit_check(mem.flags, CL_MEM_USE_HOST_PTR)
    }

    fn get_res(&self) -> CLResult<&HashMap<&'static Device, Arc<PipeResource>>> {
        self.get_parent().res.as_ref().ok_or(CL_OUT_OF_HOST_MEMORY)
    }

    pub fn get_res_of_dev(&self, dev: &Device) -> CLResult<&Arc<PipeResource>> {
        Ok(self.get_res()?.get(dev).unwrap())
    }

    fn get_parent(&self) -> &Self {
        if let Some(parent) = &self.parent {
            parent
        } else {
            self
        }
    }

    fn to_parent<'a>(&'a self, offset: &mut usize) -> &'a Self {
        if let Some(parent) = &self.parent {
            offset.add_assign(self.offset);
            parent
        } else {
            self
        }
    }

    fn has_user_shadow_buffer(&self, d: &Device) -> CLResult<bool> {
        let r = self.get_res()?.get(d).unwrap();
        Ok(!r.is_user && bit_check(self.flags, CL_MEM_USE_HOST_PTR))
    }

    pub fn read_to_user(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        offset: usize,
        ptr: *mut c_void,
        size: usize,
    ) -> CLResult<()> {
        assert!(self.is_buffer());

        let tx = self.tx(q, ctx, offset, size, RWFlags::RD)?;

        unsafe {
            ptr::copy_nonoverlapping(tx.ptr(), ptr, size);
        }

        Ok(())
    }

    pub fn write_from_user(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        mut offset: usize,
        ptr: *const c_void,
        size: usize,
    ) -> CLResult<()> {
        assert!(self.is_buffer());

        let b = self.to_parent(&mut offset);
        let r = b.get_res()?.get(&q.device).unwrap();
        ctx.buffer_subdata(
            r,
            offset.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?,
            ptr,
            size.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?,
        );
        Ok(())
    }

    pub fn copy_to(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        dst: &Arc<Mem>,
        mut src_origin: CLVec<usize>,
        mut dst_origin: CLVec<usize>,
        region: &CLVec<usize>,
    ) -> CLResult<()> {
        let dst_base = dst;
        let src = self.to_parent(&mut src_origin[0]);
        let dst = dst.to_parent(&mut dst_origin[0]);

        let src_res = src.get_res()?.get(&q.device).unwrap();
        let dst_res = dst.get_res()?.get(&q.device).unwrap();

        // We just want to use sw_copy if mem objects have different types
        // or if copy can have custom strides (image2d from buff/images)
        if self.is_buffer() != dst_base.is_buffer()
            || !self.is_buffer() && self.parent.is_some()
            || !dst_base.is_buffer() && dst_base.parent.is_some()
        {
            let tx_src;
            let tx_dst;
            let mut src_pitch = [0, 0, 0];
            let mut dst_pitch = [0, 0, 0];

            let bpp = if !self.is_buffer() {
                self.pixel_size().unwrap() as usize
            } else {
                dst_base.pixel_size().unwrap() as usize
            };

            if src.is_buffer() {
                // If image is created from a buffer, use image's slice and row pitch instead
                src_pitch[0] = bpp;
                if self.is_image_from_buffer() {
                    src_pitch[1] = self.image_desc.row_pitch()? as usize;
                    src_pitch[2] = self.image_desc.slice_pitch();
                } else {
                    src_pitch[1] = region[0] * bpp;
                    src_pitch[2] = region[0] * region[1] * bpp;
                }

                let (offset, size) = CLVec::calc_offset_size(src_origin, region, src_pitch);
                tx_src = src.tx(q, ctx, offset, size, RWFlags::RD)?;
            } else {
                tx_src = src.tx_image(
                    q,
                    ctx,
                    &create_pipe_box(src_origin, *region, src.mem_type)?,
                    RWFlags::RD,
                )?;

                src_pitch = [1, tx_src.row_pitch() as usize, tx_src.slice_pitch()];
            }

            if dst.is_buffer() {
                // If image is created from a buffer, use image's slice and row pitch instead
                dst_pitch[0] = bpp;
                if dst_base.is_image_from_buffer() {
                    dst_pitch[1] = dst_base.image_desc.row_pitch()? as usize;
                    dst_pitch[2] = dst_base.image_desc.slice_pitch();
                } else {
                    dst_pitch[1] = region[0] * bpp;
                    dst_pitch[2] = region[0] * region[1] * bpp;
                }

                let (offset, size) = CLVec::calc_offset_size(dst_origin, region, dst_pitch);
                tx_dst = dst.tx(q, ctx, offset, size, RWFlags::WR)?;
            } else {
                tx_dst = dst.tx_image(
                    q,
                    ctx,
                    &create_pipe_box(dst_origin, *region, dst.mem_type)?,
                    RWFlags::WR,
                )?;

                dst_pitch = [1, tx_dst.row_pitch() as usize, tx_dst.slice_pitch()];
            }

            // Those pitch values cannot have 0 value in its coordinates
            assert!(src_pitch[0] != 0 && src_pitch[1] != 0 && src_pitch[2] != 0);
            assert!(dst_pitch[0] != 0 && dst_pitch[1] != 0 && dst_pitch[2] != 0);

            sw_copy(
                tx_src.ptr(),
                tx_dst.ptr(),
                region,
                &CLVec::default(),
                src_pitch[1],
                src_pitch[2],
                &CLVec::default(),
                dst_pitch[1],
                dst_pitch[2],
                bpp as u8,
            )
        } else {
            let bx = create_pipe_box(src_origin, *region, src.mem_type)?;
            let mut dst_origin: [u32; 3] = dst_origin.try_into()?;

            if src.mem_type == CL_MEM_OBJECT_IMAGE1D_ARRAY {
                (dst_origin[1], dst_origin[2]) = (dst_origin[2], dst_origin[1]);
            }

            ctx.resource_copy_region(src_res, dst_res, &dst_origin, &bx);
        }
        Ok(())
    }

    pub fn fill(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        pattern: &[u8],
        mut offset: usize,
        size: usize,
    ) -> CLResult<()> {
        assert!(self.is_buffer());

        let b = self.to_parent(&mut offset);
        let res = b.get_res()?.get(&q.device).unwrap();
        ctx.clear_buffer(
            res,
            pattern,
            offset.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?,
            size.try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?,
        );
        Ok(())
    }

    pub fn fill_image(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        pattern: &[u32],
        origin: &CLVec<usize>,
        region: &CLVec<usize>,
    ) -> CLResult<()> {
        assert!(!self.is_buffer());

        let res = self.get_res()?.get(&q.device).unwrap();
        // make sure we allocate multiples of 4 bytes so drivers don't read out of bounds or
        // unaligned.
        // TODO: use div_ceil once it's available
        let pixel_size = align(self.pixel_size().unwrap() as usize, size_of::<u32>());
        let mut new_pattern: Vec<u32> = vec![0; pixel_size / size_of::<u32>()];

        // we don't support CL_DEPTH for now
        assert!(pattern.len() == 4);

        // SAFETY: pointers have to be valid for read/writes of exactly one pixel of their
        // respective format.
        // `new_pattern` has the correct size due to the `size` above.
        // `pattern` is validated through the CL API and allows undefined behavior if not followed
        // by CL API rules. It's expected to be a 4 component array of 32 bit values, except for
        // CL_DEPTH where it's just one value.
        unsafe {
            util_format_pack_rgba(
                self.pipe_format,
                new_pattern.as_mut_ptr().cast(),
                pattern.as_ptr().cast(),
                1,
            );
        }

        // If image is created from a buffer, use clear_image_buffer instead
        // for each row
        if self.is_parent_buffer() {
            let strides = (
                self.image_desc.row_pitch()? as usize,
                self.image_desc.slice_pitch(),
            );
            ctx.clear_image_buffer(res, &new_pattern, origin, region, strides, pixel_size);
        } else {
            let bx = create_pipe_box(*origin, *region, self.mem_type)?;
            ctx.clear_texture(res, &new_pattern, &bx);
        }

        Ok(())
    }

    pub fn write_from_user_rect(
        &self,
        src: *const c_void,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        region: &CLVec<usize>,
        src_origin: &CLVec<usize>,
        src_row_pitch: usize,
        mut src_slice_pitch: usize,
        dst_origin: &CLVec<usize>,
        dst_row_pitch: usize,
        dst_slice_pitch: usize,
    ) -> CLResult<()> {
        if self.is_buffer() || self.is_image_from_buffer() {
            let pixel_size = self.pixel_size().unwrap();
            let (offset, size) = CLVec::calc_offset_size(
                dst_origin,
                region,
                [
                    pixel_size.try_into().unwrap(),
                    dst_row_pitch,
                    dst_slice_pitch,
                ],
            );
            let tx = self.tx(q, ctx, offset, size, RWFlags::WR)?;

            sw_copy(
                src,
                tx.ptr(),
                region,
                src_origin,
                src_row_pitch,
                src_slice_pitch,
                &CLVec::default(),
                dst_row_pitch,
                dst_slice_pitch,
                pixel_size,
            );
        } else {
            assert!(dst_row_pitch == self.image_desc.image_row_pitch);
            assert!(dst_slice_pitch == self.image_desc.image_slice_pitch);
            assert!(src_origin == &CLVec::default());

            let res = self.get_res()?.get(&q.device).unwrap();
            let bx = create_pipe_box(*dst_origin, *region, self.mem_type)?;

            if self.mem_type == CL_MEM_OBJECT_IMAGE1D_ARRAY {
                src_slice_pitch = src_row_pitch;
            }

            ctx.texture_subdata(
                res,
                &bx,
                src,
                src_row_pitch
                    .try_into()
                    .map_err(|_| CL_OUT_OF_HOST_MEMORY)?,
                src_slice_pitch,
            );
        }
        Ok(())
    }

    pub fn read_to_user_rect(
        &self,
        dst: *mut c_void,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        region: &CLVec<usize>,
        src_origin: &CLVec<usize>,
        mut src_row_pitch: usize,
        mut src_slice_pitch: usize,
        dst_origin: &CLVec<usize>,
        dst_row_pitch: usize,
        dst_slice_pitch: usize,
    ) -> CLResult<()> {
        let tx;
        let pixel_size;

        if self.is_buffer() || self.is_image_from_buffer() {
            pixel_size = self.pixel_size().unwrap();
            let (offset, size) = CLVec::calc_offset_size(
                src_origin,
                region,
                [
                    pixel_size.try_into().unwrap(),
                    src_row_pitch,
                    src_slice_pitch,
                ],
            );
            tx = self.tx(q, ctx, offset, size, RWFlags::RD)?;
        } else {
            assert!(dst_origin == &CLVec::default());

            let bx = create_pipe_box(*src_origin, *region, self.mem_type)?;
            tx = self.tx_image(q, ctx, &bx, RWFlags::RD)?;
            src_row_pitch = tx.row_pitch() as usize;
            src_slice_pitch = tx.slice_pitch();

            pixel_size = self.pixel_size().unwrap();
        };

        sw_copy(
            tx.ptr(),
            dst,
            region,
            &CLVec::default(),
            src_row_pitch,
            src_slice_pitch,
            dst_origin,
            dst_row_pitch,
            dst_slice_pitch,
            pixel_size,
        );

        Ok(())
    }

    pub fn copy_to_rect(
        &self,
        dst: &Self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        region: &CLVec<usize>,
        src_origin: &CLVec<usize>,
        src_row_pitch: usize,
        src_slice_pitch: usize,
        dst_origin: &CLVec<usize>,
        dst_row_pitch: usize,
        dst_slice_pitch: usize,
    ) -> CLResult<()> {
        assert!(self.is_buffer());
        assert!(dst.is_buffer());

        let (offset, size) =
            CLVec::calc_offset_size(src_origin, region, [1, src_row_pitch, src_slice_pitch]);
        let tx_src = self.tx(q, ctx, offset, size, RWFlags::RD)?;

        let (offset, size) =
            CLVec::calc_offset_size(dst_origin, region, [1, dst_row_pitch, dst_slice_pitch]);
        let tx_dst = dst.tx(q, ctx, offset, size, RWFlags::WR)?;

        // TODO check to use hw accelerated paths (e.g. resource_copy_region or blits)
        sw_copy(
            tx_src.ptr(),
            tx_dst.ptr(),
            region,
            &CLVec::default(),
            src_row_pitch,
            src_slice_pitch,
            &CLVec::default(),
            dst_row_pitch,
            dst_slice_pitch,
            1,
        );

        Ok(())
    }

    // TODO: only sync on map when the memory is not mapped with discard
    pub fn sync_shadow_buffer(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        ptr: *mut c_void,
    ) -> CLResult<()> {
        let mut lock = self.maps.lock().unwrap();
        if !lock.increase_ref(q.device, ptr) {
            return Ok(());
        }

        if self.has_user_shadow_buffer(q.device)? {
            self.read_to_user(q, ctx, 0, self.host_ptr, self.size)
        } else {
            if let Some(shadow) = lock.tx.get(&q.device).and_then(|tx| tx.shadow.as_ref()) {
                let mut offset = 0;
                let b = self.to_parent(&mut offset);
                let res = b.get_res_of_dev(q.device)?;
                let bx = create_pipe_box(
                    [offset, 0, 0].into(),
                    [self.size, 1, 1].into(),
                    CL_MEM_OBJECT_BUFFER,
                )?;
                ctx.resource_copy_region(res, shadow, &[0; 3], &bx);
            }
            Ok(())
        }
    }

    // TODO: only sync on map when the memory is not mapped with discard
    pub fn sync_shadow_image(
        &self,
        q: &Arc<Queue>,
        ctx: &PipeContext,
        ptr: *mut c_void,
    ) -> CLResult<()> {
        let mut lock = self.maps.lock().unwrap();
        if !lock.increase_ref(q.device, ptr) {
            return Ok(());
        }

        if self.has_user_shadow_buffer(q.device)? {
            self.read_to_user_rect(
                self.host_ptr,
                q,
                ctx,
                &self.image_desc.size(),
                &CLVec::default(),
                0,
                0,
                &CLVec::default(),
                self.image_desc.image_row_pitch,
                self.image_desc.image_slice_pitch,
            )
        } else {
            if let Some(shadow) = lock.tx.get(q.device).and_then(|tx| tx.shadow.as_ref()) {
                let res = self.get_res_of_dev(q.device)?;
                let bx = self.image_desc.bx()?;
                ctx.resource_copy_region(res, shadow, &[0, 0, 0], &bx);
            }
            Ok(())
        }
    }

    /// Maps the queue associated device's resource.
    ///
    /// Mapping resources could have been quite straightforward if OpenCL wouldn't allow for so
    /// called non blocking maps. Non blocking maps shall return a valid pointer to the mapped
    /// region immediately, but should not synchronize data (in case of shadow buffers) until after
    /// the map event is reached in the queue.
    /// This makes it not possible to simply use pipe_transfers as those can't be explicitly synced
    /// by the frontend.
    ///
    /// In order to have a compliant implementation of the mapping API we have to consider the
    /// following cases:
    ///   1. Mapping a cl_mem object with CL_MEM_USE_HOST_PTR: We simply return the host_ptr.
    ///      Synchronization of shadowed host ptrs are done in `sync_shadow_buffer` and
    ///      `sync_shadow_image` on demand.
    ///   2. Mapping linear resources on UMA systems: We simply create the pipe_transfer with
    ///      `PIPE_MAP_DIRECTLY` and `PIPE_MAP_UNSYNCHRONIZED` and return the attached pointer.
    ///   3. On non UMA systems or when 2. fails (e.g. due to the resource being tiled) we
    ///      - create a shadow pipe_resource with `PIPE_USAGE_STAGING`,
    ///        `PIPE_RESOURCE_FLAG_MAP_PERSISTENT` and `PIPE_RESOURCE_FLAG_MAP_COHERENT`
    ///      - create a pipe_transfer with `PIPE_MAP_COHERENT`, `PIPE_MAP_PERSISTENT` and
    ///        `PIPE_MAP_UNSYNCHRONIZED`
    ///      - sync the shadow buffer like a host_ptr shadow buffer in 1.
    ///
    /// Taking this approach we guarentee that we only copy when actually needed while making sure
    /// the content behind the returned pointer is valid until unmapped.
    fn map<'a>(
        &self,
        dev: &'static Device,
        lock: &'a mut MutexGuard<Mappings>,
        rw: RWFlags,
    ) -> CLResult<&'a PipeTransfer> {
        if let Entry::Vacant(e) = lock.tx.entry(dev) {
            let (tx, res) = if self.is_buffer() {
                self.tx_raw_async(dev, rw)?
            } else {
                let bx = self.image_desc.bx()?;
                self.tx_image_raw_async(dev, &bx, rw)?
            };

            e.insert(MappingTransfer::new(tx, res));
        } else {
            lock.mark_pending(dev);
        }

        Ok(&lock.tx.get_mut(dev).unwrap().tx)
    }

    pub fn map_buffer(
        &self,
        dev: &'static Device,
        offset: usize,
        _size: usize,
    ) -> CLResult<*mut c_void> {
        assert!(self.is_buffer());

        let mut lock = self.maps.lock().unwrap();
        let ptr = if self.has_user_shadow_buffer(dev)? {
            self.host_ptr
        } else {
            let tx = self.map(dev, &mut lock, RWFlags::RW)?;
            tx.ptr()
        };

        let ptr = unsafe { ptr.add(offset) };
        Ok(ptr)
    }

    pub fn map_image(
        &self,
        dev: &'static Device,
        origin: &CLVec<usize>,
        _region: &CLVec<usize>,
        row_pitch: &mut usize,
        slice_pitch: &mut usize,
    ) -> CLResult<*mut c_void> {
        assert!(!self.is_buffer());

        let mut lock = self.maps.lock().unwrap();

        // we might have a host_ptr shadow buffer or image created from buffer
        let ptr = if self.has_user_shadow_buffer(dev)? || self.is_parent_buffer() {
            *row_pitch = self.image_desc.image_row_pitch;
            *slice_pitch = self.image_desc.image_slice_pitch;

            if let Some(src) = &self.parent {
                let tx = src.map(dev, &mut lock, RWFlags::RW)?;
                tx.ptr()
            } else {
                self.host_ptr
            }
        } else {
            let tx = self.map(dev, &mut lock, RWFlags::RW)?;

            if self.image_desc.dims() > 1 {
                *row_pitch = tx.row_pitch() as usize;
            }
            if self.image_desc.dims() > 2 || self.image_desc.is_array() {
                *slice_pitch = tx.slice_pitch();
            }

            tx.ptr()
        };

        let ptr = unsafe {
            ptr.add(
                *origin
                    * [
                        self.pixel_size().unwrap() as usize,
                        *row_pitch,
                        *slice_pitch,
                    ],
            )
        };

        Ok(ptr)
    }

    pub fn is_mapped_ptr(&self, ptr: *mut c_void) -> bool {
        self.maps.lock().unwrap().maps.contains_key(&ptr)
    }

    // TODO: only sync on unmap when the memory is not mapped for writing
    pub fn unmap(&self, q: &Arc<Queue>, ctx: &PipeContext, ptr: *mut c_void) -> CLResult<()> {
        let mut lock = self.maps.lock().unwrap();
        if !lock.maps.contains_key(&ptr) {
            return Ok(());
        }

        let (needs_sync, shadow) = lock.decrease_ref(ptr, q.device);
        if needs_sync {
            if let Some(shadow) = shadow {
                let mut offset = 0;
                let b = self.to_parent(&mut offset);
                let res = b.get_res_of_dev(q.device)?;

                let bx = if b.is_buffer() {
                    create_pipe_box(
                        CLVec::default(),
                        [self.size, 1, 1].into(),
                        CL_MEM_OBJECT_BUFFER,
                    )?
                } else {
                    self.image_desc.bx()?
                };

                ctx.resource_copy_region(shadow, res, &[offset as u32, 0, 0], &bx);
            } else if self.has_user_shadow_buffer(q.device)? {
                if self.is_buffer() {
                    self.write_from_user(q, ctx, 0, self.host_ptr, self.size)?;
                } else {
                    self.write_from_user_rect(
                        self.host_ptr,
                        q,
                        ctx,
                        &self.image_desc.size(),
                        &CLVec::default(),
                        self.image_desc.image_row_pitch,
                        self.image_desc.image_slice_pitch,
                        &CLVec::default(),
                        self.image_desc.image_row_pitch,
                        self.image_desc.image_slice_pitch,
                    )?;
                }
            }
        }

        lock.clean_up_tx(q.device, ctx);

        Ok(())
    }

    pub fn pipe_image_host_access(&self) -> u16 {
        // those flags are all mutually exclusive
        (if bit_check(self.flags, CL_MEM_HOST_READ_ONLY) {
            PIPE_IMAGE_ACCESS_READ
        } else if bit_check(self.flags, CL_MEM_HOST_WRITE_ONLY) {
            PIPE_IMAGE_ACCESS_WRITE
        } else if bit_check(self.flags, CL_MEM_HOST_NO_ACCESS) {
            0
        } else {
            PIPE_IMAGE_ACCESS_READ_WRITE
        }) as u16
    }
}

impl Drop for Mem {
    fn drop(&mut self) {
        let cbs = mem::take(self.cbs.get_mut().unwrap());
        for cb in cbs.into_iter().rev() {
            cb.call(self);
        }

        for (d, tx) in self.maps.get_mut().unwrap().tx.drain() {
            d.helper_ctx().unmap(tx.tx);
        }
    }
}

pub struct Sampler {
    pub base: CLObjectBase<CL_INVALID_SAMPLER>,
    pub context: Arc<Context>,
    pub normalized_coords: bool,
    pub addressing_mode: cl_addressing_mode,
    pub filter_mode: cl_filter_mode,
    pub props: Option<Properties<cl_sampler_properties>>,
}

impl_cl_type_trait!(cl_sampler, Sampler, CL_INVALID_SAMPLER);

impl Sampler {
    pub fn new(
        context: Arc<Context>,
        normalized_coords: bool,
        addressing_mode: cl_addressing_mode,
        filter_mode: cl_filter_mode,
        props: Option<Properties<cl_sampler_properties>>,
    ) -> Arc<Sampler> {
        Arc::new(Self {
            base: CLObjectBase::new(),
            context: context,
            normalized_coords: normalized_coords,
            addressing_mode: addressing_mode,
            filter_mode: filter_mode,
            props: props,
        })
    }

    pub fn nir_to_cl(
        addressing_mode: u32,
        filter_mode: u32,
        normalized_coords: u32,
    ) -> (cl_addressing_mode, cl_filter_mode, bool) {
        let addr_mode = match addressing_mode {
            cl_sampler_addressing_mode::SAMPLER_ADDRESSING_MODE_NONE => CL_ADDRESS_NONE,
            cl_sampler_addressing_mode::SAMPLER_ADDRESSING_MODE_CLAMP_TO_EDGE => {
                CL_ADDRESS_CLAMP_TO_EDGE
            }
            cl_sampler_addressing_mode::SAMPLER_ADDRESSING_MODE_CLAMP => CL_ADDRESS_CLAMP,
            cl_sampler_addressing_mode::SAMPLER_ADDRESSING_MODE_REPEAT => CL_ADDRESS_REPEAT,
            cl_sampler_addressing_mode::SAMPLER_ADDRESSING_MODE_REPEAT_MIRRORED => {
                CL_ADDRESS_MIRRORED_REPEAT
            }
            _ => panic!("unknown addressing_mode"),
        };

        let filter = match filter_mode {
            cl_sampler_filter_mode::SAMPLER_FILTER_MODE_NEAREST => CL_FILTER_NEAREST,
            cl_sampler_filter_mode::SAMPLER_FILTER_MODE_LINEAR => CL_FILTER_LINEAR,
            _ => panic!("unknown filter_mode"),
        };

        (addr_mode, filter, normalized_coords != 0)
    }

    pub fn cl_to_pipe(
        (addressing_mode, filter_mode, normalized_coords): (
            cl_addressing_mode,
            cl_filter_mode,
            bool,
        ),
    ) -> pipe_sampler_state {
        let mut res = pipe_sampler_state::default();

        let wrap = match addressing_mode {
            CL_ADDRESS_CLAMP_TO_EDGE => pipe_tex_wrap::PIPE_TEX_WRAP_CLAMP_TO_EDGE,
            CL_ADDRESS_CLAMP => pipe_tex_wrap::PIPE_TEX_WRAP_CLAMP_TO_BORDER,
            CL_ADDRESS_REPEAT => pipe_tex_wrap::PIPE_TEX_WRAP_REPEAT,
            CL_ADDRESS_MIRRORED_REPEAT => pipe_tex_wrap::PIPE_TEX_WRAP_MIRROR_REPEAT,
            // TODO: what's a reasonable default?
            _ => pipe_tex_wrap::PIPE_TEX_WRAP_CLAMP_TO_EDGE,
        };

        let img_filter = match filter_mode {
            CL_FILTER_NEAREST => pipe_tex_filter::PIPE_TEX_FILTER_NEAREST,
            CL_FILTER_LINEAR => pipe_tex_filter::PIPE_TEX_FILTER_LINEAR,
            _ => panic!("unknown filter_mode"),
        };

        res.set_min_img_filter(img_filter);
        res.set_mag_img_filter(img_filter);
        res.set_unnormalized_coords((!normalized_coords).into());
        res.set_wrap_r(wrap);
        res.set_wrap_s(wrap);
        res.set_wrap_t(wrap);

        res
    }

    pub fn pipe(&self) -> pipe_sampler_state {
        Self::cl_to_pipe((
            self.addressing_mode,
            self.filter_mode,
            self.normalized_coords,
        ))
    }
}
