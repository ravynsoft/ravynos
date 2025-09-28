#source: eh3.s
#source: eh3a.s
#as: --64
#ld: -melf_x86_64 -Ttext 0x400078
#readelf: -wf
#target: x86_64-*-*

Contents of the .eh_frame section:

0+0000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          ""
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: 16

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_offset: r16 \(rip\) at cfa-8
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+0018 0+001c 0+001c FDE cie=0+0000 pc=0+400078\.\.0+400090
  DW_CFA_advance_loc: 8 to 0+400080
  DW_CFA_def_cfa_offset: 16
  DW_CFA_offset: r6 \(rbp\) at cfa-16
  DW_CFA_advance_loc: 8 to 0+400088
  DW_CFA_def_cfa_register: r6 \(rbp\)

0+0038 ZERO terminator
#pass
