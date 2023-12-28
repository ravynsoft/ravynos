	.macro	start, sym
	.type	\sym, @function
	.pushsection .opd, "aw"
\sym:	.quad	.L.\sym, .TOC.@tocbase, 0
	.popsection
.L.\sym:
	mflr	0
	std	31, -8(1)
	std	0, 16(1)
	stdu	1, -128(1)
	mr	31, 1
	.endm


	.macro	end, sym
	addi	1,31,128
	ld	0, 16(1)
	mtlr	0
	ld	31, -8(1)
	blr
	.size	\sym, . - .L.\sym
	.endm


	.macro	forward, from, to
	start	\from
	bl	\to
	nop
	end	\from
	.endm


	.macro	usegot, sym
	.pushsection .data
\sym\@:	.quad	\@
	.popsection
	.pushsection .toc, "aw"
.LT\@:	.tc	\sym\@[TC], \sym\@
	.popsection
	ld	3,.LT\@@toc(2)
	.endm


	.macro	in123
	.pushsection .toc, "aw"
.LThello:
	.tc	hello[TC],hello
	.popsection

	.pushsection .rodata
hello:	.asciz	"Hello, world!\n"
	.popsection

	.pushsection .text.in123, "axG", @progbits, in123, comdat
	.weak	in123
	start	in123
	ld	3, .LThello@toc(2)
	#bl	printf
	nop
	end	in123
	.popsection
	.endm


	.macro	in23
	.pushsection .text
	forward local, in123
	.popsection

	.pushsection .text.in23, "axG", @progbits, in23, comdat
	.weak	in23
	forward	in23, local
	.popsection
	.endm


	.macro	gobblegot, sym
	.pushsection .text
	.globl	\sym
	start	\sym
	.rept	5000
	usegot	a
	.endr
	end	\sym
	.popsection
	.endm
