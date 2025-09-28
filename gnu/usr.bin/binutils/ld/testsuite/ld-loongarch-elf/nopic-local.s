	.file	"nopic-local.c"
	.text
.Ltext0:
	.file 1 "nopic-local.c"
	.globl	g_nopic
	.data
	.align	2
	.type	g_nopic, @object
	.size	g_nopic, 4
g_nopic:
	.word	305419896
	.section	.rodata
	.align	3
.LC0:
	.ascii	"0x%x\012\000"
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
.LFB6 = .
	.loc 1 7 1
	.cfi_startproc
	addi.d	$r3,$r3,-16
	.cfi_def_cfa_offset 16
	st.d	$r1,$r3,8
	stptr.d	$r22,$r3,0
	.cfi_offset 1, -8
	.cfi_offset 22, -16
	addi.d	$r22,$r3,16
	.cfi_def_cfa 22, 0
	.loc 1 8 15
	pcalau12i	$r12,%pc_hi20(g_nopic)
	addi.d	$r12,$r12,%pc_lo12(g_nopic)
	ldptr.w	$r12,$r12,0
	.loc 1 8 6
	or	$r13,$r12,$r0
	lu12i.w	$r12,305418240>>12			# 0x12345000
	ori	$r12,$r12,1656
	bne	$r13,$r12,.L2
	.loc 1 9 5
	pcalau12i	$r12,%pc_hi20(g_nopic)
	addi.d	$r12,$r12,%pc_lo12(g_nopic)
	ldptr.w	$r12,$r12,0
	or	$r5,$r12,$r0
	pcalau12i	$r12,%pc_hi20(.LC0)
	addi.d	$r4,$r12,%pc_lo12(.LC0)
	bl	%plt(printf)
	b	.L5
.L2:
	.loc 1 11 5
	bl	%plt(abort)
.L5:
	.loc 1 12 10
	or	$r12,$r0,$r0
	.loc 1 13 1
	or	$r4,$r12,$r0
	ld.d	$r1,$r3,8
	.cfi_restore 1
	ldptr.d	$r22,$r3,0
	.cfi_restore 22
	addi.d	$r3,$r3,16
	.cfi_def_cfa_register 3
	jr	$r1
	.cfi_endproc
.LFE6:
	.size	main, .-main
.Letext0:
	.file 2 "/usr/include/stdlib.h"
	.file 3 "/usr/include/stdio.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.4byte	0xd8
	.2byte	0x5
	.byte	0x1
	.byte	0x8
	.4byte	.Ldebug_abbrev0
	.uleb128 0x2
	.4byte	.LASF10
	.byte	0x1d
	.4byte	.LASF11
	.4byte	.LASF12
	.8byte	.Ltext0
	.8byte	.Letext0-.Ltext0
	.4byte	.Ldebug_line0
	.uleb128 0x1
	.byte	0x8
	.byte	0x7
	.4byte	.LASF0
	.uleb128 0x1
	.byte	0x1
	.byte	0x8
	.4byte	.LASF1
	.uleb128 0x1
	.byte	0x2
	.byte	0x7
	.4byte	.LASF2
	.uleb128 0x1
	.byte	0x4
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x1
	.byte	0x1
	.byte	0x6
	.4byte	.LASF4
	.uleb128 0x1
	.byte	0x2
	.byte	0x5
	.4byte	.LASF5
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x1
	.byte	0x8
	.byte	0x5
	.4byte	.LASF6
	.uleb128 0x1
	.byte	0x1
	.byte	0x6
	.4byte	.LASF7
	.uleb128 0x4
	.4byte	0x66
	.uleb128 0x1
	.byte	0x8
	.byte	0x5
	.4byte	.LASF8
	.uleb128 0x1
	.byte	0x8
	.byte	0x7
	.4byte	.LASF9
	.uleb128 0x5
	.4byte	.LASF13
	.byte	0x1
	.byte	0x4
	.byte	0x5
	.4byte	0x58
	.uleb128 0x9
	.byte	0x3
	.8byte	g_nopic
	.uleb128 0x6
	.4byte	.LASF14
	.byte	0x2
	.2byte	0x256
	.byte	0xd
	.uleb128 0x7
	.4byte	.LASF15
	.byte	0x3
	.2byte	0x164
	.byte	0xc
	.4byte	0x58
	.4byte	0xb7
	.uleb128 0x8
	.4byte	0xb7
	.uleb128 0x9
	.byte	0
	.uleb128 0xa
	.byte	0x8
	.4byte	0x6d
	.uleb128 0xb
	.4byte	.LASF16
	.byte	0x1
	.byte	0x6
	.byte	0x5
	.4byte	0x58
	.8byte	.LFB6
	.8byte	.LFE6-.LFB6
	.uleb128 0x1
	.byte	0x9c
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x2
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
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
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
	.uleb128 0x4
	.uleb128 0x26
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
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
	.uleb128 0x6
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x87
	.uleb128 0x19
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x18
	.byte	0
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
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
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x7c
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.4byte	0x2c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x8
	.byte	0
	.2byte	0
	.2byte	0
	.8byte	.Ltext0
	.8byte	.Letext0-.Ltext0
	.8byte	0
	.8byte	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF8:
	.ascii	"long long int\000"
.LASF3:
	.ascii	"unsigned int\000"
.LASF16:
	.ascii	"main\000"
.LASF0:
	.ascii	"long unsigned int\000"
.LASF9:
	.ascii	"long long unsigned int\000"
.LASF11:
	.ascii	"nopic-local.c\000"
.LASF10:
	.ascii	"GNU C17 13.0.0 20220512 (experimental) -mabi=lp64d -marc"
	.ascii	"h=loongarch64 -mfpu=64 -mcmodel=normal -mtune=la464 -g -"
	.ascii	"O0\000"
.LASF1:
	.ascii	"unsigned char\000"
.LASF7:
	.ascii	"char\000"
.LASF6:
	.ascii	"long int\000"
.LASF13:
	.ascii	"g_nopic\000"
.LASF2:
	.ascii	"short unsigned int\000"
.LASF15:
	.ascii	"printf\000"
.LASF12:
	.ascii	"/home/liuzhensong/test/ld/nopic/test/local_var\000"
.LASF14:
	.ascii	"abort\000"
.LASF5:
	.ascii	"short int\000"
.LASF4:
	.ascii	"signed char\000"
	.ident	"GCC: (GNU) 13.0.0 20220512 (experimental)"
	.section	.note.GNU-stack,"",@progbits
