#objdump: -Wf
#name: CFI common 8
#...
Contents of the .eh_frame section:

00000000 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01][abc]

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000014 0+00(18|1c|20) 0+0018 FDE cie=0+0000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_def_cfa: r0( \([er]ax\)|) ofs 16
  DW_CFA_val_offset(_sf|): r1( \((rdx|ecx)\)|) is cfa\+8
  DW_CFA_val_offset(_sf|): r2( \((rcx|edx)\)|) is cfa-32
#...
