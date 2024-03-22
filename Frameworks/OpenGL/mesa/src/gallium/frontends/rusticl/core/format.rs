use mesa_rust_gen::pipe_format;
use rusticl_opencl_gen::*;

pub struct RusticlImageFormat {
    pub cl_image_format: cl_image_format,
    pub req_for_full_read_or_write: bool,
    pub req_for_embeded_read_or_write: bool,
    pub req_for_full_read_and_write: bool,
    pub req_for_full_cl2: bool,
    pub is_srgb: bool,
    pub pipe: pipe_format,
}

// cl -> pipe mapping:
//
// channels (x: bit size):
// CL_R         => Rx
// CL_A         => Ax
// CL_DEPTH     => Zx
// CL_LUMINANCE => Lx
// CL_INTENSITY => Ix
// CL_RG        => RxGx
// CL_RA        => RxAx
// CL_Rx        => not supported
// CL_RGB       => RxGxBx
// CL_RGx       => not supported
// CL_RGBA      => RxGxBxAx
// CL_ARGB      => AxRxGxBx
// CL_BGRA      => BxGxRxAx
// CL_ABGR      => AxBxGxRx
// CL_RGBx      => RxGxBxXx
// CL_sRGB      => RxGxBx_SRGB
// CL_sRGBA     => RxGxBxAx_SRGB
// CL_sBGRA     => BxGxRxAx_SRGB
// CL_sRGBx     => RxGxBxXx_SRGB
//
// data types:
// SNORM        => x  SNORM
// UNORM        => x  UNORM
// SIGNED_INT   => x  SINT
// UNSIGNED_INT => x  UINT
// HALF_FLOAT   => 16 FLOAT
// FLOAT        => 32 FLOAT
macro_rules! cl_format_table {
    ([$(($order: ident, $type: ident) => $pipe: expr,)+]) => {
        #[allow(non_upper_case_globals)]
        const fn cl_format_to_pipe(
            ch_order: cl_channel_order,
            ch_type: cl_channel_type
        ) -> Option<pipe_format> {
            Some(match (ch_order, ch_type) {
                $(($order, $type) => $pipe,)+
                _ => return None,
            })
        }

        pub const FORMATS: &[RusticlImageFormat] = &[
            $(rusticl_image_format($order, $type),)+
        ];
    };
}

cl_format_table!([
// broken on iris/gen12
//  (CL_A,         CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_A16_FLOAT,
//  (CL_A,         CL_FLOAT)              => pipe_format::PIPE_FORMAT_A32_FLOAT,
//  (CL_A,         CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_A8_SINT,
//  (CL_A,         CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_A16_SINT,
    (CL_A,         CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_A32_SINT,
// broken on iris/gen12
//  (CL_A,         CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_A8_UINT,
//  (CL_A,         CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_A16_UINT,
    (CL_A,         CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_A32_UINT,
    (CL_A,         CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_A8_SNORM,
    (CL_A,         CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_A16_SNORM,
    (CL_A,         CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_A8_UNORM,
    (CL_A,         CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_A16_UNORM,

    (CL_R,         CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_R16_FLOAT,
    (CL_R,         CL_FLOAT)              => pipe_format::PIPE_FORMAT_R32_FLOAT,
    (CL_R,         CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_R8_SINT,
    (CL_R,         CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_R16_SINT,
    (CL_R,         CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_R32_SINT,
    (CL_R,         CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_R8_UINT,
    (CL_R,         CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_R16_UINT,
    (CL_R,         CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_R32_UINT,
    (CL_R,         CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_R8_SNORM,
    (CL_R,         CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_R16_SNORM,
    (CL_R,         CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8_UNORM,
    (CL_R,         CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_R16_UNORM,

    (CL_RA,        CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_R16A16_FLOAT,
    (CL_RA,        CL_FLOAT)              => pipe_format::PIPE_FORMAT_R32A32_FLOAT,
    (CL_RA,        CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_R8A8_SINT,
    (CL_RA,        CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_R16A16_SINT,
    (CL_RA,        CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_R32A32_SINT,
    (CL_RA,        CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_R8A8_UINT,
    (CL_RA,        CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_R16A16_UINT,
    (CL_RA,        CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_R32A32_UINT,
    (CL_RA,        CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_R8A8_SNORM,
    (CL_RA,        CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_R16A16_SNORM,
    (CL_RA,        CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8A8_UNORM,
    (CL_RA,        CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_R16A16_UNORM,

    (CL_RG,        CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_R16G16_FLOAT,
    (CL_RG,        CL_FLOAT)              => pipe_format::PIPE_FORMAT_R32G32_FLOAT,
    (CL_RG,        CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_R8G8_SINT,
    (CL_RG,        CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_R16G16_SINT,
    (CL_RG,        CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_R32G32_SINT,
    (CL_RG,        CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_R8G8_UINT,
    (CL_RG,        CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_R16G16_UINT,
    (CL_RG,        CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_R32G32_UINT,
    (CL_RG,        CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8_SNORM,
    (CL_RG,        CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_R16G16_SNORM,
    (CL_RG,        CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8_UNORM,
    (CL_RG,        CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_R16G16_UNORM,

    (CL_RGB,       CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_R16G16B16_FLOAT,
    (CL_RGB,       CL_FLOAT)              => pipe_format::PIPE_FORMAT_R32G32B32_FLOAT,
    (CL_RGB,       CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_R8G8B8_SINT,
    (CL_RGB,       CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_R16G16B16_SINT,
    (CL_RGB,       CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_R32G32B32_SINT,
    (CL_RGB,       CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_R8G8B8_UINT,
    (CL_RGB,       CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_R16G16B16_UINT,
    (CL_RGB,       CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_R32G32B32_UINT,
    (CL_RGB,       CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8_SNORM,
    (CL_RGB,       CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_R16G16B16_SNORM,
    (CL_RGB,       CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8_UNORM,
    (CL_RGB,       CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_R16G16B16_UNORM,
// broken
//  (CL_RGB,       CL_UNORM_SHORT_565)    => pipe_format::PIPE_FORMAT_R5G6B5_UNORM,

    (CL_ABGR,      CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_A8B8G8R8_SINT,
    (CL_ABGR,      CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_A8B8G8R8_UINT,
    (CL_ABGR,      CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_A8B8G8R8_SNORM,
    (CL_ABGR,      CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_A8B8G8R8_UNORM,

    (CL_ARGB,      CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_A8R8G8B8_SINT,
    (CL_ARGB,      CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_A8R8G8B8_UINT,
    (CL_ARGB,      CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_A8R8G8B8_SNORM,
    (CL_ARGB,      CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_A8R8G8B8_UNORM,

    (CL_BGRA,      CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_B8G8R8A8_SINT,
    (CL_BGRA,      CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_B8G8R8A8_UINT,
    (CL_BGRA,      CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_B8G8R8A8_SNORM,
    (CL_BGRA,      CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_B8G8R8A8_UNORM,

    (CL_RGBA,      CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_R16G16B16A16_FLOAT,
    (CL_RGBA,      CL_FLOAT)              => pipe_format::PIPE_FORMAT_R32G32B32A32_FLOAT,
    (CL_RGBA,      CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_R8G8B8A8_SINT,
    (CL_RGBA,      CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_R16G16B16A16_SINT,
    (CL_RGBA,      CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_R32G32B32A32_SINT,
    (CL_RGBA,      CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_R8G8B8A8_UINT,
    (CL_RGBA,      CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_R16G16B16A16_UINT,
    (CL_RGBA,      CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_R32G32B32A32_UINT,
    (CL_RGBA,      CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8A8_SNORM,
    (CL_RGBA,      CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_R16G16B16A16_SNORM,
    (CL_RGBA,      CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8A8_UNORM,
    (CL_RGBA,      CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_R16G16B16A16_UNORM,
// broken
//  (CL_RGBA,      CL_UNORM_INT_101010_2) => pipe_format::PIPE_FORMAT_R10G10B10A2_UNORM,

// broken
//  (CL_RGBx,      CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_R16G16B16X16_FLOAT,
//  (CL_RGBx,      CL_FLOAT)              => pipe_format::PIPE_FORMAT_R32G32B32X32_FLOAT,
//  (CL_RGBx,      CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_R8G8B8X8_SINT,
//  (CL_RGBx,      CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_R16G16B16X16_SINT,
//  (CL_RGBx,      CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_R32G32B32X32_SINT,
//  (CL_RGBx,      CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_R8G8B8X8_UINT,
//  (CL_RGBx,      CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_R16G16B16X16_UINT,
//  (CL_RGBx,      CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_R32G32B32X32_UINT,
//  (CL_RGBx,      CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8X8_SNORM,
//  (CL_RGBx,      CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_R16G16B16X16_SNORM,
//  (CL_RGBx,      CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8X8_UNORM,
//  (CL_RGBx,      CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_R16G16B16X16_UNORM,
//  (CL_RGBx,      CL_UNORM_SHORT_555)    => pipe_format::PIPE_FORMAT_R5G5B5X1_UNORM,
//  (CL_RGBx,      CL_UNORM_INT_101010)   => pipe_format::PIPE_FORMAT_R10G10B10X2_UNORM,

// broken
//  (CL_sRGB,      CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8_SRGB,
//  (CL_sRGBA,     CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8A8_SRGB,
//  (CL_sBGRA,     CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_B8G8R8A8_SRGB,
// broken
//  (CL_sRGBx,     CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_R8G8B8X8_SRGB,

// broken
//  (CL_DEPTH,     CL_FLOAT)              => pipe_format::PIPE_FORMAT_Z32_FLOAT,
//  (CL_DEPTH,     CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_Z16_UNORM,

    (CL_LUMINANCE, CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_L16_FLOAT,
    (CL_LUMINANCE, CL_FLOAT)              => pipe_format::PIPE_FORMAT_L32_FLOAT,
    (CL_LUMINANCE, CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_L8_SINT,
    (CL_LUMINANCE, CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_L16_SINT,
    (CL_LUMINANCE, CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_L32_SINT,
    (CL_LUMINANCE, CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_L8_UINT,
    (CL_LUMINANCE, CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_L16_UINT,
    (CL_LUMINANCE, CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_L32_UINT,
    (CL_LUMINANCE, CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_L8_SNORM,
    (CL_LUMINANCE, CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_L16_SNORM,
    (CL_LUMINANCE, CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_L8_UNORM,
    (CL_LUMINANCE, CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_L16_UNORM,

    (CL_INTENSITY, CL_HALF_FLOAT)         => pipe_format::PIPE_FORMAT_I16_FLOAT,
    (CL_INTENSITY, CL_FLOAT)              => pipe_format::PIPE_FORMAT_I32_FLOAT,
    (CL_INTENSITY, CL_SIGNED_INT8)        => pipe_format::PIPE_FORMAT_I8_SINT,
    (CL_INTENSITY, CL_SIGNED_INT16)       => pipe_format::PIPE_FORMAT_I16_SINT,
    (CL_INTENSITY, CL_SIGNED_INT32)       => pipe_format::PIPE_FORMAT_I32_SINT,
    (CL_INTENSITY, CL_UNSIGNED_INT8)      => pipe_format::PIPE_FORMAT_I8_UINT,
    (CL_INTENSITY, CL_UNSIGNED_INT16)     => pipe_format::PIPE_FORMAT_I16_UINT,
    (CL_INTENSITY, CL_UNSIGNED_INT32)     => pipe_format::PIPE_FORMAT_I32_UINT,
    (CL_INTENSITY, CL_SNORM_INT8)         => pipe_format::PIPE_FORMAT_I8_SNORM,
    (CL_INTENSITY, CL_SNORM_INT16)        => pipe_format::PIPE_FORMAT_I16_SNORM,
    (CL_INTENSITY, CL_UNORM_INT8)         => pipe_format::PIPE_FORMAT_I8_UNORM,
    (CL_INTENSITY, CL_UNORM_INT16)        => pipe_format::PIPE_FORMAT_I16_UNORM,
]);

#[rustfmt::skip]
const fn req_for_full_r_or_w(
    ch_order: cl_channel_order,
    ch_type: cl_channel_type
) -> bool {
    matches!((ch_order, ch_type),
          (CL_RGBA, CL_UNORM_INT8)
        | (CL_RGBA, CL_UNORM_INT16)
        | (CL_RGBA, CL_SIGNED_INT8)
        | (CL_RGBA, CL_SIGNED_INT16)
        | (CL_RGBA, CL_SIGNED_INT32)
        | (CL_RGBA, CL_UNSIGNED_INT8)
        | (CL_RGBA, CL_UNSIGNED_INT16)
        | (CL_RGBA, CL_UNSIGNED_INT32)
        | (CL_RGBA, CL_HALF_FLOAT)
        | (CL_RGBA, CL_FLOAT)
        | (CL_BGRA, CL_UNORM_INT8))
}

#[rustfmt::skip]
const fn req_for_embedded_r_or_w(
    ch_order: cl_channel_order,
    ch_type: cl_channel_type
) -> bool {
    matches!((ch_order, ch_type),
          (CL_RGBA, CL_UNORM_INT8)
        | (CL_RGBA, CL_UNORM_INT16)
        | (CL_RGBA, CL_SIGNED_INT8)
        | (CL_RGBA, CL_SIGNED_INT16)
        | (CL_RGBA, CL_SIGNED_INT32)
        | (CL_RGBA, CL_UNSIGNED_INT8)
        | (CL_RGBA, CL_UNSIGNED_INT16)
        | (CL_RGBA, CL_UNSIGNED_INT32)
        | (CL_RGBA, CL_HALF_FLOAT)
        | (CL_RGBA, CL_FLOAT))
}

#[rustfmt::skip]
const fn req_for_full_rw(
    ch_order: cl_channel_order,
    ch_type: cl_channel_type
) -> bool {
    matches!((ch_order, ch_type),
          (CL_R,    CL_UNORM_INT8)
        | (CL_R,    CL_SIGNED_INT8)
        | (CL_R,    CL_SIGNED_INT16)
        | (CL_R,    CL_SIGNED_INT32)
        | (CL_R,    CL_UNSIGNED_INT8)
        | (CL_R,    CL_UNSIGNED_INT16)
        | (CL_R,    CL_UNSIGNED_INT32)
        | (CL_R,    CL_HALF_FLOAT)
        | (CL_R,    CL_FLOAT)
        | (CL_RGBA, CL_UNORM_INT8)
        | (CL_RGBA, CL_SIGNED_INT8)
        | (CL_RGBA, CL_SIGNED_INT16)
        | (CL_RGBA, CL_SIGNED_INT32)
        | (CL_RGBA, CL_UNSIGNED_INT8)
        | (CL_RGBA, CL_UNSIGNED_INT16)
        | (CL_RGBA, CL_UNSIGNED_INT32)
        | (CL_RGBA, CL_HALF_FLOAT)
        | (CL_RGBA, CL_FLOAT))
}

#[rustfmt::skip]
#[allow(non_upper_case_globals)]
const fn req_for_full_cl2(
    ch_order: cl_channel_order,
    ch_type: cl_channel_type
) -> bool {
    matches!((ch_order, ch_type),
          (CL_R,     CL_UNORM_INT8)
        | (CL_R,     CL_UNORM_INT16)
        | (CL_R,     CL_SNORM_INT8)
        | (CL_R,     CL_SNORM_INT16)
        | (CL_R,     CL_SIGNED_INT8)
        | (CL_R,     CL_SIGNED_INT16)
        | (CL_R,     CL_SIGNED_INT32)
        | (CL_R,     CL_UNSIGNED_INT8)
        | (CL_R,     CL_UNSIGNED_INT16)
        | (CL_R,     CL_UNSIGNED_INT32)
        | (CL_R,     CL_HALF_FLOAT)
        | (CL_R,     CL_FLOAT)
        | (CL_DEPTH, CL_UNORM_INT16)
        | (CL_DEPTH, CL_FLOAT)
        | (CL_RG,    CL_UNORM_INT8)
        | (CL_RG,    CL_UNORM_INT16)
        | (CL_RG,    CL_SNORM_INT8)
        | (CL_RG,    CL_SNORM_INT16)
        | (CL_RG,    CL_SIGNED_INT8)
        | (CL_RG,    CL_SIGNED_INT16)
        | (CL_RG,    CL_SIGNED_INT32)
        | (CL_RG,    CL_UNSIGNED_INT8)
        | (CL_RG,    CL_UNSIGNED_INT16)
        | (CL_RG,    CL_UNSIGNED_INT32)
        | (CL_RG,    CL_HALF_FLOAT)
        | (CL_RG,    CL_FLOAT)
        | (CL_RGBA,  CL_UNORM_INT8)
        | (CL_RGBA,  CL_UNORM_INT16)
        | (CL_RGBA,  CL_SNORM_INT8)
        | (CL_RGBA,  CL_SNORM_INT16)
        | (CL_RGBA,  CL_SIGNED_INT8)
        | (CL_RGBA,  CL_SIGNED_INT16)
        | (CL_RGBA,  CL_SIGNED_INT32)
        | (CL_RGBA,  CL_UNSIGNED_INT8)
        | (CL_RGBA,  CL_UNSIGNED_INT16)
        | (CL_RGBA,  CL_UNSIGNED_INT32)
        | (CL_RGBA,  CL_HALF_FLOAT)
        | (CL_RGBA,  CL_FLOAT)
        | (CL_BGRA,  CL_UNORM_INT8)
        | (CL_sRGBA, CL_UNORM_INT8))
}

#[allow(non_upper_case_globals)]
const fn is_srgb(ch_order: cl_channel_order) -> bool {
    matches!(ch_order, CL_sBGRA | CL_sRGB | CL_sRGBA | CL_sRGBx)
}

const fn rusticl_image_format(
    ch_order: cl_channel_order,
    ch_type: cl_channel_type,
) -> RusticlImageFormat {
    let pipe = match cl_format_to_pipe(ch_order, ch_type) {
        Some(pipe) => pipe,
        None => panic!("unknown CL format!"),
    };

    RusticlImageFormat {
        cl_image_format: cl_image_format {
            image_channel_order: ch_order,
            image_channel_data_type: ch_type,
        },
        req_for_full_read_or_write: req_for_full_r_or_w(ch_order, ch_type),
        req_for_embeded_read_or_write: req_for_embedded_r_or_w(ch_order, ch_type),
        req_for_full_read_and_write: req_for_full_rw(ch_order, ch_type),
        req_for_full_cl2: req_for_full_cl2(ch_order, ch_type),
        is_srgb: is_srgb(ch_order),
        pipe: pipe,
    }
}

pub trait CLFormatInfo {
    fn channels(&self) -> Option<u8>;
    fn format_info(&self) -> Option<(u8, bool)>;
    fn to_pipe_format(&self) -> Option<pipe_format>;

    fn channel_size(&self) -> Option<u8> {
        if let Some(packed) = self.is_packed() {
            assert!(!packed);
            self.format_info().map(|i| i.0)
        } else {
            None
        }
    }

    fn packed_size(&self) -> Option<u8> {
        if let Some(packed) = self.is_packed() {
            assert!(packed);
            self.format_info().map(|i| i.0)
        } else {
            None
        }
    }

    fn is_packed(&self) -> Option<bool> {
        self.format_info().map(|i| i.1)
    }

    fn pixel_size(&self) -> Option<u8> {
        if let Some(packed) = self.is_packed() {
            if packed {
                self.packed_size()
            } else {
                self.channels().zip(self.channel_size()).map(|(c, s)| c * s)
            }
        } else {
            None
        }
    }
}

impl CLFormatInfo for cl_image_format {
    #[allow(non_upper_case_globals)]
    fn channels(&self) -> Option<u8> {
        match self.image_channel_order {
            CL_R | CL_A | CL_DEPTH | CL_INTENSITY | CL_LUMINANCE | CL_NONE => Some(1),
            CL_RG | CL_RA | CL_Rx => Some(2),
            CL_RGB | CL_RGx | CL_sRGB => Some(3),
            CL_RGBA | CL_ARGB | CL_BGRA | CL_ABGR | CL_RGBx | CL_sRGBA | CL_sBGRA | CL_sRGBx => {
                Some(4)
            }
            _ => None,
        }
    }

    fn format_info(&self) -> Option<(u8, bool)> {
        match self.image_channel_data_type {
            CL_SIGNED_INT8 | CL_UNSIGNED_INT8 | CL_SNORM_INT8 | CL_UNORM_INT8 | CL_NONE => {
                Some((1, false))
            }
            CL_SIGNED_INT16 | CL_UNSIGNED_INT16 | CL_SNORM_INT16 | CL_UNORM_INT16
            | CL_HALF_FLOAT => Some((2, false)),
            CL_SIGNED_INT32 | CL_UNSIGNED_INT32 | CL_FLOAT => Some((4, false)),
            CL_UNORM_SHORT_555 | CL_UNORM_SHORT_565 => Some((2, true)),
            CL_UNORM_INT_101010 | CL_UNORM_INT_101010_2 => Some((4, true)),
            _ => None,
        }
    }

    fn to_pipe_format(&self) -> Option<pipe_format> {
        cl_format_to_pipe(self.image_channel_order, self.image_channel_data_type)
    }
}

macro_rules! gl_cl_format_table {
    ([$($gl: ident => ($order: expr, $dtype: expr),)+]) => {
        #[allow(non_upper_case_globals)]
        const fn gl_format_to_cl(
            gl_format: cl_GLenum
        ) -> Option<(cl_channel_order, cl_channel_type)> {
            Some(match gl_format {
                $($gl => ($order, $dtype),)+
                _ => return None,
            })
        }
    };
}

gl_cl_format_table!([
    GL_RGBA8                        => (CL_RGBA, CL_UNORM_INT8),
    GL_SRGB8_ALPHA8                 => (CL_sRGBA, CL_UNORM_INT8),
    GL_RGBA                         => (CL_RGBA, CL_UNORM_INT8),
    GL_UNSIGNED_INT_8_8_8_8_REV     => (CL_RGBA, CL_UNORM_INT8),
    GL_BGRA                         => (CL_BGRA, CL_UNORM_INT8),
    GL_RGBA8I                       => (CL_RGBA, CL_SIGNED_INT8),
    GL_RGBA16I                      => (CL_RGBA, CL_SIGNED_INT16),
    GL_RGBA32I                      => (CL_RGBA, CL_SIGNED_INT32),
    GL_RGBA8UI                      => (CL_RGBA, CL_UNSIGNED_INT8),
    GL_RGBA16UI                     => (CL_RGBA, CL_UNSIGNED_INT16),
    GL_RGBA32UI                     => (CL_RGBA, CL_UNSIGNED_INT32),
    GL_RGBA8_SNORM                  => (CL_RGBA, CL_SNORM_INT8),
    GL_RGBA16                       => (CL_RGBA, CL_UNORM_INT16),
    GL_RGBA16_SNORM                 => (CL_RGBA, CL_SNORM_INT16),
    GL_RGBA16F                      => (CL_RGBA, CL_HALF_FLOAT),
    GL_RGBA32F                      => (CL_RGBA, CL_FLOAT),

    GL_R8                           => (CL_R, CL_UNORM_INT8),
    GL_R8_SNORM                     => (CL_R, CL_SNORM_INT8),
    GL_R16                          => (CL_R, CL_UNORM_INT16),
    GL_R16_SNORM                    => (CL_R, CL_SNORM_INT16),
    GL_R16F                         => (CL_R, CL_HALF_FLOAT),
    GL_R32F                         => (CL_R, CL_FLOAT),
    GL_R8I                          => (CL_R, CL_SIGNED_INT8),
    GL_R16I                         => (CL_R, CL_SIGNED_INT16),
    GL_R32I                         => (CL_R, CL_SIGNED_INT32),
    GL_R8UI                         => (CL_R, CL_UNSIGNED_INT8),
    GL_R16UI                        => (CL_R, CL_UNSIGNED_INT16),
    GL_R32UI                        => (CL_R, CL_UNSIGNED_INT32),

    GL_RG8                          => (CL_RG, CL_UNORM_INT8),
    GL_RG8_SNORM                    => (CL_RG, CL_SNORM_INT8),
    GL_RG16                         => (CL_RG, CL_UNORM_INT16),
    GL_RG16_SNORM                   => (CL_RG, CL_SNORM_INT16),
    GL_RG16F                        => (CL_RG, CL_HALF_FLOAT),
    GL_RG32F                        => (CL_RG, CL_FLOAT),
    GL_RG8I                         => (CL_RG, CL_SIGNED_INT8),
    GL_RG16I                        => (CL_RG, CL_SIGNED_INT16),
    GL_RG32I                        => (CL_RG, CL_SIGNED_INT32),
    GL_RG8UI                        => (CL_RG, CL_UNSIGNED_INT8),
    GL_RG16UI                       => (CL_RG, CL_UNSIGNED_INT16),
    GL_RG32UI                       => (CL_RG, CL_UNSIGNED_INT32),
]);

pub fn format_from_gl(internal_format: cl_GLenum) -> Option<cl_image_format> {
    gl_format_to_cl(internal_format).map(|(order, dtype)| cl_image_format {
        image_channel_order: order,
        image_channel_data_type: dtype,
    })
}
