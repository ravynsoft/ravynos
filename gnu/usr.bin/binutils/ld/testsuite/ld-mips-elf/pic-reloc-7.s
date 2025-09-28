	.text
	.globl	foo
	.ent	foo
foo:
$LCL:
	lui	$2, %highest($LCL)
	addiu	$2, %higher($LCL)
	lui	$2, %highest(bar)
	addiu	$2, %higher(bar)
	.end	foo
