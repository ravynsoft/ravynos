	.text
	mov (%rax),%ebx
	mov 3(%rax),%ebx

	mov.d8 (%rax),%ebx
	mov.d8 3(%rax),%ebx
	mov.d8 0xfff(%rax),%ebx

	mov.d32 (%rax),%ebx
	mov.d32 3(%rax),%ebx
	vmovdqu64.d32 -0x40(%rax),%xmm3

	jmp foo
	jmp.d8 foo
	jmp.d32 foo
foo:

	.intel_syntax noprefix
	mov DWORD PTR [rax], ebx
	mov DWORD PTR [rax+3], ebx
	mov DWORD PTR [rax+0xfff], ebx

	mov.d8 DWORD PTR [rax], ebx
	mov.d8 DWORD PTR [rax+3], ebx

	mov.d32 DWORD PTR [rax], ebx
	mov.d32 DWORD PTR [rax+3], ebx

	vmovdqu64.d32 xmm3,XMMWORD PTR [rax-0x40]
