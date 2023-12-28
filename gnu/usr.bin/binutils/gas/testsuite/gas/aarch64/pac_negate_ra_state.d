#objdump: --dwarf=frames
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.+:     file .+

Contents of the .eh_frame section:

0+ 0+10 0+ CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 4
  Data alignment factor: -8
  Return address column: 30
  Augmentation data:     1b
  DW_CFA_def_cfa: r31 \(sp\) ofs 0

0+14 0+18 0+18 FDE cie=0+ pc=0+\.\.0+8
  DW_CFA_advance_loc: 4 to 0+4
  DW_CFA_AARCH64_negate_ra_state
  DW_CFA_advance_loc: 4 to 0+8
  DW_CFA_def_cfa_offset: 16
  DW_CFA_offset: r29 \(x29\) at cfa-16
  DW_CFA_offset: r30 \(x30\) at cfa-8
  DW_CFA_nop
  DW_CFA_nop
