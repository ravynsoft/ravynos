#name: c.lui to c.li relaxation
#source: c-lui-2.s
#as: -march=rv32ic
#ld: -m[riscv_choose_ilp32_emul] -Tc-lui-2.ld
#objdump: -d -M no-aliases,numeric

.*:     file format .*


Disassembly of section \.text:

.* <_start>:
.*:	4501                	c.li	x10,0
.*:	7fe00513          	addi	x10,x0,2046
	...

.* <foo>:
.*:	8082                	c.jr	x1
#pass
