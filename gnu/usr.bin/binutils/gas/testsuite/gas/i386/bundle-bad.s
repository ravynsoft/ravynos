	.text

	# Using .bundle_lock without the mode enabled.
	.bundle_lock
	hlt
	.bundle_unlock

	.bundle_align_mode 3

	# This instruction is 9 bytes long, exceeding the 8-byte bundle size.
	lock addl $0xaabbccdd,%fs:0x10(%esi)

	hlt

	# This locked sequence exceeds the bundle size.
	.bundle_lock
	mov $0xaabbccdd,%eax
	mov $0xaabbccdd,%eax
	.bundle_unlock

	# Test changing subsection inside .bundle_lock.
	.text 0
	.bundle_lock
	clc
	.text 1
	cld
	.bundle_unlock

	# Trying to change the setting inside .bundle_lock.
	.bundle_lock
	.bundle_align_mode 0
	.bundle_unlock

	# Spurious .bundle_unlock.
	hlt
	.bundle_unlock

	# End of input with dangling .bundle_lock.
	.bundle_lock
	hlt
