.irp reg ax,bx,cx,dx,sp,bp,si,di,8,9,10,11,12,13,14,15
	monitorx %r\reg, %rcx, %rdx
	monitorx %rax, %r\reg, %rdx
	monitorx %rax, %rcx, %r\reg
	mwaitx %r\reg, %rcx, %rbx
	mwaitx %rax, %r\reg, %rbx
	mwaitx %rax, %rcx, %r\reg
.endr
