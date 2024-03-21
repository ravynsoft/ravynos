#objdump: -Wf
#name: CFI common 6
#...
Contents of the .eh_frame section:

00000000 0+0018 0+0000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     03 .. .. .. .. 0c 1b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0000001c 0+0018 0+0020 FDE cie=0+0000 pc=0+00(00|24)..0+00(04|28)
  Augmentation data:     (00 00 00 00 de ad be ef|ef be ad de 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000038 0+0010 0+0000 CIE
  Version:               1
  Augmentation:          "zLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     0c 1b

  DW_CFA_nop

0000004c 0+0018 0+0018 FDE cie=0+0038 pc=0+00(04|58)..0+00(08|5c)
  Augmentation data:     (00 00 00 00 de ad be ef|ef be ad de 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000068 0+0018 0+006c FDE cie=0+0000 pc=0+00(08|78)..0+00(0c|7c)
  Augmentation data:     (00 00 00 00 be ef de ad|ad de ef be 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000084 0+0018 0+0000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     1b .. .. .. .. 1b 1b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

000000a0 0+0014 0+0020 FDE cie=0+0084 pc=0+00(0c|b4)..0+00(10|b8)
  Augmentation data:     .. .. .. ..

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

000000b8 0+0014 0+0038 FDE cie=0+0084 pc=0+00(10|d0)..0+00(14|d4)
  Augmentation data:     .. .. .. ..

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

