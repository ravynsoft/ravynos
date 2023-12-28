	.file	"dwp_test_1.cc"
	.text
.Ltext0:
	.section	.text._Z4f13iv,"axG",@progbits,_Z4f13iv,comdat
	.weak	_Z4f13iv
	.type	_Z4f13iv, @function
_Z4f13iv:
.LFB0:
	.file 1 "dwp_test.h"
	.loc 1 70 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 1 70 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	_Z4f13iv, .-_Z4f13iv
	.text
	.align 2
	.globl	_ZN2C19testcase1Ev
	.type	_ZN2C19testcase1Ev, @function
_ZN2C19testcase1Ev:
.LFB1:
	.file 2 "dwp_test_1.cc"
	.loc 2 31 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	.loc 2 32 0
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C14t1_2Ev
	cmpl	$123, %eax
	sete	%al
	.loc 2 33 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	_ZN2C19testcase1Ev, .-_ZN2C19testcase1Ev
	.align 2
	.globl	_ZN2C19testcase2Ev
	.type	_ZN2C19testcase2Ev, @function
_ZN2C19testcase2Ev:
.LFB2:
	.loc 2 39 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 40 0
	movl	v2(%rip), %eax
	cmpl	$456, %eax
	sete	%al
	.loc 2 41 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	_ZN2C19testcase2Ev, .-_ZN2C19testcase2Ev
	.align 2
	.globl	_ZN2C19testcase3Ev
	.type	_ZN2C19testcase3Ev, @function
_ZN2C19testcase3Ev:
.LFB3:
	.loc 2 47 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 48 0
	movl	v3(%rip), %eax
	cmpl	$789, %eax
	sete	%al
	.loc 2 49 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	_ZN2C19testcase3Ev, .-_ZN2C19testcase3Ev
	.align 2
	.globl	_ZN2C19testcase4Ev
	.type	_ZN2C19testcase4Ev, @function
_ZN2C19testcase4Ev:
.LFB4:
	.loc 2 55 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 56 0
	movzbl	v4+5(%rip), %eax
	cmpb	$44, %al
	sete	%al
	.loc 2 57 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	_ZN2C19testcase4Ev, .-_ZN2C19testcase4Ev
	.align 2
	.globl	_ZN2C29testcase1Ev
	.type	_ZN2C29testcase1Ev, @function
_ZN2C29testcase1Ev:
.LFB5:
	.loc 2 63 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 64 0
	movzbl	v5+7(%rip), %eax
	cmpb	$119, %al
	sete	%al
	.loc 2 65 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	_ZN2C29testcase1Ev, .-_ZN2C29testcase1Ev
	.globl	p6
	.data
	.align 8
	.type	p6, @object
	.size	p6, 8
p6:
	.quad	v2
	.text
	.align 2
	.globl	_ZN2C29testcase2Ev
	.type	_ZN2C29testcase2Ev, @function
_ZN2C29testcase2Ev:
.LFB6:
	.loc 2 73 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 74 0
	movq	p6(%rip), %rax
	movl	(%rax), %eax
	cmpl	$456, %eax
	sete	%al
	.loc 2 75 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	_ZN2C29testcase2Ev, .-_ZN2C29testcase2Ev
	.globl	p7
	.data
	.align 8
	.type	p7, @object
	.size	p7, 8
p7:
	.quad	v3
	.text
	.align 2
	.globl	_ZN2C29testcase3Ev
	.type	_ZN2C29testcase3Ev, @function
_ZN2C29testcase3Ev:
.LFB7:
	.loc 2 83 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 84 0
	movq	p7(%rip), %rax
	movl	(%rax), %eax
	cmpl	$789, %eax
	sete	%al
	.loc 2 85 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	_ZN2C29testcase3Ev, .-_ZN2C29testcase3Ev
	.globl	p8
	.data
	.align 8
	.type	p8, @object
	.size	p8, 8
p8:
	.quad	v4+6
	.text
	.align 2
	.globl	_ZN2C29testcase4Ev
	.type	_ZN2C29testcase4Ev, @function
_ZN2C29testcase4Ev:
.LFB8:
	.loc 2 93 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 94 0
	movq	p8(%rip), %rax
	movzbl	(%rax), %eax
	cmpb	$32, %al
	sete	%al
	.loc 2 95 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	_ZN2C29testcase4Ev, .-_ZN2C29testcase4Ev
	.globl	p9
	.data
	.align 8
	.type	p9, @object
	.size	p9, 8
p9:
	.quad	v5+8
	.text
	.align 2
	.globl	_ZN2C39testcase1Ev
	.type	_ZN2C39testcase1Ev, @function
_ZN2C39testcase1Ev:
.LFB9:
	.loc 2 103 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 104 0
	movq	p9(%rip), %rax
	movzbl	(%rax), %eax
	cmpb	$111, %al
	sete	%al
	.loc 2 105 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	_ZN2C39testcase1Ev, .-_ZN2C39testcase1Ev
	.globl	pfn
	.data
	.align 8
	.type	pfn, @object
	.size	pfn, 8
pfn:
	.quad	_Z3f10v
	.text
	.align 2
	.globl	_ZN2C39testcase2Ev
	.type	_ZN2C39testcase2Ev, @function
_ZN2C39testcase2Ev:
.LFB10:
	.loc 2 113 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	.loc 2 114 0
	movq	pfn(%rip), %rax
	call	*%rax
	cmpl	$135, %eax
	sete	%al
	.loc 2 115 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	_ZN2C39testcase2Ev, .-_ZN2C39testcase2Ev
	.globl	_Z4f11av
	.type	_Z4f11av, @function
_Z4f11av:
.LFB11:
	.loc 2 121 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 122 0
	movl	$246, %eax
	.loc 2 123 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	_Z4f11av, .-_Z4f11av
	.align 2
	.globl	_ZN2C39testcase3Ev
	.type	_ZN2C39testcase3Ev, @function
_ZN2C39testcase3Ev:
.LFB12:
	.loc 2 127 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	.loc 2 128 0
	movl	$_Z4f11av, %edi
	call	_Z4f11bPFivE
	cmpl	$246, %eax
	sete	%al
	.loc 2 129 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	_ZN2C39testcase3Ev, .-_ZN2C39testcase3Ev
	.globl	_Z3t12v
	.type	_Z3t12v, @function
_Z3t12v:
.LFB13:
	.loc 2 135 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 136 0
	movl	$c3, %edi
	call	_ZN2C32f4Ev
	cmpq	$_Z3t12v, %rax
	sete	%al
	.loc 2 137 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	_Z3t12v, .-_Z3t12v
	.globl	_Z3t13v
	.type	_Z3t13v, @function
_Z3t13v:
.LFB14:
	.loc 2 143 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 144 0
	call	_Z3f13v
	cmpq	$_Z4f13iv, %rax
	sete	%al
	.loc 2 145 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE14:
	.size	_Z3t13v, .-_Z3t13v
	.section	.rodata
.LC0:
	.string	"test string constant"
	.text
	.globl	_Z3t14v
	.type	_Z3t14v, @function
_Z3t14v:
.LFB15:
	.loc 2 151 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
.LBB2:
	.loc 2 152 0
	movq	$.LC0, -8(%rbp)
	.loc 2 153 0
	call	_Z3f14v
	movq	%rax, -16(%rbp)
	.loc 2 154 0
	jmp	.L31
.L33:
	.loc 2 155 0
	movq	-8(%rbp), %rax
	movzbl	(%rax), %edx
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	%al, %dl
	setne	%al
	addq	$1, -8(%rbp)
	addq	$1, -16(%rbp)
	testb	%al, %al
	je	.L31
	.loc 2 156 0
	movl	$0, %eax
	jmp	.L32
.L31:
	.loc 2 154 0 discriminator 1
	movq	-8(%rbp), %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	setne	%al
	testb	%al, %al
	jne	.L33
	.loc 2 157 0
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	sete	%al
.L32:
.LBE2:
	.loc 2 158 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE15:
	.size	_Z3t14v, .-_Z3t14v
	.section	.rodata
	.align 8
.LC1:
	.string	"t"
	.string	""
	.string	""
	.string	"e"
	.string	""
	.string	""
	.string	"s"
	.string	""
	.string	""
	.string	"t"
	.string	""
	.string	""
	.string	" "
	.string	""
	.string	""
	.string	"w"
	.string	""
	.string	""
	.string	"i"
	.string	""
	.string	""
	.string	"d"
	.string	""
	.string	""
	.string	"e"
	.string	""
	.string	""
	.string	" "
	.string	""
	.string	""
	.string	"s"
	.string	""
	.string	""
	.string	"t"
	.string	""
	.string	""
	.string	"r"
	.string	""
	.string	""
	.string	"i"
	.string	""
	.string	""
	.string	"n"
	.string	""
	.string	""
	.string	"g"
	.string	""
	.string	""
	.string	" "
	.string	""
	.string	""
	.string	"c"
	.string	""
	.string	""
	.string	"o"
	.string	""
	.string	""
	.string	"n"
	.string	""
	.string	""
	.string	"s"
	.string	""
	.string	""
	.string	"t"
	.string	""
	.string	""
	.string	"a"
	.string	""
	.string	""
	.string	"n"
	.string	""
	.string	""
	.string	"t"
	.string	""
	.string	""
	.string	""
	.string	""
	.string	""
	.string	""
	.text
	.globl	_Z3t15v
	.type	_Z3t15v, @function
_Z3t15v:
.LFB16:
	.loc 2 164 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
.LBB3:
	.loc 2 165 0
	movq	$.LC1, -8(%rbp)
	.loc 2 166 0
	call	_Z3f15v
	movq	%rax, -16(%rbp)
	.loc 2 167 0
	jmp	.L35
.L37:
	.loc 2 168 0
	movq	-8(%rbp), %rax
	movl	(%rax), %edx
	movq	-16(%rbp), %rax
	movl	(%rax), %eax
	cmpl	%eax, %edx
	setne	%al
	addq	$4, -8(%rbp)
	addq	$4, -16(%rbp)
	testb	%al, %al
	je	.L35
	.loc 2 169 0
	movl	$0, %eax
	jmp	.L36
.L35:
	.loc 2 167 0 discriminator 1
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	testl	%eax, %eax
	setne	%al
	testb	%al, %al
	jne	.L37
	.loc 2 170 0
	movq	-16(%rbp), %rax
	movl	(%rax), %eax
	testl	%eax, %eax
	sete	%al
.L36:
.LBE3:
	.loc 2 171 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE16:
	.size	_Z3t15v, .-_Z3t15v
	.globl	_Z3t16v
	.type	_Z3t16v, @function
_Z3t16v:
.LFB17:
	.loc 2 177 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 178 0
	call	_Z3f10v
	cmpl	$135, %eax
	sete	%al
	.loc 2 179 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE17:
	.size	_Z3t16v, .-_Z3t16v
	.globl	_Z3t17v
	.type	_Z3t17v, @function
_Z3t17v:
.LFB18:
	.loc 2 185 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
.LBB4:
	.loc 2 186 0
	movb	$97, -1(%rbp)
.LBB5:
	.loc 2 187 0
	movl	$0, -8(%rbp)
	jmp	.L41
.L45:
	.loc 2 189 0
	movl	-8(%rbp), %eax
	cltq
	movq	t17data(,%rax,8), %rax
	movzbl	(%rax), %eax
	cmpb	-1(%rbp), %al
	jne	.L42
	.loc 2 189 0 is_stmt 0 discriminator 1
	movl	-8(%rbp), %eax
	cltq
	movq	t17data(,%rax,8), %rax
	addq	$1, %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	je	.L43
.L42:
	.loc 2 190 0 is_stmt 1
	movl	$0, %eax
	jmp	.L44
.L43:
	.loc 2 191 0
	addb	$1, -1(%rbp)
	.loc 2 187 0
	addl	$1, -8(%rbp)
.L41:
	.loc 2 187 0 is_stmt 0 discriminator 1
	cmpl	$4, -8(%rbp)
	setle	%al
	testb	%al, %al
	jne	.L45
.LBE5:
	.loc 2 193 0 is_stmt 1
	movl	$1, %eax
.L44:
.LBE4:
	.loc 2 194 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE18:
	.size	_Z3t17v, .-_Z3t17v
	.globl	_Z3t18v
	.type	_Z3t18v, @function
_Z3t18v:
.LFB19:
	.loc 2 200 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
.LBB6:
	.loc 2 201 0
	movb	$97, -1(%rbp)
.LBB7:
	.loc 2 202 0
	movl	$0, -8(%rbp)
	jmp	.L47
.L51:
.LBB8:
	.loc 2 204 0
	movl	-8(%rbp), %eax
	movl	%eax, %edi
	call	_Z3f18i
	movq	%rax, -16(%rbp)
	.loc 2 205 0
	movq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	cmpb	-1(%rbp), %al
	jne	.L48
	.loc 2 205 0 is_stmt 0 discriminator 1
	movq	-16(%rbp), %rax
	addq	$1, %rax
	movzbl	(%rax), %eax
	testb	%al, %al
	je	.L49
.L48:
	.loc 2 206 0 is_stmt 1
	movl	$0, %eax
	jmp	.L50
.L49:
	.loc 2 207 0
	addb	$1, -1(%rbp)
.LBE8:
	.loc 2 202 0
	addl	$1, -8(%rbp)
.L47:
	.loc 2 202 0 is_stmt 0 discriminator 1
	cmpl	$4, -8(%rbp)
	setle	%al
	testb	%al, %al
	jne	.L51
.LBE7:
	.loc 2 209 0 is_stmt 1
	movl	$1, %eax
.L50:
.LBE6:
	.loc 2 210 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE19:
	.size	_Z3t18v, .-_Z3t18v
.Letext0:
	.section	.debug_types.dwo,"G",@progbits,wt.bb2916f0c1bd34b5,comdat
	.long	0xc1
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.byte	0xbb
	.byte	0x29
	.byte	0x16
	.byte	0xf0
	.byte	0xc1
	.byte	0xbd
	.byte	0x34
	.byte	0xb5
	.long	0x25
	.uleb128 0x1
	.byte	0x4
	.byte	0x8a
	.byte	0xda
	.byte	0x59
	.byte	0x6e
	.byte	0x4d
	.byte	0x5c
	.byte	0xa
	.byte	0x88
	.long	.Lskeleton_debug_line0
	.uleb128 0x2
	.string	"C3"
	.byte	0x4
	.byte	0x1
	.byte	0x2f
	.long	0xa4
	.uleb128 0x3
	.uleb128 0x6
	.byte	0x1
	.byte	0x36
	.long	0xa4
	.byte	0
	.byte	0x1
	.uleb128 0x4
	.uleb128 0
	.byte	0x1
	.byte	0x32
	.uleb128 0x2
	.long	0xab
	.byte	0x1
	.long	0x4c
	.long	0x52
	.uleb128 0x5
	.long	0xb3
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x1
	.byte	0x33
	.uleb128 0x3
	.long	0xab
	.byte	0x1
	.long	0x64
	.long	0x6a
	.uleb128 0x5
	.long	0xb3
	.byte	0
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x1
	.byte	0x34
	.uleb128 0x5
	.long	0xab
	.byte	0x1
	.long	0x7c
	.long	0x82
	.uleb128 0x5
	.long	0xb3
	.byte	0
	.uleb128 0x6
	.string	"f4"
	.byte	0x1
	.byte	0x35
	.string	"_ZN2C32f4Ev"
	.long	0xb9
	.byte	0x1
	.long	0x9d
	.uleb128 0x5
	.long	0xb3
	.byte	0
	.byte	0
	.uleb128 0x7
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x7
	.byte	0x1
	.byte	0x2
	.string	"bool"
	.uleb128 0x8
	.byte	0x8
	.long	0x25
	.uleb128 0x8
	.byte	0x8
	.long	0xbf
	.uleb128 0x9
	.long	0xab
	.byte	0
	.section	.debug_types,"G",@progbits,wt.bb2916f0c1bd34b5,comdat
	.long	0x6e
	.value	0x4
	.long	.Lskeleton_debug_abbrev0
	.byte	0x8
	.byte	0xbb
	.byte	0x29
	.byte	0x16
	.byte	0xf0
	.byte	0xc1
	.byte	0xbd
	.byte	0x34
	.byte	0xb5
	.long	0
	.uleb128 0x2
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.string	"dwp_test_1.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_types.dwo,"G",@progbits,wt.66526f88bcc798ab,comdat
	.long	0xa9
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.byte	0x66
	.byte	0x52
	.byte	0x6f
	.byte	0x88
	.byte	0xbc
	.byte	0xc7
	.byte	0x98
	.byte	0xab
	.long	0x25
	.uleb128 0x1
	.byte	0x4
	.byte	0x4b
	.byte	0xf9
	.byte	0xce
	.byte	0xbf
	.byte	0xd8
	.byte	0xf0
	.byte	0x4a
	.byte	0xae
	.long	.Lskeleton_debug_line0
	.uleb128 0x2
	.string	"C2"
	.byte	0x4
	.byte	0x1
	.byte	0x25
	.long	0x97
	.uleb128 0x3
	.uleb128 0x6
	.byte	0x1
	.byte	0x2c
	.long	0x97
	.byte	0
	.byte	0x1
	.uleb128 0x4
	.uleb128 0
	.byte	0x1
	.byte	0x28
	.uleb128 0x7
	.long	0x9e
	.byte	0x1
	.long	0x4c
	.long	0x52
	.uleb128 0x5
	.long	0xa6
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x1
	.byte	0x29
	.uleb128 0x8
	.long	0x9e
	.byte	0x1
	.long	0x64
	.long	0x6a
	.uleb128 0x5
	.long	0xa6
	.byte	0
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x1
	.byte	0x2a
	.uleb128 0x9
	.long	0x9e
	.byte	0x1
	.long	0x7c
	.long	0x82
	.uleb128 0x5
	.long	0xa6
	.byte	0
	.uleb128 0xa
	.uleb128 0xa
	.byte	0x1
	.byte	0x2b
	.uleb128 0xb
	.long	0x9e
	.byte	0x1
	.long	0x90
	.uleb128 0x5
	.long	0xa6
	.byte	0
	.byte	0
	.uleb128 0x7
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x7
	.byte	0x1
	.byte	0x2
	.string	"bool"
	.uleb128 0x8
	.byte	0x8
	.long	0x25
	.byte	0
	.section	.debug_types,"G",@progbits,wt.66526f88bcc798ab,comdat
	.long	0x6e
	.value	0x4
	.long	.Lskeleton_debug_abbrev0
	.byte	0x8
	.byte	0x66
	.byte	0x52
	.byte	0x6f
	.byte	0x88
	.byte	0xbc
	.byte	0xc7
	.byte	0x98
	.byte	0xab
	.long	0
	.uleb128 0x2
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.string	"dwp_test_1.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_types.dwo,"G",@progbits,wt.c419a9b7a4a2fab5,comdat
	.long	0xf9
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.byte	0xc4
	.byte	0x19
	.byte	0xa9
	.byte	0xb7
	.byte	0xa4
	.byte	0xa2
	.byte	0xfa
	.byte	0xb5
	.long	0x25
	.uleb128 0x1
	.byte	0x4
	.byte	0xe3
	.byte	0xad
	.byte	0x5
	.byte	0x3b
	.byte	0x75
	.byte	0xeb
	.byte	0xfb
	.byte	0xc7
	.long	.Lskeleton_debug_line0
	.uleb128 0x2
	.string	"C1"
	.byte	0x4
	.byte	0x1
	.byte	0x19
	.long	0xe7
	.uleb128 0x3
	.uleb128 0x6
	.byte	0x1
	.byte	0x22
	.long	0xe7
	.byte	0
	.byte	0x1
	.uleb128 0x4
	.uleb128 0
	.byte	0x1
	.byte	0x1c
	.uleb128 0xc
	.long	0xee
	.byte	0x1
	.long	0x4c
	.long	0x52
	.uleb128 0x5
	.long	0xf6
	.byte	0
	.uleb128 0xb
	.string	"t1a"
	.byte	0x1
	.byte	0x1d
	.string	"_ZN2C13t1aEv"
	.long	0xee
	.byte	0x1
	.long	0x73
	.long	0x79
	.uleb128 0x5
	.long	0xf6
	.byte	0
	.uleb128 0xb
	.string	"t1_2"
	.byte	0x1
	.byte	0x1e
	.string	"_ZN2C14t1_2Ev"
	.long	0xe7
	.byte	0x1
	.long	0x9c
	.long	0xa2
	.uleb128 0x5
	.long	0xf6
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x1
	.byte	0x1f
	.uleb128 0xd
	.long	0xee
	.byte	0x1
	.long	0xb4
	.long	0xba
	.uleb128 0x5
	.long	0xf6
	.byte	0
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x1
	.byte	0x20
	.uleb128 0xe
	.long	0xee
	.byte	0x1
	.long	0xcc
	.long	0xd2
	.uleb128 0x5
	.long	0xf6
	.byte	0
	.uleb128 0xa
	.uleb128 0xa
	.byte	0x1
	.byte	0x21
	.uleb128 0xf
	.long	0xee
	.byte	0x1
	.long	0xe0
	.uleb128 0x5
	.long	0xf6
	.byte	0
	.byte	0
	.uleb128 0x7
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x7
	.byte	0x1
	.byte	0x2
	.string	"bool"
	.uleb128 0x8
	.byte	0x8
	.long	0x25
	.byte	0
	.section	.debug_types,"G",@progbits,wt.c419a9b7a4a2fab5,comdat
	.long	0x6e
	.value	0x4
	.long	.Lskeleton_debug_abbrev0
	.byte	0x8
	.byte	0xc4
	.byte	0x19
	.byte	0xa9
	.byte	0xb7
	.byte	0xa4
	.byte	0xa2
	.byte	0xfa
	.byte	0xb5
	.long	0
	.uleb128 0x2
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.string	"dwp_test_1.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_info.dwo,"e",@progbits
.Ldebug_info0:
	.long	0x5af
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0xc
	.string	"GNU C++ 4.7.x-google 20120720 (prerelease)"
	.byte	0x4
	.string	"dwp_test_1.cc"
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.byte	0x27
	.byte	0x37
	.byte	0xdc
	.byte	0x2f
	.byte	0x9
	.byte	0xc6
	.byte	0xf9
	.byte	0x52
	.uleb128 0xd
	.string	"C1"
	.byte	0xc4
	.byte	0x19
	.byte	0xa9
	.byte	0xb7
	.byte	0xa4
	.byte	0xa2
	.byte	0xfa
	.byte	0xb5
	.long	0xc6
	.uleb128 0xe
	.uleb128 0
	.byte	0x1
	.byte	0x1c
	.uleb128 0xc
	.long	0xcd
	.byte	0x1
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x1
	.byte	0x1f
	.uleb128 0xd
	.long	0xcd
	.byte	0x1
	.uleb128 0xe
	.uleb128 0x4
	.byte	0x1
	.byte	0x20
	.uleb128 0xe
	.long	0xcd
	.byte	0x1
	.uleb128 0xe
	.uleb128 0xa
	.byte	0x1
	.byte	0x21
	.uleb128 0xf
	.long	0xcd
	.byte	0x1
	.byte	0
	.uleb128 0x7
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x7
	.byte	0x1
	.byte	0x2
	.string	"bool"
	.uleb128 0xf
	.byte	0x8
	.byte	0xc4
	.byte	0x19
	.byte	0xa9
	.byte	0xb7
	.byte	0xa4
	.byte	0xa2
	.byte	0xfa
	.byte	0xb5
	.uleb128 0xd
	.string	"C2"
	.byte	0x66
	.byte	0x52
	.byte	0x6f
	.byte	0x88
	.byte	0xbc
	.byte	0xc7
	.byte	0x98
	.byte	0xab
	.long	0x118
	.uleb128 0xe
	.uleb128 0
	.byte	0x1
	.byte	0x28
	.uleb128 0x7
	.long	0xcd
	.byte	0x1
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x1
	.byte	0x29
	.uleb128 0x8
	.long	0xcd
	.byte	0x1
	.uleb128 0xe
	.uleb128 0x4
	.byte	0x1
	.byte	0x2a
	.uleb128 0x9
	.long	0xcd
	.byte	0x1
	.uleb128 0xe
	.uleb128 0xa
	.byte	0x1
	.byte	0x2b
	.uleb128 0xb
	.long	0xcd
	.byte	0x1
	.byte	0
	.uleb128 0xf
	.byte	0x8
	.byte	0x66
	.byte	0x52
	.byte	0x6f
	.byte	0x88
	.byte	0xbc
	.byte	0xc7
	.byte	0x98
	.byte	0xab
	.uleb128 0xd
	.string	"C3"
	.byte	0xbb
	.byte	0x29
	.byte	0x16
	.byte	0xf0
	.byte	0xc1
	.byte	0xbd
	.byte	0x34
	.byte	0xb5
	.long	0x151
	.uleb128 0xe
	.uleb128 0
	.byte	0x1
	.byte	0x32
	.uleb128 0x2
	.long	0xcd
	.byte	0x1
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x1
	.byte	0x33
	.uleb128 0x3
	.long	0xcd
	.byte	0x1
	.uleb128 0xe
	.uleb128 0x4
	.byte	0x1
	.byte	0x34
	.uleb128 0x5
	.long	0xcd
	.byte	0x1
	.byte	0
	.uleb128 0xf
	.byte	0x8
	.byte	0xbb
	.byte	0x29
	.byte	0x16
	.byte	0xf0
	.byte	0xc1
	.byte	0xbd
	.byte	0x34
	.byte	0xb5
	.uleb128 0x10
	.string	"f13i"
	.byte	0x1
	.byte	0x46
	.string	"_Z4f13iv"
	.uleb128 0
	.quad	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x11
	.long	0x9d
	.byte	0x2
	.byte	0x1e
	.uleb128 0x1
	.quad	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x191
	.long	0x19b
	.uleb128 0x12
	.uleb128 0x10
	.long	0x19b
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x13
	.long	0xd5
	.uleb128 0x14
	.long	0xa7
	.byte	0x2
	.byte	0x26
	.uleb128 0x2
	.quad	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x1ba
	.long	0x1c4
	.uleb128 0x12
	.uleb128 0x10
	.long	0x19b
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x14
	.long	0xb1
	.byte	0x2
	.byte	0x2e
	.uleb128 0x3
	.quad	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.long	0x1de
	.long	0x1e8
	.uleb128 0x12
	.uleb128 0x10
	.long	0x19b
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x14
	.long	0xbb
	.byte	0x2
	.byte	0x36
	.uleb128 0x4
	.quad	.LFE4-.LFB4
	.uleb128 0x1
	.byte	0x9c
	.long	0x202
	.long	0x20c
	.uleb128 0x12
	.uleb128 0x10
	.long	0x19b
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x14
	.long	0xef
	.byte	0x2
	.byte	0x3e
	.uleb128 0x5
	.quad	.LFE5-.LFB5
	.uleb128 0x1
	.byte	0x9c
	.long	0x226
	.long	0x230
	.uleb128 0x12
	.uleb128 0x10
	.long	0x230
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x13
	.long	0x118
	.uleb128 0x14
	.long	0xf9
	.byte	0x2
	.byte	0x48
	.uleb128 0x6
	.quad	.LFE6-.LFB6
	.uleb128 0x1
	.byte	0x9c
	.long	0x24f
	.long	0x259
	.uleb128 0x12
	.uleb128 0x10
	.long	0x230
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x14
	.long	0x103
	.byte	0x2
	.byte	0x52
	.uleb128 0x7
	.quad	.LFE7-.LFB7
	.uleb128 0x1
	.byte	0x9c
	.long	0x273
	.long	0x27d
	.uleb128 0x12
	.uleb128 0x10
	.long	0x230
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x14
	.long	0x10d
	.byte	0x2
	.byte	0x5c
	.uleb128 0x8
	.quad	.LFE8-.LFB8
	.uleb128 0x1
	.byte	0x9c
	.long	0x297
	.long	0x2a1
	.uleb128 0x12
	.uleb128 0x10
	.long	0x230
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x14
	.long	0x132
	.byte	0x2
	.byte	0x66
	.uleb128 0x9
	.quad	.LFE9-.LFB9
	.uleb128 0x1
	.byte	0x9c
	.long	0x2bb
	.long	0x2c5
	.uleb128 0x12
	.uleb128 0x10
	.long	0x2c5
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x13
	.long	0x151
	.uleb128 0x11
	.long	0x13c
	.byte	0x2
	.byte	0x70
	.uleb128 0xa
	.quad	.LFE10-.LFB10
	.uleb128 0x1
	.byte	0x9c
	.long	0x2e4
	.long	0x2ee
	.uleb128 0x12
	.uleb128 0x10
	.long	0x2c5
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x15
	.string	"f11a"
	.byte	0x2
	.byte	0x78
	.string	"_Z4f11av"
	.long	0xc6
	.uleb128 0xb
	.quad	.LFE11-.LFB11
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x11
	.long	0x146
	.byte	0x2
	.byte	0x7e
	.uleb128 0xc
	.quad	.LFE12-.LFB12
	.uleb128 0x1
	.byte	0x9c
	.long	0x328
	.long	0x332
	.uleb128 0x12
	.uleb128 0x10
	.long	0x2c5
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x16
	.string	"t12"
	.byte	0x2
	.byte	0x86
	.string	"_Z3t12v"
	.long	0xcd
	.uleb128 0xd
	.quad	.LFE13-.LFB13
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x16
	.string	"t13"
	.byte	0x2
	.byte	0x8e
	.string	"_Z3t13v"
	.long	0xcd
	.uleb128 0xe
	.quad	.LFE14-.LFB14
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x17
	.string	"t14"
	.byte	0x2
	.byte	0x96
	.string	"_Z3t14v"
	.long	0xcd
	.uleb128 0xf
	.quad	.LFE15-.LFB15
	.uleb128 0x1
	.byte	0x9c
	.long	0x3b6
	.uleb128 0x18
	.uleb128 0x10
	.quad	.LBE2-.LBB2
	.uleb128 0x19
	.string	"s1"
	.byte	0x2
	.byte	0x98
	.long	0x3b6
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x19
	.string	"s2"
	.byte	0x2
	.byte	0x99
	.long	0x3b6
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.byte	0
	.uleb128 0x8
	.byte	0x8
	.long	0x3bc
	.uleb128 0x13
	.long	0x3c1
	.uleb128 0x7
	.byte	0x1
	.byte	0x6
	.string	"char"
	.uleb128 0x17
	.string	"t15"
	.byte	0x2
	.byte	0xa3
	.string	"_Z3t15v"
	.long	0xcd
	.uleb128 0x11
	.quad	.LFE16-.LFB16
	.uleb128 0x1
	.byte	0x9c
	.long	0x411
	.uleb128 0x18
	.uleb128 0x12
	.quad	.LBE3-.LBB3
	.uleb128 0x19
	.string	"s1"
	.byte	0x2
	.byte	0xa5
	.long	0x411
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x19
	.string	"s2"
	.byte	0x2
	.byte	0xa6
	.long	0x411
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.byte	0
	.uleb128 0x8
	.byte	0x8
	.long	0x417
	.uleb128 0x13
	.long	0x41c
	.uleb128 0x7
	.byte	0x4
	.byte	0x5
	.string	"wchar_t"
	.uleb128 0x16
	.string	"t16"
	.byte	0x2
	.byte	0xb0
	.string	"_Z3t16v"
	.long	0xcd
	.uleb128 0x13
	.quad	.LFE17-.LFB17
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x1a
	.string	"t17"
	.byte	0x2
	.byte	0xb8
	.string	"_Z3t17v"
	.long	0xcd
	.uleb128 0x14
	.quad	.LFE18-.LFB18
	.uleb128 0x1
	.byte	0x9c
	.long	0x496
	.uleb128 0x18
	.uleb128 0x15
	.quad	.LBE4-.LBB4
	.uleb128 0x19
	.string	"c"
	.byte	0x2
	.byte	0xba
	.long	0x3c1
	.uleb128 0x2
	.byte	0x91
	.sleb128 -17
	.uleb128 0x18
	.uleb128 0x16
	.quad	.LBE5-.LBB5
	.uleb128 0x19
	.string	"i"
	.byte	0x2
	.byte	0xbb
	.long	0xc6
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x17
	.string	"t18"
	.byte	0x2
	.byte	0xc7
	.string	"_Z3t18v"
	.long	0xcd
	.uleb128 0x17
	.quad	.LFE19-.LFB19
	.uleb128 0x1
	.byte	0x9c
	.long	0x4fe
	.uleb128 0x18
	.uleb128 0x18
	.quad	.LBE6-.LBB6
	.uleb128 0x19
	.string	"c"
	.byte	0x2
	.byte	0xc9
	.long	0x3c1
	.uleb128 0x2
	.byte	0x91
	.sleb128 -17
	.uleb128 0x18
	.uleb128 0x19
	.quad	.LBE7-.LBB7
	.uleb128 0x19
	.string	"i"
	.byte	0x2
	.byte	0xca
	.long	0xc6
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.uleb128 0x18
	.uleb128 0x1a
	.quad	.LBE8-.LBB8
	.uleb128 0x19
	.string	"s"
	.byte	0x2
	.byte	0xcc
	.long	0x3b6
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x1b
	.string	"c3"
	.byte	0x1
	.byte	0x39
	.byte	0xbb
	.byte	0x29
	.byte	0x16
	.byte	0xf0
	.byte	0xc1
	.byte	0xbd
	.byte	0x34
	.byte	0xb5
	.uleb128 0x1c
	.string	"v2"
	.byte	0x1
	.byte	0x3b
	.long	0xc6
	.uleb128 0x1c
	.string	"v3"
	.byte	0x1
	.byte	0x3c
	.long	0xc6
	.uleb128 0x1d
	.long	0x3c1
	.long	0x52b
	.uleb128 0x1e
	.byte	0
	.uleb128 0x1c
	.string	"v4"
	.byte	0x1
	.byte	0x3d
	.long	0x520
	.uleb128 0x1c
	.string	"v5"
	.byte	0x1
	.byte	0x3e
	.long	0x520
	.uleb128 0x1d
	.long	0x3b6
	.long	0x54a
	.uleb128 0x1e
	.byte	0
	.uleb128 0x1c
	.string	"t17data"
	.byte	0x1
	.byte	0x53
	.long	0x53f
	.uleb128 0x1f
	.string	"p6"
	.byte	0x2
	.byte	0x45
	.long	0x566
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0x1b
	.uleb128 0x8
	.byte	0x8
	.long	0xc6
	.uleb128 0x1f
	.string	"p7"
	.byte	0x2
	.byte	0x4f
	.long	0x566
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0x1c
	.uleb128 0x1f
	.string	"p8"
	.byte	0x2
	.byte	0x59
	.long	0x586
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0x1d
	.uleb128 0x8
	.byte	0x8
	.long	0x3c1
	.uleb128 0x1f
	.string	"p9"
	.byte	0x2
	.byte	0x63
	.long	0x586
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0x1e
	.uleb128 0x9
	.long	0xc6
	.uleb128 0x1f
	.string	"pfn"
	.byte	0x2
	.byte	0x6d
	.long	0x5ac
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0x1f
	.uleb128 0x8
	.byte	0x8
	.long	0x599
	.byte	0
	.section	.debug_info,"",@progbits
.Lskeleton_debug_info0:
	.long	0x7e
	.value	0x4
	.long	.Lskeleton_debug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.Ldebug_ranges0+0
	.quad	0
	.long	.Ldebug_line0
	.byte	0x27
	.byte	0x37
	.byte	0xdc
	.byte	0x2f
	.byte	0x9
	.byte	0xc6
	.byte	0xf9
	.byte	0x52
	.long	.Ldebug_ranges0
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.string	"dwp_test_1.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_abbrev,"",@progbits
.Lskeleton_debug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0
	.uleb128 0x55
	.uleb128 0x17
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x17
	.uleb128 0x2131
	.uleb128 0x7
	.uleb128 0x2132
	.uleb128 0x17
	.uleb128 0x1b
	.uleb128 0x8
	.uleb128 0x2130
	.uleb128 0x8
	.uleb128 0x2134
	.uleb128 0x17
	.uleb128 0x2135
	.uleb128 0x17
	.uleb128 0x2133
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x41
	.byte	0
	.uleb128 0x1b
	.uleb128 0x8
	.uleb128 0x2130
	.uleb128 0x8
	.uleb128 0x2134
	.uleb128 0x17
	.uleb128 0x2135
	.uleb128 0x17
	.uleb128 0x2133
	.uleb128 0x17
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_abbrev.dwo,"e",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x41
	.byte	0x1
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x210f
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x2
	.byte	0x1
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0x1f02
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.uleb128 0x32
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x1f02
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x1f02
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x32
	.uleb128 0xb
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x64
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x34
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x8
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x32
	.uleb128 0xb
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x64
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x15
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x1f02
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x1f02
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x32
	.uleb128 0xb
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x64
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x8
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x32
	.uleb128 0xb
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x64
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0x8
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x1b
	.uleb128 0x8
	.uleb128 0x2131
	.uleb128 0x7
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2
	.byte	0x1
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x69
	.uleb128 0x20
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x1f02
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x1f02
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x32
	.uleb128 0xb
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xf
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x20
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x8
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x11
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x47
	.uleb128 0x13
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x64
	.uleb128 0x13
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x12
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x1f02
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x34
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x47
	.uleb128 0x13
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x64
	.uleb128 0x13
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x8
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x16
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x8
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x17
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x8
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x18
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.byte	0
	.byte	0
	.uleb128 0x19
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x1a
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0x8
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1b
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x20
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1e
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x1f
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_gnu_pubnames,"",@progbits
.Ldebug_pubnames0:
	.long	0x15b
	.value	0x2
	.long	.Lskeleton_debug_info0
	.long	0x5b3
	.long	0x15b
	.byte	0x30
	.string	"f13i"
	.long	0x177
	.byte	0x30
	.string	"C1::testcase1"
	.long	0x1a0
	.byte	0x30
	.string	"C1::testcase2"
	.long	0x1c4
	.byte	0x30
	.string	"C1::testcase3"
	.long	0x1e8
	.byte	0x30
	.string	"C1::testcase4"
	.long	0x20c
	.byte	0x30
	.string	"C2::testcase1"
	.long	0x235
	.byte	0x30
	.string	"C2::testcase2"
	.long	0x259
	.byte	0x30
	.string	"C2::testcase3"
	.long	0x27d
	.byte	0x30
	.string	"C2::testcase4"
	.long	0x2a1
	.byte	0x30
	.string	"C3::testcase1"
	.long	0x2ca
	.byte	0x30
	.string	"C3::testcase2"
	.long	0x2ee
	.byte	0x30
	.string	"f11a"
	.long	0x30e
	.byte	0x30
	.string	"C3::testcase3"
	.long	0x332
	.byte	0x30
	.string	"t12"
	.long	0x350
	.byte	0x30
	.string	"t13"
	.long	0x36e
	.byte	0x30
	.string	"t14"
	.long	0x3c9
	.byte	0x30
	.string	"t15"
	.long	0x427
	.byte	0x30
	.string	"t16"
	.long	0x445
	.byte	0x30
	.string	"t17"
	.long	0x496
	.byte	0x30
	.string	"t18"
	.long	0x559
	.byte	0x20
	.string	"p6"
	.long	0x56c
	.byte	0x20
	.string	"p7"
	.long	0x579
	.byte	0x20
	.string	"p8"
	.long	0x58c
	.byte	0x20
	.string	"p9"
	.long	0x59e
	.byte	0x20
	.string	"pfn"
	.long	0
	.section	.debug_gnu_pubtypes,"",@progbits
.Ldebug_pubtypes0:
	.long	0x50
	.value	0x2
	.long	.Lskeleton_debug_info0
	.long	0x5b3
	.long	0xc6
	.byte	0x90
	.string	"int"
	.long	0xcd
	.byte	0x90
	.string	"bool"
	.long	0x8d
	.byte	0x10
	.string	"C1"
	.long	0xdf
	.byte	0x10
	.string	"C2"
	.long	0x122
	.byte	0x10
	.string	"C3"
	.long	0x3c1
	.byte	0x90
	.string	"char"
	.long	0x41c
	.byte	0x90
	.string	"wchar_t"
	.long	0
	.section	.debug_aranges,"",@progbits
	.long	0x3c
	.value	0x2
	.long	.Lskeleton_debug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	.LFB0
	.quad	.LFE0-.LFB0
	.quad	0
	.quad	0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.quad	.Ltext0
	.quad	.Letext0
	.quad	.LFB0
	.quad	.LFE0
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_line.dwo,"e",@progbits
.Lskeleton_debug_line0:
	.long	.LELT0-.LSLT0
.LSLT0:
	.value	0x4
	.long	.LELTP0-.LASLTP0
.LASLTP0:
	.byte	0x1
	.byte	0x1
	.byte	0x1
	.byte	0xf6
	.byte	0xf2
	.byte	0xd
	.byte	0
	.byte	0x1
	.byte	0x1
	.byte	0x1
	.byte	0x1
	.byte	0
	.byte	0
	.byte	0
	.byte	0x1
	.byte	0
	.byte	0
	.byte	0x1
	.byte	0
	.string	"dwp_test.h"
	.uleb128 0
	.uleb128 0
	.uleb128 0
	.string	"dwp_test_1.cc"
	.uleb128 0
	.uleb128 0
	.uleb128 0
	.byte	0
.LELTP0:
.LELT0:
	.section	.debug_str_offsets.dwo,"e",@progbits
	.long	0
	.long	0xa
	.long	0x14
	.long	0x27
	.long	0x3a
	.long	0x44
	.long	0x57
	.long	0x5f
	.long	0x72
	.long	0x85
	.long	0x98
	.long	0xa2
	.long	0xb5
	.long	0xc8
	.long	0xdb
	.long	0xee
	.long	0x101
	.section	.debug_str.dwo,"e",@progbits
.LASF0:
	.string	"testcase1"
.LASF1:
	.string	"testcase2"
.LASF2:
	.string	"_ZN2C39testcase1Ev"
.LASF3:
	.string	"_ZN2C39testcase2Ev"
.LASF4:
	.string	"testcase3"
.LASF5:
	.string	"_ZN2C39testcase3Ev"
.LASF6:
	.string	"member1"
.LASF7:
	.string	"_ZN2C29testcase1Ev"
.LASF8:
	.string	"_ZN2C29testcase2Ev"
.LASF9:
	.string	"_ZN2C29testcase3Ev"
.LASF10:
	.string	"testcase4"
.LASF11:
	.string	"_ZN2C29testcase4Ev"
.LASF12:
	.string	"_ZN2C19testcase1Ev"
.LASF13:
	.string	"_ZN2C19testcase2Ev"
.LASF14:
	.string	"_ZN2C19testcase3Ev"
.LASF15:
	.string	"_ZN2C19testcase4Ev"
.LASF16:
	.string	"this"
	.section	.debug_addr,"",@progbits
.Ldebug_addr0:
	.quad	.LFB0
	.quad	.LFB1
	.quad	.LFB2
	.quad	.LFB3
	.quad	.LFB4
	.quad	.LFB5
	.quad	.LFB6
	.quad	.LFB7
	.quad	.LFB8
	.quad	.LFB9
	.quad	.LFB10
	.quad	.LFB11
	.quad	.LFB12
	.quad	.LFB13
	.quad	.LFB14
	.quad	.LFB15
	.quad	.LBB2
	.quad	.LFB16
	.quad	.LBB3
	.quad	.LFB17
	.quad	.LFB18
	.quad	.LBB4
	.quad	.LBB5
	.quad	.LFB19
	.quad	.LBB6
	.quad	.LBB7
	.quad	.LBB8
	.quad	p6
	.quad	p7
	.quad	p8
	.quad	p9
	.quad	pfn
	.ident	"GCC: (Google_crosstoolv16-gcc-4.7.x-grtev3) 4.7.x-google 20120720 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
