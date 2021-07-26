#ifdef WINDOWS
# Original - Christopher Lloyd <cjwl@objc.net>
.globl _objc_msgSendv
_objc_msgSendv:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	12(%ebp)
	pushl   8(%ebp)
	call	_objc_msg_lookup
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

.globl _objc_msgSendv_stret
_objc_msgSendv_stret:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	16(%ebp) # _cmd
	pushl	12(%ebp) # self
	call	_objc_msg_lookup
	movl 20(%ebp),%ecx # ecx=argumentFrameByteSize
	movl 24(%ebp),%edx # edx=argumentFrame
pushNext_stret:
	subl $4,%ecx       # argumentFrameByteSize-=sizeof(int)
	cmpl $8,%ecx       # check if we're at _cmd in argumentFrame
	jle done_stret
	pushl (%edx,%ecx)
	jmp pushNext_stret
done_stret:
	pushl 16(%ebp) # push _cmd
	pushl 12(%ebp) # push self
	pushl 8(%ebp)  # push return value ptr
	call *%eax
	leave
	ret

    .section .drectve
    .ascii " -export:objc_msgSendv"
    .ascii " -export:objc_msgSendv_stret"
#endif

