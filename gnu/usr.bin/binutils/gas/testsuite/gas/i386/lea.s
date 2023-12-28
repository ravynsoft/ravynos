	.text
start:
	lea	%ss:(%eax,%ecx), %eax
	ss lea	(%eax,%ecx), %eax
	{nooptimize} es lea (%ecx,%eax), %eax

	.allow_index_reg
	lea	1(%eax), %ecx
	lea	sym(%eax), %ecx
	lea	sym(,%eiz), %ecx

	lea	(%eax,%eax), %eax
	lea	(,%eax,2), %eax
	lea	(,%eiz), %eax
	lea	(%bx,%si), %eax

	lea	(%eax), %eax
	lea	(%eax), %ecx
	lea	1-1(%eax), %ecx
	lea	%gs:(%eax), %ecx
	{nooptimize} lea %fs:(%eax), %ecx

	lea	(%si), %eax
	lea	(%si), %esi
	leal	(%si), %eax

	lea	(%eax), %ax
	lea	(%eax), %cx
	leaw	(%eax), %cx

	lea	(%si), %ax
	lea	(%si), %si

	lea	(,%ecx,1), %ecx
	lea	(,%ecx,1), %eax

	lea	1, %eax
	lea	2, %ax

	lea	-1, %eax
	lea	-2, %ax

	addr16 lea 1, %eax
	addr16 lea 2, %ax

	addr16 lea -1, %eax
	addr16 lea -2, %ax

	lea	sym, %eax
	lea	sym, %ax

	addr16 lea sym, %eax
	addr16 lea sym, %ax

	lea	(,1), %eax
	lea	(,1), %ax
