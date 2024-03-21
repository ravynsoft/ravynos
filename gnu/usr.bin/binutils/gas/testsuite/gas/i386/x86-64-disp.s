	.text
	mov 0x7fffffff(%rax),%ebx
	mov -0x80000000(%rax),%ebx
	mov -0x80000000,%ebx
	mov 0xffffffff80000000,%ebx
	mov 0x7fffffff,%ebx
	mov -0x80000000,%eax
	mov 0xffffffff80000000,%eax
	mov 0x7fffffff,%eax
	mov 0x80000000,%eax

	.intel_syntax noprefix
	mov eax, offset 0xEE000F0

	mov DWORD PTR [rax+0xEE000F0], ebx
	mov [rax+0xEE000F0], ebx
	mov DWORD PTR gs:[rax+0xEE000F0], ebx
	mov gs:[rax+0xEE000F0], ebx

	mov DWORD PTR [0xEE000F0], ebx
	mov DWORD PTR gs:0xEE000F0, ebx

	mov DWORD PTR [0xEE000F0], eax
	mov DWORD PTR gs:0xEE000F0, eax
	mov DWORD PTR [0xFEE000F0], eax
	mov DWORD PTR gs:0xFEE000F0, eax

	mov ebx, DWORD PTR gs:0xEE000F0
	mov ebx, DWORD PTR [0xEE000F0]
	mov ebx, [0xEE000F0]

	mov eax, DWORD PTR gs:0xEE000F0
	mov eax, DWORD PTR [0xEE000F0]
	mov eax, [0xEE000F0]
	mov eax, DWORD PTR gs:0xFEE000F0
	mov eax, DWORD PTR [0xFEE000F0]
	mov eax, [0xFEE000F0]
