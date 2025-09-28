#name: Thumb-2 branch to constant address
#This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince
#objdump: -rd


.*: +file format.*arm.*


Disassembly of section .text:

00000000 <foo>:
   0:	f... b... 	b\.w	.*
			0: R_ARM_THM_JUMP24	\*ABS\*.*
