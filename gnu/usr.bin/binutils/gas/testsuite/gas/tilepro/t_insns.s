target:

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	nop

	{ mulllsa_uu r5, r6, r7 ; bbnst r15, target }
	{ mulhha_ss r5, r6, r7 ; blezt r15, target }
	{ mulhla_us r5, r6, r7 ; bbnst r15, target }
	{ mullla_uu r5, r6, r7 ; bgezt r15, target }
	{ addlis r5, r6, 0x1234 ; bzt r15, target }
	{ mulhh_uu r5, r6, r7 ; bbnst r15, target }
	{ mulhha_uu r5, r6, r7 ; bgzt r15, target }
	{ mulhl_uu r5, r6, r7 ; blezt r15, target }
	{ mulhla_us r5, r6, r7 ; blzt r15, target }
	{ mulll_uu r5, r6, r7 ; bbnst r15, target }
	{ mullla_uu r5, r6, r7 ; bgzt r15, target }
	{ addlis r5, r6, 0x1234 ; bz r15, target }
	{ crc32_32 r5, r6, r7 ; blzt r15, target }
	{ mulhh_ss r5, r6, r7 ; blzt r15, target }
	{ mulhha_ss r5, r6, r7 ; bzt r15, target }
	{ mulhl_su r5, r6, r7 ; bbst r15, target }
	{ mulhla_ss r5, r6, r7 ; bbs r15, target }
	{ mulhlsa_uu r5, r6, r7 ; bz r15, target }
	{ mulll_uu r5, r6, r7 ; blzt r15, target }
	{ packbs_u r5, r6, r7 ; bgez r15, target }
	{ addbs_u r5, r6, r7 ; bbns r15, target }
	{ auli r5, r6, 0x1234 ; bzt r15, target }
	{ maxib_u r5, r6, 5 ; bgezt r15, target }
	{ movelis r5, 0x1234 ; blez r15, target }
	{ mulhha_uu r5, r6, r7 ; bz r15, target }
	{ mulhl_uu r5, r6, r7 ; bzt r15, target }
	{ mullla_ss r5, r6, r7 ; bz r15, target }
	{ sadab_u r5, r6, r7 ; bgzt r15, target }
	{ slte_u r5, r6, r7 ; bbnst r15, target }
	{ sltib_u r5, r6, 5 ; bbnst r15, target }
	{ addhs r5, r6, r7 ; blezt r15, target }
	{ crc32_8 r5, r6, r7 ; blz r15, target }
	{ maxb_u r5, r6, r7 ; blzt r15, target }
	{ minib_u r5, r6, 5 ; blez r15, target }
	{ mulhl_su r5, r6, r7 ; bz r15, target }
	{ packhs r5, r6, r7 ; bnzt r15, target }
	{ sadah_u r5, r6, r7 ; bzt r15, target }
	{ sltb_u r5, r6, r7 ; bgez r15, target }
	{ slteh r5, r6, r7 ; bbnst r15, target }
	{ sltib_u r5, r6, 5 ; bgez r15, target }
	{ addb r5, r6, r7 ; bbnst r15, target }
	{ adds r5, r6, r7 ; bbnst r15, target }
	{ inthb r5, r6, r7 ; bgez r15, target }
	{ intlh r5, r6, r7 ; bbst r15, target }
	{ maxih r5, r6, 5 ; bgezt r15, target }
	{ mnzb r5, r6, r7 ; blezt r15, target }
	{ packhs r5, r6, r7 ; blz r15, target }
	{ sadb_u r5, r6, r7 ; bnz r15, target }
	{ seqih r5, r6, 5 ; bgezt r15, target }
	{ shrib r5, r6, 5 ; bbnst r15, target }
	{ sltb_u r5, r6, r7 ; bzt r15, target }
	{ slteh r5, r6, r7 ; bgzt r15, target }
	{ sltib r5, r6, 5 ; bbnst r15, target }
	{ sneh r5, r6, r7 ; bgezt r15, target }
	{ subh r5, r6, r7 ; blezt r15, target }
	{ tblidxb3 r5, r6 ; bbnst r15, target }
	{ addhs r5, r6, r7 ; bbs r15, target }
	{ addih r5, r6, 5 ; blzt r15, target }
	{ avgh r5, r6, r7 ; bgez r15, target }
	{ intlh r5, r6, r7 ; bbs r15, target }
	{ maxih r5, r6, 5 ; bnzt r15, target }
	{ mnzb r5, r6, r7 ; bbns r15, target }
	{ mvnz r5, r6, r7 ; bgez r15, target }
	{ s1a r5, r6, r7 ; bbnst r15, target }
	{ sadh r5, r6, r7 ; blzt r15, target }
	{ seqi r5, r6, 5 ; bbnst r15, target }
	{ shlb r5, r6, r7 ; bbns r15, target }
	{ shlib r5, r6, 5 ; bgzt r15, target }
	{ shrb r5, r6, r7 ; bnzt r15, target }
	{ shrih r5, r6, 5 ; bgez r15, target }
	{ sltb_u r5, r6, r7 ; bz r15, target }
	{ slth r5, r6, r7 ; bbst r15, target }
	{ sltib r5, r6, 5 ; blzt r15, target }
	{ sneb r5, r6, r7 ; bnzt r15, target }
	{ srah r5, r6, r7 ; bgez r15, target }
	{ sraih r5, r6, 5 ; blzt r15, target }
	{ subhs r5, r6, r7 ; bgz r15, target }
	{ tblidxb1 r5, r6 ; bgez r15, target }
	{ xor r5, r6, r7 ; bgezt r15, target }
	{ addh r5, r6, r7 ; bnz r15, target }
	{ addli r5, r6, 0x1234 ; jal target }
	{ avgh r5, r6, r7 ; bbs r15, target }
	{ minh r5, r6, r7 ; bbs r15, target }
	{ mnzb r5, r6, r7 ; bnz r15, target }
	{ mvnz r5, r6, r7 ; bnz r15, target }
	{ mzh r5, r6, r7 ; bbst r15, target }
	{ rl r5, r6, r7 ; bgezt r15, target }
	{ s3a r5, r6, r7 ; bbst r15, target }
	{ seqb r5, r6, r7 ; bgz r15, target }
	{ seqib r5, r6, 5 ; bzt r15, target }
	{ shlh r5, r6, r7 ; blz r15, target }
	{ shr r5, r6, r7 ; bbns r15, target }
	{ shri r5, r6, 5 ; bgzt r15, target }
	{ slt r5, r6, r7 ; bnzt r15, target }
	{ slti r5, r6, 5 ; bbst r15, target }
	{ sne r5, r6, r7 ; bgzt r15, target }
	{ sra r5, r6, r7 ; bnzt r15, target }
	{ sraib r5, r6, 5 ; blz r15, target }
	{ subh r5, r6, r7 ; bbs r15, target }
	{ tblidxb1 r5, r6 ; bzt r15, target }
	{ xori r5, r6, 5 ; bgez r15, target }
	{ adds r5, r6, r7 ; bz r15, target }
	{ infol 0x1234 ; blezt r15, target }
	{ mulhl_uu r5, r6, r7 ; jal target }
	{ mzb r5, r6, r7 ; bgz r15, target }
	{ or r5, r6, r7 ; bnzt r15, target }
	{ rli r5, r6, 5 ; blez r15, target }
	{ seq r5, r6, r7 ; bgz r15, target }
	{ shli r5, r6, 5 ; bbs r15, target }
	{ shrih r5, r6, 5 ; bz r15, target }
	{ sne r5, r6, r7 ; bzt r15, target }
	{ sub r5, r6, r7 ; bnz r15, target }
	{ addbs_u r5, r6, r7 ; jal target }
	{ infol 0x1234 ; blez r15, target }
	{ mullla_uu r5, r6, r7 ; j target }
	{ pcnt r5, r6 ; bbnst r15, target }
	{ shl r5, r6, r7 ; bz r15, target }
	{ bitx r5, r6 ; bbst r15, target }
	{ infol 0x1234 ; blz r15, target }
	{ movei r5, 5 ; blzt r15, target }
	{ pcnt r5, r6 ; bbns r15, target }
	{ bitx r5, r6 ; blz r15, target }
	{ inthb r5, r6, r7 ; jal target }
	{ sadab_u r5, r6, r7 ; j target }
	{ clz r5, r6 ; bbs r15, target }
	{ move r5, r6 ; bz r15, target }
	{ shrh r5, r6, r7 ; jal target }
	{ subh r5, r6, r7 ; jal target }
	{ mnz r5, r6, r7 ; jal target }
	{ slti_u r5, r6, 5 ; j target }
	{ info 19 ; bnzt r15, target }
	{ shlib r5, r6, 5 ; j target }
	{ tblidxb0 r5, r6 ; j target }
	{ s1a r5, r6, r7 ; j target }
	{ fnop ; blezt r15, target }
	{ infol 0x1234 ; j target }
	{ clz r5, r6 ; j target }
	{ bbnst r15, target ; addlis r5, r6, 0x1234 }
	{ bbnst r15, target ; inthh r5, r6, r7 }
	{ bbnst r15, target ; mulhh_su r5, r6, r7 }
	{ bbnst r15, target ; mullla_uu r5, r6, r7 }
	{ bbnst r15, target ; s3a r5, r6, r7 }
	{ bbnst r15, target ; shrb r5, r6, r7 }
	{ bbnst r15, target ; sltib_u r5, r6, 5 }
	{ bbnst r15, target ; tblidxb2 r5, r6 }
	{ bgezt r15, target ; avgb_u r5, r6, r7 }
	{ bgezt r15, target ; minb_u r5, r6, r7 }
	{ bgezt r15, target ; mulhl_su r5, r6, r7 }
	{ bgezt r15, target ; nop }
	{ bgezt r15, target ; seq r5, r6, r7 }
	{ bgezt r15, target ; sltb r5, r6, r7 }
	{ bgezt r15, target ; srab r5, r6, r7 }
	{ blezt r15, target ; addh r5, r6, r7 }
	{ blezt r15, target ; ctz r5, r6 }
	{ blezt r15, target ; mnzh r5, r6, r7 }
	{ blezt r15, target ; mulhlsa_uu r5, r6, r7 }
	{ blezt r15, target ; packlb r5, r6, r7 }
	{ blezt r15, target ; shlb r5, r6, r7 }
	{ blezt r15, target ; slteh_u r5, r6, r7 }
	{ blezt r15, target ; subbs_u r5, r6, r7 }
	{ bbns r15, target ; addlis r5, r6, 0x1234 }
	{ bbns r15, target ; inthh r5, r6, r7 }
	{ bbns r15, target ; mulhh_su r5, r6, r7 }
	{ bbns r15, target ; mullla_uu r5, r6, r7 }
	{ bbns r15, target ; s3a r5, r6, r7 }
	{ bbns r15, target ; shrb r5, r6, r7 }
	{ bbns r15, target ; sltib_u r5, r6, 5 }
	{ bbns r15, target ; tblidxb2 r5, r6 }
	{ bbst r15, target ; avgb_u r5, r6, r7 }
	{ bbst r15, target ; minb_u r5, r6, r7 }
	{ bbst r15, target ; mulhl_su r5, r6, r7 }
	{ bbst r15, target ; nop }
	{ bbst r15, target ; seq r5, r6, r7 }
	{ bbst r15, target ; sltb r5, r6, r7 }
	{ bbst r15, target ; srab r5, r6, r7 }
	{ bgez r15, target ; addh r5, r6, r7 }
	{ bgez r15, target ; ctz r5, r6 }
	{ bgez r15, target ; mnzh r5, r6, r7 }
	{ bgez r15, target ; mulhlsa_uu r5, r6, r7 }
	{ bgez r15, target ; packlb r5, r6, r7 }
	{ bgez r15, target ; shlb r5, r6, r7 }
	{ bgez r15, target ; slteh_u r5, r6, r7 }
	{ bgez r15, target ; subbs_u r5, r6, r7 }
	{ bgzt r15, target ; adds r5, r6, r7 }
	{ bgzt r15, target ; intlb r5, r6, r7 }
	{ bgzt r15, target ; mulhh_uu r5, r6, r7 }
	{ bgzt r15, target ; mulllsa_uu r5, r6, r7 }
	{ bgzt r15, target ; sadab_u r5, r6, r7 }
	{ bgzt r15, target ; shrh r5, r6, r7 }
	{ bgzt r15, target ; sltih r5, r6, 5 }
	{ bgzt r15, target ; tblidxb3 r5, r6 }
	{ blez r15, target ; avgh r5, r6, r7 }
	{ blez r15, target ; minh r5, r6, r7 }
	{ blez r15, target ; mulhl_us r5, r6, r7 }
	{ blez r15, target ; nor r5, r6, r7 }
	{ blez r15, target ; seqb r5, r6, r7 }
	{ blez r15, target ; sltb_u r5, r6, r7 }
	{ blez r15, target ; srah r5, r6, r7 }
	{ blzt r15, target ; addhs r5, r6, r7 }
	{ blzt r15, target ; dword_align r5, r6, r7 }
	{ blzt r15, target ; move r5, r6 }
	{ blzt r15, target ; mulll_ss r5, r6, r7 }
	{ blzt r15, target ; pcnt r5, r6 }
	{ blzt r15, target ; shlh r5, r6, r7 }
	{ blzt r15, target ; slth r5, r6, r7 }
	{ blzt r15, target ; subh r5, r6, r7 }
	{ bnzt r15, target ; adiffb_u r5, r6, r7 }
	{ bnzt r15, target ; intlh r5, r6, r7 }
	{ bnzt r15, target ; mulhha_ss r5, r6, r7 }
	{ bnzt r15, target ; mvnz r5, r6, r7 }
	{ bnzt r15, target ; sadah r5, r6, r7 }
	{ bnzt r15, target ; shri r5, r6, 5 }
	{ bnzt r15, target ; sltih_u r5, r6, 5 }
	{ bnzt r15, target ; xor r5, r6, r7 }
	{ bbs r15, target ; avgh r5, r6, r7 }
	{ bbs r15, target ; minh r5, r6, r7 }
	{ bbs r15, target ; mulhl_us r5, r6, r7 }
	{ bbs r15, target ; nor r5, r6, r7 }
	{ bbs r15, target ; seqb r5, r6, r7 }
	{ bbs r15, target ; sltb_u r5, r6, r7 }
	{ bbs r15, target ; srah r5, r6, r7 }
	{ bgz r15, target ; addhs r5, r6, r7 }
	{ bgz r15, target ; dword_align r5, r6, r7 }
	{ bgz r15, target ; move r5, r6 }
	{ bgz r15, target ; mulll_ss r5, r6, r7 }
	{ bgz r15, target ; pcnt r5, r6 }
	{ bgz r15, target ; shlh r5, r6, r7 }
	{ bgz r15, target ; slth r5, r6, r7 }
	{ bgz r15, target ; subh r5, r6, r7 }
	{ blz r15, target ; adiffb_u r5, r6, r7 }
	{ blz r15, target ; intlh r5, r6, r7 }
	{ blz r15, target ; mulhha_ss r5, r6, r7 }
	{ blz r15, target ; mvnz r5, r6, r7 }
	{ blz r15, target ; sadah r5, r6, r7 }
	{ blz r15, target ; shri r5, r6, 5 }
	{ blz r15, target ; sltih_u r5, r6, 5 }
	{ blz r15, target ; xor r5, r6, r7 }
	{ bnz r15, target ; bitx r5, r6 }
	{ bnz r15, target ; minib_u r5, r6, 5 }
	{ bnz r15, target ; mulhl_uu r5, r6, r7 }
	{ bnz r15, target ; or r5, r6, r7 }
	{ bnz r15, target ; seqh r5, r6, r7 }
	{ bnz r15, target ; slte r5, r6, r7 }
	{ bnz r15, target ; srai r5, r6, 5 }
	{ bzt r15, target ; addi r5, r6, 5 }
	{ bzt r15, target ; fnop }
	{ bzt r15, target ; movei r5, 5 }
	{ bzt r15, target ; mulll_su r5, r6, r7 }
	{ bzt r15, target ; rl r5, r6, r7 }
	{ bzt r15, target ; shli r5, r6, 5 }
	{ bzt r15, target ; slth_u r5, r6, r7 }
	{ bzt r15, target ; subhs r5, r6, r7 }
	{ bz r15, target ; addli r5, r6, 0x1234 }
	{ bz r15, target ; inthb r5, r6, r7 }
	{ bz r15, target ; mulhh_ss r5, r6, r7 }
	{ bz r15, target ; mullla_su r5, r6, r7 }
	{ bz r15, target ; s2a r5, r6, r7 }
	{ bz r15, target ; shr r5, r6, r7 }
	{ bz r15, target ; sltib r5, r6, 5 }
	{ bz r15, target ; tblidxb1 r5, r6 }
	{ jal target ; addb r5, r6, r7 }
	{ jal target ; crc32_32 r5, r6, r7 }
	{ jal target ; mnz r5, r6, r7 }
	{ jal target ; mulhla_us r5, r6, r7 }
	{ jal target ; packhb r5, r6, r7 }
	{ jal target ; seqih r5, r6, 5 }
	{ jal target ; slteb_u r5, r6, r7 }
	{ jal target ; sub r5, r6, r7 }
	{ j target ; addih r5, r6, 5 }
	{ j target ; infol 0x1234 }
	{ j target ; movelis r5, 0x1234 }
	{ j target ; mullla_ss r5, r6, r7 }
	{ j target ; s1a r5, r6, r7 }
	{ j target ; shlih r5, r6, 5 }
	{ j target ; slti_u r5, r6, 5 }
	{ j target ; tblidxb0 r5, r6 }
	and r5, r6, r7
	info 19
	lnk r5
	movei r5, 5
	mulll_ss r5, r6, r7
	packlb r5, r6, r7
	seqi r5, r6, 5
	sltb_u r5, r6, r7
	srah r5, r6, r7
	tns r5, r6
	{ add r15, r16, r17 ; addi r5, r6, 5 ; lh r25, r26 }
	{ add r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
	{ add r15, r16, r17 ; bitx r5, r6 ; lh r25, r26 }
	{ add r15, r16, r17 ; clz r5, r6 ; lh r25, r26 }
	{ add r15, r16, r17 ; dword_align r5, r6, r7 }
	{ add r15, r16, r17 ; info 19 }
	{ add r15, r16, r17 ; lb r25, r26 ; mulhh_uu r5, r6, r7 }
	{ add r15, r16, r17 ; lb r25, r26 ; s3a r5, r6, r7 }
	{ add r15, r16, r17 ; lb r25, r26 ; tblidxb3 r5, r6 }
	{ add r15, r16, r17 ; lb_u r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ add r15, r16, r17 ; lb_u r25, r26 ; shl r5, r6, r7 }
	{ add r15, r16, r17 ; lh r25, r26 ; add r5, r6, r7 }
	{ add r15, r16, r17 ; lh r25, r26 ; mullla_ss r5, r6, r7 }
	{ add r15, r16, r17 ; lh r25, r26 ; shri r5, r6, 5 }
	{ add r15, r16, r17 ; lh_u r25, r26 ; andi r5, r6, 5 }
	{ add r15, r16, r17 ; lh_u r25, r26 ; mvz r5, r6, r7 }
	{ add r15, r16, r17 ; lh_u r25, r26 ; slte r5, r6, r7 }
	{ add r15, r16, r17 ; lw r25, r26 ; clz r5, r6 }
	{ add r15, r16, r17 ; lw r25, r26 ; nor r5, r6, r7 }
	{ add r15, r16, r17 ; lw r25, r26 ; slti_u r5, r6, 5 }
	{ add r15, r16, r17 ; mnz r5, r6, r7 ; lb r25, r26 }
	{ add r15, r16, r17 ; move r5, r6 ; sw r25, r26 }
	{ add r15, r16, r17 ; mulhh_ss r5, r6, r7 ; sb r25, r26 }
	{ add r15, r16, r17 ; mulhha_ss r5, r6, r7 ; prefetch r25 }
	{ add r15, r16, r17 ; mulhl_uu r5, r6, r7 }
	{ add r15, r16, r17 ; mulll_ss r5, r6, r7 ; prefetch r25 }
	{ add r15, r16, r17 ; mullla_ss r5, r6, r7 ; lw r25, r26 }
	{ add r15, r16, r17 ; mvnz r5, r6, r7 ; lh r25, r26 }
	{ add r15, r16, r17 ; mz r5, r6, r7 ; lh r25, r26 }
	{ add r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
	{ add r15, r16, r17 ; ori r5, r6, 5 ; lb r25, r26 }
	{ add r15, r16, r17 ; pcnt r5, r6 ; sb r25, r26 }
	{ add r15, r16, r17 ; prefetch r25 ; mulhha_uu r5, r6, r7 }
	{ add r15, r16, r17 ; prefetch r25 ; seqi r5, r6, 5 }
	{ add r15, r16, r17 ; prefetch r25 }
	{ add r15, r16, r17 ; rli r5, r6, 5 }
	{ add r15, r16, r17 ; s2a r5, r6, r7 }
	{ add r15, r16, r17 ; sb r25, r26 ; andi r5, r6, 5 }
	{ add r15, r16, r17 ; sb r25, r26 ; mvz r5, r6, r7 }
	{ add r15, r16, r17 ; sb r25, r26 ; slte r5, r6, r7 }
	{ add r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
	{ add r15, r16, r17 ; sh r25, r26 ; and r5, r6, r7 }
	{ add r15, r16, r17 ; sh r25, r26 ; mvnz r5, r6, r7 }
	{ add r15, r16, r17 ; sh r25, r26 ; slt_u r5, r6, r7 }
	{ add r15, r16, r17 ; shl r5, r6, r7 ; prefetch r25 }
	{ add r15, r16, r17 ; shr r5, r6, r7 ; lb_u r25, r26 }
	{ add r15, r16, r17 ; shri r5, r6, 5 }
	{ add r15, r16, r17 ; slt_u r5, r6, r7 ; sh r25, r26 }
	{ add r15, r16, r17 ; slte_u r5, r6, r7 ; prefetch r25 }
	{ add r15, r16, r17 ; slti r5, r6, 5 }
	{ add r15, r16, r17 ; sne r5, r6, r7 ; prefetch r25 }
	{ add r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
	{ add r15, r16, r17 ; sub r5, r6, r7 }
	{ add r15, r16, r17 ; sw r25, r26 ; mulhh_uu r5, r6, r7 }
	{ add r15, r16, r17 ; sw r25, r26 ; s3a r5, r6, r7 }
	{ add r15, r16, r17 ; sw r25, r26 ; tblidxb3 r5, r6 }
	{ add r15, r16, r17 ; tblidxb1 r5, r6 ; sh r25, r26 }
	{ add r15, r16, r17 ; tblidxb3 r5, r6 ; sh r25, r26 }
	{ add r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
	{ add r5, r6, r7 ; addli r15, r16, 0x1234 }
	{ add r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
	{ add r5, r6, r7 ; ill ; lh r25, r26 }
	{ add r5, r6, r7 ; inthh r15, r16, r17 }
	{ add r5, r6, r7 ; lb r25, r26 ; mz r15, r16, r17 }
	{ add r5, r6, r7 ; lb r25, r26 ; slti r15, r16, 5 }
	{ add r5, r6, r7 ; lb_u r25, r26 ; nop }
	{ add r5, r6, r7 ; lb_u r25, r26 ; slti_u r15, r16, 5 }
	{ add r5, r6, r7 ; lh r25, r26 ; mz r15, r16, r17 }
	{ add r5, r6, r7 ; lh r25, r26 ; slti r15, r16, 5 }
	{ add r5, r6, r7 ; lh_u r25, r26 ; nop }
	{ add r5, r6, r7 ; lh_u r25, r26 ; slti_u r15, r16, 5 }
	{ add r5, r6, r7 ; lw r25, r26 ; movei r15, 5 }
	{ add r5, r6, r7 ; lw r25, r26 ; slte_u r15, r16, r17 }
	{ add r5, r6, r7 ; minib_u r15, r16, 5 }
	{ add r5, r6, r7 ; move r15, r16 ; prefetch r25 }
	{ add r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
	{ add r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
	{ add r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
	{ add r5, r6, r7 ; prefetch r25 ; ill }
	{ add r5, r6, r7 ; prefetch r25 ; shri r15, r16, 5 }
	{ add r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
	{ add r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
	{ add r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
	{ add r5, r6, r7 ; sb r25, r26 ; rl r15, r16, r17 }
	{ add r5, r6, r7 ; sb r25, r26 ; sub r15, r16, r17 }
	{ add r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
	{ add r5, r6, r7 ; sh r25, r26 ; nop }
	{ add r5, r6, r7 ; sh r25, r26 ; slti_u r15, r16, 5 }
	{ add r5, r6, r7 ; shli r15, r16, 5 ; lb r25, r26 }
	{ add r5, r6, r7 ; shr r15, r16, r17 ; sw r25, r26 }
	{ add r5, r6, r7 ; slt r15, r16, r17 ; lw r25, r26 }
	{ add r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
	{ add r5, r6, r7 ; slteh r15, r16, r17 }
	{ add r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
	{ add r5, r6, r7 ; sra r15, r16, r17 ; lb r25, r26 }
	{ add r5, r6, r7 ; srai r15, r16, 5 ; sw r25, r26 }
	{ add r5, r6, r7 ; sw r25, r26 ; add r15, r16, r17 }
	{ add r5, r6, r7 ; sw r25, r26 ; seq r15, r16, r17 }
	{ add r5, r6, r7 ; wh64 r15 }
	{ addb r15, r16, r17 ; addli r5, r6, 0x1234 }
	{ addb r15, r16, r17 ; inthb r5, r6, r7 }
	{ addb r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ addb r15, r16, r17 ; mullla_su r5, r6, r7 }
	{ addb r15, r16, r17 ; s2a r5, r6, r7 }
	{ addb r15, r16, r17 ; shr r5, r6, r7 }
	{ addb r15, r16, r17 ; sltib r5, r6, 5 }
	{ addb r15, r16, r17 ; tblidxb1 r5, r6 }
	{ addb r5, r6, r7 ; finv r15 }
	{ addb r5, r6, r7 ; lbadd_u r15, r16, 5 }
	{ addb r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
	{ addb r5, r6, r7 ; prefetch r15 }
	{ addb r5, r6, r7 ; shli r15, r16, 5 }
	{ addb r5, r6, r7 ; slth_u r15, r16, r17 }
	{ addb r5, r6, r7 ; subhs r15, r16, r17 }
	{ addbs_u r15, r16, r17 ; adiffh r5, r6, r7 }
	{ addbs_u r15, r16, r17 ; maxb_u r5, r6, r7 }
	{ addbs_u r15, r16, r17 ; mulhha_su r5, r6, r7 }
	{ addbs_u r15, r16, r17 ; mvz r5, r6, r7 }
	{ addbs_u r15, r16, r17 ; sadah_u r5, r6, r7 }
	{ addbs_u r15, r16, r17 ; shrib r5, r6, 5 }
	{ addbs_u r15, r16, r17 ; sne r5, r6, r7 }
	{ addbs_u r15, r16, r17 ; xori r5, r6, 5 }
	{ addbs_u r5, r6, r7 ; ill }
	{ addbs_u r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ addbs_u r5, r6, r7 ; move r15, r16 }
	{ addbs_u r5, r6, r7 ; s1a r15, r16, r17 }
	{ addbs_u r5, r6, r7 ; shrb r15, r16, r17 }
	{ addbs_u r5, r6, r7 ; sltib_u r15, r16, 5 }
	{ addbs_u r5, r6, r7 ; tns r15, r16 }
	{ addh r15, r16, r17 ; avgb_u r5, r6, r7 }
	{ addh r15, r16, r17 ; minb_u r5, r6, r7 }
	{ addh r15, r16, r17 ; mulhl_su r5, r6, r7 }
	{ addh r15, r16, r17 ; nop }
	{ addh r15, r16, r17 ; seq r5, r6, r7 }
	{ addh r15, r16, r17 ; sltb r5, r6, r7 }
	{ addh r15, r16, r17 ; srab r5, r6, r7 }
	{ addh r5, r6, r7 ; addh r15, r16, r17 }
	{ addh r5, r6, r7 ; inthh r15, r16, r17 }
	{ addh r5, r6, r7 ; lwadd r15, r16, 5 }
	{ addh r5, r6, r7 ; mtspr 0x5, r16 }
	{ addh r5, r6, r7 ; sbadd r15, r16, 5 }
	{ addh r5, r6, r7 ; shrih r15, r16, 5 }
	{ addh r5, r6, r7 ; sneb r15, r16, r17 }
	{ addhs r15, r16, r17 ; add r5, r6, r7 }
	{ addhs r15, r16, r17 ; clz r5, r6 }
	{ addhs r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
	{ addhs r15, r16, r17 ; mulhla_su r5, r6, r7 }
	{ addhs r15, r16, r17 ; packbs_u r5, r6, r7 }
	{ addhs r15, r16, r17 ; seqib r5, r6, 5 }
	{ addhs r15, r16, r17 ; slteb r5, r6, r7 }
	{ addhs r15, r16, r17 ; sraih r5, r6, 5 }
	{ addhs r5, r6, r7 ; addih r15, r16, 5 }
	{ addhs r5, r6, r7 ; iret }
	{ addhs r5, r6, r7 ; maxib_u r15, r16, 5 }
	{ addhs r5, r6, r7 ; nop }
	{ addhs r5, r6, r7 ; seqi r15, r16, 5 }
	{ addhs r5, r6, r7 ; sltb_u r15, r16, r17 }
	{ addhs r5, r6, r7 ; srah r15, r16, r17 }
	{ addi r15, r16, 5 ; add r5, r6, r7 ; lw r25, r26 }
	{ addi r15, r16, 5 ; addib r5, r6, 5 }
	{ addi r15, r16, 5 ; andi r5, r6, 5 ; lh_u r25, r26 }
	{ addi r15, r16, 5 ; bytex r5, r6 ; lb r25, r26 }
	{ addi r15, r16, 5 ; crc32_32 r5, r6, r7 }
	{ addi r15, r16, 5 ; fnop ; sh r25, r26 }
	{ addi r15, r16, 5 ; lb r25, r26 ; and r5, r6, r7 }
	{ addi r15, r16, 5 ; lb r25, r26 ; mvnz r5, r6, r7 }
	{ addi r15, r16, 5 ; lb r25, r26 ; slt_u r5, r6, r7 }
	{ addi r15, r16, 5 ; lb_u r25, r26 ; bytex r5, r6 }
	{ addi r15, r16, 5 ; lb_u r25, r26 ; nop }
	{ addi r15, r16, 5 ; lb_u r25, r26 ; slti r5, r6, 5 }
	{ addi r15, r16, 5 ; lh r25, r26 ; fnop }
	{ addi r15, r16, 5 ; lh r25, r26 ; ori r5, r6, 5 }
	{ addi r15, r16, 5 ; lh r25, r26 ; sra r5, r6, r7 }
	{ addi r15, r16, 5 ; lh_u r25, r26 ; move r5, r6 }
	{ addi r15, r16, 5 ; lh_u r25, r26 ; rli r5, r6, 5 }
	{ addi r15, r16, 5 ; lh_u r25, r26 ; tblidxb0 r5, r6 }
	{ addi r15, r16, 5 ; lw r25, r26 ; mulhh_uu r5, r6, r7 }
	{ addi r15, r16, 5 ; lw r25, r26 ; s3a r5, r6, r7 }
	{ addi r15, r16, 5 ; lw r25, r26 ; tblidxb3 r5, r6 }
	{ addi r15, r16, 5 ; mnz r5, r6, r7 ; sw r25, r26 }
	{ addi r15, r16, 5 ; movei r5, 5 ; sb r25, r26 }
	{ addi r15, r16, 5 ; mulhh_uu r5, r6, r7 ; lh_u r25, r26 }
	{ addi r15, r16, 5 ; mulhha_uu r5, r6, r7 ; lh r25, r26 }
	{ addi r15, r16, 5 ; mulhlsa_uu r5, r6, r7 ; lh_u r25, r26 }
	{ addi r15, r16, 5 ; mulll_uu r5, r6, r7 ; lh r25, r26 }
	{ addi r15, r16, 5 ; mullla_uu r5, r6, r7 ; lb_u r25, r26 }
	{ addi r15, r16, 5 ; mvz r5, r6, r7 ; lb r25, r26 }
	{ addi r15, r16, 5 ; mzb r5, r6, r7 }
	{ addi r15, r16, 5 ; nor r5, r6, r7 ; sw r25, r26 }
	{ addi r15, r16, 5 ; ori r5, r6, 5 ; sw r25, r26 }
	{ addi r15, r16, 5 ; prefetch r25 ; bitx r5, r6 }
	{ addi r15, r16, 5 ; prefetch r25 ; mz r5, r6, r7 }
	{ addi r15, r16, 5 ; prefetch r25 ; slte_u r5, r6, r7 }
	{ addi r15, r16, 5 ; rl r5, r6, r7 ; sh r25, r26 }
	{ addi r15, r16, 5 ; s1a r5, r6, r7 ; sh r25, r26 }
	{ addi r15, r16, 5 ; s3a r5, r6, r7 ; sh r25, r26 }
	{ addi r15, r16, 5 ; sb r25, r26 ; move r5, r6 }
	{ addi r15, r16, 5 ; sb r25, r26 ; rli r5, r6, 5 }
	{ addi r15, r16, 5 ; sb r25, r26 ; tblidxb0 r5, r6 }
	{ addi r15, r16, 5 ; seqi r5, r6, 5 ; lh r25, r26 }
	{ addi r15, r16, 5 ; sh r25, r26 ; mnz r5, r6, r7 }
	{ addi r15, r16, 5 ; sh r25, r26 ; rl r5, r6, r7 }
	{ addi r15, r16, 5 ; sh r25, r26 ; sub r5, r6, r7 }
	{ addi r15, r16, 5 ; shli r5, r6, 5 ; lb_u r25, r26 }
	{ addi r15, r16, 5 ; shr r5, r6, r7 }
	{ addi r15, r16, 5 ; slt r5, r6, r7 ; prefetch r25 }
	{ addi r15, r16, 5 ; slte r5, r6, r7 ; lh_u r25, r26 }
	{ addi r15, r16, 5 ; slteh_u r5, r6, r7 }
	{ addi r15, r16, 5 ; slti_u r5, r6, 5 ; sh r25, r26 }
	{ addi r15, r16, 5 ; sra r5, r6, r7 ; lb_u r25, r26 }
	{ addi r15, r16, 5 ; srai r5, r6, 5 }
	{ addi r15, r16, 5 ; sw r25, r26 ; and r5, r6, r7 }
	{ addi r15, r16, 5 ; sw r25, r26 ; mvnz r5, r6, r7 }
	{ addi r15, r16, 5 ; sw r25, r26 ; slt_u r5, r6, r7 }
	{ addi r15, r16, 5 ; tblidxb0 r5, r6 ; prefetch r25 }
	{ addi r15, r16, 5 ; tblidxb2 r5, r6 ; prefetch r25 }
	{ addi r15, r16, 5 ; xor r5, r6, r7 ; prefetch r25 }
	{ addi r5, r6, 5 ; addi r15, r16, 5 ; lb r25, r26 }
	{ addi r5, r6, 5 ; and r15, r16, r17 ; prefetch r25 }
	{ addi r5, r6, 5 ; fnop ; lb_u r25, r26 }
	{ addi r5, r6, 5 ; info 19 ; lb r25, r26 }
	{ addi r5, r6, 5 ; jrp r15 }
	{ addi r5, r6, 5 ; lb r25, r26 ; s2a r15, r16, r17 }
	{ addi r5, r6, 5 ; lb_u r15, r16 }
	{ addi r5, r6, 5 ; lb_u r25, r26 ; s3a r15, r16, r17 }
	{ addi r5, r6, 5 ; lbadd_u r15, r16, 5 }
	{ addi r5, r6, 5 ; lh r25, r26 ; s2a r15, r16, r17 }
	{ addi r5, r6, 5 ; lh_u r15, r16 }
	{ addi r5, r6, 5 ; lh_u r25, r26 ; s3a r15, r16, r17 }
	{ addi r5, r6, 5 ; lhadd_u r15, r16, 5 }
	{ addi r5, r6, 5 ; lw r25, r26 ; s1a r15, r16, r17 }
	{ addi r5, r6, 5 ; lw r25, r26 }
	{ addi r5, r6, 5 ; mnz r15, r16, r17 ; prefetch r25 }
	{ addi r5, r6, 5 ; movei r15, 5 ; lh_u r25, r26 }
	{ addi r5, r6, 5 ; mzb r15, r16, r17 }
	{ addi r5, r6, 5 ; nor r15, r16, r17 ; sw r25, r26 }
	{ addi r5, r6, 5 ; ori r15, r16, 5 ; sw r25, r26 }
	{ addi r5, r6, 5 ; prefetch r25 ; or r15, r16, r17 }
	{ addi r5, r6, 5 ; prefetch r25 ; sra r15, r16, r17 }
	{ addi r5, r6, 5 ; rli r15, r16, 5 ; lw r25, r26 }
	{ addi r5, r6, 5 ; s2a r15, r16, r17 ; lw r25, r26 }
	{ addi r5, r6, 5 ; sb r25, r26 ; andi r15, r16, 5 }
	{ addi r5, r6, 5 ; sb r25, r26 ; shli r15, r16, 5 }
	{ addi r5, r6, 5 ; seq r15, r16, r17 ; lw r25, r26 }
	{ addi r5, r6, 5 ; sh r15, r16 }
	{ addi r5, r6, 5 ; sh r25, r26 ; s3a r15, r16, r17 }
	{ addi r5, r6, 5 ; shl r15, r16, r17 ; lb r25, r26 }
	{ addi r5, r6, 5 ; shli r15, r16, 5 ; sw r25, r26 }
	{ addi r5, r6, 5 ; shri r15, r16, 5 ; lw r25, r26 }
	{ addi r5, r6, 5 ; slt_u r15, r16, r17 ; lh r25, r26 }
	{ addi r5, r6, 5 ; slte_u r15, r16, r17 ; lb r25, r26 }
	{ addi r5, r6, 5 ; slti r15, r16, 5 ; lw r25, r26 }
	{ addi r5, r6, 5 ; sne r15, r16, r17 ; lb r25, r26 }
	{ addi r5, r6, 5 ; sra r15, r16, r17 ; sw r25, r26 }
	{ addi r5, r6, 5 ; sub r15, r16, r17 ; lw r25, r26 }
	{ addi r5, r6, 5 ; sw r25, r26 ; move r15, r16 }
	{ addi r5, r6, 5 ; sw r25, r26 ; slte r15, r16, r17 }
	{ addi r5, r6, 5 ; xor r15, r16, r17 ; sh r25, r26 }
	{ addib r15, r16, 5 ; avgb_u r5, r6, r7 }
	{ addib r15, r16, 5 ; minb_u r5, r6, r7 }
	{ addib r15, r16, 5 ; mulhl_su r5, r6, r7 }
	{ addib r15, r16, 5 ; nop }
	{ addib r15, r16, 5 ; seq r5, r6, r7 }
	{ addib r15, r16, 5 ; sltb r5, r6, r7 }
	{ addib r15, r16, 5 ; srab r5, r6, r7 }
	{ addib r5, r6, 5 ; addh r15, r16, r17 }
	{ addib r5, r6, 5 ; inthh r15, r16, r17 }
	{ addib r5, r6, 5 ; lwadd r15, r16, 5 }
	{ addib r5, r6, 5 ; mtspr 0x5, r16 }
	{ addib r5, r6, 5 ; sbadd r15, r16, 5 }
	{ addib r5, r6, 5 ; shrih r15, r16, 5 }
	{ addib r5, r6, 5 ; sneb r15, r16, r17 }
	{ addih r15, r16, 5 ; add r5, r6, r7 }
	{ addih r15, r16, 5 ; clz r5, r6 }
	{ addih r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
	{ addih r15, r16, 5 ; mulhla_su r5, r6, r7 }
	{ addih r15, r16, 5 ; packbs_u r5, r6, r7 }
	{ addih r15, r16, 5 ; seqib r5, r6, 5 }
	{ addih r15, r16, 5 ; slteb r5, r6, r7 }
	{ addih r15, r16, 5 ; sraih r5, r6, 5 }
	{ addih r5, r6, 5 ; addih r15, r16, 5 }
	{ addih r5, r6, 5 ; iret }
	{ addih r5, r6, 5 ; maxib_u r15, r16, 5 }
	{ addih r5, r6, 5 ; nop }
	{ addih r5, r6, 5 ; seqi r15, r16, 5 }
	{ addih r5, r6, 5 ; sltb_u r15, r16, r17 }
	{ addih r5, r6, 5 ; srah r15, r16, r17 }
	{ addli r15, r16, 0x1234 ; addhs r5, r6, r7 }
	{ addli r15, r16, 0x1234 ; dword_align r5, r6, r7 }
	{ addli r15, r16, 0x1234 ; move r5, r6 }
	{ addli r15, r16, 0x1234 ; mulll_ss r5, r6, r7 }
	{ addli r15, r16, 0x1234 ; pcnt r5, r6 }
	{ addli r15, r16, 0x1234 ; shlh r5, r6, r7 }
	{ addli r15, r16, 0x1234 ; slth r5, r6, r7 }
	{ addli r15, r16, 0x1234 ; subh r5, r6, r7 }
	{ addli r5, r6, 0x1234 ; and r15, r16, r17 }
	{ addli r5, r6, 0x1234 ; jrp r15 }
	{ addli r5, r6, 0x1234 ; minb_u r15, r16, r17 }
	{ addli r5, r6, 0x1234 ; packbs_u r15, r16, r17 }
	{ addli r5, r6, 0x1234 ; shadd r15, r16, 5 }
	{ addli r5, r6, 0x1234 ; slteb_u r15, r16, r17 }
	{ addli r5, r6, 0x1234 ; sub r15, r16, r17 }
	{ addlis r15, r16, 0x1234 ; addli r5, r6, 0x1234 }
	{ addlis r15, r16, 0x1234 ; inthh r5, r6, r7 }
	{ addlis r15, r16, 0x1234 ; mulhh_uu r5, r6, r7 }
	{ addlis r15, r16, 0x1234 ; mulllsa_uu r5, r6, r7 }
	{ addlis r15, r16, 0x1234 ; sadab_u r5, r6, r7 }
	{ addlis r15, r16, 0x1234 ; shrh r5, r6, r7 }
	{ addlis r15, r16, 0x1234 ; sltih r5, r6, 5 }
	{ addlis r15, r16, 0x1234 ; tblidxb3 r5, r6 }
	{ addlis r5, r6, 0x1234 ; icoh r15 }
	{ addlis r5, r6, 0x1234 ; lhadd r15, r16, 5 }
	{ addlis r5, r6, 0x1234 ; mnzh r15, r16, r17 }
	{ addlis r5, r6, 0x1234 ; s1a r15, r16, r17 }
	{ addlis r5, r6, 0x1234 ; shrb r15, r16, r17 }
	{ addlis r5, r6, 0x1234 ; sltib_u r15, r16, 5 }
	{ addlis r5, r6, 0x1234 ; tns r15, r16 }
	{ adds r15, r16, r17 ; avgb_u r5, r6, r7 }
	{ adds r15, r16, r17 ; minb_u r5, r6, r7 }
	{ adds r15, r16, r17 ; mulhl_su r5, r6, r7 }
	{ adds r15, r16, r17 ; nop }
	{ adds r15, r16, r17 ; seq r5, r6, r7 }
	{ adds r15, r16, r17 ; sltb r5, r6, r7 }
	{ adds r15, r16, r17 ; srab r5, r6, r7 }
	{ adds r5, r6, r7 ; addh r15, r16, r17 }
	{ adds r5, r6, r7 ; inthh r15, r16, r17 }
	{ adds r5, r6, r7 ; lwadd r15, r16, 5 }
	{ adds r5, r6, r7 ; mtspr 0x5, r16 }
	{ adds r5, r6, r7 ; sbadd r15, r16, 5 }
	{ adds r5, r6, r7 ; shrih r15, r16, 5 }
	{ adds r5, r6, r7 ; sneb r15, r16, r17 }
	{ adiffb_u r5, r6, r7 ; add r15, r16, r17 }
	{ adiffb_u r5, r6, r7 ; info 19 }
	{ adiffb_u r5, r6, r7 ; lnk r15 }
	{ adiffb_u r5, r6, r7 ; movei r15, 5 }
	{ adiffb_u r5, r6, r7 ; s2a r15, r16, r17 }
	{ adiffb_u r5, r6, r7 ; shrh r15, r16, r17 }
	{ adiffb_u r5, r6, r7 ; sltih r15, r16, 5 }
	{ adiffb_u r5, r6, r7 ; wh64 r15 }
	{ adiffh r5, r6, r7 ; fnop }
	{ adiffh r5, r6, r7 ; lh_u r15, r16 }
	{ adiffh r5, r6, r7 ; mnzb r15, r16, r17 }
	{ adiffh r5, r6, r7 ; rl r15, r16, r17 }
	{ adiffh r5, r6, r7 ; shlih r15, r16, 5 }
	{ adiffh r5, r6, r7 ; slti_u r15, r16, 5 }
	{ adiffh r5, r6, r7 ; sw r15, r16 }
	{ and r15, r16, r17 ; addi r5, r6, 5 ; lb r25, r26 }
	{ and r15, r16, r17 ; and r5, r6, r7 ; lh_u r25, r26 }
	{ and r15, r16, r17 ; bitx r5, r6 ; lb r25, r26 }
	{ and r15, r16, r17 ; clz r5, r6 ; lb r25, r26 }
	{ and r15, r16, r17 ; ctz r5, r6 ; sw r25, r26 }
	{ and r15, r16, r17 ; info 19 ; sh r25, r26 }
	{ and r15, r16, r17 ; lb r25, r26 ; movei r5, 5 }
	{ and r15, r16, r17 ; lb r25, r26 ; s1a r5, r6, r7 }
	{ and r15, r16, r17 ; lb r25, r26 ; tblidxb1 r5, r6 }
	{ and r15, r16, r17 ; lb_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ and r15, r16, r17 ; lb_u r25, r26 ; seq r5, r6, r7 }
	{ and r15, r16, r17 ; lb_u r25, r26 ; xor r5, r6, r7 }
	{ and r15, r16, r17 ; lh r25, r26 ; mulll_ss r5, r6, r7 }
	{ and r15, r16, r17 ; lh r25, r26 ; shli r5, r6, 5 }
	{ and r15, r16, r17 ; lh_u r25, r26 ; addi r5, r6, 5 }
	{ and r15, r16, r17 ; lh_u r25, r26 ; mullla_uu r5, r6, r7 }
	{ and r15, r16, r17 ; lh_u r25, r26 ; slt r5, r6, r7 }
	{ and r15, r16, r17 ; lw r25, r26 ; bitx r5, r6 }
	{ and r15, r16, r17 ; lw r25, r26 ; mz r5, r6, r7 }
	{ and r15, r16, r17 ; lw r25, r26 ; slte_u r5, r6, r7 }
	{ and r15, r16, r17 ; minih r5, r6, 5 }
	{ and r15, r16, r17 ; move r5, r6 ; sb r25, r26 }
	{ and r15, r16, r17 ; mulhh_ss r5, r6, r7 ; lw r25, r26 }
	{ and r15, r16, r17 ; mulhha_ss r5, r6, r7 ; lh_u r25, r26 }
	{ and r15, r16, r17 ; mulhl_su r5, r6, r7 }
	{ and r15, r16, r17 ; mulll_ss r5, r6, r7 ; lh_u r25, r26 }
	{ and r15, r16, r17 ; mullla_ss r5, r6, r7 ; lh r25, r26 }
	{ and r15, r16, r17 ; mvnz r5, r6, r7 ; lb r25, r26 }
	{ and r15, r16, r17 ; mz r5, r6, r7 ; lb r25, r26 }
	{ and r15, r16, r17 ; nop ; sw r25, r26 }
	{ and r15, r16, r17 ; or r5, r6, r7 ; sw r25, r26 }
	{ and r15, r16, r17 ; pcnt r5, r6 ; lw r25, r26 }
	{ and r15, r16, r17 ; prefetch r25 ; mulhh_uu r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch r25 ; s3a r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch r25 ; tblidxb3 r5, r6 }
	{ and r15, r16, r17 ; rli r5, r6, 5 ; sh r25, r26 }
	{ and r15, r16, r17 ; s2a r5, r6, r7 ; sh r25, r26 }
	{ and r15, r16, r17 ; sb r25, r26 ; addi r5, r6, 5 }
	{ and r15, r16, r17 ; sb r25, r26 ; mullla_uu r5, r6, r7 }
	{ and r15, r16, r17 ; sb r25, r26 ; slt r5, r6, r7 }
	{ and r15, r16, r17 ; seq r5, r6, r7 ; lw r25, r26 }
	{ and r15, r16, r17 ; sh r25, r26 ; add r5, r6, r7 }
	{ and r15, r16, r17 ; sh r25, r26 ; mullla_ss r5, r6, r7 }
	{ and r15, r16, r17 ; sh r25, r26 ; shri r5, r6, 5 }
	{ and r15, r16, r17 ; shl r5, r6, r7 ; lh_u r25, r26 }
	{ and r15, r16, r17 ; shlih r5, r6, 5 }
	{ and r15, r16, r17 ; shri r5, r6, 5 ; sh r25, r26 }
	{ and r15, r16, r17 ; slt_u r5, r6, r7 ; prefetch r25 }
	{ and r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
	{ and r15, r16, r17 ; slti r5, r6, 5 ; sh r25, r26 }
	{ and r15, r16, r17 ; sne r5, r6, r7 ; lh_u r25, r26 }
	{ and r15, r16, r17 ; srah r5, r6, r7 }
	{ and r15, r16, r17 ; sub r5, r6, r7 ; sh r25, r26 }
	{ and r15, r16, r17 ; sw r25, r26 ; movei r5, 5 }
	{ and r15, r16, r17 ; sw r25, r26 ; s1a r5, r6, r7 }
	{ and r15, r16, r17 ; sw r25, r26 ; tblidxb1 r5, r6 }
	{ and r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch r25 }
	{ and r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch r25 }
	{ and r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
	{ and r5, r6, r7 ; addib r15, r16, 5 }
	{ and r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
	{ and r5, r6, r7 ; ill ; lb r25, r26 }
	{ and r5, r6, r7 ; infol 0x1234 }
	{ and r5, r6, r7 ; lb r25, r26 ; move r15, r16 }
	{ and r5, r6, r7 ; lb r25, r26 ; slte r15, r16, r17 }
	{ and r5, r6, r7 ; lb_u r25, r26 ; movei r15, 5 }
	{ and r5, r6, r7 ; lb_u r25, r26 ; slte_u r15, r16, r17 }
	{ and r5, r6, r7 ; lh r25, r26 ; move r15, r16 }
	{ and r5, r6, r7 ; lh r25, r26 ; slte r15, r16, r17 }
	{ and r5, r6, r7 ; lh_u r25, r26 ; movei r15, 5 }
	{ and r5, r6, r7 ; lh_u r25, r26 ; slte_u r15, r16, r17 }
	{ and r5, r6, r7 ; lw r25, r26 ; mnz r15, r16, r17 }
	{ and r5, r6, r7 ; lw r25, r26 ; slt_u r15, r16, r17 }
	{ and r5, r6, r7 ; minb_u r15, r16, r17 }
	{ and r5, r6, r7 ; move r15, r16 ; lh_u r25, r26 }
	{ and r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
	{ and r5, r6, r7 ; nop ; sw r25, r26 }
	{ and r5, r6, r7 ; or r15, r16, r17 ; sw r25, r26 }
	{ and r5, r6, r7 ; prefetch r25 ; andi r15, r16, 5 }
	{ and r5, r6, r7 ; prefetch r25 ; shli r15, r16, 5 }
	{ and r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
	{ and r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
	{ and r5, r6, r7 ; s3a r15, r16, r17 ; lw r25, r26 }
	{ and r5, r6, r7 ; sb r25, r26 ; or r15, r16, r17 }
	{ and r5, r6, r7 ; sb r25, r26 ; sra r15, r16, r17 }
	{ and r5, r6, r7 ; seqi r15, r16, 5 ; lh r25, r26 }
	{ and r5, r6, r7 ; sh r25, r26 ; movei r15, 5 }
	{ and r5, r6, r7 ; sh r25, r26 ; slte_u r15, r16, r17 }
	{ and r5, r6, r7 ; shlb r15, r16, r17 }
	{ and r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
	{ and r5, r6, r7 ; slt r15, r16, r17 ; lh r25, r26 }
	{ and r5, r6, r7 ; slte r15, r16, r17 ; lb r25, r26 }
	{ and r5, r6, r7 ; slteb r15, r16, r17 }
	{ and r5, r6, r7 ; slti_u r15, r16, 5 ; lw r25, r26 }
	{ and r5, r6, r7 ; sneb r15, r16, r17 }
	{ and r5, r6, r7 ; srai r15, r16, 5 ; sb r25, r26 }
	{ and r5, r6, r7 ; subs r15, r16, r17 }
	{ and r5, r6, r7 ; sw r25, r26 ; s2a r15, r16, r17 }
	{ and r5, r6, r7 ; swadd r15, r16, 5 }
	{ andi r15, r16, 5 ; add r5, r6, r7 ; sb r25, r26 }
	{ andi r15, r16, 5 ; addli r5, r6, 0x1234 }
	{ andi r15, r16, 5 ; andi r5, r6, 5 ; prefetch r25 }
	{ andi r15, r16, 5 ; bytex r5, r6 ; lh r25, r26 }
	{ andi r15, r16, 5 ; ctz r5, r6 ; lb r25, r26 }
	{ andi r15, r16, 5 ; fnop }
	{ andi r15, r16, 5 ; lb r25, r26 ; bitx r5, r6 }
	{ andi r15, r16, 5 ; lb r25, r26 ; mz r5, r6, r7 }
	{ andi r15, r16, 5 ; lb r25, r26 ; slte_u r5, r6, r7 }
	{ andi r15, r16, 5 ; lb_u r25, r26 ; ctz r5, r6 }
	{ andi r15, r16, 5 ; lb_u r25, r26 ; or r5, r6, r7 }
	{ andi r15, r16, 5 ; lb_u r25, r26 ; sne r5, r6, r7 }
	{ andi r15, r16, 5 ; lh r25, r26 ; mnz r5, r6, r7 }
	{ andi r15, r16, 5 ; lh r25, r26 ; rl r5, r6, r7 }
	{ andi r15, r16, 5 ; lh r25, r26 ; sub r5, r6, r7 }
	{ andi r15, r16, 5 ; lh_u r25, r26 ; mulhh_ss r5, r6, r7 }
	{ andi r15, r16, 5 ; lh_u r25, r26 ; s2a r5, r6, r7 }
	{ andi r15, r16, 5 ; lh_u r25, r26 ; tblidxb2 r5, r6 }
	{ andi r15, r16, 5 ; lw r25, r26 ; mulhha_uu r5, r6, r7 }
	{ andi r15, r16, 5 ; lw r25, r26 ; seqi r5, r6, 5 }
	{ andi r15, r16, 5 ; lw r25, r26 }
	{ andi r15, r16, 5 ; mnzb r5, r6, r7 }
	{ andi r15, r16, 5 ; movei r5, 5 ; sw r25, r26 }
	{ andi r15, r16, 5 ; mulhh_uu r5, r6, r7 ; prefetch r25 }
	{ andi r15, r16, 5 ; mulhha_uu r5, r6, r7 ; lw r25, r26 }
	{ andi r15, r16, 5 ; mulhlsa_uu r5, r6, r7 ; prefetch r25 }
	{ andi r15, r16, 5 ; mulll_uu r5, r6, r7 ; lw r25, r26 }
	{ andi r15, r16, 5 ; mullla_uu r5, r6, r7 ; lh_u r25, r26 }
	{ andi r15, r16, 5 ; mvz r5, r6, r7 ; lh r25, r26 }
	{ andi r15, r16, 5 ; nop ; lb r25, r26 }
	{ andi r15, r16, 5 ; or r5, r6, r7 ; lb r25, r26 }
	{ andi r15, r16, 5 ; packbs_u r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch r25 ; clz r5, r6 }
	{ andi r15, r16, 5 ; prefetch r25 ; nor r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch r25 ; slti_u r5, r6, 5 }
	{ andi r15, r16, 5 ; rl r5, r6, r7 }
	{ andi r15, r16, 5 ; s1a r5, r6, r7 }
	{ andi r15, r16, 5 ; s3a r5, r6, r7 }
	{ andi r15, r16, 5 ; sb r25, r26 ; mulhh_ss r5, r6, r7 }
	{ andi r15, r16, 5 ; sb r25, r26 ; s2a r5, r6, r7 }
	{ andi r15, r16, 5 ; sb r25, r26 ; tblidxb2 r5, r6 }
	{ andi r15, r16, 5 ; seqi r5, r6, 5 ; lw r25, r26 }
	{ andi r15, r16, 5 ; sh r25, r26 ; movei r5, 5 }
	{ andi r15, r16, 5 ; sh r25, r26 ; s1a r5, r6, r7 }
	{ andi r15, r16, 5 ; sh r25, r26 ; tblidxb1 r5, r6 }
	{ andi r15, r16, 5 ; shli r5, r6, 5 ; lh_u r25, r26 }
	{ andi r15, r16, 5 ; shrh r5, r6, r7 }
	{ andi r15, r16, 5 ; slt r5, r6, r7 ; sh r25, r26 }
	{ andi r15, r16, 5 ; slte r5, r6, r7 ; prefetch r25 }
	{ andi r15, r16, 5 ; slth_u r5, r6, r7 }
	{ andi r15, r16, 5 ; slti_u r5, r6, 5 }
	{ andi r15, r16, 5 ; sra r5, r6, r7 ; lh_u r25, r26 }
	{ andi r15, r16, 5 ; sraih r5, r6, 5 }
	{ andi r15, r16, 5 ; sw r25, r26 ; bitx r5, r6 }
	{ andi r15, r16, 5 ; sw r25, r26 ; mz r5, r6, r7 }
	{ andi r15, r16, 5 ; sw r25, r26 ; slte_u r5, r6, r7 }
	{ andi r15, r16, 5 ; tblidxb0 r5, r6 ; sh r25, r26 }
	{ andi r15, r16, 5 ; tblidxb2 r5, r6 ; sh r25, r26 }
	{ andi r15, r16, 5 ; xor r5, r6, r7 ; sh r25, r26 }
	{ andi r5, r6, 5 ; addi r15, r16, 5 ; lh r25, r26 }
	{ andi r5, r6, 5 ; and r15, r16, r17 ; sh r25, r26 }
	{ andi r5, r6, 5 ; fnop ; lh_u r25, r26 }
	{ andi r5, r6, 5 ; info 19 ; lh r25, r26 }
	{ andi r5, r6, 5 ; lb r25, r26 ; add r15, r16, r17 }
	{ andi r5, r6, 5 ; lb r25, r26 ; seq r15, r16, r17 }
	{ andi r5, r6, 5 ; lb_u r25, r26 ; addi r15, r16, 5 }
	{ andi r5, r6, 5 ; lb_u r25, r26 ; seqi r15, r16, 5 }
	{ andi r5, r6, 5 ; lh r25, r26 ; add r15, r16, r17 }
	{ andi r5, r6, 5 ; lh r25, r26 ; seq r15, r16, r17 }
	{ andi r5, r6, 5 ; lh_u r25, r26 ; addi r15, r16, 5 }
	{ andi r5, r6, 5 ; lh_u r25, r26 ; seqi r15, r16, 5 }
	{ andi r5, r6, 5 ; lw r15, r16 }
	{ andi r5, r6, 5 ; lw r25, r26 ; s3a r15, r16, r17 }
	{ andi r5, r6, 5 ; lwadd r15, r16, 5 }
	{ andi r5, r6, 5 ; mnz r15, r16, r17 ; sh r25, r26 }
	{ andi r5, r6, 5 ; movei r15, 5 ; prefetch r25 }
	{ andi r5, r6, 5 ; nop ; lb r25, r26 }
	{ andi r5, r6, 5 ; or r15, r16, r17 ; lb r25, r26 }
	{ andi r5, r6, 5 ; packbs_u r15, r16, r17 }
	{ andi r5, r6, 5 ; prefetch r25 ; rl r15, r16, r17 }
	{ andi r5, r6, 5 ; prefetch r25 ; sub r15, r16, r17 }
	{ andi r5, r6, 5 ; rli r15, r16, 5 ; sb r25, r26 }
	{ andi r5, r6, 5 ; s2a r15, r16, r17 ; sb r25, r26 }
	{ andi r5, r6, 5 ; sb r25, r26 ; ill }
	{ andi r5, r6, 5 ; sb r25, r26 ; shri r15, r16, 5 }
	{ andi r5, r6, 5 ; seq r15, r16, r17 ; sb r25, r26 }
	{ andi r5, r6, 5 ; sh r25, r26 ; addi r15, r16, 5 }
	{ andi r5, r6, 5 ; sh r25, r26 ; seqi r15, r16, 5 }
	{ andi r5, r6, 5 ; shl r15, r16, r17 ; lh r25, r26 }
	{ andi r5, r6, 5 ; shlib r15, r16, 5 }
	{ andi r5, r6, 5 ; shri r15, r16, 5 ; sb r25, r26 }
	{ andi r5, r6, 5 ; slt_u r15, r16, r17 ; lw r25, r26 }
	{ andi r5, r6, 5 ; slte_u r15, r16, r17 ; lh r25, r26 }
	{ andi r5, r6, 5 ; slti r15, r16, 5 ; sb r25, r26 }
	{ andi r5, r6, 5 ; sne r15, r16, r17 ; lh r25, r26 }
	{ andi r5, r6, 5 ; srab r15, r16, r17 }
	{ andi r5, r6, 5 ; sub r15, r16, r17 ; sb r25, r26 }
	{ andi r5, r6, 5 ; sw r25, r26 ; mz r15, r16, r17 }
	{ andi r5, r6, 5 ; sw r25, r26 ; slti r15, r16, 5 }
	{ andi r5, r6, 5 ; xor r15, r16, r17 }
	{ auli r15, r16, 0x1234 ; bitx r5, r6 }
	{ auli r15, r16, 0x1234 ; minib_u r5, r6, 5 }
	{ auli r15, r16, 0x1234 ; mulhl_uu r5, r6, r7 }
	{ auli r15, r16, 0x1234 ; or r5, r6, r7 }
	{ auli r15, r16, 0x1234 ; seqh r5, r6, r7 }
	{ auli r15, r16, 0x1234 ; slte r5, r6, r7 }
	{ auli r15, r16, 0x1234 ; srai r5, r6, 5 }
	{ auli r5, r6, 0x1234 ; addi r15, r16, 5 }
	{ auli r5, r6, 0x1234 ; intlh r15, r16, r17 }
	{ auli r5, r6, 0x1234 ; maxb_u r15, r16, r17 }
	{ auli r5, r6, 0x1234 ; mzb r15, r16, r17 }
	{ auli r5, r6, 0x1234 ; seqb r15, r16, r17 }
	{ auli r5, r6, 0x1234 ; slt_u r15, r16, r17 }
	{ auli r5, r6, 0x1234 ; sra r15, r16, r17 }
	{ avgb_u r5, r6, r7 ; addbs_u r15, r16, r17 }
	{ avgb_u r5, r6, r7 ; inthb r15, r16, r17 }
	{ avgb_u r5, r6, r7 ; lw_na r15, r16 }
	{ avgb_u r5, r6, r7 ; movelis r15, 0x1234 }
	{ avgb_u r5, r6, r7 ; sb r15, r16 }
	{ avgb_u r5, r6, r7 ; shrib r15, r16, 5 }
	{ avgb_u r5, r6, r7 ; sne r15, r16, r17 }
	{ avgb_u r5, r6, r7 ; xori r15, r16, 5 }
	{ avgh r5, r6, r7 ; ill }
	{ avgh r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ avgh r5, r6, r7 ; move r15, r16 }
	{ avgh r5, r6, r7 ; s1a r15, r16, r17 }
	{ avgh r5, r6, r7 ; shrb r15, r16, r17 }
	{ avgh r5, r6, r7 ; sltib_u r15, r16, 5 }
	{ avgh r5, r6, r7 ; tns r15, r16 }
	{ bitx r5, r6 ; addi r15, r16, 5 ; lh r25, r26 }
	{ bitx r5, r6 ; and r15, r16, r17 ; sh r25, r26 }
	{ bitx r5, r6 ; fnop ; lh_u r25, r26 }
	{ bitx r5, r6 ; info 19 ; lh r25, r26 }
	{ bitx r5, r6 ; lb r25, r26 ; add r15, r16, r17 }
	{ bitx r5, r6 ; lb r25, r26 ; seq r15, r16, r17 }
	{ bitx r5, r6 ; lb_u r25, r26 ; addi r15, r16, 5 }
	{ bitx r5, r6 ; lb_u r25, r26 ; seqi r15, r16, 5 }
	{ bitx r5, r6 ; lh r25, r26 ; add r15, r16, r17 }
	{ bitx r5, r6 ; lh r25, r26 ; seq r15, r16, r17 }
	{ bitx r5, r6 ; lh_u r25, r26 ; addi r15, r16, 5 }
	{ bitx r5, r6 ; lh_u r25, r26 ; seqi r15, r16, 5 }
	{ bitx r5, r6 ; lw r15, r16 }
	{ bitx r5, r6 ; lw r25, r26 ; s3a r15, r16, r17 }
	{ bitx r5, r6 ; lwadd r15, r16, 5 }
	{ bitx r5, r6 ; mnz r15, r16, r17 ; sh r25, r26 }
	{ bitx r5, r6 ; movei r15, 5 ; prefetch r25 }
	{ bitx r5, r6 ; nop ; lb r25, r26 }
	{ bitx r5, r6 ; or r15, r16, r17 ; lb r25, r26 }
	{ bitx r5, r6 ; packbs_u r15, r16, r17 }
	{ bitx r5, r6 ; prefetch r25 ; rl r15, r16, r17 }
	{ bitx r5, r6 ; prefetch r25 ; sub r15, r16, r17 }
	{ bitx r5, r6 ; rli r15, r16, 5 ; sb r25, r26 }
	{ bitx r5, r6 ; s2a r15, r16, r17 ; sb r25, r26 }
	{ bitx r5, r6 ; sb r25, r26 ; ill }
	{ bitx r5, r6 ; sb r25, r26 ; shri r15, r16, 5 }
	{ bitx r5, r6 ; seq r15, r16, r17 ; sb r25, r26 }
	{ bitx r5, r6 ; sh r25, r26 ; addi r15, r16, 5 }
	{ bitx r5, r6 ; sh r25, r26 ; seqi r15, r16, 5 }
	{ bitx r5, r6 ; shl r15, r16, r17 ; lh r25, r26 }
	{ bitx r5, r6 ; shlib r15, r16, 5 }
	{ bitx r5, r6 ; shri r15, r16, 5 ; sb r25, r26 }
	{ bitx r5, r6 ; slt_u r15, r16, r17 ; lw r25, r26 }
	{ bitx r5, r6 ; slte_u r15, r16, r17 ; lh r25, r26 }
	{ bitx r5, r6 ; slti r15, r16, 5 ; sb r25, r26 }
	{ bitx r5, r6 ; sne r15, r16, r17 ; lh r25, r26 }
	{ bitx r5, r6 ; srab r15, r16, r17 }
	{ bitx r5, r6 ; sub r15, r16, r17 ; sb r25, r26 }
	{ bitx r5, r6 ; sw r25, r26 ; mz r15, r16, r17 }
	{ bitx r5, r6 ; sw r25, r26 ; slti r15, r16, 5 }
	{ bitx r5, r6 ; xor r15, r16, r17 }
	{ bytex r5, r6 ; addi r15, r16, 5 ; lh_u r25, r26 }
	{ bytex r5, r6 ; and r15, r16, r17 ; sw r25, r26 }
	{ bytex r5, r6 ; fnop ; lw r25, r26 }
	{ bytex r5, r6 ; info 19 ; lh_u r25, r26 }
	{ bytex r5, r6 ; lb r25, r26 ; addi r15, r16, 5 }
	{ bytex r5, r6 ; lb r25, r26 ; seqi r15, r16, 5 }
	{ bytex r5, r6 ; lb_u r25, r26 ; and r15, r16, r17 }
	{ bytex r5, r6 ; lb_u r25, r26 ; shl r15, r16, r17 }
	{ bytex r5, r6 ; lh r25, r26 ; addi r15, r16, 5 }
	{ bytex r5, r6 ; lh r25, r26 ; seqi r15, r16, 5 }
	{ bytex r5, r6 ; lh_u r25, r26 ; and r15, r16, r17 }
	{ bytex r5, r6 ; lh_u r25, r26 ; shl r15, r16, r17 }
	{ bytex r5, r6 ; lw r25, r26 ; add r15, r16, r17 }
	{ bytex r5, r6 ; lw r25, r26 ; seq r15, r16, r17 }
	{ bytex r5, r6 ; lwadd_na r15, r16, 5 }
	{ bytex r5, r6 ; mnz r15, r16, r17 ; sw r25, r26 }
	{ bytex r5, r6 ; movei r15, 5 ; sb r25, r26 }
	{ bytex r5, r6 ; nop ; lb_u r25, r26 }
	{ bytex r5, r6 ; or r15, r16, r17 ; lb_u r25, r26 }
	{ bytex r5, r6 ; packhb r15, r16, r17 }
	{ bytex r5, r6 ; prefetch r25 ; rli r15, r16, 5 }
	{ bytex r5, r6 ; prefetch r25 ; xor r15, r16, r17 }
	{ bytex r5, r6 ; rli r15, r16, 5 ; sh r25, r26 }
	{ bytex r5, r6 ; s2a r15, r16, r17 ; sh r25, r26 }
	{ bytex r5, r6 ; sb r25, r26 ; info 19 }
	{ bytex r5, r6 ; sb r25, r26 ; slt r15, r16, r17 }
	{ bytex r5, r6 ; seq r15, r16, r17 ; sh r25, r26 }
	{ bytex r5, r6 ; sh r25, r26 ; and r15, r16, r17 }
	{ bytex r5, r6 ; sh r25, r26 ; shl r15, r16, r17 }
	{ bytex r5, r6 ; shl r15, r16, r17 ; lh_u r25, r26 }
	{ bytex r5, r6 ; shlih r15, r16, 5 }
	{ bytex r5, r6 ; shri r15, r16, 5 ; sh r25, r26 }
	{ bytex r5, r6 ; slt_u r15, r16, r17 ; prefetch r25 }
	{ bytex r5, r6 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
	{ bytex r5, r6 ; slti r15, r16, 5 ; sh r25, r26 }
	{ bytex r5, r6 ; sne r15, r16, r17 ; lh_u r25, r26 }
	{ bytex r5, r6 ; srah r15, r16, r17 }
	{ bytex r5, r6 ; sub r15, r16, r17 ; sh r25, r26 }
	{ bytex r5, r6 ; sw r25, r26 ; nop }
	{ bytex r5, r6 ; sw r25, r26 ; slti_u r15, r16, 5 }
	{ bytex r5, r6 ; xori r15, r16, 5 }
	{ clz r5, r6 ; addi r15, r16, 5 ; prefetch r25 }
	{ clz r5, r6 ; andi r15, r16, 5 ; lb r25, r26 }
	{ clz r5, r6 ; fnop ; sb r25, r26 }
	{ clz r5, r6 ; info 19 ; prefetch r25 }
	{ clz r5, r6 ; lb r25, r26 ; andi r15, r16, 5 }
	{ clz r5, r6 ; lb r25, r26 ; shli r15, r16, 5 }
	{ clz r5, r6 ; lb_u r25, r26 ; fnop }
	{ clz r5, r6 ; lb_u r25, r26 ; shr r15, r16, r17 }
	{ clz r5, r6 ; lh r25, r26 ; andi r15, r16, 5 }
	{ clz r5, r6 ; lh r25, r26 ; shli r15, r16, 5 }
	{ clz r5, r6 ; lh_u r25, r26 ; fnop }
	{ clz r5, r6 ; lh_u r25, r26 ; shr r15, r16, r17 }
	{ clz r5, r6 ; lw r25, r26 ; and r15, r16, r17 }
	{ clz r5, r6 ; lw r25, r26 ; shl r15, r16, r17 }
	{ clz r5, r6 ; maxh r15, r16, r17 }
	{ clz r5, r6 ; mnzb r15, r16, r17 }
	{ clz r5, r6 ; movei r15, 5 ; sw r25, r26 }
	{ clz r5, r6 ; nop ; lh_u r25, r26 }
	{ clz r5, r6 ; or r15, r16, r17 ; lh_u r25, r26 }
	{ clz r5, r6 ; packlb r15, r16, r17 }
	{ clz r5, r6 ; prefetch r25 ; s2a r15, r16, r17 }
	{ clz r5, r6 ; raise }
	{ clz r5, r6 ; rli r15, r16, 5 }
	{ clz r5, r6 ; s2a r15, r16, r17 }
	{ clz r5, r6 ; sb r25, r26 ; move r15, r16 }
	{ clz r5, r6 ; sb r25, r26 ; slte r15, r16, r17 }
	{ clz r5, r6 ; seq r15, r16, r17 }
	{ clz r5, r6 ; sh r25, r26 ; fnop }
	{ clz r5, r6 ; sh r25, r26 ; shr r15, r16, r17 }
	{ clz r5, r6 ; shl r15, r16, r17 ; prefetch r25 }
	{ clz r5, r6 ; shr r15, r16, r17 ; lb_u r25, r26 }
	{ clz r5, r6 ; shri r15, r16, 5 }
	{ clz r5, r6 ; slt_u r15, r16, r17 ; sh r25, r26 }
	{ clz r5, r6 ; slte_u r15, r16, r17 ; prefetch r25 }
	{ clz r5, r6 ; slti r15, r16, 5 }
	{ clz r5, r6 ; sne r15, r16, r17 ; prefetch r25 }
	{ clz r5, r6 ; srai r15, r16, 5 ; lb_u r25, r26 }
	{ clz r5, r6 ; sub r15, r16, r17 }
	{ clz r5, r6 ; sw r25, r26 ; or r15, r16, r17 }
	{ clz r5, r6 ; sw r25, r26 ; sra r15, r16, r17 }
	{ crc32_32 r5, r6, r7 ; addb r15, r16, r17 }
	{ crc32_32 r5, r6, r7 ; infol 0x1234 }
	{ crc32_32 r5, r6, r7 ; lw r15, r16 }
	{ crc32_32 r5, r6, r7 ; moveli r15, 0x1234 }
	{ crc32_32 r5, r6, r7 ; s3a r15, r16, r17 }
	{ crc32_32 r5, r6, r7 ; shri r15, r16, 5 }
	{ crc32_32 r5, r6, r7 ; sltih_u r15, r16, 5 }
	{ crc32_32 r5, r6, r7 ; xor r15, r16, r17 }
	{ crc32_8 r5, r6, r7 ; icoh r15 }
	{ crc32_8 r5, r6, r7 ; lhadd r15, r16, 5 }
	{ crc32_8 r5, r6, r7 ; mnzh r15, r16, r17 }
	{ crc32_8 r5, r6, r7 ; rli r15, r16, 5 }
	{ crc32_8 r5, r6, r7 ; shr r15, r16, r17 }
	{ crc32_8 r5, r6, r7 ; sltib r15, r16, 5 }
	{ crc32_8 r5, r6, r7 ; swadd r15, r16, 5 }
	{ ctz r5, r6 ; addi r15, r16, 5 ; lb_u r25, r26 }
	{ ctz r5, r6 ; and r15, r16, r17 ; sb r25, r26 }
	{ ctz r5, r6 ; fnop ; lh r25, r26 }
	{ ctz r5, r6 ; info 19 ; lb_u r25, r26 }
	{ ctz r5, r6 ; lb r15, r16 }
	{ ctz r5, r6 ; lb r25, r26 ; s3a r15, r16, r17 }
	{ ctz r5, r6 ; lb_u r25, r26 ; add r15, r16, r17 }
	{ ctz r5, r6 ; lb_u r25, r26 ; seq r15, r16, r17 }
	{ ctz r5, r6 ; lh r15, r16 }
	{ ctz r5, r6 ; lh r25, r26 ; s3a r15, r16, r17 }
	{ ctz r5, r6 ; lh_u r25, r26 ; add r15, r16, r17 }
	{ ctz r5, r6 ; lh_u r25, r26 ; seq r15, r16, r17 }
	{ ctz r5, r6 ; lnk r15 }
	{ ctz r5, r6 ; lw r25, r26 ; s2a r15, r16, r17 }
	{ ctz r5, r6 ; lw_na r15, r16 }
	{ ctz r5, r6 ; mnz r15, r16, r17 ; sb r25, r26 }
	{ ctz r5, r6 ; movei r15, 5 ; lw r25, r26 }
	{ ctz r5, r6 ; mzh r15, r16, r17 }
	{ ctz r5, r6 ; nor r15, r16, r17 }
	{ ctz r5, r6 ; ori r15, r16, 5 }
	{ ctz r5, r6 ; prefetch r25 ; ori r15, r16, 5 }
	{ ctz r5, r6 ; prefetch r25 ; srai r15, r16, 5 }
	{ ctz r5, r6 ; rli r15, r16, 5 ; prefetch r25 }
	{ ctz r5, r6 ; s2a r15, r16, r17 ; prefetch r25 }
	{ ctz r5, r6 ; sb r25, r26 ; fnop }
	{ ctz r5, r6 ; sb r25, r26 ; shr r15, r16, r17 }
	{ ctz r5, r6 ; seq r15, r16, r17 ; prefetch r25 }
	{ ctz r5, r6 ; sh r25, r26 ; add r15, r16, r17 }
	{ ctz r5, r6 ; sh r25, r26 ; seq r15, r16, r17 }
	{ ctz r5, r6 ; shl r15, r16, r17 ; lb_u r25, r26 }
	{ ctz r5, r6 ; shli r15, r16, 5 }
	{ ctz r5, r6 ; shri r15, r16, 5 ; prefetch r25 }
	{ ctz r5, r6 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
	{ ctz r5, r6 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
	{ ctz r5, r6 ; slti r15, r16, 5 ; prefetch r25 }
	{ ctz r5, r6 ; sne r15, r16, r17 ; lb_u r25, r26 }
	{ ctz r5, r6 ; sra r15, r16, r17 }
	{ ctz r5, r6 ; sub r15, r16, r17 ; prefetch r25 }
	{ ctz r5, r6 ; sw r25, r26 ; movei r15, 5 }
	{ ctz r5, r6 ; sw r25, r26 ; slte_u r15, r16, r17 }
	{ ctz r5, r6 ; xor r15, r16, r17 ; sw r25, r26 }
	{ dtlbpr r15 ; avgb_u r5, r6, r7 }
	{ dtlbpr r15 ; minb_u r5, r6, r7 }
	{ dtlbpr r15 ; mulhl_su r5, r6, r7 }
	{ dtlbpr r15 ; nop }
	{ dtlbpr r15 ; seq r5, r6, r7 }
	{ dtlbpr r15 ; sltb r5, r6, r7 }
	{ dtlbpr r15 ; srab r5, r6, r7 }
	{ dword_align r5, r6, r7 ; addh r15, r16, r17 }
	{ dword_align r5, r6, r7 ; inthh r15, r16, r17 }
	{ dword_align r5, r6, r7 ; lwadd r15, r16, 5 }
	{ dword_align r5, r6, r7 ; mtspr 0x5, r16 }
	{ dword_align r5, r6, r7 ; sbadd r15, r16, 5 }
	{ dword_align r5, r6, r7 ; shrih r15, r16, 5 }
	{ dword_align r5, r6, r7 ; sneb r15, r16, r17 }
	{ finv r15 ; add r5, r6, r7 }
	{ finv r15 ; clz r5, r6 }
	{ finv r15 ; mm r5, r6, r7, 5, 7 }
	{ finv r15 ; mulhla_su r5, r6, r7 }
	{ finv r15 ; packbs_u r5, r6, r7 }
	{ finv r15 ; seqib r5, r6, 5 }
	{ finv r15 ; slteb r5, r6, r7 }
	{ finv r15 ; sraih r5, r6, 5 }
	{ flush r15 ; addih r5, r6, 5 }
	{ flush r15 ; infol 0x1234 }
	{ flush r15 ; movelis r5, 0x1234 }
	{ flush r15 ; mullla_ss r5, r6, r7 }
	{ flush r15 ; s1a r5, r6, r7 }
	{ flush r15 ; shlih r5, r6, 5 }
	{ flush r15 ; slti_u r5, r6, 5 }
	{ flush r15 ; tblidxb0 r5, r6 }
	{ fnop ; add r5, r6, r7 ; lw r25, r26 }
	{ fnop ; addi r15, r16, 5 ; sb r25, r26 }
	{ fnop ; addlis r15, r16, 0x1234 }
	{ fnop ; and r5, r6, r7 ; lw r25, r26 }
	{ fnop ; andi r5, r6, 5 ; lw r25, r26 }
	{ fnop ; bytex r5, r6 ; lb r25, r26 }
	{ fnop ; crc32_32 r5, r6, r7 }
	{ fnop ; fnop ; lw r25, r26 }
	{ fnop ; info 19 ; lh_u r25, r26 }
	{ fnop ; jr r15 }
	{ fnop ; lb r25, r26 ; move r15, r16 }
	{ fnop ; lb r25, r26 ; or r15, r16, r17 }
	{ fnop ; lb r25, r26 ; shl r5, r6, r7 }
	{ fnop ; lb r25, r26 ; sne r5, r6, r7 }
	{ fnop ; lb_u r25, r26 ; and r5, r6, r7 }
	{ fnop ; lb_u r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ fnop ; lb_u r25, r26 ; rli r5, r6, 5 }
	{ fnop ; lb_u r25, r26 ; slt r5, r6, r7 }
	{ fnop ; lb_u r25, r26 ; tblidxb1 r5, r6 }
	{ fnop ; lh r25, r26 ; ctz r5, r6 }
	{ fnop ; lh r25, r26 ; mvz r5, r6, r7 }
	{ fnop ; lh r25, r26 ; s3a r5, r6, r7 }
	{ fnop ; lh r25, r26 ; slte_u r5, r6, r7 }
	{ fnop ; lh_u r15, r16 }
	{ fnop ; lh_u r25, r26 ; movei r15, 5 }
	{ fnop ; lh_u r25, r26 ; ori r15, r16, 5 }
	{ fnop ; lh_u r25, r26 ; shli r5, r6, 5 }
	{ fnop ; lh_u r25, r26 ; sra r5, r6, r7 }
	{ fnop ; lw r25, r26 ; and r15, r16, r17 }
	{ fnop ; lw r25, r26 ; mulhha_uu r5, r6, r7 }
	{ fnop ; lw r25, r26 ; rli r15, r16, 5 }
	{ fnop ; lw r25, r26 ; slt r15, r16, r17 }
	{ fnop ; lw r25, r26 ; tblidxb0 r5, r6 }
	{ fnop ; minb_u r15, r16, r17 }
	{ fnop ; mnz r5, r6, r7 ; lb r25, r26 }
	{ fnop ; move r15, r16 ; sb r25, r26 }
	{ fnop ; movei r15, 5 ; sb r25, r26 }
	{ fnop ; mulhh_ss r5, r6, r7 ; lb_u r25, r26 }
	{ fnop ; mulhha_ss r5, r6, r7 ; lb r25, r26 }
	{ fnop ; mulhha_uu r5, r6, r7 }
	{ fnop ; mulll_ss r5, r6, r7 ; lb r25, r26 }
	{ fnop ; mulll_uu r5, r6, r7 }
	{ fnop ; mullla_uu r5, r6, r7 ; sw r25, r26 }
	{ fnop ; mvz r5, r6, r7 ; sh r25, r26 }
	{ fnop ; mz r5, r6, r7 ; sh r25, r26 }
	{ fnop ; nor r15, r16, r17 ; lh_u r25, r26 }
	{ fnop ; or r15, r16, r17 ; lh_u r25, r26 }
	{ fnop ; ori r15, r16, 5 ; lh_u r25, r26 }
	{ fnop ; packhb r5, r6, r7 }
	{ fnop ; prefetch r25 ; and r15, r16, r17 }
	{ fnop ; prefetch r25 ; mulhha_uu r5, r6, r7 }
	{ fnop ; prefetch r25 ; rli r15, r16, 5 }
	{ fnop ; prefetch r25 ; slt r15, r16, r17 }
	{ fnop ; prefetch r25 ; tblidxb0 r5, r6 }
	{ fnop ; rl r5, r6, r7 ; lh r25, r26 }
	{ fnop ; rli r5, r6, 5 ; lh r25, r26 }
	{ fnop ; s1a r5, r6, r7 ; lh r25, r26 }
	{ fnop ; s2a r5, r6, r7 ; lh r25, r26 }
	{ fnop ; s3a r5, r6, r7 ; lh r25, r26 }
	{ fnop ; sb r25, r26 ; and r5, r6, r7 }
	{ fnop ; sb r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ fnop ; sb r25, r26 ; rli r5, r6, 5 }
	{ fnop ; sb r25, r26 ; slt r5, r6, r7 }
	{ fnop ; sb r25, r26 ; tblidxb1 r5, r6 }
	{ fnop ; seq r5, r6, r7 ; lh_u r25, r26 }
	{ fnop ; seqi r15, r16, 5 }
	{ fnop ; sh r25, r26 ; and r15, r16, r17 }
	{ fnop ; sh r25, r26 ; mulhha_uu r5, r6, r7 }
	{ fnop ; sh r25, r26 ; rli r15, r16, 5 }
	{ fnop ; sh r25, r26 ; slt r15, r16, r17 }
	{ fnop ; sh r25, r26 ; tblidxb0 r5, r6 }
	{ fnop ; shl r5, r6, r7 ; lh r25, r26 }
	{ fnop ; shli r15, r16, 5 ; sw r25, r26 }
	{ fnop ; shr r15, r16, r17 ; lw r25, r26 }
	{ fnop ; shri r15, r16, 5 ; lb r25, r26 }
	{ fnop ; shrib r15, r16, 5 }
	{ fnop ; slt r5, r6, r7 ; sb r25, r26 }
	{ fnop ; slt_u r5, r6, r7 ; sb r25, r26 }
	{ fnop ; slte r5, r6, r7 ; lh r25, r26 }
	{ fnop ; slte_u r5, r6, r7 ; lh r25, r26 }
	{ fnop ; slti r15, r16, 5 ; lb r25, r26 }
	{ fnop ; slti_u r15, r16, 5 ; lb r25, r26 }
	{ fnop ; sltib r15, r16, 5 }
	{ fnop ; sne r5, r6, r7 ; lh r25, r26 }
	{ fnop ; sra r15, r16, r17 ; sw r25, r26 }
	{ fnop ; srai r15, r16, 5 ; lw r25, r26 }
	{ fnop ; sub r15, r16, r17 ; lb r25, r26 }
	{ fnop ; subb r15, r16, r17 }
	{ fnop ; sw r25, r26 ; bytex r5, r6 }
	{ fnop ; sw r25, r26 ; mullla_uu r5, r6, r7 }
	{ fnop ; sw r25, r26 ; s2a r5, r6, r7 }
	{ fnop ; sw r25, r26 ; slte r5, r6, r7 }
	{ fnop ; sw r25, r26 ; xor r5, r6, r7 }
	{ fnop ; tblidxb1 r5, r6 ; sh r25, r26 }
	{ fnop ; tblidxb3 r5, r6 ; sh r25, r26 }
	{ fnop ; xor r5, r6, r7 ; prefetch r25 }
	{ icoh r15 ; and r5, r6, r7 }
	{ icoh r15 ; maxh r5, r6, r7 }
	{ icoh r15 ; mulhha_uu r5, r6, r7 }
	{ icoh r15 ; mz r5, r6, r7 }
	{ icoh r15 ; sadb_u r5, r6, r7 }
	{ icoh r15 ; shrih r5, r6, 5 }
	{ icoh r15 ; sneb r5, r6, r7 }
	{ ill ; add r5, r6, r7 ; lb r25, r26 }
	{ ill ; addi r5, r6, 5 ; sb r25, r26 }
	{ ill ; and r5, r6, r7 }
	{ ill ; bitx r5, r6 ; sb r25, r26 }
	{ ill ; clz r5, r6 ; sb r25, r26 }
	{ ill ; fnop ; lh_u r25, r26 }
	{ ill ; intlb r5, r6, r7 }
	{ ill ; lb r25, r26 ; mulll_ss r5, r6, r7 }
	{ ill ; lb r25, r26 ; shli r5, r6, 5 }
	{ ill ; lb_u r25, r26 ; addi r5, r6, 5 }
	{ ill ; lb_u r25, r26 ; mullla_uu r5, r6, r7 }
	{ ill ; lb_u r25, r26 ; slt r5, r6, r7 }
	{ ill ; lh r25, r26 ; bitx r5, r6 }
	{ ill ; lh r25, r26 ; mz r5, r6, r7 }
	{ ill ; lh r25, r26 ; slte_u r5, r6, r7 }
	{ ill ; lh_u r25, r26 ; ctz r5, r6 }
	{ ill ; lh_u r25, r26 ; or r5, r6, r7 }
	{ ill ; lh_u r25, r26 ; sne r5, r6, r7 }
	{ ill ; lw r25, r26 ; mnz r5, r6, r7 }
	{ ill ; lw r25, r26 ; rl r5, r6, r7 }
	{ ill ; lw r25, r26 ; sub r5, r6, r7 }
	{ ill ; mnz r5, r6, r7 ; lw r25, r26 }
	{ ill ; movei r5, 5 ; lh r25, r26 }
	{ ill ; mulhh_su r5, r6, r7 }
	{ ill ; mulhha_ss r5, r6, r7 }
	{ ill ; mulhla_uu r5, r6, r7 }
	{ ill ; mulll_ss r5, r6, r7 }
	{ ill ; mullla_ss r5, r6, r7 ; sw r25, r26 }
	{ ill ; mvnz r5, r6, r7 ; sb r25, r26 }
	{ ill ; mz r5, r6, r7 ; sb r25, r26 }
	{ ill ; nor r5, r6, r7 ; lw r25, r26 }
	{ ill ; ori r5, r6, 5 ; lw r25, r26 }
	{ ill ; prefetch r25 ; add r5, r6, r7 }
	{ ill ; prefetch r25 ; mullla_ss r5, r6, r7 }
	{ ill ; prefetch r25 ; shri r5, r6, 5 }
	{ ill ; rl r5, r6, r7 ; lh_u r25, r26 }
	{ ill ; s1a r5, r6, r7 ; lh_u r25, r26 }
	{ ill ; s3a r5, r6, r7 ; lh_u r25, r26 }
	{ ill ; sb r25, r26 ; ctz r5, r6 }
	{ ill ; sb r25, r26 ; or r5, r6, r7 }
	{ ill ; sb r25, r26 ; sne r5, r6, r7 }
	{ ill ; seqb r5, r6, r7 }
	{ ill ; sh r25, r26 ; clz r5, r6 }
	{ ill ; sh r25, r26 ; nor r5, r6, r7 }
	{ ill ; sh r25, r26 ; slti_u r5, r6, 5 }
	{ ill ; shl r5, r6, r7 }
	{ ill ; shr r5, r6, r7 ; prefetch r25 }
	{ ill ; slt r5, r6, r7 ; lb_u r25, r26 }
	{ ill ; sltb_u r5, r6, r7 }
	{ ill ; slte_u r5, r6, r7 }
	{ ill ; slti_u r5, r6, 5 ; lh_u r25, r26 }
	{ ill ; sne r5, r6, r7 }
	{ ill ; srai r5, r6, 5 ; prefetch r25 }
	{ ill ; subhs r5, r6, r7 }
	{ ill ; sw r25, r26 ; mulll_ss r5, r6, r7 }
	{ ill ; sw r25, r26 ; shli r5, r6, 5 }
	{ ill ; tblidxb0 r5, r6 ; lb_u r25, r26 }
	{ ill ; tblidxb2 r5, r6 ; lb_u r25, r26 }
	{ ill ; xor r5, r6, r7 ; lb_u r25, r26 }
	{ info 19 ; add r5, r6, r7 ; lb r25, r26 }
	{ info 19 ; addi r15, r16, 5 ; lh r25, r26 }
	{ info 19 ; addih r15, r16, 5 }
	{ info 19 ; and r5, r6, r7 ; lb r25, r26 }
	{ info 19 ; andi r5, r6, 5 ; lb r25, r26 }
	{ info 19 ; bitx r5, r6 ; sb r25, r26 }
	{ info 19 ; clz r5, r6 ; sb r25, r26 }
	{ info 19 ; fnop ; lb r25, r26 }
	{ info 19 ; ill }
	{ info 19 ; inv r15 }
	{ info 19 ; lb r25, r26 ; ill }
	{ info 19 ; lb r25, r26 ; mz r5, r6, r7 }
	{ info 19 ; lb r25, r26 ; seq r5, r6, r7 }
	{ info 19 ; lb r25, r26 ; slti r5, r6, 5 }
	{ info 19 ; lb_u r25, r26 ; add r5, r6, r7 }
	{ info 19 ; lb_u r25, r26 ; mulhh_ss r5, r6, r7 }
	{ info 19 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ info 19 ; lb_u r25, r26 ; shr r5, r6, r7 }
	{ info 19 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ info 19 ; lh r25, r26 ; andi r5, r6, 5 }
	{ info 19 ; lh r25, r26 ; mulll_uu r5, r6, r7 }
	{ info 19 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ info 19 ; lh r25, r26 ; slt_u r5, r6, r7 }
	{ info 19 ; lh r25, r26 ; tblidxb3 r5, r6 }
	{ info 19 ; lh_u r25, r26 ; mnz r15, r16, r17 }
	{ info 19 ; lh_u r25, r26 ; nor r15, r16, r17 }
	{ info 19 ; lh_u r25, r26 ; seqi r5, r6, 5 }
	{ info 19 ; lh_u r25, r26 ; slti_u r5, r6, 5 }
	{ info 19 ; lw r25, r26 ; add r15, r16, r17 }
	{ info 19 ; lw r25, r26 ; movei r5, 5 }
	{ info 19 ; lw r25, r26 ; ori r5, r6, 5 }
	{ info 19 ; lw r25, r26 ; shr r15, r16, r17 }
	{ info 19 ; lw r25, r26 ; srai r15, r16, 5 }
	{ info 19 ; maxih r15, r16, 5 }
	{ info 19 ; mnz r15, r16, r17 ; sb r25, r26 }
	{ info 19 ; move r15, r16 ; lh r25, r26 }
	{ info 19 ; movei r15, 5 ; lh r25, r26 }
	{ info 19 ; movelis r15, 0x1234 }
	{ info 19 ; mulhh_uu r5, r6, r7 ; sb r25, r26 }
	{ info 19 ; mulhha_uu r5, r6, r7 ; prefetch r25 }
	{ info 19 ; mulhlsa_uu r5, r6, r7 ; sb r25, r26 }
	{ info 19 ; mulll_uu r5, r6, r7 ; prefetch r25 }
	{ info 19 ; mullla_uu r5, r6, r7 ; lw r25, r26 }
	{ info 19 ; mvz r5, r6, r7 ; lh_u r25, r26 }
	{ info 19 ; mz r5, r6, r7 ; lh_u r25, r26 }
	{ info 19 ; nop }
	{ info 19 ; nor r5, r6, r7 }
	{ info 19 ; or r5, r6, r7 }
	{ info 19 ; ori r5, r6, 5 }
	{ info 19 ; prefetch r25 ; add r15, r16, r17 }
	{ info 19 ; prefetch r25 ; movei r5, 5 }
	{ info 19 ; prefetch r25 ; ori r5, r6, 5 }
	{ info 19 ; prefetch r25 ; shr r15, r16, r17 }
	{ info 19 ; prefetch r25 ; srai r15, r16, 5 }
	{ info 19 ; rl r15, r16, r17 ; sw r25, r26 }
	{ info 19 ; rli r15, r16, 5 ; sw r25, r26 }
	{ info 19 ; s1a r15, r16, r17 ; sw r25, r26 }
	{ info 19 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ info 19 ; s3a r15, r16, r17 ; sw r25, r26 }
	{ info 19 ; sb r25, r26 ; add r5, r6, r7 }
	{ info 19 ; sb r25, r26 ; mulhh_ss r5, r6, r7 }
	{ info 19 ; sb r25, r26 ; pcnt r5, r6 }
	{ info 19 ; sb r25, r26 ; shr r5, r6, r7 }
	{ info 19 ; sb r25, r26 ; srai r5, r6, 5 }
	{ info 19 ; seq r15, r16, r17 }
	{ info 19 ; seqi r15, r16, 5 ; prefetch r25 }
	{ info 19 ; sh r25, r26 ; add r15, r16, r17 }
	{ info 19 ; sh r25, r26 ; movei r5, 5 }
	{ info 19 ; sh r25, r26 ; ori r5, r6, 5 }
	{ info 19 ; sh r25, r26 ; shr r15, r16, r17 }
	{ info 19 ; sh r25, r26 ; srai r15, r16, 5 }
	{ info 19 ; shl r15, r16, r17 ; sw r25, r26 }
	{ info 19 ; shli r15, r16, 5 ; lw r25, r26 }
	{ info 19 ; shr r15, r16, r17 ; lb r25, r26 }
	{ info 19 ; shrb r15, r16, r17 }
	{ info 19 ; shri r5, r6, 5 ; sb r25, r26 }
	{ info 19 ; slt r5, r6, r7 ; lh r25, r26 }
	{ info 19 ; slt_u r5, r6, r7 ; lh r25, r26 }
	{ info 19 ; slte r15, r16, r17 ; sw r25, r26 }
	{ info 19 ; slte_u r15, r16, r17 ; sw r25, r26 }
	{ info 19 ; slth r15, r16, r17 }
	{ info 19 ; slti r5, r6, 5 ; sb r25, r26 }
	{ info 19 ; slti_u r5, r6, 5 ; sb r25, r26 }
	{ info 19 ; sne r15, r16, r17 ; sw r25, r26 }
	{ info 19 ; sra r15, r16, r17 ; lw r25, r26 }
	{ info 19 ; srai r15, r16, 5 ; lb r25, r26 }
	{ info 19 ; sraib r15, r16, 5 }
	{ info 19 ; sub r5, r6, r7 ; sb r25, r26 }
	{ info 19 ; sw r25, r26 ; and r5, r6, r7 }
	{ info 19 ; sw r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ info 19 ; sw r25, r26 ; rli r5, r6, 5 }
	{ info 19 ; sw r25, r26 ; slt r5, r6, r7 }
	{ info 19 ; sw r25, r26 ; tblidxb1 r5, r6 }
	{ info 19 ; tblidxb1 r5, r6 ; lh_u r25, r26 }
	{ info 19 ; tblidxb3 r5, r6 ; lh_u r25, r26 }
	{ info 19 ; xor r5, r6, r7 ; lb_u r25, r26 }
	{ infol 0x1234 ; addhs r5, r6, r7 }
	{ infol 0x1234 ; auli r5, r6, 0x1234 }
	{ infol 0x1234 ; inthh r15, r16, r17 }
	{ infol 0x1234 ; lnk r15 }
	{ infol 0x1234 ; minib_u r5, r6, 5 }
	{ infol 0x1234 ; mulhh_ss r5, r6, r7 }
	{ infol 0x1234 ; mullla_su r5, r6, r7 }
	{ infol 0x1234 ; packhb r15, r16, r17 }
	{ infol 0x1234 ; sadah r5, r6, r7 }
	{ infol 0x1234 ; shadd r15, r16, 5 }
	{ infol 0x1234 ; shri r5, r6, 5 }
	{ infol 0x1234 ; slteb_u r5, r6, r7 }
	{ infol 0x1234 ; sltih_u r5, r6, 5 }
	{ infol 0x1234 ; sub r5, r6, r7 }
	{ infol 0x1234 ; xor r5, r6, r7 }
	{ inthb r15, r16, r17 ; avgh r5, r6, r7 }
	{ inthb r15, r16, r17 ; minh r5, r6, r7 }
	{ inthb r15, r16, r17 ; mulhl_us r5, r6, r7 }
	{ inthb r15, r16, r17 ; nor r5, r6, r7 }
	{ inthb r15, r16, r17 ; seqb r5, r6, r7 }
	{ inthb r15, r16, r17 ; sltb_u r5, r6, r7 }
	{ inthb r15, r16, r17 ; srah r5, r6, r7 }
	{ inthb r5, r6, r7 ; addhs r15, r16, r17 }
	{ inthb r5, r6, r7 ; intlb r15, r16, r17 }
	{ inthb r5, r6, r7 ; lwadd_na r15, r16, 5 }
	{ inthb r5, r6, r7 ; mz r15, r16, r17 }
	{ inthb r5, r6, r7 ; seq r15, r16, r17 }
	{ inthb r5, r6, r7 ; slt r15, r16, r17 }
	{ inthb r5, r6, r7 ; sneh r15, r16, r17 }
	{ inthh r15, r16, r17 ; addb r5, r6, r7 }
	{ inthh r15, r16, r17 ; crc32_32 r5, r6, r7 }
	{ inthh r15, r16, r17 ; mnz r5, r6, r7 }
	{ inthh r15, r16, r17 ; mulhla_us r5, r6, r7 }
	{ inthh r15, r16, r17 ; packhb r5, r6, r7 }
	{ inthh r15, r16, r17 ; seqih r5, r6, 5 }
	{ inthh r15, r16, r17 ; slteb_u r5, r6, r7 }
	{ inthh r15, r16, r17 ; sub r5, r6, r7 }
	{ inthh r5, r6, r7 ; addli r15, r16, 0x1234 }
	{ inthh r5, r6, r7 ; jalr r15 }
	{ inthh r5, r6, r7 ; maxih r15, r16, 5 }
	{ inthh r5, r6, r7 ; nor r15, r16, r17 }
	{ inthh r5, r6, r7 ; seqib r15, r16, 5 }
	{ inthh r5, r6, r7 ; slte r15, r16, r17 }
	{ inthh r5, r6, r7 ; srai r15, r16, 5 }
	{ intlb r15, r16, r17 ; addi r5, r6, 5 }
	{ intlb r15, r16, r17 ; fnop }
	{ intlb r15, r16, r17 ; movei r5, 5 }
	{ intlb r15, r16, r17 ; mulll_su r5, r6, r7 }
	{ intlb r15, r16, r17 ; rl r5, r6, r7 }
	{ intlb r15, r16, r17 ; shli r5, r6, 5 }
	{ intlb r15, r16, r17 ; slth_u r5, r6, r7 }
	{ intlb r15, r16, r17 ; subhs r5, r6, r7 }
	{ intlb r5, r6, r7 ; andi r15, r16, 5 }
	{ intlb r5, r6, r7 ; lb r15, r16 }
	{ intlb r5, r6, r7 ; minh r15, r16, r17 }
	{ intlb r5, r6, r7 ; packhb r15, r16, r17 }
	{ intlb r5, r6, r7 ; shl r15, r16, r17 }
	{ intlb r5, r6, r7 ; slteh r15, r16, r17 }
	{ intlb r5, r6, r7 ; subb r15, r16, r17 }
	{ intlh r15, r16, r17 ; addlis r5, r6, 0x1234 }
	{ intlh r15, r16, r17 ; inthh r5, r6, r7 }
	{ intlh r15, r16, r17 ; mulhh_su r5, r6, r7 }
	{ intlh r15, r16, r17 ; mullla_uu r5, r6, r7 }
	{ intlh r15, r16, r17 ; s3a r5, r6, r7 }
	{ intlh r15, r16, r17 ; shrb r5, r6, r7 }
	{ intlh r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ intlh r15, r16, r17 ; tblidxb2 r5, r6 }
	{ intlh r5, r6, r7 ; flush r15 }
	{ intlh r5, r6, r7 ; lh r15, r16 }
	{ intlh r5, r6, r7 ; mnz r15, r16, r17 }
	{ intlh r5, r6, r7 ; raise }
	{ intlh r5, r6, r7 ; shlib r15, r16, 5 }
	{ intlh r5, r6, r7 ; slti r15, r16, 5 }
	{ intlh r5, r6, r7 ; subs r15, r16, r17 }
	{ inv r15 ; and r5, r6, r7 }
	{ inv r15 ; maxh r5, r6, r7 }
	{ inv r15 ; mulhha_uu r5, r6, r7 }
	{ inv r15 ; mz r5, r6, r7 }
	{ inv r15 ; sadb_u r5, r6, r7 }
	{ inv r15 ; shrih r5, r6, 5 }
	{ inv r15 ; sneb r5, r6, r7 }
	{ iret ; add r5, r6, r7 }
	{ iret ; clz r5, r6 }
	{ iret ; mm r5, r6, r7, 5, 7 }
	{ iret ; mulhla_su r5, r6, r7 }
	{ iret ; packbs_u r5, r6, r7 }
	{ iret ; seqib r5, r6, 5 }
	{ iret ; slteb r5, r6, r7 }
	{ iret ; sraih r5, r6, 5 }
	{ jalr r15 ; addih r5, r6, 5 }
	{ jalr r15 ; infol 0x1234 }
	{ jalr r15 ; movelis r5, 0x1234 }
	{ jalr r15 ; mullla_ss r5, r6, r7 }
	{ jalr r15 ; s1a r5, r6, r7 }
	{ jalr r15 ; shlih r5, r6, 5 }
	{ jalr r15 ; slti_u r5, r6, 5 }
	{ jalr r15 ; tblidxb0 r5, r6 }
	{ jalrp r15 ; andi r5, r6, 5 }
	{ jalrp r15 ; maxib_u r5, r6, 5 }
	{ jalrp r15 ; mulhhsa_uu r5, r6, r7 }
	{ jalrp r15 ; mzb r5, r6, r7 }
	{ jalrp r15 ; sadh r5, r6, r7 }
	{ jalrp r15 ; slt r5, r6, r7 }
	{ jalrp r15 ; sneh r5, r6, r7 }
	{ jr r15 ; addb r5, r6, r7 }
	{ jr r15 ; crc32_32 r5, r6, r7 }
	{ jr r15 ; mnz r5, r6, r7 }
	{ jr r15 ; mulhla_us r5, r6, r7 }
	{ jr r15 ; packhb r5, r6, r7 }
	{ jr r15 ; seqih r5, r6, 5 }
	{ jr r15 ; slteb_u r5, r6, r7 }
	{ jr r15 ; sub r5, r6, r7 }
	{ jrp r15 ; addli r5, r6, 0x1234 }
	{ jrp r15 ; inthb r5, r6, r7 }
	{ jrp r15 ; mulhh_ss r5, r6, r7 }
	{ jrp r15 ; mullla_su r5, r6, r7 }
	{ jrp r15 ; s2a r5, r6, r7 }
	{ jrp r15 ; shr r5, r6, r7 }
	{ jrp r15 ; sltib r5, r6, 5 }
	{ jrp r15 ; tblidxb1 r5, r6 }
	{ lb r15, r16 ; auli r5, r6, 0x1234 }
	{ lb r15, r16 ; maxih r5, r6, 5 }
	{ lb r15, r16 ; mulhl_ss r5, r6, r7 }
	{ lb r15, r16 ; mzh r5, r6, r7 }
	{ lb r15, r16 ; sadh_u r5, r6, r7 }
	{ lb r15, r16 ; slt_u r5, r6, r7 }
	{ lb r15, r16 ; sra r5, r6, r7 }
	{ lb r25, r26 ; add r15, r16, r17 ; and r5, r6, r7 }
	{ lb r25, r26 ; add r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lb r25, r26 ; add r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lb r25, r26 ; add r5, r6, r7 ; ill }
	{ lb r25, r26 ; add r5, r6, r7 ; shri r15, r16, 5 }
	{ lb r25, r26 ; addi r15, r16, 5 ; ctz r5, r6 }
	{ lb r25, r26 ; addi r15, r16, 5 ; or r5, r6, r7 }
	{ lb r25, r26 ; addi r15, r16, 5 ; sne r5, r6, r7 }
	{ lb r25, r26 ; addi r5, r6, 5 ; mz r15, r16, r17 }
	{ lb r25, r26 ; addi r5, r6, 5 ; slti r15, r16, 5 }
	{ lb r25, r26 ; and r15, r16, r17 ; movei r5, 5 }
	{ lb r25, r26 ; and r15, r16, r17 ; s1a r5, r6, r7 }
	{ lb r25, r26 ; and r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lb r25, r26 ; and r5, r6, r7 ; rl r15, r16, r17 }
	{ lb r25, r26 ; and r5, r6, r7 ; sub r15, r16, r17 }
	{ lb r25, r26 ; andi r15, r16, 5 ; mulhlsa_uu r5, r6, r7 }
	{ lb r25, r26 ; andi r15, r16, 5 ; shl r5, r6, r7 }
	{ lb r25, r26 ; andi r5, r6, 5 ; add r15, r16, r17 }
	{ lb r25, r26 ; andi r5, r6, 5 ; seq r15, r16, r17 }
	{ lb r25, r26 ; bitx r5, r6 ; and r15, r16, r17 }
	{ lb r25, r26 ; bitx r5, r6 ; shl r15, r16, r17 }
	{ lb r25, r26 ; bytex r5, r6 ; fnop }
	{ lb r25, r26 ; bytex r5, r6 ; shr r15, r16, r17 }
	{ lb r25, r26 ; clz r5, r6 ; info 19 }
	{ lb r25, r26 ; clz r5, r6 ; slt r15, r16, r17 }
	{ lb r25, r26 ; ctz r5, r6 ; move r15, r16 }
	{ lb r25, r26 ; ctz r5, r6 ; slte r15, r16, r17 }
	{ lb r25, r26 ; fnop ; clz r5, r6 }
	{ lb r25, r26 ; fnop ; mvnz r5, r6, r7 }
	{ lb r25, r26 ; fnop ; s3a r15, r16, r17 }
	{ lb r25, r26 ; fnop ; slte_u r15, r16, r17 }
	{ lb r25, r26 ; fnop }
	{ lb r25, r26 ; ill ; mulll_uu r5, r6, r7 }
	{ lb r25, r26 ; ill ; shr r5, r6, r7 }
	{ lb r25, r26 ; info 19 ; addi r15, r16, 5 }
	{ lb r25, r26 ; info 19 ; mulhh_uu r5, r6, r7 }
	{ lb r25, r26 ; info 19 ; rl r15, r16, r17 }
	{ lb r25, r26 ; info 19 ; shri r15, r16, 5 }
	{ lb r25, r26 ; info 19 ; sub r15, r16, r17 }
	{ lb r25, r26 ; mnz r15, r16, r17 ; move r5, r6 }
	{ lb r25, r26 ; mnz r15, r16, r17 ; rli r5, r6, 5 }
	{ lb r25, r26 ; mnz r15, r16, r17 ; tblidxb0 r5, r6 }
	{ lb r25, r26 ; mnz r5, r6, r7 ; ori r15, r16, 5 }
	{ lb r25, r26 ; mnz r5, r6, r7 ; srai r15, r16, 5 }
	{ lb r25, r26 ; move r15, r16 ; mulhha_uu r5, r6, r7 }
	{ lb r25, r26 ; move r15, r16 ; seqi r5, r6, 5 }
	{ lb r25, r26 ; move r15, r16 }
	{ lb r25, r26 ; move r5, r6 ; s3a r15, r16, r17 }
	{ lb r25, r26 ; movei r15, 5 ; addi r5, r6, 5 }
	{ lb r25, r26 ; movei r15, 5 ; mullla_uu r5, r6, r7 }
	{ lb r25, r26 ; movei r15, 5 ; slt r5, r6, r7 }
	{ lb r25, r26 ; movei r5, 5 ; fnop }
	{ lb r25, r26 ; movei r5, 5 ; shr r15, r16, r17 }
	{ lb r25, r26 ; mulhh_ss r5, r6, r7 ; info 19 }
	{ lb r25, r26 ; mulhh_ss r5, r6, r7 ; slt r15, r16, r17 }
	{ lb r25, r26 ; mulhh_uu r5, r6, r7 ; move r15, r16 }
	{ lb r25, r26 ; mulhh_uu r5, r6, r7 ; slte r15, r16, r17 }
	{ lb r25, r26 ; mulhha_ss r5, r6, r7 ; mz r15, r16, r17 }
	{ lb r25, r26 ; mulhha_ss r5, r6, r7 ; slti r15, r16, 5 }
	{ lb r25, r26 ; mulhha_uu r5, r6, r7 ; nor r15, r16, r17 }
	{ lb r25, r26 ; mulhha_uu r5, r6, r7 ; sne r15, r16, r17 }
	{ lb r25, r26 ; mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 }
	{ lb r25, r26 ; mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 }
	{ lb r25, r26 ; mulll_ss r5, r6, r7 ; rli r15, r16, 5 }
	{ lb r25, r26 ; mulll_ss r5, r6, r7 ; xor r15, r16, r17 }
	{ lb r25, r26 ; mulll_uu r5, r6, r7 ; s2a r15, r16, r17 }
	{ lb r25, r26 ; mullla_ss r5, r6, r7 ; add r15, r16, r17 }
	{ lb r25, r26 ; mullla_ss r5, r6, r7 ; seq r15, r16, r17 }
	{ lb r25, r26 ; mullla_uu r5, r6, r7 ; and r15, r16, r17 }
	{ lb r25, r26 ; mullla_uu r5, r6, r7 ; shl r15, r16, r17 }
	{ lb r25, r26 ; mvnz r5, r6, r7 ; fnop }
	{ lb r25, r26 ; mvnz r5, r6, r7 ; shr r15, r16, r17 }
	{ lb r25, r26 ; mvz r5, r6, r7 ; info 19 }
	{ lb r25, r26 ; mvz r5, r6, r7 ; slt r15, r16, r17 }
	{ lb r25, r26 ; mz r15, r16, r17 ; fnop }
	{ lb r25, r26 ; mz r15, r16, r17 ; ori r5, r6, 5 }
	{ lb r25, r26 ; mz r15, r16, r17 ; sra r5, r6, r7 }
	{ lb r25, r26 ; mz r5, r6, r7 ; nop }
	{ lb r25, r26 ; mz r5, r6, r7 ; slti_u r15, r16, 5 }
	{ lb r25, r26 ; nop ; ill }
	{ lb r25, r26 ; nop ; mz r5, r6, r7 }
	{ lb r25, r26 ; nop ; seq r5, r6, r7 }
	{ lb r25, r26 ; nop ; slti r5, r6, 5 }
	{ lb r25, r26 ; nor r15, r16, r17 ; and r5, r6, r7 }
	{ lb r25, r26 ; nor r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lb r25, r26 ; nor r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lb r25, r26 ; nor r5, r6, r7 ; ill }
	{ lb r25, r26 ; nor r5, r6, r7 ; shri r15, r16, 5 }
	{ lb r25, r26 ; or r15, r16, r17 ; ctz r5, r6 }
	{ lb r25, r26 ; or r15, r16, r17 ; or r5, r6, r7 }
	{ lb r25, r26 ; or r15, r16, r17 ; sne r5, r6, r7 }
	{ lb r25, r26 ; or r5, r6, r7 ; mz r15, r16, r17 }
	{ lb r25, r26 ; or r5, r6, r7 ; slti r15, r16, 5 }
	{ lb r25, r26 ; ori r15, r16, 5 ; movei r5, 5 }
	{ lb r25, r26 ; ori r15, r16, 5 ; s1a r5, r6, r7 }
	{ lb r25, r26 ; ori r15, r16, 5 ; tblidxb1 r5, r6 }
	{ lb r25, r26 ; ori r5, r6, 5 ; rl r15, r16, r17 }
	{ lb r25, r26 ; ori r5, r6, 5 ; sub r15, r16, r17 }
	{ lb r25, r26 ; pcnt r5, r6 ; s1a r15, r16, r17 }
	{ lb r25, r26 ; pcnt r5, r6 }
	{ lb r25, r26 ; rl r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ lb r25, r26 ; rl r15, r16, r17 ; shr r5, r6, r7 }
	{ lb r25, r26 ; rl r5, r6, r7 ; and r15, r16, r17 }
	{ lb r25, r26 ; rl r5, r6, r7 ; shl r15, r16, r17 }
	{ lb r25, r26 ; rli r15, r16, 5 ; bitx r5, r6 }
	{ lb r25, r26 ; rli r15, r16, 5 ; mz r5, r6, r7 }
	{ lb r25, r26 ; rli r15, r16, 5 ; slte_u r5, r6, r7 }
	{ lb r25, r26 ; rli r5, r6, 5 ; mnz r15, r16, r17 }
	{ lb r25, r26 ; rli r5, r6, 5 ; slt_u r15, r16, r17 }
	{ lb r25, r26 ; s1a r15, r16, r17 ; info 19 }
	{ lb r25, r26 ; s1a r15, r16, r17 ; pcnt r5, r6 }
	{ lb r25, r26 ; s1a r15, r16, r17 ; srai r5, r6, 5 }
	{ lb r25, r26 ; s1a r5, r6, r7 ; nor r15, r16, r17 }
	{ lb r25, r26 ; s1a r5, r6, r7 ; sne r15, r16, r17 }
	{ lb r25, r26 ; s2a r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lb r25, r26 ; s2a r15, r16, r17 ; s3a r5, r6, r7 }
	{ lb r25, r26 ; s2a r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lb r25, r26 ; s2a r5, r6, r7 ; s1a r15, r16, r17 }
	{ lb r25, r26 ; s2a r5, r6, r7 }
	{ lb r25, r26 ; s3a r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ lb r25, r26 ; s3a r15, r16, r17 ; shr r5, r6, r7 }
	{ lb r25, r26 ; s3a r5, r6, r7 ; and r15, r16, r17 }
	{ lb r25, r26 ; s3a r5, r6, r7 ; shl r15, r16, r17 }
	{ lb r25, r26 ; seq r15, r16, r17 ; bitx r5, r6 }
	{ lb r25, r26 ; seq r15, r16, r17 ; mz r5, r6, r7 }
	{ lb r25, r26 ; seq r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lb r25, r26 ; seq r5, r6, r7 ; mnz r15, r16, r17 }
	{ lb r25, r26 ; seq r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lb r25, r26 ; seqi r15, r16, 5 ; info 19 }
	{ lb r25, r26 ; seqi r15, r16, 5 ; pcnt r5, r6 }
	{ lb r25, r26 ; seqi r15, r16, 5 ; srai r5, r6, 5 }
	{ lb r25, r26 ; seqi r5, r6, 5 ; nor r15, r16, r17 }
	{ lb r25, r26 ; seqi r5, r6, 5 ; sne r15, r16, r17 }
	{ lb r25, r26 ; shl r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lb r25, r26 ; shl r15, r16, r17 ; s3a r5, r6, r7 }
	{ lb r25, r26 ; shl r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lb r25, r26 ; shl r5, r6, r7 ; s1a r15, r16, r17 }
	{ lb r25, r26 ; shl r5, r6, r7 }
	{ lb r25, r26 ; shli r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ lb r25, r26 ; shli r15, r16, 5 ; shr r5, r6, r7 }
	{ lb r25, r26 ; shli r5, r6, 5 ; and r15, r16, r17 }
	{ lb r25, r26 ; shli r5, r6, 5 ; shl r15, r16, r17 }
	{ lb r25, r26 ; shr r15, r16, r17 ; bitx r5, r6 }
	{ lb r25, r26 ; shr r15, r16, r17 ; mz r5, r6, r7 }
	{ lb r25, r26 ; shr r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lb r25, r26 ; shr r5, r6, r7 ; mnz r15, r16, r17 }
	{ lb r25, r26 ; shr r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lb r25, r26 ; shri r15, r16, 5 ; info 19 }
	{ lb r25, r26 ; shri r15, r16, 5 ; pcnt r5, r6 }
	{ lb r25, r26 ; shri r15, r16, 5 ; srai r5, r6, 5 }
	{ lb r25, r26 ; shri r5, r6, 5 ; nor r15, r16, r17 }
	{ lb r25, r26 ; shri r5, r6, 5 ; sne r15, r16, r17 }
	{ lb r25, r26 ; slt r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lb r25, r26 ; slt r15, r16, r17 ; s3a r5, r6, r7 }
	{ lb r25, r26 ; slt r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lb r25, r26 ; slt r5, r6, r7 ; s1a r15, r16, r17 }
	{ lb r25, r26 ; slt r5, r6, r7 }
	{ lb r25, r26 ; slt_u r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ lb r25, r26 ; slt_u r15, r16, r17 ; shr r5, r6, r7 }
	{ lb r25, r26 ; slt_u r5, r6, r7 ; and r15, r16, r17 }
	{ lb r25, r26 ; slt_u r5, r6, r7 ; shl r15, r16, r17 }
	{ lb r25, r26 ; slte r15, r16, r17 ; bitx r5, r6 }
	{ lb r25, r26 ; slte r15, r16, r17 ; mz r5, r6, r7 }
	{ lb r25, r26 ; slte r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lb r25, r26 ; slte r5, r6, r7 ; mnz r15, r16, r17 }
	{ lb r25, r26 ; slte r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lb r25, r26 ; slte_u r15, r16, r17 ; info 19 }
	{ lb r25, r26 ; slte_u r15, r16, r17 ; pcnt r5, r6 }
	{ lb r25, r26 ; slte_u r15, r16, r17 ; srai r5, r6, 5 }
	{ lb r25, r26 ; slte_u r5, r6, r7 ; nor r15, r16, r17 }
	{ lb r25, r26 ; slte_u r5, r6, r7 ; sne r15, r16, r17 }
	{ lb r25, r26 ; slti r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ lb r25, r26 ; slti r15, r16, 5 ; s3a r5, r6, r7 }
	{ lb r25, r26 ; slti r15, r16, 5 ; tblidxb3 r5, r6 }
	{ lb r25, r26 ; slti r5, r6, 5 ; s1a r15, r16, r17 }
	{ lb r25, r26 ; slti r5, r6, 5 }
	{ lb r25, r26 ; slti_u r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ lb r25, r26 ; slti_u r15, r16, 5 ; shr r5, r6, r7 }
	{ lb r25, r26 ; slti_u r5, r6, 5 ; and r15, r16, r17 }
	{ lb r25, r26 ; slti_u r5, r6, 5 ; shl r15, r16, r17 }
	{ lb r25, r26 ; sne r15, r16, r17 ; bitx r5, r6 }
	{ lb r25, r26 ; sne r15, r16, r17 ; mz r5, r6, r7 }
	{ lb r25, r26 ; sne r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lb r25, r26 ; sne r5, r6, r7 ; mnz r15, r16, r17 }
	{ lb r25, r26 ; sne r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lb r25, r26 ; sra r15, r16, r17 ; info 19 }
	{ lb r25, r26 ; sra r15, r16, r17 ; pcnt r5, r6 }
	{ lb r25, r26 ; sra r15, r16, r17 ; srai r5, r6, 5 }
	{ lb r25, r26 ; sra r5, r6, r7 ; nor r15, r16, r17 }
	{ lb r25, r26 ; sra r5, r6, r7 ; sne r15, r16, r17 }
	{ lb r25, r26 ; srai r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ lb r25, r26 ; srai r15, r16, 5 ; s3a r5, r6, r7 }
	{ lb r25, r26 ; srai r15, r16, 5 ; tblidxb3 r5, r6 }
	{ lb r25, r26 ; srai r5, r6, 5 ; s1a r15, r16, r17 }
	{ lb r25, r26 ; srai r5, r6, 5 }
	{ lb r25, r26 ; sub r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ lb r25, r26 ; sub r15, r16, r17 ; shr r5, r6, r7 }
	{ lb r25, r26 ; sub r5, r6, r7 ; and r15, r16, r17 }
	{ lb r25, r26 ; sub r5, r6, r7 ; shl r15, r16, r17 }
	{ lb r25, r26 ; tblidxb0 r5, r6 ; fnop }
	{ lb r25, r26 ; tblidxb0 r5, r6 ; shr r15, r16, r17 }
	{ lb r25, r26 ; tblidxb1 r5, r6 ; info 19 }
	{ lb r25, r26 ; tblidxb1 r5, r6 ; slt r15, r16, r17 }
	{ lb r25, r26 ; tblidxb2 r5, r6 ; move r15, r16 }
	{ lb r25, r26 ; tblidxb2 r5, r6 ; slte r15, r16, r17 }
	{ lb r25, r26 ; tblidxb3 r5, r6 ; mz r15, r16, r17 }
	{ lb r25, r26 ; tblidxb3 r5, r6 ; slti r15, r16, 5 }
	{ lb r25, r26 ; xor r15, r16, r17 ; movei r5, 5 }
	{ lb r25, r26 ; xor r15, r16, r17 ; s1a r5, r6, r7 }
	{ lb r25, r26 ; xor r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lb r25, r26 ; xor r5, r6, r7 ; rl r15, r16, r17 }
	{ lb r25, r26 ; xor r5, r6, r7 ; sub r15, r16, r17 }
	{ lb_u r15, r16 ; avgh r5, r6, r7 }
	{ lb_u r15, r16 ; minh r5, r6, r7 }
	{ lb_u r15, r16 ; mulhl_us r5, r6, r7 }
	{ lb_u r15, r16 ; nor r5, r6, r7 }
	{ lb_u r15, r16 ; seqb r5, r6, r7 }
	{ lb_u r15, r16 ; sltb_u r5, r6, r7 }
	{ lb_u r15, r16 ; srah r5, r6, r7 }
	{ lb_u r25, r26 ; add r15, r16, r17 ; bitx r5, r6 }
	{ lb_u r25, r26 ; add r15, r16, r17 ; mz r5, r6, r7 }
	{ lb_u r25, r26 ; add r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lb_u r25, r26 ; add r5, r6, r7 ; mnz r15, r16, r17 }
	{ lb_u r25, r26 ; add r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lb_u r25, r26 ; addi r15, r16, 5 ; info 19 }
	{ lb_u r25, r26 ; addi r15, r16, 5 ; pcnt r5, r6 }
	{ lb_u r25, r26 ; addi r15, r16, 5 ; srai r5, r6, 5 }
	{ lb_u r25, r26 ; addi r5, r6, 5 ; nor r15, r16, r17 }
	{ lb_u r25, r26 ; addi r5, r6, 5 ; sne r15, r16, r17 }
	{ lb_u r25, r26 ; and r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lb_u r25, r26 ; and r15, r16, r17 ; s3a r5, r6, r7 }
	{ lb_u r25, r26 ; and r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lb_u r25, r26 ; and r5, r6, r7 ; s1a r15, r16, r17 }
	{ lb_u r25, r26 ; and r5, r6, r7 }
	{ lb_u r25, r26 ; andi r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ lb_u r25, r26 ; andi r15, r16, 5 ; shr r5, r6, r7 }
	{ lb_u r25, r26 ; andi r5, r6, 5 ; and r15, r16, r17 }
	{ lb_u r25, r26 ; andi r5, r6, 5 ; shl r15, r16, r17 }
	{ lb_u r25, r26 ; bitx r5, r6 ; fnop }
	{ lb_u r25, r26 ; bitx r5, r6 ; shr r15, r16, r17 }
	{ lb_u r25, r26 ; bytex r5, r6 ; info 19 }
	{ lb_u r25, r26 ; bytex r5, r6 ; slt r15, r16, r17 }
	{ lb_u r25, r26 ; clz r5, r6 ; move r15, r16 }
	{ lb_u r25, r26 ; clz r5, r6 ; slte r15, r16, r17 }
	{ lb_u r25, r26 ; ctz r5, r6 ; mz r15, r16, r17 }
	{ lb_u r25, r26 ; ctz r5, r6 ; slti r15, r16, 5 }
	{ lb_u r25, r26 ; fnop ; fnop }
	{ lb_u r25, r26 ; fnop ; mz r15, r16, r17 }
	{ lb_u r25, r26 ; fnop ; seq r15, r16, r17 }
	{ lb_u r25, r26 ; fnop ; slti r15, r16, 5 }
	{ lb_u r25, r26 ; ill ; addi r5, r6, 5 }
	{ lb_u r25, r26 ; ill ; mullla_uu r5, r6, r7 }
	{ lb_u r25, r26 ; ill ; slt r5, r6, r7 }
	{ lb_u r25, r26 ; info 19 ; and r15, r16, r17 }
	{ lb_u r25, r26 ; info 19 ; mulhha_uu r5, r6, r7 }
	{ lb_u r25, r26 ; info 19 ; rli r15, r16, 5 }
	{ lb_u r25, r26 ; info 19 ; slt r15, r16, r17 }
	{ lb_u r25, r26 ; info 19 ; tblidxb0 r5, r6 }
	{ lb_u r25, r26 ; mnz r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ lb_u r25, r26 ; mnz r15, r16, r17 ; s2a r5, r6, r7 }
	{ lb_u r25, r26 ; mnz r15, r16, r17 ; tblidxb2 r5, r6 }
	{ lb_u r25, r26 ; mnz r5, r6, r7 ; rli r15, r16, 5 }
	{ lb_u r25, r26 ; mnz r5, r6, r7 ; xor r15, r16, r17 }
	{ lb_u r25, r26 ; move r15, r16 ; mulll_ss r5, r6, r7 }
	{ lb_u r25, r26 ; move r15, r16 ; shli r5, r6, 5 }
	{ lb_u r25, r26 ; move r5, r6 ; addi r15, r16, 5 }
	{ lb_u r25, r26 ; move r5, r6 ; seqi r15, r16, 5 }
	{ lb_u r25, r26 ; movei r15, 5 ; andi r5, r6, 5 }
	{ lb_u r25, r26 ; movei r15, 5 ; mvz r5, r6, r7 }
	{ lb_u r25, r26 ; movei r15, 5 ; slte r5, r6, r7 }
	{ lb_u r25, r26 ; movei r5, 5 ; info 19 }
	{ lb_u r25, r26 ; movei r5, 5 ; slt r15, r16, r17 }
	{ lb_u r25, r26 ; mulhh_ss r5, r6, r7 ; move r15, r16 }
	{ lb_u r25, r26 ; mulhh_ss r5, r6, r7 ; slte r15, r16, r17 }
	{ lb_u r25, r26 ; mulhh_uu r5, r6, r7 ; mz r15, r16, r17 }
	{ lb_u r25, r26 ; mulhh_uu r5, r6, r7 ; slti r15, r16, 5 }
	{ lb_u r25, r26 ; mulhha_ss r5, r6, r7 ; nor r15, r16, r17 }
	{ lb_u r25, r26 ; mulhha_ss r5, r6, r7 ; sne r15, r16, r17 }
	{ lb_u r25, r26 ; mulhha_uu r5, r6, r7 ; ori r15, r16, 5 }
	{ lb_u r25, r26 ; mulhha_uu r5, r6, r7 ; srai r15, r16, 5 }
	{ lb_u r25, r26 ; mulhlsa_uu r5, r6, r7 ; rli r15, r16, 5 }
	{ lb_u r25, r26 ; mulhlsa_uu r5, r6, r7 ; xor r15, r16, r17 }
	{ lb_u r25, r26 ; mulll_ss r5, r6, r7 ; s2a r15, r16, r17 }
	{ lb_u r25, r26 ; mulll_uu r5, r6, r7 ; add r15, r16, r17 }
	{ lb_u r25, r26 ; mulll_uu r5, r6, r7 ; seq r15, r16, r17 }
	{ lb_u r25, r26 ; mullla_ss r5, r6, r7 ; and r15, r16, r17 }
	{ lb_u r25, r26 ; mullla_ss r5, r6, r7 ; shl r15, r16, r17 }
	{ lb_u r25, r26 ; mullla_uu r5, r6, r7 ; fnop }
	{ lb_u r25, r26 ; mullla_uu r5, r6, r7 ; shr r15, r16, r17 }
	{ lb_u r25, r26 ; mvnz r5, r6, r7 ; info 19 }
	{ lb_u r25, r26 ; mvnz r5, r6, r7 ; slt r15, r16, r17 }
	{ lb_u r25, r26 ; mvz r5, r6, r7 ; move r15, r16 }
	{ lb_u r25, r26 ; mvz r5, r6, r7 ; slte r15, r16, r17 }
	{ lb_u r25, r26 ; mz r15, r16, r17 ; mnz r5, r6, r7 }
	{ lb_u r25, r26 ; mz r15, r16, r17 ; rl r5, r6, r7 }
	{ lb_u r25, r26 ; mz r15, r16, r17 ; sub r5, r6, r7 }
	{ lb_u r25, r26 ; mz r5, r6, r7 ; or r15, r16, r17 }
	{ lb_u r25, r26 ; mz r5, r6, r7 ; sra r15, r16, r17 }
	{ lb_u r25, r26 ; nop ; mnz r15, r16, r17 }
	{ lb_u r25, r26 ; nop ; nor r15, r16, r17 }
	{ lb_u r25, r26 ; nop ; seqi r5, r6, 5 }
	{ lb_u r25, r26 ; nop ; slti_u r5, r6, 5 }
	{ lb_u r25, r26 ; nor r15, r16, r17 ; bitx r5, r6 }
	{ lb_u r25, r26 ; nor r15, r16, r17 ; mz r5, r6, r7 }
	{ lb_u r25, r26 ; nor r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lb_u r25, r26 ; nor r5, r6, r7 ; mnz r15, r16, r17 }
	{ lb_u r25, r26 ; nor r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lb_u r25, r26 ; or r15, r16, r17 ; info 19 }
	{ lb_u r25, r26 ; or r15, r16, r17 ; pcnt r5, r6 }
	{ lb_u r25, r26 ; or r15, r16, r17 ; srai r5, r6, 5 }
	{ lb_u r25, r26 ; or r5, r6, r7 ; nor r15, r16, r17 }
	{ lb_u r25, r26 ; or r5, r6, r7 ; sne r15, r16, r17 }
	{ lb_u r25, r26 ; ori r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ lb_u r25, r26 ; ori r15, r16, 5 ; s3a r5, r6, r7 }
	{ lb_u r25, r26 ; ori r15, r16, 5 ; tblidxb3 r5, r6 }
	{ lb_u r25, r26 ; ori r5, r6, 5 ; s1a r15, r16, r17 }
	{ lb_u r25, r26 ; ori r5, r6, 5 }
	{ lb_u r25, r26 ; pcnt r5, r6 ; s3a r15, r16, r17 }
	{ lb_u r25, r26 ; rl r15, r16, r17 ; addi r5, r6, 5 }
	{ lb_u r25, r26 ; rl r15, r16, r17 ; mullla_uu r5, r6, r7 }
	{ lb_u r25, r26 ; rl r15, r16, r17 ; slt r5, r6, r7 }
	{ lb_u r25, r26 ; rl r5, r6, r7 ; fnop }
	{ lb_u r25, r26 ; rl r5, r6, r7 ; shr r15, r16, r17 }
	{ lb_u r25, r26 ; rli r15, r16, 5 ; clz r5, r6 }
	{ lb_u r25, r26 ; rli r15, r16, 5 ; nor r5, r6, r7 }
	{ lb_u r25, r26 ; rli r15, r16, 5 ; slti_u r5, r6, 5 }
	{ lb_u r25, r26 ; rli r5, r6, 5 ; movei r15, 5 }
	{ lb_u r25, r26 ; rli r5, r6, 5 ; slte_u r15, r16, r17 }
	{ lb_u r25, r26 ; s1a r15, r16, r17 ; move r5, r6 }
	{ lb_u r25, r26 ; s1a r15, r16, r17 ; rli r5, r6, 5 }
	{ lb_u r25, r26 ; s1a r15, r16, r17 ; tblidxb0 r5, r6 }
	{ lb_u r25, r26 ; s1a r5, r6, r7 ; ori r15, r16, 5 }
	{ lb_u r25, r26 ; s1a r5, r6, r7 ; srai r15, r16, 5 }
	{ lb_u r25, r26 ; s2a r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ lb_u r25, r26 ; s2a r15, r16, r17 ; seqi r5, r6, 5 }
	{ lb_u r25, r26 ; s2a r15, r16, r17 }
	{ lb_u r25, r26 ; s2a r5, r6, r7 ; s3a r15, r16, r17 }
	{ lb_u r25, r26 ; s3a r15, r16, r17 ; addi r5, r6, 5 }
	{ lb_u r25, r26 ; s3a r15, r16, r17 ; mullla_uu r5, r6, r7 }
	{ lb_u r25, r26 ; s3a r15, r16, r17 ; slt r5, r6, r7 }
	{ lb_u r25, r26 ; s3a r5, r6, r7 ; fnop }
	{ lb_u r25, r26 ; s3a r5, r6, r7 ; shr r15, r16, r17 }
	{ lb_u r25, r26 ; seq r15, r16, r17 ; clz r5, r6 }
	{ lb_u r25, r26 ; seq r15, r16, r17 ; nor r5, r6, r7 }
	{ lb_u r25, r26 ; seq r15, r16, r17 ; slti_u r5, r6, 5 }
	{ lb_u r25, r26 ; seq r5, r6, r7 ; movei r15, 5 }
	{ lb_u r25, r26 ; seq r5, r6, r7 ; slte_u r15, r16, r17 }
	{ lb_u r25, r26 ; seqi r15, r16, 5 ; move r5, r6 }
	{ lb_u r25, r26 ; seqi r15, r16, 5 ; rli r5, r6, 5 }
	{ lb_u r25, r26 ; seqi r15, r16, 5 ; tblidxb0 r5, r6 }
	{ lb_u r25, r26 ; seqi r5, r6, 5 ; ori r15, r16, 5 }
	{ lb_u r25, r26 ; seqi r5, r6, 5 ; srai r15, r16, 5 }
	{ lb_u r25, r26 ; shl r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ lb_u r25, r26 ; shl r15, r16, r17 ; seqi r5, r6, 5 }
	{ lb_u r25, r26 ; shl r15, r16, r17 }
	{ lb_u r25, r26 ; shl r5, r6, r7 ; s3a r15, r16, r17 }
	{ lb_u r25, r26 ; shli r15, r16, 5 ; addi r5, r6, 5 }
	{ lb_u r25, r26 ; shli r15, r16, 5 ; mullla_uu r5, r6, r7 }
	{ lb_u r25, r26 ; shli r15, r16, 5 ; slt r5, r6, r7 }
	{ lb_u r25, r26 ; shli r5, r6, 5 ; fnop }
	{ lb_u r25, r26 ; shli r5, r6, 5 ; shr r15, r16, r17 }
	{ lb_u r25, r26 ; shr r15, r16, r17 ; clz r5, r6 }
	{ lb_u r25, r26 ; shr r15, r16, r17 ; nor r5, r6, r7 }
	{ lb_u r25, r26 ; shr r15, r16, r17 ; slti_u r5, r6, 5 }
	{ lb_u r25, r26 ; shr r5, r6, r7 ; movei r15, 5 }
	{ lb_u r25, r26 ; shr r5, r6, r7 ; slte_u r15, r16, r17 }
	{ lb_u r25, r26 ; shri r15, r16, 5 ; move r5, r6 }
	{ lb_u r25, r26 ; shri r15, r16, 5 ; rli r5, r6, 5 }
	{ lb_u r25, r26 ; shri r15, r16, 5 ; tblidxb0 r5, r6 }
	{ lb_u r25, r26 ; shri r5, r6, 5 ; ori r15, r16, 5 }
	{ lb_u r25, r26 ; shri r5, r6, 5 ; srai r15, r16, 5 }
	{ lb_u r25, r26 ; slt r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ lb_u r25, r26 ; slt r15, r16, r17 ; seqi r5, r6, 5 }
	{ lb_u r25, r26 ; slt r15, r16, r17 }
	{ lb_u r25, r26 ; slt r5, r6, r7 ; s3a r15, r16, r17 }
	{ lb_u r25, r26 ; slt_u r15, r16, r17 ; addi r5, r6, 5 }
	{ lb_u r25, r26 ; slt_u r15, r16, r17 ; mullla_uu r5, r6, r7 }
	{ lb_u r25, r26 ; slt_u r15, r16, r17 ; slt r5, r6, r7 }
	{ lb_u r25, r26 ; slt_u r5, r6, r7 ; fnop }
	{ lb_u r25, r26 ; slt_u r5, r6, r7 ; shr r15, r16, r17 }
	{ lb_u r25, r26 ; slte r15, r16, r17 ; clz r5, r6 }
	{ lb_u r25, r26 ; slte r15, r16, r17 ; nor r5, r6, r7 }
	{ lb_u r25, r26 ; slte r15, r16, r17 ; slti_u r5, r6, 5 }
	{ lb_u r25, r26 ; slte r5, r6, r7 ; movei r15, 5 }
	{ lb_u r25, r26 ; slte r5, r6, r7 ; slte_u r15, r16, r17 }
	{ lb_u r25, r26 ; slte_u r15, r16, r17 ; move r5, r6 }
	{ lb_u r25, r26 ; slte_u r15, r16, r17 ; rli r5, r6, 5 }
	{ lb_u r25, r26 ; slte_u r15, r16, r17 ; tblidxb0 r5, r6 }
	{ lb_u r25, r26 ; slte_u r5, r6, r7 ; ori r15, r16, 5 }
	{ lb_u r25, r26 ; slte_u r5, r6, r7 ; srai r15, r16, 5 }
	{ lb_u r25, r26 ; slti r15, r16, 5 ; mulhha_uu r5, r6, r7 }
	{ lb_u r25, r26 ; slti r15, r16, 5 ; seqi r5, r6, 5 }
	{ lb_u r25, r26 ; slti r15, r16, 5 }
	{ lb_u r25, r26 ; slti r5, r6, 5 ; s3a r15, r16, r17 }
	{ lb_u r25, r26 ; slti_u r15, r16, 5 ; addi r5, r6, 5 }
	{ lb_u r25, r26 ; slti_u r15, r16, 5 ; mullla_uu r5, r6, r7 }
	{ lb_u r25, r26 ; slti_u r15, r16, 5 ; slt r5, r6, r7 }
	{ lb_u r25, r26 ; slti_u r5, r6, 5 ; fnop }
	{ lb_u r25, r26 ; slti_u r5, r6, 5 ; shr r15, r16, r17 }
	{ lb_u r25, r26 ; sne r15, r16, r17 ; clz r5, r6 }
	{ lb_u r25, r26 ; sne r15, r16, r17 ; nor r5, r6, r7 }
	{ lb_u r25, r26 ; sne r15, r16, r17 ; slti_u r5, r6, 5 }
	{ lb_u r25, r26 ; sne r5, r6, r7 ; movei r15, 5 }
	{ lb_u r25, r26 ; sne r5, r6, r7 ; slte_u r15, r16, r17 }
	{ lb_u r25, r26 ; sra r15, r16, r17 ; move r5, r6 }
	{ lb_u r25, r26 ; sra r15, r16, r17 ; rli r5, r6, 5 }
	{ lb_u r25, r26 ; sra r15, r16, r17 ; tblidxb0 r5, r6 }
	{ lb_u r25, r26 ; sra r5, r6, r7 ; ori r15, r16, 5 }
	{ lb_u r25, r26 ; sra r5, r6, r7 ; srai r15, r16, 5 }
	{ lb_u r25, r26 ; srai r15, r16, 5 ; mulhha_uu r5, r6, r7 }
	{ lb_u r25, r26 ; srai r15, r16, 5 ; seqi r5, r6, 5 }
	{ lb_u r25, r26 ; srai r15, r16, 5 }
	{ lb_u r25, r26 ; srai r5, r6, 5 ; s3a r15, r16, r17 }
	{ lb_u r25, r26 ; sub r15, r16, r17 ; addi r5, r6, 5 }
	{ lb_u r25, r26 ; sub r15, r16, r17 ; mullla_uu r5, r6, r7 }
	{ lb_u r25, r26 ; sub r15, r16, r17 ; slt r5, r6, r7 }
	{ lb_u r25, r26 ; sub r5, r6, r7 ; fnop }
	{ lb_u r25, r26 ; sub r5, r6, r7 ; shr r15, r16, r17 }
	{ lb_u r25, r26 ; tblidxb0 r5, r6 ; info 19 }
	{ lb_u r25, r26 ; tblidxb0 r5, r6 ; slt r15, r16, r17 }
	{ lb_u r25, r26 ; tblidxb1 r5, r6 ; move r15, r16 }
	{ lb_u r25, r26 ; tblidxb1 r5, r6 ; slte r15, r16, r17 }
	{ lb_u r25, r26 ; tblidxb2 r5, r6 ; mz r15, r16, r17 }
	{ lb_u r25, r26 ; tblidxb2 r5, r6 ; slti r15, r16, 5 }
	{ lb_u r25, r26 ; tblidxb3 r5, r6 ; nor r15, r16, r17 }
	{ lb_u r25, r26 ; tblidxb3 r5, r6 ; sne r15, r16, r17 }
	{ lb_u r25, r26 ; xor r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lb_u r25, r26 ; xor r15, r16, r17 ; s3a r5, r6, r7 }
	{ lb_u r25, r26 ; xor r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lb_u r25, r26 ; xor r5, r6, r7 ; s1a r15, r16, r17 }
	{ lb_u r25, r26 ; xor r5, r6, r7 }
	{ lbadd r15, r16, 5 ; bytex r5, r6 }
	{ lbadd r15, r16, 5 ; minih r5, r6, 5 }
	{ lbadd r15, r16, 5 ; mulhla_ss r5, r6, r7 }
	{ lbadd r15, r16, 5 ; ori r5, r6, 5 }
	{ lbadd r15, r16, 5 ; seqi r5, r6, 5 }
	{ lbadd r15, r16, 5 ; slte_u r5, r6, r7 }
	{ lbadd r15, r16, 5 ; sraib r5, r6, 5 }
	{ lbadd_u r15, r16, 5 ; addib r5, r6, 5 }
	{ lbadd_u r15, r16, 5 ; info 19 }
	{ lbadd_u r15, r16, 5 ; moveli r5, 0x1234 }
	{ lbadd_u r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ lbadd_u r15, r16, 5 ; rli r5, r6, 5 }
	{ lbadd_u r15, r16, 5 ; shlib r5, r6, 5 }
	{ lbadd_u r15, r16, 5 ; slti r5, r6, 5 }
	{ lbadd_u r15, r16, 5 ; subs r5, r6, r7 }
	{ lh r15, r16 ; and r5, r6, r7 }
	{ lh r15, r16 ; maxh r5, r6, r7 }
	{ lh r15, r16 ; mulhha_uu r5, r6, r7 }
	{ lh r15, r16 ; mz r5, r6, r7 }
	{ lh r15, r16 ; sadb_u r5, r6, r7 }
	{ lh r15, r16 ; shrih r5, r6, 5 }
	{ lh r15, r16 ; sneb r5, r6, r7 }
	{ lh r25, r26 ; add r15, r16, r17 ; add r5, r6, r7 }
	{ lh r25, r26 ; add r15, r16, r17 ; mullla_ss r5, r6, r7 }
	{ lh r25, r26 ; add r15, r16, r17 ; shri r5, r6, 5 }
	{ lh r25, r26 ; add r5, r6, r7 ; andi r15, r16, 5 }
	{ lh r25, r26 ; add r5, r6, r7 ; shli r15, r16, 5 }
	{ lh r25, r26 ; addi r15, r16, 5 ; bytex r5, r6 }
	{ lh r25, r26 ; addi r15, r16, 5 ; nop }
	{ lh r25, r26 ; addi r15, r16, 5 ; slti r5, r6, 5 }
	{ lh r25, r26 ; addi r5, r6, 5 ; move r15, r16 }
	{ lh r25, r26 ; addi r5, r6, 5 ; slte r15, r16, r17 }
	{ lh r25, r26 ; and r15, r16, r17 ; mnz r5, r6, r7 }
	{ lh r25, r26 ; and r15, r16, r17 ; rl r5, r6, r7 }
	{ lh r25, r26 ; and r15, r16, r17 ; sub r5, r6, r7 }
	{ lh r25, r26 ; and r5, r6, r7 ; or r15, r16, r17 }
	{ lh r25, r26 ; and r5, r6, r7 ; sra r15, r16, r17 }
	{ lh r25, r26 ; andi r15, r16, 5 ; mulhha_ss r5, r6, r7 }
	{ lh r25, r26 ; andi r15, r16, 5 ; seq r5, r6, r7 }
	{ lh r25, r26 ; andi r15, r16, 5 ; xor r5, r6, r7 }
	{ lh r25, r26 ; andi r5, r6, 5 ; s2a r15, r16, r17 }
	{ lh r25, r26 ; bitx r5, r6 ; add r15, r16, r17 }
	{ lh r25, r26 ; bitx r5, r6 ; seq r15, r16, r17 }
	{ lh r25, r26 ; bytex r5, r6 ; and r15, r16, r17 }
	{ lh r25, r26 ; bytex r5, r6 ; shl r15, r16, r17 }
	{ lh r25, r26 ; clz r5, r6 ; fnop }
	{ lh r25, r26 ; clz r5, r6 ; shr r15, r16, r17 }
	{ lh r25, r26 ; ctz r5, r6 ; info 19 }
	{ lh r25, r26 ; ctz r5, r6 ; slt r15, r16, r17 }
	{ lh r25, r26 ; fnop ; bitx r5, r6 }
	{ lh r25, r26 ; fnop ; mullla_ss r5, r6, r7 }
	{ lh r25, r26 ; fnop ; s2a r15, r16, r17 }
	{ lh r25, r26 ; fnop ; slte r15, r16, r17 }
	{ lh r25, r26 ; fnop ; xor r15, r16, r17 }
	{ lh r25, r26 ; ill ; mulhlsa_uu r5, r6, r7 }
	{ lh r25, r26 ; ill ; shl r5, r6, r7 }
	{ lh r25, r26 ; info 19 ; add r15, r16, r17 }
	{ lh r25, r26 ; info 19 ; movei r5, 5 }
	{ lh r25, r26 ; info 19 ; ori r5, r6, 5 }
	{ lh r25, r26 ; info 19 ; shr r15, r16, r17 }
	{ lh r25, r26 ; info 19 ; srai r15, r16, 5 }
	{ lh r25, r26 ; mnz r15, r16, r17 ; info 19 }
	{ lh r25, r26 ; mnz r15, r16, r17 ; pcnt r5, r6 }
	{ lh r25, r26 ; mnz r15, r16, r17 ; srai r5, r6, 5 }
	{ lh r25, r26 ; mnz r5, r6, r7 ; nor r15, r16, r17 }
	{ lh r25, r26 ; mnz r5, r6, r7 ; sne r15, r16, r17 }
	{ lh r25, r26 ; move r15, r16 ; mulhh_uu r5, r6, r7 }
	{ lh r25, r26 ; move r15, r16 ; s3a r5, r6, r7 }
	{ lh r25, r26 ; move r15, r16 ; tblidxb3 r5, r6 }
	{ lh r25, r26 ; move r5, r6 ; s1a r15, r16, r17 }
	{ lh r25, r26 ; move r5, r6 }
	{ lh r25, r26 ; movei r15, 5 ; mulll_uu r5, r6, r7 }
	{ lh r25, r26 ; movei r15, 5 ; shr r5, r6, r7 }
	{ lh r25, r26 ; movei r5, 5 ; and r15, r16, r17 }
	{ lh r25, r26 ; movei r5, 5 ; shl r15, r16, r17 }
	{ lh r25, r26 ; mulhh_ss r5, r6, r7 ; fnop }
	{ lh r25, r26 ; mulhh_ss r5, r6, r7 ; shr r15, r16, r17 }
	{ lh r25, r26 ; mulhh_uu r5, r6, r7 ; info 19 }
	{ lh r25, r26 ; mulhh_uu r5, r6, r7 ; slt r15, r16, r17 }
	{ lh r25, r26 ; mulhha_ss r5, r6, r7 ; move r15, r16 }
	{ lh r25, r26 ; mulhha_ss r5, r6, r7 ; slte r15, r16, r17 }
	{ lh r25, r26 ; mulhha_uu r5, r6, r7 ; mz r15, r16, r17 }
	{ lh r25, r26 ; mulhha_uu r5, r6, r7 ; slti r15, r16, 5 }
	{ lh r25, r26 ; mulhlsa_uu r5, r6, r7 ; nor r15, r16, r17 }
	{ lh r25, r26 ; mulhlsa_uu r5, r6, r7 ; sne r15, r16, r17 }
	{ lh r25, r26 ; mulll_ss r5, r6, r7 ; ori r15, r16, 5 }
	{ lh r25, r26 ; mulll_ss r5, r6, r7 ; srai r15, r16, 5 }
	{ lh r25, r26 ; mulll_uu r5, r6, r7 ; rli r15, r16, 5 }
	{ lh r25, r26 ; mulll_uu r5, r6, r7 ; xor r15, r16, r17 }
	{ lh r25, r26 ; mullla_ss r5, r6, r7 ; s2a r15, r16, r17 }
	{ lh r25, r26 ; mullla_uu r5, r6, r7 ; add r15, r16, r17 }
	{ lh r25, r26 ; mullla_uu r5, r6, r7 ; seq r15, r16, r17 }
	{ lh r25, r26 ; mvnz r5, r6, r7 ; and r15, r16, r17 }
	{ lh r25, r26 ; mvnz r5, r6, r7 ; shl r15, r16, r17 }
	{ lh r25, r26 ; mvz r5, r6, r7 ; fnop }
	{ lh r25, r26 ; mvz r5, r6, r7 ; shr r15, r16, r17 }
	{ lh r25, r26 ; mz r15, r16, r17 ; clz r5, r6 }
	{ lh r25, r26 ; mz r15, r16, r17 ; nor r5, r6, r7 }
	{ lh r25, r26 ; mz r15, r16, r17 ; slti_u r5, r6, 5 }
	{ lh r25, r26 ; mz r5, r6, r7 ; movei r15, 5 }
	{ lh r25, r26 ; mz r5, r6, r7 ; slte_u r15, r16, r17 }
	{ lh r25, r26 ; nop ; ctz r5, r6 }
	{ lh r25, r26 ; nop ; mvz r5, r6, r7 }
	{ lh r25, r26 ; nop ; s3a r5, r6, r7 }
	{ lh r25, r26 ; nop ; slte_u r5, r6, r7 }
	{ lh r25, r26 ; nor r15, r16, r17 ; add r5, r6, r7 }
	{ lh r25, r26 ; nor r15, r16, r17 ; mullla_ss r5, r6, r7 }
	{ lh r25, r26 ; nor r15, r16, r17 ; shri r5, r6, 5 }
	{ lh r25, r26 ; nor r5, r6, r7 ; andi r15, r16, 5 }
	{ lh r25, r26 ; nor r5, r6, r7 ; shli r15, r16, 5 }
	{ lh r25, r26 ; or r15, r16, r17 ; bytex r5, r6 }
	{ lh r25, r26 ; or r15, r16, r17 ; nop }
	{ lh r25, r26 ; or r15, r16, r17 ; slti r5, r6, 5 }
	{ lh r25, r26 ; or r5, r6, r7 ; move r15, r16 }
	{ lh r25, r26 ; or r5, r6, r7 ; slte r15, r16, r17 }
	{ lh r25, r26 ; ori r15, r16, 5 ; mnz r5, r6, r7 }
	{ lh r25, r26 ; ori r15, r16, 5 ; rl r5, r6, r7 }
	{ lh r25, r26 ; ori r15, r16, 5 ; sub r5, r6, r7 }
	{ lh r25, r26 ; ori r5, r6, 5 ; or r15, r16, r17 }
	{ lh r25, r26 ; ori r5, r6, 5 ; sra r15, r16, r17 }
	{ lh r25, r26 ; pcnt r5, r6 ; rl r15, r16, r17 }
	{ lh r25, r26 ; pcnt r5, r6 ; sub r15, r16, r17 }
	{ lh r25, r26 ; rl r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ lh r25, r26 ; rl r15, r16, r17 ; shl r5, r6, r7 }
	{ lh r25, r26 ; rl r5, r6, r7 ; add r15, r16, r17 }
	{ lh r25, r26 ; rl r5, r6, r7 ; seq r15, r16, r17 }
	{ lh r25, r26 ; rli r15, r16, 5 ; and r5, r6, r7 }
	{ lh r25, r26 ; rli r15, r16, 5 ; mvnz r5, r6, r7 }
	{ lh r25, r26 ; rli r15, r16, 5 ; slt_u r5, r6, r7 }
	{ lh r25, r26 ; rli r5, r6, 5 ; ill }
	{ lh r25, r26 ; rli r5, r6, 5 ; shri r15, r16, 5 }
	{ lh r25, r26 ; s1a r15, r16, r17 ; ctz r5, r6 }
	{ lh r25, r26 ; s1a r15, r16, r17 ; or r5, r6, r7 }
	{ lh r25, r26 ; s1a r15, r16, r17 ; sne r5, r6, r7 }
	{ lh r25, r26 ; s1a r5, r6, r7 ; mz r15, r16, r17 }
	{ lh r25, r26 ; s1a r5, r6, r7 ; slti r15, r16, 5 }
	{ lh r25, r26 ; s2a r15, r16, r17 ; movei r5, 5 }
	{ lh r25, r26 ; s2a r15, r16, r17 ; s1a r5, r6, r7 }
	{ lh r25, r26 ; s2a r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lh r25, r26 ; s2a r5, r6, r7 ; rl r15, r16, r17 }
	{ lh r25, r26 ; s2a r5, r6, r7 ; sub r15, r16, r17 }
	{ lh r25, r26 ; s3a r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ lh r25, r26 ; s3a r15, r16, r17 ; shl r5, r6, r7 }
	{ lh r25, r26 ; s3a r5, r6, r7 ; add r15, r16, r17 }
	{ lh r25, r26 ; s3a r5, r6, r7 ; seq r15, r16, r17 }
	{ lh r25, r26 ; seq r15, r16, r17 ; and r5, r6, r7 }
	{ lh r25, r26 ; seq r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lh r25, r26 ; seq r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lh r25, r26 ; seq r5, r6, r7 ; ill }
	{ lh r25, r26 ; seq r5, r6, r7 ; shri r15, r16, 5 }
	{ lh r25, r26 ; seqi r15, r16, 5 ; ctz r5, r6 }
	{ lh r25, r26 ; seqi r15, r16, 5 ; or r5, r6, r7 }
	{ lh r25, r26 ; seqi r15, r16, 5 ; sne r5, r6, r7 }
	{ lh r25, r26 ; seqi r5, r6, 5 ; mz r15, r16, r17 }
	{ lh r25, r26 ; seqi r5, r6, 5 ; slti r15, r16, 5 }
	{ lh r25, r26 ; shl r15, r16, r17 ; movei r5, 5 }
	{ lh r25, r26 ; shl r15, r16, r17 ; s1a r5, r6, r7 }
	{ lh r25, r26 ; shl r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lh r25, r26 ; shl r5, r6, r7 ; rl r15, r16, r17 }
	{ lh r25, r26 ; shl r5, r6, r7 ; sub r15, r16, r17 }
	{ lh r25, r26 ; shli r15, r16, 5 ; mulhlsa_uu r5, r6, r7 }
	{ lh r25, r26 ; shli r15, r16, 5 ; shl r5, r6, r7 }
	{ lh r25, r26 ; shli r5, r6, 5 ; add r15, r16, r17 }
	{ lh r25, r26 ; shli r5, r6, 5 ; seq r15, r16, r17 }
	{ lh r25, r26 ; shr r15, r16, r17 ; and r5, r6, r7 }
	{ lh r25, r26 ; shr r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lh r25, r26 ; shr r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lh r25, r26 ; shr r5, r6, r7 ; ill }
	{ lh r25, r26 ; shr r5, r6, r7 ; shri r15, r16, 5 }
	{ lh r25, r26 ; shri r15, r16, 5 ; ctz r5, r6 }
	{ lh r25, r26 ; shri r15, r16, 5 ; or r5, r6, r7 }
	{ lh r25, r26 ; shri r15, r16, 5 ; sne r5, r6, r7 }
	{ lh r25, r26 ; shri r5, r6, 5 ; mz r15, r16, r17 }
	{ lh r25, r26 ; shri r5, r6, 5 ; slti r15, r16, 5 }
	{ lh r25, r26 ; slt r15, r16, r17 ; movei r5, 5 }
	{ lh r25, r26 ; slt r15, r16, r17 ; s1a r5, r6, r7 }
	{ lh r25, r26 ; slt r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lh r25, r26 ; slt r5, r6, r7 ; rl r15, r16, r17 }
	{ lh r25, r26 ; slt r5, r6, r7 ; sub r15, r16, r17 }
	{ lh r25, r26 ; slt_u r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ lh r25, r26 ; slt_u r15, r16, r17 ; shl r5, r6, r7 }
	{ lh r25, r26 ; slt_u r5, r6, r7 ; add r15, r16, r17 }
	{ lh r25, r26 ; slt_u r5, r6, r7 ; seq r15, r16, r17 }
	{ lh r25, r26 ; slte r15, r16, r17 ; and r5, r6, r7 }
	{ lh r25, r26 ; slte r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lh r25, r26 ; slte r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lh r25, r26 ; slte r5, r6, r7 ; ill }
	{ lh r25, r26 ; slte r5, r6, r7 ; shri r15, r16, 5 }
	{ lh r25, r26 ; slte_u r15, r16, r17 ; ctz r5, r6 }
	{ lh r25, r26 ; slte_u r15, r16, r17 ; or r5, r6, r7 }
	{ lh r25, r26 ; slte_u r15, r16, r17 ; sne r5, r6, r7 }
	{ lh r25, r26 ; slte_u r5, r6, r7 ; mz r15, r16, r17 }
	{ lh r25, r26 ; slte_u r5, r6, r7 ; slti r15, r16, 5 }
	{ lh r25, r26 ; slti r15, r16, 5 ; movei r5, 5 }
	{ lh r25, r26 ; slti r15, r16, 5 ; s1a r5, r6, r7 }
	{ lh r25, r26 ; slti r15, r16, 5 ; tblidxb1 r5, r6 }
	{ lh r25, r26 ; slti r5, r6, 5 ; rl r15, r16, r17 }
	{ lh r25, r26 ; slti r5, r6, 5 ; sub r15, r16, r17 }
	{ lh r25, r26 ; slti_u r15, r16, 5 ; mulhlsa_uu r5, r6, r7 }
	{ lh r25, r26 ; slti_u r15, r16, 5 ; shl r5, r6, r7 }
	{ lh r25, r26 ; slti_u r5, r6, 5 ; add r15, r16, r17 }
	{ lh r25, r26 ; slti_u r5, r6, 5 ; seq r15, r16, r17 }
	{ lh r25, r26 ; sne r15, r16, r17 ; and r5, r6, r7 }
	{ lh r25, r26 ; sne r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lh r25, r26 ; sne r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lh r25, r26 ; sne r5, r6, r7 ; ill }
	{ lh r25, r26 ; sne r5, r6, r7 ; shri r15, r16, 5 }
	{ lh r25, r26 ; sra r15, r16, r17 ; ctz r5, r6 }
	{ lh r25, r26 ; sra r15, r16, r17 ; or r5, r6, r7 }
	{ lh r25, r26 ; sra r15, r16, r17 ; sne r5, r6, r7 }
	{ lh r25, r26 ; sra r5, r6, r7 ; mz r15, r16, r17 }
	{ lh r25, r26 ; sra r5, r6, r7 ; slti r15, r16, 5 }
	{ lh r25, r26 ; srai r15, r16, 5 ; movei r5, 5 }
	{ lh r25, r26 ; srai r15, r16, 5 ; s1a r5, r6, r7 }
	{ lh r25, r26 ; srai r15, r16, 5 ; tblidxb1 r5, r6 }
	{ lh r25, r26 ; srai r5, r6, 5 ; rl r15, r16, r17 }
	{ lh r25, r26 ; srai r5, r6, 5 ; sub r15, r16, r17 }
	{ lh r25, r26 ; sub r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ lh r25, r26 ; sub r15, r16, r17 ; shl r5, r6, r7 }
	{ lh r25, r26 ; sub r5, r6, r7 ; add r15, r16, r17 }
	{ lh r25, r26 ; sub r5, r6, r7 ; seq r15, r16, r17 }
	{ lh r25, r26 ; tblidxb0 r5, r6 ; and r15, r16, r17 }
	{ lh r25, r26 ; tblidxb0 r5, r6 ; shl r15, r16, r17 }
	{ lh r25, r26 ; tblidxb1 r5, r6 ; fnop }
	{ lh r25, r26 ; tblidxb1 r5, r6 ; shr r15, r16, r17 }
	{ lh r25, r26 ; tblidxb2 r5, r6 ; info 19 }
	{ lh r25, r26 ; tblidxb2 r5, r6 ; slt r15, r16, r17 }
	{ lh r25, r26 ; tblidxb3 r5, r6 ; move r15, r16 }
	{ lh r25, r26 ; tblidxb3 r5, r6 ; slte r15, r16, r17 }
	{ lh r25, r26 ; xor r15, r16, r17 ; mnz r5, r6, r7 }
	{ lh r25, r26 ; xor r15, r16, r17 ; rl r5, r6, r7 }
	{ lh r25, r26 ; xor r15, r16, r17 ; sub r5, r6, r7 }
	{ lh r25, r26 ; xor r5, r6, r7 ; or r15, r16, r17 }
	{ lh r25, r26 ; xor r5, r6, r7 ; sra r15, r16, r17 }
	{ lh_u r15, r16 ; auli r5, r6, 0x1234 }
	{ lh_u r15, r16 ; maxih r5, r6, 5 }
	{ lh_u r15, r16 ; mulhl_ss r5, r6, r7 }
	{ lh_u r15, r16 ; mzh r5, r6, r7 }
	{ lh_u r15, r16 ; sadh_u r5, r6, r7 }
	{ lh_u r15, r16 ; slt_u r5, r6, r7 }
	{ lh_u r15, r16 ; sra r5, r6, r7 }
	{ lh_u r25, r26 ; add r15, r16, r17 ; and r5, r6, r7 }
	{ lh_u r25, r26 ; add r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lh_u r25, r26 ; add r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lh_u r25, r26 ; add r5, r6, r7 ; ill }
	{ lh_u r25, r26 ; add r5, r6, r7 ; shri r15, r16, 5 }
	{ lh_u r25, r26 ; addi r15, r16, 5 ; ctz r5, r6 }
	{ lh_u r25, r26 ; addi r15, r16, 5 ; or r5, r6, r7 }
	{ lh_u r25, r26 ; addi r15, r16, 5 ; sne r5, r6, r7 }
	{ lh_u r25, r26 ; addi r5, r6, 5 ; mz r15, r16, r17 }
	{ lh_u r25, r26 ; addi r5, r6, 5 ; slti r15, r16, 5 }
	{ lh_u r25, r26 ; and r15, r16, r17 ; movei r5, 5 }
	{ lh_u r25, r26 ; and r15, r16, r17 ; s1a r5, r6, r7 }
	{ lh_u r25, r26 ; and r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lh_u r25, r26 ; and r5, r6, r7 ; rl r15, r16, r17 }
	{ lh_u r25, r26 ; and r5, r6, r7 ; sub r15, r16, r17 }
	{ lh_u r25, r26 ; andi r15, r16, 5 ; mulhlsa_uu r5, r6, r7 }
	{ lh_u r25, r26 ; andi r15, r16, 5 ; shl r5, r6, r7 }
	{ lh_u r25, r26 ; andi r5, r6, 5 ; add r15, r16, r17 }
	{ lh_u r25, r26 ; andi r5, r6, 5 ; seq r15, r16, r17 }
	{ lh_u r25, r26 ; bitx r5, r6 ; and r15, r16, r17 }
	{ lh_u r25, r26 ; bitx r5, r6 ; shl r15, r16, r17 }
	{ lh_u r25, r26 ; bytex r5, r6 ; fnop }
	{ lh_u r25, r26 ; bytex r5, r6 ; shr r15, r16, r17 }
	{ lh_u r25, r26 ; clz r5, r6 ; info 19 }
	{ lh_u r25, r26 ; clz r5, r6 ; slt r15, r16, r17 }
	{ lh_u r25, r26 ; ctz r5, r6 ; move r15, r16 }
	{ lh_u r25, r26 ; ctz r5, r6 ; slte r15, r16, r17 }
	{ lh_u r25, r26 ; fnop ; clz r5, r6 }
	{ lh_u r25, r26 ; fnop ; mvnz r5, r6, r7 }
	{ lh_u r25, r26 ; fnop ; s3a r15, r16, r17 }
	{ lh_u r25, r26 ; fnop ; slte_u r15, r16, r17 }
	{ lh_u r25, r26 ; fnop }
	{ lh_u r25, r26 ; ill ; mulll_uu r5, r6, r7 }
	{ lh_u r25, r26 ; ill ; shr r5, r6, r7 }
	{ lh_u r25, r26 ; info 19 ; addi r15, r16, 5 }
	{ lh_u r25, r26 ; info 19 ; mulhh_uu r5, r6, r7 }
	{ lh_u r25, r26 ; info 19 ; rl r15, r16, r17 }
	{ lh_u r25, r26 ; info 19 ; shri r15, r16, 5 }
	{ lh_u r25, r26 ; info 19 ; sub r15, r16, r17 }
	{ lh_u r25, r26 ; mnz r15, r16, r17 ; move r5, r6 }
	{ lh_u r25, r26 ; mnz r15, r16, r17 ; rli r5, r6, 5 }
	{ lh_u r25, r26 ; mnz r15, r16, r17 ; tblidxb0 r5, r6 }
	{ lh_u r25, r26 ; mnz r5, r6, r7 ; ori r15, r16, 5 }
	{ lh_u r25, r26 ; mnz r5, r6, r7 ; srai r15, r16, 5 }
	{ lh_u r25, r26 ; move r15, r16 ; mulhha_uu r5, r6, r7 }
	{ lh_u r25, r26 ; move r15, r16 ; seqi r5, r6, 5 }
	{ lh_u r25, r26 ; move r15, r16 }
	{ lh_u r25, r26 ; move r5, r6 ; s3a r15, r16, r17 }
	{ lh_u r25, r26 ; movei r15, 5 ; addi r5, r6, 5 }
	{ lh_u r25, r26 ; movei r15, 5 ; mullla_uu r5, r6, r7 }
	{ lh_u r25, r26 ; movei r15, 5 ; slt r5, r6, r7 }
	{ lh_u r25, r26 ; movei r5, 5 ; fnop }
	{ lh_u r25, r26 ; movei r5, 5 ; shr r15, r16, r17 }
	{ lh_u r25, r26 ; mulhh_ss r5, r6, r7 ; info 19 }
	{ lh_u r25, r26 ; mulhh_ss r5, r6, r7 ; slt r15, r16, r17 }
	{ lh_u r25, r26 ; mulhh_uu r5, r6, r7 ; move r15, r16 }
	{ lh_u r25, r26 ; mulhh_uu r5, r6, r7 ; slte r15, r16, r17 }
	{ lh_u r25, r26 ; mulhha_ss r5, r6, r7 ; mz r15, r16, r17 }
	{ lh_u r25, r26 ; mulhha_ss r5, r6, r7 ; slti r15, r16, 5 }
	{ lh_u r25, r26 ; mulhha_uu r5, r6, r7 ; nor r15, r16, r17 }
	{ lh_u r25, r26 ; mulhha_uu r5, r6, r7 ; sne r15, r16, r17 }
	{ lh_u r25, r26 ; mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 }
	{ lh_u r25, r26 ; mulhlsa_uu r5, r6, r7 ; srai r15, r16, 5 }
	{ lh_u r25, r26 ; mulll_ss r5, r6, r7 ; rli r15, r16, 5 }
	{ lh_u r25, r26 ; mulll_ss r5, r6, r7 ; xor r15, r16, r17 }
	{ lh_u r25, r26 ; mulll_uu r5, r6, r7 ; s2a r15, r16, r17 }
	{ lh_u r25, r26 ; mullla_ss r5, r6, r7 ; add r15, r16, r17 }
	{ lh_u r25, r26 ; mullla_ss r5, r6, r7 ; seq r15, r16, r17 }
	{ lh_u r25, r26 ; mullla_uu r5, r6, r7 ; and r15, r16, r17 }
	{ lh_u r25, r26 ; mullla_uu r5, r6, r7 ; shl r15, r16, r17 }
	{ lh_u r25, r26 ; mvnz r5, r6, r7 ; fnop }
	{ lh_u r25, r26 ; mvnz r5, r6, r7 ; shr r15, r16, r17 }
	{ lh_u r25, r26 ; mvz r5, r6, r7 ; info 19 }
	{ lh_u r25, r26 ; mvz r5, r6, r7 ; slt r15, r16, r17 }
	{ lh_u r25, r26 ; mz r15, r16, r17 ; fnop }
	{ lh_u r25, r26 ; mz r15, r16, r17 ; ori r5, r6, 5 }
	{ lh_u r25, r26 ; mz r15, r16, r17 ; sra r5, r6, r7 }
	{ lh_u r25, r26 ; mz r5, r6, r7 ; nop }
	{ lh_u r25, r26 ; mz r5, r6, r7 ; slti_u r15, r16, 5 }
	{ lh_u r25, r26 ; nop ; ill }
	{ lh_u r25, r26 ; nop ; mz r5, r6, r7 }
	{ lh_u r25, r26 ; nop ; seq r5, r6, r7 }
	{ lh_u r25, r26 ; nop ; slti r5, r6, 5 }
	{ lh_u r25, r26 ; nor r15, r16, r17 ; and r5, r6, r7 }
	{ lh_u r25, r26 ; nor r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lh_u r25, r26 ; nor r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lh_u r25, r26 ; nor r5, r6, r7 ; ill }
	{ lh_u r25, r26 ; nor r5, r6, r7 ; shri r15, r16, 5 }
	{ lh_u r25, r26 ; or r15, r16, r17 ; ctz r5, r6 }
	{ lh_u r25, r26 ; or r15, r16, r17 ; or r5, r6, r7 }
	{ lh_u r25, r26 ; or r15, r16, r17 ; sne r5, r6, r7 }
	{ lh_u r25, r26 ; or r5, r6, r7 ; mz r15, r16, r17 }
	{ lh_u r25, r26 ; or r5, r6, r7 ; slti r15, r16, 5 }
	{ lh_u r25, r26 ; ori r15, r16, 5 ; movei r5, 5 }
	{ lh_u r25, r26 ; ori r15, r16, 5 ; s1a r5, r6, r7 }
	{ lh_u r25, r26 ; ori r15, r16, 5 ; tblidxb1 r5, r6 }
	{ lh_u r25, r26 ; ori r5, r6, 5 ; rl r15, r16, r17 }
	{ lh_u r25, r26 ; ori r5, r6, 5 ; sub r15, r16, r17 }
	{ lh_u r25, r26 ; pcnt r5, r6 ; s1a r15, r16, r17 }
	{ lh_u r25, r26 ; pcnt r5, r6 }
	{ lh_u r25, r26 ; rl r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ lh_u r25, r26 ; rl r15, r16, r17 ; shr r5, r6, r7 }
	{ lh_u r25, r26 ; rl r5, r6, r7 ; and r15, r16, r17 }
	{ lh_u r25, r26 ; rl r5, r6, r7 ; shl r15, r16, r17 }
	{ lh_u r25, r26 ; rli r15, r16, 5 ; bitx r5, r6 }
	{ lh_u r25, r26 ; rli r15, r16, 5 ; mz r5, r6, r7 }
	{ lh_u r25, r26 ; rli r15, r16, 5 ; slte_u r5, r6, r7 }
	{ lh_u r25, r26 ; rli r5, r6, 5 ; mnz r15, r16, r17 }
	{ lh_u r25, r26 ; rli r5, r6, 5 ; slt_u r15, r16, r17 }
	{ lh_u r25, r26 ; s1a r15, r16, r17 ; info 19 }
	{ lh_u r25, r26 ; s1a r15, r16, r17 ; pcnt r5, r6 }
	{ lh_u r25, r26 ; s1a r15, r16, r17 ; srai r5, r6, 5 }
	{ lh_u r25, r26 ; s1a r5, r6, r7 ; nor r15, r16, r17 }
	{ lh_u r25, r26 ; s1a r5, r6, r7 ; sne r15, r16, r17 }
	{ lh_u r25, r26 ; s2a r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lh_u r25, r26 ; s2a r15, r16, r17 ; s3a r5, r6, r7 }
	{ lh_u r25, r26 ; s2a r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lh_u r25, r26 ; s2a r5, r6, r7 ; s1a r15, r16, r17 }
	{ lh_u r25, r26 ; s2a r5, r6, r7 }
	{ lh_u r25, r26 ; s3a r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ lh_u r25, r26 ; s3a r15, r16, r17 ; shr r5, r6, r7 }
	{ lh_u r25, r26 ; s3a r5, r6, r7 ; and r15, r16, r17 }
	{ lh_u r25, r26 ; s3a r5, r6, r7 ; shl r15, r16, r17 }
	{ lh_u r25, r26 ; seq r15, r16, r17 ; bitx r5, r6 }
	{ lh_u r25, r26 ; seq r15, r16, r17 ; mz r5, r6, r7 }
	{ lh_u r25, r26 ; seq r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lh_u r25, r26 ; seq r5, r6, r7 ; mnz r15, r16, r17 }
	{ lh_u r25, r26 ; seq r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lh_u r25, r26 ; seqi r15, r16, 5 ; info 19 }
	{ lh_u r25, r26 ; seqi r15, r16, 5 ; pcnt r5, r6 }
	{ lh_u r25, r26 ; seqi r15, r16, 5 ; srai r5, r6, 5 }
	{ lh_u r25, r26 ; seqi r5, r6, 5 ; nor r15, r16, r17 }
	{ lh_u r25, r26 ; seqi r5, r6, 5 ; sne r15, r16, r17 }
	{ lh_u r25, r26 ; shl r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lh_u r25, r26 ; shl r15, r16, r17 ; s3a r5, r6, r7 }
	{ lh_u r25, r26 ; shl r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lh_u r25, r26 ; shl r5, r6, r7 ; s1a r15, r16, r17 }
	{ lh_u r25, r26 ; shl r5, r6, r7 }
	{ lh_u r25, r26 ; shli r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ lh_u r25, r26 ; shli r15, r16, 5 ; shr r5, r6, r7 }
	{ lh_u r25, r26 ; shli r5, r6, 5 ; and r15, r16, r17 }
	{ lh_u r25, r26 ; shli r5, r6, 5 ; shl r15, r16, r17 }
	{ lh_u r25, r26 ; shr r15, r16, r17 ; bitx r5, r6 }
	{ lh_u r25, r26 ; shr r15, r16, r17 ; mz r5, r6, r7 }
	{ lh_u r25, r26 ; shr r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lh_u r25, r26 ; shr r5, r6, r7 ; mnz r15, r16, r17 }
	{ lh_u r25, r26 ; shr r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lh_u r25, r26 ; shri r15, r16, 5 ; info 19 }
	{ lh_u r25, r26 ; shri r15, r16, 5 ; pcnt r5, r6 }
	{ lh_u r25, r26 ; shri r15, r16, 5 ; srai r5, r6, 5 }
	{ lh_u r25, r26 ; shri r5, r6, 5 ; nor r15, r16, r17 }
	{ lh_u r25, r26 ; shri r5, r6, 5 ; sne r15, r16, r17 }
	{ lh_u r25, r26 ; slt r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lh_u r25, r26 ; slt r15, r16, r17 ; s3a r5, r6, r7 }
	{ lh_u r25, r26 ; slt r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lh_u r25, r26 ; slt r5, r6, r7 ; s1a r15, r16, r17 }
	{ lh_u r25, r26 ; slt r5, r6, r7 }
	{ lh_u r25, r26 ; slt_u r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ lh_u r25, r26 ; slt_u r15, r16, r17 ; shr r5, r6, r7 }
	{ lh_u r25, r26 ; slt_u r5, r6, r7 ; and r15, r16, r17 }
	{ lh_u r25, r26 ; slt_u r5, r6, r7 ; shl r15, r16, r17 }
	{ lh_u r25, r26 ; slte r15, r16, r17 ; bitx r5, r6 }
	{ lh_u r25, r26 ; slte r15, r16, r17 ; mz r5, r6, r7 }
	{ lh_u r25, r26 ; slte r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lh_u r25, r26 ; slte r5, r6, r7 ; mnz r15, r16, r17 }
	{ lh_u r25, r26 ; slte r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lh_u r25, r26 ; slte_u r15, r16, r17 ; info 19 }
	{ lh_u r25, r26 ; slte_u r15, r16, r17 ; pcnt r5, r6 }
	{ lh_u r25, r26 ; slte_u r15, r16, r17 ; srai r5, r6, 5 }
	{ lh_u r25, r26 ; slte_u r5, r6, r7 ; nor r15, r16, r17 }
	{ lh_u r25, r26 ; slte_u r5, r6, r7 ; sne r15, r16, r17 }
	{ lh_u r25, r26 ; slti r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ lh_u r25, r26 ; slti r15, r16, 5 ; s3a r5, r6, r7 }
	{ lh_u r25, r26 ; slti r15, r16, 5 ; tblidxb3 r5, r6 }
	{ lh_u r25, r26 ; slti r5, r6, 5 ; s1a r15, r16, r17 }
	{ lh_u r25, r26 ; slti r5, r6, 5 }
	{ lh_u r25, r26 ; slti_u r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ lh_u r25, r26 ; slti_u r15, r16, 5 ; shr r5, r6, r7 }
	{ lh_u r25, r26 ; slti_u r5, r6, 5 ; and r15, r16, r17 }
	{ lh_u r25, r26 ; slti_u r5, r6, 5 ; shl r15, r16, r17 }
	{ lh_u r25, r26 ; sne r15, r16, r17 ; bitx r5, r6 }
	{ lh_u r25, r26 ; sne r15, r16, r17 ; mz r5, r6, r7 }
	{ lh_u r25, r26 ; sne r15, r16, r17 ; slte_u r5, r6, r7 }
	{ lh_u r25, r26 ; sne r5, r6, r7 ; mnz r15, r16, r17 }
	{ lh_u r25, r26 ; sne r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lh_u r25, r26 ; sra r15, r16, r17 ; info 19 }
	{ lh_u r25, r26 ; sra r15, r16, r17 ; pcnt r5, r6 }
	{ lh_u r25, r26 ; sra r15, r16, r17 ; srai r5, r6, 5 }
	{ lh_u r25, r26 ; sra r5, r6, r7 ; nor r15, r16, r17 }
	{ lh_u r25, r26 ; sra r5, r6, r7 ; sne r15, r16, r17 }
	{ lh_u r25, r26 ; srai r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ lh_u r25, r26 ; srai r15, r16, 5 ; s3a r5, r6, r7 }
	{ lh_u r25, r26 ; srai r15, r16, 5 ; tblidxb3 r5, r6 }
	{ lh_u r25, r26 ; srai r5, r6, 5 ; s1a r15, r16, r17 }
	{ lh_u r25, r26 ; srai r5, r6, 5 }
	{ lh_u r25, r26 ; sub r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ lh_u r25, r26 ; sub r15, r16, r17 ; shr r5, r6, r7 }
	{ lh_u r25, r26 ; sub r5, r6, r7 ; and r15, r16, r17 }
	{ lh_u r25, r26 ; sub r5, r6, r7 ; shl r15, r16, r17 }
	{ lh_u r25, r26 ; tblidxb0 r5, r6 ; fnop }
	{ lh_u r25, r26 ; tblidxb0 r5, r6 ; shr r15, r16, r17 }
	{ lh_u r25, r26 ; tblidxb1 r5, r6 ; info 19 }
	{ lh_u r25, r26 ; tblidxb1 r5, r6 ; slt r15, r16, r17 }
	{ lh_u r25, r26 ; tblidxb2 r5, r6 ; move r15, r16 }
	{ lh_u r25, r26 ; tblidxb2 r5, r6 ; slte r15, r16, r17 }
	{ lh_u r25, r26 ; tblidxb3 r5, r6 ; mz r15, r16, r17 }
	{ lh_u r25, r26 ; tblidxb3 r5, r6 ; slti r15, r16, 5 }
	{ lh_u r25, r26 ; xor r15, r16, r17 ; movei r5, 5 }
	{ lh_u r25, r26 ; xor r15, r16, r17 ; s1a r5, r6, r7 }
	{ lh_u r25, r26 ; xor r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lh_u r25, r26 ; xor r5, r6, r7 ; rl r15, r16, r17 }
	{ lh_u r25, r26 ; xor r5, r6, r7 ; sub r15, r16, r17 }
	{ lhadd r15, r16, 5 ; avgh r5, r6, r7 }
	{ lhadd r15, r16, 5 ; minh r5, r6, r7 }
	{ lhadd r15, r16, 5 ; mulhl_us r5, r6, r7 }
	{ lhadd r15, r16, 5 ; nor r5, r6, r7 }
	{ lhadd r15, r16, 5 ; seqb r5, r6, r7 }
	{ lhadd r15, r16, 5 ; sltb_u r5, r6, r7 }
	{ lhadd r15, r16, 5 ; srah r5, r6, r7 }
	{ lhadd_u r15, r16, 5 ; addhs r5, r6, r7 }
	{ lhadd_u r15, r16, 5 ; dword_align r5, r6, r7 }
	{ lhadd_u r15, r16, 5 ; move r5, r6 }
	{ lhadd_u r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ lhadd_u r15, r16, 5 ; pcnt r5, r6 }
	{ lhadd_u r15, r16, 5 ; shlh r5, r6, r7 }
	{ lhadd_u r15, r16, 5 ; slth r5, r6, r7 }
	{ lhadd_u r15, r16, 5 ; subh r5, r6, r7 }
	{ lnk r15 ; adiffb_u r5, r6, r7 }
	{ lnk r15 ; intlh r5, r6, r7 }
	{ lnk r15 ; mulhha_ss r5, r6, r7 }
	{ lnk r15 ; mvnz r5, r6, r7 }
	{ lnk r15 ; sadah r5, r6, r7 }
	{ lnk r15 ; shri r5, r6, 5 }
	{ lnk r15 ; sltih_u r5, r6, 5 }
	{ lnk r15 ; xor r5, r6, r7 }
	{ lw r15, r16 ; bitx r5, r6 }
	{ lw r15, r16 ; minib_u r5, r6, 5 }
	{ lw r15, r16 ; mulhl_uu r5, r6, r7 }
	{ lw r15, r16 ; or r5, r6, r7 }
	{ lw r15, r16 ; seqh r5, r6, r7 }
	{ lw r15, r16 ; slte r5, r6, r7 }
	{ lw r15, r16 ; srai r5, r6, 5 }
	{ lw r25, r26 ; add r15, r16, r17 ; bytex r5, r6 }
	{ lw r25, r26 ; add r15, r16, r17 ; nop }
	{ lw r25, r26 ; add r15, r16, r17 ; slti r5, r6, 5 }
	{ lw r25, r26 ; add r5, r6, r7 ; move r15, r16 }
	{ lw r25, r26 ; add r5, r6, r7 ; slte r15, r16, r17 }
	{ lw r25, r26 ; addi r15, r16, 5 ; mnz r5, r6, r7 }
	{ lw r25, r26 ; addi r15, r16, 5 ; rl r5, r6, r7 }
	{ lw r25, r26 ; addi r15, r16, 5 ; sub r5, r6, r7 }
	{ lw r25, r26 ; addi r5, r6, 5 ; or r15, r16, r17 }
	{ lw r25, r26 ; addi r5, r6, 5 ; sra r15, r16, r17 }
	{ lw r25, r26 ; and r15, r16, r17 ; mulhha_ss r5, r6, r7 }
	{ lw r25, r26 ; and r15, r16, r17 ; seq r5, r6, r7 }
	{ lw r25, r26 ; and r15, r16, r17 ; xor r5, r6, r7 }
	{ lw r25, r26 ; and r5, r6, r7 ; s2a r15, r16, r17 }
	{ lw r25, r26 ; andi r15, r16, 5 ; add r5, r6, r7 }
	{ lw r25, r26 ; andi r15, r16, 5 ; mullla_ss r5, r6, r7 }
	{ lw r25, r26 ; andi r15, r16, 5 ; shri r5, r6, 5 }
	{ lw r25, r26 ; andi r5, r6, 5 ; andi r15, r16, 5 }
	{ lw r25, r26 ; andi r5, r6, 5 ; shli r15, r16, 5 }
	{ lw r25, r26 ; bitx r5, r6 ; ill }
	{ lw r25, r26 ; bitx r5, r6 ; shri r15, r16, 5 }
	{ lw r25, r26 ; bytex r5, r6 ; mnz r15, r16, r17 }
	{ lw r25, r26 ; bytex r5, r6 ; slt_u r15, r16, r17 }
	{ lw r25, r26 ; clz r5, r6 ; movei r15, 5 }
	{ lw r25, r26 ; clz r5, r6 ; slte_u r15, r16, r17 }
	{ lw r25, r26 ; ctz r5, r6 ; nop }
	{ lw r25, r26 ; ctz r5, r6 ; slti_u r15, r16, 5 }
	{ lw r25, r26 ; fnop ; ill }
	{ lw r25, r26 ; fnop ; mz r5, r6, r7 }
	{ lw r25, r26 ; fnop ; seq r5, r6, r7 }
	{ lw r25, r26 ; fnop ; slti r5, r6, 5 }
	{ lw r25, r26 ; ill ; and r5, r6, r7 }
	{ lw r25, r26 ; ill ; mvnz r5, r6, r7 }
	{ lw r25, r26 ; ill ; slt_u r5, r6, r7 }
	{ lw r25, r26 ; info 19 ; and r5, r6, r7 }
	{ lw r25, r26 ; info 19 ; mulhlsa_uu r5, r6, r7 }
	{ lw r25, r26 ; info 19 ; rli r5, r6, 5 }
	{ lw r25, r26 ; info 19 ; slt r5, r6, r7 }
	{ lw r25, r26 ; info 19 ; tblidxb1 r5, r6 }
	{ lw r25, r26 ; mnz r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ lw r25, r26 ; mnz r15, r16, r17 ; s3a r5, r6, r7 }
	{ lw r25, r26 ; mnz r15, r16, r17 ; tblidxb3 r5, r6 }
	{ lw r25, r26 ; mnz r5, r6, r7 ; s1a r15, r16, r17 }
	{ lw r25, r26 ; mnz r5, r6, r7 }
	{ lw r25, r26 ; move r15, r16 ; mulll_uu r5, r6, r7 }
	{ lw r25, r26 ; move r15, r16 ; shr r5, r6, r7 }
	{ lw r25, r26 ; move r5, r6 ; and r15, r16, r17 }
	{ lw r25, r26 ; move r5, r6 ; shl r15, r16, r17 }
	{ lw r25, r26 ; movei r15, 5 ; bitx r5, r6 }
	{ lw r25, r26 ; movei r15, 5 ; mz r5, r6, r7 }
	{ lw r25, r26 ; movei r15, 5 ; slte_u r5, r6, r7 }
	{ lw r25, r26 ; movei r5, 5 ; mnz r15, r16, r17 }
	{ lw r25, r26 ; movei r5, 5 ; slt_u r15, r16, r17 }
	{ lw r25, r26 ; mulhh_ss r5, r6, r7 ; movei r15, 5 }
	{ lw r25, r26 ; mulhh_ss r5, r6, r7 ; slte_u r15, r16, r17 }
	{ lw r25, r26 ; mulhh_uu r5, r6, r7 ; nop }
	{ lw r25, r26 ; mulhh_uu r5, r6, r7 ; slti_u r15, r16, 5 }
	{ lw r25, r26 ; mulhha_ss r5, r6, r7 ; or r15, r16, r17 }
	{ lw r25, r26 ; mulhha_ss r5, r6, r7 ; sra r15, r16, r17 }
	{ lw r25, r26 ; mulhha_uu r5, r6, r7 ; rl r15, r16, r17 }
	{ lw r25, r26 ; mulhha_uu r5, r6, r7 ; sub r15, r16, r17 }
	{ lw r25, r26 ; mulhlsa_uu r5, r6, r7 ; s1a r15, r16, r17 }
	{ lw r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ lw r25, r26 ; mulll_ss r5, r6, r7 ; s3a r15, r16, r17 }
	{ lw r25, r26 ; mulll_uu r5, r6, r7 ; addi r15, r16, 5 }
	{ lw r25, r26 ; mulll_uu r5, r6, r7 ; seqi r15, r16, 5 }
	{ lw r25, r26 ; mullla_ss r5, r6, r7 ; andi r15, r16, 5 }
	{ lw r25, r26 ; mullla_ss r5, r6, r7 ; shli r15, r16, 5 }
	{ lw r25, r26 ; mullla_uu r5, r6, r7 ; ill }
	{ lw r25, r26 ; mullla_uu r5, r6, r7 ; shri r15, r16, 5 }
	{ lw r25, r26 ; mvnz r5, r6, r7 ; mnz r15, r16, r17 }
	{ lw r25, r26 ; mvnz r5, r6, r7 ; slt_u r15, r16, r17 }
	{ lw r25, r26 ; mvz r5, r6, r7 ; movei r15, 5 }
	{ lw r25, r26 ; mvz r5, r6, r7 ; slte_u r15, r16, r17 }
	{ lw r25, r26 ; mz r15, r16, r17 ; move r5, r6 }
	{ lw r25, r26 ; mz r15, r16, r17 ; rli r5, r6, 5 }
	{ lw r25, r26 ; mz r15, r16, r17 ; tblidxb0 r5, r6 }
	{ lw r25, r26 ; mz r5, r6, r7 ; ori r15, r16, 5 }
	{ lw r25, r26 ; mz r5, r6, r7 ; srai r15, r16, 5 }
	{ lw r25, r26 ; nop ; mnz r5, r6, r7 }
	{ lw r25, r26 ; nop ; nor r5, r6, r7 }
	{ lw r25, r26 ; nop ; shl r15, r16, r17 }
	{ lw r25, r26 ; nop ; sne r15, r16, r17 }
	{ lw r25, r26 ; nor r15, r16, r17 ; bytex r5, r6 }
	{ lw r25, r26 ; nor r15, r16, r17 ; nop }
	{ lw r25, r26 ; nor r15, r16, r17 ; slti r5, r6, 5 }
	{ lw r25, r26 ; nor r5, r6, r7 ; move r15, r16 }
	{ lw r25, r26 ; nor r5, r6, r7 ; slte r15, r16, r17 }
	{ lw r25, r26 ; or r15, r16, r17 ; mnz r5, r6, r7 }
	{ lw r25, r26 ; or r15, r16, r17 ; rl r5, r6, r7 }
	{ lw r25, r26 ; or r15, r16, r17 ; sub r5, r6, r7 }
	{ lw r25, r26 ; or r5, r6, r7 ; or r15, r16, r17 }
	{ lw r25, r26 ; or r5, r6, r7 ; sra r15, r16, r17 }
	{ lw r25, r26 ; ori r15, r16, 5 ; mulhha_ss r5, r6, r7 }
	{ lw r25, r26 ; ori r15, r16, 5 ; seq r5, r6, r7 }
	{ lw r25, r26 ; ori r15, r16, 5 ; xor r5, r6, r7 }
	{ lw r25, r26 ; ori r5, r6, 5 ; s2a r15, r16, r17 }
	{ lw r25, r26 ; pcnt r5, r6 ; add r15, r16, r17 }
	{ lw r25, r26 ; pcnt r5, r6 ; seq r15, r16, r17 }
	{ lw r25, r26 ; rl r15, r16, r17 ; and r5, r6, r7 }
	{ lw r25, r26 ; rl r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lw r25, r26 ; rl r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lw r25, r26 ; rl r5, r6, r7 ; ill }
	{ lw r25, r26 ; rl r5, r6, r7 ; shri r15, r16, 5 }
	{ lw r25, r26 ; rli r15, r16, 5 ; ctz r5, r6 }
	{ lw r25, r26 ; rli r15, r16, 5 ; or r5, r6, r7 }
	{ lw r25, r26 ; rli r15, r16, 5 ; sne r5, r6, r7 }
	{ lw r25, r26 ; rli r5, r6, 5 ; mz r15, r16, r17 }
	{ lw r25, r26 ; rli r5, r6, 5 ; slti r15, r16, 5 }
	{ lw r25, r26 ; s1a r15, r16, r17 ; movei r5, 5 }
	{ lw r25, r26 ; s1a r15, r16, r17 ; s1a r5, r6, r7 }
	{ lw r25, r26 ; s1a r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lw r25, r26 ; s1a r5, r6, r7 ; rl r15, r16, r17 }
	{ lw r25, r26 ; s1a r5, r6, r7 ; sub r15, r16, r17 }
	{ lw r25, r26 ; s2a r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ lw r25, r26 ; s2a r15, r16, r17 ; shl r5, r6, r7 }
	{ lw r25, r26 ; s2a r5, r6, r7 ; add r15, r16, r17 }
	{ lw r25, r26 ; s2a r5, r6, r7 ; seq r15, r16, r17 }
	{ lw r25, r26 ; s3a r15, r16, r17 ; and r5, r6, r7 }
	{ lw r25, r26 ; s3a r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lw r25, r26 ; s3a r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lw r25, r26 ; s3a r5, r6, r7 ; ill }
	{ lw r25, r26 ; s3a r5, r6, r7 ; shri r15, r16, 5 }
	{ lw r25, r26 ; seq r15, r16, r17 ; ctz r5, r6 }
	{ lw r25, r26 ; seq r15, r16, r17 ; or r5, r6, r7 }
	{ lw r25, r26 ; seq r15, r16, r17 ; sne r5, r6, r7 }
	{ lw r25, r26 ; seq r5, r6, r7 ; mz r15, r16, r17 }
	{ lw r25, r26 ; seq r5, r6, r7 ; slti r15, r16, 5 }
	{ lw r25, r26 ; seqi r15, r16, 5 ; movei r5, 5 }
	{ lw r25, r26 ; seqi r15, r16, 5 ; s1a r5, r6, r7 }
	{ lw r25, r26 ; seqi r15, r16, 5 ; tblidxb1 r5, r6 }
	{ lw r25, r26 ; seqi r5, r6, 5 ; rl r15, r16, r17 }
	{ lw r25, r26 ; seqi r5, r6, 5 ; sub r15, r16, r17 }
	{ lw r25, r26 ; shl r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ lw r25, r26 ; shl r15, r16, r17 ; shl r5, r6, r7 }
	{ lw r25, r26 ; shl r5, r6, r7 ; add r15, r16, r17 }
	{ lw r25, r26 ; shl r5, r6, r7 ; seq r15, r16, r17 }
	{ lw r25, r26 ; shli r15, r16, 5 ; and r5, r6, r7 }
	{ lw r25, r26 ; shli r15, r16, 5 ; mvnz r5, r6, r7 }
	{ lw r25, r26 ; shli r15, r16, 5 ; slt_u r5, r6, r7 }
	{ lw r25, r26 ; shli r5, r6, 5 ; ill }
	{ lw r25, r26 ; shli r5, r6, 5 ; shri r15, r16, 5 }
	{ lw r25, r26 ; shr r15, r16, r17 ; ctz r5, r6 }
	{ lw r25, r26 ; shr r15, r16, r17 ; or r5, r6, r7 }
	{ lw r25, r26 ; shr r15, r16, r17 ; sne r5, r6, r7 }
	{ lw r25, r26 ; shr r5, r6, r7 ; mz r15, r16, r17 }
	{ lw r25, r26 ; shr r5, r6, r7 ; slti r15, r16, 5 }
	{ lw r25, r26 ; shri r15, r16, 5 ; movei r5, 5 }
	{ lw r25, r26 ; shri r15, r16, 5 ; s1a r5, r6, r7 }
	{ lw r25, r26 ; shri r15, r16, 5 ; tblidxb1 r5, r6 }
	{ lw r25, r26 ; shri r5, r6, 5 ; rl r15, r16, r17 }
	{ lw r25, r26 ; shri r5, r6, 5 ; sub r15, r16, r17 }
	{ lw r25, r26 ; slt r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ lw r25, r26 ; slt r15, r16, r17 ; shl r5, r6, r7 }
	{ lw r25, r26 ; slt r5, r6, r7 ; add r15, r16, r17 }
	{ lw r25, r26 ; slt r5, r6, r7 ; seq r15, r16, r17 }
	{ lw r25, r26 ; slt_u r15, r16, r17 ; and r5, r6, r7 }
	{ lw r25, r26 ; slt_u r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lw r25, r26 ; slt_u r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lw r25, r26 ; slt_u r5, r6, r7 ; ill }
	{ lw r25, r26 ; slt_u r5, r6, r7 ; shri r15, r16, 5 }
	{ lw r25, r26 ; slte r15, r16, r17 ; ctz r5, r6 }
	{ lw r25, r26 ; slte r15, r16, r17 ; or r5, r6, r7 }
	{ lw r25, r26 ; slte r15, r16, r17 ; sne r5, r6, r7 }
	{ lw r25, r26 ; slte r5, r6, r7 ; mz r15, r16, r17 }
	{ lw r25, r26 ; slte r5, r6, r7 ; slti r15, r16, 5 }
	{ lw r25, r26 ; slte_u r15, r16, r17 ; movei r5, 5 }
	{ lw r25, r26 ; slte_u r15, r16, r17 ; s1a r5, r6, r7 }
	{ lw r25, r26 ; slte_u r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lw r25, r26 ; slte_u r5, r6, r7 ; rl r15, r16, r17 }
	{ lw r25, r26 ; slte_u r5, r6, r7 ; sub r15, r16, r17 }
	{ lw r25, r26 ; slti r15, r16, 5 ; mulhlsa_uu r5, r6, r7 }
	{ lw r25, r26 ; slti r15, r16, 5 ; shl r5, r6, r7 }
	{ lw r25, r26 ; slti r5, r6, 5 ; add r15, r16, r17 }
	{ lw r25, r26 ; slti r5, r6, 5 ; seq r15, r16, r17 }
	{ lw r25, r26 ; slti_u r15, r16, 5 ; and r5, r6, r7 }
	{ lw r25, r26 ; slti_u r15, r16, 5 ; mvnz r5, r6, r7 }
	{ lw r25, r26 ; slti_u r15, r16, 5 ; slt_u r5, r6, r7 }
	{ lw r25, r26 ; slti_u r5, r6, 5 ; ill }
	{ lw r25, r26 ; slti_u r5, r6, 5 ; shri r15, r16, 5 }
	{ lw r25, r26 ; sne r15, r16, r17 ; ctz r5, r6 }
	{ lw r25, r26 ; sne r15, r16, r17 ; or r5, r6, r7 }
	{ lw r25, r26 ; sne r15, r16, r17 ; sne r5, r6, r7 }
	{ lw r25, r26 ; sne r5, r6, r7 ; mz r15, r16, r17 }
	{ lw r25, r26 ; sne r5, r6, r7 ; slti r15, r16, 5 }
	{ lw r25, r26 ; sra r15, r16, r17 ; movei r5, 5 }
	{ lw r25, r26 ; sra r15, r16, r17 ; s1a r5, r6, r7 }
	{ lw r25, r26 ; sra r15, r16, r17 ; tblidxb1 r5, r6 }
	{ lw r25, r26 ; sra r5, r6, r7 ; rl r15, r16, r17 }
	{ lw r25, r26 ; sra r5, r6, r7 ; sub r15, r16, r17 }
	{ lw r25, r26 ; srai r15, r16, 5 ; mulhlsa_uu r5, r6, r7 }
	{ lw r25, r26 ; srai r15, r16, 5 ; shl r5, r6, r7 }
	{ lw r25, r26 ; srai r5, r6, 5 ; add r15, r16, r17 }
	{ lw r25, r26 ; srai r5, r6, 5 ; seq r15, r16, r17 }
	{ lw r25, r26 ; sub r15, r16, r17 ; and r5, r6, r7 }
	{ lw r25, r26 ; sub r15, r16, r17 ; mvnz r5, r6, r7 }
	{ lw r25, r26 ; sub r15, r16, r17 ; slt_u r5, r6, r7 }
	{ lw r25, r26 ; sub r5, r6, r7 ; ill }
	{ lw r25, r26 ; sub r5, r6, r7 ; shri r15, r16, 5 }
	{ lw r25, r26 ; tblidxb0 r5, r6 ; mnz r15, r16, r17 }
	{ lw r25, r26 ; tblidxb0 r5, r6 ; slt_u r15, r16, r17 }
	{ lw r25, r26 ; tblidxb1 r5, r6 ; movei r15, 5 }
	{ lw r25, r26 ; tblidxb1 r5, r6 ; slte_u r15, r16, r17 }
	{ lw r25, r26 ; tblidxb2 r5, r6 ; nop }
	{ lw r25, r26 ; tblidxb2 r5, r6 ; slti_u r15, r16, 5 }
	{ lw r25, r26 ; tblidxb3 r5, r6 ; or r15, r16, r17 }
	{ lw r25, r26 ; tblidxb3 r5, r6 ; sra r15, r16, r17 }
	{ lw r25, r26 ; xor r15, r16, r17 ; mulhha_ss r5, r6, r7 }
	{ lw r25, r26 ; xor r15, r16, r17 ; seq r5, r6, r7 }
	{ lw r25, r26 ; xor r15, r16, r17 ; xor r5, r6, r7 }
	{ lw r25, r26 ; xor r5, r6, r7 ; s2a r15, r16, r17 }
	{ lw_na r15, r16 ; add r5, r6, r7 }
	{ lw_na r15, r16 ; clz r5, r6 }
	{ lw_na r15, r16 ; mm r5, r6, r7, 5, 7 }
	{ lw_na r15, r16 ; mulhla_su r5, r6, r7 }
	{ lw_na r15, r16 ; packbs_u r5, r6, r7 }
	{ lw_na r15, r16 ; seqib r5, r6, 5 }
	{ lw_na r15, r16 ; slteb r5, r6, r7 }
	{ lw_na r15, r16 ; sraih r5, r6, 5 }
	{ lwadd r15, r16, 5 ; addih r5, r6, 5 }
	{ lwadd r15, r16, 5 ; infol 0x1234 }
	{ lwadd r15, r16, 5 ; movelis r5, 0x1234 }
	{ lwadd r15, r16, 5 ; mullla_ss r5, r6, r7 }
	{ lwadd r15, r16, 5 ; s1a r5, r6, r7 }
	{ lwadd r15, r16, 5 ; shlih r5, r6, 5 }
	{ lwadd r15, r16, 5 ; slti_u r5, r6, 5 }
	{ lwadd r15, r16, 5 ; tblidxb0 r5, r6 }
	{ lwadd_na r15, r16, 5 ; andi r5, r6, 5 }
	{ lwadd_na r15, r16, 5 ; maxib_u r5, r6, 5 }
	{ lwadd_na r15, r16, 5 ; mulhhsa_uu r5, r6, r7 }
	{ lwadd_na r15, r16, 5 ; mzb r5, r6, r7 }
	{ lwadd_na r15, r16, 5 ; sadh r5, r6, r7 }
	{ lwadd_na r15, r16, 5 ; slt r5, r6, r7 }
	{ lwadd_na r15, r16, 5 ; sneh r5, r6, r7 }
	{ maxb_u r15, r16, r17 ; addb r5, r6, r7 }
	{ maxb_u r15, r16, r17 ; crc32_32 r5, r6, r7 }
	{ maxb_u r15, r16, r17 ; mnz r5, r6, r7 }
	{ maxb_u r15, r16, r17 ; mulhla_us r5, r6, r7 }
	{ maxb_u r15, r16, r17 ; packhb r5, r6, r7 }
	{ maxb_u r15, r16, r17 ; seqih r5, r6, 5 }
	{ maxb_u r15, r16, r17 ; slteb_u r5, r6, r7 }
	{ maxb_u r15, r16, r17 ; sub r5, r6, r7 }
	{ maxb_u r5, r6, r7 ; addli r15, r16, 0x1234 }
	{ maxb_u r5, r6, r7 ; jalr r15 }
	{ maxb_u r5, r6, r7 ; maxih r15, r16, 5 }
	{ maxb_u r5, r6, r7 ; nor r15, r16, r17 }
	{ maxb_u r5, r6, r7 ; seqib r15, r16, 5 }
	{ maxb_u r5, r6, r7 ; slte r15, r16, r17 }
	{ maxb_u r5, r6, r7 ; srai r15, r16, 5 }
	{ maxh r15, r16, r17 ; addi r5, r6, 5 }
	{ maxh r15, r16, r17 ; fnop }
	{ maxh r15, r16, r17 ; movei r5, 5 }
	{ maxh r15, r16, r17 ; mulll_su r5, r6, r7 }
	{ maxh r15, r16, r17 ; rl r5, r6, r7 }
	{ maxh r15, r16, r17 ; shli r5, r6, 5 }
	{ maxh r15, r16, r17 ; slth_u r5, r6, r7 }
	{ maxh r15, r16, r17 ; subhs r5, r6, r7 }
	{ maxh r5, r6, r7 ; andi r15, r16, 5 }
	{ maxh r5, r6, r7 ; lb r15, r16 }
	{ maxh r5, r6, r7 ; minh r15, r16, r17 }
	{ maxh r5, r6, r7 ; packhb r15, r16, r17 }
	{ maxh r5, r6, r7 ; shl r15, r16, r17 }
	{ maxh r5, r6, r7 ; slteh r15, r16, r17 }
	{ maxh r5, r6, r7 ; subb r15, r16, r17 }
	{ maxib_u r15, r16, 5 ; addlis r5, r6, 0x1234 }
	{ maxib_u r15, r16, 5 ; inthh r5, r6, r7 }
	{ maxib_u r15, r16, 5 ; mulhh_su r5, r6, r7 }
	{ maxib_u r15, r16, 5 ; mullla_uu r5, r6, r7 }
	{ maxib_u r15, r16, 5 ; s3a r5, r6, r7 }
	{ maxib_u r15, r16, 5 ; shrb r5, r6, r7 }
	{ maxib_u r15, r16, 5 ; sltib_u r5, r6, 5 }
	{ maxib_u r15, r16, 5 ; tblidxb2 r5, r6 }
	{ maxib_u r5, r6, 5 ; flush r15 }
	{ maxib_u r5, r6, 5 ; lh r15, r16 }
	{ maxib_u r5, r6, 5 ; mnz r15, r16, r17 }
	{ maxib_u r5, r6, 5 ; raise }
	{ maxib_u r5, r6, 5 ; shlib r15, r16, 5 }
	{ maxib_u r5, r6, 5 ; slti r15, r16, 5 }
	{ maxib_u r5, r6, 5 ; subs r15, r16, r17 }
	{ maxih r15, r16, 5 ; and r5, r6, r7 }
	{ maxih r15, r16, 5 ; maxh r5, r6, r7 }
	{ maxih r15, r16, 5 ; mulhha_uu r5, r6, r7 }
	{ maxih r15, r16, 5 ; mz r5, r6, r7 }
	{ maxih r15, r16, 5 ; sadb_u r5, r6, r7 }
	{ maxih r15, r16, 5 ; shrih r5, r6, 5 }
	{ maxih r15, r16, 5 ; sneb r5, r6, r7 }
	{ maxih r5, r6, 5 ; add r15, r16, r17 }
	{ maxih r5, r6, 5 ; info 19 }
	{ maxih r5, r6, 5 ; lnk r15 }
	{ maxih r5, r6, 5 ; movei r15, 5 }
	{ maxih r5, r6, 5 ; s2a r15, r16, r17 }
	{ maxih r5, r6, 5 ; shrh r15, r16, r17 }
	{ maxih r5, r6, 5 ; sltih r15, r16, 5 }
	{ maxih r5, r6, 5 ; wh64 r15 }
	{ mf ; avgh r5, r6, r7 }
	{ mf ; minh r5, r6, r7 }
	{ mf ; mulhl_us r5, r6, r7 }
	{ mf ; nor r5, r6, r7 }
	{ mf ; seqb r5, r6, r7 }
	{ mf ; sltb_u r5, r6, r7 }
	{ mf ; srah r5, r6, r7 }
	{ mfspr r16, 0x5 ; addhs r5, r6, r7 }
	{ mfspr r16, 0x5 ; dword_align r5, r6, r7 }
	{ mfspr r16, 0x5 ; move r5, r6 }
	{ mfspr r16, 0x5 ; mulll_ss r5, r6, r7 }
	{ mfspr r16, 0x5 ; pcnt r5, r6 }
	{ mfspr r16, 0x5 ; shlh r5, r6, r7 }
	{ mfspr r16, 0x5 ; slth r5, r6, r7 }
	{ mfspr r16, 0x5 ; subh r5, r6, r7 }
	{ minb_u r15, r16, r17 ; adiffb_u r5, r6, r7 }
	{ minb_u r15, r16, r17 ; intlh r5, r6, r7 }
	{ minb_u r15, r16, r17 ; mulhha_ss r5, r6, r7 }
	{ minb_u r15, r16, r17 ; mvnz r5, r6, r7 }
	{ minb_u r15, r16, r17 ; sadah r5, r6, r7 }
	{ minb_u r15, r16, r17 ; shri r5, r6, 5 }
	{ minb_u r15, r16, r17 ; sltih_u r5, r6, 5 }
	{ minb_u r15, r16, r17 ; xor r5, r6, r7 }
	{ minb_u r5, r6, r7 ; icoh r15 }
	{ minb_u r5, r6, r7 ; lhadd r15, r16, 5 }
	{ minb_u r5, r6, r7 ; mnzh r15, r16, r17 }
	{ minb_u r5, r6, r7 ; rli r15, r16, 5 }
	{ minb_u r5, r6, r7 ; shr r15, r16, r17 }
	{ minb_u r5, r6, r7 ; sltib r15, r16, 5 }
	{ minb_u r5, r6, r7 ; swadd r15, r16, 5 }
	{ minh r15, r16, r17 ; auli r5, r6, 0x1234 }
	{ minh r15, r16, r17 ; maxih r5, r6, 5 }
	{ minh r15, r16, r17 ; mulhl_ss r5, r6, r7 }
	{ minh r15, r16, r17 ; mzh r5, r6, r7 }
	{ minh r15, r16, r17 ; sadh_u r5, r6, r7 }
	{ minh r15, r16, r17 ; slt_u r5, r6, r7 }
	{ minh r15, r16, r17 ; sra r5, r6, r7 }
	{ minh r5, r6, r7 ; addbs_u r15, r16, r17 }
	{ minh r5, r6, r7 ; inthb r15, r16, r17 }
	{ minh r5, r6, r7 ; lw_na r15, r16 }
	{ minh r5, r6, r7 ; movelis r15, 0x1234 }
	{ minh r5, r6, r7 ; sb r15, r16 }
	{ minh r5, r6, r7 ; shrib r15, r16, 5 }
	{ minh r5, r6, r7 ; sne r15, r16, r17 }
	{ minh r5, r6, r7 ; xori r15, r16, 5 }
	{ minib_u r15, r16, 5 ; bytex r5, r6 }
	{ minib_u r15, r16, 5 ; minih r5, r6, 5 }
	{ minib_u r15, r16, 5 ; mulhla_ss r5, r6, r7 }
	{ minib_u r15, r16, 5 ; ori r5, r6, 5 }
	{ minib_u r15, r16, 5 ; seqi r5, r6, 5 }
	{ minib_u r15, r16, 5 ; slte_u r5, r6, r7 }
	{ minib_u r15, r16, 5 ; sraib r5, r6, 5 }
	{ minib_u r5, r6, 5 ; addib r15, r16, 5 }
	{ minib_u r5, r6, 5 ; inv r15 }
	{ minib_u r5, r6, 5 ; maxh r15, r16, r17 }
	{ minib_u r5, r6, 5 ; mzh r15, r16, r17 }
	{ minib_u r5, r6, 5 ; seqh r15, r16, r17 }
	{ minib_u r5, r6, 5 ; sltb r15, r16, r17 }
	{ minib_u r5, r6, 5 ; srab r15, r16, r17 }
	{ minih r15, r16, 5 ; addh r5, r6, r7 }
	{ minih r15, r16, 5 ; ctz r5, r6 }
	{ minih r15, r16, 5 ; mnzh r5, r6, r7 }
	{ minih r15, r16, 5 ; mulhlsa_uu r5, r6, r7 }
	{ minih r15, r16, 5 ; packlb r5, r6, r7 }
	{ minih r15, r16, 5 ; shlb r5, r6, r7 }
	{ minih r15, r16, 5 ; slteh_u r5, r6, r7 }
	{ minih r15, r16, 5 ; subbs_u r5, r6, r7 }
	{ minih r5, r6, 5 ; adds r15, r16, r17 }
	{ minih r5, r6, 5 ; jr r15 }
	{ minih r5, r6, 5 ; mfspr r16, 0x5 }
	{ minih r5, r6, 5 ; ori r15, r16, 5 }
	{ minih r5, r6, 5 ; sh r15, r16 }
	{ minih r5, r6, 5 ; slteb r15, r16, r17 }
	{ minih r5, r6, 5 ; sraih r15, r16, 5 }
	{ mm r15, r16, r17, 5, 7 ; addih r5, r6, 5 }
	{ mm r15, r16, r17, 5, 7 ; infol 0x1234 }
	{ mm r15, r16, r17, 5, 7 ; movelis r5, 0x1234 }
	{ mm r15, r16, r17, 5, 7 ; mullla_ss r5, r6, r7 }
	{ mm r15, r16, r17, 5, 7 ; s1a r5, r6, r7 }
	{ mm r15, r16, r17, 5, 7 ; shlih r5, r6, 5 }
	{ mm r15, r16, r17, 5, 7 ; slti_u r5, r6, 5 }
	{ mm r15, r16, r17, 5, 7 ; tblidxb0 r5, r6 }
	{ mm r5, r6, r7, 5, 7 ; dtlbpr r15 }
	{ mm r5, r6, r7, 5, 7 ; lbadd r15, r16, 5 }
	{ mm r5, r6, r7, 5, 7 ; minih r15, r16, 5 }
	{ mm r5, r6, r7, 5, 7 ; packlb r15, r16, r17 }
	{ mm r5, r6, r7, 5, 7 ; shlh r15, r16, r17 }
	{ mm r5, r6, r7, 5, 7 ; slth r15, r16, r17 }
	{ mm r5, r6, r7, 5, 7 ; subh r15, r16, r17 }
	{ mnz r15, r16, r17 ; addbs_u r5, r6, r7 }
	{ mnz r15, r16, r17 ; and r5, r6, r7 ; lb r25, r26 }
	{ mnz r15, r16, r17 ; auli r5, r6, 0x1234 }
	{ mnz r15, r16, r17 ; bytex r5, r6 ; sh r25, r26 }
	{ mnz r15, r16, r17 ; ctz r5, r6 ; prefetch r25 }
	{ mnz r15, r16, r17 ; info 19 ; lw r25, r26 }
	{ mnz r15, r16, r17 ; lb r25, r26 ; info 19 }
	{ mnz r15, r16, r17 ; lb r25, r26 ; pcnt r5, r6 }
	{ mnz r15, r16, r17 ; lb r25, r26 ; srai r5, r6, 5 }
	{ mnz r15, r16, r17 ; lb_u r25, r26 ; movei r5, 5 }
	{ mnz r15, r16, r17 ; lb_u r25, r26 ; s1a r5, r6, r7 }
	{ mnz r15, r16, r17 ; lb_u r25, r26 ; tblidxb1 r5, r6 }
	{ mnz r15, r16, r17 ; lh r25, r26 ; mulhha_ss r5, r6, r7 }
	{ mnz r15, r16, r17 ; lh r25, r26 ; seq r5, r6, r7 }
	{ mnz r15, r16, r17 ; lh r25, r26 ; xor r5, r6, r7 }
	{ mnz r15, r16, r17 ; lh_u r25, r26 ; mulll_ss r5, r6, r7 }
	{ mnz r15, r16, r17 ; lh_u r25, r26 ; shli r5, r6, 5 }
	{ mnz r15, r16, r17 ; lw r25, r26 ; addi r5, r6, 5 }
	{ mnz r15, r16, r17 ; lw r25, r26 ; mullla_uu r5, r6, r7 }
	{ mnz r15, r16, r17 ; lw r25, r26 ; slt r5, r6, r7 }
	{ mnz r15, r16, r17 ; minb_u r5, r6, r7 }
	{ mnz r15, r16, r17 ; move r5, r6 ; lh_u r25, r26 }
	{ mnz r15, r16, r17 ; mulhh_ss r5, r6, r7 ; lb_u r25, r26 }
	{ mnz r15, r16, r17 ; mulhha_ss r5, r6, r7 ; lb r25, r26 }
	{ mnz r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ mnz r15, r16, r17 ; mulll_ss r5, r6, r7 ; lb r25, r26 }
	{ mnz r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ mnz r15, r16, r17 ; mullla_uu r5, r6, r7 ; sw r25, r26 }
	{ mnz r15, r16, r17 ; mvz r5, r6, r7 ; sh r25, r26 }
	{ mnz r15, r16, r17 ; nop ; prefetch r25 }
	{ mnz r15, r16, r17 ; or r5, r6, r7 ; prefetch r25 }
	{ mnz r15, r16, r17 ; pcnt r5, r6 ; lb_u r25, r26 }
	{ mnz r15, r16, r17 ; prefetch r25 ; move r5, r6 }
	{ mnz r15, r16, r17 ; prefetch r25 ; rli r5, r6, 5 }
	{ mnz r15, r16, r17 ; prefetch r25 ; tblidxb0 r5, r6 }
	{ mnz r15, r16, r17 ; rli r5, r6, 5 ; lw r25, r26 }
	{ mnz r15, r16, r17 ; s2a r5, r6, r7 ; lw r25, r26 }
	{ mnz r15, r16, r17 ; sadh r5, r6, r7 }
	{ mnz r15, r16, r17 ; sb r25, r26 ; mulll_ss r5, r6, r7 }
	{ mnz r15, r16, r17 ; sb r25, r26 ; shli r5, r6, 5 }
	{ mnz r15, r16, r17 ; seq r5, r6, r7 ; lb_u r25, r26 }
	{ mnz r15, r16, r17 ; seqi r5, r6, 5 }
	{ mnz r15, r16, r17 ; sh r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ mnz r15, r16, r17 ; sh r25, r26 ; shl r5, r6, r7 }
	{ mnz r15, r16, r17 ; shl r5, r6, r7 ; lb r25, r26 }
	{ mnz r15, r16, r17 ; shli r5, r6, 5 ; sw r25, r26 }
	{ mnz r15, r16, r17 ; shri r5, r6, 5 ; lw r25, r26 }
	{ mnz r15, r16, r17 ; slt_u r5, r6, r7 ; lh r25, r26 }
	{ mnz r15, r16, r17 ; slte_u r5, r6, r7 ; lb r25, r26 }
	{ mnz r15, r16, r17 ; slti r5, r6, 5 ; lw r25, r26 }
	{ mnz r15, r16, r17 ; sne r5, r6, r7 ; lb r25, r26 }
	{ mnz r15, r16, r17 ; sra r5, r6, r7 ; sw r25, r26 }
	{ mnz r15, r16, r17 ; sub r5, r6, r7 ; lw r25, r26 }
	{ mnz r15, r16, r17 ; sw r25, r26 ; info 19 }
	{ mnz r15, r16, r17 ; sw r25, r26 ; pcnt r5, r6 }
	{ mnz r15, r16, r17 ; sw r25, r26 ; srai r5, r6, 5 }
	{ mnz r15, r16, r17 ; tblidxb1 r5, r6 ; lh r25, r26 }
	{ mnz r15, r16, r17 ; tblidxb3 r5, r6 ; lh r25, r26 }
	{ mnz r5, r6, r7 ; add r15, r16, r17 ; lb_u r25, r26 }
	{ mnz r5, r6, r7 ; addi r15, r16, 5 ; sh r25, r26 }
	{ mnz r5, r6, r7 ; andi r15, r16, 5 ; lh r25, r26 }
	{ mnz r5, r6, r7 ; fnop ; sw r25, r26 }
	{ mnz r5, r6, r7 ; info 19 ; sh r25, r26 }
	{ mnz r5, r6, r7 ; lb r25, r26 ; ill }
	{ mnz r5, r6, r7 ; lb r25, r26 ; shri r15, r16, 5 }
	{ mnz r5, r6, r7 ; lb_u r25, r26 ; info 19 }
	{ mnz r5, r6, r7 ; lb_u r25, r26 ; slt r15, r16, r17 }
	{ mnz r5, r6, r7 ; lh r25, r26 ; ill }
	{ mnz r5, r6, r7 ; lh r25, r26 ; shri r15, r16, 5 }
	{ mnz r5, r6, r7 ; lh_u r25, r26 ; info 19 }
	{ mnz r5, r6, r7 ; lh_u r25, r26 ; slt r15, r16, r17 }
	{ mnz r5, r6, r7 ; lw r25, r26 ; fnop }
	{ mnz r5, r6, r7 ; lw r25, r26 ; shr r15, r16, r17 }
	{ mnz r5, r6, r7 ; maxih r15, r16, 5 }
	{ mnz r5, r6, r7 ; move r15, r16 ; lb r25, r26 }
	{ mnz r5, r6, r7 ; moveli r15, 0x1234 }
	{ mnz r5, r6, r7 ; nop ; prefetch r25 }
	{ mnz r5, r6, r7 ; or r15, r16, r17 ; prefetch r25 }
	{ mnz r5, r6, r7 ; prefetch r25 ; add r15, r16, r17 }
	{ mnz r5, r6, r7 ; prefetch r25 ; seq r15, r16, r17 }
	{ mnz r5, r6, r7 ; rl r15, r16, r17 ; lb_u r25, r26 }
	{ mnz r5, r6, r7 ; s1a r15, r16, r17 ; lb_u r25, r26 }
	{ mnz r5, r6, r7 ; s3a r15, r16, r17 ; lb_u r25, r26 }
	{ mnz r5, r6, r7 ; sb r25, r26 ; mz r15, r16, r17 }
	{ mnz r5, r6, r7 ; sb r25, r26 ; slti r15, r16, 5 }
	{ mnz r5, r6, r7 ; seqh r15, r16, r17 }
	{ mnz r5, r6, r7 ; sh r25, r26 ; info 19 }
	{ mnz r5, r6, r7 ; sh r25, r26 ; slt r15, r16, r17 }
	{ mnz r5, r6, r7 ; shl r15, r16, r17 ; sh r25, r26 }
	{ mnz r5, r6, r7 ; shr r15, r16, r17 ; lh_u r25, r26 }
	{ mnz r5, r6, r7 ; shrih r15, r16, 5 }
	{ mnz r5, r6, r7 ; slt_u r15, r16, r17 }
	{ mnz r5, r6, r7 ; slte_u r15, r16, r17 ; sh r25, r26 }
	{ mnz r5, r6, r7 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
	{ mnz r5, r6, r7 ; sne r15, r16, r17 ; sh r25, r26 }
	{ mnz r5, r6, r7 ; srai r15, r16, 5 ; lh_u r25, r26 }
	{ mnz r5, r6, r7 ; subbs_u r15, r16, r17 }
	{ mnz r5, r6, r7 ; sw r25, r26 ; rl r15, r16, r17 }
	{ mnz r5, r6, r7 ; sw r25, r26 ; sub r15, r16, r17 }
	{ mnzb r15, r16, r17 ; addh r5, r6, r7 }
	{ mnzb r15, r16, r17 ; ctz r5, r6 }
	{ mnzb r15, r16, r17 ; mnzh r5, r6, r7 }
	{ mnzb r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ mnzb r15, r16, r17 ; packlb r5, r6, r7 }
	{ mnzb r15, r16, r17 ; shlb r5, r6, r7 }
	{ mnzb r15, r16, r17 ; slteh_u r5, r6, r7 }
	{ mnzb r15, r16, r17 ; subbs_u r5, r6, r7 }
	{ mnzb r5, r6, r7 ; adds r15, r16, r17 }
	{ mnzb r5, r6, r7 ; jr r15 }
	{ mnzb r5, r6, r7 ; mfspr r16, 0x5 }
	{ mnzb r5, r6, r7 ; ori r15, r16, 5 }
	{ mnzb r5, r6, r7 ; sh r15, r16 }
	{ mnzb r5, r6, r7 ; slteb r15, r16, r17 }
	{ mnzb r5, r6, r7 ; sraih r15, r16, 5 }
	{ mnzh r15, r16, r17 ; addih r5, r6, 5 }
	{ mnzh r15, r16, r17 ; infol 0x1234 }
	{ mnzh r15, r16, r17 ; movelis r5, 0x1234 }
	{ mnzh r15, r16, r17 ; mullla_ss r5, r6, r7 }
	{ mnzh r15, r16, r17 ; s1a r5, r6, r7 }
	{ mnzh r15, r16, r17 ; shlih r5, r6, 5 }
	{ mnzh r15, r16, r17 ; slti_u r5, r6, 5 }
	{ mnzh r15, r16, r17 ; tblidxb0 r5, r6 }
	{ mnzh r5, r6, r7 ; dtlbpr r15 }
	{ mnzh r5, r6, r7 ; lbadd r15, r16, 5 }
	{ mnzh r5, r6, r7 ; minih r15, r16, 5 }
	{ mnzh r5, r6, r7 ; packlb r15, r16, r17 }
	{ mnzh r5, r6, r7 ; shlh r15, r16, r17 }
	{ mnzh r5, r6, r7 ; slth r15, r16, r17 }
	{ mnzh r5, r6, r7 ; subh r15, r16, r17 }
	{ move r15, r16 ; addbs_u r5, r6, r7 }
	{ move r15, r16 ; and r5, r6, r7 ; lb r25, r26 }
	{ move r15, r16 ; auli r5, r6, 0x1234 }
	{ move r15, r16 ; bytex r5, r6 ; sh r25, r26 }
	{ move r15, r16 ; ctz r5, r6 ; prefetch r25 }
	{ move r15, r16 ; info 19 ; lw r25, r26 }
	{ move r15, r16 ; lb r25, r26 ; info 19 }
	{ move r15, r16 ; lb r25, r26 ; pcnt r5, r6 }
	{ move r15, r16 ; lb r25, r26 ; srai r5, r6, 5 }
	{ move r15, r16 ; lb_u r25, r26 ; movei r5, 5 }
	{ move r15, r16 ; lb_u r25, r26 ; s1a r5, r6, r7 }
	{ move r15, r16 ; lb_u r25, r26 ; tblidxb1 r5, r6 }
	{ move r15, r16 ; lh r25, r26 ; mulhha_ss r5, r6, r7 }
	{ move r15, r16 ; lh r25, r26 ; seq r5, r6, r7 }
	{ move r15, r16 ; lh r25, r26 ; xor r5, r6, r7 }
	{ move r15, r16 ; lh_u r25, r26 ; mulll_ss r5, r6, r7 }
	{ move r15, r16 ; lh_u r25, r26 ; shli r5, r6, 5 }
	{ move r15, r16 ; lw r25, r26 ; addi r5, r6, 5 }
	{ move r15, r16 ; lw r25, r26 ; mullla_uu r5, r6, r7 }
	{ move r15, r16 ; lw r25, r26 ; slt r5, r6, r7 }
	{ move r15, r16 ; minb_u r5, r6, r7 }
	{ move r15, r16 ; move r5, r6 ; lh_u r25, r26 }
	{ move r15, r16 ; mulhh_ss r5, r6, r7 ; lb_u r25, r26 }
	{ move r15, r16 ; mulhha_ss r5, r6, r7 ; lb r25, r26 }
	{ move r15, r16 ; mulhha_uu r5, r6, r7 }
	{ move r15, r16 ; mulll_ss r5, r6, r7 ; lb r25, r26 }
	{ move r15, r16 ; mulll_uu r5, r6, r7 }
	{ move r15, r16 ; mullla_uu r5, r6, r7 ; sw r25, r26 }
	{ move r15, r16 ; mvz r5, r6, r7 ; sh r25, r26 }
	{ move r15, r16 ; nop ; prefetch r25 }
	{ move r15, r16 ; or r5, r6, r7 ; prefetch r25 }
	{ move r15, r16 ; pcnt r5, r6 ; lb_u r25, r26 }
	{ move r15, r16 ; prefetch r25 ; move r5, r6 }
	{ move r15, r16 ; prefetch r25 ; rli r5, r6, 5 }
	{ move r15, r16 ; prefetch r25 ; tblidxb0 r5, r6 }
	{ move r15, r16 ; rli r5, r6, 5 ; lw r25, r26 }
	{ move r15, r16 ; s2a r5, r6, r7 ; lw r25, r26 }
	{ move r15, r16 ; sadh r5, r6, r7 }
	{ move r15, r16 ; sb r25, r26 ; mulll_ss r5, r6, r7 }
	{ move r15, r16 ; sb r25, r26 ; shli r5, r6, 5 }
	{ move r15, r16 ; seq r5, r6, r7 ; lb_u r25, r26 }
	{ move r15, r16 ; seqi r5, r6, 5 }
	{ move r15, r16 ; sh r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ move r15, r16 ; sh r25, r26 ; shl r5, r6, r7 }
	{ move r15, r16 ; shl r5, r6, r7 ; lb r25, r26 }
	{ move r15, r16 ; shli r5, r6, 5 ; sw r25, r26 }
	{ move r15, r16 ; shri r5, r6, 5 ; lw r25, r26 }
	{ move r15, r16 ; slt_u r5, r6, r7 ; lh r25, r26 }
	{ move r15, r16 ; slte_u r5, r6, r7 ; lb r25, r26 }
	{ move r15, r16 ; slti r5, r6, 5 ; lw r25, r26 }
	{ move r15, r16 ; sne r5, r6, r7 ; lb r25, r26 }
	{ move r15, r16 ; sra r5, r6, r7 ; sw r25, r26 }
	{ move r15, r16 ; sub r5, r6, r7 ; lw r25, r26 }
	{ move r15, r16 ; sw r25, r26 ; info 19 }
	{ move r15, r16 ; sw r25, r26 ; pcnt r5, r6 }
	{ move r15, r16 ; sw r25, r26 ; srai r5, r6, 5 }
	{ move r15, r16 ; tblidxb1 r5, r6 ; lh r25, r26 }
	{ move r15, r16 ; tblidxb3 r5, r6 ; lh r25, r26 }
	{ move r5, r6 ; add r15, r16, r17 ; lb_u r25, r26 }
	{ move r5, r6 ; addi r15, r16, 5 ; sh r25, r26 }
	{ move r5, r6 ; andi r15, r16, 5 ; lh r25, r26 }
	{ move r5, r6 ; fnop ; sw r25, r26 }
	{ move r5, r6 ; info 19 ; sh r25, r26 }
	{ move r5, r6 ; lb r25, r26 ; ill }
	{ move r5, r6 ; lb r25, r26 ; shri r15, r16, 5 }
	{ move r5, r6 ; lb_u r25, r26 ; info 19 }
	{ move r5, r6 ; lb_u r25, r26 ; slt r15, r16, r17 }
	{ move r5, r6 ; lh r25, r26 ; ill }
	{ move r5, r6 ; lh r25, r26 ; shri r15, r16, 5 }
	{ move r5, r6 ; lh_u r25, r26 ; info 19 }
	{ move r5, r6 ; lh_u r25, r26 ; slt r15, r16, r17 }
	{ move r5, r6 ; lw r25, r26 ; fnop }
	{ move r5, r6 ; lw r25, r26 ; shr r15, r16, r17 }
	{ move r5, r6 ; maxih r15, r16, 5 }
	{ move r5, r6 ; move r15, r16 ; lb r25, r26 }
	{ move r5, r6 ; moveli r15, 0x1234 }
	{ move r5, r6 ; nop ; prefetch r25 }
	{ move r5, r6 ; or r15, r16, r17 ; prefetch r25 }
	{ move r5, r6 ; prefetch r25 ; add r15, r16, r17 }
	{ move r5, r6 ; prefetch r25 ; seq r15, r16, r17 }
	{ move r5, r6 ; rl r15, r16, r17 ; lb_u r25, r26 }
	{ move r5, r6 ; s1a r15, r16, r17 ; lb_u r25, r26 }
	{ move r5, r6 ; s3a r15, r16, r17 ; lb_u r25, r26 }
	{ move r5, r6 ; sb r25, r26 ; mz r15, r16, r17 }
	{ move r5, r6 ; sb r25, r26 ; slti r15, r16, 5 }
	{ move r5, r6 ; seqh r15, r16, r17 }
	{ move r5, r6 ; sh r25, r26 ; info 19 }
	{ move r5, r6 ; sh r25, r26 ; slt r15, r16, r17 }
	{ move r5, r6 ; shl r15, r16, r17 ; sh r25, r26 }
	{ move r5, r6 ; shr r15, r16, r17 ; lh_u r25, r26 }
	{ move r5, r6 ; shrih r15, r16, 5 }
	{ move r5, r6 ; slt_u r15, r16, r17 }
	{ move r5, r6 ; slte_u r15, r16, r17 ; sh r25, r26 }
	{ move r5, r6 ; slti_u r15, r16, 5 ; lb_u r25, r26 }
	{ move r5, r6 ; sne r15, r16, r17 ; sh r25, r26 }
	{ move r5, r6 ; srai r15, r16, 5 ; lh_u r25, r26 }
	{ move r5, r6 ; subbs_u r15, r16, r17 }
	{ move r5, r6 ; sw r25, r26 ; rl r15, r16, r17 }
	{ move r5, r6 ; sw r25, r26 ; sub r15, r16, r17 }
	{ movei r15, 5 ; add r5, r6, r7 ; lh_u r25, r26 }
	{ movei r15, 5 ; addi r5, r6, 5 }
	{ movei r15, 5 ; andi r5, r6, 5 ; lh r25, r26 }
	{ movei r15, 5 ; bitx r5, r6 }
	{ movei r15, 5 ; clz r5, r6 }
	{ movei r15, 5 ; fnop ; sb r25, r26 }
	{ movei r15, 5 ; lb r25, r26 ; addi r5, r6, 5 }
	{ movei r15, 5 ; lb r25, r26 ; mullla_uu r5, r6, r7 }
	{ movei r15, 5 ; lb r25, r26 ; slt r5, r6, r7 }
	{ movei r15, 5 ; lb_u r25, r26 ; bitx r5, r6 }
	{ movei r15, 5 ; lb_u r25, r26 ; mz r5, r6, r7 }
	{ movei r15, 5 ; lb_u r25, r26 ; slte_u r5, r6, r7 }
	{ movei r15, 5 ; lh r25, r26 ; ctz r5, r6 }
	{ movei r15, 5 ; lh r25, r26 ; or r5, r6, r7 }
	{ movei r15, 5 ; lh r25, r26 ; sne r5, r6, r7 }
	{ movei r15, 5 ; lh_u r25, r26 ; mnz r5, r6, r7 }
	{ movei r15, 5 ; lh_u r25, r26 ; rl r5, r6, r7 }
	{ movei r15, 5 ; lh_u r25, r26 ; sub r5, r6, r7 }
	{ movei r15, 5 ; lw r25, r26 ; mulhh_ss r5, r6, r7 }
	{ movei r15, 5 ; lw r25, r26 ; s2a r5, r6, r7 }
	{ movei r15, 5 ; lw r25, r26 ; tblidxb2 r5, r6 }
	{ movei r15, 5 ; mnz r5, r6, r7 ; sh r25, r26 }
	{ movei r15, 5 ; movei r5, 5 ; prefetch r25 }
	{ movei r15, 5 ; mulhh_uu r5, r6, r7 ; lh r25, r26 }
	{ movei r15, 5 ; mulhha_uu r5, r6, r7 ; lb_u r25, r26 }
	{ movei r15, 5 ; mulhlsa_uu r5, r6, r7 ; lh r25, r26 }
	{ movei r15, 5 ; mulll_uu r5, r6, r7 ; lb_u r25, r26 }
	{ movei r15, 5 ; mullla_uu r5, r6, r7 ; lb r25, r26 }
	{ movei r15, 5 ; mvnz r5, r6, r7 }
	{ movei r15, 5 ; mz r5, r6, r7 }
	{ movei r15, 5 ; nor r5, r6, r7 ; sh r25, r26 }
	{ movei r15, 5 ; ori r5, r6, 5 ; sh r25, r26 }
	{ movei r15, 5 ; prefetch r25 ; andi r5, r6, 5 }
	{ movei r15, 5 ; prefetch r25 ; mvz r5, r6, r7 }
	{ movei r15, 5 ; prefetch r25 ; slte r5, r6, r7 }
	{ movei r15, 5 ; rl r5, r6, r7 ; sb r25, r26 }
	{ movei r15, 5 ; s1a r5, r6, r7 ; sb r25, r26 }
	{ movei r15, 5 ; s3a r5, r6, r7 ; sb r25, r26 }
	{ movei r15, 5 ; sb r25, r26 ; mnz r5, r6, r7 }
	{ movei r15, 5 ; sb r25, r26 ; rl r5, r6, r7 }
	{ movei r15, 5 ; sb r25, r26 ; sub r5, r6, r7 }
	{ movei r15, 5 ; seqi r5, r6, 5 ; lb_u r25, r26 }
	{ movei r15, 5 ; sh r25, r26 ; info 19 }
	{ movei r15, 5 ; sh r25, r26 ; pcnt r5, r6 }
	{ movei r15, 5 ; sh r25, r26 ; srai r5, r6, 5 }
	{ movei r15, 5 ; shli r5, r6, 5 ; lb r25, r26 }
	{ movei r15, 5 ; shr r5, r6, r7 ; sw r25, r26 }
	{ movei r15, 5 ; slt r5, r6, r7 ; lw r25, r26 }
	{ movei r15, 5 ; slte r5, r6, r7 ; lh r25, r26 }
	{ movei r15, 5 ; slteh r5, r6, r7 }
	{ movei r15, 5 ; slti_u r5, r6, 5 ; sb r25, r26 }
	{ movei r15, 5 ; sra r5, r6, r7 ; lb r25, r26 }
	{ movei r15, 5 ; srai r5, r6, 5 ; sw r25, r26 }
	{ movei r15, 5 ; sw r25, r26 ; addi r5, r6, 5 }
	{ movei r15, 5 ; sw r25, r26 ; mullla_uu r5, r6, r7 }
	{ movei r15, 5 ; sw r25, r26 ; slt r5, r6, r7 }
	{ movei r15, 5 ; tblidxb0 r5, r6 ; lw r25, r26 }
	{ movei r15, 5 ; tblidxb2 r5, r6 ; lw r25, r26 }
	{ movei r15, 5 ; xor r5, r6, r7 ; lw r25, r26 }
	{ movei r5, 5 ; addhs r15, r16, r17 }
	{ movei r5, 5 ; and r15, r16, r17 ; lw r25, r26 }
	{ movei r5, 5 ; fnop ; lb r25, r26 }
	{ movei r5, 5 ; ill }
	{ movei r5, 5 ; jr r15 }
	{ movei r5, 5 ; lb r25, r26 ; s1a r15, r16, r17 }
	{ movei r5, 5 ; lb r25, r26 }
	{ movei r5, 5 ; lb_u r25, r26 ; s2a r15, r16, r17 }
	{ movei r5, 5 ; lbadd r15, r16, 5 }
	{ movei r5, 5 ; lh r25, r26 ; s1a r15, r16, r17 }
	{ movei r5, 5 ; lh r25, r26 }
	{ movei r5, 5 ; lh_u r25, r26 ; s2a r15, r16, r17 }
	{ movei r5, 5 ; lhadd r15, r16, 5 }
	{ movei r5, 5 ; lw r25, r26 ; rli r15, r16, 5 }
	{ movei r5, 5 ; lw r25, r26 ; xor r15, r16, r17 }
	{ movei r5, 5 ; mnz r15, r16, r17 ; lw r25, r26 }
	{ movei r5, 5 ; movei r15, 5 ; lh r25, r26 }
	{ movei r5, 5 ; mz r15, r16, r17 }
	{ movei r5, 5 ; nor r15, r16, r17 ; sh r25, r26 }
	{ movei r5, 5 ; ori r15, r16, 5 ; sh r25, r26 }
	{ movei r5, 5 ; prefetch r25 ; nor r15, r16, r17 }
	{ movei r5, 5 ; prefetch r25 ; sne r15, r16, r17 }
	{ movei r5, 5 ; rli r15, r16, 5 ; lh_u r25, r26 }
	{ movei r5, 5 ; s2a r15, r16, r17 ; lh_u r25, r26 }
	{ movei r5, 5 ; sb r25, r26 ; and r15, r16, r17 }
	{ movei r5, 5 ; sb r25, r26 ; shl r15, r16, r17 }
	{ movei r5, 5 ; seq r15, r16, r17 ; lh_u r25, r26 }
	{ movei r5, 5 ; seqih r15, r16, 5 }
	{ movei r5, 5 ; sh r25, r26 ; s2a r15, r16, r17 }
	{ movei r5, 5 ; shadd r15, r16, 5 }
	{ movei r5, 5 ; shli r15, r16, 5 ; sh r25, r26 }
	{ movei r5, 5 ; shri r15, r16, 5 ; lh_u r25, r26 }
	{ movei r5, 5 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
	{ movei r5, 5 ; slte r15, r16, r17 }
	{ movei r5, 5 ; slti r15, r16, 5 ; lh_u r25, r26 }
	{ movei r5, 5 ; sltih_u r15, r16, 5 }
	{ movei r5, 5 ; sra r15, r16, r17 ; sh r25, r26 }
	{ movei r5, 5 ; sub r15, r16, r17 ; lh_u r25, r26 }
	{ movei r5, 5 ; sw r25, r26 ; mnz r15, r16, r17 }
	{ movei r5, 5 ; sw r25, r26 ; slt_u r15, r16, r17 }
	{ movei r5, 5 ; xor r15, r16, r17 ; sb r25, r26 }
	{ moveli r15, 0x1234 ; auli r5, r6, 0x1234 }
	{ moveli r15, 0x1234 ; maxih r5, r6, 5 }
	{ moveli r15, 0x1234 ; mulhl_ss r5, r6, r7 }
	{ moveli r15, 0x1234 ; mzh r5, r6, r7 }
	{ moveli r15, 0x1234 ; sadh_u r5, r6, r7 }
	{ moveli r15, 0x1234 ; slt_u r5, r6, r7 }
	{ moveli r15, 0x1234 ; sra r5, r6, r7 }
	{ moveli r5, 0x1234 ; addbs_u r15, r16, r17 }
	{ moveli r5, 0x1234 ; inthb r15, r16, r17 }
	{ moveli r5, 0x1234 ; lw_na r15, r16 }
	{ moveli r5, 0x1234 ; movelis r15, 0x1234 }
	{ moveli r5, 0x1234 ; sb r15, r16 }
	{ moveli r5, 0x1234 ; shrib r15, r16, 5 }
	{ moveli r5, 0x1234 ; sne r15, r16, r17 }
	{ moveli r5, 0x1234 ; xori r15, r16, 5 }
	{ movelis r15, 0x1234 ; clz r5, r6 }
	{ movelis r15, 0x1234 ; mm r5, r6, r7, 5, 7 }
	{ movelis r15, 0x1234 ; mulhla_us r5, r6, r7 }
	{ movelis r15, 0x1234 ; packhb r5, r6, r7 }
	{ movelis r15, 0x1234 ; seqih r5, r6, 5 }
	{ movelis r15, 0x1234 ; slteb_u r5, r6, r7 }
	{ movelis r15, 0x1234 ; sub r5, r6, r7 }
	{ movelis r5, 0x1234 ; addli r15, r16, 0x1234 }
	{ movelis r5, 0x1234 ; jalrp r15 }
	{ movelis r5, 0x1234 ; mf }
	{ movelis r5, 0x1234 ; ori r15, r16, 5 }
	{ movelis r5, 0x1234 ; sh r15, r16 }
	{ movelis r5, 0x1234 ; slteb r15, r16, r17 }
	{ movelis r5, 0x1234 ; sraih r15, r16, 5 }
	{ mtspr 0x5, r16 ; addih r5, r6, 5 }
	{ mtspr 0x5, r16 ; infol 0x1234 }
	{ mtspr 0x5, r16 ; movelis r5, 0x1234 }
	{ mtspr 0x5, r16 ; mullla_ss r5, r6, r7 }
	{ mtspr 0x5, r16 ; s1a r5, r6, r7 }
	{ mtspr 0x5, r16 ; shlih r5, r6, 5 }
	{ mtspr 0x5, r16 ; slti_u r5, r6, 5 }
	{ mtspr 0x5, r16 ; tblidxb0 r5, r6 }
	{ mulhh_ss r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
	{ mulhh_ss r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
	{ mulhh_ss r5, r6, r7 ; fnop ; lb_u r25, r26 }
	{ mulhh_ss r5, r6, r7 ; info 19 ; lb r25, r26 }
	{ mulhh_ss r5, r6, r7 ; jrp r15 }
	{ mulhh_ss r5, r6, r7 ; lb r25, r26 ; s2a r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; lb_u r15, r16 }
	{ mulhh_ss r5, r6, r7 ; lb_u r25, r26 ; s3a r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; lbadd_u r15, r16, 5 }
	{ mulhh_ss r5, r6, r7 ; lh r25, r26 ; s2a r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; lh_u r15, r16 }
	{ mulhh_ss r5, r6, r7 ; lh_u r25, r26 ; s3a r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ mulhh_ss r5, r6, r7 ; lw r25, r26 ; s1a r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; lw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
	{ mulhh_ss r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
	{ mulhh_ss r5, r6, r7 ; mzb r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; prefetch r25 ; or r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; prefetch r25 ; sra r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; rli r15, r16, 5 ; lw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; sb r25, r26 ; andi r15, r16, 5 }
	{ mulhh_ss r5, r6, r7 ; sb r25, r26 ; shli r15, r16, 5 }
	{ mulhh_ss r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; sh r15, r16 }
	{ mulhh_ss r5, r6, r7 ; sh r25, r26 ; s3a r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
	{ mulhh_ss r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; slt_u r15, r16, r17 ; lh r25, r26 }
	{ mulhh_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lb r25, r26 }
	{ mulhh_ss r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
	{ mulhh_ss r5, r6, r7 ; sra r15, r16, r17 ; sw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
	{ mulhh_ss r5, r6, r7 ; sw r25, r26 ; move r15, r16 }
	{ mulhh_ss r5, r6, r7 ; sw r25, r26 ; slte r15, r16, r17 }
	{ mulhh_ss r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
	{ mulhh_su r5, r6, r7 ; flush r15 }
	{ mulhh_su r5, r6, r7 ; lh r15, r16 }
	{ mulhh_su r5, r6, r7 ; mnz r15, r16, r17 }
	{ mulhh_su r5, r6, r7 ; raise }
	{ mulhh_su r5, r6, r7 ; shlib r15, r16, 5 }
	{ mulhh_su r5, r6, r7 ; slti r15, r16, 5 }
	{ mulhh_su r5, r6, r7 ; subs r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; addhs r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; and r15, r16, r17 ; lw r25, r26 }
	{ mulhh_uu r5, r6, r7 ; fnop ; lb r25, r26 }
	{ mulhh_uu r5, r6, r7 ; ill }
	{ mulhh_uu r5, r6, r7 ; jr r15 }
	{ mulhh_uu r5, r6, r7 ; lb r25, r26 ; s1a r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; lb r25, r26 }
	{ mulhh_uu r5, r6, r7 ; lb_u r25, r26 ; s2a r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; lbadd r15, r16, 5 }
	{ mulhh_uu r5, r6, r7 ; lh r25, r26 ; s1a r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; lh r25, r26 }
	{ mulhh_uu r5, r6, r7 ; lh_u r25, r26 ; s2a r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; lhadd r15, r16, 5 }
	{ mulhh_uu r5, r6, r7 ; lw r25, r26 ; rli r15, r16, 5 }
	{ mulhh_uu r5, r6, r7 ; lw r25, r26 ; xor r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; mnz r15, r16, r17 ; lw r25, r26 }
	{ mulhh_uu r5, r6, r7 ; movei r15, 5 ; lh r25, r26 }
	{ mulhh_uu r5, r6, r7 ; mz r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; nor r15, r16, r17 ; sh r25, r26 }
	{ mulhh_uu r5, r6, r7 ; ori r15, r16, 5 ; sh r25, r26 }
	{ mulhh_uu r5, r6, r7 ; prefetch r25 ; nor r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; prefetch r25 ; sne r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; rli r15, r16, 5 ; lh_u r25, r26 }
	{ mulhh_uu r5, r6, r7 ; s2a r15, r16, r17 ; lh_u r25, r26 }
	{ mulhh_uu r5, r6, r7 ; sb r25, r26 ; and r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; sb r25, r26 ; shl r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; seq r15, r16, r17 ; lh_u r25, r26 }
	{ mulhh_uu r5, r6, r7 ; seqih r15, r16, 5 }
	{ mulhh_uu r5, r6, r7 ; sh r25, r26 ; s2a r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; shadd r15, r16, 5 }
	{ mulhh_uu r5, r6, r7 ; shli r15, r16, 5 ; sh r25, r26 }
	{ mulhh_uu r5, r6, r7 ; shri r15, r16, 5 ; lh_u r25, r26 }
	{ mulhh_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lb_u r25, r26 }
	{ mulhh_uu r5, r6, r7 ; slte r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; slti r15, r16, 5 ; lh_u r25, r26 }
	{ mulhh_uu r5, r6, r7 ; sltih_u r15, r16, 5 }
	{ mulhh_uu r5, r6, r7 ; sra r15, r16, r17 ; sh r25, r26 }
	{ mulhh_uu r5, r6, r7 ; sub r15, r16, r17 ; lh_u r25, r26 }
	{ mulhh_uu r5, r6, r7 ; sw r25, r26 ; mnz r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; sw r25, r26 ; slt_u r15, r16, r17 }
	{ mulhh_uu r5, r6, r7 ; xor r15, r16, r17 ; sb r25, r26 }
	{ mulhha_ss r5, r6, r7 ; addi r15, r16, 5 ; lb_u r25, r26 }
	{ mulhha_ss r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
	{ mulhha_ss r5, r6, r7 ; fnop ; lh r25, r26 }
	{ mulhha_ss r5, r6, r7 ; info 19 ; lb_u r25, r26 }
	{ mulhha_ss r5, r6, r7 ; lb r15, r16 }
	{ mulhha_ss r5, r6, r7 ; lb r25, r26 ; s3a r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; lb_u r25, r26 ; add r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; lb_u r25, r26 ; seq r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; lh r15, r16 }
	{ mulhha_ss r5, r6, r7 ; lh r25, r26 ; s3a r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; lh_u r25, r26 ; add r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; lh_u r25, r26 ; seq r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; lnk r15 }
	{ mulhha_ss r5, r6, r7 ; lw r25, r26 ; s2a r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; lw_na r15, r16 }
	{ mulhha_ss r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
	{ mulhha_ss r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
	{ mulhha_ss r5, r6, r7 ; mzh r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; nor r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; ori r15, r16, 5 }
	{ mulhha_ss r5, r6, r7 ; prefetch r25 ; ori r15, r16, 5 }
	{ mulhha_ss r5, r6, r7 ; prefetch r25 ; srai r15, r16, 5 }
	{ mulhha_ss r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
	{ mulhha_ss r5, r6, r7 ; s2a r15, r16, r17 ; prefetch r25 }
	{ mulhha_ss r5, r6, r7 ; sb r25, r26 ; fnop }
	{ mulhha_ss r5, r6, r7 ; sb r25, r26 ; shr r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; seq r15, r16, r17 ; prefetch r25 }
	{ mulhha_ss r5, r6, r7 ; sh r25, r26 ; add r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; sh r25, r26 ; seq r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; shl r15, r16, r17 ; lb_u r25, r26 }
	{ mulhha_ss r5, r6, r7 ; shli r15, r16, 5 }
	{ mulhha_ss r5, r6, r7 ; shri r15, r16, 5 ; prefetch r25 }
	{ mulhha_ss r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
	{ mulhha_ss r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
	{ mulhha_ss r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
	{ mulhha_ss r5, r6, r7 ; sne r15, r16, r17 ; lb_u r25, r26 }
	{ mulhha_ss r5, r6, r7 ; sra r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
	{ mulhha_ss r5, r6, r7 ; sw r25, r26 ; movei r15, 5 }
	{ mulhha_ss r5, r6, r7 ; sw r25, r26 ; slte_u r15, r16, r17 }
	{ mulhha_ss r5, r6, r7 ; xor r15, r16, r17 ; sw r25, r26 }
	{ mulhha_su r5, r6, r7 ; fnop }
	{ mulhha_su r5, r6, r7 ; lh_u r15, r16 }
	{ mulhha_su r5, r6, r7 ; mnzb r15, r16, r17 }
	{ mulhha_su r5, r6, r7 ; rl r15, r16, r17 }
	{ mulhha_su r5, r6, r7 ; shlih r15, r16, 5 }
	{ mulhha_su r5, r6, r7 ; slti_u r15, r16, 5 }
	{ mulhha_su r5, r6, r7 ; sw r15, r16 }
	{ mulhha_uu r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
	{ mulhha_uu r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
	{ mulhha_uu r5, r6, r7 ; fnop ; lb_u r25, r26 }
	{ mulhha_uu r5, r6, r7 ; info 19 ; lb r25, r26 }
	{ mulhha_uu r5, r6, r7 ; jrp r15 }
	{ mulhha_uu r5, r6, r7 ; lb r25, r26 ; s2a r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; lb_u r15, r16 }
	{ mulhha_uu r5, r6, r7 ; lb_u r25, r26 ; s3a r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; lbadd_u r15, r16, 5 }
	{ mulhha_uu r5, r6, r7 ; lh r25, r26 ; s2a r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; lh_u r15, r16 }
	{ mulhha_uu r5, r6, r7 ; lh_u r25, r26 ; s3a r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ mulhha_uu r5, r6, r7 ; lw r25, r26 ; s1a r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; lw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
	{ mulhha_uu r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
	{ mulhha_uu r5, r6, r7 ; mzb r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; prefetch r25 ; or r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; prefetch r25 ; sra r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; rli r15, r16, 5 ; lw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; sb r25, r26 ; andi r15, r16, 5 }
	{ mulhha_uu r5, r6, r7 ; sb r25, r26 ; shli r15, r16, 5 }
	{ mulhha_uu r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; sh r15, r16 }
	{ mulhha_uu r5, r6, r7 ; sh r25, r26 ; s3a r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
	{ mulhha_uu r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; slt_u r15, r16, r17 ; lh r25, r26 }
	{ mulhha_uu r5, r6, r7 ; slte_u r15, r16, r17 ; lb r25, r26 }
	{ mulhha_uu r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
	{ mulhha_uu r5, r6, r7 ; sra r15, r16, r17 ; sw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
	{ mulhha_uu r5, r6, r7 ; sw r25, r26 ; move r15, r16 }
	{ mulhha_uu r5, r6, r7 ; sw r25, r26 ; slte r15, r16, r17 }
	{ mulhha_uu r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
	{ mulhhsa_uu r5, r6, r7 ; flush r15 }
	{ mulhhsa_uu r5, r6, r7 ; lh r15, r16 }
	{ mulhhsa_uu r5, r6, r7 ; mnz r15, r16, r17 }
	{ mulhhsa_uu r5, r6, r7 ; raise }
	{ mulhhsa_uu r5, r6, r7 ; shlib r15, r16, 5 }
	{ mulhhsa_uu r5, r6, r7 ; slti r15, r16, 5 }
	{ mulhhsa_uu r5, r6, r7 ; subs r15, r16, r17 }
	{ mulhl_ss r5, r6, r7 ; auli r15, r16, 0x1234 }
	{ mulhl_ss r5, r6, r7 ; lb_u r15, r16 }
	{ mulhl_ss r5, r6, r7 ; minib_u r15, r16, 5 }
	{ mulhl_ss r5, r6, r7 ; packhs r15, r16, r17 }
	{ mulhl_ss r5, r6, r7 ; shlb r15, r16, r17 }
	{ mulhl_ss r5, r6, r7 ; slteh_u r15, r16, r17 }
	{ mulhl_ss r5, r6, r7 ; subbs_u r15, r16, r17 }
	{ mulhl_su r5, r6, r7 ; adds r15, r16, r17 }
	{ mulhl_su r5, r6, r7 ; jr r15 }
	{ mulhl_su r5, r6, r7 ; mfspr r16, 0x5 }
	{ mulhl_su r5, r6, r7 ; ori r15, r16, 5 }
	{ mulhl_su r5, r6, r7 ; sh r15, r16 }
	{ mulhl_su r5, r6, r7 ; slteb r15, r16, r17 }
	{ mulhl_su r5, r6, r7 ; sraih r15, r16, 5 }
	{ mulhl_us r5, r6, r7 ; addih r15, r16, 5 }
	{ mulhl_us r5, r6, r7 ; iret }
	{ mulhl_us r5, r6, r7 ; maxib_u r15, r16, 5 }
	{ mulhl_us r5, r6, r7 ; nop }
	{ mulhl_us r5, r6, r7 ; seqi r15, r16, 5 }
	{ mulhl_us r5, r6, r7 ; sltb_u r15, r16, r17 }
	{ mulhl_us r5, r6, r7 ; srah r15, r16, r17 }
	{ mulhl_uu r5, r6, r7 ; addhs r15, r16, r17 }
	{ mulhl_uu r5, r6, r7 ; intlb r15, r16, r17 }
	{ mulhl_uu r5, r6, r7 ; lwadd_na r15, r16, 5 }
	{ mulhl_uu r5, r6, r7 ; mz r15, r16, r17 }
	{ mulhl_uu r5, r6, r7 ; seq r15, r16, r17 }
	{ mulhl_uu r5, r6, r7 ; slt r15, r16, r17 }
	{ mulhl_uu r5, r6, r7 ; sneh r15, r16, r17 }
	{ mulhla_ss r5, r6, r7 ; addb r15, r16, r17 }
	{ mulhla_ss r5, r6, r7 ; infol 0x1234 }
	{ mulhla_ss r5, r6, r7 ; lw r15, r16 }
	{ mulhla_ss r5, r6, r7 ; moveli r15, 0x1234 }
	{ mulhla_ss r5, r6, r7 ; s3a r15, r16, r17 }
	{ mulhla_ss r5, r6, r7 ; shri r15, r16, 5 }
	{ mulhla_ss r5, r6, r7 ; sltih_u r15, r16, 5 }
	{ mulhla_ss r5, r6, r7 ; xor r15, r16, r17 }
	{ mulhla_su r5, r6, r7 ; icoh r15 }
	{ mulhla_su r5, r6, r7 ; lhadd r15, r16, 5 }
	{ mulhla_su r5, r6, r7 ; mnzh r15, r16, r17 }
	{ mulhla_su r5, r6, r7 ; rli r15, r16, 5 }
	{ mulhla_su r5, r6, r7 ; shr r15, r16, r17 }
	{ mulhla_su r5, r6, r7 ; sltib r15, r16, 5 }
	{ mulhla_su r5, r6, r7 ; swadd r15, r16, 5 }
	{ mulhla_us r5, r6, r7 ; finv r15 }
	{ mulhla_us r5, r6, r7 ; lbadd_u r15, r16, 5 }
	{ mulhla_us r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
	{ mulhla_us r5, r6, r7 ; prefetch r15 }
	{ mulhla_us r5, r6, r7 ; shli r15, r16, 5 }
	{ mulhla_us r5, r6, r7 ; slth_u r15, r16, r17 }
	{ mulhla_us r5, r6, r7 ; subhs r15, r16, r17 }
	{ mulhla_uu r5, r6, r7 ; andi r15, r16, 5 }
	{ mulhla_uu r5, r6, r7 ; lb r15, r16 }
	{ mulhla_uu r5, r6, r7 ; minh r15, r16, r17 }
	{ mulhla_uu r5, r6, r7 ; packhb r15, r16, r17 }
	{ mulhla_uu r5, r6, r7 ; shl r15, r16, r17 }
	{ mulhla_uu r5, r6, r7 ; slteh r15, r16, r17 }
	{ mulhla_uu r5, r6, r7 ; subb r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; add r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; auli r15, r16, 0x1234 }
	{ mulhlsa_uu r5, r6, r7 ; ill ; prefetch r25 }
	{ mulhlsa_uu r5, r6, r7 ; inv r15 }
	{ mulhlsa_uu r5, r6, r7 ; lb r25, r26 ; or r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; lb r25, r26 ; sra r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; lb_u r25, r26 ; ori r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; lb_u r25, r26 ; srai r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; lh r25, r26 ; or r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; lh r25, r26 ; sra r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; lh_u r25, r26 ; ori r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; lh_u r25, r26 ; srai r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; lw r25, r26 ; nor r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; lw r25, r26 ; sne r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
	{ mulhlsa_uu r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; prefetch r25 ; move r15, r16 }
	{ mulhlsa_uu r5, r6, r7 ; prefetch r25 ; slte r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; rl r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; s1a r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; s3a r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; sb r25, r26 ; s2a r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; sbadd r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; sh r25, r26 ; ori r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; sh r25, r26 ; srai r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; shrh r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; slte r15, r16, r17 ; prefetch r25 }
	{ mulhlsa_uu r5, r6, r7 ; slth_u r15, r16, r17 }
	{ mulhlsa_uu r5, r6, r7 ; slti_u r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; sra r15, r16, r17 ; lh_u r25, r26 }
	{ mulhlsa_uu r5, r6, r7 ; sraih r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; sw r25, r26 ; andi r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; sw r25, r26 ; shli r15, r16, 5 }
	{ mulhlsa_uu r5, r6, r7 ; xor r15, r16, r17 ; lh r25, r26 }
	{ mulll_ss r5, r6, r7 ; addbs_u r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
	{ mulll_ss r5, r6, r7 ; finv r15 }
	{ mulll_ss r5, r6, r7 ; ill ; sh r25, r26 }
	{ mulll_ss r5, r6, r7 ; jalr r15 }
	{ mulll_ss r5, r6, r7 ; lb r25, r26 ; rl r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; lb r25, r26 ; sub r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; lb_u r25, r26 ; rli r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; lb_u r25, r26 ; xor r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; lh r25, r26 ; rl r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; lh r25, r26 ; sub r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; lh_u r25, r26 ; rli r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; lh_u r25, r26 ; xor r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; lw r25, r26 ; ori r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; lw r25, r26 ; srai r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; mnz r15, r16, r17 ; lh r25, r26 }
	{ mulll_ss r5, r6, r7 ; movei r15, 5 ; lb r25, r26 }
	{ mulll_ss r5, r6, r7 ; mz r15, r16, r17 ; sh r25, r26 }
	{ mulll_ss r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
	{ mulll_ss r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
	{ mulll_ss r5, r6, r7 ; prefetch r25 ; mz r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; prefetch r25 ; slti r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
	{ mulll_ss r5, r6, r7 ; s2a r15, r16, r17 ; lb_u r25, r26 }
	{ mulll_ss r5, r6, r7 ; sb r25, r26 ; add r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; sb r25, r26 ; seq r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; seq r15, r16, r17 ; lb_u r25, r26 }
	{ mulll_ss r5, r6, r7 ; seqi r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; sh r25, r26 ; rli r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; sh r25, r26 ; xor r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
	{ mulll_ss r5, r6, r7 ; shri r15, r16, 5 ; lb_u r25, r26 }
	{ mulll_ss r5, r6, r7 ; slt r15, r16, r17 }
	{ mulll_ss r5, r6, r7 ; slte r15, r16, r17 ; sh r25, r26 }
	{ mulll_ss r5, r6, r7 ; slti r15, r16, 5 ; lb_u r25, r26 }
	{ mulll_ss r5, r6, r7 ; sltib_u r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
	{ mulll_ss r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
	{ mulll_ss r5, r6, r7 ; sw r25, r26 ; ill }
	{ mulll_ss r5, r6, r7 ; sw r25, r26 ; shri r15, r16, 5 }
	{ mulll_ss r5, r6, r7 ; xor r15, r16, r17 ; lw r25, r26 }
	{ mulll_su r5, r6, r7 ; auli r15, r16, 0x1234 }
	{ mulll_su r5, r6, r7 ; lb_u r15, r16 }
	{ mulll_su r5, r6, r7 ; minib_u r15, r16, 5 }
	{ mulll_su r5, r6, r7 ; packhs r15, r16, r17 }
	{ mulll_su r5, r6, r7 ; shlb r15, r16, r17 }
	{ mulll_su r5, r6, r7 ; slteh_u r15, r16, r17 }
	{ mulll_su r5, r6, r7 ; subbs_u r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; addb r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
	{ mulll_uu r5, r6, r7 ; dtlbpr r15 }
	{ mulll_uu r5, r6, r7 ; ill ; sb r25, r26 }
	{ mulll_uu r5, r6, r7 ; iret }
	{ mulll_uu r5, r6, r7 ; lb r25, r26 ; ori r15, r16, 5 }
	{ mulll_uu r5, r6, r7 ; lb r25, r26 ; srai r15, r16, 5 }
	{ mulll_uu r5, r6, r7 ; lb_u r25, r26 ; rl r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; lb_u r25, r26 ; sub r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; lh r25, r26 ; ori r15, r16, 5 }
	{ mulll_uu r5, r6, r7 ; lh r25, r26 ; srai r15, r16, 5 }
	{ mulll_uu r5, r6, r7 ; lh_u r25, r26 ; rl r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; lh_u r25, r26 ; sub r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; lw r25, r26 ; or r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; lw r25, r26 ; sra r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
	{ mulll_uu r5, r6, r7 ; move r15, r16 }
	{ mulll_uu r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
	{ mulll_uu r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
	{ mulll_uu r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
	{ mulll_uu r5, r6, r7 ; prefetch r25 ; movei r15, 5 }
	{ mulll_uu r5, r6, r7 ; prefetch r25 ; slte_u r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
	{ mulll_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
	{ mulll_uu r5, r6, r7 ; sb r15, r16 }
	{ mulll_uu r5, r6, r7 ; sb r25, r26 ; s3a r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
	{ mulll_uu r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
	{ mulll_uu r5, r6, r7 ; sh r25, r26 ; rl r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; sh r25, r26 ; sub r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
	{ mulll_uu r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
	{ mulll_uu r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
	{ mulll_uu r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
	{ mulll_uu r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
	{ mulll_uu r5, r6, r7 ; sltib r15, r16, 5 }
	{ mulll_uu r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
	{ mulll_uu r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
	{ mulll_uu r5, r6, r7 ; sw r25, r26 ; fnop }
	{ mulll_uu r5, r6, r7 ; sw r25, r26 ; shr r15, r16, r17 }
	{ mulll_uu r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
	{ mullla_ss r5, r6, r7 ; addh r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
	{ mullla_ss r5, r6, r7 ; flush r15 }
	{ mullla_ss r5, r6, r7 ; ill ; sw r25, r26 }
	{ mullla_ss r5, r6, r7 ; jalrp r15 }
	{ mullla_ss r5, r6, r7 ; lb r25, r26 ; rli r15, r16, 5 }
	{ mullla_ss r5, r6, r7 ; lb r25, r26 ; xor r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; lb_u r25, r26 ; s1a r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; lb_u r25, r26 }
	{ mullla_ss r5, r6, r7 ; lh r25, r26 ; rli r15, r16, 5 }
	{ mullla_ss r5, r6, r7 ; lh r25, r26 ; xor r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; lh_u r25, r26 ; s1a r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; lh_u r25, r26 }
	{ mullla_ss r5, r6, r7 ; lw r25, r26 ; rl r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; lw r25, r26 ; sub r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
	{ mullla_ss r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
	{ mullla_ss r5, r6, r7 ; mz r15, r16, r17 ; sw r25, r26 }
	{ mullla_ss r5, r6, r7 ; nor r15, r16, r17 ; sb r25, r26 }
	{ mullla_ss r5, r6, r7 ; ori r15, r16, 5 ; sb r25, r26 }
	{ mullla_ss r5, r6, r7 ; prefetch r25 ; nop }
	{ mullla_ss r5, r6, r7 ; prefetch r25 ; slti_u r15, r16, 5 }
	{ mullla_ss r5, r6, r7 ; rli r15, r16, 5 ; lh r25, r26 }
	{ mullla_ss r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
	{ mullla_ss r5, r6, r7 ; sb r25, r26 ; addi r15, r16, 5 }
	{ mullla_ss r5, r6, r7 ; sb r25, r26 ; seqi r15, r16, 5 }
	{ mullla_ss r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
	{ mullla_ss r5, r6, r7 ; seqib r15, r16, 5 }
	{ mullla_ss r5, r6, r7 ; sh r25, r26 ; s1a r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; sh r25, r26 }
	{ mullla_ss r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
	{ mullla_ss r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
	{ mullla_ss r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
	{ mullla_ss r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
	{ mullla_ss r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
	{ mullla_ss r5, r6, r7 ; sltih r15, r16, 5 }
	{ mullla_ss r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
	{ mullla_ss r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
	{ mullla_ss r5, r6, r7 ; sw r25, r26 ; info 19 }
	{ mullla_ss r5, r6, r7 ; sw r25, r26 ; slt r15, r16, r17 }
	{ mullla_ss r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
	{ mullla_su r5, r6, r7 ; dtlbpr r15 }
	{ mullla_su r5, r6, r7 ; lbadd r15, r16, 5 }
	{ mullla_su r5, r6, r7 ; minih r15, r16, 5 }
	{ mullla_su r5, r6, r7 ; packlb r15, r16, r17 }
	{ mullla_su r5, r6, r7 ; shlh r15, r16, r17 }
	{ mullla_su r5, r6, r7 ; slth r15, r16, r17 }
	{ mullla_su r5, r6, r7 ; subh r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; addbs_u r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; and r15, r16, r17 ; lh r25, r26 }
	{ mullla_uu r5, r6, r7 ; finv r15 }
	{ mullla_uu r5, r6, r7 ; ill ; sh r25, r26 }
	{ mullla_uu r5, r6, r7 ; jalr r15 }
	{ mullla_uu r5, r6, r7 ; lb r25, r26 ; rl r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; lb r25, r26 ; sub r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; lb_u r25, r26 ; rli r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; lb_u r25, r26 ; xor r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; lh r25, r26 ; rl r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; lh r25, r26 ; sub r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; lh_u r25, r26 ; rli r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; lh_u r25, r26 ; xor r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; lw r25, r26 ; ori r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; lw r25, r26 ; srai r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; mnz r15, r16, r17 ; lh r25, r26 }
	{ mullla_uu r5, r6, r7 ; movei r15, 5 ; lb r25, r26 }
	{ mullla_uu r5, r6, r7 ; mz r15, r16, r17 ; sh r25, r26 }
	{ mullla_uu r5, r6, r7 ; nor r15, r16, r17 ; prefetch r25 }
	{ mullla_uu r5, r6, r7 ; ori r15, r16, 5 ; prefetch r25 }
	{ mullla_uu r5, r6, r7 ; prefetch r25 ; mz r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; prefetch r25 ; slti r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; rli r15, r16, 5 ; lb_u r25, r26 }
	{ mullla_uu r5, r6, r7 ; s2a r15, r16, r17 ; lb_u r25, r26 }
	{ mullla_uu r5, r6, r7 ; sb r25, r26 ; add r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; sb r25, r26 ; seq r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; seq r15, r16, r17 ; lb_u r25, r26 }
	{ mullla_uu r5, r6, r7 ; seqi r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; sh r25, r26 ; rli r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; sh r25, r26 ; xor r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; shli r15, r16, 5 ; prefetch r25 }
	{ mullla_uu r5, r6, r7 ; shri r15, r16, 5 ; lb_u r25, r26 }
	{ mullla_uu r5, r6, r7 ; slt r15, r16, r17 }
	{ mullla_uu r5, r6, r7 ; slte r15, r16, r17 ; sh r25, r26 }
	{ mullla_uu r5, r6, r7 ; slti r15, r16, 5 ; lb_u r25, r26 }
	{ mullla_uu r5, r6, r7 ; sltib_u r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; sra r15, r16, r17 ; prefetch r25 }
	{ mullla_uu r5, r6, r7 ; sub r15, r16, r17 ; lb_u r25, r26 }
	{ mullla_uu r5, r6, r7 ; sw r25, r26 ; ill }
	{ mullla_uu r5, r6, r7 ; sw r25, r26 ; shri r15, r16, 5 }
	{ mullla_uu r5, r6, r7 ; xor r15, r16, r17 ; lw r25, r26 }
	{ mulllsa_uu r5, r6, r7 ; auli r15, r16, 0x1234 }
	{ mulllsa_uu r5, r6, r7 ; lb_u r15, r16 }
	{ mulllsa_uu r5, r6, r7 ; minib_u r15, r16, 5 }
	{ mulllsa_uu r5, r6, r7 ; packhs r15, r16, r17 }
	{ mulllsa_uu r5, r6, r7 ; shlb r15, r16, r17 }
	{ mulllsa_uu r5, r6, r7 ; slteh_u r15, r16, r17 }
	{ mulllsa_uu r5, r6, r7 ; subbs_u r15, r16, r17 }
	{ mvnz r5, r6, r7 ; addb r15, r16, r17 }
	{ mvnz r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
	{ mvnz r5, r6, r7 ; dtlbpr r15 }
	{ mvnz r5, r6, r7 ; ill ; sb r25, r26 }
	{ mvnz r5, r6, r7 ; iret }
	{ mvnz r5, r6, r7 ; lb r25, r26 ; ori r15, r16, 5 }
	{ mvnz r5, r6, r7 ; lb r25, r26 ; srai r15, r16, 5 }
	{ mvnz r5, r6, r7 ; lb_u r25, r26 ; rl r15, r16, r17 }
	{ mvnz r5, r6, r7 ; lb_u r25, r26 ; sub r15, r16, r17 }
	{ mvnz r5, r6, r7 ; lh r25, r26 ; ori r15, r16, 5 }
	{ mvnz r5, r6, r7 ; lh r25, r26 ; srai r15, r16, 5 }
	{ mvnz r5, r6, r7 ; lh_u r25, r26 ; rl r15, r16, r17 }
	{ mvnz r5, r6, r7 ; lh_u r25, r26 ; sub r15, r16, r17 }
	{ mvnz r5, r6, r7 ; lw r25, r26 ; or r15, r16, r17 }
	{ mvnz r5, r6, r7 ; lw r25, r26 ; sra r15, r16, r17 }
	{ mvnz r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
	{ mvnz r5, r6, r7 ; move r15, r16 }
	{ mvnz r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
	{ mvnz r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
	{ mvnz r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
	{ mvnz r5, r6, r7 ; prefetch r25 ; movei r15, 5 }
	{ mvnz r5, r6, r7 ; prefetch r25 ; slte_u r15, r16, r17 }
	{ mvnz r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
	{ mvnz r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
	{ mvnz r5, r6, r7 ; sb r15, r16 }
	{ mvnz r5, r6, r7 ; sb r25, r26 ; s3a r15, r16, r17 }
	{ mvnz r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
	{ mvnz r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
	{ mvnz r5, r6, r7 ; sh r25, r26 ; rl r15, r16, r17 }
	{ mvnz r5, r6, r7 ; sh r25, r26 ; sub r15, r16, r17 }
	{ mvnz r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
	{ mvnz r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
	{ mvnz r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
	{ mvnz r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
	{ mvnz r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
	{ mvnz r5, r6, r7 ; sltib r15, r16, 5 }
	{ mvnz r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
	{ mvnz r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
	{ mvnz r5, r6, r7 ; sw r25, r26 ; fnop }
	{ mvnz r5, r6, r7 ; sw r25, r26 ; shr r15, r16, r17 }
	{ mvnz r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
	{ mvz r5, r6, r7 ; addh r15, r16, r17 }
	{ mvz r5, r6, r7 ; and r15, r16, r17 ; lh_u r25, r26 }
	{ mvz r5, r6, r7 ; flush r15 }
	{ mvz r5, r6, r7 ; ill ; sw r25, r26 }
	{ mvz r5, r6, r7 ; jalrp r15 }
	{ mvz r5, r6, r7 ; lb r25, r26 ; rli r15, r16, 5 }
	{ mvz r5, r6, r7 ; lb r25, r26 ; xor r15, r16, r17 }
	{ mvz r5, r6, r7 ; lb_u r25, r26 ; s1a r15, r16, r17 }
	{ mvz r5, r6, r7 ; lb_u r25, r26 }
	{ mvz r5, r6, r7 ; lh r25, r26 ; rli r15, r16, 5 }
	{ mvz r5, r6, r7 ; lh r25, r26 ; xor r15, r16, r17 }
	{ mvz r5, r6, r7 ; lh_u r25, r26 ; s1a r15, r16, r17 }
	{ mvz r5, r6, r7 ; lh_u r25, r26 }
	{ mvz r5, r6, r7 ; lw r25, r26 ; rl r15, r16, r17 }
	{ mvz r5, r6, r7 ; lw r25, r26 ; sub r15, r16, r17 }
	{ mvz r5, r6, r7 ; mnz r15, r16, r17 ; lh_u r25, r26 }
	{ mvz r5, r6, r7 ; movei r15, 5 ; lb_u r25, r26 }
	{ mvz r5, r6, r7 ; mz r15, r16, r17 ; sw r25, r26 }
	{ mvz r5, r6, r7 ; nor r15, r16, r17 ; sb r25, r26 }
	{ mvz r5, r6, r7 ; ori r15, r16, 5 ; sb r25, r26 }
	{ mvz r5, r6, r7 ; prefetch r25 ; nop }
	{ mvz r5, r6, r7 ; prefetch r25 ; slti_u r15, r16, 5 }
	{ mvz r5, r6, r7 ; rli r15, r16, 5 ; lh r25, r26 }
	{ mvz r5, r6, r7 ; s2a r15, r16, r17 ; lh r25, r26 }
	{ mvz r5, r6, r7 ; sb r25, r26 ; addi r15, r16, 5 }
	{ mvz r5, r6, r7 ; sb r25, r26 ; seqi r15, r16, 5 }
	{ mvz r5, r6, r7 ; seq r15, r16, r17 ; lh r25, r26 }
	{ mvz r5, r6, r7 ; seqib r15, r16, 5 }
	{ mvz r5, r6, r7 ; sh r25, r26 ; s1a r15, r16, r17 }
	{ mvz r5, r6, r7 ; sh r25, r26 }
	{ mvz r5, r6, r7 ; shli r15, r16, 5 ; sb r25, r26 }
	{ mvz r5, r6, r7 ; shri r15, r16, 5 ; lh r25, r26 }
	{ mvz r5, r6, r7 ; slt_u r15, r16, r17 ; lb r25, r26 }
	{ mvz r5, r6, r7 ; slte r15, r16, r17 ; sw r25, r26 }
	{ mvz r5, r6, r7 ; slti r15, r16, 5 ; lh r25, r26 }
	{ mvz r5, r6, r7 ; sltih r15, r16, 5 }
	{ mvz r5, r6, r7 ; sra r15, r16, r17 ; sb r25, r26 }
	{ mvz r5, r6, r7 ; sub r15, r16, r17 ; lh r25, r26 }
	{ mvz r5, r6, r7 ; sw r25, r26 ; info 19 }
	{ mvz r5, r6, r7 ; sw r25, r26 ; slt r15, r16, r17 }
	{ mvz r5, r6, r7 ; xor r15, r16, r17 ; prefetch r25 }
	{ mz r15, r16, r17 ; addi r5, r6, 5 ; lb r25, r26 }
	{ mz r15, r16, r17 ; and r5, r6, r7 ; lh_u r25, r26 }
	{ mz r15, r16, r17 ; bitx r5, r6 ; lb r25, r26 }
	{ mz r15, r16, r17 ; clz r5, r6 ; lb r25, r26 }
	{ mz r15, r16, r17 ; ctz r5, r6 ; sw r25, r26 }
	{ mz r15, r16, r17 ; info 19 ; sh r25, r26 }
	{ mz r15, r16, r17 ; lb r25, r26 ; movei r5, 5 }
	{ mz r15, r16, r17 ; lb r25, r26 ; s1a r5, r6, r7 }
	{ mz r15, r16, r17 ; lb r25, r26 ; tblidxb1 r5, r6 }
	{ mz r15, r16, r17 ; lb_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ mz r15, r16, r17 ; lb_u r25, r26 ; seq r5, r6, r7 }
	{ mz r15, r16, r17 ; lb_u r25, r26 ; xor r5, r6, r7 }
	{ mz r15, r16, r17 ; lh r25, r26 ; mulll_ss r5, r6, r7 }
	{ mz r15, r16, r17 ; lh r25, r26 ; shli r5, r6, 5 }
	{ mz r15, r16, r17 ; lh_u r25, r26 ; addi r5, r6, 5 }
	{ mz r15, r16, r17 ; lh_u r25, r26 ; mullla_uu r5, r6, r7 }
	{ mz r15, r16, r17 ; lh_u r25, r26 ; slt r5, r6, r7 }
	{ mz r15, r16, r17 ; lw r25, r26 ; bitx r5, r6 }
	{ mz r15, r16, r17 ; lw r25, r26 ; mz r5, r6, r7 }
	{ mz r15, r16, r17 ; lw r25, r26 ; slte_u r5, r6, r7 }
	{ mz r15, r16, r17 ; minih r5, r6, 5 }
	{ mz r15, r16, r17 ; move r5, r6 ; sb r25, r26 }
	{ mz r15, r16, r17 ; mulhh_ss r5, r6, r7 ; lw r25, r26 }
	{ mz r15, r16, r17 ; mulhha_ss r5, r6, r7 ; lh_u r25, r26 }
	{ mz r15, r16, r17 ; mulhl_su r5, r6, r7 }
	{ mz r15, r16, r17 ; mulll_ss r5, r6, r7 ; lh_u r25, r26 }
	{ mz r15, r16, r17 ; mullla_ss r5, r6, r7 ; lh r25, r26 }
	{ mz r15, r16, r17 ; mvnz r5, r6, r7 ; lb r25, r26 }
	{ mz r15, r16, r17 ; mz r5, r6, r7 ; lb r25, r26 }
	{ mz r15, r16, r17 ; nop ; sw r25, r26 }
	{ mz r15, r16, r17 ; or r5, r6, r7 ; sw r25, r26 }
	{ mz r15, r16, r17 ; pcnt r5, r6 ; lw r25, r26 }
	{ mz r15, r16, r17 ; prefetch r25 ; mulhh_uu r5, r6, r7 }
	{ mz r15, r16, r17 ; prefetch r25 ; s3a r5, r6, r7 }
	{ mz r15, r16, r17 ; prefetch r25 ; tblidxb3 r5, r6 }
	{ mz r15, r16, r17 ; rli r5, r6, 5 ; sh r25, r26 }
	{ mz r15, r16, r17 ; s2a r5, r6, r7 ; sh r25, r26 }
	{ mz r15, r16, r17 ; sb r25, r26 ; addi r5, r6, 5 }
	{ mz r15, r16, r17 ; sb r25, r26 ; mullla_uu r5, r6, r7 }
	{ mz r15, r16, r17 ; sb r25, r26 ; slt r5, r6, r7 }
	{ mz r15, r16, r17 ; seq r5, r6, r7 ; lw r25, r26 }
	{ mz r15, r16, r17 ; sh r25, r26 ; add r5, r6, r7 }
	{ mz r15, r16, r17 ; sh r25, r26 ; mullla_ss r5, r6, r7 }
	{ mz r15, r16, r17 ; sh r25, r26 ; shri r5, r6, 5 }
	{ mz r15, r16, r17 ; shl r5, r6, r7 ; lh_u r25, r26 }
	{ mz r15, r16, r17 ; shlih r5, r6, 5 }
	{ mz r15, r16, r17 ; shri r5, r6, 5 ; sh r25, r26 }
	{ mz r15, r16, r17 ; slt_u r5, r6, r7 ; prefetch r25 }
	{ mz r15, r16, r17 ; slte_u r5, r6, r7 ; lh_u r25, r26 }
	{ mz r15, r16, r17 ; slti r5, r6, 5 ; sh r25, r26 }
	{ mz r15, r16, r17 ; sne r5, r6, r7 ; lh_u r25, r26 }
	{ mz r15, r16, r17 ; srah r5, r6, r7 }
	{ mz r15, r16, r17 ; sub r5, r6, r7 ; sh r25, r26 }
	{ mz r15, r16, r17 ; sw r25, r26 ; movei r5, 5 }
	{ mz r15, r16, r17 ; sw r25, r26 ; s1a r5, r6, r7 }
	{ mz r15, r16, r17 ; sw r25, r26 ; tblidxb1 r5, r6 }
	{ mz r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch r25 }
	{ mz r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch r25 }
	{ mz r5, r6, r7 ; add r15, r16, r17 ; lw r25, r26 }
	{ mz r5, r6, r7 ; addib r15, r16, 5 }
	{ mz r5, r6, r7 ; andi r15, r16, 5 ; prefetch r25 }
	{ mz r5, r6, r7 ; ill ; lb r25, r26 }
	{ mz r5, r6, r7 ; infol 0x1234 }
	{ mz r5, r6, r7 ; lb r25, r26 ; move r15, r16 }
	{ mz r5, r6, r7 ; lb r25, r26 ; slte r15, r16, r17 }
	{ mz r5, r6, r7 ; lb_u r25, r26 ; movei r15, 5 }
	{ mz r5, r6, r7 ; lb_u r25, r26 ; slte_u r15, r16, r17 }
	{ mz r5, r6, r7 ; lh r25, r26 ; move r15, r16 }
	{ mz r5, r6, r7 ; lh r25, r26 ; slte r15, r16, r17 }
	{ mz r5, r6, r7 ; lh_u r25, r26 ; movei r15, 5 }
	{ mz r5, r6, r7 ; lh_u r25, r26 ; slte_u r15, r16, r17 }
	{ mz r5, r6, r7 ; lw r25, r26 ; mnz r15, r16, r17 }
	{ mz r5, r6, r7 ; lw r25, r26 ; slt_u r15, r16, r17 }
	{ mz r5, r6, r7 ; minb_u r15, r16, r17 }
	{ mz r5, r6, r7 ; move r15, r16 ; lh_u r25, r26 }
	{ mz r5, r6, r7 ; mz r15, r16, r17 ; lb r25, r26 }
	{ mz r5, r6, r7 ; nop ; sw r25, r26 }
	{ mz r5, r6, r7 ; or r15, r16, r17 ; sw r25, r26 }
	{ mz r5, r6, r7 ; prefetch r25 ; andi r15, r16, 5 }
	{ mz r5, r6, r7 ; prefetch r25 ; shli r15, r16, 5 }
	{ mz r5, r6, r7 ; rl r15, r16, r17 ; lw r25, r26 }
	{ mz r5, r6, r7 ; s1a r15, r16, r17 ; lw r25, r26 }
	{ mz r5, r6, r7 ; s3a r15, r16, r17 ; lw r25, r26 }
	{ mz r5, r6, r7 ; sb r25, r26 ; or r15, r16, r17 }
	{ mz r5, r6, r7 ; sb r25, r26 ; sra r15, r16, r17 }
	{ mz r5, r6, r7 ; seqi r15, r16, 5 ; lh r25, r26 }
	{ mz r5, r6, r7 ; sh r25, r26 ; movei r15, 5 }
	{ mz r5, r6, r7 ; sh r25, r26 ; slte_u r15, r16, r17 }
	{ mz r5, r6, r7 ; shlb r15, r16, r17 }
	{ mz r5, r6, r7 ; shr r15, r16, r17 ; sb r25, r26 }
	{ mz r5, r6, r7 ; slt r15, r16, r17 ; lh r25, r26 }
	{ mz r5, r6, r7 ; slte r15, r16, r17 ; lb r25, r26 }
	{ mz r5, r6, r7 ; slteb r15, r16, r17 }
	{ mz r5, r6, r7 ; slti_u r15, r16, 5 ; lw r25, r26 }
	{ mz r5, r6, r7 ; sneb r15, r16, r17 }
	{ mz r5, r6, r7 ; srai r15, r16, 5 ; sb r25, r26 }
	{ mz r5, r6, r7 ; subs r15, r16, r17 }
	{ mz r5, r6, r7 ; sw r25, r26 ; s2a r15, r16, r17 }
	{ mz r5, r6, r7 ; swadd r15, r16, 5 }
	{ mzb r15, r16, r17 ; addib r5, r6, 5 }
	{ mzb r15, r16, r17 ; info 19 }
	{ mzb r15, r16, r17 ; moveli r5, 0x1234 }
	{ mzb r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ mzb r15, r16, r17 ; rli r5, r6, 5 }
	{ mzb r15, r16, r17 ; shlib r5, r6, 5 }
	{ mzb r15, r16, r17 ; slti r5, r6, 5 }
	{ mzb r15, r16, r17 ; subs r5, r6, r7 }
	{ mzb r5, r6, r7 ; auli r15, r16, 0x1234 }
	{ mzb r5, r6, r7 ; lb_u r15, r16 }
	{ mzb r5, r6, r7 ; minib_u r15, r16, 5 }
	{ mzb r5, r6, r7 ; packhs r15, r16, r17 }
	{ mzb r5, r6, r7 ; shlb r15, r16, r17 }
	{ mzb r5, r6, r7 ; slteh_u r15, r16, r17 }
	{ mzb r5, r6, r7 ; subbs_u r15, r16, r17 }
	{ mzh r15, r16, r17 ; adds r5, r6, r7 }
	{ mzh r15, r16, r17 ; intlb r5, r6, r7 }
	{ mzh r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ mzh r15, r16, r17 ; mulllsa_uu r5, r6, r7 }
	{ mzh r15, r16, r17 ; sadab_u r5, r6, r7 }
	{ mzh r15, r16, r17 ; shrh r5, r6, r7 }
	{ mzh r15, r16, r17 ; sltih r5, r6, 5 }
	{ mzh r15, r16, r17 ; tblidxb3 r5, r6 }
	{ mzh r5, r6, r7 ; fnop }
	{ mzh r5, r6, r7 ; lh_u r15, r16 }
	{ mzh r5, r6, r7 ; mnzb r15, r16, r17 }
	{ mzh r5, r6, r7 ; rl r15, r16, r17 }
	{ mzh r5, r6, r7 ; shlih r15, r16, 5 }
	{ mzh r5, r6, r7 ; slti_u r15, r16, 5 }
	{ mzh r5, r6, r7 ; sw r15, r16 }
	{ nop ; add r5, r6, r7 ; lh_u r25, r26 }
	{ nop ; addi r15, r16, 5 ; prefetch r25 }
	{ nop ; addli r5, r6, 0x1234 }
	{ nop ; and r5, r6, r7 ; lh_u r25, r26 }
	{ nop ; andi r5, r6, 5 ; lh_u r25, r26 }
	{ nop ; bitx r5, r6 }
	{ nop ; clz r5, r6 ; sw r25, r26 }
	{ nop ; fnop ; lb_u r25, r26 }
	{ nop ; info 19 ; lb r25, r26 }
	{ nop ; iret }
	{ nop ; lb r25, r26 ; info 19 }
	{ nop ; lb r25, r26 ; nop }
	{ nop ; lb r25, r26 ; seqi r15, r16, 5 }
	{ nop ; lb r25, r26 ; slti_u r15, r16, 5 }
	{ nop ; lb_u r25, r26 ; addi r15, r16, 5 }
	{ nop ; lb_u r25, r26 ; mulhh_uu r5, r6, r7 }
	{ nop ; lb_u r25, r26 ; rl r15, r16, r17 }
	{ nop ; lb_u r25, r26 ; shri r15, r16, 5 }
	{ nop ; lb_u r25, r26 ; sub r15, r16, r17 }
	{ nop ; lh r25, r26 ; bitx r5, r6 }
	{ nop ; lh r25, r26 ; mullla_ss r5, r6, r7 }
	{ nop ; lh r25, r26 ; s2a r15, r16, r17 }
	{ nop ; lh r25, r26 ; slte r15, r16, r17 }
	{ nop ; lh r25, r26 ; xor r15, r16, r17 }
	{ nop ; lh_u r25, r26 ; mnz r5, r6, r7 }
	{ nop ; lh_u r25, r26 ; nor r5, r6, r7 }
	{ nop ; lh_u r25, r26 ; shl r15, r16, r17 }
	{ nop ; lh_u r25, r26 ; sne r15, r16, r17 }
	{ nop ; lw r25, r26 ; add r5, r6, r7 }
	{ nop ; lw r25, r26 ; mulhh_ss r5, r6, r7 }
	{ nop ; lw r25, r26 ; pcnt r5, r6 }
	{ nop ; lw r25, r26 ; shr r5, r6, r7 }
	{ nop ; lw r25, r26 ; srai r5, r6, 5 }
	{ nop ; maxih r5, r6, 5 }
	{ nop ; mnz r15, r16, r17 ; sh r25, r26 }
	{ nop ; move r15, r16 ; lh_u r25, r26 }
	{ nop ; movei r15, 5 ; lh_u r25, r26 }
	{ nop ; movelis r5, 0x1234 }
	{ nop ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ nop ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ nop ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ nop ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ nop ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ nop ; mvz r5, r6, r7 ; lw r25, r26 }
	{ nop ; mz r5, r6, r7 ; lw r25, r26 }
	{ nop ; nop }
	{ nop ; nor r5, r6, r7 }
	{ nop ; or r5, r6, r7 }
	{ nop ; ori r5, r6, 5 }
	{ nop ; prefetch r25 ; add r15, r16, r17 }
	{ nop ; prefetch r25 ; movei r5, 5 }
	{ nop ; prefetch r25 ; ori r5, r6, 5 }
	{ nop ; prefetch r25 ; shr r15, r16, r17 }
	{ nop ; prefetch r25 ; srai r15, r16, 5 }
	{ nop ; rl r15, r16, r17 ; sw r25, r26 }
	{ nop ; rli r15, r16, 5 ; sw r25, r26 }
	{ nop ; s1a r15, r16, r17 ; sw r25, r26 }
	{ nop ; s2a r15, r16, r17 ; sw r25, r26 }
	{ nop ; s3a r15, r16, r17 ; sw r25, r26 }
	{ nop ; sb r25, r26 ; add r5, r6, r7 }
	{ nop ; sb r25, r26 ; mulhh_ss r5, r6, r7 }
	{ nop ; sb r25, r26 ; pcnt r5, r6 }
	{ nop ; sb r25, r26 ; shr r5, r6, r7 }
	{ nop ; sb r25, r26 ; srai r5, r6, 5 }
	{ nop ; seq r15, r16, r17 }
	{ nop ; seqi r15, r16, 5 ; prefetch r25 }
	{ nop ; sh r25, r26 ; add r15, r16, r17 }
	{ nop ; sh r25, r26 ; movei r5, 5 }
	{ nop ; sh r25, r26 ; ori r5, r6, 5 }
	{ nop ; sh r25, r26 ; shr r15, r16, r17 }
	{ nop ; sh r25, r26 ; srai r15, r16, 5 }
	{ nop ; shl r15, r16, r17 ; sw r25, r26 }
	{ nop ; shli r15, r16, 5 ; lw r25, r26 }
	{ nop ; shr r15, r16, r17 ; lb r25, r26 }
	{ nop ; shrb r15, r16, r17 }
	{ nop ; shri r5, r6, 5 ; sb r25, r26 }
	{ nop ; slt r5, r6, r7 ; lh r25, r26 }
	{ nop ; slt_u r5, r6, r7 ; lh r25, r26 }
	{ nop ; slte r15, r16, r17 ; sw r25, r26 }
	{ nop ; slte_u r15, r16, r17 ; sw r25, r26 }
	{ nop ; slth r15, r16, r17 }
	{ nop ; slti r5, r6, 5 ; sb r25, r26 }
	{ nop ; slti_u r5, r6, 5 ; sb r25, r26 }
	{ nop ; sne r15, r16, r17 ; sw r25, r26 }
	{ nop ; sra r15, r16, r17 ; lw r25, r26 }
	{ nop ; srai r15, r16, 5 ; lb r25, r26 }
	{ nop ; sraib r15, r16, 5 }
	{ nop ; sub r5, r6, r7 ; sb r25, r26 }
	{ nop ; sw r25, r26 ; and r5, r6, r7 }
	{ nop ; sw r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ nop ; sw r25, r26 ; rli r5, r6, 5 }
	{ nop ; sw r25, r26 ; slt r5, r6, r7 }
	{ nop ; sw r25, r26 ; tblidxb1 r5, r6 }
	{ nop ; tblidxb0 r5, r6 }
	{ nop ; tblidxb2 r5, r6 }
	{ nop ; xor r15, r16, r17 ; sh r25, r26 }
	{ nor r15, r16, r17 ; add r5, r6, r7 ; prefetch r25 }
	{ nor r15, r16, r17 ; addih r5, r6, 5 }
	{ nor r15, r16, r17 ; andi r5, r6, 5 ; lw r25, r26 }
	{ nor r15, r16, r17 ; bytex r5, r6 ; lb_u r25, r26 }
	{ nor r15, r16, r17 ; crc32_8 r5, r6, r7 }
	{ nor r15, r16, r17 ; fnop ; sw r25, r26 }
	{ nor r15, r16, r17 ; lb r25, r26 ; andi r5, r6, 5 }
	{ nor r15, r16, r17 ; lb r25, r26 ; mvz r5, r6, r7 }
	{ nor r15, r16, r17 ; lb r25, r26 ; slte r5, r6, r7 }
	{ nor r15, r16, r17 ; lb_u r25, r26 ; clz r5, r6 }
	{ nor r15, r16, r17 ; lb_u r25, r26 ; nor r5, r6, r7 }
	{ nor r15, r16, r17 ; lb_u r25, r26 ; slti_u r5, r6, 5 }
	{ nor r15, r16, r17 ; lh r25, r26 ; info 19 }
	{ nor r15, r16, r17 ; lh r25, r26 ; pcnt r5, r6 }
	{ nor r15, r16, r17 ; lh r25, r26 ; srai r5, r6, 5 }
	{ nor r15, r16, r17 ; lh_u r25, r26 ; movei r5, 5 }
	{ nor r15, r16, r17 ; lh_u r25, r26 ; s1a r5, r6, r7 }
	{ nor r15, r16, r17 ; lh_u r25, r26 ; tblidxb1 r5, r6 }
	{ nor r15, r16, r17 ; lw r25, r26 ; mulhha_ss r5, r6, r7 }
	{ nor r15, r16, r17 ; lw r25, r26 ; seq r5, r6, r7 }
	{ nor r15, r16, r17 ; lw r25, r26 ; xor r5, r6, r7 }
	{ nor r15, r16, r17 ; mnz r5, r6, r7 }
	{ nor r15, r16, r17 ; movei r5, 5 ; sh r25, r26 }
	{ nor r15, r16, r17 ; mulhh_uu r5, r6, r7 ; lw r25, r26 }
	{ nor r15, r16, r17 ; mulhha_uu r5, r6, r7 ; lh_u r25, r26 }
	{ nor r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; lw r25, r26 }
	{ nor r15, r16, r17 ; mulll_uu r5, r6, r7 ; lh_u r25, r26 }
	{ nor r15, r16, r17 ; mullla_uu r5, r6, r7 ; lh r25, r26 }
	{ nor r15, r16, r17 ; mvz r5, r6, r7 ; lb_u r25, r26 }
	{ nor r15, r16, r17 ; mzh r5, r6, r7 }
	{ nor r15, r16, r17 ; nor r5, r6, r7 }
	{ nor r15, r16, r17 ; ori r5, r6, 5 }
	{ nor r15, r16, r17 ; prefetch r25 ; bytex r5, r6 }
	{ nor r15, r16, r17 ; prefetch r25 ; nop }
	{ nor r15, r16, r17 ; prefetch r25 ; slti r5, r6, 5 }
	{ nor r15, r16, r17 ; rl r5, r6, r7 ; sw r25, r26 }
	{ nor r15, r16, r17 ; s1a r5, r6, r7 ; sw r25, r26 }
	{ nor r15, r16, r17 ; s3a r5, r6, r7 ; sw r25, r26 }
	{ nor r15, r16, r17 ; sb r25, r26 ; movei r5, 5 }
	{ nor r15, r16, r17 ; sb r25, r26 ; s1a r5, r6, r7 }
	{ nor r15, r16, r17 ; sb r25, r26 ; tblidxb1 r5, r6 }
	{ nor r15, r16, r17 ; seqi r5, r6, 5 ; lh_u r25, r26 }
	{ nor r15, r16, r17 ; sh r25, r26 ; move r5, r6 }
	{ nor r15, r16, r17 ; sh r25, r26 ; rli r5, r6, 5 }
	{ nor r15, r16, r17 ; sh r25, r26 ; tblidxb0 r5, r6 }
	{ nor r15, r16, r17 ; shli r5, r6, 5 ; lh r25, r26 }
	{ nor r15, r16, r17 ; shrb r5, r6, r7 }
	{ nor r15, r16, r17 ; slt r5, r6, r7 ; sb r25, r26 }
	{ nor r15, r16, r17 ; slte r5, r6, r7 ; lw r25, r26 }
	{ nor r15, r16, r17 ; slth r5, r6, r7 }
	{ nor r15, r16, r17 ; slti_u r5, r6, 5 ; sw r25, r26 }
	{ nor r15, r16, r17 ; sra r5, r6, r7 ; lh r25, r26 }
	{ nor r15, r16, r17 ; sraib r5, r6, 5 }
	{ nor r15, r16, r17 ; sw r25, r26 ; andi r5, r6, 5 }
	{ nor r15, r16, r17 ; sw r25, r26 ; mvz r5, r6, r7 }
	{ nor r15, r16, r17 ; sw r25, r26 ; slte r5, r6, r7 }
	{ nor r15, r16, r17 ; tblidxb0 r5, r6 ; sb r25, r26 }
	{ nor r15, r16, r17 ; tblidxb2 r5, r6 ; sb r25, r26 }
	{ nor r15, r16, r17 ; xor r5, r6, r7 ; sb r25, r26 }
	{ nor r5, r6, r7 ; addi r15, r16, 5 ; lb_u r25, r26 }
	{ nor r5, r6, r7 ; and r15, r16, r17 ; sb r25, r26 }
	{ nor r5, r6, r7 ; fnop ; lh r25, r26 }
	{ nor r5, r6, r7 ; info 19 ; lb_u r25, r26 }
	{ nor r5, r6, r7 ; lb r15, r16 }
	{ nor r5, r6, r7 ; lb r25, r26 ; s3a r15, r16, r17 }
	{ nor r5, r6, r7 ; lb_u r25, r26 ; add r15, r16, r17 }
	{ nor r5, r6, r7 ; lb_u r25, r26 ; seq r15, r16, r17 }
	{ nor r5, r6, r7 ; lh r15, r16 }
	{ nor r5, r6, r7 ; lh r25, r26 ; s3a r15, r16, r17 }
	{ nor r5, r6, r7 ; lh_u r25, r26 ; add r15, r16, r17 }
	{ nor r5, r6, r7 ; lh_u r25, r26 ; seq r15, r16, r17 }
	{ nor r5, r6, r7 ; lnk r15 }
	{ nor r5, r6, r7 ; lw r25, r26 ; s2a r15, r16, r17 }
	{ nor r5, r6, r7 ; lw_na r15, r16 }
	{ nor r5, r6, r7 ; mnz r15, r16, r17 ; sb r25, r26 }
	{ nor r5, r6, r7 ; movei r15, 5 ; lw r25, r26 }
	{ nor r5, r6, r7 ; mzh r15, r16, r17 }
	{ nor r5, r6, r7 ; nor r15, r16, r17 }
	{ nor r5, r6, r7 ; ori r15, r16, 5 }
	{ nor r5, r6, r7 ; prefetch r25 ; ori r15, r16, 5 }
	{ nor r5, r6, r7 ; prefetch r25 ; srai r15, r16, 5 }
	{ nor r5, r6, r7 ; rli r15, r16, 5 ; prefetch r25 }
	{ nor r5, r6, r7 ; s2a r15, r16, r17 ; prefetch r25 }
	{ nor r5, r6, r7 ; sb r25, r26 ; fnop }
	{ nor r5, r6, r7 ; sb r25, r26 ; shr r15, r16, r17 }
	{ nor r5, r6, r7 ; seq r15, r16, r17 ; prefetch r25 }
	{ nor r5, r6, r7 ; sh r25, r26 ; add r15, r16, r17 }
	{ nor r5, r6, r7 ; sh r25, r26 ; seq r15, r16, r17 }
	{ nor r5, r6, r7 ; shl r15, r16, r17 ; lb_u r25, r26 }
	{ nor r5, r6, r7 ; shli r15, r16, 5 }
	{ nor r5, r6, r7 ; shri r15, r16, 5 ; prefetch r25 }
	{ nor r5, r6, r7 ; slt_u r15, r16, r17 ; lh_u r25, r26 }
	{ nor r5, r6, r7 ; slte_u r15, r16, r17 ; lb_u r25, r26 }
	{ nor r5, r6, r7 ; slti r15, r16, 5 ; prefetch r25 }
	{ nor r5, r6, r7 ; sne r15, r16, r17 ; lb_u r25, r26 }
	{ nor r5, r6, r7 ; sra r15, r16, r17 }
	{ nor r5, r6, r7 ; sub r15, r16, r17 ; prefetch r25 }
	{ nor r5, r6, r7 ; sw r25, r26 ; movei r15, 5 }
	{ nor r5, r6, r7 ; sw r25, r26 ; slte_u r15, r16, r17 }
	{ nor r5, r6, r7 ; xor r15, r16, r17 ; sw r25, r26 }
	{ or r15, r16, r17 ; addi r5, r6, 5 ; lh_u r25, r26 }
	{ or r15, r16, r17 ; and r5, r6, r7 ; sb r25, r26 }
	{ or r15, r16, r17 ; bitx r5, r6 ; lh_u r25, r26 }
	{ or r15, r16, r17 ; clz r5, r6 ; lh_u r25, r26 }
	{ or r15, r16, r17 ; fnop ; lb r25, r26 }
	{ or r15, r16, r17 ; infol 0x1234 }
	{ or r15, r16, r17 ; lb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ or r15, r16, r17 ; lb r25, r26 ; seq r5, r6, r7 }
	{ or r15, r16, r17 ; lb r25, r26 ; xor r5, r6, r7 }
	{ or r15, r16, r17 ; lb_u r25, r26 ; mulll_ss r5, r6, r7 }
	{ or r15, r16, r17 ; lb_u r25, r26 ; shli r5, r6, 5 }
	{ or r15, r16, r17 ; lh r25, r26 ; addi r5, r6, 5 }
	{ or r15, r16, r17 ; lh r25, r26 ; mullla_uu r5, r6, r7 }
	{ or r15, r16, r17 ; lh r25, r26 ; slt r5, r6, r7 }
	{ or r15, r16, r17 ; lh_u r25, r26 ; bitx r5, r6 }
	{ or r15, r16, r17 ; lh_u r25, r26 ; mz r5, r6, r7 }
	{ or r15, r16, r17 ; lh_u r25, r26 ; slte_u r5, r6, r7 }
	{ or r15, r16, r17 ; lw r25, r26 ; ctz r5, r6 }
	{ or r15, r16, r17 ; lw r25, r26 ; or r5, r6, r7 }
	{ or r15, r16, r17 ; lw r25, r26 ; sne r5, r6, r7 }
	{ or r15, r16, r17 ; mnz r5, r6, r7 ; lb_u r25, r26 }
	{ or r15, r16, r17 ; move r5, r6 }
	{ or r15, r16, r17 ; mulhh_ss r5, r6, r7 ; sh r25, r26 }
	{ or r15, r16, r17 ; mulhha_ss r5, r6, r7 ; sb r25, r26 }
	{ or r15, r16, r17 ; mulhla_ss r5, r6, r7 }
	{ or r15, r16, r17 ; mulll_ss r5, r6, r7 ; sb r25, r26 }
	{ or r15, r16, r17 ; mullla_ss r5, r6, r7 ; prefetch r25 }
	{ or r15, r16, r17 ; mvnz r5, r6, r7 ; lh_u r25, r26 }
	{ or r15, r16, r17 ; mz r5, r6, r7 ; lh_u r25, r26 }
	{ or r15, r16, r17 ; nor r5, r6, r7 ; lb_u r25, r26 }
	{ or r15, r16, r17 ; ori r5, r6, 5 ; lb_u r25, r26 }
	{ or r15, r16, r17 ; pcnt r5, r6 ; sh r25, r26 }
	{ or r15, r16, r17 ; prefetch r25 ; mulhlsa_uu r5, r6, r7 }
	{ or r15, r16, r17 ; prefetch r25 ; shl r5, r6, r7 }
	{ or r15, r16, r17 ; rl r5, r6, r7 ; lb r25, r26 }
	{ or r15, r16, r17 ; s1a r5, r6, r7 ; lb r25, r26 }
	{ or r15, r16, r17 ; s3a r5, r6, r7 ; lb r25, r26 }
	{ or r15, r16, r17 ; sb r25, r26 ; bitx r5, r6 }
	{ or r15, r16, r17 ; sb r25, r26 ; mz r5, r6, r7 }
	{ or r15, r16, r17 ; sb r25, r26 ; slte_u r5, r6, r7 }
	{ or r15, r16, r17 ; seq r5, r6, r7 ; sh r25, r26 }
	{ or r15, r16, r17 ; sh r25, r26 ; andi r5, r6, 5 }
	{ or r15, r16, r17 ; sh r25, r26 ; mvz r5, r6, r7 }
	{ or r15, r16, r17 ; sh r25, r26 ; slte r5, r6, r7 }
	{ or r15, r16, r17 ; shl r5, r6, r7 ; sb r25, r26 }
	{ or r15, r16, r17 ; shr r5, r6, r7 ; lh r25, r26 }
	{ or r15, r16, r17 ; shrib r5, r6, 5 }
	{ or r15, r16, r17 ; slt_u r5, r6, r7 ; sw r25, r26 }
	{ or r15, r16, r17 ; slte_u r5, r6, r7 ; sb r25, r26 }
	{ or r15, r16, r17 ; slti_u r5, r6, 5 ; lb r25, r26 }
	{ or r15, r16, r17 ; sne r5, r6, r7 ; sb r25, r26 }
	{ or r15, r16, r17 ; srai r5, r6, 5 ; lh r25, r26 }
	{ or r15, r16, r17 ; subb r5, r6, r7 }
	{ or r15, r16, r17 ; sw r25, r26 ; mulhha_ss r5, r6, r7 }
	{ or r15, r16, r17 ; sw r25, r26 ; seq r5, r6, r7 }
	{ or r15, r16, r17 ; sw r25, r26 ; xor r5, r6, r7 }
	{ or r15, r16, r17 ; tblidxb1 r5, r6 ; sw r25, r26 }
	{ or r15, r16, r17 ; tblidxb3 r5, r6 ; sw r25, r26 }
	{ or r5, r6, r7 ; add r15, r16, r17 ; sh r25, r26 }
	{ or r5, r6, r7 ; addlis r15, r16, 0x1234 }
	{ or r5, r6, r7 ; andi r15, r16, 5 ; sw r25, r26 }
	{ or r5, r6, r7 ; ill ; lh_u r25, r26 }
	{ or r5, r6, r7 ; intlb r15, r16, r17 }
	{ or r5, r6, r7 ; lb r25, r26 ; nop }
	{ or r5, r6, r7 ; lb r25, r26 ; slti_u r15, r16, 5 }
	{ or r5, r6, r7 ; lb_u r25, r26 ; nor r15, r16, r17 }
	{ or r5, r6, r7 ; lb_u r25, r26 ; sne r15, r16, r17 }
	{ or r5, r6, r7 ; lh r25, r26 ; nop }
	{ or r5, r6, r7 ; lh r25, r26 ; slti_u r15, r16, 5 }
	{ or r5, r6, r7 ; lh_u r25, r26 ; nor r15, r16, r17 }
	{ or r5, r6, r7 ; lh_u r25, r26 ; sne r15, r16, r17 }
	{ or r5, r6, r7 ; lw r25, r26 ; mz r15, r16, r17 }
	{ or r5, r6, r7 ; lw r25, r26 ; slti r15, r16, 5 }
	{ or r5, r6, r7 ; minih r15, r16, 5 }
	{ or r5, r6, r7 ; move r15, r16 ; sb r25, r26 }
	{ or r5, r6, r7 ; mz r15, r16, r17 ; lh_u r25, r26 }
	{ or r5, r6, r7 ; nor r15, r16, r17 ; lb_u r25, r26 }
	{ or r5, r6, r7 ; ori r15, r16, 5 ; lb_u r25, r26 }
	{ or r5, r6, r7 ; prefetch r25 ; info 19 }
	{ or r5, r6, r7 ; prefetch r25 ; slt r15, r16, r17 }
	{ or r5, r6, r7 ; rl r15, r16, r17 ; sh r25, r26 }
	{ or r5, r6, r7 ; s1a r15, r16, r17 ; sh r25, r26 }
	{ or r5, r6, r7 ; s3a r15, r16, r17 ; sh r25, r26 }
	{ or r5, r6, r7 ; sb r25, r26 ; rli r15, r16, 5 }
	{ or r5, r6, r7 ; sb r25, r26 ; xor r15, r16, r17 }
	{ or r5, r6, r7 ; seqi r15, r16, 5 ; prefetch r25 }
	{ or r5, r6, r7 ; sh r25, r26 ; nor r15, r16, r17 }
	{ or r5, r6, r7 ; sh r25, r26 ; sne r15, r16, r17 }
	{ or r5, r6, r7 ; shli r15, r16, 5 ; lb_u r25, r26 }
	{ or r5, r6, r7 ; shr r15, r16, r17 }
	{ or r5, r6, r7 ; slt r15, r16, r17 ; prefetch r25 }
	{ or r5, r6, r7 ; slte r15, r16, r17 ; lh_u r25, r26 }
	{ or r5, r6, r7 ; slteh_u r15, r16, r17 }
	{ or r5, r6, r7 ; slti_u r15, r16, 5 ; sh r25, r26 }
	{ or r5, r6, r7 ; sra r15, r16, r17 ; lb_u r25, r26 }
	{ or r5, r6, r7 ; srai r15, r16, 5 }
	{ or r5, r6, r7 ; sw r25, r26 ; addi r15, r16, 5 }
	{ or r5, r6, r7 ; sw r25, r26 ; seqi r15, r16, 5 }
	{ or r5, r6, r7 ; xor r15, r16, r17 ; lb r25, r26 }
	{ ori r15, r16, 5 ; add r5, r6, r7 }
	{ ori r15, r16, 5 ; adiffb_u r5, r6, r7 }
	{ ori r15, r16, 5 ; andi r5, r6, 5 ; sw r25, r26 }
	{ ori r15, r16, 5 ; bytex r5, r6 ; prefetch r25 }
	{ ori r15, r16, 5 ; ctz r5, r6 ; lh_u r25, r26 }
	{ ori r15, r16, 5 ; info 19 ; lh r25, r26 }
	{ ori r15, r16, 5 ; lb r25, r26 ; ctz r5, r6 }
	{ ori r15, r16, 5 ; lb r25, r26 ; or r5, r6, r7 }
	{ ori r15, r16, 5 ; lb r25, r26 ; sne r5, r6, r7 }
	{ ori r15, r16, 5 ; lb_u r25, r26 ; mnz r5, r6, r7 }
	{ ori r15, r16, 5 ; lb_u r25, r26 ; rl r5, r6, r7 }
	{ ori r15, r16, 5 ; lb_u r25, r26 ; sub r5, r6, r7 }
	{ ori r15, r16, 5 ; lh r25, r26 ; mulhh_ss r5, r6, r7 }
	{ ori r15, r16, 5 ; lh r25, r26 ; s2a r5, r6, r7 }
	{ ori r15, r16, 5 ; lh r25, r26 ; tblidxb2 r5, r6 }
	{ ori r15, r16, 5 ; lh_u r25, r26 ; mulhha_uu r5, r6, r7 }
	{ ori r15, r16, 5 ; lh_u r25, r26 ; seqi r5, r6, 5 }
	{ ori r15, r16, 5 ; lh_u r25, r26 }
	{ ori r15, r16, 5 ; lw r25, r26 ; mulll_uu r5, r6, r7 }
	{ ori r15, r16, 5 ; lw r25, r26 ; shr r5, r6, r7 }
	{ ori r15, r16, 5 ; maxib_u r5, r6, 5 }
	{ ori r15, r16, 5 ; move r5, r6 ; lb_u r25, r26 }
	{ ori r15, r16, 5 ; movelis r5, 0x1234 }
	{ ori r15, r16, 5 ; mulhh_uu r5, r6, r7 ; sw r25, r26 }
	{ ori r15, r16, 5 ; mulhha_uu r5, r6, r7 ; sh r25, r26 }
	{ ori r15, r16, 5 ; mulhlsa_uu r5, r6, r7 ; sw r25, r26 }
	{ ori r15, r16, 5 ; mulll_uu r5, r6, r7 ; sh r25, r26 }
	{ ori r15, r16, 5 ; mullla_uu r5, r6, r7 ; sb r25, r26 }
	{ ori r15, r16, 5 ; mvz r5, r6, r7 ; prefetch r25 }
	{ ori r15, r16, 5 ; nop ; lh_u r25, r26 }
	{ ori r15, r16, 5 ; or r5, r6, r7 ; lh_u r25, r26 }
	{ ori r15, r16, 5 ; packlb r5, r6, r7 }
	{ ori r15, r16, 5 ; prefetch r25 ; info 19 }
	{ ori r15, r16, 5 ; prefetch r25 ; pcnt r5, r6 }
	{ ori r15, r16, 5 ; prefetch r25 ; srai r5, r6, 5 }
	{ ori r15, r16, 5 ; rli r5, r6, 5 ; lh r25, r26 }
	{ ori r15, r16, 5 ; s2a r5, r6, r7 ; lh r25, r26 }
	{ ori r15, r16, 5 ; sadah_u r5, r6, r7 }
	{ ori r15, r16, 5 ; sb r25, r26 ; mulhha_uu r5, r6, r7 }
	{ ori r15, r16, 5 ; sb r25, r26 ; seqi r5, r6, 5 }
	{ ori r15, r16, 5 ; sb r25, r26 }
	{ ori r15, r16, 5 ; seqi r5, r6, 5 ; sh r25, r26 }
	{ ori r15, r16, 5 ; sh r25, r26 ; mulhha_ss r5, r6, r7 }
	{ ori r15, r16, 5 ; sh r25, r26 ; seq r5, r6, r7 }
	{ ori r15, r16, 5 ; sh r25, r26 ; xor r5, r6, r7 }
	{ ori r15, r16, 5 ; shli r5, r6, 5 ; sb r25, r26 }
	{ ori r15, r16, 5 ; shri r5, r6, 5 ; lh r25, r26 }
	{ ori r15, r16, 5 ; slt_u r5, r6, r7 ; lb r25, r26 }
	{ ori r15, r16, 5 ; slte r5, r6, r7 ; sw r25, r26 }
	{ ori r15, r16, 5 ; slti r5, r6, 5 ; lh r25, r26 }
	{ ori r15, r16, 5 ; sltih r5, r6, 5 }
	{ ori r15, r16, 5 ; sra r5, r6, r7 ; sb r25, r26 }
	{ ori r15, r16, 5 ; sub r5, r6, r7 ; lh r25, r26 }
	{ ori r15, r16, 5 ; sw r25, r26 ; ctz r5, r6 }
	{ ori r15, r16, 5 ; sw r25, r26 ; or r5, r6, r7 }
	{ ori r15, r16, 5 ; sw r25, r26 ; sne r5, r6, r7 }
	{ ori r15, r16, 5 ; tblidxb1 r5, r6 ; lb r25, r26 }
	{ ori r15, r16, 5 ; tblidxb3 r5, r6 ; lb r25, r26 }
	{ ori r15, r16, 5 ; xori r5, r6, 5 }
	{ ori r5, r6, 5 ; addi r15, r16, 5 ; prefetch r25 }
	{ ori r5, r6, 5 ; andi r15, r16, 5 ; lb r25, r26 }
	{ ori r5, r6, 5 ; fnop ; sb r25, r26 }
	{ ori r5, r6, 5 ; info 19 ; prefetch r25 }
	{ ori r5, r6, 5 ; lb r25, r26 ; andi r15, r16, 5 }
	{ ori r5, r6, 5 ; lb r25, r26 ; shli r15, r16, 5 }
	{ ori r5, r6, 5 ; lb_u r25, r26 ; fnop }
	{ ori r5, r6, 5 ; lb_u r25, r26 ; shr r15, r16, r17 }
	{ ori r5, r6, 5 ; lh r25, r26 ; andi r15, r16, 5 }
	{ ori r5, r6, 5 ; lh r25, r26 ; shli r15, r16, 5 }
	{ ori r5, r6, 5 ; lh_u r25, r26 ; fnop }
	{ ori r5, r6, 5 ; lh_u r25, r26 ; shr r15, r16, r17 }
	{ ori r5, r6, 5 ; lw r25, r26 ; and r15, r16, r17 }
	{ ori r5, r6, 5 ; lw r25, r26 ; shl r15, r16, r17 }
	{ ori r5, r6, 5 ; maxh r15, r16, r17 }
	{ ori r5, r6, 5 ; mnzb r15, r16, r17 }
	{ ori r5, r6, 5 ; movei r15, 5 ; sw r25, r26 }
	{ ori r5, r6, 5 ; nop ; lh_u r25, r26 }
	{ ori r5, r6, 5 ; or r15, r16, r17 ; lh_u r25, r26 }
	{ ori r5, r6, 5 ; packlb r15, r16, r17 }
	{ ori r5, r6, 5 ; prefetch r25 ; s2a r15, r16, r17 }
	{ ori r5, r6, 5 ; raise }
	{ ori r5, r6, 5 ; rli r15, r16, 5 }
	{ ori r5, r6, 5 ; s2a r15, r16, r17 }
	{ ori r5, r6, 5 ; sb r25, r26 ; move r15, r16 }
	{ ori r5, r6, 5 ; sb r25, r26 ; slte r15, r16, r17 }
	{ ori r5, r6, 5 ; seq r15, r16, r17 }
	{ ori r5, r6, 5 ; sh r25, r26 ; fnop }
	{ ori r5, r6, 5 ; sh r25, r26 ; shr r15, r16, r17 }
	{ ori r5, r6, 5 ; shl r15, r16, r17 ; prefetch r25 }
	{ ori r5, r6, 5 ; shr r15, r16, r17 ; lb_u r25, r26 }
	{ ori r5, r6, 5 ; shri r15, r16, 5 }
	{ ori r5, r6, 5 ; slt_u r15, r16, r17 ; sh r25, r26 }
	{ ori r5, r6, 5 ; slte_u r15, r16, r17 ; prefetch r25 }
	{ ori r5, r6, 5 ; slti r15, r16, 5 }
	{ ori r5, r6, 5 ; sne r15, r16, r17 ; prefetch r25 }
	{ ori r5, r6, 5 ; srai r15, r16, 5 ; lb_u r25, r26 }
	{ ori r5, r6, 5 ; sub r15, r16, r17 }
	{ ori r5, r6, 5 ; sw r25, r26 ; or r15, r16, r17 }
	{ ori r5, r6, 5 ; sw r25, r26 ; sra r15, r16, r17 }
	{ packbs_u r15, r16, r17 ; addb r5, r6, r7 }
	{ packbs_u r15, r16, r17 ; crc32_32 r5, r6, r7 }
	{ packbs_u r15, r16, r17 ; mnz r5, r6, r7 }
	{ packbs_u r15, r16, r17 ; mulhla_us r5, r6, r7 }
	{ packbs_u r15, r16, r17 ; packhb r5, r6, r7 }
	{ packbs_u r15, r16, r17 ; seqih r5, r6, 5 }
	{ packbs_u r15, r16, r17 ; slteb_u r5, r6, r7 }
	{ packbs_u r15, r16, r17 ; sub r5, r6, r7 }
	{ packbs_u r5, r6, r7 ; addli r15, r16, 0x1234 }
	{ packbs_u r5, r6, r7 ; jalr r15 }
	{ packbs_u r5, r6, r7 ; maxih r15, r16, 5 }
	{ packbs_u r5, r6, r7 ; nor r15, r16, r17 }
	{ packbs_u r5, r6, r7 ; seqib r15, r16, 5 }
	{ packbs_u r5, r6, r7 ; slte r15, r16, r17 }
	{ packbs_u r5, r6, r7 ; srai r15, r16, 5 }
	{ packhb r15, r16, r17 ; addi r5, r6, 5 }
	{ packhb r15, r16, r17 ; fnop }
	{ packhb r15, r16, r17 ; movei r5, 5 }
	{ packhb r15, r16, r17 ; mulll_su r5, r6, r7 }
	{ packhb r15, r16, r17 ; rl r5, r6, r7 }
	{ packhb r15, r16, r17 ; shli r5, r6, 5 }
	{ packhb r15, r16, r17 ; slth_u r5, r6, r7 }
	{ packhb r15, r16, r17 ; subhs r5, r6, r7 }
	{ packhb r5, r6, r7 ; andi r15, r16, 5 }
	{ packhb r5, r6, r7 ; lb r15, r16 }
	{ packhb r5, r6, r7 ; minh r15, r16, r17 }
	{ packhb r5, r6, r7 ; packhb r15, r16, r17 }
	{ packhb r5, r6, r7 ; shl r15, r16, r17 }
	{ packhb r5, r6, r7 ; slteh r15, r16, r17 }
	{ packhb r5, r6, r7 ; subb r15, r16, r17 }
	{ packhs r15, r16, r17 ; addlis r5, r6, 0x1234 }
	{ packhs r15, r16, r17 ; inthh r5, r6, r7 }
	{ packhs r15, r16, r17 ; mulhh_su r5, r6, r7 }
	{ packhs r15, r16, r17 ; mullla_uu r5, r6, r7 }
	{ packhs r15, r16, r17 ; s3a r5, r6, r7 }
	{ packhs r15, r16, r17 ; shrb r5, r6, r7 }
	{ packhs r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ packhs r15, r16, r17 ; tblidxb2 r5, r6 }
	{ packhs r5, r6, r7 ; flush r15 }
	{ packhs r5, r6, r7 ; lh r15, r16 }
	{ packhs r5, r6, r7 ; mnz r15, r16, r17 }
	{ packhs r5, r6, r7 ; raise }
	{ packhs r5, r6, r7 ; shlib r15, r16, 5 }
	{ packhs r5, r6, r7 ; slti r15, r16, 5 }
	{ packhs r5, r6, r7 ; subs r15, r16, r17 }
	{ packlb r15, r16, r17 ; and r5, r6, r7 }
	{ packlb r15, r16, r17 ; maxh r5, r6, r7 }
	{ packlb r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ packlb r15, r16, r17 ; mz r5, r6, r7 }
	{ packlb r15, r16, r17 ; sadb_u r5, r6, r7 }
	{ packlb r15, r16, r17 ; shrih r5, r6, 5 }
	{ packlb r15, r16, r17 ; sneb r5, r6, r7 }
	{ packlb r5, r6, r7 ; add r15, r16, r17 }
	{ packlb r5, r6, r7 ; info 19 }
	{ packlb r5, r6, r7 ; lnk r15 }
	{ packlb r5, r6, r7 ; movei r15, 5 }
	{ packlb r5, r6, r7 ; s2a r15, r16, r17 }
	{ packlb r5, r6, r7 ; shrh r15, r16, r17 }
	{ packlb r5, r6, r7 ; sltih r15, r16, 5 }
	{ packlb r5, r6, r7 ; wh64 r15 }
	{ pcnt r5, r6 ; addi r15, r16, 5 ; lh_u r25, r26 }
	{ pcnt r5, r6 ; and r15, r16, r17 ; sw r25, r26 }
	{ pcnt r5, r6 ; fnop ; lw r25, r26 }
	{ pcnt r5, r6 ; info 19 ; lh_u r25, r26 }
	{ pcnt r5, r6 ; lb r25, r26 ; addi r15, r16, 5 }
	{ pcnt r5, r6 ; lb r25, r26 ; seqi r15, r16, 5 }
	{ pcnt r5, r6 ; lb_u r25, r26 ; and r15, r16, r17 }
	{ pcnt r5, r6 ; lb_u r25, r26 ; shl r15, r16, r17 }
	{ pcnt r5, r6 ; lh r25, r26 ; addi r15, r16, 5 }
	{ pcnt r5, r6 ; lh r25, r26 ; seqi r15, r16, 5 }
	{ pcnt r5, r6 ; lh_u r25, r26 ; and r15, r16, r17 }
	{ pcnt r5, r6 ; lh_u r25, r26 ; shl r15, r16, r17 }
	{ pcnt r5, r6 ; lw r25, r26 ; add r15, r16, r17 }
	{ pcnt r5, r6 ; lw r25, r26 ; seq r15, r16, r17 }
	{ pcnt r5, r6 ; lwadd_na r15, r16, 5 }
	{ pcnt r5, r6 ; mnz r15, r16, r17 ; sw r25, r26 }
	{ pcnt r5, r6 ; movei r15, 5 ; sb r25, r26 }
	{ pcnt r5, r6 ; nop ; lb_u r25, r26 }
	{ pcnt r5, r6 ; or r15, r16, r17 ; lb_u r25, r26 }
	{ pcnt r5, r6 ; packhb r15, r16, r17 }
	{ pcnt r5, r6 ; prefetch r25 ; rli r15, r16, 5 }
	{ pcnt r5, r6 ; prefetch r25 ; xor r15, r16, r17 }
	{ pcnt r5, r6 ; rli r15, r16, 5 ; sh r25, r26 }
	{ pcnt r5, r6 ; s2a r15, r16, r17 ; sh r25, r26 }
	{ pcnt r5, r6 ; sb r25, r26 ; info 19 }
	{ pcnt r5, r6 ; sb r25, r26 ; slt r15, r16, r17 }
	{ pcnt r5, r6 ; seq r15, r16, r17 ; sh r25, r26 }
	{ pcnt r5, r6 ; sh r25, r26 ; and r15, r16, r17 }
	{ pcnt r5, r6 ; sh r25, r26 ; shl r15, r16, r17 }
	{ pcnt r5, r6 ; shl r15, r16, r17 ; lh_u r25, r26 }
	{ pcnt r5, r6 ; shlih r15, r16, 5 }
	{ pcnt r5, r6 ; shri r15, r16, 5 ; sh r25, r26 }
	{ pcnt r5, r6 ; slt_u r15, r16, r17 ; prefetch r25 }
	{ pcnt r5, r6 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
	{ pcnt r5, r6 ; slti r15, r16, 5 ; sh r25, r26 }
	{ pcnt r5, r6 ; sne r15, r16, r17 ; lh_u r25, r26 }
	{ pcnt r5, r6 ; srah r15, r16, r17 }
	{ pcnt r5, r6 ; sub r15, r16, r17 ; sh r25, r26 }
	{ pcnt r5, r6 ; sw r25, r26 ; nop }
	{ pcnt r5, r6 ; sw r25, r26 ; slti_u r15, r16, 5 }
	{ pcnt r5, r6 ; xori r15, r16, 5 }
	{ prefetch r15 ; bytex r5, r6 }
	{ prefetch r15 ; minih r5, r6, 5 }
	{ prefetch r15 ; mulhla_ss r5, r6, r7 }
	{ prefetch r15 ; ori r5, r6, 5 }
	{ prefetch r15 ; seqi r5, r6, 5 }
	{ prefetch r15 ; slte_u r5, r6, r7 }
	{ prefetch r15 ; sraib r5, r6, 5 }
	{ prefetch r25 ; add r15, r16, r17 ; clz r5, r6 }
	{ prefetch r25 ; add r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch r25 ; add r15, r16, r17 ; slti_u r5, r6, 5 }
	{ prefetch r25 ; add r5, r6, r7 ; movei r15, 5 }
	{ prefetch r25 ; add r5, r6, r7 ; slte_u r15, r16, r17 }
	{ prefetch r25 ; addi r15, r16, 5 ; move r5, r6 }
	{ prefetch r25 ; addi r15, r16, 5 ; rli r5, r6, 5 }
	{ prefetch r25 ; addi r15, r16, 5 ; tblidxb0 r5, r6 }
	{ prefetch r25 ; addi r5, r6, 5 ; ori r15, r16, 5 }
	{ prefetch r25 ; addi r5, r6, 5 ; srai r15, r16, 5 }
	{ prefetch r25 ; and r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ prefetch r25 ; and r15, r16, r17 ; seqi r5, r6, 5 }
	{ prefetch r25 ; and r15, r16, r17 }
	{ prefetch r25 ; and r5, r6, r7 ; s3a r15, r16, r17 }
	{ prefetch r25 ; andi r15, r16, 5 ; addi r5, r6, 5 }
	{ prefetch r25 ; andi r15, r16, 5 ; mullla_uu r5, r6, r7 }
	{ prefetch r25 ; andi r15, r16, 5 ; slt r5, r6, r7 }
	{ prefetch r25 ; andi r5, r6, 5 ; fnop }
	{ prefetch r25 ; andi r5, r6, 5 ; shr r15, r16, r17 }
	{ prefetch r25 ; bitx r5, r6 ; info 19 }
	{ prefetch r25 ; bitx r5, r6 ; slt r15, r16, r17 }
	{ prefetch r25 ; bytex r5, r6 ; move r15, r16 }
	{ prefetch r25 ; bytex r5, r6 ; slte r15, r16, r17 }
	{ prefetch r25 ; clz r5, r6 ; mz r15, r16, r17 }
	{ prefetch r25 ; clz r5, r6 ; slti r15, r16, 5 }
	{ prefetch r25 ; ctz r5, r6 ; nor r15, r16, r17 }
	{ prefetch r25 ; ctz r5, r6 ; sne r15, r16, r17 }
	{ prefetch r25 ; fnop ; info 19 }
	{ prefetch r25 ; fnop ; nop }
	{ prefetch r25 ; fnop ; seqi r15, r16, 5 }
	{ prefetch r25 ; fnop ; slti_u r15, r16, 5 }
	{ prefetch r25 ; ill ; andi r5, r6, 5 }
	{ prefetch r25 ; ill ; mvz r5, r6, r7 }
	{ prefetch r25 ; ill ; slte r5, r6, r7 }
	{ prefetch r25 ; info 19 ; andi r15, r16, 5 }
	{ prefetch r25 ; info 19 ; mulll_ss r5, r6, r7 }
	{ prefetch r25 ; info 19 ; s1a r15, r16, r17 }
	{ prefetch r25 ; info 19 ; slt_u r15, r16, r17 }
	{ prefetch r25 ; info 19 ; tblidxb2 r5, r6 }
	{ prefetch r25 ; mnz r15, r16, r17 ; mulhha_ss r5, r6, r7 }
	{ prefetch r25 ; mnz r15, r16, r17 ; seq r5, r6, r7 }
	{ prefetch r25 ; mnz r15, r16, r17 ; xor r5, r6, r7 }
	{ prefetch r25 ; mnz r5, r6, r7 ; s2a r15, r16, r17 }
	{ prefetch r25 ; move r15, r16 ; add r5, r6, r7 }
	{ prefetch r25 ; move r15, r16 ; mullla_ss r5, r6, r7 }
	{ prefetch r25 ; move r15, r16 ; shri r5, r6, 5 }
	{ prefetch r25 ; move r5, r6 ; andi r15, r16, 5 }
	{ prefetch r25 ; move r5, r6 ; shli r15, r16, 5 }
	{ prefetch r25 ; movei r15, 5 ; bytex r5, r6 }
	{ prefetch r25 ; movei r15, 5 ; nop }
	{ prefetch r25 ; movei r15, 5 ; slti r5, r6, 5 }
	{ prefetch r25 ; movei r5, 5 ; move r15, r16 }
	{ prefetch r25 ; movei r5, 5 ; slte r15, r16, r17 }
	{ prefetch r25 ; mulhh_ss r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch r25 ; mulhh_ss r5, r6, r7 ; slti r15, r16, 5 }
	{ prefetch r25 ; mulhh_uu r5, r6, r7 ; nor r15, r16, r17 }
	{ prefetch r25 ; mulhh_uu r5, r6, r7 ; sne r15, r16, r17 }
	{ prefetch r25 ; mulhha_ss r5, r6, r7 ; ori r15, r16, 5 }
	{ prefetch r25 ; mulhha_ss r5, r6, r7 ; srai r15, r16, 5 }
	{ prefetch r25 ; mulhha_uu r5, r6, r7 ; rli r15, r16, 5 }
	{ prefetch r25 ; mulhha_uu r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch r25 ; mulhlsa_uu r5, r6, r7 ; s2a r15, r16, r17 }
	{ prefetch r25 ; mulll_ss r5, r6, r7 ; add r15, r16, r17 }
	{ prefetch r25 ; mulll_ss r5, r6, r7 ; seq r15, r16, r17 }
	{ prefetch r25 ; mulll_uu r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch r25 ; mulll_uu r5, r6, r7 ; shl r15, r16, r17 }
	{ prefetch r25 ; mullla_ss r5, r6, r7 ; fnop }
	{ prefetch r25 ; mullla_ss r5, r6, r7 ; shr r15, r16, r17 }
	{ prefetch r25 ; mullla_uu r5, r6, r7 ; info 19 }
	{ prefetch r25 ; mullla_uu r5, r6, r7 ; slt r15, r16, r17 }
	{ prefetch r25 ; mvnz r5, r6, r7 ; move r15, r16 }
	{ prefetch r25 ; mvnz r5, r6, r7 ; slte r15, r16, r17 }
	{ prefetch r25 ; mvz r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch r25 ; mvz r5, r6, r7 ; slti r15, r16, 5 }
	{ prefetch r25 ; mz r15, r16, r17 ; movei r5, 5 }
	{ prefetch r25 ; mz r15, r16, r17 ; s1a r5, r6, r7 }
	{ prefetch r25 ; mz r15, r16, r17 ; tblidxb1 r5, r6 }
	{ prefetch r25 ; mz r5, r6, r7 ; rl r15, r16, r17 }
	{ prefetch r25 ; mz r5, r6, r7 ; sub r15, r16, r17 }
	{ prefetch r25 ; nop ; move r15, r16 }
	{ prefetch r25 ; nop ; or r15, r16, r17 }
	{ prefetch r25 ; nop ; shl r5, r6, r7 }
	{ prefetch r25 ; nop ; sne r5, r6, r7 }
	{ prefetch r25 ; nor r15, r16, r17 ; clz r5, r6 }
	{ prefetch r25 ; nor r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch r25 ; nor r15, r16, r17 ; slti_u r5, r6, 5 }
	{ prefetch r25 ; nor r5, r6, r7 ; movei r15, 5 }
	{ prefetch r25 ; nor r5, r6, r7 ; slte_u r15, r16, r17 }
	{ prefetch r25 ; or r15, r16, r17 ; move r5, r6 }
	{ prefetch r25 ; or r15, r16, r17 ; rli r5, r6, 5 }
	{ prefetch r25 ; or r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch r25 ; or r5, r6, r7 ; ori r15, r16, 5 }
	{ prefetch r25 ; or r5, r6, r7 ; srai r15, r16, 5 }
	{ prefetch r25 ; ori r15, r16, 5 ; mulhha_uu r5, r6, r7 }
	{ prefetch r25 ; ori r15, r16, 5 ; seqi r5, r6, 5 }
	{ prefetch r25 ; ori r15, r16, 5 }
	{ prefetch r25 ; ori r5, r6, 5 ; s3a r15, r16, r17 }
	{ prefetch r25 ; pcnt r5, r6 ; addi r15, r16, 5 }
	{ prefetch r25 ; pcnt r5, r6 ; seqi r15, r16, 5 }
	{ prefetch r25 ; rl r15, r16, r17 ; andi r5, r6, 5 }
	{ prefetch r25 ; rl r15, r16, r17 ; mvz r5, r6, r7 }
	{ prefetch r25 ; rl r15, r16, r17 ; slte r5, r6, r7 }
	{ prefetch r25 ; rl r5, r6, r7 ; info 19 }
	{ prefetch r25 ; rl r5, r6, r7 ; slt r15, r16, r17 }
	{ prefetch r25 ; rli r15, r16, 5 ; fnop }
	{ prefetch r25 ; rli r15, r16, 5 ; ori r5, r6, 5 }
	{ prefetch r25 ; rli r15, r16, 5 ; sra r5, r6, r7 }
	{ prefetch r25 ; rli r5, r6, 5 ; nop }
	{ prefetch r25 ; rli r5, r6, 5 ; slti_u r15, r16, 5 }
	{ prefetch r25 ; s1a r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ prefetch r25 ; s1a r15, r16, r17 ; s2a r5, r6, r7 }
	{ prefetch r25 ; s1a r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch r25 ; s1a r5, r6, r7 ; rli r15, r16, 5 }
	{ prefetch r25 ; s1a r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch r25 ; s2a r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ prefetch r25 ; s2a r15, r16, r17 ; shli r5, r6, 5 }
	{ prefetch r25 ; s2a r5, r6, r7 ; addi r15, r16, 5 }
	{ prefetch r25 ; s2a r5, r6, r7 ; seqi r15, r16, 5 }
	{ prefetch r25 ; s3a r15, r16, r17 ; andi r5, r6, 5 }
	{ prefetch r25 ; s3a r15, r16, r17 ; mvz r5, r6, r7 }
	{ prefetch r25 ; s3a r15, r16, r17 ; slte r5, r6, r7 }
	{ prefetch r25 ; s3a r5, r6, r7 ; info 19 }
	{ prefetch r25 ; s3a r5, r6, r7 ; slt r15, r16, r17 }
	{ prefetch r25 ; seq r15, r16, r17 ; fnop }
	{ prefetch r25 ; seq r15, r16, r17 ; ori r5, r6, 5 }
	{ prefetch r25 ; seq r15, r16, r17 ; sra r5, r6, r7 }
	{ prefetch r25 ; seq r5, r6, r7 ; nop }
	{ prefetch r25 ; seq r5, r6, r7 ; slti_u r15, r16, 5 }
	{ prefetch r25 ; seqi r15, r16, 5 ; mulhh_ss r5, r6, r7 }
	{ prefetch r25 ; seqi r15, r16, 5 ; s2a r5, r6, r7 }
	{ prefetch r25 ; seqi r15, r16, 5 ; tblidxb2 r5, r6 }
	{ prefetch r25 ; seqi r5, r6, 5 ; rli r15, r16, 5 }
	{ prefetch r25 ; seqi r5, r6, 5 ; xor r15, r16, r17 }
	{ prefetch r25 ; shl r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ prefetch r25 ; shl r15, r16, r17 ; shli r5, r6, 5 }
	{ prefetch r25 ; shl r5, r6, r7 ; addi r15, r16, 5 }
	{ prefetch r25 ; shl r5, r6, r7 ; seqi r15, r16, 5 }
	{ prefetch r25 ; shli r15, r16, 5 ; andi r5, r6, 5 }
	{ prefetch r25 ; shli r15, r16, 5 ; mvz r5, r6, r7 }
	{ prefetch r25 ; shli r15, r16, 5 ; slte r5, r6, r7 }
	{ prefetch r25 ; shli r5, r6, 5 ; info 19 }
	{ prefetch r25 ; shli r5, r6, 5 ; slt r15, r16, r17 }
	{ prefetch r25 ; shr r15, r16, r17 ; fnop }
	{ prefetch r25 ; shr r15, r16, r17 ; ori r5, r6, 5 }
	{ prefetch r25 ; shr r15, r16, r17 ; sra r5, r6, r7 }
	{ prefetch r25 ; shr r5, r6, r7 ; nop }
	{ prefetch r25 ; shr r5, r6, r7 ; slti_u r15, r16, 5 }
	{ prefetch r25 ; shri r15, r16, 5 ; mulhh_ss r5, r6, r7 }
	{ prefetch r25 ; shri r15, r16, 5 ; s2a r5, r6, r7 }
	{ prefetch r25 ; shri r15, r16, 5 ; tblidxb2 r5, r6 }
	{ prefetch r25 ; shri r5, r6, 5 ; rli r15, r16, 5 }
	{ prefetch r25 ; shri r5, r6, 5 ; xor r15, r16, r17 }
	{ prefetch r25 ; slt r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ prefetch r25 ; slt r15, r16, r17 ; shli r5, r6, 5 }
	{ prefetch r25 ; slt r5, r6, r7 ; addi r15, r16, 5 }
	{ prefetch r25 ; slt r5, r6, r7 ; seqi r15, r16, 5 }
	{ prefetch r25 ; slt_u r15, r16, r17 ; andi r5, r6, 5 }
	{ prefetch r25 ; slt_u r15, r16, r17 ; mvz r5, r6, r7 }
	{ prefetch r25 ; slt_u r15, r16, r17 ; slte r5, r6, r7 }
	{ prefetch r25 ; slt_u r5, r6, r7 ; info 19 }
	{ prefetch r25 ; slt_u r5, r6, r7 ; slt r15, r16, r17 }
	{ prefetch r25 ; slte r15, r16, r17 ; fnop }
	{ prefetch r25 ; slte r15, r16, r17 ; ori r5, r6, 5 }
	{ prefetch r25 ; slte r15, r16, r17 ; sra r5, r6, r7 }
	{ prefetch r25 ; slte r5, r6, r7 ; nop }
	{ prefetch r25 ; slte r5, r6, r7 ; slti_u r15, r16, 5 }
	{ prefetch r25 ; slte_u r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ prefetch r25 ; slte_u r15, r16, r17 ; s2a r5, r6, r7 }
	{ prefetch r25 ; slte_u r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch r25 ; slte_u r5, r6, r7 ; rli r15, r16, 5 }
	{ prefetch r25 ; slte_u r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch r25 ; slti r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ prefetch r25 ; slti r15, r16, 5 ; shli r5, r6, 5 }
	{ prefetch r25 ; slti r5, r6, 5 ; addi r15, r16, 5 }
	{ prefetch r25 ; slti r5, r6, 5 ; seqi r15, r16, 5 }
	{ prefetch r25 ; slti_u r15, r16, 5 ; andi r5, r6, 5 }
	{ prefetch r25 ; slti_u r15, r16, 5 ; mvz r5, r6, r7 }
	{ prefetch r25 ; slti_u r15, r16, 5 ; slte r5, r6, r7 }
	{ prefetch r25 ; slti_u r5, r6, 5 ; info 19 }
	{ prefetch r25 ; slti_u r5, r6, 5 ; slt r15, r16, r17 }
	{ prefetch r25 ; sne r15, r16, r17 ; fnop }
	{ prefetch r25 ; sne r15, r16, r17 ; ori r5, r6, 5 }
	{ prefetch r25 ; sne r15, r16, r17 ; sra r5, r6, r7 }
	{ prefetch r25 ; sne r5, r6, r7 ; nop }
	{ prefetch r25 ; sne r5, r6, r7 ; slti_u r15, r16, 5 }
	{ prefetch r25 ; sra r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ prefetch r25 ; sra r15, r16, r17 ; s2a r5, r6, r7 }
	{ prefetch r25 ; sra r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch r25 ; sra r5, r6, r7 ; rli r15, r16, 5 }
	{ prefetch r25 ; sra r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch r25 ; srai r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ prefetch r25 ; srai r15, r16, 5 ; shli r5, r6, 5 }
	{ prefetch r25 ; srai r5, r6, 5 ; addi r15, r16, 5 }
	{ prefetch r25 ; srai r5, r6, 5 ; seqi r15, r16, 5 }
	{ prefetch r25 ; sub r15, r16, r17 ; andi r5, r6, 5 }
	{ prefetch r25 ; sub r15, r16, r17 ; mvz r5, r6, r7 }
	{ prefetch r25 ; sub r15, r16, r17 ; slte r5, r6, r7 }
	{ prefetch r25 ; sub r5, r6, r7 ; info 19 }
	{ prefetch r25 ; sub r5, r6, r7 ; slt r15, r16, r17 }
	{ prefetch r25 ; tblidxb0 r5, r6 ; move r15, r16 }
	{ prefetch r25 ; tblidxb0 r5, r6 ; slte r15, r16, r17 }
	{ prefetch r25 ; tblidxb1 r5, r6 ; mz r15, r16, r17 }
	{ prefetch r25 ; tblidxb1 r5, r6 ; slti r15, r16, 5 }
	{ prefetch r25 ; tblidxb2 r5, r6 ; nor r15, r16, r17 }
	{ prefetch r25 ; tblidxb2 r5, r6 ; sne r15, r16, r17 }
	{ prefetch r25 ; tblidxb3 r5, r6 ; ori r15, r16, 5 }
	{ prefetch r25 ; tblidxb3 r5, r6 ; srai r15, r16, 5 }
	{ prefetch r25 ; xor r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ prefetch r25 ; xor r15, r16, r17 ; seqi r5, r6, 5 }
	{ prefetch r25 ; xor r15, r16, r17 }
	{ prefetch r25 ; xor r5, r6, r7 ; s3a r15, r16, r17 }
	{ raise ; addb r5, r6, r7 }
	{ raise ; crc32_32 r5, r6, r7 }
	{ raise ; mnz r5, r6, r7 }
	{ raise ; mulhla_us r5, r6, r7 }
	{ raise ; packhb r5, r6, r7 }
	{ raise ; seqih r5, r6, 5 }
	{ raise ; slteb_u r5, r6, r7 }
	{ raise ; sub r5, r6, r7 }
	{ rl r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
	{ rl r15, r16, r17 ; adds r5, r6, r7 }
	{ rl r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
	{ rl r15, r16, r17 ; bytex r5, r6 ; lw r25, r26 }
	{ rl r15, r16, r17 ; ctz r5, r6 ; lh r25, r26 }
	{ rl r15, r16, r17 ; info 19 ; lb_u r25, r26 }
	{ rl r15, r16, r17 ; lb r25, r26 ; clz r5, r6 }
	{ rl r15, r16, r17 ; lb r25, r26 ; nor r5, r6, r7 }
	{ rl r15, r16, r17 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ rl r15, r16, r17 ; lb_u r25, r26 ; info 19 }
	{ rl r15, r16, r17 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ rl r15, r16, r17 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ rl r15, r16, r17 ; lh r25, r26 ; movei r5, 5 }
	{ rl r15, r16, r17 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ rl r15, r16, r17 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ rl r15, r16, r17 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ rl r15, r16, r17 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ rl r15, r16, r17 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ rl r15, r16, r17 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ rl r15, r16, r17 ; lw r25, r26 ; shli r5, r6, 5 }
	{ rl r15, r16, r17 ; maxh r5, r6, r7 }
	{ rl r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
	{ rl r15, r16, r17 ; moveli r5, 0x1234 }
	{ rl r15, r16, r17 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ rl r15, r16, r17 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ rl r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ rl r15, r16, r17 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ rl r15, r16, r17 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ rl r15, r16, r17 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ rl r15, r16, r17 ; nop ; lh r25, r26 }
	{ rl r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
	{ rl r15, r16, r17 ; packhs r5, r6, r7 }
	{ rl r15, r16, r17 ; prefetch r25 ; fnop }
	{ rl r15, r16, r17 ; prefetch r25 ; ori r5, r6, 5 }
	{ rl r15, r16, r17 ; prefetch r25 ; sra r5, r6, r7 }
	{ rl r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ rl r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ rl r15, r16, r17 ; sadah r5, r6, r7 }
	{ rl r15, r16, r17 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ rl r15, r16, r17 ; sb r25, r26 ; seq r5, r6, r7 }
	{ rl r15, r16, r17 ; sb r25, r26 ; xor r5, r6, r7 }
	{ rl r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ rl r15, r16, r17 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ rl r15, r16, r17 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ rl r15, r16, r17 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ rl r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
	{ rl r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ rl r15, r16, r17 ; slt r5, r6, r7 }
	{ rl r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
	{ rl r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ rl r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ rl r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
	{ rl r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ rl r15, r16, r17 ; sw r25, r26 ; clz r5, r6 }
	{ rl r15, r16, r17 ; sw r25, r26 ; nor r5, r6, r7 }
	{ rl r15, r16, r17 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ rl r15, r16, r17 ; tblidxb0 r5, r6 }
	{ rl r15, r16, r17 ; tblidxb2 r5, r6 }
	{ rl r15, r16, r17 ; xor r5, r6, r7 }
	{ rl r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
	{ rl r5, r6, r7 ; and r15, r16, r17 }
	{ rl r5, r6, r7 ; fnop ; prefetch r25 }
	{ rl r5, r6, r7 ; info 19 ; lw r25, r26 }
	{ rl r5, r6, r7 ; lb r25, r26 ; and r15, r16, r17 }
	{ rl r5, r6, r7 ; lb r25, r26 ; shl r15, r16, r17 }
	{ rl r5, r6, r7 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ rl r5, r6, r7 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ rl r5, r6, r7 ; lh r25, r26 ; and r15, r16, r17 }
	{ rl r5, r6, r7 ; lh r25, r26 ; shl r15, r16, r17 }
	{ rl r5, r6, r7 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ rl r5, r6, r7 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ rl r5, r6, r7 ; lw r25, r26 ; addi r15, r16, 5 }
	{ rl r5, r6, r7 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ rl r5, r6, r7 ; maxb_u r15, r16, r17 }
	{ rl r5, r6, r7 ; mnz r15, r16, r17 }
	{ rl r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
	{ rl r5, r6, r7 ; nop ; lh r25, r26 }
	{ rl r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
	{ rl r5, r6, r7 ; packhs r15, r16, r17 }
	{ rl r5, r6, r7 ; prefetch r25 ; s1a r15, r16, r17 }
	{ rl r5, r6, r7 ; prefetch r25 }
	{ rl r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
	{ rl r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ rl r5, r6, r7 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ rl r5, r6, r7 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ rl r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
	{ rl r5, r6, r7 ; sh r25, r26 ; andi r15, r16, 5 }
	{ rl r5, r6, r7 ; sh r25, r26 ; shli r15, r16, 5 }
	{ rl r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
	{ rl r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
	{ rl r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
	{ rl r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ rl r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ rl r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
	{ rl r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
	{ rl r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
	{ rl r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
	{ rl r5, r6, r7 ; sw r25, r26 ; nor r15, r16, r17 }
	{ rl r5, r6, r7 ; sw r25, r26 ; sne r15, r16, r17 }
	{ rli r15, r16, 5 ; add r5, r6, r7 ; lb r25, r26 }
	{ rli r15, r16, 5 ; addi r5, r6, 5 ; sb r25, r26 }
	{ rli r15, r16, 5 ; and r5, r6, r7 }
	{ rli r15, r16, 5 ; bitx r5, r6 ; sb r25, r26 }
	{ rli r15, r16, 5 ; clz r5, r6 ; sb r25, r26 }
	{ rli r15, r16, 5 ; fnop ; lh_u r25, r26 }
	{ rli r15, r16, 5 ; intlb r5, r6, r7 }
	{ rli r15, r16, 5 ; lb r25, r26 ; mulll_ss r5, r6, r7 }
	{ rli r15, r16, 5 ; lb r25, r26 ; shli r5, r6, 5 }
	{ rli r15, r16, 5 ; lb_u r25, r26 ; addi r5, r6, 5 }
	{ rli r15, r16, 5 ; lb_u r25, r26 ; mullla_uu r5, r6, r7 }
	{ rli r15, r16, 5 ; lb_u r25, r26 ; slt r5, r6, r7 }
	{ rli r15, r16, 5 ; lh r25, r26 ; bitx r5, r6 }
	{ rli r15, r16, 5 ; lh r25, r26 ; mz r5, r6, r7 }
	{ rli r15, r16, 5 ; lh r25, r26 ; slte_u r5, r6, r7 }
	{ rli r15, r16, 5 ; lh_u r25, r26 ; ctz r5, r6 }
	{ rli r15, r16, 5 ; lh_u r25, r26 ; or r5, r6, r7 }
	{ rli r15, r16, 5 ; lh_u r25, r26 ; sne r5, r6, r7 }
	{ rli r15, r16, 5 ; lw r25, r26 ; mnz r5, r6, r7 }
	{ rli r15, r16, 5 ; lw r25, r26 ; rl r5, r6, r7 }
	{ rli r15, r16, 5 ; lw r25, r26 ; sub r5, r6, r7 }
	{ rli r15, r16, 5 ; mnz r5, r6, r7 ; lw r25, r26 }
	{ rli r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
	{ rli r15, r16, 5 ; mulhh_su r5, r6, r7 }
	{ rli r15, r16, 5 ; mulhha_ss r5, r6, r7 }
	{ rli r15, r16, 5 ; mulhla_uu r5, r6, r7 }
	{ rli r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ rli r15, r16, 5 ; mullla_ss r5, r6, r7 ; sw r25, r26 }
	{ rli r15, r16, 5 ; mvnz r5, r6, r7 ; sb r25, r26 }
	{ rli r15, r16, 5 ; mz r5, r6, r7 ; sb r25, r26 }
	{ rli r15, r16, 5 ; nor r5, r6, r7 ; lw r25, r26 }
	{ rli r15, r16, 5 ; ori r5, r6, 5 ; lw r25, r26 }
	{ rli r15, r16, 5 ; prefetch r25 ; add r5, r6, r7 }
	{ rli r15, r16, 5 ; prefetch r25 ; mullla_ss r5, r6, r7 }
	{ rli r15, r16, 5 ; prefetch r25 ; shri r5, r6, 5 }
	{ rli r15, r16, 5 ; rl r5, r6, r7 ; lh_u r25, r26 }
	{ rli r15, r16, 5 ; s1a r5, r6, r7 ; lh_u r25, r26 }
	{ rli r15, r16, 5 ; s3a r5, r6, r7 ; lh_u r25, r26 }
	{ rli r15, r16, 5 ; sb r25, r26 ; ctz r5, r6 }
	{ rli r15, r16, 5 ; sb r25, r26 ; or r5, r6, r7 }
	{ rli r15, r16, 5 ; sb r25, r26 ; sne r5, r6, r7 }
	{ rli r15, r16, 5 ; seqb r5, r6, r7 }
	{ rli r15, r16, 5 ; sh r25, r26 ; clz r5, r6 }
	{ rli r15, r16, 5 ; sh r25, r26 ; nor r5, r6, r7 }
	{ rli r15, r16, 5 ; sh r25, r26 ; slti_u r5, r6, 5 }
	{ rli r15, r16, 5 ; shl r5, r6, r7 }
	{ rli r15, r16, 5 ; shr r5, r6, r7 ; prefetch r25 }
	{ rli r15, r16, 5 ; slt r5, r6, r7 ; lb_u r25, r26 }
	{ rli r15, r16, 5 ; sltb_u r5, r6, r7 }
	{ rli r15, r16, 5 ; slte_u r5, r6, r7 }
	{ rli r15, r16, 5 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
	{ rli r15, r16, 5 ; sne r5, r6, r7 }
	{ rli r15, r16, 5 ; srai r5, r6, 5 ; prefetch r25 }
	{ rli r15, r16, 5 ; subhs r5, r6, r7 }
	{ rli r15, r16, 5 ; sw r25, r26 ; mulll_ss r5, r6, r7 }
	{ rli r15, r16, 5 ; sw r25, r26 ; shli r5, r6, 5 }
	{ rli r15, r16, 5 ; tblidxb0 r5, r6 ; lb_u r25, r26 }
	{ rli r15, r16, 5 ; tblidxb2 r5, r6 ; lb_u r25, r26 }
	{ rli r15, r16, 5 ; xor r5, r6, r7 ; lb_u r25, r26 }
	{ rli r5, r6, 5 ; addb r15, r16, r17 }
	{ rli r5, r6, 5 ; and r15, r16, r17 ; lb_u r25, r26 }
	{ rli r5, r6, 5 ; dtlbpr r15 }
	{ rli r5, r6, 5 ; ill ; sb r25, r26 }
	{ rli r5, r6, 5 ; iret }
	{ rli r5, r6, 5 ; lb r25, r26 ; ori r15, r16, 5 }
	{ rli r5, r6, 5 ; lb r25, r26 ; srai r15, r16, 5 }
	{ rli r5, r6, 5 ; lb_u r25, r26 ; rl r15, r16, r17 }
	{ rli r5, r6, 5 ; lb_u r25, r26 ; sub r15, r16, r17 }
	{ rli r5, r6, 5 ; lh r25, r26 ; ori r15, r16, 5 }
	{ rli r5, r6, 5 ; lh r25, r26 ; srai r15, r16, 5 }
	{ rli r5, r6, 5 ; lh_u r25, r26 ; rl r15, r16, r17 }
	{ rli r5, r6, 5 ; lh_u r25, r26 ; sub r15, r16, r17 }
	{ rli r5, r6, 5 ; lw r25, r26 ; or r15, r16, r17 }
	{ rli r5, r6, 5 ; lw r25, r26 ; sra r15, r16, r17 }
	{ rli r5, r6, 5 ; mnz r15, r16, r17 ; lb_u r25, r26 }
	{ rli r5, r6, 5 ; move r15, r16 }
	{ rli r5, r6, 5 ; mz r15, r16, r17 ; sb r25, r26 }
	{ rli r5, r6, 5 ; nor r15, r16, r17 ; lw r25, r26 }
	{ rli r5, r6, 5 ; ori r15, r16, 5 ; lw r25, r26 }
	{ rli r5, r6, 5 ; prefetch r25 ; movei r15, 5 }
	{ rli r5, r6, 5 ; prefetch r25 ; slte_u r15, r16, r17 }
	{ rli r5, r6, 5 ; rli r15, r16, 5 ; lb r25, r26 }
	{ rli r5, r6, 5 ; s2a r15, r16, r17 ; lb r25, r26 }
	{ rli r5, r6, 5 ; sb r15, r16 }
	{ rli r5, r6, 5 ; sb r25, r26 ; s3a r15, r16, r17 }
	{ rli r5, r6, 5 ; seq r15, r16, r17 ; lb r25, r26 }
	{ rli r5, r6, 5 ; seqi r15, r16, 5 ; sw r25, r26 }
	{ rli r5, r6, 5 ; sh r25, r26 ; rl r15, r16, r17 }
	{ rli r5, r6, 5 ; sh r25, r26 ; sub r15, r16, r17 }
	{ rli r5, r6, 5 ; shli r15, r16, 5 ; lw r25, r26 }
	{ rli r5, r6, 5 ; shri r15, r16, 5 ; lb r25, r26 }
	{ rli r5, r6, 5 ; slt r15, r16, r17 ; sw r25, r26 }
	{ rli r5, r6, 5 ; slte r15, r16, r17 ; sb r25, r26 }
	{ rli r5, r6, 5 ; slti r15, r16, 5 ; lb r25, r26 }
	{ rli r5, r6, 5 ; sltib r15, r16, 5 }
	{ rli r5, r6, 5 ; sra r15, r16, r17 ; lw r25, r26 }
	{ rli r5, r6, 5 ; sub r15, r16, r17 ; lb r25, r26 }
	{ rli r5, r6, 5 ; sw r25, r26 ; fnop }
	{ rli r5, r6, 5 ; sw r25, r26 ; shr r15, r16, r17 }
	{ rli r5, r6, 5 ; xor r15, r16, r17 ; lh_u r25, r26 }
	{ s1a r15, r16, r17 ; addh r5, r6, r7 }
	{ s1a r15, r16, r17 ; and r5, r6, r7 ; lb_u r25, r26 }
	{ s1a r15, r16, r17 ; avgb_u r5, r6, r7 }
	{ s1a r15, r16, r17 ; bytex r5, r6 ; sw r25, r26 }
	{ s1a r15, r16, r17 ; ctz r5, r6 ; sb r25, r26 }
	{ s1a r15, r16, r17 ; info 19 ; prefetch r25 }
	{ s1a r15, r16, r17 ; lb r25, r26 ; mnz r5, r6, r7 }
	{ s1a r15, r16, r17 ; lb r25, r26 ; rl r5, r6, r7 }
	{ s1a r15, r16, r17 ; lb r25, r26 ; sub r5, r6, r7 }
	{ s1a r15, r16, r17 ; lb_u r25, r26 ; mulhh_ss r5, r6, r7 }
	{ s1a r15, r16, r17 ; lb_u r25, r26 ; s2a r5, r6, r7 }
	{ s1a r15, r16, r17 ; lb_u r25, r26 ; tblidxb2 r5, r6 }
	{ s1a r15, r16, r17 ; lh r25, r26 ; mulhha_uu r5, r6, r7 }
	{ s1a r15, r16, r17 ; lh r25, r26 ; seqi r5, r6, 5 }
	{ s1a r15, r16, r17 ; lh r25, r26 }
	{ s1a r15, r16, r17 ; lh_u r25, r26 ; mulll_uu r5, r6, r7 }
	{ s1a r15, r16, r17 ; lh_u r25, r26 ; shr r5, r6, r7 }
	{ s1a r15, r16, r17 ; lw r25, r26 ; and r5, r6, r7 }
	{ s1a r15, r16, r17 ; lw r25, r26 ; mvnz r5, r6, r7 }
	{ s1a r15, r16, r17 ; lw r25, r26 ; slt_u r5, r6, r7 }
	{ s1a r15, r16, r17 ; minh r5, r6, r7 }
	{ s1a r15, r16, r17 ; move r5, r6 ; lw r25, r26 }
	{ s1a r15, r16, r17 ; mulhh_ss r5, r6, r7 ; lh r25, r26 }
	{ s1a r15, r16, r17 ; mulhha_ss r5, r6, r7 ; lb_u r25, r26 }
	{ s1a r15, r16, r17 ; mulhhsa_uu r5, r6, r7 }
	{ s1a r15, r16, r17 ; mulll_ss r5, r6, r7 ; lb_u r25, r26 }
	{ s1a r15, r16, r17 ; mullla_ss r5, r6, r7 ; lb r25, r26 }
	{ s1a r15, r16, r17 ; mullla_uu r5, r6, r7 }
	{ s1a r15, r16, r17 ; mvz r5, r6, r7 ; sw r25, r26 }
	{ s1a r15, r16, r17 ; nop ; sb r25, r26 }
	{ s1a r15, r16, r17 ; or r5, r6, r7 ; sb r25, r26 }
	{ s1a r15, r16, r17 ; pcnt r5, r6 ; lh r25, r26 }
	{ s1a r15, r16, r17 ; prefetch r25 ; movei r5, 5 }
	{ s1a r15, r16, r17 ; prefetch r25 ; s1a r5, r6, r7 }
	{ s1a r15, r16, r17 ; prefetch r25 ; tblidxb1 r5, r6 }
	{ s1a r15, r16, r17 ; rli r5, r6, 5 ; prefetch r25 }
	{ s1a r15, r16, r17 ; s2a r5, r6, r7 ; prefetch r25 }
	{ s1a r15, r16, r17 ; sadh_u r5, r6, r7 }
	{ s1a r15, r16, r17 ; sb r25, r26 ; mulll_uu r5, r6, r7 }
	{ s1a r15, r16, r17 ; sb r25, r26 ; shr r5, r6, r7 }
	{ s1a r15, r16, r17 ; seq r5, r6, r7 ; lh r25, r26 }
	{ s1a r15, r16, r17 ; seqib r5, r6, 5 }
	{ s1a r15, r16, r17 ; sh r25, r26 ; mulll_ss r5, r6, r7 }
	{ s1a r15, r16, r17 ; sh r25, r26 ; shli r5, r6, 5 }
	{ s1a r15, r16, r17 ; shl r5, r6, r7 ; lb_u r25, r26 }
	{ s1a r15, r16, r17 ; shli r5, r6, 5 }
	{ s1a r15, r16, r17 ; shri r5, r6, 5 ; prefetch r25 }
	{ s1a r15, r16, r17 ; slt_u r5, r6, r7 ; lh_u r25, r26 }
	{ s1a r15, r16, r17 ; slte_u r5, r6, r7 ; lb_u r25, r26 }
	{ s1a r15, r16, r17 ; slti r5, r6, 5 ; prefetch r25 }
	{ s1a r15, r16, r17 ; sne r5, r6, r7 ; lb_u r25, r26 }
	{ s1a r15, r16, r17 ; sra r5, r6, r7 }
	{ s1a r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
	{ s1a r15, r16, r17 ; sw r25, r26 ; mnz r5, r6, r7 }
	{ s1a r15, r16, r17 ; sw r25, r26 ; rl r5, r6, r7 }
	{ s1a r15, r16, r17 ; sw r25, r26 ; sub r5, r6, r7 }
	{ s1a r15, r16, r17 ; tblidxb1 r5, r6 ; lh_u r25, r26 }
	{ s1a r15, r16, r17 ; tblidxb3 r5, r6 ; lh_u r25, r26 }
	{ s1a r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
	{ s1a r5, r6, r7 ; addi r15, r16, 5 ; sw r25, r26 }
	{ s1a r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
	{ s1a r5, r6, r7 ; fnop }
	{ s1a r5, r6, r7 ; info 19 ; sw r25, r26 }
	{ s1a r5, r6, r7 ; lb r25, r26 ; info 19 }
	{ s1a r5, r6, r7 ; lb r25, r26 ; slt r15, r16, r17 }
	{ s1a r5, r6, r7 ; lb_u r25, r26 ; mnz r15, r16, r17 }
	{ s1a r5, r6, r7 ; lb_u r25, r26 ; slt_u r15, r16, r17 }
	{ s1a r5, r6, r7 ; lh r25, r26 ; info 19 }
	{ s1a r5, r6, r7 ; lh r25, r26 ; slt r15, r16, r17 }
	{ s1a r5, r6, r7 ; lh_u r25, r26 ; mnz r15, r16, r17 }
	{ s1a r5, r6, r7 ; lh_u r25, r26 ; slt_u r15, r16, r17 }
	{ s1a r5, r6, r7 ; lw r25, r26 ; ill }
	{ s1a r5, r6, r7 ; lw r25, r26 ; shri r15, r16, 5 }
	{ s1a r5, r6, r7 ; mf }
	{ s1a r5, r6, r7 ; move r15, r16 ; lb_u r25, r26 }
	{ s1a r5, r6, r7 ; movelis r15, 0x1234 }
	{ s1a r5, r6, r7 ; nop ; sb r25, r26 }
	{ s1a r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
	{ s1a r5, r6, r7 ; prefetch r25 ; addi r15, r16, 5 }
	{ s1a r5, r6, r7 ; prefetch r25 ; seqi r15, r16, 5 }
	{ s1a r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
	{ s1a r5, r6, r7 ; s1a r15, r16, r17 ; lh r25, r26 }
	{ s1a r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
	{ s1a r5, r6, r7 ; sb r25, r26 ; nop }
	{ s1a r5, r6, r7 ; sb r25, r26 ; slti_u r15, r16, 5 }
	{ s1a r5, r6, r7 ; seqi r15, r16, 5 ; lb r25, r26 }
	{ s1a r5, r6, r7 ; sh r25, r26 ; mnz r15, r16, r17 }
	{ s1a r5, r6, r7 ; sh r25, r26 ; slt_u r15, r16, r17 }
	{ s1a r5, r6, r7 ; shl r15, r16, r17 ; sw r25, r26 }
	{ s1a r5, r6, r7 ; shr r15, r16, r17 ; lw r25, r26 }
	{ s1a r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
	{ s1a r5, r6, r7 ; sltb r15, r16, r17 }
	{ s1a r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
	{ s1a r5, r6, r7 ; slti_u r15, r16, 5 ; lh r25, r26 }
	{ s1a r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
	{ s1a r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
	{ s1a r5, r6, r7 ; subh r15, r16, r17 }
	{ s1a r5, r6, r7 ; sw r25, r26 ; rli r15, r16, 5 }
	{ s1a r5, r6, r7 ; sw r25, r26 ; xor r15, r16, r17 }
	{ s2a r15, r16, r17 ; add r5, r6, r7 ; lw r25, r26 }
	{ s2a r15, r16, r17 ; addib r5, r6, 5 }
	{ s2a r15, r16, r17 ; andi r5, r6, 5 ; lh_u r25, r26 }
	{ s2a r15, r16, r17 ; bytex r5, r6 ; lb r25, r26 }
	{ s2a r15, r16, r17 ; crc32_32 r5, r6, r7 }
	{ s2a r15, r16, r17 ; fnop ; sh r25, r26 }
	{ s2a r15, r16, r17 ; lb r25, r26 ; and r5, r6, r7 }
	{ s2a r15, r16, r17 ; lb r25, r26 ; mvnz r5, r6, r7 }
	{ s2a r15, r16, r17 ; lb r25, r26 ; slt_u r5, r6, r7 }
	{ s2a r15, r16, r17 ; lb_u r25, r26 ; bytex r5, r6 }
	{ s2a r15, r16, r17 ; lb_u r25, r26 ; nop }
	{ s2a r15, r16, r17 ; lb_u r25, r26 ; slti r5, r6, 5 }
	{ s2a r15, r16, r17 ; lh r25, r26 ; fnop }
	{ s2a r15, r16, r17 ; lh r25, r26 ; ori r5, r6, 5 }
	{ s2a r15, r16, r17 ; lh r25, r26 ; sra r5, r6, r7 }
	{ s2a r15, r16, r17 ; lh_u r25, r26 ; move r5, r6 }
	{ s2a r15, r16, r17 ; lh_u r25, r26 ; rli r5, r6, 5 }
	{ s2a r15, r16, r17 ; lh_u r25, r26 ; tblidxb0 r5, r6 }
	{ s2a r15, r16, r17 ; lw r25, r26 ; mulhh_uu r5, r6, r7 }
	{ s2a r15, r16, r17 ; lw r25, r26 ; s3a r5, r6, r7 }
	{ s2a r15, r16, r17 ; lw r25, r26 ; tblidxb3 r5, r6 }
	{ s2a r15, r16, r17 ; mnz r5, r6, r7 ; sw r25, r26 }
	{ s2a r15, r16, r17 ; movei r5, 5 ; sb r25, r26 }
	{ s2a r15, r16, r17 ; mulhh_uu r5, r6, r7 ; lh_u r25, r26 }
	{ s2a r15, r16, r17 ; mulhha_uu r5, r6, r7 ; lh r25, r26 }
	{ s2a r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; lh_u r25, r26 }
	{ s2a r15, r16, r17 ; mulll_uu r5, r6, r7 ; lh r25, r26 }
	{ s2a r15, r16, r17 ; mullla_uu r5, r6, r7 ; lb_u r25, r26 }
	{ s2a r15, r16, r17 ; mvz r5, r6, r7 ; lb r25, r26 }
	{ s2a r15, r16, r17 ; mzb r5, r6, r7 }
	{ s2a r15, r16, r17 ; nor r5, r6, r7 ; sw r25, r26 }
	{ s2a r15, r16, r17 ; ori r5, r6, 5 ; sw r25, r26 }
	{ s2a r15, r16, r17 ; prefetch r25 ; bitx r5, r6 }
	{ s2a r15, r16, r17 ; prefetch r25 ; mz r5, r6, r7 }
	{ s2a r15, r16, r17 ; prefetch r25 ; slte_u r5, r6, r7 }
	{ s2a r15, r16, r17 ; rl r5, r6, r7 ; sh r25, r26 }
	{ s2a r15, r16, r17 ; s1a r5, r6, r7 ; sh r25, r26 }
	{ s2a r15, r16, r17 ; s3a r5, r6, r7 ; sh r25, r26 }
	{ s2a r15, r16, r17 ; sb r25, r26 ; move r5, r6 }
	{ s2a r15, r16, r17 ; sb r25, r26 ; rli r5, r6, 5 }
	{ s2a r15, r16, r17 ; sb r25, r26 ; tblidxb0 r5, r6 }
	{ s2a r15, r16, r17 ; seqi r5, r6, 5 ; lh r25, r26 }
	{ s2a r15, r16, r17 ; sh r25, r26 ; mnz r5, r6, r7 }
	{ s2a r15, r16, r17 ; sh r25, r26 ; rl r5, r6, r7 }
	{ s2a r15, r16, r17 ; sh r25, r26 ; sub r5, r6, r7 }
	{ s2a r15, r16, r17 ; shli r5, r6, 5 ; lb_u r25, r26 }
	{ s2a r15, r16, r17 ; shr r5, r6, r7 }
	{ s2a r15, r16, r17 ; slt r5, r6, r7 ; prefetch r25 }
	{ s2a r15, r16, r17 ; slte r5, r6, r7 ; lh_u r25, r26 }
	{ s2a r15, r16, r17 ; slteh_u r5, r6, r7 }
	{ s2a r15, r16, r17 ; slti_u r5, r6, 5 ; sh r25, r26 }
	{ s2a r15, r16, r17 ; sra r5, r6, r7 ; lb_u r25, r26 }
	{ s2a r15, r16, r17 ; srai r5, r6, 5 }
	{ s2a r15, r16, r17 ; sw r25, r26 ; and r5, r6, r7 }
	{ s2a r15, r16, r17 ; sw r25, r26 ; mvnz r5, r6, r7 }
	{ s2a r15, r16, r17 ; sw r25, r26 ; slt_u r5, r6, r7 }
	{ s2a r15, r16, r17 ; tblidxb0 r5, r6 ; prefetch r25 }
	{ s2a r15, r16, r17 ; tblidxb2 r5, r6 ; prefetch r25 }
	{ s2a r15, r16, r17 ; xor r5, r6, r7 ; prefetch r25 }
	{ s2a r5, r6, r7 ; addi r15, r16, 5 ; lb r25, r26 }
	{ s2a r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
	{ s2a r5, r6, r7 ; fnop ; lb_u r25, r26 }
	{ s2a r5, r6, r7 ; info 19 ; lb r25, r26 }
	{ s2a r5, r6, r7 ; jrp r15 }
	{ s2a r5, r6, r7 ; lb r25, r26 ; s2a r15, r16, r17 }
	{ s2a r5, r6, r7 ; lb_u r15, r16 }
	{ s2a r5, r6, r7 ; lb_u r25, r26 ; s3a r15, r16, r17 }
	{ s2a r5, r6, r7 ; lbadd_u r15, r16, 5 }
	{ s2a r5, r6, r7 ; lh r25, r26 ; s2a r15, r16, r17 }
	{ s2a r5, r6, r7 ; lh_u r15, r16 }
	{ s2a r5, r6, r7 ; lh_u r25, r26 ; s3a r15, r16, r17 }
	{ s2a r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ s2a r5, r6, r7 ; lw r25, r26 ; s1a r15, r16, r17 }
	{ s2a r5, r6, r7 ; lw r25, r26 }
	{ s2a r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
	{ s2a r5, r6, r7 ; movei r15, 5 ; lh_u r25, r26 }
	{ s2a r5, r6, r7 ; mzb r15, r16, r17 }
	{ s2a r5, r6, r7 ; nor r15, r16, r17 ; sw r25, r26 }
	{ s2a r5, r6, r7 ; ori r15, r16, 5 ; sw r25, r26 }
	{ s2a r5, r6, r7 ; prefetch r25 ; or r15, r16, r17 }
	{ s2a r5, r6, r7 ; prefetch r25 ; sra r15, r16, r17 }
	{ s2a r5, r6, r7 ; rli r15, r16, 5 ; lw r25, r26 }
	{ s2a r5, r6, r7 ; s2a r15, r16, r17 ; lw r25, r26 }
	{ s2a r5, r6, r7 ; sb r25, r26 ; andi r15, r16, 5 }
	{ s2a r5, r6, r7 ; sb r25, r26 ; shli r15, r16, 5 }
	{ s2a r5, r6, r7 ; seq r15, r16, r17 ; lw r25, r26 }
	{ s2a r5, r6, r7 ; sh r15, r16 }
	{ s2a r5, r6, r7 ; sh r25, r26 ; s3a r15, r16, r17 }
	{ s2a r5, r6, r7 ; shl r15, r16, r17 ; lb r25, r26 }
	{ s2a r5, r6, r7 ; shli r15, r16, 5 ; sw r25, r26 }
	{ s2a r5, r6, r7 ; shri r15, r16, 5 ; lw r25, r26 }
	{ s2a r5, r6, r7 ; slt_u r15, r16, r17 ; lh r25, r26 }
	{ s2a r5, r6, r7 ; slte_u r15, r16, r17 ; lb r25, r26 }
	{ s2a r5, r6, r7 ; slti r15, r16, 5 ; lw r25, r26 }
	{ s2a r5, r6, r7 ; sne r15, r16, r17 ; lb r25, r26 }
	{ s2a r5, r6, r7 ; sra r15, r16, r17 ; sw r25, r26 }
	{ s2a r5, r6, r7 ; sub r15, r16, r17 ; lw r25, r26 }
	{ s2a r5, r6, r7 ; sw r25, r26 ; move r15, r16 }
	{ s2a r5, r6, r7 ; sw r25, r26 ; slte r15, r16, r17 }
	{ s2a r5, r6, r7 ; xor r15, r16, r17 ; sh r25, r26 }
	{ s3a r15, r16, r17 ; addi r5, r6, 5 ; lh r25, r26 }
	{ s3a r15, r16, r17 ; and r5, r6, r7 ; prefetch r25 }
	{ s3a r15, r16, r17 ; bitx r5, r6 ; lh r25, r26 }
	{ s3a r15, r16, r17 ; clz r5, r6 ; lh r25, r26 }
	{ s3a r15, r16, r17 ; dword_align r5, r6, r7 }
	{ s3a r15, r16, r17 ; info 19 }
	{ s3a r15, r16, r17 ; lb r25, r26 ; mulhh_uu r5, r6, r7 }
	{ s3a r15, r16, r17 ; lb r25, r26 ; s3a r5, r6, r7 }
	{ s3a r15, r16, r17 ; lb r25, r26 ; tblidxb3 r5, r6 }
	{ s3a r15, r16, r17 ; lb_u r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ s3a r15, r16, r17 ; lb_u r25, r26 ; shl r5, r6, r7 }
	{ s3a r15, r16, r17 ; lh r25, r26 ; add r5, r6, r7 }
	{ s3a r15, r16, r17 ; lh r25, r26 ; mullla_ss r5, r6, r7 }
	{ s3a r15, r16, r17 ; lh r25, r26 ; shri r5, r6, 5 }
	{ s3a r15, r16, r17 ; lh_u r25, r26 ; andi r5, r6, 5 }
	{ s3a r15, r16, r17 ; lh_u r25, r26 ; mvz r5, r6, r7 }
	{ s3a r15, r16, r17 ; lh_u r25, r26 ; slte r5, r6, r7 }
	{ s3a r15, r16, r17 ; lw r25, r26 ; clz r5, r6 }
	{ s3a r15, r16, r17 ; lw r25, r26 ; nor r5, r6, r7 }
	{ s3a r15, r16, r17 ; lw r25, r26 ; slti_u r5, r6, 5 }
	{ s3a r15, r16, r17 ; mnz r5, r6, r7 ; lb r25, r26 }
	{ s3a r15, r16, r17 ; move r5, r6 ; sw r25, r26 }
	{ s3a r15, r16, r17 ; mulhh_ss r5, r6, r7 ; sb r25, r26 }
	{ s3a r15, r16, r17 ; mulhha_ss r5, r6, r7 ; prefetch r25 }
	{ s3a r15, r16, r17 ; mulhl_uu r5, r6, r7 }
	{ s3a r15, r16, r17 ; mulll_ss r5, r6, r7 ; prefetch r25 }
	{ s3a r15, r16, r17 ; mullla_ss r5, r6, r7 ; lw r25, r26 }
	{ s3a r15, r16, r17 ; mvnz r5, r6, r7 ; lh r25, r26 }
	{ s3a r15, r16, r17 ; mz r5, r6, r7 ; lh r25, r26 }
	{ s3a r15, r16, r17 ; nor r5, r6, r7 ; lb r25, r26 }
	{ s3a r15, r16, r17 ; ori r5, r6, 5 ; lb r25, r26 }
	{ s3a r15, r16, r17 ; pcnt r5, r6 ; sb r25, r26 }
	{ s3a r15, r16, r17 ; prefetch r25 ; mulhha_uu r5, r6, r7 }
	{ s3a r15, r16, r17 ; prefetch r25 ; seqi r5, r6, 5 }
	{ s3a r15, r16, r17 ; prefetch r25 }
	{ s3a r15, r16, r17 ; rli r5, r6, 5 }
	{ s3a r15, r16, r17 ; s2a r5, r6, r7 }
	{ s3a r15, r16, r17 ; sb r25, r26 ; andi r5, r6, 5 }
	{ s3a r15, r16, r17 ; sb r25, r26 ; mvz r5, r6, r7 }
	{ s3a r15, r16, r17 ; sb r25, r26 ; slte r5, r6, r7 }
	{ s3a r15, r16, r17 ; seq r5, r6, r7 ; sb r25, r26 }
	{ s3a r15, r16, r17 ; sh r25, r26 ; and r5, r6, r7 }
	{ s3a r15, r16, r17 ; sh r25, r26 ; mvnz r5, r6, r7 }
	{ s3a r15, r16, r17 ; sh r25, r26 ; slt_u r5, r6, r7 }
	{ s3a r15, r16, r17 ; shl r5, r6, r7 ; prefetch r25 }
	{ s3a r15, r16, r17 ; shr r5, r6, r7 ; lb_u r25, r26 }
	{ s3a r15, r16, r17 ; shri r5, r6, 5 }
	{ s3a r15, r16, r17 ; slt_u r5, r6, r7 ; sh r25, r26 }
	{ s3a r15, r16, r17 ; slte_u r5, r6, r7 ; prefetch r25 }
	{ s3a r15, r16, r17 ; slti r5, r6, 5 }
	{ s3a r15, r16, r17 ; sne r5, r6, r7 ; prefetch r25 }
	{ s3a r15, r16, r17 ; srai r5, r6, 5 ; lb_u r25, r26 }
	{ s3a r15, r16, r17 ; sub r5, r6, r7 }
	{ s3a r15, r16, r17 ; sw r25, r26 ; mulhh_uu r5, r6, r7 }
	{ s3a r15, r16, r17 ; sw r25, r26 ; s3a r5, r6, r7 }
	{ s3a r15, r16, r17 ; sw r25, r26 ; tblidxb3 r5, r6 }
	{ s3a r15, r16, r17 ; tblidxb1 r5, r6 ; sh r25, r26 }
	{ s3a r15, r16, r17 ; tblidxb3 r5, r6 ; sh r25, r26 }
	{ s3a r5, r6, r7 ; add r15, r16, r17 ; sb r25, r26 }
	{ s3a r5, r6, r7 ; addli r15, r16, 0x1234 }
	{ s3a r5, r6, r7 ; andi r15, r16, 5 ; sh r25, r26 }
	{ s3a r5, r6, r7 ; ill ; lh r25, r26 }
	{ s3a r5, r6, r7 ; inthh r15, r16, r17 }
	{ s3a r5, r6, r7 ; lb r25, r26 ; mz r15, r16, r17 }
	{ s3a r5, r6, r7 ; lb r25, r26 ; slti r15, r16, 5 }
	{ s3a r5, r6, r7 ; lb_u r25, r26 ; nop }
	{ s3a r5, r6, r7 ; lb_u r25, r26 ; slti_u r15, r16, 5 }
	{ s3a r5, r6, r7 ; lh r25, r26 ; mz r15, r16, r17 }
	{ s3a r5, r6, r7 ; lh r25, r26 ; slti r15, r16, 5 }
	{ s3a r5, r6, r7 ; lh_u r25, r26 ; nop }
	{ s3a r5, r6, r7 ; lh_u r25, r26 ; slti_u r15, r16, 5 }
	{ s3a r5, r6, r7 ; lw r25, r26 ; movei r15, 5 }
	{ s3a r5, r6, r7 ; lw r25, r26 ; slte_u r15, r16, r17 }
	{ s3a r5, r6, r7 ; minib_u r15, r16, 5 }
	{ s3a r5, r6, r7 ; move r15, r16 ; prefetch r25 }
	{ s3a r5, r6, r7 ; mz r15, r16, r17 ; lh r25, r26 }
	{ s3a r5, r6, r7 ; nor r15, r16, r17 ; lb r25, r26 }
	{ s3a r5, r6, r7 ; ori r15, r16, 5 ; lb r25, r26 }
	{ s3a r5, r6, r7 ; prefetch r25 ; ill }
	{ s3a r5, r6, r7 ; prefetch r25 ; shri r15, r16, 5 }
	{ s3a r5, r6, r7 ; rl r15, r16, r17 ; sb r25, r26 }
	{ s3a r5, r6, r7 ; s1a r15, r16, r17 ; sb r25, r26 }
	{ s3a r5, r6, r7 ; s3a r15, r16, r17 ; sb r25, r26 }
	{ s3a r5, r6, r7 ; sb r25, r26 ; rl r15, r16, r17 }
	{ s3a r5, r6, r7 ; sb r25, r26 ; sub r15, r16, r17 }
	{ s3a r5, r6, r7 ; seqi r15, r16, 5 ; lw r25, r26 }
	{ s3a r5, r6, r7 ; sh r25, r26 ; nop }
	{ s3a r5, r6, r7 ; sh r25, r26 ; slti_u r15, r16, 5 }
	{ s3a r5, r6, r7 ; shli r15, r16, 5 ; lb r25, r26 }
	{ s3a r5, r6, r7 ; shr r15, r16, r17 ; sw r25, r26 }
	{ s3a r5, r6, r7 ; slt r15, r16, r17 ; lw r25, r26 }
	{ s3a r5, r6, r7 ; slte r15, r16, r17 ; lh r25, r26 }
	{ s3a r5, r6, r7 ; slteh r15, r16, r17 }
	{ s3a r5, r6, r7 ; slti_u r15, r16, 5 ; sb r25, r26 }
	{ s3a r5, r6, r7 ; sra r15, r16, r17 ; lb r25, r26 }
	{ s3a r5, r6, r7 ; srai r15, r16, 5 ; sw r25, r26 }
	{ s3a r5, r6, r7 ; sw r25, r26 ; add r15, r16, r17 }
	{ s3a r5, r6, r7 ; sw r25, r26 ; seq r15, r16, r17 }
	{ s3a r5, r6, r7 ; wh64 r15 }
	{ sadab_u r5, r6, r7 ; addli r15, r16, 0x1234 }
	{ sadab_u r5, r6, r7 ; jalr r15 }
	{ sadab_u r5, r6, r7 ; maxih r15, r16, 5 }
	{ sadab_u r5, r6, r7 ; nor r15, r16, r17 }
	{ sadab_u r5, r6, r7 ; seqib r15, r16, 5 }
	{ sadab_u r5, r6, r7 ; slte r15, r16, r17 }
	{ sadab_u r5, r6, r7 ; srai r15, r16, 5 }
	{ sadah r5, r6, r7 ; addi r15, r16, 5 }
	{ sadah r5, r6, r7 ; intlh r15, r16, r17 }
	{ sadah r5, r6, r7 ; maxb_u r15, r16, r17 }
	{ sadah r5, r6, r7 ; mzb r15, r16, r17 }
	{ sadah r5, r6, r7 ; seqb r15, r16, r17 }
	{ sadah r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sadah r5, r6, r7 ; sra r15, r16, r17 }
	{ sadah_u r5, r6, r7 ; addbs_u r15, r16, r17 }
	{ sadah_u r5, r6, r7 ; inthb r15, r16, r17 }
	{ sadah_u r5, r6, r7 ; lw_na r15, r16 }
	{ sadah_u r5, r6, r7 ; movelis r15, 0x1234 }
	{ sadah_u r5, r6, r7 ; sb r15, r16 }
	{ sadah_u r5, r6, r7 ; shrib r15, r16, 5 }
	{ sadah_u r5, r6, r7 ; sne r15, r16, r17 }
	{ sadah_u r5, r6, r7 ; xori r15, r16, 5 }
	{ sadb_u r5, r6, r7 ; ill }
	{ sadb_u r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ sadb_u r5, r6, r7 ; move r15, r16 }
	{ sadb_u r5, r6, r7 ; s1a r15, r16, r17 }
	{ sadb_u r5, r6, r7 ; shrb r15, r16, r17 }
	{ sadb_u r5, r6, r7 ; sltib_u r15, r16, 5 }
	{ sadb_u r5, r6, r7 ; tns r15, r16 }
	{ sadh r5, r6, r7 ; flush r15 }
	{ sadh r5, r6, r7 ; lh r15, r16 }
	{ sadh r5, r6, r7 ; mnz r15, r16, r17 }
	{ sadh r5, r6, r7 ; raise }
	{ sadh r5, r6, r7 ; shlib r15, r16, 5 }
	{ sadh r5, r6, r7 ; slti r15, r16, 5 }
	{ sadh r5, r6, r7 ; subs r15, r16, r17 }
	{ sadh_u r5, r6, r7 ; auli r15, r16, 0x1234 }
	{ sadh_u r5, r6, r7 ; lb_u r15, r16 }
	{ sadh_u r5, r6, r7 ; minib_u r15, r16, 5 }
	{ sadh_u r5, r6, r7 ; packhs r15, r16, r17 }
	{ sadh_u r5, r6, r7 ; shlb r15, r16, r17 }
	{ sadh_u r5, r6, r7 ; slteh_u r15, r16, r17 }
	{ sadh_u r5, r6, r7 ; subbs_u r15, r16, r17 }
	{ sb r15, r16 ; adds r5, r6, r7 }
	{ sb r15, r16 ; intlb r5, r6, r7 }
	{ sb r15, r16 ; mulhh_uu r5, r6, r7 }
	{ sb r15, r16 ; mulllsa_uu r5, r6, r7 }
	{ sb r15, r16 ; sadab_u r5, r6, r7 }
	{ sb r15, r16 ; shrh r5, r6, r7 }
	{ sb r15, r16 ; sltih r5, r6, 5 }
	{ sb r15, r16 ; tblidxb3 r5, r6 }
	{ sb r25, r26 ; add r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ sb r25, r26 ; add r15, r16, r17 ; shl r5, r6, r7 }
	{ sb r25, r26 ; add r5, r6, r7 ; add r15, r16, r17 }
	{ sb r25, r26 ; add r5, r6, r7 ; seq r15, r16, r17 }
	{ sb r25, r26 ; addi r15, r16, 5 ; and r5, r6, r7 }
	{ sb r25, r26 ; addi r15, r16, 5 ; mvnz r5, r6, r7 }
	{ sb r25, r26 ; addi r15, r16, 5 ; slt_u r5, r6, r7 }
	{ sb r25, r26 ; addi r5, r6, 5 ; ill }
	{ sb r25, r26 ; addi r5, r6, 5 ; shri r15, r16, 5 }
	{ sb r25, r26 ; and r15, r16, r17 ; ctz r5, r6 }
	{ sb r25, r26 ; and r15, r16, r17 ; or r5, r6, r7 }
	{ sb r25, r26 ; and r15, r16, r17 ; sne r5, r6, r7 }
	{ sb r25, r26 ; and r5, r6, r7 ; mz r15, r16, r17 }
	{ sb r25, r26 ; and r5, r6, r7 ; slti r15, r16, 5 }
	{ sb r25, r26 ; andi r15, r16, 5 ; movei r5, 5 }
	{ sb r25, r26 ; andi r15, r16, 5 ; s1a r5, r6, r7 }
	{ sb r25, r26 ; andi r15, r16, 5 ; tblidxb1 r5, r6 }
	{ sb r25, r26 ; andi r5, r6, 5 ; rl r15, r16, r17 }
	{ sb r25, r26 ; andi r5, r6, 5 ; sub r15, r16, r17 }
	{ sb r25, r26 ; bitx r5, r6 ; s1a r15, r16, r17 }
	{ sb r25, r26 ; bitx r5, r6 }
	{ sb r25, r26 ; bytex r5, r6 ; s3a r15, r16, r17 }
	{ sb r25, r26 ; clz r5, r6 ; addi r15, r16, 5 }
	{ sb r25, r26 ; clz r5, r6 ; seqi r15, r16, 5 }
	{ sb r25, r26 ; ctz r5, r6 ; andi r15, r16, 5 }
	{ sb r25, r26 ; ctz r5, r6 ; shli r15, r16, 5 }
	{ sb r25, r26 ; fnop ; and r5, r6, r7 }
	{ sb r25, r26 ; fnop ; mulhlsa_uu r5, r6, r7 }
	{ sb r25, r26 ; fnop ; rli r5, r6, 5 }
	{ sb r25, r26 ; fnop ; slt r5, r6, r7 }
	{ sb r25, r26 ; fnop ; tblidxb1 r5, r6 }
	{ sb r25, r26 ; ill ; mulhh_uu r5, r6, r7 }
	{ sb r25, r26 ; ill ; s3a r5, r6, r7 }
	{ sb r25, r26 ; ill ; tblidxb3 r5, r6 }
	{ sb r25, r26 ; info 19 ; move r15, r16 }
	{ sb r25, r26 ; info 19 ; or r15, r16, r17 }
	{ sb r25, r26 ; info 19 ; shl r5, r6, r7 }
	{ sb r25, r26 ; info 19 ; sne r5, r6, r7 }
	{ sb r25, r26 ; mnz r15, r16, r17 ; clz r5, r6 }
	{ sb r25, r26 ; mnz r15, r16, r17 ; nor r5, r6, r7 }
	{ sb r25, r26 ; mnz r15, r16, r17 ; slti_u r5, r6, 5 }
	{ sb r25, r26 ; mnz r5, r6, r7 ; movei r15, 5 }
	{ sb r25, r26 ; mnz r5, r6, r7 ; slte_u r15, r16, r17 }
	{ sb r25, r26 ; move r15, r16 ; move r5, r6 }
	{ sb r25, r26 ; move r15, r16 ; rli r5, r6, 5 }
	{ sb r25, r26 ; move r15, r16 ; tblidxb0 r5, r6 }
	{ sb r25, r26 ; move r5, r6 ; ori r15, r16, 5 }
	{ sb r25, r26 ; move r5, r6 ; srai r15, r16, 5 }
	{ sb r25, r26 ; movei r15, 5 ; mulhha_uu r5, r6, r7 }
	{ sb r25, r26 ; movei r15, 5 ; seqi r5, r6, 5 }
	{ sb r25, r26 ; movei r15, 5 }
	{ sb r25, r26 ; movei r5, 5 ; s3a r15, r16, r17 }
	{ sb r25, r26 ; mulhh_ss r5, r6, r7 ; addi r15, r16, 5 }
	{ sb r25, r26 ; mulhh_ss r5, r6, r7 ; seqi r15, r16, 5 }
	{ sb r25, r26 ; mulhh_uu r5, r6, r7 ; andi r15, r16, 5 }
	{ sb r25, r26 ; mulhh_uu r5, r6, r7 ; shli r15, r16, 5 }
	{ sb r25, r26 ; mulhha_ss r5, r6, r7 ; ill }
	{ sb r25, r26 ; mulhha_ss r5, r6, r7 ; shri r15, r16, 5 }
	{ sb r25, r26 ; mulhha_uu r5, r6, r7 ; mnz r15, r16, r17 }
	{ sb r25, r26 ; mulhha_uu r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sb r25, r26 ; mulhlsa_uu r5, r6, r7 ; movei r15, 5 }
	{ sb r25, r26 ; mulhlsa_uu r5, r6, r7 ; slte_u r15, r16, r17 }
	{ sb r25, r26 ; mulll_ss r5, r6, r7 ; nop }
	{ sb r25, r26 ; mulll_ss r5, r6, r7 ; slti_u r15, r16, 5 }
	{ sb r25, r26 ; mulll_uu r5, r6, r7 ; or r15, r16, r17 }
	{ sb r25, r26 ; mulll_uu r5, r6, r7 ; sra r15, r16, r17 }
	{ sb r25, r26 ; mullla_ss r5, r6, r7 ; rl r15, r16, r17 }
	{ sb r25, r26 ; mullla_ss r5, r6, r7 ; sub r15, r16, r17 }
	{ sb r25, r26 ; mullla_uu r5, r6, r7 ; s1a r15, r16, r17 }
	{ sb r25, r26 ; mullla_uu r5, r6, r7 }
	{ sb r25, r26 ; mvnz r5, r6, r7 ; s3a r15, r16, r17 }
	{ sb r25, r26 ; mvz r5, r6, r7 ; addi r15, r16, 5 }
	{ sb r25, r26 ; mvz r5, r6, r7 ; seqi r15, r16, 5 }
	{ sb r25, r26 ; mz r15, r16, r17 ; andi r5, r6, 5 }
	{ sb r25, r26 ; mz r15, r16, r17 ; mvz r5, r6, r7 }
	{ sb r25, r26 ; mz r15, r16, r17 ; slte r5, r6, r7 }
	{ sb r25, r26 ; mz r5, r6, r7 ; info 19 }
	{ sb r25, r26 ; mz r5, r6, r7 ; slt r15, r16, r17 }
	{ sb r25, r26 ; nop ; bitx r5, r6 }
	{ sb r25, r26 ; nop ; mullla_ss r5, r6, r7 }
	{ sb r25, r26 ; nop ; s2a r15, r16, r17 }
	{ sb r25, r26 ; nop ; slte r15, r16, r17 }
	{ sb r25, r26 ; nop ; xor r15, r16, r17 }
	{ sb r25, r26 ; nor r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ sb r25, r26 ; nor r15, r16, r17 ; shl r5, r6, r7 }
	{ sb r25, r26 ; nor r5, r6, r7 ; add r15, r16, r17 }
	{ sb r25, r26 ; nor r5, r6, r7 ; seq r15, r16, r17 }
	{ sb r25, r26 ; or r15, r16, r17 ; and r5, r6, r7 }
	{ sb r25, r26 ; or r15, r16, r17 ; mvnz r5, r6, r7 }
	{ sb r25, r26 ; or r15, r16, r17 ; slt_u r5, r6, r7 }
	{ sb r25, r26 ; or r5, r6, r7 ; ill }
	{ sb r25, r26 ; or r5, r6, r7 ; shri r15, r16, 5 }
	{ sb r25, r26 ; ori r15, r16, 5 ; ctz r5, r6 }
	{ sb r25, r26 ; ori r15, r16, 5 ; or r5, r6, r7 }
	{ sb r25, r26 ; ori r15, r16, 5 ; sne r5, r6, r7 }
	{ sb r25, r26 ; ori r5, r6, 5 ; mz r15, r16, r17 }
	{ sb r25, r26 ; ori r5, r6, 5 ; slti r15, r16, 5 }
	{ sb r25, r26 ; pcnt r5, r6 ; nor r15, r16, r17 }
	{ sb r25, r26 ; pcnt r5, r6 ; sne r15, r16, r17 }
	{ sb r25, r26 ; rl r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ sb r25, r26 ; rl r15, r16, r17 ; s3a r5, r6, r7 }
	{ sb r25, r26 ; rl r15, r16, r17 ; tblidxb3 r5, r6 }
	{ sb r25, r26 ; rl r5, r6, r7 ; s1a r15, r16, r17 }
	{ sb r25, r26 ; rl r5, r6, r7 }
	{ sb r25, r26 ; rli r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ sb r25, r26 ; rli r15, r16, 5 ; shr r5, r6, r7 }
	{ sb r25, r26 ; rli r5, r6, 5 ; and r15, r16, r17 }
	{ sb r25, r26 ; rli r5, r6, 5 ; shl r15, r16, r17 }
	{ sb r25, r26 ; s1a r15, r16, r17 ; bitx r5, r6 }
	{ sb r25, r26 ; s1a r15, r16, r17 ; mz r5, r6, r7 }
	{ sb r25, r26 ; s1a r15, r16, r17 ; slte_u r5, r6, r7 }
	{ sb r25, r26 ; s1a r5, r6, r7 ; mnz r15, r16, r17 }
	{ sb r25, r26 ; s1a r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sb r25, r26 ; s2a r15, r16, r17 ; info 19 }
	{ sb r25, r26 ; s2a r15, r16, r17 ; pcnt r5, r6 }
	{ sb r25, r26 ; s2a r15, r16, r17 ; srai r5, r6, 5 }
	{ sb r25, r26 ; s2a r5, r6, r7 ; nor r15, r16, r17 }
	{ sb r25, r26 ; s2a r5, r6, r7 ; sne r15, r16, r17 }
	{ sb r25, r26 ; s3a r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ sb r25, r26 ; s3a r15, r16, r17 ; s3a r5, r6, r7 }
	{ sb r25, r26 ; s3a r15, r16, r17 ; tblidxb3 r5, r6 }
	{ sb r25, r26 ; s3a r5, r6, r7 ; s1a r15, r16, r17 }
	{ sb r25, r26 ; s3a r5, r6, r7 }
	{ sb r25, r26 ; seq r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ sb r25, r26 ; seq r15, r16, r17 ; shr r5, r6, r7 }
	{ sb r25, r26 ; seq r5, r6, r7 ; and r15, r16, r17 }
	{ sb r25, r26 ; seq r5, r6, r7 ; shl r15, r16, r17 }
	{ sb r25, r26 ; seqi r15, r16, 5 ; bitx r5, r6 }
	{ sb r25, r26 ; seqi r15, r16, 5 ; mz r5, r6, r7 }
	{ sb r25, r26 ; seqi r15, r16, 5 ; slte_u r5, r6, r7 }
	{ sb r25, r26 ; seqi r5, r6, 5 ; mnz r15, r16, r17 }
	{ sb r25, r26 ; seqi r5, r6, 5 ; slt_u r15, r16, r17 }
	{ sb r25, r26 ; shl r15, r16, r17 ; info 19 }
	{ sb r25, r26 ; shl r15, r16, r17 ; pcnt r5, r6 }
	{ sb r25, r26 ; shl r15, r16, r17 ; srai r5, r6, 5 }
	{ sb r25, r26 ; shl r5, r6, r7 ; nor r15, r16, r17 }
	{ sb r25, r26 ; shl r5, r6, r7 ; sne r15, r16, r17 }
	{ sb r25, r26 ; shli r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ sb r25, r26 ; shli r15, r16, 5 ; s3a r5, r6, r7 }
	{ sb r25, r26 ; shli r15, r16, 5 ; tblidxb3 r5, r6 }
	{ sb r25, r26 ; shli r5, r6, 5 ; s1a r15, r16, r17 }
	{ sb r25, r26 ; shli r5, r6, 5 }
	{ sb r25, r26 ; shr r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ sb r25, r26 ; shr r15, r16, r17 ; shr r5, r6, r7 }
	{ sb r25, r26 ; shr r5, r6, r7 ; and r15, r16, r17 }
	{ sb r25, r26 ; shr r5, r6, r7 ; shl r15, r16, r17 }
	{ sb r25, r26 ; shri r15, r16, 5 ; bitx r5, r6 }
	{ sb r25, r26 ; shri r15, r16, 5 ; mz r5, r6, r7 }
	{ sb r25, r26 ; shri r15, r16, 5 ; slte_u r5, r6, r7 }
	{ sb r25, r26 ; shri r5, r6, 5 ; mnz r15, r16, r17 }
	{ sb r25, r26 ; shri r5, r6, 5 ; slt_u r15, r16, r17 }
	{ sb r25, r26 ; slt r15, r16, r17 ; info 19 }
	{ sb r25, r26 ; slt r15, r16, r17 ; pcnt r5, r6 }
	{ sb r25, r26 ; slt r15, r16, r17 ; srai r5, r6, 5 }
	{ sb r25, r26 ; slt r5, r6, r7 ; nor r15, r16, r17 }
	{ sb r25, r26 ; slt r5, r6, r7 ; sne r15, r16, r17 }
	{ sb r25, r26 ; slt_u r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ sb r25, r26 ; slt_u r15, r16, r17 ; s3a r5, r6, r7 }
	{ sb r25, r26 ; slt_u r15, r16, r17 ; tblidxb3 r5, r6 }
	{ sb r25, r26 ; slt_u r5, r6, r7 ; s1a r15, r16, r17 }
	{ sb r25, r26 ; slt_u r5, r6, r7 }
	{ sb r25, r26 ; slte r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ sb r25, r26 ; slte r15, r16, r17 ; shr r5, r6, r7 }
	{ sb r25, r26 ; slte r5, r6, r7 ; and r15, r16, r17 }
	{ sb r25, r26 ; slte r5, r6, r7 ; shl r15, r16, r17 }
	{ sb r25, r26 ; slte_u r15, r16, r17 ; bitx r5, r6 }
	{ sb r25, r26 ; slte_u r15, r16, r17 ; mz r5, r6, r7 }
	{ sb r25, r26 ; slte_u r15, r16, r17 ; slte_u r5, r6, r7 }
	{ sb r25, r26 ; slte_u r5, r6, r7 ; mnz r15, r16, r17 }
	{ sb r25, r26 ; slte_u r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sb r25, r26 ; slti r15, r16, 5 ; info 19 }
	{ sb r25, r26 ; slti r15, r16, 5 ; pcnt r5, r6 }
	{ sb r25, r26 ; slti r15, r16, 5 ; srai r5, r6, 5 }
	{ sb r25, r26 ; slti r5, r6, 5 ; nor r15, r16, r17 }
	{ sb r25, r26 ; slti r5, r6, 5 ; sne r15, r16, r17 }
	{ sb r25, r26 ; slti_u r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ sb r25, r26 ; slti_u r15, r16, 5 ; s3a r5, r6, r7 }
	{ sb r25, r26 ; slti_u r15, r16, 5 ; tblidxb3 r5, r6 }
	{ sb r25, r26 ; slti_u r5, r6, 5 ; s1a r15, r16, r17 }
	{ sb r25, r26 ; slti_u r5, r6, 5 }
	{ sb r25, r26 ; sne r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ sb r25, r26 ; sne r15, r16, r17 ; shr r5, r6, r7 }
	{ sb r25, r26 ; sne r5, r6, r7 ; and r15, r16, r17 }
	{ sb r25, r26 ; sne r5, r6, r7 ; shl r15, r16, r17 }
	{ sb r25, r26 ; sra r15, r16, r17 ; bitx r5, r6 }
	{ sb r25, r26 ; sra r15, r16, r17 ; mz r5, r6, r7 }
	{ sb r25, r26 ; sra r15, r16, r17 ; slte_u r5, r6, r7 }
	{ sb r25, r26 ; sra r5, r6, r7 ; mnz r15, r16, r17 }
	{ sb r25, r26 ; sra r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sb r25, r26 ; srai r15, r16, 5 ; info 19 }
	{ sb r25, r26 ; srai r15, r16, 5 ; pcnt r5, r6 }
	{ sb r25, r26 ; srai r15, r16, 5 ; srai r5, r6, 5 }
	{ sb r25, r26 ; srai r5, r6, 5 ; nor r15, r16, r17 }
	{ sb r25, r26 ; srai r5, r6, 5 ; sne r15, r16, r17 }
	{ sb r25, r26 ; sub r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ sb r25, r26 ; sub r15, r16, r17 ; s3a r5, r6, r7 }
	{ sb r25, r26 ; sub r15, r16, r17 ; tblidxb3 r5, r6 }
	{ sb r25, r26 ; sub r5, r6, r7 ; s1a r15, r16, r17 }
	{ sb r25, r26 ; sub r5, r6, r7 }
	{ sb r25, r26 ; tblidxb0 r5, r6 ; s3a r15, r16, r17 }
	{ sb r25, r26 ; tblidxb1 r5, r6 ; addi r15, r16, 5 }
	{ sb r25, r26 ; tblidxb1 r5, r6 ; seqi r15, r16, 5 }
	{ sb r25, r26 ; tblidxb2 r5, r6 ; andi r15, r16, 5 }
	{ sb r25, r26 ; tblidxb2 r5, r6 ; shli r15, r16, 5 }
	{ sb r25, r26 ; tblidxb3 r5, r6 ; ill }
	{ sb r25, r26 ; tblidxb3 r5, r6 ; shri r15, r16, 5 }
	{ sb r25, r26 ; xor r15, r16, r17 ; ctz r5, r6 }
	{ sb r25, r26 ; xor r15, r16, r17 ; or r5, r6, r7 }
	{ sb r25, r26 ; xor r15, r16, r17 ; sne r5, r6, r7 }
	{ sb r25, r26 ; xor r5, r6, r7 ; mz r15, r16, r17 }
	{ sb r25, r26 ; xor r5, r6, r7 ; slti r15, r16, 5 }
	{ sbadd r15, r16, 5 ; adiffh r5, r6, r7 }
	{ sbadd r15, r16, 5 ; maxb_u r5, r6, r7 }
	{ sbadd r15, r16, 5 ; mulhha_su r5, r6, r7 }
	{ sbadd r15, r16, 5 ; mvz r5, r6, r7 }
	{ sbadd r15, r16, 5 ; sadah_u r5, r6, r7 }
	{ sbadd r15, r16, 5 ; shrib r5, r6, 5 }
	{ sbadd r15, r16, 5 ; sne r5, r6, r7 }
	{ sbadd r15, r16, 5 ; xori r5, r6, 5 }
	{ seq r15, r16, r17 ; addi r5, r6, 5 ; prefetch r25 }
	{ seq r15, r16, r17 ; and r5, r6, r7 ; sw r25, r26 }
	{ seq r15, r16, r17 ; bitx r5, r6 ; prefetch r25 }
	{ seq r15, r16, r17 ; clz r5, r6 ; prefetch r25 }
	{ seq r15, r16, r17 ; fnop ; lh r25, r26 }
	{ seq r15, r16, r17 ; inthh r5, r6, r7 }
	{ seq r15, r16, r17 ; lb r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ seq r15, r16, r17 ; lb r25, r26 ; shl r5, r6, r7 }
	{ seq r15, r16, r17 ; lb_u r25, r26 ; add r5, r6, r7 }
	{ seq r15, r16, r17 ; lb_u r25, r26 ; mullla_ss r5, r6, r7 }
	{ seq r15, r16, r17 ; lb_u r25, r26 ; shri r5, r6, 5 }
	{ seq r15, r16, r17 ; lh r25, r26 ; andi r5, r6, 5 }
	{ seq r15, r16, r17 ; lh r25, r26 ; mvz r5, r6, r7 }
	{ seq r15, r16, r17 ; lh r25, r26 ; slte r5, r6, r7 }
	{ seq r15, r16, r17 ; lh_u r25, r26 ; clz r5, r6 }
	{ seq r15, r16, r17 ; lh_u r25, r26 ; nor r5, r6, r7 }
	{ seq r15, r16, r17 ; lh_u r25, r26 ; slti_u r5, r6, 5 }
	{ seq r15, r16, r17 ; lw r25, r26 ; info 19 }
	{ seq r15, r16, r17 ; lw r25, r26 ; pcnt r5, r6 }
	{ seq r15, r16, r17 ; lw r25, r26 ; srai r5, r6, 5 }
	{ seq r15, r16, r17 ; mnz r5, r6, r7 ; lh_u r25, r26 }
	{ seq r15, r16, r17 ; movei r5, 5 ; lb_u r25, r26 }
	{ seq r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ seq r15, r16, r17 ; mulhha_ss r5, r6, r7 ; sw r25, r26 }
	{ seq r15, r16, r17 ; mulhla_us r5, r6, r7 }
	{ seq r15, r16, r17 ; mulll_ss r5, r6, r7 ; sw r25, r26 }
	{ seq r15, r16, r17 ; mullla_ss r5, r6, r7 ; sh r25, r26 }
	{ seq r15, r16, r17 ; mvnz r5, r6, r7 ; prefetch r25 }
	{ seq r15, r16, r17 ; mz r5, r6, r7 ; prefetch r25 }
	{ seq r15, r16, r17 ; nor r5, r6, r7 ; lh_u r25, r26 }
	{ seq r15, r16, r17 ; ori r5, r6, 5 ; lh_u r25, r26 }
	{ seq r15, r16, r17 ; pcnt r5, r6 }
	{ seq r15, r16, r17 ; prefetch r25 ; mulll_uu r5, r6, r7 }
	{ seq r15, r16, r17 ; prefetch r25 ; shr r5, r6, r7 }
	{ seq r15, r16, r17 ; rl r5, r6, r7 ; lh r25, r26 }
	{ seq r15, r16, r17 ; s1a r5, r6, r7 ; lh r25, r26 }
	{ seq r15, r16, r17 ; s3a r5, r6, r7 ; lh r25, r26 }
	{ seq r15, r16, r17 ; sb r25, r26 ; clz r5, r6 }
	{ seq r15, r16, r17 ; sb r25, r26 ; nor r5, r6, r7 }
	{ seq r15, r16, r17 ; sb r25, r26 ; slti_u r5, r6, 5 }
	{ seq r15, r16, r17 ; seq r5, r6, r7 }
	{ seq r15, r16, r17 ; sh r25, r26 ; bytex r5, r6 }
	{ seq r15, r16, r17 ; sh r25, r26 ; nop }
	{ seq r15, r16, r17 ; sh r25, r26 ; slti r5, r6, 5 }
	{ seq r15, r16, r17 ; shl r5, r6, r7 ; sw r25, r26 }
	{ seq r15, r16, r17 ; shr r5, r6, r7 ; lw r25, r26 }
	{ seq r15, r16, r17 ; slt r5, r6, r7 ; lb r25, r26 }
	{ seq r15, r16, r17 ; sltb r5, r6, r7 }
	{ seq r15, r16, r17 ; slte_u r5, r6, r7 ; sw r25, r26 }
	{ seq r15, r16, r17 ; slti_u r5, r6, 5 ; lh r25, r26 }
	{ seq r15, r16, r17 ; sne r5, r6, r7 ; sw r25, r26 }
	{ seq r15, r16, r17 ; srai r5, r6, 5 ; lw r25, r26 }
	{ seq r15, r16, r17 ; subh r5, r6, r7 }
	{ seq r15, r16, r17 ; sw r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ seq r15, r16, r17 ; sw r25, r26 ; shl r5, r6, r7 }
	{ seq r15, r16, r17 ; tblidxb0 r5, r6 ; lb r25, r26 }
	{ seq r15, r16, r17 ; tblidxb2 r5, r6 ; lb r25, r26 }
	{ seq r15, r16, r17 ; xor r5, r6, r7 ; lb r25, r26 }
	{ seq r5, r6, r7 ; add r15, r16, r17 }
	{ seq r5, r6, r7 ; and r15, r16, r17 ; lb r25, r26 }
	{ seq r5, r6, r7 ; auli r15, r16, 0x1234 }
	{ seq r5, r6, r7 ; ill ; prefetch r25 }
	{ seq r5, r6, r7 ; inv r15 }
	{ seq r5, r6, r7 ; lb r25, r26 ; or r15, r16, r17 }
	{ seq r5, r6, r7 ; lb r25, r26 ; sra r15, r16, r17 }
	{ seq r5, r6, r7 ; lb_u r25, r26 ; ori r15, r16, 5 }
	{ seq r5, r6, r7 ; lb_u r25, r26 ; srai r15, r16, 5 }
	{ seq r5, r6, r7 ; lh r25, r26 ; or r15, r16, r17 }
	{ seq r5, r6, r7 ; lh r25, r26 ; sra r15, r16, r17 }
	{ seq r5, r6, r7 ; lh_u r25, r26 ; ori r15, r16, 5 }
	{ seq r5, r6, r7 ; lh_u r25, r26 ; srai r15, r16, 5 }
	{ seq r5, r6, r7 ; lw r25, r26 ; nor r15, r16, r17 }
	{ seq r5, r6, r7 ; lw r25, r26 ; sne r15, r16, r17 }
	{ seq r5, r6, r7 ; mnz r15, r16, r17 ; lb r25, r26 }
	{ seq r5, r6, r7 ; move r15, r16 ; sw r25, r26 }
	{ seq r5, r6, r7 ; mz r15, r16, r17 ; prefetch r25 }
	{ seq r5, r6, r7 ; nor r15, r16, r17 ; lh_u r25, r26 }
	{ seq r5, r6, r7 ; ori r15, r16, 5 ; lh_u r25, r26 }
	{ seq r5, r6, r7 ; prefetch r25 ; move r15, r16 }
	{ seq r5, r6, r7 ; prefetch r25 ; slte r15, r16, r17 }
	{ seq r5, r6, r7 ; rl r15, r16, r17 }
	{ seq r5, r6, r7 ; s1a r15, r16, r17 }
	{ seq r5, r6, r7 ; s3a r15, r16, r17 }
	{ seq r5, r6, r7 ; sb r25, r26 ; s2a r15, r16, r17 }
	{ seq r5, r6, r7 ; sbadd r15, r16, 5 }
	{ seq r5, r6, r7 ; seqi r15, r16, 5 ; sh r25, r26 }
	{ seq r5, r6, r7 ; sh r25, r26 ; ori r15, r16, 5 }
	{ seq r5, r6, r7 ; sh r25, r26 ; srai r15, r16, 5 }
	{ seq r5, r6, r7 ; shli r15, r16, 5 ; lh_u r25, r26 }
	{ seq r5, r6, r7 ; shrh r15, r16, r17 }
	{ seq r5, r6, r7 ; slt r15, r16, r17 ; sh r25, r26 }
	{ seq r5, r6, r7 ; slte r15, r16, r17 ; prefetch r25 }
	{ seq r5, r6, r7 ; slth_u r15, r16, r17 }
	{ seq r5, r6, r7 ; slti_u r15, r16, 5 }
	{ seq r5, r6, r7 ; sra r15, r16, r17 ; lh_u r25, r26 }
	{ seq r5, r6, r7 ; sraih r15, r16, 5 }
	{ seq r5, r6, r7 ; sw r25, r26 ; andi r15, r16, 5 }
	{ seq r5, r6, r7 ; sw r25, r26 ; shli r15, r16, 5 }
	{ seq r5, r6, r7 ; xor r15, r16, r17 ; lh r25, r26 }
	{ seqb r15, r16, r17 ; adiffb_u r5, r6, r7 }
	{ seqb r15, r16, r17 ; intlh r5, r6, r7 }
	{ seqb r15, r16, r17 ; mulhha_ss r5, r6, r7 }
	{ seqb r15, r16, r17 ; mvnz r5, r6, r7 }
	{ seqb r15, r16, r17 ; sadah r5, r6, r7 }
	{ seqb r15, r16, r17 ; shri r5, r6, 5 }
	{ seqb r15, r16, r17 ; sltih_u r5, r6, 5 }
	{ seqb r15, r16, r17 ; xor r5, r6, r7 }
	{ seqb r5, r6, r7 ; icoh r15 }
	{ seqb r5, r6, r7 ; lhadd r15, r16, 5 }
	{ seqb r5, r6, r7 ; mnzh r15, r16, r17 }
	{ seqb r5, r6, r7 ; rli r15, r16, 5 }
	{ seqb r5, r6, r7 ; shr r15, r16, r17 }
	{ seqb r5, r6, r7 ; sltib r15, r16, 5 }
	{ seqb r5, r6, r7 ; swadd r15, r16, 5 }
	{ seqh r15, r16, r17 ; auli r5, r6, 0x1234 }
	{ seqh r15, r16, r17 ; maxih r5, r6, 5 }
	{ seqh r15, r16, r17 ; mulhl_ss r5, r6, r7 }
	{ seqh r15, r16, r17 ; mzh r5, r6, r7 }
	{ seqh r15, r16, r17 ; sadh_u r5, r6, r7 }
	{ seqh r15, r16, r17 ; slt_u r5, r6, r7 }
	{ seqh r15, r16, r17 ; sra r5, r6, r7 }
	{ seqh r5, r6, r7 ; addbs_u r15, r16, r17 }
	{ seqh r5, r6, r7 ; inthb r15, r16, r17 }
	{ seqh r5, r6, r7 ; lw_na r15, r16 }
	{ seqh r5, r6, r7 ; movelis r15, 0x1234 }
	{ seqh r5, r6, r7 ; sb r15, r16 }
	{ seqh r5, r6, r7 ; shrib r15, r16, 5 }
	{ seqh r5, r6, r7 ; sne r15, r16, r17 }
	{ seqh r5, r6, r7 ; xori r15, r16, 5 }
	{ seqi r15, r16, 5 ; addi r5, r6, 5 ; prefetch r25 }
	{ seqi r15, r16, 5 ; and r5, r6, r7 ; sw r25, r26 }
	{ seqi r15, r16, 5 ; bitx r5, r6 ; prefetch r25 }
	{ seqi r15, r16, 5 ; clz r5, r6 ; prefetch r25 }
	{ seqi r15, r16, 5 ; fnop ; lh r25, r26 }
	{ seqi r15, r16, 5 ; inthh r5, r6, r7 }
	{ seqi r15, r16, 5 ; lb r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ seqi r15, r16, 5 ; lb r25, r26 ; shl r5, r6, r7 }
	{ seqi r15, r16, 5 ; lb_u r25, r26 ; add r5, r6, r7 }
	{ seqi r15, r16, 5 ; lb_u r25, r26 ; mullla_ss r5, r6, r7 }
	{ seqi r15, r16, 5 ; lb_u r25, r26 ; shri r5, r6, 5 }
	{ seqi r15, r16, 5 ; lh r25, r26 ; andi r5, r6, 5 }
	{ seqi r15, r16, 5 ; lh r25, r26 ; mvz r5, r6, r7 }
	{ seqi r15, r16, 5 ; lh r25, r26 ; slte r5, r6, r7 }
	{ seqi r15, r16, 5 ; lh_u r25, r26 ; clz r5, r6 }
	{ seqi r15, r16, 5 ; lh_u r25, r26 ; nor r5, r6, r7 }
	{ seqi r15, r16, 5 ; lh_u r25, r26 ; slti_u r5, r6, 5 }
	{ seqi r15, r16, 5 ; lw r25, r26 ; info 19 }
	{ seqi r15, r16, 5 ; lw r25, r26 ; pcnt r5, r6 }
	{ seqi r15, r16, 5 ; lw r25, r26 ; srai r5, r6, 5 }
	{ seqi r15, r16, 5 ; mnz r5, r6, r7 ; lh_u r25, r26 }
	{ seqi r15, r16, 5 ; movei r5, 5 ; lb_u r25, r26 }
	{ seqi r15, r16, 5 ; mulhh_ss r5, r6, r7 }
	{ seqi r15, r16, 5 ; mulhha_ss r5, r6, r7 ; sw r25, r26 }
	{ seqi r15, r16, 5 ; mulhla_us r5, r6, r7 }
	{ seqi r15, r16, 5 ; mulll_ss r5, r6, r7 ; sw r25, r26 }
	{ seqi r15, r16, 5 ; mullla_ss r5, r6, r7 ; sh r25, r26 }
	{ seqi r15, r16, 5 ; mvnz r5, r6, r7 ; prefetch r25 }
	{ seqi r15, r16, 5 ; mz r5, r6, r7 ; prefetch r25 }
	{ seqi r15, r16, 5 ; nor r5, r6, r7 ; lh_u r25, r26 }
	{ seqi r15, r16, 5 ; ori r5, r6, 5 ; lh_u r25, r26 }
	{ seqi r15, r16, 5 ; pcnt r5, r6 }
	{ seqi r15, r16, 5 ; prefetch r25 ; mulll_uu r5, r6, r7 }
	{ seqi r15, r16, 5 ; prefetch r25 ; shr r5, r6, r7 }
	{ seqi r15, r16, 5 ; rl r5, r6, r7 ; lh r25, r26 }
	{ seqi r15, r16, 5 ; s1a r5, r6, r7 ; lh r25, r26 }
	{ seqi r15, r16, 5 ; s3a r5, r6, r7 ; lh r25, r26 }
	{ seqi r15, r16, 5 ; sb r25, r26 ; clz r5, r6 }
	{ seqi r15, r16, 5 ; sb r25, r26 ; nor r5, r6, r7 }
	{ seqi r15, r16, 5 ; sb r25, r26 ; slti_u r5, r6, 5 }
	{ seqi r15, r16, 5 ; seq r5, r6, r7 }
	{ seqi r15, r16, 5 ; sh r25, r26 ; bytex r5, r6 }
	{ seqi r15, r16, 5 ; sh r25, r26 ; nop }
	{ seqi r15, r16, 5 ; sh r25, r26 ; slti r5, r6, 5 }
	{ seqi r15, r16, 5 ; shl r5, r6, r7 ; sw r25, r26 }
	{ seqi r15, r16, 5 ; shr r5, r6, r7 ; lw r25, r26 }
	{ seqi r15, r16, 5 ; slt r5, r6, r7 ; lb r25, r26 }
	{ seqi r15, r16, 5 ; sltb r5, r6, r7 }
	{ seqi r15, r16, 5 ; slte_u r5, r6, r7 ; sw r25, r26 }
	{ seqi r15, r16, 5 ; slti_u r5, r6, 5 ; lh r25, r26 }
	{ seqi r15, r16, 5 ; sne r5, r6, r7 ; sw r25, r26 }
	{ seqi r15, r16, 5 ; srai r5, r6, 5 ; lw r25, r26 }
	{ seqi r15, r16, 5 ; subh r5, r6, r7 }
	{ seqi r15, r16, 5 ; sw r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ seqi r15, r16, 5 ; sw r25, r26 ; shl r5, r6, r7 }
	{ seqi r15, r16, 5 ; tblidxb0 r5, r6 ; lb r25, r26 }
	{ seqi r15, r16, 5 ; tblidxb2 r5, r6 ; lb r25, r26 }
	{ seqi r15, r16, 5 ; xor r5, r6, r7 ; lb r25, r26 }
	{ seqi r5, r6, 5 ; add r15, r16, r17 }
	{ seqi r5, r6, 5 ; and r15, r16, r17 ; lb r25, r26 }
	{ seqi r5, r6, 5 ; auli r15, r16, 0x1234 }
	{ seqi r5, r6, 5 ; ill ; prefetch r25 }
	{ seqi r5, r6, 5 ; inv r15 }
	{ seqi r5, r6, 5 ; lb r25, r26 ; or r15, r16, r17 }
	{ seqi r5, r6, 5 ; lb r25, r26 ; sra r15, r16, r17 }
	{ seqi r5, r6, 5 ; lb_u r25, r26 ; ori r15, r16, 5 }
	{ seqi r5, r6, 5 ; lb_u r25, r26 ; srai r15, r16, 5 }
	{ seqi r5, r6, 5 ; lh r25, r26 ; or r15, r16, r17 }
	{ seqi r5, r6, 5 ; lh r25, r26 ; sra r15, r16, r17 }
	{ seqi r5, r6, 5 ; lh_u r25, r26 ; ori r15, r16, 5 }
	{ seqi r5, r6, 5 ; lh_u r25, r26 ; srai r15, r16, 5 }
	{ seqi r5, r6, 5 ; lw r25, r26 ; nor r15, r16, r17 }
	{ seqi r5, r6, 5 ; lw r25, r26 ; sne r15, r16, r17 }
	{ seqi r5, r6, 5 ; mnz r15, r16, r17 ; lb r25, r26 }
	{ seqi r5, r6, 5 ; move r15, r16 ; sw r25, r26 }
	{ seqi r5, r6, 5 ; mz r15, r16, r17 ; prefetch r25 }
	{ seqi r5, r6, 5 ; nor r15, r16, r17 ; lh_u r25, r26 }
	{ seqi r5, r6, 5 ; ori r15, r16, 5 ; lh_u r25, r26 }
	{ seqi r5, r6, 5 ; prefetch r25 ; move r15, r16 }
	{ seqi r5, r6, 5 ; prefetch r25 ; slte r15, r16, r17 }
	{ seqi r5, r6, 5 ; rl r15, r16, r17 }
	{ seqi r5, r6, 5 ; s1a r15, r16, r17 }
	{ seqi r5, r6, 5 ; s3a r15, r16, r17 }
	{ seqi r5, r6, 5 ; sb r25, r26 ; s2a r15, r16, r17 }
	{ seqi r5, r6, 5 ; sbadd r15, r16, 5 }
	{ seqi r5, r6, 5 ; seqi r15, r16, 5 ; sh r25, r26 }
	{ seqi r5, r6, 5 ; sh r25, r26 ; ori r15, r16, 5 }
	{ seqi r5, r6, 5 ; sh r25, r26 ; srai r15, r16, 5 }
	{ seqi r5, r6, 5 ; shli r15, r16, 5 ; lh_u r25, r26 }
	{ seqi r5, r6, 5 ; shrh r15, r16, r17 }
	{ seqi r5, r6, 5 ; slt r15, r16, r17 ; sh r25, r26 }
	{ seqi r5, r6, 5 ; slte r15, r16, r17 ; prefetch r25 }
	{ seqi r5, r6, 5 ; slth_u r15, r16, r17 }
	{ seqi r5, r6, 5 ; slti_u r15, r16, 5 }
	{ seqi r5, r6, 5 ; sra r15, r16, r17 ; lh_u r25, r26 }
	{ seqi r5, r6, 5 ; sraih r15, r16, 5 }
	{ seqi r5, r6, 5 ; sw r25, r26 ; andi r15, r16, 5 }
	{ seqi r5, r6, 5 ; sw r25, r26 ; shli r15, r16, 5 }
	{ seqi r5, r6, 5 ; xor r15, r16, r17 ; lh r25, r26 }
	{ seqib r15, r16, 5 ; adiffb_u r5, r6, r7 }
	{ seqib r15, r16, 5 ; intlh r5, r6, r7 }
	{ seqib r15, r16, 5 ; mulhha_ss r5, r6, r7 }
	{ seqib r15, r16, 5 ; mvnz r5, r6, r7 }
	{ seqib r15, r16, 5 ; sadah r5, r6, r7 }
	{ seqib r15, r16, 5 ; shri r5, r6, 5 }
	{ seqib r15, r16, 5 ; sltih_u r5, r6, 5 }
	{ seqib r15, r16, 5 ; xor r5, r6, r7 }
	{ seqib r5, r6, 5 ; icoh r15 }
	{ seqib r5, r6, 5 ; lhadd r15, r16, 5 }
	{ seqib r5, r6, 5 ; mnzh r15, r16, r17 }
	{ seqib r5, r6, 5 ; rli r15, r16, 5 }
	{ seqib r5, r6, 5 ; shr r15, r16, r17 }
	{ seqib r5, r6, 5 ; sltib r15, r16, 5 }
	{ seqib r5, r6, 5 ; swadd r15, r16, 5 }
	{ seqih r15, r16, 5 ; auli r5, r6, 0x1234 }
	{ seqih r15, r16, 5 ; maxih r5, r6, 5 }
	{ seqih r15, r16, 5 ; mulhl_ss r5, r6, r7 }
	{ seqih r15, r16, 5 ; mzh r5, r6, r7 }
	{ seqih r15, r16, 5 ; sadh_u r5, r6, r7 }
	{ seqih r15, r16, 5 ; slt_u r5, r6, r7 }
	{ seqih r15, r16, 5 ; sra r5, r6, r7 }
	{ seqih r5, r6, 5 ; addbs_u r15, r16, r17 }
	{ seqih r5, r6, 5 ; inthb r15, r16, r17 }
	{ seqih r5, r6, 5 ; lw_na r15, r16 }
	{ seqih r5, r6, 5 ; movelis r15, 0x1234 }
	{ seqih r5, r6, 5 ; sb r15, r16 }
	{ seqih r5, r6, 5 ; shrib r15, r16, 5 }
	{ seqih r5, r6, 5 ; sne r15, r16, r17 }
	{ seqih r5, r6, 5 ; xori r15, r16, 5 }
	{ sh r15, r16 ; bytex r5, r6 }
	{ sh r15, r16 ; minih r5, r6, 5 }
	{ sh r15, r16 ; mulhla_ss r5, r6, r7 }
	{ sh r15, r16 ; ori r5, r6, 5 }
	{ sh r15, r16 ; seqi r5, r6, 5 }
	{ sh r15, r16 ; slte_u r5, r6, r7 }
	{ sh r15, r16 ; sraib r5, r6, 5 }
	{ sh r25, r26 ; add r15, r16, r17 ; clz r5, r6 }
	{ sh r25, r26 ; add r15, r16, r17 ; nor r5, r6, r7 }
	{ sh r25, r26 ; add r15, r16, r17 ; slti_u r5, r6, 5 }
	{ sh r25, r26 ; add r5, r6, r7 ; movei r15, 5 }
	{ sh r25, r26 ; add r5, r6, r7 ; slte_u r15, r16, r17 }
	{ sh r25, r26 ; addi r15, r16, 5 ; move r5, r6 }
	{ sh r25, r26 ; addi r15, r16, 5 ; rli r5, r6, 5 }
	{ sh r25, r26 ; addi r15, r16, 5 ; tblidxb0 r5, r6 }
	{ sh r25, r26 ; addi r5, r6, 5 ; ori r15, r16, 5 }
	{ sh r25, r26 ; addi r5, r6, 5 ; srai r15, r16, 5 }
	{ sh r25, r26 ; and r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ sh r25, r26 ; and r15, r16, r17 ; seqi r5, r6, 5 }
	{ sh r25, r26 ; and r15, r16, r17 }
	{ sh r25, r26 ; and r5, r6, r7 ; s3a r15, r16, r17 }
	{ sh r25, r26 ; andi r15, r16, 5 ; addi r5, r6, 5 }
	{ sh r25, r26 ; andi r15, r16, 5 ; mullla_uu r5, r6, r7 }
	{ sh r25, r26 ; andi r15, r16, 5 ; slt r5, r6, r7 }
	{ sh r25, r26 ; andi r5, r6, 5 ; fnop }
	{ sh r25, r26 ; andi r5, r6, 5 ; shr r15, r16, r17 }
	{ sh r25, r26 ; bitx r5, r6 ; info 19 }
	{ sh r25, r26 ; bitx r5, r6 ; slt r15, r16, r17 }
	{ sh r25, r26 ; bytex r5, r6 ; move r15, r16 }
	{ sh r25, r26 ; bytex r5, r6 ; slte r15, r16, r17 }
	{ sh r25, r26 ; clz r5, r6 ; mz r15, r16, r17 }
	{ sh r25, r26 ; clz r5, r6 ; slti r15, r16, 5 }
	{ sh r25, r26 ; ctz r5, r6 ; nor r15, r16, r17 }
	{ sh r25, r26 ; ctz r5, r6 ; sne r15, r16, r17 }
	{ sh r25, r26 ; fnop ; info 19 }
	{ sh r25, r26 ; fnop ; nop }
	{ sh r25, r26 ; fnop ; seqi r15, r16, 5 }
	{ sh r25, r26 ; fnop ; slti_u r15, r16, 5 }
	{ sh r25, r26 ; ill ; andi r5, r6, 5 }
	{ sh r25, r26 ; ill ; mvz r5, r6, r7 }
	{ sh r25, r26 ; ill ; slte r5, r6, r7 }
	{ sh r25, r26 ; info 19 ; andi r15, r16, 5 }
	{ sh r25, r26 ; info 19 ; mulll_ss r5, r6, r7 }
	{ sh r25, r26 ; info 19 ; s1a r15, r16, r17 }
	{ sh r25, r26 ; info 19 ; slt_u r15, r16, r17 }
	{ sh r25, r26 ; info 19 ; tblidxb2 r5, r6 }
	{ sh r25, r26 ; mnz r15, r16, r17 ; mulhha_ss r5, r6, r7 }
	{ sh r25, r26 ; mnz r15, r16, r17 ; seq r5, r6, r7 }
	{ sh r25, r26 ; mnz r15, r16, r17 ; xor r5, r6, r7 }
	{ sh r25, r26 ; mnz r5, r6, r7 ; s2a r15, r16, r17 }
	{ sh r25, r26 ; move r15, r16 ; add r5, r6, r7 }
	{ sh r25, r26 ; move r15, r16 ; mullla_ss r5, r6, r7 }
	{ sh r25, r26 ; move r15, r16 ; shri r5, r6, 5 }
	{ sh r25, r26 ; move r5, r6 ; andi r15, r16, 5 }
	{ sh r25, r26 ; move r5, r6 ; shli r15, r16, 5 }
	{ sh r25, r26 ; movei r15, 5 ; bytex r5, r6 }
	{ sh r25, r26 ; movei r15, 5 ; nop }
	{ sh r25, r26 ; movei r15, 5 ; slti r5, r6, 5 }
	{ sh r25, r26 ; movei r5, 5 ; move r15, r16 }
	{ sh r25, r26 ; movei r5, 5 ; slte r15, r16, r17 }
	{ sh r25, r26 ; mulhh_ss r5, r6, r7 ; mz r15, r16, r17 }
	{ sh r25, r26 ; mulhh_ss r5, r6, r7 ; slti r15, r16, 5 }
	{ sh r25, r26 ; mulhh_uu r5, r6, r7 ; nor r15, r16, r17 }
	{ sh r25, r26 ; mulhh_uu r5, r6, r7 ; sne r15, r16, r17 }
	{ sh r25, r26 ; mulhha_ss r5, r6, r7 ; ori r15, r16, 5 }
	{ sh r25, r26 ; mulhha_ss r5, r6, r7 ; srai r15, r16, 5 }
	{ sh r25, r26 ; mulhha_uu r5, r6, r7 ; rli r15, r16, 5 }
	{ sh r25, r26 ; mulhha_uu r5, r6, r7 ; xor r15, r16, r17 }
	{ sh r25, r26 ; mulhlsa_uu r5, r6, r7 ; s2a r15, r16, r17 }
	{ sh r25, r26 ; mulll_ss r5, r6, r7 ; add r15, r16, r17 }
	{ sh r25, r26 ; mulll_ss r5, r6, r7 ; seq r15, r16, r17 }
	{ sh r25, r26 ; mulll_uu r5, r6, r7 ; and r15, r16, r17 }
	{ sh r25, r26 ; mulll_uu r5, r6, r7 ; shl r15, r16, r17 }
	{ sh r25, r26 ; mullla_ss r5, r6, r7 ; fnop }
	{ sh r25, r26 ; mullla_ss r5, r6, r7 ; shr r15, r16, r17 }
	{ sh r25, r26 ; mullla_uu r5, r6, r7 ; info 19 }
	{ sh r25, r26 ; mullla_uu r5, r6, r7 ; slt r15, r16, r17 }
	{ sh r25, r26 ; mvnz r5, r6, r7 ; move r15, r16 }
	{ sh r25, r26 ; mvnz r5, r6, r7 ; slte r15, r16, r17 }
	{ sh r25, r26 ; mvz r5, r6, r7 ; mz r15, r16, r17 }
	{ sh r25, r26 ; mvz r5, r6, r7 ; slti r15, r16, 5 }
	{ sh r25, r26 ; mz r15, r16, r17 ; movei r5, 5 }
	{ sh r25, r26 ; mz r15, r16, r17 ; s1a r5, r6, r7 }
	{ sh r25, r26 ; mz r15, r16, r17 ; tblidxb1 r5, r6 }
	{ sh r25, r26 ; mz r5, r6, r7 ; rl r15, r16, r17 }
	{ sh r25, r26 ; mz r5, r6, r7 ; sub r15, r16, r17 }
	{ sh r25, r26 ; nop ; move r15, r16 }
	{ sh r25, r26 ; nop ; or r15, r16, r17 }
	{ sh r25, r26 ; nop ; shl r5, r6, r7 }
	{ sh r25, r26 ; nop ; sne r5, r6, r7 }
	{ sh r25, r26 ; nor r15, r16, r17 ; clz r5, r6 }
	{ sh r25, r26 ; nor r15, r16, r17 ; nor r5, r6, r7 }
	{ sh r25, r26 ; nor r15, r16, r17 ; slti_u r5, r6, 5 }
	{ sh r25, r26 ; nor r5, r6, r7 ; movei r15, 5 }
	{ sh r25, r26 ; nor r5, r6, r7 ; slte_u r15, r16, r17 }
	{ sh r25, r26 ; or r15, r16, r17 ; move r5, r6 }
	{ sh r25, r26 ; or r15, r16, r17 ; rli r5, r6, 5 }
	{ sh r25, r26 ; or r15, r16, r17 ; tblidxb0 r5, r6 }
	{ sh r25, r26 ; or r5, r6, r7 ; ori r15, r16, 5 }
	{ sh r25, r26 ; or r5, r6, r7 ; srai r15, r16, 5 }
	{ sh r25, r26 ; ori r15, r16, 5 ; mulhha_uu r5, r6, r7 }
	{ sh r25, r26 ; ori r15, r16, 5 ; seqi r5, r6, 5 }
	{ sh r25, r26 ; ori r15, r16, 5 }
	{ sh r25, r26 ; ori r5, r6, 5 ; s3a r15, r16, r17 }
	{ sh r25, r26 ; pcnt r5, r6 ; addi r15, r16, 5 }
	{ sh r25, r26 ; pcnt r5, r6 ; seqi r15, r16, 5 }
	{ sh r25, r26 ; rl r15, r16, r17 ; andi r5, r6, 5 }
	{ sh r25, r26 ; rl r15, r16, r17 ; mvz r5, r6, r7 }
	{ sh r25, r26 ; rl r15, r16, r17 ; slte r5, r6, r7 }
	{ sh r25, r26 ; rl r5, r6, r7 ; info 19 }
	{ sh r25, r26 ; rl r5, r6, r7 ; slt r15, r16, r17 }
	{ sh r25, r26 ; rli r15, r16, 5 ; fnop }
	{ sh r25, r26 ; rli r15, r16, 5 ; ori r5, r6, 5 }
	{ sh r25, r26 ; rli r15, r16, 5 ; sra r5, r6, r7 }
	{ sh r25, r26 ; rli r5, r6, 5 ; nop }
	{ sh r25, r26 ; rli r5, r6, 5 ; slti_u r15, r16, 5 }
	{ sh r25, r26 ; s1a r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ sh r25, r26 ; s1a r15, r16, r17 ; s2a r5, r6, r7 }
	{ sh r25, r26 ; s1a r15, r16, r17 ; tblidxb2 r5, r6 }
	{ sh r25, r26 ; s1a r5, r6, r7 ; rli r15, r16, 5 }
	{ sh r25, r26 ; s1a r5, r6, r7 ; xor r15, r16, r17 }
	{ sh r25, r26 ; s2a r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ sh r25, r26 ; s2a r15, r16, r17 ; shli r5, r6, 5 }
	{ sh r25, r26 ; s2a r5, r6, r7 ; addi r15, r16, 5 }
	{ sh r25, r26 ; s2a r5, r6, r7 ; seqi r15, r16, 5 }
	{ sh r25, r26 ; s3a r15, r16, r17 ; andi r5, r6, 5 }
	{ sh r25, r26 ; s3a r15, r16, r17 ; mvz r5, r6, r7 }
	{ sh r25, r26 ; s3a r15, r16, r17 ; slte r5, r6, r7 }
	{ sh r25, r26 ; s3a r5, r6, r7 ; info 19 }
	{ sh r25, r26 ; s3a r5, r6, r7 ; slt r15, r16, r17 }
	{ sh r25, r26 ; seq r15, r16, r17 ; fnop }
	{ sh r25, r26 ; seq r15, r16, r17 ; ori r5, r6, 5 }
	{ sh r25, r26 ; seq r15, r16, r17 ; sra r5, r6, r7 }
	{ sh r25, r26 ; seq r5, r6, r7 ; nop }
	{ sh r25, r26 ; seq r5, r6, r7 ; slti_u r15, r16, 5 }
	{ sh r25, r26 ; seqi r15, r16, 5 ; mulhh_ss r5, r6, r7 }
	{ sh r25, r26 ; seqi r15, r16, 5 ; s2a r5, r6, r7 }
	{ sh r25, r26 ; seqi r15, r16, 5 ; tblidxb2 r5, r6 }
	{ sh r25, r26 ; seqi r5, r6, 5 ; rli r15, r16, 5 }
	{ sh r25, r26 ; seqi r5, r6, 5 ; xor r15, r16, r17 }
	{ sh r25, r26 ; shl r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ sh r25, r26 ; shl r15, r16, r17 ; shli r5, r6, 5 }
	{ sh r25, r26 ; shl r5, r6, r7 ; addi r15, r16, 5 }
	{ sh r25, r26 ; shl r5, r6, r7 ; seqi r15, r16, 5 }
	{ sh r25, r26 ; shli r15, r16, 5 ; andi r5, r6, 5 }
	{ sh r25, r26 ; shli r15, r16, 5 ; mvz r5, r6, r7 }
	{ sh r25, r26 ; shli r15, r16, 5 ; slte r5, r6, r7 }
	{ sh r25, r26 ; shli r5, r6, 5 ; info 19 }
	{ sh r25, r26 ; shli r5, r6, 5 ; slt r15, r16, r17 }
	{ sh r25, r26 ; shr r15, r16, r17 ; fnop }
	{ sh r25, r26 ; shr r15, r16, r17 ; ori r5, r6, 5 }
	{ sh r25, r26 ; shr r15, r16, r17 ; sra r5, r6, r7 }
	{ sh r25, r26 ; shr r5, r6, r7 ; nop }
	{ sh r25, r26 ; shr r5, r6, r7 ; slti_u r15, r16, 5 }
	{ sh r25, r26 ; shri r15, r16, 5 ; mulhh_ss r5, r6, r7 }
	{ sh r25, r26 ; shri r15, r16, 5 ; s2a r5, r6, r7 }
	{ sh r25, r26 ; shri r15, r16, 5 ; tblidxb2 r5, r6 }
	{ sh r25, r26 ; shri r5, r6, 5 ; rli r15, r16, 5 }
	{ sh r25, r26 ; shri r5, r6, 5 ; xor r15, r16, r17 }
	{ sh r25, r26 ; slt r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ sh r25, r26 ; slt r15, r16, r17 ; shli r5, r6, 5 }
	{ sh r25, r26 ; slt r5, r6, r7 ; addi r15, r16, 5 }
	{ sh r25, r26 ; slt r5, r6, r7 ; seqi r15, r16, 5 }
	{ sh r25, r26 ; slt_u r15, r16, r17 ; andi r5, r6, 5 }
	{ sh r25, r26 ; slt_u r15, r16, r17 ; mvz r5, r6, r7 }
	{ sh r25, r26 ; slt_u r15, r16, r17 ; slte r5, r6, r7 }
	{ sh r25, r26 ; slt_u r5, r6, r7 ; info 19 }
	{ sh r25, r26 ; slt_u r5, r6, r7 ; slt r15, r16, r17 }
	{ sh r25, r26 ; slte r15, r16, r17 ; fnop }
	{ sh r25, r26 ; slte r15, r16, r17 ; ori r5, r6, 5 }
	{ sh r25, r26 ; slte r15, r16, r17 ; sra r5, r6, r7 }
	{ sh r25, r26 ; slte r5, r6, r7 ; nop }
	{ sh r25, r26 ; slte r5, r6, r7 ; slti_u r15, r16, 5 }
	{ sh r25, r26 ; slte_u r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ sh r25, r26 ; slte_u r15, r16, r17 ; s2a r5, r6, r7 }
	{ sh r25, r26 ; slte_u r15, r16, r17 ; tblidxb2 r5, r6 }
	{ sh r25, r26 ; slte_u r5, r6, r7 ; rli r15, r16, 5 }
	{ sh r25, r26 ; slte_u r5, r6, r7 ; xor r15, r16, r17 }
	{ sh r25, r26 ; slti r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ sh r25, r26 ; slti r15, r16, 5 ; shli r5, r6, 5 }
	{ sh r25, r26 ; slti r5, r6, 5 ; addi r15, r16, 5 }
	{ sh r25, r26 ; slti r5, r6, 5 ; seqi r15, r16, 5 }
	{ sh r25, r26 ; slti_u r15, r16, 5 ; andi r5, r6, 5 }
	{ sh r25, r26 ; slti_u r15, r16, 5 ; mvz r5, r6, r7 }
	{ sh r25, r26 ; slti_u r15, r16, 5 ; slte r5, r6, r7 }
	{ sh r25, r26 ; slti_u r5, r6, 5 ; info 19 }
	{ sh r25, r26 ; slti_u r5, r6, 5 ; slt r15, r16, r17 }
	{ sh r25, r26 ; sne r15, r16, r17 ; fnop }
	{ sh r25, r26 ; sne r15, r16, r17 ; ori r5, r6, 5 }
	{ sh r25, r26 ; sne r15, r16, r17 ; sra r5, r6, r7 }
	{ sh r25, r26 ; sne r5, r6, r7 ; nop }
	{ sh r25, r26 ; sne r5, r6, r7 ; slti_u r15, r16, 5 }
	{ sh r25, r26 ; sra r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ sh r25, r26 ; sra r15, r16, r17 ; s2a r5, r6, r7 }
	{ sh r25, r26 ; sra r15, r16, r17 ; tblidxb2 r5, r6 }
	{ sh r25, r26 ; sra r5, r6, r7 ; rli r15, r16, 5 }
	{ sh r25, r26 ; sra r5, r6, r7 ; xor r15, r16, r17 }
	{ sh r25, r26 ; srai r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ sh r25, r26 ; srai r15, r16, 5 ; shli r5, r6, 5 }
	{ sh r25, r26 ; srai r5, r6, 5 ; addi r15, r16, 5 }
	{ sh r25, r26 ; srai r5, r6, 5 ; seqi r15, r16, 5 }
	{ sh r25, r26 ; sub r15, r16, r17 ; andi r5, r6, 5 }
	{ sh r25, r26 ; sub r15, r16, r17 ; mvz r5, r6, r7 }
	{ sh r25, r26 ; sub r15, r16, r17 ; slte r5, r6, r7 }
	{ sh r25, r26 ; sub r5, r6, r7 ; info 19 }
	{ sh r25, r26 ; sub r5, r6, r7 ; slt r15, r16, r17 }
	{ sh r25, r26 ; tblidxb0 r5, r6 ; move r15, r16 }
	{ sh r25, r26 ; tblidxb0 r5, r6 ; slte r15, r16, r17 }
	{ sh r25, r26 ; tblidxb1 r5, r6 ; mz r15, r16, r17 }
	{ sh r25, r26 ; tblidxb1 r5, r6 ; slti r15, r16, 5 }
	{ sh r25, r26 ; tblidxb2 r5, r6 ; nor r15, r16, r17 }
	{ sh r25, r26 ; tblidxb2 r5, r6 ; sne r15, r16, r17 }
	{ sh r25, r26 ; tblidxb3 r5, r6 ; ori r15, r16, 5 }
	{ sh r25, r26 ; tblidxb3 r5, r6 ; srai r15, r16, 5 }
	{ sh r25, r26 ; xor r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ sh r25, r26 ; xor r15, r16, r17 ; seqi r5, r6, 5 }
	{ sh r25, r26 ; xor r15, r16, r17 }
	{ sh r25, r26 ; xor r5, r6, r7 ; s3a r15, r16, r17 }
	{ shadd r15, r16, 5 ; addb r5, r6, r7 }
	{ shadd r15, r16, 5 ; crc32_32 r5, r6, r7 }
	{ shadd r15, r16, 5 ; mnz r5, r6, r7 }
	{ shadd r15, r16, 5 ; mulhla_us r5, r6, r7 }
	{ shadd r15, r16, 5 ; packhb r5, r6, r7 }
	{ shadd r15, r16, 5 ; seqih r5, r6, 5 }
	{ shadd r15, r16, 5 ; slteb_u r5, r6, r7 }
	{ shadd r15, r16, 5 ; sub r5, r6, r7 }
	{ shl r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
	{ shl r15, r16, r17 ; adds r5, r6, r7 }
	{ shl r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
	{ shl r15, r16, r17 ; bytex r5, r6 ; lw r25, r26 }
	{ shl r15, r16, r17 ; ctz r5, r6 ; lh r25, r26 }
	{ shl r15, r16, r17 ; info 19 ; lb_u r25, r26 }
	{ shl r15, r16, r17 ; lb r25, r26 ; clz r5, r6 }
	{ shl r15, r16, r17 ; lb r25, r26 ; nor r5, r6, r7 }
	{ shl r15, r16, r17 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ shl r15, r16, r17 ; lb_u r25, r26 ; info 19 }
	{ shl r15, r16, r17 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ shl r15, r16, r17 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ shl r15, r16, r17 ; lh r25, r26 ; movei r5, 5 }
	{ shl r15, r16, r17 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ shl r15, r16, r17 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ shl r15, r16, r17 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ shl r15, r16, r17 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ shl r15, r16, r17 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ shl r15, r16, r17 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ shl r15, r16, r17 ; lw r25, r26 ; shli r5, r6, 5 }
	{ shl r15, r16, r17 ; maxh r5, r6, r7 }
	{ shl r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
	{ shl r15, r16, r17 ; moveli r5, 0x1234 }
	{ shl r15, r16, r17 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ shl r15, r16, r17 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ shl r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ shl r15, r16, r17 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ shl r15, r16, r17 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ shl r15, r16, r17 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ shl r15, r16, r17 ; nop ; lh r25, r26 }
	{ shl r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
	{ shl r15, r16, r17 ; packhs r5, r6, r7 }
	{ shl r15, r16, r17 ; prefetch r25 ; fnop }
	{ shl r15, r16, r17 ; prefetch r25 ; ori r5, r6, 5 }
	{ shl r15, r16, r17 ; prefetch r25 ; sra r5, r6, r7 }
	{ shl r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ shl r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ shl r15, r16, r17 ; sadah r5, r6, r7 }
	{ shl r15, r16, r17 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ shl r15, r16, r17 ; sb r25, r26 ; seq r5, r6, r7 }
	{ shl r15, r16, r17 ; sb r25, r26 ; xor r5, r6, r7 }
	{ shl r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ shl r15, r16, r17 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ shl r15, r16, r17 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ shl r15, r16, r17 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ shl r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
	{ shl r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ shl r15, r16, r17 ; slt r5, r6, r7 }
	{ shl r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
	{ shl r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ shl r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ shl r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
	{ shl r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ shl r15, r16, r17 ; sw r25, r26 ; clz r5, r6 }
	{ shl r15, r16, r17 ; sw r25, r26 ; nor r5, r6, r7 }
	{ shl r15, r16, r17 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ shl r15, r16, r17 ; tblidxb0 r5, r6 }
	{ shl r15, r16, r17 ; tblidxb2 r5, r6 }
	{ shl r15, r16, r17 ; xor r5, r6, r7 }
	{ shl r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
	{ shl r5, r6, r7 ; and r15, r16, r17 }
	{ shl r5, r6, r7 ; fnop ; prefetch r25 }
	{ shl r5, r6, r7 ; info 19 ; lw r25, r26 }
	{ shl r5, r6, r7 ; lb r25, r26 ; and r15, r16, r17 }
	{ shl r5, r6, r7 ; lb r25, r26 ; shl r15, r16, r17 }
	{ shl r5, r6, r7 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ shl r5, r6, r7 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ shl r5, r6, r7 ; lh r25, r26 ; and r15, r16, r17 }
	{ shl r5, r6, r7 ; lh r25, r26 ; shl r15, r16, r17 }
	{ shl r5, r6, r7 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ shl r5, r6, r7 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ shl r5, r6, r7 ; lw r25, r26 ; addi r15, r16, 5 }
	{ shl r5, r6, r7 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ shl r5, r6, r7 ; maxb_u r15, r16, r17 }
	{ shl r5, r6, r7 ; mnz r15, r16, r17 }
	{ shl r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
	{ shl r5, r6, r7 ; nop ; lh r25, r26 }
	{ shl r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
	{ shl r5, r6, r7 ; packhs r15, r16, r17 }
	{ shl r5, r6, r7 ; prefetch r25 ; s1a r15, r16, r17 }
	{ shl r5, r6, r7 ; prefetch r25 }
	{ shl r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
	{ shl r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ shl r5, r6, r7 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ shl r5, r6, r7 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ shl r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
	{ shl r5, r6, r7 ; sh r25, r26 ; andi r15, r16, 5 }
	{ shl r5, r6, r7 ; sh r25, r26 ; shli r15, r16, 5 }
	{ shl r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
	{ shl r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
	{ shl r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
	{ shl r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ shl r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ shl r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
	{ shl r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
	{ shl r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
	{ shl r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
	{ shl r5, r6, r7 ; sw r25, r26 ; nor r15, r16, r17 }
	{ shl r5, r6, r7 ; sw r25, r26 ; sne r15, r16, r17 }
	{ shlb r15, r16, r17 ; add r5, r6, r7 }
	{ shlb r15, r16, r17 ; clz r5, r6 }
	{ shlb r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
	{ shlb r15, r16, r17 ; mulhla_su r5, r6, r7 }
	{ shlb r15, r16, r17 ; packbs_u r5, r6, r7 }
	{ shlb r15, r16, r17 ; seqib r5, r6, 5 }
	{ shlb r15, r16, r17 ; slteb r5, r6, r7 }
	{ shlb r15, r16, r17 ; sraih r5, r6, 5 }
	{ shlb r5, r6, r7 ; addih r15, r16, 5 }
	{ shlb r5, r6, r7 ; iret }
	{ shlb r5, r6, r7 ; maxib_u r15, r16, 5 }
	{ shlb r5, r6, r7 ; nop }
	{ shlb r5, r6, r7 ; seqi r15, r16, 5 }
	{ shlb r5, r6, r7 ; sltb_u r15, r16, r17 }
	{ shlb r5, r6, r7 ; srah r15, r16, r17 }
	{ shlh r15, r16, r17 ; addhs r5, r6, r7 }
	{ shlh r15, r16, r17 ; dword_align r5, r6, r7 }
	{ shlh r15, r16, r17 ; move r5, r6 }
	{ shlh r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ shlh r15, r16, r17 ; pcnt r5, r6 }
	{ shlh r15, r16, r17 ; shlh r5, r6, r7 }
	{ shlh r15, r16, r17 ; slth r5, r6, r7 }
	{ shlh r15, r16, r17 ; subh r5, r6, r7 }
	{ shlh r5, r6, r7 ; and r15, r16, r17 }
	{ shlh r5, r6, r7 ; jrp r15 }
	{ shlh r5, r6, r7 ; minb_u r15, r16, r17 }
	{ shlh r5, r6, r7 ; packbs_u r15, r16, r17 }
	{ shlh r5, r6, r7 ; shadd r15, r16, 5 }
	{ shlh r5, r6, r7 ; slteb_u r15, r16, r17 }
	{ shlh r5, r6, r7 ; sub r15, r16, r17 }
	{ shli r15, r16, 5 ; add r5, r6, r7 ; sw r25, r26 }
	{ shli r15, r16, 5 ; adds r5, r6, r7 }
	{ shli r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
	{ shli r15, r16, 5 ; bytex r5, r6 ; lw r25, r26 }
	{ shli r15, r16, 5 ; ctz r5, r6 ; lh r25, r26 }
	{ shli r15, r16, 5 ; info 19 ; lb_u r25, r26 }
	{ shli r15, r16, 5 ; lb r25, r26 ; clz r5, r6 }
	{ shli r15, r16, 5 ; lb r25, r26 ; nor r5, r6, r7 }
	{ shli r15, r16, 5 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ shli r15, r16, 5 ; lb_u r25, r26 ; info 19 }
	{ shli r15, r16, 5 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ shli r15, r16, 5 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ shli r15, r16, 5 ; lh r25, r26 ; movei r5, 5 }
	{ shli r15, r16, 5 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ shli r15, r16, 5 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ shli r15, r16, 5 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ shli r15, r16, 5 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ shli r15, r16, 5 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ shli r15, r16, 5 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ shli r15, r16, 5 ; lw r25, r26 ; shli r5, r6, 5 }
	{ shli r15, r16, 5 ; maxh r5, r6, r7 }
	{ shli r15, r16, 5 ; move r5, r6 ; lb r25, r26 }
	{ shli r15, r16, 5 ; moveli r5, 0x1234 }
	{ shli r15, r16, 5 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ shli r15, r16, 5 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ shli r15, r16, 5 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ shli r15, r16, 5 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ shli r15, r16, 5 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ shli r15, r16, 5 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ shli r15, r16, 5 ; nop ; lh r25, r26 }
	{ shli r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
	{ shli r15, r16, 5 ; packhs r5, r6, r7 }
	{ shli r15, r16, 5 ; prefetch r25 ; fnop }
	{ shli r15, r16, 5 ; prefetch r25 ; ori r5, r6, 5 }
	{ shli r15, r16, 5 ; prefetch r25 ; sra r5, r6, r7 }
	{ shli r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ shli r15, r16, 5 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ shli r15, r16, 5 ; sadah r5, r6, r7 }
	{ shli r15, r16, 5 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ shli r15, r16, 5 ; sb r25, r26 ; seq r5, r6, r7 }
	{ shli r15, r16, 5 ; sb r25, r26 ; xor r5, r6, r7 }
	{ shli r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ shli r15, r16, 5 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ shli r15, r16, 5 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ shli r15, r16, 5 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ shli r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
	{ shli r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ shli r15, r16, 5 ; slt r5, r6, r7 }
	{ shli r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
	{ shli r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ shli r15, r16, 5 ; sltib_u r5, r6, 5 }
	{ shli r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
	{ shli r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ shli r15, r16, 5 ; sw r25, r26 ; clz r5, r6 }
	{ shli r15, r16, 5 ; sw r25, r26 ; nor r5, r6, r7 }
	{ shli r15, r16, 5 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ shli r15, r16, 5 ; tblidxb0 r5, r6 }
	{ shli r15, r16, 5 ; tblidxb2 r5, r6 }
	{ shli r15, r16, 5 ; xor r5, r6, r7 }
	{ shli r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
	{ shli r5, r6, 5 ; and r15, r16, r17 }
	{ shli r5, r6, 5 ; fnop ; prefetch r25 }
	{ shli r5, r6, 5 ; info 19 ; lw r25, r26 }
	{ shli r5, r6, 5 ; lb r25, r26 ; and r15, r16, r17 }
	{ shli r5, r6, 5 ; lb r25, r26 ; shl r15, r16, r17 }
	{ shli r5, r6, 5 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ shli r5, r6, 5 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ shli r5, r6, 5 ; lh r25, r26 ; and r15, r16, r17 }
	{ shli r5, r6, 5 ; lh r25, r26 ; shl r15, r16, r17 }
	{ shli r5, r6, 5 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ shli r5, r6, 5 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ shli r5, r6, 5 ; lw r25, r26 ; addi r15, r16, 5 }
	{ shli r5, r6, 5 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ shli r5, r6, 5 ; maxb_u r15, r16, r17 }
	{ shli r5, r6, 5 ; mnz r15, r16, r17 }
	{ shli r5, r6, 5 ; movei r15, 5 ; sh r25, r26 }
	{ shli r5, r6, 5 ; nop ; lh r25, r26 }
	{ shli r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
	{ shli r5, r6, 5 ; packhs r15, r16, r17 }
	{ shli r5, r6, 5 ; prefetch r25 ; s1a r15, r16, r17 }
	{ shli r5, r6, 5 ; prefetch r25 }
	{ shli r5, r6, 5 ; rli r15, r16, 5 ; sw r25, r26 }
	{ shli r5, r6, 5 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ shli r5, r6, 5 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ shli r5, r6, 5 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ shli r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
	{ shli r5, r6, 5 ; sh r25, r26 ; andi r15, r16, 5 }
	{ shli r5, r6, 5 ; sh r25, r26 ; shli r15, r16, 5 }
	{ shli r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
	{ shli r5, r6, 5 ; shr r15, r16, r17 ; lb r25, r26 }
	{ shli r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
	{ shli r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ shli r5, r6, 5 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ shli r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
	{ shli r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
	{ shli r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
	{ shli r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
	{ shli r5, r6, 5 ; sw r25, r26 ; nor r15, r16, r17 }
	{ shli r5, r6, 5 ; sw r25, r26 ; sne r15, r16, r17 }
	{ shlib r15, r16, 5 ; add r5, r6, r7 }
	{ shlib r15, r16, 5 ; clz r5, r6 }
	{ shlib r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
	{ shlib r15, r16, 5 ; mulhla_su r5, r6, r7 }
	{ shlib r15, r16, 5 ; packbs_u r5, r6, r7 }
	{ shlib r15, r16, 5 ; seqib r5, r6, 5 }
	{ shlib r15, r16, 5 ; slteb r5, r6, r7 }
	{ shlib r15, r16, 5 ; sraih r5, r6, 5 }
	{ shlib r5, r6, 5 ; addih r15, r16, 5 }
	{ shlib r5, r6, 5 ; iret }
	{ shlib r5, r6, 5 ; maxib_u r15, r16, 5 }
	{ shlib r5, r6, 5 ; nop }
	{ shlib r5, r6, 5 ; seqi r15, r16, 5 }
	{ shlib r5, r6, 5 ; sltb_u r15, r16, r17 }
	{ shlib r5, r6, 5 ; srah r15, r16, r17 }
	{ shlih r15, r16, 5 ; addhs r5, r6, r7 }
	{ shlih r15, r16, 5 ; dword_align r5, r6, r7 }
	{ shlih r15, r16, 5 ; move r5, r6 }
	{ shlih r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ shlih r15, r16, 5 ; pcnt r5, r6 }
	{ shlih r15, r16, 5 ; shlh r5, r6, r7 }
	{ shlih r15, r16, 5 ; slth r5, r6, r7 }
	{ shlih r15, r16, 5 ; subh r5, r6, r7 }
	{ shlih r5, r6, 5 ; and r15, r16, r17 }
	{ shlih r5, r6, 5 ; jrp r15 }
	{ shlih r5, r6, 5 ; minb_u r15, r16, r17 }
	{ shlih r5, r6, 5 ; packbs_u r15, r16, r17 }
	{ shlih r5, r6, 5 ; shadd r15, r16, 5 }
	{ shlih r5, r6, 5 ; slteb_u r15, r16, r17 }
	{ shlih r5, r6, 5 ; sub r15, r16, r17 }
	{ shr r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
	{ shr r15, r16, r17 ; adds r5, r6, r7 }
	{ shr r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
	{ shr r15, r16, r17 ; bytex r5, r6 ; lw r25, r26 }
	{ shr r15, r16, r17 ; ctz r5, r6 ; lh r25, r26 }
	{ shr r15, r16, r17 ; info 19 ; lb_u r25, r26 }
	{ shr r15, r16, r17 ; lb r25, r26 ; clz r5, r6 }
	{ shr r15, r16, r17 ; lb r25, r26 ; nor r5, r6, r7 }
	{ shr r15, r16, r17 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ shr r15, r16, r17 ; lb_u r25, r26 ; info 19 }
	{ shr r15, r16, r17 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ shr r15, r16, r17 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ shr r15, r16, r17 ; lh r25, r26 ; movei r5, 5 }
	{ shr r15, r16, r17 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ shr r15, r16, r17 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ shr r15, r16, r17 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ shr r15, r16, r17 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ shr r15, r16, r17 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ shr r15, r16, r17 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ shr r15, r16, r17 ; lw r25, r26 ; shli r5, r6, 5 }
	{ shr r15, r16, r17 ; maxh r5, r6, r7 }
	{ shr r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
	{ shr r15, r16, r17 ; moveli r5, 0x1234 }
	{ shr r15, r16, r17 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ shr r15, r16, r17 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ shr r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ shr r15, r16, r17 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ shr r15, r16, r17 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ shr r15, r16, r17 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ shr r15, r16, r17 ; nop ; lh r25, r26 }
	{ shr r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
	{ shr r15, r16, r17 ; packhs r5, r6, r7 }
	{ shr r15, r16, r17 ; prefetch r25 ; fnop }
	{ shr r15, r16, r17 ; prefetch r25 ; ori r5, r6, 5 }
	{ shr r15, r16, r17 ; prefetch r25 ; sra r5, r6, r7 }
	{ shr r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ shr r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ shr r15, r16, r17 ; sadah r5, r6, r7 }
	{ shr r15, r16, r17 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ shr r15, r16, r17 ; sb r25, r26 ; seq r5, r6, r7 }
	{ shr r15, r16, r17 ; sb r25, r26 ; xor r5, r6, r7 }
	{ shr r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ shr r15, r16, r17 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ shr r15, r16, r17 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ shr r15, r16, r17 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ shr r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
	{ shr r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ shr r15, r16, r17 ; slt r5, r6, r7 }
	{ shr r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
	{ shr r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ shr r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ shr r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
	{ shr r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ shr r15, r16, r17 ; sw r25, r26 ; clz r5, r6 }
	{ shr r15, r16, r17 ; sw r25, r26 ; nor r5, r6, r7 }
	{ shr r15, r16, r17 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ shr r15, r16, r17 ; tblidxb0 r5, r6 }
	{ shr r15, r16, r17 ; tblidxb2 r5, r6 }
	{ shr r15, r16, r17 ; xor r5, r6, r7 }
	{ shr r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
	{ shr r5, r6, r7 ; and r15, r16, r17 }
	{ shr r5, r6, r7 ; fnop ; prefetch r25 }
	{ shr r5, r6, r7 ; info 19 ; lw r25, r26 }
	{ shr r5, r6, r7 ; lb r25, r26 ; and r15, r16, r17 }
	{ shr r5, r6, r7 ; lb r25, r26 ; shl r15, r16, r17 }
	{ shr r5, r6, r7 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ shr r5, r6, r7 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ shr r5, r6, r7 ; lh r25, r26 ; and r15, r16, r17 }
	{ shr r5, r6, r7 ; lh r25, r26 ; shl r15, r16, r17 }
	{ shr r5, r6, r7 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ shr r5, r6, r7 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ shr r5, r6, r7 ; lw r25, r26 ; addi r15, r16, 5 }
	{ shr r5, r6, r7 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ shr r5, r6, r7 ; maxb_u r15, r16, r17 }
	{ shr r5, r6, r7 ; mnz r15, r16, r17 }
	{ shr r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
	{ shr r5, r6, r7 ; nop ; lh r25, r26 }
	{ shr r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
	{ shr r5, r6, r7 ; packhs r15, r16, r17 }
	{ shr r5, r6, r7 ; prefetch r25 ; s1a r15, r16, r17 }
	{ shr r5, r6, r7 ; prefetch r25 }
	{ shr r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
	{ shr r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ shr r5, r6, r7 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ shr r5, r6, r7 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ shr r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
	{ shr r5, r6, r7 ; sh r25, r26 ; andi r15, r16, 5 }
	{ shr r5, r6, r7 ; sh r25, r26 ; shli r15, r16, 5 }
	{ shr r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
	{ shr r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
	{ shr r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
	{ shr r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ shr r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ shr r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
	{ shr r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
	{ shr r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
	{ shr r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
	{ shr r5, r6, r7 ; sw r25, r26 ; nor r15, r16, r17 }
	{ shr r5, r6, r7 ; sw r25, r26 ; sne r15, r16, r17 }
	{ shrb r15, r16, r17 ; add r5, r6, r7 }
	{ shrb r15, r16, r17 ; clz r5, r6 }
	{ shrb r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
	{ shrb r15, r16, r17 ; mulhla_su r5, r6, r7 }
	{ shrb r15, r16, r17 ; packbs_u r5, r6, r7 }
	{ shrb r15, r16, r17 ; seqib r5, r6, 5 }
	{ shrb r15, r16, r17 ; slteb r5, r6, r7 }
	{ shrb r15, r16, r17 ; sraih r5, r6, 5 }
	{ shrb r5, r6, r7 ; addih r15, r16, 5 }
	{ shrb r5, r6, r7 ; iret }
	{ shrb r5, r6, r7 ; maxib_u r15, r16, 5 }
	{ shrb r5, r6, r7 ; nop }
	{ shrb r5, r6, r7 ; seqi r15, r16, 5 }
	{ shrb r5, r6, r7 ; sltb_u r15, r16, r17 }
	{ shrb r5, r6, r7 ; srah r15, r16, r17 }
	{ shrh r15, r16, r17 ; addhs r5, r6, r7 }
	{ shrh r15, r16, r17 ; dword_align r5, r6, r7 }
	{ shrh r15, r16, r17 ; move r5, r6 }
	{ shrh r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ shrh r15, r16, r17 ; pcnt r5, r6 }
	{ shrh r15, r16, r17 ; shlh r5, r6, r7 }
	{ shrh r15, r16, r17 ; slth r5, r6, r7 }
	{ shrh r15, r16, r17 ; subh r5, r6, r7 }
	{ shrh r5, r6, r7 ; and r15, r16, r17 }
	{ shrh r5, r6, r7 ; jrp r15 }
	{ shrh r5, r6, r7 ; minb_u r15, r16, r17 }
	{ shrh r5, r6, r7 ; packbs_u r15, r16, r17 }
	{ shrh r5, r6, r7 ; shadd r15, r16, 5 }
	{ shrh r5, r6, r7 ; slteb_u r15, r16, r17 }
	{ shrh r5, r6, r7 ; sub r15, r16, r17 }
	{ shri r15, r16, 5 ; add r5, r6, r7 ; sw r25, r26 }
	{ shri r15, r16, 5 ; adds r5, r6, r7 }
	{ shri r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
	{ shri r15, r16, 5 ; bytex r5, r6 ; lw r25, r26 }
	{ shri r15, r16, 5 ; ctz r5, r6 ; lh r25, r26 }
	{ shri r15, r16, 5 ; info 19 ; lb_u r25, r26 }
	{ shri r15, r16, 5 ; lb r25, r26 ; clz r5, r6 }
	{ shri r15, r16, 5 ; lb r25, r26 ; nor r5, r6, r7 }
	{ shri r15, r16, 5 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ shri r15, r16, 5 ; lb_u r25, r26 ; info 19 }
	{ shri r15, r16, 5 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ shri r15, r16, 5 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ shri r15, r16, 5 ; lh r25, r26 ; movei r5, 5 }
	{ shri r15, r16, 5 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ shri r15, r16, 5 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ shri r15, r16, 5 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ shri r15, r16, 5 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ shri r15, r16, 5 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ shri r15, r16, 5 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ shri r15, r16, 5 ; lw r25, r26 ; shli r5, r6, 5 }
	{ shri r15, r16, 5 ; maxh r5, r6, r7 }
	{ shri r15, r16, 5 ; move r5, r6 ; lb r25, r26 }
	{ shri r15, r16, 5 ; moveli r5, 0x1234 }
	{ shri r15, r16, 5 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ shri r15, r16, 5 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ shri r15, r16, 5 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ shri r15, r16, 5 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ shri r15, r16, 5 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ shri r15, r16, 5 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ shri r15, r16, 5 ; nop ; lh r25, r26 }
	{ shri r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
	{ shri r15, r16, 5 ; packhs r5, r6, r7 }
	{ shri r15, r16, 5 ; prefetch r25 ; fnop }
	{ shri r15, r16, 5 ; prefetch r25 ; ori r5, r6, 5 }
	{ shri r15, r16, 5 ; prefetch r25 ; sra r5, r6, r7 }
	{ shri r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ shri r15, r16, 5 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ shri r15, r16, 5 ; sadah r5, r6, r7 }
	{ shri r15, r16, 5 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ shri r15, r16, 5 ; sb r25, r26 ; seq r5, r6, r7 }
	{ shri r15, r16, 5 ; sb r25, r26 ; xor r5, r6, r7 }
	{ shri r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ shri r15, r16, 5 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ shri r15, r16, 5 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ shri r15, r16, 5 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ shri r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
	{ shri r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ shri r15, r16, 5 ; slt r5, r6, r7 }
	{ shri r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
	{ shri r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ shri r15, r16, 5 ; sltib_u r5, r6, 5 }
	{ shri r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
	{ shri r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ shri r15, r16, 5 ; sw r25, r26 ; clz r5, r6 }
	{ shri r15, r16, 5 ; sw r25, r26 ; nor r5, r6, r7 }
	{ shri r15, r16, 5 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ shri r15, r16, 5 ; tblidxb0 r5, r6 }
	{ shri r15, r16, 5 ; tblidxb2 r5, r6 }
	{ shri r15, r16, 5 ; xor r5, r6, r7 }
	{ shri r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
	{ shri r5, r6, 5 ; and r15, r16, r17 }
	{ shri r5, r6, 5 ; fnop ; prefetch r25 }
	{ shri r5, r6, 5 ; info 19 ; lw r25, r26 }
	{ shri r5, r6, 5 ; lb r25, r26 ; and r15, r16, r17 }
	{ shri r5, r6, 5 ; lb r25, r26 ; shl r15, r16, r17 }
	{ shri r5, r6, 5 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ shri r5, r6, 5 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ shri r5, r6, 5 ; lh r25, r26 ; and r15, r16, r17 }
	{ shri r5, r6, 5 ; lh r25, r26 ; shl r15, r16, r17 }
	{ shri r5, r6, 5 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ shri r5, r6, 5 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ shri r5, r6, 5 ; lw r25, r26 ; addi r15, r16, 5 }
	{ shri r5, r6, 5 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ shri r5, r6, 5 ; maxb_u r15, r16, r17 }
	{ shri r5, r6, 5 ; mnz r15, r16, r17 }
	{ shri r5, r6, 5 ; movei r15, 5 ; sh r25, r26 }
	{ shri r5, r6, 5 ; nop ; lh r25, r26 }
	{ shri r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
	{ shri r5, r6, 5 ; packhs r15, r16, r17 }
	{ shri r5, r6, 5 ; prefetch r25 ; s1a r15, r16, r17 }
	{ shri r5, r6, 5 ; prefetch r25 }
	{ shri r5, r6, 5 ; rli r15, r16, 5 ; sw r25, r26 }
	{ shri r5, r6, 5 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ shri r5, r6, 5 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ shri r5, r6, 5 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ shri r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
	{ shri r5, r6, 5 ; sh r25, r26 ; andi r15, r16, 5 }
	{ shri r5, r6, 5 ; sh r25, r26 ; shli r15, r16, 5 }
	{ shri r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
	{ shri r5, r6, 5 ; shr r15, r16, r17 ; lb r25, r26 }
	{ shri r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
	{ shri r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ shri r5, r6, 5 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ shri r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
	{ shri r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
	{ shri r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
	{ shri r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
	{ shri r5, r6, 5 ; sw r25, r26 ; nor r15, r16, r17 }
	{ shri r5, r6, 5 ; sw r25, r26 ; sne r15, r16, r17 }
	{ shrib r15, r16, 5 ; add r5, r6, r7 }
	{ shrib r15, r16, 5 ; clz r5, r6 }
	{ shrib r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
	{ shrib r15, r16, 5 ; mulhla_su r5, r6, r7 }
	{ shrib r15, r16, 5 ; packbs_u r5, r6, r7 }
	{ shrib r15, r16, 5 ; seqib r5, r6, 5 }
	{ shrib r15, r16, 5 ; slteb r5, r6, r7 }
	{ shrib r15, r16, 5 ; sraih r5, r6, 5 }
	{ shrib r5, r6, 5 ; addih r15, r16, 5 }
	{ shrib r5, r6, 5 ; iret }
	{ shrib r5, r6, 5 ; maxib_u r15, r16, 5 }
	{ shrib r5, r6, 5 ; nop }
	{ shrib r5, r6, 5 ; seqi r15, r16, 5 }
	{ shrib r5, r6, 5 ; sltb_u r15, r16, r17 }
	{ shrib r5, r6, 5 ; srah r15, r16, r17 }
	{ shrih r15, r16, 5 ; addhs r5, r6, r7 }
	{ shrih r15, r16, 5 ; dword_align r5, r6, r7 }
	{ shrih r15, r16, 5 ; move r5, r6 }
	{ shrih r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ shrih r15, r16, 5 ; pcnt r5, r6 }
	{ shrih r15, r16, 5 ; shlh r5, r6, r7 }
	{ shrih r15, r16, 5 ; slth r5, r6, r7 }
	{ shrih r15, r16, 5 ; subh r5, r6, r7 }
	{ shrih r5, r6, 5 ; and r15, r16, r17 }
	{ shrih r5, r6, 5 ; jrp r15 }
	{ shrih r5, r6, 5 ; minb_u r15, r16, r17 }
	{ shrih r5, r6, 5 ; packbs_u r15, r16, r17 }
	{ shrih r5, r6, 5 ; shadd r15, r16, 5 }
	{ shrih r5, r6, 5 ; slteb_u r15, r16, r17 }
	{ shrih r5, r6, 5 ; sub r15, r16, r17 }
	{ slt r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
	{ slt r15, r16, r17 ; adds r5, r6, r7 }
	{ slt r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
	{ slt r15, r16, r17 ; bytex r5, r6 ; lw r25, r26 }
	{ slt r15, r16, r17 ; ctz r5, r6 ; lh r25, r26 }
	{ slt r15, r16, r17 ; info 19 ; lb_u r25, r26 }
	{ slt r15, r16, r17 ; lb r25, r26 ; clz r5, r6 }
	{ slt r15, r16, r17 ; lb r25, r26 ; nor r5, r6, r7 }
	{ slt r15, r16, r17 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ slt r15, r16, r17 ; lb_u r25, r26 ; info 19 }
	{ slt r15, r16, r17 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ slt r15, r16, r17 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ slt r15, r16, r17 ; lh r25, r26 ; movei r5, 5 }
	{ slt r15, r16, r17 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ slt r15, r16, r17 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ slt r15, r16, r17 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ slt r15, r16, r17 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ slt r15, r16, r17 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ slt r15, r16, r17 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ slt r15, r16, r17 ; lw r25, r26 ; shli r5, r6, 5 }
	{ slt r15, r16, r17 ; maxh r5, r6, r7 }
	{ slt r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
	{ slt r15, r16, r17 ; moveli r5, 0x1234 }
	{ slt r15, r16, r17 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ slt r15, r16, r17 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ slt r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ slt r15, r16, r17 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ slt r15, r16, r17 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ slt r15, r16, r17 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ slt r15, r16, r17 ; nop ; lh r25, r26 }
	{ slt r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
	{ slt r15, r16, r17 ; packhs r5, r6, r7 }
	{ slt r15, r16, r17 ; prefetch r25 ; fnop }
	{ slt r15, r16, r17 ; prefetch r25 ; ori r5, r6, 5 }
	{ slt r15, r16, r17 ; prefetch r25 ; sra r5, r6, r7 }
	{ slt r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ slt r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ slt r15, r16, r17 ; sadah r5, r6, r7 }
	{ slt r15, r16, r17 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ slt r15, r16, r17 ; sb r25, r26 ; seq r5, r6, r7 }
	{ slt r15, r16, r17 ; sb r25, r26 ; xor r5, r6, r7 }
	{ slt r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ slt r15, r16, r17 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ slt r15, r16, r17 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ slt r15, r16, r17 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ slt r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
	{ slt r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ slt r15, r16, r17 ; slt r5, r6, r7 }
	{ slt r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
	{ slt r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ slt r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ slt r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
	{ slt r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ slt r15, r16, r17 ; sw r25, r26 ; clz r5, r6 }
	{ slt r15, r16, r17 ; sw r25, r26 ; nor r5, r6, r7 }
	{ slt r15, r16, r17 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ slt r15, r16, r17 ; tblidxb0 r5, r6 }
	{ slt r15, r16, r17 ; tblidxb2 r5, r6 }
	{ slt r15, r16, r17 ; xor r5, r6, r7 }
	{ slt r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
	{ slt r5, r6, r7 ; and r15, r16, r17 }
	{ slt r5, r6, r7 ; fnop ; prefetch r25 }
	{ slt r5, r6, r7 ; info 19 ; lw r25, r26 }
	{ slt r5, r6, r7 ; lb r25, r26 ; and r15, r16, r17 }
	{ slt r5, r6, r7 ; lb r25, r26 ; shl r15, r16, r17 }
	{ slt r5, r6, r7 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ slt r5, r6, r7 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ slt r5, r6, r7 ; lh r25, r26 ; and r15, r16, r17 }
	{ slt r5, r6, r7 ; lh r25, r26 ; shl r15, r16, r17 }
	{ slt r5, r6, r7 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ slt r5, r6, r7 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ slt r5, r6, r7 ; lw r25, r26 ; addi r15, r16, 5 }
	{ slt r5, r6, r7 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ slt r5, r6, r7 ; maxb_u r15, r16, r17 }
	{ slt r5, r6, r7 ; mnz r15, r16, r17 }
	{ slt r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
	{ slt r5, r6, r7 ; nop ; lh r25, r26 }
	{ slt r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
	{ slt r5, r6, r7 ; packhs r15, r16, r17 }
	{ slt r5, r6, r7 ; prefetch r25 ; s1a r15, r16, r17 }
	{ slt r5, r6, r7 ; prefetch r25 }
	{ slt r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
	{ slt r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ slt r5, r6, r7 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ slt r5, r6, r7 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ slt r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
	{ slt r5, r6, r7 ; sh r25, r26 ; andi r15, r16, 5 }
	{ slt r5, r6, r7 ; sh r25, r26 ; shli r15, r16, 5 }
	{ slt r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
	{ slt r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
	{ slt r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
	{ slt r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ slt r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ slt r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
	{ slt r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
	{ slt r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
	{ slt r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
	{ slt r5, r6, r7 ; sw r25, r26 ; nor r15, r16, r17 }
	{ slt r5, r6, r7 ; sw r25, r26 ; sne r15, r16, r17 }
	{ slt_u r15, r16, r17 ; add r5, r6, r7 ; lb r25, r26 }
	{ slt_u r15, r16, r17 ; addi r5, r6, 5 ; sb r25, r26 }
	{ slt_u r15, r16, r17 ; and r5, r6, r7 }
	{ slt_u r15, r16, r17 ; bitx r5, r6 ; sb r25, r26 }
	{ slt_u r15, r16, r17 ; clz r5, r6 ; sb r25, r26 }
	{ slt_u r15, r16, r17 ; fnop ; lh_u r25, r26 }
	{ slt_u r15, r16, r17 ; intlb r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lb r25, r26 ; mulll_ss r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lb r25, r26 ; shli r5, r6, 5 }
	{ slt_u r15, r16, r17 ; lb_u r25, r26 ; addi r5, r6, 5 }
	{ slt_u r15, r16, r17 ; lb_u r25, r26 ; mullla_uu r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lb_u r25, r26 ; slt r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lh r25, r26 ; bitx r5, r6 }
	{ slt_u r15, r16, r17 ; lh r25, r26 ; mz r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lh r25, r26 ; slte_u r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lh_u r25, r26 ; ctz r5, r6 }
	{ slt_u r15, r16, r17 ; lh_u r25, r26 ; or r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lh_u r25, r26 ; sne r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lw r25, r26 ; mnz r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lw r25, r26 ; rl r5, r6, r7 }
	{ slt_u r15, r16, r17 ; lw r25, r26 ; sub r5, r6, r7 }
	{ slt_u r15, r16, r17 ; mnz r5, r6, r7 ; lw r25, r26 }
	{ slt_u r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
	{ slt_u r15, r16, r17 ; mulhh_su r5, r6, r7 }
	{ slt_u r15, r16, r17 ; mulhha_ss r5, r6, r7 }
	{ slt_u r15, r16, r17 ; mulhla_uu r5, r6, r7 }
	{ slt_u r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ slt_u r15, r16, r17 ; mullla_ss r5, r6, r7 ; sw r25, r26 }
	{ slt_u r15, r16, r17 ; mvnz r5, r6, r7 ; sb r25, r26 }
	{ slt_u r15, r16, r17 ; mz r5, r6, r7 ; sb r25, r26 }
	{ slt_u r15, r16, r17 ; nor r5, r6, r7 ; lw r25, r26 }
	{ slt_u r15, r16, r17 ; ori r5, r6, 5 ; lw r25, r26 }
	{ slt_u r15, r16, r17 ; prefetch r25 ; add r5, r6, r7 }
	{ slt_u r15, r16, r17 ; prefetch r25 ; mullla_ss r5, r6, r7 }
	{ slt_u r15, r16, r17 ; prefetch r25 ; shri r5, r6, 5 }
	{ slt_u r15, r16, r17 ; rl r5, r6, r7 ; lh_u r25, r26 }
	{ slt_u r15, r16, r17 ; s1a r5, r6, r7 ; lh_u r25, r26 }
	{ slt_u r15, r16, r17 ; s3a r5, r6, r7 ; lh_u r25, r26 }
	{ slt_u r15, r16, r17 ; sb r25, r26 ; ctz r5, r6 }
	{ slt_u r15, r16, r17 ; sb r25, r26 ; or r5, r6, r7 }
	{ slt_u r15, r16, r17 ; sb r25, r26 ; sne r5, r6, r7 }
	{ slt_u r15, r16, r17 ; seqb r5, r6, r7 }
	{ slt_u r15, r16, r17 ; sh r25, r26 ; clz r5, r6 }
	{ slt_u r15, r16, r17 ; sh r25, r26 ; nor r5, r6, r7 }
	{ slt_u r15, r16, r17 ; sh r25, r26 ; slti_u r5, r6, 5 }
	{ slt_u r15, r16, r17 ; shl r5, r6, r7 }
	{ slt_u r15, r16, r17 ; shr r5, r6, r7 ; prefetch r25 }
	{ slt_u r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
	{ slt_u r15, r16, r17 ; sltb_u r5, r6, r7 }
	{ slt_u r15, r16, r17 ; slte_u r5, r6, r7 }
	{ slt_u r15, r16, r17 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
	{ slt_u r15, r16, r17 ; sne r5, r6, r7 }
	{ slt_u r15, r16, r17 ; srai r5, r6, 5 ; prefetch r25 }
	{ slt_u r15, r16, r17 ; subhs r5, r6, r7 }
	{ slt_u r15, r16, r17 ; sw r25, r26 ; mulll_ss r5, r6, r7 }
	{ slt_u r15, r16, r17 ; sw r25, r26 ; shli r5, r6, 5 }
	{ slt_u r15, r16, r17 ; tblidxb0 r5, r6 ; lb_u r25, r26 }
	{ slt_u r15, r16, r17 ; tblidxb2 r5, r6 ; lb_u r25, r26 }
	{ slt_u r15, r16, r17 ; xor r5, r6, r7 ; lb_u r25, r26 }
	{ slt_u r5, r6, r7 ; addb r15, r16, r17 }
	{ slt_u r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
	{ slt_u r5, r6, r7 ; dtlbpr r15 }
	{ slt_u r5, r6, r7 ; ill ; sb r25, r26 }
	{ slt_u r5, r6, r7 ; iret }
	{ slt_u r5, r6, r7 ; lb r25, r26 ; ori r15, r16, 5 }
	{ slt_u r5, r6, r7 ; lb r25, r26 ; srai r15, r16, 5 }
	{ slt_u r5, r6, r7 ; lb_u r25, r26 ; rl r15, r16, r17 }
	{ slt_u r5, r6, r7 ; lb_u r25, r26 ; sub r15, r16, r17 }
	{ slt_u r5, r6, r7 ; lh r25, r26 ; ori r15, r16, 5 }
	{ slt_u r5, r6, r7 ; lh r25, r26 ; srai r15, r16, 5 }
	{ slt_u r5, r6, r7 ; lh_u r25, r26 ; rl r15, r16, r17 }
	{ slt_u r5, r6, r7 ; lh_u r25, r26 ; sub r15, r16, r17 }
	{ slt_u r5, r6, r7 ; lw r25, r26 ; or r15, r16, r17 }
	{ slt_u r5, r6, r7 ; lw r25, r26 ; sra r15, r16, r17 }
	{ slt_u r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
	{ slt_u r5, r6, r7 ; move r15, r16 }
	{ slt_u r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
	{ slt_u r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
	{ slt_u r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
	{ slt_u r5, r6, r7 ; prefetch r25 ; movei r15, 5 }
	{ slt_u r5, r6, r7 ; prefetch r25 ; slte_u r15, r16, r17 }
	{ slt_u r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
	{ slt_u r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
	{ slt_u r5, r6, r7 ; sb r15, r16 }
	{ slt_u r5, r6, r7 ; sb r25, r26 ; s3a r15, r16, r17 }
	{ slt_u r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
	{ slt_u r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
	{ slt_u r5, r6, r7 ; sh r25, r26 ; rl r15, r16, r17 }
	{ slt_u r5, r6, r7 ; sh r25, r26 ; sub r15, r16, r17 }
	{ slt_u r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
	{ slt_u r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
	{ slt_u r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
	{ slt_u r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
	{ slt_u r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
	{ slt_u r5, r6, r7 ; sltib r15, r16, 5 }
	{ slt_u r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
	{ slt_u r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
	{ slt_u r5, r6, r7 ; sw r25, r26 ; fnop }
	{ slt_u r5, r6, r7 ; sw r25, r26 ; shr r15, r16, r17 }
	{ slt_u r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
	{ sltb r15, r16, r17 ; adiffh r5, r6, r7 }
	{ sltb r15, r16, r17 ; maxb_u r5, r6, r7 }
	{ sltb r15, r16, r17 ; mulhha_su r5, r6, r7 }
	{ sltb r15, r16, r17 ; mvz r5, r6, r7 }
	{ sltb r15, r16, r17 ; sadah_u r5, r6, r7 }
	{ sltb r15, r16, r17 ; shrib r5, r6, 5 }
	{ sltb r15, r16, r17 ; sne r5, r6, r7 }
	{ sltb r15, r16, r17 ; xori r5, r6, 5 }
	{ sltb r5, r6, r7 ; ill }
	{ sltb r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ sltb r5, r6, r7 ; move r15, r16 }
	{ sltb r5, r6, r7 ; s1a r15, r16, r17 }
	{ sltb r5, r6, r7 ; shrb r15, r16, r17 }
	{ sltb r5, r6, r7 ; sltib_u r15, r16, 5 }
	{ sltb r5, r6, r7 ; tns r15, r16 }
	{ sltb_u r15, r16, r17 ; avgb_u r5, r6, r7 }
	{ sltb_u r15, r16, r17 ; minb_u r5, r6, r7 }
	{ sltb_u r15, r16, r17 ; mulhl_su r5, r6, r7 }
	{ sltb_u r15, r16, r17 ; nop }
	{ sltb_u r15, r16, r17 ; seq r5, r6, r7 }
	{ sltb_u r15, r16, r17 ; sltb r5, r6, r7 }
	{ sltb_u r15, r16, r17 ; srab r5, r6, r7 }
	{ sltb_u r5, r6, r7 ; addh r15, r16, r17 }
	{ sltb_u r5, r6, r7 ; inthh r15, r16, r17 }
	{ sltb_u r5, r6, r7 ; lwadd r15, r16, 5 }
	{ sltb_u r5, r6, r7 ; mtspr 0x5, r16 }
	{ sltb_u r5, r6, r7 ; sbadd r15, r16, 5 }
	{ sltb_u r5, r6, r7 ; shrih r15, r16, 5 }
	{ sltb_u r5, r6, r7 ; sneb r15, r16, r17 }
	{ slte r15, r16, r17 ; add r5, r6, r7 ; lb r25, r26 }
	{ slte r15, r16, r17 ; addi r5, r6, 5 ; sb r25, r26 }
	{ slte r15, r16, r17 ; and r5, r6, r7 }
	{ slte r15, r16, r17 ; bitx r5, r6 ; sb r25, r26 }
	{ slte r15, r16, r17 ; clz r5, r6 ; sb r25, r26 }
	{ slte r15, r16, r17 ; fnop ; lh_u r25, r26 }
	{ slte r15, r16, r17 ; intlb r5, r6, r7 }
	{ slte r15, r16, r17 ; lb r25, r26 ; mulll_ss r5, r6, r7 }
	{ slte r15, r16, r17 ; lb r25, r26 ; shli r5, r6, 5 }
	{ slte r15, r16, r17 ; lb_u r25, r26 ; addi r5, r6, 5 }
	{ slte r15, r16, r17 ; lb_u r25, r26 ; mullla_uu r5, r6, r7 }
	{ slte r15, r16, r17 ; lb_u r25, r26 ; slt r5, r6, r7 }
	{ slte r15, r16, r17 ; lh r25, r26 ; bitx r5, r6 }
	{ slte r15, r16, r17 ; lh r25, r26 ; mz r5, r6, r7 }
	{ slte r15, r16, r17 ; lh r25, r26 ; slte_u r5, r6, r7 }
	{ slte r15, r16, r17 ; lh_u r25, r26 ; ctz r5, r6 }
	{ slte r15, r16, r17 ; lh_u r25, r26 ; or r5, r6, r7 }
	{ slte r15, r16, r17 ; lh_u r25, r26 ; sne r5, r6, r7 }
	{ slte r15, r16, r17 ; lw r25, r26 ; mnz r5, r6, r7 }
	{ slte r15, r16, r17 ; lw r25, r26 ; rl r5, r6, r7 }
	{ slte r15, r16, r17 ; lw r25, r26 ; sub r5, r6, r7 }
	{ slte r15, r16, r17 ; mnz r5, r6, r7 ; lw r25, r26 }
	{ slte r15, r16, r17 ; movei r5, 5 ; lh r25, r26 }
	{ slte r15, r16, r17 ; mulhh_su r5, r6, r7 }
	{ slte r15, r16, r17 ; mulhha_ss r5, r6, r7 }
	{ slte r15, r16, r17 ; mulhla_uu r5, r6, r7 }
	{ slte r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ slte r15, r16, r17 ; mullla_ss r5, r6, r7 ; sw r25, r26 }
	{ slte r15, r16, r17 ; mvnz r5, r6, r7 ; sb r25, r26 }
	{ slte r15, r16, r17 ; mz r5, r6, r7 ; sb r25, r26 }
	{ slte r15, r16, r17 ; nor r5, r6, r7 ; lw r25, r26 }
	{ slte r15, r16, r17 ; ori r5, r6, 5 ; lw r25, r26 }
	{ slte r15, r16, r17 ; prefetch r25 ; add r5, r6, r7 }
	{ slte r15, r16, r17 ; prefetch r25 ; mullla_ss r5, r6, r7 }
	{ slte r15, r16, r17 ; prefetch r25 ; shri r5, r6, 5 }
	{ slte r15, r16, r17 ; rl r5, r6, r7 ; lh_u r25, r26 }
	{ slte r15, r16, r17 ; s1a r5, r6, r7 ; lh_u r25, r26 }
	{ slte r15, r16, r17 ; s3a r5, r6, r7 ; lh_u r25, r26 }
	{ slte r15, r16, r17 ; sb r25, r26 ; ctz r5, r6 }
	{ slte r15, r16, r17 ; sb r25, r26 ; or r5, r6, r7 }
	{ slte r15, r16, r17 ; sb r25, r26 ; sne r5, r6, r7 }
	{ slte r15, r16, r17 ; seqb r5, r6, r7 }
	{ slte r15, r16, r17 ; sh r25, r26 ; clz r5, r6 }
	{ slte r15, r16, r17 ; sh r25, r26 ; nor r5, r6, r7 }
	{ slte r15, r16, r17 ; sh r25, r26 ; slti_u r5, r6, 5 }
	{ slte r15, r16, r17 ; shl r5, r6, r7 }
	{ slte r15, r16, r17 ; shr r5, r6, r7 ; prefetch r25 }
	{ slte r15, r16, r17 ; slt r5, r6, r7 ; lb_u r25, r26 }
	{ slte r15, r16, r17 ; sltb_u r5, r6, r7 }
	{ slte r15, r16, r17 ; slte_u r5, r6, r7 }
	{ slte r15, r16, r17 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
	{ slte r15, r16, r17 ; sne r5, r6, r7 }
	{ slte r15, r16, r17 ; srai r5, r6, 5 ; prefetch r25 }
	{ slte r15, r16, r17 ; subhs r5, r6, r7 }
	{ slte r15, r16, r17 ; sw r25, r26 ; mulll_ss r5, r6, r7 }
	{ slte r15, r16, r17 ; sw r25, r26 ; shli r5, r6, 5 }
	{ slte r15, r16, r17 ; tblidxb0 r5, r6 ; lb_u r25, r26 }
	{ slte r15, r16, r17 ; tblidxb2 r5, r6 ; lb_u r25, r26 }
	{ slte r15, r16, r17 ; xor r5, r6, r7 ; lb_u r25, r26 }
	{ slte r5, r6, r7 ; addb r15, r16, r17 }
	{ slte r5, r6, r7 ; and r15, r16, r17 ; lb_u r25, r26 }
	{ slte r5, r6, r7 ; dtlbpr r15 }
	{ slte r5, r6, r7 ; ill ; sb r25, r26 }
	{ slte r5, r6, r7 ; iret }
	{ slte r5, r6, r7 ; lb r25, r26 ; ori r15, r16, 5 }
	{ slte r5, r6, r7 ; lb r25, r26 ; srai r15, r16, 5 }
	{ slte r5, r6, r7 ; lb_u r25, r26 ; rl r15, r16, r17 }
	{ slte r5, r6, r7 ; lb_u r25, r26 ; sub r15, r16, r17 }
	{ slte r5, r6, r7 ; lh r25, r26 ; ori r15, r16, 5 }
	{ slte r5, r6, r7 ; lh r25, r26 ; srai r15, r16, 5 }
	{ slte r5, r6, r7 ; lh_u r25, r26 ; rl r15, r16, r17 }
	{ slte r5, r6, r7 ; lh_u r25, r26 ; sub r15, r16, r17 }
	{ slte r5, r6, r7 ; lw r25, r26 ; or r15, r16, r17 }
	{ slte r5, r6, r7 ; lw r25, r26 ; sra r15, r16, r17 }
	{ slte r5, r6, r7 ; mnz r15, r16, r17 ; lb_u r25, r26 }
	{ slte r5, r6, r7 ; move r15, r16 }
	{ slte r5, r6, r7 ; mz r15, r16, r17 ; sb r25, r26 }
	{ slte r5, r6, r7 ; nor r15, r16, r17 ; lw r25, r26 }
	{ slte r5, r6, r7 ; ori r15, r16, 5 ; lw r25, r26 }
	{ slte r5, r6, r7 ; prefetch r25 ; movei r15, 5 }
	{ slte r5, r6, r7 ; prefetch r25 ; slte_u r15, r16, r17 }
	{ slte r5, r6, r7 ; rli r15, r16, 5 ; lb r25, r26 }
	{ slte r5, r6, r7 ; s2a r15, r16, r17 ; lb r25, r26 }
	{ slte r5, r6, r7 ; sb r15, r16 }
	{ slte r5, r6, r7 ; sb r25, r26 ; s3a r15, r16, r17 }
	{ slte r5, r6, r7 ; seq r15, r16, r17 ; lb r25, r26 }
	{ slte r5, r6, r7 ; seqi r15, r16, 5 ; sw r25, r26 }
	{ slte r5, r6, r7 ; sh r25, r26 ; rl r15, r16, r17 }
	{ slte r5, r6, r7 ; sh r25, r26 ; sub r15, r16, r17 }
	{ slte r5, r6, r7 ; shli r15, r16, 5 ; lw r25, r26 }
	{ slte r5, r6, r7 ; shri r15, r16, 5 ; lb r25, r26 }
	{ slte r5, r6, r7 ; slt r15, r16, r17 ; sw r25, r26 }
	{ slte r5, r6, r7 ; slte r15, r16, r17 ; sb r25, r26 }
	{ slte r5, r6, r7 ; slti r15, r16, 5 ; lb r25, r26 }
	{ slte r5, r6, r7 ; sltib r15, r16, 5 }
	{ slte r5, r6, r7 ; sra r15, r16, r17 ; lw r25, r26 }
	{ slte r5, r6, r7 ; sub r15, r16, r17 ; lb r25, r26 }
	{ slte r5, r6, r7 ; sw r25, r26 ; fnop }
	{ slte r5, r6, r7 ; sw r25, r26 ; shr r15, r16, r17 }
	{ slte r5, r6, r7 ; xor r15, r16, r17 ; lh_u r25, r26 }
	{ slte_u r15, r16, r17 ; addh r5, r6, r7 }
	{ slte_u r15, r16, r17 ; and r5, r6, r7 ; lb_u r25, r26 }
	{ slte_u r15, r16, r17 ; avgb_u r5, r6, r7 }
	{ slte_u r15, r16, r17 ; bytex r5, r6 ; sw r25, r26 }
	{ slte_u r15, r16, r17 ; ctz r5, r6 ; sb r25, r26 }
	{ slte_u r15, r16, r17 ; info 19 ; prefetch r25 }
	{ slte_u r15, r16, r17 ; lb r25, r26 ; mnz r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lb r25, r26 ; rl r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lb r25, r26 ; sub r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lb_u r25, r26 ; mulhh_ss r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lb_u r25, r26 ; s2a r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lb_u r25, r26 ; tblidxb2 r5, r6 }
	{ slte_u r15, r16, r17 ; lh r25, r26 ; mulhha_uu r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lh r25, r26 ; seqi r5, r6, 5 }
	{ slte_u r15, r16, r17 ; lh r25, r26 }
	{ slte_u r15, r16, r17 ; lh_u r25, r26 ; mulll_uu r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lh_u r25, r26 ; shr r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lw r25, r26 ; and r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lw r25, r26 ; mvnz r5, r6, r7 }
	{ slte_u r15, r16, r17 ; lw r25, r26 ; slt_u r5, r6, r7 }
	{ slte_u r15, r16, r17 ; minh r5, r6, r7 }
	{ slte_u r15, r16, r17 ; move r5, r6 ; lw r25, r26 }
	{ slte_u r15, r16, r17 ; mulhh_ss r5, r6, r7 ; lh r25, r26 }
	{ slte_u r15, r16, r17 ; mulhha_ss r5, r6, r7 ; lb_u r25, r26 }
	{ slte_u r15, r16, r17 ; mulhhsa_uu r5, r6, r7 }
	{ slte_u r15, r16, r17 ; mulll_ss r5, r6, r7 ; lb_u r25, r26 }
	{ slte_u r15, r16, r17 ; mullla_ss r5, r6, r7 ; lb r25, r26 }
	{ slte_u r15, r16, r17 ; mullla_uu r5, r6, r7 }
	{ slte_u r15, r16, r17 ; mvz r5, r6, r7 ; sw r25, r26 }
	{ slte_u r15, r16, r17 ; nop ; sb r25, r26 }
	{ slte_u r15, r16, r17 ; or r5, r6, r7 ; sb r25, r26 }
	{ slte_u r15, r16, r17 ; pcnt r5, r6 ; lh r25, r26 }
	{ slte_u r15, r16, r17 ; prefetch r25 ; movei r5, 5 }
	{ slte_u r15, r16, r17 ; prefetch r25 ; s1a r5, r6, r7 }
	{ slte_u r15, r16, r17 ; prefetch r25 ; tblidxb1 r5, r6 }
	{ slte_u r15, r16, r17 ; rli r5, r6, 5 ; prefetch r25 }
	{ slte_u r15, r16, r17 ; s2a r5, r6, r7 ; prefetch r25 }
	{ slte_u r15, r16, r17 ; sadh_u r5, r6, r7 }
	{ slte_u r15, r16, r17 ; sb r25, r26 ; mulll_uu r5, r6, r7 }
	{ slte_u r15, r16, r17 ; sb r25, r26 ; shr r5, r6, r7 }
	{ slte_u r15, r16, r17 ; seq r5, r6, r7 ; lh r25, r26 }
	{ slte_u r15, r16, r17 ; seqib r5, r6, 5 }
	{ slte_u r15, r16, r17 ; sh r25, r26 ; mulll_ss r5, r6, r7 }
	{ slte_u r15, r16, r17 ; sh r25, r26 ; shli r5, r6, 5 }
	{ slte_u r15, r16, r17 ; shl r5, r6, r7 ; lb_u r25, r26 }
	{ slte_u r15, r16, r17 ; shli r5, r6, 5 }
	{ slte_u r15, r16, r17 ; shri r5, r6, 5 ; prefetch r25 }
	{ slte_u r15, r16, r17 ; slt_u r5, r6, r7 ; lh_u r25, r26 }
	{ slte_u r15, r16, r17 ; slte_u r5, r6, r7 ; lb_u r25, r26 }
	{ slte_u r15, r16, r17 ; slti r5, r6, 5 ; prefetch r25 }
	{ slte_u r15, r16, r17 ; sne r5, r6, r7 ; lb_u r25, r26 }
	{ slte_u r15, r16, r17 ; sra r5, r6, r7 }
	{ slte_u r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
	{ slte_u r15, r16, r17 ; sw r25, r26 ; mnz r5, r6, r7 }
	{ slte_u r15, r16, r17 ; sw r25, r26 ; rl r5, r6, r7 }
	{ slte_u r15, r16, r17 ; sw r25, r26 ; sub r5, r6, r7 }
	{ slte_u r15, r16, r17 ; tblidxb1 r5, r6 ; lh_u r25, r26 }
	{ slte_u r15, r16, r17 ; tblidxb3 r5, r6 ; lh_u r25, r26 }
	{ slte_u r5, r6, r7 ; add r15, r16, r17 ; lh r25, r26 }
	{ slte_u r5, r6, r7 ; addi r15, r16, 5 ; sw r25, r26 }
	{ slte_u r5, r6, r7 ; andi r15, r16, 5 ; lh_u r25, r26 }
	{ slte_u r5, r6, r7 ; fnop }
	{ slte_u r5, r6, r7 ; info 19 ; sw r25, r26 }
	{ slte_u r5, r6, r7 ; lb r25, r26 ; info 19 }
	{ slte_u r5, r6, r7 ; lb r25, r26 ; slt r15, r16, r17 }
	{ slte_u r5, r6, r7 ; lb_u r25, r26 ; mnz r15, r16, r17 }
	{ slte_u r5, r6, r7 ; lb_u r25, r26 ; slt_u r15, r16, r17 }
	{ slte_u r5, r6, r7 ; lh r25, r26 ; info 19 }
	{ slte_u r5, r6, r7 ; lh r25, r26 ; slt r15, r16, r17 }
	{ slte_u r5, r6, r7 ; lh_u r25, r26 ; mnz r15, r16, r17 }
	{ slte_u r5, r6, r7 ; lh_u r25, r26 ; slt_u r15, r16, r17 }
	{ slte_u r5, r6, r7 ; lw r25, r26 ; ill }
	{ slte_u r5, r6, r7 ; lw r25, r26 ; shri r15, r16, 5 }
	{ slte_u r5, r6, r7 ; mf }
	{ slte_u r5, r6, r7 ; move r15, r16 ; lb_u r25, r26 }
	{ slte_u r5, r6, r7 ; movelis r15, 0x1234 }
	{ slte_u r5, r6, r7 ; nop ; sb r25, r26 }
	{ slte_u r5, r6, r7 ; or r15, r16, r17 ; sb r25, r26 }
	{ slte_u r5, r6, r7 ; prefetch r25 ; addi r15, r16, 5 }
	{ slte_u r5, r6, r7 ; prefetch r25 ; seqi r15, r16, 5 }
	{ slte_u r5, r6, r7 ; rl r15, r16, r17 ; lh r25, r26 }
	{ slte_u r5, r6, r7 ; s1a r15, r16, r17 ; lh r25, r26 }
	{ slte_u r5, r6, r7 ; s3a r15, r16, r17 ; lh r25, r26 }
	{ slte_u r5, r6, r7 ; sb r25, r26 ; nop }
	{ slte_u r5, r6, r7 ; sb r25, r26 ; slti_u r15, r16, 5 }
	{ slte_u r5, r6, r7 ; seqi r15, r16, 5 ; lb r25, r26 }
	{ slte_u r5, r6, r7 ; sh r25, r26 ; mnz r15, r16, r17 }
	{ slte_u r5, r6, r7 ; sh r25, r26 ; slt_u r15, r16, r17 }
	{ slte_u r5, r6, r7 ; shl r15, r16, r17 ; sw r25, r26 }
	{ slte_u r5, r6, r7 ; shr r15, r16, r17 ; lw r25, r26 }
	{ slte_u r5, r6, r7 ; slt r15, r16, r17 ; lb r25, r26 }
	{ slte_u r5, r6, r7 ; sltb r15, r16, r17 }
	{ slte_u r5, r6, r7 ; slte_u r15, r16, r17 ; sw r25, r26 }
	{ slte_u r5, r6, r7 ; slti_u r15, r16, 5 ; lh r25, r26 }
	{ slte_u r5, r6, r7 ; sne r15, r16, r17 ; sw r25, r26 }
	{ slte_u r5, r6, r7 ; srai r15, r16, 5 ; lw r25, r26 }
	{ slte_u r5, r6, r7 ; subh r15, r16, r17 }
	{ slte_u r5, r6, r7 ; sw r25, r26 ; rli r15, r16, 5 }
	{ slte_u r5, r6, r7 ; sw r25, r26 ; xor r15, r16, r17 }
	{ slteb r15, r16, r17 ; addhs r5, r6, r7 }
	{ slteb r15, r16, r17 ; dword_align r5, r6, r7 }
	{ slteb r15, r16, r17 ; move r5, r6 }
	{ slteb r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ slteb r15, r16, r17 ; pcnt r5, r6 }
	{ slteb r15, r16, r17 ; shlh r5, r6, r7 }
	{ slteb r15, r16, r17 ; slth r5, r6, r7 }
	{ slteb r15, r16, r17 ; subh r5, r6, r7 }
	{ slteb r5, r6, r7 ; and r15, r16, r17 }
	{ slteb r5, r6, r7 ; jrp r15 }
	{ slteb r5, r6, r7 ; minb_u r15, r16, r17 }
	{ slteb r5, r6, r7 ; packbs_u r15, r16, r17 }
	{ slteb r5, r6, r7 ; shadd r15, r16, 5 }
	{ slteb r5, r6, r7 ; slteb_u r15, r16, r17 }
	{ slteb r5, r6, r7 ; sub r15, r16, r17 }
	{ slteb_u r15, r16, r17 ; addli r5, r6, 0x1234 }
	{ slteb_u r15, r16, r17 ; inthb r5, r6, r7 }
	{ slteb_u r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ slteb_u r15, r16, r17 ; mullla_su r5, r6, r7 }
	{ slteb_u r15, r16, r17 ; s2a r5, r6, r7 }
	{ slteb_u r15, r16, r17 ; shr r5, r6, r7 }
	{ slteb_u r15, r16, r17 ; sltib r5, r6, 5 }
	{ slteb_u r15, r16, r17 ; tblidxb1 r5, r6 }
	{ slteb_u r5, r6, r7 ; finv r15 }
	{ slteb_u r5, r6, r7 ; lbadd_u r15, r16, 5 }
	{ slteb_u r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
	{ slteb_u r5, r6, r7 ; prefetch r15 }
	{ slteb_u r5, r6, r7 ; shli r15, r16, 5 }
	{ slteb_u r5, r6, r7 ; slth_u r15, r16, r17 }
	{ slteb_u r5, r6, r7 ; subhs r15, r16, r17 }
	{ slteh r15, r16, r17 ; adiffh r5, r6, r7 }
	{ slteh r15, r16, r17 ; maxb_u r5, r6, r7 }
	{ slteh r15, r16, r17 ; mulhha_su r5, r6, r7 }
	{ slteh r15, r16, r17 ; mvz r5, r6, r7 }
	{ slteh r15, r16, r17 ; sadah_u r5, r6, r7 }
	{ slteh r15, r16, r17 ; shrib r5, r6, 5 }
	{ slteh r15, r16, r17 ; sne r5, r6, r7 }
	{ slteh r15, r16, r17 ; xori r5, r6, 5 }
	{ slteh r5, r6, r7 ; ill }
	{ slteh r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ slteh r5, r6, r7 ; move r15, r16 }
	{ slteh r5, r6, r7 ; s1a r15, r16, r17 }
	{ slteh r5, r6, r7 ; shrb r15, r16, r17 }
	{ slteh r5, r6, r7 ; sltib_u r15, r16, 5 }
	{ slteh r5, r6, r7 ; tns r15, r16 }
	{ slteh_u r15, r16, r17 ; avgb_u r5, r6, r7 }
	{ slteh_u r15, r16, r17 ; minb_u r5, r6, r7 }
	{ slteh_u r15, r16, r17 ; mulhl_su r5, r6, r7 }
	{ slteh_u r15, r16, r17 ; nop }
	{ slteh_u r15, r16, r17 ; seq r5, r6, r7 }
	{ slteh_u r15, r16, r17 ; sltb r5, r6, r7 }
	{ slteh_u r15, r16, r17 ; srab r5, r6, r7 }
	{ slteh_u r5, r6, r7 ; addh r15, r16, r17 }
	{ slteh_u r5, r6, r7 ; inthh r15, r16, r17 }
	{ slteh_u r5, r6, r7 ; lwadd r15, r16, 5 }
	{ slteh_u r5, r6, r7 ; mtspr 0x5, r16 }
	{ slteh_u r5, r6, r7 ; sbadd r15, r16, 5 }
	{ slteh_u r5, r6, r7 ; shrih r15, r16, 5 }
	{ slteh_u r5, r6, r7 ; sneb r15, r16, r17 }
	{ slth r15, r16, r17 ; add r5, r6, r7 }
	{ slth r15, r16, r17 ; clz r5, r6 }
	{ slth r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
	{ slth r15, r16, r17 ; mulhla_su r5, r6, r7 }
	{ slth r15, r16, r17 ; packbs_u r5, r6, r7 }
	{ slth r15, r16, r17 ; seqib r5, r6, 5 }
	{ slth r15, r16, r17 ; slteb r5, r6, r7 }
	{ slth r15, r16, r17 ; sraih r5, r6, 5 }
	{ slth r5, r6, r7 ; addih r15, r16, 5 }
	{ slth r5, r6, r7 ; iret }
	{ slth r5, r6, r7 ; maxib_u r15, r16, 5 }
	{ slth r5, r6, r7 ; nop }
	{ slth r5, r6, r7 ; seqi r15, r16, 5 }
	{ slth r5, r6, r7 ; sltb_u r15, r16, r17 }
	{ slth r5, r6, r7 ; srah r15, r16, r17 }
	{ slth_u r15, r16, r17 ; addhs r5, r6, r7 }
	{ slth_u r15, r16, r17 ; dword_align r5, r6, r7 }
	{ slth_u r15, r16, r17 ; move r5, r6 }
	{ slth_u r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ slth_u r15, r16, r17 ; pcnt r5, r6 }
	{ slth_u r15, r16, r17 ; shlh r5, r6, r7 }
	{ slth_u r15, r16, r17 ; slth r5, r6, r7 }
	{ slth_u r15, r16, r17 ; subh r5, r6, r7 }
	{ slth_u r5, r6, r7 ; and r15, r16, r17 }
	{ slth_u r5, r6, r7 ; jrp r15 }
	{ slth_u r5, r6, r7 ; minb_u r15, r16, r17 }
	{ slth_u r5, r6, r7 ; packbs_u r15, r16, r17 }
	{ slth_u r5, r6, r7 ; shadd r15, r16, 5 }
	{ slth_u r5, r6, r7 ; slteb_u r15, r16, r17 }
	{ slth_u r5, r6, r7 ; sub r15, r16, r17 }
	{ slti r15, r16, 5 ; add r5, r6, r7 ; sw r25, r26 }
	{ slti r15, r16, 5 ; adds r5, r6, r7 }
	{ slti r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
	{ slti r15, r16, 5 ; bytex r5, r6 ; lw r25, r26 }
	{ slti r15, r16, 5 ; ctz r5, r6 ; lh r25, r26 }
	{ slti r15, r16, 5 ; info 19 ; lb_u r25, r26 }
	{ slti r15, r16, 5 ; lb r25, r26 ; clz r5, r6 }
	{ slti r15, r16, 5 ; lb r25, r26 ; nor r5, r6, r7 }
	{ slti r15, r16, 5 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ slti r15, r16, 5 ; lb_u r25, r26 ; info 19 }
	{ slti r15, r16, 5 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ slti r15, r16, 5 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ slti r15, r16, 5 ; lh r25, r26 ; movei r5, 5 }
	{ slti r15, r16, 5 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ slti r15, r16, 5 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ slti r15, r16, 5 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ slti r15, r16, 5 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ slti r15, r16, 5 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ slti r15, r16, 5 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ slti r15, r16, 5 ; lw r25, r26 ; shli r5, r6, 5 }
	{ slti r15, r16, 5 ; maxh r5, r6, r7 }
	{ slti r15, r16, 5 ; move r5, r6 ; lb r25, r26 }
	{ slti r15, r16, 5 ; moveli r5, 0x1234 }
	{ slti r15, r16, 5 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ slti r15, r16, 5 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ slti r15, r16, 5 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ slti r15, r16, 5 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ slti r15, r16, 5 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ slti r15, r16, 5 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ slti r15, r16, 5 ; nop ; lh r25, r26 }
	{ slti r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
	{ slti r15, r16, 5 ; packhs r5, r6, r7 }
	{ slti r15, r16, 5 ; prefetch r25 ; fnop }
	{ slti r15, r16, 5 ; prefetch r25 ; ori r5, r6, 5 }
	{ slti r15, r16, 5 ; prefetch r25 ; sra r5, r6, r7 }
	{ slti r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ slti r15, r16, 5 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ slti r15, r16, 5 ; sadah r5, r6, r7 }
	{ slti r15, r16, 5 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ slti r15, r16, 5 ; sb r25, r26 ; seq r5, r6, r7 }
	{ slti r15, r16, 5 ; sb r25, r26 ; xor r5, r6, r7 }
	{ slti r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ slti r15, r16, 5 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ slti r15, r16, 5 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ slti r15, r16, 5 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ slti r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
	{ slti r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ slti r15, r16, 5 ; slt r5, r6, r7 }
	{ slti r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
	{ slti r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ slti r15, r16, 5 ; sltib_u r5, r6, 5 }
	{ slti r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
	{ slti r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ slti r15, r16, 5 ; sw r25, r26 ; clz r5, r6 }
	{ slti r15, r16, 5 ; sw r25, r26 ; nor r5, r6, r7 }
	{ slti r15, r16, 5 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ slti r15, r16, 5 ; tblidxb0 r5, r6 }
	{ slti r15, r16, 5 ; tblidxb2 r5, r6 }
	{ slti r15, r16, 5 ; xor r5, r6, r7 }
	{ slti r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
	{ slti r5, r6, 5 ; and r15, r16, r17 }
	{ slti r5, r6, 5 ; fnop ; prefetch r25 }
	{ slti r5, r6, 5 ; info 19 ; lw r25, r26 }
	{ slti r5, r6, 5 ; lb r25, r26 ; and r15, r16, r17 }
	{ slti r5, r6, 5 ; lb r25, r26 ; shl r15, r16, r17 }
	{ slti r5, r6, 5 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ slti r5, r6, 5 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ slti r5, r6, 5 ; lh r25, r26 ; and r15, r16, r17 }
	{ slti r5, r6, 5 ; lh r25, r26 ; shl r15, r16, r17 }
	{ slti r5, r6, 5 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ slti r5, r6, 5 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ slti r5, r6, 5 ; lw r25, r26 ; addi r15, r16, 5 }
	{ slti r5, r6, 5 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ slti r5, r6, 5 ; maxb_u r15, r16, r17 }
	{ slti r5, r6, 5 ; mnz r15, r16, r17 }
	{ slti r5, r6, 5 ; movei r15, 5 ; sh r25, r26 }
	{ slti r5, r6, 5 ; nop ; lh r25, r26 }
	{ slti r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
	{ slti r5, r6, 5 ; packhs r15, r16, r17 }
	{ slti r5, r6, 5 ; prefetch r25 ; s1a r15, r16, r17 }
	{ slti r5, r6, 5 ; prefetch r25 }
	{ slti r5, r6, 5 ; rli r15, r16, 5 ; sw r25, r26 }
	{ slti r5, r6, 5 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ slti r5, r6, 5 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ slti r5, r6, 5 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ slti r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
	{ slti r5, r6, 5 ; sh r25, r26 ; andi r15, r16, 5 }
	{ slti r5, r6, 5 ; sh r25, r26 ; shli r15, r16, 5 }
	{ slti r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
	{ slti r5, r6, 5 ; shr r15, r16, r17 ; lb r25, r26 }
	{ slti r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
	{ slti r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ slti r5, r6, 5 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ slti r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
	{ slti r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
	{ slti r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
	{ slti r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
	{ slti r5, r6, 5 ; sw r25, r26 ; nor r15, r16, r17 }
	{ slti r5, r6, 5 ; sw r25, r26 ; sne r15, r16, r17 }
	{ slti_u r15, r16, 5 ; add r5, r6, r7 ; lb r25, r26 }
	{ slti_u r15, r16, 5 ; addi r5, r6, 5 ; sb r25, r26 }
	{ slti_u r15, r16, 5 ; and r5, r6, r7 }
	{ slti_u r15, r16, 5 ; bitx r5, r6 ; sb r25, r26 }
	{ slti_u r15, r16, 5 ; clz r5, r6 ; sb r25, r26 }
	{ slti_u r15, r16, 5 ; fnop ; lh_u r25, r26 }
	{ slti_u r15, r16, 5 ; intlb r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lb r25, r26 ; mulll_ss r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lb r25, r26 ; shli r5, r6, 5 }
	{ slti_u r15, r16, 5 ; lb_u r25, r26 ; addi r5, r6, 5 }
	{ slti_u r15, r16, 5 ; lb_u r25, r26 ; mullla_uu r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lb_u r25, r26 ; slt r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lh r25, r26 ; bitx r5, r6 }
	{ slti_u r15, r16, 5 ; lh r25, r26 ; mz r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lh r25, r26 ; slte_u r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lh_u r25, r26 ; ctz r5, r6 }
	{ slti_u r15, r16, 5 ; lh_u r25, r26 ; or r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lh_u r25, r26 ; sne r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lw r25, r26 ; mnz r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lw r25, r26 ; rl r5, r6, r7 }
	{ slti_u r15, r16, 5 ; lw r25, r26 ; sub r5, r6, r7 }
	{ slti_u r15, r16, 5 ; mnz r5, r6, r7 ; lw r25, r26 }
	{ slti_u r15, r16, 5 ; movei r5, 5 ; lh r25, r26 }
	{ slti_u r15, r16, 5 ; mulhh_su r5, r6, r7 }
	{ slti_u r15, r16, 5 ; mulhha_ss r5, r6, r7 }
	{ slti_u r15, r16, 5 ; mulhla_uu r5, r6, r7 }
	{ slti_u r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ slti_u r15, r16, 5 ; mullla_ss r5, r6, r7 ; sw r25, r26 }
	{ slti_u r15, r16, 5 ; mvnz r5, r6, r7 ; sb r25, r26 }
	{ slti_u r15, r16, 5 ; mz r5, r6, r7 ; sb r25, r26 }
	{ slti_u r15, r16, 5 ; nor r5, r6, r7 ; lw r25, r26 }
	{ slti_u r15, r16, 5 ; ori r5, r6, 5 ; lw r25, r26 }
	{ slti_u r15, r16, 5 ; prefetch r25 ; add r5, r6, r7 }
	{ slti_u r15, r16, 5 ; prefetch r25 ; mullla_ss r5, r6, r7 }
	{ slti_u r15, r16, 5 ; prefetch r25 ; shri r5, r6, 5 }
	{ slti_u r15, r16, 5 ; rl r5, r6, r7 ; lh_u r25, r26 }
	{ slti_u r15, r16, 5 ; s1a r5, r6, r7 ; lh_u r25, r26 }
	{ slti_u r15, r16, 5 ; s3a r5, r6, r7 ; lh_u r25, r26 }
	{ slti_u r15, r16, 5 ; sb r25, r26 ; ctz r5, r6 }
	{ slti_u r15, r16, 5 ; sb r25, r26 ; or r5, r6, r7 }
	{ slti_u r15, r16, 5 ; sb r25, r26 ; sne r5, r6, r7 }
	{ slti_u r15, r16, 5 ; seqb r5, r6, r7 }
	{ slti_u r15, r16, 5 ; sh r25, r26 ; clz r5, r6 }
	{ slti_u r15, r16, 5 ; sh r25, r26 ; nor r5, r6, r7 }
	{ slti_u r15, r16, 5 ; sh r25, r26 ; slti_u r5, r6, 5 }
	{ slti_u r15, r16, 5 ; shl r5, r6, r7 }
	{ slti_u r15, r16, 5 ; shr r5, r6, r7 ; prefetch r25 }
	{ slti_u r15, r16, 5 ; slt r5, r6, r7 ; lb_u r25, r26 }
	{ slti_u r15, r16, 5 ; sltb_u r5, r6, r7 }
	{ slti_u r15, r16, 5 ; slte_u r5, r6, r7 }
	{ slti_u r15, r16, 5 ; slti_u r5, r6, 5 ; lh_u r25, r26 }
	{ slti_u r15, r16, 5 ; sne r5, r6, r7 }
	{ slti_u r15, r16, 5 ; srai r5, r6, 5 ; prefetch r25 }
	{ slti_u r15, r16, 5 ; subhs r5, r6, r7 }
	{ slti_u r15, r16, 5 ; sw r25, r26 ; mulll_ss r5, r6, r7 }
	{ slti_u r15, r16, 5 ; sw r25, r26 ; shli r5, r6, 5 }
	{ slti_u r15, r16, 5 ; tblidxb0 r5, r6 ; lb_u r25, r26 }
	{ slti_u r15, r16, 5 ; tblidxb2 r5, r6 ; lb_u r25, r26 }
	{ slti_u r15, r16, 5 ; xor r5, r6, r7 ; lb_u r25, r26 }
	{ slti_u r5, r6, 5 ; addb r15, r16, r17 }
	{ slti_u r5, r6, 5 ; and r15, r16, r17 ; lb_u r25, r26 }
	{ slti_u r5, r6, 5 ; dtlbpr r15 }
	{ slti_u r5, r6, 5 ; ill ; sb r25, r26 }
	{ slti_u r5, r6, 5 ; iret }
	{ slti_u r5, r6, 5 ; lb r25, r26 ; ori r15, r16, 5 }
	{ slti_u r5, r6, 5 ; lb r25, r26 ; srai r15, r16, 5 }
	{ slti_u r5, r6, 5 ; lb_u r25, r26 ; rl r15, r16, r17 }
	{ slti_u r5, r6, 5 ; lb_u r25, r26 ; sub r15, r16, r17 }
	{ slti_u r5, r6, 5 ; lh r25, r26 ; ori r15, r16, 5 }
	{ slti_u r5, r6, 5 ; lh r25, r26 ; srai r15, r16, 5 }
	{ slti_u r5, r6, 5 ; lh_u r25, r26 ; rl r15, r16, r17 }
	{ slti_u r5, r6, 5 ; lh_u r25, r26 ; sub r15, r16, r17 }
	{ slti_u r5, r6, 5 ; lw r25, r26 ; or r15, r16, r17 }
	{ slti_u r5, r6, 5 ; lw r25, r26 ; sra r15, r16, r17 }
	{ slti_u r5, r6, 5 ; mnz r15, r16, r17 ; lb_u r25, r26 }
	{ slti_u r5, r6, 5 ; move r15, r16 }
	{ slti_u r5, r6, 5 ; mz r15, r16, r17 ; sb r25, r26 }
	{ slti_u r5, r6, 5 ; nor r15, r16, r17 ; lw r25, r26 }
	{ slti_u r5, r6, 5 ; ori r15, r16, 5 ; lw r25, r26 }
	{ slti_u r5, r6, 5 ; prefetch r25 ; movei r15, 5 }
	{ slti_u r5, r6, 5 ; prefetch r25 ; slte_u r15, r16, r17 }
	{ slti_u r5, r6, 5 ; rli r15, r16, 5 ; lb r25, r26 }
	{ slti_u r5, r6, 5 ; s2a r15, r16, r17 ; lb r25, r26 }
	{ slti_u r5, r6, 5 ; sb r15, r16 }
	{ slti_u r5, r6, 5 ; sb r25, r26 ; s3a r15, r16, r17 }
	{ slti_u r5, r6, 5 ; seq r15, r16, r17 ; lb r25, r26 }
	{ slti_u r5, r6, 5 ; seqi r15, r16, 5 ; sw r25, r26 }
	{ slti_u r5, r6, 5 ; sh r25, r26 ; rl r15, r16, r17 }
	{ slti_u r5, r6, 5 ; sh r25, r26 ; sub r15, r16, r17 }
	{ slti_u r5, r6, 5 ; shli r15, r16, 5 ; lw r25, r26 }
	{ slti_u r5, r6, 5 ; shri r15, r16, 5 ; lb r25, r26 }
	{ slti_u r5, r6, 5 ; slt r15, r16, r17 ; sw r25, r26 }
	{ slti_u r5, r6, 5 ; slte r15, r16, r17 ; sb r25, r26 }
	{ slti_u r5, r6, 5 ; slti r15, r16, 5 ; lb r25, r26 }
	{ slti_u r5, r6, 5 ; sltib r15, r16, 5 }
	{ slti_u r5, r6, 5 ; sra r15, r16, r17 ; lw r25, r26 }
	{ slti_u r5, r6, 5 ; sub r15, r16, r17 ; lb r25, r26 }
	{ slti_u r5, r6, 5 ; sw r25, r26 ; fnop }
	{ slti_u r5, r6, 5 ; sw r25, r26 ; shr r15, r16, r17 }
	{ slti_u r5, r6, 5 ; xor r15, r16, r17 ; lh_u r25, r26 }
	{ sltib r15, r16, 5 ; adiffh r5, r6, r7 }
	{ sltib r15, r16, 5 ; maxb_u r5, r6, r7 }
	{ sltib r15, r16, 5 ; mulhha_su r5, r6, r7 }
	{ sltib r15, r16, 5 ; mvz r5, r6, r7 }
	{ sltib r15, r16, 5 ; sadah_u r5, r6, r7 }
	{ sltib r15, r16, 5 ; shrib r5, r6, 5 }
	{ sltib r15, r16, 5 ; sne r5, r6, r7 }
	{ sltib r15, r16, 5 ; xori r5, r6, 5 }
	{ sltib r5, r6, 5 ; ill }
	{ sltib r5, r6, 5 ; lhadd_u r15, r16, 5 }
	{ sltib r5, r6, 5 ; move r15, r16 }
	{ sltib r5, r6, 5 ; s1a r15, r16, r17 }
	{ sltib r5, r6, 5 ; shrb r15, r16, r17 }
	{ sltib r5, r6, 5 ; sltib_u r15, r16, 5 }
	{ sltib r5, r6, 5 ; tns r15, r16 }
	{ sltib_u r15, r16, 5 ; avgb_u r5, r6, r7 }
	{ sltib_u r15, r16, 5 ; minb_u r5, r6, r7 }
	{ sltib_u r15, r16, 5 ; mulhl_su r5, r6, r7 }
	{ sltib_u r15, r16, 5 ; nop }
	{ sltib_u r15, r16, 5 ; seq r5, r6, r7 }
	{ sltib_u r15, r16, 5 ; sltb r5, r6, r7 }
	{ sltib_u r15, r16, 5 ; srab r5, r6, r7 }
	{ sltib_u r5, r6, 5 ; addh r15, r16, r17 }
	{ sltib_u r5, r6, 5 ; inthh r15, r16, r17 }
	{ sltib_u r5, r6, 5 ; lwadd r15, r16, 5 }
	{ sltib_u r5, r6, 5 ; mtspr 0x5, r16 }
	{ sltib_u r5, r6, 5 ; sbadd r15, r16, 5 }
	{ sltib_u r5, r6, 5 ; shrih r15, r16, 5 }
	{ sltib_u r5, r6, 5 ; sneb r15, r16, r17 }
	{ sltih r15, r16, 5 ; add r5, r6, r7 }
	{ sltih r15, r16, 5 ; clz r5, r6 }
	{ sltih r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
	{ sltih r15, r16, 5 ; mulhla_su r5, r6, r7 }
	{ sltih r15, r16, 5 ; packbs_u r5, r6, r7 }
	{ sltih r15, r16, 5 ; seqib r5, r6, 5 }
	{ sltih r15, r16, 5 ; slteb r5, r6, r7 }
	{ sltih r15, r16, 5 ; sraih r5, r6, 5 }
	{ sltih r5, r6, 5 ; addih r15, r16, 5 }
	{ sltih r5, r6, 5 ; iret }
	{ sltih r5, r6, 5 ; maxib_u r15, r16, 5 }
	{ sltih r5, r6, 5 ; nop }
	{ sltih r5, r6, 5 ; seqi r15, r16, 5 }
	{ sltih r5, r6, 5 ; sltb_u r15, r16, r17 }
	{ sltih r5, r6, 5 ; srah r15, r16, r17 }
	{ sltih_u r15, r16, 5 ; addhs r5, r6, r7 }
	{ sltih_u r15, r16, 5 ; dword_align r5, r6, r7 }
	{ sltih_u r15, r16, 5 ; move r5, r6 }
	{ sltih_u r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ sltih_u r15, r16, 5 ; pcnt r5, r6 }
	{ sltih_u r15, r16, 5 ; shlh r5, r6, r7 }
	{ sltih_u r15, r16, 5 ; slth r5, r6, r7 }
	{ sltih_u r15, r16, 5 ; subh r5, r6, r7 }
	{ sltih_u r5, r6, 5 ; and r15, r16, r17 }
	{ sltih_u r5, r6, 5 ; jrp r15 }
	{ sltih_u r5, r6, 5 ; minb_u r15, r16, r17 }
	{ sltih_u r5, r6, 5 ; packbs_u r15, r16, r17 }
	{ sltih_u r5, r6, 5 ; shadd r15, r16, 5 }
	{ sltih_u r5, r6, 5 ; slteb_u r15, r16, r17 }
	{ sltih_u r5, r6, 5 ; sub r15, r16, r17 }
	{ sne r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
	{ sne r15, r16, r17 ; adds r5, r6, r7 }
	{ sne r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
	{ sne r15, r16, r17 ; bytex r5, r6 ; lw r25, r26 }
	{ sne r15, r16, r17 ; ctz r5, r6 ; lh r25, r26 }
	{ sne r15, r16, r17 ; info 19 ; lb_u r25, r26 }
	{ sne r15, r16, r17 ; lb r25, r26 ; clz r5, r6 }
	{ sne r15, r16, r17 ; lb r25, r26 ; nor r5, r6, r7 }
	{ sne r15, r16, r17 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ sne r15, r16, r17 ; lb_u r25, r26 ; info 19 }
	{ sne r15, r16, r17 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ sne r15, r16, r17 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ sne r15, r16, r17 ; lh r25, r26 ; movei r5, 5 }
	{ sne r15, r16, r17 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ sne r15, r16, r17 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ sne r15, r16, r17 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ sne r15, r16, r17 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ sne r15, r16, r17 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ sne r15, r16, r17 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ sne r15, r16, r17 ; lw r25, r26 ; shli r5, r6, 5 }
	{ sne r15, r16, r17 ; maxh r5, r6, r7 }
	{ sne r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
	{ sne r15, r16, r17 ; moveli r5, 0x1234 }
	{ sne r15, r16, r17 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ sne r15, r16, r17 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ sne r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ sne r15, r16, r17 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ sne r15, r16, r17 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ sne r15, r16, r17 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ sne r15, r16, r17 ; nop ; lh r25, r26 }
	{ sne r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
	{ sne r15, r16, r17 ; packhs r5, r6, r7 }
	{ sne r15, r16, r17 ; prefetch r25 ; fnop }
	{ sne r15, r16, r17 ; prefetch r25 ; ori r5, r6, 5 }
	{ sne r15, r16, r17 ; prefetch r25 ; sra r5, r6, r7 }
	{ sne r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ sne r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ sne r15, r16, r17 ; sadah r5, r6, r7 }
	{ sne r15, r16, r17 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ sne r15, r16, r17 ; sb r25, r26 ; seq r5, r6, r7 }
	{ sne r15, r16, r17 ; sb r25, r26 ; xor r5, r6, r7 }
	{ sne r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ sne r15, r16, r17 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ sne r15, r16, r17 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ sne r15, r16, r17 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ sne r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
	{ sne r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ sne r15, r16, r17 ; slt r5, r6, r7 }
	{ sne r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
	{ sne r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ sne r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ sne r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
	{ sne r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ sne r15, r16, r17 ; sw r25, r26 ; clz r5, r6 }
	{ sne r15, r16, r17 ; sw r25, r26 ; nor r5, r6, r7 }
	{ sne r15, r16, r17 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ sne r15, r16, r17 ; tblidxb0 r5, r6 }
	{ sne r15, r16, r17 ; tblidxb2 r5, r6 }
	{ sne r15, r16, r17 ; xor r5, r6, r7 }
	{ sne r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
	{ sne r5, r6, r7 ; and r15, r16, r17 }
	{ sne r5, r6, r7 ; fnop ; prefetch r25 }
	{ sne r5, r6, r7 ; info 19 ; lw r25, r26 }
	{ sne r5, r6, r7 ; lb r25, r26 ; and r15, r16, r17 }
	{ sne r5, r6, r7 ; lb r25, r26 ; shl r15, r16, r17 }
	{ sne r5, r6, r7 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ sne r5, r6, r7 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ sne r5, r6, r7 ; lh r25, r26 ; and r15, r16, r17 }
	{ sne r5, r6, r7 ; lh r25, r26 ; shl r15, r16, r17 }
	{ sne r5, r6, r7 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ sne r5, r6, r7 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ sne r5, r6, r7 ; lw r25, r26 ; addi r15, r16, 5 }
	{ sne r5, r6, r7 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ sne r5, r6, r7 ; maxb_u r15, r16, r17 }
	{ sne r5, r6, r7 ; mnz r15, r16, r17 }
	{ sne r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
	{ sne r5, r6, r7 ; nop ; lh r25, r26 }
	{ sne r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
	{ sne r5, r6, r7 ; packhs r15, r16, r17 }
	{ sne r5, r6, r7 ; prefetch r25 ; s1a r15, r16, r17 }
	{ sne r5, r6, r7 ; prefetch r25 }
	{ sne r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
	{ sne r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ sne r5, r6, r7 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ sne r5, r6, r7 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ sne r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
	{ sne r5, r6, r7 ; sh r25, r26 ; andi r15, r16, 5 }
	{ sne r5, r6, r7 ; sh r25, r26 ; shli r15, r16, 5 }
	{ sne r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
	{ sne r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
	{ sne r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
	{ sne r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ sne r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ sne r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
	{ sne r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
	{ sne r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
	{ sne r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
	{ sne r5, r6, r7 ; sw r25, r26 ; nor r15, r16, r17 }
	{ sne r5, r6, r7 ; sw r25, r26 ; sne r15, r16, r17 }
	{ sneb r15, r16, r17 ; add r5, r6, r7 }
	{ sneb r15, r16, r17 ; clz r5, r6 }
	{ sneb r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
	{ sneb r15, r16, r17 ; mulhla_su r5, r6, r7 }
	{ sneb r15, r16, r17 ; packbs_u r5, r6, r7 }
	{ sneb r15, r16, r17 ; seqib r5, r6, 5 }
	{ sneb r15, r16, r17 ; slteb r5, r6, r7 }
	{ sneb r15, r16, r17 ; sraih r5, r6, 5 }
	{ sneb r5, r6, r7 ; addih r15, r16, 5 }
	{ sneb r5, r6, r7 ; iret }
	{ sneb r5, r6, r7 ; maxib_u r15, r16, 5 }
	{ sneb r5, r6, r7 ; nop }
	{ sneb r5, r6, r7 ; seqi r15, r16, 5 }
	{ sneb r5, r6, r7 ; sltb_u r15, r16, r17 }
	{ sneb r5, r6, r7 ; srah r15, r16, r17 }
	{ sneh r15, r16, r17 ; addhs r5, r6, r7 }
	{ sneh r15, r16, r17 ; dword_align r5, r6, r7 }
	{ sneh r15, r16, r17 ; move r5, r6 }
	{ sneh r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ sneh r15, r16, r17 ; pcnt r5, r6 }
	{ sneh r15, r16, r17 ; shlh r5, r6, r7 }
	{ sneh r15, r16, r17 ; slth r5, r6, r7 }
	{ sneh r15, r16, r17 ; subh r5, r6, r7 }
	{ sneh r5, r6, r7 ; and r15, r16, r17 }
	{ sneh r5, r6, r7 ; jrp r15 }
	{ sneh r5, r6, r7 ; minb_u r15, r16, r17 }
	{ sneh r5, r6, r7 ; packbs_u r15, r16, r17 }
	{ sneh r5, r6, r7 ; shadd r15, r16, 5 }
	{ sneh r5, r6, r7 ; slteb_u r15, r16, r17 }
	{ sneh r5, r6, r7 ; sub r15, r16, r17 }
	{ sra r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
	{ sra r15, r16, r17 ; adds r5, r6, r7 }
	{ sra r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
	{ sra r15, r16, r17 ; bytex r5, r6 ; lw r25, r26 }
	{ sra r15, r16, r17 ; ctz r5, r6 ; lh r25, r26 }
	{ sra r15, r16, r17 ; info 19 ; lb_u r25, r26 }
	{ sra r15, r16, r17 ; lb r25, r26 ; clz r5, r6 }
	{ sra r15, r16, r17 ; lb r25, r26 ; nor r5, r6, r7 }
	{ sra r15, r16, r17 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ sra r15, r16, r17 ; lb_u r25, r26 ; info 19 }
	{ sra r15, r16, r17 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ sra r15, r16, r17 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ sra r15, r16, r17 ; lh r25, r26 ; movei r5, 5 }
	{ sra r15, r16, r17 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ sra r15, r16, r17 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ sra r15, r16, r17 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ sra r15, r16, r17 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ sra r15, r16, r17 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ sra r15, r16, r17 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ sra r15, r16, r17 ; lw r25, r26 ; shli r5, r6, 5 }
	{ sra r15, r16, r17 ; maxh r5, r6, r7 }
	{ sra r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
	{ sra r15, r16, r17 ; moveli r5, 0x1234 }
	{ sra r15, r16, r17 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ sra r15, r16, r17 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ sra r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ sra r15, r16, r17 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ sra r15, r16, r17 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ sra r15, r16, r17 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ sra r15, r16, r17 ; nop ; lh r25, r26 }
	{ sra r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
	{ sra r15, r16, r17 ; packhs r5, r6, r7 }
	{ sra r15, r16, r17 ; prefetch r25 ; fnop }
	{ sra r15, r16, r17 ; prefetch r25 ; ori r5, r6, 5 }
	{ sra r15, r16, r17 ; prefetch r25 ; sra r5, r6, r7 }
	{ sra r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ sra r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ sra r15, r16, r17 ; sadah r5, r6, r7 }
	{ sra r15, r16, r17 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ sra r15, r16, r17 ; sb r25, r26 ; seq r5, r6, r7 }
	{ sra r15, r16, r17 ; sb r25, r26 ; xor r5, r6, r7 }
	{ sra r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ sra r15, r16, r17 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ sra r15, r16, r17 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ sra r15, r16, r17 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ sra r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
	{ sra r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ sra r15, r16, r17 ; slt r5, r6, r7 }
	{ sra r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
	{ sra r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ sra r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ sra r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
	{ sra r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ sra r15, r16, r17 ; sw r25, r26 ; clz r5, r6 }
	{ sra r15, r16, r17 ; sw r25, r26 ; nor r5, r6, r7 }
	{ sra r15, r16, r17 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ sra r15, r16, r17 ; tblidxb0 r5, r6 }
	{ sra r15, r16, r17 ; tblidxb2 r5, r6 }
	{ sra r15, r16, r17 ; xor r5, r6, r7 }
	{ sra r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
	{ sra r5, r6, r7 ; and r15, r16, r17 }
	{ sra r5, r6, r7 ; fnop ; prefetch r25 }
	{ sra r5, r6, r7 ; info 19 ; lw r25, r26 }
	{ sra r5, r6, r7 ; lb r25, r26 ; and r15, r16, r17 }
	{ sra r5, r6, r7 ; lb r25, r26 ; shl r15, r16, r17 }
	{ sra r5, r6, r7 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ sra r5, r6, r7 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ sra r5, r6, r7 ; lh r25, r26 ; and r15, r16, r17 }
	{ sra r5, r6, r7 ; lh r25, r26 ; shl r15, r16, r17 }
	{ sra r5, r6, r7 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ sra r5, r6, r7 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ sra r5, r6, r7 ; lw r25, r26 ; addi r15, r16, 5 }
	{ sra r5, r6, r7 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ sra r5, r6, r7 ; maxb_u r15, r16, r17 }
	{ sra r5, r6, r7 ; mnz r15, r16, r17 }
	{ sra r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
	{ sra r5, r6, r7 ; nop ; lh r25, r26 }
	{ sra r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
	{ sra r5, r6, r7 ; packhs r15, r16, r17 }
	{ sra r5, r6, r7 ; prefetch r25 ; s1a r15, r16, r17 }
	{ sra r5, r6, r7 ; prefetch r25 }
	{ sra r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
	{ sra r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ sra r5, r6, r7 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ sra r5, r6, r7 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ sra r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
	{ sra r5, r6, r7 ; sh r25, r26 ; andi r15, r16, 5 }
	{ sra r5, r6, r7 ; sh r25, r26 ; shli r15, r16, 5 }
	{ sra r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
	{ sra r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
	{ sra r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
	{ sra r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ sra r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ sra r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
	{ sra r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
	{ sra r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
	{ sra r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
	{ sra r5, r6, r7 ; sw r25, r26 ; nor r15, r16, r17 }
	{ sra r5, r6, r7 ; sw r25, r26 ; sne r15, r16, r17 }
	{ srab r15, r16, r17 ; add r5, r6, r7 }
	{ srab r15, r16, r17 ; clz r5, r6 }
	{ srab r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
	{ srab r15, r16, r17 ; mulhla_su r5, r6, r7 }
	{ srab r15, r16, r17 ; packbs_u r5, r6, r7 }
	{ srab r15, r16, r17 ; seqib r5, r6, 5 }
	{ srab r15, r16, r17 ; slteb r5, r6, r7 }
	{ srab r15, r16, r17 ; sraih r5, r6, 5 }
	{ srab r5, r6, r7 ; addih r15, r16, 5 }
	{ srab r5, r6, r7 ; iret }
	{ srab r5, r6, r7 ; maxib_u r15, r16, 5 }
	{ srab r5, r6, r7 ; nop }
	{ srab r5, r6, r7 ; seqi r15, r16, 5 }
	{ srab r5, r6, r7 ; sltb_u r15, r16, r17 }
	{ srab r5, r6, r7 ; srah r15, r16, r17 }
	{ srah r15, r16, r17 ; addhs r5, r6, r7 }
	{ srah r15, r16, r17 ; dword_align r5, r6, r7 }
	{ srah r15, r16, r17 ; move r5, r6 }
	{ srah r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ srah r15, r16, r17 ; pcnt r5, r6 }
	{ srah r15, r16, r17 ; shlh r5, r6, r7 }
	{ srah r15, r16, r17 ; slth r5, r6, r7 }
	{ srah r15, r16, r17 ; subh r5, r6, r7 }
	{ srah r5, r6, r7 ; and r15, r16, r17 }
	{ srah r5, r6, r7 ; jrp r15 }
	{ srah r5, r6, r7 ; minb_u r15, r16, r17 }
	{ srah r5, r6, r7 ; packbs_u r15, r16, r17 }
	{ srah r5, r6, r7 ; shadd r15, r16, 5 }
	{ srah r5, r6, r7 ; slteb_u r15, r16, r17 }
	{ srah r5, r6, r7 ; sub r15, r16, r17 }
	{ srai r15, r16, 5 ; add r5, r6, r7 ; sw r25, r26 }
	{ srai r15, r16, 5 ; adds r5, r6, r7 }
	{ srai r15, r16, 5 ; andi r5, r6, 5 ; sh r25, r26 }
	{ srai r15, r16, 5 ; bytex r5, r6 ; lw r25, r26 }
	{ srai r15, r16, 5 ; ctz r5, r6 ; lh r25, r26 }
	{ srai r15, r16, 5 ; info 19 ; lb_u r25, r26 }
	{ srai r15, r16, 5 ; lb r25, r26 ; clz r5, r6 }
	{ srai r15, r16, 5 ; lb r25, r26 ; nor r5, r6, r7 }
	{ srai r15, r16, 5 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ srai r15, r16, 5 ; lb_u r25, r26 ; info 19 }
	{ srai r15, r16, 5 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ srai r15, r16, 5 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ srai r15, r16, 5 ; lh r25, r26 ; movei r5, 5 }
	{ srai r15, r16, 5 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ srai r15, r16, 5 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ srai r15, r16, 5 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ srai r15, r16, 5 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ srai r15, r16, 5 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ srai r15, r16, 5 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ srai r15, r16, 5 ; lw r25, r26 ; shli r5, r6, 5 }
	{ srai r15, r16, 5 ; maxh r5, r6, r7 }
	{ srai r15, r16, 5 ; move r5, r6 ; lb r25, r26 }
	{ srai r15, r16, 5 ; moveli r5, 0x1234 }
	{ srai r15, r16, 5 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ srai r15, r16, 5 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ srai r15, r16, 5 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ srai r15, r16, 5 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ srai r15, r16, 5 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ srai r15, r16, 5 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ srai r15, r16, 5 ; nop ; lh r25, r26 }
	{ srai r15, r16, 5 ; or r5, r6, r7 ; lh r25, r26 }
	{ srai r15, r16, 5 ; packhs r5, r6, r7 }
	{ srai r15, r16, 5 ; prefetch r25 ; fnop }
	{ srai r15, r16, 5 ; prefetch r25 ; ori r5, r6, 5 }
	{ srai r15, r16, 5 ; prefetch r25 ; sra r5, r6, r7 }
	{ srai r15, r16, 5 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ srai r15, r16, 5 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ srai r15, r16, 5 ; sadah r5, r6, r7 }
	{ srai r15, r16, 5 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ srai r15, r16, 5 ; sb r25, r26 ; seq r5, r6, r7 }
	{ srai r15, r16, 5 ; sb r25, r26 ; xor r5, r6, r7 }
	{ srai r15, r16, 5 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ srai r15, r16, 5 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ srai r15, r16, 5 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ srai r15, r16, 5 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ srai r15, r16, 5 ; shli r5, r6, 5 ; prefetch r25 }
	{ srai r15, r16, 5 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ srai r15, r16, 5 ; slt r5, r6, r7 }
	{ srai r15, r16, 5 ; slte r5, r6, r7 ; sh r25, r26 }
	{ srai r15, r16, 5 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ srai r15, r16, 5 ; sltib_u r5, r6, 5 }
	{ srai r15, r16, 5 ; sra r5, r6, r7 ; prefetch r25 }
	{ srai r15, r16, 5 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ srai r15, r16, 5 ; sw r25, r26 ; clz r5, r6 }
	{ srai r15, r16, 5 ; sw r25, r26 ; nor r5, r6, r7 }
	{ srai r15, r16, 5 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ srai r15, r16, 5 ; tblidxb0 r5, r6 }
	{ srai r15, r16, 5 ; tblidxb2 r5, r6 }
	{ srai r15, r16, 5 ; xor r5, r6, r7 }
	{ srai r5, r6, 5 ; addi r15, r16, 5 ; lw r25, r26 }
	{ srai r5, r6, 5 ; and r15, r16, r17 }
	{ srai r5, r6, 5 ; fnop ; prefetch r25 }
	{ srai r5, r6, 5 ; info 19 ; lw r25, r26 }
	{ srai r5, r6, 5 ; lb r25, r26 ; and r15, r16, r17 }
	{ srai r5, r6, 5 ; lb r25, r26 ; shl r15, r16, r17 }
	{ srai r5, r6, 5 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ srai r5, r6, 5 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ srai r5, r6, 5 ; lh r25, r26 ; and r15, r16, r17 }
	{ srai r5, r6, 5 ; lh r25, r26 ; shl r15, r16, r17 }
	{ srai r5, r6, 5 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ srai r5, r6, 5 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ srai r5, r6, 5 ; lw r25, r26 ; addi r15, r16, 5 }
	{ srai r5, r6, 5 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ srai r5, r6, 5 ; maxb_u r15, r16, r17 }
	{ srai r5, r6, 5 ; mnz r15, r16, r17 }
	{ srai r5, r6, 5 ; movei r15, 5 ; sh r25, r26 }
	{ srai r5, r6, 5 ; nop ; lh r25, r26 }
	{ srai r5, r6, 5 ; or r15, r16, r17 ; lh r25, r26 }
	{ srai r5, r6, 5 ; packhs r15, r16, r17 }
	{ srai r5, r6, 5 ; prefetch r25 ; s1a r15, r16, r17 }
	{ srai r5, r6, 5 ; prefetch r25 }
	{ srai r5, r6, 5 ; rli r15, r16, 5 ; sw r25, r26 }
	{ srai r5, r6, 5 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ srai r5, r6, 5 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ srai r5, r6, 5 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ srai r5, r6, 5 ; seq r15, r16, r17 ; sw r25, r26 }
	{ srai r5, r6, 5 ; sh r25, r26 ; andi r15, r16, 5 }
	{ srai r5, r6, 5 ; sh r25, r26 ; shli r15, r16, 5 }
	{ srai r5, r6, 5 ; shl r15, r16, r17 ; lw r25, r26 }
	{ srai r5, r6, 5 ; shr r15, r16, r17 ; lb r25, r26 }
	{ srai r5, r6, 5 ; shri r15, r16, 5 ; sw r25, r26 }
	{ srai r5, r6, 5 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ srai r5, r6, 5 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ srai r5, r6, 5 ; slti r15, r16, 5 ; sw r25, r26 }
	{ srai r5, r6, 5 ; sne r15, r16, r17 ; lw r25, r26 }
	{ srai r5, r6, 5 ; srai r15, r16, 5 ; lb r25, r26 }
	{ srai r5, r6, 5 ; sub r15, r16, r17 ; sw r25, r26 }
	{ srai r5, r6, 5 ; sw r25, r26 ; nor r15, r16, r17 }
	{ srai r5, r6, 5 ; sw r25, r26 ; sne r15, r16, r17 }
	{ sraib r15, r16, 5 ; add r5, r6, r7 }
	{ sraib r15, r16, 5 ; clz r5, r6 }
	{ sraib r15, r16, 5 ; mm r5, r6, r7, 5, 7 }
	{ sraib r15, r16, 5 ; mulhla_su r5, r6, r7 }
	{ sraib r15, r16, 5 ; packbs_u r5, r6, r7 }
	{ sraib r15, r16, 5 ; seqib r5, r6, 5 }
	{ sraib r15, r16, 5 ; slteb r5, r6, r7 }
	{ sraib r15, r16, 5 ; sraih r5, r6, 5 }
	{ sraib r5, r6, 5 ; addih r15, r16, 5 }
	{ sraib r5, r6, 5 ; iret }
	{ sraib r5, r6, 5 ; maxib_u r15, r16, 5 }
	{ sraib r5, r6, 5 ; nop }
	{ sraib r5, r6, 5 ; seqi r15, r16, 5 }
	{ sraib r5, r6, 5 ; sltb_u r15, r16, r17 }
	{ sraib r5, r6, 5 ; srah r15, r16, r17 }
	{ sraih r15, r16, 5 ; addhs r5, r6, r7 }
	{ sraih r15, r16, 5 ; dword_align r5, r6, r7 }
	{ sraih r15, r16, 5 ; move r5, r6 }
	{ sraih r15, r16, 5 ; mulll_ss r5, r6, r7 }
	{ sraih r15, r16, 5 ; pcnt r5, r6 }
	{ sraih r15, r16, 5 ; shlh r5, r6, r7 }
	{ sraih r15, r16, 5 ; slth r5, r6, r7 }
	{ sraih r15, r16, 5 ; subh r5, r6, r7 }
	{ sraih r5, r6, 5 ; and r15, r16, r17 }
	{ sraih r5, r6, 5 ; jrp r15 }
	{ sraih r5, r6, 5 ; minb_u r15, r16, r17 }
	{ sraih r5, r6, 5 ; packbs_u r15, r16, r17 }
	{ sraih r5, r6, 5 ; shadd r15, r16, 5 }
	{ sraih r5, r6, 5 ; slteb_u r15, r16, r17 }
	{ sraih r5, r6, 5 ; sub r15, r16, r17 }
	{ sub r15, r16, r17 ; add r5, r6, r7 ; sw r25, r26 }
	{ sub r15, r16, r17 ; adds r5, r6, r7 }
	{ sub r15, r16, r17 ; andi r5, r6, 5 ; sh r25, r26 }
	{ sub r15, r16, r17 ; bytex r5, r6 ; lw r25, r26 }
	{ sub r15, r16, r17 ; ctz r5, r6 ; lh r25, r26 }
	{ sub r15, r16, r17 ; info 19 ; lb_u r25, r26 }
	{ sub r15, r16, r17 ; lb r25, r26 ; clz r5, r6 }
	{ sub r15, r16, r17 ; lb r25, r26 ; nor r5, r6, r7 }
	{ sub r15, r16, r17 ; lb r25, r26 ; slti_u r5, r6, 5 }
	{ sub r15, r16, r17 ; lb_u r25, r26 ; info 19 }
	{ sub r15, r16, r17 ; lb_u r25, r26 ; pcnt r5, r6 }
	{ sub r15, r16, r17 ; lb_u r25, r26 ; srai r5, r6, 5 }
	{ sub r15, r16, r17 ; lh r25, r26 ; movei r5, 5 }
	{ sub r15, r16, r17 ; lh r25, r26 ; s1a r5, r6, r7 }
	{ sub r15, r16, r17 ; lh r25, r26 ; tblidxb1 r5, r6 }
	{ sub r15, r16, r17 ; lh_u r25, r26 ; mulhha_ss r5, r6, r7 }
	{ sub r15, r16, r17 ; lh_u r25, r26 ; seq r5, r6, r7 }
	{ sub r15, r16, r17 ; lh_u r25, r26 ; xor r5, r6, r7 }
	{ sub r15, r16, r17 ; lw r25, r26 ; mulll_ss r5, r6, r7 }
	{ sub r15, r16, r17 ; lw r25, r26 ; shli r5, r6, 5 }
	{ sub r15, r16, r17 ; maxh r5, r6, r7 }
	{ sub r15, r16, r17 ; move r5, r6 ; lb r25, r26 }
	{ sub r15, r16, r17 ; moveli r5, 0x1234 }
	{ sub r15, r16, r17 ; mulhh_uu r5, r6, r7 ; sh r25, r26 }
	{ sub r15, r16, r17 ; mulhha_uu r5, r6, r7 ; sb r25, r26 }
	{ sub r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; sh r25, r26 }
	{ sub r15, r16, r17 ; mulll_uu r5, r6, r7 ; sb r25, r26 }
	{ sub r15, r16, r17 ; mullla_uu r5, r6, r7 ; prefetch r25 }
	{ sub r15, r16, r17 ; mvz r5, r6, r7 ; lw r25, r26 }
	{ sub r15, r16, r17 ; nop ; lh r25, r26 }
	{ sub r15, r16, r17 ; or r5, r6, r7 ; lh r25, r26 }
	{ sub r15, r16, r17 ; packhs r5, r6, r7 }
	{ sub r15, r16, r17 ; prefetch r25 ; fnop }
	{ sub r15, r16, r17 ; prefetch r25 ; ori r5, r6, 5 }
	{ sub r15, r16, r17 ; prefetch r25 ; sra r5, r6, r7 }
	{ sub r15, r16, r17 ; rli r5, r6, 5 ; lb_u r25, r26 }
	{ sub r15, r16, r17 ; s2a r5, r6, r7 ; lb_u r25, r26 }
	{ sub r15, r16, r17 ; sadah r5, r6, r7 }
	{ sub r15, r16, r17 ; sb r25, r26 ; mulhha_ss r5, r6, r7 }
	{ sub r15, r16, r17 ; sb r25, r26 ; seq r5, r6, r7 }
	{ sub r15, r16, r17 ; sb r25, r26 ; xor r5, r6, r7 }
	{ sub r15, r16, r17 ; seqi r5, r6, 5 ; sb r25, r26 }
	{ sub r15, r16, r17 ; sh r25, r26 ; mulhh_uu r5, r6, r7 }
	{ sub r15, r16, r17 ; sh r25, r26 ; s3a r5, r6, r7 }
	{ sub r15, r16, r17 ; sh r25, r26 ; tblidxb3 r5, r6 }
	{ sub r15, r16, r17 ; shli r5, r6, 5 ; prefetch r25 }
	{ sub r15, r16, r17 ; shri r5, r6, 5 ; lb_u r25, r26 }
	{ sub r15, r16, r17 ; slt r5, r6, r7 }
	{ sub r15, r16, r17 ; slte r5, r6, r7 ; sh r25, r26 }
	{ sub r15, r16, r17 ; slti r5, r6, 5 ; lb_u r25, r26 }
	{ sub r15, r16, r17 ; sltib_u r5, r6, 5 }
	{ sub r15, r16, r17 ; sra r5, r6, r7 ; prefetch r25 }
	{ sub r15, r16, r17 ; sub r5, r6, r7 ; lb_u r25, r26 }
	{ sub r15, r16, r17 ; sw r25, r26 ; clz r5, r6 }
	{ sub r15, r16, r17 ; sw r25, r26 ; nor r5, r6, r7 }
	{ sub r15, r16, r17 ; sw r25, r26 ; slti_u r5, r6, 5 }
	{ sub r15, r16, r17 ; tblidxb0 r5, r6 }
	{ sub r15, r16, r17 ; tblidxb2 r5, r6 }
	{ sub r15, r16, r17 ; xor r5, r6, r7 }
	{ sub r5, r6, r7 ; addi r15, r16, 5 ; lw r25, r26 }
	{ sub r5, r6, r7 ; and r15, r16, r17 }
	{ sub r5, r6, r7 ; fnop ; prefetch r25 }
	{ sub r5, r6, r7 ; info 19 ; lw r25, r26 }
	{ sub r5, r6, r7 ; lb r25, r26 ; and r15, r16, r17 }
	{ sub r5, r6, r7 ; lb r25, r26 ; shl r15, r16, r17 }
	{ sub r5, r6, r7 ; lb_u r25, r26 ; andi r15, r16, 5 }
	{ sub r5, r6, r7 ; lb_u r25, r26 ; shli r15, r16, 5 }
	{ sub r5, r6, r7 ; lh r25, r26 ; and r15, r16, r17 }
	{ sub r5, r6, r7 ; lh r25, r26 ; shl r15, r16, r17 }
	{ sub r5, r6, r7 ; lh_u r25, r26 ; andi r15, r16, 5 }
	{ sub r5, r6, r7 ; lh_u r25, r26 ; shli r15, r16, 5 }
	{ sub r5, r6, r7 ; lw r25, r26 ; addi r15, r16, 5 }
	{ sub r5, r6, r7 ; lw r25, r26 ; seqi r15, r16, 5 }
	{ sub r5, r6, r7 ; maxb_u r15, r16, r17 }
	{ sub r5, r6, r7 ; mnz r15, r16, r17 }
	{ sub r5, r6, r7 ; movei r15, 5 ; sh r25, r26 }
	{ sub r5, r6, r7 ; nop ; lh r25, r26 }
	{ sub r5, r6, r7 ; or r15, r16, r17 ; lh r25, r26 }
	{ sub r5, r6, r7 ; packhs r15, r16, r17 }
	{ sub r5, r6, r7 ; prefetch r25 ; s1a r15, r16, r17 }
	{ sub r5, r6, r7 ; prefetch r25 }
	{ sub r5, r6, r7 ; rli r15, r16, 5 ; sw r25, r26 }
	{ sub r5, r6, r7 ; s2a r15, r16, r17 ; sw r25, r26 }
	{ sub r5, r6, r7 ; sb r25, r26 ; mnz r15, r16, r17 }
	{ sub r5, r6, r7 ; sb r25, r26 ; slt_u r15, r16, r17 }
	{ sub r5, r6, r7 ; seq r15, r16, r17 ; sw r25, r26 }
	{ sub r5, r6, r7 ; sh r25, r26 ; andi r15, r16, 5 }
	{ sub r5, r6, r7 ; sh r25, r26 ; shli r15, r16, 5 }
	{ sub r5, r6, r7 ; shl r15, r16, r17 ; lw r25, r26 }
	{ sub r5, r6, r7 ; shr r15, r16, r17 ; lb r25, r26 }
	{ sub r5, r6, r7 ; shri r15, r16, 5 ; sw r25, r26 }
	{ sub r5, r6, r7 ; slt_u r15, r16, r17 ; sb r25, r26 }
	{ sub r5, r6, r7 ; slte_u r15, r16, r17 ; lw r25, r26 }
	{ sub r5, r6, r7 ; slti r15, r16, 5 ; sw r25, r26 }
	{ sub r5, r6, r7 ; sne r15, r16, r17 ; lw r25, r26 }
	{ sub r5, r6, r7 ; srai r15, r16, 5 ; lb r25, r26 }
	{ sub r5, r6, r7 ; sub r15, r16, r17 ; sw r25, r26 }
	{ sub r5, r6, r7 ; sw r25, r26 ; nor r15, r16, r17 }
	{ sub r5, r6, r7 ; sw r25, r26 ; sne r15, r16, r17 }
	{ subb r15, r16, r17 ; add r5, r6, r7 }
	{ subb r15, r16, r17 ; clz r5, r6 }
	{ subb r15, r16, r17 ; mm r5, r6, r7, 5, 7 }
	{ subb r15, r16, r17 ; mulhla_su r5, r6, r7 }
	{ subb r15, r16, r17 ; packbs_u r5, r6, r7 }
	{ subb r15, r16, r17 ; seqib r5, r6, 5 }
	{ subb r15, r16, r17 ; slteb r5, r6, r7 }
	{ subb r15, r16, r17 ; sraih r5, r6, 5 }
	{ subb r5, r6, r7 ; addih r15, r16, 5 }
	{ subb r5, r6, r7 ; iret }
	{ subb r5, r6, r7 ; maxib_u r15, r16, 5 }
	{ subb r5, r6, r7 ; nop }
	{ subb r5, r6, r7 ; seqi r15, r16, 5 }
	{ subb r5, r6, r7 ; sltb_u r15, r16, r17 }
	{ subb r5, r6, r7 ; srah r15, r16, r17 }
	{ subbs_u r15, r16, r17 ; addhs r5, r6, r7 }
	{ subbs_u r15, r16, r17 ; dword_align r5, r6, r7 }
	{ subbs_u r15, r16, r17 ; move r5, r6 }
	{ subbs_u r15, r16, r17 ; mulll_ss r5, r6, r7 }
	{ subbs_u r15, r16, r17 ; pcnt r5, r6 }
	{ subbs_u r15, r16, r17 ; shlh r5, r6, r7 }
	{ subbs_u r15, r16, r17 ; slth r5, r6, r7 }
	{ subbs_u r15, r16, r17 ; subh r5, r6, r7 }
	{ subbs_u r5, r6, r7 ; and r15, r16, r17 }
	{ subbs_u r5, r6, r7 ; jrp r15 }
	{ subbs_u r5, r6, r7 ; minb_u r15, r16, r17 }
	{ subbs_u r5, r6, r7 ; packbs_u r15, r16, r17 }
	{ subbs_u r5, r6, r7 ; shadd r15, r16, 5 }
	{ subbs_u r5, r6, r7 ; slteb_u r15, r16, r17 }
	{ subbs_u r5, r6, r7 ; sub r15, r16, r17 }
	{ subh r15, r16, r17 ; addli r5, r6, 0x1234 }
	{ subh r15, r16, r17 ; inthb r5, r6, r7 }
	{ subh r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ subh r15, r16, r17 ; mullla_su r5, r6, r7 }
	{ subh r15, r16, r17 ; s2a r5, r6, r7 }
	{ subh r15, r16, r17 ; shr r5, r6, r7 }
	{ subh r15, r16, r17 ; sltib r5, r6, 5 }
	{ subh r15, r16, r17 ; tblidxb1 r5, r6 }
	{ subh r5, r6, r7 ; finv r15 }
	{ subh r5, r6, r7 ; lbadd_u r15, r16, 5 }
	{ subh r5, r6, r7 ; mm r15, r16, r17, 5, 7 }
	{ subh r5, r6, r7 ; prefetch r15 }
	{ subh r5, r6, r7 ; shli r15, r16, 5 }
	{ subh r5, r6, r7 ; slth_u r15, r16, r17 }
	{ subh r5, r6, r7 ; subhs r15, r16, r17 }
	{ subhs r15, r16, r17 ; adiffh r5, r6, r7 }
	{ subhs r15, r16, r17 ; maxb_u r5, r6, r7 }
	{ subhs r15, r16, r17 ; mulhha_su r5, r6, r7 }
	{ subhs r15, r16, r17 ; mvz r5, r6, r7 }
	{ subhs r15, r16, r17 ; sadah_u r5, r6, r7 }
	{ subhs r15, r16, r17 ; shrib r5, r6, 5 }
	{ subhs r15, r16, r17 ; sne r5, r6, r7 }
	{ subhs r15, r16, r17 ; xori r5, r6, 5 }
	{ subhs r5, r6, r7 ; ill }
	{ subhs r5, r6, r7 ; lhadd_u r15, r16, 5 }
	{ subhs r5, r6, r7 ; move r15, r16 }
	{ subhs r5, r6, r7 ; s1a r15, r16, r17 }
	{ subhs r5, r6, r7 ; shrb r15, r16, r17 }
	{ subhs r5, r6, r7 ; sltib_u r15, r16, 5 }
	{ subhs r5, r6, r7 ; tns r15, r16 }
	{ subs r15, r16, r17 ; avgb_u r5, r6, r7 }
	{ subs r15, r16, r17 ; minb_u r5, r6, r7 }
	{ subs r15, r16, r17 ; mulhl_su r5, r6, r7 }
	{ subs r15, r16, r17 ; nop }
	{ subs r15, r16, r17 ; seq r5, r6, r7 }
	{ subs r15, r16, r17 ; sltb r5, r6, r7 }
	{ subs r15, r16, r17 ; srab r5, r6, r7 }
	{ subs r5, r6, r7 ; addh r15, r16, r17 }
	{ subs r5, r6, r7 ; inthh r15, r16, r17 }
	{ subs r5, r6, r7 ; lwadd r15, r16, 5 }
	{ subs r5, r6, r7 ; mtspr 0x5, r16 }
	{ subs r5, r6, r7 ; sbadd r15, r16, 5 }
	{ subs r5, r6, r7 ; shrih r15, r16, 5 }
	{ subs r5, r6, r7 ; sneb r15, r16, r17 }
	{ sw r15, r16 ; add r5, r6, r7 }
	{ sw r15, r16 ; clz r5, r6 }
	{ sw r15, r16 ; mm r5, r6, r7, 5, 7 }
	{ sw r15, r16 ; mulhla_su r5, r6, r7 }
	{ sw r15, r16 ; packbs_u r5, r6, r7 }
	{ sw r15, r16 ; seqib r5, r6, 5 }
	{ sw r15, r16 ; slteb r5, r6, r7 }
	{ sw r15, r16 ; sraih r5, r6, 5 }
	{ sw r25, r26 ; add r15, r16, r17 ; ctz r5, r6 }
	{ sw r25, r26 ; add r15, r16, r17 ; or r5, r6, r7 }
	{ sw r25, r26 ; add r15, r16, r17 ; sne r5, r6, r7 }
	{ sw r25, r26 ; add r5, r6, r7 ; mz r15, r16, r17 }
	{ sw r25, r26 ; add r5, r6, r7 ; slti r15, r16, 5 }
	{ sw r25, r26 ; addi r15, r16, 5 ; movei r5, 5 }
	{ sw r25, r26 ; addi r15, r16, 5 ; s1a r5, r6, r7 }
	{ sw r25, r26 ; addi r15, r16, 5 ; tblidxb1 r5, r6 }
	{ sw r25, r26 ; addi r5, r6, 5 ; rl r15, r16, r17 }
	{ sw r25, r26 ; addi r5, r6, 5 ; sub r15, r16, r17 }
	{ sw r25, r26 ; and r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ sw r25, r26 ; and r15, r16, r17 ; shl r5, r6, r7 }
	{ sw r25, r26 ; and r5, r6, r7 ; add r15, r16, r17 }
	{ sw r25, r26 ; and r5, r6, r7 ; seq r15, r16, r17 }
	{ sw r25, r26 ; andi r15, r16, 5 ; and r5, r6, r7 }
	{ sw r25, r26 ; andi r15, r16, 5 ; mvnz r5, r6, r7 }
	{ sw r25, r26 ; andi r15, r16, 5 ; slt_u r5, r6, r7 }
	{ sw r25, r26 ; andi r5, r6, 5 ; ill }
	{ sw r25, r26 ; andi r5, r6, 5 ; shri r15, r16, 5 }
	{ sw r25, r26 ; bitx r5, r6 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; bitx r5, r6 ; slt_u r15, r16, r17 }
	{ sw r25, r26 ; bytex r5, r6 ; movei r15, 5 }
	{ sw r25, r26 ; bytex r5, r6 ; slte_u r15, r16, r17 }
	{ sw r25, r26 ; clz r5, r6 ; nop }
	{ sw r25, r26 ; clz r5, r6 ; slti_u r15, r16, 5 }
	{ sw r25, r26 ; ctz r5, r6 ; or r15, r16, r17 }
	{ sw r25, r26 ; ctz r5, r6 ; sra r15, r16, r17 }
	{ sw r25, r26 ; fnop ; mnz r15, r16, r17 }
	{ sw r25, r26 ; fnop ; nor r15, r16, r17 }
	{ sw r25, r26 ; fnop ; seqi r5, r6, 5 }
	{ sw r25, r26 ; fnop ; slti_u r5, r6, 5 }
	{ sw r25, r26 ; ill ; bitx r5, r6 }
	{ sw r25, r26 ; ill ; mz r5, r6, r7 }
	{ sw r25, r26 ; ill ; slte_u r5, r6, r7 }
	{ sw r25, r26 ; info 19 ; andi r5, r6, 5 }
	{ sw r25, r26 ; info 19 ; mulll_uu r5, r6, r7 }
	{ sw r25, r26 ; info 19 ; s1a r5, r6, r7 }
	{ sw r25, r26 ; info 19 ; slt_u r5, r6, r7 }
	{ sw r25, r26 ; info 19 ; tblidxb3 r5, r6 }
	{ sw r25, r26 ; mnz r15, r16, r17 ; mulhha_uu r5, r6, r7 }
	{ sw r25, r26 ; mnz r15, r16, r17 ; seqi r5, r6, 5 }
	{ sw r25, r26 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; mnz r5, r6, r7 ; s3a r15, r16, r17 }
	{ sw r25, r26 ; move r15, r16 ; addi r5, r6, 5 }
	{ sw r25, r26 ; move r15, r16 ; mullla_uu r5, r6, r7 }
	{ sw r25, r26 ; move r15, r16 ; slt r5, r6, r7 }
	{ sw r25, r26 ; move r5, r6 ; fnop }
	{ sw r25, r26 ; move r5, r6 ; shr r15, r16, r17 }
	{ sw r25, r26 ; movei r15, 5 ; clz r5, r6 }
	{ sw r25, r26 ; movei r15, 5 ; nor r5, r6, r7 }
	{ sw r25, r26 ; movei r15, 5 ; slti_u r5, r6, 5 }
	{ sw r25, r26 ; movei r5, 5 ; movei r15, 5 }
	{ sw r25, r26 ; movei r5, 5 ; slte_u r15, r16, r17 }
	{ sw r25, r26 ; mulhh_ss r5, r6, r7 ; nop }
	{ sw r25, r26 ; mulhh_ss r5, r6, r7 ; slti_u r15, r16, 5 }
	{ sw r25, r26 ; mulhh_uu r5, r6, r7 ; or r15, r16, r17 }
	{ sw r25, r26 ; mulhh_uu r5, r6, r7 ; sra r15, r16, r17 }
	{ sw r25, r26 ; mulhha_ss r5, r6, r7 ; rl r15, r16, r17 }
	{ sw r25, r26 ; mulhha_ss r5, r6, r7 ; sub r15, r16, r17 }
	{ sw r25, r26 ; mulhha_uu r5, r6, r7 ; s1a r15, r16, r17 }
	{ sw r25, r26 ; mulhha_uu r5, r6, r7 }
	{ sw r25, r26 ; mulhlsa_uu r5, r6, r7 ; s3a r15, r16, r17 }
	{ sw r25, r26 ; mulll_ss r5, r6, r7 ; addi r15, r16, 5 }
	{ sw r25, r26 ; mulll_ss r5, r6, r7 ; seqi r15, r16, 5 }
	{ sw r25, r26 ; mulll_uu r5, r6, r7 ; andi r15, r16, 5 }
	{ sw r25, r26 ; mulll_uu r5, r6, r7 ; shli r15, r16, 5 }
	{ sw r25, r26 ; mullla_ss r5, r6, r7 ; ill }
	{ sw r25, r26 ; mullla_ss r5, r6, r7 ; shri r15, r16, 5 }
	{ sw r25, r26 ; mullla_uu r5, r6, r7 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; mullla_uu r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sw r25, r26 ; mvnz r5, r6, r7 ; movei r15, 5 }
	{ sw r25, r26 ; mvnz r5, r6, r7 ; slte_u r15, r16, r17 }
	{ sw r25, r26 ; mvz r5, r6, r7 ; nop }
	{ sw r25, r26 ; mvz r5, r6, r7 ; slti_u r15, r16, 5 }
	{ sw r25, r26 ; mz r15, r16, r17 ; mulhh_ss r5, r6, r7 }
	{ sw r25, r26 ; mz r15, r16, r17 ; s2a r5, r6, r7 }
	{ sw r25, r26 ; mz r15, r16, r17 ; tblidxb2 r5, r6 }
	{ sw r25, r26 ; mz r5, r6, r7 ; rli r15, r16, 5 }
	{ sw r25, r26 ; mz r5, r6, r7 ; xor r15, r16, r17 }
	{ sw r25, r26 ; nop ; move r5, r6 }
	{ sw r25, r26 ; nop ; or r5, r6, r7 }
	{ sw r25, r26 ; nop ; shli r15, r16, 5 }
	{ sw r25, r26 ; nop ; sra r15, r16, r17 }
	{ sw r25, r26 ; nor r15, r16, r17 ; ctz r5, r6 }
	{ sw r25, r26 ; nor r15, r16, r17 ; or r5, r6, r7 }
	{ sw r25, r26 ; nor r15, r16, r17 ; sne r5, r6, r7 }
	{ sw r25, r26 ; nor r5, r6, r7 ; mz r15, r16, r17 }
	{ sw r25, r26 ; nor r5, r6, r7 ; slti r15, r16, 5 }
	{ sw r25, r26 ; or r15, r16, r17 ; movei r5, 5 }
	{ sw r25, r26 ; or r15, r16, r17 ; s1a r5, r6, r7 }
	{ sw r25, r26 ; or r15, r16, r17 ; tblidxb1 r5, r6 }
	{ sw r25, r26 ; or r5, r6, r7 ; rl r15, r16, r17 }
	{ sw r25, r26 ; or r5, r6, r7 ; sub r15, r16, r17 }
	{ sw r25, r26 ; ori r15, r16, 5 ; mulhlsa_uu r5, r6, r7 }
	{ sw r25, r26 ; ori r15, r16, 5 ; shl r5, r6, r7 }
	{ sw r25, r26 ; ori r5, r6, 5 ; add r15, r16, r17 }
	{ sw r25, r26 ; ori r5, r6, 5 ; seq r15, r16, r17 }
	{ sw r25, r26 ; pcnt r5, r6 ; and r15, r16, r17 }
	{ sw r25, r26 ; pcnt r5, r6 ; shl r15, r16, r17 }
	{ sw r25, r26 ; rl r15, r16, r17 ; bitx r5, r6 }
	{ sw r25, r26 ; rl r15, r16, r17 ; mz r5, r6, r7 }
	{ sw r25, r26 ; rl r15, r16, r17 ; slte_u r5, r6, r7 }
	{ sw r25, r26 ; rl r5, r6, r7 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; rl r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sw r25, r26 ; rli r15, r16, 5 ; info 19 }
	{ sw r25, r26 ; rli r15, r16, 5 ; pcnt r5, r6 }
	{ sw r25, r26 ; rli r15, r16, 5 ; srai r5, r6, 5 }
	{ sw r25, r26 ; rli r5, r6, 5 ; nor r15, r16, r17 }
	{ sw r25, r26 ; rli r5, r6, 5 ; sne r15, r16, r17 }
	{ sw r25, r26 ; s1a r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ sw r25, r26 ; s1a r15, r16, r17 ; s3a r5, r6, r7 }
	{ sw r25, r26 ; s1a r15, r16, r17 ; tblidxb3 r5, r6 }
	{ sw r25, r26 ; s1a r5, r6, r7 ; s1a r15, r16, r17 }
	{ sw r25, r26 ; s1a r5, r6, r7 }
	{ sw r25, r26 ; s2a r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ sw r25, r26 ; s2a r15, r16, r17 ; shr r5, r6, r7 }
	{ sw r25, r26 ; s2a r5, r6, r7 ; and r15, r16, r17 }
	{ sw r25, r26 ; s2a r5, r6, r7 ; shl r15, r16, r17 }
	{ sw r25, r26 ; s3a r15, r16, r17 ; bitx r5, r6 }
	{ sw r25, r26 ; s3a r15, r16, r17 ; mz r5, r6, r7 }
	{ sw r25, r26 ; s3a r15, r16, r17 ; slte_u r5, r6, r7 }
	{ sw r25, r26 ; s3a r5, r6, r7 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; s3a r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sw r25, r26 ; seq r15, r16, r17 ; info 19 }
	{ sw r25, r26 ; seq r15, r16, r17 ; pcnt r5, r6 }
	{ sw r25, r26 ; seq r15, r16, r17 ; srai r5, r6, 5 }
	{ sw r25, r26 ; seq r5, r6, r7 ; nor r15, r16, r17 }
	{ sw r25, r26 ; seq r5, r6, r7 ; sne r15, r16, r17 }
	{ sw r25, r26 ; seqi r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ sw r25, r26 ; seqi r15, r16, 5 ; s3a r5, r6, r7 }
	{ sw r25, r26 ; seqi r15, r16, 5 ; tblidxb3 r5, r6 }
	{ sw r25, r26 ; seqi r5, r6, 5 ; s1a r15, r16, r17 }
	{ sw r25, r26 ; seqi r5, r6, 5 }
	{ sw r25, r26 ; shl r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ sw r25, r26 ; shl r15, r16, r17 ; shr r5, r6, r7 }
	{ sw r25, r26 ; shl r5, r6, r7 ; and r15, r16, r17 }
	{ sw r25, r26 ; shl r5, r6, r7 ; shl r15, r16, r17 }
	{ sw r25, r26 ; shli r15, r16, 5 ; bitx r5, r6 }
	{ sw r25, r26 ; shli r15, r16, 5 ; mz r5, r6, r7 }
	{ sw r25, r26 ; shli r15, r16, 5 ; slte_u r5, r6, r7 }
	{ sw r25, r26 ; shli r5, r6, 5 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; shli r5, r6, 5 ; slt_u r15, r16, r17 }
	{ sw r25, r26 ; shr r15, r16, r17 ; info 19 }
	{ sw r25, r26 ; shr r15, r16, r17 ; pcnt r5, r6 }
	{ sw r25, r26 ; shr r15, r16, r17 ; srai r5, r6, 5 }
	{ sw r25, r26 ; shr r5, r6, r7 ; nor r15, r16, r17 }
	{ sw r25, r26 ; shr r5, r6, r7 ; sne r15, r16, r17 }
	{ sw r25, r26 ; shri r15, r16, 5 ; mulhh_uu r5, r6, r7 }
	{ sw r25, r26 ; shri r15, r16, 5 ; s3a r5, r6, r7 }
	{ sw r25, r26 ; shri r15, r16, 5 ; tblidxb3 r5, r6 }
	{ sw r25, r26 ; shri r5, r6, 5 ; s1a r15, r16, r17 }
	{ sw r25, r26 ; shri r5, r6, 5 }
	{ sw r25, r26 ; slt r15, r16, r17 ; mulll_uu r5, r6, r7 }
	{ sw r25, r26 ; slt r15, r16, r17 ; shr r5, r6, r7 }
	{ sw r25, r26 ; slt r5, r6, r7 ; and r15, r16, r17 }
	{ sw r25, r26 ; slt r5, r6, r7 ; shl r15, r16, r17 }
	{ sw r25, r26 ; slt_u r15, r16, r17 ; bitx r5, r6 }
	{ sw r25, r26 ; slt_u r15, r16, r17 ; mz r5, r6, r7 }
	{ sw r25, r26 ; slt_u r15, r16, r17 ; slte_u r5, r6, r7 }
	{ sw r25, r26 ; slt_u r5, r6, r7 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; slt_u r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sw r25, r26 ; slte r15, r16, r17 ; info 19 }
	{ sw r25, r26 ; slte r15, r16, r17 ; pcnt r5, r6 }
	{ sw r25, r26 ; slte r15, r16, r17 ; srai r5, r6, 5 }
	{ sw r25, r26 ; slte r5, r6, r7 ; nor r15, r16, r17 }
	{ sw r25, r26 ; slte r5, r6, r7 ; sne r15, r16, r17 }
	{ sw r25, r26 ; slte_u r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ sw r25, r26 ; slte_u r15, r16, r17 ; s3a r5, r6, r7 }
	{ sw r25, r26 ; slte_u r15, r16, r17 ; tblidxb3 r5, r6 }
	{ sw r25, r26 ; slte_u r5, r6, r7 ; s1a r15, r16, r17 }
	{ sw r25, r26 ; slte_u r5, r6, r7 }
	{ sw r25, r26 ; slti r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ sw r25, r26 ; slti r15, r16, 5 ; shr r5, r6, r7 }
	{ sw r25, r26 ; slti r5, r6, 5 ; and r15, r16, r17 }
	{ sw r25, r26 ; slti r5, r6, 5 ; shl r15, r16, r17 }
	{ sw r25, r26 ; slti_u r15, r16, 5 ; bitx r5, r6 }
	{ sw r25, r26 ; slti_u r15, r16, 5 ; mz r5, r6, r7 }
	{ sw r25, r26 ; slti_u r15, r16, 5 ; slte_u r5, r6, r7 }
	{ sw r25, r26 ; slti_u r5, r6, 5 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; slti_u r5, r6, 5 ; slt_u r15, r16, r17 }
	{ sw r25, r26 ; sne r15, r16, r17 ; info 19 }
	{ sw r25, r26 ; sne r15, r16, r17 ; pcnt r5, r6 }
	{ sw r25, r26 ; sne r15, r16, r17 ; srai r5, r6, 5 }
	{ sw r25, r26 ; sne r5, r6, r7 ; nor r15, r16, r17 }
	{ sw r25, r26 ; sne r5, r6, r7 ; sne r15, r16, r17 }
	{ sw r25, r26 ; sra r15, r16, r17 ; mulhh_uu r5, r6, r7 }
	{ sw r25, r26 ; sra r15, r16, r17 ; s3a r5, r6, r7 }
	{ sw r25, r26 ; sra r15, r16, r17 ; tblidxb3 r5, r6 }
	{ sw r25, r26 ; sra r5, r6, r7 ; s1a r15, r16, r17 }
	{ sw r25, r26 ; sra r5, r6, r7 }
	{ sw r25, r26 ; srai r15, r16, 5 ; mulll_uu r5, r6, r7 }
	{ sw r25, r26 ; srai r15, r16, 5 ; shr r5, r6, r7 }
	{ sw r25, r26 ; srai r5, r6, 5 ; and r15, r16, r17 }
	{ sw r25, r26 ; srai r5, r6, 5 ; shl r15, r16, r17 }
	{ sw r25, r26 ; sub r15, r16, r17 ; bitx r5, r6 }
	{ sw r25, r26 ; sub r15, r16, r17 ; mz r5, r6, r7 }
	{ sw r25, r26 ; sub r15, r16, r17 ; slte_u r5, r6, r7 }
	{ sw r25, r26 ; sub r5, r6, r7 ; mnz r15, r16, r17 }
	{ sw r25, r26 ; sub r5, r6, r7 ; slt_u r15, r16, r17 }
	{ sw r25, r26 ; tblidxb0 r5, r6 ; movei r15, 5 }
	{ sw r25, r26 ; tblidxb0 r5, r6 ; slte_u r15, r16, r17 }
	{ sw r25, r26 ; tblidxb1 r5, r6 ; nop }
	{ sw r25, r26 ; tblidxb1 r5, r6 ; slti_u r15, r16, 5 }
	{ sw r25, r26 ; tblidxb2 r5, r6 ; or r15, r16, r17 }
	{ sw r25, r26 ; tblidxb2 r5, r6 ; sra r15, r16, r17 }
	{ sw r25, r26 ; tblidxb3 r5, r6 ; rl r15, r16, r17 }
	{ sw r25, r26 ; tblidxb3 r5, r6 ; sub r15, r16, r17 }
	{ sw r25, r26 ; xor r15, r16, r17 ; mulhlsa_uu r5, r6, r7 }
	{ sw r25, r26 ; xor r15, r16, r17 ; shl r5, r6, r7 }
	{ sw r25, r26 ; xor r5, r6, r7 ; add r15, r16, r17 }
	{ sw r25, r26 ; xor r5, r6, r7 ; seq r15, r16, r17 }
	{ swadd r15, r16, 5 ; addbs_u r5, r6, r7 }
	{ swadd r15, r16, 5 ; crc32_8 r5, r6, r7 }
	{ swadd r15, r16, 5 ; mnzb r5, r6, r7 }
	{ swadd r15, r16, 5 ; mulhla_uu r5, r6, r7 }
	{ swadd r15, r16, 5 ; packhs r5, r6, r7 }
	{ swadd r15, r16, 5 ; shl r5, r6, r7 }
	{ swadd r15, r16, 5 ; slteh r5, r6, r7 }
	{ swadd r15, r16, 5 ; subb r5, r6, r7 }
	{ tblidxb0 r5, r6 ; add r15, r16, r17 ; prefetch r25 }
	{ tblidxb0 r5, r6 ; addih r15, r16, 5 }
	{ tblidxb0 r5, r6 ; andi r15, r16, 5 ; sb r25, r26 }
	{ tblidxb0 r5, r6 ; ill ; lb_u r25, r26 }
	{ tblidxb0 r5, r6 ; inthb r15, r16, r17 }
	{ tblidxb0 r5, r6 ; lb r25, r26 ; movei r15, 5 }
	{ tblidxb0 r5, r6 ; lb r25, r26 ; slte_u r15, r16, r17 }
	{ tblidxb0 r5, r6 ; lb_u r25, r26 ; mz r15, r16, r17 }
	{ tblidxb0 r5, r6 ; lb_u r25, r26 ; slti r15, r16, 5 }
	{ tblidxb0 r5, r6 ; lh r25, r26 ; movei r15, 5 }
	{ tblidxb0 r5, r6 ; lh r25, r26 ; slte_u r15, r16, r17 }
	{ tblidxb0 r5, r6 ; lh_u r25, r26 ; mz r15, r16, r17 }
	{ tblidxb0 r5, r6 ; lh_u r25, r26 ; slti r15, r16, 5 }
	{ tblidxb0 r5, r6 ; lw r25, r26 ; move r15, r16 }
	{ tblidxb0 r5, r6 ; lw r25, r26 ; slte r15, r16, r17 }
	{ tblidxb0 r5, r6 ; minh r15, r16, r17 }
	{ tblidxb0 r5, r6 ; move r15, r16 ; lw r25, r26 }
	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; lb_u r25, r26 }
	{ tblidxb0 r5, r6 ; nop }
	{ tblidxb0 r5, r6 ; or r15, r16, r17 }
	{ tblidxb0 r5, r6 ; prefetch r25 ; fnop }
	{ tblidxb0 r5, r6 ; prefetch r25 ; shr r15, r16, r17 }
	{ tblidxb0 r5, r6 ; rl r15, r16, r17 ; prefetch r25 }
	{ tblidxb0 r5, r6 ; s1a r15, r16, r17 ; prefetch r25 }
	{ tblidxb0 r5, r6 ; s3a r15, r16, r17 ; prefetch r25 }
	{ tblidxb0 r5, r6 ; sb r25, r26 ; ori r15, r16, 5 }
	{ tblidxb0 r5, r6 ; sb r25, r26 ; srai r15, r16, 5 }
	{ tblidxb0 r5, r6 ; seqi r15, r16, 5 ; lh_u r25, r26 }
	{ tblidxb0 r5, r6 ; sh r25, r26 ; mz r15, r16, r17 }
	{ tblidxb0 r5, r6 ; sh r25, r26 ; slti r15, r16, 5 }
	{ tblidxb0 r5, r6 ; shlh r15, r16, r17 }
	{ tblidxb0 r5, r6 ; shr r15, r16, r17 ; sh r25, r26 }
	{ tblidxb0 r5, r6 ; slt r15, r16, r17 ; lh_u r25, r26 }
	{ tblidxb0 r5, r6 ; slte r15, r16, r17 ; lb_u r25, r26 }
	{ tblidxb0 r5, r6 ; slteb_u r15, r16, r17 }
	{ tblidxb0 r5, r6 ; slti_u r15, r16, 5 ; prefetch r25 }
	{ tblidxb0 r5, r6 ; sneh r15, r16, r17 }
	{ tblidxb0 r5, r6 ; srai r15, r16, 5 ; sh r25, r26 }
	{ tblidxb0 r5, r6 ; sw r15, r16 }
	{ tblidxb0 r5, r6 ; sw r25, r26 ; s3a r15, r16, r17 }
	{ tblidxb0 r5, r6 ; tns r15, r16 }
	{ tblidxb1 r5, r6 ; add r15, r16, r17 ; sh r25, r26 }
	{ tblidxb1 r5, r6 ; addlis r15, r16, 0x1234 }
	{ tblidxb1 r5, r6 ; andi r15, r16, 5 ; sw r25, r26 }
	{ tblidxb1 r5, r6 ; ill ; lh_u r25, r26 }
	{ tblidxb1 r5, r6 ; intlb r15, r16, r17 }
	{ tblidxb1 r5, r6 ; lb r25, r26 ; nop }
	{ tblidxb1 r5, r6 ; lb r25, r26 ; slti_u r15, r16, 5 }
	{ tblidxb1 r5, r6 ; lb_u r25, r26 ; nor r15, r16, r17 }
	{ tblidxb1 r5, r6 ; lb_u r25, r26 ; sne r15, r16, r17 }
	{ tblidxb1 r5, r6 ; lh r25, r26 ; nop }
	{ tblidxb1 r5, r6 ; lh r25, r26 ; slti_u r15, r16, 5 }
	{ tblidxb1 r5, r6 ; lh_u r25, r26 ; nor r15, r16, r17 }
	{ tblidxb1 r5, r6 ; lh_u r25, r26 ; sne r15, r16, r17 }
	{ tblidxb1 r5, r6 ; lw r25, r26 ; mz r15, r16, r17 }
	{ tblidxb1 r5, r6 ; lw r25, r26 ; slti r15, r16, 5 }
	{ tblidxb1 r5, r6 ; minih r15, r16, 5 }
	{ tblidxb1 r5, r6 ; move r15, r16 ; sb r25, r26 }
	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; lh_u r25, r26 }
	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; lb_u r25, r26 }
	{ tblidxb1 r5, r6 ; ori r15, r16, 5 ; lb_u r25, r26 }
	{ tblidxb1 r5, r6 ; prefetch r25 ; info 19 }
	{ tblidxb1 r5, r6 ; prefetch r25 ; slt r15, r16, r17 }
	{ tblidxb1 r5, r6 ; rl r15, r16, r17 ; sh r25, r26 }
	{ tblidxb1 r5, r6 ; s1a r15, r16, r17 ; sh r25, r26 }
	{ tblidxb1 r5, r6 ; s3a r15, r16, r17 ; sh r25, r26 }
	{ tblidxb1 r5, r6 ; sb r25, r26 ; rli r15, r16, 5 }
	{ tblidxb1 r5, r6 ; sb r25, r26 ; xor r15, r16, r17 }
	{ tblidxb1 r5, r6 ; seqi r15, r16, 5 ; prefetch r25 }
	{ tblidxb1 r5, r6 ; sh r25, r26 ; nor r15, r16, r17 }
	{ tblidxb1 r5, r6 ; sh r25, r26 ; sne r15, r16, r17 }
	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; lb_u r25, r26 }
	{ tblidxb1 r5, r6 ; shr r15, r16, r17 }
	{ tblidxb1 r5, r6 ; slt r15, r16, r17 ; prefetch r25 }
	{ tblidxb1 r5, r6 ; slte r15, r16, r17 ; lh_u r25, r26 }
	{ tblidxb1 r5, r6 ; slteh_u r15, r16, r17 }
	{ tblidxb1 r5, r6 ; slti_u r15, r16, 5 ; sh r25, r26 }
	{ tblidxb1 r5, r6 ; sra r15, r16, r17 ; lb_u r25, r26 }
	{ tblidxb1 r5, r6 ; srai r15, r16, 5 }
	{ tblidxb1 r5, r6 ; sw r25, r26 ; addi r15, r16, 5 }
	{ tblidxb1 r5, r6 ; sw r25, r26 ; seqi r15, r16, 5 }
	{ tblidxb1 r5, r6 ; xor r15, r16, r17 ; lb r25, r26 }
	{ tblidxb2 r5, r6 ; add r15, r16, r17 }
	{ tblidxb2 r5, r6 ; and r15, r16, r17 ; lb r25, r26 }
	{ tblidxb2 r5, r6 ; auli r15, r16, 0x1234 }
	{ tblidxb2 r5, r6 ; ill ; prefetch r25 }
	{ tblidxb2 r5, r6 ; inv r15 }
	{ tblidxb2 r5, r6 ; lb r25, r26 ; or r15, r16, r17 }
	{ tblidxb2 r5, r6 ; lb r25, r26 ; sra r15, r16, r17 }
	{ tblidxb2 r5, r6 ; lb_u r25, r26 ; ori r15, r16, 5 }
	{ tblidxb2 r5, r6 ; lb_u r25, r26 ; srai r15, r16, 5 }
	{ tblidxb2 r5, r6 ; lh r25, r26 ; or r15, r16, r17 }
	{ tblidxb2 r5, r6 ; lh r25, r26 ; sra r15, r16, r17 }
	{ tblidxb2 r5, r6 ; lh_u r25, r26 ; ori r15, r16, 5 }
	{ tblidxb2 r5, r6 ; lh_u r25, r26 ; srai r15, r16, 5 }
	{ tblidxb2 r5, r6 ; lw r25, r26 ; nor r15, r16, r17 }
	{ tblidxb2 r5, r6 ; lw r25, r26 ; sne r15, r16, r17 }
	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; lb r25, r26 }
	{ tblidxb2 r5, r6 ; move r15, r16 ; sw r25, r26 }
	{ tblidxb2 r5, r6 ; mz r15, r16, r17 ; prefetch r25 }
	{ tblidxb2 r5, r6 ; nor r15, r16, r17 ; lh_u r25, r26 }
	{ tblidxb2 r5, r6 ; ori r15, r16, 5 ; lh_u r25, r26 }
	{ tblidxb2 r5, r6 ; prefetch r25 ; move r15, r16 }
	{ tblidxb2 r5, r6 ; prefetch r25 ; slte r15, r16, r17 }
	{ tblidxb2 r5, r6 ; rl r15, r16, r17 }
	{ tblidxb2 r5, r6 ; s1a r15, r16, r17 }
	{ tblidxb2 r5, r6 ; s3a r15, r16, r17 }
	{ tblidxb2 r5, r6 ; sb r25, r26 ; s2a r15, r16, r17 }
	{ tblidxb2 r5, r6 ; sbadd r15, r16, 5 }
	{ tblidxb2 r5, r6 ; seqi r15, r16, 5 ; sh r25, r26 }
	{ tblidxb2 r5, r6 ; sh r25, r26 ; ori r15, r16, 5 }
	{ tblidxb2 r5, r6 ; sh r25, r26 ; srai r15, r16, 5 }
	{ tblidxb2 r5, r6 ; shli r15, r16, 5 ; lh_u r25, r26 }
	{ tblidxb2 r5, r6 ; shrh r15, r16, r17 }
	{ tblidxb2 r5, r6 ; slt r15, r16, r17 ; sh r25, r26 }
	{ tblidxb2 r5, r6 ; slte r15, r16, r17 ; prefetch r25 }
	{ tblidxb2 r5, r6 ; slth_u r15, r16, r17 }
	{ tblidxb2 r5, r6 ; slti_u r15, r16, 5 }
	{ tblidxb2 r5, r6 ; sra r15, r16, r17 ; lh_u r25, r26 }
	{ tblidxb2 r5, r6 ; sraih r15, r16, 5 }
	{ tblidxb2 r5, r6 ; sw r25, r26 ; andi r15, r16, 5 }
	{ tblidxb2 r5, r6 ; sw r25, r26 ; shli r15, r16, 5 }
	{ tblidxb2 r5, r6 ; xor r15, r16, r17 ; lh r25, r26 }
	{ tblidxb3 r5, r6 ; addbs_u r15, r16, r17 }
	{ tblidxb3 r5, r6 ; and r15, r16, r17 ; lh r25, r26 }
	{ tblidxb3 r5, r6 ; finv r15 }
	{ tblidxb3 r5, r6 ; ill ; sh r25, r26 }
	{ tblidxb3 r5, r6 ; jalr r15 }
	{ tblidxb3 r5, r6 ; lb r25, r26 ; rl r15, r16, r17 }
	{ tblidxb3 r5, r6 ; lb r25, r26 ; sub r15, r16, r17 }
	{ tblidxb3 r5, r6 ; lb_u r25, r26 ; rli r15, r16, 5 }
	{ tblidxb3 r5, r6 ; lb_u r25, r26 ; xor r15, r16, r17 }
	{ tblidxb3 r5, r6 ; lh r25, r26 ; rl r15, r16, r17 }
	{ tblidxb3 r5, r6 ; lh r25, r26 ; sub r15, r16, r17 }
	{ tblidxb3 r5, r6 ; lh_u r25, r26 ; rli r15, r16, 5 }
	{ tblidxb3 r5, r6 ; lh_u r25, r26 ; xor r15, r16, r17 }
	{ tblidxb3 r5, r6 ; lw r25, r26 ; ori r15, r16, 5 }
	{ tblidxb3 r5, r6 ; lw r25, r26 ; srai r15, r16, 5 }
	{ tblidxb3 r5, r6 ; mnz r15, r16, r17 ; lh r25, r26 }
	{ tblidxb3 r5, r6 ; movei r15, 5 ; lb r25, r26 }
	{ tblidxb3 r5, r6 ; mz r15, r16, r17 ; sh r25, r26 }
	{ tblidxb3 r5, r6 ; nor r15, r16, r17 ; prefetch r25 }
	{ tblidxb3 r5, r6 ; ori r15, r16, 5 ; prefetch r25 }
	{ tblidxb3 r5, r6 ; prefetch r25 ; mz r15, r16, r17 }
	{ tblidxb3 r5, r6 ; prefetch r25 ; slti r15, r16, 5 }
	{ tblidxb3 r5, r6 ; rli r15, r16, 5 ; lb_u r25, r26 }
	{ tblidxb3 r5, r6 ; s2a r15, r16, r17 ; lb_u r25, r26 }
	{ tblidxb3 r5, r6 ; sb r25, r26 ; add r15, r16, r17 }
	{ tblidxb3 r5, r6 ; sb r25, r26 ; seq r15, r16, r17 }
	{ tblidxb3 r5, r6 ; seq r15, r16, r17 ; lb_u r25, r26 }
	{ tblidxb3 r5, r6 ; seqi r15, r16, 5 }
	{ tblidxb3 r5, r6 ; sh r25, r26 ; rli r15, r16, 5 }
	{ tblidxb3 r5, r6 ; sh r25, r26 ; xor r15, r16, r17 }
	{ tblidxb3 r5, r6 ; shli r15, r16, 5 ; prefetch r25 }
	{ tblidxb3 r5, r6 ; shri r15, r16, 5 ; lb_u r25, r26 }
	{ tblidxb3 r5, r6 ; slt r15, r16, r17 }
	{ tblidxb3 r5, r6 ; slte r15, r16, r17 ; sh r25, r26 }
	{ tblidxb3 r5, r6 ; slti r15, r16, 5 ; lb_u r25, r26 }
	{ tblidxb3 r5, r6 ; sltib_u r15, r16, 5 }
	{ tblidxb3 r5, r6 ; sra r15, r16, r17 ; prefetch r25 }
	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; lb_u r25, r26 }
	{ tblidxb3 r5, r6 ; sw r25, r26 ; ill }
	{ tblidxb3 r5, r6 ; sw r25, r26 ; shri r15, r16, 5 }
	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; lw r25, r26 }
	{ tns r15, r16 ; and r5, r6, r7 }
	{ tns r15, r16 ; maxh r5, r6, r7 }
	{ tns r15, r16 ; mulhha_uu r5, r6, r7 }
	{ tns r15, r16 ; mz r5, r6, r7 }
	{ tns r15, r16 ; sadb_u r5, r6, r7 }
	{ tns r15, r16 ; shrih r5, r6, 5 }
	{ tns r15, r16 ; sneb r5, r6, r7 }
	{ wh64 r15 ; add r5, r6, r7 }
	{ wh64 r15 ; clz r5, r6 }
	{ wh64 r15 ; mm r5, r6, r7, 5, 7 }
	{ wh64 r15 ; mulhla_su r5, r6, r7 }
	{ wh64 r15 ; packbs_u r5, r6, r7 }
	{ wh64 r15 ; seqib r5, r6, 5 }
	{ wh64 r15 ; slteb r5, r6, r7 }
	{ wh64 r15 ; sraih r5, r6, 5 }
	{ xor r15, r16, r17 ; add r5, r6, r7 ; sh r25, r26 }
	{ xor r15, r16, r17 ; addlis r5, r6, 0x1234 }
	{ xor r15, r16, r17 ; andi r5, r6, 5 ; sb r25, r26 }
	{ xor r15, r16, r17 ; bytex r5, r6 ; lh_u r25, r26 }
	{ xor r15, r16, r17 ; ctz r5, r6 ; lb_u r25, r26 }
	{ xor r15, r16, r17 ; info 19 ; lb r25, r26 }
	{ xor r15, r16, r17 ; lb r25, r26 ; bytex r5, r6 }
	{ xor r15, r16, r17 ; lb r25, r26 ; nop }
	{ xor r15, r16, r17 ; lb r25, r26 ; slti r5, r6, 5 }
	{ xor r15, r16, r17 ; lb_u r25, r26 ; fnop }
	{ xor r15, r16, r17 ; lb_u r25, r26 ; ori r5, r6, 5 }
	{ xor r15, r16, r17 ; lb_u r25, r26 ; sra r5, r6, r7 }
	{ xor r15, r16, r17 ; lh r25, r26 ; move r5, r6 }
	{ xor r15, r16, r17 ; lh r25, r26 ; rli r5, r6, 5 }
	{ xor r15, r16, r17 ; lh r25, r26 ; tblidxb0 r5, r6 }
	{ xor r15, r16, r17 ; lh_u r25, r26 ; mulhh_uu r5, r6, r7 }
	{ xor r15, r16, r17 ; lh_u r25, r26 ; s3a r5, r6, r7 }
	{ xor r15, r16, r17 ; lh_u r25, r26 ; tblidxb3 r5, r6 }
	{ xor r15, r16, r17 ; lw r25, r26 ; mulhlsa_uu r5, r6, r7 }
	{ xor r15, r16, r17 ; lw r25, r26 ; shl r5, r6, r7 }
	{ xor r15, r16, r17 ; maxb_u r5, r6, r7 }
	{ xor r15, r16, r17 ; mnzh r5, r6, r7 }
	{ xor r15, r16, r17 ; movei r5, 5 }
	{ xor r15, r16, r17 ; mulhh_uu r5, r6, r7 ; sb r25, r26 }
	{ xor r15, r16, r17 ; mulhha_uu r5, r6, r7 ; prefetch r25 }
	{ xor r15, r16, r17 ; mulhlsa_uu r5, r6, r7 ; sb r25, r26 }
	{ xor r15, r16, r17 ; mulll_uu r5, r6, r7 ; prefetch r25 }
	{ xor r15, r16, r17 ; mullla_uu r5, r6, r7 ; lw r25, r26 }
	{ xor r15, r16, r17 ; mvz r5, r6, r7 ; lh_u r25, r26 }
	{ xor r15, r16, r17 ; nop ; lb_u r25, r26 }
	{ xor r15, r16, r17 ; or r5, r6, r7 ; lb_u r25, r26 }
	{ xor r15, r16, r17 ; packhb r5, r6, r7 }
	{ xor r15, r16, r17 ; prefetch r25 ; ctz r5, r6 }
	{ xor r15, r16, r17 ; prefetch r25 ; or r5, r6, r7 }
	{ xor r15, r16, r17 ; prefetch r25 ; sne r5, r6, r7 }
	{ xor r15, r16, r17 ; rli r5, r6, 5 ; lb r25, r26 }
	{ xor r15, r16, r17 ; s2a r5, r6, r7 ; lb r25, r26 }
	{ xor r15, r16, r17 ; sadab_u r5, r6, r7 }
	{ xor r15, r16, r17 ; sb r25, r26 ; mulhh_uu r5, r6, r7 }
	{ xor r15, r16, r17 ; sb r25, r26 ; s3a r5, r6, r7 }
	{ xor r15, r16, r17 ; sb r25, r26 ; tblidxb3 r5, r6 }
	{ xor r15, r16, r17 ; seqi r5, r6, 5 ; prefetch r25 }
	{ xor r15, r16, r17 ; sh r25, r26 ; mulhh_ss r5, r6, r7 }
	{ xor r15, r16, r17 ; sh r25, r26 ; s2a r5, r6, r7 }
	{ xor r15, r16, r17 ; sh r25, r26 ; tblidxb2 r5, r6 }
	{ xor r15, r16, r17 ; shli r5, r6, 5 ; lw r25, r26 }
	{ xor r15, r16, r17 ; shri r5, r6, 5 ; lb r25, r26 }
	{ xor r15, r16, r17 ; slt r5, r6, r7 ; sw r25, r26 }
	{ xor r15, r16, r17 ; slte r5, r6, r7 ; sb r25, r26 }
	{ xor r15, r16, r17 ; slti r5, r6, 5 ; lb r25, r26 }
	{ xor r15, r16, r17 ; sltib r5, r6, 5 }
	{ xor r15, r16, r17 ; sra r5, r6, r7 ; lw r25, r26 }
	{ xor r15, r16, r17 ; sub r5, r6, r7 ; lb r25, r26 }
	{ xor r15, r16, r17 ; sw r25, r26 ; bytex r5, r6 }
	{ xor r15, r16, r17 ; sw r25, r26 ; nop }
	{ xor r15, r16, r17 ; sw r25, r26 ; slti r5, r6, 5 }
	{ xor r15, r16, r17 ; tblidxb0 r5, r6 ; sw r25, r26 }
	{ xor r15, r16, r17 ; tblidxb2 r5, r6 ; sw r25, r26 }
	{ xor r15, r16, r17 ; xor r5, r6, r7 ; sw r25, r26 }
	{ xor r5, r6, r7 ; addi r15, r16, 5 ; lh_u r25, r26 }
	{ xor r5, r6, r7 ; and r15, r16, r17 ; sw r25, r26 }
	{ xor r5, r6, r7 ; fnop ; lw r25, r26 }
	{ xor r5, r6, r7 ; info 19 ; lh_u r25, r26 }
	{ xor r5, r6, r7 ; lb r25, r26 ; addi r15, r16, 5 }
	{ xor r5, r6, r7 ; lb r25, r26 ; seqi r15, r16, 5 }
	{ xor r5, r6, r7 ; lb_u r25, r26 ; and r15, r16, r17 }
	{ xor r5, r6, r7 ; lb_u r25, r26 ; shl r15, r16, r17 }
	{ xor r5, r6, r7 ; lh r25, r26 ; addi r15, r16, 5 }
	{ xor r5, r6, r7 ; lh r25, r26 ; seqi r15, r16, 5 }
	{ xor r5, r6, r7 ; lh_u r25, r26 ; and r15, r16, r17 }
	{ xor r5, r6, r7 ; lh_u r25, r26 ; shl r15, r16, r17 }
	{ xor r5, r6, r7 ; lw r25, r26 ; add r15, r16, r17 }
	{ xor r5, r6, r7 ; lw r25, r26 ; seq r15, r16, r17 }
	{ xor r5, r6, r7 ; lwadd_na r15, r16, 5 }
	{ xor r5, r6, r7 ; mnz r15, r16, r17 ; sw r25, r26 }
	{ xor r5, r6, r7 ; movei r15, 5 ; sb r25, r26 }
	{ xor r5, r6, r7 ; nop ; lb_u r25, r26 }
	{ xor r5, r6, r7 ; or r15, r16, r17 ; lb_u r25, r26 }
	{ xor r5, r6, r7 ; packhb r15, r16, r17 }
	{ xor r5, r6, r7 ; prefetch r25 ; rli r15, r16, 5 }
	{ xor r5, r6, r7 ; prefetch r25 ; xor r15, r16, r17 }
	{ xor r5, r6, r7 ; rli r15, r16, 5 ; sh r25, r26 }
	{ xor r5, r6, r7 ; s2a r15, r16, r17 ; sh r25, r26 }
	{ xor r5, r6, r7 ; sb r25, r26 ; info 19 }
	{ xor r5, r6, r7 ; sb r25, r26 ; slt r15, r16, r17 }
	{ xor r5, r6, r7 ; seq r15, r16, r17 ; sh r25, r26 }
	{ xor r5, r6, r7 ; sh r25, r26 ; and r15, r16, r17 }
	{ xor r5, r6, r7 ; sh r25, r26 ; shl r15, r16, r17 }
	{ xor r5, r6, r7 ; shl r15, r16, r17 ; lh_u r25, r26 }
	{ xor r5, r6, r7 ; shlih r15, r16, 5 }
	{ xor r5, r6, r7 ; shri r15, r16, 5 ; sh r25, r26 }
	{ xor r5, r6, r7 ; slt_u r15, r16, r17 ; prefetch r25 }
	{ xor r5, r6, r7 ; slte_u r15, r16, r17 ; lh_u r25, r26 }
	{ xor r5, r6, r7 ; slti r15, r16, 5 ; sh r25, r26 }
	{ xor r5, r6, r7 ; sne r15, r16, r17 ; lh_u r25, r26 }
	{ xor r5, r6, r7 ; srah r15, r16, r17 }
	{ xor r5, r6, r7 ; sub r15, r16, r17 ; sh r25, r26 }
	{ xor r5, r6, r7 ; sw r25, r26 ; nop }
	{ xor r5, r6, r7 ; sw r25, r26 ; slti_u r15, r16, 5 }
	{ xor r5, r6, r7 ; xori r15, r16, 5 }
	{ xori r15, r16, 5 ; bytex r5, r6 }
	{ xori r15, r16, 5 ; minih r5, r6, 5 }
	{ xori r15, r16, 5 ; mulhla_ss r5, r6, r7 }
	{ xori r15, r16, 5 ; ori r5, r6, 5 }
	{ xori r15, r16, 5 ; seqi r5, r6, 5 }
	{ xori r15, r16, 5 ; slte_u r5, r6, r7 }
	{ xori r15, r16, 5 ; sraib r5, r6, 5 }
	{ xori r5, r6, 5 ; addib r15, r16, 5 }
	{ xori r5, r6, 5 ; inv r15 }
	{ xori r5, r6, 5 ; maxh r15, r16, r17 }
	{ xori r5, r6, 5 ; mzh r15, r16, r17 }
	{ xori r5, r6, 5 ; seqh r15, r16, r17 }
	{ xori r5, r6, 5 ; sltb r15, r16, r17 }
	{ xori r5, r6, 5 ; srab r15, r16, r17 }
