# Source file used to test the li macro.

foo:
	# Both words zero
	.set mips1
	.set fp=32
	li.d $2, 0
	li.d $f0, 0
	.set mips2
	li.d $f0, 0
	.set fp=xx
	li.d $f0, 0
	.set mips32r2
	.set fp=32
	li.d $f0, 0
	.set fp=xx
	li.d $f0, 0
	.set fp=64
	li.d $f0, 0
	.set mips3
	li.d $f0, 0

	# Only upper 16 bits of 64 non-zero
	.set mips1
	.set fp=32
	li.d $2, 1.0
	li.d $f0, 1.0
	.set mips2
	li.d $f0, 1.0
	.set fp=xx
	li.d $f0, 1.0
	.set mips32r2
	.set fp=32
	li.d $f0, 1.0
	.set fp=xx
	li.d $f0, 1.0
	.set fp=64
	li.d $f0, 1.0
	.set mips3
	li.d $f0, 1.0

	# Only lower 16 bits of 64 non-zero
	.set mips1
	.set fp=32
	li.d $2, 2.1e-320
	li.d $f0, 2.1e-320
	.set mips2
	li.d $f0, 2.1e-320
	.set fp=xx
	li.d $f0, 2.1e-320
	.set mips32r2
	.set fp=32
	li.d $f0, 2.1e-320
	.set fp=xx
	li.d $f0, 2.1e-320
	.set fp=64
	li.d $f0, 2.1e-320
	.set mips3
	li.d $f0, 2.1e-320

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  2
	.space  8
