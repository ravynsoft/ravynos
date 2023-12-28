	.file	"t4.C"
	.section	.text$_ZN5VBase1fEv,"x"
	.linkonce discard
	.align 2
	.globl	_ZN5VBase1fEv
	.def	_ZN5VBase1fEv;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN5VBase1fEv
_ZN5VBase1fEv:
.LFB0:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	nop
	popq	%rbp
	ret
	.seh_endproc
	.section	.text$_ZN10StreamBaseD2Ev,"x"
	.linkonce discard
	.align 2
	.globl	_ZN10StreamBaseD2Ev
	.def	_ZN10StreamBaseD2Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN10StreamBaseD2Ev
_ZN10StreamBaseD2Ev:
.LFB2:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	16(%rbp), %rax
	leaq	16+_ZTV10StreamBase(%rip), %rdx
	movq	%rdx, (%rax)
	movl	$0, %eax
	andl	$1, %eax
	testb	%al, %al
	je	.L2
	movq	16(%rbp), %rcx
	call	_ZdlPv
.L2:
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section	.text$_ZN10StreamBaseD1Ev,"x"
	.linkonce discard
	.align 2
	.globl	_ZN10StreamBaseD1Ev
	.def	_ZN10StreamBaseD1Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN10StreamBaseD1Ev
_ZN10StreamBaseD1Ev:
.LFB3:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	16(%rbp), %rax
	leaq	16+_ZTV10StreamBase(%rip), %rdx
	movq	%rdx, (%rax)
	movl	$2, %eax
	andl	$1, %eax
	testb	%al, %al
	je	.L5
	movq	16(%rbp), %rcx
	call	_ZdlPv
.L5:
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section	.text$_ZN10StreamBaseD0Ev,"x"
	.linkonce discard
	.align 2
	.globl	_ZN10StreamBaseD0Ev
	.def	_ZN10StreamBaseD0Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN10StreamBaseD0Ev
_ZN10StreamBaseD0Ev:
.LFB4:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	16(%rbp), %rcx
	call	_ZN10StreamBaseD1Ev
	movq	16(%rbp), %rcx
	call	_ZdlPv
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section	.text$_ZN6StreamD2Ev,"x"
	.linkonce discard
	.align 2
	.globl	_ZN6StreamD2Ev
	.def	_ZN6StreamD2Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN6StreamD2Ev
_ZN6StreamD2Ev:
.LFB6:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	%rdx, 24(%rbp)
	movq	24(%rbp), %rax
	movq	(%rax), %rdx
	movq	16(%rbp), %rax
	movq	%rdx, (%rax)
	movq	16(%rbp), %rax
	movq	(%rax), %rax
	subq	$24, %rax
	movq	(%rax), %rax
	addq	16(%rbp), %rax
	movq	24(%rbp), %rdx
	addq	$8, %rdx
	movq	(%rdx), %rdx
	movq	%rdx, (%rax)
	movq	16(%rbp), %rcx
	call	_ZN10StreamBaseD2Ev
	movl	$0, %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	.L11
	movq	16(%rbp), %rax
	addq	$8, %rax
	movq	%rax, %rcx
	call	_ZN5VBaseD2Ev
.L11:
	movl	$0, %eax
	andl	$1, %eax
	testb	%al, %al
	je	.L10
	movq	16(%rbp), %rcx
	call	_ZdlPv
.L10:
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section	.text$_ZN6StreamD1Ev,"x"
	.linkonce discard
	.align 2
	.globl	_ZN6StreamD1Ev
	.def	_ZN6StreamD1Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN6StreamD1Ev
_ZN6StreamD1Ev:
.LFB7:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	leaq	24+_ZTV6Stream(%rip), %rdx
	movq	16(%rbp), %rax
	movq	%rdx, (%rax)
	movl	$8, %eax
	addq	16(%rbp), %rax
	leaq	64+_ZTV6Stream(%rip), %rdx
	movq	%rdx, (%rax)
	movq	16(%rbp), %rcx
	call	_ZN10StreamBaseD2Ev
	movl	$2, %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	.L14
	movq	16(%rbp), %rax
	addq	$8, %rax
	movq	%rax, %rcx
	call	_ZN5VBaseD2Ev
.L14:
	movl	$2, %eax
	andl	$1, %eax
	testb	%al, %al
	je	.L13
	movq	16(%rbp), %rcx
	call	_ZdlPv
.L13:
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.section	.text$_ZN6StreamD0Ev,"x"
	.linkonce discard
	.align 2
	.globl	_ZN6StreamD0Ev
	.def	_ZN6StreamD0Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN6StreamD0Ev
_ZN6StreamD0Ev:
.LFB8:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	16(%rbp), %rcx
	call	_ZN6StreamD1Ev
	movq	16(%rbp), %rcx
	call	_ZdlPv
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.globl	r
	.bss
	.align 4
r:
	.space 4
	.section	.text$_ZN13DerivedStreamD1Ev,"x"
	.linkonce discard
	.align 2
	.globl	_ZN13DerivedStreamD1Ev
	.def	_ZN13DerivedStreamD1Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN13DerivedStreamD1Ev
_ZN13DerivedStreamD1Ev:
.LFB12:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$40, %rsp
	.seh_stackalloc	40
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	leaq	24+_ZTV13DerivedStream(%rip), %rdx
	movq	16(%rbp), %rax
	movq	%rdx, (%rax)
	movl	$8, %eax
	addq	16(%rbp), %rax
	leaq	64+_ZTV13DerivedStream(%rip), %rdx
	movq	%rdx, (%rax)
	leaq	_ZTT13DerivedStream(%rip), %rax
	addq	$8, %rax
	movq	%rax, %rdx
	movq	16(%rbp), %rcx
.LEHB0:
	call	_ZN6StreamD2Ev
.LEHE0:
	movl	$2, %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	.L19
	movq	16(%rbp), %rax
	addq	$8, %rax
	movq	%rax, %rcx
.LEHB1:
	call	_ZN5VBaseD2Ev
.LEHE1:
.L19:
	movl	$2, %eax
	andl	$1, %eax
	testb	%al, %al
	je	.L18
	movq	16(%rbp), %rcx
	call	_ZdlPv
	jmp	.L18
.L23:
	movq	%rax, %rbx
	movl	$2, %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	.L22
	movq	16(%rbp), %rax
	addq	$8, %rax
	movq	%rax, %rcx
	call	_ZN5VBaseD2Ev
.L22:
	movq	%rbx, %rax
	movq	%rax, %rcx
.LEHB2:
	call	_Unwind_Resume
	nop
.LEHE2:
.L18:
	nop
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	ret
	.def	__gxx_personality_v0;	.scl	2;	.type	32;	.endef
	.seh_handler	_GCC_specific_handler, @unwind, @except
	.seh_handlerdata
	.rva	__gxx_personality_v0
	.section	.text$_ZN13DerivedStreamD1Ev,"x"
	.linkonce discard
	.seh_handlerdata
.LLSDA12:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE12-.LLSDACSB12
.LLSDACSB12:
	.uleb128 .LEHB0-.LFB12
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L23-.LFB12
	.uleb128 0
	.uleb128 .LEHB1-.LFB12
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB2-.LFB12
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0
	.uleb128 0
.LLSDACSE12:
	.section	.text$_ZN13DerivedStreamD1Ev,"x"
	.linkonce discard
	.seh_endproc
	.section	.text$_ZN13DerivedStreamD0Ev,"x"
	.linkonce discard
	.align 2
	.globl	_ZN13DerivedStreamD0Ev
	.def	_ZN13DerivedStreamD0Ev;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN13DerivedStreamD0Ev
_ZN13DerivedStreamD0Ev:
.LFB13:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$32, %rsp
	.seh_stackalloc	32
	.seh_endprologue
	movq	%rcx, 16(%rbp)
	movq	16(%rbp), %rcx
	call	_ZN13DerivedStreamD1Ev
	movq	16(%rbp), %rcx
	call	_ZdlPv
	nop
	addq	$32, %rsp
	popq	%rbp
	ret
	.seh_endproc
	.text
	.globl	_Z7ctor2_xv
	.def	_Z7ctor2_xv;	.scl	2;	.type	32;	.endef
	.seh_proc	_Z7ctor2_xv
_Z7ctor2_xv:
.LFB9:
	pushq	%rbp
	.seh_pushreg	%rbp
	movq	%rsp, %rbp
	.seh_setframe	%rbp, 0
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	leaq	-16(%rbp), %rax
	movq	%rax, %rcx
.LEHB3:
	call	_ZN13DerivedStreamC1Ev
	leaq	-16(%rbp), %rax
	movq	%rax, %rcx
	call	_ZN13DerivedStreamD1Ev
.LEHE3:
.L29:
	movl	r(%rip), %eax
	testl	%eax, %eax
	je	.L27
.LEHB4:
	call	abort
	nop
.L27:
	movl	$0, %ecx
	call	exit
	nop
.L30:
	movq	%rax, %rcx
	call	__cxa_begin_catch
	call	__cxa_end_catch
.LEHE4:
	jmp	.L29
	.seh_handler	_GCC_specific_handler, @unwind, @except
	.seh_handlerdata
	.rva	__gxx_personality_v0
	.text
	.seh_handlerdata
	.align 4
.LLSDA9:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT9-.LLSDATTD9
.LLSDATTD9:
	.byte	0x1
	.uleb128 .LLSDACSE9-.LLSDACSB9
.LLSDACSB9:
	.uleb128 .LEHB3-.LFB9
	.uleb128 .LEHE3-.LEHB3
	.uleb128 .L30-.LFB9
	.uleb128 0x1
	.uleb128 .LEHB4-.LFB9
	.uleb128 .LEHE4-.LEHB4
	.uleb128 0
	.uleb128 0
.LLSDACSE9:
	.byte	0x1
	.byte	0
	.align 4
	.long	0

.LLSDATT9:
	.text
	.seh_endproc
	.globl	_ZTV13DerivedStream
	.section	.data$_ZTV13DerivedStream,"w"
	.linkonce same_size
	.align 32
_ZTV13DerivedStream:
	.quad	8
	.quad	0
	.quad	_ZTI13DerivedStream
	.quad	_ZN13DerivedStreamD1Ev
	.quad	_ZN13DerivedStreamD0Ev
	.quad	0
	.quad	-8
	.quad	_ZTI13DerivedStream
	.quad	_ZN5VBase1fEv
	.globl	_ZTT13DerivedStream
	.section	.data$_ZTT13DerivedStream,"w"
	.linkonce same_size
	.align 32
_ZTT13DerivedStream:
	.quad	_ZTV13DerivedStream+24
	.quad	_ZTC13DerivedStream0_6Stream+24
	.quad	_ZTC13DerivedStream0_6Stream+64
	.quad	_ZTV13DerivedStream+64
	.globl	_ZTC13DerivedStream0_6Stream
	.section	.data$_ZTC13DerivedStream0_6Stream,"w"
	.linkonce same_size
	.align 32
_ZTC13DerivedStream0_6Stream:
	.quad	8
	.quad	0
	.quad	_ZTI6Stream
	.quad	_ZN6StreamD1Ev
	.quad	_ZN6StreamD0Ev
	.quad	0
	.quad	-8
	.quad	_ZTI6Stream
	.quad	_ZN5VBase1fEv
	.globl	_ZTV6Stream
	.section	.data$_ZTV6Stream,"w"
	.linkonce same_size
	.align 32
_ZTV6Stream:
	.quad	8
	.quad	0
	.quad	_ZTI6Stream
	.quad	_ZN6StreamD1Ev
	.quad	_ZN6StreamD0Ev
	.quad	0
	.quad	-8
	.quad	_ZTI6Stream
	.quad	_ZN5VBase1fEv
	.globl	_ZTT6Stream
	.section	.data$_ZTT6Stream,"w"
	.linkonce same_size
	.align 16
_ZTT6Stream:
	.quad	_ZTV6Stream+24
	.quad	_ZTV6Stream+64
	.globl	_ZTV10StreamBase
	.section	.data$_ZTV10StreamBase,"w"
	.linkonce same_size
	.align 32
_ZTV10StreamBase:
	.quad	0
	.quad	_ZTI10StreamBase
	.quad	_ZN10StreamBaseD1Ev
	.quad	_ZN10StreamBaseD0Ev
	.globl	_ZTS13DerivedStream
	.section	.rdata$_ZTS13DerivedStream,"dr"
	.linkonce same_size
	.align 16
_ZTS13DerivedStream:
	.ascii "13DerivedStream\0"
	.globl	_ZTI13DerivedStream
	.section	.data$_ZTI13DerivedStream,"w"
	.linkonce same_size
	.align 16
_ZTI13DerivedStream:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTS13DerivedStream
	.quad	_ZTI6Stream
	.globl	_ZTS6Stream
	.section	.rdata$_ZTS6Stream,"dr"
	.linkonce same_size
_ZTS6Stream:
	.ascii "6Stream\0"
	.globl	_ZTI6Stream
	.section	.data$_ZTI6Stream,"w"
	.linkonce same_size
	.align 32
_ZTI6Stream:
	.quad	_ZTVN10__cxxabiv121__vmi_class_type_infoE+16
	.quad	_ZTS6Stream
	.long	0
	.long	2
	.quad	_ZTI5VBase
	.long	-6141
	.space 4
	.quad	_ZTI10StreamBase
	.long	2
	.space 4
	.globl	_ZTS10StreamBase
	.section	.rdata$_ZTS10StreamBase,"dr"
	.linkonce same_size
_ZTS10StreamBase:
	.ascii "10StreamBase\0"
	.globl	_ZTI10StreamBase
	.section	.data$_ZTI10StreamBase,"w"
	.linkonce same_size
	.align 16
_ZTI10StreamBase:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS10StreamBase
	.globl	_ZTS5VBase
	.section	.rdata$_ZTS5VBase,"dr"
	.linkonce same_size
_ZTS5VBase:
	.ascii "5VBase\0"
	.globl	_ZTI5VBase
	.section	.data$_ZTI5VBase,"w"
	.linkonce same_size
	.align 16
_ZTI5VBase:
	.quad	_ZTVN10__cxxabiv117__class_type_infoE+16
	.quad	_ZTS5VBase
	.def	_ZdlPv;	.scl	2;	.type	32;	.endef
	.def	_ZN5VBaseD2Ev;	.scl	2;	.type	32;	.endef
	.def	_Unwind_Resume;	.scl	2;	.type	32;	.endef
	.def	_ZN13DerivedStreamC1Ev;	.scl	2;	.type	32;	.endef
	.def	abort;	.scl	2;	.type	32;	.endef
	.def	exit;	.scl	2;	.type	32;	.endef
	.def	__cxa_begin_catch;	.scl	2;	.type	32;	.endef
	.def	__cxa_end_catch;	.scl	2;	.type	32;	.endef
