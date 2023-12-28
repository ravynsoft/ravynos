	.file	"dw2-compress-2.c"
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.p2align 4,,15
.globl foo2
	.type	foo2, @function
foo2:
.LFB1:
	.file 1 "dw2-compress-2.c"
	.loc 1 11 0
	.cfi_startproc
	.loc 1 12 0
	rep
	ret
	.cfi_endproc
.LFE1:
	.size	foo2, .-foo2
	.p2align 4,,15
.globl foo1
	.type	foo1, @function
foo1:
.LFB0:
	.loc 1 5 0
	.cfi_startproc
	subl	$12, %esp
	.cfi_def_cfa_offset 16
	.loc 1 7 0
	addl	$12, %esp
	.cfi_def_cfa_offset 4
	.loc 1 6 0
	jmp	bar
	.cfi_endproc
.LFE0:
	.size	foo1, .-foo1
.Letext0:
	.section	.debug_info
	.long	0x46
	.value	0x3
	.long	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.long	.LASF2
	.byte	0x1
	.long	.LASF3
	.long	.LASF4
	.long	.Ltext0
	.long	.Letext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.long	.LASF0
	.byte	0x1
	.byte	0xa
	.long	.LFB1
	.long	.LFE1
	.byte	0x1
	.byte	0x9c
	.uleb128 0x2
	.byte	0x1
	.long	.LASF1
	.byte	0x1
	.byte	0x4
	.long	.LFB0
	.long	.LFE0
	.byte	0x1
	.byte	0x9c
	.byte	0x0
	.section	.debug_abbrev
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x10
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_pubnames,"",@progbits
	.long	0x20
	.value	0x2
	.long	.Ldebug_info0
	.long	0x4a
	.long	0x25
	.string	"foo2"
	.long	0x37
	.string	"foo1"
	.long	0x0
	.section	.debug_aranges,"",@progbits
	.long	0x1c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x4
	.byte	0x0
	.value	0x0
	.value	0x0
	.long	.Ltext0
	.long	.Letext0-.Ltext0
	.long	0x0
	.long	0x0
	.section	.debug_str,"MS",@progbits,1
.LASF2:
	.string	"GNU C 4.4.4"
.LASF0:
	.string	"foo2"
.LASF1:
	.string	"foo1"
.LASF4:
	.string	"."
.LASF3:
	.string	"dw2-compress-2.c"
