#source: tlsldopt.s
#as: -a64
#ld: -melf64ppc
#objdump: -dr
#target: powerpc64*-*-*

.*:     file format .*

Disassembly of section \.text:

.*:
.*	nop
.*	nop
.*	mr      r3,r29
.*	addi    r3,r13,4096
.*	nop
.*	addis   r3,r3,0
.*	ld      r3,-32768\(r3\)
.*	nop
.*	nop
.*	mr      r3,r29
.*	addi    r3,r13,4096
.*	nop
.*	ld      r3,-32768\(r3\)
.*	nop
.*	nop
.*	nop
.*	nop
.*	nop
.*	mr      r3,r29
.*	addi    r3,r13,-28672
.*	nop
.*	ld      r3,0\(r3\)
.*	nop
.*	nop
.*	nop
.*	mr      r3,r29
.*	addi    r3,r13,-28672
.*	nop
.*	ld      r3,0\(r3\)
.*	nop
.*	nop
.*	nop
