#include <string.h>
#include <math.h>
#include "color_bg.h"

struct csc_vector {
    float x;
    float y;
    float z;
};

struct csc_table {
    struct csc_vector rgb_offset; // RGB offset
    struct csc_vector red_coef;   // RED coefficient
    struct csc_vector green_coef; // GREEN coefficient
    struct csc_vector blue_coef;  // BLUE coefficient
};

static struct csc_table bgcolor_to_rgbfull_table[COLOR_SPACE_MAX] = {
    [COLOR_SPACE_YCBCR601] =
        {
            {0.0f, -0.5f, -0.5f},
            {1.0f, 0.0f, 1.402f},
            {1.0f, -0.344136286f, -0.714136286f},
            {1.0f, 1.772f, 0.0f},
        },
    [COLOR_SPACE_YCBCR709] =
        {
            {0.0f, -0.5f, -0.5f},
            {1.0f, 0.0f, 1.5748f},
            {1.0f, -0.187324273f, -0.468124273f},
            {1.0f, 1.8556f, 0.0f},
        },
    [COLOR_SPACE_YCBCR601_LIMITED] =
        {
            {-0.0625f, -0.5f, -0.5f},
            {1.164383562f, 0.0f, 1.596026786f},
            {1.164383562f, -0.39176229f, -0.812967647f},
            {1.164383562f, 2.017232143f, 0.0f},
        },
    [COLOR_SPACE_YCBCR709_LIMITED] =
        {
            {-0.0625f, -0.5f, -0.5f},
            {1.164383562f, 0.0f, 1.792741071f},
            {1.164383562f, -0.213248614f, -0.532909329f},
            {1.164383562f, 2.112401786f, 0.0f},
        },
    [COLOR_SPACE_2020_YCBCR] =
        {
            {0.0f, -512.f / 1023.f, -512.f / 1023.f},
            {1.0f, 0.0f, 1.4746f},
            {1.0f, -0.164553127f, -0.571353127f},
            {1.0f, 1.8814f, 0.0f},
        },
    [COLOR_SPACE_2020_YCBCR_LIMITED] =
        {
            {-0.0625f, -0.5f, -0.5f},
            {1.167808219f, 0.0f, 1.683611384f},
            {1.167808219f, -0.187877063f, -0.652337331f},
            {1.167808219f, 2.148071652f, 0.0f},
        },
    [COLOR_SPACE_SRGB_LIMITED] =
        {
            {-0.0626221f, -0.0626221f, -0.0626221f},
            {1.167783652f, 0.0f, 0.0f},
            {0.0f, 1.167783652f, 0.0f},
            {0.0f, 0.0, 1.167783652f},
        },
    [COLOR_SPACE_2020_RGB_LIMITEDRANGE] = {
        {-0.0626221f, -0.0626221f, -0.0626221f},
        {1.167783652f, 0.0f, 0.0f},
        {0.0f, 1.167783652f, 0.0f},
        {0.0f, 0.0, 1.167783652f},
    }};

static double clip_double(double x)
{
    if (x < 0.0)
        return 0.0;
    else if (x > 1.0)
        return 1.0;
    else
        return x;
}

static float clip_float(float x)
{
    if (x < 0.0f)
        return 0.0f;
    else if (x > 1.0f)
        return 1.0f;
    else
        return x;
}

static bool bg_csc(struct vpe_color *bg_color, enum color_space cs)
{
    struct csc_table *entry             = &bgcolor_to_rgbfull_table[cs];
    float             csc_final[3]      = {0};
    float             csc_mm[3][4]      = {0};
    bool              output_is_clipped = false;

    memcpy(&csc_mm[0][0], &entry->red_coef, sizeof(struct csc_vector));
    memcpy(&csc_mm[1][0], &entry->green_coef, sizeof(struct csc_vector));
    memcpy(&csc_mm[2][0], &entry->blue_coef, sizeof(struct csc_vector));

    csc_mm[0][3] = entry->rgb_offset.x * csc_mm[0][0] + entry->rgb_offset.y * csc_mm[0][1] +
                   entry->rgb_offset.z * csc_mm[0][2];

    csc_mm[1][3] = entry->rgb_offset.x * csc_mm[1][0] + entry->rgb_offset.y * csc_mm[1][1] +
                   entry->rgb_offset.z * csc_mm[1][2];

    csc_mm[2][3] = entry->rgb_offset.x * csc_mm[2][0] + entry->rgb_offset.y * csc_mm[2][1] +
                   entry->rgb_offset.z * csc_mm[2][2];

    csc_final[0] = csc_mm[0][0] * bg_color->ycbcra.y + csc_mm[0][1] * bg_color->ycbcra.cb +
                   csc_mm[0][2] * bg_color->ycbcra.cr + csc_mm[0][3];

    csc_final[1] = csc_mm[1][0] * bg_color->ycbcra.y + csc_mm[1][1] * bg_color->ycbcra.cb +
                   csc_mm[1][2] * bg_color->ycbcra.cr + csc_mm[1][3];

    csc_final[2] = csc_mm[2][0] * bg_color->ycbcra.y + csc_mm[2][1] * bg_color->ycbcra.cb +
                   csc_mm[2][2] * bg_color->ycbcra.cr + csc_mm[2][3];

    // switch to RGB components
    bg_color->rgba.a = bg_color->ycbcra.a;
    bg_color->rgba.r = clip_float(csc_final[0]);
    bg_color->rgba.g = clip_float(csc_final[1]);
    bg_color->rgba.b = clip_float(csc_final[2]);
    if ((bg_color->rgba.r != csc_final[0]) || (bg_color->rgba.g != csc_final[1]) ||
        (bg_color->rgba.b != csc_final[2])) {
        output_is_clipped = true;
    }
    bg_color->is_ycbcr = false;
    return output_is_clipped;
}

struct gamma_coefs {
    float a0;
    float a1;
    float a2;
    float a3;
    float user_gamma;
    float user_contrast;
    float user_brightness;
};

// srgb, 709, G24
static const int32_t numerator01[] = {31308, 180000, 0};
static const int32_t numerator02[] = {12920, 4500, 0};
static const int32_t numerator03[] = {55, 99, 0};
static const int32_t numerator04[] = {55, 99, 0};
static const int32_t numerator05[] = {2400, 2222, 2400};

static bool build_coefficients(struct gamma_coefs *coefficients, enum color_transfer_func type)
{
    uint32_t index = 0;
    bool     ret   = true;

    if (type == TRANSFER_FUNC_SRGB)
        index = 0;
    else if (type == TRANSFER_FUNC_BT709)
        index = 1;
    else if (type == TRANSFER_FUNC_BT1886)
        index = 2;
    else {
        ret = false;
        goto release;
    }

    coefficients->a0         = (float)numerator01[index] / 10000000.0f;
    coefficients->a1         = (float)numerator02[index] / 1000.0f;
    coefficients->a2         = (float)numerator03[index] / 1000.0f;
    coefficients->a3         = (float)numerator04[index] / 1000.0f;
    coefficients->user_gamma = (float)numerator05[index] / 1000.0f;

release:
    return ret;
}

static double translate_to_linear_space(
    double arg, double a0, double a1, double a2, double a3, double gamma)
{
    double linear;
    double base;

    a0 *= a1;
    if (arg <= -a0) {
        base   = (a2 - arg) / (1.0 + a3);
        linear = -pow(base, gamma);
    } else if ((-a0 <= arg) && (arg <= a0))
        linear = arg / a1;
    else {
        base   = (a2 + arg) / (1.0 + a3);
        linear = pow(base, gamma);
    }

    return linear;
}

// for 709 & sRGB
static void compute_degam(enum color_transfer_func tf, double inY, double *outX, bool clip)
{
    double             ret;
    struct gamma_coefs coefs = {0};

    build_coefficients(&coefs, tf);

    ret = translate_to_linear_space(inY, (double)coefs.a0, (double)coefs.a1, (double)coefs.a2,
        (double)coefs.a3, (double)coefs.user_gamma);

    if (clip) {
        ret = clip_double(ret);
    }
    *outX = ret;
}

static double get_maximum_fp(double a, double b)
{
    if (a > b)
        return a;
    return b;
}

static void compute_depq(double inY, double *outX, bool clip)
{
    double M1 = 0.159301758;
    double M2 = 78.84375;
    double C1 = 0.8359375;
    double C2 = 18.8515625;
    double C3 = 18.6875;

    double nPowM2;
    double base;
    double one      = 1.0;
    double zero     = 0.0;
    bool   negative = false;
    double ret;

    if (inY < zero) {
        inY      = -inY;
        negative = true;
    }
    nPowM2 = pow(inY, one / M2);
    base   = get_maximum_fp(nPowM2 - C1, zero) / (C2 - C3 * nPowM2);
    ret    = pow(base, one / M1);
    if (clip) {
        ret = clip_double(ret);
    }
    if (negative)
        ret = -ret;

    *outX = ret;
}

static bool is_rgb_limited(enum color_space cs)
{
    return (cs == COLOR_SPACE_SRGB_LIMITED || cs == COLOR_SPACE_2020_RGB_LIMITEDRANGE);
}

void vpe_bg_color_convert(
    enum color_space output_cs, struct transfer_func *output_tf, struct vpe_color *bg_color)
{
    enum color_space bgcolor_cs;

    if (bg_color->is_ycbcr) {
        // Need YUV to RGB csc as internal pipe is using RGB full range
        // For range conversion, if output is limited, we assume bg color
        // is limited range too
        switch (output_cs) {
            // output is ycbr cs, follow output's setting
        case COLOR_SPACE_YCBCR601:
        case COLOR_SPACE_YCBCR709:
        case COLOR_SPACE_YCBCR601_LIMITED:
        case COLOR_SPACE_YCBCR709_LIMITED:
        case COLOR_SPACE_2020_YCBCR:
        case COLOR_SPACE_2020_YCBCR_LIMITED:
            bgcolor_cs = output_cs;
            break;
            // output is RGB cs, follow output's range
            // but need yuv to rgb csc
        case COLOR_SPACE_SRGB_LIMITED:
            bgcolor_cs = COLOR_SPACE_YCBCR709_LIMITED;
            break;
        case COLOR_SPACE_2020_RGB_LIMITEDRANGE:
            bgcolor_cs = COLOR_SPACE_2020_YCBCR_LIMITED;
            break;
        case COLOR_SPACE_SRGB:
        case COLOR_SPACE_MSREF_SCRGB:
            bgcolor_cs = COLOR_SPACE_YCBCR709;
            break;
        case COLOR_SPACE_2020_RGB_FULLRANGE:
            bgcolor_cs = COLOR_SPACE_2020_YCBCR;
            break;
        default:
            // should revise the newly added CS
            // and set corresponding bgcolor_cs accordingly
            VPE_ASSERT(0);
            bgcolor_cs = COLOR_SPACE_YCBCR709;
            break;
        }
    } else {
        // RGB BG color, use output's cs for range check
        bgcolor_cs = output_cs;
    }

    // input is [0-0xffff]
    // convert bg color to RGB full range for use inside pipe
    if (bg_color->is_ycbcr || is_rgb_limited(bgcolor_cs))
        bg_csc(bg_color, bgcolor_cs);

    if (output_tf->type == TF_TYPE_DISTRIBUTED_POINTS) {
        double degam_r = 0;
        double degam_g = 0;
        double degam_b = 0;

        // de-gam
        switch (output_tf->tf) {
        case TRANSFER_FUNC_SRGB:
        case TRANSFER_FUNC_BT709:
        case TRANSFER_FUNC_BT1886:
            compute_degam(output_tf->tf, (double)bg_color->rgba.r, &degam_r, true);
            compute_degam(output_tf->tf, (double)bg_color->rgba.g, &degam_g, true);
            compute_degam(output_tf->tf, (double)bg_color->rgba.b, &degam_b, true);
            bg_color->rgba.r = (float)degam_r;
            bg_color->rgba.g = (float)degam_g;
            bg_color->rgba.b = (float)degam_b;
            break;
        case TRANSFER_FUNC_PQ2084:
            compute_depq((double)bg_color->rgba.r, &degam_r, true);
            compute_depq((double)bg_color->rgba.g, &degam_g, true);
            compute_depq((double)bg_color->rgba.b, &degam_b, true);
            bg_color->rgba.r = (float)degam_r;
            bg_color->rgba.g = (float)degam_g;
            bg_color->rgba.b = (float)degam_b;
            break;
        case TRANSFER_FUNC_LINEAR_0_125:
            break;
        default:
            VPE_ASSERT(0);
            break;
        }
    }

    // for TF_TYPE_BYPASS, bg color should be programmed to mpc as linear
}
enum vpe_status vpe_bg_color_outside_cs_gamut(
    const struct vpe_color_space *vcs, struct vpe_color *bg_color)
{
    enum color_space         cs;
    enum color_transfer_func tf;
    struct vpe_color         bg_color_copy = *bg_color;
    vpe_color_get_color_space_and_tf(vcs, &cs, &tf);

    if (is_rgb_limited(cs) || (bg_color->is_ycbcr)) {
        // using the bg_color_copy instead as bg_csc will modify it
        // we should not do modification in checking stage
        // otherwise validate_cached_param() will fail
        if (bg_csc(&bg_color_copy, cs)) {
            return VPE_STATUS_BG_COLOR_OUT_OF_RANGE;
        }
    }
    return VPE_STATUS_OK;
}
