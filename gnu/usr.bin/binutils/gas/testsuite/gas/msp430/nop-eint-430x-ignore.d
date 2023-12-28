#name: Ignore EINT NOP Insertions (MSP430X CPU)
#source: nop-eint.s
#as: -mY -mcpu=430x
#objdump: -d --prefix-addresses --show-raw-insn
#failif
#...
0x0.*nop.*
#...
