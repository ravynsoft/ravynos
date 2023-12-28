        .text
        movb		r0, r0, r1, 4, 5, 6
        movb		r0, r0, r12, 4, 5, 6
        movb		r15, r15, r12, 4, 5, 6
        movb.cl		r0, r1, 4, 5, 6
        movb.cl		r0, r14, 4, 5, 6
        movb.cl		r13, r1, 4, 5, 6

        movh		r0, r0, 1234
        movh		r3, r3, 0xffff
        movh.cl		r0, 1234
        movh.cl		r3, 0xffff

        /* movbi */
        movbi		r14, r14, 6, 8, 4
        movbi.f		r23, r23, 20, 11, 1
        movbi.cl	r30, 10, 18, 2
        movbi.f.cl	r6, 9, 0, 8

        /* decode1 */
        decode1		r0, r0, r2, 5, 11
        decode1.f	r0, r0, r2, 5, 11
        decode1.cl	r0, r2, 11
        decode1.cl.f	r0, r2, 18

        /* fbset */
        fbset		r1, r1, r3, 3, 15
        fbset.f		r1, r1, r3, 3, 15

        /* fbclear */
        fbclr		r2, r2, r12, 3, 15
        fbclr.f	        r3, r3, r12, 3, 15

        /* encode0 */
        encode0         r2, r1, 18, 1
        encode0.f       r0, r0, 0, 32

        /* encode1 */
        encode1         r2, r1, 31, 1
        encode1.f       r0, r0, 0, 32

        /* rflt */
        rflt           r10,r12,r20
        rflt           r0,0x12345678,r20
        rflt           r6,r7,0xffffffff
        rflt           r8,0xffffffff,0xffffffff
        rflt           0,r14,r13
        rflt           0,0xffffffff,r10
        rflt           0,r12,0xffffffff
        rflt           r4,r5,0x1
        rflt           r3,0x12345678,0x2
        rflt           0,r1,0x4
        rflt           0,0xffffffff,0x1

        .macro  crc_test mnem
        \mnem   r1,r2,r3
        \mnem   r4,0xffffffff,r5
        \mnem   r6,r7,0xffffffff
        \mnem   r8,0xffffffff,0xffffffff
        \mnem   0,r9,r10
        \mnem   0,0xffffffff,r11
        \mnem   0,r12,0xffffffff
        \mnem   r13,r14,0x3f
        \mnem   r15,0xffffffff,0x3f
        \mnem   0,r16,0x3f
        \mnem   0,0xffffffff,0x3f
        .endm

        /* crc16 */
        crc_test crc16
        crc_test crc16.r

        /* crc32 */
        crc_test crc32
        crc_test crc32.r
