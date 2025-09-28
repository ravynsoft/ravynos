	.file	"test.c"
	.text
.Ltext0:
	.file 0 "/example" "test.c"
	.globl	x
	.section	.bss
	.balign 4
	.type	x, %object
	.size	x, 4
x:
	.zero	4
	.text
.Letext0:
	.file 1 "test.c"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x32
	.2byte	0x5
	.byte	0x1
	.byte	0x4
	.4byte	.Ldebug_abbrev0
	.uleb128 0x1
	.4byte	.LASF2
	.byte	0x1d
	.4byte	.LASF0
	.4byte	.LASF1
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.asciz	"x"
	.byte	0x1
	.byte	0x1
	.byte	0x5
	.4byte	0x2e
	.uleb128 0x5
	.byte	0x3
	.4byte	x
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.asciz	"int"
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x1f
	.uleb128 0x1b
	.uleb128 0x1f
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x3
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
	.byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x14
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF2:
	.asciz	"GNU C17 11.2.1 -g"
	.section	.debug_line_str,"MS",%progbits,1
.LASF1:
	.asciz	"/example"
.LASF0:
	.asciz	"test.c"
	.ident	"GCC: (GNU) 11.2.1"
	.section	.note.GNU-stack,"",%progbits
