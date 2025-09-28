#name: Ignore Unknown Interrupt State NOP Insertions (MSP430X CPU)
#source: nop-unknown-intr.s
#as: -my -mU -mcpu=430x
#objdump: -d --prefix-addresses --show-raw-insn

.*: +file format .*msp.*


Disassembly of section .text:
0x0+0000 12 42 00 00[ 	]+mov[ 	]+&0x0000,r2[ 	]+;0x0000
0x0+0004 1a 42 00 00[ 	]+mov[ 	]+&0x0000,r10[ 	]+;0x0000
0x0+0008 02 47[ 	]+mov[ 	]+r7,[ 	]+r2[ 	]+;
0x0+000a 1a 42 00 00[ 	]+mov[ 	]+&0x0000,r10[ 	]+;0x0000
