/*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file data_space.h
 */

#ifndef ANDROID_DATA_SPACE_H
#define ANDROID_DATA_SPACE_H

#include <inttypes.h>

#include <sys/cdefs.h>

__BEGIN_DECLS

/**
 * ADataSpace.
 */
enum ADataSpace {
    /**
     * Default-assumption data space, when not explicitly specified.
     *
     * It is safest to assume the buffer is an image with sRGB primaries and
     * encoding ranges, but the consumer and/or the producer of the data may
     * simply be using defaults. No automatic gamma transform should be
     * expected, except for a possible display gamma transform when drawn to a
     * screen.
     */
    ADATASPACE_UNKNOWN = 0,

    /**
     * scRGB linear encoding:
     *
     * The red, green, and blue components are stored in extended sRGB space,
     * but are linear, not gamma-encoded.
     * The RGB primaries and the white point are the same as BT.709.
     *
     * The values are floating point.
     * A pixel value of 1.0, 1.0, 1.0 corresponds to sRGB white (D65) at 80 nits.
     * Values beyond the range [0.0 - 1.0] would correspond to other colors
     * spaces and/or HDR content.
     */
    ADATASPACE_SCRGB_LINEAR = 406913024, // STANDARD_BT709 | TRANSFER_LINEAR | RANGE_EXTENDED

    /**
     * sRGB gamma encoding:
     *
     * The red, green and blue components are stored in sRGB space, and
     * converted to linear space when read, using the SRGB transfer function
     * for each of the R, G and B components. When written, the inverse
     * transformation is performed.
     *
     * The alpha component, if present, is always stored in linear space and
     * is left unmodified when read or written.
     *
     * Use full range and BT.709 standard.
     */
    ADATASPACE_SRGB = 142671872, // STANDARD_BT709 | TRANSFER_SRGB | RANGE_FULL

    /**
     * scRGB:
     *
     * The red, green, and blue components are stored in extended sRGB space,
     * and gamma-encoded using the SRGB transfer function.
     * The RGB primaries and the white point are the same as BT.709.
     *
     * The values are floating point.
     * A pixel value of 1.0, 1.0, 1.0 corresponds to sRGB white (D65) at 80 nits.
     * Values beyond the range [0.0 - 1.0] would correspond to other colors
     * spaces and/or HDR content.
     */
    ADATASPACE_SCRGB = 411107328, // STANDARD_BT709 | TRANSFER_SRGB | RANGE_EXTENDED

    /**
     * Display P3
     *
     * Use same primaries and white-point as DCI-P3
     * but sRGB transfer function.
     */
    ADATASPACE_DISPLAY_P3 = 143261696, // STANDARD_DCI_P3 | TRANSFER_SRGB | RANGE_FULL

    /**
     * ITU-R Recommendation 2020 (BT.2020)
     *
     * Ultra High-definition television
     *
     * Use full range, SMPTE 2084 (PQ) transfer and BT2020 standard
     */
    ADATASPACE_BT2020_PQ = 163971072, // STANDARD_BT2020 | TRANSFER_ST2084 | RANGE_FULL

    /**
     * Adobe RGB
     *
     * Use full range, gamma 2.2 transfer and Adobe RGB primaries
     * Note: Application is responsible for gamma encoding the data as
     * a 2.2 gamma encoding is not supported in HW.
     */
    ADATASPACE_ADOBE_RGB = 151715840, // STANDARD_ADOBE_RGB | TRANSFER_GAMMA2_2 | RANGE_FULL

    /**
     * ITU-R Recommendation 2020 (BT.2020)
     *
     * Ultra High-definition television
     *
     * Use full range, BT.709 transfer and BT2020 standard
     */
    ADATASPACE_BT2020 = 147193856, // STANDARD_BT2020 | TRANSFER_SMPTE_170M | RANGE_FULL

    /**
     * ITU-R Recommendation 709 (BT.709)
     *
     * High-definition television
     *
     * Use limited range, BT.709 transfer and BT.709 standard.
     */
    ADATASPACE_BT709 = 281083904, // STANDARD_BT709 | TRANSFER_SMPTE_170M | RANGE_LIMITED

    /**
     * SMPTE EG 432-1 and SMPTE RP 431-2.
     *
     * Digital Cinema DCI-P3
     *
     * Use full range, gamma 2.6 transfer and D65 DCI-P3 standard
     * Note: Application is responsible for gamma encoding the data as
     * a 2.6 gamma encoding is not supported in HW.
     */
    ADATASPACE_DCI_P3 = 155844608, // STANDARD_DCI_P3 | TRANSFER_GAMMA2_6 | RANGE_FULL

    /**
     * sRGB linear encoding:
     *
     * The red, green, and blue components are stored in sRGB space, but
     * are linear, not gamma-encoded.
     * The RGB primaries and the white point are the same as BT.709.
     *
     * The values are encoded using the full range ([0,255] for 8-bit) for all
     * components.
     */
    ADATASPACE_SRGB_LINEAR = 138477568, // STANDARD_BT709 | TRANSFER_LINEAR | RANGE_FULL
};

__END_DECLS

#endif // ANDROID_DATA_SPACE_H
