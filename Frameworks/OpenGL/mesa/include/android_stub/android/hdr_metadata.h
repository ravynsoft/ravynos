/*
 * Copyright 2019 The Android Open Source Project
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
 * @file hdr_metadata.h
 */

#ifndef ANDROID_HDR_METADATA_H
#define ANDROID_HDR_METADATA_H

#include <inttypes.h>

#include <sys/cdefs.h>

__BEGIN_DECLS

/**
 * These structures are used to define the display's capabilities for HDR content.
 * They can be used to better tone map content to user's display.
 */

/**
 * HDR metadata standards that are supported by Android.
 */
enum AHdrMetadataType : uint32_t {
    HDR10_SMPTE2086 = 1,
    HDR10_CTA861_3 = 2,
    HDR10PLUS_SEI = 3,
};

/**
 * Color is defined in CIE XYZ coordinates.
 */
struct AColor_xy {
    float x;
    float y;
};

/**
 * SMPTE ST 2086 "Mastering Display Color Volume" static metadata
 */
struct AHdrMetadata_smpte2086 {
    struct AColor_xy displayPrimaryRed;
    struct AColor_xy displayPrimaryGreen;
    struct AColor_xy displayPrimaryBlue;
    struct AColor_xy whitePoint;
    float maxLuminance;
    float minLuminance;
};

/**
 * CTA 861.3 "HDR Static Metadata Extension" static metadata
 */
struct AHdrMetadata_cta861_3 {
    float maxContentLightLevel;
    float maxFrameAverageLightLevel;
};

__END_DECLS

#endif // ANDROID_HDR_METADATA_H
