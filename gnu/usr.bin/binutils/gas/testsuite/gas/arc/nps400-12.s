        .text

        ; Miscellaneous
        ; whash
        whash      r2,[cm:r0],r1
        whash      r5,[cm:r3],r14
        whash      0,[cm:r0],r1
        whash      0,[cm:r3],r14
        whash      r2,[cm:r0],7
        whash      0,[cm:r0],7
        whash      r2,[cm:r0],64
        whash      0,[cm:r0],64

        ; mcmp
        mcmp       r0,[cm:r0],[cm:r1],r1
        mcmp.s     r0,[cm:r0],[cm:r1],r1
        mcmp.m     r0,[cm:r0],[cm:r1],r1
        mcmp.s.m   r0,[cm:r0],[cm:r1],r1

        mcmp       r0,[cm:r0,r0],[cm:r1],r1
        mcmp.s     r0,[cm:r0,r0],[cm:r1],r1
        mcmp.m     r0,[cm:r0,r0],[cm:r1],r1
        mcmp.s.m   r0,[cm:r0,r0],[cm:r1],r1

        mcmp       r0,[cm:r0,4],[cm:r1],r1
        mcmp.s     r0,[cm:r0,4],[cm:r1],r1
        mcmp.m     r0,[cm:r0,4],[cm:r1],r1
        mcmp.s.m   r0,[cm:r0,4],[cm:r1],r1
        mcmp       r0,[cm:r0,8],[cm:r1],r1
        mcmp       r0,[cm:r0,12],[cm:r1],r1

        mcmp       r0,[cm:r0],[cm:r1],4
        mcmp.s     r0,[cm:r0],[cm:r1],4
        mcmp.m     r0,[cm:r0],[cm:r1],4
        mcmp.s.m   r0,[cm:r0],[cm:r1],8
        mcmp       r0,[cm:r0],[cm:r1],127

        mcmp       r0,[cm:r0,8],[cm:r1],4
        mcmp.s     r0,[cm:r0,8],[cm:r1],4
        mcmp.m     r0,[cm:r0,8],[cm:r1],4
        mcmp.s.m   r0,[cm:r0,8],[cm:r1],4

        mcmp       r0,[cm:r0,r0],[cm:r1],46
        mcmp.s     r0,[cm:r0,r0],[cm:r1],70
        mcmp.m     r0,[cm:r0,r0],[cm:r1],72
        mcmp.s.m   r0,[cm:r0,r0],[cm:r1],125

        ;asri
        asri 0,    r0
        asri.core 0, r0
        asri.clsr 0,r0
        asri.all 0,r0
        asri.gic 0,r0
        rspi.gic 0,r0

        ;wkup
        wkup       0,r0
        wkup.cl

        ;getsti
        getsti     r2,[cm:r0]
        getsti     0,[cm:r0]
label:
        ;getrtc
        getrtc     r2,[cm:r0]
        getrtc     0,[cm:r0]

        ;b<cc>
        bnj label
        bnm label
        bnt label
