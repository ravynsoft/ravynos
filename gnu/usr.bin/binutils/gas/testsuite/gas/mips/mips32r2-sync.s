# Source file to test assembly of MIPS32r2 sync instructions.

	.text
foo:
	sync
	sync	2
	sync_wmb
	sync	8
	sync_mb
	sync_acquire
	sync_release
	sync_rmb
	sync	0x18
	sync	0
	sync	2
	sync	4
	sync	8
	sync	0x10
	sync	0x11
	sync	0x12
	sync	0x13
	sync	0x18

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space  8
