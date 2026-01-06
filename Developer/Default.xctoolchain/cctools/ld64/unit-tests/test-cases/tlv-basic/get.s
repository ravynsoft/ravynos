
		# _a is zerofill global TLV
		.tbss _a$tlv$init,4,2

		# _b is an initialized global TLV
		.tdata
_b$tlv$init:    
		.long   5

		 # _c is zerofill non-external TLV
		.tbss _c$tlv$init,4,2

		# _d is an initialized non-external TLV
		.tdata
_d$tlv$init:   
		.long	5

#if __x86_64__

		# _a is global TLV
		.tlv
		.globl _a
_a:		.quad	__tlv_bootstrap
		.quad	0
		.quad	_a$tlv$init

		# _b is a global TLV
		.tlv
		.globl _b
_b:		.quad	__tlv_bootstrap
		.quad	0
		.quad	_b$tlv$init

		# _c is a non-external TLV
		.tlv
_c:		.quad	__tlv_bootstrap
		.quad	0
		.quad	_c$tlv$init

		# _d is a non-external TLV
		.tlv
_d:		.quad	__tlv_bootstrap
		.quad	0
		.quad	_d$tlv$init


	.text
	.globl	_get_a
_get_a:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	_a@TLVP(%rip), %rdi
	call	*(%rdi)
	popq	%rbp
	ret

	.globl	_get_b
_get_b:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	_b@TLVP(%rip), %rdi
	call	*(%rdi)
	popq	%rbp
	ret
	
	.globl	_get_c
_get_c:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	_c@TLVP(%rip), %rdi
	call	*(%rdi)
	popq	%rbp
	ret

	.globl	_get_d
_get_d:
	pushq	%rbp
	movq	%rsp, %rbp
	movq	_d@TLVP(%rip), %rdi
	call	*(%rdi)
	popq	%rbp
	ret

#endif

#if __i386__

		# _a is global TLV
		.tlv
		.globl _a
_a:		.long	__tlv_bootstrap
		.long	0
		.long	_a$tlv$init

		# _b is a global TLV
		.tlv
		.globl _b
_b:		.long	__tlv_bootstrap
		.long	0
		.long	_b$tlv$init

		# _c is a non-external TLV
		.tlv
_c:		.long	__tlv_bootstrap
		.long	0
		.long	_c$tlv$init

		# _d is a non-external TLV
		.tlv
_d:		.long	__tlv_bootstrap
		.long	0
		.long	_d$tlv$init


	.text
	.globl	_get_a
_get_a:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_a@TLVP, %eax
	call	*(%eax)    
	movl	%ebp, %esp
	popl	%ebp
	ret

	.globl	_get_b
_get_b:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_b@TLVP, %eax
	call	*(%eax)    
	movl	%ebp, %esp
	popl	%ebp
	ret

	.globl	_get_c
_get_c:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_c@TLVP, %eax
	call	*(%eax)    
	movl	%ebp, %esp
	popl	%ebp
	ret

	.globl	_get_d
_get_d:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	_d@TLVP, %eax
	call	*(%eax)    
	movl	%ebp, %esp
	popl	%ebp
	ret

#endif


#if __arm__

		# _a is global TLV
		.tlv
		.globl _a
_a:		.long	__tlv_bootstrap
		.long	0
		.long	_a$tlv$init

		# _b is a global TLV
		.tlv
		.globl _b
_b:		.long	__tlv_bootstrap
		.long	0
		.long	_b$tlv$init

		# _c is a non-external TLV
		.tlv
_c:		.long	__tlv_bootstrap
		.long	0
		.long	_c$tlv$init

		# _d is a non-external TLV
		.tlv
_d:		.long	__tlv_bootstrap
		.long	0
		.long	_d$tlv$init


  .text

  .globl	_get_a
	.align	2
	.code	16
	.thumb_func	_get_a
_get_a:
  push	{r7, lr}
  mov	r7, sp
	movw	r0, :lower16:(La-(LPC0_0+4))
	movt	r0, :upper16:(La-(LPC0_0+4))
LPC0_0:
	add	r0, pc
	ldr	r0, [r0]
	ldr	ip, [r0]
  blx ip
  pop	{r7, pc}


  .globl	_get_b
	.align	2
	.code	16
	.thumb_func	_get_b
_get_b:
  push	{r7, lr}
  mov	r7, sp
	movw	r0, :lower16:(Lb-(LPC0_1+4))
	movt	r0, :upper16:(Lb-(LPC0_1+4))
LPC0_1:
	add	r0, pc
	ldr	r0, [r0]
	ldr	ip, [r0]
  blx ip
  pop	{r7, pc}


  .globl	_get_c
	.align	2
	.code	16
	.thumb_func	_get_c
_get_c:
  push	{r7, lr}
  mov	r7, sp
	movw	r0, :lower16:(Lc-(LPC0_2+4))
	movt	r0, :upper16:(Lc-(LPC0_2+4))
LPC0_2:
	add	r0, pc
	ldr	r0, [r0]
	ldr	ip, [r0]
  blx ip
  pop	{r7, pc}


  .globl	_get_d
	.align	2
	.code	16
	.thumb_func	_get_d
_get_d:
  push	{r7, lr}
  mov	r7, sp
	movw	r0, :lower16:(Ld-(LPC0_3+4))
	movt	r0, :upper16:(Ld-(LPC0_3+4))
LPC0_3:
	add	r0, pc
	ldr	r0, [r0]
	ldr	ip, [r0]
  blx ip
  pop	{r7, pc}


  .section __DATA,__thread_ptr,thread_local_variable_pointers
La:
  .long _a
Lb:
  .long _b
Lc:
  .long _c
Ld:
  .long _d


#endif

.subsections_via_symbols
