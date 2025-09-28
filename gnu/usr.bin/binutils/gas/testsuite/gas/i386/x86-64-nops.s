	.text

	.insn 0x0f1f/0, (%rax)
	.insn {disp8} 0x0f1f/0, 0(%rax)
	.insn {disp8} 0x0f1f/0, 0(%rax,%rax)
	.insn {disp8} data16 0x0f1f/0, 0(%rax,%rax)
	.insn {disp32} 0x0f1f/0, 0(%rax)
	.insn {disp32} 0x0f1f/0, 0(%rax,%rax)
	.insn {disp32} data16 0x0f1f/0, 0(%rax,%rax)
	.insn {disp32} data16 0x0f1f/0, %cs:0(%rax,%rax)

	# reg,reg
	.insn 0x0f19, %edi, %edi
	.insn 0x0f1a, %edi, %edi
	.insn 0x0f1b, %edi, %edi
	.insn 0x0f1c, %edi, %edi
	.insn 0x0f1d, %edi, %edi
	.insn 0x0f1e, %edi, %edi
	.insn 0x0f1f, %edi, %edi

	# with base and imm8
	.insn 0x0f19/3, 0x22(%rdx)
	.insn 0x0f1c/3, 0x22(%rdx)
	.insn 0x0f1d/3, 0x22(%rdx)
	.insn 0x0f1e/3, 0x22(%rdx)
	.insn 0x0f1f/3, 0x22(%rdx)

	# with sib and imm32
	.insn 0x0f19/3, 0x44332211(%rbp,%rbx)
	.insn 0x0f1c/3, 0x44332211(%rbp,%rbx)
	.insn 0x0f1d/3, 0x44332211(%rbp,%rbx)
	.insn 0x0f1e/3, 0x44332211(%rbp,%rbx)
	.insn 0x0f1f/3, 0x44332211(%rbp,%rbx)

	.allow_index_reg
	.insn 0x0f19/0, (%rax,%riz,2)
	.insn 0x0f1c/1, (%rax,%riz,2)
	.insn 0x0f1d/0, (%rax,%riz,2)
	.insn 0x0f1e/0, (%rax,%riz,2)
	.insn 0x0f1f/0, (%rax,%riz,2)

	.insn 0x0f19/0, (%rcx,%rbx,2)
	.insn 0x0f1c/1, (%rcx,%rbx,2)
	.insn 0x0f1d/0, (%rcx,%rbx,2)
	.insn 0x0f1e/0, (%rcx,%rbx,2)
	.insn 0x0f1f/0, (%rcx,%rbx,2)

	nop %rax
	nop %eax
	nop %ax
	nopq (%rax) 
	nopl (%rax) 
	nopw (%rax) 
	nopq %rax
	nopl %eax
	nopw %ax
	nop %r10
	nop %r10d
	nop %r10w
	nopq (%r10) 
	nopl (%r10) 
	nopw (%r10) 
	nopq %r10
	nopl %r10d
	nopw %r10w
