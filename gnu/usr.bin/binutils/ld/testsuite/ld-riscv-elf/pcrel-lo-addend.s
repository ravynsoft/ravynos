  .text
  .globl _start
_start:
  auipc ra, %pcrel_hi(tdata)
  addi ra, ra, %pcrel_lo(.text)
  lb t1, 0(ra)
foo:
  auipc ra, %pcrel_hi(tdata)
  addi ra, ra, %pcrel_lo(.text+12)
  lb t2, 1(ra)

  .data
tdata:
  .byte 0xff
  .byte 0x00
  .byte 0xf0
  .byte 0x0f
