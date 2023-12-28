.text
  nop
  nop
L1:
  nop
.data
  nop
  nop
  .8byte 0x1234567812345678
  .reloc 0,R_LARCH_64_PCREL,L1
  nop
