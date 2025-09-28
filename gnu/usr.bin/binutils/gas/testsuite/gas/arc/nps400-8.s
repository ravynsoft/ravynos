        .text

        ;; bdalc / sbdalc
        bdalc r0,[cm:r0],r0,r0
        bdalc r1,[cm:r2],r2,r3
        bdalc r0,[cm:r0],r0,0,1
        bdalc r2,[cm:r3],r3,1,1
        bdalc r3,[cm:r4],r4,1,8
        sbdalc r0, r0, 0
        sbdalc r3, r4, 1

        ;; bdfre / sbdfre
        bdfre 0,[cm:r0],r0,r0
        bdfre 0,[cm:r1],r1,r2
        bdfre 0,[cm:r0],r0,1
        bdfre 0,[cm:r2],r2,8
        bdfre 0,[cm:r0],r0,0,1
        bdfre 0,[cm:r6],r6,0,8
        bdfre 0,[cm:r0],r0,1,1
        bdfre 0,[cm:r6],r6,1,8
        sbdfre 0, r0, r0
        sbdfre 0, r1, r2

        ;; bdbgt
        bdbgt 0,r0,r0
        bdbgt 0,r4,r6

        ;; idxalc / sidxalc
        idxalc r0,[cm:r0],r0,r0
        idxalc r1,[cm:r2],r2,r3
        idxalc r4,[cm:r5],r5,2
        sidxalc r0,r0
        sidxalc r4,r2

        ;; idxfre / sidxfre
        idxfre 0,[cm:r0],r0,r0
        idxfre 0,[cm:r1],r1,r2
        idxfre 0,[cm:r0],r0,1
        idxfre 0,[cm:r2],r2,8
        sidxfre 0, r0, r0
        sidxfre 0, r1, r2

        ;; idxbgt
        idxbgt 0,r0,r0
        idxbgt 0,r7,r8

        ;; efabgt
        efabgt 0,0x0,r0
        efabgt 0,0xffffffff,r3
        efabgt 0,r0,0x0
        efabgt 0,r4,0xffffffff
        efabgt 0,r0,r0
        efabgt 0,r7,r8
        efabgt r0,0x0,r0
        efabgt r4,0xffffffff,r6
        efabgt r0,r0,0x0
        efabgt r2,r3,0xffffffff
        efabgt r0,r0,r0
        efabgt r7,r8,r9

        ;; jobget
        jobget 0,[cjid:r0]
        jobget 0,[cjid:r6]
        jobget.cl 0,[cjid:r0]
        jobget.cl 0,[cjid:r6]

        ;; jobdn
        jobdn 0,[cjid:r0],r0,r0
        jobdn 0,[cjid:r2],r2,r4
        jobdn 0,[cjid:r0],r0,0
        jobdn 0,[cjid:r2],r2,15

        ;; jobalc / sjobalc
        jobalc r0,[cm:r0],r0,r0
        jobalc r1,[cm:r2],r2,r3
        jobalc r0,[cm:r0],r0,1
        jobalc r1,[cm:r2],r2,4
        sjobalc r0,r0
        sjobalc r6,r5

        ;; jobbgt

        jobbgt r0,r0,r0
        jobbgt r2,r5,r6

        ;; cnljob

        cnljob 0

        ;; qseq
        qseq r0,[r0]
        qseq r2,[r4]
