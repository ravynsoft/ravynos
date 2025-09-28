#source: cfia.s
#source: cfib.s
#ld: --file-align 1 --section-align 1
#objdump: -Wf

#...
00000000 00000010 ffffffff CIE
  Version:               1
  Augmentation:          ""
  Code alignment factor: 1
  Data alignment factor: \-4
  Return address column: 8

  DW_CFA_def_cfa: r4 \(esp\) ofs 4
  DW_CFA_offset: r8 \(eip\) at cfa\-4
  DW_CFA_nop
  DW_CFA_nop

00000014 00000018 00000000 FDE cie=00000000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa_offset: 16
  DW_CFA_offset: r6 \(esi\) at cfa\-16
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r7 \(edi\) ofs 8
  DW_CFA_restore: r6 \(esi\)
  DW_CFA_nop
  DW_CFA_nop
#pass
