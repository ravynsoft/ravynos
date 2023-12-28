	.text
	.intel_syntax noprefix

long64:
	lwpins	rax, fs:[eax*8], 0x11223344
	lwpval	eax, fs:[eax*4], 0x11223344
	bextr	rax, fs:[eax*2], 0x11223344

	xacquire lock add qword ptr gs:[eax*8], 0x11223344
	xrelease lock sub qword ptr gs:[eax*8], 0x11223344
