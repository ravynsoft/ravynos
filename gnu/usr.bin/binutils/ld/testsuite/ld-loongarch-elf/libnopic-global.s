	.file	"libnopic-global.c"
	.text
.Ltext0:
	.file 1 "libnopic-global.c"
	.globl	g_nopic
	.data
	.align	2
	.type	g_nopic, @object
	.size	g_nopic, 4
g_nopic:
	.word	305419896
	.text
.Letext0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.4byte	0x38
	.2byte	0x5
	.byte	0x1
	.byte	0x8
	.4byte	.Ldebug_abbrev0
	.uleb128 0x1
	.4byte	.LASF0
	.byte	0x1d
	.4byte	.LASF1
	.4byte	.LASF2
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.4byte	.LASF3
	.byte	0x1
	.byte	0x1
	.byte	0x5
	.4byte	0x34
	.uleb128 0x9
	.byte	0x3
	.8byte	g_nopic
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
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
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
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
	.section	.debug_aranges,"",@progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x8
	.byte	0
	.2byte	0
	.2byte	0
	.8byte	0
	.8byte	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF1:
	.ascii	"libnopic-global.c\000"
.LASF0:
	.ascii	"GNU C17 13.0.0 20220512 (experimental) -mabi=lp64d -marc"
	.ascii	"h=loongarch64 -mfpu=64 -mcmodel=normal -mtune=la464 -g -"
	.ascii	"O0 -fPIC\000"
.LASF2:
	.ascii	"/home/liuzhensong/test/ld/nopic/test/global_var\000"
.LASF3:
	.ascii	"g_nopic\000"
	.ident	"GCC: (GNU) 13.0.0 20220512 (experimental)"
	.section	.note.GNU-stack,"",@progbits
