 .data
foodata: .hword 42
 .text
footext:
	.text
	.global beq16

	.text
	.global beq
bgt16:
	bgt 4
	bgt 4
	bgt -4
	bgt footext
	bgt foodata
	bgt 4
	bgt footext
	bgt footext
	.text
	.global bgt

bgtu16:
	bgtu 4
	bgtu -4
	bgtu footext
	bgtu 4
	bgtu -4
	bgtu footext
	bgtu footext
	bgtu 4
	.text
	.global bgtu

bgte16:
	bgte footext
	bgte footext
	bgte footext
	bgte footext
	bgte footext
	bgte -4
	bgte foodata
	bgte foodata

	.text
	.global bgteu16
bgteu16:
	bgteu 4
	bgteu -4
	bgteu foodata
	bgteu 4
	bgteu footext
	bgteu 4
	bgteu foodata
	bgteu foodata
	.text
	.global bgteu
bgteu:
	.text
	.global blt16
blt16:
	blt -4
	blt 4
	blt -4
	blt 4
	blt -4
	blt 4
	blt foodata
	blt foodata
	.text
	.global blt
blt:
	.text
	.global bltu16
bltu16:
	bltu -4
	bltu 4
	bltu -4
	bltu footext
	bltu footext
	bltu footext
	bltu 4
	bltu foodata
	.text
	.global bltu
bltu:
	.text
	.global blte16
blte16:
	blte footext
	blte foodata
	blte foodata
	blte footext
	blte -4
	blte footext
	blte footext
	blte 4
	.text
	.global blte
blte:
	.text
	.global blteu16
blteu16:
	blteu footext
	blteu foodata
	blteu footext
	blteu foodata
	blteu footext
	blteu -4
	blteu foodata
	blteu foodata
	.text
	.global blteu
blteu:
	.text
	.global bbeq16
bbeq16:
	bbeq footext
	bbeq footext
	bbeq foodata
	bbeq footext
	bbeq 4
	bbeq foodata
	bbeq foodata
	bbeq 4
	.text
	.global bbeq
bbeq:
	.text
	.global bbne16
bbne16:
	bbne foodata
	bbne -4
	bbne 4
	bbne footext
	bbne 4
	bbne 4
	bbne footext
	bbne footext
	.text
	.global bbne
bbne:
	.text
	.global bblt16
bblt16:
	bblt foodata
	bblt 4
	bblt 4
	bblt 4
	bblt -4
	bblt 4
	bblt footext
	bblt -4
	.text
	.global bblt
bblt:
	.text
	.global bblte16
bblte16:
	bblte 4
	bblte 4
	bblte footext
	bblte footext
	bblte 4
	bblte -4
	bblte foodata
	bblte 4
	.text
	.global bblte
bblte:
	.text
	.global b16
b16:
	b footext
	b footext
	b 4
	b -4
	b footext
	b foodata
	b foodata
	b -4
	.text
	.global b
b:
	.text
	.global bl16
bl16:
	bl -4
	bl 4
	bl footext
	bl -4
	bl footext
	bl -4
	bl -4
	bl footext
	.text
	.global bl
bl:
	.text
	.global jr16
jr16:
	jr ip
	jr r3
	jr r0
	jr fp
	jr sp
	jr r0
	jr r3
	jr r0
	.text
	.global jr
jr:
	jr ip
	jr r59
	jr r28
	jr r27
	jr sp
	jr r51
	jr r56
	jr r45
	.text
	.global jalr16
jalr16:
	jalr ip
	jalr r3
	jalr r0
	jalr fp
	jalr sp
	jalr r3
	jalr fp
	jalr ip
	.text
	.global jalr
jalr:
	jalr ip
	jalr r59
	jalr r28
	jalr r27
	jalr sp
	jalr r11
	jalr r28
	jalr r59
	.text
	.global ldrbx16
ldrbx16:
	ldrb ip,[ip,ip]
	ldrb r3,[r3,r3]
	ldrb r0,[r0,r0]
	ldrb fp,[fp,fp]
	ldrb sp,[sp,sp]
	ldrb ip,[r0,r0]
	ldrb r3,[r2,lr]
	ldrb r2,[ip,r3]
	.text
	.global ldrbp16
ldrbp16:
	ldrb sp,[r0],fp
	ldrb lr,[r1],ip
	ldrb fp,[r0],fp
	.text
	.global ldrbx
ldrbx:
	ldrb ip,[ip,ip]
	ldrb r59,[r59,r59]
	ldrb r28,[r28,r28]
	ldrb r27,[r27,r27]
	ldrb sp,[sp,sp]
	ldrb r41,[r18,r47]
	ldrb r43,[r16,r21]
	ldrb r32,[r8,r8]
	.text
	.global ldrbp
ldrbp:
	ldrb r36,[r49],r18
	ldrb r32,[r59],r50
	ldrb r58,[r11],r25
	.text
	.global ldrbd16
ldrbd16:
	ldrb ip,[ip,0]
	ldrb r3,[r3,7]
	ldrb r0,[r0,4]
	ldrb fp,[fp,3]
	ldrb sp,[sp,1]
	ldrb lr,[sp,1]
	ldrb r1,[r0,0]
	ldrb r1,[r1,1]
	.text
	.global ldrbd
ldrbd:
	ldrb ip,[ip,0]
	ldrb r59,[r59,2047]
	ldrb r28,[r28,1024]
	ldrb r27,[r27,1023]
	ldrb sp,[sp,1]
	ldrb r7,[r33,1574]
	ldrb r31,[r6,1957]
	ldrb r10,[r0,1831]
	.text
	.global ldrhx16
ldrhx16:
	ldrh ip,[ip,ip]
	ldrh r3,[r3,r3]
	ldrh r0,[r0,r0]
	ldrh fp,[fp,fp]
	ldrh sp,[sp,sp]
	ldrh r0,[r0,lr]
	ldrh lr,[lr,sp]
	ldrh r0,[fp,fp]
	.text
	.global ldrhp16
ldrhp16:
	ldrh r2,[sp],fp
	ldrh r22,[sp],fp
	.text
	.global ldrhx
ldrhx:
	ldrh ip,[ip,ip]
	ldrh r59,[r59,r59]
	ldrh r28,[r28,r28]
	ldrh r27,[r27,r27]
	ldrh sp,[sp,sp]
	ldrh r46,[r17,r21]
	ldrh r30,[r1,r47]
	ldrh r43,[r19,r20]
	.text
	.global ldrhp
ldrhp:



	ldrh r32,[r31],r29
	ldrh r52,[r47],r10
	ldrh r31,[r40],r3
	.text
	.global ldrhd16
ldrhd16:
	ldrh ip,[ip,0]
	ldrh r3,[r3,7]
	ldrh r0,[r0,4]
	ldrh fp,[fp,3]
	ldrh sp,[sp,1]
	ldrh lr,[r2,0]
	ldrh r3,[r0,7]
	ldrh r0,[r3,6]
	.text
	.global ldrhd
ldrhd:
	ldrh ip,[ip,0]
	ldrh r59,[r59,2047]
	ldrh r28,[r28,1024]
	ldrh r27,[r27,1023]
	ldrh sp,[sp,1]
	ldrh r45,[r24,1221]
	ldrh r36,[r43,1738]
	ldrh r42,[r48,25]
	.text
	.global ldrx16
ldrx16:
	ldr ip,[ip,ip]
	ldr r3,[r3,r3]
	ldr r0,[r0,r0]
	ldr fp,[fp,fp]
	ldr sp,[sp,sp]
	ldr r3,[fp,lr]
	ldr ip,[lr,r2]
	ldr r3,[r2,lr]
	.text
	.global ldrp16
ldrp16:
	ldr lr,[fp],sp
	ldr r0,[sp],r0
	ldr fp,[r2],r1
	.text
	.global ldrx
ldrx:
	ldr ip,[ip,ip]
	ldr r59,[r59,r59]
	ldr r28,[r28,r28]
	ldr r27,[r27,r27]
	ldr sp,[sp,sp]
	ldr r24,[r16,r47]
	ldr r22,[r41,r49]
	ldr r14,[fp,r39]
	.text
	.global ldrp
ldrp:
	ldr r21,[r5],r30
	ldr r36,[r12],r14
	ldr r12,[r4],r11
	.text
	.global ldrd16
ldrd16:
	ldr ip,[ip,0]
	ldr r3,[r3,7]
	ldr r0,[r0,4]
	ldr fp,[fp,3]
	ldr sp,[sp,1]
	ldr r0,[sp,0]
	ldr ip,[r1,7]
	ldr fp,[r1,1]
	.text
	.global ldrd
ldrd:
	ldr ip,[ip,0]
	ldr r59,[r59,2047]
	ldr r28,[r28,1024]
	ldr r27,[r27,1023]
	ldr sp,[sp,1]
	ldr r22,[r30,975]
	ldr r7,[r44,1361]
	ldr r23,[r19,1855]
	.text
	.global ldrdx16
ldrdx16:
	ldrd ip,[ip,ip]
	ldrd r4,[r3,r3]
	ldrd r0,[r0,r0]
	ldrd r14,[fp,fp]
	ldrd r16,[sp,sp]
	ldrd r30,[r2,ip]
	ldrd r0,[fp,r3]
	ldrd r20,[ip,lr]
	.text
	.global ldrdp16
ldrdp16:
	ldrd r4,[r3],r3
	ldrd r16,[fp],fp
	ldrd r20,[sp],sp
	ldrd r10,[ip],r1
	ldrd r30,[fp],lr
	ldrd r62,[lr],sp
	.text
	.global ldrdx
ldrdx:
	ldrd ip,[ip,ip]
	ldrd r58,[r59,r59]
	ldrd r28,[r28,r28]
	ldrd r26,[r27,r27]
	ldrd r12,[sp,sp]
	ldrd r32,[fp,r59]
	ldrd r4,[r17,r6]
	ldrd r32,[r40,r1]
	.text
	.global ldrdp
ldrdp:
	ldrd r16,[sp],sp
	ldrd r46,[r33],r30
	ldrd r24,[r36],r59
	ldrd r58,[r32],r11
	.text
	.global ldrdd16
ldrdd16:
	ldrd ip,[ip,0]
	ldrd r4,[r3,7]
	ldrd r0,[r0,4]
	ldrd r16,[fp,3]
	ldrd r18,[sp,1]
	ldrd r0,[fp,3]
	ldrd lr,[fp,7]
	ldrd lr,[ip,1]
	.text
	.global ldrdd
ldrdd:
	ldrd ip,[ip,0]
	ldrd r58,[r59,2047]
	ldrd r28,[r28,1024]
	ldrd r2,[r27,1023]
	ldrd r16,[sp,1]
	ldrd r4,[r21,761]
	ldrd lr,[r41,1553]
	ldrd r6,[r14,1922]
	.text
	.global strbx16
strbx16:
	strb ip,[ip,ip]
	strb r3,[r3,r3]
	strb r0,[r0,r0]
	strb fp,[fp,fp]
	strb sp,[sp,sp]
	strb r1,[lr,r3]
	strb ip,[r3,lr]
	strb lr,[ip,ip]
	.text
	.global strbx
strbx:
	strb ip,[ip,ip]
	strb r59,[r59,r59]
	strb r28,[r28,r28]
	strb r27,[r27,r27]
	strb sp,[sp,sp]
	strb r50,[r15,sp]
	strb lr,[fp,r52]
	strb r14,[r24,r51]
	.text
	.global strbp16
strbp16:
	strb ip,[ip],ip
	strb r3,[r3],r3
	strb r0,[r0],r0
	strb fp,[fp],fp
	strb sp,[sp],sp
	strb r2,[fp],ip
	strb fp,[r0],r1
	strb r2,[r2],r3
	.text
	.global strbp
strbp:
	strb ip,[ip],ip
	strb r59,[r59],r59
	strb r28,[r28],r28
	strb r27,[r27],r27
	strb sp,[sp],sp
	strb r14,[r51],r2
	strb r6,[r44],r50
	strb r44,[r9],r49
	.text
	.global strbd16
strbd16:
	strb ip,[ip,0]
	strb r3,[r3,7]
	strb r0,[r0,4]
	strb fp,[fp,3]
	strb sp,[sp,1]
	strb r0,[r2,1]
	strb sp,[r2,3]
	strb fp,[r2,4]
	.text
	.global strbd
strbd:
	strb ip,[ip,0]
	strb r59,[r59,2047]
	strb r28,[r28,1024]
	strb r27,[r27,1023]
	strb sp,[sp,1]
	strb r23,[r10,1404]
	strb r12,[r35,1461]
	strb r54,[r58,1090]
	.text
	.global strhx16
strhx16:
	strh ip,[ip,ip]
	strh r3,[r3,r3]
	strh r0,[r0,r0]
	strh fp,[fp,fp]
	strh sp,[sp,sp]
	strh r0,[r3,r1]
	strh r1,[fp,r2]
	strh r3,[r3,fp]
	.text
	.global strhx
strhx:
	strh ip,[ip,ip]
	strh r59,[r59,r59]
	strh r28,[r28,r28]
	strh r27,[r27,r27]
	strh sp,[sp,sp]
	strh r16,[r38,r31]
	strh r32,[r12,r28]
	strh r57,[r11,r9]
	.text
	.global strhp16
strhp16:
	strh ip,[ip],ip
	strh r3,[r3],r3
	strh r0,[r0],r0
	strh fp,[fp],fp
	strh sp,[sp],sp
	strh r0,[r2],sp
	strh sp,[r3],r0
	strh r1,[r0],r0
	.text
	.global strhp
strhp:
	strh ip,[ip],ip
	strh r59,[r59],r59
	strh r28,[r28],r28
	strh r27,[r27],r27
	strh sp,[sp],sp
	strh r3,[r37],r54
	strh r4,[r54],r25
	strh r5,[r32],r25
	.text
	.global strhd16
strhd16:
	strh ip,[ip,0]
	strh r3,[r3,7]
	strh r0,[r0,4]
	strh fp,[fp,3]
	strh sp,[sp,1]
	strh r3,[r0,3]
	strh lr,[ip,7]
	strh r3,[r2,7]
	.text
	.global strhd
strhd:
	strh ip,[ip,0]
	strh r59,[r59,2047]
	strh r28,[r28,1024]
	strh r27,[r27,1023]
	strh sp,[sp,1]
	strh r7,[r38,1181]
	strh r25,[r4,77]
	strh r11,[fp,631]
	.text
	.global strx16
strx16:
	str ip,[ip,ip]
	str r3,[r3,r3]
	str r0,[r0,r0]
	str fp,[fp,fp]
	str sp,[sp,sp]
	str lr,[r3,r3]
	str r3,[fp,r0]
	str ip,[sp,r1]
	.text
	.global strx
strx:
	str ip,[ip,ip]
	str r59,[r59,r59]
	str r28,[r28,r28]
	str r27,[r27,r27]
	str sp,[sp,sp]
	str r53,[r29,r28]
	str r30,[r22,r34]
	str r28,[r28,r44]
	.text
	.global strp16
strp16:
	str ip,[ip],ip
	str r3,[r3],r3
	str r0,[r0],r0
	str fp,[fp],fp
	str sp,[sp],sp
	str lr,[r0],r0
	str fp,[r0],sp
	str r3,[fp],r0
	.text
	.global strp
strp:
	str ip,[ip],ip
	str r59,[r59],r59
	str r28,[r28],r28
	str r27,[r27],r27
	str sp,[sp],sp
	str r22,[r36],r15
	str r44,[r13],r47
	str r19,[r48],sp
	.text
	.global strd16
strd16:
	str ip,[ip,0]
	str r3,[r3,7]
	str r0,[r0,4]
	str fp,[fp,3]
	str sp,[sp,1]
	str r3,[fp,3]
	str sp,[ip,6]
	str r1,[lr,3]
	.text
	.global strd
strd:
	str ip,[ip,0]
	str r59,[r59,2047]
	str r28,[r28,1024]
	str r27,[r27,1023]
	str sp,[sp,1]
	str r45,[r44,74]
	str r58,[r50,370]
	str r40,[r3,626]
	.text
	.global strdx16
strdx16:
	strd ip,[ip,ip]
	strd r2,[r3,r3]
	strd r0,[r0,r0]
	strd r16,[fp,fp]
	strd r18,[sp,sp]
	strd ip,[r3,r1]
	strd r2,[lr,fp]
	strd ip,[r2,r2]
	.text
	.global strdx
strdx:
	strd ip,[ip,ip]
	strd r58,[r59,r59]
	strd r28,[r28,r28]
	strd r26,[r27,r27]
	strd r14,[sp,sp]
	strd r38,[r53,lr]
	strd r24,[r19,r43]
	strd r12,[r10,r30]
	.text
	.global strdp16
strdp16:
	strd ip,[ip],ip
	strd r2,[r3],r3
	strd r0,[r0],r0
	strd r6,[fp],fp
	strd r4,[sp],sp
	strd r2,[r3],r0
	strd r2,[r0],r1
	strd r2,[lr],r1
	.text
	.global strdp
strdp:
	strd ip,[ip],ip
	strd r58,[r59],r59
	strd r28,[r28],r28
	strd r26,[r27],r27
	strd r22,[sp],sp
	strd r6,[r10],r44
	strd r10,[r43],r5
	strd r46,[r17],lr
	.text
	.global strdd16
strdd16:
	strd r0,[ip,0]
	strd r2,[r3,7]
	strd r0,[r0,4]
	strd r2,[fp,3]
	strd r4,[sp,1]
	strd r2,[r2,5]
	strd r6,[r3,7]
	strd r6,[r1,2]
	.text
	.global strdd
strdd:
	strd ip,[ip,0]
	strd r58,[r59,2047]
	strd r28,[r28,1024]
	strd r26,[r27,1023]
	strd r14,[sp,1]
	strd r28,[r52,719]
	strd r40,[r53,1994]
	strd r44,[r57,494]
	.text
	.global mov16EQ
mov16EQ:
	moveq ip,ip
	moveq r3,r3
	moveq r0,r0
	moveq fp,fp
	moveq sp,sp
	moveq ip,r2
	moveq r2,fp
	moveq fp,sp
	.text
	.global movEQ
movEQ:
	moveq ip,ip
	moveq r59,r59
	moveq r28,r28
	moveq r27,r27
	moveq sp,sp
	moveq r32,r30
	moveq r43,r39
	moveq r25,r33
	.text
	.global mov16NE
mov16NE:
	movne ip,ip
	movne r3,r3
	movne r0,r0
	movne fp,fp
	movne sp,sp
	movne r3,r3
	movne r0,fp
	movne fp,fp
	.text
	.global movNE
movNE:
	movne ip,ip
	movne r59,r59
	movne r28,r28
	movne r27,r27
	movne sp,sp
	movne r4,r3
	movne r28,fp
	movne r23,r39
	.text
	.global mov16GT
mov16GT:
	movgt ip,ip
	movgt r3,r3
	movgt r0,r0
	movgt fp,fp
	movgt sp,sp
	movgt r1,r3
	movgt lr,r3
	movgt r1,ip
	.text
	.global movGT
movGT:
	movgt ip,ip
	movgt r59,r59
	movgt r28,r28
	movgt r27,r27
	movgt sp,sp
	movgt r1,r21
	movgt r13,r3
	movgt r28,r43
	.text
	.global mov16GTU
mov16GTU:
	movgtu ip,ip
	movgtu r3,r3
	movgtu r0,r0
	movgtu fp,fp
	movgtu sp,sp
	movgtu ip,lr
	movgtu sp,ip
	movgtu ip,sp
	.text
	.global movGTU
movGTU:
	movgtu ip,ip
	movgtu r59,r59
	movgtu r28,r28
	movgtu r27,r27
	movgtu sp,sp
	movgtu r34,r33
	movgtu r17,r48
	movgtu r35,r24
	.text
	.global mov16GTE
mov16GTE:
	movgte ip,ip
	movgte r3,r3
	movgte r0,r0
	movgte fp,fp
	movgte sp,sp
	movgte r0,r0
	movgte r2,sp
	movgte lr,r2
	.text
	.global movGTE
movGTE:
	movgte ip,ip
	movgte r59,r59
	movgte r28,r28
	movgte r27,r27
	movgte sp,sp
	movgte ip,r59
	movgte r37,r42
	movgte r44,r26
	.text
	.global mov16GTEU
mov16GTEU:
	movgteu ip,ip
	movgteu r3,r3
	movgteu r0,r0
	movgteu fp,fp
	movgteu sp,sp
	movgteu lr,ip
	movgteu sp,r1
	movgteu ip,lr
	.text
	.global movGTEU
movGTEU:
	movgteu ip,ip
	movgteu r59,r59
	movgteu r28,r28
	movgteu r27,r27
	movgteu sp,sp
	movgteu r58,r47
	movgteu r56,r5
	movgteu r20,r52
	.text
	.global mov16LT
mov16LT:
	movlt ip,ip
	movlt r3,r3
	movlt r0,r0
	movlt fp,fp
	movlt sp,sp
	movlt r3,r3
	movlt r2,r2
	movlt ip,lr
	.text
	.global movLT
movLT:
	movlt ip,ip
	movlt r59,r59
	movlt r28,r28
	movlt r27,r27
	movlt sp,sp
	movlt r52,r12
	movlt r57,r22
	movlt r8,r7
	.text
	.global mov16LTU
mov16LTU:
	movltu ip,ip
	movltu r3,r3
	movltu r0,r0
	movltu fp,fp
	movltu sp,sp
	movltu ip,r2
	movltu sp,ip
	movltu r1,r0
	.text
	.global movLTU
movLTU:
	movltu ip,ip
	movltu r59,r59
	movltu r28,r28
	movltu r27,r27
	movltu sp,sp
	movltu r13,r31
	movltu r43,ip
	movltu r7,r56
	.text
	.global mov16LTE
mov16LTE:
	movlte ip,ip
	movlte r3,r3
	movlte r0,r0
	movlte fp,fp
	movlte sp,sp
	movlte r0,r3
	movlte r3,ip
	movlte r3,lr
	.text
	.global movLTE
movLTE:
	movlte ip,ip
	movlte r59,r59
	movlte r28,r28
	movlte r27,r27
	movlte sp,sp
	movlte r30,r27
	movlte r35,r52
	movlte r15,r53
	.text
	.global mov16LTEU
mov16LTEU:
	movlteu ip,ip
	movlteu r3,r3
	movlteu r0,r0
	movlteu fp,fp
	movlteu sp,sp
	movlteu ip,lr
	movlteu r2,r2
	movlteu r2,fp
	.text
	.global movLTEU
movLTEU:
	movlteu ip,ip
	movlteu r59,r59
	movlteu r28,r28
	movlteu r27,r27
	movlteu sp,sp
	movlteu r31,r36
	movlteu r24,r50
	movlteu r52,r54
	.text
	.global mov16B
mov16B:
	mov ip,ip
	mov r3,r3
	mov r0,r0
	mov fp,fp
	mov sp,sp
	mov ip,r1
	mov ip,r0
	mov r0,ip
	.text
	.global movB
movB:
	mov ip,ip
	mov r59,r59
	mov r28,r28
	mov r27,r27
	mov sp,sp
	mov r1,r59
	mov r28,r12
	mov r5,r42
	.text
	.global mov16BEQ
mov16BEQ:
	movbeq ip,ip
	movbeq r3,r3
	movbeq r0,r0
	movbeq fp,fp
	movbeq sp,sp
	movbeq lr,r2
	movbeq fp,r2
	movbeq ip,r1
	.text
	.global movBEQ
movBEQ:
	movbeq ip,ip
	movbeq r59,r59
	movbeq r28,r28
	movbeq r27,r27
	movbeq sp,sp
	movbeq r29,r16
	movbeq r18,r46
	movbeq lr,r1
	.text
	.global mov16BNE
mov16BNE:
	movbne ip,ip
	movbne r3,r3
	movbne r0,r0
	movbne fp,fp
	movbne sp,sp
	movbne r1,r2
	movbne ip,r1
	movbne ip,r3
	.text
	.global movBNE
movBNE:
	movbne ip,ip
	movbne r59,r59
	movbne r28,r28
	movbne r27,r27
	movbne sp,sp
	movbne r15,r7
	movbne r24,r43
	movbne r23,r52
	.text
	.global mov16BLT
mov16BLT:
	movblt ip,ip
	movblt r3,r3
	movblt r0,r0
	movblt fp,fp
	movblt sp,sp
	movblt sp,lr
	movblt ip,lr
	movblt lr,sp
	.text
	.global movBLT
movBLT:
	movblt ip,ip
	movblt r59,r59
	movblt r28,r28
	movblt r27,r27
	movblt sp,sp
	movblt r52,r44
	movblt r57,r35
	movblt r53,r33
	.text
	.global mov16BLTE
mov16BLTE:
	movblte ip,ip
	movblte r3,r3
	movblte r0,r0
	movblte fp,fp
	movblte sp,sp
	movblte sp,ip
	movblte r0,fp
	movblte r0,sp
	.text
	.global movBLTE
movBLTE:
	movblte ip,ip
	movblte r59,r59
	movblte r28,r28
	movblte r27,r27
	movblte sp,sp
	movblte r58,r44
	movblte r35,r22
	movblte r8,r2
	.text
	.global movts16
movts16:
	movts config,ip
	movts ipend,r3
	movts iret,r0
	movts debug,fp
	movts status,sp
	movts status,fp
	movts pc,fp
	movts imask,r0
	.text
	.global movts
movts:
	movts config,ip
	movts ipend,r59
	movts iret,r28
	movts debug,r27
	movts status,sp
	movts debug,r50
	movts ipend,r33
	movts status,ip
	.text
	.global movfs16
movfs16:
	movfs ip,config
	movfs r3,ipend
	movfs r0,iret
	movfs fp,debug
	movfs sp,status
	movfs r1,iret
	movfs r2,status
	movfs lr,debug
	.text
	.global movfs
movfs:
	movfs ip,config
	movfs r59,ipend
	movfs r28,iret
	movfs r27,debug
	movfs sp,status
	movfs r13,debug
	movfs r15,status
	movfs r16,imask
	.text
	.global nop
nop:
	nop
	.text
	.global idle
idle:
	idle
	.text
	.global bkpt
bkpt:
	bkpt
	.text
	.global rti
rti:
	rti
	.text
	.global trap16
trap16:
	trap 0
	trap 7
	trap 4
	trap 3
	trap 1
	trap 6
	trap 3
	trap 5
	.text
	.global add16
add16:
	add ip,ip,ip
	add r3,r3,r3
	add r0,r0,r0
	add fp,fp,fp
	add sp,sp,sp
	add sp,r2,lr
	add r0,r2,r1
	add ip,fp,fp
	.text
	.global add
add:
	add ip,ip,ip
	add r59,r59,r59
	add r28,r28,r28
	add r27,r27,r27
	add sp,sp,sp
	add r56,r10,r16
	add r36,r25,r34
	add r2,r49,r17
	.text
	.global sub16
sub16:
	sub ip,ip,ip
	sub r3,r3,r3
	sub r0,r0,r0
	sub fp,fp,fp
	sub sp,sp,sp
	sub r2,ip,lr
	sub lr,lr,r0
	sub r3,r3,r3
	.text
	.global sub
sub:
	sub ip,ip,ip
	sub r59,r59,r59
	sub r28,r28,r28
	sub r27,r27,r27
	sub sp,sp,sp
	sub ip,lr,r20
	sub r48,r22,r47
	sub r19,r48,r13
	.text
	.global and16
and16:
	and ip,ip,ip
	and r3,r3,r3
	and r0,r0,r0
	and fp,fp,fp
	and sp,sp,sp
	and fp,sp,r3
	and r3,r3,r3
	and ip,sp,sp
	.text
	.global and
and:
	and ip,ip,ip
	and r59,r59,r59
	and r28,r28,r28
	and r27,r27,r27
	and sp,sp,sp
	and r52,ip,r46
	and r44,r40,r44
	and r24,r58,r31
	.text
	.global orr16
orr16:
	orr ip,ip,ip
	orr r3,r3,r3
	orr r0,r0,r0
	orr fp,fp,fp
	orr sp,sp,sp
	orr lr,r1,sp
	orr r3,lr,lr
	orr r2,r3,r2
	.text
	.global orr
orr:
	orr ip,ip,ip
	orr r59,r59,r59
	orr r28,r28,r28
	orr r27,r27,r27
	orr sp,sp,sp
	orr r52,r5,r59
	orr r15,r32,r43
	orr r56,r29,r44
	.text
	.global eor16
eor16:
	eor ip,ip,ip
	eor r3,r3,r3
	eor r0,r0,r0
	eor fp,fp,fp
	eor sp,sp,sp
	eor ip,r3,r2
	eor r3,sp,r2
	eor fp,sp,r2
	.text
	.global eor
eor:
	eor ip,ip,ip
	eor r59,r59,r59
	eor r28,r28,r28
	eor r27,r27,r27
	eor sp,sp,sp
	eor r17,r56,r29
	eor sp,r41,r27
	eor r11,r10,r43
	.text
	.global asr16
asr16:
	asr ip,ip,ip
	asr r3,r3,r3
	asr r0,r0,r0
	asr fp,fp,fp
	asr sp,sp,sp
	asr r3,r0,r3
	asr r3,r1,lr
	asr r0,fp,sp
	.text
	.global asr
asr:
	asr ip,ip,ip
	asr r59,r59,r59
	asr r28,r28,r28
	asr r27,r27,r27
	asr sp,sp,sp
	asr r34,r9,r25
	asr r51,r17,r33
	asr ip,r7,r11
	.text
	.global lsr16
lsr16:
	lsr ip,ip,ip
	lsr r3,r3,r3
	lsr r0,r0,r0
	lsr fp,fp,fp
	lsr sp,sp,sp
	lsr sp,r3,fp
	lsr fp,r1,lr
	lsr lr,r2,r2
	.text
	.global lsr
lsr:
	lsr ip,ip,ip
	lsr r59,r59,r59
	lsr r28,r28,r28
	lsr r27,r27,r27
	lsr sp,sp,sp
	lsr r6,r25,r19
	lsr r12,r54,r32
	lsr r13,sp,ip
	.text
	.global lsl16
lsl16:
	lsl ip,ip,ip
	lsl r3,r3,r3
	lsl r0,r0,r0
	lsl fp,fp,fp
	lsl sp,sp,sp
	lsl ip,ip,ip
	lsl lr,r1,ip
	lsl lr,sp,r3
	.text
	.global lsl
lsl:
	lsl ip,ip,ip
	lsl r59,r59,r59
	lsl r28,r28,r28
	lsl r27,r27,r27
	lsl sp,sp,sp
	lsl r36,r43,r15
	lsl r34,r39,r37
	lsl r23,r33,r29
	.text
	.global addi16
addi16:
	add ip,ip,0
	add r3,r3,7
	add r0,r0,4
	add fp,fp,3
	add sp,sp,1
	add r3,r1,1
	add r1,fp,3
	add r0,fp,7
	.text
	.global addi
addi:
	add ip,ip,0
	add r59,r59,1023
	add r28,r28,047
	add r27,r27,1023
	add sp,sp,1
	add r49,r28,165
	add r31,r2,623
	add r16,r9,945
	.text
	.global subi16
subi16:
	sub ip,ip,0
	sub r3,r3,7
	sub r0,r0,4
	sub fp,fp,3
	sub sp,sp,1
	sub ip,r3,2
	sub lr,r3,4
	sub ip,r2,1
	.text
	.global subi
subi:
	sub ip,ip,0
	sub r59,r59,-2047
	sub r28,r28,1023
	sub r27,r27,1022
	sub sp,sp,1
	sub r51,r6,836
	sub r47,r40,772
	sub r55,r4,488
	.text
	.global lsri16
lsri16:
	lsr ip,ip,0
	lsr r3,r3,31
	lsr r0,r0,16
	lsr fp,fp,15
	lsr sp,sp,1
	lsr r0,r3,6
	lsr r1,r2,8
	lsr fp,lr,14
	.text
	.global lsri32
lsri32:
	lsr ip,ip,0
	lsr r59,r59,31
	lsr r28,r28,16
	lsr r27,r27,15
	lsr sp,sp,1
	lsr r30,r48,19
	lsr r43,r7,23
	lsr r28,r2,28
	.text
	.global lsli16
lsli16:
	lsl ip,ip,0
	lsl r3,r3,31
	lsl r0,r0,16
	lsl fp,fp,15
	lsl sp,sp,1
	lsl r2,r3,11
	lsl lr,r2,6
	lsl r0,r2,16
	.text
	.global lsli32
lsli32:
	lsl ip,ip,0
	lsl r59,r59,31
	lsl r28,r28,16
	lsl r27,r27,15
	lsl sp,sp,1
	lsl r56,r51,19
	lsl r17,r39,19
	lsl r2,r12,12
	.text
	.global asri16
asri16:
	asr ip,ip,0
	asr r3,r3,31
	asr r0,r0,16
	asr fp,fp,15
	asr sp,sp,1
	asr lr,ip,21
	asr r3,r3,22
	asr r3,r3,9
	.text
	.global asri32
asri32:
	asr ip,ip,0
	asr r59,r59,31
	asr r28,r28,16
	asr r27,r27,15
	asr sp,sp,1
	asr r52,r46,17
	asr r23,r56,22
	asr r21,r46,28
	.text
	.global mov8
mov8:
	mov ip,0
	mov r3,255
	mov r0,128
	mov fp,127
	mov sp,1
	mov lr,91
	mov r0,77
	mov fp,10
	.text
	.global mov16
mov16:
	mov ip,0
	mov r59,65535
	mov r28,32768
	mov r27,32767
	mov sp,1
	mov r53,61169
	mov r18,52207
	mov r16,36386
	.text
	.global faddf16
faddf16:
	fadd ip,ip,ip
	fadd r3,r3,r3
	fadd r0,r0,r0
	fadd fp,fp,fp
	fadd sp,sp,sp
	fadd sp,ip,r2
	fadd sp,r2,r2
	fadd sp,lr,fp
	.text
	.global faddf32
faddf32:
	fadd ip,ip,ip
	fadd r59,r59,r59
	fadd r28,r28,r28
	fadd r27,r27,r27
	fadd sp,sp,sp
	fadd r13,r29,r39
	fadd r32,r40,r3
	fadd r40,r29,lr
	.text
	.global fsubf16
fsubf16:
	fsub ip,ip,ip
	fsub r3,r3,r3
	fsub r0,r0,r0
	fsub fp,fp,fp
	fsub sp,sp,sp
	fsub r2,lr,sp
	fsub r3,r1,ip
	fsub r3,ip,r2
	.text
	.global fsubf32
fsubf32:
	fsub ip,ip,ip
	fsub r59,r59,r59
	fsub r28,r28,r28
	fsub r27,r27,r27
	fsub sp,sp,sp
	fsub r1,r56,r11
	fsub r3,r22,r15
	fsub r6,r48,r45
	.text
	.global fmulf16
fmulf16:
	fmul ip,ip,ip
	fmul r3,r3,r3
	fmul r0,r0,r0
	fmul fp,fp,fp
	fmul sp,sp,sp
	fmul r3,ip,fp
	fmul lr,r1,r2
	fmul sp,lr,lr
	.text
	.global fmulf32
fmulf32:
	fmul ip,ip,ip
	fmul r59,r59,r59
	fmul r28,r28,r28
	fmul r27,r27,r27
	fmul sp,sp,sp
	fmul r58,r23,r51
	fmul r22,r2,r47
	fmul r46,r14,r10
	.text
	.global fmaddf16
fmaddf16:
	fmadd ip,ip,ip
	fmadd r3,r3,r3
	fmadd r0,r0,r0
	fmadd fp,fp,fp
	fmadd sp,sp,sp
	fmadd sp,r1,r3
	fmadd r3,r3,r0
	fmadd r2,ip,ip
	.text
	.global fmaddf32
fmaddf32:
	fmadd ip,ip,ip
	fmadd r59,r59,r59
	fmadd r28,r28,r28
	fmadd r27,r27,r27
	fmadd sp,sp,sp
	fmadd r28,r54,r32
	fmadd r12,r2,fp
	fmadd fp,r40,r22
	.text
	.global fmsubf16
fmsubf16:
	fmsub ip,ip,ip
	fmsub r3,r3,r3
	fmsub r0,r0,r0
	fmsub fp,fp,fp
	fmsub sp,sp,sp
	fmsub sp,fp,r1
	fmsub r1,fp,sp
	fmsub r0,r3,r0
	.text
	.global fmsubf32
fmsubf32:
	fmsub ip,ip,ip
	fmsub r59,r59,r59
	fmsub r28,r28,r28
	fmsub r27,r27,r27
	fmsub sp,sp,sp
	fmsub r42,r20,r9
	fmsub r22,r24,r42
	fmsub r15,r22,r19

;; add some negative displacement ld/store
	ldr	r1,[r2,-12]
	strh	r22,[r30,-2047]
	ldrd	r12,[r14,2047]

;; add bitr
        bitr    r1,r0
        bitr    r31,r15
