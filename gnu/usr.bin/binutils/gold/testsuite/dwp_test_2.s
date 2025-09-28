	.file	"dwp_test_2.cc"
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
	.globl	_ZN2C14t1_2Ev
	.type	_ZN2C14t1_2Ev, @function
_ZN2C14t1_2Ev:
.LFB1:
	.file 2 "dwp_test_2.cc"
	.loc 2 31 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 32 0
	movl	$123, %eax
	.loc 2 33 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	_ZN2C14t1_2Ev, .-_ZN2C14t1_2Ev
	.align 2
	.globl	_ZN2C13t1aEv
	.type	_ZN2C13t1aEv, @function
_ZN2C13t1aEv:
.LFB2:
	.loc 2 37 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$8, %rsp
	movq	%rdi, -8(%rbp)
	.loc 2 38 0
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	_ZN2C14t1_2Ev
	cmpl	$123, %eax
	sete	%al
	.loc 2 39 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	_ZN2C13t1aEv, .-_ZN2C13t1aEv
	.globl	v2
	.data
	.align 4
	.type	v2, @object
	.size	v2, 4
v2:
	.long	456
	.globl	v3
	.bss
	.align 4
	.type	v3, @object
	.size	v3, 4
v3:
	.zero	4
	.globl	v4
	.data
	.type	v4, @object
	.size	v4, 13
v4:
	.string	"Hello, world"
	.globl	v5
	.bss
	.type	v5, @object
	.size	v5, 13
v5:
	.zero	13
	.text
	.globl	_Z3f10v
	.type	_Z3f10v, @function
_Z3f10v:
.LFB3:
	.loc 2 73 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 74 0
	movl	$135, %eax
	.loc 2 75 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	_Z3f10v, .-_Z3f10v
	.globl	_Z4f11bPFivE
	.type	_Z4f11bPFivE, @function
_Z4f11bPFivE:
.LFB4:
	.loc 2 81 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	.loc 2 82 0
	movq	-8(%rbp), %rax
	call	*%rax
	.loc 2 83 0
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	_Z4f11bPFivE, .-_Z4f11bPFivE
	.align 2
	.globl	_ZN2C32f4Ev
	.type	_ZN2C32f4Ev, @function
_ZN2C32f4Ev:
.LFB5:
	.loc 2 89 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	.loc 2 90 0
	movl	$_Z3t12v, %eax
	.loc 2 91 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	_ZN2C32f4Ev, .-_ZN2C32f4Ev
	.globl	_Z3f13v
	.type	_Z3f13v, @function
_Z3f13v:
.LFB6:
	.loc 2 97 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 98 0
	movl	$_Z4f13iv, %eax
	.loc 2 99 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	_Z3f13v, .-_Z3f13v
	.section	.rodata
.LC0:
	.string	"test string constant"
	.text
	.globl	_Z3f14v
	.type	_Z3f14v, @function
_Z3f14v:
.LFB7:
	.loc 2 105 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 106 0
	movl	$.LC0, %eax
	.loc 2 107 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	_Z3f14v, .-_Z3f14v
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
	.globl	_Z3f15v
	.type	_Z3f15v, @function
_Z3f15v:
.LFB8:
	.loc 2 113 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	.loc 2 114 0
	movl	$.LC1, %eax
	.loc 2 115 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	_Z3f15v, .-_Z3f15v
	.globl	t17data
	.section	.rodata
.LC2:
	.string	"a"
.LC3:
	.string	"b"
.LC4:
	.string	"c"
.LC5:
	.string	"d"
.LC6:
	.string	"e"
	.data
	.align 32
	.type	t17data, @object
	.size	t17data, 40
t17data:
	.quad	.LC2
	.quad	.LC3
	.quad	.LC4
	.quad	.LC5
	.quad	.LC6
	.text
	.globl	_Z3f18i
	.type	_Z3f18i, @function
_Z3f18i:
.LFB9:
	.loc 2 128 0
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -4(%rbp)
	.loc 2 129 0
	cmpl	$4, -4(%rbp)
	ja	.L19
	movl	-4(%rbp), %eax
	movq	.L25(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L25:
	.quad	.L20
	.quad	.L21
	.quad	.L22
	.quad	.L23
	.quad	.L24
	.text
.L20:
	.loc 2 132 0
	movl	$.LC2, %eax
	jmp	.L26
.L21:
	.loc 2 134 0
	movl	$.LC3, %eax
	jmp	.L26
.L22:
	.loc 2 136 0
	movl	$.LC4, %eax
	jmp	.L26
.L23:
	.loc 2 138 0
	movl	$.LC5, %eax
	jmp	.L26
.L24:
	.loc 2 140 0
	movl	$.LC6, %eax
	jmp	.L26
.L19:
	.loc 2 142 0
	movl	$0, %eax
.L26:
	.loc 2 144 0
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	_Z3f18i, .-_Z3f18i
.Letext0:
	.section	.debug_types.dwo,"G",@progbits,wt.bb2916f0c1bd34b5,comdat
	.long	0xf3
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
	.long	0xd6
	.uleb128 0x3
	.string	"member1"
	.byte	0x1
	.byte	0x36
	.long	0xd6
	.byte	0
	.byte	0x1
	.uleb128 0x4
	.uleb128 0
	.byte	0x1
	.byte	0x32
	.string	"_ZN2C39testcase1Ev"
	.long	0xdd
	.byte	0x1
	.long	0x65
	.long	0x6b
	.uleb128 0x5
	.long	0xe5
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x1
	.byte	0x33
	.string	"_ZN2C39testcase2Ev"
	.long	0xdd
	.byte	0x1
	.long	0x8f
	.long	0x95
	.uleb128 0x5
	.long	0xe5
	.byte	0
	.uleb128 0x4
	.uleb128 0x2
	.byte	0x1
	.byte	0x34
	.string	"_ZN2C39testcase3Ev"
	.long	0xdd
	.byte	0x1
	.long	0xb9
	.long	0xbf
	.uleb128 0x5
	.long	0xe5
	.byte	0
	.uleb128 0x6
	.string	"f4"
	.byte	0x1
	.byte	0x35
	.uleb128 0x3
	.long	0xeb
	.byte	0x1
	.long	0xcf
	.uleb128 0x5
	.long	0xe5
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
	.long	0xf1
	.uleb128 0x9
	.long	0xdd
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
	.string	"dwp_test_2.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_types.dwo,"G",@progbits,wt.c419a9b7a4a2fab5,comdat
	.long	0x138
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
	.long	0x126
	.uleb128 0x3
	.string	"member1"
	.byte	0x1
	.byte	0x22
	.long	0x126
	.byte	0
	.byte	0x1
	.uleb128 0x4
	.uleb128 0
	.byte	0x1
	.byte	0x1c
	.string	"_ZN2C19testcase1Ev"
	.long	0x12d
	.byte	0x1
	.long	0x65
	.long	0x6b
	.uleb128 0x5
	.long	0x135
	.byte	0
	.uleb128 0xa
	.string	"t1a"
	.byte	0x1
	.byte	0x1d
	.uleb128 0x4
	.long	0x12d
	.byte	0x1
	.long	0x80
	.long	0x86
	.uleb128 0x5
	.long	0x135
	.byte	0
	.uleb128 0xa
	.string	"t1_2"
	.byte	0x1
	.byte	0x1e
	.uleb128 0x5
	.long	0x126
	.byte	0x1
	.long	0x9c
	.long	0xa2
	.uleb128 0x5
	.long	0x135
	.byte	0
	.uleb128 0x4
	.uleb128 0x1
	.byte	0x1
	.byte	0x1f
	.string	"_ZN2C19testcase2Ev"
	.long	0x12d
	.byte	0x1
	.long	0xc6
	.long	0xcc
	.uleb128 0x5
	.long	0x135
	.byte	0
	.uleb128 0x4
	.uleb128 0x2
	.byte	0x1
	.byte	0x20
	.string	"_ZN2C19testcase3Ev"
	.long	0x12d
	.byte	0x1
	.long	0xf0
	.long	0xf6
	.uleb128 0x5
	.long	0x135
	.byte	0
	.uleb128 0xb
	.string	"testcase4"
	.byte	0x1
	.byte	0x21
	.string	"_ZN2C19testcase4Ev"
	.long	0x12d
	.byte	0x1
	.long	0x11f
	.uleb128 0x5
	.long	0x135
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
	.string	"dwp_test_2.dwo"
	.long	.Ldebug_pubnames0
	.long	.Ldebug_pubtypes0
	.long	.Ldebug_addr0
	.section	.debug_info.dwo,"e",@progbits
.Ldebug_info0:
	.long	0x329
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0xc
	.string	"GNU C++ 4.7.x-google 20120720 (prerelease)"
	.byte	0x4
	.string	"dwp_test_2.cc"
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.byte	0xb9
	.byte	0xf8
	.byte	0xe0
	.byte	0x8c
	.byte	0x71
	.byte	0xab
	.byte	0xc
	.byte	0xcf
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
	.long	0xb9
	.uleb128 0xe
	.string	"t1a"
	.byte	0x1
	.byte	0x1d
	.uleb128 0x4
	.long	0xc0
	.byte	0x1
	.uleb128 0xe
	.string	"t1_2"
	.byte	0x1
	.byte	0x1e
	.uleb128 0x5
	.long	0xb9
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
	.string	"C3"
	.byte	0xbb
	.byte	0x29
	.byte	0x16
	.byte	0xf0
	.byte	0xc1
	.byte	0xbd
	.byte	0x34
	.byte	0xb5
	.long	0xef
	.uleb128 0xe
	.string	"f4"
	.byte	0x1
	.byte	0x35
	.uleb128 0x3
	.long	0xfe
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
	.uleb128 0x9
	.long	0xc0
	.uleb128 0x8
	.byte	0x8
	.long	0xf9
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
	.long	0xaa
	.byte	0x2
	.uleb128 0x1
	.quad	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.long	0x139
	.long	0x147
	.uleb128 0x12
	.string	"this"
	.long	0x147
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x13
	.long	0xc8
	.uleb128 0x14
	.long	0x9d
	.byte	0x2
	.byte	0x24
	.uleb128 0x2
	.quad	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x166
	.long	0x174
	.uleb128 0x12
	.string	"this"
	.long	0x147
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x15
	.string	"f10"
	.byte	0x2
	.byte	0x48
	.string	"_Z3f10v"
	.long	0xb9
	.uleb128 0x3
	.quad	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x16
	.string	"f11b"
	.byte	0x2
	.byte	0x50
	.string	"_Z4f11bPFivE"
	.long	0xb9
	.uleb128 0x4
	.quad	.LFE4-.LFB4
	.uleb128 0x1
	.byte	0x9c
	.long	0x1c9
	.uleb128 0x17
	.string	"pfn"
	.byte	0x2
	.byte	0x50
	.long	0x1ce
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x9
	.long	0xb9
	.uleb128 0x8
	.byte	0x8
	.long	0x1c9
	.uleb128 0x18
	.long	0xe2
	.byte	0x2
	.byte	0x58
	.uleb128 0x5
	.quad	.LFE5-.LFB5
	.uleb128 0x1
	.byte	0x9c
	.long	0x1ee
	.long	0x1fc
	.uleb128 0x12
	.string	"this"
	.long	0x1fc
	.uleb128 0x2
	.byte	0x91
	.sleb128 -24
	.byte	0
	.uleb128 0x13
	.long	0xef
	.uleb128 0x19
	.uleb128 0x15
	.string	"f13"
	.byte	0x2
	.byte	0x60
	.string	"_Z3f13v"
	.long	0x220
	.uleb128 0x6
	.quad	.LFE6-.LFB6
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x8
	.byte	0x8
	.long	0x201
	.uleb128 0x15
	.string	"f14"
	.byte	0x2
	.byte	0x68
	.string	"_Z3f14v"
	.long	0x244
	.uleb128 0x7
	.quad	.LFE7-.LFB7
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x8
	.byte	0x8
	.long	0x24a
	.uleb128 0x13
	.long	0x24f
	.uleb128 0x7
	.byte	0x1
	.byte	0x6
	.string	"char"
	.uleb128 0x15
	.string	"f15"
	.byte	0x2
	.byte	0x70
	.string	"_Z3f15v"
	.long	0x275
	.uleb128 0x8
	.quad	.LFE8-.LFB8
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x8
	.byte	0x8
	.long	0x27b
	.uleb128 0x13
	.long	0x280
	.uleb128 0x7
	.byte	0x4
	.byte	0x5
	.string	"wchar_t"
	.uleb128 0x1a
	.string	"f18"
	.byte	0x2
	.byte	0x7f
	.string	"_Z3f18i"
	.long	0x244
	.uleb128 0x9
	.quad	.LFE9-.LFB9
	.uleb128 0x1
	.byte	0x9c
	.long	0x2ba
	.uleb128 0x17
	.string	"i"
	.byte	0x2
	.byte	0x7f
	.long	0xb9
	.uleb128 0x2
	.byte	0x91
	.sleb128 -20
	.byte	0
	.uleb128 0x1b
	.string	"v2"
	.byte	0x2
	.byte	0x2b
	.long	0xb9
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0xa
	.uleb128 0x1b
	.string	"v3"
	.byte	0x2
	.byte	0x30
	.long	0xb9
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0xb
	.uleb128 0x1c
	.long	0x24f
	.long	0x2e4
	.uleb128 0x1d
	.long	0x2e4
	.byte	0xc
	.byte	0
	.uleb128 0x7
	.byte	0x8
	.byte	0x7
	.string	"sizetype"
	.uleb128 0x1b
	.string	"v4"
	.byte	0x2
	.byte	0x34
	.long	0x2d4
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0xc
	.uleb128 0x1b
	.string	"v5"
	.byte	0x2
	.byte	0x39
	.long	0x2d4
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0xd
	.uleb128 0x1c
	.long	0x244
	.long	0x31a
	.uleb128 0x1d
	.long	0x2e4
	.byte	0x4
	.byte	0
	.uleb128 0x1b
	.string	"t17data"
	.byte	0x2
	.byte	0x77
	.long	0x30a
	.uleb128 0x2
	.byte	0xfb
	.uleb128 0xe
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
	.byte	0xb9
	.byte	0xf8
	.byte	0xe0
	.byte	0x8c
	.byte	0x71
	.byte	0xab
	.byte	0xc
	.byte	0xcf
	.long	.Ldebug_ranges0
	.string	"/home/ccoutant/opensource/binutils-git/binutils/gold/testsuite"
	.string	"dwp_test_2.dwo"
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
	.uleb128 0x8
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
	.uleb128 0x8
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
	.uleb128 0x8
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
	.uleb128 0x12
	.uleb128 0x5
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
	.uleb128 0x2116
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
	.uleb128 0x17
	.uleb128 0x5
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
	.uleb128 0x18
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
	.uleb128 0x19
	.uleb128 0x15
	.byte	0
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
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x1c
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x1d
	.uleb128 0x21
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_gnu_pubnames,"",@progbits
.Ldebug_pubnames0:
	.long	0xa3
	.value	0x2
	.long	.Lskeleton_debug_info0
	.long	0x32d
	.long	0x104
	.byte	0x30
	.string	"f13i"
	.long	0x120
	.byte	0x30
	.string	"C1::t1_2"
	.long	0x14c
	.byte	0x30
	.string	"C1::t1a"
	.long	0x174
	.byte	0x30
	.string	"f10"
	.long	0x192
	.byte	0x30
	.string	"f11b"
	.long	0x1d4
	.byte	0x30
	.string	"C3::f4"
	.long	0x202
	.byte	0x30
	.string	"f13"
	.long	0x226
	.byte	0x30
	.string	"f14"
	.long	0x257
	.byte	0x30
	.string	"f15"
	.long	0x28b
	.byte	0x30
	.string	"f18"
	.long	0x2ba
	.byte	0x20
	.string	"v2"
	.long	0x2c7
	.byte	0x20
	.string	"v3"
	.long	0x2f0
	.byte	0x20
	.string	"v4"
	.long	0x2fd
	.byte	0x20
	.string	"v5"
	.long	0x31a
	.byte	0x20
	.string	"t17data"
	.long	0
	.section	.debug_gnu_pubtypes,"",@progbits
.Ldebug_pubtypes0:
	.long	0x56
	.value	0x2
	.long	.Lskeleton_debug_info0
	.long	0x32d
	.long	0xb9
	.byte	0x90
	.string	"int"
	.long	0xc0
	.byte	0x90
	.string	"bool"
	.long	0x8d
	.byte	0x10
	.string	"C1"
	.long	0xd2
	.byte	0x10
	.string	"C3"
	.long	0x24f
	.byte	0x90
	.string	"char"
	.long	0x280
	.byte	0x90
	.string	"wchar_t"
	.long	0x2e4
	.byte	0x90
	.string	"sizetype"
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
	.string	"dwp_test_2.cc"
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
	.long	0x2a
	.long	0x37
	.section	.debug_str.dwo,"e",@progbits
.LASF0:
	.string	"testcase1"
.LASF1:
	.string	"testcase2"
.LASF2:
	.string	"testcase3"
.LASF3:
	.string	"_ZN2C32f4Ev"
.LASF4:
	.string	"_ZN2C13t1aEv"
.LASF5:
	.string	"_ZN2C14t1_2Ev"
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
	.quad	v2
	.quad	v3
	.quad	v4
	.quad	v5
	.quad	t17data
	.ident	"GCC: (Google_crosstoolv16-gcc-4.7.x-grtev3) 4.7.x-google 20120720 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
