	.text
	.globl	foo
	.globl  global_tls
	.ent	foo
foo:
$LCL:
	li	$2, %hi($LCL)
	addiu	$2, %lo($LCL)
	li	$2, %hi(bar)
	addiu	$2, %lo(bar)
	li	$a0,%tprel_hi(local_tls)
	lw	$a1,%tprel_lo(local_tls)($a0)
	li	$a0,%tprel_hi(global_tls)
	lw	$a1,%tprel_lo(global_tls)($a0)
	.end	foo
	.type	local_tls,%object
	.type	global_tls,%object
	.section	.tbss,"awT",@nobits
local_tls:
	.word
global_tls:
	.word
