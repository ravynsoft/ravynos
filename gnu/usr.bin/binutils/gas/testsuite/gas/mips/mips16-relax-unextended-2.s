	.module		mips3
	.set		mips16
	.set		noautoextend
foo:
						# Operand code:
	sll		$2, $3, 17		# <
	sll		$2, $3, bar
	dsll		$4, $5, 17		# [
	dsll		$4, $5, bar
	dsrl		$6, 17			# ]
	dsrl		$6, bar

	lb		$4, 0x1234($5)		# 5
	lb		$4, bar($5)
	lb		$4, %hi(baz)($5)
	slti		$6, 0x5678		# 8
	slti		$6, bar

	la		$2, . + 0x1234		# A
	la		$2, . + bar
	ld		$3, . + 0x5678		# B
	ld		$3, . + bar
	sd		$31, 0x5678($29)	# C
	sd		$31, bar($29)
	sd		$31, %lo(baz)($29)
	sd		$4, 0x5678($29)		# D
	sd		$4, bar($29)
	sd		$4, %lo(baz)($29)
	dla		$5, . + 0x5678		# E
	dla		$5, . + bar
	daddiu		$2, $3, 0x5678		# F
	daddiu		$2, $3, bar
	lh		$6, 0x1234($7)		# H
	lh		$6, bar($7)
	lh		$6, %lo(baz)($7)
	addiu		$29, 0x5678		# K
	addiu		$29, bar
	addiu		$29, %lo(baz)
	cmpi		$2, 0x1234		# U
	cmpi		$2, bar
	cmpi		$2, %hi(baz)
	addiu		$3, $pc, 0x5678		# V
	addiu		$3, $pc, bar
	addiu		$3, $pc, %lo(baz)
	daddiu		$4, $pc, 0x5678		# W
	daddiu		$4, $pc, bar
	daddiu		$4, $pc, %lo(baz)

	daddiu		$5, 0x5678		# j
	daddiu		$5, bar
	daddiu		$5, %lo(baz)
	addiu		$6, 0x1234		# k
	addiu		$6, bar
	addiu		$2, %lo(baz)
	beqz		$7, . + 0x5678		# p
	b		. + 0x1234		# q

	.set		bar, 0x5678
