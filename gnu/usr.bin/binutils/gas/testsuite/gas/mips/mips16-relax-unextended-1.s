	.module		mips3
	.set		mips16
	.set		autoextend
foo:
						# Operand code:
	sll.t		$2, $3, 17		# <
	sll.t		$2, $3, bar
	dsll.t		$4, $5, 17		# [
	dsll.t		$4, $5, bar
	dsrl.t		$6, 17			# ]
	dsrl.t		$6, bar

	lb.t		$4, 0x1234($5)		# 5
	lb.t		$4, bar($5)
	lb.t		$4, %hi(baz)($5)
	slti.t		$6, 0x5678		# 8
	slti.t		$6, bar

	la.t		$2, . + 0x1234		# A
	la.t		$2, . + bar
	ld.t		$3, . + 0x5678		# B
	ld.t		$3, . + bar
	sd.t		$31, 0x5678($29)	# C
	sd.t		$31, bar($29)
	sd.t		$31, %lo(baz)($29)
	sd.t		$4, 0x5678($29)		# D
	sd.t		$4, bar($29)
	sd.t		$4, %lo(baz)($29)
	dla.t		$5, . + 0x5678		# E
	dla.t		$5, . + bar
	daddiu.t	$2, $3, 0x5678		# F
	daddiu.t	$2, $3, bar
	lh.t		$6, 0x1234($7)		# H
	lh.t		$6, bar($7)
	lh.t		$6, %lo(baz)($7)
	addiu.t		$29, 0x5678		# K
	addiu.t		$29, bar
	addiu.t		$29, %lo(baz)
	cmpi.t		$2, 0x1234		# U
	cmpi.t		$2, bar
	cmpi.t		$2, %hi(baz)
	addiu.t		$3, $pc, 0x5678		# V
	addiu.t		$3, $pc, bar
	addiu.t		$3, $pc, %lo(baz)
	daddiu.t	$4, $pc, 0x5678		# W
	daddiu.t	$4, $pc, bar
	daddiu.t	$4, $pc, %lo(baz)

	daddiu.t	$5, 0x5678		# j
	daddiu.t	$5, bar
	daddiu.t	$5, %lo(baz)
	addiu.t		$6, 0x1234		# k
	addiu.t		$6, bar
	addiu.t		$2, %lo(baz)
	beqz.t		$7, . + 0x5678		# p
	b.t		. + 0x1234		# q

	.set		bar, 0x5678
