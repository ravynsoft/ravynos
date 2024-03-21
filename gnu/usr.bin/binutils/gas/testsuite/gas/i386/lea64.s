	.text
start:
	lea	%fs:(%rax,%rcx), %eax
	gs lea	(%rax,%rcx), %eax

	.allow_index_reg
	lea	1(%rax), %ecx
	lea	sym(%rax), %ecx
	lea	sym(,%riz), %ecx

	lea	(%rax,%rax), %eax
	lea	(,%rax,2), %eax
	lea	(%rip), %eax
	lea	(,%riz), %eax

	lea	(%rax), %rax
	lea	(%rax), %rcx
	lea	1-1(%rax), %rcx
	lea	%gs:(%rax), %rcx

	lea	(%rsi), %eax
	lea	(%rsi), %esi
	leal	(%rsi), %eax

	lea	(%rsi), %ax
	lea	(%rsi), %si
	leaw	(%rsi), %ax

	lea	(%eax), %rax
	lea	(%eax), %rcx
	leaq	(%eax), %rcx

	lea	(%eax), %eax
	lea	(%eax), %ecx

	lea	(%esi), %ax
	lea	(%esi), %si
	leaw	(%esi), %ax

	lea	(,%rcx,1), %rcx
	lea	(,%rcx,1), %rax

	lea	(,%rcx,1), %ecx
	lea	(,%rcx,1), %eax

	lea	(,%rcx,1), %cx
	lea	(,%rcx,1), %ax

	lea	(,%ecx,1), %rcx
	lea	(,%ecx,1), %rax

	lea	(,%ecx,1), %ecx
	lea	(,%ecx,1), %eax

	lea	(,%ecx,1), %cx
	lea	(,%ecx,1), %ax

	lea	1, %rax
	lea	2, %eax
	lea	3, %ax

	lea	-1, %rax
	lea	-2, %eax
	lea	-3, %ax

	addr32 lea 1, %rax
	addr32 lea 2, %eax
	addr32 lea 3, %ax

	addr32 lea -1, %rax
	addr32 lea -2, %eax
	addr32 lea -3, %ax

	lea	sym, %rax
	lea	sym, %eax
	lea	sym, %ax

	addr32 lea sym, %rax
	addr32 lea sym, %eax
	addr32 lea sym, %ax

	lea	(,1), %rax
	lea	(,1), %eax
	lea	(,1), %ax

	lea	0xffffffff(%ecx), %eax
	lea	0xffffffff(%ecx), %rax
	lea	0xffffffff(%rcx), %eax

	lea	-0xffffffff(%ecx), %eax
	lea	-0xffffffff(%ecx), %rax
	lea	-0xffffffff(%rcx), %eax
