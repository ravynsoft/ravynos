.text
all:
      bkpt
      sync
      rte
      rfe
      rfi
      stop
      wait
      doze
      idly4
      trap 3
      mvc  r1
      mvcv r2
      ldq r4-r7, (r2)
      stq r4-r7, (r2)
      ldm r1-r15, (r0)
      dect r2
      decf r2
      inct r2
      incf r2
      jmp  r1
      jsr  r1
      ff1  r11
      brev r1
      xtrb3 r2
      xtrb2 r2
      xtrb1 r2
      xtrb0 r2
      xtrb0 r1, r2
      xtrb0 r3, r2
      zextb r2
      sextb r2
      zexth r2
      sexth r2
      declt r2
      decne r1
      decgt r1
      clrt  r1
      clrf  r1
      abs   r3
      not   r12
      movt  r1, r2
      mult  r3, r4
      sub   r7, r8
      subu  r7, r8
      addc  r9, r10
      subc  r11, r12
      movf  r12, r13
      lsr   r12, r13
      cmphs r13, r14
      cmplt r13, r14
      tst r13, r14
      cmpne r13, r14
      psrclr ee, ie, fe
      mov   r3, r5
      bgenr r2, r3
      and   r3, r4
      ixw   r3, r4
      asr   r3, r4
      addu  r3, r4
      ixh   r2, r3
      andn  r3, r4
      addi  r3, 32
      cmplti  r3, 32
      subi  r3, 2
      rsubi  r3, 2
      cmpnei  r3, 3
      bmaski  r3, 8
      divu  r3, r1
      mflos r2
      mfhis r2
      mtlo r2
      mthi r2
      mflo r2
      mfhi r2
      andi  r3, 3
      bclri  r3, 3
      bgeni  r3, 9
      bgeni  r3, 6
      divs  r3, r1
      divs  r3, r2
      bseti  r3, 9
      btsti  r3, 9
      xsr   r3
      rotli r3, 2
      asrc  r3
      asri r1, 3
      movi r7, 127
      ld   r2, (r0, 0)
      ldw  r2, (r0, 4)
      ld.w  r2, (r0, 8)
      st   r2, (r0, 0)
      stw  r2, (r0, 4)
      st.w  r2, (r0, 8)
      ldh  r2, (r0, 2)
      ld.h  r2, (r0, 4)
      sth  r2, (r0, 2)
      st.h  r2, (r0, 4)
      ldb  r2, (r0, 0)
      ld.b  r2, (r0, 1)
      stb  r2, (r0, 0)
      st.b  r2, (r0, 1)
      bt    all
      bf    all
      br    all
      setc
      clrc
      tstle r5
      cmplei r3, 6
      neg   r7
      tstne r6
      tstlt r0
      mclri  r3, 0x80000000
      mgeni  r4, 0x40
      mgeni  r4, 0x80
      mseti  r1, 0x10000
      mtsti r4, 16
      rori  r6, 20
      rotri r2, 1
      nop
      idly  4
      rolc  r4, 1
      rotlc r5, 1
      sxtrb0 r1, r2
      sxtrb1 r1, r3
      sxtrb2 r1, r4
      movtf r1, r2, r3
      addc64 r2, r2, r7
      subc64 r4, r4, r6
      or64   r6, r6, r2
      xor64  r5, r5, r1
