#name: Silence Interrupt NOP Warnings (MSP430X CPU)
#source: nop-int.s
#as: -mY -mcpu=430x
#objdump: -d --prefix-addresses --show-raw-insn
#pass
