	.text
	.intel_syntax noprefix
.ifdef x86_16
	.code16
.endif

.ifdef x86_64
 .equ adi, rdi
 .equ asi, rsi
.else
 .equ adi, di
 .equ asi, si
.endif

movs:
	movsb

	movsb	es:[edi], [esi]
	movsb	es:[edi], fs:[esi]
	movsb	[edi], [esi]
	movsb	byte ptr es:[edi], [esi]
	movsb	es:[edi], byte ptr [esi]
	movsb	byte ptr es:[edi], byte ptr [esi]
	movs	byte ptr es:[edi], [esi]
	movs	es:[edi], byte ptr [esi]
	movs	byte ptr es:[edi], byte ptr [esi]

	movsb	es:[adi], [asi]
	movsb	es:[adi], fs:[asi]
	movsb	[adi], [asi]
	movsb	byte ptr es:[adi], [asi]
	movsb	es:[adi], byte ptr [asi]
	movsb	byte ptr es:[adi], byte ptr [asi]
	movs	byte ptr es:[adi], [asi]
	movs	es:[adi], byte ptr [asi]
	movs	byte ptr es:[adi], byte ptr [asi]

	movsw

	movsw	es:[edi], [esi]
	movsw	es:[edi], fs:[esi]
	movsw	[edi], [esi]
	movsw	word ptr es:[edi], [esi]
	movsw	es:[edi], word ptr [esi]
	movsw	word ptr es:[edi], word ptr [esi]
	movs	word ptr es:[edi], [esi]
	movs	es:[edi], word ptr [esi]
	movs	word ptr es:[edi], word ptr [esi]

	movsw	es:[adi], [asi]
	movsw	es:[adi], fs:[asi]
	movsw	[adi], [asi]
	movsw	word ptr es:[adi], [asi]
	movsw	es:[adi], word ptr [asi]
	movsw	word ptr es:[adi], word ptr [asi]
	movs	word ptr es:[adi], [asi]
	movs	es:[adi], word ptr [asi]
	movs	word ptr es:[adi], word ptr [asi]

	movsd

	movsd	es:[edi], [esi]
	movsd	es:[edi], fs:[esi]
	movsd	[edi], [esi]
	movsd	dword ptr es:[edi], [esi]
	movsd	es:[edi], dword ptr [esi]
	movsd	dword ptr es:[edi], dword ptr [esi]
	movs	dword ptr es:[edi], [esi]
	movs	es:[edi], dword ptr [esi]
	movs	dword ptr es:[edi], dword ptr [esi]

	movsd	es:[adi], [asi]
	movsd	es:[adi], fs:[asi]
	movsd	[adi], [asi]
	movsd	dword ptr es:[adi], [asi]
	movsd	es:[adi], dword ptr [asi]
	movsd	dword ptr es:[adi], dword ptr [asi]
	movs	dword ptr es:[adi], [asi]
	movs	es:[adi], dword ptr [asi]
	movs	dword ptr es:[adi], dword ptr [asi]

.ifdef x86_64
	movsq

	movsq	es:[rdi], [rsi]
	movsq	es:[rdi], fs:[rsi]
	movsq	[rdi], [rsi]
	movsq	qword ptr es:[rdi], [rsi]
	movsq	es:[rdi], qword ptr [rsi]
	movsq	qword ptr es:[rdi], qword ptr [rsi]
	movs	qword ptr es:[rdi], [rsi]
	movs	es:[rdi], qword ptr [rsi]
	movs	qword ptr es:[rdi], qword ptr [rsi]

	movsq	es:[edi], [esi]
	movsq	es:[edi], fs:[esi]
	movsq	[edi], [esi]
	movsq	qword ptr es:[edi], [esi]
	movsq	es:[edi], qword ptr [esi]
	movsq	qword ptr es:[edi], qword ptr [esi]
	movs	qword ptr es:[edi], [esi]
	movs	es:[edi], qword ptr [esi]
	movs	qword ptr es:[edi], qword ptr [esi]
.endif
