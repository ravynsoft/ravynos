	/* Force .data aligned to 4K, so .got very likely gets at 0x102190
	   (0x60 bytes .tdata and 0x130 bytes .dynamic)  */
        .data
        .balign 4096
	.section ".tdata", "awT", @progbits
	.globl sg1, sg2, sg3, sg4, sg5, sg6, sg7, sg8
	.globl sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8
	.hidden sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8
sg1:	.long 17
sg2:	.long 18
sg3:	.long 19
sg4:	.long 20
sg5:	.long 21
sg6:	.long 22
sg7:	.long 23
sg8:	.long 24
sl1:	.long 65
sl2:	.long 66
sl3:	.long 67
sl4:	.long 68
sl5:	.long 69
sl6:	.long 70
sl7:	.long 71
sl8:	.long 72
sh1:	.long 257
sh2:	.long 258
sh3:	.long 259
sh4:	.long 260
sh5:	.long 261
sh6:	.long 262
sh7:	.long 263
sh8:	.long 264
	/* Force .text aligned to 4K, so it very likely gets at 0x1000.  */
	.text
	.balign	4096
	.globl	fn1
	.type	fn1,@function
fn1:
	pushq	%rbp
	movq	%rsp, %rbp
	nop;nop;nop;nop

	/* GD */
	.byte	0x66
	leaq	sg1@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop;nop;nop;nop

	/* GD -> IE because variable is referenced through IE too */
	.byte	0x66
	leaq	sg2@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop;nop;nop;nop

	/* GD against local variable */
	.byte	0x66
	leaq	sl1@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop;nop;nop;nop

	/* GD -> IE against local variable referenced through IE too */
	.byte	0x66
	leaq	sl2@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop;nop;nop;nop

	/* GD against hidden and local variable */
	.byte	0x66
	leaq	sh1@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop;nop;nop;nop

	/* GD -> IE against hidden and local variable referenced through
	   IE too */
	.byte	0x66
	leaq	sh2@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop;nop;nop;nop

	/* GD against hidden but not local variable */
	.byte	0x66
	leaq	sH1@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop;nop;nop;nop

	/* GD -> IE against hidden but not local variable referenced through
	   IE too */
	.byte	0x66
	leaq	sH2@tlsgd(%rip), %rdi
	.byte	0x66
	rex64
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop;nop;nop;nop

	/* LD */
	leaq	sl1@tlsld(%rip), %rdi
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop
	leaq	sl1@dtpoff(%rax), %rdx
	nop;nop
	leaq	2+sl2@dtpoff(%rax), %r9
	nop;nop;nop;nop

	/* LD against hidden and local variables */
	leaq	sh1@tlsld(%rip), %rdi
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop
	leaq	sh1@dtpoff(%rax), %rdx
	nop;nop
	leaq	sh2@dtpoff+3(%rax), %rcx
	nop;nop;nop;nop

	/* LD against hidden but not local variables */
	leaq	sH1@tlsld(%rip), %rdi
	call	*__tls_get_addr@GOTPCREL(%rip)
	nop
	leaq	sH1@dtpoff(%rax), %r12
	nop;nop
	leaq	sH2@dtpoff+1(%rax), %rcx
	nop;nop

	/* IE against global var  */
	movq	%fs:0, %rcx
	nop;nop
	addq	sg2@gottpoff(%rip), %rcx
	nop;nop;nop;nop

	/* IE against local var  */
	movq	%fs:0, %r14
	nop;nop
	addq	sl2@gottpoff(%rip), %r14
	nop;nop;nop;nop

	/* IE against hidden and local var  */
	movq	%fs:0, %rcx
	nop;nop
	addq	sh2@gottpoff(%rip), %rcx
	nop;nop;nop;nop

	/* IE against hidden but not local var  */
	movq	%fs:0, %rcx
	nop;nop
	addq	sH2@gottpoff(%rip), %rcx
	nop;nop;nop;nop

	/* Direct access through %fs  */

	/* IE against global var  */
	movq	sg5@gottpoff(%rip), %rcx
	nop;nop
	movq	%fs:(%rcx), %rdx
	nop;nop;nop;nop

	/* IE against local var  */
	movq	sl5@gottpoff(%rip), %r10
	nop;nop
	movq	%fs:(%r10), %r12
	nop;nop;nop;nop

	/* IE against hidden and local var  */
	movq	sh5@gottpoff(%rip), %rdx
	nop;nop
	movq	%fs:(%rdx), %rdx
	nop;nop;nop;nop

	/* IE against hidden but not local var  */
	movq	sH5@gottpoff(%rip), %rcx
	nop;nop
	movq	%fs:(%rcx), %rdx
	nop;nop;nop;nop

1:	movabsq	$_GLOBAL_OFFSET_TABLE_-1b, %r11
	pushq	%r15
	pushq	%r15
	leaq	1b(%rip), %r15
	addq	%r11, %r15
	nop;nop

	/* -mcmodel=large sequences  */

	/* -mcmodel=large GD  */
	leaq	sg1@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop;nop;nop

	/* -mcmodel=large GD -> IE because variable is referenced through IE too */
	leaq	sg2@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop;nop;nop

	/* -mcmodel=large GD against local variable */
	leaq	sl1@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop;nop;nop

	/* -mcmodel=large GD -> IE against local variable referenced through IE too */
	leaq	sl2@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop;nop;nop

	/* -mcmodel=large GD against hidden and local variable */
	leaq	sh1@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop;nop;nop

	/* -mcmodel=large GD -> IE against hidden and local variable referenced through
	   IE too */
	leaq	sh2@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop;nop;nop

	/* -mcmodel=large GD against hidden but not local variable */
	leaq	sH1@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop;nop;nop

	/* -mcmodel=large GD -> IE against hidden but not local variable referenced through
	   IE too */
	leaq	sH2@tlsgd(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop;nop;nop

	/* -mcmodel=large LD */
	leaq	sl1@tlsld(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop
	leaq	sl1@dtpoff(%rax), %rdx
	nop;nop
	leaq	2+sl2@dtpoff(%rax), %r9
	nop;nop;nop;nop

	/* -mcmodel=large LD against hidden and local variables */
	leaq	sh1@tlsld(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop
	leaq	sh1@dtpoff(%rax), %rdx
	nop;nop
	leaq	sh2@dtpoff+3(%rax), %rcx
	nop;nop;nop;nop

	/* -mcmodel=large LD against hidden but not local variables */
	leaq	sH1@tlsld(%rip), %rdi
	movabsq	$__tls_get_addr@pltoff, %rax
	addq	%r15, %rax
	call	*%rax
	nop;nop
	leaq	sH1@dtpoff(%rax), %r12
	nop;nop
	leaq	sH2@dtpoff+1(%rax), %rcx
	nop;nop

	popq	%r15
	popq	%r15

	leave
	ret
