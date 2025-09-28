        .text
        .proc foo#
foo:
        .mlx
        mov r25 = r0
        brl.call.sptk.many b0 = bar#
        .endp foo#
