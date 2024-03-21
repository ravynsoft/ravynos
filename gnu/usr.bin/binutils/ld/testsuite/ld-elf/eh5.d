#source: eh5.s
#source: eh5a.s
#source: eh5b.s
#ld:
#readelf: -wf
#target: [check_as_cfi]
#xfail: alpha-*-*ecoff tile*-*-* visium-*-*

Contents of the .eh_frame section:

0+0000 0+001[04] 0+0000 CIE
  Version:               [13]
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01][bc]

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

0+001[48] 0+001[4c] 0+001[8c] FDE cie=0+0000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00(2c|30|38) 0+0014 0+0000 CIE
  Version:               [13]
  Augmentation:          "zPR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     03 .. .. .. .. [01][bc]

  DW_CFA_nop

0+00(44|48|50) 0+001[4c] 0+001c FDE cie=0+00(2c|30|38) pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00(5c|60|70) 0+001[4c] 0+00(60|64|74) FDE cie=0+0000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00(74|78|90) 0+001[8c] 0+0000 CIE
  Version:               [13]
  Augmentation:          "zPLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     03 .. .. .. .. 0c [01][bc]

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

0+00(90|98|b0) 0+00(1c|24) 0+002[04] FDE cie=0+00(74|78|90) pc=.*
  Augmentation data:     (ef be ad de 00 00 00 00|00 00 00 00 de ad be ef)

  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+00(b0|b8|d8) 0+001[04] 0+0000 CIE
  Version:               [13]
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01][bc]

  DW_CFA_def_cfa: r0(.*) ofs 16
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

0+00(c4|d0|f0) 0+001[048] 0+001[8c] FDE cie=0+00(b0|b8|d8) pc=.*
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

0+0(0d8|0e8|10c) 0+001[48] 0+0000 CIE
  Version:               [13]
  Augmentation:          "zPR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     03 .. .. .. .. [01][bc]

  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

0+0(0f0|100|128) 0+001[4c] 0+00(1c|20) FDE cie=0+0(0d8|0e8|10c) pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+01[014]8 0+001[048] 0+00(5c|64|74) FDE cie=0+00(b0|b8|d8) pc=.*
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

0+01(1c|30|64) 0+001[8c] 0+0000 CIE
  Version:               [13]
  Augmentation:          "zPLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     03 .. .. .. .. 0c [01][bc]

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

0+01(38|50|80) 0+00(1c|24) 0+002[04] FDE cie=0+01(1c|30|64) pc=.*
  Augmentation data:     (ef be ad de 00 00 00 00|00 00 00 00 de ad be ef)

  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+01(58|70|a8) 0+001[4c] 0+01(5c|74|ac) FDE cie=0+0000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#?0+0170 0+0014 0+ CIE
#?  Version:               1
#?  Augmentation:          "zPR"
#?  Code alignment factor: .*
#?  Data alignment factor: .*
#?  Return address column: .*
#?  Augmentation data:     03 .. .. .. .. 1b
#?  DW_CFA_nop

0+01(70|88|c8) 0+001[4c] 0+0(01c|148|15c|194) FDE cie=0+0(02c|030|038|170) pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0+01(88|a0|e8) 0+001[4c] 0+01(8c|a4|ec) FDE cie=0+0000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

#?0+01b8 0+0018 0+ CIE
#?  Version:               1
#?  Augmentation:          "zPLR"
#?  Code alignment factor: .*
#?  Data alignment factor: .*
#?  Return address column: .*
#?  Augmentation data:     03 .. .. .. .. 0c 1b
#?  DW_CFA_nop
#?  DW_CFA_nop
#?  DW_CFA_nop

0+0(1a0|1b8|1d4|208) 0+00(1c|24) 0+0(020|130|144|17c) FDE cie=0+0(074|078|090|1b8) pc=.*
  Augmentation data:     (ef be ad de 00 00 00 00|00 00 00 00 de ad be ef)

  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0(.*) ofs 16
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

