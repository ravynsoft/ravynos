	.file	"undefined.c"
	.pred.safe_across_calls p1-p5,p16-p63
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.text
.Ltext0:
	.align 16
	.global function#
	.proc function#
function:
[.LFB2:]
	.file 1 "undefined.c"
	.loc 1 8 0
	.prologue 12, 32
	.mii
	.save ar.pfs, r33
	alloc r33 = ar.pfs, 0, 3, 0, 0
	.save rp, r32
	mov r32 = b0
	mov r34 = r1
	.body
	.loc 1 9 0
	;;
	.mib
	nop 0
	nop 0
	br.call.sptk.many b0 = this_function_is_not_defined#
	.loc 1 10 0
	;;
	.loc 1 9 0
	.mmi
	nop 0
	mov r1 = r34
	.loc 1 10 0
	mov b0 = r32
	.mib
	nop 0
	mov ar.pfs = r33
	br.ret.sptk.many b0
.LFE2:
	.endp function#
.Letext0:
	.section	.debug_info
	data4.ua	0x4c
	data2.ua	0x2
	data4.ua	@secrel(.Ldebug_abbrev0)
	data1	0x8
	.uleb128 0x1
	data4.ua	@secrel(.Ldebug_line0)
	data8.ua	.Letext0
	data8.ua	.Ltext0
	data4.ua	@secrel(.LASF0)
	data1	0x1
	data4.ua	@secrel(.LASF1)
	.uleb128 0x2
	data1	0x1
	data4.ua	@secrel(.LASF2)
	data1	0x1
	data1	0x8
	data4.ua	0x48
	data8.ua	.LFB2
	data8.ua	.LFE2
	data1	0x2
	data1	0x7c
	.sleb128 16
	.uleb128 0x3
	stringz	"int"
	data1	0x4
	data1	0x5
	data1	0x0
	.section	.debug_abbrev
	.uleb128 0x1
	.uleb128 0x11
	data1	0x1
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
	data1	0x0
	data1	0x0
	.uleb128 0x2
	.uleb128 0x2e
	data1	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	data1	0x0
	data1	0x0
	.uleb128 0x3
	.uleb128 0x24
	data1	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	data1	0x0
	data1	0x0
	data1	0x0
	.section	.debug_pubnames,"",@progbits
	data4.ua	0x1b
	data2.ua	0x2
	data4.ua	@secrel(.Ldebug_info0)
	data4.ua	0x50
	data4.ua	0x29
	stringz	"function"
	data4.ua	0x0
	.section	.debug_aranges,"",@progbits
	data4.ua	0x2c
	data2.ua	0x2
	data4.ua	@secrel(.Ldebug_info0)
	data1	0x8
	data1	0x0
	data2.ua	0x0
	data2.ua	0x0
	data8.ua	.Ltext0
	data8.ua	.Letext0-.Ltext0
	data8.ua	0x0
	data8.ua	0x0
	.section	.debug_str,"MS",@progbits,1
.LASF0:
	stringz	"GNU C 4.1.2"
.LASF1:
	stringz	"undefined.c"
.LASF2:
	stringz	"function"
