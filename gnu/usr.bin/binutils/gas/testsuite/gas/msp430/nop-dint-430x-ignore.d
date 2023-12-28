#name: Ignore DINT NOP Insertions (MSP430X CPU)
#source: nop-dint.s
#as: -mY -mcpu=430x
#objdump: -d --prefix-addresses --show-raw-insn
#failif
#...
0x0.*nop.*
#...
