/*
************************************************************************************************************************
*
*  Copyright (C) 2017-2022 Advanced Micro Devices, Inc.  All rights reserved.
*  SPDX-License-Identifier: MIT
*
***********************************************************************************************************************/

#ifndef _AMDGPU_ASIC_ADDR_H
#define _AMDGPU_ASIC_ADDR_H

#define ATI_VENDOR_ID         0x1002
#define AMD_VENDOR_ID         0x1022

// AMDGPU_VENDOR_IS_AMD(vendorId)
#define AMDGPU_VENDOR_IS_AMD(v) ((v == ATI_VENDOR_ID) || (v == AMD_VENDOR_ID))

#define FAMILY_UNKNOWN 0x00
#define FAMILY_TN      0x69 //# 105 / Trinity APUs
#define FAMILY_SI      0x6E //# 110 / Southern Islands: Tahiti, Pitcairn, CapeVerde, Oland, Hainan
#define FAMILY_CI      0x78 //# 120 / Sea Islands: Bonaire, Hawaii
#define FAMILY_KV      0x7D //# 125 / Kaveri APUs: Spectre, Spooky, Kalindi, Godavari
#define FAMILY_VI      0x82 //# 130 / Volcanic Islands: Iceland, Tonga, Fiji
#define FAMILY_POLARIS 0x82 //# 130 / Polaris: 10, 11, 12
#define FAMILY_CZ      0x87 //# 135 / Carrizo APUs: Carrizo, Stoney
#define FAMILY_AI      0x8D //# 141 / Vega: 10, 20
#define FAMILY_RV      0x8E //# 142 / Raven
#define FAMILY_NV      0x8F //# 143 / Navi: 10
#define FAMILY_VGH     0x90 //# 144 / Van Gogh
#define FAMILY_NV3     0x91 //# 145 / Navi: 3x
#define FAMILY_GFX1103 0x94
#define FAMILY_GFX1150 0x96
#define FAMILY_RMB     0x92 //# 146 / Rembrandt
#define FAMILY_RPL     0x95 //# 149 / Raphael
#define FAMILY_MDN     0x97 //# 151 / Mendocino

// AMDGPU_FAMILY_IS(familyId, familyName)
#define FAMILY_IS(f, fn)     (f == FAMILY_##fn)
#define FAMILY_IS_TN(f)      FAMILY_IS(f, TN)
#define FAMILY_IS_SI(f)      FAMILY_IS(f, SI)
#define FAMILY_IS_CI(f)      FAMILY_IS(f, CI)
#define FAMILY_IS_KV(f)      FAMILY_IS(f, KV)
#define FAMILY_IS_VI(f)      FAMILY_IS(f, VI)
#define FAMILY_IS_POLARIS(f) FAMILY_IS(f, POLARIS)
#define FAMILY_IS_CZ(f)      FAMILY_IS(f, CZ)
#define FAMILY_IS_AI(f)      FAMILY_IS(f, AI)
#define FAMILY_IS_RV(f)      FAMILY_IS(f, RV)
#define FAMILY_IS_NV(f)      FAMILY_IS(f, NV)
#define FAMILY_IS_NV3(f)     FAMILY_IS(f, NV3)
#define FAMILY_IS_RMB(f)     FAMILY_IS(f, RMB)

#define AMDGPU_UNKNOWN          0xFF

#define AMDGPU_TAHITI_RANGE     0x05, 0x14 //#  5 <= x < 20
#define AMDGPU_PITCAIRN_RANGE   0x15, 0x28 //# 21 <= x < 40
#define AMDGPU_CAPEVERDE_RANGE  0x29, 0x3C //# 41 <= x < 60
#define AMDGPU_OLAND_RANGE      0x3C, 0x46 //# 60 <= x < 70
#define AMDGPU_HAINAN_RANGE     0x46, 0xFF //# 70 <= x < max

#define AMDGPU_BONAIRE_RANGE    0x14, 0x28 //# 20 <= x < 40
#define AMDGPU_HAWAII_RANGE     0x28, 0x3C //# 40 <= x < 60

#define AMDGPU_SPECTRE_RANGE    0x01, 0x41 //#   1 <= x < 65
#define AMDGPU_SPOOKY_RANGE     0x41, 0x81 //#  65 <= x < 129
#define AMDGPU_KALINDI_RANGE    0x81, 0xA1 //# 129 <= x < 161
#define AMDGPU_GODAVARI_RANGE   0xA1, 0xFF //# 161 <= x < max

#define AMDGPU_ICELAND_RANGE    0x01, 0x14 //#  1 <= x < 20
#define AMDGPU_TONGA_RANGE      0x14, 0x28 //# 20 <= x < 40
#define AMDGPU_FIJI_RANGE       0x3C, 0x50 //# 60 <= x < 80

#define AMDGPU_POLARIS10_RANGE  0x50, 0x5A //#  80 <= x < 90
#define AMDGPU_POLARIS11_RANGE  0x5A, 0x64 //#  90 <= x < 100
#define AMDGPU_POLARIS12_RANGE  0x64, 0x6E //# 100 <= x < 110
#define AMDGPU_VEGAM_RANGE      0x6E, 0xFF //# 110 <= x < max

#define AMDGPU_CARRIZO_RANGE    0x01, 0x21 //#  1 <= x < 33
#define AMDGPU_BRISTOL_RANGE    0x10, 0x21 //# 16 <= x < 33
#define AMDGPU_STONEY_RANGE     0x61, 0xFF //# 97 <= x < max

#define AMDGPU_VEGA10_RANGE     0x01, 0x14 //#  1 <= x < 20
#define AMDGPU_VEGA12_RANGE     0x14, 0x28 //# 20 <= x < 40
#define AMDGPU_VEGA20_RANGE     0x28, 0xFF //# 40 <= x < max

#define AMDGPU_RAVEN_RANGE      0x01, 0x81 //#   1 <= x < 129
#define AMDGPU_RAVEN2_RANGE     0x81, 0x90 //# 129 <= x < 144
#define AMDGPU_RENOIR_RANGE     0x91, 0xFF //# 145 <= x < max

#define AMDGPU_NAVI10_RANGE     0x01, 0x0A //# 1  <= x < 10
#define AMDGPU_NAVI12_RANGE     0x0A, 0x14 //# 10 <= x < 20
#define AMDGPU_NAVI14_RANGE     0x14, 0x28 //# 20 <= x < 40
#define AMDGPU_NAVI21_RANGE     0x28, 0x32 //# 40  <= x < 50
#define AMDGPU_NAVI22_RANGE     0x32, 0x3C //# 50  <= x < 60
#define AMDGPU_NAVI23_RANGE     0x3C, 0x46 //# 60  <= x < 70
#define AMDGPU_NAVI24_RANGE     0x46, 0x50 //# 70  <= x < 80

#define AMDGPU_VANGOGH_RANGE    0x01, 0xFF //# 1 <= x < max

#define AMDGPU_NAVI31_RANGE     0x01, 0x10 //# 01 <= x < 16
#define AMDGPU_NAVI32_RANGE     0x20, 0xFF //# 32 <= x < 255
#define AMDGPU_NAVI33_RANGE     0x10, 0x20 //# 16 <= x < 32
#define AMDGPU_GFX1103_R1_RANGE 0x01, 0x80 //# 1 <= x < 128
#define AMDGPU_GFX1103_R2_RANGE 0x80, 0xFF //# 128 <= x < max

#define AMDGPU_GFX1150_RANGE    0x01, 0xFF //# 1 <= x < max

#define AMDGPU_REMBRANDT_RANGE  0x01, 0xFF //# 01 <= x < 255

#define AMDGPU_RAPHAEL_RANGE    0x01, 0xFF //# 1 <= x < max

#define AMDGPU_MENDOCINO_RANGE  0x01, 0xFF //# 1 <= x < max

#define AMDGPU_EXPAND_FIX(x) x
#define AMDGPU_RANGE_HELPER(val, min, max) ((val >= min) && (val < max))
#define AMDGPU_IN_RANGE(val, ...)   AMDGPU_EXPAND_FIX(AMDGPU_RANGE_HELPER(val, __VA_ARGS__))


// ASICREV_IS(eRevisionId, revisionName)
#define ASICREV_IS(r, rn)              AMDGPU_IN_RANGE(r, AMDGPU_##rn##_RANGE)
#define ASICREV_IS_TAHITI_P(r)         ASICREV_IS(r, TAHITI)
#define ASICREV_IS_PITCAIRN_PM(r)      ASICREV_IS(r, PITCAIRN)
#define ASICREV_IS_CAPEVERDE_M(r)      ASICREV_IS(r, CAPEVERDE)
#define ASICREV_IS_OLAND_M(r)          ASICREV_IS(r, OLAND)
#define ASICREV_IS_HAINAN_V(r)         ASICREV_IS(r, HAINAN)

#define ASICREV_IS_BONAIRE_M(r)        ASICREV_IS(r, BONAIRE)
#define ASICREV_IS_HAWAII_P(r)         ASICREV_IS(r, HAWAII)

#define ASICREV_IS_SPECTRE(r)          ASICREV_IS(r, SPECTRE)
#define ASICREV_IS_SPOOKY(r)           ASICREV_IS(r, SPOOKY)
#define ASICREV_IS_KALINDI(r)          ASICREV_IS(r, KALINDI)
#define ASICREV_IS_KALINDI_GODAVARI(r) ASICREV_IS(r, GODAVARI)

#define ASICREV_IS_ICELAND_M(r)        ASICREV_IS(r, ICELAND)
#define ASICREV_IS_TONGA_P(r)          ASICREV_IS(r, TONGA)
#define ASICREV_IS_FIJI_P(r)           ASICREV_IS(r, FIJI)

#define ASICREV_IS_POLARIS10_P(r)      ASICREV_IS(r, POLARIS10)
#define ASICREV_IS_POLARIS11_M(r)      ASICREV_IS(r, POLARIS11)
#define ASICREV_IS_POLARIS12_V(r)      ASICREV_IS(r, POLARIS12)
#define ASICREV_IS_VEGAM_P(r)          ASICREV_IS(r, VEGAM)

#define ASICREV_IS_CARRIZO(r)          ASICREV_IS(r, CARRIZO)
#define ASICREV_IS_CARRIZO_BRISTOL(r)  ASICREV_IS(r, BRISTOL)
#define ASICREV_IS_STONEY(r)           ASICREV_IS(r, STONEY)

#define ASICREV_IS_VEGA10_M(r)         ASICREV_IS(r, VEGA10)
#define ASICREV_IS_VEGA10_P(r)         ASICREV_IS(r, VEGA10)
#define ASICREV_IS_VEGA12_P(r)         ASICREV_IS(r, VEGA12)
#define ASICREV_IS_VEGA12_p(r)         ASICREV_IS(r, VEGA12)
#define ASICREV_IS_VEGA20_P(r)         ASICREV_IS(r, VEGA20)

#define ASICREV_IS_RAVEN(r)            ASICREV_IS(r, RAVEN)
#define ASICREV_IS_RAVEN2(r)           ASICREV_IS(r, RAVEN2)
#define ASICREV_IS_RENOIR(r)           ASICREV_IS(r, RENOIR)

#define ASICREV_IS_NAVI10_P(r)         ASICREV_IS(r, NAVI10)

#define ASICREV_IS_NAVI12_P(r)         ASICREV_IS(r, NAVI12)

#define ASICREV_IS_NAVI14_M(r)         ASICREV_IS(r, NAVI14)

#define ASICREV_IS_NAVI21_M(r)         ASICREV_IS(r, NAVI21)

#define ASICREV_IS_NAVI22_P(r)         ASICREV_IS(r, NAVI22)

#define ASICREV_IS_NAVI23_P(r)         ASICREV_IS(r, NAVI23)

#define ASICREV_IS_NAVI24_P(r)         ASICREV_IS(r, NAVI24)

#define ASICREV_IS_VANGOGH(r)          ASICREV_IS(r, VANGOGH)

#define ASICREV_IS_NAVI31_P(r)         ASICREV_IS(r, NAVI31)
#define ASICREV_IS_NAVI32_P(r)         ASICREV_IS(r, NAVI32)
#define ASICREV_IS_NAVI33_P(r)         ASICREV_IS(r, NAVI33)
#define ASICREV_IS_GFX1103_R1(r)       ASICREV_IS(r, GFX1103_R1)
#define ASICREV_IS_GFX1103_R2(r)       ASICREV_IS(r, GFX1103_R2)
#define ASICREV_IS_GFX1150(r)          ASICREV_IS(r, GFX1150)

#define ASICREV_IS_REMBRANDT(r)        ASICREV_IS(r, REMBRANDT)

#define ASICREV_IS_RAPHAEL(r)          ASICREV_IS(r, RAPHAEL)

#define ASICREV_IS_MENDOCINO(r)        ASICREV_IS(r, MENDOCINO)

#endif // _AMDGPU_ASIC_ADDR_H
