	.globl	_start
	.type	_start,@function
	.text
_start:
	.cfi_startproc
0:
 addis 2,12,(.TOC.-0b)@ha
 addi 2,2,(.TOC.-0b)@l
	.localentry _start,.-0b
 mflr 0
 std 0,16(1)
 stdu 1,-32(1)
	.cfi_def_cfa_offset 32
	.cfi_offset 65, 16

	.macro call f
 bl \f
 nop
	.endm

# 3000 plt calls, giving over 64k in .plt size.  With a small .got
# this guarantees some plt call stubs can use a 16-bit signed offset
# from .TOC. while others need a 32-bit signed offset (and are larger).

	.irpc t4,012
	.irpc t3,0123456789
	.irpc t2,0123456789
	.irpc t1,0123456789
	.if \t4
	call f\t4\t3\t2\t1
	.elseif \t3
	call f\t3\t2\t1
	.elseif \t2
	call f\t2\t1
	.else
	call f\t1
	.endif
	.endr
	.endr
	.endr
	.endr

 addi 1,1,32
	.cfi_def_cfa_offset 0
 ld 0,16(1)
 mtlr 0
	.cfi_restore 65
 blr
	.cfi_endproc
	.size _start,.-_start

# Padding to trigger a stub sizing error with commit 2f83249c13 and
# c804c6f98d (relro changes).  This particular testcase gives a decrease
# in .got to .plt gap after .eh_frame editing, resulting in some plt
# call stubs being smaller.  If the very last one is smaller the size
# error triggers.  Arguably, the ppc64 backend should not report an
# error for shrinkage.  However, the actual PR object files showed an
# *increase* in .got to .plt gap after .eh_frame editing, resulting in
# some plt call stubs being larger.  That hit an assertion failure
# when a long branch stub followed the larger plt call stub and
# overwrote the end of the plt call stub.
# With enough fiddling of this testcase it likely would be possible to
# find the right padding here and .eh_frame sizing to trigger an
# increase in .got to .plt gap.  The point of this testcase is to show
# that the .got to .plt gap should not change after sizing.
	.space 50000

# Generate some .eh_frame info that -gc-sections will trim
	.macro fundef f
	.section .text.\f,"ax",@progbits
	.type	\f,@function
\f:
	.cfi_startproc
 blr
	.cfi_endproc
	.size \f,.-\f
	.endm

	.irpc t2,0123456789
	.irpc t1,0123456789
	fundef dummy\t2\t1
	.endr
	.endr
