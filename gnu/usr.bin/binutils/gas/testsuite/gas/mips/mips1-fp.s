# Source file used to test -mips1 fp instructions.

# This is not a complete list of mips1 FP instructions.

foo:
	add.s	$f0,$f2,$f4
	mfc1	$2,$f0
