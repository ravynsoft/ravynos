        .file   "dwarf2-test.c"
        .text
        .section .text.startup,"ax",@progbits
        .p2align 4
        .globl  main
        .type   main, @function
main:
        .cfi_startproc
        nop
        ret
        .cfi_endproc
        .size   main, .-main
        .text

        .section .debug_info,"",%progbits
        .long   0x0
        .value  0x2
        .long   .Ldebug_abbrev0
        .byte   0x8
        .uleb128 0x1

        .section .debug_abbrev,"",@progbits
.Ldebug_abbrev0:
        .uleb128 0x0    # (abbrev code)
        .uleb128 0x0    # (abbrev code)
        .uleb128 0x0    # (abbrev code)

# A non-empty .debug_line section is ok when not using .loc directives
	.section .debug_line
.Lline1_begin:
	.4byte		.Lline1_end - .Lline1_start	/* Initial length */
.Lline1_start:
	.2byte		2			/* Version */
	.4byte		.Lline1_lines - .Lline1_hdr	/* header_length */
.Lline1_hdr:
	.byte		1			/* Minimum insn length */
	.byte		1			/* default_is_stmt */
	.byte		1			/* line_base */
	.byte		1			/* line_range */
	.byte		0x10			/* opcode_base */

	/* Standard lengths */
	.byte		0
	.byte		1
	.byte		1
	.byte		1
	.byte		1
	.byte		0
	.byte		0
	.byte		0
	.byte		1
	.byte		0
	.byte		0
	.byte		1
	.byte		0
	.byte		0
	.byte		0

	/* Include directories */
	.byte		0

	/* File names */
	.ascii		"file1.txt\0"
	.uleb128	0
	.uleb128	0
	.uleb128	0

	.byte		0

.Lline1_lines:
	.byte		0	/* DW_LNE_set_address */
	.uleb128	5
	.byte		2
	.4byte		.Lbegin_func_cu1

	.byte		3	/* DW_LNS_advance_line */
	.sleb128	3	/* ... to 4 */

	.byte		1	/* DW_LNS_copy */

	.byte		1	/* DW_LNS_copy (second time as an end-of-prologue marker) */

	.byte		0	/* DW_LNE_set_address */
	.uleb128	5
	.byte		2
	.4byte		.Lend_func_cu1

	.byte		0	/* DW_LNE_end_of_sequence */
	.uleb128	1
	.byte		1

