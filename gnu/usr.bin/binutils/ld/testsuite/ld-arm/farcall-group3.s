@ Test to ensure that ARM calls exceeding 32Mb generate stubs,
@ and that stubs are correctly inserted between input sections
@ when one contribution size exceeds the limit.

	.text
	.global bar
	.global _start
_start:
	bl bar
