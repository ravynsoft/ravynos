	.text
	.globl	foo
	.globl  global_tls
	.ent	foo
foo:
$LCL:
	lui	$2, %hi($LCL)
	addiu	$2, %lo($LCL)
	lui	$2, %hi(bar)
	addiu	$2, %lo(bar)
	lui	$a0,%tprel_hi(local_tls)
	addiu	$a0,$a0,%tprel_lo(local_tls)
	lui	$a0,%tprel_hi(global_tls)
	addiu	$a0,$a0,%tprel_lo(global_tls)
	.end	foo
	.type	local_tls,%object
	.type	global_tls,%object
	.section	.tbss,"awT",@nobits
local_tls:
	.word
global_tls:
	.word
