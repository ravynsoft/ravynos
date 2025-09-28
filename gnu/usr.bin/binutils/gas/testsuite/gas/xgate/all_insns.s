# Example of XGATE instructions
	.sect .text
_start:
L0: 	adc r1, r2, r3
L1:	bcc END_CODE
L2:	add r4, r5, r6
L3:	add r7 , #225
L4:	addh r1, 255
L5:	addl r2, #255
L6:	add  r4, 8004
L7:	and r3, r4, r5
L8:	and r1, #0x8004
L9: 	add r5, END_CODE
L10:    and r7, END_CODE
L11:	and r4, #65281
L12:    andl r3, #01
L13:    andh r6, #255
L14:    asr r0, #3
L15:    asr r1, r2
L16:    bcc END_CODE
L17:    bcs END_CODE
L18:    beq END_CODE
L19:    bfext r3, r4, r5
L20:    bffo r6, r7
L21:    bfins r0, r1, r2
L22:    bfinsi r3, r4, r5
L23:    bfinsx r6, r7, r0
L24:    bge END_CODE
L25:    bgt END_CODE
L26:    bhi END_CODE
L27:    bhs END_CODE
L28:    bith r1, #32
L29:    bitl r2, #0
L30:    ble  END_CODE
L31:    blo END_CODE
L32:    bls END_CODE
L33:	blt END_CODE
L34:	bmi END_CODE
L35:    bne END_CODE
L36:    bpl END_CODE
L37:    bra END_CODE
L38:    brk
L39:    bvc END_CODE
L40:    bvs END_CODE
L41:    cmp r1, r2
L42:    cmpl r3, #255
L43:    com r4, r5
L44:    cpc r6, r7
L45:    cmp r1, #65535
L46:    cpch r2, #255
L47:    csem #4
L48:    csem r5
L49:    csl r6, #11
L50:    csl r7, r0
L51:    csr r1, #2
L52:    csr r2, r3
L53:    jal r4
L54:    ldb r5, (r6, #20)
L55:    ldb r7, (r0, r1+)
L56:    ldb r7, (r0, -r1)
L57:    ldb r0, (r0, r0)
L58:    ldh r1, #255
L59:    ldl r2, #255
L60:	ldd r3, END_CODE
L61:    ldw r4, (r5, #20)
L62:    ldw r5, (r6, r7+)
L63:    ldw r5, (r6, -r7)
L64:    ldw r1, (r2, r4)
L65:    lsl r1, #4
L66:    lsl r2, r3
L67:    lsr r4, #5
L68:    lsr r5, r6
L69:    mov r6, r7
L70:    neg r1, r2
L71:    nop
L72:    or r1, r2, r3
L73:    orh r4, #255
L74:    orl r5, #255
L75:    par r6
L76:    rol r7, #6
L77:    rol r1, r2
L78:    ror r3, #5
L79:    ror r4, r5
L80:    rts
L81:    sbc r1, r2, r3
L82:    ssem #4
L83:    ssem r1
L84:    sex r2
L85:    sif
L86:    sif r4
L87:    stb r5, (r6, #5)
L88:    stb r0, (r0, r0+)
L89:    stb r0, (r0, -r0)
L90:    stb r2, (r0, r0)
L91:    stw r1, (r2, #16)
L92:    stw r1, (r2, r3+)
L93:    stw r1, (r2, -r3)
L94:    stw r2, (r3 ,r4)
L95:    sub r3, r4, r6
L96:    sub r4, #65535
L97:    subh r5, #255
L98:    subl r6, #255
L99:    tfr r7, pc
L100:   tfr r7,ccr
L101:   tfr ccr, r7
L102:   tst r1
L103:   xnor r1, r2, r3
L104:   xnorh r4, #255
L105:   xnorl r5, #255
L106:   com r3
END_CODE:

