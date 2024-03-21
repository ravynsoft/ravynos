#as: -Av9 -32
#objdump: -dr --prefix-addresses
#name: sparc natural regs and insns

.*: +file format .*

Disassembly of section .text:
0x00000000 b  %icc, 0x00000030
0x00000004 nop 
0x00000008 ld  \[ %g1 \], %g2
0x0000000c lda  \[ %g1 \] #ASI_AIUP, %g2
0x00000010 st  %g1, \[ %g2 \]
0x00000014 sta  %g1, \[ %g2 \] #ASI_AIUP
0x00000018 sll  %g1, 0xa, %g2
0x0000001c srl  %g1, 0xa, %g2
0x00000020 sra  %g1, 0xa, %g2
0x00000024 cas  \[ %g1 \], %g2, %g3
0x00000028 casa  \[ %g1 \] #ASI_AIUP, %g2, %g3
0x0000002c clr  \[ %g1 \]
0x00000030 nop 
