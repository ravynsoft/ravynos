.text
all:
   bkpt
   movi    r3, 22
   movi    r16, 22
   movi    r3, 300
   lsli    r20, r16, 0
   lsri    r8, r2, 20
   asri    r1, r2, 7
   addc    r1, r2
   addc    r17, r1
   addc    r1, r2, r3
   addc    r1, r2, r1
   addc    r1, r17
   addc    r18, r18, r30
   subc    r1, r2
   subc    r17, r1
   subc    r1, r2, r3
   subc    r1, r2, r1
   subc    r1, r17
   subc    r18, r18, r30
   cmphs   r3, r4
   cmplt   r3, r4
   cmpne   r3, r4
   mvcv    r3
   mvcv    r16
   and     r1,  r2
   andn    r1,  r2
   tst     r3, r4
   tst     r16, r4
   tstnbz  r3
   tstnbz  r16
   or      r18, r23
   xor     r1,  r1,  r2
   nor     r1,  r2,  r1
   mov     r2, r3
   jmp     r2
   jmp     r16
   jsr     r2
   jsr     r16
   rts
   rts32
   nop
   lsl     r22, r1
   lsr     r1,  r2,  r1
   asr     r1,  r1,  r2
   rotl    r1,  r1,  r16
   zextb   r2, r3
   zexth   r2, r3
   sextb   r2, r3
   sexth   r2, r3
   revb    r2, r3
   revh    r2, r3
   revb    r16, r3
   revh    r2, r16
   mult    r1,  r1,  r17
   mul     r4,  r7
   mulsh   r7,  r18
   muls.h  r2,  r8
   mulsw   r30, r30, r1
   mulsw   r1,  r2
   ld.b    r2, (r3, 4)
   ld.h    r2, (r3, 4)
   ld.w    r2, (r14, 4)
   st.b    r2, (r3, 4)
   st.h    r2, (r3, 4)
   st.w    r2, (r14, 4)
   ld.b    r8, (r3, 4)
   ld.h    r2, (r8, 4)
   ld.w    r2, (r14, 4)
   st.b    r2, (r8, 4)
   st.h    r2, (r8, 4)
   st.w    r8, (r14, 4)
   ld.bs   r2, (r3, 3)
   ld.d    r2, (r3, 4)
   st.d    r2, (r3, 4)
   stex.w   r2, (r3, 4)
   ldex.w   r2, (r3, 4)
   addi    r14,  r14, 0x30
   addi    r3,  r14, 0x4
   addi    r1,  20
   addi    r1,  r1, 20
   addi    r21, 20
   addi    r1,  0x200
   addi    r2,  r4, 1
   addi    r8,  r4, 1
   addi    r1,  r4, 9
   addi    r1,  r28, 9
   addi    r3,  r14, 0x1
   addi    r3,  r14, 0x400
   addi    r14,  r14, 0x33
   addi    r14,  r14, 0x200
   addi16  r1,  20
   addi16  r2,  r4, 1
   addi32  r2,  r4, 1
   addi32  r3,  r14, 0x400
   addi32  r14,  r14, 0x33
   subi    r14,  r14, 0x30
   subi    r1,  20
   subi    r1,  r1, 20
   subi    r21, 20
   subi    r1,  0x200
   subi    r2,  r4, 1
   subi    r8,  r4, 1
   subi    r1,  r4, 9
   subi    r1,  r28, 9
   subi    r14,  r14, 0x33
   subi    r14,  r14, 0x200
   subi16  r1,  20
   subi16  r2,  r4, 1
   subi32  r2,  r4, 1
   subi32  r14,  r14, 0x33
   sub     r3, r0
   sub     r8, r0
   sub     r9, r3, r0
   sub     r3, r3, r0
   sub     r9, r9, r0
   sub     r13, r23, r0
   add     r3, r0
   add     r8, r0
   add     r9, r3, r0
   add     r3, r3, r0
   add     r9, r9, r0
   add     r13, r23, r0
   cmplei  r1,   1
   cmplei  r18,  3
   cmpls   r12, r3
   cmpls   r22, r3
   cmpgt   r2, r2
   cmpgt   r25, r2
   tstle   r5
   tstle   r25
   tstne   r2
   tstne   r24
   tstlt   r4
   tstlt   r24
   setc
   clrc
   rotlc   r2, 1
   rotlc   r16, 1
   sce     5
   trap    2
   clrf    r2
   clrt    r26
   rte
   rfi
   stop
   wait
   doze
   we
   se
   mvc    r23
   mfhis  r3
   mflos  r17
   mvtc
   mfhi   r18
   mthi   r19
   mflo   r3
   mtlo   r8
   sync
   sync   1
   sync   20
   idly   0
   idly   2
   idly   4
   idly   5
   idly   32
   cprc   <1, 1234>
   cpop   <1, 1234>
   cpwgr  r20, <1, 1234>
   cpwcr  r20, <1, 1234>
   cprgr  r20, <1, 1234>
   cprcr  r20, <1, 1234>
   movi   r2, 12
   movih  r2, 16
   bgeni  r2, 12
   bgeni  r2, 16
   pop    r4, r15
   pop    r15, r4-r11
   pop    r28
   pop    r16
   push   r4, r15
   push   r15, r4-r11
   push   r28
   push   r16
   ins    r3, r2, 4, 2
   zext   r3, r2, 4, 2
   sext   r3, r2, 4, 2
   andi   r3, r2, 2
   andni  r3, r2, 2
   xori   r3, r2, 12
   ldm    r3-r5, (r2)
   stm    r3-r5, (r2)
   rsub   r3, r2, r1
   str.b  r3, (r2, r1 << 0)
   str.h  r3, (r2, r1 << 0)
   str.w  r3, (r2, r1 << 0)
   ldr.b  r3, (r2, r1 << 1)
   ldr.h  r3, (r2, r1 << 1)
   ldr.w  r3, (r2, r1 << 1)
   ldr.bs r2, (r2, r3 << 0)
   ldr.bs r2, (r2, r3 << 1)
   ldr.bs r2, (r2, r3 << 2)
   ldr.bs r2, (r2, r3 << 3)
   ldr.hs r2, (r2, r3 << 0)
   ldr.hs r2, (r2, r3 << 1)
   ldr.hs r2, (r2, r3 << 2)
   ldr.hs r2, (r2, r3 << 3)
   xsr    r3, r2, 3
   asrc   r3, r2, 3
   lsrc   r3, r2, 3
   lslc   r3, r2, 3
   rotli  r3, r2, 4
   rotri  r1, 32
   rotri  r1, 1
   rotri  r1, r16, 7
   rotli  r1, 31
   rotli  r1, 0
   rotli  r1, r16, 7
   decne  r3, r2, 4
   declt  r3, r2, 4
   decgt  r3, r2, 4
   dect   r3, r2, 4
   decf   r3, r2, 4
   incf   r3, r2, 4
   inct   r3, r2, 4
   ldq    r4-r7, (r3)
   stq    r4-r7, (r3)
   psrclr ee
   psrclr af, fe
   psrset ee, fe, af
   psrset ie, ee, fe, af
   abs    r17, r20
   bgenr  r3,  r21
   brev   r23, r1
   xtrb0  r5,  r3
   xtrb1  r3,  r9
   xtrb2  r16, r20
   xtrb3  r11, r24
   ff0    r2,  r21
   ff1    r18, r1
   mtcr   r2, cr<3, 0>
   mtcr   r2, cr4
   mtcr   r2, vbr
   mfcr   r2, cr<0, 0>
   mfcr   r2, cr0
   mfcr   r2, psr
   not    r2
   not    r16
   not    r2, r16
   not    r2, r2
   ixh    r2, r3, r4
   ixw    r2, r3, r4
   ixd    r2, r3, r4
   divs   r2, r3, r4
   divu   r2, r3, r4
   movf   r1, r2
   movt   r23, r2
   bmaski r8, 8
   bmaski r1, 0
   bmaski r1, 4
   bmaski r1, 17
   bmaski r21, 16
   bmaski r13, 16
   bmaski r1, 31
   bmaski r1, 32
   pldr   (r2, 0x8)
   pldw   (r2, 0x8)
   neg    r1
   rsubi  r2, 23
   asrc   r3
   incf   r4
   inct   r13
   decf   r16
   decgt  r17
   declt  r19
   decne  r20
   dect   r31
   lslc   r11
   lsrc   r25
   xsr    r12
   divs   r23, r3
   divu   r1,  r30
   abs    r13
   brev   r12
   ff1    r8
   not    r1
   not    r17
   zextb  r2
   zexth  r19
   sextb  r29
   sexth  r11
   ixh    r1,  r17
   ixw    r23, r1
   rsub   r3,  r3
   rsub   r17, r31
   lsri   r1,  12
   lsli   r21, 2
   mulsw  r1, r2
   andi   r3, 123
   rori   r8, 21
   bt     all
   bf     all
   jbt    all
   jbf    all
   br     all
   jbr    all
   bsr    all
   srs.b  r2, [all]
   lrs.b  r2, [all]
   srs.h  r2, [all]
   lrs.h  r2, [all]
   srs.w  r2, [all]
   lrs.w  r2, [all]
   ori    r2, r3, 2
   ori    r2, r3, 10
