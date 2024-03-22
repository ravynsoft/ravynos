use rusticl_opencl_gen::*;

use std::convert::TryFrom;
use std::os::raw::c_char;

pub const CL1_0_VER: cl_version = mk_cl_version(1, 0, 0);
pub const CL1_1_VER: cl_version = mk_cl_version(1, 1, 0);
pub const CL1_2_VER: cl_version = mk_cl_version(1, 2, 0);
pub const CL2_0_VER: cl_version = mk_cl_version(2, 0, 0);
pub const CL2_1_VER: cl_version = mk_cl_version(2, 1, 0);
pub const CL2_2_VER: cl_version = mk_cl_version(2, 2, 0);
pub const CL3_0_VER: cl_version = mk_cl_version(3, 0, 0);

#[repr(u32)]
#[derive(Copy, Clone, Eq, Hash, PartialEq, PartialOrd, Ord)]
pub enum CLVersion {
    Cl1_0 = CL1_0_VER,
    Cl1_1 = CL1_1_VER,
    Cl1_2 = CL1_2_VER,
    Cl2_0 = CL2_0_VER,
    Cl2_1 = CL2_1_VER,
    Cl2_2 = CL2_2_VER,
    Cl3_0 = CL3_0_VER,
}

pub const fn mk_cl_version_ext(major: u32, minor: u32, patch: u32, ext: &str) -> cl_name_version {
    let mut name: [c_char; 64] = [0; 64];
    let ext = ext.as_bytes();

    let mut i = 0;
    while i < ext.len() {
        name[i] = ext[i] as c_char;
        i += 1;
    }

    cl_name_version {
        version: mk_cl_version(major, minor, patch),
        name,
    }
}

pub const fn mk_cl_version(major: u32, minor: u32, patch: u32) -> u32 {
    ((major & CL_VERSION_MAJOR_MASK) << (CL_VERSION_MINOR_BITS + CL_VERSION_PATCH_BITS))
        | ((minor & CL_VERSION_MINOR_MASK) << CL_VERSION_PATCH_BITS)
        | (patch & CL_VERSION_PATCH_MASK)
}

impl CLVersion {
    pub fn api_str(&self) -> &'static str {
        match self {
            CLVersion::Cl1_0 => "1.0",
            CLVersion::Cl1_1 => "1.1",
            CLVersion::Cl1_2 => "1.2",
            CLVersion::Cl2_0 => "2.0",
            CLVersion::Cl2_1 => "2.1",
            CLVersion::Cl2_2 => "2.2",
            CLVersion::Cl3_0 => "3.0",
        }
    }

    pub fn clc_str(&self) -> &'static str {
        match self {
            CLVersion::Cl1_0 => "100",
            CLVersion::Cl1_1 => "110",
            CLVersion::Cl1_2 => "120",
            CLVersion::Cl2_0 => "200",
            CLVersion::Cl2_1 => "210",
            CLVersion::Cl2_2 => "220",
            CLVersion::Cl3_0 => "300",
        }
    }
}

impl From<CLVersion> for cl_version {
    fn from(cl_version: CLVersion) -> Self {
        cl_version as Self
    }
}

impl TryFrom<cl_version> for CLVersion {
    type Error = cl_int;

    fn try_from(value: cl_version) -> Result<Self, Self::Error> {
        Ok(match value {
            CL1_0_VER => CLVersion::Cl1_0,
            CL1_1_VER => CLVersion::Cl1_1,
            CL1_2_VER => CLVersion::Cl1_2,
            CL2_0_VER => CLVersion::Cl2_0,
            CL2_1_VER => CLVersion::Cl2_1,
            CL2_2_VER => CLVersion::Cl2_2,
            CL3_0_VER => CLVersion::Cl3_0,
            _ => return Err(CL_INVALID_VALUE),
        })
    }
}
