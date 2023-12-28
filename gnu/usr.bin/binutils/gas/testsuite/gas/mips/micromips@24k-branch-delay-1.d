#objdump: -dr
#as: -mfix-24k -32
#name: 24K: Delay slot filling (microMIPS)
#source: 24k-branch-delay-1.s

.*: +file format .*mips.*

Disassembly of section \.text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	3043 0005 	addiu	v0,v1,5
 *[0-9a-f]+:	6a20      	lw	a0,0\(v0\)
 *[0-9a-f]+:	e9a0      	sw	v1,0\(v0\)
 *[0-9a-f]+:	e9a2      	sw	v1,8\(v0\)
 *([0-9a-f]+):	9403 fffe 	beqz	v1,\1 <.*\+0x\1>
			[0-9a-f]+: R_MICROMIPS_PC16_S1	.*
 *[0-9a-f]+:	e9a4      	sw	v1,16\(v0\)
 *[0-9a-f]+:	69a2      	lw	v1,8\(v0\)

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	6aa4      	lw	a1,16\(v0\)
	\.\.\.
