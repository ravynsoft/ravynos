	.align	2
	.globl	_pinit
.LFB84:
	.set	nomips16
	.ent	_pinit
_pinit:

	lw	$3,8($23)
	li	$5,1
	lui	$2,%hi(__var1)
	ori	$3,$3,0x1
	lui	$4,%hi(var4)
	sw	$3,8($23)
	addiu	$4,$4,%lo(var4)
	lui	$3,%hi(var5)
	sw	$5,%lo(__var1)($2)
	lui	$19,%hi(hilo_match)
.LVL100:
	lui	$2,%hi(__var3)
	sw	$5,%lo(var5)($3)
	.set	noreorder
	.set	nomacro
	jal	func4
	sw	$5,%lo(__var3)($2)
	.set	macro
	.set	reorder

	lw	$17,%lo(hilo_match)($19)
.LVL101:
	lui	$2,%hi(var6)
	lui	$3,%hi(var6+704)
	addiu	$16,$2,%lo(var6)
.LVL102:
	addiu	$18,$3,%lo(var6+704)
	.set	noreorder
	.set	nomacro
	jal	func3
	sw	$2,%lo(hilo_match)($19)

	.end	_pinit
.LFE84:
	.size	_pinit, .-_pinit
	.align	2
	.globl	pdelt
.LFB120:
	.set	mips16
	.ent	pdelt
pdelt:
	.set	macro
	.set	reorder

	li	$2,16
.L321:
.LVL212:
	j	$31
.LVL213:
.L322:
	lhu	$2,36($17)
	move	$4,$16
	li	$16,%hi(var2)
	sll	$16,$16,8
	addiu	$2,1
	sll	$16,$16,8
	addiu	$16,%lo(var2)
	.set	noreorder
	.set	nomacro
	jal	func1
	sh	$2,36($17)
	.set	macro
	.set	reorder

	.set	noreorder
	.set	nomacro
	jal	func2
	move	$4,$16
	.set	macro
	.set	reorder

	li	$3,%hi(hilo_match)
	sll	$3,$3,8
	sll	$3,$3,8
	lw	$2,%lo(hilo_match)($3)
	sw	$2,0($17)
	sw	$17,%lo(hilo_match)($3)
	.set	noreorder
	.set	nomacro
	jal	func1
	move	$4,$16
	.set	macro
	.set	reorder

.LVL214:
	.set	noreorder
	.set	nomacro
	j	$31
	li	$2,0
	.set	macro
	.set	reorder

	.end	pdelt
	.align	2
	.weak	__var3
	.section	.sbss,"aw",@nobits
	.align	2
	.type	__var3, @object
	.size	__var3, 4
__var3:
	.space	4
	.weak	__var1
	.align	2
	.type	__var1, @object
	.size	__var1, 4
__var1:
	.space	4
	.data
	.align	2
	.weak	__hilo_match
	.align	2
	.type	__hilo_match, @object
	.size	__hilo_match, 4
__hilo_match:
	.space	4
	.data
	.align	2
	.align	2
	.type	var2, @object
	.size	var2, 32
var2:
	.word	0
	.word	-1
	.word	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.word	0
	.word	0
	.half	0
	.space	6
	.align	2
	.rdata
	.align	2
	.space	8
	.local	var5
	.comm	var5,4,4
	.align	2
	.local	var6
	.comm	var6,704,4
