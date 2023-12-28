#source: ../../../cfi/cfi-common-6.s
#readelf: -wf
#name: CFI common 6
Contents of the .eh_frame section:

00000000 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     03 .. .. .. .. 0c 1b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

0000001c 00000018 00000020 FDE cie=00000000 pc=000000(00|24)..000000(04|28)
  Augmentation data:     (00 00 00 00 de ad be ef|ef be ad de 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000038 00000010 00000000 CIE
  Version:               1
  Augmentation:          "zLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     0c 1b

  DW_CFA_nop

0000004c 00000018 00000018 FDE cie=00000038 pc=000000(04|58)..000000(08|5c)
  Augmentation data:     (00 00 00 00 de ad be ef|ef be ad de 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000068 00000018 0000006c FDE cie=00000000 pc=000000(08|78)..000000(0c|7c)
  Augmentation data:     (00 00 00 00 be ef de ad|ad de ef be 00 00 00 00)

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

00000084 00000018 00000000 CIE
  Version:               1
  Augmentation:          "zPLR"
  Code alignment factor: .*
  Data alignment factor: .*
  Return address column: .*
  Augmentation data:     1b .. .. .. .. 1b 1b

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

000000a0 00000014 00000020 FDE cie=00000084 pc=000000(0c|b4)..000000(10|b8)
  Augmentation data:     .. .. .. ..

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

000000b8 00000014 00000038 FDE cie=00000084 pc=000000(10|d0)..000000(14|d4)
  Augmentation data:     .. .. .. ..

  DW_CFA_nop
  DW_CFA_nop
  DW_CFA_nop

