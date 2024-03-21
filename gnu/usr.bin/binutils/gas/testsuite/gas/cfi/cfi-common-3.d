#objdump: -Wf
#name: CFI common 3
#...
Contents of the .eh_frame section:

00000000 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     [01][abc]
#...
00000014 0+001[08] 0+0018 FDE cie=00000000 pc=.*
  DW_CFA_advance_loc: 4 to .*
  DW_CFA_remember_state
  DW_CFA_restore_state
#pass
