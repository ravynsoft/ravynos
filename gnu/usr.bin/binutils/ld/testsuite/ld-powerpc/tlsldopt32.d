#source: tlsldopt32.s
#as: -a32
#ld: -melf32ppc
#objdump: -dr
#target: powerpc*-*-*

.*:     file format .*

Disassembly of section \.text:

.*:
.*	nop
.*	nop
.*	mr      r3,r29
.*	addi    r3,r2,4096
.*	addis   r3,r3,0
.*	lwz     r3,-32768\(r3\)
.*	nop
.*	nop
.*	nop
.*	mr      r3,r29
.*	addi    r3,r2,4096
.*	lwz     r3,-32768\(r3\)
.*	nop
.*	nop
.*	nop
.*	nop
.*	nop
.*	nop
.*	mr      r3,r29
.*	addi    r3,r2,-28672
.*	lwz     r3,0\(r3\)
.*	nop
.*	nop
.*	nop
.*	nop
.*	mr      r3,r29
.*	addi    r3,r2,-28672
.*	lwz     r3,0\(r3\)
.*	nop
.*	nop
.*	nop
.*	nop
#pass
