# name: Disassembling variable width insns with relocs (PR 24907)
# as:
# objdump: -d
# This test is only valid on ELF based ports.
#notarget: *-*-pe *-*-wince *-*-vxworks

.*: +file format .*arm.*

Disassembly of section \.text:

0+000 <foo>:
   0:	46c0      	nop			@ .*
   2:	f7ff fffe 	bl	0 <log_func>
   6:	e002      	b\.n	e <func\+0x2>
   8:	f7ff fffe 	bl	c <func>

0+000c <func>:
   c:	46c0      	nop			@ .*
   e:	46c0      	nop			@ .*
