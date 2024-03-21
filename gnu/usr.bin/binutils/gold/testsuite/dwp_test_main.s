	.file	"dwp_test_main.cc"
	.text
.Ltext0:
	.section	.rodata
.LC0:
	.string	"dwp_test_main.cc"
.LC1:
	.string	"c1.testcase1()"
.LC2:
	.string	"c1.t1a()"
.LC3:
	.string	"c1.testcase2()"
.LC4:
	.string	"c1.testcase3()"
.LC5:
	.string	"c1.testcase4()"
.LC6:
	.string	"c2.testcase1()"
.LC7:
	.string	"c2.testcase2()"
.LC8:
	.string	"c2.testcase3()"
.LC9:
	.string	"c2.testcase4()"
.LC10:
	.string	"c3.testcase1()"
.LC11:
	.string	"c3.testcase2()"
.LC12:
	.string	"c3.testcase3()"
.LC13:
	.string	"t12()"
.LC14:
	.string	"t13()"
.LC15:
	.string	"t16()"
.LC16:
	.string	"t16a()"
.LC17:
	.string	"t17()"
.LC18:
	.string	"t18()"
	.text
	.globl	main
	.type	main, @function
main:
.LFB1:
	.file 1 "dwp_test_main.cc"
	.loc 1 31 0
	.cfi_startproc
	.cfi_personality 0x3,__gxx_personality_v0
	.cfi_lsda 0x3,.LLSDA1
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
.LBB2:
	.loc 1 36 0
	movl	$789, v3(%rip)
.LBB3:
	.loc 1 37 0
	movl	$0, -4(%rbp)
	jmp	.L2
.L3:
	.loc 1 38 0
	movl	-4(%rbp), %eax
	cltq
	movzbl	v4(%rax), %edx
	movl	-4(%rbp), %eax
	cltq
	movb	%dl, v5(%rax)
	.loc 1 37 0 discriminator 2
	addl	$1, -4(%rbp)
.L2:
	.loc 1 37 0 is_stmt 0 discriminator 1
	cmpl	$12, -4(%rbp)
	setle	%al
	testb	%al, %al
	jne	.L3
.LBE3:
	.loc 1 40 0 is_stmt 1
	leaq	-16(%rbp), %rax
	movq	%rax, %rdi
.LEHB0:
	call	_ZN2C19testcase1Ev
	.loc 1 40 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L4
	.loc 1 40 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$40, %edx
	movl	$.LC0, %esi
	movl	$.LC1, %edi
	call	__assert_fail
.L4:
	.loc 1 41 0 is_stmt 1
	leaq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C13t1aEv
	.loc 1 41 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L5
	.loc 1 41 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$41, %edx
	movl	$.LC0, %esi
	movl	$.LC2, %edi
	call	__assert_fail
.L5:
	.loc 1 42 0 is_stmt 1
	leaq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C19testcase2Ev
	.loc 1 42 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L6
	.loc 1 42 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$42, %edx
	movl	$.LC0, %esi
	movl	$.LC3, %edi
	call	__assert_fail
.L6:
	.loc 1 43 0 is_stmt 1
	leaq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C19testcase3Ev
	.loc 1 43 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L7
	.loc 1 43 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$43, %edx
	movl	$.LC0, %esi
	movl	$.LC4, %edi
	call	__assert_fail
.L7:
	.loc 1 44 0 is_stmt 1
	leaq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C19testcase4Ev
	.loc 1 44 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L8
	.loc 1 44 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$44, %edx
	movl	$.LC0, %esi
	movl	$.LC5, %edi
	call	__assert_fail
.L8:
	.loc 1 45 0 is_stmt 1
	leaq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C29testcase1Ev
	.loc 1 45 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L9
	.loc 1 45 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$45, %edx
	movl	$.LC0, %esi
	movl	$.LC6, %edi
	call	__assert_fail
.L9:
	.loc 1 46 0 is_stmt 1
	leaq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C29testcase2Ev
	.loc 1 46 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L10
	.loc 1 46 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$46, %edx
	movl	$.LC0, %esi
	movl	$.LC7, %edi
	call	__assert_fail
.L10:
	.loc 1 47 0 is_stmt 1
	leaq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C29testcase3Ev
	.loc 1 47 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L11
	.loc 1 47 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$47, %edx
	movl	$.LC0, %esi
	movl	$.LC8, %edi
	call	__assert_fail
.L11:
	.loc 1 48 0 is_stmt 1
	leaq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C29testcase4Ev
	.loc 1 48 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L12
	.loc 1 48 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$48, %edx
	movl	$.LC0, %esi
	movl	$.LC9, %edi
	call	__assert_fail
.L12:
	.loc 1 49 0 is_stmt 1
	movl	$c3, %edi
	call	_ZN2C39testcase1Ev
	.loc 1 49 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L13
	.loc 1 49 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$49, %edx
	movl	$.LC0, %esi
	movl	$.LC10, %edi
	call	__assert_fail
.L13:
	.loc 1 50 0 is_stmt 1
	movl	$c3, %edi
	call	_ZN2C39testcase2Ev
	.loc 1 50 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L14
	.loc 1 50 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$50, %edx
	movl	$.LC0, %esi
	movl	$.LC11, %edi
	call	__assert_fail
.L14:
	.loc 1 51 0 is_stmt 1
	movl	$c3, %edi
	call	_ZN2C39testcase3Ev
	.loc 1 51 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L15
	.loc 1 51 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$51, %edx
	movl	$.LC0, %esi
	movl	$.LC12, %edi
	call	__assert_fail
.L15:
	.loc 1 52 0 is_stmt 1
	call	_Z3t12v
	.loc 1 52 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L16
	.loc 1 52 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$52, %edx
	movl	$.LC0, %esi
	movl	$.LC13, %edi
	call	__assert_fail
.L16:
	.loc 1 53 0 is_stmt 1
	call	_Z3t13v
	.loc 1 53 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L17
	.loc 1 53 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$53, %edx
	movl	$.LC0, %esi
	movl	$.LC14, %edi
	call	__assert_fail
.L17:
	.loc 1 54 0 is_stmt 1
	call	_Z3t16v
	.loc 1 54 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L18
	.loc 1 54 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$54, %edx
	movl	$.LC0, %esi
	movl	$.LC15, %edi
	call	__assert_fail
.L18:
	.loc 1 55 0 is_stmt 1
	call	_Z4t16av
	.loc 1 55 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L19
	.loc 1 55 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$55, %edx
	movl	$.LC0, %esi
	movl	$.LC16, %edi
	call	__assert_fail
.L19:
	.loc 1 56 0 is_stmt 1
	call	_Z3t17v
	.loc 1 56 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L20
	.loc 1 56 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$56, %edx
	movl	$.LC0, %esi
	movl	$.LC17, %edi
	call	__assert_fail
.L20:
	.loc 1 57 0 is_stmt 1
	call	_Z3t18v
.LEHE0:
	.loc 1 57 0 is_stmt 0 discriminator 1
	testb	%al, %al
	jne	.L21
	.loc 1 57 0 discriminator 2
	movl	$_ZZ4mainE19__PRETTY_FUNCTION__, %ecx
	movl	$57, %edx
	movl	$.LC0, %esi
	movl	$.LC18, %edi
	call	__assert_fail
.L21:
	.loc 1 58 0 is_stmt 1
	movl	$0, %eax
	jmp	.L25
.L24:
	movq	%rax, %rdi
.LEHB1:
	call	_Unwind_Resume
.LEHE1:
.L25:
.LBE2:
	.loc 1 59 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
.LLSDA1:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE1-.LLSDACSB1
.LLSDACSB1:
	.uleb128 .LEHB0-.LFB1
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L24-.LFB1
	.uleb128 0
	.uleb128 .LEHB1-.LFB1
	.uleb128 .LEHE1-.LEHB1
	.uleb128 0
	.uleb128 0
.LLSDACSE1:
	.text
	.size	main, .-main
	.section	.rodata
	.type	_ZZ4mainE19__PRETTY_FUNCTION__, @object
	.size	_ZZ4mainE19__PRETTY_FUNCTION__, 11
_ZZ4mainE19__PRETTY_FUNCTION__:
	.string	"int main()"
	.text
.Letext0:
	.file 2 "dwp_test.h"
	.section	.debug_types.dwo,"G",@progbits,wt.bb2916f0c1bd34b5,comdat
	.long	0xf7
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
	.byte	0x2
	.byte	0x2f
	.long	0xda
	.uleb128 0x3
	.uleb128 0x3
	.byte	0x2
	.byte	0x36
	.long	0xda
	.byte	0
	.byte	0x1
	.uleb128 0x4
	.uleb128 0
	.byte	0x2
	.byte	0x32
	.string	"_ZN2C39testcase1Ev"
	.long	0xe1
	.byte	0x1
	.long	0x5e
	.long	0x64
	.uleb128 0x5
	.long	0xe9
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x2
	.byte	0x33
	.string	"_ZN2C39testcase2Ev"
	.long	0xe1
	.byte	0x1
	.long	0x88
	.long	0x8e
	.uleb128 0x5
	.long	0xe9
	.byte	0
	.uleb128 0x4
	.uleb128 0x2
	.byte	0x2
	.byte	0x34
	.string	"_ZN2C39testcase3Ev"
	.long	0xe1
	.byte	0x1
	.long	0xb2
	.long	0xb8
	.uleb128 0x5
	.long	0xe9
	.byte	0
	.uleb128 0x6
	.string	"f4"
	.byte	0x2
	.byte	0x35
	.string	"_ZN2C32f4Ev"
	.long	0xef
	.byte	0x1
	.long	0xd3
	.uleb128 0x5
	.long	0xe9
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
	.long	0xf5
	.uleb128 0x9
	.long	0xe1
	.byte	0
	.section	.debug_types,"G",@progbits,wt.bb2916f0c1bd34b5,comdat
	.long	0x71
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
	.string	"dwp_test_main.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_types.dwo,"G",@progbits,wt.66526f88bcc798ab,comdat
	.long	0xf1
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
	.byte	0x2
	.byte	0x25
	.long	0xdf
	.uleb128 0x3
	.uleb128 0x3
	.byte	0x2
	.byte	0x2c
	.long	0xdf
	.byte	0
	.byte	0x1
	.uleb128 0x4
	.uleb128 0
	.byte	0x2
	.byte	0x28
	.string	"_ZN2C29testcase1Ev"
	.long	0xe6
	.byte	0x1
	.long	0x5e
	.long	0x64
	.uleb128 0x5
	.long	0xee
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x2
	.byte	0x29
	.string	"_ZN2C29testcase2Ev"
	.long	0xe6
	.byte	0x1
	.long	0x88
	.long	0x8e
	.uleb128 0x5
	.long	0xee
	.byte	0
	.uleb128 0x4
	.uleb128 0x2
	.byte	0x2
	.byte	0x2a
	.string	"_ZN2C29testcase3Ev"
	.long	0xe6
	.byte	0x1
	.long	0xb2
	.long	0xb8
	.uleb128 0x5
	.long	0xee
	.byte	0
	.uleb128 0xa
	.uleb128 0x4
	.byte	0x2
	.byte	0x2b
	.string	"_ZN2C29testcase4Ev"
	.long	0xe6
	.byte	0x1
	.long	0xd8
	.uleb128 0x5
	.long	0xee
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
	.long	0x71
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
	.string	"dwp_test_main.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_types.dwo,"G",@progbits,wt.c419a9b7a4a2fab5,comdat
	.long	0x141
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
	.byte	0x2
	.byte	0x19
	.long	0x12f
	.uleb128 0x3
	.uleb128 0x3
	.byte	0x2
	.byte	0x22
	.long	0x12f
	.byte	0
	.byte	0x1
	.uleb128 0x4
	.uleb128 0
	.byte	0x2
	.byte	0x1c
	.string	"_ZN2C19testcase1Ev"
	.long	0x136
	.byte	0x1
	.long	0x5e
	.long	0x64
	.uleb128 0x5
	.long	0x13e
	.byte	0
	.uleb128 0xb
	.string	"t1a"
	.byte	0x2
	.byte	0x1d
	.string	"_ZN2C13t1aEv"
	.long	0x136
	.byte	0x1
	.long	0x85
	.long	0x8b
	.uleb128 0x5
	.long	0x13e
	.byte	0
	.uleb128 0xb
	.string	"t1_2"
	.byte	0x2
	.byte	0x1e
	.string	"_ZN2C14t1_2Ev"
	.long	0x12f
	.byte	0x1
	.long	0xae
	.long	0xb4
	.uleb128 0x5
	.long	0x13e
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x2
	.byte	0x1f
	.string	"_ZN2C19testcase2Ev"
	.long	0x136
	.byte	0x1
	.long	0xd8
	.long	0xde
	.uleb128 0x5
	.long	0x13e
	.byte	0
	.uleb128 0x4
	.uleb128 0x2
	.byte	0x2
	.byte	0x20
	.string	"_ZN2C19testcase3Ev"
	.long	0x136
	.byte	0x1
	.long	0x102
	.long	0x108
	.uleb128 0x5
	.long	0x13e
	.byte	0
	.uleb128 0xa
	.uleb128 0x4
	.byte	0x2
	.byte	0x21
	.string	"_ZN2C19testcase4Ev"
	.long	0x136
	.byte	0x1
	.long	0x128
	.uleb128 0x5
	.long	0x13e
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
	.long	0x71
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
	.string	"dwp_test_main.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_info.dwo,"e",@progbits
.Ldebug_info0:
	.long	0x178
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0xc
	.string	"GNU C++ 4.7.x-google 20120720 (prerelease)"
	.byte	0x4
	.string	"dwp_test_main.cc"
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.byte	0xc8
	.byte	0xeb
	.byte	0x9a
	.byte	0x5c
	.byte	0xd9
	.byte	0x51
	.byte	0xba
	.byte	0xe5
	.uleb128 0x7
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x7
	.byte	0x1
	.byte	0x2
	.string	"bool"
	.uleb128 0xd
	.string	"main"
	.byte	0x1
	.byte	0x1e
	.long	0x90
	.uleb128 0
	.quad	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x11b
	.uleb128 0xe
	.uleb128 0x1
	.quad	.LBE2-.LBB2
	.uleb128 0xf
	.string	"c1"
	.byte	0x1
	.byte	0x20
	.byte	0xc4
	.byte	0x19
	.byte	0xa9
	.byte	0xb7
	.byte	0xa4
	.byte	0xa2
	.byte	0xfa
	.byte	0xb5
	.uleb128 0x2
	.byte	0x91
	.sleb128 -32
	.uleb128 0xf
	.string	"c2"
	.byte	0x1
	.byte	0x21
	.byte	0x66
	.byte	0x52
	.byte	0x6f
	.byte	0x88
	.byte	0xbc
	.byte	0xc7
	.byte	0x98
	.byte	0xab
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x10
	.string	"__PRETTY_FUNCTION__"
	.long	0x13f
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0x2
	.uleb128 0xe
	.uleb128 0x3
	.quad	.LBE3-.LBB3
	.uleb128 0x11
	.string	"i"
	.byte	0x1
	.byte	0x25
	.long	0x90
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0x12
	.long	0x137
	.long	0x12b
	.uleb128 0x13
	.long	0x12b
	.byte	0xa
	.byte	0
	.uleb128 0x7
	.byte	0x8
	.byte	0x7
	.string	"sizetype"
	.uleb128 0x7
	.byte	0x1
	.byte	0x6
	.string	"char"
	.uleb128 0x14
	.long	0x11b
	.uleb128 0x15
	.string	"c3"
	.byte	0x2
	.byte	0x39
	.byte	0xbb
	.byte	0x29
	.byte	0x16
	.byte	0xf0
	.byte	0xc1
	.byte	0xbd
	.byte	0x34
	.byte	0xb5
	.uleb128 0x16
	.string	"v3"
	.byte	0x2
	.byte	0x3c
	.long	0x90
	.uleb128 0x12
	.long	0x137
	.long	0x167
	.uleb128 0x17
	.byte	0
	.uleb128 0x16
	.string	"v4"
	.byte	0x2
	.byte	0x3d
	.long	0x15c
	.uleb128 0x16
	.string	"v5"
	.byte	0x2
	.byte	0x3e
	.long	0x15c
	.byte	0
	.section	.debug_info,"",@progbits
.Lskeleton_debug_info0:
	.long	0x81
	.value	0x4
	.long	.Lskeleton_debug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.byte	0xc8
	.byte	0xeb
	.byte	0x9a
	.byte	0x5c
	.byte	0xd9
	.byte	0x51
	.byte	0xba
	.byte	0xe5
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.string	"dwp_test_main.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_abbrev,"",@progbits
.Lskeleton_debug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.uleb128 0x2131
	.uleb128 0x7
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
	.uleb128 0xe
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1f01
	.uleb128 0x12
	.uleb128 0x7
	.byte	0
	.byte	0
	.uleb128 0xf
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
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x10
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x34
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x11
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
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x13
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x14
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x15
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
	.uleb128 0x16
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
	.uleb128 0x17
	.uleb128 0x21
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_gnu_pubnames,"",@progbits
.Ldebug_pubnames0:
	.long	0x18
	.value	0x2
	.long	.Lskeleton_debug_info0
	.long	0x17c
	.long	0x9f
	.byte	0x30
	.string	"main"
	.long	0
	.section	.debug_gnu_pubtypes,"",@progbits
.Ldebug_pubtypes0:
	.long	0x51
	.value	0x2
	.long	.Lskeleton_debug_info0
	.long	0x17c
	.long	0x90
	.byte	0x90
	.string	"int"
	.long	0x97
	.byte	0x90
	.string	"bool"
	.long	0
	.byte	0x10
	.string	"C1"
	.long	0
	.byte	0x10
	.string	"C2"
	.long	0
	.byte	0x10
	.string	"C3"
	.long	0x12b
	.byte	0x90
	.string	"sizetype"
	.long	0x137
	.byte	0x90
	.string	"char"
	.long	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Lskeleton_debug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
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
	.string	"dwp_test_main.cc"
	.uleb128 0
	.uleb128 0
	.uleb128 0
	.string	"dwp_test.h"
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
	.long	0x1e
	.long	0x26
	.section	.debug_str.dwo,"e",@progbits
.LASF0:
	.string	"testcase1"
.LASF1:
	.string	"testcase2"
.LASF2:
	.string	"testcase3"
.LASF3:
	.string	"member1"
.LASF4:
	.string	"testcase4"
	.section	.debug_addr,"",@progbits
.Ldebug_addr0:
	.quad	.LFB1
	.quad	.LBB2
	.quad	_ZZ4mainE19__PRETTY_FUNCTION__
	.quad	.LBB3
	.ident	"GCC: (Google_crosstoolv16-gcc-4.7.x-grtev3) 4.7.x-google 20120720 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
