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

cmps:
	cmpsb

	cmpsb	[esi], es:[edi]
	cmpsb	fs:[esi], es:[edi]
	cmpsb	[esi], [edi]
	cmpsb	byte ptr [esi], es:[edi]
	cmpsb	[esi], byte ptr es:[edi]
	cmpsb	byte ptr [esi], byte ptr es:[edi]
	cmps	byte ptr [esi], es:[edi]
	cmps	[esi], byte ptr es:[edi]
	cmps	byte ptr [esi], byte ptr es:[edi]

	cmpsb	[asi], es:[adi]
	cmpsb	fs:[asi], es:[adi]
	cmpsb	[asi], [adi]
	cmpsb	byte ptr [asi], es:[adi]
	cmpsb	[asi], byte ptr es:[adi]
	cmpsb	byte ptr [asi], byte ptr es:[adi]
	cmps	byte ptr [asi], es:[adi]
	cmps	[asi], byte ptr es:[adi]
	cmps	byte ptr [asi], byte ptr es:[adi]

	cmpsw

	cmpsw	[esi], es:[edi]
	cmpsw	fs:[esi], es:[edi]
	cmpsw	[esi], [edi]
	cmpsw	word ptr [esi], es:[edi]
	cmpsw	[esi], word ptr es:[edi]
	cmpsw	word ptr [esi], word ptr es:[edi]
	cmps	word ptr [esi], es:[edi]
	cmps	[esi], word ptr es:[edi]
	cmps	word ptr [esi], word ptr es:[edi]

	cmpsw	[asi], es:[adi]
	cmpsw	fs:[asi], es:[adi]
	cmpsw	[asi], [adi]
	cmpsw	word ptr [asi], es:[adi]
	cmpsw	[asi], word ptr es:[adi]
	cmpsw	word ptr [asi], word ptr es:[adi]
	cmps	word ptr [asi], es:[adi]
	cmps	[asi], word ptr es:[adi]
	cmps	word ptr [asi], word ptr es:[adi]

	cmpsd

	cmpsd	[esi], es:[edi]
	cmpsd	fs:[esi], es:[edi]
	cmpsd	[esi], [edi]
	cmpsd	dword ptr [esi], es:[edi]
	cmpsd	[esi], dword ptr es:[edi]
	cmpsd	dword ptr [esi], dword ptr es:[edi]
	cmps	dword ptr [esi], es:[edi]
	cmps	[esi], dword ptr es:[edi]
	cmps	dword ptr [esi], dword ptr es:[edi]

	cmpsd	[asi], es:[adi]
	cmpsd	fs:[asi], es:[adi]
	cmpsd	[asi], [adi]
	cmpsd	dword ptr [asi], es:[adi]
	cmpsd	[asi], dword ptr es:[adi]
	cmpsd	dword ptr [asi], dword ptr es:[adi]
	cmps	dword ptr [asi], es:[adi]
	cmps	[asi], dword ptr es:[adi]
	cmps	dword ptr [asi], dword ptr es:[adi]

.ifdef x86_64
	cmpsq

	cmpsq	[rsi], es:[rdi]
	cmpsq	fs:[rsi], es:[rdi]
	cmpsq	[rsi], [rdi]
	cmpsq	qword ptr [rsi], es:[rdi]
	cmpsq	[rsi], qword ptr es:[rdi]
	cmpsq	qword ptr [rsi], qword ptr es:[rdi]
	cmps	qword ptr [rsi], es:[rdi]
	cmps	[rsi], qword ptr es:[rdi]
	cmps	qword ptr [rsi], qword ptr es:[rdi]

	cmpsq	[esi], es:[edi]
	cmpsq	fs:[esi], es:[edi]
	cmpsq	[esi], [edi]
	cmpsq	qword ptr [esi], es:[edi]
	cmpsq	[esi], qword ptr es:[edi]
	cmpsq	qword ptr [esi], qword ptr es:[edi]
	cmps	qword ptr [esi], es:[edi]
	cmps	[esi], qword ptr es:[edi]
	cmps	qword ptr [esi], qword ptr es:[edi]
.endif
