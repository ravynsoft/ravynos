        .text
        .org    0

; load group
        ld    a,ixh
        ld    b,ixh
        ld    c,ixh
        ld    d,ixh
        ld    e,ixh
        ld    ixh,ixh
        ld    ixl,ixh

        ld    a,ixl
        ld    b,ixl
        ld    c,ixl
        ld    d,ixl
        ld    e,ixl
        ld    ixh,ixl
        ld    ixl,ixl

        ld    a,iyh
        ld    b,iyh
        ld    c,iyh
        ld    d,iyh
        ld    e,iyh
        ld    iyh,iyh
        ld    iyl,iyh

        ld    a,iyl
        ld    b,iyl
        ld    c,iyl
        ld    d,iyl
        ld    e,iyl
        ld    iyh,iyl
        ld    iyl,iyl

        ld    ixh,a
        ld    ixh,b
        ld    ixh,c
        ld    ixh,d
        ld    ixh,e
        ld    ixh,ixh
        ld    ixh,ixl
        ld    ixh,25

        ld    ixl,a
        ld    ixl,b
        ld    ixl,c
        ld    ixl,d
        ld    ixl,e
        ld    ixl,ixh
        ld    ixl,ixl
        ld    ixl,25

        ld    iyh,a
        ld    iyh,b
        ld    iyh,c
        ld    iyh,d
        ld    iyh,e
        ld    iyh,iyh
        ld    iyh,iyl
        ld    iyh,25

        ld    iyl,a
        ld    iyl,b
        ld    iyl,c
        ld    iyl,d
        ld    iyl,e
        ld    iyl,iyh
        ld    iyl,iyl
        ld    iyl,25

; arithmetic group
        add   a,ixh
        add   a,ixl
        add   a,iyh
        add   a,iyl

        adc   a,ixh
        adc   a,ixl
        adc   a,iyh
        adc   a,iyl

        cp    ixh
        cp    ixl
        cp    iyh
        cp    iyl

        dec   ixh
        dec   ixl
        dec   iyh
        dec   iyl

        inc   ixh
        inc   ixl
        inc   iyh
        inc   iyl

        sbc   a,ixh
        sbc   a,ixl
        sbc   a,iyh
        sbc   a,iyl

        sub   ixh
        sub   ixl
        sub   iyh
        sub   iyl

; logic group
        and   ixh
        and   ixl
        and   iyh
        and   iyl

        or    ixh
        or    ixl
        or    iyh
        or    iyl

        xor   ixh
        xor   ixl
        xor   iyh
        xor   iyl
