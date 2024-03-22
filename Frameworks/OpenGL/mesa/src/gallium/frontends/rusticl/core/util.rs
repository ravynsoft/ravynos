use crate::api::{icd::CLResult, types::CLVec};

use mesa_rust_gen::*;
use rusticl_opencl_gen::*;

use std::mem;

use super::gl::is_cube_map_face;

pub fn cl_mem_type_to_texture_target(mem_type: cl_mem_object_type) -> pipe_texture_target {
    match mem_type {
        CL_MEM_OBJECT_BUFFER => pipe_texture_target::PIPE_BUFFER,
        CL_MEM_OBJECT_IMAGE1D => pipe_texture_target::PIPE_TEXTURE_1D,
        CL_MEM_OBJECT_IMAGE2D => pipe_texture_target::PIPE_TEXTURE_2D,
        CL_MEM_OBJECT_IMAGE3D => pipe_texture_target::PIPE_TEXTURE_3D,
        CL_MEM_OBJECT_IMAGE1D_ARRAY => pipe_texture_target::PIPE_TEXTURE_1D_ARRAY,
        CL_MEM_OBJECT_IMAGE2D_ARRAY => pipe_texture_target::PIPE_TEXTURE_2D_ARRAY,
        CL_MEM_OBJECT_IMAGE1D_BUFFER => pipe_texture_target::PIPE_BUFFER,
        _ => pipe_texture_target::PIPE_TEXTURE_2D,
    }
}

pub fn cl_mem_type_to_texture_target_gl(
    mem_type: cl_mem_object_type,
    target: cl_GLenum,
) -> pipe_texture_target {
    if is_cube_map_face(target) {
        debug_assert_eq!(mem_type, CL_MEM_OBJECT_IMAGE2D);
        pipe_texture_target::PIPE_TEXTURE_CUBE
    } else {
        cl_mem_type_to_texture_target(mem_type)
    }
}

pub fn create_pipe_box(
    base: CLVec<usize>,
    region: CLVec<usize>,
    mem_type: cl_mem_object_type,
) -> CLResult<pipe_box> {
    let x = base[0].try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
    let mut y = base[1].try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
    let mut z = base[2].try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
    let width = region[0].try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
    let mut height = region[1].try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?;
    let mut depth = region[2].try_into().map_err(|_| CL_OUT_OF_HOST_MEMORY)?;

    if matches!(
        mem_type,
        CL_MEM_OBJECT_BUFFER
            | CL_MEM_OBJECT_IMAGE1D
            | CL_MEM_OBJECT_IMAGE1D_ARRAY
            | CL_MEM_OBJECT_IMAGE1D_BUFFER
            | CL_MEM_OBJECT_IMAGE2D
    ) {
        debug_assert!(depth == 1);
        depth = 1;
    }

    if matches!(
        mem_type,
        CL_MEM_OBJECT_BUFFER | CL_MEM_OBJECT_IMAGE1D | CL_MEM_OBJECT_IMAGE1D_BUFFER
    ) {
        debug_assert!(height == 1);
        height = 1;
    }

    if mem_type == CL_MEM_OBJECT_IMAGE1D_ARRAY {
        mem::swap(&mut height, &mut depth);
        mem::swap(&mut y, &mut z);
    }

    Ok(pipe_box {
        x: x,
        y: y,
        z: z,
        width: width,
        height: height,
        depth: depth,
    })
}
