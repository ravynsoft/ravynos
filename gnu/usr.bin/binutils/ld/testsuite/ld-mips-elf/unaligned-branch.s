	.text
	.align	4
	.globl	foo
	.ent	foo
foo:
	b	bar0
	beqzc	$2, bar0
	bc	bar0
	b	bar1
	beqzc	$2, bar1
	bc	bar1
	b	bar2
	beqzc	$2, bar2
	bc	bar2
	b	bar3
	beqzc	$2, bar3
	bc	bar3
	b	bar4
	beqzc	$2, bar4
	bc	bar4
	.end	foo
