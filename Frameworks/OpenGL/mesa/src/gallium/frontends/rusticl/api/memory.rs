#![allow(non_upper_case_globals)]

use crate::api::event::create_and_queue;
use crate::api::icd::*;
use crate::api::types::*;
use crate::api::util::*;
use crate::core::context::Context;
use crate::core::device::*;
use crate::core::event::EventSig;
use crate::core::format::*;
use crate::core::gl::*;
use crate::core::memory::*;

use mesa_rust_util::properties::Properties;
use mesa_rust_util::ptr::*;
use mesa_rust_util::static_assert;
use rusticl_opencl_gen::*;
use rusticl_proc_macros::cl_entrypoint;
use rusticl_proc_macros::cl_info_entrypoint;

use std::alloc;
use std::alloc::Layout;
use std::cmp::Ordering;
use std::mem::{self, MaybeUninit};
use std::os::raw::c_void;
use std::ptr;
use std::slice;
use std::sync::Arc;

fn validate_mem_flags(flags: cl_mem_flags, images: bool) -> CLResult<()> {
    let mut valid_flags = cl_bitfield::from(
        CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_KERNEL_READ_AND_WRITE,
    );

    if !images {
        valid_flags |= cl_bitfield::from(
            CL_MEM_USE_HOST_PTR
                | CL_MEM_ALLOC_HOST_PTR
                | CL_MEM_COPY_HOST_PTR
                | CL_MEM_HOST_WRITE_ONLY
                | CL_MEM_HOST_READ_ONLY
                | CL_MEM_HOST_NO_ACCESS,
        );
    }

    let read_write_group =
        cl_bitfield::from(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY);

    let alloc_host_group = cl_bitfield::from(CL_MEM_ALLOC_HOST_PTR | CL_MEM_USE_HOST_PTR);

    let copy_host_group = cl_bitfield::from(CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR);

    let host_read_write_group =
        cl_bitfield::from(CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS);

    if (flags & !valid_flags != 0)
        || (flags & read_write_group).count_ones() > 1
        || (flags & alloc_host_group).count_ones() > 1
        || (flags & copy_host_group).count_ones() > 1
        || (flags & host_read_write_group).count_ones() > 1
    {
        return Err(CL_INVALID_VALUE);
    }
    Ok(())
}

fn validate_map_flags_common(map_flags: cl_mem_flags) -> CLResult<()> {
    // CL_INVALID_VALUE ... if values specified in map_flags are not valid.
    let valid_flags =
        cl_bitfield::from(CL_MAP_READ | CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION);
    let read_write_group = cl_bitfield::from(CL_MAP_READ | CL_MAP_WRITE);
    let invalidate_group = cl_bitfield::from(CL_MAP_WRITE_INVALIDATE_REGION);

    if (map_flags & !valid_flags != 0)
        || ((map_flags & read_write_group != 0) && (map_flags & invalidate_group != 0))
    {
        return Err(CL_INVALID_VALUE);
    }

    Ok(())
}

fn validate_map_flags(m: &Mem, map_flags: cl_mem_flags) -> CLResult<()> {
    validate_map_flags_common(map_flags)?;

    // CL_INVALID_OPERATION if buffer has been created with CL_MEM_HOST_WRITE_ONLY or
    // CL_MEM_HOST_NO_ACCESS and CL_MAP_READ is set in map_flags
    if bit_check(m.flags, CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS) &&
      bit_check(map_flags, CL_MAP_READ) ||
      // or if buffer has been created with CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS and
      // CL_MAP_WRITE or CL_MAP_WRITE_INVALIDATE_REGION is set in map_flags.
      bit_check(m.flags, CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS) &&
      bit_check(map_flags, CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)
    {
        return Err(CL_INVALID_OPERATION);
    }

    Ok(())
}

fn filter_image_access_flags(flags: cl_mem_flags) -> cl_mem_flags {
    flags
        & (CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY | CL_MEM_KERNEL_READ_AND_WRITE)
            as cl_mem_flags
}

fn inherit_mem_flags(mut flags: cl_mem_flags, mem: &Mem) -> cl_mem_flags {
    let read_write_mask = cl_bitfield::from(
        CL_MEM_READ_WRITE |
      CL_MEM_WRITE_ONLY |
      CL_MEM_READ_ONLY |
      // not in spec, but...
      CL_MEM_KERNEL_READ_AND_WRITE,
    );
    let host_ptr_mask =
        cl_bitfield::from(CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR);
    let host_mask =
        cl_bitfield::from(CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS);

    // For CL_MEM_OBJECT_IMAGE1D_BUFFER image type, or an image created from another memory object
    // (image or buffer)...
    //
    // ... if the CL_MEM_READ_WRITE, CL_MEM_READ_ONLY or CL_MEM_WRITE_ONLY values are not
    // specified in flags, they are inherited from the corresponding memory access qualifiers
    // associated with mem_object. ...
    if flags & read_write_mask == 0 {
        flags |= mem.flags & read_write_mask;
    }

    // ... The CL_MEM_USE_HOST_PTR, CL_MEM_ALLOC_HOST_PTR and CL_MEM_COPY_HOST_PTR values cannot
    // be specified in flags but are inherited from the corresponding memory access qualifiers
    // associated with mem_object. ...
    flags &= !host_ptr_mask;
    flags |= mem.flags & host_ptr_mask;

    // ... If the CL_MEM_HOST_WRITE_ONLY, CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS values
    // are not specified in flags, they are inherited from the corresponding memory access
    // qualifiers associated with mem_object.
    if flags & host_mask == 0 {
        flags |= mem.flags & host_mask;
    }

    flags
}

fn image_type_valid(image_type: cl_mem_object_type) -> bool {
    CL_IMAGE_TYPES.contains(&image_type)
}

fn validate_addressing_mode(addressing_mode: cl_addressing_mode) -> CLResult<()> {
    match addressing_mode {
        CL_ADDRESS_NONE
        | CL_ADDRESS_CLAMP_TO_EDGE
        | CL_ADDRESS_CLAMP
        | CL_ADDRESS_REPEAT
        | CL_ADDRESS_MIRRORED_REPEAT => Ok(()),
        _ => Err(CL_INVALID_VALUE),
    }
}

fn validate_filter_mode(filter_mode: cl_filter_mode) -> CLResult<()> {
    match filter_mode {
        CL_FILTER_NEAREST | CL_FILTER_LINEAR => Ok(()),
        _ => Err(CL_INVALID_VALUE),
    }
}

fn validate_host_ptr(host_ptr: *mut ::std::os::raw::c_void, flags: cl_mem_flags) -> CLResult<()> {
    // CL_INVALID_HOST_PTR if host_ptr is NULL and CL_MEM_USE_HOST_PTR or CL_MEM_COPY_HOST_PTR are
    // set in flags
    if host_ptr.is_null()
        && flags & (cl_mem_flags::from(CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) != 0
    {
        return Err(CL_INVALID_HOST_PTR);
    }

    // or if host_ptr is not NULL but CL_MEM_COPY_HOST_PTR or CL_MEM_USE_HOST_PTR are not set in
    // flags.
    if !host_ptr.is_null()
        && flags & (cl_mem_flags::from(CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) == 0
    {
        return Err(CL_INVALID_HOST_PTR);
    }

    Ok(())
}

fn validate_matching_buffer_flags(mem: &Mem, flags: cl_mem_flags) -> CLResult<()> {
    // CL_INVALID_VALUE if an image is being created from another memory object (buffer or image)
    // under one of the following circumstances:
    //
    // 1) mem_object was created with CL_MEM_WRITE_ONLY and
    //    flags specifies CL_MEM_READ_WRITE or CL_MEM_READ_ONLY,
    if bit_check(mem.flags, CL_MEM_WRITE_ONLY) && bit_check(flags, CL_MEM_READ_WRITE | CL_MEM_READ_ONLY) ||
      // 2) mem_object was created with CL_MEM_READ_ONLY and
      //    flags specifies CL_MEM_READ_WRITE or CL_MEM_WRITE_ONLY,
      bit_check(mem.flags, CL_MEM_READ_ONLY) && bit_check(flags, CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY) ||
      // 3) flags specifies CL_MEM_USE_HOST_PTR or CL_MEM_ALLOC_HOST_PTR or CL_MEM_COPY_HOST_PTR.
      bit_check(flags, CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR) ||
      // CL_INVALID_VALUE if an image is being created from another memory object (buffer or image)
      // and mem_object was created with CL_MEM_HOST_WRITE_ONLY and flags specifies CL_MEM_HOST_READ_ONLY
      bit_check(mem.flags, CL_MEM_HOST_WRITE_ONLY) && bit_check(flags, CL_MEM_HOST_READ_ONLY) ||
      // or if mem_object was created with CL_MEM_HOST_READ_ONLY and flags specifies CL_MEM_HOST_WRITE_ONLY
      bit_check(mem.flags, CL_MEM_HOST_READ_ONLY) && bit_check(flags, CL_MEM_HOST_WRITE_ONLY) ||
      // or if mem_object was created with CL_MEM_HOST_NO_ACCESS and_flags_ specifies CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_WRITE_ONLY.
      bit_check(mem.flags, CL_MEM_HOST_NO_ACCESS) && bit_check(flags, CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY)
    {
        return Err(CL_INVALID_VALUE);
    }

    Ok(())
}

#[cl_info_entrypoint(cl_get_mem_object_info)]
impl CLInfo<cl_mem_info> for cl_mem {
    fn query(&self, q: cl_mem_info, _: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>> {
        let mem = self.get_ref()?;
        Ok(match *q {
            CL_MEM_ASSOCIATED_MEMOBJECT => {
                let ptr = match mem.parent.as_ref() {
                    // Note we use as_ptr here which doesn't increase the reference count.
                    Some(parent) => Arc::as_ptr(parent),
                    None => ptr::null(),
                };
                cl_prop::<cl_mem>(cl_mem::from_ptr(ptr))
            }
            CL_MEM_CONTEXT => {
                // Note we use as_ptr here which doesn't increase the reference count.
                let ptr = Arc::as_ptr(&mem.context);
                cl_prop::<cl_context>(cl_context::from_ptr(ptr))
            }
            CL_MEM_FLAGS => cl_prop::<cl_mem_flags>(mem.flags),
            // TODO debugging feature
            CL_MEM_MAP_COUNT => cl_prop::<cl_uint>(0),
            CL_MEM_HOST_PTR => cl_prop::<*mut c_void>(mem.host_ptr),
            CL_MEM_OFFSET => cl_prop::<usize>(mem.offset),
            CL_MEM_PROPERTIES => cl_prop::<&Vec<cl_mem_properties>>(&mem.props),
            CL_MEM_REFERENCE_COUNT => cl_prop::<cl_uint>(self.refcnt()?),
            CL_MEM_SIZE => cl_prop::<usize>(mem.size),
            CL_MEM_TYPE => cl_prop::<cl_mem_object_type>(mem.mem_type),
            CL_MEM_USES_SVM_POINTER | CL_MEM_USES_SVM_POINTER_ARM => {
                cl_prop::<cl_bool>(mem.is_svm().into())
            }
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}

#[cl_entrypoint]
fn create_buffer_with_properties(
    context: cl_context,
    properties: *const cl_mem_properties,
    flags: cl_mem_flags,
    size: usize,
    host_ptr: *mut ::std::os::raw::c_void,
) -> CLResult<cl_mem> {
    let c = context.get_arc()?;

    // CL_INVALID_VALUE if values specified in flags are not valid as defined in the Memory Flags table.
    validate_mem_flags(flags, false)?;

    // CL_INVALID_BUFFER_SIZE if size is 0
    if size == 0 {
        return Err(CL_INVALID_BUFFER_SIZE);
    }

    // ... or if size is greater than CL_DEVICE_MAX_MEM_ALLOC_SIZE for all devices in context.
    if checked_compare(size, Ordering::Greater, c.max_mem_alloc()) {
        return Err(CL_INVALID_BUFFER_SIZE);
    }

    validate_host_ptr(host_ptr, flags)?;

    let props = Properties::from_ptr_raw(properties);
    // CL_INVALID_PROPERTY if a property name in properties is not a supported property name, if
    // the value specified for a supported property name is not valid, or if the same property name
    // is specified more than once.
    if props.len() > 1 {
        // we don't support any properties besides the 0 property
        return Err(CL_INVALID_PROPERTY);
    }

    Ok(cl_mem::from_arc(Mem::new_buffer(
        c, flags, size, host_ptr, props,
    )?))
}

#[cl_entrypoint]
fn create_buffer(
    context: cl_context,
    flags: cl_mem_flags,
    size: usize,
    host_ptr: *mut ::std::os::raw::c_void,
) -> CLResult<cl_mem> {
    create_buffer_with_properties(context, ptr::null(), flags, size, host_ptr)
}

#[cl_entrypoint]
fn create_sub_buffer(
    buffer: cl_mem,
    mut flags: cl_mem_flags,
    buffer_create_type: cl_buffer_create_type,
    buffer_create_info: *const ::std::os::raw::c_void,
) -> CLResult<cl_mem> {
    let b = buffer.get_arc()?;

    // CL_INVALID_MEM_OBJECT if buffer ... is a sub-buffer object.
    if b.parent.is_some() {
        return Err(CL_INVALID_MEM_OBJECT);
    }

    validate_matching_buffer_flags(&b, flags)?;

    flags = inherit_mem_flags(flags, &b);
    validate_mem_flags(flags, false)?;

    let (offset, size) = match buffer_create_type {
        CL_BUFFER_CREATE_TYPE_REGION => {
            // buffer_create_info is a pointer to a cl_buffer_region structure specifying a region of
            // the buffer.
            // CL_INVALID_VALUE if value(s) specified in buffer_create_info (for a given
            // buffer_create_type) is not valid or if buffer_create_info is NULL.
            let region = unsafe { buffer_create_info.cast::<cl_buffer_region>().as_ref() }
                .ok_or(CL_INVALID_VALUE)?;

            // CL_INVALID_BUFFER_SIZE if the size field of the cl_buffer_region structure passed in
            // buffer_create_info is 0.
            if region.size == 0 {
                return Err(CL_INVALID_BUFFER_SIZE);
            }

            // CL_INVALID_VALUE if the region specified by the cl_buffer_region structure passed in
            // buffer_create_info is out of bounds in buffer.
            if region.origin + region.size > b.size {
                return Err(CL_INVALID_VALUE);
            }

            (region.origin, region.size)
        }
        // CL_INVALID_VALUE if the value specified in buffer_create_type is not valid.
        _ => return Err(CL_INVALID_VALUE),
    };

    Ok(cl_mem::from_arc(Mem::new_sub_buffer(
        b, flags, offset, size,
    )))

    // TODO
    // CL_MISALIGNED_SUB_BUFFER_OFFSET if there are no devices in context associated with buffer for which the origin field of the cl_buffer_region structure passed in buffer_create_info is aligned to the CL_DEVICE_MEM_BASE_ADDR_ALIGN value.
}

#[cl_entrypoint]
fn set_mem_object_destructor_callback(
    memobj: cl_mem,
    pfn_notify: Option<FuncMemCB>,
    user_data: *mut ::std::os::raw::c_void,
) -> CLResult<()> {
    let m = memobj.get_ref()?;

    // SAFETY: The requirements on `MemCB::new` match the requirements
    // imposed by the OpenCL specification. It is the caller's duty to uphold them.
    let cb = unsafe { MemCB::new(pfn_notify, user_data)? };

    m.cbs.lock().unwrap().push(cb);
    Ok(())
}

fn validate_image_format<'a>(
    image_format: *const cl_image_format,
) -> CLResult<(&'a cl_image_format, u8)> {
    // CL_INVALID_IMAGE_FORMAT_DESCRIPTOR ... if image_format is NULL.
    let format = unsafe { image_format.as_ref() }.ok_or(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)?;
    let pixel_size = format
        .pixel_size()
        .ok_or(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)?;

    // special validation
    let valid_combination = match format.image_channel_data_type {
        CL_UNORM_SHORT_565 | CL_UNORM_SHORT_555 | CL_UNORM_INT_101010 => {
            [CL_RGB, CL_RGBx].contains(&format.image_channel_order)
        }
        CL_UNORM_INT_101010_2 => format.image_channel_order == CL_RGBA,
        _ => true,
    };
    if !valid_combination {
        return Err(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
    }

    Ok((format, pixel_size))
}

fn validate_image_desc(
    image_desc: *const cl_image_desc,
    host_ptr: *mut ::std::os::raw::c_void,
    elem_size: usize,
    devs: &[&Device],
) -> CLResult<(cl_image_desc, Option<Arc<Mem>>)> {
    // CL_INVALID_IMAGE_DESCRIPTOR if values specified in image_desc are not valid
    const err: cl_int = CL_INVALID_IMAGE_DESCRIPTOR;

    // CL_INVALID_IMAGE_DESCRIPTOR ... if image_desc is NULL.
    let mut desc = *unsafe { image_desc.as_ref() }.ok_or(err)?;

    // image_type describes the image type and must be either CL_MEM_OBJECT_IMAGE1D,
    // CL_MEM_OBJECT_IMAGE1D_BUFFER, CL_MEM_OBJECT_IMAGE1D_ARRAY, CL_MEM_OBJECT_IMAGE2D,
    // CL_MEM_OBJECT_IMAGE2D_ARRAY, or CL_MEM_OBJECT_IMAGE3D.
    if !CL_IMAGE_TYPES.contains(&desc.image_type) {
        return Err(err);
    }

    let (dims, array) = desc.type_info();

    // image_width is the width of the image in pixels. For a 2D image and image array, the image
    // width must be a value ≥ 1 and ≤ CL_DEVICE_IMAGE2D_MAX_WIDTH. For a 3D image, the image width
    // must be a value ≥ 1 and ≤ CL_DEVICE_IMAGE3D_MAX_WIDTH. For a 1D image buffer, the image width
    // must be a value ≥ 1 and ≤ CL_DEVICE_IMAGE_MAX_BUFFER_SIZE. For a 1D image and 1D image array,
    // the image width must be a value ≥ 1 and ≤ CL_DEVICE_IMAGE2D_MAX_WIDTH.
    //
    // image_height is the height of the image in pixels. This is only used if the image is a 2D or
    // 3D image, or a 2D image array. For a 2D image or image array, the image height must be a
    // value ≥ 1 and ≤ CL_DEVICE_IMAGE2D_MAX_HEIGHT. For a 3D image, the image height must be a
    // value ≥ 1 and ≤ CL_DEVICE_IMAGE3D_MAX_HEIGHT.
    //
    // image_depth is the depth of the image in pixels. This is only used if the image is a 3D image
    // and must be a value ≥ 1 and ≤ CL_DEVICE_IMAGE3D_MAX_DEPTH.
    if desc.image_width < 1
        || desc.image_height < 1 && dims >= 2
        || desc.image_depth < 1 && dims >= 3
        || desc.image_array_size < 1 && array
    {
        return Err(err);
    }

    let max_size = if dims == 3 {
        devs.iter().map(|d| d.image_3d_size()).min()
    } else if desc.image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER {
        devs.iter().map(|d| d.image_buffer_size()).min()
    } else {
        devs.iter().map(|d| d.image_2d_size()).min()
    }
    .unwrap();
    let max_array = devs.iter().map(|d| d.image_array_size()).min().unwrap();

    // CL_INVALID_IMAGE_SIZE if image dimensions specified in image_desc exceed the maximum image
    // dimensions described in the Device Queries table for all devices in context.
    if desc.image_width > max_size
        || desc.image_height > max_size && dims >= 2
        || desc.image_depth > max_size && dims >= 3
        || desc.image_array_size > max_array && array
    {
        return Err(CL_INVALID_IMAGE_SIZE);
    }

    // num_mip_levels and num_samples must be 0.
    if desc.num_mip_levels != 0 || desc.num_samples != 0 {
        return Err(err);
    }

    // mem_object may refer to a valid buffer or image memory object. mem_object can be a buffer
    // memory object if image_type is CL_MEM_OBJECT_IMAGE1D_BUFFER or CL_MEM_OBJECT_IMAGE2D.
    // mem_object can be an image object if image_type is CL_MEM_OBJECT_IMAGE2D. Otherwise it must
    // be NULL.
    //
    // TODO: cl_khr_image2d_from_buffer is an optional feature
    let p = unsafe { &desc.anon_1.mem_object };
    let parent = if !p.is_null() {
        let p = p.get_arc()?;
        if !match desc.image_type {
            CL_MEM_OBJECT_IMAGE1D_BUFFER => p.is_buffer(),
            CL_MEM_OBJECT_IMAGE2D => {
                (p.is_buffer() && devs.iter().any(|d| d.image2d_from_buffer_supported()))
                    || p.mem_type == CL_MEM_OBJECT_IMAGE2D
            }
            _ => false,
        } {
            return Err(CL_INVALID_OPERATION);
        }
        Some(p)
    } else {
        None
    };

    // image_row_pitch is the scan-line pitch in bytes. This must be 0 if host_ptr is NULL and can
    // be either 0 or ≥ image_width × size of element in bytes if host_ptr is not NULL. If host_ptr
    // is not NULL and image_row_pitch = 0, image_row_pitch is calculated as image_width × size of
    // element in bytes. If image_row_pitch is not 0, it must be a multiple of the image element
    // size in bytes. For a 2D image created from a buffer, the pitch specified (or computed if
    // pitch specified is 0) must be a multiple of the maximum of the
    // CL_DEVICE_IMAGE_PITCH_ALIGNMENT value for all devices in the context associated with the
    // buffer specified by mem_object that support images.
    //
    // image_slice_pitch is the size in bytes of each 2D slice in the 3D image or the size in bytes
    // of each image in a 1D or 2D image array. This must be 0 if host_ptr is NULL. If host_ptr is
    // not NULL, image_slice_pitch can be either 0 or ≥ image_row_pitch × image_height for a 2D
    // image array or 3D image and can be either 0 or ≥ image_row_pitch for a 1D image array. If
    // host_ptr is not NULL and image_slice_pitch = 0, image_slice_pitch is calculated as
    // image_row_pitch × image_height for a 2D image array or 3D image and image_row_pitch for a 1D
    // image array. If image_slice_pitch is not 0, it must be a multiple of the image_row_pitch.
    let has_buf_parent = parent.as_ref().map_or(false, |p| p.is_buffer());
    if host_ptr.is_null() {
        if (desc.image_row_pitch != 0 || desc.image_slice_pitch != 0) && !has_buf_parent {
            return Err(err);
        }

        if desc.image_row_pitch == 0 {
            desc.image_row_pitch = desc.image_width * elem_size;
        }
        if desc.image_slice_pitch == 0 {
            desc.image_slice_pitch = desc.image_row_pitch * desc.image_height;
        }

        if has_buf_parent && desc.image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER {
            let pitch_alignment = devs
                .iter()
                .map(|d| d.image_pitch_alignment())
                .max()
                .unwrap() as usize;
            if desc.image_row_pitch % (pitch_alignment * elem_size) != 0 {
                return Err(err);
            }
        }
    } else {
        if desc.image_row_pitch == 0 {
            desc.image_row_pitch = desc.image_width * elem_size;
        } else if desc.image_row_pitch % elem_size != 0 {
            return Err(err);
        }

        if dims == 3 || array {
            let valid_slice_pitch =
                desc.image_row_pitch * if dims == 1 { 1 } else { desc.image_height };
            if desc.image_slice_pitch == 0 {
                desc.image_slice_pitch = valid_slice_pitch;
            } else if desc.image_slice_pitch < valid_slice_pitch
                || desc.image_slice_pitch % desc.image_row_pitch != 0
            {
                return Err(err);
            }
        }
    }

    Ok((desc, parent))
}

fn validate_image_bounds(i: &Mem, origin: CLVec<usize>, region: CLVec<usize>) -> CLResult<()> {
    let dims = i.image_desc.dims_with_array();
    let bound = region + origin;
    if bound > i.image_desc.size() {
        return Err(CL_INVALID_VALUE);
    }

    // If image is a 2D image object, origin[2] must be 0. If image is a 1D image or 1D image buffer
    // object, origin[1] and origin[2] must be 0. If image is a 1D image array object, origin[2]
    // must be 0.
    if dims < 3 && origin[2] != 0 || dims < 2 && origin[1] != 0 {
        return Err(CL_INVALID_VALUE);
    }

    // If image is a 2D image object, region[2] must be 1. If image is a 1D image or 1D image buffer
    // object, region[1] and region[2] must be 1. If image is a 1D image array object, region[2]
    // must be 1. The values in region cannot be 0.
    if dims < 3 && region[2] != 1 || dims < 2 && region[1] != 1 || region.contains(&0) {
        return Err(CL_INVALID_VALUE);
    }

    Ok(())
}

fn desc_eq_no_buffer(a: &cl_image_desc, b: &cl_image_desc) -> bool {
    a.image_type == b.image_type
        && a.image_width == b.image_width
        && a.image_height == b.image_height
        && a.image_depth == b.image_depth
        && a.image_array_size == b.image_array_size
        && a.image_row_pitch == b.image_row_pitch
        && a.image_slice_pitch == b.image_slice_pitch
        && a.num_mip_levels == b.num_mip_levels
        && a.num_samples == b.num_samples
}

fn validate_buffer(
    desc: &cl_image_desc,
    mut flags: cl_mem_flags,
    format: &cl_image_format,
    host_ptr: *mut ::std::os::raw::c_void,
    elem_size: usize,
) -> CLResult<cl_mem_flags> {
    // CL_INVALID_IMAGE_DESCRIPTOR if values specified in image_desc are not valid
    const err: cl_int = CL_INVALID_IMAGE_DESCRIPTOR;
    let mem_object = unsafe { desc.anon_1.mem_object };

    // mem_object may refer to a valid buffer or image memory object. mem_object can be a buffer
    // memory object if image_type is CL_MEM_OBJECT_IMAGE1D_BUFFER or CL_MEM_OBJECT_IMAGE2D
    // mem_object can be an image object if image_type is CL_MEM_OBJECT_IMAGE2D. Otherwise it must
    // be NULL. The image pixels are taken from the memory objects data store. When the contents of
    // the specified memory objects data store are modified, those changes are reflected in the
    // contents of the image object and vice-versa at corresponding synchronization points.
    if !mem_object.is_null() {
        let mem = mem_object.get_ref()?;

        match mem.mem_type {
            CL_MEM_OBJECT_BUFFER => {
                match desc.image_type {
                    // For a 1D image buffer created from a buffer object, the image_width × size of
                    // element in bytes must be ≤ size of the buffer object.
                    CL_MEM_OBJECT_IMAGE1D_BUFFER => {
                        if desc.image_width * elem_size > mem.size {
                            return Err(err);
                        }
                    }
                    // For a 2D image created from a buffer object, the image_row_pitch × image_height
                    // must be ≤ size of the buffer object specified by mem_object.
                    CL_MEM_OBJECT_IMAGE2D => {
                        //TODO
                        //• CL_INVALID_IMAGE_FORMAT_DESCRIPTOR if a 2D image is created from a buffer and the row pitch and base address alignment does not follow the rules described for creating a 2D image from a buffer.
                        if desc.image_row_pitch * desc.image_height > mem.size {
                            return Err(err);
                        }
                    }
                    _ => return Err(err),
                }
            }
            // For an image object created from another image object, the values specified in the
            // image descriptor except for mem_object must match the image descriptor information
            // associated with mem_object.
            CL_MEM_OBJECT_IMAGE2D => {
                if desc.image_type != mem.mem_type || !desc_eq_no_buffer(desc, &mem.image_desc) {
                    return Err(err);
                }

                // CL_INVALID_IMAGE_FORMAT_DESCRIPTOR if a 2D image is created from a 2D image object
                // and the rules described above are not followed.

                // Creating a 2D image object from another 2D image object creates a new 2D image
                // object that shares the image data store with mem_object but views the pixels in the
                //  image with a different image channel order. Restrictions are:
                //
                // The image channel data type specified in image_format must match the image channel
                // data type associated with mem_object.
                if format.image_channel_data_type != mem.image_format.image_channel_data_type {
                    return Err(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
                }

                // The image channel order specified in image_format must be compatible with the image
                // channel order associated with mem_object. Compatible image channel orders are:
                if format.image_channel_order != mem.image_format.image_channel_order {
                    // in image_format | in  mem_object:
                    // CL_sBGRA | CL_BGRA
                    // CL_BGRA  | CL_sBGRA
                    // CL_sRGBA | CL_RGBA
                    // CL_RGBA  | CL_sRGBA
                    // CL_sRGB  | CL_RGB
                    // CL_RGB   | CL_sRGB
                    // CL_sRGBx | CL_RGBx
                    // CL_RGBx  | CL_sRGBx
                    // CL_DEPTH | CL_R
                    match (
                        format.image_channel_order,
                        mem.image_format.image_channel_order,
                    ) {
                        (CL_sBGRA, CL_BGRA)
                        | (CL_BGRA, CL_sBGRA)
                        | (CL_sRGBA, CL_RGBA)
                        | (CL_RGBA, CL_sRGBA)
                        | (CL_sRGB, CL_RGB)
                        | (CL_RGB, CL_sRGB)
                        | (CL_sRGBx, CL_RGBx)
                        | (CL_RGBx, CL_sRGBx)
                        | (CL_DEPTH, CL_R) => (),
                        _ => return Err(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR),
                    }
                }
            }
            _ => return Err(err),
        }

        // If the buffer object specified by mem_object was created with CL_MEM_USE_HOST_PTR, the
        // host_ptr specified to clCreateBuffer or clCreateBufferWithProperties must be aligned to
        // the maximum of the CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT value for all devices in the
        // context associated with the buffer specified by mem_object that support images.
        if mem.flags & CL_MEM_USE_HOST_PTR as cl_mem_flags != 0 {
            for dev in &mem.context.devs {
                let addr_alignment = dev.image_base_address_alignment();
                if addr_alignment == 0 {
                    return Err(CL_INVALID_OPERATION);
                } else if !is_alligned(host_ptr, addr_alignment as usize) {
                    return Err(err);
                }
            }
        }

        validate_matching_buffer_flags(mem, flags)?;

        flags = inherit_mem_flags(flags, mem);
    // implied by spec
    } else if desc.image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER {
        return Err(err);
    }

    Ok(flags)
}

#[cl_info_entrypoint(cl_get_image_info)]
impl CLInfo<cl_image_info> for cl_mem {
    fn query(&self, q: cl_image_info, _: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>> {
        let mem = self.get_ref()?;
        Ok(match *q {
            CL_IMAGE_ARRAY_SIZE => cl_prop::<usize>(mem.image_desc.image_array_size),
            CL_IMAGE_BUFFER => cl_prop::<cl_mem>(unsafe { mem.image_desc.anon_1.buffer }),
            CL_IMAGE_DEPTH => cl_prop::<usize>(mem.image_desc.image_depth),
            CL_IMAGE_ELEMENT_SIZE => cl_prop::<usize>(mem.image_elem_size.into()),
            CL_IMAGE_FORMAT => cl_prop::<cl_image_format>(mem.image_format),
            CL_IMAGE_HEIGHT => cl_prop::<usize>(mem.image_desc.image_height),
            CL_IMAGE_NUM_MIP_LEVELS => cl_prop::<cl_uint>(mem.image_desc.num_mip_levels),
            CL_IMAGE_NUM_SAMPLES => cl_prop::<cl_uint>(mem.image_desc.num_samples),
            CL_IMAGE_ROW_PITCH => cl_prop::<usize>(mem.image_desc.image_row_pitch),
            CL_IMAGE_SLICE_PITCH => cl_prop::<usize>(mem.image_desc.image_slice_pitch),
            CL_IMAGE_WIDTH => cl_prop::<usize>(mem.image_desc.image_width),
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}

#[cl_entrypoint]
fn create_image_with_properties(
    context: cl_context,
    properties: *const cl_mem_properties,
    mut flags: cl_mem_flags,
    image_format: *const cl_image_format,
    image_desc: *const cl_image_desc,
    host_ptr: *mut ::std::os::raw::c_void,
) -> CLResult<cl_mem> {
    let c = context.get_arc()?;

    // CL_INVALID_OPERATION if there are no devices in context that support images (i.e.
    // CL_DEVICE_IMAGE_SUPPORT specified in the Device Queries table is CL_FALSE).
    c.devs
        .iter()
        .find(|d| d.image_supported())
        .ok_or(CL_INVALID_OPERATION)?;

    let (format, elem_size) = validate_image_format(image_format)?;
    let (desc, parent) = validate_image_desc(image_desc, host_ptr, elem_size.into(), &c.devs)?;

    // validate host_ptr before merging flags
    validate_host_ptr(host_ptr, flags)?;

    flags = validate_buffer(&desc, flags, format, host_ptr, elem_size.into())?;

    // For all image types except CL_MEM_OBJECT_IMAGE1D_BUFFER, if the value specified for flags is 0, the
    // default is used which is CL_MEM_READ_WRITE.
    if flags == 0 && desc.image_type != CL_MEM_OBJECT_IMAGE1D_BUFFER {
        flags = CL_MEM_READ_WRITE.into();
    }

    validate_mem_flags(flags, false)?;

    let filtered_flags = filter_image_access_flags(flags);
    // CL_IMAGE_FORMAT_NOT_SUPPORTED if there are no devices in context that support image_format.
    c.devs
        .iter()
        .filter_map(|d| d.formats.get(format))
        .filter_map(|f| f.get(&desc.image_type))
        .find(|f| *f & filtered_flags == filtered_flags)
        .ok_or(CL_IMAGE_FORMAT_NOT_SUPPORTED)?;

    let props = Properties::from_ptr_raw(properties);
    // CL_INVALID_PROPERTY if a property name in properties is not a supported property name, if
    // the value specified for a supported property name is not valid, or if the same property name
    // is specified more than once.
    if props.len() > 1 {
        // we don't support any properties besides the 0 property
        return Err(CL_INVALID_PROPERTY);
    }

    Ok(cl_mem::from_arc(Mem::new_image(
        c,
        parent,
        desc.image_type,
        flags,
        format,
        desc,
        elem_size,
        host_ptr,
        props,
    )?))
}

#[cl_entrypoint]
fn create_image(
    context: cl_context,
    flags: cl_mem_flags,
    image_format: *const cl_image_format,
    image_desc: *const cl_image_desc,
    host_ptr: *mut ::std::os::raw::c_void,
) -> CLResult<cl_mem> {
    create_image_with_properties(
        context,
        ptr::null(),
        flags,
        image_format,
        image_desc,
        host_ptr,
    )
}

#[cl_entrypoint]
fn create_image_2d(
    context: cl_context,
    flags: cl_mem_flags,
    image_format: *const cl_image_format,
    image_width: usize,
    image_height: usize,
    image_row_pitch: usize,
    host_ptr: *mut ::std::os::raw::c_void,
) -> CLResult<cl_mem> {
    let image_desc = cl_image_desc {
        image_type: CL_MEM_OBJECT_IMAGE2D,
        image_width: image_width,
        image_height: image_height,
        image_row_pitch: image_row_pitch,
        ..Default::default()
    };

    create_image(context, flags, image_format, &image_desc, host_ptr)
}

#[cl_entrypoint]
fn create_image_3d(
    context: cl_context,
    flags: cl_mem_flags,
    image_format: *const cl_image_format,
    image_width: usize,
    image_height: usize,
    image_depth: usize,
    image_row_pitch: usize,
    image_slice_pitch: usize,
    host_ptr: *mut ::std::os::raw::c_void,
) -> CLResult<cl_mem> {
    let image_desc = cl_image_desc {
        image_type: CL_MEM_OBJECT_IMAGE3D,
        image_width: image_width,
        image_height: image_height,
        image_depth: image_depth,
        image_row_pitch: image_row_pitch,
        image_slice_pitch: image_slice_pitch,
        ..Default::default()
    };

    create_image(context, flags, image_format, &image_desc, host_ptr)
}

#[cl_entrypoint]
fn get_supported_image_formats(
    context: cl_context,
    flags: cl_mem_flags,
    image_type: cl_mem_object_type,
    num_entries: cl_uint,
    image_formats: *mut cl_image_format,
    num_image_formats: *mut cl_uint,
) -> CLResult<()> {
    let c = context.get_ref()?;

    // CL_INVALID_VALUE if flags
    validate_mem_flags(flags, true)?;

    // or image_type are not valid
    if !image_type_valid(image_type) {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE ... if num_entries is 0 and image_formats is not NULL.
    if num_entries == 0 && !image_formats.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    let mut res = Vec::<cl_image_format>::new();
    let filtered_flags = filter_image_access_flags(flags);
    for dev in &c.devs {
        for f in &dev.formats {
            let s = f.1.get(&image_type).unwrap_or(&0);

            if filtered_flags & s == filtered_flags {
                res.push(*f.0);
            }
        }
    }

    res.sort();
    res.dedup();

    num_image_formats.write_checked(res.len() as cl_uint);
    unsafe { image_formats.copy_checked(res.as_ptr(), res.len()) };

    Ok(())
}

#[cl_info_entrypoint(cl_get_sampler_info)]
impl CLInfo<cl_sampler_info> for cl_sampler {
    fn query(&self, q: cl_sampler_info, _: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>> {
        let sampler = self.get_ref()?;
        Ok(match q {
            CL_SAMPLER_ADDRESSING_MODE => cl_prop::<cl_addressing_mode>(sampler.addressing_mode),
            CL_SAMPLER_CONTEXT => {
                // Note we use as_ptr here which doesn't increase the reference count.
                let ptr = Arc::as_ptr(&sampler.context);
                cl_prop::<cl_context>(cl_context::from_ptr(ptr))
            }
            CL_SAMPLER_FILTER_MODE => cl_prop::<cl_filter_mode>(sampler.filter_mode),
            CL_SAMPLER_NORMALIZED_COORDS => cl_prop::<bool>(sampler.normalized_coords),
            CL_SAMPLER_REFERENCE_COUNT => cl_prop::<cl_uint>(self.refcnt()?),
            CL_SAMPLER_PROPERTIES => {
                cl_prop::<&Option<Properties<cl_sampler_properties>>>(&sampler.props)
            }
            // CL_INVALID_VALUE if param_name is not one of the supported values
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}

fn create_sampler_impl(
    context: cl_context,
    normalized_coords: cl_bool,
    addressing_mode: cl_addressing_mode,
    filter_mode: cl_filter_mode,
    props: Option<Properties<cl_sampler_properties>>,
) -> CLResult<cl_sampler> {
    let c = context.get_arc()?;

    // CL_INVALID_OPERATION if images are not supported by any device associated with context (i.e.
    // CL_DEVICE_IMAGE_SUPPORT specified in the Device Queries table is CL_FALSE).
    c.devs
        .iter()
        .find(|d| d.image_supported())
        .ok_or(CL_INVALID_OPERATION)?;

    // CL_INVALID_VALUE if addressing_mode, filter_mode, normalized_coords or a combination of these
    // arguements are not valid.
    validate_addressing_mode(addressing_mode)?;
    validate_filter_mode(filter_mode)?;

    let sampler = Sampler::new(
        c,
        check_cl_bool(normalized_coords).ok_or(CL_INVALID_VALUE)?,
        addressing_mode,
        filter_mode,
        props,
    );
    Ok(cl_sampler::from_arc(sampler))
}

#[cl_entrypoint]
fn create_sampler(
    context: cl_context,
    normalized_coords: cl_bool,
    addressing_mode: cl_addressing_mode,
    filter_mode: cl_filter_mode,
) -> CLResult<cl_sampler> {
    create_sampler_impl(
        context,
        normalized_coords,
        addressing_mode,
        filter_mode,
        None,
    )
}

#[cl_entrypoint]
fn create_sampler_with_properties(
    context: cl_context,
    sampler_properties: *const cl_sampler_properties,
) -> CLResult<cl_sampler> {
    let mut normalized_coords = CL_TRUE;
    let mut addressing_mode = CL_ADDRESS_CLAMP;
    let mut filter_mode = CL_FILTER_NEAREST;

    // CL_INVALID_VALUE if the same property name is specified more than once.
    let sampler_properties = if sampler_properties.is_null() {
        None
    } else {
        let sampler_properties =
            Properties::from_ptr(sampler_properties).ok_or(CL_INVALID_VALUE)?;
        for p in &sampler_properties.props {
            match p.0 as u32 {
                CL_SAMPLER_ADDRESSING_MODE => addressing_mode = p.1 as u32,
                CL_SAMPLER_FILTER_MODE => filter_mode = p.1 as u32,
                CL_SAMPLER_NORMALIZED_COORDS => normalized_coords = p.1 as u32,
                // CL_INVALID_VALUE if the property name in sampler_properties is not a supported
                // property name
                _ => return Err(CL_INVALID_VALUE),
            }
        }
        Some(sampler_properties)
    };

    create_sampler_impl(
        context,
        normalized_coords,
        addressing_mode,
        filter_mode,
        sampler_properties,
    )
}

#[cl_entrypoint]
fn retain_sampler(sampler: cl_sampler) -> CLResult<()> {
    sampler.retain()
}

#[cl_entrypoint]
fn release_sampler(sampler: cl_sampler) -> CLResult<()> {
    sampler.release()
}

#[cl_entrypoint]
fn enqueue_read_buffer(
    command_queue: cl_command_queue,
    buffer: cl_mem,
    blocking_read: cl_bool,
    offset: usize,
    cb: usize,
    ptr: *mut ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let b = buffer.get_arc()?;
    let block = check_cl_bool(blocking_read).ok_or(CL_INVALID_VALUE)?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_VALUE if the region being read or written specified by (offset, size) is out of
    // bounds or if ptr is a NULL value.
    if offset + cb > b.size || ptr.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_CONTEXT if the context associated with command_queue and buffer are not the same
    if b.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the read and write operations are blocking
    // and the execution status of any of the events in event_wait_list is a negative integer value.
    if block && evs.iter().any(|e| e.is_error()) {
        return Err(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    }

    // CL_INVALID_OPERATION if clEnqueueReadBuffer is called on buffer which has been created with
    // CL_MEM_HOST_WRITE_ONLY or CL_MEM_HOST_NO_ACCESS.
    if bit_check(b.flags, CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS) {
        return Err(CL_INVALID_OPERATION);
    }

    create_and_queue(
        q,
        CL_COMMAND_READ_BUFFER,
        evs,
        event,
        block,
        Box::new(move |q, ctx| b.read_to_user(q, ctx, offset, ptr, cb)),
    )

    // TODO
    // CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
}

#[cl_entrypoint]
fn enqueue_write_buffer(
    command_queue: cl_command_queue,
    buffer: cl_mem,
    blocking_write: cl_bool,
    offset: usize,
    cb: usize,
    ptr: *const ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let b = buffer.get_arc()?;
    let block = check_cl_bool(blocking_write).ok_or(CL_INVALID_VALUE)?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_VALUE if the region being read or written specified by (offset, size) is out of
    // bounds or if ptr is a NULL value.
    if offset + cb > b.size || ptr.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_CONTEXT if the context associated with command_queue and buffer are not the same
    if b.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the read and write operations are blocking
    // and the execution status of any of the events in event_wait_list is a negative integer value.
    if block && evs.iter().any(|e| e.is_error()) {
        return Err(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    }

    // CL_INVALID_OPERATION if clEnqueueWriteBuffer is called on buffer which has been created with
    // CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS.
    if bit_check(b.flags, CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS) {
        return Err(CL_INVALID_OPERATION);
    }

    create_and_queue(
        q,
        CL_COMMAND_WRITE_BUFFER,
        evs,
        event,
        block,
        Box::new(move |q, ctx| b.write_from_user(q, ctx, offset, ptr, cb)),
    )

    // TODO
    // CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
}

#[cl_entrypoint]
fn enqueue_copy_buffer(
    command_queue: cl_command_queue,
    src_buffer: cl_mem,
    dst_buffer: cl_mem,
    src_offset: usize,
    dst_offset: usize,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let src = src_buffer.get_arc()?;
    let dst = dst_buffer.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_CONTEXT if the context associated with command_queue, src_buffer and dst_buffer
    // are not the same
    if q.context != src.context || q.context != dst.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_VALUE if src_offset, dst_offset, size, src_offset + size or dst_offset + size
    // require accessing elements outside the src_buffer and dst_buffer buffer objects respectively.
    if src_offset + size > src.size || dst_offset + size > dst.size {
        return Err(CL_INVALID_VALUE);
    }

    // CL_MEM_COPY_OVERLAP if src_buffer and dst_buffer are the same buffer or sub-buffer object
    // and the source and destination regions overlap or if src_buffer and dst_buffer are different
    // sub-buffers of the same associated buffer object and they overlap. The regions overlap if
    // src_offset ≤ dst_offset ≤ src_offset + size - 1 or if dst_offset ≤ src_offset ≤ dst_offset + size - 1.
    if src.has_same_parent(&dst) {
        let src_offset = src_offset + src.offset;
        let dst_offset = dst_offset + dst.offset;

        if (src_offset <= dst_offset && dst_offset < src_offset + size)
            || (dst_offset <= src_offset && src_offset < dst_offset + size)
        {
            return Err(CL_MEM_COPY_OVERLAP);
        }
    }

    create_and_queue(
        q,
        CL_COMMAND_COPY_BUFFER,
        evs,
        event,
        false,
        Box::new(move |q, ctx| {
            src.copy_to(
                q,
                ctx,
                &dst,
                CLVec::new([src_offset, 0, 0]),
                CLVec::new([dst_offset, 0, 0]),
                &CLVec::new([size, 1, 1]),
            )
        }),
    )

    // TODO
    //• CL_MISALIGNED_SUB_BUFFER_OFFSET if src_buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
    //• CL_MISALIGNED_SUB_BUFFER_OFFSET if dst_buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
    //• CL_MEM_OBJECT_ALLOCATION_FAILURE if there is a failure to allocate memory for data store associated with src_buffer or dst_buffer.
}

#[cl_entrypoint]
fn enqueue_read_buffer_rect(
    command_queue: cl_command_queue,
    buffer: cl_mem,
    blocking_read: cl_bool,
    buffer_origin: *const usize,
    host_origin: *const usize,
    region: *const usize,
    mut buffer_row_pitch: usize,
    mut buffer_slice_pitch: usize,
    mut host_row_pitch: usize,
    mut host_slice_pitch: usize,
    ptr: *mut ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let block = check_cl_bool(blocking_read).ok_or(CL_INVALID_VALUE)?;
    let q = command_queue.get_arc()?;
    let buf = buffer.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_OPERATION if clEnqueueReadBufferRect is called on buffer which has been created
    // with CL_MEM_HOST_WRITE_ONLY or CL_MEM_HOST_NO_ACCESS.
    if bit_check(buf.flags, CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS) {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the read and write operations are blocking
    // and the execution status of any of the events in event_wait_list is a negative integer value.
    if block && evs.iter().any(|e| e.is_error()) {
        return Err(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    }

    // CL_INVALID_VALUE if buffer_origin, host_origin, or region is NULL.
    if buffer_origin.is_null() ||
      host_origin.is_null() ||
      region.is_null() ||
      // CL_INVALID_VALUE if ptr is NULL.
      ptr.is_null()
    {
        return Err(CL_INVALID_VALUE);
    }

    let r = unsafe { CLVec::from_raw(region) };
    let buf_ori = unsafe { CLVec::from_raw(buffer_origin) };
    let host_ori = unsafe { CLVec::from_raw(host_origin) };

    // CL_INVALID_VALUE if any region array element is 0.
    if r.contains(&0) ||
      // CL_INVALID_VALUE if buffer_row_pitch is not 0 and is less than region[0].
      buffer_row_pitch != 0 && buffer_row_pitch < r[0] ||
      // CL_INVALID_VALUE if host_row_pitch is not 0 and is less than region[0].
      host_row_pitch != 0 && host_row_pitch < r[0]
    {
        return Err(CL_INVALID_VALUE);
    }

    // If buffer_row_pitch is 0, buffer_row_pitch is computed as region[0].
    if buffer_row_pitch == 0 {
        buffer_row_pitch = r[0];
    }

    // If host_row_pitch is 0, host_row_pitch is computed as region[0].
    if host_row_pitch == 0 {
        host_row_pitch = r[0];
    }

    // CL_INVALID_VALUE if buffer_slice_pitch is not 0 and is less than region[1] × buffer_row_pitch and not a multiple of buffer_row_pitch.
    if buffer_slice_pitch != 0 && buffer_slice_pitch < r[1] * buffer_row_pitch && buffer_slice_pitch % buffer_row_pitch != 0 ||
      // CL_INVALID_VALUE if host_slice_pitch is not 0 and is less than region[1] × host_row_pitch and not a multiple of host_row_pitch.
      host_slice_pitch != 0 && host_slice_pitch < r[1] * host_row_pitch && host_slice_pitch % host_row_pitch != 0
    {
        return Err(CL_INVALID_VALUE);
    }

    // If buffer_slice_pitch is 0, buffer_slice_pitch is computed as region[1] × buffer_row_pitch.
    if buffer_slice_pitch == 0 {
        buffer_slice_pitch = r[1] * buffer_row_pitch;
    }

    // If host_slice_pitch is 0, host_slice_pitch is computed as region[1] × host_row_pitch.
    if host_slice_pitch == 0 {
        host_slice_pitch = r[1] * host_row_pitch
    }

    // CL_INVALID_VALUE if the region being read or written specified by (buffer_origin, region,
    // buffer_row_pitch, buffer_slice_pitch) is out of bounds.
    if CLVec::calc_size(r + buf_ori, [1, buffer_row_pitch, buffer_slice_pitch]) > buf.size {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_CONTEXT if the context associated with command_queue and buffer are not the same
    if q.context != buf.context {
        return Err(CL_INVALID_CONTEXT);
    }

    create_and_queue(
        q,
        CL_COMMAND_READ_BUFFER_RECT,
        evs,
        event,
        block,
        Box::new(move |q, ctx| {
            buf.read_to_user_rect(
                ptr,
                q,
                ctx,
                &r,
                &buf_ori,
                buffer_row_pitch,
                buffer_slice_pitch,
                &host_ori,
                host_row_pitch,
                host_slice_pitch,
            )
        }),
    )

    // TODO
    // CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
}

#[cl_entrypoint]
fn enqueue_write_buffer_rect(
    command_queue: cl_command_queue,
    buffer: cl_mem,
    blocking_write: cl_bool,
    buffer_origin: *const usize,
    host_origin: *const usize,
    region: *const usize,
    mut buffer_row_pitch: usize,
    mut buffer_slice_pitch: usize,
    mut host_row_pitch: usize,
    mut host_slice_pitch: usize,
    ptr: *const ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let block = check_cl_bool(blocking_write).ok_or(CL_INVALID_VALUE)?;
    let q = command_queue.get_arc()?;
    let buf = buffer.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_OPERATION if clEnqueueWriteBufferRect is called on buffer which has been created
    // with CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS.
    if bit_check(buf.flags, CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS) {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the read and write operations are blocking
    // and the execution status of any of the events in event_wait_list is a negative integer value.
    if block && evs.iter().any(|e| e.is_error()) {
        return Err(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    }

    // CL_INVALID_VALUE if buffer_origin, host_origin, or region is NULL.
    if buffer_origin.is_null() ||
      host_origin.is_null() ||
      region.is_null() ||
      // CL_INVALID_VALUE if ptr is NULL.
      ptr.is_null()
    {
        return Err(CL_INVALID_VALUE);
    }

    let r = unsafe { CLVec::from_raw(region) };
    let buf_ori = unsafe { CLVec::from_raw(buffer_origin) };
    let host_ori = unsafe { CLVec::from_raw(host_origin) };

    // CL_INVALID_VALUE if any region array element is 0.
    if r.contains(&0) ||
      // CL_INVALID_VALUE if buffer_row_pitch is not 0 and is less than region[0].
      buffer_row_pitch != 0 && buffer_row_pitch < r[0] ||
      // CL_INVALID_VALUE if host_row_pitch is not 0 and is less than region[0].
      host_row_pitch != 0 && host_row_pitch < r[0]
    {
        return Err(CL_INVALID_VALUE);
    }

    // If buffer_row_pitch is 0, buffer_row_pitch is computed as region[0].
    if buffer_row_pitch == 0 {
        buffer_row_pitch = r[0];
    }

    // If host_row_pitch is 0, host_row_pitch is computed as region[0].
    if host_row_pitch == 0 {
        host_row_pitch = r[0];
    }

    // CL_INVALID_VALUE if buffer_slice_pitch is not 0 and is less than region[1] × buffer_row_pitch and not a multiple of buffer_row_pitch.
    if buffer_slice_pitch != 0 && buffer_slice_pitch < r[1] * buffer_row_pitch && buffer_slice_pitch % buffer_row_pitch != 0 ||
      // CL_INVALID_VALUE if host_slice_pitch is not 0 and is less than region[1] × host_row_pitch and not a multiple of host_row_pitch.
      host_slice_pitch != 0 && host_slice_pitch < r[1] * host_row_pitch && host_slice_pitch % host_row_pitch != 0
    {
        return Err(CL_INVALID_VALUE);
    }

    // If buffer_slice_pitch is 0, buffer_slice_pitch is computed as region[1] × buffer_row_pitch.
    if buffer_slice_pitch == 0 {
        buffer_slice_pitch = r[1] * buffer_row_pitch;
    }

    // If host_slice_pitch is 0, host_slice_pitch is computed as region[1] × host_row_pitch.
    if host_slice_pitch == 0 {
        host_slice_pitch = r[1] * host_row_pitch
    }

    // CL_INVALID_VALUE if the region being read or written specified by (buffer_origin, region,
    // buffer_row_pitch, buffer_slice_pitch) is out of bounds.
    if CLVec::calc_size(r + buf_ori, [1, buffer_row_pitch, buffer_slice_pitch]) > buf.size {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_CONTEXT if the context associated with command_queue and buffer are not the same
    if q.context != buf.context {
        return Err(CL_INVALID_CONTEXT);
    }

    create_and_queue(
        q,
        CL_COMMAND_WRITE_BUFFER_RECT,
        evs,
        event,
        block,
        Box::new(move |q, ctx| {
            buf.write_from_user_rect(
                ptr,
                q,
                ctx,
                &r,
                &host_ori,
                host_row_pitch,
                host_slice_pitch,
                &buf_ori,
                buffer_row_pitch,
                buffer_slice_pitch,
            )
        }),
    )

    // TODO
    // CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
}

#[cl_entrypoint]
fn enqueue_copy_buffer_rect(
    command_queue: cl_command_queue,
    src_buffer: cl_mem,
    dst_buffer: cl_mem,
    src_origin: *const usize,
    dst_origin: *const usize,
    region: *const usize,
    mut src_row_pitch: usize,
    mut src_slice_pitch: usize,
    mut dst_row_pitch: usize,
    mut dst_slice_pitch: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let src = src_buffer.get_arc()?;
    let dst = dst_buffer.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_VALUE if src_origin, dst_origin, or region is NULL.
    if src_origin.is_null() || dst_origin.is_null() || region.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    let r = unsafe { CLVec::from_raw(region) };
    let src_ori = unsafe { CLVec::from_raw(src_origin) };
    let dst_ori = unsafe { CLVec::from_raw(dst_origin) };

    // CL_INVALID_VALUE if any region array element is 0.
    if r.contains(&0) ||
      // CL_INVALID_VALUE if src_row_pitch is not 0 and is less than region[0].
      src_row_pitch != 0 && src_row_pitch < r[0] ||
      // CL_INVALID_VALUE if dst_row_pitch is not 0 and is less than region[0].
      dst_row_pitch != 0 && dst_row_pitch < r[0]
    {
        return Err(CL_INVALID_VALUE);
    }

    // If src_row_pitch is 0, src_row_pitch is computed as region[0].
    if src_row_pitch == 0 {
        src_row_pitch = r[0];
    }

    // If dst_row_pitch is 0, dst_row_pitch is computed as region[0].
    if dst_row_pitch == 0 {
        dst_row_pitch = r[0];
    }

    // CL_INVALID_VALUE if src_slice_pitch is not 0 and is less than region[1] × src_row_pitch
    if src_slice_pitch != 0 && src_slice_pitch < r[1] * src_row_pitch ||
      // CL_INVALID_VALUE if dst_slice_pitch is not 0 and is less than region[1] × dst_row_pitch
      dst_slice_pitch != 0 && dst_slice_pitch < r[1] * dst_row_pitch ||
      // if src_slice_pitch is not 0 and is not a multiple of src_row_pitch.
      src_slice_pitch != 0 && src_slice_pitch % src_row_pitch != 0 ||
      // if dst_slice_pitch is not 0 and is not a multiple of dst_row_pitch.
      dst_slice_pitch != 0 && dst_slice_pitch % dst_row_pitch != 0
    {
        return Err(CL_INVALID_VALUE);
    }

    // If src_slice_pitch is 0, src_slice_pitch is computed as region[1] × src_row_pitch.
    if src_slice_pitch == 0 {
        src_slice_pitch = r[1] * src_row_pitch;
    }

    // If dst_slice_pitch is 0, dst_slice_pitch is computed as region[1] × dst_row_pitch.
    if dst_slice_pitch == 0 {
        dst_slice_pitch = r[1] * dst_row_pitch;
    }

    // CL_INVALID_VALUE if src_buffer and dst_buffer are the same buffer object and src_slice_pitch
    // is not equal to dst_slice_pitch and src_row_pitch is not equal to dst_row_pitch.
    if src_buffer == dst_buffer
        && src_slice_pitch != dst_slice_pitch
        && src_row_pitch != dst_row_pitch
    {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE if (src_origin, region, src_row_pitch, src_slice_pitch) or (dst_origin,
    // region, dst_row_pitch, dst_slice_pitch) require accessing elements outside the src_buffer
    // and dst_buffer buffer objects respectively.
    if CLVec::calc_size(r + src_ori, [1, src_row_pitch, src_slice_pitch]) > src.size
        || CLVec::calc_size(r + dst_ori, [1, dst_row_pitch, dst_slice_pitch]) > dst.size
    {
        return Err(CL_INVALID_VALUE);
    }

    // CL_MEM_COPY_OVERLAP if src_buffer and dst_buffer are the same buffer or sub-buffer object and
    // the source and destination regions overlap or if src_buffer and dst_buffer are different
    // sub-buffers of the same associated buffer object and they overlap.
    if src.has_same_parent(&dst)
        && check_copy_overlap(
            &src_ori,
            src.offset,
            &dst_ori,
            dst.offset,
            &r,
            src_row_pitch,
            src_slice_pitch,
        )
    {
        return Err(CL_MEM_COPY_OVERLAP);
    }

    // CL_INVALID_CONTEXT if the context associated with command_queue, src_buffer and dst_buffer
    // are not the same
    if src.context != q.context || dst.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    create_and_queue(
        q,
        CL_COMMAND_COPY_BUFFER_RECT,
        evs,
        event,
        false,
        Box::new(move |q, ctx| {
            src.copy_to_rect(
                &dst,
                q,
                ctx,
                &r,
                &src_ori,
                src_row_pitch,
                src_slice_pitch,
                &dst_ori,
                dst_row_pitch,
                dst_slice_pitch,
            )
        }),
    )

    // TODO
    // CL_MISALIGNED_SUB_BUFFER_OFFSET if src_buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
}

#[cl_entrypoint]
fn enqueue_fill_buffer(
    command_queue: cl_command_queue,
    buffer: cl_mem,
    pattern: *const ::std::os::raw::c_void,
    pattern_size: usize,
    offset: usize,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let b = buffer.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_VALUE if offset or offset + size require accessing elements outside the buffer
    // buffer object respectively.
    if offset + size > b.size {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE if pattern is NULL or if pattern_size is 0 or if pattern_size is not one of
    // { 1, 2, 4, 8, 16, 32, 64, 128 }.
    if pattern.is_null() || pattern_size.count_ones() != 1 || pattern_size > 128 {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE if offset and size are not a multiple of pattern_size.
    if offset % pattern_size != 0 || size % pattern_size != 0 {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_CONTEXT if the context associated with command_queue and buffer are not the same
    if b.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // we have to copy memory
    let pattern = unsafe { slice::from_raw_parts(pattern.cast(), pattern_size).to_vec() };
    create_and_queue(
        q,
        CL_COMMAND_FILL_BUFFER,
        evs,
        event,
        false,
        Box::new(move |q, ctx| b.fill(q, ctx, &pattern, offset, size)),
    )

    // TODO
    //• CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
    //• CL_MEM_OBJECT_ALLOCATION_FAILURE if there is a failure to allocate memory for data store associated with buffer.
}

#[cl_entrypoint]
fn enqueue_map_buffer(
    command_queue: cl_command_queue,
    buffer: cl_mem,
    blocking_map: cl_bool,
    map_flags: cl_map_flags,
    offset: usize,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<*mut c_void> {
    let q = command_queue.get_arc()?;
    let b = buffer.get_arc()?;
    let block = check_cl_bool(blocking_map).ok_or(CL_INVALID_VALUE)?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    validate_map_flags(&b, map_flags)?;

    // CL_INVALID_VALUE if region being mapped given by (offset, size) is out of bounds or if size
    // is 0
    if offset + size > b.size || size == 0 {
        return Err(CL_INVALID_VALUE);
    }

    // CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the map operation is blocking and the
    // execution status of any of the events in event_wait_list is a negative integer value.
    if block && evs.iter().any(|e| e.is_error()) {
        return Err(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
    }

    // CL_INVALID_CONTEXT if context associated with command_queue and buffer are not the same
    if b.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    let ptr = b.map_buffer(q.device, offset, size)?;
    create_and_queue(
        q,
        CL_COMMAND_MAP_BUFFER,
        evs,
        event,
        block,
        Box::new(move |q, ctx| b.sync_shadow_buffer(q, ctx, ptr)),
    )?;

    Ok(ptr)

    // TODO
    // CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for the device associated with queue. This error code is missing before version 1.1.
    // CL_MAP_FAILURE if there is a failure to map the requested region into the host address space. This error cannot occur for buffer objects created with CL_MEM_USE_HOST_PTR or CL_MEM_ALLOC_HOST_PTR.
    // CL_INVALID_OPERATION if mapping would lead to overlapping regions being mapped for writing.
}

#[cl_entrypoint]
fn enqueue_read_image(
    command_queue: cl_command_queue,
    image: cl_mem,
    blocking_read: cl_bool,
    origin: *const usize,
    region: *const usize,
    mut row_pitch: usize,
    mut slice_pitch: usize,
    ptr: *mut ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let i = image.get_arc()?;
    let block = check_cl_bool(blocking_read).ok_or(CL_INVALID_VALUE)?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;
    let pixel_size = i.image_format.pixel_size().unwrap() as usize;

    // CL_INVALID_CONTEXT if the context associated with command_queue and image are not the same
    if i.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_OPERATION if clEnqueueReadImage is called on image which has been created with
    // CL_MEM_HOST_WRITE_ONLY or CL_MEM_HOST_NO_ACCESS.
    if bit_check(i.flags, CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS) {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_INVALID_VALUE if origin or region is NULL.
    // CL_INVALID_VALUE if ptr is NULL.
    if origin.is_null() || region.is_null() || ptr.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE if image is a 1D or 2D image and slice_pitch or input_slice_pitch is not 0.
    if !i.image_desc.has_slice() && slice_pitch != 0 {
        return Err(CL_INVALID_VALUE);
    }

    let r = unsafe { CLVec::from_raw(region) };
    let o = unsafe { CLVec::from_raw(origin) };

    // CL_INVALID_VALUE if the region being read or written specified by origin and region is out of
    // bounds.
    // CL_INVALID_VALUE if values in origin and region do not follow rules described in the argument
    // description for origin and region.
    validate_image_bounds(&i, o, r)?;

    // If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated based
    // on the size of each element in bytes multiplied by width.
    if row_pitch == 0 {
        row_pitch = r[0] * pixel_size;
    }

    // If slice_pitch (or input_slice_pitch) is set to 0, the appropriate slice pitch is calculated
    // based on the row_pitch × height.
    if slice_pitch == 0 {
        slice_pitch = row_pitch * r[1];
    }

    create_and_queue(
        q,
        CL_COMMAND_READ_IMAGE,
        evs,
        event,
        block,
        Box::new(move |q, ctx| {
            i.read_to_user_rect(
                ptr,
                q,
                ctx,
                &r,
                &o,
                i.image_desc.image_row_pitch,
                i.image_desc.image_slice_pitch,
                &CLVec::default(),
                row_pitch,
                slice_pitch,
            )
        }),
    )

    //• CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified or compute row and/or slice pitch) for image are not supported by device associated with queue.
    //• CL_IMAGE_FORMAT_NOT_SUPPORTED if image format (image channel order and data type) for image are not supported by device associated with queue.
    //• CL_INVALID_OPERATION if the device associated with command_queue does not support images (i.e. CL_DEVICE_IMAGE_SUPPORT specified in the Device Queries table is CL_FALSE).
    //• CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the read and write operations are blocking and the execution status of any of the events in event_wait_list is a negative integer value.
}

#[cl_entrypoint]
fn enqueue_write_image(
    command_queue: cl_command_queue,
    image: cl_mem,
    blocking_write: cl_bool,
    origin: *const usize,
    region: *const usize,
    mut row_pitch: usize,
    mut slice_pitch: usize,
    ptr: *const ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let i = image.get_arc()?;
    let block = check_cl_bool(blocking_write).ok_or(CL_INVALID_VALUE)?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;
    let pixel_size = i.image_format.pixel_size().unwrap() as usize;

    // CL_INVALID_CONTEXT if the context associated with command_queue and image are not the same
    if i.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_OPERATION if clEnqueueWriteImage is called on image which has been created with
    // CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS.
    if bit_check(i.flags, CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS) {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_INVALID_VALUE if origin or region is NULL.
    // CL_INVALID_VALUE if ptr is NULL.
    if origin.is_null() || region.is_null() || ptr.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE if image is a 1D or 2D image and slice_pitch or input_slice_pitch is not 0.
    if !i.image_desc.has_slice() && slice_pitch != 0 {
        return Err(CL_INVALID_VALUE);
    }

    let r = unsafe { CLVec::from_raw(region) };
    let o = unsafe { CLVec::from_raw(origin) };

    // CL_INVALID_VALUE if the region being read or written specified by origin and region is out of
    // bounds.
    // CL_INVALID_VALUE if values in origin and region do not follow rules described in the argument
    // description for origin and region.
    validate_image_bounds(&i, o, r)?;

    // If row_pitch (or input_row_pitch) is set to 0, the appropriate row pitch is calculated based
    // on the size of each element in bytes multiplied by width.
    if row_pitch == 0 {
        row_pitch = r[0] * pixel_size;
    }

    // If slice_pitch (or input_slice_pitch) is set to 0, the appropriate slice pitch is calculated
    // based on the row_pitch × height.
    if slice_pitch == 0 {
        slice_pitch = row_pitch * r[1];
    }

    create_and_queue(
        q,
        CL_COMMAND_WRITE_BUFFER_RECT,
        evs,
        event,
        block,
        Box::new(move |q, ctx| {
            i.write_from_user_rect(
                ptr,
                q,
                ctx,
                &r,
                &CLVec::default(),
                row_pitch,
                slice_pitch,
                &o,
                i.image_desc.image_row_pitch,
                i.image_desc.image_slice_pitch,
            )
        }),
    )

    //• CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified or compute row and/or slice pitch) for image are not supported by device associated with queue.
    //• CL_IMAGE_FORMAT_NOT_SUPPORTED if image format (image channel order and data type) for image are not supported by device associated with queue.
    //• CL_INVALID_OPERATION if the device associated with command_queue does not support images (i.e. CL_DEVICE_IMAGE_SUPPORT specified in the Device Queries table is CL_FALSE).
    //• CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the read and write operations are blocking and the execution status of any of the events in event_wait_list is a negative integer value.
}

#[cl_entrypoint]
fn enqueue_copy_image(
    command_queue: cl_command_queue,
    src_image: cl_mem,
    dst_image: cl_mem,
    src_origin: *const usize,
    dst_origin: *const usize,
    region: *const usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let src_image = src_image.get_arc()?;
    let dst_image = dst_image.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_CONTEXT if the context associated with command_queue, src_image and dst_image are not the same
    if src_image.context != q.context || dst_image.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_IMAGE_FORMAT_MISMATCH if src_image and dst_image do not use the same image format.
    if src_image.image_format != dst_image.image_format {
        return Err(CL_IMAGE_FORMAT_MISMATCH);
    }

    // CL_INVALID_VALUE if src_origin, dst_origin, or region is NULL.
    if src_origin.is_null() || dst_origin.is_null() || region.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    let region = unsafe { CLVec::from_raw(region) };
    let dst_origin = unsafe { CLVec::from_raw(dst_origin) };
    let src_origin = unsafe { CLVec::from_raw(src_origin) };

    // CL_INVALID_VALUE if the 2D or 3D rectangular region specified by src_origin and
    // src_origin + region refers to a region outside src_image, or if the 2D or 3D rectangular
    // region specified by dst_origin and dst_origin + region refers to a region outside dst_image.
    // CL_INVALID_VALUE if values in src_origin, dst_origin and region do not follow rules described
    // in the argument description for src_origin, dst_origin and region.
    validate_image_bounds(&src_image, src_origin, region)?;
    validate_image_bounds(&dst_image, dst_origin, region)?;

    create_and_queue(
        q,
        CL_COMMAND_COPY_IMAGE,
        evs,
        event,
        false,
        Box::new(move |q, ctx| {
            src_image.copy_to(q, ctx, &dst_image, src_origin, dst_origin, &region)
        }),
    )

    //• CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified or compute row and/or slice pitch) for src_image or dst_image are not supported by device associated with queue.
    //• CL_IMAGE_FORMAT_NOT_SUPPORTED if image format (image channel order and data type) for src_image or dst_image are not supported by device associated with queue.
    //• CL_INVALID_OPERATION if the device associated with command_queue does not support images (i.e. CL_DEVICE_IMAGE_SUPPORT specified in the Device Queries table is CL_FALSE).
    //• CL_MEM_COPY_OVERLAP if src_image and dst_image are the same image object and the source and destination regions overlap.
}

#[cl_entrypoint]
fn enqueue_fill_image(
    command_queue: cl_command_queue,
    image: cl_mem,
    fill_color: *const ::std::os::raw::c_void,
    origin: *const [usize; 3],
    region: *const [usize; 3],
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let i = image.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_CONTEXT if the context associated with command_queue and image are not the same
    if i.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_VALUE if fill_color is NULL.
    // CL_INVALID_VALUE if origin or region is NULL.
    if fill_color.is_null() || origin.is_null() || region.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    let region = unsafe { CLVec::from_raw(region.cast()) };
    let origin = unsafe { CLVec::from_raw(origin.cast()) };

    // CL_INVALID_VALUE if the region being filled as specified by origin and region is out of
    // bounds.
    // CL_INVALID_VALUE if values in origin and region do not follow rules described in the argument
    // description for origin and region.
    validate_image_bounds(&i, origin, region)?;

    // we have to copy memory and it's always a 4 component int value
    // TODO but not for CL_DEPTH
    let fill_color = unsafe { slice::from_raw_parts(fill_color.cast(), 4).to_vec() };
    create_and_queue(
        q,
        CL_COMMAND_FILL_BUFFER,
        evs,
        event,
        false,
        Box::new(move |q, ctx| i.fill_image(q, ctx, &fill_color, &origin, &region)),
    )

    //• CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified or compute row and/or slice pitch) for image are not supported by device associated with queue.
    //• CL_IMAGE_FORMAT_NOT_SUPPORTED if image format (image channel order and data type) for
    //image are not supported by device associated with queue.
}

#[cl_entrypoint]
fn enqueue_copy_buffer_to_image(
    command_queue: cl_command_queue,
    src_buffer: cl_mem,
    dst_image: cl_mem,
    src_offset: usize,
    dst_origin: *const usize,
    region: *const usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let src = src_buffer.get_arc()?;
    let dst = dst_image.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_CONTEXT if the context associated with command_queue, src_buffer and dst_image
    // are not the same
    if q.context != src.context || q.context != dst.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_VALUE if dst_origin or region is NULL.
    if dst_origin.is_null() || region.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    let region = unsafe { CLVec::from_raw(region) };
    let src_origin = CLVec::new([src_offset, 0, 0]);
    let dst_origin = unsafe { CLVec::from_raw(dst_origin) };

    // CL_INVALID_VALUE if values in dst_origin and region do not follow rules described in the
    // argument description for dst_origin and region.
    // CL_INVALID_VALUE if the 1D, 2D or 3D rectangular region specified by dst_origin and
    // dst_origin + region refer to a region outside dst_image,
    validate_image_bounds(&dst, dst_origin, region)?;

    create_and_queue(
        q,
        CL_COMMAND_COPY_BUFFER_TO_IMAGE,
        evs,
        event,
        false,
        Box::new(move |q, ctx| src.copy_to(q, ctx, &dst, src_origin, dst_origin, &region)),
    )

    //• CL_INVALID_MEM_OBJECT if src_buffer is not a valid buffer object or dst_image is not a valid image object or if dst_image is a 1D image buffer object created from src_buffer.
    //• CL_INVALID_VALUE ... if the region specified by src_offset and src_offset + src_cb refer to a region outside src_buffer.
    //• CL_MISALIGNED_SUB_BUFFER_OFFSET if src_buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
    //• CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified or compute row and/or slice pitch) for dst_image are not supported by device associated with queue.
    //• CL_IMAGE_FORMAT_NOT_SUPPORTED if image format (image channel order and data type) for dst_image are not supported by device associated with queue.
    //• CL_MEM_OBJECT_ALLOCATION_FAILURE if there is a failure to allocate memory for data store associated with src_buffer or dst_image.
    //• CL_INVALID_OPERATION if the device associated with command_queue does not support images (i.e. CL_DEVICE_IMAGE_SUPPORT specified in the Device Queries table is CL_FALSE).
}

#[cl_entrypoint]
fn enqueue_copy_image_to_buffer(
    command_queue: cl_command_queue,
    src_image: cl_mem,
    dst_buffer: cl_mem,
    src_origin: *const usize,
    region: *const usize,
    dst_offset: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let src = src_image.get_arc()?;
    let dst = dst_buffer.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_CONTEXT if the context associated with command_queue, src_image and dst_buffer
    // are not the same
    if q.context != src.context || q.context != dst.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_VALUE if src_origin or region is NULL.
    if src_origin.is_null() || region.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    let region = unsafe { CLVec::from_raw(region) };
    let src_origin = unsafe { CLVec::from_raw(src_origin) };
    let dst_origin = CLVec::new([dst_offset, 0, 0]);

    // CL_INVALID_VALUE if values in src_origin and region do not follow rules described in the
    // argument description for src_origin and region.
    // CL_INVALID_VALUE if the 1D, 2D or 3D rectangular region specified by src_origin and
    // src_origin + region refers to a region outside src_image, or if the region specified by
    // dst_offset and dst_offset + dst_cb to a region outside dst_buffer.
    validate_image_bounds(&src, src_origin, region)?;

    create_and_queue(
        q,
        CL_COMMAND_COPY_IMAGE_TO_BUFFER,
        evs,
        event,
        false,
        Box::new(move |q, ctx| src.copy_to(q, ctx, &dst, src_origin, dst_origin, &region)),
    )

    //• CL_INVALID_MEM_OBJECT if src_image is not a valid image object or dst_buffer is not a valid buffer object or if src_image is a 1D image buffer object created from dst_buffer.
    //• CL_INVALID_VALUE ... if the region specified by dst_offset and dst_offset + dst_cb to a region outside dst_buffer.
    //• CL_MISALIGNED_SUB_BUFFER_OFFSET if dst_buffer is a sub-buffer object and offset specified when the sub-buffer object is created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue. This error code is missing before version 1.1.
    //• CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified or compute row and/or slice pitch) for src_image are not supported by device associated with queue.
    //• CL_IMAGE_FORMAT_NOT_SUPPORTED if image format (image channel order and data type) for src_image are not supported by device associated with queue.
    //• CL_MEM_OBJECT_ALLOCATION_FAILURE if there is a failure to allocate memory for data store associated with src_image or dst_buffer.
    //• CL_INVALID_OPERATION if the device associated with command_queue does not support images (i.e. CL_DEVICE_IMAGE_SUPPORT specified in the Device Queries table is CL_FALSE).
}

#[cl_entrypoint]
fn enqueue_map_image(
    command_queue: cl_command_queue,
    image: cl_mem,
    blocking_map: cl_bool,
    map_flags: cl_map_flags,
    origin: *const usize,
    region: *const usize,
    image_row_pitch: *mut usize,
    image_slice_pitch: *mut usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<*mut ::std::os::raw::c_void> {
    let q = command_queue.get_arc()?;
    let i = image.get_arc()?;
    let block = check_cl_bool(blocking_map).ok_or(CL_INVALID_VALUE)?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_VALUE ... or if values specified in map_flags are not valid.
    validate_map_flags(&i, map_flags)?;

    // CL_INVALID_CONTEXT if context associated with command_queue and image are not the same
    if i.context != q.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_VALUE if origin or region is NULL.
    // CL_INVALID_VALUE if image_row_pitch is NULL.
    if origin.is_null() || region.is_null() || image_row_pitch.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    let region = unsafe { CLVec::from_raw(region) };
    let origin = unsafe { CLVec::from_raw(origin) };

    // CL_INVALID_VALUE if region being mapped given by (origin, origin + region) is out of bounds
    // CL_INVALID_VALUE if values in origin and region do not follow rules described in the argument
    // description for origin and region.
    validate_image_bounds(&i, origin, region)?;

    let mut dummy_slice_pitch: usize = 0;
    let image_slice_pitch = if image_slice_pitch.is_null() {
        // CL_INVALID_VALUE if image is a 3D image, 1D or 2D image array object and
        // image_slice_pitch is NULL.
        if i.image_desc.is_array() || i.image_desc.image_type == CL_MEM_OBJECT_IMAGE3D {
            return Err(CL_INVALID_VALUE);
        }
        &mut dummy_slice_pitch
    } else {
        unsafe { image_slice_pitch.as_mut().unwrap() }
    };

    let ptr = i.map_image(
        q.device,
        &origin,
        &region,
        unsafe { image_row_pitch.as_mut().unwrap() },
        image_slice_pitch,
    )?;

    create_and_queue(
        q.clone(),
        CL_COMMAND_MAP_IMAGE,
        evs,
        event,
        block,
        Box::new(move |q, ctx| i.sync_shadow_image(q, ctx, ptr)),
    )?;

    Ok(ptr)

    //• CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified or compute row and/or slice pitch) for image are not supported by device associated with queue.
    //• CL_IMAGE_FORMAT_NOT_SUPPORTED if image format (image channel order and data type) for image are not supported by device associated with queue.
    //• CL_MAP_FAILURE if there is a failure to map the requested region into the host address space. This error cannot occur for image objects created with CL_MEM_USE_HOST_PTR or CL_MEM_ALLOC_HOST_PTR.
    //• CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the map operation is blocking and the execution status of any of the events in event_wait_list is a negative integer value.
    //• CL_INVALID_OPERATION if the device associated with command_queue does not support images (i.e. CL_DEVICE_IMAGE_SUPPORT specified in the Device Queries table is CL_FALSE).
    //• CL_INVALID_OPERATION if mapping would lead to overlapping regions being mapped for writing.
}

#[cl_entrypoint]
fn retain_mem_object(mem: cl_mem) -> CLResult<()> {
    mem.retain()
}

#[cl_entrypoint]
fn release_mem_object(mem: cl_mem) -> CLResult<()> {
    mem.release()
}

#[cl_entrypoint]
fn enqueue_unmap_mem_object(
    command_queue: cl_command_queue,
    memobj: cl_mem,
    mapped_ptr: *mut ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let m = memobj.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_CONTEXT if context associated with command_queue and memobj are not the same
    if q.context != m.context {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_VALUE if mapped_ptr is not a valid pointer returned by clEnqueueMapBuffer or
    // clEnqueueMapImage for memobj.
    if !m.is_mapped_ptr(mapped_ptr) {
        return Err(CL_INVALID_VALUE);
    }

    create_and_queue(
        q,
        CL_COMMAND_UNMAP_MEM_OBJECT,
        evs,
        event,
        false,
        Box::new(move |q, ctx| m.unmap(q, ctx, mapped_ptr)),
    )
}

#[cl_entrypoint]
fn enqueue_migrate_mem_objects(
    command_queue: cl_command_queue,
    num_mem_objects: cl_uint,
    mem_objects: *const cl_mem,
    flags: cl_mem_migration_flags,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;
    let bufs = cl_mem::get_arc_vec_from_arr(mem_objects, num_mem_objects)?;

    // CL_INVALID_VALUE if num_mem_objects is zero or if mem_objects is NULL.
    if bufs.is_empty() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_CONTEXT if the context associated with command_queue and memory objects in
    // mem_objects are not the same
    if bufs.iter().any(|b| b.context != q.context) {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_VALUE if flags is not 0 or is not any of the values described in the table above.
    if flags != 0
        && bit_check(
            flags,
            !(CL_MIGRATE_MEM_OBJECT_HOST | CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED),
        )
    {
        return Err(CL_INVALID_VALUE);
    }

    // we should do something, but it's legal to not do anything at all
    create_and_queue(
        q,
        CL_COMMAND_MIGRATE_MEM_OBJECTS,
        evs,
        event,
        false,
        Box::new(|_, _| Ok(())),
    )

    //• CL_MEM_OBJECT_ALLOCATION_FAILURE if there is a failure to allocate memory for the specified set of memory objects in mem_objects.
}

#[cl_info_entrypoint(cl_get_pipe_info)]
impl CLInfo<cl_pipe_info> for cl_mem {
    fn query(&self, _q: cl_pipe_info, _: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>> {
        // CL_INVALID_MEM_OBJECT if pipe is a not a valid pipe object.
        Err(CL_INVALID_MEM_OBJECT)
    }
}

pub fn svm_alloc(
    context: cl_context,
    flags: cl_svm_mem_flags,
    size: usize,
    mut alignment: cl_uint,
) -> CLResult<*mut c_void> {
    // clSVMAlloc will fail if

    // context is not a valid context
    let c = context.get_ref()?;

    // or no devices in context support SVM.
    if !c.has_svm_devs() {
        return Err(CL_INVALID_OPERATION);
    }

    // flags does not contain CL_MEM_SVM_FINE_GRAIN_BUFFER but does contain CL_MEM_SVM_ATOMICS.
    if !bit_check(flags, CL_MEM_SVM_FINE_GRAIN_BUFFER) && bit_check(flags, CL_MEM_SVM_ATOMICS) {
        return Err(CL_INVALID_VALUE);
    }

    // size is 0 or > CL_DEVICE_MAX_MEM_ALLOC_SIZE value for any device in context.
    if size == 0 || checked_compare(size, Ordering::Greater, c.max_mem_alloc()) {
        return Err(CL_INVALID_VALUE);
    }

    if alignment == 0 {
        alignment = mem::size_of::<[u64; 16]>() as cl_uint;
    }

    // alignment is not a power of two
    if !alignment.is_power_of_two() {
        return Err(CL_INVALID_VALUE);
    }

    let layout;
    let ptr;

    // SAFETY: we already verify the parameters to from_size_align above and layout is of non zero
    // size
    unsafe {
        layout = Layout::from_size_align_unchecked(size, alignment as usize);
        ptr = alloc::alloc(layout);
    }

    if ptr.is_null() {
        return Err(CL_OUT_OF_HOST_MEMORY);
    }

    c.add_svm_ptr(ptr.cast(), layout);
    Ok(ptr.cast())

    // Values specified in flags do not follow rules described for supported values in the SVM Memory Flags table.
    // CL_MEM_SVM_FINE_GRAIN_BUFFER or CL_MEM_SVM_ATOMICS is specified in flags and these are not supported by at least one device in context.
    // The values specified in flags are not valid, i.e. don’t match those defined in the SVM Memory Flags table.
    // the OpenCL implementation cannot support the specified alignment for at least one device in context.
    // There was a failure to allocate resources.
}

fn svm_free_impl(c: &Context, svm_pointer: *mut c_void) {
    if let Some(layout) = c.remove_svm_ptr(svm_pointer) {
        // SAFETY: we make sure that svm_pointer is a valid allocation and reuse the same layout
        // from the allocation
        unsafe {
            alloc::dealloc(svm_pointer.cast(), layout);
        }
    }
}

pub fn svm_free(context: cl_context, svm_pointer: *mut c_void) -> CLResult<()> {
    let c = context.get_ref()?;
    svm_free_impl(c, svm_pointer);
    Ok(())
}

fn enqueue_svm_free_impl(
    command_queue: cl_command_queue,
    num_svm_pointers: cl_uint,
    svm_pointers: *mut *mut c_void,
    pfn_free_func: Option<FuncSVMFreeCb>,
    user_data: *mut c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
    cmd_type: cl_command_type,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_VALUE if num_svm_pointers is 0 and svm_pointers is non-NULL, or if svm_pointers is
    // NULL and num_svm_pointers is not 0.
    if num_svm_pointers == 0 && !svm_pointers.is_null()
        || num_svm_pointers != 0 && svm_pointers.is_null()
    {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_OPERATION if the device associated with command queue does not support SVM.
    if !q.device.svm_supported() {
        return Err(CL_INVALID_OPERATION);
    }

    // The application is allowed to reuse or free the memory referenced by `svm_pointers` after this
    // function returns so we have to make a copy.
    // SAFETY: num_svm_pointers specifies the amount of elements in svm_pointers
    let mut svm_pointers =
        unsafe { slice::from_raw_parts(svm_pointers, num_svm_pointers as usize) }.to_vec();
    // SAFETY: The requirements on `SVMFreeCb::new` match the requirements
    // imposed by the OpenCL specification. It is the caller's duty to uphold them.
    let cb_opt = unsafe { SVMFreeCb::new(pfn_free_func, user_data) }.ok();

    create_and_queue(
        q,
        cmd_type,
        evs,
        event,
        false,
        Box::new(move |q, _| {
            if let Some(cb) = cb_opt {
                cb.call(q, &mut svm_pointers);
            } else {
                for ptr in svm_pointers {
                    svm_free_impl(&q.context, ptr);
                }
            }

            Ok(())
        }),
    )
}

#[cl_entrypoint]
fn enqueue_svm_free(
    command_queue: cl_command_queue,
    num_svm_pointers: cl_uint,
    svm_pointers: *mut *mut c_void,
    pfn_free_func: Option<FuncSVMFreeCb>,
    user_data: *mut c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_free_impl(
        command_queue,
        num_svm_pointers,
        svm_pointers,
        pfn_free_func,
        user_data,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_FREE,
    )
}

#[cl_entrypoint]
fn enqueue_svm_free_arm(
    command_queue: cl_command_queue,
    num_svm_pointers: cl_uint,
    svm_pointers: *mut *mut c_void,
    pfn_free_func: Option<FuncSVMFreeCb>,
    user_data: *mut c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_free_impl(
        command_queue,
        num_svm_pointers,
        svm_pointers,
        pfn_free_func,
        user_data,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_FREE_ARM,
    )
}

fn enqueue_svm_memcpy_impl(
    command_queue: cl_command_queue,
    blocking_copy: cl_bool,
    dst_ptr: *mut c_void,
    src_ptr: *const c_void,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
    cmd_type: cl_command_type,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;
    let block = check_cl_bool(blocking_copy).ok_or(CL_INVALID_VALUE)?;

    // CL_INVALID_OPERATION if the device associated with command queue does not support SVM.
    if !q.device.svm_supported() {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_MEM_COPY_OVERLAP if the values specified for dst_ptr, src_ptr and size result in an
    // overlapping copy.
    let dst_ptr_addr = dst_ptr as usize;
    let src_ptr_addr = src_ptr as usize;
    if (src_ptr_addr <= dst_ptr_addr && dst_ptr_addr < src_ptr_addr + size)
        || (dst_ptr_addr <= src_ptr_addr && src_ptr_addr < dst_ptr_addr + size)
    {
        return Err(CL_MEM_COPY_OVERLAP);
    }

    // CAST: We have no idea about the type or initialization status of these bytes.
    // MaybeUninit<u8> is the safe bet.
    let src_ptr = src_ptr.cast::<MaybeUninit<u8>>();

    // CAST: We have no idea about the type or initialization status of these bytes.
    // MaybeUninit<u8> is the safe bet.
    let dst_ptr = dst_ptr.cast::<MaybeUninit<u8>>();

    // SAFETY: It is up to the application to ensure the memory is valid to read for `size` bytes
    // and that it doesn't modify it until the command has completed.
    let src = unsafe { cl_slice::from_raw_parts(src_ptr, size)? };

    // SAFETY: We've ensured there's no aliasing between src and dst. It is up to the application
    // to ensure the memory is valid to read and write for `size` bytes and that it doesn't modify
    // or read from it until the command has completed.
    let dst = unsafe { cl_slice::from_raw_parts_mut(dst_ptr, size)? };

    create_and_queue(
        q,
        cmd_type,
        evs,
        event,
        block,
        Box::new(move |_, _| {
            dst.copy_from_slice(src);
            Ok(())
        }),
    )
}

#[cl_entrypoint]
fn enqueue_svm_memcpy(
    command_queue: cl_command_queue,
    blocking_copy: cl_bool,
    dst_ptr: *mut c_void,
    src_ptr: *const c_void,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_memcpy_impl(
        command_queue,
        blocking_copy,
        dst_ptr,
        src_ptr,
        size,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_MEMCPY,
    )
}

#[cl_entrypoint]
fn enqueue_svm_memcpy_arm(
    command_queue: cl_command_queue,
    blocking_copy: cl_bool,
    dst_ptr: *mut c_void,
    src_ptr: *const c_void,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_memcpy_impl(
        command_queue,
        blocking_copy,
        dst_ptr,
        src_ptr,
        size,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_MEMCPY_ARM,
    )
}

fn enqueue_svm_mem_fill_impl(
    command_queue: cl_command_queue,
    svm_ptr: *mut ::std::os::raw::c_void,
    pattern: *const ::std::os::raw::c_void,
    pattern_size: usize,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
    cmd_type: cl_command_type,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_OPERATION if the device associated with command queue does not support SVM.
    if !q.device.svm_supported() {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_INVALID_VALUE if pattern is NULL [...]
    if pattern.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE if size is not a multiple of pattern_size.
    if size % pattern_size != 0 {
        return Err(CL_INVALID_VALUE);
    }

    // The provided `$bytesize` must equal `pattern_size`.
    macro_rules! generate_fill_closure {
        ($bytesize:literal) => {{
            // We need the value of `$bytesize`` at compile time, so we need to pass it in, but it
            // should always match `pattern_size`.
            assert!($bytesize == pattern_size);

            // Three reasons we define our own bag-of-bytes type here:
            //
            // We'd otherwise have to pass a type to this macro. Verifying that the type we passed
            // upholds all the properties we need or want is more trouble than defining our own.
            //
            // The primitive Rust types only go up to `u128` anyway and their alignments are
            // platfrom defined. E.g. At the time of this writing `u128` only has an alignment of 8
            // on x86-64, even though its size is 16. Defining our own type with an alignment of 16
            // allows the compiler to generate better code.
            //
            // The alignment of OpenCL types is currently what we need on x86-64, but the spec
            // explicitly states that's just a recommendation and ultimately it's up to the
            // cl_platform.h header. The very descriptive names of the CL types don't make
            // verifying the match calling this macro any easier on a glance.
            // "Was `cl_uint` 4 byte or 8 byte? Eh, I'm sure nobody got it wrong by accident."
            #[repr(C)]
            #[repr(align($bytesize))]
            #[derive(Copy, Clone)]
            struct Pattern([u8; $bytesize]);

            // Just to make sure the compiler didn't generate anything weird.
            static_assert!($bytesize == mem::size_of::<Pattern>());
            static_assert!($bytesize == mem::align_of::<Pattern>());

            // CAST: We don't know exactly which type `pattern` points to, but we know it's an
            // Application Scalar Data Type (cl_char, cl_ulong, etc.) or an Application Vector Data
            // Type (cl_double4, etc.). All of them are `Copy`, do not contain padding bytes, and
            // have no invalid bit patterns. AKA they are POD data types.
            // Since we only copy it around, we can cast to any POD type as long as its size
            // matches `pattern_size`.
            let pattern_ptr = pattern.cast::<Pattern>();

            // The application is allowed to reuse or free the memory referenced by `pattern_ptr`
            // after this function returns, so we need to create a copy.
            //
            // There's no explicit alignment guarantee and we don't rely on `Pattern` matching the
            // alignment of whichever Application Data Type we're actually presented with. Thus, do
            // an unaligned read.
            //
            // SAFETY: We've checked that `pattern_ptr` is not NULL above. It is otherwise the
            // calling application's responsibility to ensure that it is valid for reads of
            // `pattern_size` bytes and properly initialized.
            // Creating a bitwise copy can't create memory safety issues, since `Pattern` is `Copy`.
            let pattern = unsafe { pattern_ptr.read_unaligned() };

            // CAST: Same as with `pattern`, we don't know the exact type of `svm_ptr`, but we do
            // know it's fine if we choose the same type here. The application might reasonably
            // give us uninitialized memory though, so cast to a `MaybeUninit<Pattern>`, which has
            // the same layout as `Pattern`.
            let svm_ptr = svm_ptr.cast::<MaybeUninit<Pattern>>();

            // SAFETY: It is the calling application's responsibility to ensure that `svm_ptr` is
            // valid for reads and writes up to `size` bytes.
            // Since `pattern_size == mem::size_of::<Pattern>()` and `MaybeUninit<Pattern>` has the
            // same layout as `Pattern`, we know that
            // `size / pattern_size * mem::size_of<MaybeUninit<Pattern>>` equals `size`.
            //
            // Since we're creating a `&[MaybeUninit<Pattern>]` the initialization status does not
            // matter.
            //
            // From here on out we only access the referenced memory though this slice. In
            // particular, since we've made a copy of `pattern`, it doesn't matter if the memory
            // region referenced by `pattern` aliases the one referenced by this slice. It is up to
            // the application not to access it at all until this command has been completed.
            let svm_slice = unsafe { cl_slice::from_raw_parts_mut(svm_ptr, size / pattern_size)? };

            Box::new(move |_, _| {
                for x in svm_slice {
                    x.write(pattern);
                }

                Ok(())
            })
        }};
    }

    // Generate optimized code paths for each of the possible pattern sizes.
    let work: EventSig = match pattern_size {
        1 => generate_fill_closure!(1),
        2 => generate_fill_closure!(2),
        4 => generate_fill_closure!(4),
        8 => generate_fill_closure!(8),
        16 => generate_fill_closure!(16),
        32 => generate_fill_closure!(32),
        64 => generate_fill_closure!(64),
        128 => generate_fill_closure!(128),
        _ => {
            // CL_INVALID_VALUE if [...] pattern_size is 0 or if pattern_size is not one of
            // {1, 2, 4, 8, 16, 32, 64, 128}.
            return Err(CL_INVALID_VALUE);
        }
    };

    create_and_queue(q, cmd_type, evs, event, false, work)
}

#[cl_entrypoint]
fn enqueue_svm_mem_fill(
    command_queue: cl_command_queue,
    svm_ptr: *mut ::std::os::raw::c_void,
    pattern: *const ::std::os::raw::c_void,
    pattern_size: usize,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_mem_fill_impl(
        command_queue,
        svm_ptr,
        pattern,
        pattern_size,
        size,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_MEMFILL,
    )
}

#[cl_entrypoint]
fn enqueue_svm_mem_fill_arm(
    command_queue: cl_command_queue,
    svm_ptr: *mut ::std::os::raw::c_void,
    pattern: *const ::std::os::raw::c_void,
    pattern_size: usize,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_mem_fill_impl(
        command_queue,
        svm_ptr,
        pattern,
        pattern_size,
        size,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_MEMFILL_ARM,
    )
}

fn enqueue_svm_map_impl(
    command_queue: cl_command_queue,
    blocking_map: cl_bool,
    flags: cl_map_flags,
    svm_ptr: *mut ::std::os::raw::c_void,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
    cmd_type: cl_command_type,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;
    let block = check_cl_bool(blocking_map).ok_or(CL_INVALID_VALUE)?;

    // CL_INVALID_OPERATION if the device associated with command queue does not support SVM.
    if !q.device.svm_supported() {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_INVALID_VALUE if svm_ptr is NULL.
    if svm_ptr.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE if size is 0 ...
    if size == 0 {
        return Err(CL_INVALID_VALUE);
    }

    // ... or if values specified in map_flags are not valid.
    validate_map_flags_common(flags)?;

    create_and_queue(q, cmd_type, evs, event, block, Box::new(|_, _| Ok(())))
}

#[cl_entrypoint]
fn enqueue_svm_map(
    command_queue: cl_command_queue,
    blocking_map: cl_bool,
    flags: cl_map_flags,
    svm_ptr: *mut ::std::os::raw::c_void,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_map_impl(
        command_queue,
        blocking_map,
        flags,
        svm_ptr,
        size,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_MAP,
    )
}

#[cl_entrypoint]
fn enqueue_svm_map_arm(
    command_queue: cl_command_queue,
    blocking_map: cl_bool,
    flags: cl_map_flags,
    svm_ptr: *mut ::std::os::raw::c_void,
    size: usize,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_map_impl(
        command_queue,
        blocking_map,
        flags,
        svm_ptr,
        size,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_MAP_ARM,
    )
}

fn enqueue_svm_unmap_impl(
    command_queue: cl_command_queue,
    svm_ptr: *mut ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
    cmd_type: cl_command_type,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_OPERATION if the device associated with command queue does not support SVM.
    if !q.device.svm_supported() {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_INVALID_VALUE if svm_ptr is NULL.
    if svm_ptr.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    create_and_queue(q, cmd_type, evs, event, false, Box::new(|_, _| Ok(())))
}

#[cl_entrypoint]
fn enqueue_svm_unmap(
    command_queue: cl_command_queue,
    svm_ptr: *mut ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_unmap_impl(
        command_queue,
        svm_ptr,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_UNMAP,
    )
}

#[cl_entrypoint]
fn enqueue_svm_unmap_arm(
    command_queue: cl_command_queue,
    svm_ptr: *mut ::std::os::raw::c_void,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    enqueue_svm_unmap_impl(
        command_queue,
        svm_ptr,
        num_events_in_wait_list,
        event_wait_list,
        event,
        CL_COMMAND_SVM_UNMAP_ARM,
    )
}

#[cl_entrypoint]
fn enqueue_svm_migrate_mem(
    command_queue: cl_command_queue,
    num_svm_pointers: cl_uint,
    svm_pointers: *mut *const ::std::os::raw::c_void,
    sizes: *const usize,
    flags: cl_mem_migration_flags,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;

    // CL_INVALID_OPERATION if the device associated with command queue does not support SVM.
    if !q.device.svm_supported() {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_INVALID_VALUE if num_svm_pointers is zero
    if num_svm_pointers == 0 {
        return Err(CL_INVALID_VALUE);
    }

    let num_svm_pointers = num_svm_pointers as usize;
    // SAFETY: Just hoping the application is alright.
    let mut svm_pointers =
        unsafe { cl_slice::from_raw_parts(svm_pointers, num_svm_pointers)? }.to_owned();
    // if sizes is NULL, every allocation containing the pointers need to be migrated
    let mut sizes = if sizes.is_null() {
        vec![0; num_svm_pointers]
    } else {
        unsafe { cl_slice::from_raw_parts(sizes, num_svm_pointers)? }.to_owned()
    };

    // CL_INVALID_VALUE if sizes[i] is non-zero range [svm_pointers[i], svm_pointers[i]+sizes[i]) is
    // not contained within an existing clSVMAlloc allocation.
    for (ptr, size) in svm_pointers.iter_mut().zip(&mut sizes) {
        if let Some((alloc, layout)) = q.context.find_svm_alloc(ptr.cast()) {
            let ptr_addr = *ptr as usize;
            let alloc_addr = alloc as usize;

            // if the offset + size is bigger than the allocation we are out of bounds
            if (ptr_addr - alloc_addr) + *size <= layout.size() {
                // if the size is 0, the entire allocation should be migrated
                if *size == 0 {
                    *ptr = alloc.cast();
                    *size = layout.size();
                }
                continue;
            }
        }

        return Err(CL_INVALID_VALUE);
    }

    let to_device = !bit_check(flags, CL_MIGRATE_MEM_OBJECT_HOST);
    let content_undefined = bit_check(flags, CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED);

    create_and_queue(
        q,
        CL_COMMAND_SVM_MIGRATE_MEM,
        evs,
        event,
        false,
        Box::new(move |_, ctx| {
            ctx.svm_migrate(&svm_pointers, &sizes, to_device, content_undefined);
            Ok(())
        }),
    )
}

#[cl_entrypoint]
fn create_pipe(
    _context: cl_context,
    _flags: cl_mem_flags,
    _pipe_packet_size: cl_uint,
    _pipe_max_packets: cl_uint,
    _properties: *const cl_pipe_properties,
) -> CLResult<cl_mem> {
    Err(CL_INVALID_OPERATION)
}

#[cl_info_entrypoint(cl_get_gl_texture_info)]
impl CLInfo<cl_gl_texture_info> for cl_mem {
    fn query(&self, q: cl_gl_texture_info, _: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>> {
        let mem = self.get_ref()?;
        Ok(match *q {
            CL_GL_MIPMAP_LEVEL => cl_prop::<cl_GLint>(0),
            CL_GL_TEXTURE_TARGET => cl_prop::<cl_GLenum>(
                mem.gl_obj
                    .as_ref()
                    .ok_or(CL_INVALID_GL_OBJECT)?
                    .gl_object_target,
            ),
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}

fn create_from_gl(
    context: cl_context,
    flags: cl_mem_flags,
    target: cl_GLenum,
    miplevel: cl_GLint,
    texture: cl_GLuint,
) -> CLResult<cl_mem> {
    let c = context.get_arc()?;
    let gl_ctx_manager = &c.gl_ctx_manager;

    // CL_INVALID_CONTEXT if context associated with command_queue was not created from an OpenGL context
    if gl_ctx_manager.is_none() {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_VALUE if values specified in flags are not valid or if value specified in
    // texture_target is not one of the values specified in the description of texture_target.
    validate_mem_flags(flags, target == GL_ARRAY_BUFFER)?;

    // CL_INVALID_MIP_LEVEL if miplevel is greather than zero and the OpenGL
    // implementation does not support creating from non-zero mipmap levels.
    if miplevel > 0 {
        return Err(CL_INVALID_MIP_LEVEL);
    }

    // CL_INVALID_CONTEXT if context [..] was not created from a GL context.
    if let Some(gl_ctx_manager) = gl_ctx_manager {
        let gl_export_manager =
            gl_ctx_manager.export_object(&c, target, flags as u32, miplevel, texture)?;

        Ok(cl_mem::from_arc(Mem::from_gl(
            c,
            flags,
            &gl_export_manager,
        )?))
    } else {
        Err(CL_INVALID_CONTEXT)
    }
}

#[cl_entrypoint]
fn create_from_gl_texture(
    context: cl_context,
    flags: cl_mem_flags,
    target: cl_GLenum,
    miplevel: cl_GLint,
    texture: cl_GLuint,
) -> CLResult<cl_mem> {
    // CL_INVALID_VALUE if values specified in flags are not valid or if value specified in
    // texture_target is not one of the values specified in the description of texture_target.
    if !is_valid_gl_texture(target) {
        return Err(CL_INVALID_VALUE);
    }

    create_from_gl(context, flags, target, miplevel, texture)
}

#[cl_entrypoint]
fn create_from_gl_texture_2d(
    context: cl_context,
    flags: cl_mem_flags,
    target: cl_GLenum,
    miplevel: cl_GLint,
    texture: cl_GLuint,
) -> CLResult<cl_mem> {
    // CL_INVALID_VALUE if values specified in flags are not valid or if value specified in
    // texture_target is not one of the values specified in the description of texture_target.
    if !is_valid_gl_texture_2d(target) {
        return Err(CL_INVALID_VALUE);
    }

    create_from_gl(context, flags, target, miplevel, texture)
}

#[cl_entrypoint]
fn create_from_gl_texture_3d(
    context: cl_context,
    flags: cl_mem_flags,
    target: cl_GLenum,
    miplevel: cl_GLint,
    texture: cl_GLuint,
) -> CLResult<cl_mem> {
    // CL_INVALID_VALUE if values specified in flags are not valid or if value specified in
    // texture_target is not one of the values specified in the description of texture_target.
    if target != GL_TEXTURE_3D {
        return Err(CL_INVALID_VALUE);
    }

    create_from_gl(context, flags, target, miplevel, texture)
}

#[cl_entrypoint]
fn create_from_gl_buffer(
    context: cl_context,
    flags: cl_mem_flags,
    bufobj: cl_GLuint,
) -> CLResult<cl_mem> {
    create_from_gl(context, flags, GL_ARRAY_BUFFER, 0, bufobj)
}

#[cl_entrypoint]
fn create_from_gl_renderbuffer(
    context: cl_context,
    flags: cl_mem_flags,
    renderbuffer: cl_GLuint,
) -> CLResult<cl_mem> {
    create_from_gl(context, flags, GL_RENDERBUFFER, 0, renderbuffer)
}

#[cl_entrypoint]
fn get_gl_object_info(
    memobj: cl_mem,
    gl_object_type: *mut cl_gl_object_type,
    gl_object_name: *mut cl_GLuint,
) -> CLResult<()> {
    let m = memobj.get_ref()?;

    match &m.gl_obj {
        Some(gl_obj) => {
            gl_object_type.write_checked(gl_obj.gl_object_type);
            gl_object_name.write_checked(gl_obj.gl_object_name);
        }
        None => {
            // CL_INVALID_GL_OBJECT if there is no GL object associated with memobj.
            return Err(CL_INVALID_GL_OBJECT);
        }
    }

    Ok(())
}

#[cl_entrypoint]
fn enqueue_acquire_gl_objects(
    command_queue: cl_command_queue,
    num_objects: cl_uint,
    mem_objects: *const cl_mem,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;
    let objs = cl_mem::get_arc_vec_from_arr(mem_objects, num_objects)?;
    let gl_ctx_manager = &q.context.gl_ctx_manager;

    // CL_INVALID_CONTEXT if context associated with command_queue was not created from an OpenGL context
    if gl_ctx_manager.is_none() {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_GL_OBJECT if memory objects in mem_objects have not been created from a GL object(s).
    if objs.iter().any(|o| o.gl_obj.is_none()) {
        return Err(CL_INVALID_GL_OBJECT);
    }

    create_and_queue(
        q,
        CL_COMMAND_ACQUIRE_GL_OBJECTS,
        evs,
        event,
        false,
        Box::new(move |q, ctx| copy_cube_to_slice(q, ctx, &objs)),
    )
}

#[cl_entrypoint]
fn enqueue_release_gl_objects(
    command_queue: cl_command_queue,
    num_objects: cl_uint,
    mem_objects: *const cl_mem,
    num_events_in_wait_list: cl_uint,
    event_wait_list: *const cl_event,
    event: *mut cl_event,
) -> CLResult<()> {
    let q = command_queue.get_arc()?;
    let evs = event_list_from_cl(&q, num_events_in_wait_list, event_wait_list)?;
    let objs = cl_mem::get_arc_vec_from_arr(mem_objects, num_objects)?;
    let gl_ctx_manager = &q.context.gl_ctx_manager;

    // CL_INVALID_CONTEXT if context associated with command_queue was not created from an OpenGL context
    if gl_ctx_manager.is_none() {
        return Err(CL_INVALID_CONTEXT);
    }

    // CL_INVALID_GL_OBJECT if memory objects in mem_objects have not been created from a GL object(s).
    if objs.iter().any(|o| o.gl_obj.is_none()) {
        return Err(CL_INVALID_GL_OBJECT);
    }

    create_and_queue(
        q,
        CL_COMMAND_RELEASE_GL_OBJECTS,
        evs,
        event,
        false,
        Box::new(move |q, ctx| copy_slice_to_cube(q, ctx, &objs)),
    )
}
