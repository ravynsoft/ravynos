	.file	"beginwarn.c"
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.text
.Ltext0:
	.section	.init_array
	.align 4
	.type	init_array, %object
	.size	init_array, 4
init_array:
	.long	foo
	.section	.gnu.warning.foo,"a",%progbits
	.type	_evoke_link_warning_foo, %object
	.size	_evoke_link_warning_foo, 27
_evoke_link_warning_foo:
	.string	"function foo is deprecated"
	.file 1 "/beginwarn.c"
	.text
.Letext0:
	.section	.debug_info
	.4byte	0x8a
	.2byte  0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.Ldebug_line0
	.4byte	.Letext0
	.4byte	.Ltext0
	.4byte	.LASF4
	.byte	0x1
	.4byte	.LASF5
	.uleb128 0x2
	.4byte	0x31
	.4byte	0x38
	.uleb128 0x3
	.4byte	0x31
	.byte	0x1a
	.byte	0x0
	.uleb128 0x4
	.4byte	.LASF0
	.byte	0x4
	.byte	0x7
	.uleb128 0x5
	.4byte	0x3d
	.uleb128 0x4
	.4byte	.LASF1
	.byte	0x1
	.byte	0x6
	.uleb128 0x6
	.4byte	.LASF2
	.byte	0x1
	.byte	0x3
	.4byte	0x55
	.byte	0x5
	.byte	0x3
	.4byte	_evoke_link_warning_foo
	.uleb128 0x5
	.4byte	0x21
	.uleb128 0x2
	.4byte	0x6a
	.4byte	0x6c
	.uleb128 0x3
	.4byte	0x31
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.byte	0x1
	.uleb128 0x5
	.4byte	0x71
	.uleb128 0x8
	.byte	0x4
	.4byte	0x6a
	.uleb128 0x6
	.4byte	.LASF3
	.byte	0x1
	.byte	0x9
	.4byte	0x88
	.byte	0x5
	.byte	0x3
	.4byte	init_array
	.uleb128 0x5
	.4byte	0x5a
	.byte	0x0
	.section	.debug_abbrev
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x10
	.uleb128 0x6
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x5
	.uleb128 0x26
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.uleb128 0x15
	.byte	0x0
	.uleb128 0x27
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0x8
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_str,"MS",%progbits,1
.LASF5:
	.string	"/beginwarn.c"
.LASF0:
	.string	"unsigned int"
.LASF3:
	.string	"init_array"
.LASF2:
	.string	"_evoke_link_warning_foo"
.LASF4:
	.string	"GNU C 3.4.6"
.LASF1:
	.string	"char"
