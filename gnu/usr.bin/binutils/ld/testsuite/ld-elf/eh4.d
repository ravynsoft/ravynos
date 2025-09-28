#source: eh4.s
#source: eh4a.s
#as: --64
#ld: -melf_x86_64 -shared -Ttext 0x400 -z max-page-size=0x200000 -z noseparate-code -z noexecstack
#readelf: -wf
#target: x86_64-*-*

Contents of the .eh_frame section:

0+0000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -8
  Return address column: 16
  Augmentation data:     1b

  DW_CFA_def_cfa: r7 \(rsp\) ofs 8
  DW_CFA_offset: r16 \(rip\) at cfa-8
  DW_CFA_nop
  DW_CFA_nop

0+0018 0+0014 0+001c FDE cie=0+0000 pc=0+0400..0+0413
  DW_CFA_set_loc: 0+0404
  DW_CFA_def_cfa_offset: 80

0+0030 0+0014 0+0034 FDE cie=0+0000 pc=0+0413..0+0426
  DW_CFA_set_loc: 0+0417
  DW_CFA_def_cfa_offset: 80

0+0048 0+002[04] 0+004c FDE cie=0+0000 pc=[0-9a-f]+\.\.[0-9a-f]+
  DW_CFA_def_cfa_offset: 16
  DW_CFA_advance_loc: [0-9a-f]+ to [0-9a-f]+
  DW_CFA_def_cfa_offset: 24
  DW_CFA_advance_loc: [0-9a-f]+ to [0-9a-f]+
  DW_CFA_def_cfa_expression \(DW_OP_breg7 \(rsp\): 8; DW_OP_breg16 \(rip\): 0;.*
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

[0-9a-f]+ ZERO terminator
#pass
