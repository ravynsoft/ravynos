        .file   "dwarf2-test.c"
        .text
        .section .text.startup,"ax",@progbits
        .p2align 4
        .globl  main
        .type   main, @function
main:
        .cfi_startproc
        nop
	.file 1 "dwarf2-test.c"
	.loc 1 1
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

# No .debug_line ok even if there is a .debug_info section and using .locs
