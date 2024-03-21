	.intel_syntax
	.data

	.long	0
	.long	+ 1
	.long	- 2
	.long	not 3
	.long	4 + 1
	.long	5 - 2
	.long	6 * 3
	.long	7 / 4
	.long	8 mod 5
	.long	9 shl 6
	.long	10 shr 7
	.long	11 and 8
	.long	12 xor 9
	.long	13 or 10
	.long	14 eq 14
	.long	15 ne 15
	.long	16 le 16
	.long	17 lt 17
	.long	18 ge 18
	.long	19 gt 19

	.p2align 4, 0xcc

	.byte	byte, word, dword, fword, qword, mmword, tbyte
	.byte	oword, xmmword, ymmword, zmmword

	.p2align 4, 0xcc

	.code16
	.word	near, far

	.code32
	.word	near, far

	.code64
	.word	near, far

	.p2align 4, 0xcc
