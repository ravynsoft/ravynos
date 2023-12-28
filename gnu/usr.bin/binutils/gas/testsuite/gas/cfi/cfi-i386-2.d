#readelf: -wf
#name: CFI on i386, 2
# PE based targets do not support the .type pseudo-op
#notarget: *-*-mingw* *-*-cygwin* *-*-pe

Contents of the .eh_frame section:

00000000 0+0014 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: 1
  Data alignment factor: -4
  Return address column: 8
  Augmentation data:     1b

  DW_CFA_def_cfa: r4 \(esp\) ofs 4
  DW_CFA_offset: r8 \(eip\) at cfa-4
  DW_CFA_nop
  DW_CFA_nop

00000018 0+0018 0+001c FDE cie=0+0000 pc=0+0000..0+0009
  DW_CFA_advance_loc: 1 to 0+0001
  DW_CFA_def_cfa_offset: 8
  DW_CFA_offset: r5 \(ebp\) at cfa-8
  DW_CFA_advance_loc: 4 to 0+0005
  DW_CFA_offset: r3 \(ebx\) at cfa-12
  DW_CFA_def_cfa_offset: 12
  DW_CFA_nop

