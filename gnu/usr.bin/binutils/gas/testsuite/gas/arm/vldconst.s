@       Test file for ARM/GAS -- vldr reg, =... expressions.
.fpu neon
.text
.align
foo:
	# test both low and high index of the
	# Advanced SIMD and Floating-point reg.
	.macro vlxr regtype const
	.irp regindex, 0, 14, 28, 31
		vldr \regtype\regindex, \const
	.endr
	.endm

	.macro vlxreq regtype const
	.irp regindex, 0, 14, 28, 31
		vldreq \regtype\regindex, \const
	.endr
	.endm

	.macro vlxrmi regtype const
	.irp regindex, 0, 14, 28, 31
		vldrmi \regtype\regindex, \const
	.endr
	.endm

	vlxr	s "=0"
	vlxr	s "=0xff000000"
	vlxr	s "=-1"
	vlxr	s "=0x0fff0000"
	.pool

	vlxr	s "=0"
	vlxr	s "=0x00ff0000"
	vlxr	s "=0xff00ffff"
	vlxr	s "=0x00fff000"
	.pool

	vlxreq	s "=0"
	vlxreq	s "=0x0000ff00"
	vlxreq	s "=0xffff00ff"
	vlxreq	s "=0x000fff00"
	.pool

	vlxrmi	s "=0"
	vlxrmi	s "=0x000000ff"
	vlxrmi	s "=0xffffff00"
	vlxrmi	s "=0x0000fff0"
	.pool

	vlxr	d "=0"
	vlxr	d "=0xca000000"
	vlxr	d "=-1"
	vlxr	d "=0x0fff0000"
	.pool

	vlxr	d "=0"
	vlxr	d "=0x00ff0000"
	vlxr	d "=0xff0000ff"
	vlxr	d "=0x00fff000"
	.pool

	vlxreq	d "=0"
	vlxreq	d "=0x0000ff00"
	vlxreq	d "=0xffff00ff"
	vlxreq	d "=0x000fff00"
	.pool

	vlxrmi	d "=0"
	vlxrmi	d "=0x000000ff"
	vlxrmi	d "=0xffffff00"
	vlxrmi	d "=0x0000ffff"
	.pool

	vlxr	d "=0"
	vlxr	d "=0xff00000000000000"
	vlxr	d "=-1"
	vlxr	d "=0x0fff000000000000"
	.pool

	vlxr	d "=0"
	vlxr	d "=0x00ff00000000000"
	vlxr	d "=0xff00ffff0000000"
	vlxr	d "=0x00fff0000000000"
	.pool

	vlxreq	d "=0"
	vlxreq	d "=0x0000ff0000000000"
	vlxreq	d "=0xffff00ff00000000"
	vlxreq	d "=0x000fff0000000000"
	.pool

	vlxrmi	d "=0"
	vlxrmi	d "=0x000000ff00000000"
	vlxrmi	d "=0xffffff0000000000"
	vlxrmi	d "=0x0000fff000000000"
	.pool

	# pool should be aligned to 8-byte.
	.p2align 3
	vldr	d1, =0x0000fff000000000
	.pool

	# no error when code is align already.
	.p2align 3
	add	r0, r1, #0
	vldr	d1, =0x0000fff000000000
	.pool

	.p2align 3
	vldr	d1, =0x0000fff000000000
	vldr	s2, =0xff000000
	# padding A
	vldr	d3, =0x0000fff000000001
	# reuse padding slot A
	vldr	s4, =0xff000001
	# reuse d3
	vldr	d5, =0x0000fff000000001
	# new 8-byte entry
	vldr	d6, =0x0000fff000000002
	# new 8-byte entry
	vldr	d7, =0x0000fff000000003
	# new 4-byte entry
	vldr	s8, =0xff000002
	# padding B
	vldr	d9, =0x0000fff000000004
	# reuse padding slot B
	vldr	s10, =0xff000003
	# new 8-byte entry
	vldr	d11, =0x0000fff000000005
	# new 4 entry
	vldr	s12, =0xff000004
	# new 4 entry
	vldr	s13, =0xff000005
	# reuse value of s4 in pool
	vldr	s14, =0xff000001
	# reuse high part of d1 in pool
	vldr	s15, =0x0000fff0
	# 8-byte entry reuse two 4-byte entries.
	# this reuse should only happen for
	# little-endian
	# d16 reuse s12, s13
	vldr	d16, =0xff000005ff000004
	# d17 should not reuse high part of d11 and s12.
	# because the it's align 8-byte aligned.
	vldr	d17, =0xff0000040000fff0
	.pool
