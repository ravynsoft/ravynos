start:
        cmp     r0,(r1)
        cmp     r0,@r1
        cmp     (r1),r0
        cmp     @r1,r0
        cmp     (r1),@r1

        halt

        .END
