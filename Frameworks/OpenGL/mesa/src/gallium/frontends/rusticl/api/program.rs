use crate::api::icd::*;
use crate::api::types::*;
use crate::api::util::*;
use crate::core::device::*;
use crate::core::platform::*;
use crate::core::program::*;

use mesa_rust::compiler::clc::*;
use mesa_rust_util::string::*;
use rusticl_opencl_gen::*;
use rusticl_proc_macros::cl_entrypoint;
use rusticl_proc_macros::cl_info_entrypoint;

use std::ffi::CStr;
use std::ffi::CString;
use std::iter;
use std::mem::MaybeUninit;
use std::num::NonZeroUsize;
use std::os::raw::c_char;
use std::ptr;
use std::slice;
use std::sync::Arc;

#[cl_info_entrypoint(cl_get_program_info)]
impl CLInfo<cl_program_info> for cl_program {
    fn query(&self, q: cl_program_info, vals: &[u8]) -> CLResult<Vec<MaybeUninit<u8>>> {
        let prog = self.get_ref()?;
        Ok(match q {
            CL_PROGRAM_BINARIES => cl_prop::<Vec<*mut u8>>(prog.binaries(vals)),
            CL_PROGRAM_BINARY_SIZES => cl_prop::<Vec<usize>>(prog.bin_sizes()),
            CL_PROGRAM_CONTEXT => {
                // Note we use as_ptr here which doesn't increase the reference count.
                let ptr = Arc::as_ptr(&prog.context);
                cl_prop::<cl_context>(cl_context::from_ptr(ptr))
            }
            CL_PROGRAM_DEVICES => cl_prop::<Vec<cl_device_id>>(
                prog.devs
                    .iter()
                    .map(|&d| cl_device_id::from_ptr(d))
                    .collect(),
            ),
            CL_PROGRAM_IL => match &prog.src {
                ProgramSourceType::Il(il) => to_maybeuninit_vec(il.to_bin().to_vec()),
                _ => Vec::new(),
            },
            CL_PROGRAM_KERNEL_NAMES => cl_prop::<&str>(&*prog.kernels().join(";")),
            CL_PROGRAM_NUM_DEVICES => cl_prop::<cl_uint>(prog.devs.len() as cl_uint),
            CL_PROGRAM_NUM_KERNELS => cl_prop::<usize>(prog.kernels().len()),
            CL_PROGRAM_REFERENCE_COUNT => cl_prop::<cl_uint>(self.refcnt()?),
            CL_PROGRAM_SCOPE_GLOBAL_CTORS_PRESENT => cl_prop::<cl_bool>(CL_FALSE),
            CL_PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT => cl_prop::<cl_bool>(CL_FALSE),
            CL_PROGRAM_SOURCE => match &prog.src {
                ProgramSourceType::Src(src) => cl_prop::<&CStr>(src.as_c_str()),
                _ => Vec::new(),
            },
            // CL_INVALID_VALUE if param_name is not one of the supported values
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}

#[cl_info_entrypoint(cl_get_program_build_info)]
impl CLInfoObj<cl_program_build_info, cl_device_id> for cl_program {
    fn query(&self, d: cl_device_id, q: cl_program_build_info) -> CLResult<Vec<MaybeUninit<u8>>> {
        let prog = self.get_ref()?;
        let dev = d.get_arc()?;
        Ok(match q {
            CL_PROGRAM_BINARY_TYPE => cl_prop::<cl_program_binary_type>(prog.bin_type(&dev)),
            CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE => cl_prop::<usize>(0),
            CL_PROGRAM_BUILD_LOG => cl_prop::<&str>(&prog.log(&dev)),
            CL_PROGRAM_BUILD_OPTIONS => cl_prop::<&str>(&prog.options(&dev)),
            CL_PROGRAM_BUILD_STATUS => cl_prop::<cl_build_status>(prog.status(&dev)),
            // CL_INVALID_VALUE if param_name is not one of the supported values
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}

fn validate_devices<'a>(
    device_list: *const cl_device_id,
    num_devices: cl_uint,
    default: &[&'a Device],
) -> CLResult<Vec<&'a Device>> {
    let mut devs = cl_device_id::get_ref_vec_from_arr(device_list, num_devices)?;

    // If device_list is a NULL value, the compile is performed for all devices associated with
    // program.
    if devs.is_empty() {
        devs = default.to_vec();
    }

    Ok(devs)
}

#[cl_entrypoint]
fn create_program_with_source(
    context: cl_context,
    count: cl_uint,
    strings: *mut *const c_char,
    lengths: *const usize,
) -> CLResult<cl_program> {
    let c = context.get_arc()?;

    // CL_INVALID_VALUE if count is zero or if strings ...
    if count == 0 || strings.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // ... or any entry in strings is NULL.
    let srcs = unsafe { slice::from_raw_parts(strings, count as usize) };
    if srcs.contains(&ptr::null()) {
        return Err(CL_INVALID_VALUE);
    }

    // "lengths argument is an array with the number of chars in each string
    // (the string length). If an element in lengths is zero, its accompanying
    // string is null-terminated. If lengths is NULL, all strings in the
    // strings argument are considered null-terminated."

    // A length of zero represents "no length given", so semantically we're
    // dealing not with a slice of usize but actually with a slice of
    // Option<NonZeroUsize>. Handily those two are layout compatible, so simply
    // reinterpret the data.
    //
    // Take either an iterator over the given slice or - if the `lengths`
    // pointer is NULL - an iterator that always returns None (infinite, but
    // later bounded by being zipped with the finite `srcs`).
    //
    // Looping over different iterators is no problem as long as they return
    // the same item type. However, since we can only decide which to use at
    // runtime, we need to use dynamic dispatch. The compiler also needs to
    // know how much space to reserve on the stack, but different
    // implementations of the `Iterator` trait will need different amounts of
    // memory. This is resolved by putting the actual iterator on the heap
    // with `Box` and only a reference to it on the stack.
    let lengths: Box<dyn Iterator<Item = _>> = if lengths.is_null() {
        Box::new(iter::repeat(&None))
    } else {
        // SAFETY: Option<NonZeroUsize> is guaranteed to be layout compatible
        // with usize. The zero niche represents None.
        let lengths = lengths as *const Option<NonZeroUsize>;
        Box::new(unsafe { slice::from_raw_parts(lengths, count as usize) }.iter())
    };

    // We don't want encoding or any other problems with the source to prevent
    // compilation, so don't convert this to a Rust `String`.
    let mut source = Vec::new();
    for (&string_ptr, len_opt) in iter::zip(srcs, lengths) {
        let arr = match len_opt {
            Some(len) => {
                // The spec doesn't say how nul bytes should be handled here or
                // if they are legal at all. Assume they truncate the string.
                let arr = unsafe { slice::from_raw_parts(string_ptr.cast(), len.get()) };
                // TODO: simplify this a bit with from_bytes_until_nul once
                // that's stabilized and available in our msrv
                arr.iter()
                    .position(|&x| x == 0)
                    .map_or(arr, |nul_index| &arr[..nul_index])
            }
            None => unsafe { CStr::from_ptr(string_ptr) }.to_bytes(),
        };

        source.extend_from_slice(arr);
    }

    Ok(cl_program::from_arc(Program::new(
        &c,
        &c.devs,
        // SAFETY: We've constructed `source` such that it contains no nul bytes.
        unsafe { CString::from_vec_unchecked(source) },
    )))
}

#[cl_entrypoint]
fn create_program_with_binary(
    context: cl_context,
    num_devices: cl_uint,
    device_list: *const cl_device_id,
    lengths: *const usize,
    binaries: *mut *const ::std::os::raw::c_uchar,
    binary_status: *mut cl_int,
) -> CLResult<cl_program> {
    let c = context.get_arc()?;
    let devs = cl_device_id::get_ref_vec_from_arr(device_list, num_devices)?;

    // CL_INVALID_VALUE if device_list is NULL or num_devices is zero.
    if devs.is_empty() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_VALUE if lengths or binaries is NULL
    if lengths.is_null() || binaries.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_DEVICE if any device in device_list is not in the list of devices associated with
    // context.
    if !devs.iter().all(|d| c.devs.contains(d)) {
        return Err(CL_INVALID_DEVICE);
    }

    let lengths = unsafe { slice::from_raw_parts(lengths, num_devices as usize) };
    let binaries = unsafe { slice::from_raw_parts(binaries, num_devices as usize) };

    // now device specific stuff
    let mut err = 0;
    let mut bins: Vec<&[u8]> = vec![&[]; num_devices as usize];
    for i in 0..num_devices as usize {
        let mut dev_err = 0;

        // CL_INVALID_VALUE if lengths[i] is zero or if binaries[i] is a NULL value
        if lengths[i] == 0 || binaries[i].is_null() {
            dev_err = CL_INVALID_VALUE;
        }

        if !binary_status.is_null() {
            unsafe { binary_status.add(i).write(dev_err) };
        }

        // just return the last one
        err = dev_err;
        bins[i] = unsafe { slice::from_raw_parts(binaries[i], lengths[i]) };
    }

    if err != 0 {
        return Err(err);
    }

    let prog = Program::from_bins(c, devs, &bins);

    Ok(cl_program::from_arc(prog))
    //• CL_INVALID_BINARY if an invalid program binary was encountered for any device. binary_status will return specific status for each device.
}

#[cl_entrypoint]
fn create_program_with_il(
    context: cl_context,
    il: *const ::std::os::raw::c_void,
    length: usize,
) -> CLResult<cl_program> {
    let c = context.get_arc()?;

    // CL_INVALID_VALUE if il is NULL or if length is zero.
    if il.is_null() || length == 0 {
        return Err(CL_INVALID_VALUE);
    }

    // SAFETY: according to API spec
    let spirv = unsafe { slice::from_raw_parts(il.cast(), length) };
    Ok(cl_program::from_arc(Program::from_spirv(c, spirv)))
}

#[cl_entrypoint]
fn retain_program(program: cl_program) -> CLResult<()> {
    program.retain()
}

#[cl_entrypoint]
fn release_program(program: cl_program) -> CLResult<()> {
    program.release()
}

#[cl_entrypoint]
fn build_program(
    program: cl_program,
    num_devices: cl_uint,
    device_list: *const cl_device_id,
    options: *const c_char,
    pfn_notify: Option<FuncProgramCB>,
    user_data: *mut ::std::os::raw::c_void,
) -> CLResult<()> {
    let mut res = true;
    let p = program.get_ref()?;
    let devs = validate_devices(device_list, num_devices, &p.devs)?;

    // SAFETY: The requirements on `ProgramCB::try_new` match the requirements
    // imposed by the OpenCL specification. It is the caller's duty to uphold them.
    let cb_opt = unsafe { ProgramCB::try_new(pfn_notify, user_data)? };

    // CL_INVALID_OPERATION if there are kernel objects attached to program.
    if p.active_kernels() {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_BUILD_PROGRAM_FAILURE if there is a failure to build the program executable. This error
    // will be returned if clBuildProgram does not return until the build has completed.
    for dev in &devs {
        res &= p.build(dev, c_string_to_string(options));
    }

    if let Some(cb) = cb_opt {
        cb.call(p);
    }

    //• CL_INVALID_BINARY if program is created with clCreateProgramWithBinary and devices listed in device_list do not have a valid program binary loaded.
    //• CL_INVALID_BUILD_OPTIONS if the build options specified by options are invalid.
    //• CL_INVALID_OPERATION if the build of a program executable for any of the devices listed in device_list by a previous call to clBuildProgram for program has not completed.
    //• CL_INVALID_OPERATION if program was not created with clCreateProgramWithSource, clCreateProgramWithIL or clCreateProgramWithBinary.

    if res {
        Ok(())
    } else {
        if Platform::dbg().program {
            for dev in &devs {
                eprintln!("{}", p.log(dev));
            }
        }
        Err(CL_BUILD_PROGRAM_FAILURE)
    }
}

#[cl_entrypoint]
fn compile_program(
    program: cl_program,
    num_devices: cl_uint,
    device_list: *const cl_device_id,
    options: *const c_char,
    num_input_headers: cl_uint,
    input_headers: *const cl_program,
    header_include_names: *mut *const c_char,
    pfn_notify: Option<FuncProgramCB>,
    user_data: *mut ::std::os::raw::c_void,
) -> CLResult<()> {
    let mut res = true;
    let p = program.get_ref()?;
    let devs = validate_devices(device_list, num_devices, &p.devs)?;

    // SAFETY: The requirements on `ProgramCB::try_new` match the requirements
    // imposed by the OpenCL specification. It is the caller's duty to uphold them.
    let cb_opt = unsafe { ProgramCB::try_new(pfn_notify, user_data)? };

    // CL_INVALID_VALUE if num_input_headers is zero and header_include_names or input_headers are
    // not NULL or if num_input_headers is not zero and header_include_names or input_headers are
    // NULL.
    if num_input_headers == 0 && (!header_include_names.is_null() || !input_headers.is_null())
        || num_input_headers != 0 && (header_include_names.is_null() || input_headers.is_null())
    {
        return Err(CL_INVALID_VALUE);
    }

    let mut headers = Vec::new();

    // If program was created using clCreateProgramWithIL, then num_input_headers, input_headers,
    // and header_include_names are ignored.
    if !p.is_il() {
        for h in 0..num_input_headers as usize {
            // SAFETY: have to trust the application here
            let header = unsafe { (*input_headers.add(h)).get_ref()? };
            match &header.src {
                ProgramSourceType::Src(src) => headers.push(spirv::CLCHeader {
                    // SAFETY: have to trust the application here
                    name: unsafe { CStr::from_ptr(*header_include_names.add(h)).to_owned() },
                    source: src,
                }),
                _ => return Err(CL_INVALID_OPERATION),
            }
        }
    }

    // CL_INVALID_OPERATION if program has no source or IL available, i.e. it has not been created
    // with clCreateProgramWithSource or clCreateProgramWithIL.
    if !(p.is_src() || p.is_il()) {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_INVALID_OPERATION if there are kernel objects attached to program.
    if p.active_kernels() {
        return Err(CL_INVALID_OPERATION);
    }

    // CL_COMPILE_PROGRAM_FAILURE if there is a failure to compile the program source. This error
    // will be returned if clCompileProgram does not return until the compile has completed.
    for dev in &devs {
        res &= p.compile(dev, c_string_to_string(options), &headers);
    }

    if let Some(cb) = cb_opt {
        cb.call(p);
    }

    // • CL_INVALID_COMPILER_OPTIONS if the compiler options specified by options are invalid.
    // • CL_INVALID_OPERATION if the compilation or build of a program executable for any of the devices listed in device_list by a previous call to clCompileProgram or clBuildProgram for program has not completed.

    if res {
        Ok(())
    } else {
        if Platform::dbg().program {
            for dev in &devs {
                eprintln!("{}", p.log(dev));
            }
        }
        Err(CL_COMPILE_PROGRAM_FAILURE)
    }
}

pub fn link_program(
    context: cl_context,
    num_devices: cl_uint,
    device_list: *const cl_device_id,
    options: *const ::std::os::raw::c_char,
    num_input_programs: cl_uint,
    input_programs: *const cl_program,
    pfn_notify: Option<FuncProgramCB>,
    user_data: *mut ::std::os::raw::c_void,
) -> CLResult<(cl_program, cl_int)> {
    let c = context.get_arc()?;
    let devs = validate_devices(device_list, num_devices, &c.devs)?;
    let progs = cl_program::get_arc_vec_from_arr(input_programs, num_input_programs)?;

    // SAFETY: The requirements on `ProgramCB::try_new` match the requirements
    // imposed by the OpenCL specification. It is the caller's duty to uphold them.
    let cb_opt = unsafe { ProgramCB::try_new(pfn_notify, user_data)? };

    // CL_INVALID_VALUE if num_input_programs is zero and input_programs is NULL
    if progs.is_empty() {
        return Err(CL_INVALID_VALUE);
    }

    // CL_INVALID_DEVICE if any device in device_list is not in the list of devices associated with
    // context.
    if !devs.iter().all(|d| c.devs.contains(d)) {
        return Err(CL_INVALID_DEVICE);
    }

    // CL_INVALID_OPERATION if the compilation or build of a program executable for any of the
    // devices listed in device_list by a previous call to clCompileProgram or clBuildProgram for
    // program has not completed.
    for d in &devs {
        if progs
            .iter()
            .map(|p| p.status(d))
            .any(|s| s != CL_BUILD_SUCCESS as cl_build_status)
        {
            return Err(CL_INVALID_OPERATION);
        }
    }

    // CL_LINK_PROGRAM_FAILURE if there is a failure to link the compiled binaries and/or libraries.
    let res = Program::link(c, &devs, &progs, c_string_to_string(options));
    let code = if devs
        .iter()
        .map(|d| res.status(d))
        .all(|s| s == CL_BUILD_SUCCESS as cl_build_status)
    {
        CL_SUCCESS as cl_int
    } else {
        CL_LINK_PROGRAM_FAILURE
    };

    if let Some(cb) = cb_opt {
        cb.call(&res);
    }

    Ok((cl_program::from_arc(res), code))

    //• CL_INVALID_LINKER_OPTIONS if the linker options specified by options are invalid.
    //• CL_INVALID_OPERATION if the rules for devices containing compiled binaries or libraries as described in input_programs argument above are not followed.
}

#[cl_entrypoint]
fn set_program_specialization_constant(
    program: cl_program,
    spec_id: cl_uint,
    spec_size: usize,
    spec_value: *const ::std::os::raw::c_void,
) -> CLResult<()> {
    let program = program.get_ref()?;

    // CL_INVALID_PROGRAM if program is not a valid program object created from an intermediate
    // language (e.g. SPIR-V)
    // TODO: or if the intermediate language does not support specialization constants.
    if !program.is_il() {
        return Err(CL_INVALID_PROGRAM);
    }

    if spec_size != program.get_spec_constant_size(spec_id).into() {
        // CL_INVALID_VALUE if spec_size does not match the size of the specialization constant in
        // the module,
        return Err(CL_INVALID_VALUE);
    }

    // or if spec_value is NULL.
    if spec_value.is_null() {
        return Err(CL_INVALID_VALUE);
    }

    // SAFETY: according to API spec
    program.set_spec_constant(spec_id, unsafe {
        slice::from_raw_parts(spec_value.cast(), spec_size)
    });

    Ok(())
}

#[cl_entrypoint]
fn set_program_release_callback(
    _program: cl_program,
    _pfn_notify: ::std::option::Option<FuncProgramCB>,
    _user_data: *mut ::std::os::raw::c_void,
) -> CLResult<()> {
    Err(CL_INVALID_OPERATION)
}
