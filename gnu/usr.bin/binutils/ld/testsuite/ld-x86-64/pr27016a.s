        .text
        .comm   global_int,4,4
        .globl  main
        .type   main, @function
main:
        .cfi_startproc
        pushq   %rbp
        .cfi_def_cfa_offset 16
        .cfi_offset 6, -16
        movq    %rsp, %rbp
        .cfi_def_cfa_register 6
        movq    thesym@GOTPCREL(%rip), %r11
        movl    (%r11), %eax
        leal    1(%rax), %edx
        movq    thesym@GOTPCREL(%rip), %r11
        movl    %edx, (%r11)
        movl    $0, %eax
        popq    %rbp
        .cfi_def_cfa 7, 8
        ret
        .cfi_endproc
        .size   main, .-main
        .section        .note.GNU-stack,"",@progbits
