	.text
	.code32
start32:
	cmpsb	(%edi), %cs:(%esi)
	cmpsb	%es:(%edi), (%esi)
	cmpsb	(%di), (%si)
	cmpsb	(%esi), (%edi)

	insb	(%dx), %es:(%edi)
	insb	(%dx), (%esi)

	lodsb	%cs:(%esi)
	lodsb	(%edi)

	movsb	%cs:(%esi), (%edi)
	movsb	(%esi), %es:(%edi)
	movsb	(%si), (%di)
	movsb	(%ebx), (%edi)
	movsb	(%esi), (%ebx)

	outsb	%cs:(%esi), (%dx)
	outsb	(%edi), (%dx)

	scasb	%es:(%edi)
	scasb	(%esi)

	stosb	%es:(%edi)
	stosb	(%esi)

	xlat	(%ebx)
	xlat	(%bx)
	xlat	%ds:(%ebx)
	xlatb
	xlatb	(%ebx)
	xlatb	%cs:(%ebx)

	.code16
start16:
	cmpsb	(%di), (%si)
	movsb	(%esi), (%edi)

	.code64
start64:
	cmpsb	(%rdi), (%rsi)
	movsb	(%esi), (%edi)

	.intel_syntax noprefix
	.code32
intel32:
	cmps	byte ptr cs:[esi], [edi]
	cmps	byte ptr [esi], es:[edi]
	cmps	byte ptr [esi], byte ptr [edi]
	cmps	byte ptr [si], [di]
	cmps	byte ptr [edi], [esi]

	ins	byte ptr es:[edi], dx
	ins	byte ptr [esi], dx

	lods	byte ptr cs:[esi]
	lods	byte ptr [edi]

	movs	byte ptr [edi], cs:[esi]
	movs	byte ptr es:[edi], [esi]
	movs	byte ptr [edi], byte ptr [esi]
	movs	byte ptr [di], [si]
	movs	byte ptr [edi], [ebx]
	movs	byte ptr [ebx], [esi]

	outs	dx, byte ptr cs:[esi]
	outs	dx, byte ptr [edi]

	scas	byte ptr es:[edi]
	scas	byte ptr [esi]

	stos	byte ptr es:[edi]
	stos	byte ptr [esi]

	xlatb
	xlat	[bx]
	xlat	ds:[ebx]
	xlat	byte ptr [ebx]
	xlat	byte ptr cs:[ebx]

	.code16
intel16:
	cmps	byte ptr [si], [di]
	movs	byte ptr [edi], [esi]

	.code64
intel64:
	cmps	byte ptr [rsi], [rdi]
	movs	byte ptr [edi], [esi]
