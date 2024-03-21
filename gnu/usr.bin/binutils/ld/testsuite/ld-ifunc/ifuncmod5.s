	.file	"ifuncmod5.c"

	.text
	.type ifuncmod5.c, STT_NOTYPE
ifuncmod5.c:
	.size ifuncmod5.c, 0

	.pushsection .gnu.build.attributes, "", %note
	.balign 4
	.dc.l 8 	
	.dc.l 16	
	.dc.l 0x100	
	.asciz "GA$3p4"	
	.dc.a ifuncmod5.c
	.dc.a ifuncmod5.c_end	
	.popsection

.Ltext0:
#APP
	.protected global
	.type foo, %gnu_indirect_function
	.type foo_hidden, %gnu_indirect_function
	.type foo_protected, %gnu_indirect_function
	.hidden foo_hidden
	.protected foo_protected
#NO_APP
	.align	8
	.type	one, %function
one:
	.dc.l 0
	.size	one, .-one
	.align	8

.globl foo
	.type	foo, %function
foo:
	.dc.l	0
	.size	foo, .-foo

	.pushsection .gnu.build.attributes
	.dc.l 6		
	.dc.l 16	
	.dc.l 0x101	
	.dc.b 0x47, 0x41, 0x2a, 0x2, 0, 0 	
	.dc.b 0, 0 	
	.dc.a foo
	.dc.a foo_end	
	.popsection

foo_end:
	.align	8
.globl foo_hidden
	.type	foo_hidden, %function
foo_hidden:
	.dc.l	0
	.size	foo_hidden, .-foo_hidden

	.pushsection .gnu.build.attributes
	.dc.l 6		
	.dc.l 16	
	.dc.l 0x101	
	.dc.b 0x47, 0x41, 0x2a, 0x2, 0, 0 	
	.dc.b 0, 0 	
	.dc.a foo_hidden
	.dc.a foo_hidden_end	
	.popsection

foo_hidden_end:
	.align	8

	.globl foo_protected
	.type	foo_protected, %function
foo_protected:
	.dc.l	0

	.size	foo_protected, .-foo_protected

	.pushsection .gnu.build.attributes
	.dc.l 6		
	.dc.l 16	
	.dc.l 0x101	
	.dc.b 0x47, 0x41, 0x2a, 0x2, 0, 0 	
	.dc.b 0, 0 	
	.dc.a foo_protected
	.dc.a foo_protected_end	
	.popsection

foo_protected_end:
	.globl global

	.data
	.align	4
	.type	global, %object
	.size	global, 4
global:
	.long	-1

	.text
	.Letext0:

ifuncmod5.c_end:
	.type ifuncmod5.c_end, STT_NOTYPE
	.size ifuncmod5.c_end, 0


