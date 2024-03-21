// tbz.s Test file for AArch64 tbz.

	.text

	tbz	x0, #1, bar + 0x8000
