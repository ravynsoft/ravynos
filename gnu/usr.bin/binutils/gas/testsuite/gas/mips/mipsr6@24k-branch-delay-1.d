#objdump: -dr
#as: -mfix-24k -32
#source: 24k-branch-delay-1.s
#name: 24K: Delay slot filling

.*: +file format .*mips.*

Disassembly of section .text:
0+ <.*>:
   0:	24620005 	addiu	v0,v1,5
   4:	8c440000 	lw	a0,0\(v0\)
   8:	ac430000 	sw	v1,0\(v0\)
   c:	ac430008 	sw	v1,8\(v0\)
  10:	00000000 	nop
  14:	ac430010 	sw	v1,16\(v0\)
  18:	1060ffff 	beqz	v1,18 <.*>
[	]*18: .*R_MIPS_PC16	.L1
  1c:	00000000 	nop
  20:	8c430008 	lw	v1,8\(v0\)

0+24 <.L1>:
  24:	8c450010 	lw	a1,16\(v0\)
	...
