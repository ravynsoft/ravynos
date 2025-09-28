# Original - Christopher Lloyd <cjwl@objc.net>
.globl objc_msgSendv
	.type	objc_msgSendv, @function
objc_msgSendv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	12(%ebp)
	pushl   8(%ebp)
	call	objc_msg_lookup
	movl 16(%ebp),%ecx # ecx=argumentFrameByteSize
	movl 20(%ebp),%edx # edx=argumentFrame
pushNext:
	subl $4,%ecx       # argumentFrameByteSize-=sizeof(int)
	cmpl $4,%ecx       # check if we're at _cmd in argumentFrame
	jle done
	pushl (%edx,%ecx)
	jmp pushNext
done:
	pushl 12(%ebp) # push _cmd
	pushl 8(%ebp)  # push self
	call *%eax
	leave
	ret
	.size	objc_msgSendv, .-objc_msgSendv
	.ident	"GCC: (GNU) 3.3.2"
