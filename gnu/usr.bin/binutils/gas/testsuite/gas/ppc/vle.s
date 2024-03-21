# Freescale PowerPC VLE instruction tests
#as: -mvle
	.text
	.extern extern_subr
	.equ	UI8,0x37
	.equ	SCI0,UI8<<0
	.equ	SCI1,UI8<<8
	.equ	SCI2,UI8<<16
	.equ	SCI3,UI8<<24
	.equ	r0,0
	.equ	r1,1
	.equ	r2,2
	.equ	r3,3
	.equ	r4,4
	.equ	r5,5
	.equ	r6,6
	.equ	r7,7
	.equ	r8,8
	.equ	r9,9
	.equ	r10,10
	.equ	r11,11
	.equ	r12,12
	.equ	r13,13
	.equ	r14,14
	.equ	r15,15
	.equ	r16,16
	.equ	r17,17
	.equ	r18,18
	.equ	r19,19
	.equ	r20,20
	.equ	r21,21
	.equ	r22,22
	.equ	r23,23
	.equ	r24,24
	.equ	r25,25
	.equ	r26,26
	.equ	r27,27
	.equ	r28,28
	.equ	r29,29
	.equ	r30,30
	.equ	r31,31
	.equ	r32,32
	.equ	rsp,r1


start_label:
	e_add16i	r4,r3,27
	e_add2i.	r0,0x3456
	e_add2is	r1,0x4321
	e_addi.		r2,r6,SCI0
	e_addi		r3,r5,SCI1
	e_addic.	r4,r4,SCI2
	e_addic		r7,r8,SCI3
	e_and2i.	r9,0xfeed
	e_and2is.	r10,5
	e_andi.		r11,r13,0x39
	e_andi		r12,r15,SCI2
	e_b		middle_label
	e_bl		extern_subr
	e_bc		0,3,start_label
	e_bcl		1,15,extern_subr
	e_cmp16i	r2,0x3333
	e_cmpi		2,r6,SCI1
	e_cmph		1,r7,r11
	e_cmph16i	r12,0xfdef
	e_cmphl		0,r6,r8
	e_cmphl16i	r13,0x1234
	e_cmpl16i	r1, 0xfee0
	e_cmpli		1,r3,SCI3
	e_crand		0x1d,3,0
	e_crandc	0,2,0x1d
	e_creqv		15,16,17
	e_crnand	0xf,0,3
	e_crnor		0xf,0,3
	e_cror		12,13,14
	e_crorc		19,18,17
	e_crxor		0,0,0
	e_lbz		r7,0xffffcc0d(r3)
	e_lbzu		r7,-52(r5)
	e_lha		r8,0x1ff(r10)
	e_lhau		r8,-1(r1)
	e_lhz		r7,6200(r0)
	e_lhzu		r7,62(r0)
	e_li		r0,0x33333
	e_lis		r1,0x3333
	e_lmw		r5,24(r3)
	e_lwz		r5,10024(r3)
	e_lwzu		r6,0x72(r2)
	e_mcrf		1,6
	e_mulli		r9,r10,SCI0
	e_mull2i	r1,0x668
	e_or2i		r5,0x2345
	e_or2is		r5,0xa345
	e_ori.		r7,r9,SCI0
	e_ori		r7,r8,SCI1
	e_rlw		r18, r22,r0
	e_rlw.		r8, r2,r0
	e_rlwi		r20,r3,21
	e_rlwi.		r2,r3,21
	e_rlwimi	r4,r19,13,8,15
	e_rlwinm	r4,r1,13,1,17
	e_slwi		r12,r19,6
	e_slwi.		r12,r10,20
	e_srwi		r0,r1,16
	e_srwi.		r0,r1,11
	e_stb		r3,22000(r1)
	e_stbu		r19,-4(r22)
	e_sth		r0,666(r21)
	e_sthu		r1,-1(r23)
	e_stmw		r0,4(r3)
	e_stw		r3,16161(r0)
	e_stwu		r22,0xffffffee(r4)
	e_subfic	r0,r21,SCI2
	e_subfic.	r22,r0,SCI3
	e_xori		r21,r3,SCI1
	e_xori.		r0,r20,SCI0
middle_label:
	se_add		r31,r7
	se_addi		r28,0x1f
	se_and		r0,r1
	se_and.		r1,r0
	se_andc		r2, r3
	se_andi		r4,0x11
	se_b		middle_label
	se_bl		extern_subr
	se_bc		1,3,not_end_label
	se_bclri	r27,0x12
	se_bctr
	se_bctrl
	se_bgeni	r7,17
	se_blr
	se_blrl
	se_bmaski	r6,0
	se_bseti	r0,1
	se_btsti	r4,7
	se_cmp		r0,r1
	se_cmph		r31,r28
	se_cmphl	r1,r25
	se_cmpi		r3,22
	se_cmpl		r6,r7
	se_cmpli	r28,0xc
	se_extsb	r1
	se_extsh	r2
	se_extzb	r30
	se_extzh	r24
not_end_label:
	se_illegal
	se_isync
	se_lbz		r1,8(r24)
	se_lhz		r24,18(r4)
	se_li		r4,0x4f
	se_lwz		r6,60(r0)
	se_mfar		r7,r8
	se_mfctr	r3
	se_mflr		r4
	se_mr		r31,r0
	se_mtar		r23,r2
	se_mtctr	r6
	se_mtlr		r31
	se_mullw	r3,r4
	se_neg		r24
	se_not		r25
	se_or		r0,r1
	se_rfci
	se_rfdi
	se_rfi
	se_sc
	se_slw		r5,r6
	se_slwi		r7,7
	se_sraw		r6,r30
	se_srawi	r25,8
	se_srw		r30,r0
	se_srwi		r29,25
	se_stb		r0,10(r2)
	se_sth		r1,12(r30)
	se_stw		r7,0(r29)
	se_sub		r1,r2
	se_subf		r29,r26
	se_subi		r7,24
end_label:
	se_subi.	r25,19
	se_bl		middle_label
	e_b		middle_label
	e_bl		start_label
	se_rfgi
	e_sc
	e_sc		0
	e_sc		1
