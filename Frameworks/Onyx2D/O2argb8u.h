#import <Foundation/NSObject.h>
#import <Onyx2D/O2Geometry.h>
#import <Onyx2D/VGmath.h>

typedef struct {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} O2argb8u_BE;

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} O2argb8u_LE;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} O2rgba8u_BE;

#ifdef __LITTLE_ENDIAN__
typedef O2argb8u_LE O2argb8u;
#endif
#ifdef __BIG_ENDIAN__
typedef O2argb8u_BE O2argb8u;
#endif

#define COVERAGE_MULTIPLIER 256

static inline uint32_t inverseCoverage(uint32_t coverage) {
    return COVERAGE_MULTIPLIER - coverage;
}

static inline uint32_t multiplyByCoverage(uint32_t value, uint32_t coverage) {
    return (value * coverage) / COVERAGE_MULTIPLIER;
}

static inline uint32_t O2Image_16u_div_255(uint32_t t) {
    // t/255
    // From Imaging Compositing Fundamentals, Technical Memo 4 by Alvy Ray Smith, Aug 15, 1995
    // Faster and more accurate in that it rounds instead of truncates
    // Perfectly accurate for 8 and 16 bit values
    // (loses a small amount of precision for >16 bits values)
    t = t + 0x80;
    t = ((((t) >> 8) + (t)) >> 8);

    return t;
}

static inline uint8_t O2Image_8u_mul_8u_div_255(uint8_t c, uint8_t a) {
    return O2Image_16u_div_255(c * a);
}

static inline uint16_t O2Image_16u_mul_8u_div_255(uint16_t c, uint8_t a) {
    return O2Image_16u_div_255(c * a);
}

ONYX2D_STATIC_INLINE uint32_t Mul8x2(uint32_t pair, uint32_t alpha) {
    const uint32_t rbmask = 0x00FF00FF;
    pair &= rbmask;
    uint32_t i = alpha * pair + 0x800080;
    return (i + ((i >> 8) & rbmask)) >> 8 & rbmask;
}

static inline O2argb8u O2argb8uMultiplyByMask8u(O2argb8u result, uint32_t value) {
    result.r = O2Image_8u_mul_8u_div_255(result.r, value);
    result.g = O2Image_8u_mul_8u_div_255(result.g, value);
    result.b = O2Image_8u_mul_8u_div_255(result.b, value);
    result.a = O2Image_8u_mul_8u_div_255(result.a, value);

    return result;
}

static inline O2argb8u O2argb8uMultiplyByCoverageNoBypass(O2argb8u result, unsigned value) {
    result.r = multiplyByCoverage(result.r, value);
    result.g = multiplyByCoverage(result.g, value);
    result.b = multiplyByCoverage(result.b, value);
    result.a = multiplyByCoverage(result.a, value);

    return result;
}

static inline O2argb8u O2argb8uMultiplyByCoverage(O2argb8u result, unsigned value) {
    if(value != COVERAGE_MULTIPLIER)
        result = O2argb8uMultiplyByCoverageNoBypass(result, value);

    return result;
}

static inline O2argb8u O2argb8uMultiplyByCoverageAdd(O2argb8u left, uint32_t leftCoverage, O2argb8u right, uint32_t rightCoverage) {
    uint32_t srb = *(uint32_t *)&left;
    uint32_t sag = srb >> 8;

    sag &= 0x00FF00FF;
    srb &= 0x00FF00FF;
    sag = ((sag * leftCoverage) >> 8) & 0x00FF00FF;
    srb = ((srb * leftCoverage) >> 8) & 0x00FF00FF;

    uint32_t drb = *(uint32_t *)&right;
    uint32_t dag = drb >> 8;

    dag &= 0x00FF00FF;
    drb &= 0x00FF00FF;

    dag = ((dag * rightCoverage) >> 8) & 0x00FF00FF;
    drb = ((drb * rightCoverage) >> 8) & 0x00FF00FF;

    uint32_t r;

    sag += dag;
    r = RI_INT_MIN(sag, 0x00FF0000) << 8;
    r |= RI_INT_MIN(sag & 0xFFFF, 255) << 8;
    srb += drb;
    r |= RI_INT_MIN(srb, 0x00FF0000);
    r |= RI_INT_MIN(srb & 0xFFFF, 255);

    return *(O2argb8u *)&r;
}

static inline O2argb8u O2argb8uAdd(O2argb8u result, O2argb8u other) {
    result.r = RI_INT_MIN((unsigned)result.r + (unsigned)other.r, 255);
    result.g = RI_INT_MIN((unsigned)result.g + (unsigned)other.g, 255);
    result.b = RI_INT_MIN((unsigned)result.b + (unsigned)other.b, 255);
    result.a = RI_INT_MIN((unsigned)result.a + (unsigned)other.a, 255);
    return result;
}

void O2ApplyCoverageAndMaskToSpan_largb8u_PRE(O2argb8u *dst, uint32_t icoverage, uint8_t *mask, O2argb8u *src, int length);

void O2ApplyCoverageToSpan_largb8u_PRE(O2argb8u *dst, int coverage, O2argb8u *src, int length);

void O2argb8u_sover_by_coverage(O2argb8u *src, O2argb8u *dst, unsigned coverage, int length);

void O2argb8u_copy_by_coverage(O2argb8u *src, O2argb8u *dst, unsigned coverage, int length);

void O2BlendSpanNormal_8888(O2argb8u *src, O2argb8u *dst, int length);

void O2BlendSpanClear_8888(O2argb8u *src, O2argb8u *dst, int length);

void O2BlendSpanCopy_8888(O2argb8u *src, O2argb8u *dst, int length);

void O2BlendSpanSourceIn_8888(O2argb8u *src, O2argb8u *dst, int length);
void O2BlendSpanXOR_8888(O2argb8u *src, O2argb8u *dst, int length);
void O2BlendSpanPlusLighter_8888(O2argb8u *src, O2argb8u *dst, int length);
