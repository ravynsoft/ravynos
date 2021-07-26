# Original - Christopher Lloyd <cjwl@objc.net>
.globl objc_msgSendv
	.type	objc_msgSendv, @function
objc_msgSendv:
	pushq	%rbp
	movq	%rsp, %rbp
	pushq	24(%rbp)
	pushq   16(%rbp)
	call	objc_msg_lookup@PLT
	movq 32(%rbp),%rcx # rcx=argumentFrameByteSize
	movq 40(%rbp),%rdx # rdx=argumentFrame
pushNext:
	subq $8,%rcx       # argumentFrameByteSize-=sizeof(int_64bit)
	cmpq $8,%rcx       # check if we're at _cmd in argumentFrame
	jle done
	pushq (%rdx,%rcx)
	jmp pushNext
done:
	pushq 24(%rbp) # push _cmd
	pushq 16(%rbp)  # push self
	call *%rax
	leave
	ret
	.size	objc_msgSendv, .-objc_msgSendv
	.ident	"GCC: (GNU) 3.3.2"
