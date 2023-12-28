	.text
	.set	noreorder
	.space	0x1000

	.align	4
	.set	mips16
	.ent	foo
foo:
	not	$2, $2
	jalx	bar0
	 not	$2, $2
	jal	bar0
	 not	$2, $2
	jalx	bar1
	 not	$2, $2
	jal	bar1
	 not	$2, $2
	jalx	bar2
	 not	$2, $2
	jal	bar2
	 not	$2, $2
	jalx	bar3
	 not	$2, $2
	jal	bar3
	 not	$2, $2
	jalx	bar4
	 not	$2, $2
	jal	bar4
	 not	$2, $2
	jalx	bar4 + 1
	 not	$2, $2
	jal	bar4 + 1
	 not	$2, $2
	jalx	bar4 + 2
	 not	$2, $2
	jal	bar4 + 2
	 not	$2, $2
	jalx	bar4 + 3
	 not	$2, $2
	jal	bar4 + 3
	 not	$2, $2
	jalx	bar4 + 4
	 not	$2, $2
	jal	bar4 + 4
	 not	$2, $2
	jalx	bar16
	 not	$2, $2
	jal	bar16
	 not	$2, $2
	jalx	bar17
	 not	$2, $2
	jal	bar17
	 not	$2, $2
	jalx	bar18
	 not	$2, $2
	jal	bar18
	 not	$2, $2
	jalx	bar18 + 1
	 not	$2, $2
	jal	bar18 + 1
	 not	$2, $2
	jalx	bar18 + 2
	 not	$2, $2
	jal	bar18 + 2
	 not	$2, $2
	jalx	bar18 + 3
	 not	$2, $2
	jal	bar18 + 3
	 not	$2, $2
	jalx	bar18 + 4
	 not	$2, $2
	jal	bar18 + 4
	 not	$2, $2
	jr	$ra
	 not	$2, $2
	.end	foo

# Force some (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	4, 0
	.space	16

	.macro	obj n:req
	.globl	bar\@
	.type	bar\@, @object
bar\@ :
	.byte	0
	.size	bar\@, . - bar\@
	.if	\n - 1
	obj	\n - 1
	.endif
	.endm

	.macro	fun n:req
	.globl	bar\@
	.type	bar\@, @function
bar\@ :
	.insn
	.hword	0
	.size	bar\@, . - bar\@
	.if	\n - 1
	fun	\n - 1
	.endif
	.endm

	.align	4
	obj	16
	fun	8
