#name: Unknown Interrupt State NOP Insertions (MSP430 CPU)
#source: nop-unknown-intr.s
#as: -my -mu -mcpu=430
#warning_output: nop-unknown-intr-430.l
#objdump: -d --prefix-addresses --show-raw-insn

.*: +file format .*msp.*


Disassembly of section .text:
0x0+0000 12 42 00 00[ 	]+mov[ 	]+&0x0000,r2[ 	]+;0x0000
0x0+0004 1a 42 00 00[ 	]+mov[ 	]+&0x0000,r10[ 	]+;0x0000
0x0+0008 02 47[ 	]+mov[ 	]+r7,[ 	]+r2[ 	]+;
0x0+000a 1a 42 00 00[ 	]+mov[ 	]+&0x0000,r10[ 	]+;0x0000
