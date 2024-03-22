/*
 * Copyright © 2017 Advanced Micro Devices, Inc.
 *
 * SPDX-License-Identifier: MIT
 */

#include "amd_family.h"
#include "addrlib/src/amdgpu_asic_addr.h"
#include "util/macros.h"

const char *ac_get_family_name(enum radeon_family family)
{
   switch (family) {
   case CHIP_TAHITI:
      return "TAHITI";
   case CHIP_PITCAIRN:
      return "PITCAIRN";
   case CHIP_VERDE:
      return "VERDE";
   case CHIP_OLAND:
      return "OLAND";
   case CHIP_HAINAN:
      return "HAINAN";
   case CHIP_BONAIRE:
      return "BONAIRE";
   case CHIP_KABINI:
      return "KABINI";
   case CHIP_KAVERI:
      return "KAVERI";
   case CHIP_HAWAII:
      return "HAWAII";
   case CHIP_TONGA:
      return "TONGA";
   case CHIP_ICELAND:
      return "ICELAND";
   case CHIP_CARRIZO:
      return "CARRIZO";
   case CHIP_FIJI:
      return "FIJI";
   case CHIP_STONEY:
      return "STONEY";
   case CHIP_POLARIS10:
      return "POLARIS10";
   case CHIP_POLARIS11:
      return "POLARIS11";
   case CHIP_POLARIS12:
      return "POLARIS12";
   case CHIP_VEGAM:
      return "VEGAM";
   case CHIP_VEGA10:
      return "VEGA10";
   case CHIP_RAVEN:
      return "RAVEN";
   case CHIP_VEGA12:
      return "VEGA12";
   case CHIP_VEGA20:
      return "VEGA20";
   case CHIP_RAVEN2:
      return "RAVEN2";
   case CHIP_RENOIR:
      return "RENOIR";
   case CHIP_MI100:
      return "MI100";
   case CHIP_MI200:
      return "MI200";
   case CHIP_GFX940:
      return "GFX940";
   case CHIP_NAVI10:
      return "NAVI10";
   case CHIP_NAVI12:
      return "NAVI12";
   case CHIP_NAVI14:
      return "NAVI14";
   case CHIP_NAVI21:
      return "NAVI21";
   case CHIP_NAVI22:
      return "NAVI22";
   case CHIP_NAVI23:
      return "NAVI23";
   case CHIP_VANGOGH:
      return "VANGOGH";
   case CHIP_NAVI24:
      return "NAVI24";
   case CHIP_REMBRANDT:
      return "REMBRANDT";
   case CHIP_RAPHAEL_MENDOCINO:
      return "RAPHAEL_MENDOCINO";
   case CHIP_NAVI31:
      return "NAVI31";
   case CHIP_NAVI32:
      return "NAVI32";
   case CHIP_NAVI33:
      return "NAVI33";
   case CHIP_GFX1103_R1:
      return "GFX1103_R1";
   case CHIP_GFX1103_R2:
      return "GFX1103_R2";
   case CHIP_GFX1150:
      return "GFX1150";
   default:
      unreachable("Unknown GPU family");
   }
}

enum amd_gfx_level ac_get_gfx_level(enum radeon_family family)
{
   if (family >= CHIP_GFX1150)
      return GFX11_5;
   if (family >= CHIP_NAVI31)
      return GFX11;
   if (family >= CHIP_NAVI21)
      return GFX10_3;
   if (family >= CHIP_NAVI10)
      return GFX10;
   if (family >= CHIP_VEGA10)
      return GFX9;
   if (family >= CHIP_TONGA)
      return GFX8;
   if (family >= CHIP_BONAIRE)
      return GFX7;

   return GFX6;
}

unsigned ac_get_family_id(enum radeon_family family)
{
   if (family >= CHIP_GFX1150)
      return FAMILY_GFX1150;
   if (family >= CHIP_NAVI31)
      return FAMILY_NV3;
   if (family >= CHIP_NAVI21)
      return FAMILY_NV;
   if (family >= CHIP_NAVI10)
      return FAMILY_NV;
   if (family >= CHIP_VEGA10)
      return FAMILY_AI;
   if (family >= CHIP_TONGA)
      return FAMILY_VI;
   if (family >= CHIP_BONAIRE)
      return FAMILY_CI;

   return FAMILY_SI;
}

const char *ac_get_llvm_processor_name(enum radeon_family family)
{
   switch (family) {
   case CHIP_TAHITI:
      return "tahiti";
   case CHIP_PITCAIRN:
      return "pitcairn";
   case CHIP_VERDE:
      return "verde";
   case CHIP_OLAND:
      return "oland";
   case CHIP_HAINAN:
      return "hainan";
   case CHIP_BONAIRE:
      return "bonaire";
   case CHIP_KABINI:
      return "kabini";
   case CHIP_KAVERI:
      return "kaveri";
   case CHIP_HAWAII:
      return "hawaii";
   case CHIP_TONGA:
      return "tonga";
   case CHIP_ICELAND:
      return "iceland";
   case CHIP_CARRIZO:
      return "carrizo";
   case CHIP_FIJI:
      return "fiji";
   case CHIP_STONEY:
      return "stoney";
   case CHIP_POLARIS10:
      return "polaris10";
   case CHIP_POLARIS11:
   case CHIP_POLARIS12:
   case CHIP_VEGAM:
      return "polaris11";
   case CHIP_VEGA10:
      return "gfx900";
   case CHIP_RAVEN:
      return "gfx902";
   case CHIP_VEGA12:
      return "gfx904";
   case CHIP_VEGA20:
      return "gfx906";
   case CHIP_RAVEN2:
   case CHIP_RENOIR:
      return "gfx909";
   case CHIP_MI100:
      return "gfx908";
   case CHIP_MI200:
      return "gfx90a";
   case CHIP_GFX940:
      return "gfx940";
   case CHIP_NAVI10:
      return "gfx1010";
   case CHIP_NAVI12:
      return "gfx1011";
   case CHIP_NAVI14:
      return "gfx1012";
   case CHIP_NAVI21:
      return "gfx1030";
   case CHIP_NAVI22:
      return "gfx1031";
   case CHIP_NAVI23:
      return "gfx1032";
   case CHIP_VANGOGH:
      return "gfx1033";
   case CHIP_NAVI24:
      return "gfx1034";
   case CHIP_REMBRANDT:
      return "gfx1035";
   case CHIP_RAPHAEL_MENDOCINO:
      return "gfx1036";
   case CHIP_NAVI31:
      return "gfx1100";
   case CHIP_NAVI32:
      return "gfx1101";
   case CHIP_NAVI33:
      return "gfx1102";
   case CHIP_GFX1103_R1:
   case CHIP_GFX1103_R2:
      return "gfx1103";
   case CHIP_GFX1150:
      return "gfx1150";
   default:
      return "";
   }
}
