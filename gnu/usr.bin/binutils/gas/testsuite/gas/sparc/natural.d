#as: -Av9 -64
#objdump: -dr --prefix-addresses
#name: sparc natural regs and insns

.*: +file format .*

Disassembly of section .text:
0x0+0000000 b  %xcc, 0x0+0000030
0x0+0000004 nop 
0x0+0000008 ldx  \[ %g1 \], %g2
0x0+000000c ldxa  \[ %g1 \] #ASI_AIUP, %g2
0x0+0000010 stx  %g1, \[ %g2 \]
0x0+0000014 stxa  %g1, \[ %g2 \] #ASI_AIUP
0x0+0000018 sllx  %g1, 0xa, %g2
0x0+000001c srlx  %g1, 0xa, %g2
0x0+0000020 srax  %g1, 0xa, %g2
0x0+0000024 casx  \[ %g1 \], %g2, %g3
0x0+0000028 casxa  \[ %g1 \] #ASI_AIUP, %g2, %g3
0x0+000002c clrx  \[ %g1 \]
0x0+0000030 nop 
