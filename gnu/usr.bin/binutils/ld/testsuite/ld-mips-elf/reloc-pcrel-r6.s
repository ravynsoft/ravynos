	.text
	.globl test
	.globl x1
	.globl x2
	.globl x3
	.globl x4
	.ent test
test:
1:
	auipc	$a0,%pcrel_hi(x1)
2:
	addiu	$a0,$a0,%pcrel_lo(x1+(2b-1b))
1:
	auipc	$a0,%pcrel_hi(x2)
2:
	addiu	$a0,$a0, %pcrel_lo(x2+(2b-1b))
1:
	auipc	$a0,%pcrel_hi(x3)
2:
	addiu	$a0,$a0,%pcrel_lo(x3+(2b-1b))
1:
	auipc	$a0,%pcrel_hi(x4)
2:
	addiu	$a0,$a0,%pcrel_lo(x4+(2b-1b))
	.end test
	.align	2, 0
	.space	8
