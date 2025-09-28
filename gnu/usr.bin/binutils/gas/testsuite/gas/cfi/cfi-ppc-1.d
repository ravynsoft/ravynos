#objdump: -Wf
#name: CFI on ppc
#as: -a32

.*

Contents of the .eh_frame section:

0+0000 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: [24]
  Data alignment factor: -4
  Return address column: 65
  Augmentation data:     [01]b

  DW_CFA_def_cfa: r1 ofs 0

0+0014 0+0020 0+0018 FDE cie=0+0000 pc=0+0000..0+0070
  DW_CFA_advance_loc: 4 to 0+0004
  DW_CFA_def_cfa_offset: 48
  DW_CFA_advance_loc: 16 to 0+0014
  DW_CFA_offset: r27 at cfa-20
  DW_CFA_offset: r26 at cfa-24
  DW_CFA_offset_extended_sf: r65 at cfa\+4
  DW_CFA_advance_loc: 8 to 0+001c
  DW_CFA_offset: r28 at cfa-16
  DW_CFA_advance_loc: 8 to 0+0024
  DW_CFA_offset: r29 at cfa-12
  DW_CFA_nop
  DW_CFA_nop

