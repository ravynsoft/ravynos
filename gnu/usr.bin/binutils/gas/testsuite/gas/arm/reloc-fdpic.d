#as: --fdpic
#objdump: -dr --show-raw-insn
#name: FDPIC relocations
# This test is only valid on ELF based ports.
#noskip: arm*-*-uclinuxfdpiceabi

.*: +file format .*arm.*


Disassembly of section .text:

00000000 <myfunc-0xc>:
.*
			0: R_ARM_GOTFUNCDESC	myfunc
			4: R_ARM_GOTOFFFUNCDESC	myfunc
			8: R_ARM_FUNCDESC	myfunc

0000000c <myfunc>:
   c:	e12fff1e 	bx	lr
.*