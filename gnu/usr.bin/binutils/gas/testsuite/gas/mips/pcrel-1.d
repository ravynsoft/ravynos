#objdump: -dr
#name: Locally-resolvable PC-relative code references

.*:     file format .*

Disassembly of section .text:

0+000000 <func>:
       0:	3c040001 	lui	a0,0x1
       4:	2484800c 	addiu	a0,a0,-32756
	...

0+008010 <foo>:
#pass
