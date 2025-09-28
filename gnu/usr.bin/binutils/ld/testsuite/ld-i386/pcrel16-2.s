	.text
	.global _start
_start:
	data16 xbegin t16
	ret

	.fill 0x8000,1,0xcc

	.global t16
t16:
	data16 xbegin _start
	ret
