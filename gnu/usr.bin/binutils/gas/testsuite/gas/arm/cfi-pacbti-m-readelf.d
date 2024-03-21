#readelf: -wf
#source: cfi-pacbti-m.s
#name: Call Frame information for Armv8.1-M.Mainline PACBTI extension
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince
# VxWorks needs a special variant of this file.
#skip: *-*-vxworks*

Contents of the .eh_frame section:


00000000 00000010 00000000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 2
  Data alignment factor: -4
  Return address column: 14
  Augmentation data:     1b
  DW_CFA_def_cfa: r13 ofs 0

00000014 00000020 00000018 FDE cie=00000000 pc=00000000..0000000c
  DW_CFA_advance_loc: 4 to 00000004
  DW_CFA_register: r143 in r12
  DW_CFA_advance_loc: 4 to 00000008
  DW_CFA_def_cfa_offset: 8
  DW_CFA_offset: r14 at cfa-8
  DW_CFA_offset: r12 at cfa-4
  DW_CFA_advance_loc: 4 to 0000000c
  DW_CFA_restore_extended: r143
  DW_CFA_restore: r14
  DW_CFA_def_cfa_offset: 0
