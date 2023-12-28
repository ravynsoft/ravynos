	.file	"t3.c"
	.text
	.globl	nMainCRTStartup
	.def	nMainCRTStartup;	.scl	2;	.type	32;	.endef
	.seh_proc	nMainCRTStartup
nMainCRTStartup:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	movl	$255, -4(%rbp)
.l_startw:
	.seh_handler __C_specific_handler, @except
	.seh_handlerdata
	.long 1
	.rva .l_startw, .l_endw, _gnu_exception_handler ,.l_endw
	.text
	call	__security_init_cookie
	call	__tmainCRTStartup
	movl	%eax, -4(%rbp)
.l_endw: nop
	movl	-4(%rbp), %eax
	addq	$48, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.def	__security_init_cookie;	.scl	2;	.type	32;	.endef
	.def	__tmainCRTStartup;	.scl	2;	.type	32;	.endef
