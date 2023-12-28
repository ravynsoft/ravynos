.irp reg ax,bx,cx,dx,sp,bp,si,di
	monitorx %e\reg, %ecx, %edx
	monitorx %eax, %e\reg, %edx
	monitorx %eax, %ecx, %e\reg
	mwaitx %e\reg, %ecx, %ebx
	mwaitx %eax, %e\reg, %ebx
	mwaitx %eax, %ecx, %e\reg
.endr
