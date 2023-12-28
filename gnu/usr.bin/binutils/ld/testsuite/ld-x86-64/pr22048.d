#source: pr22048a.s
#source: pr22048b.s
#as: --64
#ld: -melf_x86_64 -Ttext 0x400078
#readelf: -wf

Contents of the .eh_frame section:

0+0000 0+014 0+0000 CIE
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

0+0018 0+0010 0+001c FDE cie=0+0000 pc=0+400078\.\.0+40007a
  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop
