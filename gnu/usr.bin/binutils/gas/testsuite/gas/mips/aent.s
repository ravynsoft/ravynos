	.text

	.globl	foo
	.ent	foo
foo:
	sllv	$2, $4, $6
	srav	$8, $10, $12

	.globl	bar
	.aent	bar
bar:
	sllv	$2, $4, $6
	srav	$8, $10, $12

	.end	foo
	.size	foo, . - foo

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space  8
