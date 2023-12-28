        .file   "1.c"
        .text
        .align  2
        .p2align 3,,7
        .global dec
        .arch armv8-a+fp+simd
        //.tune generic
        .type   dec, %function
dec:
        adrp    x0, var_2
        ldr     w0, [x0, #:lo12:var_2]
        ret
        .size   dec, .-dec
        .ident  "GCC: (GNU) 6.0.0 20160208 (experimental) [trunk revision 233206]"
