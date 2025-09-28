	.text
	.code32
start:
	movsb	(%esi), (%di)
	movsb	(%si), (%edi)
	movsb	(%esi), %ds:(%edi)
	stosb	%ds:(%edi)
	cmpsb	%ds:(%edi), (%esi)
	scasb	%ds:(%edi)
	insb	(%dx), %ds:(%edi)
	xlatb	(%esi)
	xlatb	(,%ebx)
	xlatb	1(%ebx)
	xlatb	x(%ebx)
	xlatb	0

	.intel_syntax noprefix

	movs	byte ptr [edi], [si]
	movs	byte ptr [di], [esi]
	movs	byte ptr ds:[edi], [esi]
	movs	byte ptr [edi], word ptr [esi]
	stos	byte ptr ds:[edi]
	cmps	byte ptr [esi], ds:[edi]
	cmps	byte ptr [esi], dword ptr [edi]
	scas	byte ptr ds:[edi]
	ins	byte ptr ds:[edi], dx
	xlat	byte ptr [esi]
	xlat	byte ptr [%ebx*1]
	xlat	byte ptr [ebx+1]
	xlat	byte ptr x[ebx]
	xlat	byte ptr FLAT:0
