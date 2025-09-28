#source: ibt-plt-3.s
#as: --64 -defsym __64_bit__=1
#ld: -shared -m elf_x86_64 -z ibtplt --hash-style=sysv -z max-page-size=0x200000 -z noseparate-code
#readelf: -wf -n

Contents of the .eh_frame section:

0+ 0000000000000014 00000000 CIE
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

0+18 0000000000000014 0000001c FDE cie=00000000 pc=0000000000000240..0000000000000252
  DW_CFA_advance_loc: 4 to 0000000000000244
  DW_CFA_def_cfa_offset: 16
  DW_CFA_advance_loc: 9 to 000000000000024d
  DW_CFA_def_cfa_offset: 8
  DW_CFA_nop

0+30 0000000000000024 00000034 FDE cie=00000000 pc=00000000000001f0..0000000000000220
  DW_CFA_def_cfa_offset: 16
  DW_CFA_advance_loc: 6 to 00000000000001f6
  DW_CFA_def_cfa_offset: 24
  DW_CFA_advance_loc: 10 to 0000000000000200
  DW_CFA_def_cfa_expression \(DW_OP_breg7 \(rsp\): 8; DW_OP_breg16 \(rip\): 0; DW_OP_lit15; DW_OP_and; DW_OP_lit9; DW_OP_ge; DW_OP_lit3; DW_OP_shl; DW_OP_plus\)
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+58 0000000000000010 0000005c FDE cie=00000000 pc=0000000000000220..0000000000000240
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

