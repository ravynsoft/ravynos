	.text
unspec:
	bextr	%eax, (%rax), %rax
	bextr	%rax, (%rax), %eax
	bzhi	%eax, (%rax), %rax
	bzhi	%rax, (%rax), %eax
	sarx	%eax, (%rax), %rax
	sarx	%rax, (%rax), %eax
	shlx	%eax, (%rax), %rax
	shlx	%rax, (%rax), %eax
	shrx	%eax, (%rax), %rax
	shrx	%rax, (%rax), %eax

	.intel_syntax noprefix

	bextr	eax, [rax], rax
	bextr	rax, [rax], eax
	bzhi	eax, [rax], rax
	bzhi	rax, [rax], eax
	sarx	eax, [rax], rax
	sarx	rax, [rax], eax
	shlx	eax, [rax], rax
	shlx	rax, [rax], eax
	shrx	eax, [rax], rax
	shrx	rax, [rax], eax
