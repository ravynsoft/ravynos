	.text
	.code16
	rep; movsd
	rep; cmpsd
	rep movsl %ds:(%si),%es:(%di)
	rep cmpsl %es:(%di),%ds:(%si)

	mov	%cr2, %ecx
	mov	%ecx, %cr2

	mov	%dr2, %ecx
	mov	%ecx, %dr2

	mov	%tr2, %ecx
	mov	%ecx, %tr2

	bswap	%ecx

	.intel_syntax noprefix
	rep movsd dword ptr es:[di], dword ptr ds:[si]
	rep cmpsd dword ptr ds:[si], dword ptr es:[di]
