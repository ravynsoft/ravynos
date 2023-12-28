# name: EABI attribute names
# source: attr-names.s
# as:
# readelf: -A
# This test is only valid on EABI based ports.
# target: *-*-*eabi* *-*-nacl*

Attribute Section: aeabi
File Attributes
  Tag_conformance: "2.08"
  Tag_nodefaults: True
  Tag_CPU_raw_name: "random-cpu"
  Tag_CPU_name: "cpu"
  Tag_CPU_arch: v4
  Tag_CPU_arch_profile: Application or Realtime
  Tag_ARM_ISA_use: Yes
  Tag_THUMB_ISA_use: Thumb-1
  Tag_FP_arch: VFPv1
  Tag_WMMX_arch: WMMXv1
  Tag_Advanced_SIMD_arch: NEONv1
  Tag_PCS_config: Bare platform
  Tag_ABI_PCS_R9_use: SB
  Tag_ABI_PCS_RW_data: PC-relative
  Tag_ABI_PCS_RO_data: PC-relative
  Tag_ABI_PCS_GOT_use: direct
  Tag_ABI_PCS_wchar_t: 2
  Tag_ABI_FP_rounding: Needed
  Tag_ABI_FP_denormal: Needed
  Tag_ABI_FP_exceptions: Needed
  Tag_ABI_FP_user_exceptions: Needed
  Tag_ABI_FP_number_model: Finite
  Tag_ABI_align_needed: 8-byte
  Tag_ABI_align_preserved: 8-byte, except leaf SP
  Tag_ABI_enum_size: small
  Tag_ABI_HardFP_use: SP only
  Tag_ABI_VFP_args: VFP registers
  Tag_ABI_WMMX_args: WMMX registers
  Tag_ABI_optimization_goals: Prefer Speed
  Tag_ABI_FP_optimization_goals: Prefer Speed
  Tag_compatibility: flag = 1, vendor = gnu
  Tag_CPU_unaligned_access: v6
  Tag_FP_HP_extension: Allowed
  Tag_ABI_FP_16bit_format: IEEE 754
  Tag_MPextension_use: Allowed
  Tag_DIV_use: Not allowed
  Tag_also_compatible_with: v6-M
  Tag_T2EE_use: Allowed
  Tag_Virtualization_use: TrustZone and Virtualization Extensions
