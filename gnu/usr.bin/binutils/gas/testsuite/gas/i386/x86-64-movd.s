# Check movd/vmovd with memory and register.

	.text
_start:
	movd 128(%rax), %xmm1
	movd %rax, %xmm1
	movd %xmm1, 128(%rax)
	movd %xmm1, %rax
	vmovd 128(%rax), %xmm1
	vmovd %rax, %xmm1
	vmovd %xmm1, 128(%rax)
	vmovd %xmm1, %rax
	{evex} vmovd 128(%rax), %xmm1
	{evex} vmovd %xmm1, 128(%rax)
	.intel_syntax noprefix
	movd xmm1, [rax + 128]
	movd xmm1, dword ptr [rax + 128]
	movd xmm1, eax
	movd dword ptr [rax + 128], xmm1
	movd [rax + 128], xmm1
	movd eax, xmm1
	movd xmm1, qword ptr [rax + 128]
	movd xmm1, rax
	movd qword ptr [rax + 128], xmm1
	movd rax, xmm1
	vmovd xmm1, dword ptr [rax + 128]
	vmovd xmm1, [rax + 128]
	vmovd xmm1, eax
	vmovd dword ptr [rax + 128], xmm1
	vmovd [rax + 128], xmm1
	vmovd eax, xmm1
	{evex} vmovd xmm1, dword ptr [rax + 128]
	{evex} vmovd xmm1, [rax + 128]
	{evex} vmovd xmm1, eax
	{evex} vmovd dword ptr [rax + 128], xmm1
	{evex} vmovd [rax + 128], xmm1
	{evex} vmovd eax, xmm1
	vmovd xmm1, rax
	vmovd rax, xmm1
