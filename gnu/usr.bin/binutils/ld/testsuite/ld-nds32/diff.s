	.global	_start
	.global	WORD
	.global	HALF
	.global	BYTE
	.global	ULEB128
.text
_start:
	nop
.L0:
	l.w	$r0, WORD
	.zero	122
.L1:
	nop

.section	code, "ax"
FOO:
	ret

.data
WORD:
	.word		.L1-.L0
HALF:
	.half		.L1-.L0
BYTE:
	.byte		.L1-.L0
ULEB128:
	.uleb128	.L1-.L0
ULEB128_2:
	.uleb128	.L1-.L0
	.align 2
PAD:
	.long		0
