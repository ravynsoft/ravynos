	.arch armv7-a
	.text
	# PR 20827
	add r5, r4, lsl r0
	add r5, r5, r4, lsl r0
