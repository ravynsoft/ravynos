# name: Thumb branch to weak
# as:
# objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince *-*-vxworks

.*: +file format .*arm.*


Disassembly of section .text:

0+000 <Weak>:
   0:	f7ff bffe 	b.w	4 <Strong>
			0: R_ARM_THM_JUMP24	Strong

0+004 <Strong>:
   4:	f7ff bffe 	b.w	0 <Random>
			4: R_ARM_THM_JUMP24	Random
   8:	f7ff bffe 	b.w	0 <Weak>
			8: R_ARM_THM_JUMP24	Weak
