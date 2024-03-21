	.text
	.intel_syntax noprefix
	.code16

long16:
	lwpins	eax, es:[eax*8], 0x11223344
	lwpval	eax, es:[eax*4], 0x11223344
	bextr	eax, es:[eax*2], 0x11223344

	xacquire lock add dword ptr es:[eax*2], 0x11223344
	xrelease lock sub dword ptr es:[eax*2], 0x11223344
