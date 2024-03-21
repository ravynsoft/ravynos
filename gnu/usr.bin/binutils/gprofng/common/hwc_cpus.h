/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* Hardware counter profiling: cpu types */

#ifndef __HWC_CPUS_H
#define __HWC_CPUS_H

#define MAX_PICS    20 /* Max # of HW ctrs that can be enabled simultaneously */

  /* type for specifying CPU register number */
  typedef int regno_t;
#define REGNO_ANY       ((regno_t)-1)
#define REGNO_INVALID   ((regno_t)-2)

  /* --- Utilities for use with regno_t and reg_list[] --- */
#define REG_LIST_IS_EMPTY(reg_list) (!(reg_list) || (reg_list)[0] == REGNO_ANY)
#define REG_LIST_EOL(regno)         ((regno)==REGNO_ANY)
#define REG_LIST_SINGLE_VALID_ENTRY(reg_list) \
  (((reg_list) && (reg_list)[1] == REGNO_ANY && \
      (reg_list)[0] != REGNO_ANY ) ? (reg_list)[0] : REGNO_ANY)

  /* enum for specifying unknown or uninitialized CPU */
  enum
  {
    CPUVER_GENERIC = 0,
    CPUVER_UNDEFINED = -1
  };

  // Note: changing an values below may make older HWC experiments unreadable.
  // --- Sun/Oracle SPARC ---
#define CPC_ULTRA1              1000
#define CPC_ULTRA2              1001
#define CPC_ULTRA3              1002
#define CPC_ULTRA3_PLUS         1003
#define CPC_ULTRA3_I            1004
#define CPC_ULTRA4_PLUS         1005 /* Panther */
#define CPC_ULTRA4              1017 /* Jaguar */
#define CPC_ULTRA_T1            1100 /* Niagara1 */
#define CPC_ULTRA_T2            1101 /* Niagara2 */
#define CPC_ULTRA_T2P           1102
#define CPC_ULTRA_T3            1103
#define CPC_SPARC_T4            1104
#define CPC_SPARC_T5            1110
#define CPC_SPARC_T6            1120
// #define CPC_SPARC_T7         1130 // use CPC_SPARC_M7
#define CPC_SPARC_M4            1204 /* Obsolete */
#define CPC_SPARC_M5            1210
#define CPC_SPARC_M6            1220
#define CPC_SPARC_M7            1230
#define CPC_SPARC_M8            1240

  // --- Intel ---
  // Pentium
#define CPC_PENTIUM             2000
#define CPC_PENTIUM_MMX         2001
#define CPC_PENTIUM_PRO         2002
#define CPC_PENTIUM_PRO_MMX     2003
#define CPC_PENTIUM_4           2017
#define CPC_PENTIUM_4_HT        2027

  // Core Microarchitecture (Merom/Menryn)
#define CPC_INTEL_CORE2         2028
#define CPC_INTEL_NEHALEM       2040
#define CPC_INTEL_WESTMERE      2042
#define CPC_INTEL_SANDYBRIDGE   2045
#define CPC_INTEL_IVYBRIDGE     2047
#define CPC_INTEL_ATOM          2050 /* Atom*/
#define CPC_INTEL_HASWELL       2060
#define CPC_INTEL_BROADWELL     2070
#define CPC_INTEL_SKYLAKE       2080
#define CPC_INTEL_UNKNOWN       2499
#define CPC_AMD_K8C             2500 /* Opteron, Athlon... */
#define CPC_AMD_FAM_10H         2501 /* Barcelona, Shanghai... */
#define CPC_AMD_FAM_11H         2502 /* Griffin... */
#define CPC_AMD_FAM_15H         2503
#define CPC_KPROF               3003 // OBSOLETE (To support 12.3 and earlier)
#define CPC_FOX                 3004 /* pseudo-chip */

  // --- Fujitsu ---
#define CPC_SPARC64_III     3000
#define CPC_SPARC64_V       3002
#define CPC_SPARC64_VI      4003 /* OPL-C */
#define CPC_SPARC64_VII     4004 /* Jupiter */
#define CPC_SPARC64_X       4006 /* Athena */
#define CPC_SPARC64_XII     4010 /* Athena++ */

// aarch64. Constants from arch/arm64/include/asm/cputype.h
enum {
    ARM_CPU_IMP_ARM     = 0x41,
    ARM_CPU_IMP_BRCM    = 0x42,
    ARM_CPU_IMP_CAVIUM  = 0x43,
    ARM_CPU_IMP_APM     = 0x50,
    ARM_CPU_IMP_QCOM    = 0x51
};

#define	AARCH64_VENDORSTR_ARM	"ARM"

  /* strings below must match those returned by cpc_getcpuver() */
  typedef struct
  {
    int cpc2_cpuver;
    const char * cpc2_cciname;
  } libcpc2_cpu_lookup_t;
#define LIBCPC2_CPU_LOOKUP_LIST \
  {CPC_AMD_K8C               , "AMD Opteron & Athlon64"}, \
  {CPC_AMD_FAM_10H           , "AMD Family 10h"}, \
  {CPC_AMD_FAM_11H           , "AMD Family 11h"}, \
  {CPC_AMD_FAM_15H           , "AMD Family 15h Model 01h"}, \
  {CPC_AMD_FAM_15H           , "AMD Family 15h Model 02h"},/*future*/ \
  {CPC_AMD_FAM_15H           , "AMD Family 15h Model 03h"},/*future*/ \
  {CPC_PENTIUM_4_HT          , "Pentium 4 with HyperThreading"}, \
  {CPC_PENTIUM_4             , "Pentium 4"}, \
  {CPC_PENTIUM_PRO_MMX       , "Pentium Pro with MMX, Pentium II"}, \
  {CPC_PENTIUM_PRO           , "Pentium Pro, Pentium II"}, \
  {CPC_PENTIUM_MMX           , "Pentium with MMX"}, \
  {CPC_PENTIUM               , "Pentium"}, \
  {CPC_INTEL_CORE2           , "Core Microarchitecture"}, \
    /* Merom:  F6M15: Clovertown, Kentsfield, Conroe, Merom, Woodcrest */ \
    /* Merom:  F6M22: Merom Conroe */ \
    /* Penryn: F6M23: Yorkfield, Wolfdale, Penryn, Harpertown */ \
    /* Penryn: F6M29: Dunnington */ \
  {CPC_INTEL_NEHALEM         , "Intel Arch PerfMon v3 on Family 6 Model 26"},/*Bloomfield, Nehalem EP*/ \
  {CPC_INTEL_NEHALEM         , "Intel Arch PerfMon v3 on Family 6 Model 30"},/*Clarksfield, Lynnfield, Jasper Forest*/ \
  {CPC_INTEL_NEHALEM         , "Intel Arch PerfMon v3 on Family 6 Model 31"},/*(TBD)*/ \
  {CPC_INTEL_NEHALEM         , "Intel Arch PerfMon v3 on Family 6 Model 46"},/*Nehalem EX*/ \
  {CPC_INTEL_WESTMERE        , "Intel Arch PerfMon v3 on Family 6 Model 37"},/*Arrandale, Clarskdale*/ \
  {CPC_INTEL_WESTMERE        , "Intel Arch PerfMon v3 on Family 6 Model 44"},/*Gulftown, Westmere EP*/ \
  {CPC_INTEL_WESTMERE        , "Intel Arch PerfMon v3 on Family 6 Model 47"},/*Westmere EX*/ \
  {CPC_INTEL_SANDYBRIDGE     , "Intel Arch PerfMon v3 on Family 6 Model 42"},/*Sandy Bridge*/ \
  {CPC_INTEL_SANDYBRIDGE     , "Intel Arch PerfMon v3 on Family 6 Model 45"},/*Sandy Bridge E, SandyBridge-EN, SandyBridge EP*/ \
  {CPC_INTEL_IVYBRIDGE       , "Intel Arch PerfMon v3 on Family 6 Model 58"},/*Ivy Bridge*/ \
  {CPC_INTEL_IVYBRIDGE       , "Intel Arch PerfMon v3 on Family 6 Model 62"},/*(TBD)*/ \
  {CPC_INTEL_ATOM            , "Intel Arch PerfMon v3 on Family 6 Model 28"},/*Atom*/ \
  {CPC_INTEL_HASWELL         , "Intel Arch PerfMon v3 on Family 6 Model 60"},/*Haswell*/ \
  {CPC_INTEL_HASWELL         , "Intel Arch PerfMon v3 on Family 6 Model 63"},/*Haswell*/ \
  {CPC_INTEL_HASWELL         , "Intel Arch PerfMon v3 on Family 6 Model 69"},/*Haswell*/ \
  {CPC_INTEL_HASWELL         , "Intel Arch PerfMon v3 on Family 6 Model 70"},/*Haswell*/ \
  {CPC_INTEL_BROADWELL       , "Intel Arch PerfMon v3 on Family 6 Model 61"},/*Broadwell*/ \
  {CPC_INTEL_BROADWELL       , "Intel Arch PerfMon v3 on Family 6 Model 71"},/*Broadwell*/ \
  {CPC_INTEL_BROADWELL       , "Intel Arch PerfMon v3 on Family 6 Model 79"},/*Broadwell*/ \
  {CPC_INTEL_BROADWELL       , "Intel Arch PerfMon v3 on Family 6 Model 86"},/*Broadwell*/ \
  {CPC_INTEL_SKYLAKE         , "Intel Arch PerfMon v4 on Family 6 Model 78"},/*Skylake*/ \
  {CPC_INTEL_SKYLAKE         , "Intel Arch PerfMon v4 on Family 6 Model 85"},/*Skylake*/ \
  {CPC_INTEL_SKYLAKE         , "Intel Arch PerfMon v4 on Family 6 Model 94"},/*Skylake*/ \
  {CPC_INTEL_UNKNOWN         , "Intel Arch PerfMon"},/*Not yet in table*/ \
  {CPC_SPARC64_III           , "SPARC64 III"/*?*/}, \
  {CPC_SPARC64_V             , "SPARC64 V"/*?*/}, \
  {CPC_SPARC64_VI            , "SPARC64 VI"}, \
  {CPC_SPARC64_VII           , "SPARC64 VI & VII"}, \
  {CPC_SPARC64_X             , "SPARC64 X"}, \
  {CPC_SPARC64_XII           , "SPARC64 XII"}, \
  {CPC_ULTRA_T1              , "UltraSPARC T1"}, \
  {CPC_ULTRA_T2              , "UltraSPARC T2"}, \
  {CPC_ULTRA_T2P             , "UltraSPARC T2+"}, \
  {CPC_ULTRA_T3              , "SPARC T3"},  \
  {CPC_SPARC_T4              , "SPARC T4"},  \
  {CPC_SPARC_M4              , "SPARC M4"},  \
  {CPC_SPARC_T5              , "SPARC T5"},  \
  {CPC_SPARC_M5              , "SPARC M5"},  \
  {CPC_SPARC_T6              , "SPARC T6"},  \
  {CPC_SPARC_M6              , "SPARC M6"},  \
  {CPC_SPARC_M7              , "SPARC T7"},  \
  {CPC_SPARC_M7              , "SPARC 3e40"},  \
  {CPC_SPARC_M7              , "SPARC M7"},  \
  {CPC_SPARC_M8              , "SPARC 3e50"},  \
  {CPC_ULTRA4_PLUS           , "UltraSPARC IV+"}, \
  {CPC_ULTRA4                , "UltraSPARC IV"}, \
  {CPC_ULTRA3_I              , "UltraSPARC IIIi"}, \
  {CPC_ULTRA3_I              , "UltraSPARC IIIi & IIIi+"}, \
  {CPC_ULTRA3_PLUS           , "UltraSPARC III+"}, \
  {CPC_ULTRA3_PLUS           , "UltraSPARC III+ & IV"}, \
  {CPC_ULTRA3                , "UltraSPARC III"}, \
  {CPC_ULTRA2                , "UltraSPARC I&II"}, \
  {CPC_ULTRA1                , "UltraSPARC I&II"}, \
  {ARM_CPU_IMP_APM           , AARCH64_VENDORSTR_ARM}, \
  {0, NULL}
  /* init like this:
     static libcpc2_cpu_lookup_t cpu_table[]={LIBCPC2_CPU_LOOKUP_LIST};
   */
#endif
