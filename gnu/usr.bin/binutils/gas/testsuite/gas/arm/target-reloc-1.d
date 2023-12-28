#objdump: -dr --show-raw-insn
#skip: *-*-pe *-*-wince *-*-vxworks
#name: TARGET reloc

.*:     file format .*arm.*

Disassembly of section .text:

00000000 <foo>:
   0:	00001234 	.*
			0: R_ARM_TARGET2	foo
   4:	cdef0000 	.*
			4: R_ARM_TARGET2	foo
   8:	76543210 	.*
			8: R_ARM_TARGET2	foo
