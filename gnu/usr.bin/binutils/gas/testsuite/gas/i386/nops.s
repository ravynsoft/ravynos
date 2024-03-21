	.text

	.insn 0x0f1f/0, (%eax)
	.insn {disp8} 0x0f1f/0, 0(%eax)
	.insn {disp8} 0x0f1f/0, 0(%eax,%eax)
	.insn {disp8} data16 0x0f1f/0, 0(%eax,%eax)
	.insn {disp32} 0x0f1f/0, 0(%eax)
	.insn {disp32} 0x0f1f/0, 0(%eax,%eax)
	.insn {disp32} data16 0x0f1f/0, 0(%eax,%eax)
	.insn {disp32} data16 0x0f1f/0, %cs:0(%eax,%eax)

	# reg,reg
	.insn 0x0f19, %edi, %edi
	.insn 0x0f1a, %edi, %edi
	.insn 0x0f1b, %edi, %edi
	.insn 0x0f1c, %edi, %edi
	.insn 0x0f1d, %edi, %edi
	.insn 0x0f1e, %edi, %edi
	.insn 0x0f1f, %edi, %edi

	# with base and imm8
	.insn 0x0f19/3, 0x22(%edx)
	.insn 0x0f1c/3, 0x22(%edx)
	.insn 0x0f1d/3, 0x22(%edx)
	.insn 0x0f1e/3, 0x22(%edx)
	.insn 0x0f1f/3, 0x22(%edx)

	# with sib and imm32
	.insn 0x0f19/3, 0x44332211(%ebp,%ebx)
	.insn 0x0f1c/3, 0x44332211(%ebp,%ebx)
	.insn 0x0f1d/3, 0x44332211(%ebp,%ebx)
	.insn 0x0f1e/3, 0x44332211(%ebp,%ebx)
	.insn 0x0f1f/3, 0x44332211(%ebp,%ebx)

	.allow_index_reg
	.insn 0x0f19/0, (%eax,%eiz,2)
	.insn 0x0f1c/1, (%eax,%eiz,2)
	.insn 0x0f1d/0, (%eax,%eiz,2)
	.insn 0x0f1e/0, (%eax,%eiz,2)
	.insn 0x0f1f/0, (%eax,%eiz,2)

	.insn 0x0f19/0, (%ecx,%ebx,2)
	.insn 0x0f1c/1, (%ecx,%ebx,2)
	.insn 0x0f1d/0, (%ecx,%ebx,2)
	.insn 0x0f1e/0, (%ecx,%ebx,2)
	.insn 0x0f1f/0, (%ecx,%ebx,2)

	nop %eax
	nop %ax
	nopl (%eax) 
	nopw (%eax) 
	nopl %eax
	nopw %ax
