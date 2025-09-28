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

	{ fdouble_sub_flags r5, r6, r7 ; bnezt r15, target }
	{ fdouble_sub_flags r5, r6, r7 ; bnez r15, target }
	{ fdouble_addsub r5, r6, r7 ; bnez r15, target }
	{ fdouble_pack1 r5, r6, r7 ; bnez r15, target }
	{ fsingle_pack2 r5, r6, r7 ; bnez r15, target }
	{ fsingle_mul2 r5, r6, r7 ; blez r15, target }
	{ mula_hs_hu r5, r6, r7 ; bgtzt r15, target }
	{ mula_hu_lu r5, r6, r7 ; bgtzt r15, target }
	{ addli r5, r6, 0x1234 ; bgtzt r15, target }
	{ fsingle_pack1 r5, r6 ; beqzt r15, target }
	{ mul_hu_hu r5, r6, r7 ; beqzt r15, target }
	{ mul_lu_lu r5, r6, r7 ; beqzt r15, target }
	{ mula_hu_hu r5, r6, r7 ; beqz r15, target }
	{ mula_lu_lu r5, r6, r7 ; beqz r15, target }
	{ addli r5, r6, 0x1234 ; beqz r15, target }
	{ dblalign2 r5, r6, r7 ; beqz r15, target }
	{ mul_hs_hs r5, r6, r7 ; blbs r15, target }
	{ mul_hu_ls r5, r6, r7 ; blbs r15, target }
	{ shl1addx r5, r6, r7 ; blbst r15, target }
	{ v1cmpleu r5, r6, r7 ; blbst r15, target }
	{ v1ddotpu r5, r6, r7 ; blbst r15, target }
	{ v1dotpusa r5, r6, r7 ; blbs r15, target }
	{ v2cmpltsi r5, r6, 5 ; blbst r15, target }
	{ v4packsc r5, r6, r7 ; blbst r15, target }
	{ cmovnez r5, r6, r7 ; blbst r15, target }
	{ shl1addx r5, r6, r7 ; bgtz r15, target }
	{ v1adduc r5, r6, r7 ; bgtzt r15, target }
	{ v1cmpleu r5, r6, r7 ; bgtz r15, target }
	{ v1cmpne r5, r6, r7 ; bgtzt r15, target }
	{ v1dotpus r5, r6, r7 ; bgtz r15, target }
	{ v1sadau r5, r6, r7 ; bgtzt r15, target }
	{ v2cmpeqi r5, r6, 5 ; bgtzt r15, target }
	{ v2cmpltu r5, r6, r7 ; bgtz r15, target }
	{ v2int_l r5, r6, r7 ; bgtzt r15, target }
	{ v2packuc r5, r6, r7 ; bgtz r15, target }
	{ v4addsc r5, r6, r7 ; bgtzt r15, target }
	{ v4subsc r5, r6, r7 ; bgtzt r15, target }
	{ cmples r5, r6, r7 ; bgtzt r15, target }
	{ cmpltui r5, r6, 5 ; bgtzt r15, target }
	{ fsingle_addsub2 r5, r6, r7 ; j target }
	{ subxsc r5, r6, r7 ; bltzt r15, target }
	{ v1cmpne r5, r6, r7 ; bltz r15, target }
	{ v1int_l r5, r6, r7 ; bltz r15, target }
	{ v1multu r5, r6, r7 ; bltz r15, target }
	{ v1shrs r5, r6, r7 ; bltzt r15, target }
	{ v2addsc r5, r6, r7 ; bltz r15, target }
	{ v2dotp r5, r6, r7 ; bltzt r15, target }
	{ v2maxsi r5, r6, 5 ; bltzt r15, target }
	{ v2packh r5, r6, r7 ; bltz r15, target }
	{ v2sadu r5, r6, r7 ; bltzt r15, target }
	{ v2shrui r5, r6, 5 ; bltzt r15, target }
	{ v4shlsc r5, r6, r7 ; bltz r15, target }
	{ cmpeq r5, r6, r7 ; bltzt r15, target }
	{ cmpltsi r5, r6, 5 ; bltz r15, target }
	{ cmulaf r5, r6, r7 ; bltz r15, target }
	{ moveli r5, 0x1234 ; bgez r15, target }
	{ subxsc r5, r6, r7 ; bnez r15, target }
	{ v1maxu r5, r6, r7 ; bnez r15, target }
	{ v1mulu r5, r6, r7 ; bnez r15, target }
	{ v1shrsi r5, r6, 5 ; bnez r15, target }
	{ v2addi r5, r6, 5 ; bnezt r15, target }
	{ v2mins r5, r6, r7 ; bnez r15, target }
	{ v2sadu r5, r6, r7 ; bnez r15, target }
	{ v2shru r5, r6, r7 ; bnez r15, target }
	{ v4shrs r5, r6, r7 ; bnez r15, target }
	{ cmpeq r5, r6, r7 ; bnez r15, target }
	{ cmulf r5, r6, r7 ; bnez r15, target }
	{ revbytes r5, r6 ; blbst r15, target }
	{ shrs r5, r6, r7 ; blbst r15, target }
	{ shruxi r5, r6, 5 ; blbs r15, target }
	{ tblidxb3 r5, r6 ; blbst r15, target }
	{ v1shl r5, r6, r7 ; blbs r15, target }
	{ v2mnz r5, r6, r7 ; blbs r15, target }
	{ v4add r5, r6, r7 ; blbs r15, target }
	{ addx r5, r6, r7 ; blbs r15, target }
	{ fsingle_sub1 r5, r6, r7 ; j target }
	{ nor r5, r6, r7 ; blezt r15, target }
	{ shl r5, r6, r7 ; blezt r15, target }
	{ shrsi r5, r6, 5 ; blez r15, target }
	{ tblidxb0 r5, r6 ; blbs r15, target }
	{ v2mz r5, r6, r7 ; blbc r15, target }
	{ and r5, r6, r7 ; bgtz r15, target }
	{ mz r5, r6, r7 ; blbst r15, target }
	{ shl r5, r6, r7 ; blbs r15, target }
	{ bfexts r5, r6, 5, 7 ; jal target }
	{ ori r5, r6, 5 ; bgtz r15, target }
	{ infol 0x1234 ; bgez r15, target }
	{ pcnt r5, r6 ; bnezt r15, target }
	{ bfextu r5, r6, 5, 7 ; j target }
	{ movei r5, 5 ; blbs r15, target }
	{ v2avgs r5, r6, r7 ; jal target }
	{ cmulh r5, r6, r7 ; jal target }
	{ v2dotpa r5, r6, r7 ; j target }
	{ rotli r5, r6, 5 ; jal target }
	{ v4shrs r5, r6, r7 ; j target }
	{ v2sub r5, r6, r7 ; j target }
	{ and r5, r6, r7 ; j target }
	{ nop ; blbst r15, target }
	{ beqzt r15, target ; cmpltu r5, r6, r7 }
	{ beqzt r15, target ; mul_hs_hs r5, r6, r7 }
	{ beqzt r15, target ; shli r5, r6, 5 }
	{ beqzt r15, target ; v1dotpusa r5, r6, r7 }
	{ beqzt r15, target ; v2maxs r5, r6, r7 }
	{ bgezt r15, target ; addli r5, r6, 0x1234 }
	{ bgezt r15, target ; fdouble_pack2 r5, r6, r7 }
	{ bgezt r15, target ; mulx r5, r6, r7 }
	{ bgezt r15, target ; v1avgu r5, r6, r7 }
	{ bgezt r15, target ; v1subuc r5, r6, r7 }
	{ bgezt r15, target ; v2shru r5, r6, r7 }
	{ bgtzt r15, target ; cmpne r5, r6, r7 }
	{ bgtzt r15, target ; mul_hs_ls r5, r6, r7 }
	{ bgtzt r15, target ; shlxi r5, r6, 5 }
	{ bgtzt r15, target ; v1int_l r5, r6, r7 }
	{ bgtzt r15, target ; v2mins r5, r6, r7 }
	{ blbct r15, target ; addxi r5, r6, 5 }
	{ blbct r15, target ; fdouble_unpack_max r5, r6, r7 }
	{ blbct r15, target ; nop }
	{ blbct r15, target ; v1cmpeqi r5, r6, 5 }
	{ blbct r15, target ; v2addi r5, r6, 5 }
	{ blbct r15, target ; v2sub r5, r6, r7 }
	{ blbst r15, target ; cmula r5, r6, r7 }
	{ blbst r15, target ; mul_hu_hu r5, r6, r7 }
	{ blbst r15, target ; shrsi r5, r6, 5 }
	{ blbst r15, target ; v1maxui r5, r6, 5 }
	{ blbst r15, target ; v2mnz r5, r6, r7 }
	{ blezt r15, target ; addxsc r5, r6, r7 }
	{ blezt r15, target ; fnop }
	{ blezt r15, target ; or r5, r6, r7 }
	{ blezt r15, target ; v1cmpleu r5, r6, r7 }
	{ blezt r15, target ; v2adiffs r5, r6, r7 }
	{ blezt r15, target ; v4add r5, r6, r7 }
	{ bltzt r15, target ; cmulf r5, r6, r7 }
	{ bltzt r15, target ; mul_hu_lu r5, r6, r7 }
	{ bltzt r15, target ; shrui r5, r6, 5 }
	{ bltzt r15, target ; v1minui r5, r6, 5 }
	{ bltzt r15, target ; v2muls r5, r6, r7 }
	{ bnezt r15, target ; andi r5, r6, 5 }
	{ bnezt r15, target ; fsingle_addsub2 r5, r6, r7 }
	{ bnezt r15, target ; pcnt r5, r6 }
	{ bnezt r15, target ; v1cmpltsi r5, r6, 5 }
	{ bnezt r15, target ; v2cmpeq r5, r6, r7 }
	{ bnezt r15, target ; v4int_h r5, r6, r7 }
	{ beqz r15, target ; cmulfr r5, r6, r7 }
	{ beqz r15, target ; mul_ls_ls r5, r6, r7 }
	{ beqz r15, target ; shrux r5, r6, r7 }
	{ beqz r15, target ; v1mnz r5, r6, r7 }
	{ beqz r15, target ; v2mults r5, r6, r7 }
	{ bgez r15, target ; bfexts r5, r6, 5, 7 }
	{ bgez r15, target ; fsingle_mul1 r5, r6, r7 }
	{ bgez r15, target ; revbits r5, r6 }
	{ bgez r15, target ; v1cmpltu r5, r6, r7 }
	{ bgez r15, target ; v2cmpeqi r5, r6, 5 }
	{ bgez r15, target ; v4int_l r5, r6, r7 }
	{ bgtz r15, target ; cmulhr r5, r6, r7 }
	{ bgtz r15, target ; mul_lu_lu r5, r6, r7 }
	{ bgtz r15, target ; shufflebytes r5, r6, r7 }
	{ bgtz r15, target ; v1mulu r5, r6, r7 }
	{ bgtz r15, target ; v2packh r5, r6, r7 }
	{ blbc r15, target ; bfins r5, r6, 5, 7 }
	{ blbc r15, target ; fsingle_pack1 r5, r6 }
	{ blbc r15, target ; rotl r5, r6, r7 }
	{ blbc r15, target ; v1cmpne r5, r6, r7 }
	{ blbc r15, target ; v2cmpleu r5, r6, r7 }
	{ blbc r15, target ; v4shl r5, r6, r7 }
	{ blbs r15, target ; crc32_8 r5, r6, r7 }
	{ blbs r15, target ; mula_hs_hu r5, r6, r7 }
	{ blbs r15, target ; subx r5, r6, r7 }
	{ blbs r15, target ; v1mz r5, r6, r7 }
	{ blbs r15, target ; v2packuc r5, r6, r7 }
	{ blez r15, target ; cmoveqz r5, r6, r7 }
	{ blez r15, target ; fsingle_sub1 r5, r6, r7 }
	{ blez r15, target ; shl r5, r6, r7 }
	{ blez r15, target ; v1ddotpua r5, r6, r7 }
	{ blez r15, target ; v2cmpltsi r5, r6, 5 }
	{ blez r15, target ; v4shrs r5, r6, r7 }
	{ bltz r15, target ; dblalign r5, r6, r7 }
	{ bltz r15, target ; mula_hs_lu r5, r6, r7 }
	{ bltz r15, target ; tblidxb0 r5, r6 }
	{ bltz r15, target ; v1sadu r5, r6, r7 }
	{ bltz r15, target ; v2sadau r5, r6, r7 }
	{ bnez r15, target ; cmpeq r5, r6, r7 }
	{ bnez r15, target ; infol 0x1234 }
	{ bnez r15, target ; shl1add r5, r6, r7 }
	{ bnez r15, target ; v1ddotpusa r5, r6, r7 }
	{ bnez r15, target ; v2cmpltui r5, r6, 5 }
	{ bnez r15, target ; v4sub r5, r6, r7 }
	{ jal target ; cmples r5, r6, r7 }
	{ jal target ; mnz r5, r6, r7 }
	{ jal target ; shl2add r5, r6, r7 }
	{ jal target ; v1dotpa r5, r6, r7 }
	{ jal target ; v2dotp r5, r6, r7 }
	{ jal target ; xor r5, r6, r7 }
	{ j target ; dblalign6 r5, r6, r7 }
	{ j target ; mula_hu_lu r5, r6, r7 }
	{ j target ; tblidxb3 r5, r6 }
	{ j target ; v1shrs r5, r6, r7 }
	{ j target ; v2shl r5, r6, r7 }
	cmpeqi r5, r6, 5
	fetchand r5, r6, r7
	ldna_add r5, r6, 5
	mula_hu_lu r5, r6, r7
	shlx r5, r6, r7
	v1avgu r5, r6, r7
	v1subuc r5, r6, r7
	v2shru r5, r6, r7
	{ add r15, r16, r17 ; addi r5, r6, 5 ; ld2s r25, r26 }
	{ add r15, r16, r17 ; addxi r5, r6, 5 ; ld2u r25, r26 }
	{ add r15, r16, r17 ; andi r5, r6, 5 ; ld2u r25, r26 }
	{ add r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld2s r25, r26 }
	{ add r15, r16, r17 ; cmpeq r5, r6, r7 ; ld4s r25, r26 }
	{ add r15, r16, r17 ; cmples r5, r6, r7 ; prefetch r25 }
	{ add r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l1_fault r25 }
	{ add r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l2_fault r25 }
	{ add r15, r16, r17 ; ctz r5, r6 ; ld2s r25, r26 }
	{ add r15, r16, r17 ; fnop ; prefetch_l3 r25 }
	{ add r15, r16, r17 ; info 19 ; prefetch_l1 r25 }
	{ add r15, r16, r17 ; ld r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ add r15, r16, r17 ; ld1s r25, r26 ; andi r5, r6, 5 }
	{ add r15, r16, r17 ; ld1s r25, r26 ; shl1addx r5, r6, r7 }
	{ add r15, r16, r17 ; ld1u r25, r26 ; move r5, r6 }
	{ add r15, r16, r17 ; ld1u r25, r26 }
	{ add r15, r16, r17 ; ld2s r25, r26 ; revbits r5, r6 }
	{ add r15, r16, r17 ; ld2u r25, r26 ; cmpne r5, r6, r7 }
	{ add r15, r16, r17 ; ld2u r25, r26 ; subx r5, r6, r7 }
	{ add r15, r16, r17 ; ld4s r25, r26 ; mulx r5, r6, r7 }
	{ add r15, r16, r17 ; ld4u r25, r26 ; cmpeqi r5, r6, 5 }
	{ add r15, r16, r17 ; ld4u r25, r26 ; shli r5, r6, 5 }
	{ add r15, r16, r17 ; move r5, r6 ; prefetch r25 }
	{ add r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ add r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ add r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; ld4u r25, r26 }
	{ add r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld2s r25, r26 }
	{ add r15, r16, r17 ; mulax r5, r6, r7 ; ld2u r25, r26 }
	{ add r15, r16, r17 ; mz r5, r6, r7 ; ld4u r25, r26 }
	{ add r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l1 r25 }
	{ add r15, r16, r17 ; pcnt r5, r6 ; prefetch_l1_fault r25 }
	{ add r15, r16, r17 ; prefetch r25 ; mula_ls_ls r5, r6, r7 }
	{ add r15, r16, r17 ; prefetch_l1 r25 ; cmoveqz r5, r6, r7 }
	{ add r15, r16, r17 ; prefetch_l1 r25 ; shl2addx r5, r6, r7 }
	{ add r15, r16, r17 ; prefetch_l1_fault r25 ; mul_hs_hs r5, r6, r7 }
	{ add r15, r16, r17 ; prefetch_l2 r25 ; addi r5, r6, 5 }
	{ add r15, r16, r17 ; prefetch_l2 r25 ; rotl r5, r6, r7 }
	{ add r15, r16, r17 ; prefetch_l2_fault r25 ; fnop }
	{ add r15, r16, r17 ; prefetch_l2_fault r25 ; tblidxb1 r5, r6 }
	{ add r15, r16, r17 ; prefetch_l3 r25 ; nop }
	{ add r15, r16, r17 ; prefetch_l3_fault r25 ; cmpleu r5, r6, r7 }
	{ add r15, r16, r17 ; prefetch_l3_fault r25 ; shrsi r5, r6, 5 }
	{ add r15, r16, r17 ; revbytes r5, r6 ; prefetch_l2 r25 }
	{ add r15, r16, r17 ; rotli r5, r6, 5 ; prefetch_l3 r25 }
	{ add r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l3_fault r25 }
	{ add r15, r16, r17 ; shl2add r5, r6, r7 ; st1 r25, r26 }
	{ add r15, r16, r17 ; shl3add r5, r6, r7 ; st4 r25, r26 }
	{ add r15, r16, r17 ; shlx r5, r6, r7 }
	{ add r15, r16, r17 ; shru r5, r6, r7 ; ld r25, r26 }
	{ add r15, r16, r17 ; shufflebytes r5, r6, r7 }
	{ add r15, r16, r17 ; st r25, r26 ; revbits r5, r6 }
	{ add r15, r16, r17 ; st1 r25, r26 ; cmpne r5, r6, r7 }
	{ add r15, r16, r17 ; st1 r25, r26 ; subx r5, r6, r7 }
	{ add r15, r16, r17 ; st2 r25, r26 ; mulx r5, r6, r7 }
	{ add r15, r16, r17 ; st4 r25, r26 ; cmpeqi r5, r6, 5 }
	{ add r15, r16, r17 ; st4 r25, r26 ; shli r5, r6, 5 }
	{ add r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l1 r25 }
	{ add r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l1_fault r25 }
	{ add r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l2_fault r25 }
	{ add r15, r16, r17 ; v1mulu r5, r6, r7 }
	{ add r15, r16, r17 ; v2packh r5, r6, r7 }
	{ add r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
	{ add r5, r6, r7 ; addi r15, r16, 5 ; st r25, r26 }
	{ add r5, r6, r7 ; addxi r15, r16, 5 ; st1 r25, r26 }
	{ add r5, r6, r7 ; andi r15, r16, 5 ; st1 r25, r26 }
	{ add r5, r6, r7 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
	{ add r5, r6, r7 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
	{ add r5, r6, r7 ; cmpltu r15, r16, r17 ; ld r25, r26 }
	{ add r5, r6, r7 ; dblalign4 r15, r16, r17 }
	{ add r5, r6, r7 ; ill ; ld2u r25, r26 }
	{ add r5, r6, r7 ; jalr r15 ; ld2s r25, r26 }
	{ add r5, r6, r7 ; jr r15 ; ld4s r25, r26 }
	{ add r5, r6, r7 ; ld r25, r26 ; cmpeq r15, r16, r17 }
	{ add r5, r6, r7 ; ld r25, r26 }
	{ add r5, r6, r7 ; ld1s r25, r26 ; shli r15, r16, 5 }
	{ add r5, r6, r7 ; ld1u r25, r26 ; rotl r15, r16, r17 }
	{ add r5, r6, r7 ; ld2s r25, r26 ; jrp r15 }
	{ add r5, r6, r7 ; ld2u r25, r26 ; cmpltsi r15, r16, 5 }
	{ add r5, r6, r7 ; ld4s r25, r26 ; addx r15, r16, r17 }
	{ add r5, r6, r7 ; ld4s r25, r26 ; shrui r15, r16, 5 }
	{ add r5, r6, r7 ; ld4u r25, r26 ; shl1addx r15, r16, r17 }
	{ add r5, r6, r7 ; lnk r15 ; prefetch_l1 r25 }
	{ add r5, r6, r7 ; move r15, r16 ; prefetch_l1 r25 }
	{ add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l1 r25 }
	{ add r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l2 r25 }
	{ add r5, r6, r7 ; prefetch r25 ; cmplts r15, r16, r17 }
	{ add r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
	{ add r5, r6, r7 ; prefetch_l1 r25 ; shl3add r15, r16, r17 }
	{ add r5, r6, r7 ; prefetch_l1_fault r25 ; or r15, r16, r17 }
	{ add r5, r6, r7 ; prefetch_l2 r25 ; jrp r15 }
	{ add r5, r6, r7 ; prefetch_l2_fault r25 ; cmpltu r15, r16, r17 }
	{ add r5, r6, r7 ; prefetch_l3 r25 ; and r15, r16, r17 }
	{ add r5, r6, r7 ; prefetch_l3 r25 ; subx r15, r16, r17 }
	{ add r5, r6, r7 ; prefetch_l3_fault r25 ; shl3add r15, r16, r17 }
	{ add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
	{ add r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
	{ add r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l3 r25 }
	{ add r5, r6, r7 ; shl3add r15, r16, r17 ; st r25, r26 }
	{ add r5, r6, r7 ; shli r15, r16, 5 ; st2 r25, r26 }
	{ add r5, r6, r7 ; shrsi r15, r16, 5 ; st2 r25, r26 }
	{ add r5, r6, r7 ; shrui r15, r16, 5 }
	{ add r5, r6, r7 ; st r25, r26 ; shl3add r15, r16, r17 }
	{ add r5, r6, r7 ; st1 r25, r26 ; or r15, r16, r17 }
	{ add r5, r6, r7 ; st2 r25, r26 ; jr r15 }
	{ add r5, r6, r7 ; st4 r25, r26 ; cmplts r15, r16, r17 }
	{ add r5, r6, r7 ; stnt1 r15, r16 }
	{ add r5, r6, r7 ; subx r15, r16, r17 ; st r25, r26 }
	{ add r5, r6, r7 ; v2cmpleu r15, r16, r17 }
	{ add r5, r6, r7 ; xor r15, r16, r17 ; ld1u r25, r26 }
	{ addi r15, r16, 5 ; addi r5, r6, 5 ; ld2s r25, r26 }
	{ addi r15, r16, 5 ; addxi r5, r6, 5 ; ld2u r25, r26 }
	{ addi r15, r16, 5 ; andi r5, r6, 5 ; ld2u r25, r26 }
	{ addi r15, r16, 5 ; cmoveqz r5, r6, r7 ; ld2s r25, r26 }
	{ addi r15, r16, 5 ; cmpeq r5, r6, r7 ; ld4s r25, r26 }
	{ addi r15, r16, 5 ; cmples r5, r6, r7 ; prefetch r25 }
	{ addi r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch_l1_fault r25 }
	{ addi r15, r16, 5 ; cmpltu r5, r6, r7 ; prefetch_l2_fault r25 }
	{ addi r15, r16, 5 ; ctz r5, r6 ; ld2s r25, r26 }
	{ addi r15, r16, 5 ; fnop ; prefetch_l3 r25 }
	{ addi r15, r16, 5 ; info 19 ; prefetch_l1 r25 }
	{ addi r15, r16, 5 ; ld r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ addi r15, r16, 5 ; ld1s r25, r26 ; andi r5, r6, 5 }
	{ addi r15, r16, 5 ; ld1s r25, r26 ; shl1addx r5, r6, r7 }
	{ addi r15, r16, 5 ; ld1u r25, r26 ; move r5, r6 }
	{ addi r15, r16, 5 ; ld1u r25, r26 }
	{ addi r15, r16, 5 ; ld2s r25, r26 ; revbits r5, r6 }
	{ addi r15, r16, 5 ; ld2u r25, r26 ; cmpne r5, r6, r7 }
	{ addi r15, r16, 5 ; ld2u r25, r26 ; subx r5, r6, r7 }
	{ addi r15, r16, 5 ; ld4s r25, r26 ; mulx r5, r6, r7 }
	{ addi r15, r16, 5 ; ld4u r25, r26 ; cmpeqi r5, r6, 5 }
	{ addi r15, r16, 5 ; ld4u r25, r26 ; shli r5, r6, 5 }
	{ addi r15, r16, 5 ; move r5, r6 ; prefetch r25 }
	{ addi r15, r16, 5 ; mul_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ addi r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ addi r15, r16, 5 ; mula_hs_hs r5, r6, r7 ; ld4u r25, r26 }
	{ addi r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; ld2s r25, r26 }
	{ addi r15, r16, 5 ; mulax r5, r6, r7 ; ld2u r25, r26 }
	{ addi r15, r16, 5 ; mz r5, r6, r7 ; ld4u r25, r26 }
	{ addi r15, r16, 5 ; nor r5, r6, r7 ; prefetch_l1 r25 }
	{ addi r15, r16, 5 ; pcnt r5, r6 ; prefetch_l1_fault r25 }
	{ addi r15, r16, 5 ; prefetch r25 ; mula_ls_ls r5, r6, r7 }
	{ addi r15, r16, 5 ; prefetch_l1 r25 ; cmoveqz r5, r6, r7 }
	{ addi r15, r16, 5 ; prefetch_l1 r25 ; shl2addx r5, r6, r7 }
	{ addi r15, r16, 5 ; prefetch_l1_fault r25 ; mul_hs_hs r5, r6, r7 }
	{ addi r15, r16, 5 ; prefetch_l2 r25 ; addi r5, r6, 5 }
	{ addi r15, r16, 5 ; prefetch_l2 r25 ; rotl r5, r6, r7 }
	{ addi r15, r16, 5 ; prefetch_l2_fault r25 ; fnop }
	{ addi r15, r16, 5 ; prefetch_l2_fault r25 ; tblidxb1 r5, r6 }
	{ addi r15, r16, 5 ; prefetch_l3 r25 ; nop }
	{ addi r15, r16, 5 ; prefetch_l3_fault r25 ; cmpleu r5, r6, r7 }
	{ addi r15, r16, 5 ; prefetch_l3_fault r25 ; shrsi r5, r6, 5 }
	{ addi r15, r16, 5 ; revbytes r5, r6 ; prefetch_l2 r25 }
	{ addi r15, r16, 5 ; rotli r5, r6, 5 ; prefetch_l3 r25 }
	{ addi r15, r16, 5 ; shl1add r5, r6, r7 ; prefetch_l3_fault r25 }
	{ addi r15, r16, 5 ; shl2add r5, r6, r7 ; st1 r25, r26 }
	{ addi r15, r16, 5 ; shl3add r5, r6, r7 ; st4 r25, r26 }
	{ addi r15, r16, 5 ; shlx r5, r6, r7 }
	{ addi r15, r16, 5 ; shru r5, r6, r7 ; ld r25, r26 }
	{ addi r15, r16, 5 ; shufflebytes r5, r6, r7 }
	{ addi r15, r16, 5 ; st r25, r26 ; revbits r5, r6 }
	{ addi r15, r16, 5 ; st1 r25, r26 ; cmpne r5, r6, r7 }
	{ addi r15, r16, 5 ; st1 r25, r26 ; subx r5, r6, r7 }
	{ addi r15, r16, 5 ; st2 r25, r26 ; mulx r5, r6, r7 }
	{ addi r15, r16, 5 ; st4 r25, r26 ; cmpeqi r5, r6, 5 }
	{ addi r15, r16, 5 ; st4 r25, r26 ; shli r5, r6, 5 }
	{ addi r15, r16, 5 ; subx r5, r6, r7 ; prefetch_l1 r25 }
	{ addi r15, r16, 5 ; tblidxb1 r5, r6 ; prefetch_l1_fault r25 }
	{ addi r15, r16, 5 ; tblidxb3 r5, r6 ; prefetch_l2_fault r25 }
	{ addi r15, r16, 5 ; v1mulu r5, r6, r7 }
	{ addi r15, r16, 5 ; v2packh r5, r6, r7 }
	{ addi r15, r16, 5 ; xor r5, r6, r7 ; prefetch_l3_fault r25 }
	{ addi r5, r6, 5 ; addi r15, r16, 5 ; st r25, r26 }
	{ addi r5, r6, 5 ; addxi r15, r16, 5 ; st1 r25, r26 }
	{ addi r5, r6, 5 ; andi r15, r16, 5 ; st1 r25, r26 }
	{ addi r5, r6, 5 ; cmpeqi r15, r16, 5 ; st4 r25, r26 }
	{ addi r5, r6, 5 ; cmpleu r15, r16, r17 ; st4 r25, r26 }
	{ addi r5, r6, 5 ; cmpltu r15, r16, r17 ; ld r25, r26 }
	{ addi r5, r6, 5 ; dblalign4 r15, r16, r17 }
	{ addi r5, r6, 5 ; ill ; ld2u r25, r26 }
	{ addi r5, r6, 5 ; jalr r15 ; ld2s r25, r26 }
	{ addi r5, r6, 5 ; jr r15 ; ld4s r25, r26 }
	{ addi r5, r6, 5 ; ld r25, r26 ; cmpeq r15, r16, r17 }
	{ addi r5, r6, 5 ; ld r25, r26 }
	{ addi r5, r6, 5 ; ld1s r25, r26 ; shli r15, r16, 5 }
	{ addi r5, r6, 5 ; ld1u r25, r26 ; rotl r15, r16, r17 }
	{ addi r5, r6, 5 ; ld2s r25, r26 ; jrp r15 }
	{ addi r5, r6, 5 ; ld2u r25, r26 ; cmpltsi r15, r16, 5 }
	{ addi r5, r6, 5 ; ld4s r25, r26 ; addx r15, r16, r17 }
	{ addi r5, r6, 5 ; ld4s r25, r26 ; shrui r15, r16, 5 }
	{ addi r5, r6, 5 ; ld4u r25, r26 ; shl1addx r15, r16, r17 }
	{ addi r5, r6, 5 ; lnk r15 ; prefetch_l1 r25 }
	{ addi r5, r6, 5 ; move r15, r16 ; prefetch_l1 r25 }
	{ addi r5, r6, 5 ; mz r15, r16, r17 ; prefetch_l1 r25 }
	{ addi r5, r6, 5 ; nor r15, r16, r17 ; prefetch_l2 r25 }
	{ addi r5, r6, 5 ; prefetch r25 ; cmplts r15, r16, r17 }
	{ addi r5, r6, 5 ; prefetch_add_l2_fault r15, 5 }
	{ addi r5, r6, 5 ; prefetch_l1 r25 ; shl3add r15, r16, r17 }
	{ addi r5, r6, 5 ; prefetch_l1_fault r25 ; or r15, r16, r17 }
	{ addi r5, r6, 5 ; prefetch_l2 r25 ; jrp r15 }
	{ addi r5, r6, 5 ; prefetch_l2_fault r25 ; cmpltu r15, r16, r17 }
	{ addi r5, r6, 5 ; prefetch_l3 r25 ; and r15, r16, r17 }
	{ addi r5, r6, 5 ; prefetch_l3 r25 ; subx r15, r16, r17 }
	{ addi r5, r6, 5 ; prefetch_l3_fault r25 ; shl3add r15, r16, r17 }
	{ addi r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l1_fault r25 }
	{ addi r5, r6, 5 ; shl1add r15, r16, r17 ; prefetch_l2 r25 }
	{ addi r5, r6, 5 ; shl2add r15, r16, r17 ; prefetch_l3 r25 }
	{ addi r5, r6, 5 ; shl3add r15, r16, r17 ; st r25, r26 }
	{ addi r5, r6, 5 ; shli r15, r16, 5 ; st2 r25, r26 }
	{ addi r5, r6, 5 ; shrsi r15, r16, 5 ; st2 r25, r26 }
	{ addi r5, r6, 5 ; shrui r15, r16, 5 }
	{ addi r5, r6, 5 ; st r25, r26 ; shl3add r15, r16, r17 }
	{ addi r5, r6, 5 ; st1 r25, r26 ; or r15, r16, r17 }
	{ addi r5, r6, 5 ; st2 r25, r26 ; jr r15 }
	{ addi r5, r6, 5 ; st4 r25, r26 ; cmplts r15, r16, r17 }
	{ addi r5, r6, 5 ; stnt1 r15, r16 }
	{ addi r5, r6, 5 ; subx r15, r16, r17 ; st r25, r26 }
	{ addi r5, r6, 5 ; v2cmpleu r15, r16, r17 }
	{ addi r5, r6, 5 ; xor r15, r16, r17 ; ld1u r25, r26 }
	{ addli r15, r16, 0x1234 ; cmpltui r5, r6, 5 }
	{ addli r15, r16, 0x1234 ; mul_hs_hu r5, r6, r7 }
	{ addli r15, r16, 0x1234 ; shlx r5, r6, r7 }
	{ addli r15, r16, 0x1234 ; v1int_h r5, r6, r7 }
	{ addli r15, r16, 0x1234 ; v2maxsi r5, r6, 5 }
	{ addli r5, r6, 0x1234 ; addx r15, r16, r17 }
	{ addli r5, r6, 0x1234 ; iret }
	{ addli r5, r6, 0x1234 ; movei r15, 5 }
	{ addli r5, r6, 0x1234 ; shruxi r15, r16, 5 }
	{ addli r5, r6, 0x1234 ; v1shl r15, r16, r17 }
	{ addli r5, r6, 0x1234 ; v4add r15, r16, r17 }
	{ addx r15, r16, r17 ; addi r5, r6, 5 ; prefetch r25 }
	{ addx r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l1 r25 }
	{ addx r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l1 r25 }
	{ addx r15, r16, r17 ; cmoveqz r5, r6, r7 ; prefetch r25 }
	{ addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch_l1_fault r25 }
	{ addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l2_fault r25 }
	{ addx r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l3_fault r25 }
	{ addx r15, r16, r17 ; cmpltu r5, r6, r7 ; st1 r25, r26 }
	{ addx r15, r16, r17 ; ctz r5, r6 ; prefetch r25 }
	{ addx r15, r16, r17 ; fnop ; st2 r25, r26 }
	{ addx r15, r16, r17 ; info 19 ; prefetch_l3 r25 }
	{ addx r15, r16, r17 ; ld r25, r26 ; mulax r5, r6, r7 }
	{ addx r15, r16, r17 ; ld1s r25, r26 ; cmpeq r5, r6, r7 }
	{ addx r15, r16, r17 ; ld1s r25, r26 ; shl3addx r5, r6, r7 }
	{ addx r15, r16, r17 ; ld1u r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ addx r15, r16, r17 ; ld2s r25, r26 ; addxi r5, r6, 5 }
	{ addx r15, r16, r17 ; ld2s r25, r26 ; shl r5, r6, r7 }
	{ addx r15, r16, r17 ; ld2u r25, r26 ; info 19 }
	{ addx r15, r16, r17 ; ld2u r25, r26 ; tblidxb3 r5, r6 }
	{ addx r15, r16, r17 ; ld4s r25, r26 ; or r5, r6, r7 }
	{ addx r15, r16, r17 ; ld4u r25, r26 ; cmpltsi r5, r6, 5 }
	{ addx r15, r16, r17 ; ld4u r25, r26 ; shrui r5, r6, 5 }
	{ addx r15, r16, r17 ; move r5, r6 ; prefetch_l2_fault r25 }
	{ addx r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l3 r25 }
	{ addx r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch_l1_fault r25 }
	{ addx r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ addx r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; prefetch r25 }
	{ addx r15, r16, r17 ; mulax r5, r6, r7 ; prefetch_l1 r25 }
	{ addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l2 r25 }
	{ addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l3 r25 }
	{ addx r15, r16, r17 ; pcnt r5, r6 ; prefetch_l3_fault r25 }
	{ addx r15, r16, r17 ; prefetch r25 ; mz r5, r6, r7 }
	{ addx r15, r16, r17 ; prefetch_l1 r25 ; cmples r5, r6, r7 }
	{ addx r15, r16, r17 ; prefetch_l1 r25 ; shrs r5, r6, r7 }
	{ addx r15, r16, r17 ; prefetch_l1_fault r25 ; mula_hs_hs r5, r6, r7 }
	{ addx r15, r16, r17 ; prefetch_l2 r25 ; andi r5, r6, 5 }
	{ addx r15, r16, r17 ; prefetch_l2 r25 ; shl1addx r5, r6, r7 }
	{ addx r15, r16, r17 ; prefetch_l2_fault r25 ; move r5, r6 }
	{ addx r15, r16, r17 ; prefetch_l2_fault r25 }
	{ addx r15, r16, r17 ; prefetch_l3 r25 ; revbits r5, r6 }
	{ addx r15, r16, r17 ; prefetch_l3_fault r25 ; cmpne r5, r6, r7 }
	{ addx r15, r16, r17 ; prefetch_l3_fault r25 ; subx r5, r6, r7 }
	{ addx r15, r16, r17 ; revbytes r5, r6 ; st r25, r26 }
	{ addx r15, r16, r17 ; rotli r5, r6, 5 ; st2 r25, r26 }
	{ addx r15, r16, r17 ; shl1add r5, r6, r7 ; st4 r25, r26 }
	{ addx r15, r16, r17 ; shl2addx r5, r6, r7 ; ld r25, r26 }
	{ addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld1u r25, r26 }
	{ addx r15, r16, r17 ; shrs r5, r6, r7 ; ld1u r25, r26 }
	{ addx r15, r16, r17 ; shru r5, r6, r7 ; ld2u r25, r26 }
	{ addx r15, r16, r17 ; st r25, r26 ; addxi r5, r6, 5 }
	{ addx r15, r16, r17 ; st r25, r26 ; shl r5, r6, r7 }
	{ addx r15, r16, r17 ; st1 r25, r26 ; info 19 }
	{ addx r15, r16, r17 ; st1 r25, r26 ; tblidxb3 r5, r6 }
	{ addx r15, r16, r17 ; st2 r25, r26 ; or r5, r6, r7 }
	{ addx r15, r16, r17 ; st4 r25, r26 ; cmpltsi r5, r6, 5 }
	{ addx r15, r16, r17 ; st4 r25, r26 ; shrui r5, r6, 5 }
	{ addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l3 r25 }
	{ addx r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l3_fault r25 }
	{ addx r15, r16, r17 ; tblidxb3 r5, r6 ; st1 r25, r26 }
	{ addx r15, r16, r17 ; v1sadu r5, r6, r7 }
	{ addx r15, r16, r17 ; v2sadau r5, r6, r7 }
	{ addx r15, r16, r17 ; xor r5, r6, r7 ; st4 r25, r26 }
	{ addx r5, r6, r7 ; addi r15, r16, 5 }
	{ addx r5, r6, r7 ; addxli r15, r16, 0x1234 }
	{ addx r5, r6, r7 ; cmpeq r15, r16, r17 ; ld r25, r26 }
	{ addx r5, r6, r7 ; cmples r15, r16, r17 ; ld r25, r26 }
	{ addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
	{ addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld2u r25, r26 }
	{ addx r5, r6, r7 ; exch4 r15, r16, r17 }
	{ addx r5, r6, r7 ; ill ; prefetch_l1 r25 }
	{ addx r5, r6, r7 ; jalr r15 ; prefetch r25 }
	{ addx r5, r6, r7 ; jr r15 ; prefetch_l1_fault r25 }
	{ addx r5, r6, r7 ; ld r25, r26 ; cmplts r15, r16, r17 }
	{ addx r5, r6, r7 ; ld1s r25, r26 ; addx r15, r16, r17 }
	{ addx r5, r6, r7 ; ld1s r25, r26 ; shrui r15, r16, 5 }
	{ addx r5, r6, r7 ; ld1u r25, r26 ; shl1addx r15, r16, r17 }
	{ addx r5, r6, r7 ; ld2s r25, r26 ; movei r15, 5 }
	{ addx r5, r6, r7 ; ld2u r25, r26 ; ill }
	{ addx r5, r6, r7 ; ld4s r25, r26 ; cmpeq r15, r16, r17 }
	{ addx r5, r6, r7 ; ld4s r25, r26 }
	{ addx r5, r6, r7 ; ld4u r25, r26 ; shl3addx r15, r16, r17 }
	{ addx r5, r6, r7 ; lnk r15 ; prefetch_l3 r25 }
	{ addx r5, r6, r7 ; move r15, r16 ; prefetch_l3 r25 }
	{ addx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l3 r25 }
	{ addx r5, r6, r7 ; nor r15, r16, r17 ; st r25, r26 }
	{ addx r5, r6, r7 ; prefetch r25 ; fnop }
	{ addx r5, r6, r7 ; prefetch_l1 r25 ; add r15, r16, r17 }
	{ addx r5, r6, r7 ; prefetch_l1 r25 ; shrsi r15, r16, 5 }
	{ addx r5, r6, r7 ; prefetch_l1_fault r25 ; shl1add r15, r16, r17 }
	{ addx r5, r6, r7 ; prefetch_l2 r25 ; movei r15, 5 }
	{ addx r5, r6, r7 ; prefetch_l2_fault r25 ; info 19 }
	{ addx r5, r6, r7 ; prefetch_l3 r25 ; cmples r15, r16, r17 }
	{ addx r5, r6, r7 ; prefetch_l3_fault r25 ; add r15, r16, r17 }
	{ addx r5, r6, r7 ; prefetch_l3_fault r25 ; shrsi r15, r16, 5 }
	{ addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l3_fault r25 }
	{ addx r5, r6, r7 ; shl1add r15, r16, r17 ; st r25, r26 }
	{ addx r5, r6, r7 ; shl2add r15, r16, r17 ; st2 r25, r26 }
	{ addx r5, r6, r7 ; shl3add r15, r16, r17 }
	{ addx r5, r6, r7 ; shlxi r15, r16, 5 }
	{ addx r5, r6, r7 ; shru r15, r16, r17 ; ld1s r25, r26 }
	{ addx r5, r6, r7 ; st r25, r26 ; add r15, r16, r17 }
	{ addx r5, r6, r7 ; st r25, r26 ; shrsi r15, r16, 5 }
	{ addx r5, r6, r7 ; st1 r25, r26 ; shl1add r15, r16, r17 }
	{ addx r5, r6, r7 ; st2 r25, r26 ; move r15, r16 }
	{ addx r5, r6, r7 ; st4 r25, r26 ; fnop }
	{ addx r5, r6, r7 ; stnt4 r15, r16 }
	{ addx r5, r6, r7 ; subx r15, r16, r17 }
	{ addx r5, r6, r7 ; v2cmpltui r15, r16, 5 }
	{ addx r5, r6, r7 ; xor r15, r16, r17 ; ld4u r25, r26 }
	{ addxi r15, r16, 5 ; addi r5, r6, 5 ; prefetch r25 }
	{ addxi r15, r16, 5 ; addxi r5, r6, 5 ; prefetch_l1 r25 }
	{ addxi r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l1 r25 }
	{ addxi r15, r16, 5 ; cmoveqz r5, r6, r7 ; prefetch r25 }
	{ addxi r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l1_fault r25 }
	{ addxi r15, r16, 5 ; cmples r5, r6, r7 ; prefetch_l2_fault r25 }
	{ addxi r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch_l3_fault r25 }
	{ addxi r15, r16, 5 ; cmpltu r5, r6, r7 ; st1 r25, r26 }
	{ addxi r15, r16, 5 ; ctz r5, r6 ; prefetch r25 }
	{ addxi r15, r16, 5 ; fnop ; st2 r25, r26 }
	{ addxi r15, r16, 5 ; info 19 ; prefetch_l3 r25 }
	{ addxi r15, r16, 5 ; ld r25, r26 ; mulax r5, r6, r7 }
	{ addxi r15, r16, 5 ; ld1s r25, r26 ; cmpeq r5, r6, r7 }
	{ addxi r15, r16, 5 ; ld1s r25, r26 ; shl3addx r5, r6, r7 }
	{ addxi r15, r16, 5 ; ld1u r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ addxi r15, r16, 5 ; ld2s r25, r26 ; addxi r5, r6, 5 }
	{ addxi r15, r16, 5 ; ld2s r25, r26 ; shl r5, r6, r7 }
	{ addxi r15, r16, 5 ; ld2u r25, r26 ; info 19 }
	{ addxi r15, r16, 5 ; ld2u r25, r26 ; tblidxb3 r5, r6 }
	{ addxi r15, r16, 5 ; ld4s r25, r26 ; or r5, r6, r7 }
	{ addxi r15, r16, 5 ; ld4u r25, r26 ; cmpltsi r5, r6, 5 }
	{ addxi r15, r16, 5 ; ld4u r25, r26 ; shrui r5, r6, 5 }
	{ addxi r15, r16, 5 ; move r5, r6 ; prefetch_l2_fault r25 }
	{ addxi r15, r16, 5 ; mul_hs_hs r5, r6, r7 ; prefetch_l3 r25 }
	{ addxi r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; prefetch_l1_fault r25 }
	{ addxi r15, r16, 5 ; mula_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ addxi r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; prefetch r25 }
	{ addxi r15, r16, 5 ; mulax r5, r6, r7 ; prefetch_l1 r25 }
	{ addxi r15, r16, 5 ; mz r5, r6, r7 ; prefetch_l2 r25 }
	{ addxi r15, r16, 5 ; nor r5, r6, r7 ; prefetch_l3 r25 }
	{ addxi r15, r16, 5 ; pcnt r5, r6 ; prefetch_l3_fault r25 }
	{ addxi r15, r16, 5 ; prefetch r25 ; mz r5, r6, r7 }
	{ addxi r15, r16, 5 ; prefetch_l1 r25 ; cmples r5, r6, r7 }
	{ addxi r15, r16, 5 ; prefetch_l1 r25 ; shrs r5, r6, r7 }
	{ addxi r15, r16, 5 ; prefetch_l1_fault r25 ; mula_hs_hs r5, r6, r7 }
	{ addxi r15, r16, 5 ; prefetch_l2 r25 ; andi r5, r6, 5 }
	{ addxi r15, r16, 5 ; prefetch_l2 r25 ; shl1addx r5, r6, r7 }
	{ addxi r15, r16, 5 ; prefetch_l2_fault r25 ; move r5, r6 }
	{ addxi r15, r16, 5 ; prefetch_l2_fault r25 }
	{ addxi r15, r16, 5 ; prefetch_l3 r25 ; revbits r5, r6 }
	{ addxi r15, r16, 5 ; prefetch_l3_fault r25 ; cmpne r5, r6, r7 }
	{ addxi r15, r16, 5 ; prefetch_l3_fault r25 ; subx r5, r6, r7 }
	{ addxi r15, r16, 5 ; revbytes r5, r6 ; st r25, r26 }
	{ addxi r15, r16, 5 ; rotli r5, r6, 5 ; st2 r25, r26 }
	{ addxi r15, r16, 5 ; shl1add r5, r6, r7 ; st4 r25, r26 }
	{ addxi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld r25, r26 }
	{ addxi r15, r16, 5 ; shl3addx r5, r6, r7 ; ld1u r25, r26 }
	{ addxi r15, r16, 5 ; shrs r5, r6, r7 ; ld1u r25, r26 }
	{ addxi r15, r16, 5 ; shru r5, r6, r7 ; ld2u r25, r26 }
	{ addxi r15, r16, 5 ; st r25, r26 ; addxi r5, r6, 5 }
	{ addxi r15, r16, 5 ; st r25, r26 ; shl r5, r6, r7 }
	{ addxi r15, r16, 5 ; st1 r25, r26 ; info 19 }
	{ addxi r15, r16, 5 ; st1 r25, r26 ; tblidxb3 r5, r6 }
	{ addxi r15, r16, 5 ; st2 r25, r26 ; or r5, r6, r7 }
	{ addxi r15, r16, 5 ; st4 r25, r26 ; cmpltsi r5, r6, 5 }
	{ addxi r15, r16, 5 ; st4 r25, r26 ; shrui r5, r6, 5 }
	{ addxi r15, r16, 5 ; subx r5, r6, r7 ; prefetch_l3 r25 }
	{ addxi r15, r16, 5 ; tblidxb1 r5, r6 ; prefetch_l3_fault r25 }
	{ addxi r15, r16, 5 ; tblidxb3 r5, r6 ; st1 r25, r26 }
	{ addxi r15, r16, 5 ; v1sadu r5, r6, r7 }
	{ addxi r15, r16, 5 ; v2sadau r5, r6, r7 }
	{ addxi r15, r16, 5 ; xor r5, r6, r7 ; st4 r25, r26 }
	{ addxi r5, r6, 5 ; addi r15, r16, 5 }
	{ addxi r5, r6, 5 ; addxli r15, r16, 0x1234 }
	{ addxi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld r25, r26 }
	{ addxi r5, r6, 5 ; cmples r15, r16, r17 ; ld r25, r26 }
	{ addxi r5, r6, 5 ; cmplts r15, r16, r17 ; ld1u r25, r26 }
	{ addxi r5, r6, 5 ; cmpltu r15, r16, r17 ; ld2u r25, r26 }
	{ addxi r5, r6, 5 ; exch4 r15, r16, r17 }
	{ addxi r5, r6, 5 ; ill ; prefetch_l1 r25 }
	{ addxi r5, r6, 5 ; jalr r15 ; prefetch r25 }
	{ addxi r5, r6, 5 ; jr r15 ; prefetch_l1_fault r25 }
	{ addxi r5, r6, 5 ; ld r25, r26 ; cmplts r15, r16, r17 }
	{ addxi r5, r6, 5 ; ld1s r25, r26 ; addx r15, r16, r17 }
	{ addxi r5, r6, 5 ; ld1s r25, r26 ; shrui r15, r16, 5 }
	{ addxi r5, r6, 5 ; ld1u r25, r26 ; shl1addx r15, r16, r17 }
	{ addxi r5, r6, 5 ; ld2s r25, r26 ; movei r15, 5 }
	{ addxi r5, r6, 5 ; ld2u r25, r26 ; ill }
	{ addxi r5, r6, 5 ; ld4s r25, r26 ; cmpeq r15, r16, r17 }
	{ addxi r5, r6, 5 ; ld4s r25, r26 }
	{ addxi r5, r6, 5 ; ld4u r25, r26 ; shl3addx r15, r16, r17 }
	{ addxi r5, r6, 5 ; lnk r15 ; prefetch_l3 r25 }
	{ addxi r5, r6, 5 ; move r15, r16 ; prefetch_l3 r25 }
	{ addxi r5, r6, 5 ; mz r15, r16, r17 ; prefetch_l3 r25 }
	{ addxi r5, r6, 5 ; nor r15, r16, r17 ; st r25, r26 }
	{ addxi r5, r6, 5 ; prefetch r25 ; fnop }
	{ addxi r5, r6, 5 ; prefetch_l1 r25 ; add r15, r16, r17 }
	{ addxi r5, r6, 5 ; prefetch_l1 r25 ; shrsi r15, r16, 5 }
	{ addxi r5, r6, 5 ; prefetch_l1_fault r25 ; shl1add r15, r16, r17 }
	{ addxi r5, r6, 5 ; prefetch_l2 r25 ; movei r15, 5 }
	{ addxi r5, r6, 5 ; prefetch_l2_fault r25 ; info 19 }
	{ addxi r5, r6, 5 ; prefetch_l3 r25 ; cmples r15, r16, r17 }
	{ addxi r5, r6, 5 ; prefetch_l3_fault r25 ; add r15, r16, r17 }
	{ addxi r5, r6, 5 ; prefetch_l3_fault r25 ; shrsi r15, r16, 5 }
	{ addxi r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l3_fault r25 }
	{ addxi r5, r6, 5 ; shl1add r15, r16, r17 ; st r25, r26 }
	{ addxi r5, r6, 5 ; shl2add r15, r16, r17 ; st2 r25, r26 }
	{ addxi r5, r6, 5 ; shl3add r15, r16, r17 }
	{ addxi r5, r6, 5 ; shlxi r15, r16, 5 }
	{ addxi r5, r6, 5 ; shru r15, r16, r17 ; ld1s r25, r26 }
	{ addxi r5, r6, 5 ; st r25, r26 ; add r15, r16, r17 }
	{ addxi r5, r6, 5 ; st r25, r26 ; shrsi r15, r16, 5 }
	{ addxi r5, r6, 5 ; st1 r25, r26 ; shl1add r15, r16, r17 }
	{ addxi r5, r6, 5 ; st2 r25, r26 ; move r15, r16 }
	{ addxi r5, r6, 5 ; st4 r25, r26 ; fnop }
	{ addxi r5, r6, 5 ; stnt4 r15, r16 }
	{ addxi r5, r6, 5 ; subx r15, r16, r17 }
	{ addxi r5, r6, 5 ; v2cmpltui r15, r16, 5 }
	{ addxi r5, r6, 5 ; xor r15, r16, r17 ; ld4u r25, r26 }
	{ addxli r15, r16, 0x1234 ; cmulaf r5, r6, r7 }
	{ addxli r15, r16, 0x1234 ; mul_hu_ls r5, r6, r7 }
	{ addxli r15, r16, 0x1234 ; shru r5, r6, r7 }
	{ addxli r15, r16, 0x1234 ; v1minu r5, r6, r7 }
	{ addxli r15, r16, 0x1234 ; v2mulfsc r5, r6, r7 }
	{ addxli r5, r6, 0x1234 ; and r15, r16, r17 }
	{ addxli r5, r6, 0x1234 ; jrp r15 }
	{ addxli r5, r6, 0x1234 ; nop }
	{ addxli r5, r6, 0x1234 ; st2 r15, r16 }
	{ addxli r5, r6, 0x1234 ; v1shru r15, r16, r17 }
	{ addxli r5, r6, 0x1234 ; v4packsc r15, r16, r17 }
	{ addxsc r15, r16, r17 ; cmulhr r5, r6, r7 }
	{ addxsc r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ addxsc r15, r16, r17 ; shufflebytes r5, r6, r7 }
	{ addxsc r15, r16, r17 ; v1mulu r5, r6, r7 }
	{ addxsc r15, r16, r17 ; v2packh r5, r6, r7 }
	{ addxsc r5, r6, r7 ; cmpexch r15, r16, r17 }
	{ addxsc r5, r6, r7 ; ld1u r15, r16 }
	{ addxsc r5, r6, r7 ; prefetch r15 }
	{ addxsc r5, r6, r7 ; st_add r15, r16, 5 }
	{ addxsc r5, r6, r7 ; v2add r15, r16, r17 }
	{ addxsc r5, r6, r7 ; v4shru r15, r16, r17 }
	{ and r15, r16, r17 ; addi r5, r6, 5 ; st1 r25, r26 }
	{ and r15, r16, r17 ; addxi r5, r6, 5 ; st2 r25, r26 }
	{ and r15, r16, r17 ; andi r5, r6, 5 ; st2 r25, r26 }
	{ and r15, r16, r17 ; cmoveqz r5, r6, r7 ; st1 r25, r26 }
	{ and r15, r16, r17 ; cmpeq r5, r6, r7 ; st4 r25, r26 }
	{ and r15, r16, r17 ; cmpleu r5, r6, r7 ; ld r25, r26 }
	{ and r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld1u r25, r26 }
	{ and r15, r16, r17 ; cmpne r5, r6, r7 ; ld2s r25, r26 }
	{ and r15, r16, r17 ; ctz r5, r6 ; st1 r25, r26 }
	{ and r15, r16, r17 ; fsingle_pack1 r5, r6 ; ld1s r25, r26 }
	{ and r15, r16, r17 ; ld r25, r26 ; add r5, r6, r7 }
	{ and r15, r16, r17 ; ld r25, r26 ; revbytes r5, r6 }
	{ and r15, r16, r17 ; ld1s r25, r26 ; ctz r5, r6 }
	{ and r15, r16, r17 ; ld1s r25, r26 ; tblidxb0 r5, r6 }
	{ and r15, r16, r17 ; ld1u r25, r26 ; mz r5, r6, r7 }
	{ and r15, r16, r17 ; ld2s r25, r26 ; cmples r5, r6, r7 }
	{ and r15, r16, r17 ; ld2s r25, r26 ; shrs r5, r6, r7 }
	{ and r15, r16, r17 ; ld2u r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ and r15, r16, r17 ; ld4s r25, r26 ; andi r5, r6, 5 }
	{ and r15, r16, r17 ; ld4s r25, r26 ; shl1addx r5, r6, r7 }
	{ and r15, r16, r17 ; ld4u r25, r26 ; move r5, r6 }
	{ and r15, r16, r17 ; ld4u r25, r26 }
	{ and r15, r16, r17 ; movei r5, 5 ; ld r25, r26 }
	{ and r15, r16, r17 ; mul_hs_ls r5, r6, r7 }
	{ and r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; st4 r25, r26 }
	{ and r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ and r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ and r15, r16, r17 ; mulax r5, r6, r7 ; st2 r25, r26 }
	{ and r15, r16, r17 ; mz r5, r6, r7 }
	{ and r15, r16, r17 ; or r5, r6, r7 ; ld1s r25, r26 }
	{ and r15, r16, r17 ; prefetch r25 ; addx r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch r25 ; rotli r5, r6, 5 }
	{ and r15, r16, r17 ; prefetch_l1 r25 ; fsingle_pack1 r5, r6 }
	{ and r15, r16, r17 ; prefetch_l1 r25 ; tblidxb2 r5, r6 }
	{ and r15, r16, r17 ; prefetch_l1_fault r25 ; nor r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch_l2 r25 ; cmplts r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch_l2 r25 ; shru r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch_l2_fault r25 ; mula_ls_ls r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch_l3 r25 ; cmoveqz r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch_l3 r25 ; shl2addx r5, r6, r7 }
	{ and r15, r16, r17 ; prefetch_l3_fault r25 ; mul_hs_hs r5, r6, r7 }
	{ and r15, r16, r17 ; revbits r5, r6 ; ld1s r25, r26 }
	{ and r15, r16, r17 ; rotl r5, r6, r7 ; ld2s r25, r26 }
	{ and r15, r16, r17 ; shl r5, r6, r7 ; ld4s r25, r26 }
	{ and r15, r16, r17 ; shl1addx r5, r6, r7 ; ld4u r25, r26 }
	{ and r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l1 r25 }
	{ and r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2 r25 }
	{ and r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
	{ and r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l3 r25 }
	{ and r15, r16, r17 ; st r25, r26 ; cmples r5, r6, r7 }
	{ and r15, r16, r17 ; st r25, r26 ; shrs r5, r6, r7 }
	{ and r15, r16, r17 ; st1 r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ and r15, r16, r17 ; st2 r25, r26 ; andi r5, r6, 5 }
	{ and r15, r16, r17 ; st2 r25, r26 ; shl1addx r5, r6, r7 }
	{ and r15, r16, r17 ; st4 r25, r26 ; move r5, r6 }
	{ and r15, r16, r17 ; st4 r25, r26 }
	{ and r15, r16, r17 ; tblidxb0 r5, r6 ; ld r25, r26 }
	{ and r15, r16, r17 ; tblidxb2 r5, r6 ; ld1u r25, r26 }
	{ and r15, r16, r17 ; v1avgu r5, r6, r7 }
	{ and r15, r16, r17 ; v1subuc r5, r6, r7 }
	{ and r15, r16, r17 ; v2shru r5, r6, r7 }
	{ and r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
	{ and r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
	{ and r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
	{ and r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1 r25 }
	{ and r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1 r25 }
	{ and r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
	{ and r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
	{ and r5, r6, r7 ; fetchor4 r15, r16, r17 }
	{ and r5, r6, r7 ; ill ; st2 r25, r26 }
	{ and r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
	{ and r5, r6, r7 ; jr r15 ; st4 r25, r26 }
	{ and r5, r6, r7 ; ld r25, r26 ; jalrp r15 }
	{ and r5, r6, r7 ; ld1s r25, r26 ; cmplts r15, r16, r17 }
	{ and r5, r6, r7 ; ld1u r25, r26 ; addi r15, r16, 5 }
	{ and r5, r6, r7 ; ld1u r25, r26 ; shru r15, r16, r17 }
	{ and r5, r6, r7 ; ld2s r25, r26 ; shl1add r15, r16, r17 }
	{ and r5, r6, r7 ; ld2u r25, r26 ; move r15, r16 }
	{ and r5, r6, r7 ; ld4s r25, r26 ; fnop }
	{ and r5, r6, r7 ; ld4u r25, r26 ; andi r15, r16, 5 }
	{ and r5, r6, r7 ; ld4u r25, r26 ; xor r15, r16, r17 }
	{ and r5, r6, r7 ; mfspr r16, 0x5 }
	{ and r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
	{ and r5, r6, r7 ; nop ; ld1s r25, r26 }
	{ and r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
	{ and r5, r6, r7 ; prefetch r25 ; mnz r15, r16, r17 }
	{ and r5, r6, r7 ; prefetch_l1 r25 ; cmples r15, r16, r17 }
	{ and r5, r6, r7 ; prefetch_l1_fault r25 ; add r15, r16, r17 }
	{ and r5, r6, r7 ; prefetch_l1_fault r25 ; shrsi r15, r16, 5 }
	{ and r5, r6, r7 ; prefetch_l2 r25 ; shl1add r15, r16, r17 }
	{ and r5, r6, r7 ; prefetch_l2_fault r25 ; movei r15, 5 }
	{ and r5, r6, r7 ; prefetch_l3 r25 ; info 19 }
	{ and r5, r6, r7 ; prefetch_l3_fault r25 ; cmples r15, r16, r17 }
	{ and r5, r6, r7 ; rotl r15, r16, r17 ; ld r25, r26 }
	{ and r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
	{ and r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
	{ and r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
	{ and r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
	{ and r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
	{ and r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
	{ and r5, r6, r7 ; st r25, r26 ; cmples r15, r16, r17 }
	{ and r5, r6, r7 ; st1 r25, r26 ; add r15, r16, r17 }
	{ and r5, r6, r7 ; st1 r25, r26 ; shrsi r15, r16, 5 }
	{ and r5, r6, r7 ; st2 r25, r26 ; shl r15, r16, r17 }
	{ and r5, r6, r7 ; st4 r25, r26 ; mnz r15, r16, r17 }
	{ and r5, r6, r7 ; sub r15, r16, r17 ; ld4s r25, r26 }
	{ and r5, r6, r7 ; v1cmpleu r15, r16, r17 }
	{ and r5, r6, r7 ; v2mnz r15, r16, r17 }
	{ and r5, r6, r7 ; xor r15, r16, r17 ; st r25, r26 }
	{ andi r15, r16, 5 ; addi r5, r6, 5 ; st1 r25, r26 }
	{ andi r15, r16, 5 ; addxi r5, r6, 5 ; st2 r25, r26 }
	{ andi r15, r16, 5 ; andi r5, r6, 5 ; st2 r25, r26 }
	{ andi r15, r16, 5 ; cmoveqz r5, r6, r7 ; st1 r25, r26 }
	{ andi r15, r16, 5 ; cmpeq r5, r6, r7 ; st4 r25, r26 }
	{ andi r15, r16, 5 ; cmpleu r5, r6, r7 ; ld r25, r26 }
	{ andi r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld1u r25, r26 }
	{ andi r15, r16, 5 ; cmpne r5, r6, r7 ; ld2s r25, r26 }
	{ andi r15, r16, 5 ; ctz r5, r6 ; st1 r25, r26 }
	{ andi r15, r16, 5 ; fsingle_pack1 r5, r6 ; ld1s r25, r26 }
	{ andi r15, r16, 5 ; ld r25, r26 ; add r5, r6, r7 }
	{ andi r15, r16, 5 ; ld r25, r26 ; revbytes r5, r6 }
	{ andi r15, r16, 5 ; ld1s r25, r26 ; ctz r5, r6 }
	{ andi r15, r16, 5 ; ld1s r25, r26 ; tblidxb0 r5, r6 }
	{ andi r15, r16, 5 ; ld1u r25, r26 ; mz r5, r6, r7 }
	{ andi r15, r16, 5 ; ld2s r25, r26 ; cmples r5, r6, r7 }
	{ andi r15, r16, 5 ; ld2s r25, r26 ; shrs r5, r6, r7 }
	{ andi r15, r16, 5 ; ld2u r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ andi r15, r16, 5 ; ld4s r25, r26 ; andi r5, r6, 5 }
	{ andi r15, r16, 5 ; ld4s r25, r26 ; shl1addx r5, r6, r7 }
	{ andi r15, r16, 5 ; ld4u r25, r26 ; move r5, r6 }
	{ andi r15, r16, 5 ; ld4u r25, r26 }
	{ andi r15, r16, 5 ; movei r5, 5 ; ld r25, r26 }
	{ andi r15, r16, 5 ; mul_hs_ls r5, r6, r7 }
	{ andi r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; st4 r25, r26 }
	{ andi r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ andi r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ andi r15, r16, 5 ; mulax r5, r6, r7 ; st2 r25, r26 }
	{ andi r15, r16, 5 ; mz r5, r6, r7 }
	{ andi r15, r16, 5 ; or r5, r6, r7 ; ld1s r25, r26 }
	{ andi r15, r16, 5 ; prefetch r25 ; addx r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch r25 ; rotli r5, r6, 5 }
	{ andi r15, r16, 5 ; prefetch_l1 r25 ; fsingle_pack1 r5, r6 }
	{ andi r15, r16, 5 ; prefetch_l1 r25 ; tblidxb2 r5, r6 }
	{ andi r15, r16, 5 ; prefetch_l1_fault r25 ; nor r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch_l2 r25 ; cmplts r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch_l2 r25 ; shru r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch_l2_fault r25 ; mula_ls_ls r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch_l3 r25 ; cmoveqz r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch_l3 r25 ; shl2addx r5, r6, r7 }
	{ andi r15, r16, 5 ; prefetch_l3_fault r25 ; mul_hs_hs r5, r6, r7 }
	{ andi r15, r16, 5 ; revbits r5, r6 ; ld1s r25, r26 }
	{ andi r15, r16, 5 ; rotl r5, r6, r7 ; ld2s r25, r26 }
	{ andi r15, r16, 5 ; shl r5, r6, r7 ; ld4s r25, r26 }
	{ andi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld4u r25, r26 }
	{ andi r15, r16, 5 ; shl2addx r5, r6, r7 ; prefetch_l1 r25 }
	{ andi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l2 r25 }
	{ andi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
	{ andi r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l3 r25 }
	{ andi r15, r16, 5 ; st r25, r26 ; cmples r5, r6, r7 }
	{ andi r15, r16, 5 ; st r25, r26 ; shrs r5, r6, r7 }
	{ andi r15, r16, 5 ; st1 r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ andi r15, r16, 5 ; st2 r25, r26 ; andi r5, r6, 5 }
	{ andi r15, r16, 5 ; st2 r25, r26 ; shl1addx r5, r6, r7 }
	{ andi r15, r16, 5 ; st4 r25, r26 ; move r5, r6 }
	{ andi r15, r16, 5 ; st4 r25, r26 }
	{ andi r15, r16, 5 ; tblidxb0 r5, r6 ; ld r25, r26 }
	{ andi r15, r16, 5 ; tblidxb2 r5, r6 ; ld1u r25, r26 }
	{ andi r15, r16, 5 ; v1avgu r5, r6, r7 }
	{ andi r15, r16, 5 ; v1subuc r5, r6, r7 }
	{ andi r15, r16, 5 ; v2shru r5, r6, r7 }
	{ andi r5, r6, 5 ; add r15, r16, r17 ; ld4s r25, r26 }
	{ andi r5, r6, 5 ; addx r15, r16, r17 ; ld4u r25, r26 }
	{ andi r5, r6, 5 ; and r15, r16, r17 ; ld4u r25, r26 }
	{ andi r5, r6, 5 ; cmpeq r15, r16, r17 ; prefetch_l1 r25 }
	{ andi r5, r6, 5 ; cmples r15, r16, r17 ; prefetch_l1 r25 }
	{ andi r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
	{ andi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
	{ andi r5, r6, 5 ; fetchor4 r15, r16, r17 }
	{ andi r5, r6, 5 ; ill ; st2 r25, r26 }
	{ andi r5, r6, 5 ; jalr r15 ; st1 r25, r26 }
	{ andi r5, r6, 5 ; jr r15 ; st4 r25, r26 }
	{ andi r5, r6, 5 ; ld r25, r26 ; jalrp r15 }
	{ andi r5, r6, 5 ; ld1s r25, r26 ; cmplts r15, r16, r17 }
	{ andi r5, r6, 5 ; ld1u r25, r26 ; addi r15, r16, 5 }
	{ andi r5, r6, 5 ; ld1u r25, r26 ; shru r15, r16, r17 }
	{ andi r5, r6, 5 ; ld2s r25, r26 ; shl1add r15, r16, r17 }
	{ andi r5, r6, 5 ; ld2u r25, r26 ; move r15, r16 }
	{ andi r5, r6, 5 ; ld4s r25, r26 ; fnop }
	{ andi r5, r6, 5 ; ld4u r25, r26 ; andi r15, r16, 5 }
	{ andi r5, r6, 5 ; ld4u r25, r26 ; xor r15, r16, r17 }
	{ andi r5, r6, 5 ; mfspr r16, 0x5 }
	{ andi r5, r6, 5 ; movei r15, 5 ; ld1s r25, r26 }
	{ andi r5, r6, 5 ; nop ; ld1s r25, r26 }
	{ andi r5, r6, 5 ; or r15, r16, r17 ; ld2s r25, r26 }
	{ andi r5, r6, 5 ; prefetch r25 ; mnz r15, r16, r17 }
	{ andi r5, r6, 5 ; prefetch_l1 r25 ; cmples r15, r16, r17 }
	{ andi r5, r6, 5 ; prefetch_l1_fault r25 ; add r15, r16, r17 }
	{ andi r5, r6, 5 ; prefetch_l1_fault r25 ; shrsi r15, r16, 5 }
	{ andi r5, r6, 5 ; prefetch_l2 r25 ; shl1add r15, r16, r17 }
	{ andi r5, r6, 5 ; prefetch_l2_fault r25 ; movei r15, 5 }
	{ andi r5, r6, 5 ; prefetch_l3 r25 ; info 19 }
	{ andi r5, r6, 5 ; prefetch_l3_fault r25 ; cmples r15, r16, r17 }
	{ andi r5, r6, 5 ; rotl r15, r16, r17 ; ld r25, r26 }
	{ andi r5, r6, 5 ; shl r15, r16, r17 ; ld1u r25, r26 }
	{ andi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
	{ andi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
	{ andi r5, r6, 5 ; shl3addx r15, r16, r17 ; prefetch r25 }
	{ andi r5, r6, 5 ; shrs r15, r16, r17 ; prefetch r25 }
	{ andi r5, r6, 5 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
	{ andi r5, r6, 5 ; st r25, r26 ; cmples r15, r16, r17 }
	{ andi r5, r6, 5 ; st1 r25, r26 ; add r15, r16, r17 }
	{ andi r5, r6, 5 ; st1 r25, r26 ; shrsi r15, r16, 5 }
	{ andi r5, r6, 5 ; st2 r25, r26 ; shl r15, r16, r17 }
	{ andi r5, r6, 5 ; st4 r25, r26 ; mnz r15, r16, r17 }
	{ andi r5, r6, 5 ; sub r15, r16, r17 ; ld4s r25, r26 }
	{ andi r5, r6, 5 ; v1cmpleu r15, r16, r17 }
	{ andi r5, r6, 5 ; v2mnz r15, r16, r17 }
	{ andi r5, r6, 5 ; xor r15, r16, r17 ; st r25, r26 }
	{ bfexts r5, r6, 5, 7 ; finv r15 }
	{ bfexts r5, r6, 5, 7 ; ldnt4s_add r15, r16, 5 }
	{ bfexts r5, r6, 5, 7 ; shl3addx r15, r16, r17 }
	{ bfexts r5, r6, 5, 7 ; v1cmpne r15, r16, r17 }
	{ bfexts r5, r6, 5, 7 ; v2shl r15, r16, r17 }
	{ bfextu r5, r6, 5, 7 ; cmpltu r15, r16, r17 }
	{ bfextu r5, r6, 5, 7 ; ld4s r15, r16 }
	{ bfextu r5, r6, 5, 7 ; prefetch_add_l3_fault r15, 5 }
	{ bfextu r5, r6, 5, 7 ; stnt4 r15, r16 }
	{ bfextu r5, r6, 5, 7 ; v2cmpleu r15, r16, r17 }
	{ bfins r5, r6, 5, 7 ; add r15, r16, r17 }
	{ bfins r5, r6, 5, 7 ; info 19 }
	{ bfins r5, r6, 5, 7 ; mfspr r16, 0x5 }
	{ bfins r5, r6, 5, 7 ; shru r15, r16, r17 }
	{ bfins r5, r6, 5, 7 ; v1minui r15, r16, 5 }
	{ bfins r5, r6, 5, 7 ; v2shrui r15, r16, 5 }
	{ clz r5, r6 ; addi r15, r16, 5 ; ld2s r25, r26 }
	{ clz r5, r6 ; addxi r15, r16, 5 ; ld2u r25, r26 }
	{ clz r5, r6 ; andi r15, r16, 5 ; ld2u r25, r26 }
	{ clz r5, r6 ; cmpeqi r15, r16, 5 ; ld4u r25, r26 }
	{ clz r5, r6 ; cmpleu r15, r16, r17 ; ld4u r25, r26 }
	{ clz r5, r6 ; cmpltsi r15, r16, 5 ; prefetch_l1 r25 }
	{ clz r5, r6 ; cmpne r15, r16, r17 ; prefetch_l1_fault r25 }
	{ clz r5, r6 ; fnop ; prefetch_l3_fault r25 }
	{ clz r5, r6 ; info 19 ; st r25, r26 }
	{ clz r5, r6 ; jalrp r15 ; prefetch_l3_fault r25 }
	{ clz r5, r6 ; jrp r15 ; st1 r25, r26 }
	{ clz r5, r6 ; ld r25, r26 ; shl2addx r15, r16, r17 }
	{ clz r5, r6 ; ld1s r25, r26 ; nor r15, r16, r17 }
	{ clz r5, r6 ; ld1u r25, r26 ; jalrp r15 }
	{ clz r5, r6 ; ld2s r25, r26 ; cmpleu r15, r16, r17 }
	{ clz r5, r6 ; ld2u r25, r26 ; add r15, r16, r17 }
	{ clz r5, r6 ; ld2u r25, r26 ; shrsi r15, r16, 5 }
	{ clz r5, r6 ; ld4s r25, r26 ; shl r15, r16, r17 }
	{ clz r5, r6 ; ld4u r25, r26 ; mnz r15, r16, r17 }
	{ clz r5, r6 ; ldnt4u r15, r16 }
	{ clz r5, r6 ; mnz r15, r16, r17 ; st2 r25, r26 }
	{ clz r5, r6 ; movei r15, 5 }
	{ clz r5, r6 ; nop }
	{ clz r5, r6 ; prefetch r15 }
	{ clz r5, r6 ; prefetch r25 ; shrs r15, r16, r17 }
	{ clz r5, r6 ; prefetch_l1 r25 ; mz r15, r16, r17 }
	{ clz r5, r6 ; prefetch_l1_fault r25 ; jalr r15 }
	{ clz r5, r6 ; prefetch_l2 r25 ; cmpleu r15, r16, r17 }
	{ clz r5, r6 ; prefetch_l2_fault r25 ; addi r15, r16, 5 }
	{ clz r5, r6 ; prefetch_l2_fault r25 ; shru r15, r16, r17 }
	{ clz r5, r6 ; prefetch_l3 r25 ; shl1addx r15, r16, r17 }
	{ clz r5, r6 ; prefetch_l3_fault r25 ; mz r15, r16, r17 }
	{ clz r5, r6 ; rotl r15, r16, r17 ; st4 r25, r26 }
	{ clz r5, r6 ; shl16insli r15, r16, 0x1234 }
	{ clz r5, r6 ; shl2add r15, r16, r17 ; ld1s r25, r26 }
	{ clz r5, r6 ; shl3add r15, r16, r17 ; ld2s r25, r26 }
	{ clz r5, r6 ; shli r15, r16, 5 ; ld4s r25, r26 }
	{ clz r5, r6 ; shrsi r15, r16, 5 ; ld4s r25, r26 }
	{ clz r5, r6 ; shrui r15, r16, 5 ; prefetch r25 }
	{ clz r5, r6 ; st r25, r26 ; mz r15, r16, r17 }
	{ clz r5, r6 ; st1 r25, r26 ; jalr r15 }
	{ clz r5, r6 ; st2 r25, r26 ; cmples r15, r16, r17 }
	{ clz r5, r6 ; st4 r15, r16 }
	{ clz r5, r6 ; st4 r25, r26 ; shrs r15, r16, r17 }
	{ clz r5, r6 ; subx r15, r16, r17 ; ld2s r25, r26 }
	{ clz r5, r6 ; v1shrsi r15, r16, 5 }
	{ clz r5, r6 ; v4int_l r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; add r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmoveqz r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l3 r25 }
	{ cmoveqz r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3 r25 }
	{ cmoveqz r5, r6, r7 ; cmpeq r15, r16, r17 ; st r25, r26 }
	{ cmoveqz r5, r6, r7 ; cmples r15, r16, r17 ; st r25, r26 }
	{ cmoveqz r5, r6, r7 ; cmplts r15, r16, r17 ; st2 r25, r26 }
	{ cmoveqz r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; fnop ; ld1u r25, r26 }
	{ cmoveqz r5, r6, r7 ; info 19 ; ld2s r25, r26 }
	{ cmoveqz r5, r6, r7 ; jalrp r15 ; ld1u r25, r26 }
	{ cmoveqz r5, r6, r7 ; jrp r15 ; ld2u r25, r26 }
	{ cmoveqz r5, r6, r7 ; ld r25, r26 ; movei r15, 5 }
	{ cmoveqz r5, r6, r7 ; ld1s r25, r26 ; info 19 }
	{ cmoveqz r5, r6, r7 ; ld1u r25, r26 ; cmpeqi r15, r16, 5 }
	{ cmoveqz r5, r6, r7 ; ld1u_add r15, r16, 5 }
	{ cmoveqz r5, r6, r7 ; ld2s r25, r26 ; shli r15, r16, 5 }
	{ cmoveqz r5, r6, r7 ; ld2u r25, r26 ; rotl r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; ld4s r25, r26 ; jrp r15 }
	{ cmoveqz r5, r6, r7 ; ld4u r25, r26 ; cmpltsi r15, r16, 5 }
	{ cmoveqz r5, r6, r7 ; ldnt r15, r16 }
	{ cmoveqz r5, r6, r7 ; mnz r15, r16, r17 ; ld4s r25, r26 }
	{ cmoveqz r5, r6, r7 ; movei r15, 5 ; prefetch r25 }
	{ cmoveqz r5, r6, r7 ; nop ; prefetch r25 }
	{ cmoveqz r5, r6, r7 ; or r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmoveqz r5, r6, r7 ; prefetch r25 ; or r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; prefetch_l1 r25 ; fnop }
	{ cmoveqz r5, r6, r7 ; prefetch_l1_fault r25 ; cmpeq r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; prefetch_l1_fault r25 }
	{ cmoveqz r5, r6, r7 ; prefetch_l2 r25 ; shli r15, r16, 5 }
	{ cmoveqz r5, r6, r7 ; prefetch_l2_fault r25 ; rotli r15, r16, 5 }
	{ cmoveqz r5, r6, r7 ; prefetch_l3 r25 ; mnz r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; prefetch_l3_fault r25 ; fnop }
	{ cmoveqz r5, r6, r7 ; rotl r15, r16, r17 ; ld4u r25, r26 }
	{ cmoveqz r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l1 r25 }
	{ cmoveqz r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmoveqz r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmoveqz r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3_fault r25 }
	{ cmoveqz r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3_fault r25 }
	{ cmoveqz r5, r6, r7 ; shru r15, r16, r17 ; st1 r25, r26 }
	{ cmoveqz r5, r6, r7 ; st r25, r26 ; fnop }
	{ cmoveqz r5, r6, r7 ; st1 r25, r26 ; cmpeq r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; st1 r25, r26 }
	{ cmoveqz r5, r6, r7 ; st2 r25, r26 ; shl3addx r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; st4 r25, r26 ; or r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmoveqz r5, r6, r7 ; v1int_h r15, r16, r17 }
	{ cmoveqz r5, r6, r7 ; v2shli r15, r16, 5 }
	{ cmovnez r5, r6, r7 ; add r15, r16, r17 ; ld r25, r26 }
	{ cmovnez r5, r6, r7 ; addx r15, r16, r17 ; ld1s r25, r26 }
	{ cmovnez r5, r6, r7 ; and r15, r16, r17 ; ld1s r25, r26 }
	{ cmovnez r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
	{ cmovnez r5, r6, r7 ; cmples r15, r16, r17 ; ld2s r25, r26 }
	{ cmovnez r5, r6, r7 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
	{ cmovnez r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
	{ cmovnez r5, r6, r7 ; fetchaddgez r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
	{ cmovnez r5, r6, r7 ; jalr r15 ; prefetch_l2 r25 }
	{ cmovnez r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
	{ cmovnez r5, r6, r7 ; ld r25, r26 ; cmpne r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; ld1s r25, r26 ; andi r15, r16, 5 }
	{ cmovnez r5, r6, r7 ; ld1s r25, r26 ; xor r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; ld1u r25, r26 ; shl3add r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; ld2s r25, r26 ; nor r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; ld2u r25, r26 ; jalrp r15 }
	{ cmovnez r5, r6, r7 ; ld4s r25, r26 ; cmpleu r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; ld4u r25, r26 ; add r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; ld4u r25, r26 ; shrsi r15, r16, 5 }
	{ cmovnez r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
	{ cmovnez r5, r6, r7 ; move r15, r16 ; st1 r25, r26 }
	{ cmovnez r5, r6, r7 ; mz r15, r16, r17 ; st1 r25, r26 }
	{ cmovnez r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
	{ cmovnez r5, r6, r7 ; prefetch r25 ; jalr r15 }
	{ cmovnez r5, r6, r7 ; prefetch_l1 r25 ; addxi r15, r16, 5 }
	{ cmovnez r5, r6, r7 ; prefetch_l1 r25 ; sub r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; prefetch_l1_fault r25 ; shl2addx r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; prefetch_l2 r25 ; nor r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; prefetch_l2_fault r25 ; jr r15 }
	{ cmovnez r5, r6, r7 ; prefetch_l3 r25 ; cmpltsi r15, r16, 5 }
	{ cmovnez r5, r6, r7 ; prefetch_l3_fault r25 ; addxi r15, r16, 5 }
	{ cmovnez r5, r6, r7 ; prefetch_l3_fault r25 ; sub r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
	{ cmovnez r5, r6, r7 ; shl1add r15, r16, r17 ; st4 r25, r26 }
	{ cmovnez r5, r6, r7 ; shl2addx r15, r16, r17 ; ld r25, r26 }
	{ cmovnez r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
	{ cmovnez r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
	{ cmovnez r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
	{ cmovnez r5, r6, r7 ; st r25, r26 ; addxi r15, r16, 5 }
	{ cmovnez r5, r6, r7 ; st r25, r26 ; sub r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; st1 r25, r26 ; shl2addx r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; st2 r25, r26 ; nop }
	{ cmovnez r5, r6, r7 ; st4 r25, r26 ; jalr r15 }
	{ cmovnez r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
	{ cmovnez r5, r6, r7 ; v1addi r15, r16, 5 }
	{ cmovnez r5, r6, r7 ; v2int_l r15, r16, r17 }
	{ cmovnez r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmpeq r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l2 r25 }
	{ cmpeq r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l2_fault r25 }
	{ cmpeq r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l2_fault r25 }
	{ cmpeq r15, r16, r17 ; cmoveqz r5, r6, r7 ; prefetch_l2 r25 }
	{ cmpeq r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpeq r15, r16, r17 ; cmples r5, r6, r7 ; st r25, r26 }
	{ cmpeq r15, r16, r17 ; cmplts r5, r6, r7 ; st2 r25, r26 }
	{ cmpeq r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; ctz r5, r6 ; prefetch_l2 r25 }
	{ cmpeq r15, r16, r17 ; fsingle_add1 r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; info 19 ; st1 r25, r26 }
	{ cmpeq r15, r16, r17 ; ld r25, r26 ; nop }
	{ cmpeq r15, r16, r17 ; ld1s r25, r26 ; cmpleu r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; ld1s r25, r26 ; shrsi r5, r6, 5 }
	{ cmpeq r15, r16, r17 ; ld1u r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; ld2s r25, r26 ; clz r5, r6 }
	{ cmpeq r15, r16, r17 ; ld2s r25, r26 ; shl2add r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; ld2u r25, r26 ; movei r5, 5 }
	{ cmpeq r15, r16, r17 ; ld4s r25, r26 ; add r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; ld4s r25, r26 ; revbytes r5, r6 }
	{ cmpeq r15, r16, r17 ; ld4u r25, r26 ; ctz r5, r6 }
	{ cmpeq r15, r16, r17 ; ld4u r25, r26 ; tblidxb0 r5, r6 }
	{ cmpeq r15, r16, r17 ; move r5, r6 ; st r25, r26 }
	{ cmpeq r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; st1 r25, r26 }
	{ cmpeq r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpeq r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmpeq r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; prefetch_l2 r25 }
	{ cmpeq r15, r16, r17 ; mulax r5, r6, r7 ; prefetch_l2_fault r25 }
	{ cmpeq r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmpeq r15, r16, r17 ; nor r5, r6, r7 ; st1 r25, r26 }
	{ cmpeq r15, r16, r17 ; pcnt r5, r6 ; st2 r25, r26 }
	{ cmpeq r15, r16, r17 ; prefetch r25 ; or r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; prefetch_l1 r25 ; cmpltsi r5, r6, 5 }
	{ cmpeq r15, r16, r17 ; prefetch_l1 r25 ; shrui r5, r6, 5 }
	{ cmpeq r15, r16, r17 ; prefetch_l1_fault r25 ; mula_lu_lu r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; prefetch_l2 r25 ; cmovnez r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; prefetch_l2 r25 ; shl3add r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; prefetch_l2_fault r25 ; mul_hu_hu r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; prefetch_l3 r25 ; addx r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; prefetch_l3 r25 ; rotli r5, r6, 5 }
	{ cmpeq r15, r16, r17 ; prefetch_l3_fault r25 ; fsingle_pack1 r5, r6 }
	{ cmpeq r15, r16, r17 ; prefetch_l3_fault r25 ; tblidxb2 r5, r6 }
	{ cmpeq r15, r16, r17 ; revbytes r5, r6 ; st4 r25, r26 }
	{ cmpeq r15, r16, r17 ; shl r5, r6, r7 ; ld r25, r26 }
	{ cmpeq r15, r16, r17 ; shl1addx r5, r6, r7 ; ld1s r25, r26 }
	{ cmpeq r15, r16, r17 ; shl2addx r5, r6, r7 ; ld2s r25, r26 }
	{ cmpeq r15, r16, r17 ; shl3addx r5, r6, r7 ; ld4s r25, r26 }
	{ cmpeq r15, r16, r17 ; shrs r5, r6, r7 ; ld4s r25, r26 }
	{ cmpeq r15, r16, r17 ; shru r5, r6, r7 ; prefetch r25 }
	{ cmpeq r15, r16, r17 ; st r25, r26 ; clz r5, r6 }
	{ cmpeq r15, r16, r17 ; st r25, r26 ; shl2add r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; st1 r25, r26 ; movei r5, 5 }
	{ cmpeq r15, r16, r17 ; st2 r25, r26 ; add r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; st2 r25, r26 ; revbytes r5, r6 }
	{ cmpeq r15, r16, r17 ; st4 r25, r26 ; ctz r5, r6 }
	{ cmpeq r15, r16, r17 ; st4 r25, r26 ; tblidxb0 r5, r6 }
	{ cmpeq r15, r16, r17 ; subx r5, r6, r7 ; st1 r25, r26 }
	{ cmpeq r15, r16, r17 ; tblidxb1 r5, r6 ; st2 r25, r26 }
	{ cmpeq r15, r16, r17 ; tblidxb3 r5, r6 }
	{ cmpeq r15, r16, r17 ; v1shrs r5, r6, r7 }
	{ cmpeq r15, r16, r17 ; v2shl r5, r6, r7 }
	{ cmpeq r5, r6, r7 ; add r15, r16, r17 ; ld r25, r26 }
	{ cmpeq r5, r6, r7 ; addx r15, r16, r17 ; ld1s r25, r26 }
	{ cmpeq r5, r6, r7 ; and r15, r16, r17 ; ld1s r25, r26 }
	{ cmpeq r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
	{ cmpeq r5, r6, r7 ; cmples r15, r16, r17 ; ld2s r25, r26 }
	{ cmpeq r5, r6, r7 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
	{ cmpeq r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
	{ cmpeq r5, r6, r7 ; fetchaddgez r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
	{ cmpeq r5, r6, r7 ; jalr r15 ; prefetch_l2 r25 }
	{ cmpeq r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
	{ cmpeq r5, r6, r7 ; ld r25, r26 ; cmpne r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; ld1s r25, r26 ; andi r15, r16, 5 }
	{ cmpeq r5, r6, r7 ; ld1s r25, r26 ; xor r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; ld1u r25, r26 ; shl3add r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; ld2s r25, r26 ; nor r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; ld2u r25, r26 ; jalrp r15 }
	{ cmpeq r5, r6, r7 ; ld4s r25, r26 ; cmpleu r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; ld4u r25, r26 ; add r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; ld4u r25, r26 ; shrsi r15, r16, 5 }
	{ cmpeq r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
	{ cmpeq r5, r6, r7 ; move r15, r16 ; st1 r25, r26 }
	{ cmpeq r5, r6, r7 ; mz r15, r16, r17 ; st1 r25, r26 }
	{ cmpeq r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
	{ cmpeq r5, r6, r7 ; prefetch r25 ; jalr r15 }
	{ cmpeq r5, r6, r7 ; prefetch_l1 r25 ; addxi r15, r16, 5 }
	{ cmpeq r5, r6, r7 ; prefetch_l1 r25 ; sub r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; prefetch_l1_fault r25 ; shl2addx r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; prefetch_l2 r25 ; nor r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; prefetch_l2_fault r25 ; jr r15 }
	{ cmpeq r5, r6, r7 ; prefetch_l3 r25 ; cmpltsi r15, r16, 5 }
	{ cmpeq r5, r6, r7 ; prefetch_l3_fault r25 ; addxi r15, r16, 5 }
	{ cmpeq r5, r6, r7 ; prefetch_l3_fault r25 ; sub r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
	{ cmpeq r5, r6, r7 ; shl1add r15, r16, r17 ; st4 r25, r26 }
	{ cmpeq r5, r6, r7 ; shl2addx r15, r16, r17 ; ld r25, r26 }
	{ cmpeq r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
	{ cmpeq r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
	{ cmpeq r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
	{ cmpeq r5, r6, r7 ; st r25, r26 ; addxi r15, r16, 5 }
	{ cmpeq r5, r6, r7 ; st r25, r26 ; sub r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; st1 r25, r26 ; shl2addx r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; st2 r25, r26 ; nop }
	{ cmpeq r5, r6, r7 ; st4 r25, r26 ; jalr r15 }
	{ cmpeq r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
	{ cmpeq r5, r6, r7 ; v1addi r15, r16, 5 }
	{ cmpeq r5, r6, r7 ; v2int_l r15, r16, r17 }
	{ cmpeq r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmpeqi r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l2 r25 }
	{ cmpeqi r15, r16, 5 ; addxi r5, r6, 5 ; prefetch_l2_fault r25 }
	{ cmpeqi r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l2_fault r25 }
	{ cmpeqi r15, r16, 5 ; cmoveqz r5, r6, r7 ; prefetch_l2 r25 }
	{ cmpeqi r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpeqi r15, r16, 5 ; cmples r5, r6, r7 ; st r25, r26 }
	{ cmpeqi r15, r16, 5 ; cmplts r5, r6, r7 ; st2 r25, r26 }
	{ cmpeqi r15, r16, 5 ; cmpltu r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; ctz r5, r6 ; prefetch_l2 r25 }
	{ cmpeqi r15, r16, 5 ; fsingle_add1 r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; info 19 ; st1 r25, r26 }
	{ cmpeqi r15, r16, 5 ; ld r25, r26 ; nop }
	{ cmpeqi r15, r16, 5 ; ld1s r25, r26 ; cmpleu r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; ld1s r25, r26 ; shrsi r5, r6, 5 }
	{ cmpeqi r15, r16, 5 ; ld1u r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; ld2s r25, r26 ; clz r5, r6 }
	{ cmpeqi r15, r16, 5 ; ld2s r25, r26 ; shl2add r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; ld2u r25, r26 ; movei r5, 5 }
	{ cmpeqi r15, r16, 5 ; ld4s r25, r26 ; add r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; ld4s r25, r26 ; revbytes r5, r6 }
	{ cmpeqi r15, r16, 5 ; ld4u r25, r26 ; ctz r5, r6 }
	{ cmpeqi r15, r16, 5 ; ld4u r25, r26 ; tblidxb0 r5, r6 }
	{ cmpeqi r15, r16, 5 ; move r5, r6 ; st r25, r26 }
	{ cmpeqi r15, r16, 5 ; mul_hs_hs r5, r6, r7 ; st1 r25, r26 }
	{ cmpeqi r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpeqi r15, r16, 5 ; mula_hs_hs r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmpeqi r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; prefetch_l2 r25 }
	{ cmpeqi r15, r16, 5 ; mulax r5, r6, r7 ; prefetch_l2_fault r25 }
	{ cmpeqi r15, r16, 5 ; mz r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmpeqi r15, r16, 5 ; nor r5, r6, r7 ; st1 r25, r26 }
	{ cmpeqi r15, r16, 5 ; pcnt r5, r6 ; st2 r25, r26 }
	{ cmpeqi r15, r16, 5 ; prefetch r25 ; or r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; prefetch_l1 r25 ; cmpltsi r5, r6, 5 }
	{ cmpeqi r15, r16, 5 ; prefetch_l1 r25 ; shrui r5, r6, 5 }
	{ cmpeqi r15, r16, 5 ; prefetch_l1_fault r25 ; mula_lu_lu r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; prefetch_l2 r25 ; cmovnez r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; prefetch_l2 r25 ; shl3add r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 ; mul_hu_hu r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; prefetch_l3 r25 ; addx r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; prefetch_l3 r25 ; rotli r5, r6, 5 }
	{ cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 ; fsingle_pack1 r5, r6 }
	{ cmpeqi r15, r16, 5 ; prefetch_l3_fault r25 ; tblidxb2 r5, r6 }
	{ cmpeqi r15, r16, 5 ; revbytes r5, r6 ; st4 r25, r26 }
	{ cmpeqi r15, r16, 5 ; shl r5, r6, r7 ; ld r25, r26 }
	{ cmpeqi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld1s r25, r26 }
	{ cmpeqi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld2s r25, r26 }
	{ cmpeqi r15, r16, 5 ; shl3addx r5, r6, r7 ; ld4s r25, r26 }
	{ cmpeqi r15, r16, 5 ; shrs r5, r6, r7 ; ld4s r25, r26 }
	{ cmpeqi r15, r16, 5 ; shru r5, r6, r7 ; prefetch r25 }
	{ cmpeqi r15, r16, 5 ; st r25, r26 ; clz r5, r6 }
	{ cmpeqi r15, r16, 5 ; st r25, r26 ; shl2add r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; st1 r25, r26 ; movei r5, 5 }
	{ cmpeqi r15, r16, 5 ; st2 r25, r26 ; add r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; st2 r25, r26 ; revbytes r5, r6 }
	{ cmpeqi r15, r16, 5 ; st4 r25, r26 ; ctz r5, r6 }
	{ cmpeqi r15, r16, 5 ; st4 r25, r26 ; tblidxb0 r5, r6 }
	{ cmpeqi r15, r16, 5 ; subx r5, r6, r7 ; st1 r25, r26 }
	{ cmpeqi r15, r16, 5 ; tblidxb1 r5, r6 ; st2 r25, r26 }
	{ cmpeqi r15, r16, 5 ; tblidxb3 r5, r6 }
	{ cmpeqi r15, r16, 5 ; v1shrs r5, r6, r7 }
	{ cmpeqi r15, r16, 5 ; v2shl r5, r6, r7 }
	{ cmpeqi r5, r6, 5 ; add r15, r16, r17 ; ld r25, r26 }
	{ cmpeqi r5, r6, 5 ; addx r15, r16, r17 ; ld1s r25, r26 }
	{ cmpeqi r5, r6, 5 ; and r15, r16, r17 ; ld1s r25, r26 }
	{ cmpeqi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
	{ cmpeqi r5, r6, 5 ; cmples r15, r16, r17 ; ld2s r25, r26 }
	{ cmpeqi r5, r6, 5 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
	{ cmpeqi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch r25 }
	{ cmpeqi r5, r6, 5 ; fetchaddgez r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; ill ; prefetch_l2_fault r25 }
	{ cmpeqi r5, r6, 5 ; jalr r15 ; prefetch_l2 r25 }
	{ cmpeqi r5, r6, 5 ; jr r15 ; prefetch_l3 r25 }
	{ cmpeqi r5, r6, 5 ; ld r25, r26 ; cmpne r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; ld1s r25, r26 ; andi r15, r16, 5 }
	{ cmpeqi r5, r6, 5 ; ld1s r25, r26 ; xor r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; ld1u r25, r26 ; shl3add r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; ld2s r25, r26 ; nor r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; ld2u r25, r26 ; jalrp r15 }
	{ cmpeqi r5, r6, 5 ; ld4s r25, r26 ; cmpleu r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; ld4u r25, r26 ; add r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; ld4u r25, r26 ; shrsi r15, r16, 5 }
	{ cmpeqi r5, r6, 5 ; lnk r15 ; st1 r25, r26 }
	{ cmpeqi r5, r6, 5 ; move r15, r16 ; st1 r25, r26 }
	{ cmpeqi r5, r6, 5 ; mz r15, r16, r17 ; st1 r25, r26 }
	{ cmpeqi r5, r6, 5 ; nor r15, r16, r17 ; st4 r25, r26 }
	{ cmpeqi r5, r6, 5 ; prefetch r25 ; jalr r15 }
	{ cmpeqi r5, r6, 5 ; prefetch_l1 r25 ; addxi r15, r16, 5 }
	{ cmpeqi r5, r6, 5 ; prefetch_l1 r25 ; sub r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; prefetch_l1_fault r25 ; shl2addx r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; prefetch_l2 r25 ; nor r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; prefetch_l2_fault r25 ; jr r15 }
	{ cmpeqi r5, r6, 5 ; prefetch_l3 r25 ; cmpltsi r15, r16, 5 }
	{ cmpeqi r5, r6, 5 ; prefetch_l3_fault r25 ; addxi r15, r16, 5 }
	{ cmpeqi r5, r6, 5 ; prefetch_l3_fault r25 ; sub r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; rotli r15, r16, 5 ; st2 r25, r26 }
	{ cmpeqi r5, r6, 5 ; shl1add r15, r16, r17 ; st4 r25, r26 }
	{ cmpeqi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld r25, r26 }
	{ cmpeqi r5, r6, 5 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
	{ cmpeqi r5, r6, 5 ; shrs r15, r16, r17 ; ld1u r25, r26 }
	{ cmpeqi r5, r6, 5 ; shru r15, r16, r17 ; ld2u r25, r26 }
	{ cmpeqi r5, r6, 5 ; st r25, r26 ; addxi r15, r16, 5 }
	{ cmpeqi r5, r6, 5 ; st r25, r26 ; sub r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; st1 r25, r26 ; shl2addx r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; st2 r25, r26 ; nop }
	{ cmpeqi r5, r6, 5 ; st4 r25, r26 ; jalr r15 }
	{ cmpeqi r5, r6, 5 ; sub r15, r16, r17 ; ld r25, r26 }
	{ cmpeqi r5, r6, 5 ; v1addi r15, r16, 5 }
	{ cmpeqi r5, r6, 5 ; v2int_l r15, r16, r17 }
	{ cmpeqi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmpexch r15, r16, r17 ; cmulh r5, r6, r7 }
	{ cmpexch r15, r16, r17 ; mul_ls_lu r5, r6, r7 }
	{ cmpexch r15, r16, r17 ; shruxi r5, r6, 5 }
	{ cmpexch r15, r16, r17 ; v1multu r5, r6, r7 }
	{ cmpexch r15, r16, r17 ; v2mz r5, r6, r7 }
	{ cmpexch4 r15, r16, r17 ; bfextu r5, r6, 5, 7 }
	{ cmpexch4 r15, r16, r17 ; fsingle_mul2 r5, r6, r7 }
	{ cmpexch4 r15, r16, r17 ; revbytes r5, r6 }
	{ cmpexch4 r15, r16, r17 ; v1cmpltui r5, r6, 5 }
	{ cmpexch4 r15, r16, r17 ; v2cmples r5, r6, r7 }
	{ cmpexch4 r15, r16, r17 ; v4packsc r5, r6, r7 }
	{ cmples r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3 r25 }
	{ cmples r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmples r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmples r15, r16, r17 ; cmoveqz r5, r6, r7 ; prefetch_l3 r25 }
	{ cmples r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
	{ cmples r15, r16, r17 ; cmples r5, r6, r7 ; st2 r25, r26 }
	{ cmples r15, r16, r17 ; cmplts r5, r6, r7 }
	{ cmples r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
	{ cmples r15, r16, r17 ; ctz r5, r6 ; prefetch_l3 r25 }
	{ cmples r15, r16, r17 ; fsingle_mul1 r5, r6, r7 }
	{ cmples r15, r16, r17 ; info 19 ; st4 r25, r26 }
	{ cmples r15, r16, r17 ; ld r25, r26 ; or r5, r6, r7 }
	{ cmples r15, r16, r17 ; ld1s r25, r26 ; cmpltsi r5, r6, 5 }
	{ cmples r15, r16, r17 ; ld1s r25, r26 ; shrui r5, r6, 5 }
	{ cmples r15, r16, r17 ; ld1u r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ cmples r15, r16, r17 ; ld2s r25, r26 ; cmovnez r5, r6, r7 }
	{ cmples r15, r16, r17 ; ld2s r25, r26 ; shl3add r5, r6, r7 }
	{ cmples r15, r16, r17 ; ld2u r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmples r15, r16, r17 ; ld4s r25, r26 ; addx r5, r6, r7 }
	{ cmples r15, r16, r17 ; ld4s r25, r26 ; rotli r5, r6, 5 }
	{ cmples r15, r16, r17 ; ld4u r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmples r15, r16, r17 ; ld4u r25, r26 ; tblidxb2 r5, r6 }
	{ cmples r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
	{ cmples r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; st4 r25, r26 }
	{ cmples r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; st r25, r26 }
	{ cmples r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; st1 r25, r26 }
	{ cmples r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; prefetch_l3 r25 }
	{ cmples r15, r16, r17 ; mulax r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmples r15, r16, r17 ; mz r5, r6, r7 ; st1 r25, r26 }
	{ cmples r15, r16, r17 ; nor r5, r6, r7 ; st4 r25, r26 }
	{ cmples r15, r16, r17 ; pcnt r5, r6 }
	{ cmples r15, r16, r17 ; prefetch r25 ; revbits r5, r6 }
	{ cmples r15, r16, r17 ; prefetch_l1 r25 ; cmpne r5, r6, r7 }
	{ cmples r15, r16, r17 ; prefetch_l1 r25 ; subx r5, r6, r7 }
	{ cmples r15, r16, r17 ; prefetch_l1_fault r25 ; mulx r5, r6, r7 }
	{ cmples r15, r16, r17 ; prefetch_l2 r25 ; cmpeqi r5, r6, 5 }
	{ cmples r15, r16, r17 ; prefetch_l2 r25 ; shli r5, r6, 5 }
	{ cmples r15, r16, r17 ; prefetch_l2_fault r25 ; mul_lu_lu r5, r6, r7 }
	{ cmples r15, r16, r17 ; prefetch_l3 r25 ; and r5, r6, r7 }
	{ cmples r15, r16, r17 ; prefetch_l3 r25 ; shl1add r5, r6, r7 }
	{ cmples r15, r16, r17 ; prefetch_l3_fault r25 ; mnz r5, r6, r7 }
	{ cmples r15, r16, r17 ; prefetch_l3_fault r25 ; xor r5, r6, r7 }
	{ cmples r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
	{ cmples r15, r16, r17 ; shl r5, r6, r7 ; ld1u r25, r26 }
	{ cmples r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
	{ cmples r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
	{ cmples r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
	{ cmples r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
	{ cmples r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
	{ cmples r15, r16, r17 ; st r25, r26 ; cmovnez r5, r6, r7 }
	{ cmples r15, r16, r17 ; st r25, r26 ; shl3add r5, r6, r7 }
	{ cmples r15, r16, r17 ; st1 r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmples r15, r16, r17 ; st2 r25, r26 ; addx r5, r6, r7 }
	{ cmples r15, r16, r17 ; st2 r25, r26 ; rotli r5, r6, 5 }
	{ cmples r15, r16, r17 ; st4 r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmples r15, r16, r17 ; st4 r25, r26 ; tblidxb2 r5, r6 }
	{ cmples r15, r16, r17 ; subx r5, r6, r7 ; st4 r25, r26 }
	{ cmples r15, r16, r17 ; tblidxb1 r5, r6 }
	{ cmples r15, r16, r17 ; v1addi r5, r6, 5 }
	{ cmples r15, r16, r17 ; v1shru r5, r6, r7 }
	{ cmples r15, r16, r17 ; v2shlsc r5, r6, r7 }
	{ cmples r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
	{ cmples r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
	{ cmples r5, r6, r7 ; and r15, r16, r17 ; ld2s r25, r26 }
	{ cmples r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
	{ cmples r5, r6, r7 ; cmples r15, r16, r17 ; ld4s r25, r26 }
	{ cmples r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
	{ cmples r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmples r5, r6, r7 ; fetchand r15, r16, r17 }
	{ cmples r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
	{ cmples r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
	{ cmples r5, r6, r7 ; jr r15 ; st r25, r26 }
	{ cmples r5, r6, r7 ; ld r25, r26 ; ill }
	{ cmples r5, r6, r7 ; ld1s r25, r26 ; cmpeqi r15, r16, 5 }
	{ cmples r5, r6, r7 ; ld1s_add r15, r16, 5 }
	{ cmples r5, r6, r7 ; ld1u r25, r26 ; shli r15, r16, 5 }
	{ cmples r5, r6, r7 ; ld2s r25, r26 ; rotl r15, r16, r17 }
	{ cmples r5, r6, r7 ; ld2u r25, r26 ; jrp r15 }
	{ cmples r5, r6, r7 ; ld4s r25, r26 ; cmpltsi r15, r16, 5 }
	{ cmples r5, r6, r7 ; ld4u r25, r26 ; addx r15, r16, r17 }
	{ cmples r5, r6, r7 ; ld4u r25, r26 ; shrui r15, r16, 5 }
	{ cmples r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
	{ cmples r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
	{ cmples r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
	{ cmples r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
	{ cmples r5, r6, r7 ; prefetch r25 ; jr r15 }
	{ cmples r5, r6, r7 ; prefetch_l1 r25 ; andi r15, r16, 5 }
	{ cmples r5, r6, r7 ; prefetch_l1 r25 ; xor r15, r16, r17 }
	{ cmples r5, r6, r7 ; prefetch_l1_fault r25 ; shl3addx r15, r16, r17 }
	{ cmples r5, r6, r7 ; prefetch_l2 r25 ; rotl r15, r16, r17 }
	{ cmples r5, r6, r7 ; prefetch_l2_fault r25 ; lnk r15 }
	{ cmples r5, r6, r7 ; prefetch_l3 r25 ; cmpne r15, r16, r17 }
	{ cmples r5, r6, r7 ; prefetch_l3_fault r25 ; andi r15, r16, 5 }
	{ cmples r5, r6, r7 ; prefetch_l3_fault r25 ; xor r15, r16, r17 }
	{ cmples r5, r6, r7 ; rotli r15, r16, 5 }
	{ cmples r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
	{ cmples r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
	{ cmples r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
	{ cmples r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
	{ cmples r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
	{ cmples r5, r6, r7 ; st r25, r26 ; andi r15, r16, 5 }
	{ cmples r5, r6, r7 ; st r25, r26 ; xor r15, r16, r17 }
	{ cmples r5, r6, r7 ; st1 r25, r26 ; shl3addx r15, r16, r17 }
	{ cmples r5, r6, r7 ; st2 r25, r26 ; or r15, r16, r17 }
	{ cmples r5, r6, r7 ; st4 r25, r26 ; jr r15 }
	{ cmples r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
	{ cmples r5, r6, r7 ; v1cmpeq r15, r16, r17 }
	{ cmples r5, r6, r7 ; v2maxsi r15, r16, 5 }
	{ cmples r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmpleu r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3 r25 }
	{ cmpleu r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmpleu r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmpleu r15, r16, r17 ; cmoveqz r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpleu r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
	{ cmpleu r15, r16, r17 ; cmples r5, r6, r7 ; st2 r25, r26 }
	{ cmpleu r15, r16, r17 ; cmplts r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
	{ cmpleu r15, r16, r17 ; ctz r5, r6 ; prefetch_l3 r25 }
	{ cmpleu r15, r16, r17 ; fsingle_mul1 r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; info 19 ; st4 r25, r26 }
	{ cmpleu r15, r16, r17 ; ld r25, r26 ; or r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; ld1s r25, r26 ; cmpltsi r5, r6, 5 }
	{ cmpleu r15, r16, r17 ; ld1s r25, r26 ; shrui r5, r6, 5 }
	{ cmpleu r15, r16, r17 ; ld1u r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; ld2s r25, r26 ; cmovnez r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; ld2s r25, r26 ; shl3add r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; ld2u r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; ld4s r25, r26 ; addx r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; ld4s r25, r26 ; rotli r5, r6, 5 }
	{ cmpleu r15, r16, r17 ; ld4u r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmpleu r15, r16, r17 ; ld4u r25, r26 ; tblidxb2 r5, r6 }
	{ cmpleu r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
	{ cmpleu r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; st4 r25, r26 }
	{ cmpleu r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; st r25, r26 }
	{ cmpleu r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; st1 r25, r26 }
	{ cmpleu r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpleu r15, r16, r17 ; mulax r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmpleu r15, r16, r17 ; mz r5, r6, r7 ; st1 r25, r26 }
	{ cmpleu r15, r16, r17 ; nor r5, r6, r7 ; st4 r25, r26 }
	{ cmpleu r15, r16, r17 ; pcnt r5, r6 }
	{ cmpleu r15, r16, r17 ; prefetch r25 ; revbits r5, r6 }
	{ cmpleu r15, r16, r17 ; prefetch_l1 r25 ; cmpne r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; prefetch_l1 r25 ; subx r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; prefetch_l1_fault r25 ; mulx r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; prefetch_l2 r25 ; cmpeqi r5, r6, 5 }
	{ cmpleu r15, r16, r17 ; prefetch_l2 r25 ; shli r5, r6, 5 }
	{ cmpleu r15, r16, r17 ; prefetch_l2_fault r25 ; mul_lu_lu r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; prefetch_l3 r25 ; and r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; prefetch_l3 r25 ; shl1add r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; prefetch_l3_fault r25 ; mnz r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; prefetch_l3_fault r25 ; xor r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
	{ cmpleu r15, r16, r17 ; shl r5, r6, r7 ; ld1u r25, r26 }
	{ cmpleu r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
	{ cmpleu r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
	{ cmpleu r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
	{ cmpleu r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
	{ cmpleu r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
	{ cmpleu r15, r16, r17 ; st r25, r26 ; cmovnez r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; st r25, r26 ; shl3add r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; st1 r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; st2 r25, r26 ; addx r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; st2 r25, r26 ; rotli r5, r6, 5 }
	{ cmpleu r15, r16, r17 ; st4 r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmpleu r15, r16, r17 ; st4 r25, r26 ; tblidxb2 r5, r6 }
	{ cmpleu r15, r16, r17 ; subx r5, r6, r7 ; st4 r25, r26 }
	{ cmpleu r15, r16, r17 ; tblidxb1 r5, r6 }
	{ cmpleu r15, r16, r17 ; v1addi r5, r6, 5 }
	{ cmpleu r15, r16, r17 ; v1shru r5, r6, r7 }
	{ cmpleu r15, r16, r17 ; v2shlsc r5, r6, r7 }
	{ cmpleu r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
	{ cmpleu r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
	{ cmpleu r5, r6, r7 ; and r15, r16, r17 ; ld2s r25, r26 }
	{ cmpleu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
	{ cmpleu r5, r6, r7 ; cmples r15, r16, r17 ; ld4s r25, r26 }
	{ cmpleu r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
	{ cmpleu r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmpleu r5, r6, r7 ; fetchand r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
	{ cmpleu r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
	{ cmpleu r5, r6, r7 ; jr r15 ; st r25, r26 }
	{ cmpleu r5, r6, r7 ; ld r25, r26 ; ill }
	{ cmpleu r5, r6, r7 ; ld1s r25, r26 ; cmpeqi r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; ld1s_add r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; ld1u r25, r26 ; shli r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; ld2s r25, r26 ; rotl r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; ld2u r25, r26 ; jrp r15 }
	{ cmpleu r5, r6, r7 ; ld4s r25, r26 ; cmpltsi r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; ld4u r25, r26 ; addx r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; ld4u r25, r26 ; shrui r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
	{ cmpleu r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
	{ cmpleu r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
	{ cmpleu r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
	{ cmpleu r5, r6, r7 ; prefetch r25 ; jr r15 }
	{ cmpleu r5, r6, r7 ; prefetch_l1 r25 ; andi r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; prefetch_l1 r25 ; xor r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; prefetch_l1_fault r25 ; shl3addx r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; prefetch_l2 r25 ; rotl r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; prefetch_l2_fault r25 ; lnk r15 }
	{ cmpleu r5, r6, r7 ; prefetch_l3 r25 ; cmpne r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; prefetch_l3_fault r25 ; andi r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; prefetch_l3_fault r25 ; xor r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; rotli r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
	{ cmpleu r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
	{ cmpleu r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
	{ cmpleu r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
	{ cmpleu r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
	{ cmpleu r5, r6, r7 ; st r25, r26 ; andi r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; st r25, r26 ; xor r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; st1 r25, r26 ; shl3addx r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; st2 r25, r26 ; or r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; st4 r25, r26 ; jr r15 }
	{ cmpleu r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
	{ cmpleu r5, r6, r7 ; v1cmpeq r15, r16, r17 }
	{ cmpleu r5, r6, r7 ; v2maxsi r15, r16, 5 }
	{ cmpleu r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmplts r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3 r25 }
	{ cmplts r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmplts r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmplts r15, r16, r17 ; cmoveqz r5, r6, r7 ; prefetch_l3 r25 }
	{ cmplts r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
	{ cmplts r15, r16, r17 ; cmples r5, r6, r7 ; st2 r25, r26 }
	{ cmplts r15, r16, r17 ; cmplts r5, r6, r7 }
	{ cmplts r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
	{ cmplts r15, r16, r17 ; ctz r5, r6 ; prefetch_l3 r25 }
	{ cmplts r15, r16, r17 ; fsingle_mul1 r5, r6, r7 }
	{ cmplts r15, r16, r17 ; info 19 ; st4 r25, r26 }
	{ cmplts r15, r16, r17 ; ld r25, r26 ; or r5, r6, r7 }
	{ cmplts r15, r16, r17 ; ld1s r25, r26 ; cmpltsi r5, r6, 5 }
	{ cmplts r15, r16, r17 ; ld1s r25, r26 ; shrui r5, r6, 5 }
	{ cmplts r15, r16, r17 ; ld1u r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ cmplts r15, r16, r17 ; ld2s r25, r26 ; cmovnez r5, r6, r7 }
	{ cmplts r15, r16, r17 ; ld2s r25, r26 ; shl3add r5, r6, r7 }
	{ cmplts r15, r16, r17 ; ld2u r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmplts r15, r16, r17 ; ld4s r25, r26 ; addx r5, r6, r7 }
	{ cmplts r15, r16, r17 ; ld4s r25, r26 ; rotli r5, r6, 5 }
	{ cmplts r15, r16, r17 ; ld4u r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmplts r15, r16, r17 ; ld4u r25, r26 ; tblidxb2 r5, r6 }
	{ cmplts r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
	{ cmplts r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; st4 r25, r26 }
	{ cmplts r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; st r25, r26 }
	{ cmplts r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; st1 r25, r26 }
	{ cmplts r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; prefetch_l3 r25 }
	{ cmplts r15, r16, r17 ; mulax r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmplts r15, r16, r17 ; mz r5, r6, r7 ; st1 r25, r26 }
	{ cmplts r15, r16, r17 ; nor r5, r6, r7 ; st4 r25, r26 }
	{ cmplts r15, r16, r17 ; pcnt r5, r6 }
	{ cmplts r15, r16, r17 ; prefetch r25 ; revbits r5, r6 }
	{ cmplts r15, r16, r17 ; prefetch_l1 r25 ; cmpne r5, r6, r7 }
	{ cmplts r15, r16, r17 ; prefetch_l1 r25 ; subx r5, r6, r7 }
	{ cmplts r15, r16, r17 ; prefetch_l1_fault r25 ; mulx r5, r6, r7 }
	{ cmplts r15, r16, r17 ; prefetch_l2 r25 ; cmpeqi r5, r6, 5 }
	{ cmplts r15, r16, r17 ; prefetch_l2 r25 ; shli r5, r6, 5 }
	{ cmplts r15, r16, r17 ; prefetch_l2_fault r25 ; mul_lu_lu r5, r6, r7 }
	{ cmplts r15, r16, r17 ; prefetch_l3 r25 ; and r5, r6, r7 }
	{ cmplts r15, r16, r17 ; prefetch_l3 r25 ; shl1add r5, r6, r7 }
	{ cmplts r15, r16, r17 ; prefetch_l3_fault r25 ; mnz r5, r6, r7 }
	{ cmplts r15, r16, r17 ; prefetch_l3_fault r25 ; xor r5, r6, r7 }
	{ cmplts r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
	{ cmplts r15, r16, r17 ; shl r5, r6, r7 ; ld1u r25, r26 }
	{ cmplts r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
	{ cmplts r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
	{ cmplts r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
	{ cmplts r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
	{ cmplts r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
	{ cmplts r15, r16, r17 ; st r25, r26 ; cmovnez r5, r6, r7 }
	{ cmplts r15, r16, r17 ; st r25, r26 ; shl3add r5, r6, r7 }
	{ cmplts r15, r16, r17 ; st1 r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmplts r15, r16, r17 ; st2 r25, r26 ; addx r5, r6, r7 }
	{ cmplts r15, r16, r17 ; st2 r25, r26 ; rotli r5, r6, 5 }
	{ cmplts r15, r16, r17 ; st4 r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmplts r15, r16, r17 ; st4 r25, r26 ; tblidxb2 r5, r6 }
	{ cmplts r15, r16, r17 ; subx r5, r6, r7 ; st4 r25, r26 }
	{ cmplts r15, r16, r17 ; tblidxb1 r5, r6 }
	{ cmplts r15, r16, r17 ; v1addi r5, r6, 5 }
	{ cmplts r15, r16, r17 ; v1shru r5, r6, r7 }
	{ cmplts r15, r16, r17 ; v2shlsc r5, r6, r7 }
	{ cmplts r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
	{ cmplts r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
	{ cmplts r5, r6, r7 ; and r15, r16, r17 ; ld2s r25, r26 }
	{ cmplts r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
	{ cmplts r5, r6, r7 ; cmples r15, r16, r17 ; ld4s r25, r26 }
	{ cmplts r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
	{ cmplts r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmplts r5, r6, r7 ; fetchand r15, r16, r17 }
	{ cmplts r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
	{ cmplts r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
	{ cmplts r5, r6, r7 ; jr r15 ; st r25, r26 }
	{ cmplts r5, r6, r7 ; ld r25, r26 ; ill }
	{ cmplts r5, r6, r7 ; ld1s r25, r26 ; cmpeqi r15, r16, 5 }
	{ cmplts r5, r6, r7 ; ld1s_add r15, r16, 5 }
	{ cmplts r5, r6, r7 ; ld1u r25, r26 ; shli r15, r16, 5 }
	{ cmplts r5, r6, r7 ; ld2s r25, r26 ; rotl r15, r16, r17 }
	{ cmplts r5, r6, r7 ; ld2u r25, r26 ; jrp r15 }
	{ cmplts r5, r6, r7 ; ld4s r25, r26 ; cmpltsi r15, r16, 5 }
	{ cmplts r5, r6, r7 ; ld4u r25, r26 ; addx r15, r16, r17 }
	{ cmplts r5, r6, r7 ; ld4u r25, r26 ; shrui r15, r16, 5 }
	{ cmplts r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
	{ cmplts r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
	{ cmplts r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
	{ cmplts r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
	{ cmplts r5, r6, r7 ; prefetch r25 ; jr r15 }
	{ cmplts r5, r6, r7 ; prefetch_l1 r25 ; andi r15, r16, 5 }
	{ cmplts r5, r6, r7 ; prefetch_l1 r25 ; xor r15, r16, r17 }
	{ cmplts r5, r6, r7 ; prefetch_l1_fault r25 ; shl3addx r15, r16, r17 }
	{ cmplts r5, r6, r7 ; prefetch_l2 r25 ; rotl r15, r16, r17 }
	{ cmplts r5, r6, r7 ; prefetch_l2_fault r25 ; lnk r15 }
	{ cmplts r5, r6, r7 ; prefetch_l3 r25 ; cmpne r15, r16, r17 }
	{ cmplts r5, r6, r7 ; prefetch_l3_fault r25 ; andi r15, r16, 5 }
	{ cmplts r5, r6, r7 ; prefetch_l3_fault r25 ; xor r15, r16, r17 }
	{ cmplts r5, r6, r7 ; rotli r15, r16, 5 }
	{ cmplts r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
	{ cmplts r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
	{ cmplts r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
	{ cmplts r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
	{ cmplts r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
	{ cmplts r5, r6, r7 ; st r25, r26 ; andi r15, r16, 5 }
	{ cmplts r5, r6, r7 ; st r25, r26 ; xor r15, r16, r17 }
	{ cmplts r5, r6, r7 ; st1 r25, r26 ; shl3addx r15, r16, r17 }
	{ cmplts r5, r6, r7 ; st2 r25, r26 ; or r15, r16, r17 }
	{ cmplts r5, r6, r7 ; st4 r25, r26 ; jr r15 }
	{ cmplts r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
	{ cmplts r5, r6, r7 ; v1cmpeq r15, r16, r17 }
	{ cmplts r5, r6, r7 ; v2maxsi r15, r16, 5 }
	{ cmplts r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmpltsi r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l3 r25 }
	{ cmpltsi r15, r16, 5 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmpltsi r15, r16, 5 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmpltsi r15, r16, 5 ; cmoveqz r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpltsi r15, r16, 5 ; cmpeq r5, r6, r7 ; st r25, r26 }
	{ cmpltsi r15, r16, 5 ; cmples r5, r6, r7 ; st2 r25, r26 }
	{ cmpltsi r15, r16, 5 ; cmplts r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; cmpne r5, r6, r7 ; ld r25, r26 }
	{ cmpltsi r15, r16, 5 ; ctz r5, r6 ; prefetch_l3 r25 }
	{ cmpltsi r15, r16, 5 ; fsingle_mul1 r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; info 19 ; st4 r25, r26 }
	{ cmpltsi r15, r16, 5 ; ld r25, r26 ; or r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; ld1s r25, r26 ; cmpltsi r5, r6, 5 }
	{ cmpltsi r15, r16, 5 ; ld1s r25, r26 ; shrui r5, r6, 5 }
	{ cmpltsi r15, r16, 5 ; ld1u r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; ld2s r25, r26 ; cmovnez r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; ld2s r25, r26 ; shl3add r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; ld2u r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; ld4s r25, r26 ; addx r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; ld4s r25, r26 ; rotli r5, r6, 5 }
	{ cmpltsi r15, r16, 5 ; ld4u r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmpltsi r15, r16, 5 ; ld4u r25, r26 ; tblidxb2 r5, r6 }
	{ cmpltsi r15, r16, 5 ; move r5, r6 ; st2 r25, r26 }
	{ cmpltsi r15, r16, 5 ; mul_hs_hs r5, r6, r7 ; st4 r25, r26 }
	{ cmpltsi r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; st r25, r26 }
	{ cmpltsi r15, r16, 5 ; mula_hs_hs r5, r6, r7 ; st1 r25, r26 }
	{ cmpltsi r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpltsi r15, r16, 5 ; mulax r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmpltsi r15, r16, 5 ; mz r5, r6, r7 ; st1 r25, r26 }
	{ cmpltsi r15, r16, 5 ; nor r5, r6, r7 ; st4 r25, r26 }
	{ cmpltsi r15, r16, 5 ; pcnt r5, r6 }
	{ cmpltsi r15, r16, 5 ; prefetch r25 ; revbits r5, r6 }
	{ cmpltsi r15, r16, 5 ; prefetch_l1 r25 ; cmpne r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; prefetch_l1 r25 ; subx r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; prefetch_l1_fault r25 ; mulx r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; prefetch_l2 r25 ; cmpeqi r5, r6, 5 }
	{ cmpltsi r15, r16, 5 ; prefetch_l2 r25 ; shli r5, r6, 5 }
	{ cmpltsi r15, r16, 5 ; prefetch_l2_fault r25 ; mul_lu_lu r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; prefetch_l3 r25 ; and r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; prefetch_l3 r25 ; shl1add r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; prefetch_l3_fault r25 ; mnz r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; prefetch_l3_fault r25 ; xor r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; rotl r5, r6, r7 ; ld r25, r26 }
	{ cmpltsi r15, r16, 5 ; shl r5, r6, r7 ; ld1u r25, r26 }
	{ cmpltsi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
	{ cmpltsi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
	{ cmpltsi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch r25 }
	{ cmpltsi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch r25 }
	{ cmpltsi r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
	{ cmpltsi r15, r16, 5 ; st r25, r26 ; cmovnez r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; st r25, r26 ; shl3add r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; st1 r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; st2 r25, r26 ; addx r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; st2 r25, r26 ; rotli r5, r6, 5 }
	{ cmpltsi r15, r16, 5 ; st4 r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmpltsi r15, r16, 5 ; st4 r25, r26 ; tblidxb2 r5, r6 }
	{ cmpltsi r15, r16, 5 ; subx r5, r6, r7 ; st4 r25, r26 }
	{ cmpltsi r15, r16, 5 ; tblidxb1 r5, r6 }
	{ cmpltsi r15, r16, 5 ; v1addi r5, r6, 5 }
	{ cmpltsi r15, r16, 5 ; v1shru r5, r6, r7 }
	{ cmpltsi r15, r16, 5 ; v2shlsc r5, r6, r7 }
	{ cmpltsi r5, r6, 5 ; add r15, r16, r17 ; ld1u r25, r26 }
	{ cmpltsi r5, r6, 5 ; addx r15, r16, r17 ; ld2s r25, r26 }
	{ cmpltsi r5, r6, 5 ; and r15, r16, r17 ; ld2s r25, r26 }
	{ cmpltsi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
	{ cmpltsi r5, r6, 5 ; cmples r15, r16, r17 ; ld4s r25, r26 }
	{ cmpltsi r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch r25 }
	{ cmpltsi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmpltsi r5, r6, 5 ; fetchand r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; ill ; prefetch_l3_fault r25 }
	{ cmpltsi r5, r6, 5 ; jalr r15 ; prefetch_l3 r25 }
	{ cmpltsi r5, r6, 5 ; jr r15 ; st r25, r26 }
	{ cmpltsi r5, r6, 5 ; ld r25, r26 ; ill }
	{ cmpltsi r5, r6, 5 ; ld1s r25, r26 ; cmpeqi r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; ld1s_add r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; ld1u r25, r26 ; shli r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; ld2s r25, r26 ; rotl r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; ld2u r25, r26 ; jrp r15 }
	{ cmpltsi r5, r6, 5 ; ld4s r25, r26 ; cmpltsi r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; ld4u r25, r26 ; addx r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; ld4u r25, r26 ; shrui r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; lnk r15 ; st4 r25, r26 }
	{ cmpltsi r5, r6, 5 ; move r15, r16 ; st4 r25, r26 }
	{ cmpltsi r5, r6, 5 ; mz r15, r16, r17 ; st4 r25, r26 }
	{ cmpltsi r5, r6, 5 ; or r15, r16, r17 ; ld r25, r26 }
	{ cmpltsi r5, r6, 5 ; prefetch r25 ; jr r15 }
	{ cmpltsi r5, r6, 5 ; prefetch_l1 r25 ; andi r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; prefetch_l1 r25 ; xor r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; prefetch_l1_fault r25 ; shl3addx r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; prefetch_l2 r25 ; rotl r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; prefetch_l2_fault r25 ; lnk r15 }
	{ cmpltsi r5, r6, 5 ; prefetch_l3 r25 ; cmpne r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 ; andi r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 ; xor r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; rotli r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld r25, r26 }
	{ cmpltsi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
	{ cmpltsi r5, r6, 5 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
	{ cmpltsi r5, r6, 5 ; shrs r15, r16, r17 ; ld2u r25, r26 }
	{ cmpltsi r5, r6, 5 ; shru r15, r16, r17 ; ld4u r25, r26 }
	{ cmpltsi r5, r6, 5 ; st r25, r26 ; andi r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; st r25, r26 ; xor r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; st1 r25, r26 ; shl3addx r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; st2 r25, r26 ; or r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; st4 r25, r26 ; jr r15 }
	{ cmpltsi r5, r6, 5 ; sub r15, r16, r17 ; ld1u r25, r26 }
	{ cmpltsi r5, r6, 5 ; v1cmpeq r15, r16, r17 }
	{ cmpltsi r5, r6, 5 ; v2maxsi r15, r16, 5 }
	{ cmpltsi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmpltu r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3 r25 }
	{ cmpltu r15, r16, r17 ; addxi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmpltu r15, r16, r17 ; andi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ cmpltu r15, r16, r17 ; cmoveqz r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpltu r15, r16, r17 ; cmpeq r5, r6, r7 ; st r25, r26 }
	{ cmpltu r15, r16, r17 ; cmples r5, r6, r7 ; st2 r25, r26 }
	{ cmpltu r15, r16, r17 ; cmplts r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; cmpne r5, r6, r7 ; ld r25, r26 }
	{ cmpltu r15, r16, r17 ; ctz r5, r6 ; prefetch_l3 r25 }
	{ cmpltu r15, r16, r17 ; fsingle_mul1 r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; info 19 ; st4 r25, r26 }
	{ cmpltu r15, r16, r17 ; ld r25, r26 ; or r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; ld1s r25, r26 ; cmpltsi r5, r6, 5 }
	{ cmpltu r15, r16, r17 ; ld1s r25, r26 ; shrui r5, r6, 5 }
	{ cmpltu r15, r16, r17 ; ld1u r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; ld2s r25, r26 ; cmovnez r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; ld2s r25, r26 ; shl3add r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; ld2u r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; ld4s r25, r26 ; addx r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; ld4s r25, r26 ; rotli r5, r6, 5 }
	{ cmpltu r15, r16, r17 ; ld4u r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmpltu r15, r16, r17 ; ld4u r25, r26 ; tblidxb2 r5, r6 }
	{ cmpltu r15, r16, r17 ; move r5, r6 ; st2 r25, r26 }
	{ cmpltu r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; st4 r25, r26 }
	{ cmpltu r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; st r25, r26 }
	{ cmpltu r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; st1 r25, r26 }
	{ cmpltu r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; prefetch_l3 r25 }
	{ cmpltu r15, r16, r17 ; mulax r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmpltu r15, r16, r17 ; mz r5, r6, r7 ; st1 r25, r26 }
	{ cmpltu r15, r16, r17 ; nor r5, r6, r7 ; st4 r25, r26 }
	{ cmpltu r15, r16, r17 ; pcnt r5, r6 }
	{ cmpltu r15, r16, r17 ; prefetch r25 ; revbits r5, r6 }
	{ cmpltu r15, r16, r17 ; prefetch_l1 r25 ; cmpne r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; prefetch_l1 r25 ; subx r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; prefetch_l1_fault r25 ; mulx r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; prefetch_l2 r25 ; cmpeqi r5, r6, 5 }
	{ cmpltu r15, r16, r17 ; prefetch_l2 r25 ; shli r5, r6, 5 }
	{ cmpltu r15, r16, r17 ; prefetch_l2_fault r25 ; mul_lu_lu r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; prefetch_l3 r25 ; and r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; prefetch_l3 r25 ; shl1add r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; prefetch_l3_fault r25 ; mnz r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; prefetch_l3_fault r25 ; xor r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; rotl r5, r6, r7 ; ld r25, r26 }
	{ cmpltu r15, r16, r17 ; shl r5, r6, r7 ; ld1u r25, r26 }
	{ cmpltu r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2s r25, r26 }
	{ cmpltu r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4s r25, r26 }
	{ cmpltu r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch r25 }
	{ cmpltu r15, r16, r17 ; shrs r5, r6, r7 ; prefetch r25 }
	{ cmpltu r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l1_fault r25 }
	{ cmpltu r15, r16, r17 ; st r25, r26 ; cmovnez r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; st r25, r26 ; shl3add r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; st1 r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; st2 r25, r26 ; addx r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; st2 r25, r26 ; rotli r5, r6, 5 }
	{ cmpltu r15, r16, r17 ; st4 r25, r26 ; fsingle_pack1 r5, r6 }
	{ cmpltu r15, r16, r17 ; st4 r25, r26 ; tblidxb2 r5, r6 }
	{ cmpltu r15, r16, r17 ; subx r5, r6, r7 ; st4 r25, r26 }
	{ cmpltu r15, r16, r17 ; tblidxb1 r5, r6 }
	{ cmpltu r15, r16, r17 ; v1addi r5, r6, 5 }
	{ cmpltu r15, r16, r17 ; v1shru r5, r6, r7 }
	{ cmpltu r15, r16, r17 ; v2shlsc r5, r6, r7 }
	{ cmpltu r5, r6, r7 ; add r15, r16, r17 ; ld1u r25, r26 }
	{ cmpltu r5, r6, r7 ; addx r15, r16, r17 ; ld2s r25, r26 }
	{ cmpltu r5, r6, r7 ; and r15, r16, r17 ; ld2s r25, r26 }
	{ cmpltu r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
	{ cmpltu r5, r6, r7 ; cmples r15, r16, r17 ; ld4s r25, r26 }
	{ cmpltu r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch r25 }
	{ cmpltu r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmpltu r5, r6, r7 ; fetchand r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; ill ; prefetch_l3_fault r25 }
	{ cmpltu r5, r6, r7 ; jalr r15 ; prefetch_l3 r25 }
	{ cmpltu r5, r6, r7 ; jr r15 ; st r25, r26 }
	{ cmpltu r5, r6, r7 ; ld r25, r26 ; ill }
	{ cmpltu r5, r6, r7 ; ld1s r25, r26 ; cmpeqi r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; ld1s_add r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; ld1u r25, r26 ; shli r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; ld2s r25, r26 ; rotl r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; ld2u r25, r26 ; jrp r15 }
	{ cmpltu r5, r6, r7 ; ld4s r25, r26 ; cmpltsi r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; ld4u r25, r26 ; addx r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; ld4u r25, r26 ; shrui r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; lnk r15 ; st4 r25, r26 }
	{ cmpltu r5, r6, r7 ; move r15, r16 ; st4 r25, r26 }
	{ cmpltu r5, r6, r7 ; mz r15, r16, r17 ; st4 r25, r26 }
	{ cmpltu r5, r6, r7 ; or r15, r16, r17 ; ld r25, r26 }
	{ cmpltu r5, r6, r7 ; prefetch r25 ; jr r15 }
	{ cmpltu r5, r6, r7 ; prefetch_l1 r25 ; andi r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; prefetch_l1 r25 ; xor r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; prefetch_l1_fault r25 ; shl3addx r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; prefetch_l2 r25 ; rotl r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; prefetch_l2_fault r25 ; lnk r15 }
	{ cmpltu r5, r6, r7 ; prefetch_l3 r25 ; cmpne r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; prefetch_l3_fault r25 ; andi r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; prefetch_l3_fault r25 ; xor r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; rotli r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; shl1addx r15, r16, r17 ; ld r25, r26 }
	{ cmpltu r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
	{ cmpltu r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
	{ cmpltu r5, r6, r7 ; shrs r15, r16, r17 ; ld2u r25, r26 }
	{ cmpltu r5, r6, r7 ; shru r15, r16, r17 ; ld4u r25, r26 }
	{ cmpltu r5, r6, r7 ; st r25, r26 ; andi r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; st r25, r26 ; xor r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; st1 r25, r26 ; shl3addx r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; st2 r25, r26 ; or r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; st4 r25, r26 ; jr r15 }
	{ cmpltu r5, r6, r7 ; sub r15, r16, r17 ; ld1u r25, r26 }
	{ cmpltu r5, r6, r7 ; v1cmpeq r15, r16, r17 }
	{ cmpltu r5, r6, r7 ; v2maxsi r15, r16, 5 }
	{ cmpltu r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmpltui r15, r16, 5 ; crc32_32 r5, r6, r7 }
	{ cmpltui r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ cmpltui r15, r16, 5 ; sub r5, r6, r7 }
	{ cmpltui r15, r16, 5 ; v1mulus r5, r6, r7 }
	{ cmpltui r15, r16, 5 ; v2packl r5, r6, r7 }
	{ cmpltui r5, r6, 5 ; cmpexch4 r15, r16, r17 }
	{ cmpltui r5, r6, 5 ; ld1u_add r15, r16, 5 }
	{ cmpltui r5, r6, 5 ; prefetch_add_l1 r15, 5 }
	{ cmpltui r5, r6, 5 ; stnt r15, r16 }
	{ cmpltui r5, r6, 5 ; v2addi r15, r16, 5 }
	{ cmpltui r5, r6, 5 ; v4sub r15, r16, r17 }
	{ cmpne r15, r16, r17 ; addi r5, r6, 5 ; st2 r25, r26 }
	{ cmpne r15, r16, r17 ; addxi r5, r6, 5 ; st4 r25, r26 }
	{ cmpne r15, r16, r17 ; andi r5, r6, 5 ; st4 r25, r26 }
	{ cmpne r15, r16, r17 ; cmoveqz r5, r6, r7 ; st2 r25, r26 }
	{ cmpne r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ cmpne r15, r16, r17 ; cmpleu r5, r6, r7 ; ld1s r25, r26 }
	{ cmpne r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2s r25, r26 }
	{ cmpne r15, r16, r17 ; cmpne r5, r6, r7 ; ld2u r25, r26 }
	{ cmpne r15, r16, r17 ; ctz r5, r6 ; st2 r25, r26 }
	{ cmpne r15, r16, r17 ; fsingle_pack1 r5, r6 ; ld1u r25, r26 }
	{ cmpne r15, r16, r17 ; ld r25, r26 ; addi r5, r6, 5 }
	{ cmpne r15, r16, r17 ; ld r25, r26 ; rotl r5, r6, r7 }
	{ cmpne r15, r16, r17 ; ld1s r25, r26 ; fnop }
	{ cmpne r15, r16, r17 ; ld1s r25, r26 ; tblidxb1 r5, r6 }
	{ cmpne r15, r16, r17 ; ld1u r25, r26 ; nop }
	{ cmpne r15, r16, r17 ; ld2s r25, r26 ; cmpleu r5, r6, r7 }
	{ cmpne r15, r16, r17 ; ld2s r25, r26 ; shrsi r5, r6, 5 }
	{ cmpne r15, r16, r17 ; ld2u r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ cmpne r15, r16, r17 ; ld4s r25, r26 ; clz r5, r6 }
	{ cmpne r15, r16, r17 ; ld4s r25, r26 ; shl2add r5, r6, r7 }
	{ cmpne r15, r16, r17 ; ld4u r25, r26 ; movei r5, 5 }
	{ cmpne r15, r16, r17 ; mm r5, r6, 5, 7 }
	{ cmpne r15, r16, r17 ; movei r5, 5 ; ld1s r25, r26 }
	{ cmpne r15, r16, r17 ; mul_hs_lu r5, r6, r7 }
	{ cmpne r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ cmpne r15, r16, r17 ; mula_hs_hu r5, r6, r7 }
	{ cmpne r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; st2 r25, r26 }
	{ cmpne r15, r16, r17 ; mulax r5, r6, r7 ; st4 r25, r26 }
	{ cmpne r15, r16, r17 ; nop ; ld r25, r26 }
	{ cmpne r15, r16, r17 ; or r5, r6, r7 ; ld1u r25, r26 }
	{ cmpne r15, r16, r17 ; prefetch r25 ; addxi r5, r6, 5 }
	{ cmpne r15, r16, r17 ; prefetch r25 ; shl r5, r6, r7 }
	{ cmpne r15, r16, r17 ; prefetch_l1 r25 ; info 19 }
	{ cmpne r15, r16, r17 ; prefetch_l1 r25 ; tblidxb3 r5, r6 }
	{ cmpne r15, r16, r17 ; prefetch_l1_fault r25 ; or r5, r6, r7 }
	{ cmpne r15, r16, r17 ; prefetch_l2 r25 ; cmpltsi r5, r6, 5 }
	{ cmpne r15, r16, r17 ; prefetch_l2 r25 ; shrui r5, r6, 5 }
	{ cmpne r15, r16, r17 ; prefetch_l2_fault r25 ; mula_lu_lu r5, r6, r7 }
	{ cmpne r15, r16, r17 ; prefetch_l3 r25 ; cmovnez r5, r6, r7 }
	{ cmpne r15, r16, r17 ; prefetch_l3 r25 ; shl3add r5, r6, r7 }
	{ cmpne r15, r16, r17 ; prefetch_l3_fault r25 ; mul_hu_hu r5, r6, r7 }
	{ cmpne r15, r16, r17 ; revbits r5, r6 ; ld1u r25, r26 }
	{ cmpne r15, r16, r17 ; rotl r5, r6, r7 ; ld2u r25, r26 }
	{ cmpne r15, r16, r17 ; shl r5, r6, r7 ; ld4u r25, r26 }
	{ cmpne r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch r25 }
	{ cmpne r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
	{ cmpne r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
	{ cmpne r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2_fault r25 }
	{ cmpne r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l3_fault r25 }
	{ cmpne r15, r16, r17 ; st r25, r26 ; cmpleu r5, r6, r7 }
	{ cmpne r15, r16, r17 ; st r25, r26 ; shrsi r5, r6, 5 }
	{ cmpne r15, r16, r17 ; st1 r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ cmpne r15, r16, r17 ; st2 r25, r26 ; clz r5, r6 }
	{ cmpne r15, r16, r17 ; st2 r25, r26 ; shl2add r5, r6, r7 }
	{ cmpne r15, r16, r17 ; st4 r25, r26 ; movei r5, 5 }
	{ cmpne r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
	{ cmpne r15, r16, r17 ; tblidxb0 r5, r6 ; ld1s r25, r26 }
	{ cmpne r15, r16, r17 ; tblidxb2 r5, r6 ; ld2s r25, r26 }
	{ cmpne r15, r16, r17 ; v1cmpeq r5, r6, r7 }
	{ cmpne r15, r16, r17 ; v2add r5, r6, r7 }
	{ cmpne r15, r16, r17 ; v2shrui r5, r6, 5 }
	{ cmpne r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
	{ cmpne r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
	{ cmpne r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
	{ cmpne r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmpne r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
	{ cmpne r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
	{ cmpne r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
	{ cmpne r5, r6, r7 ; finv r15 }
	{ cmpne r5, r6, r7 ; ill ; st4 r25, r26 }
	{ cmpne r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
	{ cmpne r5, r6, r7 ; jr r15 }
	{ cmpne r5, r6, r7 ; ld r25, r26 ; jr r15 }
	{ cmpne r5, r6, r7 ; ld1s r25, r26 ; cmpltsi r15, r16, 5 }
	{ cmpne r5, r6, r7 ; ld1u r25, r26 ; addx r15, r16, r17 }
	{ cmpne r5, r6, r7 ; ld1u r25, r26 ; shrui r15, r16, 5 }
	{ cmpne r5, r6, r7 ; ld2s r25, r26 ; shl1addx r15, r16, r17 }
	{ cmpne r5, r6, r7 ; ld2u r25, r26 ; movei r15, 5 }
	{ cmpne r5, r6, r7 ; ld4s r25, r26 ; ill }
	{ cmpne r5, r6, r7 ; ld4u r25, r26 ; cmpeq r15, r16, r17 }
	{ cmpne r5, r6, r7 ; ld4u r25, r26 }
	{ cmpne r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
	{ cmpne r5, r6, r7 ; movei r15, 5 ; ld1u r25, r26 }
	{ cmpne r5, r6, r7 ; nop ; ld1u r25, r26 }
	{ cmpne r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
	{ cmpne r5, r6, r7 ; prefetch r25 ; move r15, r16 }
	{ cmpne r5, r6, r7 ; prefetch_l1 r25 ; cmpleu r15, r16, r17 }
	{ cmpne r5, r6, r7 ; prefetch_l1_fault r25 ; addi r15, r16, 5 }
	{ cmpne r5, r6, r7 ; prefetch_l1_fault r25 ; shru r15, r16, r17 }
	{ cmpne r5, r6, r7 ; prefetch_l2 r25 ; shl1addx r15, r16, r17 }
	{ cmpne r5, r6, r7 ; prefetch_l2_fault r25 ; mz r15, r16, r17 }
	{ cmpne r5, r6, r7 ; prefetch_l3 r25 ; jalr r15 }
	{ cmpne r5, r6, r7 ; prefetch_l3_fault r25 ; cmpleu r15, r16, r17 }
	{ cmpne r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
	{ cmpne r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
	{ cmpne r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
	{ cmpne r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
	{ cmpne r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l1 r25 }
	{ cmpne r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l1 r25 }
	{ cmpne r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l2 r25 }
	{ cmpne r5, r6, r7 ; st r25, r26 ; cmpleu r15, r16, r17 }
	{ cmpne r5, r6, r7 ; st1 r25, r26 ; addi r15, r16, 5 }
	{ cmpne r5, r6, r7 ; st1 r25, r26 ; shru r15, r16, r17 }
	{ cmpne r5, r6, r7 ; st2 r25, r26 ; shl1add r15, r16, r17 }
	{ cmpne r5, r6, r7 ; st4 r25, r26 ; move r15, r16 }
	{ cmpne r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
	{ cmpne r5, r6, r7 ; v1cmplts r15, r16, r17 }
	{ cmpne r5, r6, r7 ; v2mz r15, r16, r17 }
	{ cmpne r5, r6, r7 ; xor r15, r16, r17 ; st1 r25, r26 }
	{ cmul r5, r6, r7 ; flush r15 }
	{ cmul r5, r6, r7 ; ldnt4u r15, r16 }
	{ cmul r5, r6, r7 ; shli r15, r16, 5 }
	{ cmul r5, r6, r7 ; v1int_h r15, r16, r17 }
	{ cmul r5, r6, r7 ; v2shli r15, r16, 5 }
	{ cmula r5, r6, r7 ; cmpltui r15, r16, 5 }
	{ cmula r5, r6, r7 ; ld4s_add r15, r16, 5 }
	{ cmula r5, r6, r7 ; prefetch_l1 r15 }
	{ cmula r5, r6, r7 ; stnt4_add r15, r16, 5 }
	{ cmula r5, r6, r7 ; v2cmplts r15, r16, r17 }
	{ cmulaf r5, r6, r7 ; addi r15, r16, 5 }
	{ cmulaf r5, r6, r7 ; infol 0x1234 }
	{ cmulaf r5, r6, r7 ; mnz r15, r16, r17 }
	{ cmulaf r5, r6, r7 ; shrui r15, r16, 5 }
	{ cmulaf r5, r6, r7 ; v1mnz r15, r16, r17 }
	{ cmulaf r5, r6, r7 ; v2sub r15, r16, r17 }
	{ cmulf r5, r6, r7 ; exch r15, r16, r17 }
	{ cmulf r5, r6, r7 ; ldnt r15, r16 }
	{ cmulf r5, r6, r7 ; raise }
	{ cmulf r5, r6, r7 ; v1addi r15, r16, 5 }
	{ cmulf r5, r6, r7 ; v2int_l r15, r16, r17 }
	{ cmulfr r5, r6, r7 ; and r15, r16, r17 }
	{ cmulfr r5, r6, r7 ; jrp r15 }
	{ cmulfr r5, r6, r7 ; nop }
	{ cmulfr r5, r6, r7 ; st2 r15, r16 }
	{ cmulfr r5, r6, r7 ; v1shru r15, r16, r17 }
	{ cmulfr r5, r6, r7 ; v4packsc r15, r16, r17 }
	{ cmulh r5, r6, r7 ; fetchand r15, r16, r17 }
	{ cmulh r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
	{ cmulh r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ cmulh r5, r6, r7 ; v1cmplts r15, r16, r17 }
	{ cmulh r5, r6, r7 ; v2mz r15, r16, r17 }
	{ cmulhr r5, r6, r7 ; cmples r15, r16, r17 }
	{ cmulhr r5, r6, r7 ; ld2s r15, r16 }
	{ cmulhr r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
	{ cmulhr r5, r6, r7 ; stnt1 r15, r16 }
	{ cmulhr r5, r6, r7 ; v2addsc r15, r16, r17 }
	{ cmulhr r5, r6, r7 ; v4subsc r15, r16, r17 }
	{ crc32_32 r5, r6, r7 ; flushwb }
	{ crc32_32 r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
	{ crc32_32 r5, r6, r7 ; shlx r15, r16, r17 }
	{ crc32_32 r5, r6, r7 ; v1int_l r15, r16, r17 }
	{ crc32_32 r5, r6, r7 ; v2shlsc r15, r16, r17 }
	{ crc32_8 r5, r6, r7 ; cmpne r15, r16, r17 }
	{ crc32_8 r5, r6, r7 ; ld4u r15, r16 }
	{ crc32_8 r5, r6, r7 ; prefetch_l1_fault r15 }
	{ crc32_8 r5, r6, r7 ; stnt_add r15, r16, 5 }
	{ crc32_8 r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ ctz r5, r6 ; add r15, r16, r17 ; ld1u r25, r26 }
	{ ctz r5, r6 ; addx r15, r16, r17 ; ld2s r25, r26 }
	{ ctz r5, r6 ; and r15, r16, r17 ; ld2s r25, r26 }
	{ ctz r5, r6 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
	{ ctz r5, r6 ; cmples r15, r16, r17 ; ld4s r25, r26 }
	{ ctz r5, r6 ; cmplts r15, r16, r17 ; prefetch r25 }
	{ ctz r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
	{ ctz r5, r6 ; fetchand r15, r16, r17 }
	{ ctz r5, r6 ; ill ; prefetch_l3_fault r25 }
	{ ctz r5, r6 ; jalr r15 ; prefetch_l3 r25 }
	{ ctz r5, r6 ; jr r15 ; st r25, r26 }
	{ ctz r5, r6 ; ld r25, r26 ; ill }
	{ ctz r5, r6 ; ld1s r25, r26 ; cmpeqi r15, r16, 5 }
	{ ctz r5, r6 ; ld1s_add r15, r16, 5 }
	{ ctz r5, r6 ; ld1u r25, r26 ; shli r15, r16, 5 }
	{ ctz r5, r6 ; ld2s r25, r26 ; rotl r15, r16, r17 }
	{ ctz r5, r6 ; ld2u r25, r26 ; jrp r15 }
	{ ctz r5, r6 ; ld4s r25, r26 ; cmpltsi r15, r16, 5 }
	{ ctz r5, r6 ; ld4u r25, r26 ; addx r15, r16, r17 }
	{ ctz r5, r6 ; ld4u r25, r26 ; shrui r15, r16, 5 }
	{ ctz r5, r6 ; lnk r15 ; st4 r25, r26 }
	{ ctz r5, r6 ; move r15, r16 ; st4 r25, r26 }
	{ ctz r5, r6 ; mz r15, r16, r17 ; st4 r25, r26 }
	{ ctz r5, r6 ; or r15, r16, r17 ; ld r25, r26 }
	{ ctz r5, r6 ; prefetch r25 ; jr r15 }
	{ ctz r5, r6 ; prefetch_l1 r25 ; andi r15, r16, 5 }
	{ ctz r5, r6 ; prefetch_l1 r25 ; xor r15, r16, r17 }
	{ ctz r5, r6 ; prefetch_l1_fault r25 ; shl3addx r15, r16, r17 }
	{ ctz r5, r6 ; prefetch_l2 r25 ; rotl r15, r16, r17 }
	{ ctz r5, r6 ; prefetch_l2_fault r25 ; lnk r15 }
	{ ctz r5, r6 ; prefetch_l3 r25 ; cmpne r15, r16, r17 }
	{ ctz r5, r6 ; prefetch_l3_fault r25 ; andi r15, r16, 5 }
	{ ctz r5, r6 ; prefetch_l3_fault r25 ; xor r15, r16, r17 }
	{ ctz r5, r6 ; rotli r15, r16, 5 }
	{ ctz r5, r6 ; shl1addx r15, r16, r17 ; ld r25, r26 }
	{ ctz r5, r6 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
	{ ctz r5, r6 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
	{ ctz r5, r6 ; shrs r15, r16, r17 ; ld2u r25, r26 }
	{ ctz r5, r6 ; shru r15, r16, r17 ; ld4u r25, r26 }
	{ ctz r5, r6 ; st r25, r26 ; andi r15, r16, 5 }
	{ ctz r5, r6 ; st r25, r26 ; xor r15, r16, r17 }
	{ ctz r5, r6 ; st1 r25, r26 ; shl3addx r15, r16, r17 }
	{ ctz r5, r6 ; st2 r25, r26 ; or r15, r16, r17 }
	{ ctz r5, r6 ; st4 r25, r26 ; jr r15 }
	{ ctz r5, r6 ; sub r15, r16, r17 ; ld1u r25, r26 }
	{ ctz r5, r6 ; v1cmpeq r15, r16, r17 }
	{ ctz r5, r6 ; v2maxsi r15, r16, 5 }
	{ ctz r5, r6 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
	{ dblalign r5, r6, r7 ; fetchand4 r15, r16, r17 }
	{ dblalign r5, r6, r7 ; ldnt2u r15, r16 }
	{ dblalign r5, r6, r7 ; shl2add r15, r16, r17 }
	{ dblalign r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
	{ dblalign r5, r6, r7 ; v2packh r15, r16, r17 }
	{ dblalign2 r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ dblalign2 r15, r16, r17 ; info 19 }
	{ dblalign2 r15, r16, r17 ; shl16insli r5, r6, 0x1234 }
	{ dblalign2 r15, r16, r17 ; v1ddotpus r5, r6, r7 }
	{ dblalign2 r15, r16, r17 ; v2cmpltu r5, r6, r7 }
	{ dblalign2 r15, r16, r17 ; v4shru r5, r6, r7 }
	{ dblalign2 r5, r6, r7 ; flush r15 }
	{ dblalign2 r5, r6, r7 ; ldnt4u r15, r16 }
	{ dblalign2 r5, r6, r7 ; shli r15, r16, 5 }
	{ dblalign2 r5, r6, r7 ; v1int_h r15, r16, r17 }
	{ dblalign2 r5, r6, r7 ; v2shli r15, r16, 5 }
	{ dblalign4 r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ dblalign4 r15, r16, r17 ; move r5, r6 }
	{ dblalign4 r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ dblalign4 r15, r16, r17 ; v1dotpu r5, r6, r7 }
	{ dblalign4 r15, r16, r17 ; v2dotpa r5, r6, r7 }
	{ dblalign4 r15, r16, r17 ; xori r5, r6, 5 }
	{ dblalign4 r5, r6, r7 ; ill }
	{ dblalign4 r5, r6, r7 ; mf }
	{ dblalign4 r5, r6, r7 ; shrsi r15, r16, 5 }
	{ dblalign4 r5, r6, r7 ; v1minu r15, r16, r17 }
	{ dblalign4 r5, r6, r7 ; v2shru r15, r16, r17 }
	{ dblalign6 r15, r16, r17 ; cmpltui r5, r6, 5 }
	{ dblalign6 r15, r16, r17 ; mul_hs_hu r5, r6, r7 }
	{ dblalign6 r15, r16, r17 ; shlx r5, r6, r7 }
	{ dblalign6 r15, r16, r17 ; v1int_h r5, r6, r7 }
	{ dblalign6 r15, r16, r17 ; v2maxsi r5, r6, 5 }
	{ dblalign6 r5, r6, r7 ; addx r15, r16, r17 }
	{ dblalign6 r5, r6, r7 ; iret }
	{ dblalign6 r5, r6, r7 ; movei r15, 5 }
	{ dblalign6 r5, r6, r7 ; shruxi r15, r16, 5 }
	{ dblalign6 r5, r6, r7 ; v1shl r15, r16, r17 }
	{ dblalign6 r5, r6, r7 ; v4add r15, r16, r17 }
	{ dtlbpr r15 ; cmula r5, r6, r7 }
	{ dtlbpr r15 ; mul_hu_hu r5, r6, r7 }
	{ dtlbpr r15 ; shrsi r5, r6, 5 }
	{ dtlbpr r15 ; v1maxui r5, r6, 5 }
	{ dtlbpr r15 ; v2mnz r5, r6, r7 }
	{ exch r15, r16, r17 ; addxsc r5, r6, r7 }
	{ exch r15, r16, r17 ; fnop }
	{ exch r15, r16, r17 ; or r5, r6, r7 }
	{ exch r15, r16, r17 ; v1cmpleu r5, r6, r7 }
	{ exch r15, r16, r17 ; v2adiffs r5, r6, r7 }
	{ exch r15, r16, r17 ; v4add r5, r6, r7 }
	{ exch4 r15, r16, r17 ; cmulf r5, r6, r7 }
	{ exch4 r15, r16, r17 ; mul_hu_lu r5, r6, r7 }
	{ exch4 r15, r16, r17 ; shrui r5, r6, 5 }
	{ exch4 r15, r16, r17 ; v1minui r5, r6, 5 }
	{ exch4 r15, r16, r17 ; v2muls r5, r6, r7 }
	{ fdouble_add_flags r5, r6, r7 ; andi r15, r16, 5 }
	{ fdouble_add_flags r5, r6, r7 ; ld r15, r16 }
	{ fdouble_add_flags r5, r6, r7 ; nor r15, r16, r17 }
	{ fdouble_add_flags r5, r6, r7 ; st2_add r15, r16, 5 }
	{ fdouble_add_flags r5, r6, r7 ; v1shrui r15, r16, 5 }
	{ fdouble_add_flags r5, r6, r7 ; v4shl r15, r16, r17 }
	{ fdouble_addsub r5, r6, r7 ; fetchand4 r15, r16, r17 }
	{ fdouble_addsub r5, r6, r7 ; ldnt2u r15, r16 }
	{ fdouble_addsub r5, r6, r7 ; shl2add r15, r16, r17 }
	{ fdouble_addsub r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
	{ fdouble_addsub r5, r6, r7 ; v2packh r15, r16, r17 }
	{ fdouble_mul_flags r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ fdouble_mul_flags r5, r6, r7 ; ld2s_add r15, r16, 5 }
	{ fdouble_mul_flags r5, r6, r7 ; prefetch_add_l2 r15, 5 }
	{ fdouble_mul_flags r5, r6, r7 ; stnt1_add r15, r16, 5 }
	{ fdouble_mul_flags r5, r6, r7 ; v2cmpeq r15, r16, r17 }
	{ fdouble_mul_flags r5, r6, r7 ; wh64 r15 }
	{ fdouble_pack1 r5, r6, r7 ; fnop }
	{ fdouble_pack1 r5, r6, r7 ; ldnt_add r15, r16, 5 }
	{ fdouble_pack1 r5, r6, r7 ; shlxi r15, r16, 5 }
	{ fdouble_pack1 r5, r6, r7 ; v1maxu r15, r16, r17 }
	{ fdouble_pack1 r5, r6, r7 ; v2shrs r15, r16, r17 }
	{ fdouble_pack2 r5, r6, r7 ; dblalign2 r15, r16, r17 }
	{ fdouble_pack2 r5, r6, r7 ; ld4u_add r15, r16, 5 }
	{ fdouble_pack2 r5, r6, r7 ; prefetch_l2 r15 }
	{ fdouble_pack2 r5, r6, r7 ; sub r15, r16, r17 }
	{ fdouble_pack2 r5, r6, r7 ; v2cmpltu r15, r16, r17 }
	{ fdouble_sub_flags r5, r6, r7 ; addx r15, r16, r17 }
	{ fdouble_sub_flags r5, r6, r7 ; iret }
	{ fdouble_sub_flags r5, r6, r7 ; movei r15, 5 }
	{ fdouble_sub_flags r5, r6, r7 ; shruxi r15, r16, 5 }
	{ fdouble_sub_flags r5, r6, r7 ; v1shl r15, r16, r17 }
	{ fdouble_sub_flags r5, r6, r7 ; v4add r15, r16, r17 }
	{ fdouble_unpack_max r5, r6, r7 ; fetchadd r15, r16, r17 }
	{ fdouble_unpack_max r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
	{ fdouble_unpack_max r5, r6, r7 ; rotli r15, r16, 5 }
	{ fdouble_unpack_max r5, r6, r7 ; v1cmpeq r15, r16, r17 }
	{ fdouble_unpack_max r5, r6, r7 ; v2maxsi r15, r16, 5 }
	{ fdouble_unpack_min r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ fdouble_unpack_min r5, r6, r7 ; ld1s r15, r16 }
	{ fdouble_unpack_min r5, r6, r7 ; or r15, r16, r17 }
	{ fdouble_unpack_min r5, r6, r7 ; st4 r15, r16 }
	{ fdouble_unpack_min r5, r6, r7 ; v1sub r15, r16, r17 }
	{ fdouble_unpack_min r5, r6, r7 ; v4shlsc r15, r16, r17 }
	{ fetchadd r15, r16, r17 ; crc32_8 r5, r6, r7 }
	{ fetchadd r15, r16, r17 ; mula_hs_hu r5, r6, r7 }
	{ fetchadd r15, r16, r17 ; subx r5, r6, r7 }
	{ fetchadd r15, r16, r17 ; v1mz r5, r6, r7 }
	{ fetchadd r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ fetchadd4 r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ fetchadd4 r15, r16, r17 ; fsingle_sub1 r5, r6, r7 }
	{ fetchadd4 r15, r16, r17 ; shl r5, r6, r7 }
	{ fetchadd4 r15, r16, r17 ; v1ddotpua r5, r6, r7 }
	{ fetchadd4 r15, r16, r17 ; v2cmpltsi r5, r6, 5 }
	{ fetchadd4 r15, r16, r17 ; v4shrs r5, r6, r7 }
	{ fetchaddgez r15, r16, r17 ; dblalign r5, r6, r7 }
	{ fetchaddgez r15, r16, r17 ; mula_hs_lu r5, r6, r7 }
	{ fetchaddgez r15, r16, r17 ; tblidxb0 r5, r6 }
	{ fetchaddgez r15, r16, r17 ; v1sadu r5, r6, r7 }
	{ fetchaddgez r15, r16, r17 ; v2sadau r5, r6, r7 }
	{ fetchaddgez4 r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ fetchaddgez4 r15, r16, r17 ; infol 0x1234 }
	{ fetchaddgez4 r15, r16, r17 ; shl1add r5, r6, r7 }
	{ fetchaddgez4 r15, r16, r17 ; v1ddotpusa r5, r6, r7 }
	{ fetchaddgez4 r15, r16, r17 ; v2cmpltui r5, r6, 5 }
	{ fetchaddgez4 r15, r16, r17 ; v4sub r5, r6, r7 }
	{ fetchand r15, r16, r17 ; dblalign4 r5, r6, r7 }
	{ fetchand r15, r16, r17 ; mula_hu_ls r5, r6, r7 }
	{ fetchand r15, r16, r17 ; tblidxb2 r5, r6 }
	{ fetchand r15, r16, r17 ; v1shli r5, r6, 5 }
	{ fetchand r15, r16, r17 ; v2sadu r5, r6, r7 }
	{ fetchand4 r15, r16, r17 ; cmples r5, r6, r7 }
	{ fetchand4 r15, r16, r17 ; mnz r5, r6, r7 }
	{ fetchand4 r15, r16, r17 ; shl2add r5, r6, r7 }
	{ fetchand4 r15, r16, r17 ; v1dotpa r5, r6, r7 }
	{ fetchand4 r15, r16, r17 ; v2dotp r5, r6, r7 }
	{ fetchand4 r15, r16, r17 ; xor r5, r6, r7 }
	{ fetchor r15, r16, r17 ; fdouble_add_flags r5, r6, r7 }
	{ fetchor r15, r16, r17 ; mula_ls_ls r5, r6, r7 }
	{ fetchor r15, r16, r17 ; v1add r5, r6, r7 }
	{ fetchor r15, r16, r17 ; v1shrsi r5, r6, 5 }
	{ fetchor r15, r16, r17 ; v2shli r5, r6, 5 }
	{ fetchor4 r15, r16, r17 ; cmplts r5, r6, r7 }
	{ fetchor4 r15, r16, r17 ; movei r5, 5 }
	{ fetchor4 r15, r16, r17 ; shl3add r5, r6, r7 }
	{ fetchor4 r15, r16, r17 ; v1dotpua r5, r6, r7 }
	{ fetchor4 r15, r16, r17 ; v2int_h r5, r6, r7 }
	{ finv r15 ; add r5, r6, r7 }
	{ finv r15 ; fdouble_mul_flags r5, r6, r7 }
	{ finv r15 ; mula_lu_lu r5, r6, r7 }
	{ finv r15 ; v1adduc r5, r6, r7 }
	{ finv r15 ; v1shrui r5, r6, 5 }
	{ finv r15 ; v2shrs r5, r6, r7 }
	{ flush r15 ; cmpltu r5, r6, r7 }
	{ flush r15 ; mul_hs_hs r5, r6, r7 }
	{ flush r15 ; shli r5, r6, 5 }
	{ flush r15 ; v1dotpusa r5, r6, r7 }
	{ flush r15 ; v2maxs r5, r6, r7 }
	{ flushwb ; addli r5, r6, 0x1234 }
	{ flushwb ; fdouble_pack2 r5, r6, r7 }
	{ flushwb ; mulx r5, r6, r7 }
	{ flushwb ; v1avgu r5, r6, r7 }
	{ flushwb ; v1subuc r5, r6, r7 }
	{ flushwb ; v2shru r5, r6, r7 }
	{ fnop ; add r5, r6, r7 ; ld2u r25, r26 }
	{ fnop ; addi r5, r6, 5 ; ld4u r25, r26 }
	{ fnop ; addx r5, r6, r7 ; ld4u r25, r26 }
	{ fnop ; addxi r5, r6, 5 ; prefetch_l1 r25 }
	{ fnop ; and r5, r6, r7 ; ld4u r25, r26 }
	{ fnop ; andi r5, r6, 5 ; prefetch_l1 r25 }
	{ fnop ; cmoveqz r5, r6, r7 ; prefetch r25 }
	{ fnop ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
	{ fnop ; cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 }
	{ fnop ; cmples r15, r16, r17 ; prefetch_l2_fault r25 }
	{ fnop ; cmpleu r15, r16, r17 ; prefetch_l3_fault r25 }
	{ fnop ; cmplts r15, r16, r17 ; st1 r25, r26 }
	{ fnop ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
	{ fnop ; cmpltu r5, r6, r7 ; ld r25, r26 }
	{ fnop ; cmpne r5, r6, r7 ; ld r25, r26 }
	{ fnop ; ctz r5, r6 ; prefetch_l3 r25 }
	{ fnop ; fnop ; ld2u r25, r26 }
	{ fnop ; icoh r15 }
	{ fnop ; inv r15 }
	{ fnop ; jr r15 ; ld r25, r26 }
	{ fnop ; ld r25, r26 ; add r5, r6, r7 }
	{ fnop ; ld r25, r26 ; mnz r15, r16, r17 }
	{ fnop ; ld r25, r26 ; shl3add r15, r16, r17 }
	{ fnop ; ld1s r25, r26 ; cmovnez r5, r6, r7 }
	{ fnop ; ld1s r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ fnop ; ld1s r25, r26 ; shrui r5, r6, 5 }
	{ fnop ; ld1u r25, r26 ; cmpltsi r5, r6, 5 }
	{ fnop ; ld1u r25, r26 ; revbytes r5, r6 }
	{ fnop ; ld1u_add r15, r16, 5 }
	{ fnop ; ld2s r25, r26 ; jr r15 }
	{ fnop ; ld2s r25, r26 ; shl2add r5, r6, r7 }
	{ fnop ; ld2u r25, r26 ; andi r15, r16, 5 }
	{ fnop ; ld2u r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ fnop ; ld2u r25, r26 ; shrsi r5, r6, 5 }
	{ fnop ; ld4s r25, r26 ; cmpleu r5, r6, r7 }
	{ fnop ; ld4s r25, r26 ; or r15, r16, r17 }
	{ fnop ; ld4s r25, r26 ; tblidxb3 r5, r6 }
	{ fnop ; ld4u r25, r26 ; ill }
	{ fnop ; ld4u r25, r26 ; shl1add r5, r6, r7 }
	{ fnop ; ldnt1u_add r15, r16, 5 }
	{ fnop ; mnz r15, r16, r17 ; prefetch_l1 r25 }
	{ fnop ; move r15, r16 ; prefetch_l2 r25 }
	{ fnop ; movei r15, 5 ; prefetch_l3 r25 }
	{ fnop ; mul_hs_hs r5, r6, r7 ; prefetch_l2_fault r25 }
	{ fnop ; mul_ls_ls r5, r6, r7 ; prefetch_l1 r25 }
	{ fnop ; mula_hs_hs r5, r6, r7 ; prefetch_l1_fault r25 }
	{ fnop ; mula_ls_ls r5, r6, r7 ; ld4u r25, r26 }
	{ fnop ; mulax r5, r6, r7 ; prefetch r25 }
	{ fnop ; mz r15, r16, r17 ; prefetch_l1_fault r25 }
	{ fnop ; nop ; prefetch_l2_fault r25 }
	{ fnop ; nor r5, r6, r7 ; prefetch_l3_fault r25 }
	{ fnop ; or r5, r6, r7 ; st1 r25, r26 }
	{ fnop ; prefetch r25 ; cmovnez r5, r6, r7 }
	{ fnop ; prefetch r25 ; mula_lu_lu r5, r6, r7 }
	{ fnop ; prefetch r25 ; shrui r5, r6, 5 }
	{ fnop ; prefetch_l1 r25 ; cmpleu r15, r16, r17 }
	{ fnop ; prefetch_l1 r25 ; nor r5, r6, r7 }
	{ fnop ; prefetch_l1 r25 ; tblidxb2 r5, r6 }
	{ fnop ; prefetch_l1_fault r25 ; ill }
	{ fnop ; prefetch_l1_fault r25 ; shl1add r5, r6, r7 }
	{ fnop ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ fnop ; prefetch_l2 r25 ; mul_hs_hs r5, r6, r7 }
	{ fnop ; prefetch_l2 r25 ; shrs r15, r16, r17 }
	{ fnop ; prefetch_l2_fault r25 ; cmples r5, r6, r7 }
	{ fnop ; prefetch_l2_fault r25 ; nor r15, r16, r17 }
	{ fnop ; prefetch_l2_fault r25 ; tblidxb1 r5, r6 }
	{ fnop ; prefetch_l3 r25 ; fsingle_pack1 r5, r6 }
	{ fnop ; prefetch_l3 r25 ; shl1add r15, r16, r17 }
	{ fnop ; prefetch_l3_fault r25 ; addxi r15, r16, 5 }
	{ fnop ; prefetch_l3_fault r25 ; movei r5, 5 }
	{ fnop ; prefetch_l3_fault r25 ; shli r5, r6, 5 }
	{ fnop ; revbytes r5, r6 ; ld r25, r26 }
	{ fnop ; rotl r5, r6, r7 ; ld1u r25, r26 }
	{ fnop ; rotli r5, r6, 5 ; ld2u r25, r26 }
	{ fnop ; shl r5, r6, r7 ; ld4u r25, r26 }
	{ fnop ; shl1add r5, r6, r7 ; ld4u r25, r26 }
	{ fnop ; shl1addx r5, r6, r7 ; prefetch_l1 r25 }
	{ fnop ; shl2add r5, r6, r7 ; prefetch_l2 r25 }
	{ fnop ; shl2addx r5, r6, r7 ; prefetch_l3 r25 }
	{ fnop ; shl3add r5, r6, r7 ; st r25, r26 }
	{ fnop ; shl3addx r5, r6, r7 ; st2 r25, r26 }
	{ fnop ; shli r5, r6, 5 }
	{ fnop ; shrs r5, r6, r7 ; st2 r25, r26 }
	{ fnop ; shrsi r5, r6, 5 }
	{ fnop ; shrui r15, r16, 5 ; ld1s r25, r26 }
	{ fnop ; shruxi r5, r6, 5 }
	{ fnop ; st r25, r26 ; jalrp r15 }
	{ fnop ; st r25, r26 ; shl2add r15, r16, r17 }
	{ fnop ; st1 r25, r26 ; andi r15, r16, 5 }
	{ fnop ; st1 r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ fnop ; st1 r25, r26 ; shrsi r5, r6, 5 }
	{ fnop ; st2 r25, r26 ; cmpleu r5, r6, r7 }
	{ fnop ; st2 r25, r26 ; or r15, r16, r17 }
	{ fnop ; st2 r25, r26 ; tblidxb3 r5, r6 }
	{ fnop ; st4 r25, r26 ; ill }
	{ fnop ; st4 r25, r26 ; shl1add r5, r6, r7 }
	{ fnop ; stnt4_add r15, r16, 5 }
	{ fnop ; subx r15, r16, r17 ; ld r25, r26 }
	{ fnop ; tblidxb0 r5, r6 ; ld r25, r26 }
	{ fnop ; tblidxb2 r5, r6 ; ld1u r25, r26 }
	{ fnop ; v1adduc r15, r16, r17 }
	{ fnop ; v1minu r15, r16, r17 }
	{ fnop ; v2cmpeqi r5, r6, 5 }
	{ fnop ; v2packuc r15, r16, r17 }
	{ fnop ; v4shru r15, r16, r17 }
	{ fnop ; xor r5, r6, r7 ; st r25, r26 }
	{ fsingle_add1 r5, r6, r7 ; fetchor4 r15, r16, r17 }
	{ fsingle_add1 r5, r6, r7 ; ldnt4s r15, r16 }
	{ fsingle_add1 r5, r6, r7 ; shl3add r15, r16, r17 }
	{ fsingle_add1 r5, r6, r7 ; v1cmpltui r15, r16, 5 }
	{ fsingle_add1 r5, r6, r7 ; v2packuc r15, r16, r17 }
	{ fsingle_addsub2 r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ fsingle_addsub2 r5, r6, r7 ; ld2u_add r15, r16, 5 }
	{ fsingle_addsub2 r5, r6, r7 ; prefetch_add_l3 r15, 5 }
	{ fsingle_addsub2 r5, r6, r7 ; stnt2_add r15, r16, 5 }
	{ fsingle_addsub2 r5, r6, r7 ; v2cmples r15, r16, r17 }
	{ fsingle_addsub2 r5, r6, r7 ; xori r15, r16, 5 }
	{ fsingle_mul1 r5, r6, r7 ; ill }
	{ fsingle_mul1 r5, r6, r7 ; mf }
	{ fsingle_mul1 r5, r6, r7 ; shrsi r15, r16, 5 }
	{ fsingle_mul1 r5, r6, r7 ; v1minu r15, r16, r17 }
	{ fsingle_mul1 r5, r6, r7 ; v2shru r15, r16, r17 }
	{ fsingle_mul2 r5, r6, r7 ; dblalign6 r15, r16, r17 }
	{ fsingle_mul2 r5, r6, r7 ; ldna r15, r16 }
	{ fsingle_mul2 r5, r6, r7 ; prefetch_l3 r15 }
	{ fsingle_mul2 r5, r6, r7 ; subxsc r15, r16, r17 }
	{ fsingle_mul2 r5, r6, r7 ; v2cmpne r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; add r15, r16, r17 ; ld4s r25, r26 }
	{ fsingle_pack1 r5, r6 ; addx r15, r16, r17 ; ld4u r25, r26 }
	{ fsingle_pack1 r5, r6 ; and r15, r16, r17 ; ld4u r25, r26 }
	{ fsingle_pack1 r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l1 r25 }
	{ fsingle_pack1 r5, r6 ; cmples r15, r16, r17 ; prefetch_l1 r25 }
	{ fsingle_pack1 r5, r6 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
	{ fsingle_pack1 r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
	{ fsingle_pack1 r5, r6 ; fetchor4 r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; ill ; st2 r25, r26 }
	{ fsingle_pack1 r5, r6 ; jalr r15 ; st1 r25, r26 }
	{ fsingle_pack1 r5, r6 ; jr r15 ; st4 r25, r26 }
	{ fsingle_pack1 r5, r6 ; ld r25, r26 ; jalrp r15 }
	{ fsingle_pack1 r5, r6 ; ld1s r25, r26 ; cmplts r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; ld1u r25, r26 ; addi r15, r16, 5 }
	{ fsingle_pack1 r5, r6 ; ld1u r25, r26 ; shru r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; ld2s r25, r26 ; shl1add r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; ld2u r25, r26 ; move r15, r16 }
	{ fsingle_pack1 r5, r6 ; ld4s r25, r26 ; fnop }
	{ fsingle_pack1 r5, r6 ; ld4u r25, r26 ; andi r15, r16, 5 }
	{ fsingle_pack1 r5, r6 ; ld4u r25, r26 ; xor r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; mfspr r16, 0x5 }
	{ fsingle_pack1 r5, r6 ; movei r15, 5 ; ld1s r25, r26 }
	{ fsingle_pack1 r5, r6 ; nop ; ld1s r25, r26 }
	{ fsingle_pack1 r5, r6 ; or r15, r16, r17 ; ld2s r25, r26 }
	{ fsingle_pack1 r5, r6 ; prefetch r25 ; mnz r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; prefetch_l1 r25 ; cmples r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; prefetch_l1_fault r25 ; add r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; prefetch_l1_fault r25 ; shrsi r15, r16, 5 }
	{ fsingle_pack1 r5, r6 ; prefetch_l2 r25 ; shl1add r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; prefetch_l2_fault r25 ; movei r15, 5 }
	{ fsingle_pack1 r5, r6 ; prefetch_l3 r25 ; info 19 }
	{ fsingle_pack1 r5, r6 ; prefetch_l3_fault r25 ; cmples r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; rotl r15, r16, r17 ; ld r25, r26 }
	{ fsingle_pack1 r5, r6 ; shl r15, r16, r17 ; ld1u r25, r26 }
	{ fsingle_pack1 r5, r6 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
	{ fsingle_pack1 r5, r6 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
	{ fsingle_pack1 r5, r6 ; shl3addx r15, r16, r17 ; prefetch r25 }
	{ fsingle_pack1 r5, r6 ; shrs r15, r16, r17 ; prefetch r25 }
	{ fsingle_pack1 r5, r6 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
	{ fsingle_pack1 r5, r6 ; st r25, r26 ; cmples r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; st1 r25, r26 ; add r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; st1 r25, r26 ; shrsi r15, r16, 5 }
	{ fsingle_pack1 r5, r6 ; st2 r25, r26 ; shl r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; st4 r25, r26 ; mnz r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; sub r15, r16, r17 ; ld4s r25, r26 }
	{ fsingle_pack1 r5, r6 ; v1cmpleu r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; v2mnz r15, r16, r17 }
	{ fsingle_pack1 r5, r6 ; xor r15, r16, r17 ; st r25, r26 }
	{ fsingle_pack2 r5, r6, r7 ; finv r15 }
	{ fsingle_pack2 r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
	{ fsingle_pack2 r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ fsingle_pack2 r5, r6, r7 ; v1cmpne r15, r16, r17 }
	{ fsingle_pack2 r5, r6, r7 ; v2shl r15, r16, r17 }
	{ fsingle_sub1 r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ fsingle_sub1 r5, r6, r7 ; ld4s r15, r16 }
	{ fsingle_sub1 r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ fsingle_sub1 r5, r6, r7 ; stnt4 r15, r16 }
	{ fsingle_sub1 r5, r6, r7 ; v2cmpleu r15, r16, r17 }
	{ icoh r15 ; add r5, r6, r7 }
	{ icoh r15 ; fdouble_mul_flags r5, r6, r7 }
	{ icoh r15 ; mula_lu_lu r5, r6, r7 }
	{ icoh r15 ; v1adduc r5, r6, r7 }
	{ icoh r15 ; v1shrui r5, r6, 5 }
	{ icoh r15 ; v2shrs r5, r6, r7 }
	{ ill ; addi r5, r6, 5 ; ld1u r25, r26 }
	{ ill ; addxi r5, r6, 5 ; ld2s r25, r26 }
	{ ill ; andi r5, r6, 5 ; ld2s r25, r26 }
	{ ill ; cmoveqz r5, r6, r7 ; ld1u r25, r26 }
	{ ill ; cmpeq r5, r6, r7 ; ld2u r25, r26 }
	{ ill ; cmples r5, r6, r7 ; ld4u r25, r26 }
	{ ill ; cmplts r5, r6, r7 ; prefetch_l1 r25 }
	{ ill ; cmpltu r5, r6, r7 ; prefetch_l2 r25 }
	{ ill ; ctz r5, r6 ; ld1u r25, r26 }
	{ ill ; fnop ; prefetch_l2_fault r25 }
	{ ill ; info 19 ; prefetch r25 }
	{ ill ; ld r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ ill ; ld1s r25, r26 ; and r5, r6, r7 }
	{ ill ; ld1s r25, r26 ; shl1add r5, r6, r7 }
	{ ill ; ld1u r25, r26 ; mnz r5, r6, r7 }
	{ ill ; ld1u r25, r26 ; xor r5, r6, r7 }
	{ ill ; ld2s r25, r26 ; pcnt r5, r6 }
	{ ill ; ld2u r25, r26 ; cmpltu r5, r6, r7 }
	{ ill ; ld2u r25, r26 ; sub r5, r6, r7 }
	{ ill ; ld4s r25, r26 ; mulax r5, r6, r7 }
	{ ill ; ld4u r25, r26 ; cmpeq r5, r6, r7 }
	{ ill ; ld4u r25, r26 ; shl3addx r5, r6, r7 }
	{ ill ; move r5, r6 ; ld4u r25, r26 }
	{ ill ; mul_hs_hs r5, r6, r7 ; prefetch r25 }
	{ ill ; mul_ls_ls r5, r6, r7 ; ld2u r25, r26 }
	{ ill ; mula_hs_hs r5, r6, r7 ; ld4s r25, r26 }
	{ ill ; mula_ls_ls r5, r6, r7 ; ld1u r25, r26 }
	{ ill ; mulax r5, r6, r7 ; ld2s r25, r26 }
	{ ill ; mz r5, r6, r7 ; ld4s r25, r26 }
	{ ill ; nor r5, r6, r7 ; prefetch r25 }
	{ ill ; pcnt r5, r6 ; prefetch_l1 r25 }
	{ ill ; prefetch r25 ; mula_hu_hu r5, r6, r7 }
	{ ill ; prefetch_l1 r25 ; clz r5, r6 }
	{ ill ; prefetch_l1 r25 ; shl2add r5, r6, r7 }
	{ ill ; prefetch_l1_fault r25 ; movei r5, 5 }
	{ ill ; prefetch_l2 r25 ; add r5, r6, r7 }
	{ ill ; prefetch_l2 r25 ; revbytes r5, r6 }
	{ ill ; prefetch_l2_fault r25 ; ctz r5, r6 }
	{ ill ; prefetch_l2_fault r25 ; tblidxb0 r5, r6 }
	{ ill ; prefetch_l3 r25 ; mz r5, r6, r7 }
	{ ill ; prefetch_l3_fault r25 ; cmples r5, r6, r7 }
	{ ill ; prefetch_l3_fault r25 ; shrs r5, r6, r7 }
	{ ill ; revbytes r5, r6 ; prefetch_l1_fault r25 }
	{ ill ; rotli r5, r6, 5 ; prefetch_l2_fault r25 }
	{ ill ; shl1add r5, r6, r7 ; prefetch_l3 r25 }
	{ ill ; shl2add r5, r6, r7 ; st r25, r26 }
	{ ill ; shl3add r5, r6, r7 ; st2 r25, r26 }
	{ ill ; shli r5, r6, 5 }
	{ ill ; shrsi r5, r6, 5 }
	{ ill ; shruxi r5, r6, 5 }
	{ ill ; st r25, r26 ; pcnt r5, r6 }
	{ ill ; st1 r25, r26 ; cmpltu r5, r6, r7 }
	{ ill ; st1 r25, r26 ; sub r5, r6, r7 }
	{ ill ; st2 r25, r26 ; mulax r5, r6, r7 }
	{ ill ; st4 r25, r26 ; cmpeq r5, r6, r7 }
	{ ill ; st4 r25, r26 ; shl3addx r5, r6, r7 }
	{ ill ; subx r5, r6, r7 ; prefetch r25 }
	{ ill ; tblidxb1 r5, r6 ; prefetch_l1 r25 }
	{ ill ; tblidxb3 r5, r6 ; prefetch_l2 r25 }
	{ ill ; v1multu r5, r6, r7 }
	{ ill ; v2mz r5, r6, r7 }
	{ ill ; xor r5, r6, r7 ; prefetch_l3 r25 }
	{ info 19 ; add r5, r6, r7 ; prefetch_l3_fault r25 }
	{ info 19 ; addi r5, r6, 5 ; st1 r25, r26 }
	{ info 19 ; addx r5, r6, r7 ; st1 r25, r26 }
	{ info 19 ; addxi r5, r6, 5 ; st4 r25, r26 }
	{ info 19 ; and r5, r6, r7 ; st1 r25, r26 }
	{ info 19 ; andi r5, r6, 5 ; st4 r25, r26 }
	{ info 19 ; cmoveqz r5, r6, r7 ; st2 r25, r26 }
	{ info 19 ; cmpeq r15, r16, r17 }
	{ info 19 ; cmpeqi r5, r6, 5 ; ld1s r25, r26 }
	{ info 19 ; cmples r5, r6, r7 ; ld1s r25, r26 }
	{ info 19 ; cmpleu r5, r6, r7 ; ld2s r25, r26 }
	{ info 19 ; cmplts r5, r6, r7 ; ld4s r25, r26 }
	{ info 19 ; cmpltsi r5, r6, 5 ; prefetch r25 }
	{ info 19 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
	{ info 19 ; cmpne r5, r6, r7 ; prefetch_l1_fault r25 }
	{ info 19 ; dblalign2 r5, r6, r7 }
	{ info 19 ; fnop ; prefetch_l3_fault r25 }
	{ info 19 ; ill ; prefetch_l1 r25 }
	{ info 19 ; jalr r15 ; prefetch r25 }
	{ info 19 ; jr r15 ; prefetch_l1_fault r25 }
	{ info 19 ; ld r25, r26 ; andi r15, r16, 5 }
	{ info 19 ; ld r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ info 19 ; ld r25, r26 ; shrsi r5, r6, 5 }
	{ info 19 ; ld1s r25, r26 ; cmplts r15, r16, r17 }
	{ info 19 ; ld1s r25, r26 ; or r5, r6, r7 }
	{ info 19 ; ld1s r25, r26 ; xor r15, r16, r17 }
	{ info 19 ; ld1u r25, r26 ; info 19 }
	{ info 19 ; ld1u r25, r26 ; shl1addx r15, r16, r17 }
	{ info 19 ; ld2s r25, r26 ; addxi r5, r6, 5 }
	{ info 19 ; ld2s r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ info 19 ; ld2s r25, r26 ; shrs r15, r16, r17 }
	{ info 19 ; ld2u r25, r26 ; cmples r15, r16, r17 }
	{ info 19 ; ld2u r25, r26 ; nop }
	{ info 19 ; ld2u r25, r26 ; tblidxb0 r5, r6 }
	{ info 19 ; ld4s r25, r26 ; ctz r5, r6 }
	{ info 19 ; ld4s r25, r26 ; shl r15, r16, r17 }
	{ info 19 ; ld4u r25, r26 ; addi r5, r6, 5 }
	{ info 19 ; ld4u r25, r26 ; move r15, r16 }
	{ info 19 ; ld4u r25, r26 ; shl3addx r15, r16, r17 }
	{ info 19 ; ldnt_add r15, r16, 5 }
	{ info 19 ; mnz r15, r16, r17 ; st4 r25, r26 }
	{ info 19 ; move r5, r6 ; ld r25, r26 }
	{ info 19 ; movei r5, 5 ; ld1u r25, r26 }
	{ info 19 ; mul_hs_ls r5, r6, r7 }
	{ info 19 ; mul_ls_ls r5, r6, r7 ; st4 r25, r26 }
	{ info 19 ; mula_hs_hs r5, r6, r7 }
	{ info 19 ; mula_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ info 19 ; mulax r5, r6, r7 ; st2 r25, r26 }
	{ info 19 ; mz r15, r16, r17 }
	{ info 19 ; nor r15, r16, r17 ; ld1s r25, r26 }
	{ info 19 ; or r15, r16, r17 ; ld2s r25, r26 }
	{ info 19 ; pcnt r5, r6 ; ld2s r25, r26 }
	{ info 19 ; prefetch r25 ; cmplts r15, r16, r17 }
	{ info 19 ; prefetch r25 ; or r5, r6, r7 }
	{ info 19 ; prefetch r25 ; xor r15, r16, r17 }
	{ info 19 ; prefetch_l1 r25 ; cmpne r5, r6, r7 }
	{ info 19 ; prefetch_l1 r25 ; rotli r5, r6, 5 }
	{ info 19 ; prefetch_l1_fault r25 ; addi r5, r6, 5 }
	{ info 19 ; prefetch_l1_fault r25 ; move r15, r16 }
	{ info 19 ; prefetch_l1_fault r25 ; shl3addx r15, r16, r17 }
	{ info 19 ; prefetch_l2 r25 ; cmpeq r5, r6, r7 }
	{ info 19 ; prefetch_l2 r25 ; mulx r5, r6, r7 }
	{ info 19 ; prefetch_l2 r25 ; sub r5, r6, r7 }
	{ info 19 ; prefetch_l2_fault r25 ; cmpne r15, r16, r17 }
	{ info 19 ; prefetch_l2_fault r25 ; rotli r15, r16, 5 }
	{ info 19 ; prefetch_l3 r25 ; addi r15, r16, 5 }
	{ info 19 ; prefetch_l3 r25 ; mnz r5, r6, r7 }
	{ info 19 ; prefetch_l3 r25 ; shl3add r5, r6, r7 }
	{ info 19 ; prefetch_l3_fault r25 ; cmpeq r15, r16, r17 }
	{ info 19 ; prefetch_l3_fault r25 ; mulax r5, r6, r7 }
	{ info 19 ; prefetch_l3_fault r25 ; sub r15, r16, r17 }
	{ info 19 ; revbytes r5, r6 ; prefetch_l1_fault r25 }
	{ info 19 ; rotl r5, r6, r7 ; prefetch_l2_fault r25 }
	{ info 19 ; rotli r5, r6, 5 ; prefetch_l3_fault r25 }
	{ info 19 ; shl r5, r6, r7 ; st1 r25, r26 }
	{ info 19 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ info 19 ; shl1addx r5, r6, r7 ; st4 r25, r26 }
	{ info 19 ; shl2addx r15, r16, r17 ; ld r25, r26 }
	{ info 19 ; shl3add r15, r16, r17 ; ld1u r25, r26 }
	{ info 19 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
	{ info 19 ; shli r15, r16, 5 ; ld4u r25, r26 }
	{ info 19 ; shrs r15, r16, r17 ; ld2u r25, r26 }
	{ info 19 ; shrsi r15, r16, 5 ; ld4u r25, r26 }
	{ info 19 ; shru r15, r16, r17 ; prefetch_l1 r25 }
	{ info 19 ; shrui r15, r16, 5 ; prefetch_l2 r25 }
	{ info 19 ; st r25, r26 ; addxi r15, r16, 5 }
	{ info 19 ; st r25, r26 ; movei r5, 5 }
	{ info 19 ; st r25, r26 ; shli r5, r6, 5 }
	{ info 19 ; st1 r25, r26 ; cmples r15, r16, r17 }
	{ info 19 ; st1 r25, r26 ; nop }
	{ info 19 ; st1 r25, r26 ; tblidxb0 r5, r6 }
	{ info 19 ; st2 r25, r26 ; ctz r5, r6 }
	{ info 19 ; st2 r25, r26 ; shl r15, r16, r17 }
	{ info 19 ; st4 r25, r26 ; addi r5, r6, 5 }
	{ info 19 ; st4 r25, r26 ; move r15, r16 }
	{ info 19 ; st4 r25, r26 ; shl3addx r15, r16, r17 }
	{ info 19 ; sub r15, r16, r17 ; prefetch r25 }
	{ info 19 ; subx r15, r16, r17 ; prefetch_l1_fault r25 }
	{ info 19 ; tblidxb0 r5, r6 ; prefetch_l1_fault r25 }
	{ info 19 ; tblidxb2 r5, r6 ; prefetch_l2_fault r25 }
	{ info 19 ; v1cmples r5, r6, r7 }
	{ info 19 ; v1mz r15, r16, r17 }
	{ info 19 ; v2cmpltu r15, r16, r17 }
	{ info 19 ; v2shli r5, r6, 5 }
	{ info 19 ; xor r15, r16, r17 ; ld1u r25, r26 }
	{ infol 0x1234 ; addi r15, r16, 5 }
	{ infol 0x1234 ; cmpne r15, r16, r17 }
	{ infol 0x1234 ; flushwb }
	{ infol 0x1234 ; ldnt2s r15, r16 }
	{ infol 0x1234 ; mula_ls_lu r5, r6, r7 }
	{ infol 0x1234 ; shl1addx r15, r16, r17 }
	{ infol 0x1234 ; stnt2 r15, r16 }
	{ infol 0x1234 ; v1cmpne r5, r6, r7 }
	{ infol 0x1234 ; v1shru r15, r16, r17 }
	{ infol 0x1234 ; v2maxs r15, r16, r17 }
	{ infol 0x1234 ; v2sub r5, r6, r7 }
	{ inv r15 ; bfextu r5, r6, 5, 7 }
	{ inv r15 ; fsingle_mul2 r5, r6, r7 }
	{ inv r15 ; revbytes r5, r6 }
	{ inv r15 ; v1cmpltui r5, r6, 5 }
	{ inv r15 ; v2cmples r5, r6, r7 }
	{ inv r15 ; v4packsc r5, r6, r7 }
	{ iret ; crc32_32 r5, r6, r7 }
	{ iret ; mula_hs_hs r5, r6, r7 }
	{ iret ; sub r5, r6, r7 }
	{ iret ; v1mulus r5, r6, r7 }
	{ iret ; v2packl r5, r6, r7 }
	{ jalr r15 ; add r5, r6, r7 ; prefetch_l3 r25 }
	{ jalr r15 ; addx r5, r6, r7 ; prefetch_l3_fault r25 }
	{ jalr r15 ; and r5, r6, r7 ; prefetch_l3_fault r25 }
	{ jalr r15 ; clz r5, r6 ; prefetch_l3 r25 }
	{ jalr r15 ; cmovnez r5, r6, r7 ; st r25, r26 }
	{ jalr r15 ; cmpeqi r5, r6, 5 ; st2 r25, r26 }
	{ jalr r15 ; cmpleu r5, r6, r7 }
	{ jalr r15 ; cmpltu r5, r6, r7 ; ld1s r25, r26 }
	{ jalr r15 ; cmulaf r5, r6, r7 }
	{ jalr r15 ; fnop ; ld1u r25, r26 }
	{ jalr r15 ; fsingle_pack2 r5, r6, r7 }
	{ jalr r15 ; ld r25, r26 ; fnop }
	{ jalr r15 ; ld r25, r26 ; tblidxb1 r5, r6 }
	{ jalr r15 ; ld1s r25, r26 ; nop }
	{ jalr r15 ; ld1u r25, r26 ; cmpleu r5, r6, r7 }
	{ jalr r15 ; ld1u r25, r26 ; shrsi r5, r6, 5 }
	{ jalr r15 ; ld2s r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ jalr r15 ; ld2u r25, r26 ; clz r5, r6 }
	{ jalr r15 ; ld2u r25, r26 ; shl2add r5, r6, r7 }
	{ jalr r15 ; ld4s r25, r26 ; movei r5, 5 }
	{ jalr r15 ; ld4u r25, r26 ; add r5, r6, r7 }
	{ jalr r15 ; ld4u r25, r26 ; revbytes r5, r6 }
	{ jalr r15 ; mnz r5, r6, r7 ; st2 r25, r26 }
	{ jalr r15 ; movei r5, 5 }
	{ jalr r15 ; mul_hu_hu r5, r6, r7 ; st2 r25, r26 }
	{ jalr r15 ; mul_lu_lu r5, r6, r7 ; st1 r25, r26 }
	{ jalr r15 ; mula_hu_hu r5, r6, r7 ; st r25, r26 }
	{ jalr r15 ; mula_lu_lu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ jalr r15 ; mulx r5, r6, r7 ; st1 r25, r26 }
	{ jalr r15 ; nop ; st4 r25, r26 }
	{ jalr r15 ; ori r5, r6, 5 }
	{ jalr r15 ; prefetch r25 ; info 19 }
	{ jalr r15 ; prefetch r25 ; tblidxb3 r5, r6 }
	{ jalr r15 ; prefetch_l1 r25 ; or r5, r6, r7 }
	{ jalr r15 ; prefetch_l1_fault r25 ; cmpltsi r5, r6, 5 }
	{ jalr r15 ; prefetch_l1_fault r25 ; shrui r5, r6, 5 }
	{ jalr r15 ; prefetch_l2 r25 ; mula_lu_lu r5, r6, r7 }
	{ jalr r15 ; prefetch_l2_fault r25 ; cmovnez r5, r6, r7 }
	{ jalr r15 ; prefetch_l2_fault r25 ; shl3add r5, r6, r7 }
	{ jalr r15 ; prefetch_l3 r25 ; mul_hu_hu r5, r6, r7 }
	{ jalr r15 ; prefetch_l3_fault r25 ; addx r5, r6, r7 }
	{ jalr r15 ; prefetch_l3_fault r25 ; rotli r5, r6, 5 }
	{ jalr r15 ; revbytes r5, r6 ; ld r25, r26 }
	{ jalr r15 ; rotli r5, r6, 5 ; ld1u r25, r26 }
	{ jalr r15 ; shl1add r5, r6, r7 ; ld2s r25, r26 }
	{ jalr r15 ; shl2add r5, r6, r7 ; ld4s r25, r26 }
	{ jalr r15 ; shl3add r5, r6, r7 ; prefetch r25 }
	{ jalr r15 ; shli r5, r6, 5 ; prefetch_l1_fault r25 }
	{ jalr r15 ; shrsi r5, r6, 5 ; prefetch_l1_fault r25 }
	{ jalr r15 ; shrui r5, r6, 5 ; prefetch_l2_fault r25 }
	{ jalr r15 ; st r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ jalr r15 ; st1 r25, r26 ; clz r5, r6 }
	{ jalr r15 ; st1 r25, r26 ; shl2add r5, r6, r7 }
	{ jalr r15 ; st2 r25, r26 ; movei r5, 5 }
	{ jalr r15 ; st4 r25, r26 ; add r5, r6, r7 }
	{ jalr r15 ; st4 r25, r26 ; revbytes r5, r6 }
	{ jalr r15 ; sub r5, r6, r7 ; st4 r25, r26 }
	{ jalr r15 ; tblidxb0 r5, r6 }
	{ jalr r15 ; tblidxb3 r5, r6 ; ld1s r25, r26 }
	{ jalr r15 ; v1dotpus r5, r6, r7 }
	{ jalr r15 ; v2int_l r5, r6, r7 }
	{ jalr r15 ; xor r5, r6, r7 ; ld2s r25, r26 }
	{ jalrp r15 ; addi r5, r6, 5 ; ld2u r25, r26 }
	{ jalrp r15 ; addxi r5, r6, 5 ; ld4s r25, r26 }
	{ jalrp r15 ; andi r5, r6, 5 ; ld4s r25, r26 }
	{ jalrp r15 ; cmoveqz r5, r6, r7 ; ld2u r25, r26 }
	{ jalrp r15 ; cmpeq r5, r6, r7 ; ld4u r25, r26 }
	{ jalrp r15 ; cmples r5, r6, r7 ; prefetch_l1 r25 }
	{ jalrp r15 ; cmplts r5, r6, r7 ; prefetch_l2 r25 }
	{ jalrp r15 ; cmpltu r5, r6, r7 ; prefetch_l3 r25 }
	{ jalrp r15 ; ctz r5, r6 ; ld2u r25, r26 }
	{ jalrp r15 ; fnop ; prefetch_l3_fault r25 }
	{ jalrp r15 ; info 19 ; prefetch_l1_fault r25 }
	{ jalrp r15 ; ld r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ jalrp r15 ; ld1s r25, r26 ; clz r5, r6 }
	{ jalrp r15 ; ld1s r25, r26 ; shl2add r5, r6, r7 }
	{ jalrp r15 ; ld1u r25, r26 ; movei r5, 5 }
	{ jalrp r15 ; ld2s r25, r26 ; add r5, r6, r7 }
	{ jalrp r15 ; ld2s r25, r26 ; revbytes r5, r6 }
	{ jalrp r15 ; ld2u r25, r26 ; ctz r5, r6 }
	{ jalrp r15 ; ld2u r25, r26 ; tblidxb0 r5, r6 }
	{ jalrp r15 ; ld4s r25, r26 ; mz r5, r6, r7 }
	{ jalrp r15 ; ld4u r25, r26 ; cmples r5, r6, r7 }
	{ jalrp r15 ; ld4u r25, r26 ; shrs r5, r6, r7 }
	{ jalrp r15 ; move r5, r6 ; prefetch_l1 r25 }
	{ jalrp r15 ; mul_hs_hs r5, r6, r7 ; prefetch_l1_fault r25 }
	{ jalrp r15 ; mul_ls_ls r5, r6, r7 ; ld4u r25, r26 }
	{ jalrp r15 ; mula_hs_hs r5, r6, r7 ; prefetch r25 }
	{ jalrp r15 ; mula_ls_ls r5, r6, r7 ; ld2u r25, r26 }
	{ jalrp r15 ; mulax r5, r6, r7 ; ld4s r25, r26 }
	{ jalrp r15 ; mz r5, r6, r7 ; prefetch r25 }
	{ jalrp r15 ; nor r5, r6, r7 ; prefetch_l1_fault r25 }
	{ jalrp r15 ; pcnt r5, r6 ; prefetch_l2 r25 }
	{ jalrp r15 ; prefetch r25 ; mula_lu_lu r5, r6, r7 }
	{ jalrp r15 ; prefetch_l1 r25 ; cmovnez r5, r6, r7 }
	{ jalrp r15 ; prefetch_l1 r25 ; shl3add r5, r6, r7 }
	{ jalrp r15 ; prefetch_l1_fault r25 ; mul_hu_hu r5, r6, r7 }
	{ jalrp r15 ; prefetch_l2 r25 ; addx r5, r6, r7 }
	{ jalrp r15 ; prefetch_l2 r25 ; rotli r5, r6, 5 }
	{ jalrp r15 ; prefetch_l2_fault r25 ; fsingle_pack1 r5, r6 }
	{ jalrp r15 ; prefetch_l2_fault r25 ; tblidxb2 r5, r6 }
	{ jalrp r15 ; prefetch_l3 r25 ; nor r5, r6, r7 }
	{ jalrp r15 ; prefetch_l3_fault r25 ; cmplts r5, r6, r7 }
	{ jalrp r15 ; prefetch_l3_fault r25 ; shru r5, r6, r7 }
	{ jalrp r15 ; revbytes r5, r6 ; prefetch_l2_fault r25 }
	{ jalrp r15 ; rotli r5, r6, 5 ; prefetch_l3_fault r25 }
	{ jalrp r15 ; shl1add r5, r6, r7 ; st r25, r26 }
	{ jalrp r15 ; shl2add r5, r6, r7 ; st2 r25, r26 }
	{ jalrp r15 ; shl3add r5, r6, r7 }
	{ jalrp r15 ; shlxi r5, r6, 5 }
	{ jalrp r15 ; shru r5, r6, r7 ; ld1s r25, r26 }
	{ jalrp r15 ; st r25, r26 ; add r5, r6, r7 }
	{ jalrp r15 ; st r25, r26 ; revbytes r5, r6 }
	{ jalrp r15 ; st1 r25, r26 ; ctz r5, r6 }
	{ jalrp r15 ; st1 r25, r26 ; tblidxb0 r5, r6 }
	{ jalrp r15 ; st2 r25, r26 ; mz r5, r6, r7 }
	{ jalrp r15 ; st4 r25, r26 ; cmples r5, r6, r7 }
	{ jalrp r15 ; st4 r25, r26 ; shrs r5, r6, r7 }
	{ jalrp r15 ; subx r5, r6, r7 ; prefetch_l1_fault r25 }
	{ jalrp r15 ; tblidxb1 r5, r6 ; prefetch_l2 r25 }
	{ jalrp r15 ; tblidxb3 r5, r6 ; prefetch_l3 r25 }
	{ jalrp r15 ; v1mulus r5, r6, r7 }
	{ jalrp r15 ; v2packl r5, r6, r7 }
	{ jalrp r15 ; xor r5, r6, r7 ; st r25, r26 }
	{ jr r15 ; addi r5, r6, 5 ; st1 r25, r26 }
	{ jr r15 ; addxi r5, r6, 5 ; st2 r25, r26 }
	{ jr r15 ; andi r5, r6, 5 ; st2 r25, r26 }
	{ jr r15 ; cmoveqz r5, r6, r7 ; st1 r25, r26 }
	{ jr r15 ; cmpeq r5, r6, r7 ; st4 r25, r26 }
	{ jr r15 ; cmpleu r5, r6, r7 ; ld r25, r26 }
	{ jr r15 ; cmpltsi r5, r6, 5 ; ld1u r25, r26 }
	{ jr r15 ; cmpne r5, r6, r7 ; ld2s r25, r26 }
	{ jr r15 ; ctz r5, r6 ; st1 r25, r26 }
	{ jr r15 ; fsingle_pack1 r5, r6 ; ld1s r25, r26 }
	{ jr r15 ; ld r25, r26 ; add r5, r6, r7 }
	{ jr r15 ; ld r25, r26 ; revbytes r5, r6 }
	{ jr r15 ; ld1s r25, r26 ; ctz r5, r6 }
	{ jr r15 ; ld1s r25, r26 ; tblidxb0 r5, r6 }
	{ jr r15 ; ld1u r25, r26 ; mz r5, r6, r7 }
	{ jr r15 ; ld2s r25, r26 ; cmples r5, r6, r7 }
	{ jr r15 ; ld2s r25, r26 ; shrs r5, r6, r7 }
	{ jr r15 ; ld2u r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ jr r15 ; ld4s r25, r26 ; andi r5, r6, 5 }
	{ jr r15 ; ld4s r25, r26 ; shl1addx r5, r6, r7 }
	{ jr r15 ; ld4u r25, r26 ; move r5, r6 }
	{ jr r15 ; ld4u r25, r26 }
	{ jr r15 ; movei r5, 5 ; ld r25, r26 }
	{ jr r15 ; mul_hs_ls r5, r6, r7 }
	{ jr r15 ; mul_ls_ls r5, r6, r7 ; st4 r25, r26 }
	{ jr r15 ; mula_hs_hs r5, r6, r7 }
	{ jr r15 ; mula_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ jr r15 ; mulax r5, r6, r7 ; st2 r25, r26 }
	{ jr r15 ; mz r5, r6, r7 }
	{ jr r15 ; or r5, r6, r7 ; ld1s r25, r26 }
	{ jr r15 ; prefetch r25 ; addx r5, r6, r7 }
	{ jr r15 ; prefetch r25 ; rotli r5, r6, 5 }
	{ jr r15 ; prefetch_l1 r25 ; fsingle_pack1 r5, r6 }
	{ jr r15 ; prefetch_l1 r25 ; tblidxb2 r5, r6 }
	{ jr r15 ; prefetch_l1_fault r25 ; nor r5, r6, r7 }
	{ jr r15 ; prefetch_l2 r25 ; cmplts r5, r6, r7 }
	{ jr r15 ; prefetch_l2 r25 ; shru r5, r6, r7 }
	{ jr r15 ; prefetch_l2_fault r25 ; mula_ls_ls r5, r6, r7 }
	{ jr r15 ; prefetch_l3 r25 ; cmoveqz r5, r6, r7 }
	{ jr r15 ; prefetch_l3 r25 ; shl2addx r5, r6, r7 }
	{ jr r15 ; prefetch_l3_fault r25 ; mul_hs_hs r5, r6, r7 }
	{ jr r15 ; revbits r5, r6 ; ld1s r25, r26 }
	{ jr r15 ; rotl r5, r6, r7 ; ld2s r25, r26 }
	{ jr r15 ; shl r5, r6, r7 ; ld4s r25, r26 }
	{ jr r15 ; shl1addx r5, r6, r7 ; ld4u r25, r26 }
	{ jr r15 ; shl2addx r5, r6, r7 ; prefetch_l1 r25 }
	{ jr r15 ; shl3addx r5, r6, r7 ; prefetch_l2 r25 }
	{ jr r15 ; shrs r5, r6, r7 ; prefetch_l2 r25 }
	{ jr r15 ; shru r5, r6, r7 ; prefetch_l3 r25 }
	{ jr r15 ; st r25, r26 ; cmples r5, r6, r7 }
	{ jr r15 ; st r25, r26 ; shrs r5, r6, r7 }
	{ jr r15 ; st1 r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ jr r15 ; st2 r25, r26 ; andi r5, r6, 5 }
	{ jr r15 ; st2 r25, r26 ; shl1addx r5, r6, r7 }
	{ jr r15 ; st4 r25, r26 ; move r5, r6 }
	{ jr r15 ; st4 r25, r26 }
	{ jr r15 ; tblidxb0 r5, r6 ; ld r25, r26 }
	{ jr r15 ; tblidxb2 r5, r6 ; ld1u r25, r26 }
	{ jr r15 ; v1avgu r5, r6, r7 }
	{ jr r15 ; v1subuc r5, r6, r7 }
	{ jr r15 ; v2shru r5, r6, r7 }
	{ jrp r15 ; add r5, r6, r7 ; ld4s r25, r26 }
	{ jrp r15 ; addx r5, r6, r7 ; ld4u r25, r26 }
	{ jrp r15 ; and r5, r6, r7 ; ld4u r25, r26 }
	{ jrp r15 ; clz r5, r6 ; ld4s r25, r26 }
	{ jrp r15 ; cmovnez r5, r6, r7 ; prefetch r25 }
	{ jrp r15 ; cmpeqi r5, r6, 5 ; prefetch_l1_fault r25 }
	{ jrp r15 ; cmpleu r5, r6, r7 ; prefetch_l2_fault r25 }
	{ jrp r15 ; cmpltsi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ jrp r15 ; cmpne r5, r6, r7 ; st r25, r26 }
	{ jrp r15 ; fdouble_pack1 r5, r6, r7 }
	{ jrp r15 ; fsingle_pack1 r5, r6 ; prefetch_l3 r25 }
	{ jrp r15 ; ld r25, r26 ; cmples r5, r6, r7 }
	{ jrp r15 ; ld r25, r26 ; shrs r5, r6, r7 }
	{ jrp r15 ; ld1s r25, r26 ; mula_hs_hs r5, r6, r7 }
	{ jrp r15 ; ld1u r25, r26 ; andi r5, r6, 5 }
	{ jrp r15 ; ld1u r25, r26 ; shl1addx r5, r6, r7 }
	{ jrp r15 ; ld2s r25, r26 ; move r5, r6 }
	{ jrp r15 ; ld2s r25, r26 }
	{ jrp r15 ; ld2u r25, r26 ; revbits r5, r6 }
	{ jrp r15 ; ld4s r25, r26 ; cmpne r5, r6, r7 }
	{ jrp r15 ; ld4s r25, r26 ; subx r5, r6, r7 }
	{ jrp r15 ; ld4u r25, r26 ; mulx r5, r6, r7 }
	{ jrp r15 ; mnz r5, r6, r7 ; prefetch_l1_fault r25 }
	{ jrp r15 ; movei r5, 5 ; prefetch_l2_fault r25 }
	{ jrp r15 ; mul_hu_hu r5, r6, r7 ; prefetch_l1_fault r25 }
	{ jrp r15 ; mul_lu_lu r5, r6, r7 ; prefetch_l1 r25 }
	{ jrp r15 ; mula_hu_hu r5, r6, r7 ; prefetch r25 }
	{ jrp r15 ; mula_lu_lu r5, r6, r7 ; ld4u r25, r26 }
	{ jrp r15 ; mulx r5, r6, r7 ; prefetch_l1 r25 }
	{ jrp r15 ; nop ; prefetch_l2 r25 }
	{ jrp r15 ; or r5, r6, r7 ; prefetch_l3 r25 }
	{ jrp r15 ; prefetch r25 ; cmplts r5, r6, r7 }
	{ jrp r15 ; prefetch r25 ; shru r5, r6, r7 }
	{ jrp r15 ; prefetch_l1 r25 ; mula_ls_ls r5, r6, r7 }
	{ jrp r15 ; prefetch_l1_fault r25 ; cmoveqz r5, r6, r7 }
	{ jrp r15 ; prefetch_l1_fault r25 ; shl2addx r5, r6, r7 }
	{ jrp r15 ; prefetch_l2 r25 ; mul_hs_hs r5, r6, r7 }
	{ jrp r15 ; prefetch_l2_fault r25 ; addi r5, r6, 5 }
	{ jrp r15 ; prefetch_l2_fault r25 ; rotl r5, r6, r7 }
	{ jrp r15 ; prefetch_l3 r25 ; fnop }
	{ jrp r15 ; prefetch_l3 r25 ; tblidxb1 r5, r6 }
	{ jrp r15 ; prefetch_l3_fault r25 ; nop }
	{ jrp r15 ; revbits r5, r6 ; prefetch_l3 r25 }
	{ jrp r15 ; rotl r5, r6, r7 ; st r25, r26 }
	{ jrp r15 ; shl r5, r6, r7 ; st2 r25, r26 }
	{ jrp r15 ; shl1addx r5, r6, r7 ; st4 r25, r26 }
	{ jrp r15 ; shl3add r5, r6, r7 ; ld r25, r26 }
	{ jrp r15 ; shli r5, r6, 5 ; ld1u r25, r26 }
	{ jrp r15 ; shrsi r5, r6, 5 ; ld1u r25, r26 }
	{ jrp r15 ; shrui r5, r6, 5 ; ld2u r25, r26 }
	{ jrp r15 ; st r25, r26 ; move r5, r6 }
	{ jrp r15 ; st r25, r26 }
	{ jrp r15 ; st1 r25, r26 ; revbits r5, r6 }
	{ jrp r15 ; st2 r25, r26 ; cmpne r5, r6, r7 }
	{ jrp r15 ; st2 r25, r26 ; subx r5, r6, r7 }
	{ jrp r15 ; st4 r25, r26 ; mulx r5, r6, r7 }
	{ jrp r15 ; sub r5, r6, r7 ; prefetch_l2 r25 }
	{ jrp r15 ; tblidxb0 r5, r6 ; prefetch_l2_fault r25 }
	{ jrp r15 ; tblidxb2 r5, r6 ; prefetch_l3_fault r25 }
	{ jrp r15 ; v1ddotpua r5, r6, r7 }
	{ jrp r15 ; v2cmpltsi r5, r6, 5 }
	{ jrp r15 ; v4shrs r5, r6, r7 }
	{ ld r15, r16 ; cmpeqi r5, r6, 5 }
	{ ld r15, r16 ; mm r5, r6, 5, 7 }
	{ ld r15, r16 ; shl1addx r5, r6, r7 }
	{ ld r15, r16 ; v1dotp r5, r6, r7 }
	{ ld r15, r16 ; v2cmpne r5, r6, r7 }
	{ ld r15, r16 ; v4subsc r5, r6, r7 }
	{ ld r25, r26 ; add r15, r16, r17 ; or r5, r6, r7 }
	{ ld r25, r26 ; add r5, r6, r7 ; fnop }
	{ ld r25, r26 ; addi r15, r16, 5 ; cmoveqz r5, r6, r7 }
	{ ld r25, r26 ; addi r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ ld r25, r26 ; addi r5, r6, 5 ; movei r15, 5 }
	{ ld r25, r26 ; addx r15, r16, r17 ; ctz r5, r6 }
	{ ld r25, r26 ; addx r15, r16, r17 ; tblidxb0 r5, r6 }
	{ ld r25, r26 ; addx r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld r25, r26 ; addxi r15, r16, 5 ; mul_lu_lu r5, r6, r7 }
	{ ld r25, r26 ; addxi r5, r6, 5 ; and r15, r16, r17 }
	{ ld r25, r26 ; addxi r5, r6, 5 ; subx r15, r16, r17 }
	{ ld r25, r26 ; and r15, r16, r17 ; or r5, r6, r7 }
	{ ld r25, r26 ; and r5, r6, r7 ; fnop }
	{ ld r25, r26 ; andi r15, r16, 5 ; cmoveqz r5, r6, r7 }
	{ ld r25, r26 ; andi r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ ld r25, r26 ; andi r5, r6, 5 ; movei r15, 5 }
	{ ld r25, r26 ; clz r5, r6 ; jalr r15 }
	{ ld r25, r26 ; cmoveqz r5, r6, r7 ; cmplts r15, r16, r17 }
	{ ld r25, r26 ; cmovnez r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld r25, r26 ; cmovnez r5, r6, r7 ; sub r15, r16, r17 }
	{ ld r25, r26 ; cmpeq r15, r16, r17 ; nor r5, r6, r7 }
	{ ld r25, r26 ; cmpeq r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld r25, r26 ; cmpeqi r15, r16, 5 ; clz r5, r6 }
	{ ld r25, r26 ; cmpeqi r15, r16, 5 ; shl2add r5, r6, r7 }
	{ ld r25, r26 ; cmpeqi r5, r6, 5 ; move r15, r16 }
	{ ld r25, r26 ; cmples r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld r25, r26 ; cmples r15, r16, r17 ; subx r5, r6, r7 }
	{ ld r25, r26 ; cmples r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld r25, r26 ; cmpleu r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld r25, r26 ; cmpleu r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld r25, r26 ; cmpleu r5, r6, r7 ; sub r15, r16, r17 }
	{ ld r25, r26 ; cmplts r15, r16, r17 ; nor r5, r6, r7 }
	{ ld r25, r26 ; cmplts r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld r25, r26 ; cmpltsi r15, r16, 5 ; clz r5, r6 }
	{ ld r25, r26 ; cmpltsi r15, r16, 5 ; shl2add r5, r6, r7 }
	{ ld r25, r26 ; cmpltsi r5, r6, 5 ; move r15, r16 }
	{ ld r25, r26 ; cmpltu r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld r25, r26 ; cmpltu r15, r16, r17 ; subx r5, r6, r7 }
	{ ld r25, r26 ; cmpltu r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld r25, r26 ; cmpne r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld r25, r26 ; cmpne r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld r25, r26 ; cmpne r5, r6, r7 ; sub r15, r16, r17 }
	{ ld r25, r26 ; ctz r5, r6 ; shl3add r15, r16, r17 }
	{ ld r25, r26 ; fnop ; cmpne r15, r16, r17 }
	{ ld r25, r26 ; fnop ; rotli r15, r16, 5 }
	{ ld r25, r26 ; fsingle_pack1 r5, r6 ; addxi r15, r16, 5 }
	{ ld r25, r26 ; fsingle_pack1 r5, r6 ; sub r15, r16, r17 }
	{ ld r25, r26 ; ill ; nor r5, r6, r7 }
	{ ld r25, r26 ; info 19 ; cmoveqz r5, r6, r7 }
	{ ld r25, r26 ; info 19 ; mula_ls_ls r5, r6, r7 }
	{ ld r25, r26 ; info 19 ; shrui r15, r16, 5 }
	{ ld r25, r26 ; jalr r15 ; mul_lu_lu r5, r6, r7 }
	{ ld r25, r26 ; jalrp r15 ; and r5, r6, r7 }
	{ ld r25, r26 ; jalrp r15 ; shl1add r5, r6, r7 }
	{ ld r25, r26 ; jr r15 ; mnz r5, r6, r7 }
	{ ld r25, r26 ; jr r15 ; xor r5, r6, r7 }
	{ ld r25, r26 ; jrp r15 ; pcnt r5, r6 }
	{ ld r25, r26 ; lnk r15 ; cmpltu r5, r6, r7 }
	{ ld r25, r26 ; lnk r15 ; sub r5, r6, r7 }
	{ ld r25, r26 ; mnz r15, r16, r17 ; mulax r5, r6, r7 }
	{ ld r25, r26 ; mnz r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ ld r25, r26 ; move r15, r16 ; addx r5, r6, r7 }
	{ ld r25, r26 ; move r15, r16 ; rotli r5, r6, 5 }
	{ ld r25, r26 ; move r5, r6 ; jr r15 }
	{ ld r25, r26 ; movei r15, 5 ; cmpleu r5, r6, r7 }
	{ ld r25, r26 ; movei r15, 5 ; shrsi r5, r6, 5 }
	{ ld r25, r26 ; movei r5, 5 ; rotl r15, r16, r17 }
	{ ld r25, r26 ; mul_hs_hs r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld r25, r26 ; mul_hu_hu r5, r6, r7 ; ill }
	{ ld r25, r26 ; mul_ls_ls r5, r6, r7 ; cmples r15, r16, r17 }
	{ ld r25, r26 ; mul_lu_lu r5, r6, r7 ; addi r15, r16, 5 }
	{ ld r25, r26 ; mul_lu_lu r5, r6, r7 ; shru r15, r16, r17 }
	{ ld r25, r26 ; mula_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld r25, r26 ; mula_hu_hu r5, r6, r7 ; nor r15, r16, r17 }
	{ ld r25, r26 ; mula_ls_ls r5, r6, r7 ; jrp r15 }
	{ ld r25, r26 ; mula_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld r25, r26 ; mulax r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ ld r25, r26 ; mulax r5, r6, r7 }
	{ ld r25, r26 ; mulx r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld r25, r26 ; mz r15, r16, r17 ; mulax r5, r6, r7 }
	{ ld r25, r26 ; mz r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ ld r25, r26 ; nop ; addi r15, r16, 5 }
	{ ld r25, r26 ; nop ; mnz r5, r6, r7 }
	{ ld r25, r26 ; nop ; shl3add r5, r6, r7 }
	{ ld r25, r26 ; nor r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld r25, r26 ; nor r15, r16, r17 ; subx r5, r6, r7 }
	{ ld r25, r26 ; nor r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld r25, r26 ; or r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld r25, r26 ; or r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld r25, r26 ; or r5, r6, r7 ; sub r15, r16, r17 }
	{ ld r25, r26 ; pcnt r5, r6 ; shl3add r15, r16, r17 }
	{ ld r25, r26 ; revbits r5, r6 ; rotl r15, r16, r17 }
	{ ld r25, r26 ; revbytes r5, r6 ; mnz r15, r16, r17 }
	{ ld r25, r26 ; rotl r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld r25, r26 ; rotl r15, r16, r17 ; sub r5, r6, r7 }
	{ ld r25, r26 ; rotl r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld r25, r26 ; rotli r15, r16, 5 ; mul_hu_hu r5, r6, r7 }
	{ ld r25, r26 ; rotli r5, r6, 5 ; addx r15, r16, r17 }
	{ ld r25, r26 ; rotli r5, r6, 5 ; shrui r15, r16, 5 }
	{ ld r25, r26 ; shl r15, r16, r17 ; nop }
	{ ld r25, r26 ; shl r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ ld r25, r26 ; shl1add r15, r16, r17 ; andi r5, r6, 5 }
	{ ld r25, r26 ; shl1add r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ ld r25, r26 ; shl1add r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld r25, r26 ; shl1addx r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld r25, r26 ; shl1addx r15, r16, r17 ; sub r5, r6, r7 }
	{ ld r25, r26 ; shl1addx r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld r25, r26 ; shl2add r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ ld r25, r26 ; shl2add r5, r6, r7 ; addx r15, r16, r17 }
	{ ld r25, r26 ; shl2add r5, r6, r7 ; shrui r15, r16, 5 }
	{ ld r25, r26 ; shl2addx r15, r16, r17 ; nop }
	{ ld r25, r26 ; shl2addx r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ ld r25, r26 ; shl3add r15, r16, r17 ; andi r5, r6, 5 }
	{ ld r25, r26 ; shl3add r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ ld r25, r26 ; shl3add r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld r25, r26 ; shl3addx r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld r25, r26 ; shl3addx r15, r16, r17 ; sub r5, r6, r7 }
	{ ld r25, r26 ; shl3addx r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld r25, r26 ; shli r15, r16, 5 ; mul_hu_hu r5, r6, r7 }
	{ ld r25, r26 ; shli r5, r6, 5 ; addx r15, r16, r17 }
	{ ld r25, r26 ; shli r5, r6, 5 ; shrui r15, r16, 5 }
	{ ld r25, r26 ; shrs r15, r16, r17 ; nop }
	{ ld r25, r26 ; shrs r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ ld r25, r26 ; shrsi r15, r16, 5 ; andi r5, r6, 5 }
	{ ld r25, r26 ; shrsi r15, r16, 5 ; shl1addx r5, r6, r7 }
	{ ld r25, r26 ; shrsi r5, r6, 5 ; mnz r15, r16, r17 }
	{ ld r25, r26 ; shru r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld r25, r26 ; shru r15, r16, r17 ; sub r5, r6, r7 }
	{ ld r25, r26 ; shru r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld r25, r26 ; shrui r15, r16, 5 ; mul_hu_hu r5, r6, r7 }
	{ ld r25, r26 ; shrui r5, r6, 5 ; addx r15, r16, r17 }
	{ ld r25, r26 ; shrui r5, r6, 5 ; shrui r15, r16, 5 }
	{ ld r25, r26 ; sub r15, r16, r17 ; nop }
	{ ld r25, r26 ; sub r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ ld r25, r26 ; subx r15, r16, r17 ; andi r5, r6, 5 }
	{ ld r25, r26 ; subx r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ ld r25, r26 ; subx r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld r25, r26 ; tblidxb0 r5, r6 ; ill }
	{ ld r25, r26 ; tblidxb1 r5, r6 ; cmples r15, r16, r17 }
	{ ld r25, r26 ; tblidxb2 r5, r6 ; addi r15, r16, 5 }
	{ ld r25, r26 ; tblidxb2 r5, r6 ; shru r15, r16, r17 }
	{ ld r25, r26 ; tblidxb3 r5, r6 ; shl2add r15, r16, r17 }
	{ ld r25, r26 ; xor r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ ld r25, r26 ; xor r5, r6, r7 ; and r15, r16, r17 }
	{ ld r25, r26 ; xor r5, r6, r7 ; subx r15, r16, r17 }
	{ ld1s r15, r16 ; dblalign6 r5, r6, r7 }
	{ ld1s r15, r16 ; mula_hu_lu r5, r6, r7 }
	{ ld1s r15, r16 ; tblidxb3 r5, r6 }
	{ ld1s r15, r16 ; v1shrs r5, r6, r7 }
	{ ld1s r15, r16 ; v2shl r5, r6, r7 }
	{ ld1s r25, r26 ; add r15, r16, r17 ; fnop }
	{ ld1s r25, r26 ; add r15, r16, r17 ; tblidxb1 r5, r6 }
	{ ld1s r25, r26 ; add r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld1s r25, r26 ; addi r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ ld1s r25, r26 ; addi r5, r6, 5 ; andi r15, r16, 5 }
	{ ld1s r25, r26 ; addi r5, r6, 5 ; xor r15, r16, r17 }
	{ ld1s r25, r26 ; addx r15, r16, r17 ; pcnt r5, r6 }
	{ ld1s r25, r26 ; addx r5, r6, r7 ; ill }
	{ ld1s r25, r26 ; addxi r15, r16, 5 ; cmovnez r5, r6, r7 }
	{ ld1s r25, r26 ; addxi r15, r16, 5 ; shl3add r5, r6, r7 }
	{ ld1s r25, r26 ; addxi r5, r6, 5 ; mz r15, r16, r17 }
	{ ld1s r25, r26 ; and r15, r16, r17 ; fnop }
	{ ld1s r25, r26 ; and r15, r16, r17 ; tblidxb1 r5, r6 }
	{ ld1s r25, r26 ; and r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld1s r25, r26 ; andi r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ ld1s r25, r26 ; andi r5, r6, 5 ; andi r15, r16, 5 }
	{ ld1s r25, r26 ; andi r5, r6, 5 ; xor r15, r16, r17 }
	{ ld1s r25, r26 ; clz r5, r6 ; shli r15, r16, 5 }
	{ ld1s r25, r26 ; cmoveqz r5, r6, r7 ; shl r15, r16, r17 }
	{ ld1s r25, r26 ; cmovnez r5, r6, r7 ; movei r15, 5 }
	{ ld1s r25, r26 ; cmpeq r15, r16, r17 ; ctz r5, r6 }
	{ ld1s r25, r26 ; cmpeq r15, r16, r17 ; tblidxb0 r5, r6 }
	{ ld1s r25, r26 ; cmpeq r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld1s r25, r26 ; cmpeqi r15, r16, 5 ; mul_lu_lu r5, r6, r7 }
	{ ld1s r25, r26 ; cmpeqi r5, r6, 5 ; and r15, r16, r17 }
	{ ld1s r25, r26 ; cmpeqi r5, r6, 5 ; subx r15, r16, r17 }
	{ ld1s r25, r26 ; cmples r15, r16, r17 ; or r5, r6, r7 }
	{ ld1s r25, r26 ; cmples r5, r6, r7 ; fnop }
	{ ld1s r25, r26 ; cmpleu r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ ld1s r25, r26 ; cmpleu r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ ld1s r25, r26 ; cmpleu r5, r6, r7 ; movei r15, 5 }
	{ ld1s r25, r26 ; cmplts r15, r16, r17 ; ctz r5, r6 }
	{ ld1s r25, r26 ; cmplts r15, r16, r17 ; tblidxb0 r5, r6 }
	{ ld1s r25, r26 ; cmplts r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld1s r25, r26 ; cmpltsi r15, r16, 5 ; mul_lu_lu r5, r6, r7 }
	{ ld1s r25, r26 ; cmpltsi r5, r6, 5 ; and r15, r16, r17 }
	{ ld1s r25, r26 ; cmpltsi r5, r6, 5 ; subx r15, r16, r17 }
	{ ld1s r25, r26 ; cmpltu r15, r16, r17 ; or r5, r6, r7 }
	{ ld1s r25, r26 ; cmpltu r5, r6, r7 ; fnop }
	{ ld1s r25, r26 ; cmpne r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ ld1s r25, r26 ; cmpne r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ ld1s r25, r26 ; cmpne r5, r6, r7 ; movei r15, 5 }
	{ ld1s r25, r26 ; ctz r5, r6 ; jalr r15 }
	{ ld1s r25, r26 ; fnop ; andi r15, r16, 5 }
	{ ld1s r25, r26 ; fnop ; mul_lu_lu r5, r6, r7 }
	{ ld1s r25, r26 ; fnop ; shrsi r5, r6, 5 }
	{ ld1s r25, r26 ; fsingle_pack1 r5, r6 ; movei r15, 5 }
	{ ld1s r25, r26 ; ill ; ctz r5, r6 }
	{ ld1s r25, r26 ; ill ; tblidxb0 r5, r6 }
	{ ld1s r25, r26 ; info 19 ; ill }
	{ ld1s r25, r26 ; info 19 ; shl1add r5, r6, r7 }
	{ ld1s r25, r26 ; jalr r15 ; cmovnez r5, r6, r7 }
	{ ld1s r25, r26 ; jalr r15 ; shl3add r5, r6, r7 }
	{ ld1s r25, r26 ; jalrp r15 ; mul_hu_hu r5, r6, r7 }
	{ ld1s r25, r26 ; jr r15 ; addx r5, r6, r7 }
	{ ld1s r25, r26 ; jr r15 ; rotli r5, r6, 5 }
	{ ld1s r25, r26 ; jrp r15 ; fsingle_pack1 r5, r6 }
	{ ld1s r25, r26 ; jrp r15 ; tblidxb2 r5, r6 }
	{ ld1s r25, r26 ; lnk r15 ; nor r5, r6, r7 }
	{ ld1s r25, r26 ; mnz r15, r16, r17 ; cmplts r5, r6, r7 }
	{ ld1s r25, r26 ; mnz r15, r16, r17 ; shru r5, r6, r7 }
	{ ld1s r25, r26 ; mnz r5, r6, r7 ; rotli r15, r16, 5 }
	{ ld1s r25, r26 ; move r15, r16 ; movei r5, 5 }
	{ ld1s r25, r26 ; move r5, r6 ; add r15, r16, r17 }
	{ ld1s r25, r26 ; move r5, r6 ; shrsi r15, r16, 5 }
	{ ld1s r25, r26 ; movei r15, 5 ; mulx r5, r6, r7 }
	{ ld1s r25, r26 ; movei r5, 5 ; cmplts r15, r16, r17 }
	{ ld1s r25, r26 ; mul_hs_hs r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld1s r25, r26 ; mul_hs_hs r5, r6, r7 ; sub r15, r16, r17 }
	{ ld1s r25, r26 ; mul_hu_hu r5, r6, r7 ; shl3add r15, r16, r17 }
	{ ld1s r25, r26 ; mul_ls_ls r5, r6, r7 ; rotl r15, r16, r17 }
	{ ld1s r25, r26 ; mul_lu_lu r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld1s r25, r26 ; mula_hs_hs r5, r6, r7 ; ill }
	{ ld1s r25, r26 ; mula_hu_hu r5, r6, r7 ; cmples r15, r16, r17 }
	{ ld1s r25, r26 ; mula_ls_ls r5, r6, r7 ; addi r15, r16, 5 }
	{ ld1s r25, r26 ; mula_ls_ls r5, r6, r7 ; shru r15, r16, r17 }
	{ ld1s r25, r26 ; mula_lu_lu r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld1s r25, r26 ; mulax r5, r6, r7 ; nor r15, r16, r17 }
	{ ld1s r25, r26 ; mulx r5, r6, r7 ; jrp r15 }
	{ ld1s r25, r26 ; mz r15, r16, r17 ; cmplts r5, r6, r7 }
	{ ld1s r25, r26 ; mz r15, r16, r17 ; shru r5, r6, r7 }
	{ ld1s r25, r26 ; mz r5, r6, r7 ; rotli r15, r16, 5 }
	{ ld1s r25, r26 ; nop ; cmplts r15, r16, r17 }
	{ ld1s r25, r26 ; nop ; or r5, r6, r7 }
	{ ld1s r25, r26 ; nop ; xor r15, r16, r17 }
	{ ld1s r25, r26 ; nor r15, r16, r17 ; or r5, r6, r7 }
	{ ld1s r25, r26 ; nor r5, r6, r7 ; fnop }
	{ ld1s r25, r26 ; or r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ ld1s r25, r26 ; or r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ ld1s r25, r26 ; or r5, r6, r7 ; movei r15, 5 }
	{ ld1s r25, r26 ; pcnt r5, r6 ; jalr r15 }
	{ ld1s r25, r26 ; revbits r5, r6 ; cmplts r15, r16, r17 }
	{ ld1s r25, r26 ; revbytes r5, r6 ; addxi r15, r16, 5 }
	{ ld1s r25, r26 ; revbytes r5, r6 ; sub r15, r16, r17 }
	{ ld1s r25, r26 ; rotl r15, r16, r17 ; nor r5, r6, r7 }
	{ ld1s r25, r26 ; rotl r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld1s r25, r26 ; rotli r15, r16, 5 ; clz r5, r6 }
	{ ld1s r25, r26 ; rotli r15, r16, 5 ; shl2add r5, r6, r7 }
	{ ld1s r25, r26 ; rotli r5, r6, 5 ; move r15, r16 }
	{ ld1s r25, r26 ; shl r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld1s r25, r26 ; shl r15, r16, r17 ; subx r5, r6, r7 }
	{ ld1s r25, r26 ; shl r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld1s r25, r26 ; shl1add r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld1s r25, r26 ; shl1add r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld1s r25, r26 ; shl1add r5, r6, r7 ; sub r15, r16, r17 }
	{ ld1s r25, r26 ; shl1addx r15, r16, r17 ; nor r5, r6, r7 }
	{ ld1s r25, r26 ; shl1addx r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld1s r25, r26 ; shl2add r15, r16, r17 ; clz r5, r6 }
	{ ld1s r25, r26 ; shl2add r15, r16, r17 ; shl2add r5, r6, r7 }
	{ ld1s r25, r26 ; shl2add r5, r6, r7 ; move r15, r16 }
	{ ld1s r25, r26 ; shl2addx r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld1s r25, r26 ; shl2addx r15, r16, r17 ; subx r5, r6, r7 }
	{ ld1s r25, r26 ; shl2addx r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld1s r25, r26 ; shl3add r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld1s r25, r26 ; shl3add r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld1s r25, r26 ; shl3add r5, r6, r7 ; sub r15, r16, r17 }
	{ ld1s r25, r26 ; shl3addx r15, r16, r17 ; nor r5, r6, r7 }
	{ ld1s r25, r26 ; shl3addx r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld1s r25, r26 ; shli r15, r16, 5 ; clz r5, r6 }
	{ ld1s r25, r26 ; shli r15, r16, 5 ; shl2add r5, r6, r7 }
	{ ld1s r25, r26 ; shli r5, r6, 5 ; move r15, r16 }
	{ ld1s r25, r26 ; shrs r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld1s r25, r26 ; shrs r15, r16, r17 ; subx r5, r6, r7 }
	{ ld1s r25, r26 ; shrs r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld1s r25, r26 ; shrsi r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ ld1s r25, r26 ; shrsi r5, r6, 5 ; addxi r15, r16, 5 }
	{ ld1s r25, r26 ; shrsi r5, r6, 5 ; sub r15, r16, r17 }
	{ ld1s r25, r26 ; shru r15, r16, r17 ; nor r5, r6, r7 }
	{ ld1s r25, r26 ; shru r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld1s r25, r26 ; shrui r15, r16, 5 ; clz r5, r6 }
	{ ld1s r25, r26 ; shrui r15, r16, 5 ; shl2add r5, r6, r7 }
	{ ld1s r25, r26 ; shrui r5, r6, 5 ; move r15, r16 }
	{ ld1s r25, r26 ; sub r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld1s r25, r26 ; sub r15, r16, r17 ; subx r5, r6, r7 }
	{ ld1s r25, r26 ; sub r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld1s r25, r26 ; subx r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld1s r25, r26 ; subx r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld1s r25, r26 ; subx r5, r6, r7 ; sub r15, r16, r17 }
	{ ld1s r25, r26 ; tblidxb0 r5, r6 ; shl3add r15, r16, r17 }
	{ ld1s r25, r26 ; tblidxb1 r5, r6 ; rotl r15, r16, r17 }
	{ ld1s r25, r26 ; tblidxb2 r5, r6 ; mnz r15, r16, r17 }
	{ ld1s r25, r26 ; tblidxb3 r5, r6 ; ill }
	{ ld1s r25, r26 ; xor r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ ld1s r25, r26 ; xor r15, r16, r17 ; shl3add r5, r6, r7 }
	{ ld1s r25, r26 ; xor r5, r6, r7 ; mz r15, r16, r17 }
	{ ld1s_add r15, r16, 5 ; cmpleu r5, r6, r7 }
	{ ld1s_add r15, r16, 5 ; move r5, r6 }
	{ ld1s_add r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ ld1s_add r15, r16, 5 ; v1dotpu r5, r6, r7 }
	{ ld1s_add r15, r16, 5 ; v2dotpa r5, r6, r7 }
	{ ld1s_add r15, r16, 5 ; xori r5, r6, 5 }
	{ ld1u r15, r16 ; fdouble_addsub r5, r6, r7 }
	{ ld1u r15, r16 ; mula_ls_lu r5, r6, r7 }
	{ ld1u r15, r16 ; v1addi r5, r6, 5 }
	{ ld1u r15, r16 ; v1shru r5, r6, r7 }
	{ ld1u r15, r16 ; v2shlsc r5, r6, r7 }
	{ ld1u r25, r26 ; add r15, r16, r17 ; info 19 }
	{ ld1u r25, r26 ; add r15, r16, r17 ; tblidxb3 r5, r6 }
	{ ld1u r25, r26 ; add r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld1u r25, r26 ; addi r15, r16, 5 ; mula_ls_ls r5, r6, r7 }
	{ ld1u r25, r26 ; addi r5, r6, 5 ; cmpeqi r15, r16, 5 }
	{ ld1u r25, r26 ; addx r15, r16, r17 ; add r5, r6, r7 }
	{ ld1u r25, r26 ; addx r15, r16, r17 ; revbytes r5, r6 }
	{ ld1u r25, r26 ; addx r5, r6, r7 ; jalr r15 }
	{ ld1u r25, r26 ; addxi r15, r16, 5 ; cmpeqi r5, r6, 5 }
	{ ld1u r25, r26 ; addxi r15, r16, 5 ; shli r5, r6, 5 }
	{ ld1u r25, r26 ; addxi r5, r6, 5 ; nor r15, r16, r17 }
	{ ld1u r25, r26 ; and r15, r16, r17 ; info 19 }
	{ ld1u r25, r26 ; and r15, r16, r17 ; tblidxb3 r5, r6 }
	{ ld1u r25, r26 ; and r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld1u r25, r26 ; andi r15, r16, 5 ; mula_ls_ls r5, r6, r7 }
	{ ld1u r25, r26 ; andi r5, r6, 5 ; cmpeqi r15, r16, 5 }
	{ ld1u r25, r26 ; clz r5, r6 ; add r15, r16, r17 }
	{ ld1u r25, r26 ; clz r5, r6 ; shrsi r15, r16, 5 }
	{ ld1u r25, r26 ; cmoveqz r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld1u r25, r26 ; cmovnez r5, r6, r7 ; nop }
	{ ld1u r25, r26 ; cmpeq r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ ld1u r25, r26 ; cmpeq r15, r16, r17 ; tblidxb2 r5, r6 }
	{ ld1u r25, r26 ; cmpeq r5, r6, r7 ; shl3add r15, r16, r17 }
	{ ld1u r25, r26 ; cmpeqi r15, r16, 5 ; mula_hu_hu r5, r6, r7 }
	{ ld1u r25, r26 ; cmpeqi r5, r6, 5 ; cmpeq r15, r16, r17 }
	{ ld1u r25, r26 ; cmpeqi r5, r6, 5 }
	{ ld1u r25, r26 ; cmples r15, r16, r17 ; revbits r5, r6 }
	{ ld1u r25, r26 ; cmples r5, r6, r7 ; info 19 }
	{ ld1u r25, r26 ; cmpleu r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ ld1u r25, r26 ; cmpleu r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ ld1u r25, r26 ; cmpleu r5, r6, r7 ; nop }
	{ ld1u r25, r26 ; cmplts r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ ld1u r25, r26 ; cmplts r15, r16, r17 ; tblidxb2 r5, r6 }
	{ ld1u r25, r26 ; cmplts r5, r6, r7 ; shl3add r15, r16, r17 }
	{ ld1u r25, r26 ; cmpltsi r15, r16, 5 ; mula_hu_hu r5, r6, r7 }
	{ ld1u r25, r26 ; cmpltsi r5, r6, 5 ; cmpeq r15, r16, r17 }
	{ ld1u r25, r26 ; cmpltsi r5, r6, 5 }
	{ ld1u r25, r26 ; cmpltu r15, r16, r17 ; revbits r5, r6 }
	{ ld1u r25, r26 ; cmpltu r5, r6, r7 ; info 19 }
	{ ld1u r25, r26 ; cmpne r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ ld1u r25, r26 ; cmpne r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ ld1u r25, r26 ; cmpne r5, r6, r7 ; nop }
	{ ld1u r25, r26 ; ctz r5, r6 ; jr r15 }
	{ ld1u r25, r26 ; fnop ; clz r5, r6 }
	{ ld1u r25, r26 ; fnop ; mula_hu_hu r5, r6, r7 }
	{ ld1u r25, r26 ; fnop ; shru r5, r6, r7 }
	{ ld1u r25, r26 ; fsingle_pack1 r5, r6 ; nop }
	{ ld1u r25, r26 ; ill ; fsingle_pack1 r5, r6 }
	{ ld1u r25, r26 ; ill ; tblidxb2 r5, r6 }
	{ ld1u r25, r26 ; info 19 ; jalr r15 }
	{ ld1u r25, r26 ; info 19 ; shl1addx r5, r6, r7 }
	{ ld1u r25, r26 ; jalr r15 ; cmpeqi r5, r6, 5 }
	{ ld1u r25, r26 ; jalr r15 ; shli r5, r6, 5 }
	{ ld1u r25, r26 ; jalrp r15 ; mul_lu_lu r5, r6, r7 }
	{ ld1u r25, r26 ; jr r15 ; and r5, r6, r7 }
	{ ld1u r25, r26 ; jr r15 ; shl1add r5, r6, r7 }
	{ ld1u r25, r26 ; jrp r15 ; mnz r5, r6, r7 }
	{ ld1u r25, r26 ; jrp r15 ; xor r5, r6, r7 }
	{ ld1u r25, r26 ; lnk r15 ; pcnt r5, r6 }
	{ ld1u r25, r26 ; mnz r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld1u r25, r26 ; mnz r15, r16, r17 ; sub r5, r6, r7 }
	{ ld1u r25, r26 ; mnz r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld1u r25, r26 ; move r15, r16 ; mul_hu_hu r5, r6, r7 }
	{ ld1u r25, r26 ; move r5, r6 ; addx r15, r16, r17 }
	{ ld1u r25, r26 ; move r5, r6 ; shrui r15, r16, 5 }
	{ ld1u r25, r26 ; movei r15, 5 ; nop }
	{ ld1u r25, r26 ; movei r5, 5 ; cmpltu r15, r16, r17 }
	{ ld1u r25, r26 ; mul_hs_hs r5, r6, r7 ; andi r15, r16, 5 }
	{ ld1u r25, r26 ; mul_hs_hs r5, r6, r7 ; xor r15, r16, r17 }
	{ ld1u r25, r26 ; mul_hu_hu r5, r6, r7 ; shli r15, r16, 5 }
	{ ld1u r25, r26 ; mul_ls_ls r5, r6, r7 ; shl r15, r16, r17 }
	{ ld1u r25, r26 ; mul_lu_lu r5, r6, r7 ; movei r15, 5 }
	{ ld1u r25, r26 ; mula_hs_hs r5, r6, r7 ; jalr r15 }
	{ ld1u r25, r26 ; mula_hu_hu r5, r6, r7 ; cmplts r15, r16, r17 }
	{ ld1u r25, r26 ; mula_ls_ls r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld1u r25, r26 ; mula_ls_ls r5, r6, r7 ; sub r15, r16, r17 }
	{ ld1u r25, r26 ; mula_lu_lu r5, r6, r7 ; shl3add r15, r16, r17 }
	{ ld1u r25, r26 ; mulax r5, r6, r7 ; rotl r15, r16, r17 }
	{ ld1u r25, r26 ; mulx r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld1u r25, r26 ; mz r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld1u r25, r26 ; mz r15, r16, r17 ; sub r5, r6, r7 }
	{ ld1u r25, r26 ; mz r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld1u r25, r26 ; nop ; cmpltsi r15, r16, 5 }
	{ ld1u r25, r26 ; nop ; revbits r5, r6 }
	{ ld1u r25, r26 ; nop }
	{ ld1u r25, r26 ; nor r15, r16, r17 ; revbits r5, r6 }
	{ ld1u r25, r26 ; nor r5, r6, r7 ; info 19 }
	{ ld1u r25, r26 ; or r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ ld1u r25, r26 ; or r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ ld1u r25, r26 ; or r5, r6, r7 ; nop }
	{ ld1u r25, r26 ; pcnt r5, r6 ; jr r15 }
	{ ld1u r25, r26 ; revbits r5, r6 ; cmpltu r15, r16, r17 }
	{ ld1u r25, r26 ; revbytes r5, r6 ; andi r15, r16, 5 }
	{ ld1u r25, r26 ; revbytes r5, r6 ; xor r15, r16, r17 }
	{ ld1u r25, r26 ; rotl r15, r16, r17 ; pcnt r5, r6 }
	{ ld1u r25, r26 ; rotl r5, r6, r7 ; ill }
	{ ld1u r25, r26 ; rotli r15, r16, 5 ; cmovnez r5, r6, r7 }
	{ ld1u r25, r26 ; rotli r15, r16, 5 ; shl3add r5, r6, r7 }
	{ ld1u r25, r26 ; rotli r5, r6, 5 ; mz r15, r16, r17 }
	{ ld1u r25, r26 ; shl r15, r16, r17 ; fnop }
	{ ld1u r25, r26 ; shl r15, r16, r17 ; tblidxb1 r5, r6 }
	{ ld1u r25, r26 ; shl r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld1u r25, r26 ; shl1add r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ ld1u r25, r26 ; shl1add r5, r6, r7 ; andi r15, r16, 5 }
	{ ld1u r25, r26 ; shl1add r5, r6, r7 ; xor r15, r16, r17 }
	{ ld1u r25, r26 ; shl1addx r15, r16, r17 ; pcnt r5, r6 }
	{ ld1u r25, r26 ; shl1addx r5, r6, r7 ; ill }
	{ ld1u r25, r26 ; shl2add r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ ld1u r25, r26 ; shl2add r15, r16, r17 ; shl3add r5, r6, r7 }
	{ ld1u r25, r26 ; shl2add r5, r6, r7 ; mz r15, r16, r17 }
	{ ld1u r25, r26 ; shl2addx r15, r16, r17 ; fnop }
	{ ld1u r25, r26 ; shl2addx r15, r16, r17 ; tblidxb1 r5, r6 }
	{ ld1u r25, r26 ; shl2addx r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld1u r25, r26 ; shl3add r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ ld1u r25, r26 ; shl3add r5, r6, r7 ; andi r15, r16, 5 }
	{ ld1u r25, r26 ; shl3add r5, r6, r7 ; xor r15, r16, r17 }
	{ ld1u r25, r26 ; shl3addx r15, r16, r17 ; pcnt r5, r6 }
	{ ld1u r25, r26 ; shl3addx r5, r6, r7 ; ill }
	{ ld1u r25, r26 ; shli r15, r16, 5 ; cmovnez r5, r6, r7 }
	{ ld1u r25, r26 ; shli r15, r16, 5 ; shl3add r5, r6, r7 }
	{ ld1u r25, r26 ; shli r5, r6, 5 ; mz r15, r16, r17 }
	{ ld1u r25, r26 ; shrs r15, r16, r17 ; fnop }
	{ ld1u r25, r26 ; shrs r15, r16, r17 ; tblidxb1 r5, r6 }
	{ ld1u r25, r26 ; shrs r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld1u r25, r26 ; shrsi r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ ld1u r25, r26 ; shrsi r5, r6, 5 ; andi r15, r16, 5 }
	{ ld1u r25, r26 ; shrsi r5, r6, 5 ; xor r15, r16, r17 }
	{ ld1u r25, r26 ; shru r15, r16, r17 ; pcnt r5, r6 }
	{ ld1u r25, r26 ; shru r5, r6, r7 ; ill }
	{ ld1u r25, r26 ; shrui r15, r16, 5 ; cmovnez r5, r6, r7 }
	{ ld1u r25, r26 ; shrui r15, r16, 5 ; shl3add r5, r6, r7 }
	{ ld1u r25, r26 ; shrui r5, r6, 5 ; mz r15, r16, r17 }
	{ ld1u r25, r26 ; sub r15, r16, r17 ; fnop }
	{ ld1u r25, r26 ; sub r15, r16, r17 ; tblidxb1 r5, r6 }
	{ ld1u r25, r26 ; sub r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld1u r25, r26 ; subx r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ ld1u r25, r26 ; subx r5, r6, r7 ; andi r15, r16, 5 }
	{ ld1u r25, r26 ; subx r5, r6, r7 ; xor r15, r16, r17 }
	{ ld1u r25, r26 ; tblidxb0 r5, r6 ; shli r15, r16, 5 }
	{ ld1u r25, r26 ; tblidxb1 r5, r6 ; shl r15, r16, r17 }
	{ ld1u r25, r26 ; tblidxb2 r5, r6 ; movei r15, 5 }
	{ ld1u r25, r26 ; tblidxb3 r5, r6 ; jalr r15 }
	{ ld1u r25, r26 ; xor r15, r16, r17 ; cmpeqi r5, r6, 5 }
	{ ld1u r25, r26 ; xor r15, r16, r17 ; shli r5, r6, 5 }
	{ ld1u r25, r26 ; xor r5, r6, r7 ; nor r15, r16, r17 }
	{ ld1u_add r15, r16, 5 ; cmpltsi r5, r6, 5 }
	{ ld1u_add r15, r16, 5 ; moveli r5, 0x1234 }
	{ ld1u_add r15, r16, 5 ; shl3addx r5, r6, r7 }
	{ ld1u_add r15, r16, 5 ; v1dotpus r5, r6, r7 }
	{ ld1u_add r15, r16, 5 ; v2int_l r5, r6, r7 }
	{ ld2s r15, r16 ; addi r5, r6, 5 }
	{ ld2s r15, r16 ; fdouble_pack1 r5, r6, r7 }
	{ ld2s r15, r16 ; mulax r5, r6, r7 }
	{ ld2s r15, r16 ; v1adiffu r5, r6, r7 }
	{ ld2s r15, r16 ; v1sub r5, r6, r7 }
	{ ld2s r15, r16 ; v2shrsi r5, r6, 5 }
	{ ld2s r25, r26 ; add r15, r16, r17 ; move r5, r6 }
	{ ld2s r25, r26 ; add r15, r16, r17 }
	{ ld2s r25, r26 ; add r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld2s r25, r26 ; addi r15, r16, 5 ; mulax r5, r6, r7 }
	{ ld2s r25, r26 ; addi r5, r6, 5 ; cmpleu r15, r16, r17 }
	{ ld2s r25, r26 ; addx r15, r16, r17 ; addx r5, r6, r7 }
	{ ld2s r25, r26 ; addx r15, r16, r17 ; rotli r5, r6, 5 }
	{ ld2s r25, r26 ; addx r5, r6, r7 ; jr r15 }
	{ ld2s r25, r26 ; addxi r15, r16, 5 ; cmpleu r5, r6, r7 }
	{ ld2s r25, r26 ; addxi r15, r16, 5 ; shrsi r5, r6, 5 }
	{ ld2s r25, r26 ; addxi r5, r6, 5 ; rotl r15, r16, r17 }
	{ ld2s r25, r26 ; and r15, r16, r17 ; move r5, r6 }
	{ ld2s r25, r26 ; and r15, r16, r17 }
	{ ld2s r25, r26 ; and r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld2s r25, r26 ; andi r15, r16, 5 ; mulax r5, r6, r7 }
	{ ld2s r25, r26 ; andi r5, r6, 5 ; cmpleu r15, r16, r17 }
	{ ld2s r25, r26 ; clz r5, r6 ; addx r15, r16, r17 }
	{ ld2s r25, r26 ; clz r5, r6 ; shrui r15, r16, 5 }
	{ ld2s r25, r26 ; cmoveqz r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld2s r25, r26 ; cmovnez r5, r6, r7 ; or r15, r16, r17 }
	{ ld2s r25, r26 ; cmpeq r15, r16, r17 ; mnz r5, r6, r7 }
	{ ld2s r25, r26 ; cmpeq r15, r16, r17 ; xor r5, r6, r7 }
	{ ld2s r25, r26 ; cmpeq r5, r6, r7 ; shli r15, r16, 5 }
	{ ld2s r25, r26 ; cmpeqi r15, r16, 5 ; mula_lu_lu r5, r6, r7 }
	{ ld2s r25, r26 ; cmpeqi r5, r6, 5 ; cmples r15, r16, r17 }
	{ ld2s r25, r26 ; cmples r15, r16, r17 ; addi r5, r6, 5 }
	{ ld2s r25, r26 ; cmples r15, r16, r17 ; rotl r5, r6, r7 }
	{ ld2s r25, r26 ; cmples r5, r6, r7 ; jalrp r15 }
	{ ld2s r25, r26 ; cmpleu r15, r16, r17 ; cmples r5, r6, r7 }
	{ ld2s r25, r26 ; cmpleu r15, r16, r17 ; shrs r5, r6, r7 }
	{ ld2s r25, r26 ; cmpleu r5, r6, r7 ; or r15, r16, r17 }
	{ ld2s r25, r26 ; cmplts r15, r16, r17 ; mnz r5, r6, r7 }
	{ ld2s r25, r26 ; cmplts r15, r16, r17 ; xor r5, r6, r7 }
	{ ld2s r25, r26 ; cmplts r5, r6, r7 ; shli r15, r16, 5 }
	{ ld2s r25, r26 ; cmpltsi r15, r16, 5 ; mula_lu_lu r5, r6, r7 }
	{ ld2s r25, r26 ; cmpltsi r5, r6, 5 ; cmples r15, r16, r17 }
	{ ld2s r25, r26 ; cmpltu r15, r16, r17 ; addi r5, r6, 5 }
	{ ld2s r25, r26 ; cmpltu r15, r16, r17 ; rotl r5, r6, r7 }
	{ ld2s r25, r26 ; cmpltu r5, r6, r7 ; jalrp r15 }
	{ ld2s r25, r26 ; cmpne r15, r16, r17 ; cmples r5, r6, r7 }
	{ ld2s r25, r26 ; cmpne r15, r16, r17 ; shrs r5, r6, r7 }
	{ ld2s r25, r26 ; cmpne r5, r6, r7 ; or r15, r16, r17 }
	{ ld2s r25, r26 ; ctz r5, r6 ; lnk r15 }
	{ ld2s r25, r26 ; fnop ; cmovnez r5, r6, r7 }
	{ ld2s r25, r26 ; fnop ; mula_lu_lu r5, r6, r7 }
	{ ld2s r25, r26 ; fnop ; shrui r5, r6, 5 }
	{ ld2s r25, r26 ; fsingle_pack1 r5, r6 ; or r15, r16, r17 }
	{ ld2s r25, r26 ; ill ; mnz r5, r6, r7 }
	{ ld2s r25, r26 ; ill ; xor r5, r6, r7 }
	{ ld2s r25, r26 ; info 19 ; jr r15 }
	{ ld2s r25, r26 ; info 19 ; shl2add r5, r6, r7 }
	{ ld2s r25, r26 ; jalr r15 ; cmpleu r5, r6, r7 }
	{ ld2s r25, r26 ; jalr r15 ; shrsi r5, r6, 5 }
	{ ld2s r25, r26 ; jalrp r15 ; mula_hu_hu r5, r6, r7 }
	{ ld2s r25, r26 ; jr r15 ; clz r5, r6 }
	{ ld2s r25, r26 ; jr r15 ; shl2add r5, r6, r7 }
	{ ld2s r25, r26 ; jrp r15 ; movei r5, 5 }
	{ ld2s r25, r26 ; lnk r15 ; add r5, r6, r7 }
	{ ld2s r25, r26 ; lnk r15 ; revbytes r5, r6 }
	{ ld2s r25, r26 ; mnz r15, r16, r17 ; ctz r5, r6 }
	{ ld2s r25, r26 ; mnz r15, r16, r17 ; tblidxb0 r5, r6 }
	{ ld2s r25, r26 ; mnz r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld2s r25, r26 ; move r15, r16 ; mul_lu_lu r5, r6, r7 }
	{ ld2s r25, r26 ; move r5, r6 ; and r15, r16, r17 }
	{ ld2s r25, r26 ; move r5, r6 ; subx r15, r16, r17 }
	{ ld2s r25, r26 ; movei r15, 5 ; or r5, r6, r7 }
	{ ld2s r25, r26 ; movei r5, 5 ; fnop }
	{ ld2s r25, r26 ; mul_hs_hs r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ ld2s r25, r26 ; mul_hu_hu r5, r6, r7 ; add r15, r16, r17 }
	{ ld2s r25, r26 ; mul_hu_hu r5, r6, r7 ; shrsi r15, r16, 5 }
	{ ld2s r25, r26 ; mul_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld2s r25, r26 ; mul_lu_lu r5, r6, r7 ; nop }
	{ ld2s r25, r26 ; mula_hs_hs r5, r6, r7 ; jr r15 }
	{ ld2s r25, r26 ; mula_hu_hu r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ ld2s r25, r26 ; mula_ls_ls r5, r6, r7 ; andi r15, r16, 5 }
	{ ld2s r25, r26 ; mula_ls_ls r5, r6, r7 ; xor r15, r16, r17 }
	{ ld2s r25, r26 ; mula_lu_lu r5, r6, r7 ; shli r15, r16, 5 }
	{ ld2s r25, r26 ; mulax r5, r6, r7 ; shl r15, r16, r17 }
	{ ld2s r25, r26 ; mulx r5, r6, r7 ; movei r15, 5 }
	{ ld2s r25, r26 ; mz r15, r16, r17 ; ctz r5, r6 }
	{ ld2s r25, r26 ; mz r15, r16, r17 ; tblidxb0 r5, r6 }
	{ ld2s r25, r26 ; mz r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld2s r25, r26 ; nop ; cmpltu r15, r16, r17 }
	{ ld2s r25, r26 ; nop ; rotl r15, r16, r17 }
	{ ld2s r25, r26 ; nor r15, r16, r17 ; addi r5, r6, 5 }
	{ ld2s r25, r26 ; nor r15, r16, r17 ; rotl r5, r6, r7 }
	{ ld2s r25, r26 ; nor r5, r6, r7 ; jalrp r15 }
	{ ld2s r25, r26 ; or r15, r16, r17 ; cmples r5, r6, r7 }
	{ ld2s r25, r26 ; or r15, r16, r17 ; shrs r5, r6, r7 }
	{ ld2s r25, r26 ; or r5, r6, r7 ; or r15, r16, r17 }
	{ ld2s r25, r26 ; pcnt r5, r6 ; lnk r15 }
	{ ld2s r25, r26 ; revbits r5, r6 ; fnop }
	{ ld2s r25, r26 ; revbytes r5, r6 ; cmpeqi r15, r16, 5 }
	{ ld2s r25, r26 ; rotl r15, r16, r17 ; add r5, r6, r7 }
	{ ld2s r25, r26 ; rotl r15, r16, r17 ; revbytes r5, r6 }
	{ ld2s r25, r26 ; rotl r5, r6, r7 ; jalr r15 }
	{ ld2s r25, r26 ; rotli r15, r16, 5 ; cmpeqi r5, r6, 5 }
	{ ld2s r25, r26 ; rotli r15, r16, 5 ; shli r5, r6, 5 }
	{ ld2s r25, r26 ; rotli r5, r6, 5 ; nor r15, r16, r17 }
	{ ld2s r25, r26 ; shl r15, r16, r17 ; info 19 }
	{ ld2s r25, r26 ; shl r15, r16, r17 ; tblidxb3 r5, r6 }
	{ ld2s r25, r26 ; shl r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld2s r25, r26 ; shl1add r15, r16, r17 ; mula_ls_ls r5, r6, r7 }
	{ ld2s r25, r26 ; shl1add r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ ld2s r25, r26 ; shl1addx r15, r16, r17 ; add r5, r6, r7 }
	{ ld2s r25, r26 ; shl1addx r15, r16, r17 ; revbytes r5, r6 }
	{ ld2s r25, r26 ; shl1addx r5, r6, r7 ; jalr r15 }
	{ ld2s r25, r26 ; shl2add r15, r16, r17 ; cmpeqi r5, r6, 5 }
	{ ld2s r25, r26 ; shl2add r15, r16, r17 ; shli r5, r6, 5 }
	{ ld2s r25, r26 ; shl2add r5, r6, r7 ; nor r15, r16, r17 }
	{ ld2s r25, r26 ; shl2addx r15, r16, r17 ; info 19 }
	{ ld2s r25, r26 ; shl2addx r15, r16, r17 ; tblidxb3 r5, r6 }
	{ ld2s r25, r26 ; shl2addx r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld2s r25, r26 ; shl3add r15, r16, r17 ; mula_ls_ls r5, r6, r7 }
	{ ld2s r25, r26 ; shl3add r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ ld2s r25, r26 ; shl3addx r15, r16, r17 ; add r5, r6, r7 }
	{ ld2s r25, r26 ; shl3addx r15, r16, r17 ; revbytes r5, r6 }
	{ ld2s r25, r26 ; shl3addx r5, r6, r7 ; jalr r15 }
	{ ld2s r25, r26 ; shli r15, r16, 5 ; cmpeqi r5, r6, 5 }
	{ ld2s r25, r26 ; shli r15, r16, 5 ; shli r5, r6, 5 }
	{ ld2s r25, r26 ; shli r5, r6, 5 ; nor r15, r16, r17 }
	{ ld2s r25, r26 ; shrs r15, r16, r17 ; info 19 }
	{ ld2s r25, r26 ; shrs r15, r16, r17 ; tblidxb3 r5, r6 }
	{ ld2s r25, r26 ; shrs r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld2s r25, r26 ; shrsi r15, r16, 5 ; mula_ls_ls r5, r6, r7 }
	{ ld2s r25, r26 ; shrsi r5, r6, 5 ; cmpeqi r15, r16, 5 }
	{ ld2s r25, r26 ; shru r15, r16, r17 ; add r5, r6, r7 }
	{ ld2s r25, r26 ; shru r15, r16, r17 ; revbytes r5, r6 }
	{ ld2s r25, r26 ; shru r5, r6, r7 ; jalr r15 }
	{ ld2s r25, r26 ; shrui r15, r16, 5 ; cmpeqi r5, r6, 5 }
	{ ld2s r25, r26 ; shrui r15, r16, 5 ; shli r5, r6, 5 }
	{ ld2s r25, r26 ; shrui r5, r6, 5 ; nor r15, r16, r17 }
	{ ld2s r25, r26 ; sub r15, r16, r17 ; info 19 }
	{ ld2s r25, r26 ; sub r15, r16, r17 ; tblidxb3 r5, r6 }
	{ ld2s r25, r26 ; sub r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld2s r25, r26 ; subx r15, r16, r17 ; mula_ls_ls r5, r6, r7 }
	{ ld2s r25, r26 ; subx r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ ld2s r25, r26 ; tblidxb0 r5, r6 ; add r15, r16, r17 }
	{ ld2s r25, r26 ; tblidxb0 r5, r6 ; shrsi r15, r16, 5 }
	{ ld2s r25, r26 ; tblidxb1 r5, r6 ; shl1addx r15, r16, r17 }
	{ ld2s r25, r26 ; tblidxb2 r5, r6 ; nop }
	{ ld2s r25, r26 ; tblidxb3 r5, r6 ; jr r15 }
	{ ld2s r25, r26 ; xor r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ ld2s r25, r26 ; xor r15, r16, r17 ; shrsi r5, r6, 5 }
	{ ld2s r25, r26 ; xor r5, r6, r7 ; rotl r15, r16, r17 }
	{ ld2s_add r15, r16, 5 ; cmpltui r5, r6, 5 }
	{ ld2s_add r15, r16, 5 ; mul_hs_hu r5, r6, r7 }
	{ ld2s_add r15, r16, 5 ; shlx r5, r6, r7 }
	{ ld2s_add r15, r16, 5 ; v1int_h r5, r6, r7 }
	{ ld2s_add r15, r16, 5 ; v2maxsi r5, r6, 5 }
	{ ld2u r15, r16 ; addx r5, r6, r7 }
	{ ld2u r15, r16 ; fdouble_sub_flags r5, r6, r7 }
	{ ld2u r15, r16 ; mz r5, r6, r7 }
	{ ld2u r15, r16 ; v1cmpeq r5, r6, r7 }
	{ ld2u r15, r16 ; v2add r5, r6, r7 }
	{ ld2u r15, r16 ; v2shrui r5, r6, 5 }
	{ ld2u r25, r26 ; add r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ ld2u r25, r26 ; add r5, r6, r7 ; addi r15, r16, 5 }
	{ ld2u r25, r26 ; add r5, r6, r7 ; shru r15, r16, r17 }
	{ ld2u r25, r26 ; addi r15, r16, 5 ; mz r5, r6, r7 }
	{ ld2u r25, r26 ; addi r5, r6, 5 ; cmpltsi r15, r16, 5 }
	{ ld2u r25, r26 ; addx r15, r16, r17 ; and r5, r6, r7 }
	{ ld2u r25, r26 ; addx r15, r16, r17 ; shl1add r5, r6, r7 }
	{ ld2u r25, r26 ; addx r5, r6, r7 ; lnk r15 }
	{ ld2u r25, r26 ; addxi r15, r16, 5 ; cmpltsi r5, r6, 5 }
	{ ld2u r25, r26 ; addxi r15, r16, 5 ; shrui r5, r6, 5 }
	{ ld2u r25, r26 ; addxi r5, r6, 5 ; shl r15, r16, r17 }
	{ ld2u r25, r26 ; and r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ ld2u r25, r26 ; and r5, r6, r7 ; addi r15, r16, 5 }
	{ ld2u r25, r26 ; and r5, r6, r7 ; shru r15, r16, r17 }
	{ ld2u r25, r26 ; andi r15, r16, 5 ; mz r5, r6, r7 }
	{ ld2u r25, r26 ; andi r5, r6, 5 ; cmpltsi r15, r16, 5 }
	{ ld2u r25, r26 ; clz r5, r6 ; and r15, r16, r17 }
	{ ld2u r25, r26 ; clz r5, r6 ; subx r15, r16, r17 }
	{ ld2u r25, r26 ; cmoveqz r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld2u r25, r26 ; cmovnez r5, r6, r7 ; rotli r15, r16, 5 }
	{ ld2u r25, r26 ; cmpeq r15, r16, r17 ; movei r5, 5 }
	{ ld2u r25, r26 ; cmpeq r5, r6, r7 ; add r15, r16, r17 }
	{ ld2u r25, r26 ; cmpeq r5, r6, r7 ; shrsi r15, r16, 5 }
	{ ld2u r25, r26 ; cmpeqi r15, r16, 5 ; mulx r5, r6, r7 }
	{ ld2u r25, r26 ; cmpeqi r5, r6, 5 ; cmplts r15, r16, r17 }
	{ ld2u r25, r26 ; cmples r15, r16, r17 ; addxi r5, r6, 5 }
	{ ld2u r25, r26 ; cmples r15, r16, r17 ; shl r5, r6, r7 }
	{ ld2u r25, r26 ; cmples r5, r6, r7 ; jrp r15 }
	{ ld2u r25, r26 ; cmpleu r15, r16, r17 ; cmplts r5, r6, r7 }
	{ ld2u r25, r26 ; cmpleu r15, r16, r17 ; shru r5, r6, r7 }
	{ ld2u r25, r26 ; cmpleu r5, r6, r7 ; rotli r15, r16, 5 }
	{ ld2u r25, r26 ; cmplts r15, r16, r17 ; movei r5, 5 }
	{ ld2u r25, r26 ; cmplts r5, r6, r7 ; add r15, r16, r17 }
	{ ld2u r25, r26 ; cmplts r5, r6, r7 ; shrsi r15, r16, 5 }
	{ ld2u r25, r26 ; cmpltsi r15, r16, 5 ; mulx r5, r6, r7 }
	{ ld2u r25, r26 ; cmpltsi r5, r6, 5 ; cmplts r15, r16, r17 }
	{ ld2u r25, r26 ; cmpltu r15, r16, r17 ; addxi r5, r6, 5 }
	{ ld2u r25, r26 ; cmpltu r15, r16, r17 ; shl r5, r6, r7 }
	{ ld2u r25, r26 ; cmpltu r5, r6, r7 ; jrp r15 }
	{ ld2u r25, r26 ; cmpne r15, r16, r17 ; cmplts r5, r6, r7 }
	{ ld2u r25, r26 ; cmpne r15, r16, r17 ; shru r5, r6, r7 }
	{ ld2u r25, r26 ; cmpne r5, r6, r7 ; rotli r15, r16, 5 }
	{ ld2u r25, r26 ; ctz r5, r6 ; move r15, r16 }
	{ ld2u r25, r26 ; fnop ; cmpeq r5, r6, r7 }
	{ ld2u r25, r26 ; fnop ; mulx r5, r6, r7 }
	{ ld2u r25, r26 ; fnop ; sub r5, r6, r7 }
	{ ld2u r25, r26 ; fsingle_pack1 r5, r6 ; rotli r15, r16, 5 }
	{ ld2u r25, r26 ; ill ; movei r5, 5 }
	{ ld2u r25, r26 ; info 19 ; add r15, r16, r17 }
	{ ld2u r25, r26 ; info 19 ; lnk r15 }
	{ ld2u r25, r26 ; info 19 ; shl2addx r5, r6, r7 }
	{ ld2u r25, r26 ; jalr r15 ; cmpltsi r5, r6, 5 }
	{ ld2u r25, r26 ; jalr r15 ; shrui r5, r6, 5 }
	{ ld2u r25, r26 ; jalrp r15 ; mula_lu_lu r5, r6, r7 }
	{ ld2u r25, r26 ; jr r15 ; cmovnez r5, r6, r7 }
	{ ld2u r25, r26 ; jr r15 ; shl3add r5, r6, r7 }
	{ ld2u r25, r26 ; jrp r15 ; mul_hu_hu r5, r6, r7 }
	{ ld2u r25, r26 ; lnk r15 ; addx r5, r6, r7 }
	{ ld2u r25, r26 ; lnk r15 ; rotli r5, r6, 5 }
	{ ld2u r25, r26 ; mnz r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ ld2u r25, r26 ; mnz r15, r16, r17 ; tblidxb2 r5, r6 }
	{ ld2u r25, r26 ; mnz r5, r6, r7 ; shl3add r15, r16, r17 }
	{ ld2u r25, r26 ; move r15, r16 ; mula_hu_hu r5, r6, r7 }
	{ ld2u r25, r26 ; move r5, r6 ; cmpeq r15, r16, r17 }
	{ ld2u r25, r26 ; move r5, r6 }
	{ ld2u r25, r26 ; movei r15, 5 ; revbits r5, r6 }
	{ ld2u r25, r26 ; movei r5, 5 ; info 19 }
	{ ld2u r25, r26 ; mul_hs_hs r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ ld2u r25, r26 ; mul_hu_hu r5, r6, r7 ; addx r15, r16, r17 }
	{ ld2u r25, r26 ; mul_hu_hu r5, r6, r7 ; shrui r15, r16, 5 }
	{ ld2u r25, r26 ; mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld2u r25, r26 ; mul_lu_lu r5, r6, r7 ; or r15, r16, r17 }
	{ ld2u r25, r26 ; mula_hs_hs r5, r6, r7 ; lnk r15 }
	{ ld2u r25, r26 ; mula_hu_hu r5, r6, r7 ; fnop }
	{ ld2u r25, r26 ; mula_ls_ls r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ ld2u r25, r26 ; mula_lu_lu r5, r6, r7 ; add r15, r16, r17 }
	{ ld2u r25, r26 ; mula_lu_lu r5, r6, r7 ; shrsi r15, r16, 5 }
	{ ld2u r25, r26 ; mulax r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld2u r25, r26 ; mulx r5, r6, r7 ; nop }
	{ ld2u r25, r26 ; mz r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ ld2u r25, r26 ; mz r15, r16, r17 ; tblidxb2 r5, r6 }
	{ ld2u r25, r26 ; mz r5, r6, r7 ; shl3add r15, r16, r17 }
	{ ld2u r25, r26 ; nop ; cmpne r15, r16, r17 }
	{ ld2u r25, r26 ; nop ; rotli r15, r16, 5 }
	{ ld2u r25, r26 ; nor r15, r16, r17 ; addxi r5, r6, 5 }
	{ ld2u r25, r26 ; nor r15, r16, r17 ; shl r5, r6, r7 }
	{ ld2u r25, r26 ; nor r5, r6, r7 ; jrp r15 }
	{ ld2u r25, r26 ; or r15, r16, r17 ; cmplts r5, r6, r7 }
	{ ld2u r25, r26 ; or r15, r16, r17 ; shru r5, r6, r7 }
	{ ld2u r25, r26 ; or r5, r6, r7 ; rotli r15, r16, 5 }
	{ ld2u r25, r26 ; pcnt r5, r6 ; move r15, r16 }
	{ ld2u r25, r26 ; revbits r5, r6 ; info 19 }
	{ ld2u r25, r26 ; revbytes r5, r6 ; cmpleu r15, r16, r17 }
	{ ld2u r25, r26 ; rotl r15, r16, r17 ; addx r5, r6, r7 }
	{ ld2u r25, r26 ; rotl r15, r16, r17 ; rotli r5, r6, 5 }
	{ ld2u r25, r26 ; rotl r5, r6, r7 ; jr r15 }
	{ ld2u r25, r26 ; rotli r15, r16, 5 ; cmpleu r5, r6, r7 }
	{ ld2u r25, r26 ; rotli r15, r16, 5 ; shrsi r5, r6, 5 }
	{ ld2u r25, r26 ; rotli r5, r6, 5 ; rotl r15, r16, r17 }
	{ ld2u r25, r26 ; shl r15, r16, r17 ; move r5, r6 }
	{ ld2u r25, r26 ; shl r15, r16, r17 }
	{ ld2u r25, r26 ; shl r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld2u r25, r26 ; shl1add r15, r16, r17 ; mulax r5, r6, r7 }
	{ ld2u r25, r26 ; shl1add r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ ld2u r25, r26 ; shl1addx r15, r16, r17 ; addx r5, r6, r7 }
	{ ld2u r25, r26 ; shl1addx r15, r16, r17 ; rotli r5, r6, 5 }
	{ ld2u r25, r26 ; shl1addx r5, r6, r7 ; jr r15 }
	{ ld2u r25, r26 ; shl2add r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ ld2u r25, r26 ; shl2add r15, r16, r17 ; shrsi r5, r6, 5 }
	{ ld2u r25, r26 ; shl2add r5, r6, r7 ; rotl r15, r16, r17 }
	{ ld2u r25, r26 ; shl2addx r15, r16, r17 ; move r5, r6 }
	{ ld2u r25, r26 ; shl2addx r15, r16, r17 }
	{ ld2u r25, r26 ; shl2addx r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld2u r25, r26 ; shl3add r15, r16, r17 ; mulax r5, r6, r7 }
	{ ld2u r25, r26 ; shl3add r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ ld2u r25, r26 ; shl3addx r15, r16, r17 ; addx r5, r6, r7 }
	{ ld2u r25, r26 ; shl3addx r15, r16, r17 ; rotli r5, r6, 5 }
	{ ld2u r25, r26 ; shl3addx r5, r6, r7 ; jr r15 }
	{ ld2u r25, r26 ; shli r15, r16, 5 ; cmpleu r5, r6, r7 }
	{ ld2u r25, r26 ; shli r15, r16, 5 ; shrsi r5, r6, 5 }
	{ ld2u r25, r26 ; shli r5, r6, 5 ; rotl r15, r16, r17 }
	{ ld2u r25, r26 ; shrs r15, r16, r17 ; move r5, r6 }
	{ ld2u r25, r26 ; shrs r15, r16, r17 }
	{ ld2u r25, r26 ; shrs r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld2u r25, r26 ; shrsi r15, r16, 5 ; mulax r5, r6, r7 }
	{ ld2u r25, r26 ; shrsi r5, r6, 5 ; cmpleu r15, r16, r17 }
	{ ld2u r25, r26 ; shru r15, r16, r17 ; addx r5, r6, r7 }
	{ ld2u r25, r26 ; shru r15, r16, r17 ; rotli r5, r6, 5 }
	{ ld2u r25, r26 ; shru r5, r6, r7 ; jr r15 }
	{ ld2u r25, r26 ; shrui r15, r16, 5 ; cmpleu r5, r6, r7 }
	{ ld2u r25, r26 ; shrui r15, r16, 5 ; shrsi r5, r6, 5 }
	{ ld2u r25, r26 ; shrui r5, r6, 5 ; rotl r15, r16, r17 }
	{ ld2u r25, r26 ; sub r15, r16, r17 ; move r5, r6 }
	{ ld2u r25, r26 ; sub r15, r16, r17 }
	{ ld2u r25, r26 ; sub r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld2u r25, r26 ; subx r15, r16, r17 ; mulax r5, r6, r7 }
	{ ld2u r25, r26 ; subx r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ ld2u r25, r26 ; tblidxb0 r5, r6 ; addx r15, r16, r17 }
	{ ld2u r25, r26 ; tblidxb0 r5, r6 ; shrui r15, r16, 5 }
	{ ld2u r25, r26 ; tblidxb1 r5, r6 ; shl2addx r15, r16, r17 }
	{ ld2u r25, r26 ; tblidxb2 r5, r6 ; or r15, r16, r17 }
	{ ld2u r25, r26 ; tblidxb3 r5, r6 ; lnk r15 }
	{ ld2u r25, r26 ; xor r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ ld2u r25, r26 ; xor r15, r16, r17 ; shrui r5, r6, 5 }
	{ ld2u r25, r26 ; xor r5, r6, r7 ; shl r15, r16, r17 }
	{ ld2u_add r15, r16, 5 ; cmul r5, r6, r7 }
	{ ld2u_add r15, r16, 5 ; mul_hs_lu r5, r6, r7 }
	{ ld2u_add r15, r16, 5 ; shrs r5, r6, r7 }
	{ ld2u_add r15, r16, 5 ; v1maxu r5, r6, r7 }
	{ ld2u_add r15, r16, 5 ; v2minsi r5, r6, 5 }
	{ ld4s r15, r16 ; addxli r5, r6, 0x1234 }
	{ ld4s r15, r16 ; fdouble_unpack_min r5, r6, r7 }
	{ ld4s r15, r16 ; nor r5, r6, r7 }
	{ ld4s r15, r16 ; v1cmples r5, r6, r7 }
	{ ld4s r15, r16 ; v2addsc r5, r6, r7 }
	{ ld4s r15, r16 ; v2subsc r5, r6, r7 }
	{ ld4s r25, r26 ; add r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld4s r25, r26 ; add r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld4s r25, r26 ; add r5, r6, r7 ; sub r15, r16, r17 }
	{ ld4s r25, r26 ; addi r15, r16, 5 ; nor r5, r6, r7 }
	{ ld4s r25, r26 ; addi r5, r6, 5 ; cmpne r15, r16, r17 }
	{ ld4s r25, r26 ; addx r15, r16, r17 ; clz r5, r6 }
	{ ld4s r25, r26 ; addx r15, r16, r17 ; shl2add r5, r6, r7 }
	{ ld4s r25, r26 ; addx r5, r6, r7 ; move r15, r16 }
	{ ld4s r25, r26 ; addxi r15, r16, 5 ; cmpne r5, r6, r7 }
	{ ld4s r25, r26 ; addxi r15, r16, 5 ; subx r5, r6, r7 }
	{ ld4s r25, r26 ; addxi r5, r6, 5 ; shl1addx r15, r16, r17 }
	{ ld4s r25, r26 ; and r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld4s r25, r26 ; and r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld4s r25, r26 ; and r5, r6, r7 ; sub r15, r16, r17 }
	{ ld4s r25, r26 ; andi r15, r16, 5 ; nor r5, r6, r7 }
	{ ld4s r25, r26 ; andi r5, r6, 5 ; cmpne r15, r16, r17 }
	{ ld4s r25, r26 ; clz r5, r6 ; cmpeq r15, r16, r17 }
	{ ld4s r25, r26 ; clz r5, r6 }
	{ ld4s r25, r26 ; cmoveqz r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld4s r25, r26 ; cmovnez r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld4s r25, r26 ; cmpeq r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ ld4s r25, r26 ; cmpeq r5, r6, r7 ; addx r15, r16, r17 }
	{ ld4s r25, r26 ; cmpeq r5, r6, r7 ; shrui r15, r16, 5 }
	{ ld4s r25, r26 ; cmpeqi r15, r16, 5 ; nop }
	{ ld4s r25, r26 ; cmpeqi r5, r6, 5 ; cmpltu r15, r16, r17 }
	{ ld4s r25, r26 ; cmples r15, r16, r17 ; andi r5, r6, 5 }
	{ ld4s r25, r26 ; cmples r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ ld4s r25, r26 ; cmples r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld4s r25, r26 ; cmpleu r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld4s r25, r26 ; cmpleu r15, r16, r17 ; sub r5, r6, r7 }
	{ ld4s r25, r26 ; cmpleu r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld4s r25, r26 ; cmplts r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ ld4s r25, r26 ; cmplts r5, r6, r7 ; addx r15, r16, r17 }
	{ ld4s r25, r26 ; cmplts r5, r6, r7 ; shrui r15, r16, 5 }
	{ ld4s r25, r26 ; cmpltsi r15, r16, 5 ; nop }
	{ ld4s r25, r26 ; cmpltsi r5, r6, 5 ; cmpltu r15, r16, r17 }
	{ ld4s r25, r26 ; cmpltu r15, r16, r17 ; andi r5, r6, 5 }
	{ ld4s r25, r26 ; cmpltu r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ ld4s r25, r26 ; cmpltu r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld4s r25, r26 ; cmpne r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld4s r25, r26 ; cmpne r15, r16, r17 ; sub r5, r6, r7 }
	{ ld4s r25, r26 ; cmpne r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld4s r25, r26 ; ctz r5, r6 ; mz r15, r16, r17 }
	{ ld4s r25, r26 ; fnop ; cmpeqi r5, r6, 5 }
	{ ld4s r25, r26 ; fnop ; mz r5, r6, r7 }
	{ ld4s r25, r26 ; fnop ; subx r5, r6, r7 }
	{ ld4s r25, r26 ; fsingle_pack1 r5, r6 ; shl1add r15, r16, r17 }
	{ ld4s r25, r26 ; ill ; mul_hu_hu r5, r6, r7 }
	{ ld4s r25, r26 ; info 19 ; addi r15, r16, 5 }
	{ ld4s r25, r26 ; info 19 ; mnz r5, r6, r7 }
	{ ld4s r25, r26 ; info 19 ; shl3add r5, r6, r7 }
	{ ld4s r25, r26 ; jalr r15 ; cmpne r5, r6, r7 }
	{ ld4s r25, r26 ; jalr r15 ; subx r5, r6, r7 }
	{ ld4s r25, r26 ; jalrp r15 ; mulx r5, r6, r7 }
	{ ld4s r25, r26 ; jr r15 ; cmpeqi r5, r6, 5 }
	{ ld4s r25, r26 ; jr r15 ; shli r5, r6, 5 }
	{ ld4s r25, r26 ; jrp r15 ; mul_lu_lu r5, r6, r7 }
	{ ld4s r25, r26 ; lnk r15 ; and r5, r6, r7 }
	{ ld4s r25, r26 ; lnk r15 ; shl1add r5, r6, r7 }
	{ ld4s r25, r26 ; mnz r15, r16, r17 ; mnz r5, r6, r7 }
	{ ld4s r25, r26 ; mnz r15, r16, r17 ; xor r5, r6, r7 }
	{ ld4s r25, r26 ; mnz r5, r6, r7 ; shli r15, r16, 5 }
	{ ld4s r25, r26 ; move r15, r16 ; mula_lu_lu r5, r6, r7 }
	{ ld4s r25, r26 ; move r5, r6 ; cmples r15, r16, r17 }
	{ ld4s r25, r26 ; movei r15, 5 ; addi r5, r6, 5 }
	{ ld4s r25, r26 ; movei r15, 5 ; rotl r5, r6, r7 }
	{ ld4s r25, r26 ; movei r5, 5 ; jalrp r15 }
	{ ld4s r25, r26 ; mul_hs_hs r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ ld4s r25, r26 ; mul_hu_hu r5, r6, r7 ; and r15, r16, r17 }
	{ ld4s r25, r26 ; mul_hu_hu r5, r6, r7 ; subx r15, r16, r17 }
	{ ld4s r25, r26 ; mul_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld4s r25, r26 ; mul_lu_lu r5, r6, r7 ; rotli r15, r16, 5 }
	{ ld4s r25, r26 ; mula_hs_hs r5, r6, r7 ; move r15, r16 }
	{ ld4s r25, r26 ; mula_hu_hu r5, r6, r7 ; info 19 }
	{ ld4s r25, r26 ; mula_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ ld4s r25, r26 ; mula_lu_lu r5, r6, r7 ; addx r15, r16, r17 }
	{ ld4s r25, r26 ; mula_lu_lu r5, r6, r7 ; shrui r15, r16, 5 }
	{ ld4s r25, r26 ; mulax r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld4s r25, r26 ; mulx r5, r6, r7 ; or r15, r16, r17 }
	{ ld4s r25, r26 ; mz r15, r16, r17 ; mnz r5, r6, r7 }
	{ ld4s r25, r26 ; mz r15, r16, r17 ; xor r5, r6, r7 }
	{ ld4s r25, r26 ; mz r5, r6, r7 ; shli r15, r16, 5 }
	{ ld4s r25, r26 ; nop ; ctz r5, r6 }
	{ ld4s r25, r26 ; nop ; shl r15, r16, r17 }
	{ ld4s r25, r26 ; nor r15, r16, r17 ; andi r5, r6, 5 }
	{ ld4s r25, r26 ; nor r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ ld4s r25, r26 ; nor r5, r6, r7 ; mnz r15, r16, r17 }
	{ ld4s r25, r26 ; or r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ ld4s r25, r26 ; or r15, r16, r17 ; sub r5, r6, r7 }
	{ ld4s r25, r26 ; or r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld4s r25, r26 ; pcnt r5, r6 ; mz r15, r16, r17 }
	{ ld4s r25, r26 ; revbits r5, r6 ; jalrp r15 }
	{ ld4s r25, r26 ; revbytes r5, r6 ; cmpltsi r15, r16, 5 }
	{ ld4s r25, r26 ; rotl r15, r16, r17 ; and r5, r6, r7 }
	{ ld4s r25, r26 ; rotl r15, r16, r17 ; shl1add r5, r6, r7 }
	{ ld4s r25, r26 ; rotl r5, r6, r7 ; lnk r15 }
	{ ld4s r25, r26 ; rotli r15, r16, 5 ; cmpltsi r5, r6, 5 }
	{ ld4s r25, r26 ; rotli r15, r16, 5 ; shrui r5, r6, 5 }
	{ ld4s r25, r26 ; rotli r5, r6, 5 ; shl r15, r16, r17 }
	{ ld4s r25, r26 ; shl r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ ld4s r25, r26 ; shl r5, r6, r7 ; addi r15, r16, 5 }
	{ ld4s r25, r26 ; shl r5, r6, r7 ; shru r15, r16, r17 }
	{ ld4s r25, r26 ; shl1add r15, r16, r17 ; mz r5, r6, r7 }
	{ ld4s r25, r26 ; shl1add r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ ld4s r25, r26 ; shl1addx r15, r16, r17 ; and r5, r6, r7 }
	{ ld4s r25, r26 ; shl1addx r15, r16, r17 ; shl1add r5, r6, r7 }
	{ ld4s r25, r26 ; shl1addx r5, r6, r7 ; lnk r15 }
	{ ld4s r25, r26 ; shl2add r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ ld4s r25, r26 ; shl2add r15, r16, r17 ; shrui r5, r6, 5 }
	{ ld4s r25, r26 ; shl2add r5, r6, r7 ; shl r15, r16, r17 }
	{ ld4s r25, r26 ; shl2addx r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ ld4s r25, r26 ; shl2addx r5, r6, r7 ; addi r15, r16, 5 }
	{ ld4s r25, r26 ; shl2addx r5, r6, r7 ; shru r15, r16, r17 }
	{ ld4s r25, r26 ; shl3add r15, r16, r17 ; mz r5, r6, r7 }
	{ ld4s r25, r26 ; shl3add r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ ld4s r25, r26 ; shl3addx r15, r16, r17 ; and r5, r6, r7 }
	{ ld4s r25, r26 ; shl3addx r15, r16, r17 ; shl1add r5, r6, r7 }
	{ ld4s r25, r26 ; shl3addx r5, r6, r7 ; lnk r15 }
	{ ld4s r25, r26 ; shli r15, r16, 5 ; cmpltsi r5, r6, 5 }
	{ ld4s r25, r26 ; shli r15, r16, 5 ; shrui r5, r6, 5 }
	{ ld4s r25, r26 ; shli r5, r6, 5 ; shl r15, r16, r17 }
	{ ld4s r25, r26 ; shrs r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ ld4s r25, r26 ; shrs r5, r6, r7 ; addi r15, r16, 5 }
	{ ld4s r25, r26 ; shrs r5, r6, r7 ; shru r15, r16, r17 }
	{ ld4s r25, r26 ; shrsi r15, r16, 5 ; mz r5, r6, r7 }
	{ ld4s r25, r26 ; shrsi r5, r6, 5 ; cmpltsi r15, r16, 5 }
	{ ld4s r25, r26 ; shru r15, r16, r17 ; and r5, r6, r7 }
	{ ld4s r25, r26 ; shru r15, r16, r17 ; shl1add r5, r6, r7 }
	{ ld4s r25, r26 ; shru r5, r6, r7 ; lnk r15 }
	{ ld4s r25, r26 ; shrui r15, r16, 5 ; cmpltsi r5, r6, 5 }
	{ ld4s r25, r26 ; shrui r15, r16, 5 ; shrui r5, r6, 5 }
	{ ld4s r25, r26 ; shrui r5, r6, 5 ; shl r15, r16, r17 }
	{ ld4s r25, r26 ; sub r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ ld4s r25, r26 ; sub r5, r6, r7 ; addi r15, r16, 5 }
	{ ld4s r25, r26 ; sub r5, r6, r7 ; shru r15, r16, r17 }
	{ ld4s r25, r26 ; subx r15, r16, r17 ; mz r5, r6, r7 }
	{ ld4s r25, r26 ; subx r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ ld4s r25, r26 ; tblidxb0 r5, r6 ; and r15, r16, r17 }
	{ ld4s r25, r26 ; tblidxb0 r5, r6 ; subx r15, r16, r17 }
	{ ld4s r25, r26 ; tblidxb1 r5, r6 ; shl3addx r15, r16, r17 }
	{ ld4s r25, r26 ; tblidxb2 r5, r6 ; rotli r15, r16, 5 }
	{ ld4s r25, r26 ; tblidxb3 r5, r6 ; move r15, r16 }
	{ ld4s r25, r26 ; xor r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld4s r25, r26 ; xor r15, r16, r17 ; subx r5, r6, r7 }
	{ ld4s r25, r26 ; xor r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld4s_add r15, r16, 5 ; cmulaf r5, r6, r7 }
	{ ld4s_add r15, r16, 5 ; mul_hu_ls r5, r6, r7 }
	{ ld4s_add r15, r16, 5 ; shru r5, r6, r7 }
	{ ld4s_add r15, r16, 5 ; v1minu r5, r6, r7 }
	{ ld4s_add r15, r16, 5 ; v2mulfsc r5, r6, r7 }
	{ ld4u r15, r16 ; and r5, r6, r7 }
	{ ld4u r15, r16 ; fsingle_add1 r5, r6, r7 }
	{ ld4u r15, r16 ; ori r5, r6, 5 }
	{ ld4u r15, r16 ; v1cmplts r5, r6, r7 }
	{ ld4u r15, r16 ; v2avgs r5, r6, r7 }
	{ ld4u r15, r16 ; v4addsc r5, r6, r7 }
	{ ld4u r25, r26 ; add r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ ld4u r25, r26 ; add r5, r6, r7 ; andi r15, r16, 5 }
	{ ld4u r25, r26 ; add r5, r6, r7 ; xor r15, r16, r17 }
	{ ld4u r25, r26 ; addi r15, r16, 5 ; pcnt r5, r6 }
	{ ld4u r25, r26 ; addi r5, r6, 5 ; ill }
	{ ld4u r25, r26 ; addx r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ ld4u r25, r26 ; addx r15, r16, r17 ; shl3add r5, r6, r7 }
	{ ld4u r25, r26 ; addx r5, r6, r7 ; mz r15, r16, r17 }
	{ ld4u r25, r26 ; addxi r15, r16, 5 ; fnop }
	{ ld4u r25, r26 ; addxi r15, r16, 5 ; tblidxb1 r5, r6 }
	{ ld4u r25, r26 ; addxi r5, r6, 5 ; shl2addx r15, r16, r17 }
	{ ld4u r25, r26 ; and r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ ld4u r25, r26 ; and r5, r6, r7 ; andi r15, r16, 5 }
	{ ld4u r25, r26 ; and r5, r6, r7 ; xor r15, r16, r17 }
	{ ld4u r25, r26 ; andi r15, r16, 5 ; pcnt r5, r6 }
	{ ld4u r25, r26 ; andi r5, r6, 5 ; ill }
	{ ld4u r25, r26 ; clz r5, r6 ; cmples r15, r16, r17 }
	{ ld4u r25, r26 ; cmoveqz r5, r6, r7 ; addi r15, r16, 5 }
	{ ld4u r25, r26 ; cmoveqz r5, r6, r7 ; shru r15, r16, r17 }
	{ ld4u r25, r26 ; cmovnez r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld4u r25, r26 ; cmpeq r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ ld4u r25, r26 ; cmpeq r5, r6, r7 ; and r15, r16, r17 }
	{ ld4u r25, r26 ; cmpeq r5, r6, r7 ; subx r15, r16, r17 }
	{ ld4u r25, r26 ; cmpeqi r15, r16, 5 ; or r5, r6, r7 }
	{ ld4u r25, r26 ; cmpeqi r5, r6, 5 ; fnop }
	{ ld4u r25, r26 ; cmples r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ ld4u r25, r26 ; cmples r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ ld4u r25, r26 ; cmples r5, r6, r7 ; movei r15, 5 }
	{ ld4u r25, r26 ; cmpleu r15, r16, r17 ; ctz r5, r6 }
	{ ld4u r25, r26 ; cmpleu r15, r16, r17 ; tblidxb0 r5, r6 }
	{ ld4u r25, r26 ; cmpleu r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld4u r25, r26 ; cmplts r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ ld4u r25, r26 ; cmplts r5, r6, r7 ; and r15, r16, r17 }
	{ ld4u r25, r26 ; cmplts r5, r6, r7 ; subx r15, r16, r17 }
	{ ld4u r25, r26 ; cmpltsi r15, r16, 5 ; or r5, r6, r7 }
	{ ld4u r25, r26 ; cmpltsi r5, r6, 5 ; fnop }
	{ ld4u r25, r26 ; cmpltu r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ ld4u r25, r26 ; cmpltu r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ ld4u r25, r26 ; cmpltu r5, r6, r7 ; movei r15, 5 }
	{ ld4u r25, r26 ; cmpne r15, r16, r17 ; ctz r5, r6 }
	{ ld4u r25, r26 ; cmpne r15, r16, r17 ; tblidxb0 r5, r6 }
	{ ld4u r25, r26 ; cmpne r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld4u r25, r26 ; ctz r5, r6 ; nor r15, r16, r17 }
	{ ld4u r25, r26 ; fnop ; cmples r5, r6, r7 }
	{ ld4u r25, r26 ; fnop ; nor r15, r16, r17 }
	{ ld4u r25, r26 ; fnop ; tblidxb1 r5, r6 }
	{ ld4u r25, r26 ; fsingle_pack1 r5, r6 ; shl2add r15, r16, r17 }
	{ ld4u r25, r26 ; ill ; mul_lu_lu r5, r6, r7 }
	{ ld4u r25, r26 ; info 19 ; addx r15, r16, r17 }
	{ ld4u r25, r26 ; info 19 ; move r5, r6 }
	{ ld4u r25, r26 ; info 19 ; shl3addx r5, r6, r7 }
	{ ld4u r25, r26 ; jalr r15 ; fnop }
	{ ld4u r25, r26 ; jalr r15 ; tblidxb1 r5, r6 }
	{ ld4u r25, r26 ; jalrp r15 ; nop }
	{ ld4u r25, r26 ; jr r15 ; cmpleu r5, r6, r7 }
	{ ld4u r25, r26 ; jr r15 ; shrsi r5, r6, 5 }
	{ ld4u r25, r26 ; jrp r15 ; mula_hu_hu r5, r6, r7 }
	{ ld4u r25, r26 ; lnk r15 ; clz r5, r6 }
	{ ld4u r25, r26 ; lnk r15 ; shl2add r5, r6, r7 }
	{ ld4u r25, r26 ; mnz r15, r16, r17 ; movei r5, 5 }
	{ ld4u r25, r26 ; mnz r5, r6, r7 ; add r15, r16, r17 }
	{ ld4u r25, r26 ; mnz r5, r6, r7 ; shrsi r15, r16, 5 }
	{ ld4u r25, r26 ; move r15, r16 ; mulx r5, r6, r7 }
	{ ld4u r25, r26 ; move r5, r6 ; cmplts r15, r16, r17 }
	{ ld4u r25, r26 ; movei r15, 5 ; addxi r5, r6, 5 }
	{ ld4u r25, r26 ; movei r15, 5 ; shl r5, r6, r7 }
	{ ld4u r25, r26 ; movei r5, 5 ; jrp r15 }
	{ ld4u r25, r26 ; mul_hs_hs r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld4u r25, r26 ; mul_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ ld4u r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ ld4u r25, r26 ; mul_ls_ls r5, r6, r7 ; shrs r15, r16, r17 }
	{ ld4u r25, r26 ; mul_lu_lu r5, r6, r7 ; shl1add r15, r16, r17 }
	{ ld4u r25, r26 ; mula_hs_hs r5, r6, r7 ; mz r15, r16, r17 }
	{ ld4u r25, r26 ; mula_hu_hu r5, r6, r7 ; jalrp r15 }
	{ ld4u r25, r26 ; mula_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ ld4u r25, r26 ; mula_lu_lu r5, r6, r7 ; and r15, r16, r17 }
	{ ld4u r25, r26 ; mula_lu_lu r5, r6, r7 ; subx r15, r16, r17 }
	{ ld4u r25, r26 ; mulax r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ ld4u r25, r26 ; mulx r5, r6, r7 ; rotli r15, r16, 5 }
	{ ld4u r25, r26 ; mz r15, r16, r17 ; movei r5, 5 }
	{ ld4u r25, r26 ; mz r5, r6, r7 ; add r15, r16, r17 }
	{ ld4u r25, r26 ; mz r5, r6, r7 ; shrsi r15, r16, 5 }
	{ ld4u r25, r26 ; nop ; fsingle_pack1 r5, r6 }
	{ ld4u r25, r26 ; nop ; shl1add r15, r16, r17 }
	{ ld4u r25, r26 ; nor r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ ld4u r25, r26 ; nor r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ ld4u r25, r26 ; nor r5, r6, r7 ; movei r15, 5 }
	{ ld4u r25, r26 ; or r15, r16, r17 ; ctz r5, r6 }
	{ ld4u r25, r26 ; or r15, r16, r17 ; tblidxb0 r5, r6 }
	{ ld4u r25, r26 ; or r5, r6, r7 ; shl2add r15, r16, r17 }
	{ ld4u r25, r26 ; pcnt r5, r6 ; nor r15, r16, r17 }
	{ ld4u r25, r26 ; revbits r5, r6 ; jrp r15 }
	{ ld4u r25, r26 ; revbytes r5, r6 ; cmpne r15, r16, r17 }
	{ ld4u r25, r26 ; rotl r15, r16, r17 ; clz r5, r6 }
	{ ld4u r25, r26 ; rotl r15, r16, r17 ; shl2add r5, r6, r7 }
	{ ld4u r25, r26 ; rotl r5, r6, r7 ; move r15, r16 }
	{ ld4u r25, r26 ; rotli r15, r16, 5 ; cmpne r5, r6, r7 }
	{ ld4u r25, r26 ; rotli r15, r16, 5 ; subx r5, r6, r7 }
	{ ld4u r25, r26 ; rotli r5, r6, 5 ; shl1addx r15, r16, r17 }
	{ ld4u r25, r26 ; shl r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld4u r25, r26 ; shl r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld4u r25, r26 ; shl r5, r6, r7 ; sub r15, r16, r17 }
	{ ld4u r25, r26 ; shl1add r15, r16, r17 ; nor r5, r6, r7 }
	{ ld4u r25, r26 ; shl1add r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld4u r25, r26 ; shl1addx r15, r16, r17 ; clz r5, r6 }
	{ ld4u r25, r26 ; shl1addx r15, r16, r17 ; shl2add r5, r6, r7 }
	{ ld4u r25, r26 ; shl1addx r5, r6, r7 ; move r15, r16 }
	{ ld4u r25, r26 ; shl2add r15, r16, r17 ; cmpne r5, r6, r7 }
	{ ld4u r25, r26 ; shl2add r15, r16, r17 ; subx r5, r6, r7 }
	{ ld4u r25, r26 ; shl2add r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ ld4u r25, r26 ; shl2addx r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld4u r25, r26 ; shl2addx r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld4u r25, r26 ; shl2addx r5, r6, r7 ; sub r15, r16, r17 }
	{ ld4u r25, r26 ; shl3add r15, r16, r17 ; nor r5, r6, r7 }
	{ ld4u r25, r26 ; shl3add r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld4u r25, r26 ; shl3addx r15, r16, r17 ; clz r5, r6 }
	{ ld4u r25, r26 ; shl3addx r15, r16, r17 ; shl2add r5, r6, r7 }
	{ ld4u r25, r26 ; shl3addx r5, r6, r7 ; move r15, r16 }
	{ ld4u r25, r26 ; shli r15, r16, 5 ; cmpne r5, r6, r7 }
	{ ld4u r25, r26 ; shli r15, r16, 5 ; subx r5, r6, r7 }
	{ ld4u r25, r26 ; shli r5, r6, 5 ; shl1addx r15, r16, r17 }
	{ ld4u r25, r26 ; shrs r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld4u r25, r26 ; shrs r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld4u r25, r26 ; shrs r5, r6, r7 ; sub r15, r16, r17 }
	{ ld4u r25, r26 ; shrsi r15, r16, 5 ; nor r5, r6, r7 }
	{ ld4u r25, r26 ; shrsi r5, r6, 5 ; cmpne r15, r16, r17 }
	{ ld4u r25, r26 ; shru r15, r16, r17 ; clz r5, r6 }
	{ ld4u r25, r26 ; shru r15, r16, r17 ; shl2add r5, r6, r7 }
	{ ld4u r25, r26 ; shru r5, r6, r7 ; move r15, r16 }
	{ ld4u r25, r26 ; shrui r15, r16, 5 ; cmpne r5, r6, r7 }
	{ ld4u r25, r26 ; shrui r15, r16, 5 ; subx r5, r6, r7 }
	{ ld4u r25, r26 ; shrui r5, r6, 5 ; shl1addx r15, r16, r17 }
	{ ld4u r25, r26 ; sub r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ ld4u r25, r26 ; sub r5, r6, r7 ; addxi r15, r16, 5 }
	{ ld4u r25, r26 ; sub r5, r6, r7 ; sub r15, r16, r17 }
	{ ld4u r25, r26 ; subx r15, r16, r17 ; nor r5, r6, r7 }
	{ ld4u r25, r26 ; subx r5, r6, r7 ; cmpne r15, r16, r17 }
	{ ld4u r25, r26 ; tblidxb0 r5, r6 ; cmpeq r15, r16, r17 }
	{ ld4u r25, r26 ; tblidxb0 r5, r6 }
	{ ld4u r25, r26 ; tblidxb1 r5, r6 ; shrs r15, r16, r17 }
	{ ld4u r25, r26 ; tblidxb2 r5, r6 ; shl1add r15, r16, r17 }
	{ ld4u r25, r26 ; tblidxb3 r5, r6 ; mz r15, r16, r17 }
	{ ld4u r25, r26 ; xor r15, r16, r17 ; fnop }
	{ ld4u r25, r26 ; xor r15, r16, r17 ; tblidxb1 r5, r6 }
	{ ld4u r25, r26 ; xor r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ ld4u_add r15, r16, 5 ; cmulfr r5, r6, r7 }
	{ ld4u_add r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ ld4u_add r15, r16, 5 ; shrux r5, r6, r7 }
	{ ld4u_add r15, r16, 5 ; v1mnz r5, r6, r7 }
	{ ld4u_add r15, r16, 5 ; v2mults r5, r6, r7 }
	{ ld_add r15, r16, 5 ; bfexts r5, r6, 5, 7 }
	{ ld_add r15, r16, 5 ; fsingle_mul1 r5, r6, r7 }
	{ ld_add r15, r16, 5 ; revbits r5, r6 }
	{ ld_add r15, r16, 5 ; v1cmpltu r5, r6, r7 }
	{ ld_add r15, r16, 5 ; v2cmpeqi r5, r6, 5 }
	{ ld_add r15, r16, 5 ; v4int_l r5, r6, r7 }
	{ ldna r15, r16 ; cmulhr r5, r6, r7 }
	{ ldna r15, r16 ; mul_lu_lu r5, r6, r7 }
	{ ldna r15, r16 ; shufflebytes r5, r6, r7 }
	{ ldna r15, r16 ; v1mulu r5, r6, r7 }
	{ ldna r15, r16 ; v2packh r5, r6, r7 }
	{ ldna_add r15, r16, 5 ; bfins r5, r6, 5, 7 }
	{ ldna_add r15, r16, 5 ; fsingle_pack1 r5, r6 }
	{ ldna_add r15, r16, 5 ; rotl r5, r6, r7 }
	{ ldna_add r15, r16, 5 ; v1cmpne r5, r6, r7 }
	{ ldna_add r15, r16, 5 ; v2cmpleu r5, r6, r7 }
	{ ldna_add r15, r16, 5 ; v4shl r5, r6, r7 }
	{ ldnt r15, r16 ; crc32_8 r5, r6, r7 }
	{ ldnt r15, r16 ; mula_hs_hu r5, r6, r7 }
	{ ldnt r15, r16 ; subx r5, r6, r7 }
	{ ldnt r15, r16 ; v1mz r5, r6, r7 }
	{ ldnt r15, r16 ; v2packuc r5, r6, r7 }
	{ ldnt1s r15, r16 ; cmoveqz r5, r6, r7 }
	{ ldnt1s r15, r16 ; fsingle_sub1 r5, r6, r7 }
	{ ldnt1s r15, r16 ; shl r5, r6, r7 }
	{ ldnt1s r15, r16 ; v1ddotpua r5, r6, r7 }
	{ ldnt1s r15, r16 ; v2cmpltsi r5, r6, 5 }
	{ ldnt1s r15, r16 ; v4shrs r5, r6, r7 }
	{ ldnt1s_add r15, r16, 5 ; dblalign r5, r6, r7 }
	{ ldnt1s_add r15, r16, 5 ; mula_hs_lu r5, r6, r7 }
	{ ldnt1s_add r15, r16, 5 ; tblidxb0 r5, r6 }
	{ ldnt1s_add r15, r16, 5 ; v1sadu r5, r6, r7 }
	{ ldnt1s_add r15, r16, 5 ; v2sadau r5, r6, r7 }
	{ ldnt1u r15, r16 ; cmpeq r5, r6, r7 }
	{ ldnt1u r15, r16 ; infol 0x1234 }
	{ ldnt1u r15, r16 ; shl1add r5, r6, r7 }
	{ ldnt1u r15, r16 ; v1ddotpusa r5, r6, r7 }
	{ ldnt1u r15, r16 ; v2cmpltui r5, r6, 5 }
	{ ldnt1u r15, r16 ; v4sub r5, r6, r7 }
	{ ldnt1u_add r15, r16, 5 ; dblalign4 r5, r6, r7 }
	{ ldnt1u_add r15, r16, 5 ; mula_hu_ls r5, r6, r7 }
	{ ldnt1u_add r15, r16, 5 ; tblidxb2 r5, r6 }
	{ ldnt1u_add r15, r16, 5 ; v1shli r5, r6, 5 }
	{ ldnt1u_add r15, r16, 5 ; v2sadu r5, r6, r7 }
	{ ldnt2s r15, r16 ; cmples r5, r6, r7 }
	{ ldnt2s r15, r16 ; mnz r5, r6, r7 }
	{ ldnt2s r15, r16 ; shl2add r5, r6, r7 }
	{ ldnt2s r15, r16 ; v1dotpa r5, r6, r7 }
	{ ldnt2s r15, r16 ; v2dotp r5, r6, r7 }
	{ ldnt2s r15, r16 ; xor r5, r6, r7 }
	{ ldnt2s_add r15, r16, 5 ; fdouble_add_flags r5, r6, r7 }
	{ ldnt2s_add r15, r16, 5 ; mula_ls_ls r5, r6, r7 }
	{ ldnt2s_add r15, r16, 5 ; v1add r5, r6, r7 }
	{ ldnt2s_add r15, r16, 5 ; v1shrsi r5, r6, 5 }
	{ ldnt2s_add r15, r16, 5 ; v2shli r5, r6, 5 }
	{ ldnt2u r15, r16 ; cmplts r5, r6, r7 }
	{ ldnt2u r15, r16 ; movei r5, 5 }
	{ ldnt2u r15, r16 ; shl3add r5, r6, r7 }
	{ ldnt2u r15, r16 ; v1dotpua r5, r6, r7 }
	{ ldnt2u r15, r16 ; v2int_h r5, r6, r7 }
	{ ldnt2u_add r15, r16, 5 ; add r5, r6, r7 }
	{ ldnt2u_add r15, r16, 5 ; fdouble_mul_flags r5, r6, r7 }
	{ ldnt2u_add r15, r16, 5 ; mula_lu_lu r5, r6, r7 }
	{ ldnt2u_add r15, r16, 5 ; v1adduc r5, r6, r7 }
	{ ldnt2u_add r15, r16, 5 ; v1shrui r5, r6, 5 }
	{ ldnt2u_add r15, r16, 5 ; v2shrs r5, r6, r7 }
	{ ldnt4s r15, r16 ; cmpltu r5, r6, r7 }
	{ ldnt4s r15, r16 ; mul_hs_hs r5, r6, r7 }
	{ ldnt4s r15, r16 ; shli r5, r6, 5 }
	{ ldnt4s r15, r16 ; v1dotpusa r5, r6, r7 }
	{ ldnt4s r15, r16 ; v2maxs r5, r6, r7 }
	{ ldnt4s_add r15, r16, 5 ; addli r5, r6, 0x1234 }
	{ ldnt4s_add r15, r16, 5 ; fdouble_pack2 r5, r6, r7 }
	{ ldnt4s_add r15, r16, 5 ; mulx r5, r6, r7 }
	{ ldnt4s_add r15, r16, 5 ; v1avgu r5, r6, r7 }
	{ ldnt4s_add r15, r16, 5 ; v1subuc r5, r6, r7 }
	{ ldnt4s_add r15, r16, 5 ; v2shru r5, r6, r7 }
	{ ldnt4u r15, r16 ; cmpne r5, r6, r7 }
	{ ldnt4u r15, r16 ; mul_hs_ls r5, r6, r7 }
	{ ldnt4u r15, r16 ; shlxi r5, r6, 5 }
	{ ldnt4u r15, r16 ; v1int_l r5, r6, r7 }
	{ ldnt4u r15, r16 ; v2mins r5, r6, r7 }
	{ ldnt4u_add r15, r16, 5 ; addxi r5, r6, 5 }
	{ ldnt4u_add r15, r16, 5 ; fdouble_unpack_max r5, r6, r7 }
	{ ldnt4u_add r15, r16, 5 ; nop }
	{ ldnt4u_add r15, r16, 5 ; v1cmpeqi r5, r6, 5 }
	{ ldnt4u_add r15, r16, 5 ; v2addi r5, r6, 5 }
	{ ldnt4u_add r15, r16, 5 ; v2sub r5, r6, r7 }
	{ ldnt_add r15, r16, 5 ; cmula r5, r6, r7 }
	{ ldnt_add r15, r16, 5 ; mul_hu_hu r5, r6, r7 }
	{ ldnt_add r15, r16, 5 ; shrsi r5, r6, 5 }
	{ ldnt_add r15, r16, 5 ; v1maxui r5, r6, 5 }
	{ ldnt_add r15, r16, 5 ; v2mnz r5, r6, r7 }
	{ lnk r15 ; add r5, r6, r7 ; ld4u r25, r26 }
	{ lnk r15 ; addx r5, r6, r7 ; prefetch r25 }
	{ lnk r15 ; and r5, r6, r7 ; prefetch r25 }
	{ lnk r15 ; clz r5, r6 ; ld4u r25, r26 }
	{ lnk r15 ; cmovnez r5, r6, r7 ; prefetch_l1 r25 }
	{ lnk r15 ; cmpeqi r5, r6, 5 ; prefetch_l2 r25 }
	{ lnk r15 ; cmpleu r5, r6, r7 ; prefetch_l3 r25 }
	{ lnk r15 ; cmpltsi r5, r6, 5 ; st r25, r26 }
	{ lnk r15 ; cmpne r5, r6, r7 ; st1 r25, r26 }
	{ lnk r15 ; fdouble_pack2 r5, r6, r7 }
	{ lnk r15 ; fsingle_pack1 r5, r6 ; prefetch_l3_fault r25 }
	{ lnk r15 ; ld r25, r26 ; cmpleu r5, r6, r7 }
	{ lnk r15 ; ld r25, r26 ; shrsi r5, r6, 5 }
	{ lnk r15 ; ld1s r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ lnk r15 ; ld1u r25, r26 ; clz r5, r6 }
	{ lnk r15 ; ld1u r25, r26 ; shl2add r5, r6, r7 }
	{ lnk r15 ; ld2s r25, r26 ; movei r5, 5 }
	{ lnk r15 ; ld2u r25, r26 ; add r5, r6, r7 }
	{ lnk r15 ; ld2u r25, r26 ; revbytes r5, r6 }
	{ lnk r15 ; ld4s r25, r26 ; ctz r5, r6 }
	{ lnk r15 ; ld4s r25, r26 ; tblidxb0 r5, r6 }
	{ lnk r15 ; ld4u r25, r26 ; mz r5, r6, r7 }
	{ lnk r15 ; mnz r5, r6, r7 ; prefetch_l2 r25 }
	{ lnk r15 ; movei r5, 5 ; prefetch_l3 r25 }
	{ lnk r15 ; mul_hu_hu r5, r6, r7 ; prefetch_l2 r25 }
	{ lnk r15 ; mul_lu_lu r5, r6, r7 ; prefetch_l1_fault r25 }
	{ lnk r15 ; mula_hu_hu r5, r6, r7 ; prefetch_l1 r25 }
	{ lnk r15 ; mula_lu_lu r5, r6, r7 ; prefetch r25 }
	{ lnk r15 ; mulx r5, r6, r7 ; prefetch_l1_fault r25 }
	{ lnk r15 ; nop ; prefetch_l2_fault r25 }
	{ lnk r15 ; or r5, r6, r7 ; prefetch_l3_fault r25 }
	{ lnk r15 ; prefetch r25 ; cmpltsi r5, r6, 5 }
	{ lnk r15 ; prefetch r25 ; shrui r5, r6, 5 }
	{ lnk r15 ; prefetch_l1 r25 ; mula_lu_lu r5, r6, r7 }
	{ lnk r15 ; prefetch_l1_fault r25 ; cmovnez r5, r6, r7 }
	{ lnk r15 ; prefetch_l1_fault r25 ; shl3add r5, r6, r7 }
	{ lnk r15 ; prefetch_l2 r25 ; mul_hu_hu r5, r6, r7 }
	{ lnk r15 ; prefetch_l2_fault r25 ; addx r5, r6, r7 }
	{ lnk r15 ; prefetch_l2_fault r25 ; rotli r5, r6, 5 }
	{ lnk r15 ; prefetch_l3 r25 ; fsingle_pack1 r5, r6 }
	{ lnk r15 ; prefetch_l3 r25 ; tblidxb2 r5, r6 }
	{ lnk r15 ; prefetch_l3_fault r25 ; nor r5, r6, r7 }
	{ lnk r15 ; revbits r5, r6 ; prefetch_l3_fault r25 }
	{ lnk r15 ; rotl r5, r6, r7 ; st1 r25, r26 }
	{ lnk r15 ; shl r5, r6, r7 ; st4 r25, r26 }
	{ lnk r15 ; shl1addx r5, r6, r7 }
	{ lnk r15 ; shl3add r5, r6, r7 ; ld1s r25, r26 }
	{ lnk r15 ; shli r5, r6, 5 ; ld2s r25, r26 }
	{ lnk r15 ; shrsi r5, r6, 5 ; ld2s r25, r26 }
	{ lnk r15 ; shrui r5, r6, 5 ; ld4s r25, r26 }
	{ lnk r15 ; st r25, r26 ; movei r5, 5 }
	{ lnk r15 ; st1 r25, r26 ; add r5, r6, r7 }
	{ lnk r15 ; st1 r25, r26 ; revbytes r5, r6 }
	{ lnk r15 ; st2 r25, r26 ; ctz r5, r6 }
	{ lnk r15 ; st2 r25, r26 ; tblidxb0 r5, r6 }
	{ lnk r15 ; st4 r25, r26 ; mz r5, r6, r7 }
	{ lnk r15 ; sub r5, r6, r7 ; prefetch_l2_fault r25 }
	{ lnk r15 ; tblidxb0 r5, r6 ; prefetch_l3 r25 }
	{ lnk r15 ; tblidxb2 r5, r6 ; st r25, r26 }
	{ lnk r15 ; v1ddotpus r5, r6, r7 }
	{ lnk r15 ; v2cmpltu r5, r6, r7 }
	{ lnk r15 ; v4shru r5, r6, r7 }
	{ mf ; cmples r5, r6, r7 }
	{ mf ; mnz r5, r6, r7 }
	{ mf ; shl2add r5, r6, r7 }
	{ mf ; v1dotpa r5, r6, r7 }
	{ mf ; v2dotp r5, r6, r7 }
	{ mf ; xor r5, r6, r7 }
	{ mfspr r16, 0x5 ; fdouble_add_flags r5, r6, r7 }
	{ mfspr r16, 0x5 ; mula_ls_ls r5, r6, r7 }
	{ mfspr r16, 0x5 ; v1add r5, r6, r7 }
	{ mfspr r16, 0x5 ; v1shrsi r5, r6, 5 }
	{ mfspr r16, 0x5 ; v2shli r5, r6, 5 }
	{ mm r5, r6, 5, 7 ; cmpne r15, r16, r17 }
	{ mm r5, r6, 5, 7 ; ld4u r15, r16 }
	{ mm r5, r6, 5, 7 ; prefetch_l1_fault r15 }
	{ mm r5, r6, 5, 7 ; stnt_add r15, r16, 5 }
	{ mm r5, r6, 5, 7 ; v2cmpltsi r15, r16, 5 }
	{ mnz r15, r16, r17 ; add r5, r6, r7 ; ld1u r25, r26 }
	{ mnz r15, r16, r17 ; addx r5, r6, r7 ; ld2s r25, r26 }
	{ mnz r15, r16, r17 ; and r5, r6, r7 ; ld2s r25, r26 }
	{ mnz r15, r16, r17 ; clz r5, r6 ; ld1u r25, r26 }
	{ mnz r15, r16, r17 ; cmovnez r5, r6, r7 ; ld2u r25, r26 }
	{ mnz r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld4u r25, r26 }
	{ mnz r15, r16, r17 ; cmpleu r5, r6, r7 ; prefetch_l1 r25 }
	{ mnz r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
	{ mnz r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
	{ mnz r15, r16, r17 ; fdouble_add_flags r5, r6, r7 }
	{ mnz r15, r16, r17 ; fsingle_pack1 r5, r6 ; prefetch_l1_fault r25 }
	{ mnz r15, r16, r17 ; ld r25, r26 ; cmovnez r5, r6, r7 }
	{ mnz r15, r16, r17 ; ld r25, r26 ; shl3add r5, r6, r7 }
	{ mnz r15, r16, r17 ; ld1s r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ mnz r15, r16, r17 ; ld1u r25, r26 ; addx r5, r6, r7 }
	{ mnz r15, r16, r17 ; ld1u r25, r26 ; rotli r5, r6, 5 }
	{ mnz r15, r16, r17 ; ld2s r25, r26 ; fsingle_pack1 r5, r6 }
	{ mnz r15, r16, r17 ; ld2s r25, r26 ; tblidxb2 r5, r6 }
	{ mnz r15, r16, r17 ; ld2u r25, r26 ; nor r5, r6, r7 }
	{ mnz r15, r16, r17 ; ld4s r25, r26 ; cmplts r5, r6, r7 }
	{ mnz r15, r16, r17 ; ld4s r25, r26 ; shru r5, r6, r7 }
	{ mnz r15, r16, r17 ; ld4u r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ mnz r15, r16, r17 ; mnz r5, r6, r7 ; ld4u r25, r26 }
	{ mnz r15, r16, r17 ; movei r5, 5 ; prefetch_l1 r25 }
	{ mnz r15, r16, r17 ; mul_hu_hu r5, r6, r7 ; ld4u r25, r26 }
	{ mnz r15, r16, r17 ; mul_lu_lu r5, r6, r7 ; ld4s r25, r26 }
	{ mnz r15, r16, r17 ; mula_hu_hu r5, r6, r7 ; ld2u r25, r26 }
	{ mnz r15, r16, r17 ; mula_lu_lu r5, r6, r7 ; ld2s r25, r26 }
	{ mnz r15, r16, r17 ; mulx r5, r6, r7 ; ld4s r25, r26 }
	{ mnz r15, r16, r17 ; nop ; prefetch r25 }
	{ mnz r15, r16, r17 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
	{ mnz r15, r16, r17 ; prefetch r25 ; cmpeqi r5, r6, 5 }
	{ mnz r15, r16, r17 ; prefetch r25 ; shli r5, r6, 5 }
	{ mnz r15, r16, r17 ; prefetch_l1 r25 ; mul_lu_lu r5, r6, r7 }
	{ mnz r15, r16, r17 ; prefetch_l1_fault r25 ; and r5, r6, r7 }
	{ mnz r15, r16, r17 ; prefetch_l1_fault r25 ; shl1add r5, r6, r7 }
	{ mnz r15, r16, r17 ; prefetch_l2 r25 ; mnz r5, r6, r7 }
	{ mnz r15, r16, r17 ; prefetch_l2 r25 ; xor r5, r6, r7 }
	{ mnz r15, r16, r17 ; prefetch_l2_fault r25 ; pcnt r5, r6 }
	{ mnz r15, r16, r17 ; prefetch_l3 r25 ; cmpltu r5, r6, r7 }
	{ mnz r15, r16, r17 ; prefetch_l3 r25 ; sub r5, r6, r7 }
	{ mnz r15, r16, r17 ; prefetch_l3_fault r25 ; mulax r5, r6, r7 }
	{ mnz r15, r16, r17 ; revbits r5, r6 ; prefetch_l1_fault r25 }
	{ mnz r15, r16, r17 ; rotl r5, r6, r7 ; prefetch_l2_fault r25 }
	{ mnz r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l3_fault r25 }
	{ mnz r15, r16, r17 ; shl1addx r5, r6, r7 ; st r25, r26 }
	{ mnz r15, r16, r17 ; shl2addx r5, r6, r7 ; st2 r25, r26 }
	{ mnz r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ mnz r15, r16, r17 ; shrs r5, r6, r7 }
	{ mnz r15, r16, r17 ; shrui r5, r6, 5 ; ld1s r25, r26 }
	{ mnz r15, r16, r17 ; st r25, r26 ; fsingle_pack1 r5, r6 }
	{ mnz r15, r16, r17 ; st r25, r26 ; tblidxb2 r5, r6 }
	{ mnz r15, r16, r17 ; st1 r25, r26 ; nor r5, r6, r7 }
	{ mnz r15, r16, r17 ; st2 r25, r26 ; cmplts r5, r6, r7 }
	{ mnz r15, r16, r17 ; st2 r25, r26 ; shru r5, r6, r7 }
	{ mnz r15, r16, r17 ; st4 r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ mnz r15, r16, r17 ; sub r5, r6, r7 ; prefetch r25 }
	{ mnz r15, r16, r17 ; tblidxb0 r5, r6 ; prefetch_l1 r25 }
	{ mnz r15, r16, r17 ; tblidxb2 r5, r6 ; prefetch_l2 r25 }
	{ mnz r15, r16, r17 ; v1cmpltui r5, r6, 5 }
	{ mnz r15, r16, r17 ; v2cmples r5, r6, r7 }
	{ mnz r15, r16, r17 ; v4packsc r5, r6, r7 }
	{ mnz r5, r6, r7 ; add r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mnz r5, r6, r7 ; addx r15, r16, r17 ; st r25, r26 }
	{ mnz r5, r6, r7 ; and r15, r16, r17 ; st r25, r26 }
	{ mnz r5, r6, r7 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
	{ mnz r5, r6, r7 ; cmples r15, r16, r17 ; st2 r25, r26 }
	{ mnz r5, r6, r7 ; cmplts r15, r16, r17 }
	{ mnz r5, r6, r7 ; cmpne r15, r16, r17 ; ld r25, r26 }
	{ mnz r5, r6, r7 ; fnop ; ld2u r25, r26 }
	{ mnz r5, r6, r7 ; info 19 ; ld4s r25, r26 }
	{ mnz r5, r6, r7 ; jalrp r15 ; ld2u r25, r26 }
	{ mnz r5, r6, r7 ; jrp r15 ; ld4u r25, r26 }
	{ mnz r5, r6, r7 ; ld r25, r26 ; nop }
	{ mnz r5, r6, r7 ; ld1s r25, r26 ; jalrp r15 }
	{ mnz r5, r6, r7 ; ld1u r25, r26 ; cmpleu r15, r16, r17 }
	{ mnz r5, r6, r7 ; ld2s r25, r26 ; add r15, r16, r17 }
	{ mnz r5, r6, r7 ; ld2s r25, r26 ; shrsi r15, r16, 5 }
	{ mnz r5, r6, r7 ; ld2u r25, r26 ; shl r15, r16, r17 }
	{ mnz r5, r6, r7 ; ld4s r25, r26 ; mnz r15, r16, r17 }
	{ mnz r5, r6, r7 ; ld4u r25, r26 ; cmpne r15, r16, r17 }
	{ mnz r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
	{ mnz r5, r6, r7 ; mnz r15, r16, r17 ; prefetch r25 }
	{ mnz r5, r6, r7 ; movei r15, 5 ; prefetch_l1_fault r25 }
	{ mnz r5, r6, r7 ; nop ; prefetch_l1_fault r25 }
	{ mnz r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
	{ mnz r5, r6, r7 ; prefetch r25 ; rotli r15, r16, 5 }
	{ mnz r5, r6, r7 ; prefetch_l1 r25 ; info 19 }
	{ mnz r5, r6, r7 ; prefetch_l1_fault r25 ; cmples r15, r16, r17 }
	{ mnz r5, r6, r7 ; prefetch_l2 r25 ; add r15, r16, r17 }
	{ mnz r5, r6, r7 ; prefetch_l2 r25 ; shrsi r15, r16, 5 }
	{ mnz r5, r6, r7 ; prefetch_l2_fault r25 ; shl1add r15, r16, r17 }
	{ mnz r5, r6, r7 ; prefetch_l3 r25 ; movei r15, 5 }
	{ mnz r5, r6, r7 ; prefetch_l3_fault r25 ; info 19 }
	{ mnz r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l1 r25 }
	{ mnz r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2 r25 }
	{ mnz r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
	{ mnz r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mnz r5, r6, r7 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
	{ mnz r5, r6, r7 ; shrs r15, r16, r17 ; st1 r25, r26 }
	{ mnz r5, r6, r7 ; shru r15, r16, r17 ; st4 r25, r26 }
	{ mnz r5, r6, r7 ; st r25, r26 ; info 19 }
	{ mnz r5, r6, r7 ; st1 r25, r26 ; cmples r15, r16, r17 }
	{ mnz r5, r6, r7 ; st2 r15, r16 }
	{ mnz r5, r6, r7 ; st2 r25, r26 ; shrs r15, r16, r17 }
	{ mnz r5, r6, r7 ; st4 r25, r26 ; rotli r15, r16, 5 }
	{ mnz r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mnz r5, r6, r7 ; v1maxu r15, r16, r17 }
	{ mnz r5, r6, r7 ; v2shrs r15, r16, r17 }
	{ move r15, r16 ; add r5, r6, r7 ; ld1u r25, r26 }
	{ move r15, r16 ; addx r5, r6, r7 ; ld2s r25, r26 }
	{ move r15, r16 ; and r5, r6, r7 ; ld2s r25, r26 }
	{ move r15, r16 ; clz r5, r6 ; ld1u r25, r26 }
	{ move r15, r16 ; cmovnez r5, r6, r7 ; ld2u r25, r26 }
	{ move r15, r16 ; cmpeqi r5, r6, 5 ; ld4u r25, r26 }
	{ move r15, r16 ; cmpleu r5, r6, r7 ; prefetch_l1 r25 }
	{ move r15, r16 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
	{ move r15, r16 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
	{ move r15, r16 ; fdouble_add_flags r5, r6, r7 }
	{ move r15, r16 ; fsingle_pack1 r5, r6 ; prefetch_l1_fault r25 }
	{ move r15, r16 ; ld r25, r26 ; cmovnez r5, r6, r7 }
	{ move r15, r16 ; ld r25, r26 ; shl3add r5, r6, r7 }
	{ move r15, r16 ; ld1s r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ move r15, r16 ; ld1u r25, r26 ; addx r5, r6, r7 }
	{ move r15, r16 ; ld1u r25, r26 ; rotli r5, r6, 5 }
	{ move r15, r16 ; ld2s r25, r26 ; fsingle_pack1 r5, r6 }
	{ move r15, r16 ; ld2s r25, r26 ; tblidxb2 r5, r6 }
	{ move r15, r16 ; ld2u r25, r26 ; nor r5, r6, r7 }
	{ move r15, r16 ; ld4s r25, r26 ; cmplts r5, r6, r7 }
	{ move r15, r16 ; ld4s r25, r26 ; shru r5, r6, r7 }
	{ move r15, r16 ; ld4u r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ move r15, r16 ; mnz r5, r6, r7 ; ld4u r25, r26 }
	{ move r15, r16 ; movei r5, 5 ; prefetch_l1 r25 }
	{ move r15, r16 ; mul_hu_hu r5, r6, r7 ; ld4u r25, r26 }
	{ move r15, r16 ; mul_lu_lu r5, r6, r7 ; ld4s r25, r26 }
	{ move r15, r16 ; mula_hu_hu r5, r6, r7 ; ld2u r25, r26 }
	{ move r15, r16 ; mula_lu_lu r5, r6, r7 ; ld2s r25, r26 }
	{ move r15, r16 ; mulx r5, r6, r7 ; ld4s r25, r26 }
	{ move r15, r16 ; nop ; prefetch r25 }
	{ move r15, r16 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
	{ move r15, r16 ; prefetch r25 ; cmpeqi r5, r6, 5 }
	{ move r15, r16 ; prefetch r25 ; shli r5, r6, 5 }
	{ move r15, r16 ; prefetch_l1 r25 ; mul_lu_lu r5, r6, r7 }
	{ move r15, r16 ; prefetch_l1_fault r25 ; and r5, r6, r7 }
	{ move r15, r16 ; prefetch_l1_fault r25 ; shl1add r5, r6, r7 }
	{ move r15, r16 ; prefetch_l2 r25 ; mnz r5, r6, r7 }
	{ move r15, r16 ; prefetch_l2 r25 ; xor r5, r6, r7 }
	{ move r15, r16 ; prefetch_l2_fault r25 ; pcnt r5, r6 }
	{ move r15, r16 ; prefetch_l3 r25 ; cmpltu r5, r6, r7 }
	{ move r15, r16 ; prefetch_l3 r25 ; sub r5, r6, r7 }
	{ move r15, r16 ; prefetch_l3_fault r25 ; mulax r5, r6, r7 }
	{ move r15, r16 ; revbits r5, r6 ; prefetch_l1_fault r25 }
	{ move r15, r16 ; rotl r5, r6, r7 ; prefetch_l2_fault r25 }
	{ move r15, r16 ; shl r5, r6, r7 ; prefetch_l3_fault r25 }
	{ move r15, r16 ; shl1addx r5, r6, r7 ; st r25, r26 }
	{ move r15, r16 ; shl2addx r5, r6, r7 ; st2 r25, r26 }
	{ move r15, r16 ; shl3addx r5, r6, r7 }
	{ move r15, r16 ; shrs r5, r6, r7 }
	{ move r15, r16 ; shrui r5, r6, 5 ; ld1s r25, r26 }
	{ move r15, r16 ; st r25, r26 ; fsingle_pack1 r5, r6 }
	{ move r15, r16 ; st r25, r26 ; tblidxb2 r5, r6 }
	{ move r15, r16 ; st1 r25, r26 ; nor r5, r6, r7 }
	{ move r15, r16 ; st2 r25, r26 ; cmplts r5, r6, r7 }
	{ move r15, r16 ; st2 r25, r26 ; shru r5, r6, r7 }
	{ move r15, r16 ; st4 r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ move r15, r16 ; sub r5, r6, r7 ; prefetch r25 }
	{ move r15, r16 ; tblidxb0 r5, r6 ; prefetch_l1 r25 }
	{ move r15, r16 ; tblidxb2 r5, r6 ; prefetch_l2 r25 }
	{ move r15, r16 ; v1cmpltui r5, r6, 5 }
	{ move r15, r16 ; v2cmples r5, r6, r7 }
	{ move r15, r16 ; v4packsc r5, r6, r7 }
	{ move r5, r6 ; add r15, r16, r17 ; prefetch_l3_fault r25 }
	{ move r5, r6 ; addx r15, r16, r17 ; st r25, r26 }
	{ move r5, r6 ; and r15, r16, r17 ; st r25, r26 }
	{ move r5, r6 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
	{ move r5, r6 ; cmples r15, r16, r17 ; st2 r25, r26 }
	{ move r5, r6 ; cmplts r15, r16, r17 }
	{ move r5, r6 ; cmpne r15, r16, r17 ; ld r25, r26 }
	{ move r5, r6 ; fnop ; ld2u r25, r26 }
	{ move r5, r6 ; info 19 ; ld4s r25, r26 }
	{ move r5, r6 ; jalrp r15 ; ld2u r25, r26 }
	{ move r5, r6 ; jrp r15 ; ld4u r25, r26 }
	{ move r5, r6 ; ld r25, r26 ; nop }
	{ move r5, r6 ; ld1s r25, r26 ; jalrp r15 }
	{ move r5, r6 ; ld1u r25, r26 ; cmpleu r15, r16, r17 }
	{ move r5, r6 ; ld2s r25, r26 ; add r15, r16, r17 }
	{ move r5, r6 ; ld2s r25, r26 ; shrsi r15, r16, 5 }
	{ move r5, r6 ; ld2u r25, r26 ; shl r15, r16, r17 }
	{ move r5, r6 ; ld4s r25, r26 ; mnz r15, r16, r17 }
	{ move r5, r6 ; ld4u r25, r26 ; cmpne r15, r16, r17 }
	{ move r5, r6 ; ldnt1s_add r15, r16, 5 }
	{ move r5, r6 ; mnz r15, r16, r17 ; prefetch r25 }
	{ move r5, r6 ; movei r15, 5 ; prefetch_l1_fault r25 }
	{ move r5, r6 ; nop ; prefetch_l1_fault r25 }
	{ move r5, r6 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
	{ move r5, r6 ; prefetch r25 ; rotli r15, r16, 5 }
	{ move r5, r6 ; prefetch_l1 r25 ; info 19 }
	{ move r5, r6 ; prefetch_l1_fault r25 ; cmples r15, r16, r17 }
	{ move r5, r6 ; prefetch_l2 r25 ; add r15, r16, r17 }
	{ move r5, r6 ; prefetch_l2 r25 ; shrsi r15, r16, 5 }
	{ move r5, r6 ; prefetch_l2_fault r25 ; shl1add r15, r16, r17 }
	{ move r5, r6 ; prefetch_l3 r25 ; movei r15, 5 }
	{ move r5, r6 ; prefetch_l3_fault r25 ; info 19 }
	{ move r5, r6 ; rotl r15, r16, r17 ; prefetch_l1 r25 }
	{ move r5, r6 ; shl r15, r16, r17 ; prefetch_l2 r25 }
	{ move r5, r6 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
	{ move r5, r6 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
	{ move r5, r6 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
	{ move r5, r6 ; shrs r15, r16, r17 ; st1 r25, r26 }
	{ move r5, r6 ; shru r15, r16, r17 ; st4 r25, r26 }
	{ move r5, r6 ; st r25, r26 ; info 19 }
	{ move r5, r6 ; st1 r25, r26 ; cmples r15, r16, r17 }
	{ move r5, r6 ; st2 r15, r16 }
	{ move r5, r6 ; st2 r25, r26 ; shrs r15, r16, r17 }
	{ move r5, r6 ; st4 r25, r26 ; rotli r15, r16, 5 }
	{ move r5, r6 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
	{ move r5, r6 ; v1maxu r15, r16, r17 }
	{ move r5, r6 ; v2shrs r15, r16, r17 }
	{ movei r15, 5 ; add r5, r6, r7 ; ld1u r25, r26 }
	{ movei r15, 5 ; addx r5, r6, r7 ; ld2s r25, r26 }
	{ movei r15, 5 ; and r5, r6, r7 ; ld2s r25, r26 }
	{ movei r15, 5 ; clz r5, r6 ; ld1u r25, r26 }
	{ movei r15, 5 ; cmovnez r5, r6, r7 ; ld2u r25, r26 }
	{ movei r15, 5 ; cmpeqi r5, r6, 5 ; ld4u r25, r26 }
	{ movei r15, 5 ; cmpleu r5, r6, r7 ; prefetch_l1 r25 }
	{ movei r15, 5 ; cmpltsi r5, r6, 5 ; prefetch_l2 r25 }
	{ movei r15, 5 ; cmpne r5, r6, r7 ; prefetch_l2_fault r25 }
	{ movei r15, 5 ; fdouble_add_flags r5, r6, r7 }
	{ movei r15, 5 ; fsingle_pack1 r5, r6 ; prefetch_l1_fault r25 }
	{ movei r15, 5 ; ld r25, r26 ; cmovnez r5, r6, r7 }
	{ movei r15, 5 ; ld r25, r26 ; shl3add r5, r6, r7 }
	{ movei r15, 5 ; ld1s r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ movei r15, 5 ; ld1u r25, r26 ; addx r5, r6, r7 }
	{ movei r15, 5 ; ld1u r25, r26 ; rotli r5, r6, 5 }
	{ movei r15, 5 ; ld2s r25, r26 ; fsingle_pack1 r5, r6 }
	{ movei r15, 5 ; ld2s r25, r26 ; tblidxb2 r5, r6 }
	{ movei r15, 5 ; ld2u r25, r26 ; nor r5, r6, r7 }
	{ movei r15, 5 ; ld4s r25, r26 ; cmplts r5, r6, r7 }
	{ movei r15, 5 ; ld4s r25, r26 ; shru r5, r6, r7 }
	{ movei r15, 5 ; ld4u r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ movei r15, 5 ; mnz r5, r6, r7 ; ld4u r25, r26 }
	{ movei r15, 5 ; movei r5, 5 ; prefetch_l1 r25 }
	{ movei r15, 5 ; mul_hu_hu r5, r6, r7 ; ld4u r25, r26 }
	{ movei r15, 5 ; mul_lu_lu r5, r6, r7 ; ld4s r25, r26 }
	{ movei r15, 5 ; mula_hu_hu r5, r6, r7 ; ld2u r25, r26 }
	{ movei r15, 5 ; mula_lu_lu r5, r6, r7 ; ld2s r25, r26 }
	{ movei r15, 5 ; mulx r5, r6, r7 ; ld4s r25, r26 }
	{ movei r15, 5 ; nop ; prefetch r25 }
	{ movei r15, 5 ; or r5, r6, r7 ; prefetch_l1_fault r25 }
	{ movei r15, 5 ; prefetch r25 ; cmpeqi r5, r6, 5 }
	{ movei r15, 5 ; prefetch r25 ; shli r5, r6, 5 }
	{ movei r15, 5 ; prefetch_l1 r25 ; mul_lu_lu r5, r6, r7 }
	{ movei r15, 5 ; prefetch_l1_fault r25 ; and r5, r6, r7 }
	{ movei r15, 5 ; prefetch_l1_fault r25 ; shl1add r5, r6, r7 }
	{ movei r15, 5 ; prefetch_l2 r25 ; mnz r5, r6, r7 }
	{ movei r15, 5 ; prefetch_l2 r25 ; xor r5, r6, r7 }
	{ movei r15, 5 ; prefetch_l2_fault r25 ; pcnt r5, r6 }
	{ movei r15, 5 ; prefetch_l3 r25 ; cmpltu r5, r6, r7 }
	{ movei r15, 5 ; prefetch_l3 r25 ; sub r5, r6, r7 }
	{ movei r15, 5 ; prefetch_l3_fault r25 ; mulax r5, r6, r7 }
	{ movei r15, 5 ; revbits r5, r6 ; prefetch_l1_fault r25 }
	{ movei r15, 5 ; rotl r5, r6, r7 ; prefetch_l2_fault r25 }
	{ movei r15, 5 ; shl r5, r6, r7 ; prefetch_l3_fault r25 }
	{ movei r15, 5 ; shl1addx r5, r6, r7 ; st r25, r26 }
	{ movei r15, 5 ; shl2addx r5, r6, r7 ; st2 r25, r26 }
	{ movei r15, 5 ; shl3addx r5, r6, r7 }
	{ movei r15, 5 ; shrs r5, r6, r7 }
	{ movei r15, 5 ; shrui r5, r6, 5 ; ld1s r25, r26 }
	{ movei r15, 5 ; st r25, r26 ; fsingle_pack1 r5, r6 }
	{ movei r15, 5 ; st r25, r26 ; tblidxb2 r5, r6 }
	{ movei r15, 5 ; st1 r25, r26 ; nor r5, r6, r7 }
	{ movei r15, 5 ; st2 r25, r26 ; cmplts r5, r6, r7 }
	{ movei r15, 5 ; st2 r25, r26 ; shru r5, r6, r7 }
	{ movei r15, 5 ; st4 r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ movei r15, 5 ; sub r5, r6, r7 ; prefetch r25 }
	{ movei r15, 5 ; tblidxb0 r5, r6 ; prefetch_l1 r25 }
	{ movei r15, 5 ; tblidxb2 r5, r6 ; prefetch_l2 r25 }
	{ movei r15, 5 ; v1cmpltui r5, r6, 5 }
	{ movei r15, 5 ; v2cmples r5, r6, r7 }
	{ movei r15, 5 ; v4packsc r5, r6, r7 }
	{ movei r5, 5 ; add r15, r16, r17 ; prefetch_l3_fault r25 }
	{ movei r5, 5 ; addx r15, r16, r17 ; st r25, r26 }
	{ movei r5, 5 ; and r15, r16, r17 ; st r25, r26 }
	{ movei r5, 5 ; cmpeq r15, r16, r17 ; st2 r25, r26 }
	{ movei r5, 5 ; cmples r15, r16, r17 ; st2 r25, r26 }
	{ movei r5, 5 ; cmplts r15, r16, r17 }
	{ movei r5, 5 ; cmpne r15, r16, r17 ; ld r25, r26 }
	{ movei r5, 5 ; fnop ; ld2u r25, r26 }
	{ movei r5, 5 ; info 19 ; ld4s r25, r26 }
	{ movei r5, 5 ; jalrp r15 ; ld2u r25, r26 }
	{ movei r5, 5 ; jrp r15 ; ld4u r25, r26 }
	{ movei r5, 5 ; ld r25, r26 ; nop }
	{ movei r5, 5 ; ld1s r25, r26 ; jalrp r15 }
	{ movei r5, 5 ; ld1u r25, r26 ; cmpleu r15, r16, r17 }
	{ movei r5, 5 ; ld2s r25, r26 ; add r15, r16, r17 }
	{ movei r5, 5 ; ld2s r25, r26 ; shrsi r15, r16, 5 }
	{ movei r5, 5 ; ld2u r25, r26 ; shl r15, r16, r17 }
	{ movei r5, 5 ; ld4s r25, r26 ; mnz r15, r16, r17 }
	{ movei r5, 5 ; ld4u r25, r26 ; cmpne r15, r16, r17 }
	{ movei r5, 5 ; ldnt1s_add r15, r16, 5 }
	{ movei r5, 5 ; mnz r15, r16, r17 ; prefetch r25 }
	{ movei r5, 5 ; movei r15, 5 ; prefetch_l1_fault r25 }
	{ movei r5, 5 ; nop ; prefetch_l1_fault r25 }
	{ movei r5, 5 ; or r15, r16, r17 ; prefetch_l2_fault r25 }
	{ movei r5, 5 ; prefetch r25 ; rotli r15, r16, 5 }
	{ movei r5, 5 ; prefetch_l1 r25 ; info 19 }
	{ movei r5, 5 ; prefetch_l1_fault r25 ; cmples r15, r16, r17 }
	{ movei r5, 5 ; prefetch_l2 r25 ; add r15, r16, r17 }
	{ movei r5, 5 ; prefetch_l2 r25 ; shrsi r15, r16, 5 }
	{ movei r5, 5 ; prefetch_l2_fault r25 ; shl1add r15, r16, r17 }
	{ movei r5, 5 ; prefetch_l3 r25 ; movei r15, 5 }
	{ movei r5, 5 ; prefetch_l3_fault r25 ; info 19 }
	{ movei r5, 5 ; rotl r15, r16, r17 ; prefetch_l1 r25 }
	{ movei r5, 5 ; shl r15, r16, r17 ; prefetch_l2 r25 }
	{ movei r5, 5 ; shl1addx r15, r16, r17 ; prefetch_l2_fault r25 }
	{ movei r5, 5 ; shl2addx r15, r16, r17 ; prefetch_l3_fault r25 }
	{ movei r5, 5 ; shl3addx r15, r16, r17 ; st1 r25, r26 }
	{ movei r5, 5 ; shrs r15, r16, r17 ; st1 r25, r26 }
	{ movei r5, 5 ; shru r15, r16, r17 ; st4 r25, r26 }
	{ movei r5, 5 ; st r25, r26 ; info 19 }
	{ movei r5, 5 ; st1 r25, r26 ; cmples r15, r16, r17 }
	{ movei r5, 5 ; st2 r15, r16 }
	{ movei r5, 5 ; st2 r25, r26 ; shrs r15, r16, r17 }
	{ movei r5, 5 ; st4 r25, r26 ; rotli r15, r16, 5 }
	{ movei r5, 5 ; sub r15, r16, r17 ; prefetch_l3_fault r25 }
	{ movei r5, 5 ; v1maxu r15, r16, r17 }
	{ movei r5, 5 ; v2shrs r15, r16, r17 }
	{ moveli r15, 0x1234 ; addli r5, r6, 0x1234 }
	{ moveli r15, 0x1234 ; fdouble_pack2 r5, r6, r7 }
	{ moveli r15, 0x1234 ; mulx r5, r6, r7 }
	{ moveli r15, 0x1234 ; v1avgu r5, r6, r7 }
	{ moveli r15, 0x1234 ; v1subuc r5, r6, r7 }
	{ moveli r15, 0x1234 ; v2shru r5, r6, r7 }
	{ moveli r5, 0x1234 ; dtlbpr r15 }
	{ moveli r5, 0x1234 ; ldna_add r15, r16, 5 }
	{ moveli r5, 0x1234 ; prefetch_l3_fault r15 }
	{ moveli r5, 0x1234 ; v1add r15, r16, r17 }
	{ moveli r5, 0x1234 ; v2int_h r15, r16, r17 }
	{ mtspr 0x5, r16 ; addxsc r5, r6, r7 }
	{ mtspr 0x5, r16 ; fnop }
	{ mtspr 0x5, r16 ; or r5, r6, r7 }
	{ mtspr 0x5, r16 ; v1cmpleu r5, r6, r7 }
	{ mtspr 0x5, r16 ; v2adiffs r5, r6, r7 }
	{ mtspr 0x5, r16 ; v4add r5, r6, r7 }
	{ mul_hs_hs r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l1 r25 }
	{ mul_hs_hs r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l1_fault r25 }
	{ mul_hs_hs r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l1_fault r25 }
	{ mul_hs_hs r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2_fault r25 }
	{ mul_hs_hs r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2_fault r25 }
	{ mul_hs_hs r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ mul_hs_hs r5, r6, r7 ; cmpne r15, r16, r17 ; st r25, r26 }
	{ mul_hs_hs r5, r6, r7 ; fnop }
	{ mul_hs_hs r5, r6, r7 ; infol 0x1234 }
	{ mul_hs_hs r5, r6, r7 ; jalrp r15 }
	{ mul_hs_hs r5, r6, r7 ; ld r25, r26 ; add r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; ld r25, r26 ; shrsi r15, r16, 5 }
	{ mul_hs_hs r5, r6, r7 ; ld1s r25, r26 ; shl1add r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; ld1u r25, r26 ; move r15, r16 }
	{ mul_hs_hs r5, r6, r7 ; ld2s r25, r26 ; fnop }
	{ mul_hs_hs r5, r6, r7 ; ld2u r25, r26 ; andi r15, r16, 5 }
	{ mul_hs_hs r5, r6, r7 ; ld2u r25, r26 ; xor r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; ld4s r25, r26 ; shl3add r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; ld4u r25, r26 ; nor r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; lnk r15 ; ld1u r25, r26 }
	{ mul_hs_hs r5, r6, r7 ; move r15, r16 ; ld1u r25, r26 }
	{ mul_hs_hs r5, r6, r7 ; mz r15, r16, r17 ; ld1u r25, r26 }
	{ mul_hs_hs r5, r6, r7 ; nor r15, r16, r17 ; ld2u r25, r26 }
	{ mul_hs_hs r5, r6, r7 ; prefetch r25 ; and r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; prefetch r25 ; subx r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; prefetch_l1 r25 ; rotli r15, r16, 5 }
	{ mul_hs_hs r5, r6, r7 ; prefetch_l1_fault r25 ; mnz r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 ; fnop }
	{ mul_hs_hs r5, r6, r7 ; prefetch_l2_fault r25 ; cmpeq r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; prefetch_l2_fault r25 }
	{ mul_hs_hs r5, r6, r7 ; prefetch_l3 r25 ; shli r15, r16, 5 }
	{ mul_hs_hs r5, r6, r7 ; prefetch_l3_fault r25 ; rotli r15, r16, 5 }
	{ mul_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; ld2s r25, r26 }
	{ mul_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 ; ld2u r25, r26 }
	{ mul_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 ; ld4u r25, r26 }
	{ mul_hs_hs r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l1 r25 }
	{ mul_hs_hs r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l2 r25 }
	{ mul_hs_hs r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l2 r25 }
	{ mul_hs_hs r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l3 r25 }
	{ mul_hs_hs r5, r6, r7 ; st r25, r26 ; rotli r15, r16, 5 }
	{ mul_hs_hs r5, r6, r7 ; st1 r25, r26 ; mnz r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; st2 r25, r26 ; cmpne r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; st4 r25, r26 ; and r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; st4 r25, r26 ; subx r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l1 r25 }
	{ mul_hs_hs r5, r6, r7 ; v2add r15, r16, r17 }
	{ mul_hs_hs r5, r6, r7 ; v4shru r15, r16, r17 }
	{ mul_hs_hu r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ mul_hs_hu r5, r6, r7 ; ld2u_add r15, r16, 5 }
	{ mul_hs_hu r5, r6, r7 ; prefetch_add_l3 r15, 5 }
	{ mul_hs_hu r5, r6, r7 ; stnt2_add r15, r16, 5 }
	{ mul_hs_hu r5, r6, r7 ; v2cmples r15, r16, r17 }
	{ mul_hs_hu r5, r6, r7 ; xori r15, r16, 5 }
	{ mul_hs_ls r5, r6, r7 ; ill }
	{ mul_hs_ls r5, r6, r7 ; mf }
	{ mul_hs_ls r5, r6, r7 ; shrsi r15, r16, 5 }
	{ mul_hs_ls r5, r6, r7 ; v1minu r15, r16, r17 }
	{ mul_hs_ls r5, r6, r7 ; v2shru r15, r16, r17 }
	{ mul_hs_lu r5, r6, r7 ; dblalign6 r15, r16, r17 }
	{ mul_hs_lu r5, r6, r7 ; ldna r15, r16 }
	{ mul_hs_lu r5, r6, r7 ; prefetch_l3 r15 }
	{ mul_hs_lu r5, r6, r7 ; subxsc r15, r16, r17 }
	{ mul_hs_lu r5, r6, r7 ; v2cmpne r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; add r15, r16, r17 ; ld4s r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; addx r15, r16, r17 ; ld4u r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; and r15, r16, r17 ; ld4u r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1 r25 }
	{ mul_hu_hu r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1 r25 }
	{ mul_hu_hu r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2 r25 }
	{ mul_hu_hu r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3 r25 }
	{ mul_hu_hu r5, r6, r7 ; fetchor4 r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; ill ; st2 r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; jalr r15 ; st1 r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; jr r15 ; st4 r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; ld r25, r26 ; jalrp r15 }
	{ mul_hu_hu r5, r6, r7 ; ld1s r25, r26 ; cmplts r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; ld1u r25, r26 ; addi r15, r16, 5 }
	{ mul_hu_hu r5, r6, r7 ; ld1u r25, r26 ; shru r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; ld2s r25, r26 ; shl1add r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; ld2u r25, r26 ; move r15, r16 }
	{ mul_hu_hu r5, r6, r7 ; ld4s r25, r26 ; fnop }
	{ mul_hu_hu r5, r6, r7 ; ld4u r25, r26 ; andi r15, r16, 5 }
	{ mul_hu_hu r5, r6, r7 ; ld4u r25, r26 ; xor r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; mfspr r16, 0x5 }
	{ mul_hu_hu r5, r6, r7 ; movei r15, 5 ; ld1s r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; nop ; ld1s r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; or r15, r16, r17 ; ld2s r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; prefetch r25 ; mnz r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; prefetch_l1 r25 ; cmples r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; prefetch_l1_fault r25 ; add r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; prefetch_l1_fault r25 ; shrsi r15, r16, 5 }
	{ mul_hu_hu r5, r6, r7 ; prefetch_l2 r25 ; shl1add r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; prefetch_l2_fault r25 ; movei r15, 5 }
	{ mul_hu_hu r5, r6, r7 ; prefetch_l3 r25 ; info 19 }
	{ mul_hu_hu r5, r6, r7 ; prefetch_l3_fault r25 ; cmples r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; rotl r15, r16, r17 ; ld r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; shl r15, r16, r17 ; ld1u r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2s r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4s r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch r25 }
	{ mul_hu_hu r5, r6, r7 ; shrs r15, r16, r17 ; prefetch r25 }
	{ mul_hu_hu r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1_fault r25 }
	{ mul_hu_hu r5, r6, r7 ; st r25, r26 ; cmples r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; st1 r25, r26 ; add r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; st1 r25, r26 ; shrsi r15, r16, 5 }
	{ mul_hu_hu r5, r6, r7 ; st2 r25, r26 ; shl r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; st4 r25, r26 ; mnz r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; sub r15, r16, r17 ; ld4s r25, r26 }
	{ mul_hu_hu r5, r6, r7 ; v1cmpleu r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; v2mnz r15, r16, r17 }
	{ mul_hu_hu r5, r6, r7 ; xor r15, r16, r17 ; st r25, r26 }
	{ mul_hu_ls r5, r6, r7 ; finv r15 }
	{ mul_hu_ls r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
	{ mul_hu_ls r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ mul_hu_ls r5, r6, r7 ; v1cmpne r15, r16, r17 }
	{ mul_hu_ls r5, r6, r7 ; v2shl r15, r16, r17 }
	{ mul_hu_lu r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ mul_hu_lu r5, r6, r7 ; ld4s r15, r16 }
	{ mul_hu_lu r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ mul_hu_lu r5, r6, r7 ; stnt4 r15, r16 }
	{ mul_hu_lu r5, r6, r7 ; v2cmpleu r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; add r15, r16, r17 ; ld r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; addx r15, r16, r17 ; ld1s r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; and r15, r16, r17 ; ld1s r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2s r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; cmples r15, r16, r17 ; ld2s r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; cmplts r15, r16, r17 ; ld4s r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch r25 }
	{ mul_ls_ls r5, r6, r7 ; fetchaddgez r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; ill ; prefetch_l2_fault r25 }
	{ mul_ls_ls r5, r6, r7 ; jalr r15 ; prefetch_l2 r25 }
	{ mul_ls_ls r5, r6, r7 ; jr r15 ; prefetch_l3 r25 }
	{ mul_ls_ls r5, r6, r7 ; ld r25, r26 ; cmpne r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; ld1s r25, r26 ; andi r15, r16, 5 }
	{ mul_ls_ls r5, r6, r7 ; ld1s r25, r26 ; xor r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; ld1u r25, r26 ; shl3add r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; ld2s r25, r26 ; nor r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; ld2u r25, r26 ; jalrp r15 }
	{ mul_ls_ls r5, r6, r7 ; ld4s r25, r26 ; cmpleu r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; ld4u r25, r26 ; add r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; ld4u r25, r26 ; shrsi r15, r16, 5 }
	{ mul_ls_ls r5, r6, r7 ; lnk r15 ; st1 r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; move r15, r16 ; st1 r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; mz r15, r16, r17 ; st1 r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; nor r15, r16, r17 ; st4 r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; prefetch r25 ; jalr r15 }
	{ mul_ls_ls r5, r6, r7 ; prefetch_l1 r25 ; addxi r15, r16, 5 }
	{ mul_ls_ls r5, r6, r7 ; prefetch_l1 r25 ; sub r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; prefetch_l1_fault r25 ; shl2addx r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; prefetch_l2 r25 ; nor r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; prefetch_l2_fault r25 ; jr r15 }
	{ mul_ls_ls r5, r6, r7 ; prefetch_l3 r25 ; cmpltsi r15, r16, 5 }
	{ mul_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 ; addxi r15, r16, 5 }
	{ mul_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 ; sub r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; rotli r15, r16, 5 ; st2 r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 ; st4 r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; ld r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; ld1u r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; ld1u r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; shru r15, r16, r17 ; ld2u r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; st r25, r26 ; addxi r15, r16, 5 }
	{ mul_ls_ls r5, r6, r7 ; st r25, r26 ; sub r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; st1 r25, r26 ; shl2addx r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; st2 r25, r26 ; nop }
	{ mul_ls_ls r5, r6, r7 ; st4 r25, r26 ; jalr r15 }
	{ mul_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; ld r25, r26 }
	{ mul_ls_ls r5, r6, r7 ; v1addi r15, r16, 5 }
	{ mul_ls_ls r5, r6, r7 ; v2int_l r15, r16, r17 }
	{ mul_ls_ls r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l1_fault r25 }
	{ mul_ls_lu r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
	{ mul_ls_lu r5, r6, r7 ; ldnt2s r15, r16 }
	{ mul_ls_lu r5, r6, r7 ; shl1add r15, r16, r17 }
	{ mul_ls_lu r5, r6, r7 ; v1cmpleu r15, r16, r17 }
	{ mul_ls_lu r5, r6, r7 ; v2mnz r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; add r15, r16, r17 ; prefetch_l3 r25 }
	{ mul_lu_lu r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mul_lu_lu r5, r6, r7 ; and r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mul_lu_lu r5, r6, r7 ; cmpeq r15, r16, r17 ; st1 r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; cmples r15, r16, r17 ; st1 r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; cmplts r15, r16, r17 ; st4 r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; cmpltui r15, r16, 5 }
	{ mul_lu_lu r5, r6, r7 ; fnop ; ld2s r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; info 19 ; ld2u r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; jalrp r15 ; ld2s r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; jrp r15 ; ld4s r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; ld r25, r26 ; mz r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; ld1s r25, r26 ; jalr r15 }
	{ mul_lu_lu r5, r6, r7 ; ld1u r25, r26 ; cmples r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; ld2s r15, r16 }
	{ mul_lu_lu r5, r6, r7 ; ld2s r25, r26 ; shrs r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; ld2u r25, r26 ; rotli r15, r16, 5 }
	{ mul_lu_lu r5, r6, r7 ; ld4s r25, r26 ; lnk r15 }
	{ mul_lu_lu r5, r6, r7 ; ld4u r25, r26 ; cmpltu r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; ldnt1s r15, r16 }
	{ mul_lu_lu r5, r6, r7 ; mnz r15, r16, r17 ; ld4u r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; movei r15, 5 ; prefetch_l1 r25 }
	{ mul_lu_lu r5, r6, r7 ; nop ; prefetch_l1 r25 }
	{ mul_lu_lu r5, r6, r7 ; or r15, r16, r17 ; prefetch_l2 r25 }
	{ mul_lu_lu r5, r6, r7 ; prefetch r25 ; rotl r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; prefetch_l1 r25 ; ill }
	{ mul_lu_lu r5, r6, r7 ; prefetch_l1_fault r25 ; cmpeqi r15, r16, 5 }
	{ mul_lu_lu r5, r6, r7 ; prefetch_l2 r15 }
	{ mul_lu_lu r5, r6, r7 ; prefetch_l2 r25 ; shrs r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; prefetch_l2_fault r25 ; shl r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; prefetch_l3 r25 ; move r15, r16 }
	{ mul_lu_lu r5, r6, r7 ; prefetch_l3_fault r25 ; ill }
	{ mul_lu_lu r5, r6, r7 ; rotl r15, r16, r17 ; prefetch r25 }
	{ mul_lu_lu r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l1_fault r25 }
	{ mul_lu_lu r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l2 r25 }
	{ mul_lu_lu r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l3 r25 }
	{ mul_lu_lu r5, r6, r7 ; shl3addx r15, r16, r17 ; st r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; shrs r15, r16, r17 ; st r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; shru r15, r16, r17 ; st2 r25, r26 }
	{ mul_lu_lu r5, r6, r7 ; st r25, r26 ; ill }
	{ mul_lu_lu r5, r6, r7 ; st1 r25, r26 ; cmpeqi r15, r16, 5 }
	{ mul_lu_lu r5, r6, r7 ; st1_add r15, r16, 5 }
	{ mul_lu_lu r5, r6, r7 ; st2 r25, r26 ; shli r15, r16, 5 }
	{ mul_lu_lu r5, r6, r7 ; st4 r25, r26 ; rotl r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l3 r25 }
	{ mul_lu_lu r5, r6, r7 ; v1int_l r15, r16, r17 }
	{ mul_lu_lu r5, r6, r7 ; v2shlsc r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; add r15, r16, r17 ; ld1s r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; addx r15, r16, r17 ; ld1u r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; and r15, r16, r17 ; ld1u r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 ; ld2u r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; cmples r15, r16, r17 ; ld2u r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 ; ld4u r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l1 r25 }
	{ mula_hs_hs r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; ill ; prefetch_l3 r25 }
	{ mula_hs_hs r5, r6, r7 ; jalr r15 ; prefetch_l2_fault r25 }
	{ mula_hs_hs r5, r6, r7 ; jr r15 ; prefetch_l3_fault r25 }
	{ mula_hs_hs r5, r6, r7 ; ld r25, r26 ; fnop }
	{ mula_hs_hs r5, r6, r7 ; ld1s r25, r26 ; cmpeq r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; ld1s r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; ld1u r25, r26 ; shl3addx r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; ld2s r25, r26 ; or r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; ld2u r25, r26 ; jr r15 }
	{ mula_hs_hs r5, r6, r7 ; ld4s r25, r26 ; cmplts r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; ld4u r25, r26 ; addi r15, r16, 5 }
	{ mula_hs_hs r5, r6, r7 ; ld4u r25, r26 ; shru r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; lnk r15 ; st2 r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; move r15, r16 ; st2 r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; mz r15, r16, r17 ; st2 r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; nor r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; prefetch r25 ; jalrp r15 }
	{ mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 ; and r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 ; subx r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; prefetch_l1_fault r25 ; shl3add r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; prefetch_l2 r25 ; or r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; prefetch_l2_fault r25 ; jrp r15 }
	{ mula_hs_hs r5, r6, r7 ; prefetch_l3 r25 ; cmpltu r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; prefetch_l3_fault r25 ; and r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; prefetch_l3_fault r25 ; subx r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; rotli r15, r16, 5 ; st4 r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; shl1add r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; shl2addx r15, r16, r17 ; ld1s r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; shl3addx r15, r16, r17 ; ld2s r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; shrs r15, r16, r17 ; ld2s r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; shru r15, r16, r17 ; ld4s r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; st r25, r26 ; and r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; st r25, r26 ; subx r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; st1 r25, r26 ; shl3add r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; st2 r25, r26 ; nor r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; st4 r25, r26 ; jalrp r15 }
	{ mula_hs_hs r5, r6, r7 ; sub r15, r16, r17 ; ld1s r25, r26 }
	{ mula_hs_hs r5, r6, r7 ; v1adduc r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; v2maxs r15, r16, r17 }
	{ mula_hs_hs r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l2 r25 }
	{ mula_hs_hu r5, r6, r7 ; fetchand r15, r16, r17 }
	{ mula_hs_hu r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
	{ mula_hs_hu r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ mula_hs_hu r5, r6, r7 ; v1cmplts r15, r16, r17 }
	{ mula_hs_hu r5, r6, r7 ; v2mz r15, r16, r17 }
	{ mula_hs_ls r5, r6, r7 ; cmples r15, r16, r17 }
	{ mula_hs_ls r5, r6, r7 ; ld2s r15, r16 }
	{ mula_hs_ls r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
	{ mula_hs_ls r5, r6, r7 ; stnt1 r15, r16 }
	{ mula_hs_ls r5, r6, r7 ; v2addsc r15, r16, r17 }
	{ mula_hs_ls r5, r6, r7 ; v4subsc r15, r16, r17 }
	{ mula_hs_lu r5, r6, r7 ; flushwb }
	{ mula_hs_lu r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
	{ mula_hs_lu r5, r6, r7 ; shlx r15, r16, r17 }
	{ mula_hs_lu r5, r6, r7 ; v1int_l r15, r16, r17 }
	{ mula_hs_lu r5, r6, r7 ; v2shlsc r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; addi r15, r16, 5 ; ld r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; addxi r15, r16, 5 ; ld1s r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; andi r15, r16, 5 ; ld1s r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 ; ld2s r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; cmpleu r15, r16, r17 ; ld2s r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld4s r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; cmpne r15, r16, r17 ; ld4u r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; fnop ; prefetch_l2 r25 }
	{ mula_hu_hu r5, r6, r7 ; info 19 ; prefetch_l2_fault r25 }
	{ mula_hu_hu r5, r6, r7 ; jalrp r15 ; prefetch_l2 r25 }
	{ mula_hu_hu r5, r6, r7 ; jrp r15 ; prefetch_l3 r25 }
	{ mula_hu_hu r5, r6, r7 ; ld r25, r26 ; shl1add r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; ld1s r25, r26 ; movei r15, 5 }
	{ mula_hu_hu r5, r6, r7 ; ld1u r25, r26 ; ill }
	{ mula_hu_hu r5, r6, r7 ; ld2s r25, r26 ; cmpeq r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; ld2s r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; ld2u r25, r26 ; shl3addx r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; ld4s r25, r26 ; or r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; ld4u r25, r26 ; jr r15 }
	{ mula_hu_hu r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
	{ mula_hu_hu r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mula_hu_hu r5, r6, r7 ; movei r15, 5 ; st1 r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; nop ; st1 r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; or r15, r16, r17 ; st4 r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; prefetch r25 ; shl3add r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; prefetch_l1 r25 ; mnz r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; prefetch_l1_fault r25 ; fnop }
	{ mula_hu_hu r5, r6, r7 ; prefetch_l2 r25 ; cmpeq r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; prefetch_l2 r25 }
	{ mula_hu_hu r5, r6, r7 ; prefetch_l2_fault r25 ; shli r15, r16, 5 }
	{ mula_hu_hu r5, r6, r7 ; prefetch_l3 r25 ; rotli r15, r16, 5 }
	{ mula_hu_hu r5, r6, r7 ; prefetch_l3_fault r25 ; mnz r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; rotl r15, r16, r17 ; st r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; shl r15, r16, r17 ; st2 r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; shl1addx r15, r16, r17 ; st4 r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; shl3add r15, r16, r17 ; ld r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; shli r15, r16, 5 ; ld1u r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; shrsi r15, r16, 5 ; ld1u r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; shrui r15, r16, 5 ; ld2u r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; st r25, r26 ; mnz r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; st1 r25, r26 ; fnop }
	{ mula_hu_hu r5, r6, r7 ; st2 r25, r26 ; andi r15, r16, 5 }
	{ mula_hu_hu r5, r6, r7 ; st2 r25, r26 ; xor r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; st4 r25, r26 ; shl3add r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; subx r15, r16, r17 ; ld r25, r26 }
	{ mula_hu_hu r5, r6, r7 ; v1shl r15, r16, r17 }
	{ mula_hu_hu r5, r6, r7 ; v4add r15, r16, r17 }
	{ mula_hu_ls r5, r6, r7 ; andi r15, r16, 5 }
	{ mula_hu_ls r5, r6, r7 ; ld r15, r16 }
	{ mula_hu_ls r5, r6, r7 ; nor r15, r16, r17 }
	{ mula_hu_ls r5, r6, r7 ; st2_add r15, r16, 5 }
	{ mula_hu_ls r5, r6, r7 ; v1shrui r15, r16, 5 }
	{ mula_hu_ls r5, r6, r7 ; v4shl r15, r16, r17 }
	{ mula_hu_lu r5, r6, r7 ; fetchand4 r15, r16, r17 }
	{ mula_hu_lu r5, r6, r7 ; ldnt2u r15, r16 }
	{ mula_hu_lu r5, r6, r7 ; shl2add r15, r16, r17 }
	{ mula_hu_lu r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
	{ mula_hu_lu r5, r6, r7 ; v2packh r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; add r15, r16, r17 ; st r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; addx r15, r16, r17 ; st1 r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; and r15, r16, r17 ; st1 r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; cmpeq r15, r16, r17 ; st4 r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; cmples r15, r16, r17 ; st4 r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; cmpne r15, r16, r17 ; ld1s r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; fnop ; ld4s r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; info 19 ; ld4u r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; jalrp r15 ; ld4s r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; jrp r15 ; prefetch r25 }
	{ mula_ls_ls r5, r6, r7 ; ld r25, r26 ; nor r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; ld1s r25, r26 ; jr r15 }
	{ mula_ls_ls r5, r6, r7 ; ld1u r25, r26 ; cmplts r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; ld2s r25, r26 ; addi r15, r16, 5 }
	{ mula_ls_ls r5, r6, r7 ; ld2s r25, r26 ; shru r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; ld2u r25, r26 ; shl1add r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; ld4s r25, r26 ; move r15, r16 }
	{ mula_ls_ls r5, r6, r7 ; ld4u r25, r26 ; fnop }
	{ mula_ls_ls r5, r6, r7 ; ldnt1u r15, r16 }
	{ mula_ls_ls r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l1 r25 }
	{ mula_ls_ls r5, r6, r7 ; movei r15, 5 ; prefetch_l2 r25 }
	{ mula_ls_ls r5, r6, r7 ; nop ; prefetch_l2 r25 }
	{ mula_ls_ls r5, r6, r7 ; or r15, r16, r17 ; prefetch_l3 r25 }
	{ mula_ls_ls r5, r6, r7 ; prefetch r25 ; shl r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; prefetch_l1 r25 ; jalr r15 }
	{ mula_ls_ls r5, r6, r7 ; prefetch_l1_fault r25 ; cmpleu r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; prefetch_l2 r25 ; addi r15, r16, 5 }
	{ mula_ls_ls r5, r6, r7 ; prefetch_l2 r25 ; shru r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; prefetch_l2_fault r25 ; shl1addx r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; prefetch_l3 r25 ; mz r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 ; jalr r15 }
	{ mula_ls_ls r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l1_fault r25 }
	{ mula_ls_ls r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l2_fault r25 }
	{ mula_ls_ls r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l3 r25 }
	{ mula_ls_ls r5, r6, r7 ; shl2addx r15, r16, r17 ; st r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; shl3addx r15, r16, r17 ; st2 r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; shrs r15, r16, r17 ; st2 r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; shru r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; st r25, r26 ; jalr r15 }
	{ mula_ls_ls r5, r6, r7 ; st1 r25, r26 ; cmpleu r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; st2 r25, r26 ; add r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; st2 r25, r26 ; shrsi r15, r16, 5 }
	{ mula_ls_ls r5, r6, r7 ; st4 r25, r26 ; shl r15, r16, r17 }
	{ mula_ls_ls r5, r6, r7 ; sub r15, r16, r17 ; st r25, r26 }
	{ mula_ls_ls r5, r6, r7 ; v1maxui r15, r16, 5 }
	{ mula_ls_ls r5, r6, r7 ; v2shrsi r15, r16, 5 }
	{ mula_ls_lu r5, r6, r7 ; addx r15, r16, r17 }
	{ mula_ls_lu r5, r6, r7 ; iret }
	{ mula_ls_lu r5, r6, r7 ; movei r15, 5 }
	{ mula_ls_lu r5, r6, r7 ; shruxi r15, r16, 5 }
	{ mula_ls_lu r5, r6, r7 ; v1shl r15, r16, r17 }
	{ mula_ls_lu r5, r6, r7 ; v4add r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; addi r15, r16, 5 ; prefetch r25 }
	{ mula_lu_lu r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l1 r25 }
	{ mula_lu_lu r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l1 r25 }
	{ mula_lu_lu r5, r6, r7 ; cmpeqi r15, r16, 5 ; prefetch_l2 r25 }
	{ mula_lu_lu r5, r6, r7 ; cmpleu r15, r16, r17 ; prefetch_l2 r25 }
	{ mula_lu_lu r5, r6, r7 ; cmpltsi r15, r16, 5 ; prefetch_l3 r25 }
	{ mula_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mula_lu_lu r5, r6, r7 ; fnop ; st4 r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; info 19 }
	{ mula_lu_lu r5, r6, r7 ; jalrp r15 ; st4 r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; ld r15, r16 }
	{ mula_lu_lu r5, r6, r7 ; ld r25, r26 ; shrs r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; ld1s r25, r26 ; shl r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; ld1u r25, r26 ; mnz r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; ld2s r25, r26 ; cmpne r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; ld2u r25, r26 ; and r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; ld2u r25, r26 ; subx r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; ld4s r25, r26 ; shl2addx r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; ld4u r25, r26 ; nop }
	{ mula_lu_lu r5, r6, r7 ; lnk r15 ; ld1s r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; move r15, r16 ; ld1s r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; mz r15, r16, r17 ; ld1s r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; nor r15, r16, r17 ; ld2s r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; prefetch r25 ; addxi r15, r16, 5 }
	{ mula_lu_lu r5, r6, r7 ; prefetch r25 ; sub r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; prefetch_l1 r25 ; rotl r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; prefetch_l1_fault r25 ; lnk r15 }
	{ mula_lu_lu r5, r6, r7 ; prefetch_l2 r25 ; cmpne r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; prefetch_l2_fault r25 ; andi r15, r16, 5 }
	{ mula_lu_lu r5, r6, r7 ; prefetch_l2_fault r25 ; xor r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; prefetch_l3 r25 ; shl3addx r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; prefetch_l3_fault r25 ; rotl r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; rotli r15, r16, 5 ; ld1u r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; shl1add r15, r16, r17 ; ld2s r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; shl2add r15, r16, r17 ; ld4s r25, r26 }
	{ mula_lu_lu r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch r25 }
	{ mula_lu_lu r5, r6, r7 ; shli r15, r16, 5 ; prefetch_l1_fault r25 }
	{ mula_lu_lu r5, r6, r7 ; shrsi r15, r16, 5 ; prefetch_l1_fault r25 }
	{ mula_lu_lu r5, r6, r7 ; shrui r15, r16, 5 ; prefetch_l2_fault r25 }
	{ mula_lu_lu r5, r6, r7 ; st r25, r26 ; rotl r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; st1 r25, r26 ; lnk r15 }
	{ mula_lu_lu r5, r6, r7 ; st2 r25, r26 ; cmpltu r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; st4 r25, r26 ; addxi r15, r16, 5 }
	{ mula_lu_lu r5, r6, r7 ; st4 r25, r26 ; sub r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; subx r15, r16, r17 ; prefetch r25 }
	{ mula_lu_lu r5, r6, r7 ; v1subuc r15, r16, r17 }
	{ mula_lu_lu r5, r6, r7 ; v4shrs r15, r16, r17 }
	{ mulax r5, r6, r7 ; add r15, r16, r17 ; st1 r25, r26 }
	{ mulax r5, r6, r7 ; addx r15, r16, r17 ; st2 r25, r26 }
	{ mulax r5, r6, r7 ; and r15, r16, r17 ; st2 r25, r26 }
	{ mulax r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ mulax r5, r6, r7 ; cmples r15, r16, r17 }
	{ mulax r5, r6, r7 ; cmpltsi r15, r16, 5 ; ld1s r25, r26 }
	{ mulax r5, r6, r7 ; cmpne r15, r16, r17 ; ld1u r25, r26 }
	{ mulax r5, r6, r7 ; fnop ; ld4u r25, r26 }
	{ mulax r5, r6, r7 ; info 19 ; prefetch r25 }
	{ mulax r5, r6, r7 ; jalrp r15 ; ld4u r25, r26 }
	{ mulax r5, r6, r7 ; jrp r15 ; prefetch_l1 r25 }
	{ mulax r5, r6, r7 ; ld r25, r26 ; or r15, r16, r17 }
	{ mulax r5, r6, r7 ; ld1s r25, r26 ; jrp r15 }
	{ mulax r5, r6, r7 ; ld1u r25, r26 ; cmpltsi r15, r16, 5 }
	{ mulax r5, r6, r7 ; ld2s r25, r26 ; addx r15, r16, r17 }
	{ mulax r5, r6, r7 ; ld2s r25, r26 ; shrui r15, r16, 5 }
	{ mulax r5, r6, r7 ; ld2u r25, r26 ; shl1addx r15, r16, r17 }
	{ mulax r5, r6, r7 ; ld4s r25, r26 ; movei r15, 5 }
	{ mulax r5, r6, r7 ; ld4u r25, r26 ; ill }
	{ mulax r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
	{ mulax r5, r6, r7 ; mnz r15, r16, r17 ; prefetch_l1_fault r25 }
	{ mulax r5, r6, r7 ; movei r15, 5 ; prefetch_l2_fault r25 }
	{ mulax r5, r6, r7 ; nop ; prefetch_l2_fault r25 }
	{ mulax r5, r6, r7 ; or r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mulax r5, r6, r7 ; prefetch r25 ; shl1add r15, r16, r17 }
	{ mulax r5, r6, r7 ; prefetch_l1 r25 ; jalrp r15 }
	{ mulax r5, r6, r7 ; prefetch_l1_fault r25 ; cmplts r15, r16, r17 }
	{ mulax r5, r6, r7 ; prefetch_l2 r25 ; addx r15, r16, r17 }
	{ mulax r5, r6, r7 ; prefetch_l2 r25 ; shrui r15, r16, 5 }
	{ mulax r5, r6, r7 ; prefetch_l2_fault r25 ; shl2add r15, r16, r17 }
	{ mulax r5, r6, r7 ; prefetch_l3 r25 ; nop }
	{ mulax r5, r6, r7 ; prefetch_l3_fault r25 ; jalrp r15 }
	{ mulax r5, r6, r7 ; rotl r15, r16, r17 ; prefetch_l2 r25 }
	{ mulax r5, r6, r7 ; shl r15, r16, r17 ; prefetch_l3 r25 }
	{ mulax r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mulax r5, r6, r7 ; shl2addx r15, r16, r17 ; st1 r25, r26 }
	{ mulax r5, r6, r7 ; shl3addx r15, r16, r17 ; st4 r25, r26 }
	{ mulax r5, r6, r7 ; shrs r15, r16, r17 ; st4 r25, r26 }
	{ mulax r5, r6, r7 ; shrui r15, r16, 5 ; ld r25, r26 }
	{ mulax r5, r6, r7 ; st r25, r26 ; jalrp r15 }
	{ mulax r5, r6, r7 ; st1 r25, r26 ; cmplts r15, r16, r17 }
	{ mulax r5, r6, r7 ; st2 r25, r26 ; addi r15, r16, 5 }
	{ mulax r5, r6, r7 ; st2 r25, r26 ; shru r15, r16, r17 }
	{ mulax r5, r6, r7 ; st4 r25, r26 ; shl1add r15, r16, r17 }
	{ mulax r5, r6, r7 ; sub r15, r16, r17 ; st1 r25, r26 }
	{ mulax r5, r6, r7 ; v1minu r15, r16, r17 }
	{ mulax r5, r6, r7 ; v2shru r15, r16, r17 }
	{ mulx r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
	{ mulx r5, r6, r7 ; addx r15, r16, r17 ; ld4s r25, r26 }
	{ mulx r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
	{ mulx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
	{ mulx r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
	{ mulx r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l1_fault r25 }
	{ mulx r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2_fault r25 }
	{ mulx r5, r6, r7 ; fetchor r15, r16, r17 }
	{ mulx r5, r6, r7 ; ill ; st1 r25, r26 }
	{ mulx r5, r6, r7 ; jalr r15 ; st r25, r26 }
	{ mulx r5, r6, r7 ; jr r15 ; st2 r25, r26 }
	{ mulx r5, r6, r7 ; ld r25, r26 ; jalr r15 }
	{ mulx r5, r6, r7 ; ld1s r25, r26 ; cmpleu r15, r16, r17 }
	{ mulx r5, r6, r7 ; ld1u r25, r26 ; add r15, r16, r17 }
	{ mulx r5, r6, r7 ; ld1u r25, r26 ; shrsi r15, r16, 5 }
	{ mulx r5, r6, r7 ; ld2s r25, r26 ; shl r15, r16, r17 }
	{ mulx r5, r6, r7 ; ld2u r25, r26 ; mnz r15, r16, r17 }
	{ mulx r5, r6, r7 ; ld4s r25, r26 ; cmpne r15, r16, r17 }
	{ mulx r5, r6, r7 ; ld4u r25, r26 ; and r15, r16, r17 }
	{ mulx r5, r6, r7 ; ld4u r25, r26 ; subx r15, r16, r17 }
	{ mulx r5, r6, r7 ; mf }
	{ mulx r5, r6, r7 ; movei r15, 5 ; ld r25, r26 }
	{ mulx r5, r6, r7 ; nop ; ld r25, r26 }
	{ mulx r5, r6, r7 ; or r15, r16, r17 ; ld1u r25, r26 }
	{ mulx r5, r6, r7 ; prefetch r25 ; lnk r15 }
	{ mulx r5, r6, r7 ; prefetch_l1 r25 ; cmpeqi r15, r16, 5 }
	{ mulx r5, r6, r7 ; prefetch_l1_fault r15 }
	{ mulx r5, r6, r7 ; prefetch_l1_fault r25 ; shrs r15, r16, r17 }
	{ mulx r5, r6, r7 ; prefetch_l2 r25 ; shl r15, r16, r17 }
	{ mulx r5, r6, r7 ; prefetch_l2_fault r25 ; move r15, r16 }
	{ mulx r5, r6, r7 ; prefetch_l3 r25 ; ill }
	{ mulx r5, r6, r7 ; prefetch_l3_fault r25 ; cmpeqi r15, r16, 5 }
	{ mulx r5, r6, r7 ; raise }
	{ mulx r5, r6, r7 ; shl r15, r16, r17 ; ld1s r25, r26 }
	{ mulx r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
	{ mulx r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2u r25, r26 }
	{ mulx r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
	{ mulx r5, r6, r7 ; shrs r15, r16, r17 ; ld4u r25, r26 }
	{ mulx r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1 r25 }
	{ mulx r5, r6, r7 ; st r25, r26 ; cmpeqi r15, r16, 5 }
	{ mulx r5, r6, r7 ; st1 r15, r16 }
	{ mulx r5, r6, r7 ; st1 r25, r26 ; shrs r15, r16, r17 }
	{ mulx r5, r6, r7 ; st2 r25, r26 ; rotli r15, r16, 5 }
	{ mulx r5, r6, r7 ; st4 r25, r26 ; lnk r15 }
	{ mulx r5, r6, r7 ; sub r15, r16, r17 ; ld2u r25, r26 }
	{ mulx r5, r6, r7 ; v1cmples r15, r16, r17 }
	{ mulx r5, r6, r7 ; v2minsi r15, r16, 5 }
	{ mulx r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
	{ mz r15, r16, r17 ; addi r5, r6, 5 ; st r25, r26 }
	{ mz r15, r16, r17 ; addxi r5, r6, 5 ; st1 r25, r26 }
	{ mz r15, r16, r17 ; andi r5, r6, 5 ; st1 r25, r26 }
	{ mz r15, r16, r17 ; cmoveqz r5, r6, r7 ; st r25, r26 }
	{ mz r15, r16, r17 ; cmpeq r5, r6, r7 ; st2 r25, r26 }
	{ mz r15, r16, r17 ; cmples r5, r6, r7 }
	{ mz r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld1s r25, r26 }
	{ mz r15, r16, r17 ; cmpne r5, r6, r7 ; ld1u r25, r26 }
	{ mz r15, r16, r17 ; ctz r5, r6 ; st r25, r26 }
	{ mz r15, r16, r17 ; fsingle_pack1 r5, r6 ; ld r25, r26 }
	{ mz r15, r16, r17 ; infol 0x1234 }
	{ mz r15, r16, r17 ; ld r25, r26 ; revbits r5, r6 }
	{ mz r15, r16, r17 ; ld1s r25, r26 ; cmpne r5, r6, r7 }
	{ mz r15, r16, r17 ; ld1s r25, r26 ; subx r5, r6, r7 }
	{ mz r15, r16, r17 ; ld1u r25, r26 ; mulx r5, r6, r7 }
	{ mz r15, r16, r17 ; ld2s r25, r26 ; cmpeqi r5, r6, 5 }
	{ mz r15, r16, r17 ; ld2s r25, r26 ; shli r5, r6, 5 }
	{ mz r15, r16, r17 ; ld2u r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ mz r15, r16, r17 ; ld4s r25, r26 ; and r5, r6, r7 }
	{ mz r15, r16, r17 ; ld4s r25, r26 ; shl1add r5, r6, r7 }
	{ mz r15, r16, r17 ; ld4u r25, r26 ; mnz r5, r6, r7 }
	{ mz r15, r16, r17 ; ld4u r25, r26 ; xor r5, r6, r7 }
	{ mz r15, r16, r17 ; move r5, r6 }
	{ mz r15, r16, r17 ; mul_hs_hu r5, r6, r7 }
	{ mz r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; st2 r25, r26 }
	{ mz r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; st4 r25, r26 }
	{ mz r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; st r25, r26 }
	{ mz r15, r16, r17 ; mulax r5, r6, r7 ; st1 r25, r26 }
	{ mz r15, r16, r17 ; mz r5, r6, r7 ; st4 r25, r26 }
	{ mz r15, r16, r17 ; or r5, r6, r7 ; ld r25, r26 }
	{ mz r15, r16, r17 ; prefetch r25 ; addi r5, r6, 5 }
	{ mz r15, r16, r17 ; prefetch r25 ; rotl r5, r6, r7 }
	{ mz r15, r16, r17 ; prefetch_l1 r25 ; fnop }
	{ mz r15, r16, r17 ; prefetch_l1 r25 ; tblidxb1 r5, r6 }
	{ mz r15, r16, r17 ; prefetch_l1_fault r25 ; nop }
	{ mz r15, r16, r17 ; prefetch_l2 r25 ; cmpleu r5, r6, r7 }
	{ mz r15, r16, r17 ; prefetch_l2 r25 ; shrsi r5, r6, 5 }
	{ mz r15, r16, r17 ; prefetch_l2_fault r25 ; mula_hu_hu r5, r6, r7 }
	{ mz r15, r16, r17 ; prefetch_l3 r25 ; clz r5, r6 }
	{ mz r15, r16, r17 ; prefetch_l3 r25 ; shl2add r5, r6, r7 }
	{ mz r15, r16, r17 ; prefetch_l3_fault r25 ; movei r5, 5 }
	{ mz r15, r16, r17 ; revbits r5, r6 ; ld r25, r26 }
	{ mz r15, r16, r17 ; rotl r5, r6, r7 ; ld1u r25, r26 }
	{ mz r15, r16, r17 ; shl r5, r6, r7 ; ld2u r25, r26 }
	{ mz r15, r16, r17 ; shl1addx r5, r6, r7 ; ld4s r25, r26 }
	{ mz r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch r25 }
	{ mz r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l1_fault r25 }
	{ mz r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l1_fault r25 }
	{ mz r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l2_fault r25 }
	{ mz r15, r16, r17 ; st r25, r26 ; cmpeqi r5, r6, 5 }
	{ mz r15, r16, r17 ; st r25, r26 ; shli r5, r6, 5 }
	{ mz r15, r16, r17 ; st1 r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ mz r15, r16, r17 ; st2 r25, r26 ; and r5, r6, r7 }
	{ mz r15, r16, r17 ; st2 r25, r26 ; shl1add r5, r6, r7 }
	{ mz r15, r16, r17 ; st4 r25, r26 ; mnz r5, r6, r7 }
	{ mz r15, r16, r17 ; st4 r25, r26 ; xor r5, r6, r7 }
	{ mz r15, r16, r17 ; subxsc r5, r6, r7 }
	{ mz r15, r16, r17 ; tblidxb2 r5, r6 ; ld1s r25, r26 }
	{ mz r15, r16, r17 ; v1adiffu r5, r6, r7 }
	{ mz r15, r16, r17 ; v1sub r5, r6, r7 }
	{ mz r15, r16, r17 ; v2shrsi r5, r6, 5 }
	{ mz r5, r6, r7 ; add r15, r16, r17 ; ld2u r25, r26 }
	{ mz r5, r6, r7 ; addx r15, r16, r17 ; ld4s r25, r26 }
	{ mz r5, r6, r7 ; and r15, r16, r17 ; ld4s r25, r26 }
	{ mz r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch r25 }
	{ mz r5, r6, r7 ; cmples r15, r16, r17 ; prefetch r25 }
	{ mz r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l1_fault r25 }
	{ mz r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2_fault r25 }
	{ mz r5, r6, r7 ; fetchor r15, r16, r17 }
	{ mz r5, r6, r7 ; ill ; st1 r25, r26 }
	{ mz r5, r6, r7 ; jalr r15 ; st r25, r26 }
	{ mz r5, r6, r7 ; jr r15 ; st2 r25, r26 }
	{ mz r5, r6, r7 ; ld r25, r26 ; jalr r15 }
	{ mz r5, r6, r7 ; ld1s r25, r26 ; cmpleu r15, r16, r17 }
	{ mz r5, r6, r7 ; ld1u r25, r26 ; add r15, r16, r17 }
	{ mz r5, r6, r7 ; ld1u r25, r26 ; shrsi r15, r16, 5 }
	{ mz r5, r6, r7 ; ld2s r25, r26 ; shl r15, r16, r17 }
	{ mz r5, r6, r7 ; ld2u r25, r26 ; mnz r15, r16, r17 }
	{ mz r5, r6, r7 ; ld4s r25, r26 ; cmpne r15, r16, r17 }
	{ mz r5, r6, r7 ; ld4u r25, r26 ; and r15, r16, r17 }
	{ mz r5, r6, r7 ; ld4u r25, r26 ; subx r15, r16, r17 }
	{ mz r5, r6, r7 ; mf }
	{ mz r5, r6, r7 ; movei r15, 5 ; ld r25, r26 }
	{ mz r5, r6, r7 ; nop ; ld r25, r26 }
	{ mz r5, r6, r7 ; or r15, r16, r17 ; ld1u r25, r26 }
	{ mz r5, r6, r7 ; prefetch r25 ; lnk r15 }
	{ mz r5, r6, r7 ; prefetch_l1 r25 ; cmpeqi r15, r16, 5 }
	{ mz r5, r6, r7 ; prefetch_l1_fault r15 }
	{ mz r5, r6, r7 ; prefetch_l1_fault r25 ; shrs r15, r16, r17 }
	{ mz r5, r6, r7 ; prefetch_l2 r25 ; shl r15, r16, r17 }
	{ mz r5, r6, r7 ; prefetch_l2_fault r25 ; move r15, r16 }
	{ mz r5, r6, r7 ; prefetch_l3 r25 ; ill }
	{ mz r5, r6, r7 ; prefetch_l3_fault r25 ; cmpeqi r15, r16, 5 }
	{ mz r5, r6, r7 ; raise }
	{ mz r5, r6, r7 ; shl r15, r16, r17 ; ld1s r25, r26 }
	{ mz r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1u r25, r26 }
	{ mz r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2u r25, r26 }
	{ mz r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4u r25, r26 }
	{ mz r5, r6, r7 ; shrs r15, r16, r17 ; ld4u r25, r26 }
	{ mz r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l1 r25 }
	{ mz r5, r6, r7 ; st r25, r26 ; cmpeqi r15, r16, 5 }
	{ mz r5, r6, r7 ; st1 r15, r16 }
	{ mz r5, r6, r7 ; st1 r25, r26 ; shrs r15, r16, r17 }
	{ mz r5, r6, r7 ; st2 r25, r26 ; rotli r15, r16, 5 }
	{ mz r5, r6, r7 ; st4 r25, r26 ; lnk r15 }
	{ mz r5, r6, r7 ; sub r15, r16, r17 ; ld2u r25, r26 }
	{ mz r5, r6, r7 ; v1cmples r15, r16, r17 }
	{ mz r5, r6, r7 ; v2minsi r15, r16, 5 }
	{ mz r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3_fault r25 }
	{ nop ; add r5, r6, r7 ; prefetch_l3_fault r25 }
	{ nop ; addi r5, r6, 5 ; st1 r25, r26 }
	{ nop ; addx r5, r6, r7 ; st1 r25, r26 }
	{ nop ; addxi r5, r6, 5 ; st4 r25, r26 }
	{ nop ; and r5, r6, r7 ; st1 r25, r26 }
	{ nop ; andi r5, r6, 5 ; st4 r25, r26 }
	{ nop ; cmoveqz r5, r6, r7 ; st1 r25, r26 }
	{ nop ; cmpeq r15, r16, r17 ; st4 r25, r26 }
	{ nop ; cmpeqi r5, r6, 5 ; ld r25, r26 }
	{ nop ; cmples r5, r6, r7 ; ld r25, r26 }
	{ nop ; cmpleu r5, r6, r7 ; ld1u r25, r26 }
	{ nop ; cmplts r5, r6, r7 ; ld2u r25, r26 }
	{ nop ; cmpltsi r5, r6, 5 ; ld4u r25, r26 }
	{ nop ; cmpltu r5, r6, r7 ; prefetch_l1 r25 }
	{ nop ; cmpne r5, r6, r7 ; prefetch_l1 r25 }
	{ nop ; dblalign2 r15, r16, r17 }
	{ nop ; fnop ; prefetch_l2_fault r25 }
	{ nop ; ill ; ld4u r25, r26 }
	{ nop ; jalr r15 ; ld4s r25, r26 }
	{ nop ; jr r15 ; prefetch r25 }
	{ nop ; ld r25, r26 ; and r15, r16, r17 }
	{ nop ; ld r25, r26 ; mul_hu_hu r5, r6, r7 }
	{ nop ; ld r25, r26 ; shrs r5, r6, r7 }
	{ nop ; ld1s r25, r26 ; cmpleu r15, r16, r17 }
	{ nop ; ld1s r25, r26 ; nor r5, r6, r7 }
	{ nop ; ld1s r25, r26 ; tblidxb2 r5, r6 }
	{ nop ; ld1u r25, r26 ; fsingle_pack1 r5, r6 }
	{ nop ; ld1u r25, r26 ; shl1add r15, r16, r17 }
	{ nop ; ld2s r25, r26 ; addx r5, r6, r7 }
	{ nop ; ld2s r25, r26 ; movei r15, 5 }
	{ nop ; ld2s r25, r26 ; shli r15, r16, 5 }
	{ nop ; ld2u r25, r26 ; cmpeqi r15, r16, 5 }
	{ nop ; ld2u r25, r26 ; mz r15, r16, r17 }
	{ nop ; ld2u r25, r26 ; subx r15, r16, r17 }
	{ nop ; ld4s r25, r26 ; cmpne r15, r16, r17 }
	{ nop ; ld4s r25, r26 ; rotli r15, r16, 5 }
	{ nop ; ld4u r25, r26 ; add r5, r6, r7 }
	{ nop ; ld4u r25, r26 ; mnz r15, r16, r17 }
	{ nop ; ld4u r25, r26 ; shl3add r15, r16, r17 }
	{ nop ; ldnt4u r15, r16 }
	{ nop ; mnz r15, r16, r17 ; st1 r25, r26 }
	{ nop ; move r15, r16 ; st4 r25, r26 }
	{ nop ; movei r5, 5 ; ld r25, r26 }
	{ nop ; mul_hs_hs r5, r6, r7 }
	{ nop ; mul_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ nop ; mula_hs_hs r5, r6, r7 ; st2 r25, r26 }
	{ nop ; mula_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 }
	{ nop ; mulax r5, r6, r7 ; st r25, r26 }
	{ nop ; mz r15, r16, r17 ; st2 r25, r26 }
	{ nop ; nop ; st4 r25, r26 }
	{ nop ; or r15, r16, r17 ; ld r25, r26 }
	{ nop ; pcnt r5, r6 ; ld r25, r26 }
	{ nop ; prefetch r25 ; cmples r5, r6, r7 }
	{ nop ; prefetch r25 ; nor r15, r16, r17 }
	{ nop ; prefetch r25 ; tblidxb1 r5, r6 }
	{ nop ; prefetch_l1 r25 ; cmpltu r15, r16, r17 }
	{ nop ; prefetch_l1 r25 ; rotl r15, r16, r17 }
	{ nop ; prefetch_l1_fault r25 ; add r15, r16, r17 }
	{ nop ; prefetch_l1_fault r25 ; lnk r15 }
	{ nop ; prefetch_l1_fault r25 ; shl2addx r5, r6, r7 }
	{ nop ; prefetch_l2 r25 ; cmoveqz r5, r6, r7 }
	{ nop ; prefetch_l2 r25 ; mula_ls_ls r5, r6, r7 }
	{ nop ; prefetch_l2 r25 ; shrui r15, r16, 5 }
	{ nop ; prefetch_l2_fault r25 ; cmpltsi r5, r6, 5 }
	{ nop ; prefetch_l2_fault r25 ; revbytes r5, r6 }
	{ nop ; prefetch_l3 r15 }
	{ nop ; prefetch_l3 r25 ; jrp r15 }
	{ nop ; prefetch_l3 r25 ; shl2addx r15, r16, r17 }
	{ nop ; prefetch_l3_fault r25 ; clz r5, r6 }
	{ nop ; prefetch_l3_fault r25 ; mula_hu_hu r5, r6, r7 }
	{ nop ; prefetch_l3_fault r25 ; shru r5, r6, r7 }
	{ nop ; revbytes r5, r6 ; ld4u r25, r26 }
	{ nop ; rotl r5, r6, r7 ; prefetch_l1 r25 }
	{ nop ; rotli r5, r6, 5 ; prefetch_l2 r25 }
	{ nop ; shl r5, r6, r7 ; prefetch_l3 r25 }
	{ nop ; shl1add r5, r6, r7 ; prefetch_l3 r25 }
	{ nop ; shl1addx r5, r6, r7 ; st r25, r26 }
	{ nop ; shl2add r5, r6, r7 ; st2 r25, r26 }
	{ nop ; shl2addx r5, r6, r7 }
	{ nop ; shl3addx r15, r16, r17 ; ld1s r25, r26 }
	{ nop ; shli r15, r16, 5 ; ld2s r25, r26 }
	{ nop ; shrs r15, r16, r17 ; ld1s r25, r26 }
	{ nop ; shrsi r15, r16, 5 ; ld2s r25, r26 }
	{ nop ; shru r15, r16, r17 ; ld4s r25, r26 }
	{ nop ; shrui r15, r16, 5 ; prefetch r25 }
	{ nop ; st r25, r26 ; addi r5, r6, 5 }
	{ nop ; st r25, r26 ; move r15, r16 }
	{ nop ; st r25, r26 ; shl3addx r15, r16, r17 }
	{ nop ; st1 r25, r26 ; cmpeq r5, r6, r7 }
	{ nop ; st1 r25, r26 ; mulx r5, r6, r7 }
	{ nop ; st1 r25, r26 ; sub r5, r6, r7 }
	{ nop ; st2 r25, r26 ; cmpltu r5, r6, r7 }
	{ nop ; st2 r25, r26 ; rotl r5, r6, r7 }
	{ nop ; st4 r25, r26 ; add r15, r16, r17 }
	{ nop ; st4 r25, r26 ; lnk r15 }
	{ nop ; st4 r25, r26 ; shl2addx r5, r6, r7 }
	{ nop ; sub r15, r16, r17 ; ld2u r25, r26 }
	{ nop ; subx r15, r16, r17 ; ld4u r25, r26 }
	{ nop ; tblidxb0 r5, r6 ; ld1u r25, r26 }
	{ nop ; tblidxb2 r5, r6 ; ld2u r25, r26 }
	{ nop ; v1adiffu r5, r6, r7 }
	{ nop ; v1minui r15, r16, 5 }
	{ nop ; v2cmples r5, r6, r7 }
	{ nop ; v2sadas r5, r6, r7 }
	{ nop ; v4sub r15, r16, r17 }
	{ nop ; xor r5, r6, r7 ; st2 r25, r26 }
	{ nor r15, r16, r17 ; addi r5, r6, 5 ; st2 r25, r26 }
	{ nor r15, r16, r17 ; addxi r5, r6, 5 ; st4 r25, r26 }
	{ nor r15, r16, r17 ; andi r5, r6, 5 ; st4 r25, r26 }
	{ nor r15, r16, r17 ; cmoveqz r5, r6, r7 ; st2 r25, r26 }
	{ nor r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ nor r15, r16, r17 ; cmpleu r5, r6, r7 ; ld1s r25, r26 }
	{ nor r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2s r25, r26 }
	{ nor r15, r16, r17 ; cmpne r5, r6, r7 ; ld2u r25, r26 }
	{ nor r15, r16, r17 ; ctz r5, r6 ; st2 r25, r26 }
	{ nor r15, r16, r17 ; fsingle_pack1 r5, r6 ; ld1u r25, r26 }
	{ nor r15, r16, r17 ; ld r25, r26 ; addi r5, r6, 5 }
	{ nor r15, r16, r17 ; ld r25, r26 ; rotl r5, r6, r7 }
	{ nor r15, r16, r17 ; ld1s r25, r26 ; fnop }
	{ nor r15, r16, r17 ; ld1s r25, r26 ; tblidxb1 r5, r6 }
	{ nor r15, r16, r17 ; ld1u r25, r26 ; nop }
	{ nor r15, r16, r17 ; ld2s r25, r26 ; cmpleu r5, r6, r7 }
	{ nor r15, r16, r17 ; ld2s r25, r26 ; shrsi r5, r6, 5 }
	{ nor r15, r16, r17 ; ld2u r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ nor r15, r16, r17 ; ld4s r25, r26 ; clz r5, r6 }
	{ nor r15, r16, r17 ; ld4s r25, r26 ; shl2add r5, r6, r7 }
	{ nor r15, r16, r17 ; ld4u r25, r26 ; movei r5, 5 }
	{ nor r15, r16, r17 ; mm r5, r6, 5, 7 }
	{ nor r15, r16, r17 ; movei r5, 5 ; ld1s r25, r26 }
	{ nor r15, r16, r17 ; mul_hs_lu r5, r6, r7 }
	{ nor r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ nor r15, r16, r17 ; mula_hs_hu r5, r6, r7 }
	{ nor r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; st2 r25, r26 }
	{ nor r15, r16, r17 ; mulax r5, r6, r7 ; st4 r25, r26 }
	{ nor r15, r16, r17 ; nop ; ld r25, r26 }
	{ nor r15, r16, r17 ; or r5, r6, r7 ; ld1u r25, r26 }
	{ nor r15, r16, r17 ; prefetch r25 ; addxi r5, r6, 5 }
	{ nor r15, r16, r17 ; prefetch r25 ; shl r5, r6, r7 }
	{ nor r15, r16, r17 ; prefetch_l1 r25 ; info 19 }
	{ nor r15, r16, r17 ; prefetch_l1 r25 ; tblidxb3 r5, r6 }
	{ nor r15, r16, r17 ; prefetch_l1_fault r25 ; or r5, r6, r7 }
	{ nor r15, r16, r17 ; prefetch_l2 r25 ; cmpltsi r5, r6, 5 }
	{ nor r15, r16, r17 ; prefetch_l2 r25 ; shrui r5, r6, 5 }
	{ nor r15, r16, r17 ; prefetch_l2_fault r25 ; mula_lu_lu r5, r6, r7 }
	{ nor r15, r16, r17 ; prefetch_l3 r25 ; cmovnez r5, r6, r7 }
	{ nor r15, r16, r17 ; prefetch_l3 r25 ; shl3add r5, r6, r7 }
	{ nor r15, r16, r17 ; prefetch_l3_fault r25 ; mul_hu_hu r5, r6, r7 }
	{ nor r15, r16, r17 ; revbits r5, r6 ; ld1u r25, r26 }
	{ nor r15, r16, r17 ; rotl r5, r6, r7 ; ld2u r25, r26 }
	{ nor r15, r16, r17 ; shl r5, r6, r7 ; ld4u r25, r26 }
	{ nor r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch r25 }
	{ nor r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
	{ nor r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
	{ nor r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2_fault r25 }
	{ nor r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l3_fault r25 }
	{ nor r15, r16, r17 ; st r25, r26 ; cmpleu r5, r6, r7 }
	{ nor r15, r16, r17 ; st r25, r26 ; shrsi r5, r6, 5 }
	{ nor r15, r16, r17 ; st1 r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ nor r15, r16, r17 ; st2 r25, r26 ; clz r5, r6 }
	{ nor r15, r16, r17 ; st2 r25, r26 ; shl2add r5, r6, r7 }
	{ nor r15, r16, r17 ; st4 r25, r26 ; movei r5, 5 }
	{ nor r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
	{ nor r15, r16, r17 ; tblidxb0 r5, r6 ; ld1s r25, r26 }
	{ nor r15, r16, r17 ; tblidxb2 r5, r6 ; ld2s r25, r26 }
	{ nor r15, r16, r17 ; v1cmpeq r5, r6, r7 }
	{ nor r15, r16, r17 ; v2add r5, r6, r7 }
	{ nor r15, r16, r17 ; v2shrui r5, r6, 5 }
	{ nor r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
	{ nor r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
	{ nor r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
	{ nor r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
	{ nor r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
	{ nor r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
	{ nor r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
	{ nor r5, r6, r7 ; finv r15 }
	{ nor r5, r6, r7 ; ill ; st4 r25, r26 }
	{ nor r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
	{ nor r5, r6, r7 ; jr r15 }
	{ nor r5, r6, r7 ; ld r25, r26 ; jr r15 }
	{ nor r5, r6, r7 ; ld1s r25, r26 ; cmpltsi r15, r16, 5 }
	{ nor r5, r6, r7 ; ld1u r25, r26 ; addx r15, r16, r17 }
	{ nor r5, r6, r7 ; ld1u r25, r26 ; shrui r15, r16, 5 }
	{ nor r5, r6, r7 ; ld2s r25, r26 ; shl1addx r15, r16, r17 }
	{ nor r5, r6, r7 ; ld2u r25, r26 ; movei r15, 5 }
	{ nor r5, r6, r7 ; ld4s r25, r26 ; ill }
	{ nor r5, r6, r7 ; ld4u r25, r26 ; cmpeq r15, r16, r17 }
	{ nor r5, r6, r7 ; ld4u r25, r26 }
	{ nor r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
	{ nor r5, r6, r7 ; movei r15, 5 ; ld1u r25, r26 }
	{ nor r5, r6, r7 ; nop ; ld1u r25, r26 }
	{ nor r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
	{ nor r5, r6, r7 ; prefetch r25 ; move r15, r16 }
	{ nor r5, r6, r7 ; prefetch_l1 r25 ; cmpleu r15, r16, r17 }
	{ nor r5, r6, r7 ; prefetch_l1_fault r25 ; addi r15, r16, 5 }
	{ nor r5, r6, r7 ; prefetch_l1_fault r25 ; shru r15, r16, r17 }
	{ nor r5, r6, r7 ; prefetch_l2 r25 ; shl1addx r15, r16, r17 }
	{ nor r5, r6, r7 ; prefetch_l2_fault r25 ; mz r15, r16, r17 }
	{ nor r5, r6, r7 ; prefetch_l3 r25 ; jalr r15 }
	{ nor r5, r6, r7 ; prefetch_l3_fault r25 ; cmpleu r15, r16, r17 }
	{ nor r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
	{ nor r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
	{ nor r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
	{ nor r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
	{ nor r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l1 r25 }
	{ nor r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l1 r25 }
	{ nor r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l2 r25 }
	{ nor r5, r6, r7 ; st r25, r26 ; cmpleu r15, r16, r17 }
	{ nor r5, r6, r7 ; st1 r25, r26 ; addi r15, r16, 5 }
	{ nor r5, r6, r7 ; st1 r25, r26 ; shru r15, r16, r17 }
	{ nor r5, r6, r7 ; st2 r25, r26 ; shl1add r15, r16, r17 }
	{ nor r5, r6, r7 ; st4 r25, r26 ; move r15, r16 }
	{ nor r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
	{ nor r5, r6, r7 ; v1cmplts r15, r16, r17 }
	{ nor r5, r6, r7 ; v2mz r15, r16, r17 }
	{ nor r5, r6, r7 ; xor r15, r16, r17 ; st1 r25, r26 }
	{ or r15, r16, r17 ; addi r5, r6, 5 ; st2 r25, r26 }
	{ or r15, r16, r17 ; addxi r5, r6, 5 ; st4 r25, r26 }
	{ or r15, r16, r17 ; andi r5, r6, 5 ; st4 r25, r26 }
	{ or r15, r16, r17 ; cmoveqz r5, r6, r7 ; st2 r25, r26 }
	{ or r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ or r15, r16, r17 ; cmpleu r5, r6, r7 ; ld1s r25, r26 }
	{ or r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld2s r25, r26 }
	{ or r15, r16, r17 ; cmpne r5, r6, r7 ; ld2u r25, r26 }
	{ or r15, r16, r17 ; ctz r5, r6 ; st2 r25, r26 }
	{ or r15, r16, r17 ; fsingle_pack1 r5, r6 ; ld1u r25, r26 }
	{ or r15, r16, r17 ; ld r25, r26 ; addi r5, r6, 5 }
	{ or r15, r16, r17 ; ld r25, r26 ; rotl r5, r6, r7 }
	{ or r15, r16, r17 ; ld1s r25, r26 ; fnop }
	{ or r15, r16, r17 ; ld1s r25, r26 ; tblidxb1 r5, r6 }
	{ or r15, r16, r17 ; ld1u r25, r26 ; nop }
	{ or r15, r16, r17 ; ld2s r25, r26 ; cmpleu r5, r6, r7 }
	{ or r15, r16, r17 ; ld2s r25, r26 ; shrsi r5, r6, 5 }
	{ or r15, r16, r17 ; ld2u r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ or r15, r16, r17 ; ld4s r25, r26 ; clz r5, r6 }
	{ or r15, r16, r17 ; ld4s r25, r26 ; shl2add r5, r6, r7 }
	{ or r15, r16, r17 ; ld4u r25, r26 ; movei r5, 5 }
	{ or r15, r16, r17 ; mm r5, r6, 5, 7 }
	{ or r15, r16, r17 ; movei r5, 5 ; ld1s r25, r26 }
	{ or r15, r16, r17 ; mul_hs_lu r5, r6, r7 }
	{ or r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ or r15, r16, r17 ; mula_hs_hu r5, r6, r7 }
	{ or r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; st2 r25, r26 }
	{ or r15, r16, r17 ; mulax r5, r6, r7 ; st4 r25, r26 }
	{ or r15, r16, r17 ; nop ; ld r25, r26 }
	{ or r15, r16, r17 ; or r5, r6, r7 ; ld1u r25, r26 }
	{ or r15, r16, r17 ; prefetch r25 ; addxi r5, r6, 5 }
	{ or r15, r16, r17 ; prefetch r25 ; shl r5, r6, r7 }
	{ or r15, r16, r17 ; prefetch_l1 r25 ; info 19 }
	{ or r15, r16, r17 ; prefetch_l1 r25 ; tblidxb3 r5, r6 }
	{ or r15, r16, r17 ; prefetch_l1_fault r25 ; or r5, r6, r7 }
	{ or r15, r16, r17 ; prefetch_l2 r25 ; cmpltsi r5, r6, 5 }
	{ or r15, r16, r17 ; prefetch_l2 r25 ; shrui r5, r6, 5 }
	{ or r15, r16, r17 ; prefetch_l2_fault r25 ; mula_lu_lu r5, r6, r7 }
	{ or r15, r16, r17 ; prefetch_l3 r25 ; cmovnez r5, r6, r7 }
	{ or r15, r16, r17 ; prefetch_l3 r25 ; shl3add r5, r6, r7 }
	{ or r15, r16, r17 ; prefetch_l3_fault r25 ; mul_hu_hu r5, r6, r7 }
	{ or r15, r16, r17 ; revbits r5, r6 ; ld1u r25, r26 }
	{ or r15, r16, r17 ; rotl r5, r6, r7 ; ld2u r25, r26 }
	{ or r15, r16, r17 ; shl r5, r6, r7 ; ld4u r25, r26 }
	{ or r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch r25 }
	{ or r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l1_fault r25 }
	{ or r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l2_fault r25 }
	{ or r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l2_fault r25 }
	{ or r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l3_fault r25 }
	{ or r15, r16, r17 ; st r25, r26 ; cmpleu r5, r6, r7 }
	{ or r15, r16, r17 ; st r25, r26 ; shrsi r5, r6, 5 }
	{ or r15, r16, r17 ; st1 r25, r26 ; mula_hu_hu r5, r6, r7 }
	{ or r15, r16, r17 ; st2 r25, r26 ; clz r5, r6 }
	{ or r15, r16, r17 ; st2 r25, r26 ; shl2add r5, r6, r7 }
	{ or r15, r16, r17 ; st4 r25, r26 ; movei r5, 5 }
	{ or r15, r16, r17 ; sub r5, r6, r7 ; ld r25, r26 }
	{ or r15, r16, r17 ; tblidxb0 r5, r6 ; ld1s r25, r26 }
	{ or r15, r16, r17 ; tblidxb2 r5, r6 ; ld2s r25, r26 }
	{ or r15, r16, r17 ; v1cmpeq r5, r6, r7 }
	{ or r15, r16, r17 ; v2add r5, r6, r7 }
	{ or r15, r16, r17 ; v2shrui r5, r6, 5 }
	{ or r5, r6, r7 ; add r15, r16, r17 ; ld4u r25, r26 }
	{ or r5, r6, r7 ; addx r15, r16, r17 ; prefetch r25 }
	{ or r5, r6, r7 ; and r15, r16, r17 ; prefetch r25 }
	{ or r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
	{ or r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
	{ or r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
	{ or r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
	{ or r5, r6, r7 ; finv r15 }
	{ or r5, r6, r7 ; ill ; st4 r25, r26 }
	{ or r5, r6, r7 ; jalr r15 ; st2 r25, r26 }
	{ or r5, r6, r7 ; jr r15 }
	{ or r5, r6, r7 ; ld r25, r26 ; jr r15 }
	{ or r5, r6, r7 ; ld1s r25, r26 ; cmpltsi r15, r16, 5 }
	{ or r5, r6, r7 ; ld1u r25, r26 ; addx r15, r16, r17 }
	{ or r5, r6, r7 ; ld1u r25, r26 ; shrui r15, r16, 5 }
	{ or r5, r6, r7 ; ld2s r25, r26 ; shl1addx r15, r16, r17 }
	{ or r5, r6, r7 ; ld2u r25, r26 ; movei r15, 5 }
	{ or r5, r6, r7 ; ld4s r25, r26 ; ill }
	{ or r5, r6, r7 ; ld4u r25, r26 ; cmpeq r15, r16, r17 }
	{ or r5, r6, r7 ; ld4u r25, r26 }
	{ or r5, r6, r7 ; mnz r15, r16, r17 ; ld r25, r26 }
	{ or r5, r6, r7 ; movei r15, 5 ; ld1u r25, r26 }
	{ or r5, r6, r7 ; nop ; ld1u r25, r26 }
	{ or r5, r6, r7 ; or r15, r16, r17 ; ld2u r25, r26 }
	{ or r5, r6, r7 ; prefetch r25 ; move r15, r16 }
	{ or r5, r6, r7 ; prefetch_l1 r25 ; cmpleu r15, r16, r17 }
	{ or r5, r6, r7 ; prefetch_l1_fault r25 ; addi r15, r16, 5 }
	{ or r5, r6, r7 ; prefetch_l1_fault r25 ; shru r15, r16, r17 }
	{ or r5, r6, r7 ; prefetch_l2 r25 ; shl1addx r15, r16, r17 }
	{ or r5, r6, r7 ; prefetch_l2_fault r25 ; mz r15, r16, r17 }
	{ or r5, r6, r7 ; prefetch_l3 r25 ; jalr r15 }
	{ or r5, r6, r7 ; prefetch_l3_fault r25 ; cmpleu r15, r16, r17 }
	{ or r5, r6, r7 ; rotl r15, r16, r17 ; ld1s r25, r26 }
	{ or r5, r6, r7 ; shl r15, r16, r17 ; ld2s r25, r26 }
	{ or r5, r6, r7 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
	{ or r5, r6, r7 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
	{ or r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l1 r25 }
	{ or r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l1 r25 }
	{ or r5, r6, r7 ; shru r15, r16, r17 ; prefetch_l2 r25 }
	{ or r5, r6, r7 ; st r25, r26 ; cmpleu r15, r16, r17 }
	{ or r5, r6, r7 ; st1 r25, r26 ; addi r15, r16, 5 }
	{ or r5, r6, r7 ; st1 r25, r26 ; shru r15, r16, r17 }
	{ or r5, r6, r7 ; st2 r25, r26 ; shl1add r15, r16, r17 }
	{ or r5, r6, r7 ; st4 r25, r26 ; move r15, r16 }
	{ or r5, r6, r7 ; sub r15, r16, r17 ; ld4u r25, r26 }
	{ or r5, r6, r7 ; v1cmplts r15, r16, r17 }
	{ or r5, r6, r7 ; v2mz r15, r16, r17 }
	{ or r5, r6, r7 ; xor r15, r16, r17 ; st1 r25, r26 }
	{ ori r15, r16, 5 ; dblalign2 r5, r6, r7 }
	{ ori r15, r16, 5 ; mula_hu_hu r5, r6, r7 }
	{ ori r15, r16, 5 ; tblidxb1 r5, r6 }
	{ ori r15, r16, 5 ; v1shl r5, r6, r7 }
	{ ori r15, r16, 5 ; v2sads r5, r6, r7 }
	{ ori r5, r6, 5 ; cmpltsi r15, r16, 5 }
	{ ori r5, r6, 5 ; ld2u_add r15, r16, 5 }
	{ ori r5, r6, 5 ; prefetch_add_l3 r15, 5 }
	{ ori r5, r6, 5 ; stnt2_add r15, r16, 5 }
	{ ori r5, r6, 5 ; v2cmples r15, r16, r17 }
	{ ori r5, r6, 5 ; xori r15, r16, 5 }
	{ pcnt r5, r6 ; addx r15, r16, r17 ; ld r25, r26 }
	{ pcnt r5, r6 ; and r15, r16, r17 ; ld r25, r26 }
	{ pcnt r5, r6 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
	{ pcnt r5, r6 ; cmples r15, r16, r17 ; ld1u r25, r26 }
	{ pcnt r5, r6 ; cmplts r15, r16, r17 ; ld2u r25, r26 }
	{ pcnt r5, r6 ; cmpltu r15, r16, r17 ; ld4u r25, r26 }
	{ pcnt r5, r6 ; fetchadd4 r15, r16, r17 }
	{ pcnt r5, r6 ; ill ; prefetch_l2 r25 }
	{ pcnt r5, r6 ; jalr r15 ; prefetch_l1_fault r25 }
	{ pcnt r5, r6 ; jr r15 ; prefetch_l2_fault r25 }
	{ pcnt r5, r6 ; ld r25, r26 ; cmpltu r15, r16, r17 }
	{ pcnt r5, r6 ; ld1s r25, r26 ; and r15, r16, r17 }
	{ pcnt r5, r6 ; ld1s r25, r26 ; subx r15, r16, r17 }
	{ pcnt r5, r6 ; ld1u r25, r26 ; shl2addx r15, r16, r17 }
	{ pcnt r5, r6 ; ld2s r25, r26 ; nop }
	{ pcnt r5, r6 ; ld2u r25, r26 ; jalr r15 }
	{ pcnt r5, r6 ; ld4s r25, r26 ; cmples r15, r16, r17 }
	{ pcnt r5, r6 ; ld4u r15, r16 }
	{ pcnt r5, r6 ; ld4u r25, r26 ; shrs r15, r16, r17 }
	{ pcnt r5, r6 ; lnk r15 ; st r25, r26 }
	{ pcnt r5, r6 ; move r15, r16 ; st r25, r26 }
	{ pcnt r5, r6 ; mz r15, r16, r17 ; st r25, r26 }
	{ pcnt r5, r6 ; nor r15, r16, r17 ; st2 r25, r26 }
	{ pcnt r5, r6 ; prefetch r25 ; info 19 }
	{ pcnt r5, r6 ; prefetch_l1 r25 ; addx r15, r16, r17 }
	{ pcnt r5, r6 ; prefetch_l1 r25 ; shrui r15, r16, 5 }
	{ pcnt r5, r6 ; prefetch_l1_fault r25 ; shl2add r15, r16, r17 }
	{ pcnt r5, r6 ; prefetch_l2 r25 ; nop }
	{ pcnt r5, r6 ; prefetch_l2_fault r25 ; jalrp r15 }
	{ pcnt r5, r6 ; prefetch_l3 r25 ; cmplts r15, r16, r17 }
	{ pcnt r5, r6 ; prefetch_l3_fault r25 ; addx r15, r16, r17 }
	{ pcnt r5, r6 ; prefetch_l3_fault r25 ; shrui r15, r16, 5 }
	{ pcnt r5, r6 ; rotli r15, r16, 5 ; st1 r25, r26 }
	{ pcnt r5, r6 ; shl1add r15, r16, r17 ; st2 r25, r26 }
	{ pcnt r5, r6 ; shl2add r15, r16, r17 }
	{ pcnt r5, r6 ; shl3addx r15, r16, r17 ; ld1s r25, r26 }
	{ pcnt r5, r6 ; shrs r15, r16, r17 ; ld1s r25, r26 }
	{ pcnt r5, r6 ; shru r15, r16, r17 ; ld2s r25, r26 }
	{ pcnt r5, r6 ; st r25, r26 ; addx r15, r16, r17 }
	{ pcnt r5, r6 ; st r25, r26 ; shrui r15, r16, 5 }
	{ pcnt r5, r6 ; st1 r25, r26 ; shl2add r15, r16, r17 }
	{ pcnt r5, r6 ; st2 r25, r26 ; mz r15, r16, r17 }
	{ pcnt r5, r6 ; st4 r25, r26 ; info 19 }
	{ pcnt r5, r6 ; stnt_add r15, r16, 5 }
	{ pcnt r5, r6 ; v1add r15, r16, r17 }
	{ pcnt r5, r6 ; v2int_h r15, r16, r17 }
	{ pcnt r5, r6 ; xor r15, r16, r17 ; prefetch_l1 r25 }
	{ prefetch r15 ; cmulfr r5, r6, r7 }
	{ prefetch r15 ; mul_ls_ls r5, r6, r7 }
	{ prefetch r15 ; shrux r5, r6, r7 }
	{ prefetch r15 ; v1mnz r5, r6, r7 }
	{ prefetch r15 ; v2mults r5, r6, r7 }
	{ prefetch r25 ; add r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ prefetch r25 ; add r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ prefetch r25 ; add r5, r6, r7 ; nop }
	{ prefetch r25 ; addi r15, r16, 5 ; fsingle_pack1 r5, r6 }
	{ prefetch r25 ; addi r15, r16, 5 ; tblidxb2 r5, r6 }
	{ prefetch r25 ; addi r5, r6, 5 ; shl3add r15, r16, r17 }
	{ prefetch r25 ; addx r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ prefetch r25 ; addx r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch r25 ; addx r5, r6, r7 }
	{ prefetch r25 ; addxi r15, r16, 5 ; revbits r5, r6 }
	{ prefetch r25 ; addxi r5, r6, 5 ; info 19 }
	{ prefetch r25 ; and r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ prefetch r25 ; and r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ prefetch r25 ; and r5, r6, r7 ; nop }
	{ prefetch r25 ; andi r15, r16, 5 ; fsingle_pack1 r5, r6 }
	{ prefetch r25 ; andi r15, r16, 5 ; tblidxb2 r5, r6 }
	{ prefetch r25 ; andi r5, r6, 5 ; shl3add r15, r16, r17 }
	{ prefetch r25 ; clz r5, r6 ; rotl r15, r16, r17 }
	{ prefetch r25 ; cmoveqz r5, r6, r7 ; mnz r15, r16, r17 }
	{ prefetch r25 ; cmovnez r5, r6, r7 ; ill }
	{ prefetch r25 ; cmpeq r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ prefetch r25 ; cmpeq r15, r16, r17 ; shl3add r5, r6, r7 }
	{ prefetch r25 ; cmpeq r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch r25 ; cmpeqi r15, r16, 5 ; fnop }
	{ prefetch r25 ; cmpeqi r15, r16, 5 ; tblidxb1 r5, r6 }
	{ prefetch r25 ; cmpeqi r5, r6, 5 ; shl2addx r15, r16, r17 }
	{ prefetch r25 ; cmples r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ prefetch r25 ; cmples r5, r6, r7 ; andi r15, r16, 5 }
	{ prefetch r25 ; cmples r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch r25 ; cmpleu r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch r25 ; cmpleu r5, r6, r7 ; ill }
	{ prefetch r25 ; cmplts r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ prefetch r25 ; cmplts r15, r16, r17 ; shl3add r5, r6, r7 }
	{ prefetch r25 ; cmplts r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch r25 ; cmpltsi r15, r16, 5 ; fnop }
	{ prefetch r25 ; cmpltsi r15, r16, 5 ; tblidxb1 r5, r6 }
	{ prefetch r25 ; cmpltsi r5, r6, 5 ; shl2addx r15, r16, r17 }
	{ prefetch r25 ; cmpltu r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ prefetch r25 ; cmpltu r5, r6, r7 ; andi r15, r16, 5 }
	{ prefetch r25 ; cmpltu r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch r25 ; cmpne r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch r25 ; cmpne r5, r6, r7 ; ill }
	{ prefetch r25 ; ctz r5, r6 ; cmples r15, r16, r17 }
	{ prefetch r25 ; fnop ; add r5, r6, r7 }
	{ prefetch r25 ; fnop ; mnz r15, r16, r17 }
	{ prefetch r25 ; fnop ; shl3add r15, r16, r17 }
	{ prefetch r25 ; fsingle_pack1 r5, r6 ; ill }
	{ prefetch r25 ; ill ; cmovnez r5, r6, r7 }
	{ prefetch r25 ; ill ; shl3add r5, r6, r7 }
	{ prefetch r25 ; info 19 ; cmpltsi r15, r16, 5 }
	{ prefetch r25 ; info 19 ; revbits r5, r6 }
	{ prefetch r25 ; info 19 }
	{ prefetch r25 ; jalr r15 ; revbits r5, r6 }
	{ prefetch r25 ; jalrp r15 ; cmpne r5, r6, r7 }
	{ prefetch r25 ; jalrp r15 ; subx r5, r6, r7 }
	{ prefetch r25 ; jr r15 ; mulx r5, r6, r7 }
	{ prefetch r25 ; jrp r15 ; cmpeqi r5, r6, 5 }
	{ prefetch r25 ; jrp r15 ; shli r5, r6, 5 }
	{ prefetch r25 ; lnk r15 ; mul_lu_lu r5, r6, r7 }
	{ prefetch r25 ; mnz r15, r16, r17 ; and r5, r6, r7 }
	{ prefetch r25 ; mnz r15, r16, r17 ; shl1add r5, r6, r7 }
	{ prefetch r25 ; mnz r5, r6, r7 ; lnk r15 }
	{ prefetch r25 ; move r15, r16 ; cmpltsi r5, r6, 5 }
	{ prefetch r25 ; move r15, r16 ; shrui r5, r6, 5 }
	{ prefetch r25 ; move r5, r6 ; shl r15, r16, r17 }
	{ prefetch r25 ; movei r15, 5 ; mul_hs_hs r5, r6, r7 }
	{ prefetch r25 ; movei r5, 5 ; addi r15, r16, 5 }
	{ prefetch r25 ; movei r5, 5 ; shru r15, r16, r17 }
	{ prefetch r25 ; mul_hs_hs r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch r25 ; mul_hu_hu r5, r6, r7 ; nor r15, r16, r17 }
	{ prefetch r25 ; mul_ls_ls r5, r6, r7 ; jrp r15 }
	{ prefetch r25 ; mul_lu_lu r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch r25 ; mula_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch r25 ; mula_hs_hs r5, r6, r7 }
	{ prefetch r25 ; mula_hu_hu r5, r6, r7 ; shrs r15, r16, r17 }
	{ prefetch r25 ; mula_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch r25 ; mula_lu_lu r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch r25 ; mulax r5, r6, r7 ; jalrp r15 }
	{ prefetch r25 ; mulx r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ prefetch r25 ; mz r15, r16, r17 ; and r5, r6, r7 }
	{ prefetch r25 ; mz r15, r16, r17 ; shl1add r5, r6, r7 }
	{ prefetch r25 ; mz r5, r6, r7 ; lnk r15 }
	{ prefetch r25 ; nop ; cmovnez r5, r6, r7 }
	{ prefetch r25 ; nop ; mula_lu_lu r5, r6, r7 }
	{ prefetch r25 ; nop ; shrui r5, r6, 5 }
	{ prefetch r25 ; nor r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ prefetch r25 ; nor r5, r6, r7 ; andi r15, r16, 5 }
	{ prefetch r25 ; nor r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch r25 ; or r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch r25 ; or r5, r6, r7 ; ill }
	{ prefetch r25 ; pcnt r5, r6 ; cmples r15, r16, r17 }
	{ prefetch r25 ; revbits r5, r6 ; addi r15, r16, 5 }
	{ prefetch r25 ; revbits r5, r6 ; shru r15, r16, r17 }
	{ prefetch r25 ; revbytes r5, r6 ; shl2add r15, r16, r17 }
	{ prefetch r25 ; rotl r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch r25 ; rotl r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch r25 ; rotl r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch r25 ; rotli r15, r16, 5 ; or r5, r6, r7 }
	{ prefetch r25 ; rotli r5, r6, 5 ; fnop }
	{ prefetch r25 ; shl r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ prefetch r25 ; shl r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ prefetch r25 ; shl r5, r6, r7 ; movei r15, 5 }
	{ prefetch r25 ; shl1add r15, r16, r17 ; ctz r5, r6 }
	{ prefetch r25 ; shl1add r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch r25 ; shl1add r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch r25 ; shl1addx r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch r25 ; shl1addx r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch r25 ; shl1addx r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch r25 ; shl2add r15, r16, r17 ; or r5, r6, r7 }
	{ prefetch r25 ; shl2add r5, r6, r7 ; fnop }
	{ prefetch r25 ; shl2addx r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ prefetch r25 ; shl2addx r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ prefetch r25 ; shl2addx r5, r6, r7 ; movei r15, 5 }
	{ prefetch r25 ; shl3add r15, r16, r17 ; ctz r5, r6 }
	{ prefetch r25 ; shl3add r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch r25 ; shl3add r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch r25 ; shl3addx r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch r25 ; shl3addx r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch r25 ; shl3addx r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch r25 ; shli r15, r16, 5 ; or r5, r6, r7 }
	{ prefetch r25 ; shli r5, r6, 5 ; fnop }
	{ prefetch r25 ; shrs r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ prefetch r25 ; shrs r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ prefetch r25 ; shrs r5, r6, r7 ; movei r15, 5 }
	{ prefetch r25 ; shrsi r15, r16, 5 ; ctz r5, r6 }
	{ prefetch r25 ; shrsi r15, r16, 5 ; tblidxb0 r5, r6 }
	{ prefetch r25 ; shrsi r5, r6, 5 ; shl2add r15, r16, r17 }
	{ prefetch r25 ; shru r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch r25 ; shru r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch r25 ; shru r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch r25 ; shrui r15, r16, 5 ; or r5, r6, r7 }
	{ prefetch r25 ; shrui r5, r6, 5 ; fnop }
	{ prefetch r25 ; sub r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ prefetch r25 ; sub r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ prefetch r25 ; sub r5, r6, r7 ; movei r15, 5 }
	{ prefetch r25 ; subx r15, r16, r17 ; ctz r5, r6 }
	{ prefetch r25 ; subx r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch r25 ; subx r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch r25 ; tblidxb0 r5, r6 ; nor r15, r16, r17 }
	{ prefetch r25 ; tblidxb1 r5, r6 ; jrp r15 }
	{ prefetch r25 ; tblidxb2 r5, r6 ; cmpne r15, r16, r17 }
	{ prefetch r25 ; tblidxb3 r5, r6 ; cmpeq r15, r16, r17 }
	{ prefetch r25 ; tblidxb3 r5, r6 }
	{ prefetch r25 ; xor r15, r16, r17 ; revbits r5, r6 }
	{ prefetch r25 ; xor r5, r6, r7 ; info 19 }
	{ prefetch_add_l1 r15, 5 ; bfexts r5, r6, 5, 7 }
	{ prefetch_add_l1 r15, 5 ; fsingle_mul1 r5, r6, r7 }
	{ prefetch_add_l1 r15, 5 ; revbits r5, r6 }
	{ prefetch_add_l1 r15, 5 ; v1cmpltu r5, r6, r7 }
	{ prefetch_add_l1 r15, 5 ; v2cmpeqi r5, r6, 5 }
	{ prefetch_add_l1 r15, 5 ; v4int_l r5, r6, r7 }
	{ prefetch_add_l1_fault r15, 5 ; cmulhr r5, r6, r7 }
	{ prefetch_add_l1_fault r15, 5 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_add_l1_fault r15, 5 ; shufflebytes r5, r6, r7 }
	{ prefetch_add_l1_fault r15, 5 ; v1mulu r5, r6, r7 }
	{ prefetch_add_l1_fault r15, 5 ; v2packh r5, r6, r7 }
	{ prefetch_add_l2 r15, 5 ; bfins r5, r6, 5, 7 }
	{ prefetch_add_l2 r15, 5 ; fsingle_pack1 r5, r6 }
	{ prefetch_add_l2 r15, 5 ; rotl r5, r6, r7 }
	{ prefetch_add_l2 r15, 5 ; v1cmpne r5, r6, r7 }
	{ prefetch_add_l2 r15, 5 ; v2cmpleu r5, r6, r7 }
	{ prefetch_add_l2 r15, 5 ; v4shl r5, r6, r7 }
	{ prefetch_add_l2_fault r15, 5 ; crc32_8 r5, r6, r7 }
	{ prefetch_add_l2_fault r15, 5 ; mula_hs_hu r5, r6, r7 }
	{ prefetch_add_l2_fault r15, 5 ; subx r5, r6, r7 }
	{ prefetch_add_l2_fault r15, 5 ; v1mz r5, r6, r7 }
	{ prefetch_add_l2_fault r15, 5 ; v2packuc r5, r6, r7 }
	{ prefetch_add_l3 r15, 5 ; cmoveqz r5, r6, r7 }
	{ prefetch_add_l3 r15, 5 ; fsingle_sub1 r5, r6, r7 }
	{ prefetch_add_l3 r15, 5 ; shl r5, r6, r7 }
	{ prefetch_add_l3 r15, 5 ; v1ddotpua r5, r6, r7 }
	{ prefetch_add_l3 r15, 5 ; v2cmpltsi r5, r6, 5 }
	{ prefetch_add_l3 r15, 5 ; v4shrs r5, r6, r7 }
	{ prefetch_add_l3_fault r15, 5 ; dblalign r5, r6, r7 }
	{ prefetch_add_l3_fault r15, 5 ; mula_hs_lu r5, r6, r7 }
	{ prefetch_add_l3_fault r15, 5 ; tblidxb0 r5, r6 }
	{ prefetch_add_l3_fault r15, 5 ; v1sadu r5, r6, r7 }
	{ prefetch_add_l3_fault r15, 5 ; v2sadau r5, r6, r7 }
	{ prefetch_l1 r15 ; cmpeq r5, r6, r7 }
	{ prefetch_l1 r15 ; infol 0x1234 }
	{ prefetch_l1 r15 ; shl1add r5, r6, r7 }
	{ prefetch_l1 r15 ; v1ddotpusa r5, r6, r7 }
	{ prefetch_l1 r15 ; v2cmpltui r5, r6, 5 }
	{ prefetch_l1 r15 ; v4sub r5, r6, r7 }
	{ prefetch_l1 r25 ; add r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l1 r25 ; add r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l1 r25 ; addi r15, r16, 5 ; clz r5, r6 }
	{ prefetch_l1 r25 ; addi r15, r16, 5 ; shl2add r5, r6, r7 }
	{ prefetch_l1 r25 ; addi r5, r6, 5 ; move r15, r16 }
	{ prefetch_l1 r25 ; addx r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l1 r25 ; addx r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l1 r25 ; addx r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l1 r25 ; addxi r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l1 r25 ; addxi r5, r6, 5 ; addxi r15, r16, 5 }
	{ prefetch_l1 r25 ; addxi r5, r6, 5 ; sub r15, r16, r17 }
	{ prefetch_l1 r25 ; and r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l1 r25 ; and r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l1 r25 ; andi r15, r16, 5 ; clz r5, r6 }
	{ prefetch_l1 r25 ; andi r15, r16, 5 ; shl2add r5, r6, r7 }
	{ prefetch_l1 r25 ; andi r5, r6, 5 ; move r15, r16 }
	{ prefetch_l1 r25 ; clz r5, r6 ; info 19 }
	{ prefetch_l1 r25 ; cmoveqz r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ prefetch_l1 r25 ; cmovnez r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l1 r25 ; cmovnez r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l1 r25 ; cmpeq r15, r16, r17 ; nop }
	{ prefetch_l1 r25 ; cmpeq r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l1 r25 ; cmpeqi r15, r16, 5 ; andi r5, r6, 5 }
	{ prefetch_l1 r25 ; cmpeqi r15, r16, 5 ; shl1addx r5, r6, r7 }
	{ prefetch_l1 r25 ; cmpeqi r5, r6, 5 ; mnz r15, r16, r17 }
	{ prefetch_l1 r25 ; cmples r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ prefetch_l1 r25 ; cmples r15, r16, r17 ; sub r5, r6, r7 }
	{ prefetch_l1 r25 ; cmples r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l1 r25 ; cmpleu r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ prefetch_l1 r25 ; cmpleu r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l1 r25 ; cmpleu r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l1 r25 ; cmplts r15, r16, r17 ; nop }
	{ prefetch_l1 r25 ; cmplts r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l1 r25 ; cmpltsi r15, r16, 5 ; andi r5, r6, 5 }
	{ prefetch_l1 r25 ; cmpltsi r15, r16, 5 ; shl1addx r5, r6, r7 }
	{ prefetch_l1 r25 ; cmpltsi r5, r6, 5 ; mnz r15, r16, r17 }
	{ prefetch_l1 r25 ; cmpltu r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ prefetch_l1 r25 ; cmpltu r15, r16, r17 ; sub r5, r6, r7 }
	{ prefetch_l1 r25 ; cmpltu r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l1 r25 ; cmpne r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ prefetch_l1 r25 ; cmpne r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l1 r25 ; cmpne r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l1 r25 ; ctz r5, r6 ; shl2addx r15, r16, r17 }
	{ prefetch_l1 r25 ; fnop ; cmpltu r5, r6, r7 }
	{ prefetch_l1 r25 ; fnop ; rotl r5, r6, r7 }
	{ prefetch_l1 r25 ; fsingle_pack1 r5, r6 ; addx r15, r16, r17 }
	{ prefetch_l1 r25 ; fsingle_pack1 r5, r6 ; shrui r15, r16, 5 }
	{ prefetch_l1 r25 ; ill ; nop }
	{ prefetch_l1 r25 ; info 19 ; clz r5, r6 }
	{ prefetch_l1 r25 ; info 19 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l1 r25 ; info 19 ; shru r5, r6, r7 }
	{ prefetch_l1 r25 ; jalr r15 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l1 r25 ; jalrp r15 ; addxi r5, r6, 5 }
	{ prefetch_l1 r25 ; jalrp r15 ; shl r5, r6, r7 }
	{ prefetch_l1 r25 ; jr r15 ; info 19 }
	{ prefetch_l1 r25 ; jr r15 ; tblidxb3 r5, r6 }
	{ prefetch_l1 r25 ; jrp r15 ; or r5, r6, r7 }
	{ prefetch_l1 r25 ; lnk r15 ; cmpltsi r5, r6, 5 }
	{ prefetch_l1 r25 ; lnk r15 ; shrui r5, r6, 5 }
	{ prefetch_l1 r25 ; mnz r15, r16, r17 ; mula_lu_lu r5, r6, r7 }
	{ prefetch_l1 r25 ; mnz r5, r6, r7 ; cmples r15, r16, r17 }
	{ prefetch_l1 r25 ; move r15, r16 ; addi r5, r6, 5 }
	{ prefetch_l1 r25 ; move r15, r16 ; rotl r5, r6, r7 }
	{ prefetch_l1 r25 ; move r5, r6 ; jalrp r15 }
	{ prefetch_l1 r25 ; movei r15, 5 ; cmples r5, r6, r7 }
	{ prefetch_l1 r25 ; movei r15, 5 ; shrs r5, r6, r7 }
	{ prefetch_l1 r25 ; movei r5, 5 ; or r15, r16, r17 }
	{ prefetch_l1 r25 ; mul_hs_hs r5, r6, r7 ; lnk r15 }
	{ prefetch_l1 r25 ; mul_hu_hu r5, r6, r7 ; fnop }
	{ prefetch_l1 r25 ; mul_ls_ls r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ prefetch_l1 r25 ; mul_lu_lu r5, r6, r7 ; add r15, r16, r17 }
	{ prefetch_l1 r25 ; mul_lu_lu r5, r6, r7 ; shrsi r15, r16, 5 }
	{ prefetch_l1 r25 ; mula_hs_hs r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l1 r25 ; mula_hu_hu r5, r6, r7 ; nop }
	{ prefetch_l1 r25 ; mula_ls_ls r5, r6, r7 ; jr r15 }
	{ prefetch_l1 r25 ; mula_lu_lu r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l1 r25 ; mulax r5, r6, r7 ; andi r15, r16, 5 }
	{ prefetch_l1 r25 ; mulax r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch_l1 r25 ; mulx r5, r6, r7 ; shli r15, r16, 5 }
	{ prefetch_l1 r25 ; mz r15, r16, r17 ; mula_lu_lu r5, r6, r7 }
	{ prefetch_l1 r25 ; mz r5, r6, r7 ; cmples r15, r16, r17 }
	{ prefetch_l1 r25 ; nop ; add r5, r6, r7 }
	{ prefetch_l1 r25 ; nop ; mnz r15, r16, r17 }
	{ prefetch_l1 r25 ; nop ; shl3add r15, r16, r17 }
	{ prefetch_l1 r25 ; nor r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ prefetch_l1 r25 ; nor r15, r16, r17 ; sub r5, r6, r7 }
	{ prefetch_l1 r25 ; nor r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l1 r25 ; or r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ prefetch_l1 r25 ; or r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l1 r25 ; or r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l1 r25 ; pcnt r5, r6 ; shl2addx r15, r16, r17 }
	{ prefetch_l1 r25 ; revbits r5, r6 ; or r15, r16, r17 }
	{ prefetch_l1 r25 ; revbytes r5, r6 ; lnk r15 }
	{ prefetch_l1 r25 ; rotl r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ prefetch_l1 r25 ; rotl r15, r16, r17 ; shrui r5, r6, 5 }
	{ prefetch_l1 r25 ; rotl r5, r6, r7 ; shl r15, r16, r17 }
	{ prefetch_l1 r25 ; rotli r15, r16, 5 ; mul_hs_hs r5, r6, r7 }
	{ prefetch_l1 r25 ; rotli r5, r6, 5 ; addi r15, r16, 5 }
	{ prefetch_l1 r25 ; rotli r5, r6, 5 ; shru r15, r16, r17 }
	{ prefetch_l1 r25 ; shl r15, r16, r17 ; mz r5, r6, r7 }
	{ prefetch_l1 r25 ; shl r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ prefetch_l1 r25 ; shl1add r15, r16, r17 ; and r5, r6, r7 }
	{ prefetch_l1 r25 ; shl1add r15, r16, r17 ; shl1add r5, r6, r7 }
	{ prefetch_l1 r25 ; shl1add r5, r6, r7 ; lnk r15 }
	{ prefetch_l1 r25 ; shl1addx r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ prefetch_l1 r25 ; shl1addx r15, r16, r17 ; shrui r5, r6, 5 }
	{ prefetch_l1 r25 ; shl1addx r5, r6, r7 ; shl r15, r16, r17 }
	{ prefetch_l1 r25 ; shl2add r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ prefetch_l1 r25 ; shl2add r5, r6, r7 ; addi r15, r16, 5 }
	{ prefetch_l1 r25 ; shl2add r5, r6, r7 ; shru r15, r16, r17 }
	{ prefetch_l1 r25 ; shl2addx r15, r16, r17 ; mz r5, r6, r7 }
	{ prefetch_l1 r25 ; shl2addx r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ prefetch_l1 r25 ; shl3add r15, r16, r17 ; and r5, r6, r7 }
	{ prefetch_l1 r25 ; shl3add r15, r16, r17 ; shl1add r5, r6, r7 }
	{ prefetch_l1 r25 ; shl3add r5, r6, r7 ; lnk r15 }
	{ prefetch_l1 r25 ; shl3addx r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ prefetch_l1 r25 ; shl3addx r15, r16, r17 ; shrui r5, r6, 5 }
	{ prefetch_l1 r25 ; shl3addx r5, r6, r7 ; shl r15, r16, r17 }
	{ prefetch_l1 r25 ; shli r15, r16, 5 ; mul_hs_hs r5, r6, r7 }
	{ prefetch_l1 r25 ; shli r5, r6, 5 ; addi r15, r16, 5 }
	{ prefetch_l1 r25 ; shli r5, r6, 5 ; shru r15, r16, r17 }
	{ prefetch_l1 r25 ; shrs r15, r16, r17 ; mz r5, r6, r7 }
	{ prefetch_l1 r25 ; shrs r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ prefetch_l1 r25 ; shrsi r15, r16, 5 ; and r5, r6, r7 }
	{ prefetch_l1 r25 ; shrsi r15, r16, 5 ; shl1add r5, r6, r7 }
	{ prefetch_l1 r25 ; shrsi r5, r6, 5 ; lnk r15 }
	{ prefetch_l1 r25 ; shru r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ prefetch_l1 r25 ; shru r15, r16, r17 ; shrui r5, r6, 5 }
	{ prefetch_l1 r25 ; shru r5, r6, r7 ; shl r15, r16, r17 }
	{ prefetch_l1 r25 ; shrui r15, r16, 5 ; mul_hs_hs r5, r6, r7 }
	{ prefetch_l1 r25 ; shrui r5, r6, 5 ; addi r15, r16, 5 }
	{ prefetch_l1 r25 ; shrui r5, r6, 5 ; shru r15, r16, r17 }
	{ prefetch_l1 r25 ; sub r15, r16, r17 ; mz r5, r6, r7 }
	{ prefetch_l1 r25 ; sub r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ prefetch_l1 r25 ; subx r15, r16, r17 ; and r5, r6, r7 }
	{ prefetch_l1 r25 ; subx r15, r16, r17 ; shl1add r5, r6, r7 }
	{ prefetch_l1 r25 ; subx r5, r6, r7 ; lnk r15 }
	{ prefetch_l1 r25 ; tblidxb0 r5, r6 ; fnop }
	{ prefetch_l1 r25 ; tblidxb1 r5, r6 ; cmpeqi r15, r16, 5 }
	{ prefetch_l1 r25 ; tblidxb2 r5, r6 ; add r15, r16, r17 }
	{ prefetch_l1 r25 ; tblidxb2 r5, r6 ; shrsi r15, r16, 5 }
	{ prefetch_l1 r25 ; tblidxb3 r5, r6 ; shl1addx r15, r16, r17 }
	{ prefetch_l1 r25 ; xor r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l1 r25 ; xor r5, r6, r7 ; addxi r15, r16, 5 }
	{ prefetch_l1 r25 ; xor r5, r6, r7 ; sub r15, r16, r17 }
	{ prefetch_l1_fault r15 ; dblalign4 r5, r6, r7 }
	{ prefetch_l1_fault r15 ; mula_hu_ls r5, r6, r7 }
	{ prefetch_l1_fault r15 ; tblidxb2 r5, r6 }
	{ prefetch_l1_fault r15 ; v1shli r5, r6, 5 }
	{ prefetch_l1_fault r15 ; v2sadu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; add r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l1_fault r25 ; add r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l1_fault r25 ; add r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l1_fault r25 ; addi r15, r16, 5 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; addi r5, r6, 5 ; and r15, r16, r17 }
	{ prefetch_l1_fault r25 ; addi r5, r6, 5 ; subx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; addx r15, r16, r17 ; or r5, r6, r7 }
	{ prefetch_l1_fault r25 ; addx r5, r6, r7 ; fnop }
	{ prefetch_l1_fault r25 ; addxi r15, r16, 5 ; cmoveqz r5, r6, r7 }
	{ prefetch_l1_fault r25 ; addxi r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; addxi r5, r6, 5 ; movei r15, 5 }
	{ prefetch_l1_fault r25 ; and r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l1_fault r25 ; and r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l1_fault r25 ; and r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l1_fault r25 ; andi r15, r16, 5 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; andi r5, r6, 5 ; and r15, r16, r17 }
	{ prefetch_l1_fault r25 ; andi r5, r6, 5 ; subx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; clz r5, r6 ; shl3addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; cmoveqz r5, r6, r7 ; rotli r15, r16, 5 }
	{ prefetch_l1_fault r25 ; cmovnez r5, r6, r7 ; move r15, r16 }
	{ prefetch_l1_fault r25 ; cmpeq r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmpeq r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmpeq r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; cmpeqi r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmpeqi r5, r6, 5 ; addxi r15, r16, 5 }
	{ prefetch_l1_fault r25 ; cmpeqi r5, r6, 5 ; sub r15, r16, r17 }
	{ prefetch_l1_fault r25 ; cmples r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmples r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l1_fault r25 ; cmpleu r15, r16, r17 ; clz r5, r6 }
	{ prefetch_l1_fault r25 ; cmpleu r15, r16, r17 ; shl2add r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmpleu r5, r6, r7 ; move r15, r16 }
	{ prefetch_l1_fault r25 ; cmplts r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmplts r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmplts r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; cmpltsi r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmpltsi r5, r6, 5 ; addxi r15, r16, 5 }
	{ prefetch_l1_fault r25 ; cmpltsi r5, r6, 5 ; sub r15, r16, r17 }
	{ prefetch_l1_fault r25 ; cmpltu r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmpltu r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l1_fault r25 ; cmpne r15, r16, r17 ; clz r5, r6 }
	{ prefetch_l1_fault r25 ; cmpne r15, r16, r17 ; shl2add r5, r6, r7 }
	{ prefetch_l1_fault r25 ; cmpne r5, r6, r7 ; move r15, r16 }
	{ prefetch_l1_fault r25 ; ctz r5, r6 ; info 19 }
	{ prefetch_l1_fault r25 ; fnop ; and r5, r6, r7 }
	{ prefetch_l1_fault r25 ; fnop ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l1_fault r25 ; fnop ; shrsi r15, r16, 5 }
	{ prefetch_l1_fault r25 ; fsingle_pack1 r5, r6 ; move r15, r16 }
	{ prefetch_l1_fault r25 ; ill ; cmpne r5, r6, r7 }
	{ prefetch_l1_fault r25 ; ill ; subx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; info 19 ; fsingle_pack1 r5, r6 }
	{ prefetch_l1_fault r25 ; info 19 ; shl1add r15, r16, r17 }
	{ prefetch_l1_fault r25 ; jalr r15 ; cmoveqz r5, r6, r7 }
	{ prefetch_l1_fault r25 ; jalr r15 ; shl2addx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; jalrp r15 ; mul_hs_hs r5, r6, r7 }
	{ prefetch_l1_fault r25 ; jr r15 ; addi r5, r6, 5 }
	{ prefetch_l1_fault r25 ; jr r15 ; rotl r5, r6, r7 }
	{ prefetch_l1_fault r25 ; jrp r15 ; fnop }
	{ prefetch_l1_fault r25 ; jrp r15 ; tblidxb1 r5, r6 }
	{ prefetch_l1_fault r25 ; lnk r15 ; nop }
	{ prefetch_l1_fault r25 ; mnz r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; mnz r15, r16, r17 ; shrsi r5, r6, 5 }
	{ prefetch_l1_fault r25 ; mnz r5, r6, r7 ; rotl r15, r16, r17 }
	{ prefetch_l1_fault r25 ; move r15, r16 ; move r5, r6 }
	{ prefetch_l1_fault r25 ; move r15, r16 }
	{ prefetch_l1_fault r25 ; move r5, r6 ; shrs r15, r16, r17 }
	{ prefetch_l1_fault r25 ; movei r15, 5 ; mulax r5, r6, r7 }
	{ prefetch_l1_fault r25 ; movei r5, 5 ; cmpleu r15, r16, r17 }
	{ prefetch_l1_fault r25 ; mul_hs_hs r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; mul_hs_hs r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l1_fault r25 ; mul_hu_hu r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 ; or r15, r16, r17 }
	{ prefetch_l1_fault r25 ; mul_lu_lu r5, r6, r7 ; lnk r15 }
	{ prefetch_l1_fault r25 ; mula_hs_hs r5, r6, r7 ; fnop }
	{ prefetch_l1_fault r25 ; mula_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ prefetch_l1_fault r25 ; mula_ls_ls r5, r6, r7 ; add r15, r16, r17 }
	{ prefetch_l1_fault r25 ; mula_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 }
	{ prefetch_l1_fault r25 ; mula_lu_lu r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; mulax r5, r6, r7 ; nop }
	{ prefetch_l1_fault r25 ; mulx r5, r6, r7 ; jr r15 }
	{ prefetch_l1_fault r25 ; mz r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; mz r15, r16, r17 ; shrsi r5, r6, 5 }
	{ prefetch_l1_fault r25 ; mz r5, r6, r7 ; rotl r15, r16, r17 }
	{ prefetch_l1_fault r25 ; nop ; cmpleu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; nop ; or r15, r16, r17 }
	{ prefetch_l1_fault r25 ; nop ; tblidxb3 r5, r6 }
	{ prefetch_l1_fault r25 ; nor r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l1_fault r25 ; nor r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l1_fault r25 ; or r15, r16, r17 ; clz r5, r6 }
	{ prefetch_l1_fault r25 ; or r15, r16, r17 ; shl2add r5, r6, r7 }
	{ prefetch_l1_fault r25 ; or r5, r6, r7 ; move r15, r16 }
	{ prefetch_l1_fault r25 ; pcnt r5, r6 ; info 19 }
	{ prefetch_l1_fault r25 ; revbits r5, r6 ; cmpleu r15, r16, r17 }
	{ prefetch_l1_fault r25 ; revbytes r5, r6 ; addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; revbytes r5, r6 ; shrui r15, r16, 5 }
	{ prefetch_l1_fault r25 ; rotl r15, r16, r17 ; nop }
	{ prefetch_l1_fault r25 ; rotl r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l1_fault r25 ; rotli r15, r16, 5 ; andi r5, r6, 5 }
	{ prefetch_l1_fault r25 ; rotli r15, r16, 5 ; shl1addx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; rotli r5, r6, 5 ; mnz r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shl r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shl r15, r16, r17 ; sub r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shl r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shl1add r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shl1add r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shl1add r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l1_fault r25 ; shl1addx r15, r16, r17 ; nop }
	{ prefetch_l1_fault r25 ; shl1addx r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shl2add r15, r16, r17 ; andi r5, r6, 5 }
	{ prefetch_l1_fault r25 ; shl2add r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shl2add r5, r6, r7 ; mnz r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shl2addx r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shl2addx r15, r16, r17 ; sub r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shl2addx r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shl3add r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shl3add r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shl3add r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l1_fault r25 ; shl3addx r15, r16, r17 ; nop }
	{ prefetch_l1_fault r25 ; shl3addx r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shli r15, r16, 5 ; andi r5, r6, 5 }
	{ prefetch_l1_fault r25 ; shli r15, r16, 5 ; shl1addx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shli r5, r6, 5 ; mnz r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shrs r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shrs r15, r16, r17 ; sub r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shrs r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shrsi r15, r16, 5 ; mul_hu_hu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shrsi r5, r6, 5 ; addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shrsi r5, r6, 5 ; shrui r15, r16, 5 }
	{ prefetch_l1_fault r25 ; shru r15, r16, r17 ; nop }
	{ prefetch_l1_fault r25 ; shru r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l1_fault r25 ; shrui r15, r16, 5 ; andi r5, r6, 5 }
	{ prefetch_l1_fault r25 ; shrui r15, r16, 5 ; shl1addx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; shrui r5, r6, 5 ; mnz r15, r16, r17 }
	{ prefetch_l1_fault r25 ; sub r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; sub r15, r16, r17 ; sub r5, r6, r7 }
	{ prefetch_l1_fault r25 ; sub r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l1_fault r25 ; subx r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ prefetch_l1_fault r25 ; subx r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; subx r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l1_fault r25 ; tblidxb0 r5, r6 ; shl2addx r15, r16, r17 }
	{ prefetch_l1_fault r25 ; tblidxb1 r5, r6 ; or r15, r16, r17 }
	{ prefetch_l1_fault r25 ; tblidxb2 r5, r6 ; lnk r15 }
	{ prefetch_l1_fault r25 ; tblidxb3 r5, r6 ; fnop }
	{ prefetch_l1_fault r25 ; xor r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ prefetch_l1_fault r25 ; xor r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ prefetch_l1_fault r25 ; xor r5, r6, r7 ; movei r15, 5 }
	{ prefetch_l2 r15 ; cmples r5, r6, r7 }
	{ prefetch_l2 r15 ; mnz r5, r6, r7 }
	{ prefetch_l2 r15 ; shl2add r5, r6, r7 }
	{ prefetch_l2 r15 ; v1dotpa r5, r6, r7 }
	{ prefetch_l2 r15 ; v2dotp r5, r6, r7 }
	{ prefetch_l2 r15 ; xor r5, r6, r7 }
	{ prefetch_l2 r25 ; add r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l2 r25 ; add r5, r6, r7 ; ill }
	{ prefetch_l2 r25 ; addi r15, r16, 5 ; cmovnez r5, r6, r7 }
	{ prefetch_l2 r25 ; addi r15, r16, 5 ; shl3add r5, r6, r7 }
	{ prefetch_l2 r25 ; addi r5, r6, 5 ; mz r15, r16, r17 }
	{ prefetch_l2 r25 ; addx r15, r16, r17 ; fnop }
	{ prefetch_l2 r25 ; addx r15, r16, r17 ; tblidxb1 r5, r6 }
	{ prefetch_l2 r25 ; addx r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l2 r25 ; addxi r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l2 r25 ; addxi r5, r6, 5 ; andi r15, r16, 5 }
	{ prefetch_l2 r25 ; addxi r5, r6, 5 ; xor r15, r16, r17 }
	{ prefetch_l2 r25 ; and r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l2 r25 ; and r5, r6, r7 ; ill }
	{ prefetch_l2 r25 ; andi r15, r16, 5 ; cmovnez r5, r6, r7 }
	{ prefetch_l2 r25 ; andi r15, r16, 5 ; shl3add r5, r6, r7 }
	{ prefetch_l2 r25 ; andi r5, r6, 5 ; mz r15, r16, r17 }
	{ prefetch_l2 r25 ; clz r5, r6 ; jalrp r15 }
	{ prefetch_l2 r25 ; cmoveqz r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ prefetch_l2 r25 ; cmovnez r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l2 r25 ; cmovnez r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l2 r25 ; cmpeq r15, r16, r17 ; or r5, r6, r7 }
	{ prefetch_l2 r25 ; cmpeq r5, r6, r7 ; fnop }
	{ prefetch_l2 r25 ; cmpeqi r15, r16, 5 ; cmoveqz r5, r6, r7 }
	{ prefetch_l2 r25 ; cmpeqi r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ prefetch_l2 r25 ; cmpeqi r5, r6, 5 ; movei r15, 5 }
	{ prefetch_l2 r25 ; cmples r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l2 r25 ; cmples r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l2 r25 ; cmples r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l2 r25 ; cmpleu r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l2 r25 ; cmpleu r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l2 r25 ; cmpleu r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l2 r25 ; cmplts r15, r16, r17 ; or r5, r6, r7 }
	{ prefetch_l2 r25 ; cmplts r5, r6, r7 ; fnop }
	{ prefetch_l2 r25 ; cmpltsi r15, r16, 5 ; cmoveqz r5, r6, r7 }
	{ prefetch_l2 r25 ; cmpltsi r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ prefetch_l2 r25 ; cmpltsi r5, r6, 5 ; movei r15, 5 }
	{ prefetch_l2 r25 ; cmpltu r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l2 r25 ; cmpltu r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l2 r25 ; cmpltu r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l2 r25 ; cmpne r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l2 r25 ; cmpne r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l2 r25 ; cmpne r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l2 r25 ; ctz r5, r6 ; shl3addx r15, r16, r17 }
	{ prefetch_l2 r25 ; fnop ; cmpne r5, r6, r7 }
	{ prefetch_l2 r25 ; fnop ; rotli r5, r6, 5 }
	{ prefetch_l2 r25 ; fsingle_pack1 r5, r6 ; and r15, r16, r17 }
	{ prefetch_l2 r25 ; fsingle_pack1 r5, r6 ; subx r15, r16, r17 }
	{ prefetch_l2 r25 ; ill ; or r5, r6, r7 }
	{ prefetch_l2 r25 ; info 19 ; cmovnez r5, r6, r7 }
	{ prefetch_l2 r25 ; info 19 ; mula_lu_lu r5, r6, r7 }
	{ prefetch_l2 r25 ; info 19 ; shrui r5, r6, 5 }
	{ prefetch_l2 r25 ; jalr r15 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l2 r25 ; jalrp r15 ; andi r5, r6, 5 }
	{ prefetch_l2 r25 ; jalrp r15 ; shl1addx r5, r6, r7 }
	{ prefetch_l2 r25 ; jr r15 ; move r5, r6 }
	{ prefetch_l2 r25 ; jr r15 }
	{ prefetch_l2 r25 ; jrp r15 ; revbits r5, r6 }
	{ prefetch_l2 r25 ; lnk r15 ; cmpne r5, r6, r7 }
	{ prefetch_l2 r25 ; lnk r15 ; subx r5, r6, r7 }
	{ prefetch_l2 r25 ; mnz r15, r16, r17 ; mulx r5, r6, r7 }
	{ prefetch_l2 r25 ; mnz r5, r6, r7 ; cmplts r15, r16, r17 }
	{ prefetch_l2 r25 ; move r15, r16 ; addxi r5, r6, 5 }
	{ prefetch_l2 r25 ; move r15, r16 ; shl r5, r6, r7 }
	{ prefetch_l2 r25 ; move r5, r6 ; jrp r15 }
	{ prefetch_l2 r25 ; movei r15, 5 ; cmplts r5, r6, r7 }
	{ prefetch_l2 r25 ; movei r15, 5 ; shru r5, r6, r7 }
	{ prefetch_l2 r25 ; movei r5, 5 ; rotli r15, r16, 5 }
	{ prefetch_l2 r25 ; mul_hs_hs r5, r6, r7 ; move r15, r16 }
	{ prefetch_l2 r25 ; mul_hu_hu r5, r6, r7 ; info 19 }
	{ prefetch_l2 r25 ; mul_ls_ls r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ prefetch_l2 r25 ; mul_lu_lu r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l2 r25 ; mul_lu_lu r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l2 r25 ; mula_hs_hs r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l2 r25 ; mula_hu_hu r5, r6, r7 ; or r15, r16, r17 }
	{ prefetch_l2 r25 ; mula_ls_ls r5, r6, r7 ; lnk r15 }
	{ prefetch_l2 r25 ; mula_lu_lu r5, r6, r7 ; fnop }
	{ prefetch_l2 r25 ; mulax r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ prefetch_l2 r25 ; mulx r5, r6, r7 ; add r15, r16, r17 }
	{ prefetch_l2 r25 ; mulx r5, r6, r7 ; shrsi r15, r16, 5 }
	{ prefetch_l2 r25 ; mz r15, r16, r17 ; mulx r5, r6, r7 }
	{ prefetch_l2 r25 ; mz r5, r6, r7 ; cmplts r15, r16, r17 }
	{ prefetch_l2 r25 ; nop ; addi r5, r6, 5 }
	{ prefetch_l2 r25 ; nop ; move r15, r16 }
	{ prefetch_l2 r25 ; nop ; shl3addx r15, r16, r17 }
	{ prefetch_l2 r25 ; nor r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l2 r25 ; nor r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l2 r25 ; nor r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l2 r25 ; or r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l2 r25 ; or r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l2 r25 ; or r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l2 r25 ; pcnt r5, r6 ; shl3addx r15, r16, r17 }
	{ prefetch_l2 r25 ; revbits r5, r6 ; rotli r15, r16, 5 }
	{ prefetch_l2 r25 ; revbytes r5, r6 ; move r15, r16 }
	{ prefetch_l2 r25 ; rotl r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l2 r25 ; rotl r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l2 r25 ; rotl r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l2 r25 ; rotli r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l2 r25 ; rotli r5, r6, 5 ; addxi r15, r16, 5 }
	{ prefetch_l2 r25 ; rotli r5, r6, 5 ; sub r15, r16, r17 }
	{ prefetch_l2 r25 ; shl r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l2 r25 ; shl r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l2 r25 ; shl1add r15, r16, r17 ; clz r5, r6 }
	{ prefetch_l2 r25 ; shl1add r15, r16, r17 ; shl2add r5, r6, r7 }
	{ prefetch_l2 r25 ; shl1add r5, r6, r7 ; move r15, r16 }
	{ prefetch_l2 r25 ; shl1addx r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l2 r25 ; shl1addx r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l2 r25 ; shl1addx r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l2 r25 ; shl2add r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l2 r25 ; shl2add r5, r6, r7 ; addxi r15, r16, 5 }
	{ prefetch_l2 r25 ; shl2add r5, r6, r7 ; sub r15, r16, r17 }
	{ prefetch_l2 r25 ; shl2addx r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l2 r25 ; shl2addx r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l2 r25 ; shl3add r15, r16, r17 ; clz r5, r6 }
	{ prefetch_l2 r25 ; shl3add r15, r16, r17 ; shl2add r5, r6, r7 }
	{ prefetch_l2 r25 ; shl3add r5, r6, r7 ; move r15, r16 }
	{ prefetch_l2 r25 ; shl3addx r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l2 r25 ; shl3addx r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l2 r25 ; shl3addx r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l2 r25 ; shli r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l2 r25 ; shli r5, r6, 5 ; addxi r15, r16, 5 }
	{ prefetch_l2 r25 ; shli r5, r6, 5 ; sub r15, r16, r17 }
	{ prefetch_l2 r25 ; shrs r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l2 r25 ; shrs r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l2 r25 ; shrsi r15, r16, 5 ; clz r5, r6 }
	{ prefetch_l2 r25 ; shrsi r15, r16, 5 ; shl2add r5, r6, r7 }
	{ prefetch_l2 r25 ; shrsi r5, r6, 5 ; move r15, r16 }
	{ prefetch_l2 r25 ; shru r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l2 r25 ; shru r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l2 r25 ; shru r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l2 r25 ; shrui r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l2 r25 ; shrui r5, r6, 5 ; addxi r15, r16, 5 }
	{ prefetch_l2 r25 ; shrui r5, r6, 5 ; sub r15, r16, r17 }
	{ prefetch_l2 r25 ; sub r15, r16, r17 ; nor r5, r6, r7 }
	{ prefetch_l2 r25 ; sub r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l2 r25 ; subx r15, r16, r17 ; clz r5, r6 }
	{ prefetch_l2 r25 ; subx r15, r16, r17 ; shl2add r5, r6, r7 }
	{ prefetch_l2 r25 ; subx r5, r6, r7 ; move r15, r16 }
	{ prefetch_l2 r25 ; tblidxb0 r5, r6 ; info 19 }
	{ prefetch_l2 r25 ; tblidxb1 r5, r6 ; cmpleu r15, r16, r17 }
	{ prefetch_l2 r25 ; tblidxb2 r5, r6 ; addx r15, r16, r17 }
	{ prefetch_l2 r25 ; tblidxb2 r5, r6 ; shrui r15, r16, 5 }
	{ prefetch_l2 r25 ; tblidxb3 r5, r6 ; shl2addx r15, r16, r17 }
	{ prefetch_l2 r25 ; xor r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l2 r25 ; xor r5, r6, r7 ; andi r15, r16, 5 }
	{ prefetch_l2 r25 ; xor r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch_l2_fault r15 ; fdouble_add_flags r5, r6, r7 }
	{ prefetch_l2_fault r15 ; mula_ls_ls r5, r6, r7 }
	{ prefetch_l2_fault r15 ; v1add r5, r6, r7 }
	{ prefetch_l2_fault r15 ; v1shrsi r5, r6, 5 }
	{ prefetch_l2_fault r15 ; v2shli r5, r6, 5 }
	{ prefetch_l2_fault r25 ; add r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l2_fault r25 ; add r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l2_fault r25 ; add r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l2_fault r25 ; addi r15, r16, 5 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l2_fault r25 ; addi r5, r6, 5 ; cmpeq r15, r16, r17 }
	{ prefetch_l2_fault r25 ; addi r5, r6, 5 }
	{ prefetch_l2_fault r25 ; addx r15, r16, r17 ; revbits r5, r6 }
	{ prefetch_l2_fault r25 ; addx r5, r6, r7 ; info 19 }
	{ prefetch_l2_fault r25 ; addxi r15, r16, 5 ; cmpeq r5, r6, r7 }
	{ prefetch_l2_fault r25 ; addxi r15, r16, 5 ; shl3addx r5, r6, r7 }
	{ prefetch_l2_fault r25 ; addxi r5, r6, 5 ; nop }
	{ prefetch_l2_fault r25 ; and r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l2_fault r25 ; and r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l2_fault r25 ; and r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l2_fault r25 ; andi r15, r16, 5 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l2_fault r25 ; andi r5, r6, 5 ; cmpeq r15, r16, r17 }
	{ prefetch_l2_fault r25 ; andi r5, r6, 5 }
	{ prefetch_l2_fault r25 ; clz r5, r6 ; shrs r15, r16, r17 }
	{ prefetch_l2_fault r25 ; cmoveqz r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l2_fault r25 ; cmovnez r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l2_fault r25 ; cmpeq r15, r16, r17 ; fnop }
	{ prefetch_l2_fault r25 ; cmpeq r15, r16, r17 ; tblidxb1 r5, r6 }
	{ prefetch_l2_fault r25 ; cmpeq r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; cmpeqi r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l2_fault r25 ; cmpeqi r5, r6, 5 ; andi r15, r16, 5 }
	{ prefetch_l2_fault r25 ; cmpeqi r5, r6, 5 ; xor r15, r16, r17 }
	{ prefetch_l2_fault r25 ; cmples r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l2_fault r25 ; cmples r5, r6, r7 ; ill }
	{ prefetch_l2_fault r25 ; cmpleu r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ prefetch_l2_fault r25 ; cmpleu r15, r16, r17 ; shl3add r5, r6, r7 }
	{ prefetch_l2_fault r25 ; cmpleu r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l2_fault r25 ; cmplts r15, r16, r17 ; fnop }
	{ prefetch_l2_fault r25 ; cmplts r15, r16, r17 ; tblidxb1 r5, r6 }
	{ prefetch_l2_fault r25 ; cmplts r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; cmpltsi r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l2_fault r25 ; cmpltsi r5, r6, 5 ; andi r15, r16, 5 }
	{ prefetch_l2_fault r25 ; cmpltsi r5, r6, 5 ; xor r15, r16, r17 }
	{ prefetch_l2_fault r25 ; cmpltu r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l2_fault r25 ; cmpltu r5, r6, r7 ; ill }
	{ prefetch_l2_fault r25 ; cmpne r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ prefetch_l2_fault r25 ; cmpne r15, r16, r17 ; shl3add r5, r6, r7 }
	{ prefetch_l2_fault r25 ; cmpne r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l2_fault r25 ; ctz r5, r6 ; jalrp r15 }
	{ prefetch_l2_fault r25 ; fnop ; andi r5, r6, 5 }
	{ prefetch_l2_fault r25 ; fnop ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l2_fault r25 ; fnop ; shru r15, r16, r17 }
	{ prefetch_l2_fault r25 ; fsingle_pack1 r5, r6 ; mz r15, r16, r17 }
	{ prefetch_l2_fault r25 ; ill ; fnop }
	{ prefetch_l2_fault r25 ; ill ; tblidxb1 r5, r6 }
	{ prefetch_l2_fault r25 ; info 19 ; info 19 }
	{ prefetch_l2_fault r25 ; info 19 ; shl1addx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; jalr r15 ; cmpeq r5, r6, r7 }
	{ prefetch_l2_fault r25 ; jalr r15 ; shl3addx r5, r6, r7 }
	{ prefetch_l2_fault r25 ; jalrp r15 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l2_fault r25 ; jr r15 ; addxi r5, r6, 5 }
	{ prefetch_l2_fault r25 ; jr r15 ; shl r5, r6, r7 }
	{ prefetch_l2_fault r25 ; jrp r15 ; info 19 }
	{ prefetch_l2_fault r25 ; jrp r15 ; tblidxb3 r5, r6 }
	{ prefetch_l2_fault r25 ; lnk r15 ; or r5, r6, r7 }
	{ prefetch_l2_fault r25 ; mnz r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ prefetch_l2_fault r25 ; mnz r15, r16, r17 ; shrui r5, r6, 5 }
	{ prefetch_l2_fault r25 ; mnz r5, r6, r7 ; shl r15, r16, r17 }
	{ prefetch_l2_fault r25 ; move r15, r16 ; mul_hs_hs r5, r6, r7 }
	{ prefetch_l2_fault r25 ; move r5, r6 ; addi r15, r16, 5 }
	{ prefetch_l2_fault r25 ; move r5, r6 ; shru r15, r16, r17 }
	{ prefetch_l2_fault r25 ; movei r15, 5 ; mz r5, r6, r7 }
	{ prefetch_l2_fault r25 ; movei r5, 5 ; cmpltsi r15, r16, 5 }
	{ prefetch_l2_fault r25 ; mul_hs_hs r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l2_fault r25 ; mul_hs_hs r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; mul_hu_hu r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; mul_ls_ls r5, r6, r7 ; rotli r15, r16, 5 }
	{ prefetch_l2_fault r25 ; mul_lu_lu r5, r6, r7 ; move r15, r16 }
	{ prefetch_l2_fault r25 ; mula_hs_hs r5, r6, r7 ; info 19 }
	{ prefetch_l2_fault r25 ; mula_hu_hu r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ prefetch_l2_fault r25 ; mula_ls_ls r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; mula_ls_ls r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l2_fault r25 ; mula_lu_lu r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; mulax r5, r6, r7 ; or r15, r16, r17 }
	{ prefetch_l2_fault r25 ; mulx r5, r6, r7 ; lnk r15 }
	{ prefetch_l2_fault r25 ; mz r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ prefetch_l2_fault r25 ; mz r15, r16, r17 ; shrui r5, r6, 5 }
	{ prefetch_l2_fault r25 ; mz r5, r6, r7 ; shl r15, r16, r17 }
	{ prefetch_l2_fault r25 ; nop ; cmplts r5, r6, r7 }
	{ prefetch_l2_fault r25 ; nop ; pcnt r5, r6 }
	{ prefetch_l2_fault r25 ; nop ; xor r5, r6, r7 }
	{ prefetch_l2_fault r25 ; nor r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l2_fault r25 ; nor r5, r6, r7 ; ill }
	{ prefetch_l2_fault r25 ; or r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ prefetch_l2_fault r25 ; or r15, r16, r17 ; shl3add r5, r6, r7 }
	{ prefetch_l2_fault r25 ; or r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l2_fault r25 ; pcnt r5, r6 ; jalrp r15 }
	{ prefetch_l2_fault r25 ; revbits r5, r6 ; cmpltsi r15, r16, 5 }
	{ prefetch_l2_fault r25 ; revbytes r5, r6 ; and r15, r16, r17 }
	{ prefetch_l2_fault r25 ; revbytes r5, r6 ; subx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; rotl r15, r16, r17 ; or r5, r6, r7 }
	{ prefetch_l2_fault r25 ; rotl r5, r6, r7 ; fnop }
	{ prefetch_l2_fault r25 ; rotli r15, r16, 5 ; cmoveqz r5, r6, r7 }
	{ prefetch_l2_fault r25 ; rotli r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ prefetch_l2_fault r25 ; rotli r5, r6, 5 ; movei r15, 5 }
	{ prefetch_l2_fault r25 ; shl r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l2_fault r25 ; shl r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l2_fault r25 ; shl r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shl1add r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shl1add r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shl1add r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shl1addx r15, r16, r17 ; or r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shl1addx r5, r6, r7 ; fnop }
	{ prefetch_l2_fault r25 ; shl2add r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shl2add r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shl2add r5, r6, r7 ; movei r15, 5 }
	{ prefetch_l2_fault r25 ; shl2addx r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l2_fault r25 ; shl2addx r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l2_fault r25 ; shl2addx r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shl3add r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shl3add r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shl3add r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shl3addx r15, r16, r17 ; or r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shl3addx r5, r6, r7 ; fnop }
	{ prefetch_l2_fault r25 ; shli r15, r16, 5 ; cmoveqz r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shli r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shli r5, r6, 5 ; movei r15, 5 }
	{ prefetch_l2_fault r25 ; shrs r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l2_fault r25 ; shrs r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l2_fault r25 ; shrs r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shrsi r15, r16, 5 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shrsi r5, r6, 5 ; and r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shrsi r5, r6, 5 ; subx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; shru r15, r16, r17 ; or r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shru r5, r6, r7 ; fnop }
	{ prefetch_l2_fault r25 ; shrui r15, r16, 5 ; cmoveqz r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shrui r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ prefetch_l2_fault r25 ; shrui r5, r6, 5 ; movei r15, 5 }
	{ prefetch_l2_fault r25 ; sub r15, r16, r17 ; ctz r5, r6 }
	{ prefetch_l2_fault r25 ; sub r15, r16, r17 ; tblidxb0 r5, r6 }
	{ prefetch_l2_fault r25 ; sub r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l2_fault r25 ; subx r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ prefetch_l2_fault r25 ; subx r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l2_fault r25 ; subx r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; tblidxb0 r5, r6 ; shl3addx r15, r16, r17 }
	{ prefetch_l2_fault r25 ; tblidxb1 r5, r6 ; rotli r15, r16, 5 }
	{ prefetch_l2_fault r25 ; tblidxb2 r5, r6 ; move r15, r16 }
	{ prefetch_l2_fault r25 ; tblidxb3 r5, r6 ; info 19 }
	{ prefetch_l2_fault r25 ; xor r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ prefetch_l2_fault r25 ; xor r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ prefetch_l2_fault r25 ; xor r5, r6, r7 ; nop }
	{ prefetch_l3 r15 ; cmplts r5, r6, r7 }
	{ prefetch_l3 r15 ; movei r5, 5 }
	{ prefetch_l3 r15 ; shl3add r5, r6, r7 }
	{ prefetch_l3 r15 ; v1dotpua r5, r6, r7 }
	{ prefetch_l3 r15 ; v2int_h r5, r6, r7 }
	{ prefetch_l3 r25 ; add r15, r16, r17 ; add r5, r6, r7 }
	{ prefetch_l3 r25 ; add r15, r16, r17 ; revbytes r5, r6 }
	{ prefetch_l3 r25 ; add r5, r6, r7 ; jalr r15 }
	{ prefetch_l3 r25 ; addi r15, r16, 5 ; cmpeqi r5, r6, 5 }
	{ prefetch_l3 r25 ; addi r15, r16, 5 ; shli r5, r6, 5 }
	{ prefetch_l3 r25 ; addi r5, r6, 5 ; nor r15, r16, r17 }
	{ prefetch_l3 r25 ; addx r15, r16, r17 ; info 19 }
	{ prefetch_l3 r25 ; addx r15, r16, r17 ; tblidxb3 r5, r6 }
	{ prefetch_l3 r25 ; addx r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ prefetch_l3 r25 ; addxi r15, r16, 5 ; mula_ls_ls r5, r6, r7 }
	{ prefetch_l3 r25 ; addxi r5, r6, 5 ; cmpeqi r15, r16, 5 }
	{ prefetch_l3 r25 ; and r15, r16, r17 ; add r5, r6, r7 }
	{ prefetch_l3 r25 ; and r15, r16, r17 ; revbytes r5, r6 }
	{ prefetch_l3 r25 ; and r5, r6, r7 ; jalr r15 }
	{ prefetch_l3 r25 ; andi r15, r16, 5 ; cmpeqi r5, r6, 5 }
	{ prefetch_l3 r25 ; andi r15, r16, 5 ; shli r5, r6, 5 }
	{ prefetch_l3 r25 ; andi r5, r6, 5 ; nor r15, r16, r17 }
	{ prefetch_l3 r25 ; clz r5, r6 ; jrp r15 }
	{ prefetch_l3 r25 ; cmoveqz r5, r6, r7 ; cmpne r15, r16, r17 }
	{ prefetch_l3 r25 ; cmovnez r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch_l3 r25 ; cmovnez r5, r6, r7 }
	{ prefetch_l3 r25 ; cmpeq r15, r16, r17 ; revbits r5, r6 }
	{ prefetch_l3 r25 ; cmpeq r5, r6, r7 ; info 19 }
	{ prefetch_l3 r25 ; cmpeqi r15, r16, 5 ; cmpeq r5, r6, r7 }
	{ prefetch_l3 r25 ; cmpeqi r15, r16, 5 ; shl3addx r5, r6, r7 }
	{ prefetch_l3 r25 ; cmpeqi r5, r6, 5 ; nop }
	{ prefetch_l3 r25 ; cmples r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l3 r25 ; cmples r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l3 r25 ; cmples r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l3 r25 ; cmpleu r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l3 r25 ; cmpleu r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch_l3 r25 ; cmpleu r5, r6, r7 }
	{ prefetch_l3 r25 ; cmplts r15, r16, r17 ; revbits r5, r6 }
	{ prefetch_l3 r25 ; cmplts r5, r6, r7 ; info 19 }
	{ prefetch_l3 r25 ; cmpltsi r15, r16, 5 ; cmpeq r5, r6, r7 }
	{ prefetch_l3 r25 ; cmpltsi r15, r16, 5 ; shl3addx r5, r6, r7 }
	{ prefetch_l3 r25 ; cmpltsi r5, r6, 5 ; nop }
	{ prefetch_l3 r25 ; cmpltu r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l3 r25 ; cmpltu r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l3 r25 ; cmpltu r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l3 r25 ; cmpne r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l3 r25 ; cmpne r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch_l3 r25 ; cmpne r5, r6, r7 }
	{ prefetch_l3 r25 ; ctz r5, r6 ; shrs r15, r16, r17 }
	{ prefetch_l3 r25 ; fnop ; fnop }
	{ prefetch_l3 r25 ; fnop ; shl r5, r6, r7 }
	{ prefetch_l3 r25 ; fsingle_pack1 r5, r6 ; cmpeq r15, r16, r17 }
	{ prefetch_l3 r25 ; fsingle_pack1 r5, r6 }
	{ prefetch_l3 r25 ; ill ; revbits r5, r6 }
	{ prefetch_l3 r25 ; info 19 ; cmpeq r5, r6, r7 }
	{ prefetch_l3 r25 ; info 19 ; mulx r5, r6, r7 }
	{ prefetch_l3 r25 ; info 19 ; sub r5, r6, r7 }
	{ prefetch_l3 r25 ; jalr r15 ; mula_ls_ls r5, r6, r7 }
	{ prefetch_l3 r25 ; jalrp r15 ; cmoveqz r5, r6, r7 }
	{ prefetch_l3 r25 ; jalrp r15 ; shl2addx r5, r6, r7 }
	{ prefetch_l3 r25 ; jr r15 ; mul_hs_hs r5, r6, r7 }
	{ prefetch_l3 r25 ; jrp r15 ; addi r5, r6, 5 }
	{ prefetch_l3 r25 ; jrp r15 ; rotl r5, r6, r7 }
	{ prefetch_l3 r25 ; lnk r15 ; fnop }
	{ prefetch_l3 r25 ; lnk r15 ; tblidxb1 r5, r6 }
	{ prefetch_l3 r25 ; mnz r15, r16, r17 ; nop }
	{ prefetch_l3 r25 ; mnz r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l3 r25 ; move r15, r16 ; andi r5, r6, 5 }
	{ prefetch_l3 r25 ; move r15, r16 ; shl1addx r5, r6, r7 }
	{ prefetch_l3 r25 ; move r5, r6 ; mnz r15, r16, r17 }
	{ prefetch_l3 r25 ; movei r15, 5 ; cmpltu r5, r6, r7 }
	{ prefetch_l3 r25 ; movei r15, 5 ; sub r5, r6, r7 }
	{ prefetch_l3 r25 ; movei r5, 5 ; shl1add r15, r16, r17 }
	{ prefetch_l3 r25 ; mul_hs_hs r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l3 r25 ; mul_hu_hu r5, r6, r7 ; jalrp r15 }
	{ prefetch_l3 r25 ; mul_ls_ls r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ prefetch_l3 r25 ; mul_lu_lu r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l3 r25 ; mul_lu_lu r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l3 r25 ; mula_hs_hs r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ prefetch_l3 r25 ; mula_hu_hu r5, r6, r7 ; rotli r15, r16, 5 }
	{ prefetch_l3 r25 ; mula_ls_ls r5, r6, r7 ; move r15, r16 }
	{ prefetch_l3 r25 ; mula_lu_lu r5, r6, r7 ; info 19 }
	{ prefetch_l3 r25 ; mulax r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ prefetch_l3 r25 ; mulx r5, r6, r7 ; addx r15, r16, r17 }
	{ prefetch_l3 r25 ; mulx r5, r6, r7 ; shrui r15, r16, 5 }
	{ prefetch_l3 r25 ; mz r15, r16, r17 ; nop }
	{ prefetch_l3 r25 ; mz r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ prefetch_l3 r25 ; nop ; addx r5, r6, r7 }
	{ prefetch_l3 r25 ; nop ; movei r15, 5 }
	{ prefetch_l3 r25 ; nop ; shli r15, r16, 5 }
	{ prefetch_l3 r25 ; nor r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l3 r25 ; nor r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l3 r25 ; nor r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l3 r25 ; or r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l3 r25 ; or r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch_l3 r25 ; or r5, r6, r7 }
	{ prefetch_l3 r25 ; pcnt r5, r6 ; shrs r15, r16, r17 }
	{ prefetch_l3 r25 ; revbits r5, r6 ; shl1add r15, r16, r17 }
	{ prefetch_l3 r25 ; revbytes r5, r6 ; mz r15, r16, r17 }
	{ prefetch_l3 r25 ; rotl r15, r16, r17 ; fnop }
	{ prefetch_l3 r25 ; rotl r15, r16, r17 ; tblidxb1 r5, r6 }
	{ prefetch_l3 r25 ; rotl r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l3 r25 ; rotli r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l3 r25 ; rotli r5, r6, 5 ; andi r15, r16, 5 }
	{ prefetch_l3 r25 ; rotli r5, r6, 5 ; xor r15, r16, r17 }
	{ prefetch_l3 r25 ; shl r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l3 r25 ; shl r5, r6, r7 ; ill }
	{ prefetch_l3 r25 ; shl1add r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ prefetch_l3 r25 ; shl1add r15, r16, r17 ; shl3add r5, r6, r7 }
	{ prefetch_l3 r25 ; shl1add r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l3 r25 ; shl1addx r15, r16, r17 ; fnop }
	{ prefetch_l3 r25 ; shl1addx r15, r16, r17 ; tblidxb1 r5, r6 }
	{ prefetch_l3 r25 ; shl1addx r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l3 r25 ; shl2add r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l3 r25 ; shl2add r5, r6, r7 ; andi r15, r16, 5 }
	{ prefetch_l3 r25 ; shl2add r5, r6, r7 ; xor r15, r16, r17 }
	{ prefetch_l3 r25 ; shl2addx r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l3 r25 ; shl2addx r5, r6, r7 ; ill }
	{ prefetch_l3 r25 ; shl3add r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ prefetch_l3 r25 ; shl3add r15, r16, r17 ; shl3add r5, r6, r7 }
	{ prefetch_l3 r25 ; shl3add r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l3 r25 ; shl3addx r15, r16, r17 ; fnop }
	{ prefetch_l3 r25 ; shl3addx r15, r16, r17 ; tblidxb1 r5, r6 }
	{ prefetch_l3 r25 ; shl3addx r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l3 r25 ; shli r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l3 r25 ; shli r5, r6, 5 ; andi r15, r16, 5 }
	{ prefetch_l3 r25 ; shli r5, r6, 5 ; xor r15, r16, r17 }
	{ prefetch_l3 r25 ; shrs r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l3 r25 ; shrs r5, r6, r7 ; ill }
	{ prefetch_l3 r25 ; shrsi r15, r16, 5 ; cmovnez r5, r6, r7 }
	{ prefetch_l3 r25 ; shrsi r15, r16, 5 ; shl3add r5, r6, r7 }
	{ prefetch_l3 r25 ; shrsi r5, r6, 5 ; mz r15, r16, r17 }
	{ prefetch_l3 r25 ; shru r15, r16, r17 ; fnop }
	{ prefetch_l3 r25 ; shru r15, r16, r17 ; tblidxb1 r5, r6 }
	{ prefetch_l3 r25 ; shru r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ prefetch_l3 r25 ; shrui r15, r16, 5 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l3 r25 ; shrui r5, r6, 5 ; andi r15, r16, 5 }
	{ prefetch_l3 r25 ; shrui r5, r6, 5 ; xor r15, r16, r17 }
	{ prefetch_l3 r25 ; sub r15, r16, r17 ; pcnt r5, r6 }
	{ prefetch_l3 r25 ; sub r5, r6, r7 ; ill }
	{ prefetch_l3 r25 ; subx r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ prefetch_l3 r25 ; subx r15, r16, r17 ; shl3add r5, r6, r7 }
	{ prefetch_l3 r25 ; subx r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l3 r25 ; tblidxb0 r5, r6 ; jalrp r15 }
	{ prefetch_l3 r25 ; tblidxb1 r5, r6 ; cmpltsi r15, r16, 5 }
	{ prefetch_l3 r25 ; tblidxb2 r5, r6 ; and r15, r16, r17 }
	{ prefetch_l3 r25 ; tblidxb2 r5, r6 ; subx r15, r16, r17 }
	{ prefetch_l3 r25 ; tblidxb3 r5, r6 ; shl3addx r15, r16, r17 }
	{ prefetch_l3 r25 ; xor r15, r16, r17 ; mula_ls_ls r5, r6, r7 }
	{ prefetch_l3 r25 ; xor r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ prefetch_l3_fault r15 ; add r5, r6, r7 }
	{ prefetch_l3_fault r15 ; fdouble_mul_flags r5, r6, r7 }
	{ prefetch_l3_fault r15 ; mula_lu_lu r5, r6, r7 }
	{ prefetch_l3_fault r15 ; v1adduc r5, r6, r7 }
	{ prefetch_l3_fault r15 ; v1shrui r5, r6, 5 }
	{ prefetch_l3_fault r15 ; v2shrs r5, r6, r7 }
	{ prefetch_l3_fault r25 ; add r15, r16, r17 ; mnz r5, r6, r7 }
	{ prefetch_l3_fault r25 ; add r15, r16, r17 ; xor r5, r6, r7 }
	{ prefetch_l3_fault r25 ; add r5, r6, r7 ; shli r15, r16, 5 }
	{ prefetch_l3_fault r25 ; addi r15, r16, 5 ; mula_lu_lu r5, r6, r7 }
	{ prefetch_l3_fault r25 ; addi r5, r6, 5 ; cmples r15, r16, r17 }
	{ prefetch_l3_fault r25 ; addx r15, r16, r17 ; addi r5, r6, 5 }
	{ prefetch_l3_fault r25 ; addx r15, r16, r17 ; rotl r5, r6, r7 }
	{ prefetch_l3_fault r25 ; addx r5, r6, r7 ; jalrp r15 }
	{ prefetch_l3_fault r25 ; addxi r15, r16, 5 ; cmples r5, r6, r7 }
	{ prefetch_l3_fault r25 ; addxi r15, r16, 5 ; shrs r5, r6, r7 }
	{ prefetch_l3_fault r25 ; addxi r5, r6, 5 ; or r15, r16, r17 }
	{ prefetch_l3_fault r25 ; and r15, r16, r17 ; mnz r5, r6, r7 }
	{ prefetch_l3_fault r25 ; and r15, r16, r17 ; xor r5, r6, r7 }
	{ prefetch_l3_fault r25 ; and r5, r6, r7 ; shli r15, r16, 5 }
	{ prefetch_l3_fault r25 ; andi r15, r16, 5 ; mula_lu_lu r5, r6, r7 }
	{ prefetch_l3_fault r25 ; andi r5, r6, 5 ; cmples r15, r16, r17 }
	{ prefetch_l3_fault r25 ; clz r5, r6 ; addi r15, r16, 5 }
	{ prefetch_l3_fault r25 ; clz r5, r6 ; shru r15, r16, r17 }
	{ prefetch_l3_fault r25 ; cmoveqz r5, r6, r7 ; shl2add r15, r16, r17 }
	{ prefetch_l3_fault r25 ; cmovnez r5, r6, r7 ; nor r15, r16, r17 }
	{ prefetch_l3_fault r25 ; cmpeq r15, r16, r17 ; info 19 }
	{ prefetch_l3_fault r25 ; cmpeq r15, r16, r17 ; tblidxb3 r5, r6 }
	{ prefetch_l3_fault r25 ; cmpeq r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ prefetch_l3_fault r25 ; cmpeqi r15, r16, 5 ; mula_ls_ls r5, r6, r7 }
	{ prefetch_l3_fault r25 ; cmpeqi r5, r6, 5 ; cmpeqi r15, r16, 5 }
	{ prefetch_l3_fault r25 ; cmples r15, r16, r17 ; add r5, r6, r7 }
	{ prefetch_l3_fault r25 ; cmples r15, r16, r17 ; revbytes r5, r6 }
	{ prefetch_l3_fault r25 ; cmples r5, r6, r7 ; jalr r15 }
	{ prefetch_l3_fault r25 ; cmpleu r15, r16, r17 ; cmpeqi r5, r6, 5 }
	{ prefetch_l3_fault r25 ; cmpleu r15, r16, r17 ; shli r5, r6, 5 }
	{ prefetch_l3_fault r25 ; cmpleu r5, r6, r7 ; nor r15, r16, r17 }
	{ prefetch_l3_fault r25 ; cmplts r15, r16, r17 ; info 19 }
	{ prefetch_l3_fault r25 ; cmplts r15, r16, r17 ; tblidxb3 r5, r6 }
	{ prefetch_l3_fault r25 ; cmplts r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ prefetch_l3_fault r25 ; cmpltsi r15, r16, 5 ; mula_ls_ls r5, r6, r7 }
	{ prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 ; cmpeqi r15, r16, 5 }
	{ prefetch_l3_fault r25 ; cmpltu r15, r16, r17 ; add r5, r6, r7 }
	{ prefetch_l3_fault r25 ; cmpltu r15, r16, r17 ; revbytes r5, r6 }
	{ prefetch_l3_fault r25 ; cmpltu r5, r6, r7 ; jalr r15 }
	{ prefetch_l3_fault r25 ; cmpne r15, r16, r17 ; cmpeqi r5, r6, 5 }
	{ prefetch_l3_fault r25 ; cmpne r15, r16, r17 ; shli r5, r6, 5 }
	{ prefetch_l3_fault r25 ; cmpne r5, r6, r7 ; nor r15, r16, r17 }
	{ prefetch_l3_fault r25 ; ctz r5, r6 ; jrp r15 }
	{ prefetch_l3_fault r25 ; fnop ; cmoveqz r5, r6, r7 }
	{ prefetch_l3_fault r25 ; fnop ; mula_ls_ls r5, r6, r7 }
	{ prefetch_l3_fault r25 ; fnop ; shrui r15, r16, 5 }
	{ prefetch_l3_fault r25 ; fsingle_pack1 r5, r6 ; nor r15, r16, r17 }
	{ prefetch_l3_fault r25 ; ill ; info 19 }
	{ prefetch_l3_fault r25 ; ill ; tblidxb3 r5, r6 }
	{ prefetch_l3_fault r25 ; info 19 ; jalrp r15 }
	{ prefetch_l3_fault r25 ; info 19 ; shl2add r15, r16, r17 }
	{ prefetch_l3_fault r25 ; jalr r15 ; cmples r5, r6, r7 }
	{ prefetch_l3_fault r25 ; jalr r15 ; shrs r5, r6, r7 }
	{ prefetch_l3_fault r25 ; jalrp r15 ; mula_hs_hs r5, r6, r7 }
	{ prefetch_l3_fault r25 ; jr r15 ; andi r5, r6, 5 }
	{ prefetch_l3_fault r25 ; jr r15 ; shl1addx r5, r6, r7 }
	{ prefetch_l3_fault r25 ; jrp r15 ; move r5, r6 }
	{ prefetch_l3_fault r25 ; jrp r15 }
	{ prefetch_l3_fault r25 ; lnk r15 ; revbits r5, r6 }
	{ prefetch_l3_fault r25 ; mnz r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l3_fault r25 ; mnz r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l3_fault r25 ; mnz r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l3_fault r25 ; move r15, r16 ; mul_ls_ls r5, r6, r7 }
	{ prefetch_l3_fault r25 ; move r5, r6 ; addxi r15, r16, 5 }
	{ prefetch_l3_fault r25 ; move r5, r6 ; sub r15, r16, r17 }
	{ prefetch_l3_fault r25 ; movei r15, 5 ; nor r5, r6, r7 }
	{ prefetch_l3_fault r25 ; movei r5, 5 ; cmpne r15, r16, r17 }
	{ prefetch_l3_fault r25 ; mul_hs_hs r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch_l3_fault r25 ; mul_hs_hs r5, r6, r7 }
	{ prefetch_l3_fault r25 ; mul_hu_hu r5, r6, r7 ; shrs r15, r16, r17 }
	{ prefetch_l3_fault r25 ; mul_ls_ls r5, r6, r7 ; shl1add r15, r16, r17 }
	{ prefetch_l3_fault r25 ; mul_lu_lu r5, r6, r7 ; mz r15, r16, r17 }
	{ prefetch_l3_fault r25 ; mula_hs_hs r5, r6, r7 ; jalrp r15 }
	{ prefetch_l3_fault r25 ; mula_hu_hu r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ prefetch_l3_fault r25 ; mula_ls_ls r5, r6, r7 ; and r15, r16, r17 }
	{ prefetch_l3_fault r25 ; mula_ls_ls r5, r6, r7 ; subx r15, r16, r17 }
	{ prefetch_l3_fault r25 ; mula_lu_lu r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ prefetch_l3_fault r25 ; mulax r5, r6, r7 ; rotli r15, r16, 5 }
	{ prefetch_l3_fault r25 ; mulx r5, r6, r7 ; move r15, r16 }
	{ prefetch_l3_fault r25 ; mz r15, r16, r17 ; cmpne r5, r6, r7 }
	{ prefetch_l3_fault r25 ; mz r15, r16, r17 ; subx r5, r6, r7 }
	{ prefetch_l3_fault r25 ; mz r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ prefetch_l3_fault r25 ; nop ; cmpltsi r5, r6, 5 }
	{ prefetch_l3_fault r25 ; nop ; revbytes r5, r6 }
	{ prefetch_l3_fault r25 ; nor r15, r16, r17 ; add r5, r6, r7 }
	{ prefetch_l3_fault r25 ; nor r15, r16, r17 ; revbytes r5, r6 }
	{ prefetch_l3_fault r25 ; nor r5, r6, r7 ; jalr r15 }
	{ prefetch_l3_fault r25 ; or r15, r16, r17 ; cmpeqi r5, r6, 5 }
	{ prefetch_l3_fault r25 ; or r15, r16, r17 ; shli r5, r6, 5 }
	{ prefetch_l3_fault r25 ; or r5, r6, r7 ; nor r15, r16, r17 }
	{ prefetch_l3_fault r25 ; pcnt r5, r6 ; jrp r15 }
	{ prefetch_l3_fault r25 ; revbits r5, r6 ; cmpne r15, r16, r17 }
	{ prefetch_l3_fault r25 ; revbytes r5, r6 ; cmpeq r15, r16, r17 }
	{ prefetch_l3_fault r25 ; revbytes r5, r6 }
	{ prefetch_l3_fault r25 ; rotl r15, r16, r17 ; revbits r5, r6 }
	{ prefetch_l3_fault r25 ; rotl r5, r6, r7 ; info 19 }
	{ prefetch_l3_fault r25 ; rotli r15, r16, 5 ; cmpeq r5, r6, r7 }
	{ prefetch_l3_fault r25 ; rotli r15, r16, 5 ; shl3addx r5, r6, r7 }
	{ prefetch_l3_fault r25 ; rotli r5, r6, 5 ; nop }
	{ prefetch_l3_fault r25 ; shl r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l3_fault r25 ; shl r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l3_fault r25 ; shl r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l3_fault r25 ; shl1add r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shl1add r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch_l3_fault r25 ; shl1add r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shl1addx r15, r16, r17 ; revbits r5, r6 }
	{ prefetch_l3_fault r25 ; shl1addx r5, r6, r7 ; info 19 }
	{ prefetch_l3_fault r25 ; shl2add r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shl2add r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shl2add r5, r6, r7 ; nop }
	{ prefetch_l3_fault r25 ; shl2addx r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l3_fault r25 ; shl2addx r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l3_fault r25 ; shl2addx r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l3_fault r25 ; shl3add r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shl3add r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch_l3_fault r25 ; shl3add r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shl3addx r15, r16, r17 ; revbits r5, r6 }
	{ prefetch_l3_fault r25 ; shl3addx r5, r6, r7 ; info 19 }
	{ prefetch_l3_fault r25 ; shli r15, r16, 5 ; cmpeq r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shli r15, r16, 5 ; shl3addx r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shli r5, r6, 5 ; nop }
	{ prefetch_l3_fault r25 ; shrs r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l3_fault r25 ; shrs r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l3_fault r25 ; shrs r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l3_fault r25 ; shrsi r15, r16, 5 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shrsi r5, r6, 5 ; cmpeq r15, r16, r17 }
	{ prefetch_l3_fault r25 ; shrsi r5, r6, 5 }
	{ prefetch_l3_fault r25 ; shru r15, r16, r17 ; revbits r5, r6 }
	{ prefetch_l3_fault r25 ; shru r5, r6, r7 ; info 19 }
	{ prefetch_l3_fault r25 ; shrui r15, r16, 5 ; cmpeq r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shrui r15, r16, 5 ; shl3addx r5, r6, r7 }
	{ prefetch_l3_fault r25 ; shrui r5, r6, 5 ; nop }
	{ prefetch_l3_fault r25 ; sub r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ prefetch_l3_fault r25 ; sub r15, r16, r17 ; tblidxb2 r5, r6 }
	{ prefetch_l3_fault r25 ; sub r5, r6, r7 ; shl3add r15, r16, r17 }
	{ prefetch_l3_fault r25 ; subx r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ prefetch_l3_fault r25 ; subx r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ prefetch_l3_fault r25 ; subx r5, r6, r7 }
	{ prefetch_l3_fault r25 ; tblidxb0 r5, r6 ; shrs r15, r16, r17 }
	{ prefetch_l3_fault r25 ; tblidxb1 r5, r6 ; shl1add r15, r16, r17 }
	{ prefetch_l3_fault r25 ; tblidxb2 r5, r6 ; mz r15, r16, r17 }
	{ prefetch_l3_fault r25 ; tblidxb3 r5, r6 ; jalrp r15 }
	{ prefetch_l3_fault r25 ; xor r15, r16, r17 ; cmples r5, r6, r7 }
	{ prefetch_l3_fault r25 ; xor r15, r16, r17 ; shrs r5, r6, r7 }
	{ prefetch_l3_fault r25 ; xor r5, r6, r7 ; or r15, r16, r17 }
	{ raise ; cmpltu r5, r6, r7 }
	{ raise ; mul_hs_hs r5, r6, r7 }
	{ raise ; shli r5, r6, 5 }
	{ raise ; v1dotpusa r5, r6, r7 }
	{ raise ; v2maxs r5, r6, r7 }
	{ revbits r5, r6 ; add r15, r16, r17 ; ld1u r25, r26 }
	{ revbits r5, r6 ; addx r15, r16, r17 ; ld2s r25, r26 }
	{ revbits r5, r6 ; and r15, r16, r17 ; ld2s r25, r26 }
	{ revbits r5, r6 ; cmpeq r15, r16, r17 ; ld4s r25, r26 }
	{ revbits r5, r6 ; cmples r15, r16, r17 ; ld4s r25, r26 }
	{ revbits r5, r6 ; cmplts r15, r16, r17 ; prefetch r25 }
	{ revbits r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l1_fault r25 }
	{ revbits r5, r6 ; fetchand r15, r16, r17 }
	{ revbits r5, r6 ; ill ; prefetch_l3_fault r25 }
	{ revbits r5, r6 ; jalr r15 ; prefetch_l3 r25 }
	{ revbits r5, r6 ; jr r15 ; st r25, r26 }
	{ revbits r5, r6 ; ld r25, r26 ; ill }
	{ revbits r5, r6 ; ld1s r25, r26 ; cmpeqi r15, r16, 5 }
	{ revbits r5, r6 ; ld1s_add r15, r16, 5 }
	{ revbits r5, r6 ; ld1u r25, r26 ; shli r15, r16, 5 }
	{ revbits r5, r6 ; ld2s r25, r26 ; rotl r15, r16, r17 }
	{ revbits r5, r6 ; ld2u r25, r26 ; jrp r15 }
	{ revbits r5, r6 ; ld4s r25, r26 ; cmpltsi r15, r16, 5 }
	{ revbits r5, r6 ; ld4u r25, r26 ; addx r15, r16, r17 }
	{ revbits r5, r6 ; ld4u r25, r26 ; shrui r15, r16, 5 }
	{ revbits r5, r6 ; lnk r15 ; st4 r25, r26 }
	{ revbits r5, r6 ; move r15, r16 ; st4 r25, r26 }
	{ revbits r5, r6 ; mz r15, r16, r17 ; st4 r25, r26 }
	{ revbits r5, r6 ; or r15, r16, r17 ; ld r25, r26 }
	{ revbits r5, r6 ; prefetch r25 ; jr r15 }
	{ revbits r5, r6 ; prefetch_l1 r25 ; andi r15, r16, 5 }
	{ revbits r5, r6 ; prefetch_l1 r25 ; xor r15, r16, r17 }
	{ revbits r5, r6 ; prefetch_l1_fault r25 ; shl3addx r15, r16, r17 }
	{ revbits r5, r6 ; prefetch_l2 r25 ; rotl r15, r16, r17 }
	{ revbits r5, r6 ; prefetch_l2_fault r25 ; lnk r15 }
	{ revbits r5, r6 ; prefetch_l3 r25 ; cmpne r15, r16, r17 }
	{ revbits r5, r6 ; prefetch_l3_fault r25 ; andi r15, r16, 5 }
	{ revbits r5, r6 ; prefetch_l3_fault r25 ; xor r15, r16, r17 }
	{ revbits r5, r6 ; rotli r15, r16, 5 }
	{ revbits r5, r6 ; shl1addx r15, r16, r17 ; ld r25, r26 }
	{ revbits r5, r6 ; shl2addx r15, r16, r17 ; ld1u r25, r26 }
	{ revbits r5, r6 ; shl3addx r15, r16, r17 ; ld2u r25, r26 }
	{ revbits r5, r6 ; shrs r15, r16, r17 ; ld2u r25, r26 }
	{ revbits r5, r6 ; shru r15, r16, r17 ; ld4u r25, r26 }
	{ revbits r5, r6 ; st r25, r26 ; andi r15, r16, 5 }
	{ revbits r5, r6 ; st r25, r26 ; xor r15, r16, r17 }
	{ revbits r5, r6 ; st1 r25, r26 ; shl3addx r15, r16, r17 }
	{ revbits r5, r6 ; st2 r25, r26 ; or r15, r16, r17 }
	{ revbits r5, r6 ; st4 r25, r26 ; jr r15 }
	{ revbits r5, r6 ; sub r15, r16, r17 ; ld1u r25, r26 }
	{ revbits r5, r6 ; v1cmpeq r15, r16, r17 }
	{ revbits r5, r6 ; v2maxsi r15, r16, 5 }
	{ revbits r5, r6 ; xor r15, r16, r17 ; prefetch_l2_fault r25 }
	{ revbytes r5, r6 ; addi r15, r16, 5 ; prefetch_l3 r25 }
	{ revbytes r5, r6 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ revbytes r5, r6 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ revbytes r5, r6 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
	{ revbytes r5, r6 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
	{ revbytes r5, r6 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
	{ revbytes r5, r6 ; cmpne r15, r16, r17 }
	{ revbytes r5, r6 ; ill ; ld1u r25, r26 }
	{ revbytes r5, r6 ; jalr r15 ; ld1s r25, r26 }
	{ revbytes r5, r6 ; jr r15 ; ld2s r25, r26 }
	{ revbytes r5, r6 ; ld r25, r26 ; and r15, r16, r17 }
	{ revbytes r5, r6 ; ld r25, r26 ; subx r15, r16, r17 }
	{ revbytes r5, r6 ; ld1s r25, r26 ; shl3add r15, r16, r17 }
	{ revbytes r5, r6 ; ld1u r25, r26 ; nor r15, r16, r17 }
	{ revbytes r5, r6 ; ld2s r25, r26 ; jalrp r15 }
	{ revbytes r5, r6 ; ld2u r25, r26 ; cmpleu r15, r16, r17 }
	{ revbytes r5, r6 ; ld4s r25, r26 ; add r15, r16, r17 }
	{ revbytes r5, r6 ; ld4s r25, r26 ; shrsi r15, r16, 5 }
	{ revbytes r5, r6 ; ld4u r25, r26 ; shl r15, r16, r17 }
	{ revbytes r5, r6 ; lnk r15 ; ld4u r25, r26 }
	{ revbytes r5, r6 ; move r15, r16 ; ld4u r25, r26 }
	{ revbytes r5, r6 ; mz r15, r16, r17 ; ld4u r25, r26 }
	{ revbytes r5, r6 ; nor r15, r16, r17 ; prefetch_l1 r25 }
	{ revbytes r5, r6 ; prefetch r25 ; cmples r15, r16, r17 }
	{ revbytes r5, r6 ; prefetch_add_l1_fault r15, 5 }
	{ revbytes r5, r6 ; prefetch_l1 r25 ; shl2add r15, r16, r17 }
	{ revbytes r5, r6 ; prefetch_l1_fault r25 ; nop }
	{ revbytes r5, r6 ; prefetch_l2 r25 ; jalrp r15 }
	{ revbytes r5, r6 ; prefetch_l2_fault r25 ; cmplts r15, r16, r17 }
	{ revbytes r5, r6 ; prefetch_l3 r25 ; addx r15, r16, r17 }
	{ revbytes r5, r6 ; prefetch_l3 r25 ; shrui r15, r16, 5 }
	{ revbytes r5, r6 ; prefetch_l3_fault r25 ; shl2add r15, r16, r17 }
	{ revbytes r5, r6 ; rotli r15, r16, 5 ; prefetch r25 }
	{ revbytes r5, r6 ; shl1add r15, r16, r17 ; prefetch_l1 r25 }
	{ revbytes r5, r6 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
	{ revbytes r5, r6 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
	{ revbytes r5, r6 ; shli r15, r16, 5 ; st r25, r26 }
	{ revbytes r5, r6 ; shrsi r15, r16, 5 ; st r25, r26 }
	{ revbytes r5, r6 ; shrui r15, r16, 5 ; st2 r25, r26 }
	{ revbytes r5, r6 ; st r25, r26 ; shl2add r15, r16, r17 }
	{ revbytes r5, r6 ; st1 r25, r26 ; nop }
	{ revbytes r5, r6 ; st2 r25, r26 ; jalr r15 }
	{ revbytes r5, r6 ; st4 r25, r26 ; cmples r15, r16, r17 }
	{ revbytes r5, r6 ; st_add r15, r16, 5 }
	{ revbytes r5, r6 ; subx r15, r16, r17 ; prefetch_l3 r25 }
	{ revbytes r5, r6 ; v2cmpeqi r15, r16, 5 }
	{ revbytes r5, r6 ; xor r15, r16, r17 ; ld r25, r26 }
	{ rotl r15, r16, r17 ; addi r5, r6, 5 ; ld1s r25, r26 }
	{ rotl r15, r16, r17 ; addxi r5, r6, 5 ; ld1u r25, r26 }
	{ rotl r15, r16, r17 ; andi r5, r6, 5 ; ld1u r25, r26 }
	{ rotl r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld1s r25, r26 }
	{ rotl r15, r16, r17 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
	{ rotl r15, r16, r17 ; cmples r5, r6, r7 ; ld4s r25, r26 }
	{ rotl r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch r25 }
	{ rotl r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
	{ rotl r15, r16, r17 ; ctz r5, r6 ; ld1s r25, r26 }
	{ rotl r15, r16, r17 ; fnop ; prefetch_l2 r25 }
	{ rotl r15, r16, r17 ; info 19 ; ld4u r25, r26 }
	{ rotl r15, r16, r17 ; ld r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ rotl r15, r16, r17 ; ld1s r25, r26 ; addxi r5, r6, 5 }
	{ rotl r15, r16, r17 ; ld1s r25, r26 ; shl r5, r6, r7 }
	{ rotl r15, r16, r17 ; ld1u r25, r26 ; info 19 }
	{ rotl r15, r16, r17 ; ld1u r25, r26 ; tblidxb3 r5, r6 }
	{ rotl r15, r16, r17 ; ld2s r25, r26 ; or r5, r6, r7 }
	{ rotl r15, r16, r17 ; ld2u r25, r26 ; cmpltsi r5, r6, 5 }
	{ rotl r15, r16, r17 ; ld2u r25, r26 ; shrui r5, r6, 5 }
	{ rotl r15, r16, r17 ; ld4s r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ rotl r15, r16, r17 ; ld4u r25, r26 ; cmovnez r5, r6, r7 }
	{ rotl r15, r16, r17 ; ld4u r25, r26 ; shl3add r5, r6, r7 }
	{ rotl r15, r16, r17 ; move r5, r6 ; ld4s r25, r26 }
	{ rotl r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; ld4u r25, r26 }
	{ rotl r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; ld2s r25, r26 }
	{ rotl r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; ld2u r25, r26 }
	{ rotl r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld1s r25, r26 }
	{ rotl r15, r16, r17 ; mulax r5, r6, r7 ; ld1u r25, r26 }
	{ rotl r15, r16, r17 ; mz r5, r6, r7 ; ld2u r25, r26 }
	{ rotl r15, r16, r17 ; nor r5, r6, r7 ; ld4u r25, r26 }
	{ rotl r15, r16, r17 ; pcnt r5, r6 ; prefetch r25 }
	{ rotl r15, r16, r17 ; prefetch r25 ; mula_hs_hs r5, r6, r7 }
	{ rotl r15, r16, r17 ; prefetch_l1 r25 ; andi r5, r6, 5 }
	{ rotl r15, r16, r17 ; prefetch_l1 r25 ; shl1addx r5, r6, r7 }
	{ rotl r15, r16, r17 ; prefetch_l1_fault r25 ; move r5, r6 }
	{ rotl r15, r16, r17 ; prefetch_l1_fault r25 }
	{ rotl r15, r16, r17 ; prefetch_l2 r25 ; revbits r5, r6 }
	{ rotl r15, r16, r17 ; prefetch_l2_fault r25 ; cmpne r5, r6, r7 }
	{ rotl r15, r16, r17 ; prefetch_l2_fault r25 ; subx r5, r6, r7 }
	{ rotl r15, r16, r17 ; prefetch_l3 r25 ; mulx r5, r6, r7 }
	{ rotl r15, r16, r17 ; prefetch_l3_fault r25 ; cmpeqi r5, r6, 5 }
	{ rotl r15, r16, r17 ; prefetch_l3_fault r25 ; shli r5, r6, 5 }
	{ rotl r15, r16, r17 ; revbytes r5, r6 ; prefetch_l1 r25 }
	{ rotl r15, r16, r17 ; rotli r5, r6, 5 ; prefetch_l2 r25 }
	{ rotl r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l2_fault r25 }
	{ rotl r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l3_fault r25 }
	{ rotl r15, r16, r17 ; shl3add r5, r6, r7 ; st1 r25, r26 }
	{ rotl r15, r16, r17 ; shli r5, r6, 5 ; st4 r25, r26 }
	{ rotl r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
	{ rotl r15, r16, r17 ; shrux r5, r6, r7 }
	{ rotl r15, r16, r17 ; st r25, r26 ; or r5, r6, r7 }
	{ rotl r15, r16, r17 ; st1 r25, r26 ; cmpltsi r5, r6, 5 }
	{ rotl r15, r16, r17 ; st1 r25, r26 ; shrui r5, r6, 5 }
	{ rotl r15, r16, r17 ; st2 r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ rotl r15, r16, r17 ; st4 r25, r26 ; cmovnez r5, r6, r7 }
	{ rotl r15, r16, r17 ; st4 r25, r26 ; shl3add r5, r6, r7 }
	{ rotl r15, r16, r17 ; subx r5, r6, r7 ; ld4u r25, r26 }
	{ rotl r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch r25 }
	{ rotl r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l1_fault r25 }
	{ rotl r15, r16, r17 ; v1mnz r5, r6, r7 }
	{ rotl r15, r16, r17 ; v2mults r5, r6, r7 }
	{ rotl r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l2_fault r25 }
	{ rotl r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l3 r25 }
	{ rotl r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ rotl r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ rotl r5, r6, r7 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
	{ rotl r5, r6, r7 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
	{ rotl r5, r6, r7 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
	{ rotl r5, r6, r7 ; cmpne r15, r16, r17 }
	{ rotl r5, r6, r7 ; ill ; ld1u r25, r26 }
	{ rotl r5, r6, r7 ; jalr r15 ; ld1s r25, r26 }
	{ rotl r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
	{ rotl r5, r6, r7 ; ld r25, r26 ; and r15, r16, r17 }
	{ rotl r5, r6, r7 ; ld r25, r26 ; subx r15, r16, r17 }
	{ rotl r5, r6, r7 ; ld1s r25, r26 ; shl3add r15, r16, r17 }
	{ rotl r5, r6, r7 ; ld1u r25, r26 ; nor r15, r16, r17 }
	{ rotl r5, r6, r7 ; ld2s r25, r26 ; jalrp r15 }
	{ rotl r5, r6, r7 ; ld2u r25, r26 ; cmpleu r15, r16, r17 }
	{ rotl r5, r6, r7 ; ld4s r25, r26 ; add r15, r16, r17 }
	{ rotl r5, r6, r7 ; ld4s r25, r26 ; shrsi r15, r16, 5 }
	{ rotl r5, r6, r7 ; ld4u r25, r26 ; shl r15, r16, r17 }
	{ rotl r5, r6, r7 ; lnk r15 ; ld4u r25, r26 }
	{ rotl r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
	{ rotl r5, r6, r7 ; mz r15, r16, r17 ; ld4u r25, r26 }
	{ rotl r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l1 r25 }
	{ rotl r5, r6, r7 ; prefetch r25 ; cmples r15, r16, r17 }
	{ rotl r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
	{ rotl r5, r6, r7 ; prefetch_l1 r25 ; shl2add r15, r16, r17 }
	{ rotl r5, r6, r7 ; prefetch_l1_fault r25 ; nop }
	{ rotl r5, r6, r7 ; prefetch_l2 r25 ; jalrp r15 }
	{ rotl r5, r6, r7 ; prefetch_l2_fault r25 ; cmplts r15, r16, r17 }
	{ rotl r5, r6, r7 ; prefetch_l3 r25 ; addx r15, r16, r17 }
	{ rotl r5, r6, r7 ; prefetch_l3 r25 ; shrui r15, r16, 5 }
	{ rotl r5, r6, r7 ; prefetch_l3_fault r25 ; shl2add r15, r16, r17 }
	{ rotl r5, r6, r7 ; rotli r15, r16, 5 ; prefetch r25 }
	{ rotl r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1 r25 }
	{ rotl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
	{ rotl r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
	{ rotl r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
	{ rotl r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
	{ rotl r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
	{ rotl r5, r6, r7 ; st r25, r26 ; shl2add r15, r16, r17 }
	{ rotl r5, r6, r7 ; st1 r25, r26 ; nop }
	{ rotl r5, r6, r7 ; st2 r25, r26 ; jalr r15 }
	{ rotl r5, r6, r7 ; st4 r25, r26 ; cmples r15, r16, r17 }
	{ rotl r5, r6, r7 ; st_add r15, r16, 5 }
	{ rotl r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3 r25 }
	{ rotl r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
	{ rotl r5, r6, r7 ; xor r15, r16, r17 ; ld r25, r26 }
	{ rotli r15, r16, 5 ; addi r5, r6, 5 ; ld1s r25, r26 }
	{ rotli r15, r16, 5 ; addxi r5, r6, 5 ; ld1u r25, r26 }
	{ rotli r15, r16, 5 ; andi r5, r6, 5 ; ld1u r25, r26 }
	{ rotli r15, r16, 5 ; cmoveqz r5, r6, r7 ; ld1s r25, r26 }
	{ rotli r15, r16, 5 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
	{ rotli r15, r16, 5 ; cmples r5, r6, r7 ; ld4s r25, r26 }
	{ rotli r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch r25 }
	{ rotli r15, r16, 5 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
	{ rotli r15, r16, 5 ; ctz r5, r6 ; ld1s r25, r26 }
	{ rotli r15, r16, 5 ; fnop ; prefetch_l2 r25 }
	{ rotli r15, r16, 5 ; info 19 ; ld4u r25, r26 }
	{ rotli r15, r16, 5 ; ld r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ rotli r15, r16, 5 ; ld1s r25, r26 ; addxi r5, r6, 5 }
	{ rotli r15, r16, 5 ; ld1s r25, r26 ; shl r5, r6, r7 }
	{ rotli r15, r16, 5 ; ld1u r25, r26 ; info 19 }
	{ rotli r15, r16, 5 ; ld1u r25, r26 ; tblidxb3 r5, r6 }
	{ rotli r15, r16, 5 ; ld2s r25, r26 ; or r5, r6, r7 }
	{ rotli r15, r16, 5 ; ld2u r25, r26 ; cmpltsi r5, r6, 5 }
	{ rotli r15, r16, 5 ; ld2u r25, r26 ; shrui r5, r6, 5 }
	{ rotli r15, r16, 5 ; ld4s r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ rotli r15, r16, 5 ; ld4u r25, r26 ; cmovnez r5, r6, r7 }
	{ rotli r15, r16, 5 ; ld4u r25, r26 ; shl3add r5, r6, r7 }
	{ rotli r15, r16, 5 ; move r5, r6 ; ld4s r25, r26 }
	{ rotli r15, r16, 5 ; mul_hs_hs r5, r6, r7 ; ld4u r25, r26 }
	{ rotli r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; ld2s r25, r26 }
	{ rotli r15, r16, 5 ; mula_hs_hs r5, r6, r7 ; ld2u r25, r26 }
	{ rotli r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; ld1s r25, r26 }
	{ rotli r15, r16, 5 ; mulax r5, r6, r7 ; ld1u r25, r26 }
	{ rotli r15, r16, 5 ; mz r5, r6, r7 ; ld2u r25, r26 }
	{ rotli r15, r16, 5 ; nor r5, r6, r7 ; ld4u r25, r26 }
	{ rotli r15, r16, 5 ; pcnt r5, r6 ; prefetch r25 }
	{ rotli r15, r16, 5 ; prefetch r25 ; mula_hs_hs r5, r6, r7 }
	{ rotli r15, r16, 5 ; prefetch_l1 r25 ; andi r5, r6, 5 }
	{ rotli r15, r16, 5 ; prefetch_l1 r25 ; shl1addx r5, r6, r7 }
	{ rotli r15, r16, 5 ; prefetch_l1_fault r25 ; move r5, r6 }
	{ rotli r15, r16, 5 ; prefetch_l1_fault r25 }
	{ rotli r15, r16, 5 ; prefetch_l2 r25 ; revbits r5, r6 }
	{ rotli r15, r16, 5 ; prefetch_l2_fault r25 ; cmpne r5, r6, r7 }
	{ rotli r15, r16, 5 ; prefetch_l2_fault r25 ; subx r5, r6, r7 }
	{ rotli r15, r16, 5 ; prefetch_l3 r25 ; mulx r5, r6, r7 }
	{ rotli r15, r16, 5 ; prefetch_l3_fault r25 ; cmpeqi r5, r6, 5 }
	{ rotli r15, r16, 5 ; prefetch_l3_fault r25 ; shli r5, r6, 5 }
	{ rotli r15, r16, 5 ; revbytes r5, r6 ; prefetch_l1 r25 }
	{ rotli r15, r16, 5 ; rotli r5, r6, 5 ; prefetch_l2 r25 }
	{ rotli r15, r16, 5 ; shl1add r5, r6, r7 ; prefetch_l2_fault r25 }
	{ rotli r15, r16, 5 ; shl2add r5, r6, r7 ; prefetch_l3_fault r25 }
	{ rotli r15, r16, 5 ; shl3add r5, r6, r7 ; st1 r25, r26 }
	{ rotli r15, r16, 5 ; shli r5, r6, 5 ; st4 r25, r26 }
	{ rotli r15, r16, 5 ; shrsi r5, r6, 5 ; st4 r25, r26 }
	{ rotli r15, r16, 5 ; shrux r5, r6, r7 }
	{ rotli r15, r16, 5 ; st r25, r26 ; or r5, r6, r7 }
	{ rotli r15, r16, 5 ; st1 r25, r26 ; cmpltsi r5, r6, 5 }
	{ rotli r15, r16, 5 ; st1 r25, r26 ; shrui r5, r6, 5 }
	{ rotli r15, r16, 5 ; st2 r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ rotli r15, r16, 5 ; st4 r25, r26 ; cmovnez r5, r6, r7 }
	{ rotli r15, r16, 5 ; st4 r25, r26 ; shl3add r5, r6, r7 }
	{ rotli r15, r16, 5 ; subx r5, r6, r7 ; ld4u r25, r26 }
	{ rotli r15, r16, 5 ; tblidxb1 r5, r6 ; prefetch r25 }
	{ rotli r15, r16, 5 ; tblidxb3 r5, r6 ; prefetch_l1_fault r25 }
	{ rotli r15, r16, 5 ; v1mnz r5, r6, r7 }
	{ rotli r15, r16, 5 ; v2mults r5, r6, r7 }
	{ rotli r15, r16, 5 ; xor r5, r6, r7 ; prefetch_l2_fault r25 }
	{ rotli r5, r6, 5 ; addi r15, r16, 5 ; prefetch_l3 r25 }
	{ rotli r5, r6, 5 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ rotli r5, r6, 5 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ rotli r5, r6, 5 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
	{ rotli r5, r6, 5 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
	{ rotli r5, r6, 5 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
	{ rotli r5, r6, 5 ; cmpne r15, r16, r17 }
	{ rotli r5, r6, 5 ; ill ; ld1u r25, r26 }
	{ rotli r5, r6, 5 ; jalr r15 ; ld1s r25, r26 }
	{ rotli r5, r6, 5 ; jr r15 ; ld2s r25, r26 }
	{ rotli r5, r6, 5 ; ld r25, r26 ; and r15, r16, r17 }
	{ rotli r5, r6, 5 ; ld r25, r26 ; subx r15, r16, r17 }
	{ rotli r5, r6, 5 ; ld1s r25, r26 ; shl3add r15, r16, r17 }
	{ rotli r5, r6, 5 ; ld1u r25, r26 ; nor r15, r16, r17 }
	{ rotli r5, r6, 5 ; ld2s r25, r26 ; jalrp r15 }
	{ rotli r5, r6, 5 ; ld2u r25, r26 ; cmpleu r15, r16, r17 }
	{ rotli r5, r6, 5 ; ld4s r25, r26 ; add r15, r16, r17 }
	{ rotli r5, r6, 5 ; ld4s r25, r26 ; shrsi r15, r16, 5 }
	{ rotli r5, r6, 5 ; ld4u r25, r26 ; shl r15, r16, r17 }
	{ rotli r5, r6, 5 ; lnk r15 ; ld4u r25, r26 }
	{ rotli r5, r6, 5 ; move r15, r16 ; ld4u r25, r26 }
	{ rotli r5, r6, 5 ; mz r15, r16, r17 ; ld4u r25, r26 }
	{ rotli r5, r6, 5 ; nor r15, r16, r17 ; prefetch_l1 r25 }
	{ rotli r5, r6, 5 ; prefetch r25 ; cmples r15, r16, r17 }
	{ rotli r5, r6, 5 ; prefetch_add_l1_fault r15, 5 }
	{ rotli r5, r6, 5 ; prefetch_l1 r25 ; shl2add r15, r16, r17 }
	{ rotli r5, r6, 5 ; prefetch_l1_fault r25 ; nop }
	{ rotli r5, r6, 5 ; prefetch_l2 r25 ; jalrp r15 }
	{ rotli r5, r6, 5 ; prefetch_l2_fault r25 ; cmplts r15, r16, r17 }
	{ rotli r5, r6, 5 ; prefetch_l3 r25 ; addx r15, r16, r17 }
	{ rotli r5, r6, 5 ; prefetch_l3 r25 ; shrui r15, r16, 5 }
	{ rotli r5, r6, 5 ; prefetch_l3_fault r25 ; shl2add r15, r16, r17 }
	{ rotli r5, r6, 5 ; rotli r15, r16, 5 ; prefetch r25 }
	{ rotli r5, r6, 5 ; shl1add r15, r16, r17 ; prefetch_l1 r25 }
	{ rotli r5, r6, 5 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
	{ rotli r5, r6, 5 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
	{ rotli r5, r6, 5 ; shli r15, r16, 5 ; st r25, r26 }
	{ rotli r5, r6, 5 ; shrsi r15, r16, 5 ; st r25, r26 }
	{ rotli r5, r6, 5 ; shrui r15, r16, 5 ; st2 r25, r26 }
	{ rotli r5, r6, 5 ; st r25, r26 ; shl2add r15, r16, r17 }
	{ rotli r5, r6, 5 ; st1 r25, r26 ; nop }
	{ rotli r5, r6, 5 ; st2 r25, r26 ; jalr r15 }
	{ rotli r5, r6, 5 ; st4 r25, r26 ; cmples r15, r16, r17 }
	{ rotli r5, r6, 5 ; st_add r15, r16, 5 }
	{ rotli r5, r6, 5 ; subx r15, r16, r17 ; prefetch_l3 r25 }
	{ rotli r5, r6, 5 ; v2cmpeqi r15, r16, 5 }
	{ rotli r5, r6, 5 ; xor r15, r16, r17 ; ld r25, r26 }
	{ shl r15, r16, r17 ; addi r5, r6, 5 ; ld1s r25, r26 }
	{ shl r15, r16, r17 ; addxi r5, r6, 5 ; ld1u r25, r26 }
	{ shl r15, r16, r17 ; andi r5, r6, 5 ; ld1u r25, r26 }
	{ shl r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld1s r25, r26 }
	{ shl r15, r16, r17 ; cmpeq r5, r6, r7 ; ld2s r25, r26 }
	{ shl r15, r16, r17 ; cmples r5, r6, r7 ; ld4s r25, r26 }
	{ shl r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch r25 }
	{ shl r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l1_fault r25 }
	{ shl r15, r16, r17 ; ctz r5, r6 ; ld1s r25, r26 }
	{ shl r15, r16, r17 ; fnop ; prefetch_l2 r25 }
	{ shl r15, r16, r17 ; info 19 ; ld4u r25, r26 }
	{ shl r15, r16, r17 ; ld r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shl r15, r16, r17 ; ld1s r25, r26 ; addxi r5, r6, 5 }
	{ shl r15, r16, r17 ; ld1s r25, r26 ; shl r5, r6, r7 }
	{ shl r15, r16, r17 ; ld1u r25, r26 ; info 19 }
	{ shl r15, r16, r17 ; ld1u r25, r26 ; tblidxb3 r5, r6 }
	{ shl r15, r16, r17 ; ld2s r25, r26 ; or r5, r6, r7 }
	{ shl r15, r16, r17 ; ld2u r25, r26 ; cmpltsi r5, r6, 5 }
	{ shl r15, r16, r17 ; ld2u r25, r26 ; shrui r5, r6, 5 }
	{ shl r15, r16, r17 ; ld4s r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ shl r15, r16, r17 ; ld4u r25, r26 ; cmovnez r5, r6, r7 }
	{ shl r15, r16, r17 ; ld4u r25, r26 ; shl3add r5, r6, r7 }
	{ shl r15, r16, r17 ; move r5, r6 ; ld4s r25, r26 }
	{ shl r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; ld4u r25, r26 }
	{ shl r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; ld2s r25, r26 }
	{ shl r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; ld2u r25, r26 }
	{ shl r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld1s r25, r26 }
	{ shl r15, r16, r17 ; mulax r5, r6, r7 ; ld1u r25, r26 }
	{ shl r15, r16, r17 ; mz r5, r6, r7 ; ld2u r25, r26 }
	{ shl r15, r16, r17 ; nor r5, r6, r7 ; ld4u r25, r26 }
	{ shl r15, r16, r17 ; pcnt r5, r6 ; prefetch r25 }
	{ shl r15, r16, r17 ; prefetch r25 ; mula_hs_hs r5, r6, r7 }
	{ shl r15, r16, r17 ; prefetch_l1 r25 ; andi r5, r6, 5 }
	{ shl r15, r16, r17 ; prefetch_l1 r25 ; shl1addx r5, r6, r7 }
	{ shl r15, r16, r17 ; prefetch_l1_fault r25 ; move r5, r6 }
	{ shl r15, r16, r17 ; prefetch_l1_fault r25 }
	{ shl r15, r16, r17 ; prefetch_l2 r25 ; revbits r5, r6 }
	{ shl r15, r16, r17 ; prefetch_l2_fault r25 ; cmpne r5, r6, r7 }
	{ shl r15, r16, r17 ; prefetch_l2_fault r25 ; subx r5, r6, r7 }
	{ shl r15, r16, r17 ; prefetch_l3 r25 ; mulx r5, r6, r7 }
	{ shl r15, r16, r17 ; prefetch_l3_fault r25 ; cmpeqi r5, r6, 5 }
	{ shl r15, r16, r17 ; prefetch_l3_fault r25 ; shli r5, r6, 5 }
	{ shl r15, r16, r17 ; revbytes r5, r6 ; prefetch_l1 r25 }
	{ shl r15, r16, r17 ; rotli r5, r6, 5 ; prefetch_l2 r25 }
	{ shl r15, r16, r17 ; shl1add r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shl r15, r16, r17 ; shl2add r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shl r15, r16, r17 ; shl3add r5, r6, r7 ; st1 r25, r26 }
	{ shl r15, r16, r17 ; shli r5, r6, 5 ; st4 r25, r26 }
	{ shl r15, r16, r17 ; shrsi r5, r6, 5 ; st4 r25, r26 }
	{ shl r15, r16, r17 ; shrux r5, r6, r7 }
	{ shl r15, r16, r17 ; st r25, r26 ; or r5, r6, r7 }
	{ shl r15, r16, r17 ; st1 r25, r26 ; cmpltsi r5, r6, 5 }
	{ shl r15, r16, r17 ; st1 r25, r26 ; shrui r5, r6, 5 }
	{ shl r15, r16, r17 ; st2 r25, r26 ; mula_lu_lu r5, r6, r7 }
	{ shl r15, r16, r17 ; st4 r25, r26 ; cmovnez r5, r6, r7 }
	{ shl r15, r16, r17 ; st4 r25, r26 ; shl3add r5, r6, r7 }
	{ shl r15, r16, r17 ; subx r5, r6, r7 ; ld4u r25, r26 }
	{ shl r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch r25 }
	{ shl r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l1_fault r25 }
	{ shl r15, r16, r17 ; v1mnz r5, r6, r7 }
	{ shl r15, r16, r17 ; v2mults r5, r6, r7 }
	{ shl r15, r16, r17 ; xor r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shl r5, r6, r7 ; addi r15, r16, 5 ; prefetch_l3 r25 }
	{ shl r5, r6, r7 ; addxi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ shl r5, r6, r7 ; andi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ shl r5, r6, r7 ; cmpeqi r15, r16, 5 ; st1 r25, r26 }
	{ shl r5, r6, r7 ; cmpleu r15, r16, r17 ; st1 r25, r26 }
	{ shl r5, r6, r7 ; cmpltsi r15, r16, 5 ; st4 r25, r26 }
	{ shl r5, r6, r7 ; cmpne r15, r16, r17 }
	{ shl r5, r6, r7 ; ill ; ld1u r25, r26 }
	{ shl r5, r6, r7 ; jalr r15 ; ld1s r25, r26 }
	{ shl r5, r6, r7 ; jr r15 ; ld2s r25, r26 }
	{ shl r5, r6, r7 ; ld r25, r26 ; and r15, r16, r17 }
	{ shl r5, r6, r7 ; ld r25, r26 ; subx r15, r16, r17 }
	{ shl r5, r6, r7 ; ld1s r25, r26 ; shl3add r15, r16, r17 }
	{ shl r5, r6, r7 ; ld1u r25, r26 ; nor r15, r16, r17 }
	{ shl r5, r6, r7 ; ld2s r25, r26 ; jalrp r15 }
	{ shl r5, r6, r7 ; ld2u r25, r26 ; cmpleu r15, r16, r17 }
	{ shl r5, r6, r7 ; ld4s r25, r26 ; add r15, r16, r17 }
	{ shl r5, r6, r7 ; ld4s r25, r26 ; shrsi r15, r16, 5 }
	{ shl r5, r6, r7 ; ld4u r25, r26 ; shl r15, r16, r17 }
	{ shl r5, r6, r7 ; lnk r15 ; ld4u r25, r26 }
	{ shl r5, r6, r7 ; move r15, r16 ; ld4u r25, r26 }
	{ shl r5, r6, r7 ; mz r15, r16, r17 ; ld4u r25, r26 }
	{ shl r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l1 r25 }
	{ shl r5, r6, r7 ; prefetch r25 ; cmples r15, r16, r17 }
	{ shl r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
	{ shl r5, r6, r7 ; prefetch_l1 r25 ; shl2add r15, r16, r17 }
	{ shl r5, r6, r7 ; prefetch_l1_fault r25 ; nop }
	{ shl r5, r6, r7 ; prefetch_l2 r25 ; jalrp r15 }
	{ shl r5, r6, r7 ; prefetch_l2_fault r25 ; cmplts r15, r16, r17 }
	{ shl r5, r6, r7 ; prefetch_l3 r25 ; addx r15, r16, r17 }
	{ shl r5, r6, r7 ; prefetch_l3 r25 ; shrui r15, r16, 5 }
	{ shl r5, r6, r7 ; prefetch_l3_fault r25 ; shl2add r15, r16, r17 }
	{ shl r5, r6, r7 ; rotli r15, r16, 5 ; prefetch r25 }
	{ shl r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l1 r25 }
	{ shl r5, r6, r7 ; shl2add r15, r16, r17 ; prefetch_l2 r25 }
	{ shl r5, r6, r7 ; shl3add r15, r16, r17 ; prefetch_l3 r25 }
	{ shl r5, r6, r7 ; shli r15, r16, 5 ; st r25, r26 }
	{ shl r5, r6, r7 ; shrsi r15, r16, 5 ; st r25, r26 }
	{ shl r5, r6, r7 ; shrui r15, r16, 5 ; st2 r25, r26 }
	{ shl r5, r6, r7 ; st r25, r26 ; shl2add r15, r16, r17 }
	{ shl r5, r6, r7 ; st1 r25, r26 ; nop }
	{ shl r5, r6, r7 ; st2 r25, r26 ; jalr r15 }
	{ shl r5, r6, r7 ; st4 r25, r26 ; cmples r15, r16, r17 }
	{ shl r5, r6, r7 ; st_add r15, r16, 5 }
	{ shl r5, r6, r7 ; subx r15, r16, r17 ; prefetch_l3 r25 }
	{ shl r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
	{ shl r5, r6, r7 ; xor r15, r16, r17 ; ld r25, r26 }
	{ shl16insli r15, r16, 0x1234 ; cmpltsi r5, r6, 5 }
	{ shl16insli r15, r16, 0x1234 ; moveli r5, 0x1234 }
	{ shl16insli r15, r16, 0x1234 ; shl3addx r5, r6, r7 }
	{ shl16insli r15, r16, 0x1234 ; v1dotpus r5, r6, r7 }
	{ shl16insli r15, r16, 0x1234 ; v2int_l r5, r6, r7 }
	{ shl16insli r5, r6, 0x1234 ; addi r15, r16, 5 }
	{ shl16insli r5, r6, 0x1234 ; infol 0x1234 }
	{ shl16insli r5, r6, 0x1234 ; mnz r15, r16, r17 }
	{ shl16insli r5, r6, 0x1234 ; shrui r15, r16, 5 }
	{ shl16insli r5, r6, 0x1234 ; v1mnz r15, r16, r17 }
	{ shl16insli r5, r6, 0x1234 ; v2sub r15, r16, r17 }
	{ shl1add r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
	{ shl1add r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
	{ shl1add r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
	{ shl1add r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld4s r25, r26 }
	{ shl1add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
	{ shl1add r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
	{ shl1add r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shl1add r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shl1add r15, r16, r17 ; ctz r5, r6 ; ld4s r25, r26 }
	{ shl1add r15, r16, r17 ; fnop ; st r25, r26 }
	{ shl1add r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
	{ shl1add r15, r16, r17 ; ld r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ shl1add r15, r16, r17 ; ld1s r25, r26 ; cmoveqz r5, r6, r7 }
	{ shl1add r15, r16, r17 ; ld1s r25, r26 ; shl2addx r5, r6, r7 }
	{ shl1add r15, r16, r17 ; ld1u r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ shl1add r15, r16, r17 ; ld2s r25, r26 ; addi r5, r6, 5 }
	{ shl1add r15, r16, r17 ; ld2s r25, r26 ; rotl r5, r6, r7 }
	{ shl1add r15, r16, r17 ; ld2u r25, r26 ; fnop }
	{ shl1add r15, r16, r17 ; ld2u r25, r26 ; tblidxb1 r5, r6 }
	{ shl1add r15, r16, r17 ; ld4s r25, r26 ; nop }
	{ shl1add r15, r16, r17 ; ld4u r25, r26 ; cmpleu r5, r6, r7 }
	{ shl1add r15, r16, r17 ; ld4u r25, r26 ; shrsi r5, r6, 5 }
	{ shl1add r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
	{ shl1add r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ shl1add r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch r25 }
	{ shl1add r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ shl1add r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ shl1add r15, r16, r17 ; mulax r5, r6, r7 ; ld4u r25, r26 }
	{ shl1add r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1 r25 }
	{ shl1add r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
	{ shl1add r15, r16, r17 ; pcnt r5, r6 ; prefetch_l2_fault r25 }
	{ shl1add r15, r16, r17 ; prefetch r25 ; mulax r5, r6, r7 }
	{ shl1add r15, r16, r17 ; prefetch_l1 r25 ; cmpeq r5, r6, r7 }
	{ shl1add r15, r16, r17 ; prefetch_l1 r25 ; shl3addx r5, r6, r7 }
	{ shl1add r15, r16, r17 ; prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 }
	{ shl1add r15, r16, r17 ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ shl1add r15, r16, r17 ; prefetch_l2 r25 ; shl r5, r6, r7 }
	{ shl1add r15, r16, r17 ; prefetch_l2_fault r25 ; info 19 }
	{ shl1add r15, r16, r17 ; prefetch_l2_fault r25 ; tblidxb3 r5, r6 }
	{ shl1add r15, r16, r17 ; prefetch_l3 r25 ; or r5, r6, r7 }
	{ shl1add r15, r16, r17 ; prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 }
	{ shl1add r15, r16, r17 ; prefetch_l3_fault r25 ; shrui r5, r6, 5 }
	{ shl1add r15, r16, r17 ; revbytes r5, r6 ; prefetch_l3 r25 }
	{ shl1add r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
	{ shl1add r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ shl1add r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
	{ shl1add r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
	{ shl1add r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
	{ shl1add r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
	{ shl1add r15, r16, r17 ; st r25, r26 ; addi r5, r6, 5 }
	{ shl1add r15, r16, r17 ; st r25, r26 ; rotl r5, r6, r7 }
	{ shl1add r15, r16, r17 ; st1 r25, r26 ; fnop }
	{ shl1add r15, r16, r17 ; st1 r25, r26 ; tblidxb1 r5, r6 }
	{ shl1add r15, r16, r17 ; st2 r25, r26 ; nop }
	{ shl1add r15, r16, r17 ; st4 r25, r26 ; cmpleu r5, r6, r7 }
	{ shl1add r15, r16, r17 ; st4 r25, r26 ; shrsi r5, r6, 5 }
	{ shl1add r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
	{ shl1add r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
	{ shl1add r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l3_fault r25 }
	{ shl1add r15, r16, r17 ; v1mz r5, r6, r7 }
	{ shl1add r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ shl1add r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
	{ shl1add r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
	{ shl1add r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
	{ shl1add r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
	{ shl1add r5, r6, r7 ; cmpexch r15, r16, r17 }
	{ shl1add r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
	{ shl1add r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
	{ shl1add r5, r6, r7 ; dtlbpr r15 }
	{ shl1add r5, r6, r7 ; ill ; ld4u r25, r26 }
	{ shl1add r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
	{ shl1add r5, r6, r7 ; jr r15 ; prefetch r25 }
	{ shl1add r5, r6, r7 ; ld r25, r26 ; cmples r15, r16, r17 }
	{ shl1add r5, r6, r7 ; ld1s r25, r26 ; add r15, r16, r17 }
	{ shl1add r5, r6, r7 ; ld1s r25, r26 ; shrsi r15, r16, 5 }
	{ shl1add r5, r6, r7 ; ld1u r25, r26 ; shl r15, r16, r17 }
	{ shl1add r5, r6, r7 ; ld2s r25, r26 ; mnz r15, r16, r17 }
	{ shl1add r5, r6, r7 ; ld2u r25, r26 ; cmpne r15, r16, r17 }
	{ shl1add r5, r6, r7 ; ld4s r25, r26 ; and r15, r16, r17 }
	{ shl1add r5, r6, r7 ; ld4s r25, r26 ; subx r15, r16, r17 }
	{ shl1add r5, r6, r7 ; ld4u r25, r26 ; shl2addx r15, r16, r17 }
	{ shl1add r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
	{ shl1add r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
	{ shl1add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
	{ shl1add r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
	{ shl1add r5, r6, r7 ; prefetch r25 ; cmpltu r15, r16, r17 }
	{ shl1add r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ shl1add r5, r6, r7 ; prefetch_l1 r25 ; shli r15, r16, 5 }
	{ shl1add r5, r6, r7 ; prefetch_l1_fault r25 ; rotli r15, r16, 5 }
	{ shl1add r5, r6, r7 ; prefetch_l2 r25 ; mnz r15, r16, r17 }
	{ shl1add r5, r6, r7 ; prefetch_l2_fault r25 ; fnop }
	{ shl1add r5, r6, r7 ; prefetch_l3 r25 ; cmpeq r15, r16, r17 }
	{ shl1add r5, r6, r7 ; prefetch_l3 r25 }
	{ shl1add r5, r6, r7 ; prefetch_l3_fault r25 ; shli r15, r16, 5 }
	{ shl1add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ shl1add r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
	{ shl1add r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
	{ shl1add r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
	{ shl1add r5, r6, r7 ; shli r15, r16, 5 }
	{ shl1add r5, r6, r7 ; shrsi r15, r16, 5 }
	{ shl1add r5, r6, r7 ; shruxi r15, r16, 5 }
	{ shl1add r5, r6, r7 ; st r25, r26 ; shli r15, r16, 5 }
	{ shl1add r5, r6, r7 ; st1 r25, r26 ; rotli r15, r16, 5 }
	{ shl1add r5, r6, r7 ; st2 r25, r26 ; lnk r15 }
	{ shl1add r5, r6, r7 ; st4 r25, r26 ; cmpltu r15, r16, r17 }
	{ shl1add r5, r6, r7 ; stnt2 r15, r16 }
	{ shl1add r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
	{ shl1add r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ shl1add r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
	{ shl1addx r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
	{ shl1addx r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
	{ shl1addx r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
	{ shl1addx r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld4s r25, r26 }
	{ shl1addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
	{ shl1addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
	{ shl1addx r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shl1addx r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shl1addx r15, r16, r17 ; ctz r5, r6 ; ld4s r25, r26 }
	{ shl1addx r15, r16, r17 ; fnop ; st r25, r26 }
	{ shl1addx r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
	{ shl1addx r15, r16, r17 ; ld r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; ld1s r25, r26 ; cmoveqz r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; ld1s r25, r26 ; shl2addx r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; ld1u r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; ld2s r25, r26 ; addi r5, r6, 5 }
	{ shl1addx r15, r16, r17 ; ld2s r25, r26 ; rotl r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; ld2u r25, r26 ; fnop }
	{ shl1addx r15, r16, r17 ; ld2u r25, r26 ; tblidxb1 r5, r6 }
	{ shl1addx r15, r16, r17 ; ld4s r25, r26 ; nop }
	{ shl1addx r15, r16, r17 ; ld4u r25, r26 ; cmpleu r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; ld4u r25, r26 ; shrsi r5, r6, 5 }
	{ shl1addx r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
	{ shl1addx r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ shl1addx r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch r25 }
	{ shl1addx r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ shl1addx r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ shl1addx r15, r16, r17 ; mulax r5, r6, r7 ; ld4u r25, r26 }
	{ shl1addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1 r25 }
	{ shl1addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
	{ shl1addx r15, r16, r17 ; pcnt r5, r6 ; prefetch_l2_fault r25 }
	{ shl1addx r15, r16, r17 ; prefetch r25 ; mulax r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; prefetch_l1 r25 ; cmpeq r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; prefetch_l1 r25 ; shl3addx r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ shl1addx r15, r16, r17 ; prefetch_l2 r25 ; shl r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; prefetch_l2_fault r25 ; info 19 }
	{ shl1addx r15, r16, r17 ; prefetch_l2_fault r25 ; tblidxb3 r5, r6 }
	{ shl1addx r15, r16, r17 ; prefetch_l3 r25 ; or r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 }
	{ shl1addx r15, r16, r17 ; prefetch_l3_fault r25 ; shrui r5, r6, 5 }
	{ shl1addx r15, r16, r17 ; revbytes r5, r6 ; prefetch_l3 r25 }
	{ shl1addx r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
	{ shl1addx r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ shl1addx r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
	{ shl1addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
	{ shl1addx r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
	{ shl1addx r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
	{ shl1addx r15, r16, r17 ; st r25, r26 ; addi r5, r6, 5 }
	{ shl1addx r15, r16, r17 ; st r25, r26 ; rotl r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; st1 r25, r26 ; fnop }
	{ shl1addx r15, r16, r17 ; st1 r25, r26 ; tblidxb1 r5, r6 }
	{ shl1addx r15, r16, r17 ; st2 r25, r26 ; nop }
	{ shl1addx r15, r16, r17 ; st4 r25, r26 ; cmpleu r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; st4 r25, r26 ; shrsi r5, r6, 5 }
	{ shl1addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
	{ shl1addx r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
	{ shl1addx r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l3_fault r25 }
	{ shl1addx r15, r16, r17 ; v1mz r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ shl1addx r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
	{ shl1addx r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
	{ shl1addx r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
	{ shl1addx r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
	{ shl1addx r5, r6, r7 ; cmpexch r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
	{ shl1addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
	{ shl1addx r5, r6, r7 ; dtlbpr r15 }
	{ shl1addx r5, r6, r7 ; ill ; ld4u r25, r26 }
	{ shl1addx r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
	{ shl1addx r5, r6, r7 ; jr r15 ; prefetch r25 }
	{ shl1addx r5, r6, r7 ; ld r25, r26 ; cmples r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; ld1s r25, r26 ; add r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; ld1s r25, r26 ; shrsi r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; ld1u r25, r26 ; shl r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; ld2s r25, r26 ; mnz r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; ld2u r25, r26 ; cmpne r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; ld4s r25, r26 ; and r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; ld4s r25, r26 ; subx r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; ld4u r25, r26 ; shl2addx r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
	{ shl1addx r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
	{ shl1addx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
	{ shl1addx r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
	{ shl1addx r5, r6, r7 ; prefetch r25 ; cmpltu r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ shl1addx r5, r6, r7 ; prefetch_l1 r25 ; shli r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; prefetch_l1_fault r25 ; rotli r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; prefetch_l2 r25 ; mnz r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; prefetch_l2_fault r25 ; fnop }
	{ shl1addx r5, r6, r7 ; prefetch_l3 r25 ; cmpeq r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; prefetch_l3 r25 }
	{ shl1addx r5, r6, r7 ; prefetch_l3_fault r25 ; shli r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ shl1addx r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
	{ shl1addx r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
	{ shl1addx r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
	{ shl1addx r5, r6, r7 ; shli r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; shrsi r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; shruxi r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; st r25, r26 ; shli r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; st1 r25, r26 ; rotli r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; st2 r25, r26 ; lnk r15 }
	{ shl1addx r5, r6, r7 ; st4 r25, r26 ; cmpltu r15, r16, r17 }
	{ shl1addx r5, r6, r7 ; stnt2 r15, r16 }
	{ shl1addx r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
	{ shl1addx r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ shl1addx r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
	{ shl2add r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
	{ shl2add r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
	{ shl2add r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
	{ shl2add r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld4s r25, r26 }
	{ shl2add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
	{ shl2add r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
	{ shl2add r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shl2add r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shl2add r15, r16, r17 ; ctz r5, r6 ; ld4s r25, r26 }
	{ shl2add r15, r16, r17 ; fnop ; st r25, r26 }
	{ shl2add r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
	{ shl2add r15, r16, r17 ; ld r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ shl2add r15, r16, r17 ; ld1s r25, r26 ; cmoveqz r5, r6, r7 }
	{ shl2add r15, r16, r17 ; ld1s r25, r26 ; shl2addx r5, r6, r7 }
	{ shl2add r15, r16, r17 ; ld1u r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ shl2add r15, r16, r17 ; ld2s r25, r26 ; addi r5, r6, 5 }
	{ shl2add r15, r16, r17 ; ld2s r25, r26 ; rotl r5, r6, r7 }
	{ shl2add r15, r16, r17 ; ld2u r25, r26 ; fnop }
	{ shl2add r15, r16, r17 ; ld2u r25, r26 ; tblidxb1 r5, r6 }
	{ shl2add r15, r16, r17 ; ld4s r25, r26 ; nop }
	{ shl2add r15, r16, r17 ; ld4u r25, r26 ; cmpleu r5, r6, r7 }
	{ shl2add r15, r16, r17 ; ld4u r25, r26 ; shrsi r5, r6, 5 }
	{ shl2add r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
	{ shl2add r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ shl2add r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch r25 }
	{ shl2add r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ shl2add r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ shl2add r15, r16, r17 ; mulax r5, r6, r7 ; ld4u r25, r26 }
	{ shl2add r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1 r25 }
	{ shl2add r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
	{ shl2add r15, r16, r17 ; pcnt r5, r6 ; prefetch_l2_fault r25 }
	{ shl2add r15, r16, r17 ; prefetch r25 ; mulax r5, r6, r7 }
	{ shl2add r15, r16, r17 ; prefetch_l1 r25 ; cmpeq r5, r6, r7 }
	{ shl2add r15, r16, r17 ; prefetch_l1 r25 ; shl3addx r5, r6, r7 }
	{ shl2add r15, r16, r17 ; prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 }
	{ shl2add r15, r16, r17 ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ shl2add r15, r16, r17 ; prefetch_l2 r25 ; shl r5, r6, r7 }
	{ shl2add r15, r16, r17 ; prefetch_l2_fault r25 ; info 19 }
	{ shl2add r15, r16, r17 ; prefetch_l2_fault r25 ; tblidxb3 r5, r6 }
	{ shl2add r15, r16, r17 ; prefetch_l3 r25 ; or r5, r6, r7 }
	{ shl2add r15, r16, r17 ; prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 }
	{ shl2add r15, r16, r17 ; prefetch_l3_fault r25 ; shrui r5, r6, 5 }
	{ shl2add r15, r16, r17 ; revbytes r5, r6 ; prefetch_l3 r25 }
	{ shl2add r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
	{ shl2add r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ shl2add r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
	{ shl2add r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
	{ shl2add r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
	{ shl2add r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
	{ shl2add r15, r16, r17 ; st r25, r26 ; addi r5, r6, 5 }
	{ shl2add r15, r16, r17 ; st r25, r26 ; rotl r5, r6, r7 }
	{ shl2add r15, r16, r17 ; st1 r25, r26 ; fnop }
	{ shl2add r15, r16, r17 ; st1 r25, r26 ; tblidxb1 r5, r6 }
	{ shl2add r15, r16, r17 ; st2 r25, r26 ; nop }
	{ shl2add r15, r16, r17 ; st4 r25, r26 ; cmpleu r5, r6, r7 }
	{ shl2add r15, r16, r17 ; st4 r25, r26 ; shrsi r5, r6, 5 }
	{ shl2add r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
	{ shl2add r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
	{ shl2add r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l3_fault r25 }
	{ shl2add r15, r16, r17 ; v1mz r5, r6, r7 }
	{ shl2add r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ shl2add r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
	{ shl2add r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
	{ shl2add r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
	{ shl2add r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
	{ shl2add r5, r6, r7 ; cmpexch r15, r16, r17 }
	{ shl2add r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
	{ shl2add r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
	{ shl2add r5, r6, r7 ; dtlbpr r15 }
	{ shl2add r5, r6, r7 ; ill ; ld4u r25, r26 }
	{ shl2add r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
	{ shl2add r5, r6, r7 ; jr r15 ; prefetch r25 }
	{ shl2add r5, r6, r7 ; ld r25, r26 ; cmples r15, r16, r17 }
	{ shl2add r5, r6, r7 ; ld1s r25, r26 ; add r15, r16, r17 }
	{ shl2add r5, r6, r7 ; ld1s r25, r26 ; shrsi r15, r16, 5 }
	{ shl2add r5, r6, r7 ; ld1u r25, r26 ; shl r15, r16, r17 }
	{ shl2add r5, r6, r7 ; ld2s r25, r26 ; mnz r15, r16, r17 }
	{ shl2add r5, r6, r7 ; ld2u r25, r26 ; cmpne r15, r16, r17 }
	{ shl2add r5, r6, r7 ; ld4s r25, r26 ; and r15, r16, r17 }
	{ shl2add r5, r6, r7 ; ld4s r25, r26 ; subx r15, r16, r17 }
	{ shl2add r5, r6, r7 ; ld4u r25, r26 ; shl2addx r15, r16, r17 }
	{ shl2add r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
	{ shl2add r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
	{ shl2add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
	{ shl2add r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
	{ shl2add r5, r6, r7 ; prefetch r25 ; cmpltu r15, r16, r17 }
	{ shl2add r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ shl2add r5, r6, r7 ; prefetch_l1 r25 ; shli r15, r16, 5 }
	{ shl2add r5, r6, r7 ; prefetch_l1_fault r25 ; rotli r15, r16, 5 }
	{ shl2add r5, r6, r7 ; prefetch_l2 r25 ; mnz r15, r16, r17 }
	{ shl2add r5, r6, r7 ; prefetch_l2_fault r25 ; fnop }
	{ shl2add r5, r6, r7 ; prefetch_l3 r25 ; cmpeq r15, r16, r17 }
	{ shl2add r5, r6, r7 ; prefetch_l3 r25 }
	{ shl2add r5, r6, r7 ; prefetch_l3_fault r25 ; shli r15, r16, 5 }
	{ shl2add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ shl2add r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
	{ shl2add r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
	{ shl2add r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
	{ shl2add r5, r6, r7 ; shli r15, r16, 5 }
	{ shl2add r5, r6, r7 ; shrsi r15, r16, 5 }
	{ shl2add r5, r6, r7 ; shruxi r15, r16, 5 }
	{ shl2add r5, r6, r7 ; st r25, r26 ; shli r15, r16, 5 }
	{ shl2add r5, r6, r7 ; st1 r25, r26 ; rotli r15, r16, 5 }
	{ shl2add r5, r6, r7 ; st2 r25, r26 ; lnk r15 }
	{ shl2add r5, r6, r7 ; st4 r25, r26 ; cmpltu r15, r16, r17 }
	{ shl2add r5, r6, r7 ; stnt2 r15, r16 }
	{ shl2add r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
	{ shl2add r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ shl2add r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
	{ shl2addx r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
	{ shl2addx r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
	{ shl2addx r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
	{ shl2addx r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld4s r25, r26 }
	{ shl2addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
	{ shl2addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
	{ shl2addx r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shl2addx r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shl2addx r15, r16, r17 ; ctz r5, r6 ; ld4s r25, r26 }
	{ shl2addx r15, r16, r17 ; fnop ; st r25, r26 }
	{ shl2addx r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
	{ shl2addx r15, r16, r17 ; ld r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; ld1s r25, r26 ; cmoveqz r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; ld1s r25, r26 ; shl2addx r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; ld1u r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; ld2s r25, r26 ; addi r5, r6, 5 }
	{ shl2addx r15, r16, r17 ; ld2s r25, r26 ; rotl r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; ld2u r25, r26 ; fnop }
	{ shl2addx r15, r16, r17 ; ld2u r25, r26 ; tblidxb1 r5, r6 }
	{ shl2addx r15, r16, r17 ; ld4s r25, r26 ; nop }
	{ shl2addx r15, r16, r17 ; ld4u r25, r26 ; cmpleu r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; ld4u r25, r26 ; shrsi r5, r6, 5 }
	{ shl2addx r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
	{ shl2addx r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ shl2addx r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch r25 }
	{ shl2addx r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ shl2addx r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ shl2addx r15, r16, r17 ; mulax r5, r6, r7 ; ld4u r25, r26 }
	{ shl2addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1 r25 }
	{ shl2addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
	{ shl2addx r15, r16, r17 ; pcnt r5, r6 ; prefetch_l2_fault r25 }
	{ shl2addx r15, r16, r17 ; prefetch r25 ; mulax r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; prefetch_l1 r25 ; cmpeq r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; prefetch_l1 r25 ; shl3addx r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ shl2addx r15, r16, r17 ; prefetch_l2 r25 ; shl r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; prefetch_l2_fault r25 ; info 19 }
	{ shl2addx r15, r16, r17 ; prefetch_l2_fault r25 ; tblidxb3 r5, r6 }
	{ shl2addx r15, r16, r17 ; prefetch_l3 r25 ; or r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 }
	{ shl2addx r15, r16, r17 ; prefetch_l3_fault r25 ; shrui r5, r6, 5 }
	{ shl2addx r15, r16, r17 ; revbytes r5, r6 ; prefetch_l3 r25 }
	{ shl2addx r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
	{ shl2addx r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ shl2addx r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
	{ shl2addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
	{ shl2addx r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
	{ shl2addx r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
	{ shl2addx r15, r16, r17 ; st r25, r26 ; addi r5, r6, 5 }
	{ shl2addx r15, r16, r17 ; st r25, r26 ; rotl r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; st1 r25, r26 ; fnop }
	{ shl2addx r15, r16, r17 ; st1 r25, r26 ; tblidxb1 r5, r6 }
	{ shl2addx r15, r16, r17 ; st2 r25, r26 ; nop }
	{ shl2addx r15, r16, r17 ; st4 r25, r26 ; cmpleu r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; st4 r25, r26 ; shrsi r5, r6, 5 }
	{ shl2addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
	{ shl2addx r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
	{ shl2addx r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l3_fault r25 }
	{ shl2addx r15, r16, r17 ; v1mz r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ shl2addx r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
	{ shl2addx r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
	{ shl2addx r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
	{ shl2addx r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
	{ shl2addx r5, r6, r7 ; cmpexch r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
	{ shl2addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
	{ shl2addx r5, r6, r7 ; dtlbpr r15 }
	{ shl2addx r5, r6, r7 ; ill ; ld4u r25, r26 }
	{ shl2addx r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
	{ shl2addx r5, r6, r7 ; jr r15 ; prefetch r25 }
	{ shl2addx r5, r6, r7 ; ld r25, r26 ; cmples r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; ld1s r25, r26 ; add r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; ld1s r25, r26 ; shrsi r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; ld1u r25, r26 ; shl r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; ld2s r25, r26 ; mnz r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; ld2u r25, r26 ; cmpne r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; ld4s r25, r26 ; and r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; ld4s r25, r26 ; subx r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; ld4u r25, r26 ; shl2addx r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
	{ shl2addx r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
	{ shl2addx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
	{ shl2addx r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
	{ shl2addx r5, r6, r7 ; prefetch r25 ; cmpltu r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ shl2addx r5, r6, r7 ; prefetch_l1 r25 ; shli r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; prefetch_l1_fault r25 ; rotli r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; prefetch_l2 r25 ; mnz r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; prefetch_l2_fault r25 ; fnop }
	{ shl2addx r5, r6, r7 ; prefetch_l3 r25 ; cmpeq r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; prefetch_l3 r25 }
	{ shl2addx r5, r6, r7 ; prefetch_l3_fault r25 ; shli r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ shl2addx r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
	{ shl2addx r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
	{ shl2addx r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
	{ shl2addx r5, r6, r7 ; shli r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; shrsi r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; shruxi r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; st r25, r26 ; shli r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; st1 r25, r26 ; rotli r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; st2 r25, r26 ; lnk r15 }
	{ shl2addx r5, r6, r7 ; st4 r25, r26 ; cmpltu r15, r16, r17 }
	{ shl2addx r5, r6, r7 ; stnt2 r15, r16 }
	{ shl2addx r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
	{ shl2addx r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ shl2addx r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
	{ shl3add r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
	{ shl3add r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
	{ shl3add r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
	{ shl3add r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld4s r25, r26 }
	{ shl3add r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
	{ shl3add r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
	{ shl3add r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shl3add r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shl3add r15, r16, r17 ; ctz r5, r6 ; ld4s r25, r26 }
	{ shl3add r15, r16, r17 ; fnop ; st r25, r26 }
	{ shl3add r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
	{ shl3add r15, r16, r17 ; ld r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ shl3add r15, r16, r17 ; ld1s r25, r26 ; cmoveqz r5, r6, r7 }
	{ shl3add r15, r16, r17 ; ld1s r25, r26 ; shl2addx r5, r6, r7 }
	{ shl3add r15, r16, r17 ; ld1u r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ shl3add r15, r16, r17 ; ld2s r25, r26 ; addi r5, r6, 5 }
	{ shl3add r15, r16, r17 ; ld2s r25, r26 ; rotl r5, r6, r7 }
	{ shl3add r15, r16, r17 ; ld2u r25, r26 ; fnop }
	{ shl3add r15, r16, r17 ; ld2u r25, r26 ; tblidxb1 r5, r6 }
	{ shl3add r15, r16, r17 ; ld4s r25, r26 ; nop }
	{ shl3add r15, r16, r17 ; ld4u r25, r26 ; cmpleu r5, r6, r7 }
	{ shl3add r15, r16, r17 ; ld4u r25, r26 ; shrsi r5, r6, 5 }
	{ shl3add r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
	{ shl3add r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ shl3add r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch r25 }
	{ shl3add r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ shl3add r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ shl3add r15, r16, r17 ; mulax r5, r6, r7 ; ld4u r25, r26 }
	{ shl3add r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1 r25 }
	{ shl3add r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
	{ shl3add r15, r16, r17 ; pcnt r5, r6 ; prefetch_l2_fault r25 }
	{ shl3add r15, r16, r17 ; prefetch r25 ; mulax r5, r6, r7 }
	{ shl3add r15, r16, r17 ; prefetch_l1 r25 ; cmpeq r5, r6, r7 }
	{ shl3add r15, r16, r17 ; prefetch_l1 r25 ; shl3addx r5, r6, r7 }
	{ shl3add r15, r16, r17 ; prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 }
	{ shl3add r15, r16, r17 ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ shl3add r15, r16, r17 ; prefetch_l2 r25 ; shl r5, r6, r7 }
	{ shl3add r15, r16, r17 ; prefetch_l2_fault r25 ; info 19 }
	{ shl3add r15, r16, r17 ; prefetch_l2_fault r25 ; tblidxb3 r5, r6 }
	{ shl3add r15, r16, r17 ; prefetch_l3 r25 ; or r5, r6, r7 }
	{ shl3add r15, r16, r17 ; prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 }
	{ shl3add r15, r16, r17 ; prefetch_l3_fault r25 ; shrui r5, r6, 5 }
	{ shl3add r15, r16, r17 ; revbytes r5, r6 ; prefetch_l3 r25 }
	{ shl3add r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
	{ shl3add r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ shl3add r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
	{ shl3add r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
	{ shl3add r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
	{ shl3add r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
	{ shl3add r15, r16, r17 ; st r25, r26 ; addi r5, r6, 5 }
	{ shl3add r15, r16, r17 ; st r25, r26 ; rotl r5, r6, r7 }
	{ shl3add r15, r16, r17 ; st1 r25, r26 ; fnop }
	{ shl3add r15, r16, r17 ; st1 r25, r26 ; tblidxb1 r5, r6 }
	{ shl3add r15, r16, r17 ; st2 r25, r26 ; nop }
	{ shl3add r15, r16, r17 ; st4 r25, r26 ; cmpleu r5, r6, r7 }
	{ shl3add r15, r16, r17 ; st4 r25, r26 ; shrsi r5, r6, 5 }
	{ shl3add r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
	{ shl3add r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
	{ shl3add r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l3_fault r25 }
	{ shl3add r15, r16, r17 ; v1mz r5, r6, r7 }
	{ shl3add r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ shl3add r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
	{ shl3add r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
	{ shl3add r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
	{ shl3add r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
	{ shl3add r5, r6, r7 ; cmpexch r15, r16, r17 }
	{ shl3add r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
	{ shl3add r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
	{ shl3add r5, r6, r7 ; dtlbpr r15 }
	{ shl3add r5, r6, r7 ; ill ; ld4u r25, r26 }
	{ shl3add r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
	{ shl3add r5, r6, r7 ; jr r15 ; prefetch r25 }
	{ shl3add r5, r6, r7 ; ld r25, r26 ; cmples r15, r16, r17 }
	{ shl3add r5, r6, r7 ; ld1s r25, r26 ; add r15, r16, r17 }
	{ shl3add r5, r6, r7 ; ld1s r25, r26 ; shrsi r15, r16, 5 }
	{ shl3add r5, r6, r7 ; ld1u r25, r26 ; shl r15, r16, r17 }
	{ shl3add r5, r6, r7 ; ld2s r25, r26 ; mnz r15, r16, r17 }
	{ shl3add r5, r6, r7 ; ld2u r25, r26 ; cmpne r15, r16, r17 }
	{ shl3add r5, r6, r7 ; ld4s r25, r26 ; and r15, r16, r17 }
	{ shl3add r5, r6, r7 ; ld4s r25, r26 ; subx r15, r16, r17 }
	{ shl3add r5, r6, r7 ; ld4u r25, r26 ; shl2addx r15, r16, r17 }
	{ shl3add r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
	{ shl3add r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
	{ shl3add r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
	{ shl3add r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
	{ shl3add r5, r6, r7 ; prefetch r25 ; cmpltu r15, r16, r17 }
	{ shl3add r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ shl3add r5, r6, r7 ; prefetch_l1 r25 ; shli r15, r16, 5 }
	{ shl3add r5, r6, r7 ; prefetch_l1_fault r25 ; rotli r15, r16, 5 }
	{ shl3add r5, r6, r7 ; prefetch_l2 r25 ; mnz r15, r16, r17 }
	{ shl3add r5, r6, r7 ; prefetch_l2_fault r25 ; fnop }
	{ shl3add r5, r6, r7 ; prefetch_l3 r25 ; cmpeq r15, r16, r17 }
	{ shl3add r5, r6, r7 ; prefetch_l3 r25 }
	{ shl3add r5, r6, r7 ; prefetch_l3_fault r25 ; shli r15, r16, 5 }
	{ shl3add r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ shl3add r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
	{ shl3add r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
	{ shl3add r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
	{ shl3add r5, r6, r7 ; shli r15, r16, 5 }
	{ shl3add r5, r6, r7 ; shrsi r15, r16, 5 }
	{ shl3add r5, r6, r7 ; shruxi r15, r16, 5 }
	{ shl3add r5, r6, r7 ; st r25, r26 ; shli r15, r16, 5 }
	{ shl3add r5, r6, r7 ; st1 r25, r26 ; rotli r15, r16, 5 }
	{ shl3add r5, r6, r7 ; st2 r25, r26 ; lnk r15 }
	{ shl3add r5, r6, r7 ; st4 r25, r26 ; cmpltu r15, r16, r17 }
	{ shl3add r5, r6, r7 ; stnt2 r15, r16 }
	{ shl3add r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
	{ shl3add r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ shl3add r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
	{ shl3addx r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
	{ shl3addx r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
	{ shl3addx r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
	{ shl3addx r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld4s r25, r26 }
	{ shl3addx r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
	{ shl3addx r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
	{ shl3addx r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shl3addx r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shl3addx r15, r16, r17 ; ctz r5, r6 ; ld4s r25, r26 }
	{ shl3addx r15, r16, r17 ; fnop ; st r25, r26 }
	{ shl3addx r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
	{ shl3addx r15, r16, r17 ; ld r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; ld1s r25, r26 ; cmoveqz r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; ld1s r25, r26 ; shl2addx r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; ld1u r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; ld2s r25, r26 ; addi r5, r6, 5 }
	{ shl3addx r15, r16, r17 ; ld2s r25, r26 ; rotl r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; ld2u r25, r26 ; fnop }
	{ shl3addx r15, r16, r17 ; ld2u r25, r26 ; tblidxb1 r5, r6 }
	{ shl3addx r15, r16, r17 ; ld4s r25, r26 ; nop }
	{ shl3addx r15, r16, r17 ; ld4u r25, r26 ; cmpleu r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; ld4u r25, r26 ; shrsi r5, r6, 5 }
	{ shl3addx r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
	{ shl3addx r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ shl3addx r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch r25 }
	{ shl3addx r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ shl3addx r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ shl3addx r15, r16, r17 ; mulax r5, r6, r7 ; ld4u r25, r26 }
	{ shl3addx r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1 r25 }
	{ shl3addx r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
	{ shl3addx r15, r16, r17 ; pcnt r5, r6 ; prefetch_l2_fault r25 }
	{ shl3addx r15, r16, r17 ; prefetch r25 ; mulax r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; prefetch_l1 r25 ; cmpeq r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; prefetch_l1 r25 ; shl3addx r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ shl3addx r15, r16, r17 ; prefetch_l2 r25 ; shl r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; prefetch_l2_fault r25 ; info 19 }
	{ shl3addx r15, r16, r17 ; prefetch_l2_fault r25 ; tblidxb3 r5, r6 }
	{ shl3addx r15, r16, r17 ; prefetch_l3 r25 ; or r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 }
	{ shl3addx r15, r16, r17 ; prefetch_l3_fault r25 ; shrui r5, r6, 5 }
	{ shl3addx r15, r16, r17 ; revbytes r5, r6 ; prefetch_l3 r25 }
	{ shl3addx r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
	{ shl3addx r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ shl3addx r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
	{ shl3addx r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
	{ shl3addx r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
	{ shl3addx r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
	{ shl3addx r15, r16, r17 ; st r25, r26 ; addi r5, r6, 5 }
	{ shl3addx r15, r16, r17 ; st r25, r26 ; rotl r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; st1 r25, r26 ; fnop }
	{ shl3addx r15, r16, r17 ; st1 r25, r26 ; tblidxb1 r5, r6 }
	{ shl3addx r15, r16, r17 ; st2 r25, r26 ; nop }
	{ shl3addx r15, r16, r17 ; st4 r25, r26 ; cmpleu r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; st4 r25, r26 ; shrsi r5, r6, 5 }
	{ shl3addx r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
	{ shl3addx r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
	{ shl3addx r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l3_fault r25 }
	{ shl3addx r15, r16, r17 ; v1mz r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ shl3addx r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
	{ shl3addx r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
	{ shl3addx r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
	{ shl3addx r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
	{ shl3addx r5, r6, r7 ; cmpexch r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
	{ shl3addx r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
	{ shl3addx r5, r6, r7 ; dtlbpr r15 }
	{ shl3addx r5, r6, r7 ; ill ; ld4u r25, r26 }
	{ shl3addx r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
	{ shl3addx r5, r6, r7 ; jr r15 ; prefetch r25 }
	{ shl3addx r5, r6, r7 ; ld r25, r26 ; cmples r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; ld1s r25, r26 ; add r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; ld1s r25, r26 ; shrsi r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; ld1u r25, r26 ; shl r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; ld2s r25, r26 ; mnz r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; ld2u r25, r26 ; cmpne r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; ld4s r25, r26 ; and r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; ld4s r25, r26 ; subx r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; ld4u r25, r26 ; shl2addx r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
	{ shl3addx r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
	{ shl3addx r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
	{ shl3addx r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
	{ shl3addx r5, r6, r7 ; prefetch r25 ; cmpltu r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ shl3addx r5, r6, r7 ; prefetch_l1 r25 ; shli r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; prefetch_l1_fault r25 ; rotli r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; prefetch_l2 r25 ; mnz r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; prefetch_l2_fault r25 ; fnop }
	{ shl3addx r5, r6, r7 ; prefetch_l3 r25 ; cmpeq r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; prefetch_l3 r25 }
	{ shl3addx r5, r6, r7 ; prefetch_l3_fault r25 ; shli r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ shl3addx r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
	{ shl3addx r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
	{ shl3addx r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
	{ shl3addx r5, r6, r7 ; shli r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; shrsi r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; shruxi r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; st r25, r26 ; shli r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; st1 r25, r26 ; rotli r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; st2 r25, r26 ; lnk r15 }
	{ shl3addx r5, r6, r7 ; st4 r25, r26 ; cmpltu r15, r16, r17 }
	{ shl3addx r5, r6, r7 ; stnt2 r15, r16 }
	{ shl3addx r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
	{ shl3addx r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ shl3addx r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
	{ shli r15, r16, 5 ; addi r5, r6, 5 ; ld4s r25, r26 }
	{ shli r15, r16, 5 ; addxi r5, r6, 5 ; ld4u r25, r26 }
	{ shli r15, r16, 5 ; andi r5, r6, 5 ; ld4u r25, r26 }
	{ shli r15, r16, 5 ; cmoveqz r5, r6, r7 ; ld4s r25, r26 }
	{ shli r15, r16, 5 ; cmpeq r5, r6, r7 ; prefetch r25 }
	{ shli r15, r16, 5 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
	{ shli r15, r16, 5 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
	{ shli r15, r16, 5 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shli r15, r16, 5 ; ctz r5, r6 ; ld4s r25, r26 }
	{ shli r15, r16, 5 ; fnop ; st r25, r26 }
	{ shli r15, r16, 5 ; info 19 ; prefetch_l2 r25 }
	{ shli r15, r16, 5 ; ld r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ shli r15, r16, 5 ; ld1s r25, r26 ; cmoveqz r5, r6, r7 }
	{ shli r15, r16, 5 ; ld1s r25, r26 ; shl2addx r5, r6, r7 }
	{ shli r15, r16, 5 ; ld1u r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ shli r15, r16, 5 ; ld2s r25, r26 ; addi r5, r6, 5 }
	{ shli r15, r16, 5 ; ld2s r25, r26 ; rotl r5, r6, r7 }
	{ shli r15, r16, 5 ; ld2u r25, r26 ; fnop }
	{ shli r15, r16, 5 ; ld2u r25, r26 ; tblidxb1 r5, r6 }
	{ shli r15, r16, 5 ; ld4s r25, r26 ; nop }
	{ shli r15, r16, 5 ; ld4u r25, r26 ; cmpleu r5, r6, r7 }
	{ shli r15, r16, 5 ; ld4u r25, r26 ; shrsi r5, r6, 5 }
	{ shli r15, r16, 5 ; move r5, r6 ; prefetch_l1_fault r25 }
	{ shli r15, r16, 5 ; mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ shli r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; prefetch r25 }
	{ shli r15, r16, 5 ; mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ shli r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ shli r15, r16, 5 ; mulax r5, r6, r7 ; ld4u r25, r26 }
	{ shli r15, r16, 5 ; mz r5, r6, r7 ; prefetch_l1 r25 }
	{ shli r15, r16, 5 ; nor r5, r6, r7 ; prefetch_l2 r25 }
	{ shli r15, r16, 5 ; pcnt r5, r6 ; prefetch_l2_fault r25 }
	{ shli r15, r16, 5 ; prefetch r25 ; mulax r5, r6, r7 }
	{ shli r15, r16, 5 ; prefetch_l1 r25 ; cmpeq r5, r6, r7 }
	{ shli r15, r16, 5 ; prefetch_l1 r25 ; shl3addx r5, r6, r7 }
	{ shli r15, r16, 5 ; prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 }
	{ shli r15, r16, 5 ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ shli r15, r16, 5 ; prefetch_l2 r25 ; shl r5, r6, r7 }
	{ shli r15, r16, 5 ; prefetch_l2_fault r25 ; info 19 }
	{ shli r15, r16, 5 ; prefetch_l2_fault r25 ; tblidxb3 r5, r6 }
	{ shli r15, r16, 5 ; prefetch_l3 r25 ; or r5, r6, r7 }
	{ shli r15, r16, 5 ; prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 }
	{ shli r15, r16, 5 ; prefetch_l3_fault r25 ; shrui r5, r6, 5 }
	{ shli r15, r16, 5 ; revbytes r5, r6 ; prefetch_l3 r25 }
	{ shli r15, r16, 5 ; rotli r5, r6, 5 ; st r25, r26 }
	{ shli r15, r16, 5 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ shli r15, r16, 5 ; shl2add r5, r6, r7 ; st4 r25, r26 }
	{ shli r15, r16, 5 ; shl3addx r5, r6, r7 ; ld r25, r26 }
	{ shli r15, r16, 5 ; shrs r5, r6, r7 ; ld r25, r26 }
	{ shli r15, r16, 5 ; shru r5, r6, r7 ; ld1u r25, r26 }
	{ shli r15, r16, 5 ; st r25, r26 ; addi r5, r6, 5 }
	{ shli r15, r16, 5 ; st r25, r26 ; rotl r5, r6, r7 }
	{ shli r15, r16, 5 ; st1 r25, r26 ; fnop }
	{ shli r15, r16, 5 ; st1 r25, r26 ; tblidxb1 r5, r6 }
	{ shli r15, r16, 5 ; st2 r25, r26 ; nop }
	{ shli r15, r16, 5 ; st4 r25, r26 ; cmpleu r5, r6, r7 }
	{ shli r15, r16, 5 ; st4 r25, r26 ; shrsi r5, r6, 5 }
	{ shli r15, r16, 5 ; subx r5, r6, r7 ; prefetch_l2 r25 }
	{ shli r15, r16, 5 ; tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
	{ shli r15, r16, 5 ; tblidxb3 r5, r6 ; prefetch_l3_fault r25 }
	{ shli r15, r16, 5 ; v1mz r5, r6, r7 }
	{ shli r15, r16, 5 ; v2packuc r5, r6, r7 }
	{ shli r15, r16, 5 ; xor r5, r6, r7 ; st1 r25, r26 }
	{ shli r5, r6, 5 ; addi r15, r16, 5 ; st2 r25, r26 }
	{ shli r5, r6, 5 ; addxi r15, r16, 5 ; st4 r25, r26 }
	{ shli r5, r6, 5 ; andi r15, r16, 5 ; st4 r25, r26 }
	{ shli r5, r6, 5 ; cmpexch r15, r16, r17 }
	{ shli r5, r6, 5 ; cmplts r15, r16, r17 ; ld r25, r26 }
	{ shli r5, r6, 5 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
	{ shli r5, r6, 5 ; dtlbpr r15 }
	{ shli r5, r6, 5 ; ill ; ld4u r25, r26 }
	{ shli r5, r6, 5 ; jalr r15 ; ld4s r25, r26 }
	{ shli r5, r6, 5 ; jr r15 ; prefetch r25 }
	{ shli r5, r6, 5 ; ld r25, r26 ; cmples r15, r16, r17 }
	{ shli r5, r6, 5 ; ld1s r25, r26 ; add r15, r16, r17 }
	{ shli r5, r6, 5 ; ld1s r25, r26 ; shrsi r15, r16, 5 }
	{ shli r5, r6, 5 ; ld1u r25, r26 ; shl r15, r16, r17 }
	{ shli r5, r6, 5 ; ld2s r25, r26 ; mnz r15, r16, r17 }
	{ shli r5, r6, 5 ; ld2u r25, r26 ; cmpne r15, r16, r17 }
	{ shli r5, r6, 5 ; ld4s r25, r26 ; and r15, r16, r17 }
	{ shli r5, r6, 5 ; ld4s r25, r26 ; subx r15, r16, r17 }
	{ shli r5, r6, 5 ; ld4u r25, r26 ; shl2addx r15, r16, r17 }
	{ shli r5, r6, 5 ; lnk r15 ; prefetch_l2 r25 }
	{ shli r5, r6, 5 ; move r15, r16 ; prefetch_l2 r25 }
	{ shli r5, r6, 5 ; mz r15, r16, r17 ; prefetch_l2 r25 }
	{ shli r5, r6, 5 ; nor r15, r16, r17 ; prefetch_l3 r25 }
	{ shli r5, r6, 5 ; prefetch r25 ; cmpltu r15, r16, r17 }
	{ shli r5, r6, 5 ; prefetch_add_l3_fault r15, 5 }
	{ shli r5, r6, 5 ; prefetch_l1 r25 ; shli r15, r16, 5 }
	{ shli r5, r6, 5 ; prefetch_l1_fault r25 ; rotli r15, r16, 5 }
	{ shli r5, r6, 5 ; prefetch_l2 r25 ; mnz r15, r16, r17 }
	{ shli r5, r6, 5 ; prefetch_l2_fault r25 ; fnop }
	{ shli r5, r6, 5 ; prefetch_l3 r25 ; cmpeq r15, r16, r17 }
	{ shli r5, r6, 5 ; prefetch_l3 r25 }
	{ shli r5, r6, 5 ; prefetch_l3_fault r25 ; shli r15, r16, 5 }
	{ shli r5, r6, 5 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ shli r5, r6, 5 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
	{ shli r5, r6, 5 ; shl2add r15, r16, r17 ; st r25, r26 }
	{ shli r5, r6, 5 ; shl3add r15, r16, r17 ; st2 r25, r26 }
	{ shli r5, r6, 5 ; shli r15, r16, 5 }
	{ shli r5, r6, 5 ; shrsi r15, r16, 5 }
	{ shli r5, r6, 5 ; shruxi r15, r16, 5 }
	{ shli r5, r6, 5 ; st r25, r26 ; shli r15, r16, 5 }
	{ shli r5, r6, 5 ; st1 r25, r26 ; rotli r15, r16, 5 }
	{ shli r5, r6, 5 ; st2 r25, r26 ; lnk r15 }
	{ shli r5, r6, 5 ; st4 r25, r26 ; cmpltu r15, r16, r17 }
	{ shli r5, r6, 5 ; stnt2 r15, r16 }
	{ shli r5, r6, 5 ; subx r15, r16, r17 ; st2 r25, r26 }
	{ shli r5, r6, 5 ; v2cmpltsi r15, r16, 5 }
	{ shli r5, r6, 5 ; xor r15, r16, r17 ; ld2u r25, r26 }
	{ shlx r15, r16, r17 ; cmul r5, r6, r7 }
	{ shlx r15, r16, r17 ; mul_hs_lu r5, r6, r7 }
	{ shlx r15, r16, r17 ; shrs r5, r6, r7 }
	{ shlx r15, r16, r17 ; v1maxu r5, r6, r7 }
	{ shlx r15, r16, r17 ; v2minsi r5, r6, 5 }
	{ shlx r5, r6, r7 ; addxli r15, r16, 0x1234 }
	{ shlx r5, r6, r7 ; jalrp r15 }
	{ shlx r5, r6, r7 ; mtspr 0x5, r16 }
	{ shlx r5, r6, r7 ; st1 r15, r16 }
	{ shlx r5, r6, r7 ; v1shrs r15, r16, r17 }
	{ shlx r5, r6, r7 ; v4int_h r15, r16, r17 }
	{ shlxi r15, r16, 5 ; cmulfr r5, r6, r7 }
	{ shlxi r15, r16, 5 ; mul_ls_ls r5, r6, r7 }
	{ shlxi r15, r16, 5 ; shrux r5, r6, r7 }
	{ shlxi r15, r16, 5 ; v1mnz r5, r6, r7 }
	{ shlxi r15, r16, 5 ; v2mults r5, r6, r7 }
	{ shlxi r5, r6, 5 ; cmpeq r15, r16, r17 }
	{ shlxi r5, r6, 5 ; ld1s r15, r16 }
	{ shlxi r5, r6, 5 ; or r15, r16, r17 }
	{ shlxi r5, r6, 5 ; st4 r15, r16 }
	{ shlxi r5, r6, 5 ; v1sub r15, r16, r17 }
	{ shlxi r5, r6, 5 ; v4shlsc r15, r16, r17 }
	{ shrs r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ shrs r15, r16, r17 ; addxi r5, r6, 5 ; st r25, r26 }
	{ shrs r15, r16, r17 ; andi r5, r6, 5 ; st r25, r26 }
	{ shrs r15, r16, r17 ; cmoveqz r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shrs r15, r16, r17 ; cmpeq r5, r6, r7 ; st1 r25, r26 }
	{ shrs r15, r16, r17 ; cmples r5, r6, r7 ; st4 r25, r26 }
	{ shrs r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld r25, r26 }
	{ shrs r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
	{ shrs r15, r16, r17 ; ctz r5, r6 ; prefetch_l3_fault r25 }
	{ shrs r15, r16, r17 ; fsingle_mul2 r5, r6, r7 }
	{ shrs r15, r16, r17 ; info 19 }
	{ shrs r15, r16, r17 ; ld r25, r26 ; pcnt r5, r6 }
	{ shrs r15, r16, r17 ; ld1s r25, r26 ; cmpltu r5, r6, r7 }
	{ shrs r15, r16, r17 ; ld1s r25, r26 ; sub r5, r6, r7 }
	{ shrs r15, r16, r17 ; ld1u r25, r26 ; mulax r5, r6, r7 }
	{ shrs r15, r16, r17 ; ld2s r25, r26 ; cmpeq r5, r6, r7 }
	{ shrs r15, r16, r17 ; ld2s r25, r26 ; shl3addx r5, r6, r7 }
	{ shrs r15, r16, r17 ; ld2u r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shrs r15, r16, r17 ; ld4s r25, r26 ; addxi r5, r6, 5 }
	{ shrs r15, r16, r17 ; ld4s r25, r26 ; shl r5, r6, r7 }
	{ shrs r15, r16, r17 ; ld4u r25, r26 ; info 19 }
	{ shrs r15, r16, r17 ; ld4u r25, r26 ; tblidxb3 r5, r6 }
	{ shrs r15, r16, r17 ; move r5, r6 ; st4 r25, r26 }
	{ shrs r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ shrs r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ shrs r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; st2 r25, r26 }
	{ shrs r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shrs r15, r16, r17 ; mulax r5, r6, r7 ; st r25, r26 }
	{ shrs r15, r16, r17 ; mz r5, r6, r7 ; st2 r25, r26 }
	{ shrs r15, r16, r17 ; nor r5, r6, r7 }
	{ shrs r15, r16, r17 ; prefetch r25 ; add r5, r6, r7 }
	{ shrs r15, r16, r17 ; prefetch r25 ; revbytes r5, r6 }
	{ shrs r15, r16, r17 ; prefetch_l1 r25 ; ctz r5, r6 }
	{ shrs r15, r16, r17 ; prefetch_l1 r25 ; tblidxb0 r5, r6 }
	{ shrs r15, r16, r17 ; prefetch_l1_fault r25 ; mz r5, r6, r7 }
	{ shrs r15, r16, r17 ; prefetch_l2 r25 ; cmples r5, r6, r7 }
	{ shrs r15, r16, r17 ; prefetch_l2 r25 ; shrs r5, r6, r7 }
	{ shrs r15, r16, r17 ; prefetch_l2_fault r25 ; mula_hs_hs r5, r6, r7 }
	{ shrs r15, r16, r17 ; prefetch_l3 r25 ; andi r5, r6, 5 }
	{ shrs r15, r16, r17 ; prefetch_l3 r25 ; shl1addx r5, r6, r7 }
	{ shrs r15, r16, r17 ; prefetch_l3_fault r25 ; move r5, r6 }
	{ shrs r15, r16, r17 ; prefetch_l3_fault r25 }
	{ shrs r15, r16, r17 ; rotl r5, r6, r7 ; ld1s r25, r26 }
	{ shrs r15, r16, r17 ; shl r5, r6, r7 ; ld2s r25, r26 }
	{ shrs r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2u r25, r26 }
	{ shrs r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
	{ shrs r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l1 r25 }
	{ shrs r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l1 r25 }
	{ shrs r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l2 r25 }
	{ shrs r15, r16, r17 ; st r25, r26 ; cmpeq r5, r6, r7 }
	{ shrs r15, r16, r17 ; st r25, r26 ; shl3addx r5, r6, r7 }
	{ shrs r15, r16, r17 ; st1 r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shrs r15, r16, r17 ; st2 r25, r26 ; addxi r5, r6, 5 }
	{ shrs r15, r16, r17 ; st2 r25, r26 ; shl r5, r6, r7 }
	{ shrs r15, r16, r17 ; st4 r25, r26 ; info 19 }
	{ shrs r15, r16, r17 ; st4 r25, r26 ; tblidxb3 r5, r6 }
	{ shrs r15, r16, r17 ; subx r5, r6, r7 }
	{ shrs r15, r16, r17 ; tblidxb2 r5, r6 ; ld r25, r26 }
	{ shrs r15, r16, r17 ; v1adduc r5, r6, r7 }
	{ shrs r15, r16, r17 ; v1shrui r5, r6, 5 }
	{ shrs r15, r16, r17 ; v2shrs r5, r6, r7 }
	{ shrs r5, r6, r7 ; add r15, r16, r17 ; ld2s r25, r26 }
	{ shrs r5, r6, r7 ; addx r15, r16, r17 ; ld2u r25, r26 }
	{ shrs r5, r6, r7 ; and r15, r16, r17 ; ld2u r25, r26 }
	{ shrs r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
	{ shrs r5, r6, r7 ; cmples r15, r16, r17 ; ld4u r25, r26 }
	{ shrs r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l1 r25 }
	{ shrs r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
	{ shrs r5, r6, r7 ; fetchand4 r15, r16, r17 }
	{ shrs r5, r6, r7 ; ill ; st r25, r26 }
	{ shrs r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
	{ shrs r5, r6, r7 ; jr r15 ; st1 r25, r26 }
	{ shrs r5, r6, r7 ; ld r25, r26 ; info 19 }
	{ shrs r5, r6, r7 ; ld1s r25, r26 ; cmples r15, r16, r17 }
	{ shrs r5, r6, r7 ; ld1u r15, r16 }
	{ shrs r5, r6, r7 ; ld1u r25, r26 ; shrs r15, r16, r17 }
	{ shrs r5, r6, r7 ; ld2s r25, r26 ; rotli r15, r16, 5 }
	{ shrs r5, r6, r7 ; ld2u r25, r26 ; lnk r15 }
	{ shrs r5, r6, r7 ; ld4s r25, r26 ; cmpltu r15, r16, r17 }
	{ shrs r5, r6, r7 ; ld4u r25, r26 ; addxi r15, r16, 5 }
	{ shrs r5, r6, r7 ; ld4u r25, r26 ; sub r15, r16, r17 }
	{ shrs r5, r6, r7 ; lnk r15 }
	{ shrs r5, r6, r7 ; move r15, r16 }
	{ shrs r5, r6, r7 ; mz r15, r16, r17 }
	{ shrs r5, r6, r7 ; or r15, r16, r17 ; ld1s r25, r26 }
	{ shrs r5, r6, r7 ; prefetch r25 ; jrp r15 }
	{ shrs r5, r6, r7 ; prefetch_l1 r25 ; cmpeq r15, r16, r17 }
	{ shrs r5, r6, r7 ; prefetch_l1 r25 }
	{ shrs r5, r6, r7 ; prefetch_l1_fault r25 ; shli r15, r16, 5 }
	{ shrs r5, r6, r7 ; prefetch_l2 r25 ; rotli r15, r16, 5 }
	{ shrs r5, r6, r7 ; prefetch_l2_fault r25 ; mnz r15, r16, r17 }
	{ shrs r5, r6, r7 ; prefetch_l3 r25 ; fnop }
	{ shrs r5, r6, r7 ; prefetch_l3_fault r25 ; cmpeq r15, r16, r17 }
	{ shrs r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shrs r5, r6, r7 ; shl r15, r16, r17 ; ld r25, r26 }
	{ shrs r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
	{ shrs r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
	{ shrs r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
	{ shrs r5, r6, r7 ; shrs r15, r16, r17 ; ld4s r25, r26 }
	{ shrs r5, r6, r7 ; shru r15, r16, r17 ; prefetch r25 }
	{ shrs r5, r6, r7 ; st r25, r26 ; cmpeq r15, r16, r17 }
	{ shrs r5, r6, r7 ; st r25, r26 }
	{ shrs r5, r6, r7 ; st1 r25, r26 ; shli r15, r16, 5 }
	{ shrs r5, r6, r7 ; st2 r25, r26 ; rotl r15, r16, r17 }
	{ shrs r5, r6, r7 ; st4 r25, r26 ; jrp r15 }
	{ shrs r5, r6, r7 ; sub r15, r16, r17 ; ld2s r25, r26 }
	{ shrs r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
	{ shrs r5, r6, r7 ; v2mins r15, r16, r17 }
	{ shrs r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3 r25 }
	{ shrsi r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ shrsi r15, r16, 5 ; addxi r5, r6, 5 ; st r25, r26 }
	{ shrsi r15, r16, 5 ; andi r5, r6, 5 ; st r25, r26 }
	{ shrsi r15, r16, 5 ; cmoveqz r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shrsi r15, r16, 5 ; cmpeq r5, r6, r7 ; st1 r25, r26 }
	{ shrsi r15, r16, 5 ; cmples r5, r6, r7 ; st4 r25, r26 }
	{ shrsi r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld r25, r26 }
	{ shrsi r15, r16, 5 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
	{ shrsi r15, r16, 5 ; ctz r5, r6 ; prefetch_l3_fault r25 }
	{ shrsi r15, r16, 5 ; fsingle_mul2 r5, r6, r7 }
	{ shrsi r15, r16, 5 ; info 19 }
	{ shrsi r15, r16, 5 ; ld r25, r26 ; pcnt r5, r6 }
	{ shrsi r15, r16, 5 ; ld1s r25, r26 ; cmpltu r5, r6, r7 }
	{ shrsi r15, r16, 5 ; ld1s r25, r26 ; sub r5, r6, r7 }
	{ shrsi r15, r16, 5 ; ld1u r25, r26 ; mulax r5, r6, r7 }
	{ shrsi r15, r16, 5 ; ld2s r25, r26 ; cmpeq r5, r6, r7 }
	{ shrsi r15, r16, 5 ; ld2s r25, r26 ; shl3addx r5, r6, r7 }
	{ shrsi r15, r16, 5 ; ld2u r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shrsi r15, r16, 5 ; ld4s r25, r26 ; addxi r5, r6, 5 }
	{ shrsi r15, r16, 5 ; ld4s r25, r26 ; shl r5, r6, r7 }
	{ shrsi r15, r16, 5 ; ld4u r25, r26 ; info 19 }
	{ shrsi r15, r16, 5 ; ld4u r25, r26 ; tblidxb3 r5, r6 }
	{ shrsi r15, r16, 5 ; move r5, r6 ; st4 r25, r26 }
	{ shrsi r15, r16, 5 ; mul_hs_hs r5, r6, r7 }
	{ shrsi r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ shrsi r15, r16, 5 ; mula_hs_hs r5, r6, r7 ; st2 r25, r26 }
	{ shrsi r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shrsi r15, r16, 5 ; mulax r5, r6, r7 ; st r25, r26 }
	{ shrsi r15, r16, 5 ; mz r5, r6, r7 ; st2 r25, r26 }
	{ shrsi r15, r16, 5 ; nor r5, r6, r7 }
	{ shrsi r15, r16, 5 ; prefetch r25 ; add r5, r6, r7 }
	{ shrsi r15, r16, 5 ; prefetch r25 ; revbytes r5, r6 }
	{ shrsi r15, r16, 5 ; prefetch_l1 r25 ; ctz r5, r6 }
	{ shrsi r15, r16, 5 ; prefetch_l1 r25 ; tblidxb0 r5, r6 }
	{ shrsi r15, r16, 5 ; prefetch_l1_fault r25 ; mz r5, r6, r7 }
	{ shrsi r15, r16, 5 ; prefetch_l2 r25 ; cmples r5, r6, r7 }
	{ shrsi r15, r16, 5 ; prefetch_l2 r25 ; shrs r5, r6, r7 }
	{ shrsi r15, r16, 5 ; prefetch_l2_fault r25 ; mula_hs_hs r5, r6, r7 }
	{ shrsi r15, r16, 5 ; prefetch_l3 r25 ; andi r5, r6, 5 }
	{ shrsi r15, r16, 5 ; prefetch_l3 r25 ; shl1addx r5, r6, r7 }
	{ shrsi r15, r16, 5 ; prefetch_l3_fault r25 ; move r5, r6 }
	{ shrsi r15, r16, 5 ; prefetch_l3_fault r25 }
	{ shrsi r15, r16, 5 ; rotl r5, r6, r7 ; ld1s r25, r26 }
	{ shrsi r15, r16, 5 ; shl r5, r6, r7 ; ld2s r25, r26 }
	{ shrsi r15, r16, 5 ; shl1addx r5, r6, r7 ; ld2u r25, r26 }
	{ shrsi r15, r16, 5 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
	{ shrsi r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l1 r25 }
	{ shrsi r15, r16, 5 ; shrs r5, r6, r7 ; prefetch_l1 r25 }
	{ shrsi r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l2 r25 }
	{ shrsi r15, r16, 5 ; st r25, r26 ; cmpeq r5, r6, r7 }
	{ shrsi r15, r16, 5 ; st r25, r26 ; shl3addx r5, r6, r7 }
	{ shrsi r15, r16, 5 ; st1 r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shrsi r15, r16, 5 ; st2 r25, r26 ; addxi r5, r6, 5 }
	{ shrsi r15, r16, 5 ; st2 r25, r26 ; shl r5, r6, r7 }
	{ shrsi r15, r16, 5 ; st4 r25, r26 ; info 19 }
	{ shrsi r15, r16, 5 ; st4 r25, r26 ; tblidxb3 r5, r6 }
	{ shrsi r15, r16, 5 ; subx r5, r6, r7 }
	{ shrsi r15, r16, 5 ; tblidxb2 r5, r6 ; ld r25, r26 }
	{ shrsi r15, r16, 5 ; v1adduc r5, r6, r7 }
	{ shrsi r15, r16, 5 ; v1shrui r5, r6, 5 }
	{ shrsi r15, r16, 5 ; v2shrs r5, r6, r7 }
	{ shrsi r5, r6, 5 ; add r15, r16, r17 ; ld2s r25, r26 }
	{ shrsi r5, r6, 5 ; addx r15, r16, r17 ; ld2u r25, r26 }
	{ shrsi r5, r6, 5 ; and r15, r16, r17 ; ld2u r25, r26 }
	{ shrsi r5, r6, 5 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
	{ shrsi r5, r6, 5 ; cmples r15, r16, r17 ; ld4u r25, r26 }
	{ shrsi r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch_l1 r25 }
	{ shrsi r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
	{ shrsi r5, r6, 5 ; fetchand4 r15, r16, r17 }
	{ shrsi r5, r6, 5 ; ill ; st r25, r26 }
	{ shrsi r5, r6, 5 ; jalr r15 ; prefetch_l3_fault r25 }
	{ shrsi r5, r6, 5 ; jr r15 ; st1 r25, r26 }
	{ shrsi r5, r6, 5 ; ld r25, r26 ; info 19 }
	{ shrsi r5, r6, 5 ; ld1s r25, r26 ; cmples r15, r16, r17 }
	{ shrsi r5, r6, 5 ; ld1u r15, r16 }
	{ shrsi r5, r6, 5 ; ld1u r25, r26 ; shrs r15, r16, r17 }
	{ shrsi r5, r6, 5 ; ld2s r25, r26 ; rotli r15, r16, 5 }
	{ shrsi r5, r6, 5 ; ld2u r25, r26 ; lnk r15 }
	{ shrsi r5, r6, 5 ; ld4s r25, r26 ; cmpltu r15, r16, r17 }
	{ shrsi r5, r6, 5 ; ld4u r25, r26 ; addxi r15, r16, 5 }
	{ shrsi r5, r6, 5 ; ld4u r25, r26 ; sub r15, r16, r17 }
	{ shrsi r5, r6, 5 ; lnk r15 }
	{ shrsi r5, r6, 5 ; move r15, r16 }
	{ shrsi r5, r6, 5 ; mz r15, r16, r17 }
	{ shrsi r5, r6, 5 ; or r15, r16, r17 ; ld1s r25, r26 }
	{ shrsi r5, r6, 5 ; prefetch r25 ; jrp r15 }
	{ shrsi r5, r6, 5 ; prefetch_l1 r25 ; cmpeq r15, r16, r17 }
	{ shrsi r5, r6, 5 ; prefetch_l1 r25 }
	{ shrsi r5, r6, 5 ; prefetch_l1_fault r25 ; shli r15, r16, 5 }
	{ shrsi r5, r6, 5 ; prefetch_l2 r25 ; rotli r15, r16, 5 }
	{ shrsi r5, r6, 5 ; prefetch_l2_fault r25 ; mnz r15, r16, r17 }
	{ shrsi r5, r6, 5 ; prefetch_l3 r25 ; fnop }
	{ shrsi r5, r6, 5 ; prefetch_l3_fault r25 ; cmpeq r15, r16, r17 }
	{ shrsi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ shrsi r5, r6, 5 ; shl r15, r16, r17 ; ld r25, r26 }
	{ shrsi r5, r6, 5 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
	{ shrsi r5, r6, 5 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
	{ shrsi r5, r6, 5 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
	{ shrsi r5, r6, 5 ; shrs r15, r16, r17 ; ld4s r25, r26 }
	{ shrsi r5, r6, 5 ; shru r15, r16, r17 ; prefetch r25 }
	{ shrsi r5, r6, 5 ; st r25, r26 ; cmpeq r15, r16, r17 }
	{ shrsi r5, r6, 5 ; st r25, r26 }
	{ shrsi r5, r6, 5 ; st1 r25, r26 ; shli r15, r16, 5 }
	{ shrsi r5, r6, 5 ; st2 r25, r26 ; rotl r15, r16, r17 }
	{ shrsi r5, r6, 5 ; st4 r25, r26 ; jrp r15 }
	{ shrsi r5, r6, 5 ; sub r15, r16, r17 ; ld2s r25, r26 }
	{ shrsi r5, r6, 5 ; v1cmpeqi r15, r16, 5 }
	{ shrsi r5, r6, 5 ; v2mins r15, r16, r17 }
	{ shrsi r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l3 r25 }
	{ shru r15, r16, r17 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ shru r15, r16, r17 ; addxi r5, r6, 5 ; st r25, r26 }
	{ shru r15, r16, r17 ; andi r5, r6, 5 ; st r25, r26 }
	{ shru r15, r16, r17 ; cmoveqz r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shru r15, r16, r17 ; cmpeq r5, r6, r7 ; st1 r25, r26 }
	{ shru r15, r16, r17 ; cmples r5, r6, r7 ; st4 r25, r26 }
	{ shru r15, r16, r17 ; cmpltsi r5, r6, 5 ; ld r25, r26 }
	{ shru r15, r16, r17 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
	{ shru r15, r16, r17 ; ctz r5, r6 ; prefetch_l3_fault r25 }
	{ shru r15, r16, r17 ; fsingle_mul2 r5, r6, r7 }
	{ shru r15, r16, r17 ; info 19 }
	{ shru r15, r16, r17 ; ld r25, r26 ; pcnt r5, r6 }
	{ shru r15, r16, r17 ; ld1s r25, r26 ; cmpltu r5, r6, r7 }
	{ shru r15, r16, r17 ; ld1s r25, r26 ; sub r5, r6, r7 }
	{ shru r15, r16, r17 ; ld1u r25, r26 ; mulax r5, r6, r7 }
	{ shru r15, r16, r17 ; ld2s r25, r26 ; cmpeq r5, r6, r7 }
	{ shru r15, r16, r17 ; ld2s r25, r26 ; shl3addx r5, r6, r7 }
	{ shru r15, r16, r17 ; ld2u r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shru r15, r16, r17 ; ld4s r25, r26 ; addxi r5, r6, 5 }
	{ shru r15, r16, r17 ; ld4s r25, r26 ; shl r5, r6, r7 }
	{ shru r15, r16, r17 ; ld4u r25, r26 ; info 19 }
	{ shru r15, r16, r17 ; ld4u r25, r26 ; tblidxb3 r5, r6 }
	{ shru r15, r16, r17 ; move r5, r6 ; st4 r25, r26 }
	{ shru r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ shru r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ shru r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; st2 r25, r26 }
	{ shru r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shru r15, r16, r17 ; mulax r5, r6, r7 ; st r25, r26 }
	{ shru r15, r16, r17 ; mz r5, r6, r7 ; st2 r25, r26 }
	{ shru r15, r16, r17 ; nor r5, r6, r7 }
	{ shru r15, r16, r17 ; prefetch r25 ; add r5, r6, r7 }
	{ shru r15, r16, r17 ; prefetch r25 ; revbytes r5, r6 }
	{ shru r15, r16, r17 ; prefetch_l1 r25 ; ctz r5, r6 }
	{ shru r15, r16, r17 ; prefetch_l1 r25 ; tblidxb0 r5, r6 }
	{ shru r15, r16, r17 ; prefetch_l1_fault r25 ; mz r5, r6, r7 }
	{ shru r15, r16, r17 ; prefetch_l2 r25 ; cmples r5, r6, r7 }
	{ shru r15, r16, r17 ; prefetch_l2 r25 ; shrs r5, r6, r7 }
	{ shru r15, r16, r17 ; prefetch_l2_fault r25 ; mula_hs_hs r5, r6, r7 }
	{ shru r15, r16, r17 ; prefetch_l3 r25 ; andi r5, r6, 5 }
	{ shru r15, r16, r17 ; prefetch_l3 r25 ; shl1addx r5, r6, r7 }
	{ shru r15, r16, r17 ; prefetch_l3_fault r25 ; move r5, r6 }
	{ shru r15, r16, r17 ; prefetch_l3_fault r25 }
	{ shru r15, r16, r17 ; rotl r5, r6, r7 ; ld1s r25, r26 }
	{ shru r15, r16, r17 ; shl r5, r6, r7 ; ld2s r25, r26 }
	{ shru r15, r16, r17 ; shl1addx r5, r6, r7 ; ld2u r25, r26 }
	{ shru r15, r16, r17 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
	{ shru r15, r16, r17 ; shl3addx r5, r6, r7 ; prefetch_l1 r25 }
	{ shru r15, r16, r17 ; shrs r5, r6, r7 ; prefetch_l1 r25 }
	{ shru r15, r16, r17 ; shru r5, r6, r7 ; prefetch_l2 r25 }
	{ shru r15, r16, r17 ; st r25, r26 ; cmpeq r5, r6, r7 }
	{ shru r15, r16, r17 ; st r25, r26 ; shl3addx r5, r6, r7 }
	{ shru r15, r16, r17 ; st1 r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shru r15, r16, r17 ; st2 r25, r26 ; addxi r5, r6, 5 }
	{ shru r15, r16, r17 ; st2 r25, r26 ; shl r5, r6, r7 }
	{ shru r15, r16, r17 ; st4 r25, r26 ; info 19 }
	{ shru r15, r16, r17 ; st4 r25, r26 ; tblidxb3 r5, r6 }
	{ shru r15, r16, r17 ; subx r5, r6, r7 }
	{ shru r15, r16, r17 ; tblidxb2 r5, r6 ; ld r25, r26 }
	{ shru r15, r16, r17 ; v1adduc r5, r6, r7 }
	{ shru r15, r16, r17 ; v1shrui r5, r6, 5 }
	{ shru r15, r16, r17 ; v2shrs r5, r6, r7 }
	{ shru r5, r6, r7 ; add r15, r16, r17 ; ld2s r25, r26 }
	{ shru r5, r6, r7 ; addx r15, r16, r17 ; ld2u r25, r26 }
	{ shru r5, r6, r7 ; and r15, r16, r17 ; ld2u r25, r26 }
	{ shru r5, r6, r7 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
	{ shru r5, r6, r7 ; cmples r15, r16, r17 ; ld4u r25, r26 }
	{ shru r5, r6, r7 ; cmplts r15, r16, r17 ; prefetch_l1 r25 }
	{ shru r5, r6, r7 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
	{ shru r5, r6, r7 ; fetchand4 r15, r16, r17 }
	{ shru r5, r6, r7 ; ill ; st r25, r26 }
	{ shru r5, r6, r7 ; jalr r15 ; prefetch_l3_fault r25 }
	{ shru r5, r6, r7 ; jr r15 ; st1 r25, r26 }
	{ shru r5, r6, r7 ; ld r25, r26 ; info 19 }
	{ shru r5, r6, r7 ; ld1s r25, r26 ; cmples r15, r16, r17 }
	{ shru r5, r6, r7 ; ld1u r15, r16 }
	{ shru r5, r6, r7 ; ld1u r25, r26 ; shrs r15, r16, r17 }
	{ shru r5, r6, r7 ; ld2s r25, r26 ; rotli r15, r16, 5 }
	{ shru r5, r6, r7 ; ld2u r25, r26 ; lnk r15 }
	{ shru r5, r6, r7 ; ld4s r25, r26 ; cmpltu r15, r16, r17 }
	{ shru r5, r6, r7 ; ld4u r25, r26 ; addxi r15, r16, 5 }
	{ shru r5, r6, r7 ; ld4u r25, r26 ; sub r15, r16, r17 }
	{ shru r5, r6, r7 ; lnk r15 }
	{ shru r5, r6, r7 ; move r15, r16 }
	{ shru r5, r6, r7 ; mz r15, r16, r17 }
	{ shru r5, r6, r7 ; or r15, r16, r17 ; ld1s r25, r26 }
	{ shru r5, r6, r7 ; prefetch r25 ; jrp r15 }
	{ shru r5, r6, r7 ; prefetch_l1 r25 ; cmpeq r15, r16, r17 }
	{ shru r5, r6, r7 ; prefetch_l1 r25 }
	{ shru r5, r6, r7 ; prefetch_l1_fault r25 ; shli r15, r16, 5 }
	{ shru r5, r6, r7 ; prefetch_l2 r25 ; rotli r15, r16, 5 }
	{ shru r5, r6, r7 ; prefetch_l2_fault r25 ; mnz r15, r16, r17 }
	{ shru r5, r6, r7 ; prefetch_l3 r25 ; fnop }
	{ shru r5, r6, r7 ; prefetch_l3_fault r25 ; cmpeq r15, r16, r17 }
	{ shru r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shru r5, r6, r7 ; shl r15, r16, r17 ; ld r25, r26 }
	{ shru r5, r6, r7 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
	{ shru r5, r6, r7 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
	{ shru r5, r6, r7 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
	{ shru r5, r6, r7 ; shrs r15, r16, r17 ; ld4s r25, r26 }
	{ shru r5, r6, r7 ; shru r15, r16, r17 ; prefetch r25 }
	{ shru r5, r6, r7 ; st r25, r26 ; cmpeq r15, r16, r17 }
	{ shru r5, r6, r7 ; st r25, r26 }
	{ shru r5, r6, r7 ; st1 r25, r26 ; shli r15, r16, 5 }
	{ shru r5, r6, r7 ; st2 r25, r26 ; rotl r15, r16, r17 }
	{ shru r5, r6, r7 ; st4 r25, r26 ; jrp r15 }
	{ shru r5, r6, r7 ; sub r15, r16, r17 ; ld2s r25, r26 }
	{ shru r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
	{ shru r5, r6, r7 ; v2mins r15, r16, r17 }
	{ shru r5, r6, r7 ; xor r15, r16, r17 ; prefetch_l3 r25 }
	{ shrui r15, r16, 5 ; addi r5, r6, 5 ; prefetch_l3_fault r25 }
	{ shrui r15, r16, 5 ; addxi r5, r6, 5 ; st r25, r26 }
	{ shrui r15, r16, 5 ; andi r5, r6, 5 ; st r25, r26 }
	{ shrui r15, r16, 5 ; cmoveqz r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shrui r15, r16, 5 ; cmpeq r5, r6, r7 ; st1 r25, r26 }
	{ shrui r15, r16, 5 ; cmples r5, r6, r7 ; st4 r25, r26 }
	{ shrui r15, r16, 5 ; cmpltsi r5, r6, 5 ; ld r25, r26 }
	{ shrui r15, r16, 5 ; cmpne r5, r6, r7 ; ld1s r25, r26 }
	{ shrui r15, r16, 5 ; ctz r5, r6 ; prefetch_l3_fault r25 }
	{ shrui r15, r16, 5 ; fsingle_mul2 r5, r6, r7 }
	{ shrui r15, r16, 5 ; info 19 }
	{ shrui r15, r16, 5 ; ld r25, r26 ; pcnt r5, r6 }
	{ shrui r15, r16, 5 ; ld1s r25, r26 ; cmpltu r5, r6, r7 }
	{ shrui r15, r16, 5 ; ld1s r25, r26 ; sub r5, r6, r7 }
	{ shrui r15, r16, 5 ; ld1u r25, r26 ; mulax r5, r6, r7 }
	{ shrui r15, r16, 5 ; ld2s r25, r26 ; cmpeq r5, r6, r7 }
	{ shrui r15, r16, 5 ; ld2s r25, r26 ; shl3addx r5, r6, r7 }
	{ shrui r15, r16, 5 ; ld2u r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shrui r15, r16, 5 ; ld4s r25, r26 ; addxi r5, r6, 5 }
	{ shrui r15, r16, 5 ; ld4s r25, r26 ; shl r5, r6, r7 }
	{ shrui r15, r16, 5 ; ld4u r25, r26 ; info 19 }
	{ shrui r15, r16, 5 ; ld4u r25, r26 ; tblidxb3 r5, r6 }
	{ shrui r15, r16, 5 ; move r5, r6 ; st4 r25, r26 }
	{ shrui r15, r16, 5 ; mul_hs_hs r5, r6, r7 }
	{ shrui r15, r16, 5 ; mul_ls_ls r5, r6, r7 ; st1 r25, r26 }
	{ shrui r15, r16, 5 ; mula_hs_hs r5, r6, r7 ; st2 r25, r26 }
	{ shrui r15, r16, 5 ; mula_ls_ls r5, r6, r7 ; prefetch_l3_fault r25 }
	{ shrui r15, r16, 5 ; mulax r5, r6, r7 ; st r25, r26 }
	{ shrui r15, r16, 5 ; mz r5, r6, r7 ; st2 r25, r26 }
	{ shrui r15, r16, 5 ; nor r5, r6, r7 }
	{ shrui r15, r16, 5 ; prefetch r25 ; add r5, r6, r7 }
	{ shrui r15, r16, 5 ; prefetch r25 ; revbytes r5, r6 }
	{ shrui r15, r16, 5 ; prefetch_l1 r25 ; ctz r5, r6 }
	{ shrui r15, r16, 5 ; prefetch_l1 r25 ; tblidxb0 r5, r6 }
	{ shrui r15, r16, 5 ; prefetch_l1_fault r25 ; mz r5, r6, r7 }
	{ shrui r15, r16, 5 ; prefetch_l2 r25 ; cmples r5, r6, r7 }
	{ shrui r15, r16, 5 ; prefetch_l2 r25 ; shrs r5, r6, r7 }
	{ shrui r15, r16, 5 ; prefetch_l2_fault r25 ; mula_hs_hs r5, r6, r7 }
	{ shrui r15, r16, 5 ; prefetch_l3 r25 ; andi r5, r6, 5 }
	{ shrui r15, r16, 5 ; prefetch_l3 r25 ; shl1addx r5, r6, r7 }
	{ shrui r15, r16, 5 ; prefetch_l3_fault r25 ; move r5, r6 }
	{ shrui r15, r16, 5 ; prefetch_l3_fault r25 }
	{ shrui r15, r16, 5 ; rotl r5, r6, r7 ; ld1s r25, r26 }
	{ shrui r15, r16, 5 ; shl r5, r6, r7 ; ld2s r25, r26 }
	{ shrui r15, r16, 5 ; shl1addx r5, r6, r7 ; ld2u r25, r26 }
	{ shrui r15, r16, 5 ; shl2addx r5, r6, r7 ; ld4u r25, r26 }
	{ shrui r15, r16, 5 ; shl3addx r5, r6, r7 ; prefetch_l1 r25 }
	{ shrui r15, r16, 5 ; shrs r5, r6, r7 ; prefetch_l1 r25 }
	{ shrui r15, r16, 5 ; shru r5, r6, r7 ; prefetch_l2 r25 }
	{ shrui r15, r16, 5 ; st r25, r26 ; cmpeq r5, r6, r7 }
	{ shrui r15, r16, 5 ; st r25, r26 ; shl3addx r5, r6, r7 }
	{ shrui r15, r16, 5 ; st1 r25, r26 ; mul_ls_ls r5, r6, r7 }
	{ shrui r15, r16, 5 ; st2 r25, r26 ; addxi r5, r6, 5 }
	{ shrui r15, r16, 5 ; st2 r25, r26 ; shl r5, r6, r7 }
	{ shrui r15, r16, 5 ; st4 r25, r26 ; info 19 }
	{ shrui r15, r16, 5 ; st4 r25, r26 ; tblidxb3 r5, r6 }
	{ shrui r15, r16, 5 ; subx r5, r6, r7 }
	{ shrui r15, r16, 5 ; tblidxb2 r5, r6 ; ld r25, r26 }
	{ shrui r15, r16, 5 ; v1adduc r5, r6, r7 }
	{ shrui r15, r16, 5 ; v1shrui r5, r6, 5 }
	{ shrui r15, r16, 5 ; v2shrs r5, r6, r7 }
	{ shrui r5, r6, 5 ; add r15, r16, r17 ; ld2s r25, r26 }
	{ shrui r5, r6, 5 ; addx r15, r16, r17 ; ld2u r25, r26 }
	{ shrui r5, r6, 5 ; and r15, r16, r17 ; ld2u r25, r26 }
	{ shrui r5, r6, 5 ; cmpeq r15, r16, r17 ; ld4u r25, r26 }
	{ shrui r5, r6, 5 ; cmples r15, r16, r17 ; ld4u r25, r26 }
	{ shrui r5, r6, 5 ; cmplts r15, r16, r17 ; prefetch_l1 r25 }
	{ shrui r5, r6, 5 ; cmpltu r15, r16, r17 ; prefetch_l2 r25 }
	{ shrui r5, r6, 5 ; fetchand4 r15, r16, r17 }
	{ shrui r5, r6, 5 ; ill ; st r25, r26 }
	{ shrui r5, r6, 5 ; jalr r15 ; prefetch_l3_fault r25 }
	{ shrui r5, r6, 5 ; jr r15 ; st1 r25, r26 }
	{ shrui r5, r6, 5 ; ld r25, r26 ; info 19 }
	{ shrui r5, r6, 5 ; ld1s r25, r26 ; cmples r15, r16, r17 }
	{ shrui r5, r6, 5 ; ld1u r15, r16 }
	{ shrui r5, r6, 5 ; ld1u r25, r26 ; shrs r15, r16, r17 }
	{ shrui r5, r6, 5 ; ld2s r25, r26 ; rotli r15, r16, 5 }
	{ shrui r5, r6, 5 ; ld2u r25, r26 ; lnk r15 }
	{ shrui r5, r6, 5 ; ld4s r25, r26 ; cmpltu r15, r16, r17 }
	{ shrui r5, r6, 5 ; ld4u r25, r26 ; addxi r15, r16, 5 }
	{ shrui r5, r6, 5 ; ld4u r25, r26 ; sub r15, r16, r17 }
	{ shrui r5, r6, 5 ; lnk r15 }
	{ shrui r5, r6, 5 ; move r15, r16 }
	{ shrui r5, r6, 5 ; mz r15, r16, r17 }
	{ shrui r5, r6, 5 ; or r15, r16, r17 ; ld1s r25, r26 }
	{ shrui r5, r6, 5 ; prefetch r25 ; jrp r15 }
	{ shrui r5, r6, 5 ; prefetch_l1 r25 ; cmpeq r15, r16, r17 }
	{ shrui r5, r6, 5 ; prefetch_l1 r25 }
	{ shrui r5, r6, 5 ; prefetch_l1_fault r25 ; shli r15, r16, 5 }
	{ shrui r5, r6, 5 ; prefetch_l2 r25 ; rotli r15, r16, 5 }
	{ shrui r5, r6, 5 ; prefetch_l2_fault r25 ; mnz r15, r16, r17 }
	{ shrui r5, r6, 5 ; prefetch_l3 r25 ; fnop }
	{ shrui r5, r6, 5 ; prefetch_l3_fault r25 ; cmpeq r15, r16, r17 }
	{ shrui r5, r6, 5 ; prefetch_l3_fault r25 }
	{ shrui r5, r6, 5 ; shl r15, r16, r17 ; ld r25, r26 }
	{ shrui r5, r6, 5 ; shl1addx r15, r16, r17 ; ld1s r25, r26 }
	{ shrui r5, r6, 5 ; shl2addx r15, r16, r17 ; ld2s r25, r26 }
	{ shrui r5, r6, 5 ; shl3addx r15, r16, r17 ; ld4s r25, r26 }
	{ shrui r5, r6, 5 ; shrs r15, r16, r17 ; ld4s r25, r26 }
	{ shrui r5, r6, 5 ; shru r15, r16, r17 ; prefetch r25 }
	{ shrui r5, r6, 5 ; st r25, r26 ; cmpeq r15, r16, r17 }
	{ shrui r5, r6, 5 ; st r25, r26 }
	{ shrui r5, r6, 5 ; st1 r25, r26 ; shli r15, r16, 5 }
	{ shrui r5, r6, 5 ; st2 r25, r26 ; rotl r15, r16, r17 }
	{ shrui r5, r6, 5 ; st4 r25, r26 ; jrp r15 }
	{ shrui r5, r6, 5 ; sub r15, r16, r17 ; ld2s r25, r26 }
	{ shrui r5, r6, 5 ; v1cmpeqi r15, r16, 5 }
	{ shrui r5, r6, 5 ; v2mins r15, r16, r17 }
	{ shrui r5, r6, 5 ; xor r15, r16, r17 ; prefetch_l3 r25 }
	{ shrux r15, r16, r17 ; crc32_8 r5, r6, r7 }
	{ shrux r15, r16, r17 ; mula_hs_hu r5, r6, r7 }
	{ shrux r15, r16, r17 ; subx r5, r6, r7 }
	{ shrux r15, r16, r17 ; v1mz r5, r6, r7 }
	{ shrux r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ shrux r5, r6, r7 ; cmples r15, r16, r17 }
	{ shrux r5, r6, r7 ; ld2s r15, r16 }
	{ shrux r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
	{ shrux r5, r6, r7 ; stnt1 r15, r16 }
	{ shrux r5, r6, r7 ; v2addsc r15, r16, r17 }
	{ shrux r5, r6, r7 ; v4subsc r15, r16, r17 }
	{ shruxi r15, r16, 5 ; dblalign4 r5, r6, r7 }
	{ shruxi r15, r16, 5 ; mula_hu_ls r5, r6, r7 }
	{ shruxi r15, r16, 5 ; tblidxb2 r5, r6 }
	{ shruxi r15, r16, 5 ; v1shli r5, r6, 5 }
	{ shruxi r15, r16, 5 ; v2sadu r5, r6, r7 }
	{ shruxi r5, r6, 5 ; cmpltu r15, r16, r17 }
	{ shruxi r5, r6, 5 ; ld4s r15, r16 }
	{ shruxi r5, r6, 5 ; prefetch_add_l3_fault r15, 5 }
	{ shruxi r5, r6, 5 ; stnt4 r15, r16 }
	{ shruxi r5, r6, 5 ; v2cmpleu r15, r16, r17 }
	{ shufflebytes r5, r6, r7 ; add r15, r16, r17 }
	{ shufflebytes r5, r6, r7 ; info 19 }
	{ shufflebytes r5, r6, r7 ; mfspr r16, 0x5 }
	{ shufflebytes r5, r6, r7 ; shru r15, r16, r17 }
	{ shufflebytes r5, r6, r7 ; v1minui r15, r16, 5 }
	{ shufflebytes r5, r6, r7 ; v2shrui r15, r16, 5 }
	{ st r15, r16 ; cmpne r5, r6, r7 }
	{ st r15, r16 ; mul_hs_ls r5, r6, r7 }
	{ st r15, r16 ; shlxi r5, r6, 5 }
	{ st r15, r16 ; v1int_l r5, r6, r7 }
	{ st r15, r16 ; v2mins r5, r6, r7 }
	{ st r25, r26 ; add r15, r16, r17 ; and r5, r6, r7 }
	{ st r25, r26 ; add r15, r16, r17 ; shl1add r5, r6, r7 }
	{ st r25, r26 ; add r5, r6, r7 ; lnk r15 }
	{ st r25, r26 ; addi r15, r16, 5 ; cmpltsi r5, r6, 5 }
	{ st r25, r26 ; addi r15, r16, 5 ; shrui r5, r6, 5 }
	{ st r25, r26 ; addi r5, r6, 5 ; shl r15, r16, r17 }
	{ st r25, r26 ; addx r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ st r25, r26 ; addx r5, r6, r7 ; addi r15, r16, 5 }
	{ st r25, r26 ; addx r5, r6, r7 ; shru r15, r16, r17 }
	{ st r25, r26 ; addxi r15, r16, 5 ; mz r5, r6, r7 }
	{ st r25, r26 ; addxi r5, r6, 5 ; cmpltsi r15, r16, 5 }
	{ st r25, r26 ; and r15, r16, r17 ; and r5, r6, r7 }
	{ st r25, r26 ; and r15, r16, r17 ; shl1add r5, r6, r7 }
	{ st r25, r26 ; and r5, r6, r7 ; lnk r15 }
	{ st r25, r26 ; andi r15, r16, 5 ; cmpltsi r5, r6, 5 }
	{ st r25, r26 ; andi r15, r16, 5 ; shrui r5, r6, 5 }
	{ st r25, r26 ; andi r5, r6, 5 ; shl r15, r16, r17 }
	{ st r25, r26 ; clz r5, r6 ; movei r15, 5 }
	{ st r25, r26 ; cmoveqz r5, r6, r7 ; jalr r15 }
	{ st r25, r26 ; cmovnez r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st r25, r26 ; cmpeq r15, r16, r17 ; addxi r5, r6, 5 }
	{ st r25, r26 ; cmpeq r15, r16, r17 ; shl r5, r6, r7 }
	{ st r25, r26 ; cmpeq r5, r6, r7 ; jrp r15 }
	{ st r25, r26 ; cmpeqi r15, r16, 5 ; cmplts r5, r6, r7 }
	{ st r25, r26 ; cmpeqi r15, r16, 5 ; shru r5, r6, r7 }
	{ st r25, r26 ; cmpeqi r5, r6, 5 ; rotli r15, r16, 5 }
	{ st r25, r26 ; cmples r15, r16, r17 ; movei r5, 5 }
	{ st r25, r26 ; cmples r5, r6, r7 ; add r15, r16, r17 }
	{ st r25, r26 ; cmples r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st r25, r26 ; cmpleu r15, r16, r17 ; mulx r5, r6, r7 }
	{ st r25, r26 ; cmpleu r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st r25, r26 ; cmplts r15, r16, r17 ; addxi r5, r6, 5 }
	{ st r25, r26 ; cmplts r15, r16, r17 ; shl r5, r6, r7 }
	{ st r25, r26 ; cmplts r5, r6, r7 ; jrp r15 }
	{ st r25, r26 ; cmpltsi r15, r16, 5 ; cmplts r5, r6, r7 }
	{ st r25, r26 ; cmpltsi r15, r16, 5 ; shru r5, r6, r7 }
	{ st r25, r26 ; cmpltsi r5, r6, 5 ; rotli r15, r16, 5 }
	{ st r25, r26 ; cmpltu r15, r16, r17 ; movei r5, 5 }
	{ st r25, r26 ; cmpltu r5, r6, r7 ; add r15, r16, r17 }
	{ st r25, r26 ; cmpltu r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st r25, r26 ; cmpne r15, r16, r17 ; mulx r5, r6, r7 }
	{ st r25, r26 ; cmpne r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st r25, r26 ; ctz r5, r6 ; addxi r15, r16, 5 }
	{ st r25, r26 ; ctz r5, r6 ; sub r15, r16, r17 }
	{ st r25, r26 ; fnop ; jalr r15 }
	{ st r25, r26 ; fnop ; shl1addx r5, r6, r7 }
	{ st r25, r26 ; fsingle_pack1 r5, r6 ; cmplts r15, r16, r17 }
	{ st r25, r26 ; ill ; addxi r5, r6, 5 }
	{ st r25, r26 ; ill ; shl r5, r6, r7 }
	{ st r25, r26 ; info 19 ; cmples r5, r6, r7 }
	{ st r25, r26 ; info 19 ; nor r15, r16, r17 }
	{ st r25, r26 ; info 19 ; tblidxb1 r5, r6 }
	{ st r25, r26 ; jalr r15 ; mz r5, r6, r7 }
	{ st r25, r26 ; jalrp r15 ; cmples r5, r6, r7 }
	{ st r25, r26 ; jalrp r15 ; shrs r5, r6, r7 }
	{ st r25, r26 ; jr r15 ; mula_hs_hs r5, r6, r7 }
	{ st r25, r26 ; jrp r15 ; andi r5, r6, 5 }
	{ st r25, r26 ; jrp r15 ; shl1addx r5, r6, r7 }
	{ st r25, r26 ; lnk r15 ; move r5, r6 }
	{ st r25, r26 ; lnk r15 }
	{ st r25, r26 ; mnz r15, r16, r17 ; revbits r5, r6 }
	{ st r25, r26 ; mnz r5, r6, r7 ; info 19 }
	{ st r25, r26 ; move r15, r16 ; cmpeq r5, r6, r7 }
	{ st r25, r26 ; move r15, r16 ; shl3addx r5, r6, r7 }
	{ st r25, r26 ; move r5, r6 ; nop }
	{ st r25, r26 ; movei r15, 5 ; fsingle_pack1 r5, r6 }
	{ st r25, r26 ; movei r15, 5 ; tblidxb2 r5, r6 }
	{ st r25, r26 ; movei r5, 5 ; shl3add r15, r16, r17 }
	{ st r25, r26 ; mul_hs_hs r5, r6, r7 ; rotl r15, r16, r17 }
	{ st r25, r26 ; mul_hu_hu r5, r6, r7 ; mnz r15, r16, r17 }
	{ st r25, r26 ; mul_ls_ls r5, r6, r7 ; ill }
	{ st r25, r26 ; mul_lu_lu r5, r6, r7 ; cmples r15, r16, r17 }
	{ st r25, r26 ; mula_hs_hs r5, r6, r7 ; addi r15, r16, 5 }
	{ st r25, r26 ; mula_hs_hs r5, r6, r7 ; shru r15, r16, r17 }
	{ st r25, r26 ; mula_hu_hu r5, r6, r7 ; shl2add r15, r16, r17 }
	{ st r25, r26 ; mula_ls_ls r5, r6, r7 ; nor r15, r16, r17 }
	{ st r25, r26 ; mula_lu_lu r5, r6, r7 ; jrp r15 }
	{ st r25, r26 ; mulax r5, r6, r7 ; cmpne r15, r16, r17 }
	{ st r25, r26 ; mulx r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ st r25, r26 ; mulx r5, r6, r7 }
	{ st r25, r26 ; mz r15, r16, r17 ; revbits r5, r6 }
	{ st r25, r26 ; mz r5, r6, r7 ; info 19 }
	{ st r25, r26 ; nop ; and r5, r6, r7 }
	{ st r25, r26 ; nop ; mul_ls_ls r5, r6, r7 }
	{ st r25, r26 ; nop ; shrsi r15, r16, 5 }
	{ st r25, r26 ; nor r15, r16, r17 ; movei r5, 5 }
	{ st r25, r26 ; nor r5, r6, r7 ; add r15, r16, r17 }
	{ st r25, r26 ; nor r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st r25, r26 ; or r15, r16, r17 ; mulx r5, r6, r7 }
	{ st r25, r26 ; or r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st r25, r26 ; pcnt r5, r6 ; addxi r15, r16, 5 }
	{ st r25, r26 ; pcnt r5, r6 ; sub r15, r16, r17 }
	{ st r25, r26 ; revbits r5, r6 ; shl3add r15, r16, r17 }
	{ st r25, r26 ; revbytes r5, r6 ; rotl r15, r16, r17 }
	{ st r25, r26 ; rotl r15, r16, r17 ; move r5, r6 }
	{ st r25, r26 ; rotl r15, r16, r17 }
	{ st r25, r26 ; rotl r5, r6, r7 ; shrs r15, r16, r17 }
	{ st r25, r26 ; rotli r15, r16, 5 ; mulax r5, r6, r7 }
	{ st r25, r26 ; rotli r5, r6, 5 ; cmpleu r15, r16, r17 }
	{ st r25, r26 ; shl r15, r16, r17 ; addx r5, r6, r7 }
	{ st r25, r26 ; shl r15, r16, r17 ; rotli r5, r6, 5 }
	{ st r25, r26 ; shl r5, r6, r7 ; jr r15 }
	{ st r25, r26 ; shl1add r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ st r25, r26 ; shl1add r15, r16, r17 ; shrsi r5, r6, 5 }
	{ st r25, r26 ; shl1add r5, r6, r7 ; rotl r15, r16, r17 }
	{ st r25, r26 ; shl1addx r15, r16, r17 ; move r5, r6 }
	{ st r25, r26 ; shl1addx r15, r16, r17 }
	{ st r25, r26 ; shl1addx r5, r6, r7 ; shrs r15, r16, r17 }
	{ st r25, r26 ; shl2add r15, r16, r17 ; mulax r5, r6, r7 }
	{ st r25, r26 ; shl2add r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ st r25, r26 ; shl2addx r15, r16, r17 ; addx r5, r6, r7 }
	{ st r25, r26 ; shl2addx r15, r16, r17 ; rotli r5, r6, 5 }
	{ st r25, r26 ; shl2addx r5, r6, r7 ; jr r15 }
	{ st r25, r26 ; shl3add r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ st r25, r26 ; shl3add r15, r16, r17 ; shrsi r5, r6, 5 }
	{ st r25, r26 ; shl3add r5, r6, r7 ; rotl r15, r16, r17 }
	{ st r25, r26 ; shl3addx r15, r16, r17 ; move r5, r6 }
	{ st r25, r26 ; shl3addx r15, r16, r17 }
	{ st r25, r26 ; shl3addx r5, r6, r7 ; shrs r15, r16, r17 }
	{ st r25, r26 ; shli r15, r16, 5 ; mulax r5, r6, r7 }
	{ st r25, r26 ; shli r5, r6, 5 ; cmpleu r15, r16, r17 }
	{ st r25, r26 ; shrs r15, r16, r17 ; addx r5, r6, r7 }
	{ st r25, r26 ; shrs r15, r16, r17 ; rotli r5, r6, 5 }
	{ st r25, r26 ; shrs r5, r6, r7 ; jr r15 }
	{ st r25, r26 ; shrsi r15, r16, 5 ; cmpleu r5, r6, r7 }
	{ st r25, r26 ; shrsi r15, r16, 5 ; shrsi r5, r6, 5 }
	{ st r25, r26 ; shrsi r5, r6, 5 ; rotl r15, r16, r17 }
	{ st r25, r26 ; shru r15, r16, r17 ; move r5, r6 }
	{ st r25, r26 ; shru r15, r16, r17 }
	{ st r25, r26 ; shru r5, r6, r7 ; shrs r15, r16, r17 }
	{ st r25, r26 ; shrui r15, r16, 5 ; mulax r5, r6, r7 }
	{ st r25, r26 ; shrui r5, r6, 5 ; cmpleu r15, r16, r17 }
	{ st r25, r26 ; sub r15, r16, r17 ; addx r5, r6, r7 }
	{ st r25, r26 ; sub r15, r16, r17 ; rotli r5, r6, 5 }
	{ st r25, r26 ; sub r5, r6, r7 ; jr r15 }
	{ st r25, r26 ; subx r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ st r25, r26 ; subx r15, r16, r17 ; shrsi r5, r6, 5 }
	{ st r25, r26 ; subx r5, r6, r7 ; rotl r15, r16, r17 }
	{ st r25, r26 ; tblidxb0 r5, r6 ; mnz r15, r16, r17 }
	{ st r25, r26 ; tblidxb1 r5, r6 ; ill }
	{ st r25, r26 ; tblidxb2 r5, r6 ; cmples r15, r16, r17 }
	{ st r25, r26 ; tblidxb3 r5, r6 ; addi r15, r16, 5 }
	{ st r25, r26 ; tblidxb3 r5, r6 ; shru r15, r16, r17 }
	{ st r25, r26 ; xor r15, r16, r17 ; mz r5, r6, r7 }
	{ st r25, r26 ; xor r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ st1 r15, r16 ; addxi r5, r6, 5 }
	{ st1 r15, r16 ; fdouble_unpack_max r5, r6, r7 }
	{ st1 r15, r16 ; nop }
	{ st1 r15, r16 ; v1cmpeqi r5, r6, 5 }
	{ st1 r15, r16 ; v2addi r5, r6, 5 }
	{ st1 r15, r16 ; v2sub r5, r6, r7 }
	{ st1 r25, r26 ; add r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ st1 r25, r26 ; add r5, r6, r7 ; addx r15, r16, r17 }
	{ st1 r25, r26 ; add r5, r6, r7 ; shrui r15, r16, 5 }
	{ st1 r25, r26 ; addi r15, r16, 5 ; nop }
	{ st1 r25, r26 ; addi r5, r6, 5 ; cmpltu r15, r16, r17 }
	{ st1 r25, r26 ; addx r15, r16, r17 ; andi r5, r6, 5 }
	{ st1 r25, r26 ; addx r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ st1 r25, r26 ; addx r5, r6, r7 ; mnz r15, r16, r17 }
	{ st1 r25, r26 ; addxi r15, r16, 5 ; cmpltu r5, r6, r7 }
	{ st1 r25, r26 ; addxi r15, r16, 5 ; sub r5, r6, r7 }
	{ st1 r25, r26 ; addxi r5, r6, 5 ; shl1add r15, r16, r17 }
	{ st1 r25, r26 ; and r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ st1 r25, r26 ; and r5, r6, r7 ; addx r15, r16, r17 }
	{ st1 r25, r26 ; and r5, r6, r7 ; shrui r15, r16, 5 }
	{ st1 r25, r26 ; andi r15, r16, 5 ; nop }
	{ st1 r25, r26 ; andi r5, r6, 5 ; cmpltu r15, r16, r17 }
	{ st1 r25, r26 ; clz r5, r6 ; andi r15, r16, 5 }
	{ st1 r25, r26 ; clz r5, r6 ; xor r15, r16, r17 }
	{ st1 r25, r26 ; cmoveqz r5, r6, r7 ; shli r15, r16, 5 }
	{ st1 r25, r26 ; cmovnez r5, r6, r7 ; shl r15, r16, r17 }
	{ st1 r25, r26 ; cmpeq r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ st1 r25, r26 ; cmpeq r5, r6, r7 ; addi r15, r16, 5 }
	{ st1 r25, r26 ; cmpeq r5, r6, r7 ; shru r15, r16, r17 }
	{ st1 r25, r26 ; cmpeqi r15, r16, 5 ; mz r5, r6, r7 }
	{ st1 r25, r26 ; cmpeqi r5, r6, 5 ; cmpltsi r15, r16, 5 }
	{ st1 r25, r26 ; cmples r15, r16, r17 ; and r5, r6, r7 }
	{ st1 r25, r26 ; cmples r15, r16, r17 ; shl1add r5, r6, r7 }
	{ st1 r25, r26 ; cmples r5, r6, r7 ; lnk r15 }
	{ st1 r25, r26 ; cmpleu r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ st1 r25, r26 ; cmpleu r15, r16, r17 ; shrui r5, r6, 5 }
	{ st1 r25, r26 ; cmpleu r5, r6, r7 ; shl r15, r16, r17 }
	{ st1 r25, r26 ; cmplts r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ st1 r25, r26 ; cmplts r5, r6, r7 ; addi r15, r16, 5 }
	{ st1 r25, r26 ; cmplts r5, r6, r7 ; shru r15, r16, r17 }
	{ st1 r25, r26 ; cmpltsi r15, r16, 5 ; mz r5, r6, r7 }
	{ st1 r25, r26 ; cmpltsi r5, r6, 5 ; cmpltsi r15, r16, 5 }
	{ st1 r25, r26 ; cmpltu r15, r16, r17 ; and r5, r6, r7 }
	{ st1 r25, r26 ; cmpltu r15, r16, r17 ; shl1add r5, r6, r7 }
	{ st1 r25, r26 ; cmpltu r5, r6, r7 ; lnk r15 }
	{ st1 r25, r26 ; cmpne r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ st1 r25, r26 ; cmpne r15, r16, r17 ; shrui r5, r6, 5 }
	{ st1 r25, r26 ; cmpne r5, r6, r7 ; shl r15, r16, r17 }
	{ st1 r25, r26 ; ctz r5, r6 ; movei r15, 5 }
	{ st1 r25, r26 ; fnop ; cmpeqi r15, r16, 5 }
	{ st1 r25, r26 ; fnop ; mz r15, r16, r17 }
	{ st1 r25, r26 ; fnop ; subx r15, r16, r17 }
	{ st1 r25, r26 ; fsingle_pack1 r5, r6 ; shl r15, r16, r17 }
	{ st1 r25, r26 ; ill ; mul_hs_hs r5, r6, r7 }
	{ st1 r25, r26 ; info 19 ; add r5, r6, r7 }
	{ st1 r25, r26 ; info 19 ; mnz r15, r16, r17 }
	{ st1 r25, r26 ; info 19 ; shl3add r15, r16, r17 }
	{ st1 r25, r26 ; jalr r15 ; cmpltu r5, r6, r7 }
	{ st1 r25, r26 ; jalr r15 ; sub r5, r6, r7 }
	{ st1 r25, r26 ; jalrp r15 ; mulax r5, r6, r7 }
	{ st1 r25, r26 ; jr r15 ; cmpeq r5, r6, r7 }
	{ st1 r25, r26 ; jr r15 ; shl3addx r5, r6, r7 }
	{ st1 r25, r26 ; jrp r15 ; mul_ls_ls r5, r6, r7 }
	{ st1 r25, r26 ; lnk r15 ; addxi r5, r6, 5 }
	{ st1 r25, r26 ; lnk r15 ; shl r5, r6, r7 }
	{ st1 r25, r26 ; mnz r15, r16, r17 ; info 19 }
	{ st1 r25, r26 ; mnz r15, r16, r17 ; tblidxb3 r5, r6 }
	{ st1 r25, r26 ; mnz r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ st1 r25, r26 ; move r15, r16 ; mula_ls_ls r5, r6, r7 }
	{ st1 r25, r26 ; move r5, r6 ; cmpeqi r15, r16, 5 }
	{ st1 r25, r26 ; movei r15, 5 ; add r5, r6, r7 }
	{ st1 r25, r26 ; movei r15, 5 ; revbytes r5, r6 }
	{ st1 r25, r26 ; movei r5, 5 ; jalr r15 }
	{ st1 r25, r26 ; mul_hs_hs r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st1 r25, r26 ; mul_hu_hu r5, r6, r7 ; addxi r15, r16, 5 }
	{ st1 r25, r26 ; mul_hu_hu r5, r6, r7 ; sub r15, r16, r17 }
	{ st1 r25, r26 ; mul_ls_ls r5, r6, r7 ; shl3add r15, r16, r17 }
	{ st1 r25, r26 ; mul_lu_lu r5, r6, r7 ; rotl r15, r16, r17 }
	{ st1 r25, r26 ; mula_hs_hs r5, r6, r7 ; mnz r15, r16, r17 }
	{ st1 r25, r26 ; mula_hu_hu r5, r6, r7 ; ill }
	{ st1 r25, r26 ; mula_ls_ls r5, r6, r7 ; cmples r15, r16, r17 }
	{ st1 r25, r26 ; mula_lu_lu r5, r6, r7 ; addi r15, r16, 5 }
	{ st1 r25, r26 ; mula_lu_lu r5, r6, r7 ; shru r15, r16, r17 }
	{ st1 r25, r26 ; mulax r5, r6, r7 ; shl2add r15, r16, r17 }
	{ st1 r25, r26 ; mulx r5, r6, r7 ; nor r15, r16, r17 }
	{ st1 r25, r26 ; mz r15, r16, r17 ; info 19 }
	{ st1 r25, r26 ; mz r15, r16, r17 ; tblidxb3 r5, r6 }
	{ st1 r25, r26 ; mz r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ st1 r25, r26 ; nop ; cmpne r5, r6, r7 }
	{ st1 r25, r26 ; nop ; rotli r5, r6, 5 }
	{ st1 r25, r26 ; nor r15, r16, r17 ; and r5, r6, r7 }
	{ st1 r25, r26 ; nor r15, r16, r17 ; shl1add r5, r6, r7 }
	{ st1 r25, r26 ; nor r5, r6, r7 ; lnk r15 }
	{ st1 r25, r26 ; or r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ st1 r25, r26 ; or r15, r16, r17 ; shrui r5, r6, 5 }
	{ st1 r25, r26 ; or r5, r6, r7 ; shl r15, r16, r17 }
	{ st1 r25, r26 ; pcnt r5, r6 ; movei r15, 5 }
	{ st1 r25, r26 ; revbits r5, r6 ; jalr r15 }
	{ st1 r25, r26 ; revbytes r5, r6 ; cmplts r15, r16, r17 }
	{ st1 r25, r26 ; rotl r15, r16, r17 ; addxi r5, r6, 5 }
	{ st1 r25, r26 ; rotl r15, r16, r17 ; shl r5, r6, r7 }
	{ st1 r25, r26 ; rotl r5, r6, r7 ; jrp r15 }
	{ st1 r25, r26 ; rotli r15, r16, 5 ; cmplts r5, r6, r7 }
	{ st1 r25, r26 ; rotli r15, r16, 5 ; shru r5, r6, r7 }
	{ st1 r25, r26 ; rotli r5, r6, 5 ; rotli r15, r16, 5 }
	{ st1 r25, r26 ; shl r15, r16, r17 ; movei r5, 5 }
	{ st1 r25, r26 ; shl r5, r6, r7 ; add r15, r16, r17 }
	{ st1 r25, r26 ; shl r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st1 r25, r26 ; shl1add r15, r16, r17 ; mulx r5, r6, r7 }
	{ st1 r25, r26 ; shl1add r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st1 r25, r26 ; shl1addx r15, r16, r17 ; addxi r5, r6, 5 }
	{ st1 r25, r26 ; shl1addx r15, r16, r17 ; shl r5, r6, r7 }
	{ st1 r25, r26 ; shl1addx r5, r6, r7 ; jrp r15 }
	{ st1 r25, r26 ; shl2add r15, r16, r17 ; cmplts r5, r6, r7 }
	{ st1 r25, r26 ; shl2add r15, r16, r17 ; shru r5, r6, r7 }
	{ st1 r25, r26 ; shl2add r5, r6, r7 ; rotli r15, r16, 5 }
	{ st1 r25, r26 ; shl2addx r15, r16, r17 ; movei r5, 5 }
	{ st1 r25, r26 ; shl2addx r5, r6, r7 ; add r15, r16, r17 }
	{ st1 r25, r26 ; shl2addx r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st1 r25, r26 ; shl3add r15, r16, r17 ; mulx r5, r6, r7 }
	{ st1 r25, r26 ; shl3add r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st1 r25, r26 ; shl3addx r15, r16, r17 ; addxi r5, r6, 5 }
	{ st1 r25, r26 ; shl3addx r15, r16, r17 ; shl r5, r6, r7 }
	{ st1 r25, r26 ; shl3addx r5, r6, r7 ; jrp r15 }
	{ st1 r25, r26 ; shli r15, r16, 5 ; cmplts r5, r6, r7 }
	{ st1 r25, r26 ; shli r15, r16, 5 ; shru r5, r6, r7 }
	{ st1 r25, r26 ; shli r5, r6, 5 ; rotli r15, r16, 5 }
	{ st1 r25, r26 ; shrs r15, r16, r17 ; movei r5, 5 }
	{ st1 r25, r26 ; shrs r5, r6, r7 ; add r15, r16, r17 }
	{ st1 r25, r26 ; shrs r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st1 r25, r26 ; shrsi r15, r16, 5 ; mulx r5, r6, r7 }
	{ st1 r25, r26 ; shrsi r5, r6, 5 ; cmplts r15, r16, r17 }
	{ st1 r25, r26 ; shru r15, r16, r17 ; addxi r5, r6, 5 }
	{ st1 r25, r26 ; shru r15, r16, r17 ; shl r5, r6, r7 }
	{ st1 r25, r26 ; shru r5, r6, r7 ; jrp r15 }
	{ st1 r25, r26 ; shrui r15, r16, 5 ; cmplts r5, r6, r7 }
	{ st1 r25, r26 ; shrui r15, r16, 5 ; shru r5, r6, r7 }
	{ st1 r25, r26 ; shrui r5, r6, 5 ; rotli r15, r16, 5 }
	{ st1 r25, r26 ; sub r15, r16, r17 ; movei r5, 5 }
	{ st1 r25, r26 ; sub r5, r6, r7 ; add r15, r16, r17 }
	{ st1 r25, r26 ; sub r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st1 r25, r26 ; subx r15, r16, r17 ; mulx r5, r6, r7 }
	{ st1 r25, r26 ; subx r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st1 r25, r26 ; tblidxb0 r5, r6 ; addxi r15, r16, 5 }
	{ st1 r25, r26 ; tblidxb0 r5, r6 ; sub r15, r16, r17 }
	{ st1 r25, r26 ; tblidxb1 r5, r6 ; shl3add r15, r16, r17 }
	{ st1 r25, r26 ; tblidxb2 r5, r6 ; rotl r15, r16, r17 }
	{ st1 r25, r26 ; tblidxb3 r5, r6 ; mnz r15, r16, r17 }
	{ st1 r25, r26 ; xor r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ st1 r25, r26 ; xor r15, r16, r17 ; sub r5, r6, r7 }
	{ st1 r25, r26 ; xor r5, r6, r7 ; shl1add r15, r16, r17 }
	{ st1_add r15, r16, 5 ; cmula r5, r6, r7 }
	{ st1_add r15, r16, 5 ; mul_hu_hu r5, r6, r7 }
	{ st1_add r15, r16, 5 ; shrsi r5, r6, 5 }
	{ st1_add r15, r16, 5 ; v1maxui r5, r6, 5 }
	{ st1_add r15, r16, 5 ; v2mnz r5, r6, r7 }
	{ st2 r15, r16 ; addxsc r5, r6, r7 }
	{ st2 r15, r16 ; fnop }
	{ st2 r15, r16 ; or r5, r6, r7 }
	{ st2 r15, r16 ; v1cmpleu r5, r6, r7 }
	{ st2 r15, r16 ; v2adiffs r5, r6, r7 }
	{ st2 r15, r16 ; v4add r5, r6, r7 }
	{ st2 r25, r26 ; add r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ st2 r25, r26 ; add r5, r6, r7 ; and r15, r16, r17 }
	{ st2 r25, r26 ; add r5, r6, r7 ; subx r15, r16, r17 }
	{ st2 r25, r26 ; addi r15, r16, 5 ; or r5, r6, r7 }
	{ st2 r25, r26 ; addi r5, r6, 5 ; fnop }
	{ st2 r25, r26 ; addx r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ st2 r25, r26 ; addx r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ st2 r25, r26 ; addx r5, r6, r7 ; movei r15, 5 }
	{ st2 r25, r26 ; addxi r15, r16, 5 ; ctz r5, r6 }
	{ st2 r25, r26 ; addxi r15, r16, 5 ; tblidxb0 r5, r6 }
	{ st2 r25, r26 ; addxi r5, r6, 5 ; shl2add r15, r16, r17 }
	{ st2 r25, r26 ; and r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ st2 r25, r26 ; and r5, r6, r7 ; and r15, r16, r17 }
	{ st2 r25, r26 ; and r5, r6, r7 ; subx r15, r16, r17 }
	{ st2 r25, r26 ; andi r15, r16, 5 ; or r5, r6, r7 }
	{ st2 r25, r26 ; andi r5, r6, 5 ; fnop }
	{ st2 r25, r26 ; clz r5, r6 ; cmpeqi r15, r16, 5 }
	{ st2 r25, r26 ; cmoveqz r5, r6, r7 ; add r15, r16, r17 }
	{ st2 r25, r26 ; cmoveqz r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st2 r25, r26 ; cmovnez r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ st2 r25, r26 ; cmpeq r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ st2 r25, r26 ; cmpeq r5, r6, r7 ; addxi r15, r16, 5 }
	{ st2 r25, r26 ; cmpeq r5, r6, r7 ; sub r15, r16, r17 }
	{ st2 r25, r26 ; cmpeqi r15, r16, 5 ; nor r5, r6, r7 }
	{ st2 r25, r26 ; cmpeqi r5, r6, 5 ; cmpne r15, r16, r17 }
	{ st2 r25, r26 ; cmples r15, r16, r17 ; clz r5, r6 }
	{ st2 r25, r26 ; cmples r15, r16, r17 ; shl2add r5, r6, r7 }
	{ st2 r25, r26 ; cmples r5, r6, r7 ; move r15, r16 }
	{ st2 r25, r26 ; cmpleu r15, r16, r17 ; cmpne r5, r6, r7 }
	{ st2 r25, r26 ; cmpleu r15, r16, r17 ; subx r5, r6, r7 }
	{ st2 r25, r26 ; cmpleu r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ st2 r25, r26 ; cmplts r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ st2 r25, r26 ; cmplts r5, r6, r7 ; addxi r15, r16, 5 }
	{ st2 r25, r26 ; cmplts r5, r6, r7 ; sub r15, r16, r17 }
	{ st2 r25, r26 ; cmpltsi r15, r16, 5 ; nor r5, r6, r7 }
	{ st2 r25, r26 ; cmpltsi r5, r6, 5 ; cmpne r15, r16, r17 }
	{ st2 r25, r26 ; cmpltu r15, r16, r17 ; clz r5, r6 }
	{ st2 r25, r26 ; cmpltu r15, r16, r17 ; shl2add r5, r6, r7 }
	{ st2 r25, r26 ; cmpltu r5, r6, r7 ; move r15, r16 }
	{ st2 r25, r26 ; cmpne r15, r16, r17 ; cmpne r5, r6, r7 }
	{ st2 r25, r26 ; cmpne r15, r16, r17 ; subx r5, r6, r7 }
	{ st2 r25, r26 ; cmpne r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ st2 r25, r26 ; ctz r5, r6 ; nop }
	{ st2 r25, r26 ; fnop ; cmples r15, r16, r17 }
	{ st2 r25, r26 ; fnop ; nop }
	{ st2 r25, r26 ; fnop ; tblidxb0 r5, r6 }
	{ st2 r25, r26 ; fsingle_pack1 r5, r6 ; shl1addx r15, r16, r17 }
	{ st2 r25, r26 ; ill ; mul_ls_ls r5, r6, r7 }
	{ st2 r25, r26 ; info 19 ; addi r5, r6, 5 }
	{ st2 r25, r26 ; info 19 ; move r15, r16 }
	{ st2 r25, r26 ; info 19 ; shl3addx r15, r16, r17 }
	{ st2 r25, r26 ; jalr r15 ; ctz r5, r6 }
	{ st2 r25, r26 ; jalr r15 ; tblidxb0 r5, r6 }
	{ st2 r25, r26 ; jalrp r15 ; mz r5, r6, r7 }
	{ st2 r25, r26 ; jr r15 ; cmples r5, r6, r7 }
	{ st2 r25, r26 ; jr r15 ; shrs r5, r6, r7 }
	{ st2 r25, r26 ; jrp r15 ; mula_hs_hs r5, r6, r7 }
	{ st2 r25, r26 ; lnk r15 ; andi r5, r6, 5 }
	{ st2 r25, r26 ; lnk r15 ; shl1addx r5, r6, r7 }
	{ st2 r25, r26 ; mnz r15, r16, r17 ; move r5, r6 }
	{ st2 r25, r26 ; mnz r15, r16, r17 }
	{ st2 r25, r26 ; mnz r5, r6, r7 ; shrs r15, r16, r17 }
	{ st2 r25, r26 ; move r15, r16 ; mulax r5, r6, r7 }
	{ st2 r25, r26 ; move r5, r6 ; cmpleu r15, r16, r17 }
	{ st2 r25, r26 ; movei r15, 5 ; addx r5, r6, r7 }
	{ st2 r25, r26 ; movei r15, 5 ; rotli r5, r6, 5 }
	{ st2 r25, r26 ; movei r5, 5 ; jr r15 }
	{ st2 r25, r26 ; mul_hs_hs r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ st2 r25, r26 ; mul_hu_hu r5, r6, r7 ; andi r15, r16, 5 }
	{ st2 r25, r26 ; mul_hu_hu r5, r6, r7 ; xor r15, r16, r17 }
	{ st2 r25, r26 ; mul_ls_ls r5, r6, r7 ; shli r15, r16, 5 }
	{ st2 r25, r26 ; mul_lu_lu r5, r6, r7 ; shl r15, r16, r17 }
	{ st2 r25, r26 ; mula_hs_hs r5, r6, r7 ; movei r15, 5 }
	{ st2 r25, r26 ; mula_hu_hu r5, r6, r7 ; jalr r15 }
	{ st2 r25, r26 ; mula_ls_ls r5, r6, r7 ; cmplts r15, r16, r17 }
	{ st2 r25, r26 ; mula_lu_lu r5, r6, r7 ; addxi r15, r16, 5 }
	{ st2 r25, r26 ; mula_lu_lu r5, r6, r7 ; sub r15, r16, r17 }
	{ st2 r25, r26 ; mulax r5, r6, r7 ; shl3add r15, r16, r17 }
	{ st2 r25, r26 ; mulx r5, r6, r7 ; rotl r15, r16, r17 }
	{ st2 r25, r26 ; mz r15, r16, r17 ; move r5, r6 }
	{ st2 r25, r26 ; mz r15, r16, r17 }
	{ st2 r25, r26 ; mz r5, r6, r7 ; shrs r15, r16, r17 }
	{ st2 r25, r26 ; nop ; fnop }
	{ st2 r25, r26 ; nop ; shl r5, r6, r7 }
	{ st2 r25, r26 ; nor r15, r16, r17 ; clz r5, r6 }
	{ st2 r25, r26 ; nor r15, r16, r17 ; shl2add r5, r6, r7 }
	{ st2 r25, r26 ; nor r5, r6, r7 ; move r15, r16 }
	{ st2 r25, r26 ; or r15, r16, r17 ; cmpne r5, r6, r7 }
	{ st2 r25, r26 ; or r15, r16, r17 ; subx r5, r6, r7 }
	{ st2 r25, r26 ; or r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ st2 r25, r26 ; pcnt r5, r6 ; nop }
	{ st2 r25, r26 ; revbits r5, r6 ; jr r15 }
	{ st2 r25, r26 ; revbytes r5, r6 ; cmpltu r15, r16, r17 }
	{ st2 r25, r26 ; rotl r15, r16, r17 ; andi r5, r6, 5 }
	{ st2 r25, r26 ; rotl r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ st2 r25, r26 ; rotl r5, r6, r7 ; mnz r15, r16, r17 }
	{ st2 r25, r26 ; rotli r15, r16, 5 ; cmpltu r5, r6, r7 }
	{ st2 r25, r26 ; rotli r15, r16, 5 ; sub r5, r6, r7 }
	{ st2 r25, r26 ; rotli r5, r6, 5 ; shl1add r15, r16, r17 }
	{ st2 r25, r26 ; shl r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ st2 r25, r26 ; shl r5, r6, r7 ; addx r15, r16, r17 }
	{ st2 r25, r26 ; shl r5, r6, r7 ; shrui r15, r16, 5 }
	{ st2 r25, r26 ; shl1add r15, r16, r17 ; nop }
	{ st2 r25, r26 ; shl1add r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ st2 r25, r26 ; shl1addx r15, r16, r17 ; andi r5, r6, 5 }
	{ st2 r25, r26 ; shl1addx r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ st2 r25, r26 ; shl1addx r5, r6, r7 ; mnz r15, r16, r17 }
	{ st2 r25, r26 ; shl2add r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ st2 r25, r26 ; shl2add r15, r16, r17 ; sub r5, r6, r7 }
	{ st2 r25, r26 ; shl2add r5, r6, r7 ; shl1add r15, r16, r17 }
	{ st2 r25, r26 ; shl2addx r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ st2 r25, r26 ; shl2addx r5, r6, r7 ; addx r15, r16, r17 }
	{ st2 r25, r26 ; shl2addx r5, r6, r7 ; shrui r15, r16, 5 }
	{ st2 r25, r26 ; shl3add r15, r16, r17 ; nop }
	{ st2 r25, r26 ; shl3add r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ st2 r25, r26 ; shl3addx r15, r16, r17 ; andi r5, r6, 5 }
	{ st2 r25, r26 ; shl3addx r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ st2 r25, r26 ; shl3addx r5, r6, r7 ; mnz r15, r16, r17 }
	{ st2 r25, r26 ; shli r15, r16, 5 ; cmpltu r5, r6, r7 }
	{ st2 r25, r26 ; shli r15, r16, 5 ; sub r5, r6, r7 }
	{ st2 r25, r26 ; shli r5, r6, 5 ; shl1add r15, r16, r17 }
	{ st2 r25, r26 ; shrs r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ st2 r25, r26 ; shrs r5, r6, r7 ; addx r15, r16, r17 }
	{ st2 r25, r26 ; shrs r5, r6, r7 ; shrui r15, r16, 5 }
	{ st2 r25, r26 ; shrsi r15, r16, 5 ; nop }
	{ st2 r25, r26 ; shrsi r5, r6, 5 ; cmpltu r15, r16, r17 }
	{ st2 r25, r26 ; shru r15, r16, r17 ; andi r5, r6, 5 }
	{ st2 r25, r26 ; shru r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ st2 r25, r26 ; shru r5, r6, r7 ; mnz r15, r16, r17 }
	{ st2 r25, r26 ; shrui r15, r16, 5 ; cmpltu r5, r6, r7 }
	{ st2 r25, r26 ; shrui r15, r16, 5 ; sub r5, r6, r7 }
	{ st2 r25, r26 ; shrui r5, r6, 5 ; shl1add r15, r16, r17 }
	{ st2 r25, r26 ; sub r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ st2 r25, r26 ; sub r5, r6, r7 ; addx r15, r16, r17 }
	{ st2 r25, r26 ; sub r5, r6, r7 ; shrui r15, r16, 5 }
	{ st2 r25, r26 ; subx r15, r16, r17 ; nop }
	{ st2 r25, r26 ; subx r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ st2 r25, r26 ; tblidxb0 r5, r6 ; andi r15, r16, 5 }
	{ st2 r25, r26 ; tblidxb0 r5, r6 ; xor r15, r16, r17 }
	{ st2 r25, r26 ; tblidxb1 r5, r6 ; shli r15, r16, 5 }
	{ st2 r25, r26 ; tblidxb2 r5, r6 ; shl r15, r16, r17 }
	{ st2 r25, r26 ; tblidxb3 r5, r6 ; movei r15, 5 }
	{ st2 r25, r26 ; xor r15, r16, r17 ; ctz r5, r6 }
	{ st2 r25, r26 ; xor r15, r16, r17 ; tblidxb0 r5, r6 }
	{ st2 r25, r26 ; xor r5, r6, r7 ; shl2add r15, r16, r17 }
	{ st2_add r15, r16, 5 ; cmulf r5, r6, r7 }
	{ st2_add r15, r16, 5 ; mul_hu_lu r5, r6, r7 }
	{ st2_add r15, r16, 5 ; shrui r5, r6, 5 }
	{ st2_add r15, r16, 5 ; v1minui r5, r6, 5 }
	{ st2_add r15, r16, 5 ; v2muls r5, r6, r7 }
	{ st4 r15, r16 ; andi r5, r6, 5 }
	{ st4 r15, r16 ; fsingle_addsub2 r5, r6, r7 }
	{ st4 r15, r16 ; pcnt r5, r6 }
	{ st4 r15, r16 ; v1cmpltsi r5, r6, 5 }
	{ st4 r15, r16 ; v2cmpeq r5, r6, r7 }
	{ st4 r15, r16 ; v4int_h r5, r6, r7 }
	{ st4 r25, r26 ; add r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ st4 r25, r26 ; add r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ st4 r25, r26 ; add r5, r6, r7 }
	{ st4 r25, r26 ; addi r15, r16, 5 ; revbits r5, r6 }
	{ st4 r25, r26 ; addi r5, r6, 5 ; info 19 }
	{ st4 r25, r26 ; addx r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ st4 r25, r26 ; addx r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ st4 r25, r26 ; addx r5, r6, r7 ; nop }
	{ st4 r25, r26 ; addxi r15, r16, 5 ; fsingle_pack1 r5, r6 }
	{ st4 r25, r26 ; addxi r15, r16, 5 ; tblidxb2 r5, r6 }
	{ st4 r25, r26 ; addxi r5, r6, 5 ; shl3add r15, r16, r17 }
	{ st4 r25, r26 ; and r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ st4 r25, r26 ; and r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ st4 r25, r26 ; and r5, r6, r7 }
	{ st4 r25, r26 ; andi r15, r16, 5 ; revbits r5, r6 }
	{ st4 r25, r26 ; andi r5, r6, 5 ; info 19 }
	{ st4 r25, r26 ; clz r5, r6 ; cmpleu r15, r16, r17 }
	{ st4 r25, r26 ; cmoveqz r5, r6, r7 ; addx r15, r16, r17 }
	{ st4 r25, r26 ; cmoveqz r5, r6, r7 ; shrui r15, r16, 5 }
	{ st4 r25, r26 ; cmovnez r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ st4 r25, r26 ; cmpeq r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ st4 r25, r26 ; cmpeq r5, r6, r7 ; andi r15, r16, 5 }
	{ st4 r25, r26 ; cmpeq r5, r6, r7 ; xor r15, r16, r17 }
	{ st4 r25, r26 ; cmpeqi r15, r16, 5 ; pcnt r5, r6 }
	{ st4 r25, r26 ; cmpeqi r5, r6, 5 ; ill }
	{ st4 r25, r26 ; cmples r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ st4 r25, r26 ; cmples r15, r16, r17 ; shl3add r5, r6, r7 }
	{ st4 r25, r26 ; cmples r5, r6, r7 ; mz r15, r16, r17 }
	{ st4 r25, r26 ; cmpleu r15, r16, r17 ; fnop }
	{ st4 r25, r26 ; cmpleu r15, r16, r17 ; tblidxb1 r5, r6 }
	{ st4 r25, r26 ; cmpleu r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ st4 r25, r26 ; cmplts r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ st4 r25, r26 ; cmplts r5, r6, r7 ; andi r15, r16, 5 }
	{ st4 r25, r26 ; cmplts r5, r6, r7 ; xor r15, r16, r17 }
	{ st4 r25, r26 ; cmpltsi r15, r16, 5 ; pcnt r5, r6 }
	{ st4 r25, r26 ; cmpltsi r5, r6, 5 ; ill }
	{ st4 r25, r26 ; cmpltu r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ st4 r25, r26 ; cmpltu r15, r16, r17 ; shl3add r5, r6, r7 }
	{ st4 r25, r26 ; cmpltu r5, r6, r7 ; mz r15, r16, r17 }
	{ st4 r25, r26 ; cmpne r15, r16, r17 ; fnop }
	{ st4 r25, r26 ; cmpne r15, r16, r17 ; tblidxb1 r5, r6 }
	{ st4 r25, r26 ; cmpne r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ st4 r25, r26 ; ctz r5, r6 ; or r15, r16, r17 }
	{ st4 r25, r26 ; fnop ; cmpleu r15, r16, r17 }
	{ st4 r25, r26 ; fnop ; nor r5, r6, r7 }
	{ st4 r25, r26 ; fnop ; tblidxb2 r5, r6 }
	{ st4 r25, r26 ; fsingle_pack1 r5, r6 ; shl2addx r15, r16, r17 }
	{ st4 r25, r26 ; ill ; mula_hs_hs r5, r6, r7 }
	{ st4 r25, r26 ; info 19 ; addx r5, r6, r7 }
	{ st4 r25, r26 ; info 19 ; movei r15, 5 }
	{ st4 r25, r26 ; info 19 ; shli r15, r16, 5 }
	{ st4 r25, r26 ; jalr r15 ; fsingle_pack1 r5, r6 }
	{ st4 r25, r26 ; jalr r15 ; tblidxb2 r5, r6 }
	{ st4 r25, r26 ; jalrp r15 ; nor r5, r6, r7 }
	{ st4 r25, r26 ; jr r15 ; cmplts r5, r6, r7 }
	{ st4 r25, r26 ; jr r15 ; shru r5, r6, r7 }
	{ st4 r25, r26 ; jrp r15 ; mula_ls_ls r5, r6, r7 }
	{ st4 r25, r26 ; lnk r15 ; cmoveqz r5, r6, r7 }
	{ st4 r25, r26 ; lnk r15 ; shl2addx r5, r6, r7 }
	{ st4 r25, r26 ; mnz r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ st4 r25, r26 ; mnz r5, r6, r7 ; addi r15, r16, 5 }
	{ st4 r25, r26 ; mnz r5, r6, r7 ; shru r15, r16, r17 }
	{ st4 r25, r26 ; move r15, r16 ; mz r5, r6, r7 }
	{ st4 r25, r26 ; move r5, r6 ; cmpltsi r15, r16, 5 }
	{ st4 r25, r26 ; movei r15, 5 ; and r5, r6, r7 }
	{ st4 r25, r26 ; movei r15, 5 ; shl1add r5, r6, r7 }
	{ st4 r25, r26 ; movei r5, 5 ; lnk r15 }
	{ st4 r25, r26 ; mul_hs_hs r5, r6, r7 ; fnop }
	{ st4 r25, r26 ; mul_hu_hu r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ st4 r25, r26 ; mul_ls_ls r5, r6, r7 ; add r15, r16, r17 }
	{ st4 r25, r26 ; mul_ls_ls r5, r6, r7 ; shrsi r15, r16, 5 }
	{ st4 r25, r26 ; mul_lu_lu r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ st4 r25, r26 ; mula_hs_hs r5, r6, r7 ; nop }
	{ st4 r25, r26 ; mula_hu_hu r5, r6, r7 ; jr r15 }
	{ st4 r25, r26 ; mula_ls_ls r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ st4 r25, r26 ; mula_lu_lu r5, r6, r7 ; andi r15, r16, 5 }
	{ st4 r25, r26 ; mula_lu_lu r5, r6, r7 ; xor r15, r16, r17 }
	{ st4 r25, r26 ; mulax r5, r6, r7 ; shli r15, r16, 5 }
	{ st4 r25, r26 ; mulx r5, r6, r7 ; shl r15, r16, r17 }
	{ st4 r25, r26 ; mz r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ st4 r25, r26 ; mz r5, r6, r7 ; addi r15, r16, 5 }
	{ st4 r25, r26 ; mz r5, r6, r7 ; shru r15, r16, r17 }
	{ st4 r25, r26 ; nop ; ill }
	{ st4 r25, r26 ; nop ; shl1add r5, r6, r7 }
	{ st4 r25, r26 ; nor r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ st4 r25, r26 ; nor r15, r16, r17 ; shl3add r5, r6, r7 }
	{ st4 r25, r26 ; nor r5, r6, r7 ; mz r15, r16, r17 }
	{ st4 r25, r26 ; or r15, r16, r17 ; fnop }
	{ st4 r25, r26 ; or r15, r16, r17 ; tblidxb1 r5, r6 }
	{ st4 r25, r26 ; or r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ st4 r25, r26 ; pcnt r5, r6 ; or r15, r16, r17 }
	{ st4 r25, r26 ; revbits r5, r6 ; lnk r15 }
	{ st4 r25, r26 ; revbytes r5, r6 ; fnop }
	{ st4 r25, r26 ; rotl r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ st4 r25, r26 ; rotl r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ st4 r25, r26 ; rotl r5, r6, r7 ; movei r15, 5 }
	{ st4 r25, r26 ; rotli r15, r16, 5 ; ctz r5, r6 }
	{ st4 r25, r26 ; rotli r15, r16, 5 ; tblidxb0 r5, r6 }
	{ st4 r25, r26 ; rotli r5, r6, 5 ; shl2add r15, r16, r17 }
	{ st4 r25, r26 ; shl r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ st4 r25, r26 ; shl r5, r6, r7 ; and r15, r16, r17 }
	{ st4 r25, r26 ; shl r5, r6, r7 ; subx r15, r16, r17 }
	{ st4 r25, r26 ; shl1add r15, r16, r17 ; or r5, r6, r7 }
	{ st4 r25, r26 ; shl1add r5, r6, r7 ; fnop }
	{ st4 r25, r26 ; shl1addx r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ st4 r25, r26 ; shl1addx r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ st4 r25, r26 ; shl1addx r5, r6, r7 ; movei r15, 5 }
	{ st4 r25, r26 ; shl2add r15, r16, r17 ; ctz r5, r6 }
	{ st4 r25, r26 ; shl2add r15, r16, r17 ; tblidxb0 r5, r6 }
	{ st4 r25, r26 ; shl2add r5, r6, r7 ; shl2add r15, r16, r17 }
	{ st4 r25, r26 ; shl2addx r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ st4 r25, r26 ; shl2addx r5, r6, r7 ; and r15, r16, r17 }
	{ st4 r25, r26 ; shl2addx r5, r6, r7 ; subx r15, r16, r17 }
	{ st4 r25, r26 ; shl3add r15, r16, r17 ; or r5, r6, r7 }
	{ st4 r25, r26 ; shl3add r5, r6, r7 ; fnop }
	{ st4 r25, r26 ; shl3addx r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ st4 r25, r26 ; shl3addx r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ st4 r25, r26 ; shl3addx r5, r6, r7 ; movei r15, 5 }
	{ st4 r25, r26 ; shli r15, r16, 5 ; ctz r5, r6 }
	{ st4 r25, r26 ; shli r15, r16, 5 ; tblidxb0 r5, r6 }
	{ st4 r25, r26 ; shli r5, r6, 5 ; shl2add r15, r16, r17 }
	{ st4 r25, r26 ; shrs r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ st4 r25, r26 ; shrs r5, r6, r7 ; and r15, r16, r17 }
	{ st4 r25, r26 ; shrs r5, r6, r7 ; subx r15, r16, r17 }
	{ st4 r25, r26 ; shrsi r15, r16, 5 ; or r5, r6, r7 }
	{ st4 r25, r26 ; shrsi r5, r6, 5 ; fnop }
	{ st4 r25, r26 ; shru r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ st4 r25, r26 ; shru r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ st4 r25, r26 ; shru r5, r6, r7 ; movei r15, 5 }
	{ st4 r25, r26 ; shrui r15, r16, 5 ; ctz r5, r6 }
	{ st4 r25, r26 ; shrui r15, r16, 5 ; tblidxb0 r5, r6 }
	{ st4 r25, r26 ; shrui r5, r6, 5 ; shl2add r15, r16, r17 }
	{ st4 r25, r26 ; sub r15, r16, r17 ; mul_lu_lu r5, r6, r7 }
	{ st4 r25, r26 ; sub r5, r6, r7 ; and r15, r16, r17 }
	{ st4 r25, r26 ; sub r5, r6, r7 ; subx r15, r16, r17 }
	{ st4 r25, r26 ; subx r15, r16, r17 ; or r5, r6, r7 }
	{ st4 r25, r26 ; subx r5, r6, r7 ; fnop }
	{ st4 r25, r26 ; tblidxb0 r5, r6 ; cmpeqi r15, r16, 5 }
	{ st4 r25, r26 ; tblidxb1 r5, r6 ; add r15, r16, r17 }
	{ st4 r25, r26 ; tblidxb1 r5, r6 ; shrsi r15, r16, 5 }
	{ st4 r25, r26 ; tblidxb2 r5, r6 ; shl1addx r15, r16, r17 }
	{ st4 r25, r26 ; tblidxb3 r5, r6 ; nop }
	{ st4 r25, r26 ; xor r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ st4 r25, r26 ; xor r15, r16, r17 ; tblidxb2 r5, r6 }
	{ st4 r25, r26 ; xor r5, r6, r7 ; shl3add r15, r16, r17 }
	{ st4_add r15, r16, 5 ; cmulh r5, r6, r7 }
	{ st4_add r15, r16, 5 ; mul_ls_lu r5, r6, r7 }
	{ st4_add r15, r16, 5 ; shruxi r5, r6, 5 }
	{ st4_add r15, r16, 5 ; v1multu r5, r6, r7 }
	{ st4_add r15, r16, 5 ; v2mz r5, r6, r7 }
	{ st_add r15, r16, 5 ; bfextu r5, r6, 5, 7 }
	{ st_add r15, r16, 5 ; fsingle_mul2 r5, r6, r7 }
	{ st_add r15, r16, 5 ; revbytes r5, r6 }
	{ st_add r15, r16, 5 ; v1cmpltui r5, r6, 5 }
	{ st_add r15, r16, 5 ; v2cmples r5, r6, r7 }
	{ st_add r15, r16, 5 ; v4packsc r5, r6, r7 }
	{ stnt r15, r16 ; crc32_32 r5, r6, r7 }
	{ stnt r15, r16 ; mula_hs_hs r5, r6, r7 }
	{ stnt r15, r16 ; sub r5, r6, r7 }
	{ stnt r15, r16 ; v1mulus r5, r6, r7 }
	{ stnt r15, r16 ; v2packl r5, r6, r7 }
	{ stnt1 r15, r16 ; clz r5, r6 }
	{ stnt1 r15, r16 ; fsingle_pack2 r5, r6, r7 }
	{ stnt1 r15, r16 ; rotli r5, r6, 5 }
	{ stnt1 r15, r16 ; v1ddotpu r5, r6, r7 }
	{ stnt1 r15, r16 ; v2cmplts r5, r6, r7 }
	{ stnt1 r15, r16 ; v4shlsc r5, r6, r7 }
	{ stnt1_add r15, r16, 5 ; ctz r5, r6 }
	{ stnt1_add r15, r16, 5 ; mula_hs_ls r5, r6, r7 }
	{ stnt1_add r15, r16, 5 ; subxsc r5, r6, r7 }
	{ stnt1_add r15, r16, 5 ; v1sadau r5, r6, r7 }
	{ stnt1_add r15, r16, 5 ; v2sadas r5, r6, r7 }
	{ stnt2 r15, r16 ; cmovnez r5, r6, r7 }
	{ stnt2 r15, r16 ; info 19 }
	{ stnt2 r15, r16 ; shl16insli r5, r6, 0x1234 }
	{ stnt2 r15, r16 ; v1ddotpus r5, r6, r7 }
	{ stnt2 r15, r16 ; v2cmpltu r5, r6, r7 }
	{ stnt2 r15, r16 ; v4shru r5, r6, r7 }
	{ stnt2_add r15, r16, 5 ; dblalign2 r5, r6, r7 }
	{ stnt2_add r15, r16, 5 ; mula_hu_hu r5, r6, r7 }
	{ stnt2_add r15, r16, 5 ; tblidxb1 r5, r6 }
	{ stnt2_add r15, r16, 5 ; v1shl r5, r6, r7 }
	{ stnt2_add r15, r16, 5 ; v2sads r5, r6, r7 }
	{ stnt4 r15, r16 ; cmpeqi r5, r6, 5 }
	{ stnt4 r15, r16 ; mm r5, r6, 5, 7 }
	{ stnt4 r15, r16 ; shl1addx r5, r6, r7 }
	{ stnt4 r15, r16 ; v1dotp r5, r6, r7 }
	{ stnt4 r15, r16 ; v2cmpne r5, r6, r7 }
	{ stnt4 r15, r16 ; v4subsc r5, r6, r7 }
	{ stnt4_add r15, r16, 5 ; dblalign6 r5, r6, r7 }
	{ stnt4_add r15, r16, 5 ; mula_hu_lu r5, r6, r7 }
	{ stnt4_add r15, r16, 5 ; tblidxb3 r5, r6 }
	{ stnt4_add r15, r16, 5 ; v1shrs r5, r6, r7 }
	{ stnt4_add r15, r16, 5 ; v2shl r5, r6, r7 }
	{ stnt_add r15, r16, 5 ; cmpleu r5, r6, r7 }
	{ stnt_add r15, r16, 5 ; move r5, r6 }
	{ stnt_add r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ stnt_add r15, r16, 5 ; v1dotpu r5, r6, r7 }
	{ stnt_add r15, r16, 5 ; v2dotpa r5, r6, r7 }
	{ stnt_add r15, r16, 5 ; xori r5, r6, 5 }
	{ sub r15, r16, r17 ; addx r5, r6, r7 ; ld r25, r26 }
	{ sub r15, r16, r17 ; and r5, r6, r7 ; ld r25, r26 }
	{ sub r15, r16, r17 ; bfins r5, r6, 5, 7 }
	{ sub r15, r16, r17 ; cmovnez r5, r6, r7 ; ld1s r25, r26 }
	{ sub r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
	{ sub r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4s r25, r26 }
	{ sub r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
	{ sub r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l1 r25 }
	{ sub r15, r16, r17 ; dblalign2 r5, r6, r7 }
	{ sub r15, r16, r17 ; fsingle_pack1 r5, r6 ; ld4u r25, r26 }
	{ sub r15, r16, r17 ; ld r25, r26 ; andi r5, r6, 5 }
	{ sub r15, r16, r17 ; ld r25, r26 ; shl1addx r5, r6, r7 }
	{ sub r15, r16, r17 ; ld1s r25, r26 ; move r5, r6 }
	{ sub r15, r16, r17 ; ld1s r25, r26 }
	{ sub r15, r16, r17 ; ld1u r25, r26 ; revbits r5, r6 }
	{ sub r15, r16, r17 ; ld2s r25, r26 ; cmpne r5, r6, r7 }
	{ sub r15, r16, r17 ; ld2s r25, r26 ; subx r5, r6, r7 }
	{ sub r15, r16, r17 ; ld2u r25, r26 ; mulx r5, r6, r7 }
	{ sub r15, r16, r17 ; ld4s r25, r26 ; cmpeqi r5, r6, 5 }
	{ sub r15, r16, r17 ; ld4s r25, r26 ; shli r5, r6, 5 }
	{ sub r15, r16, r17 ; ld4u r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ sub r15, r16, r17 ; mnz r5, r6, r7 ; ld2s r25, r26 }
	{ sub r15, r16, r17 ; movei r5, 5 ; ld4s r25, r26 }
	{ sub r15, r16, r17 ; mul_hu_hu r5, r6, r7 ; ld2s r25, r26 }
	{ sub r15, r16, r17 ; mul_lu_lu r5, r6, r7 ; ld1u r25, r26 }
	{ sub r15, r16, r17 ; mula_hu_hu r5, r6, r7 ; ld1s r25, r26 }
	{ sub r15, r16, r17 ; mula_lu_lu r5, r6, r7 ; ld r25, r26 }
	{ sub r15, r16, r17 ; mulx r5, r6, r7 ; ld1u r25, r26 }
	{ sub r15, r16, r17 ; nop ; ld2u r25, r26 }
	{ sub r15, r16, r17 ; or r5, r6, r7 ; ld4u r25, r26 }
	{ sub r15, r16, r17 ; prefetch r25 ; cmoveqz r5, r6, r7 }
	{ sub r15, r16, r17 ; prefetch r25 ; shl2addx r5, r6, r7 }
	{ sub r15, r16, r17 ; prefetch_l1 r25 ; mul_hs_hs r5, r6, r7 }
	{ sub r15, r16, r17 ; prefetch_l1_fault r25 ; addi r5, r6, 5 }
	{ sub r15, r16, r17 ; prefetch_l1_fault r25 ; rotl r5, r6, r7 }
	{ sub r15, r16, r17 ; prefetch_l2 r25 ; fnop }
	{ sub r15, r16, r17 ; prefetch_l2 r25 ; tblidxb1 r5, r6 }
	{ sub r15, r16, r17 ; prefetch_l2_fault r25 ; nop }
	{ sub r15, r16, r17 ; prefetch_l3 r25 ; cmpleu r5, r6, r7 }
	{ sub r15, r16, r17 ; prefetch_l3 r25 ; shrsi r5, r6, 5 }
	{ sub r15, r16, r17 ; prefetch_l3_fault r25 ; mula_hu_hu r5, r6, r7 }
	{ sub r15, r16, r17 ; revbits r5, r6 ; ld4u r25, r26 }
	{ sub r15, r16, r17 ; rotl r5, r6, r7 ; prefetch_l1 r25 }
	{ sub r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
	{ sub r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch_l2_fault r25 }
	{ sub r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l3_fault r25 }
	{ sub r15, r16, r17 ; shl3addx r5, r6, r7 ; st1 r25, r26 }
	{ sub r15, r16, r17 ; shrs r5, r6, r7 ; st1 r25, r26 }
	{ sub r15, r16, r17 ; shru r5, r6, r7 ; st4 r25, r26 }
	{ sub r15, r16, r17 ; st r25, r26 ; cmpne r5, r6, r7 }
	{ sub r15, r16, r17 ; st r25, r26 ; subx r5, r6, r7 }
	{ sub r15, r16, r17 ; st1 r25, r26 ; mulx r5, r6, r7 }
	{ sub r15, r16, r17 ; st2 r25, r26 ; cmpeqi r5, r6, 5 }
	{ sub r15, r16, r17 ; st2 r25, r26 ; shli r5, r6, 5 }
	{ sub r15, r16, r17 ; st4 r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ sub r15, r16, r17 ; sub r5, r6, r7 ; ld2u r25, r26 }
	{ sub r15, r16, r17 ; tblidxb0 r5, r6 ; ld4s r25, r26 }
	{ sub r15, r16, r17 ; tblidxb2 r5, r6 ; prefetch r25 }
	{ sub r15, r16, r17 ; v1cmplts r5, r6, r7 }
	{ sub r15, r16, r17 ; v2avgs r5, r6, r7 }
	{ sub r15, r16, r17 ; v4addsc r5, r6, r7 }
	{ sub r5, r6, r7 ; add r15, r16, r17 ; prefetch_l2 r25 }
	{ sub r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l2_fault r25 }
	{ sub r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
	{ sub r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
	{ sub r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
	{ sub r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
	{ sub r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
	{ sub r5, r6, r7 ; fnop ; ld1s r25, r26 }
	{ sub r5, r6, r7 ; info 19 ; ld1u r25, r26 }
	{ sub r5, r6, r7 ; jalrp r15 ; ld1s r25, r26 }
	{ sub r5, r6, r7 ; jrp r15 ; ld2s r25, r26 }
	{ sub r5, r6, r7 ; ld r25, r26 ; move r15, r16 }
	{ sub r5, r6, r7 ; ld1s r25, r26 ; ill }
	{ sub r5, r6, r7 ; ld1u r25, r26 ; cmpeq r15, r16, r17 }
	{ sub r5, r6, r7 ; ld1u r25, r26 }
	{ sub r5, r6, r7 ; ld2s r25, r26 ; shl3addx r15, r16, r17 }
	{ sub r5, r6, r7 ; ld2u r25, r26 ; or r15, r16, r17 }
	{ sub r5, r6, r7 ; ld4s r25, r26 ; jr r15 }
	{ sub r5, r6, r7 ; ld4u r25, r26 ; cmplts r15, r16, r17 }
	{ sub r5, r6, r7 ; ldna_add r15, r16, 5 }
	{ sub r5, r6, r7 ; mnz r15, r16, r17 ; ld2u r25, r26 }
	{ sub r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
	{ sub r5, r6, r7 ; nop ; ld4u r25, r26 }
	{ sub r5, r6, r7 ; or r15, r16, r17 ; prefetch_l1 r25 }
	{ sub r5, r6, r7 ; prefetch r25 ; nor r15, r16, r17 }
	{ sub r5, r6, r7 ; prefetch_l1 r25 ; cmpne r15, r16, r17 }
	{ sub r5, r6, r7 ; prefetch_l1_fault r25 ; andi r15, r16, 5 }
	{ sub r5, r6, r7 ; prefetch_l1_fault r25 ; xor r15, r16, r17 }
	{ sub r5, r6, r7 ; prefetch_l2 r25 ; shl3addx r15, r16, r17 }
	{ sub r5, r6, r7 ; prefetch_l2_fault r25 ; rotl r15, r16, r17 }
	{ sub r5, r6, r7 ; prefetch_l3 r25 ; lnk r15 }
	{ sub r5, r6, r7 ; prefetch_l3_fault r25 ; cmpne r15, r16, r17 }
	{ sub r5, r6, r7 ; rotl r15, r16, r17 ; ld4s r25, r26 }
	{ sub r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
	{ sub r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l1 r25 }
	{ sub r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2 r25 }
	{ sub r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
	{ sub r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3 r25 }
	{ sub r5, r6, r7 ; shru r15, r16, r17 ; st r25, r26 }
	{ sub r5, r6, r7 ; st r25, r26 ; cmpne r15, r16, r17 }
	{ sub r5, r6, r7 ; st1 r25, r26 ; andi r15, r16, 5 }
	{ sub r5, r6, r7 ; st1 r25, r26 ; xor r15, r16, r17 }
	{ sub r5, r6, r7 ; st2 r25, r26 ; shl3add r15, r16, r17 }
	{ sub r5, r6, r7 ; st4 r25, r26 ; nor r15, r16, r17 }
	{ sub r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l2 r25 }
	{ sub r5, r6, r7 ; v1cmpne r15, r16, r17 }
	{ sub r5, r6, r7 ; v2shl r15, r16, r17 }
	{ sub r5, r6, r7 ; xori r15, r16, 5 }
	{ subx r15, r16, r17 ; addx r5, r6, r7 ; ld r25, r26 }
	{ subx r15, r16, r17 ; and r5, r6, r7 ; ld r25, r26 }
	{ subx r15, r16, r17 ; bfins r5, r6, 5, 7 }
	{ subx r15, r16, r17 ; cmovnez r5, r6, r7 ; ld1s r25, r26 }
	{ subx r15, r16, r17 ; cmpeqi r5, r6, 5 ; ld2s r25, r26 }
	{ subx r15, r16, r17 ; cmpleu r5, r6, r7 ; ld4s r25, r26 }
	{ subx r15, r16, r17 ; cmpltsi r5, r6, 5 ; prefetch r25 }
	{ subx r15, r16, r17 ; cmpne r5, r6, r7 ; prefetch_l1 r25 }
	{ subx r15, r16, r17 ; dblalign2 r5, r6, r7 }
	{ subx r15, r16, r17 ; fsingle_pack1 r5, r6 ; ld4u r25, r26 }
	{ subx r15, r16, r17 ; ld r25, r26 ; andi r5, r6, 5 }
	{ subx r15, r16, r17 ; ld r25, r26 ; shl1addx r5, r6, r7 }
	{ subx r15, r16, r17 ; ld1s r25, r26 ; move r5, r6 }
	{ subx r15, r16, r17 ; ld1s r25, r26 }
	{ subx r15, r16, r17 ; ld1u r25, r26 ; revbits r5, r6 }
	{ subx r15, r16, r17 ; ld2s r25, r26 ; cmpne r5, r6, r7 }
	{ subx r15, r16, r17 ; ld2s r25, r26 ; subx r5, r6, r7 }
	{ subx r15, r16, r17 ; ld2u r25, r26 ; mulx r5, r6, r7 }
	{ subx r15, r16, r17 ; ld4s r25, r26 ; cmpeqi r5, r6, 5 }
	{ subx r15, r16, r17 ; ld4s r25, r26 ; shli r5, r6, 5 }
	{ subx r15, r16, r17 ; ld4u r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ subx r15, r16, r17 ; mnz r5, r6, r7 ; ld2s r25, r26 }
	{ subx r15, r16, r17 ; movei r5, 5 ; ld4s r25, r26 }
	{ subx r15, r16, r17 ; mul_hu_hu r5, r6, r7 ; ld2s r25, r26 }
	{ subx r15, r16, r17 ; mul_lu_lu r5, r6, r7 ; ld1u r25, r26 }
	{ subx r15, r16, r17 ; mula_hu_hu r5, r6, r7 ; ld1s r25, r26 }
	{ subx r15, r16, r17 ; mula_lu_lu r5, r6, r7 ; ld r25, r26 }
	{ subx r15, r16, r17 ; mulx r5, r6, r7 ; ld1u r25, r26 }
	{ subx r15, r16, r17 ; nop ; ld2u r25, r26 }
	{ subx r15, r16, r17 ; or r5, r6, r7 ; ld4u r25, r26 }
	{ subx r15, r16, r17 ; prefetch r25 ; cmoveqz r5, r6, r7 }
	{ subx r15, r16, r17 ; prefetch r25 ; shl2addx r5, r6, r7 }
	{ subx r15, r16, r17 ; prefetch_l1 r25 ; mul_hs_hs r5, r6, r7 }
	{ subx r15, r16, r17 ; prefetch_l1_fault r25 ; addi r5, r6, 5 }
	{ subx r15, r16, r17 ; prefetch_l1_fault r25 ; rotl r5, r6, r7 }
	{ subx r15, r16, r17 ; prefetch_l2 r25 ; fnop }
	{ subx r15, r16, r17 ; prefetch_l2 r25 ; tblidxb1 r5, r6 }
	{ subx r15, r16, r17 ; prefetch_l2_fault r25 ; nop }
	{ subx r15, r16, r17 ; prefetch_l3 r25 ; cmpleu r5, r6, r7 }
	{ subx r15, r16, r17 ; prefetch_l3 r25 ; shrsi r5, r6, 5 }
	{ subx r15, r16, r17 ; prefetch_l3_fault r25 ; mula_hu_hu r5, r6, r7 }
	{ subx r15, r16, r17 ; revbits r5, r6 ; ld4u r25, r26 }
	{ subx r15, r16, r17 ; rotl r5, r6, r7 ; prefetch_l1 r25 }
	{ subx r15, r16, r17 ; shl r5, r6, r7 ; prefetch_l2 r25 }
	{ subx r15, r16, r17 ; shl1addx r5, r6, r7 ; prefetch_l2_fault r25 }
	{ subx r15, r16, r17 ; shl2addx r5, r6, r7 ; prefetch_l3_fault r25 }
	{ subx r15, r16, r17 ; shl3addx r5, r6, r7 ; st1 r25, r26 }
	{ subx r15, r16, r17 ; shrs r5, r6, r7 ; st1 r25, r26 }
	{ subx r15, r16, r17 ; shru r5, r6, r7 ; st4 r25, r26 }
	{ subx r15, r16, r17 ; st r25, r26 ; cmpne r5, r6, r7 }
	{ subx r15, r16, r17 ; st r25, r26 ; subx r5, r6, r7 }
	{ subx r15, r16, r17 ; st1 r25, r26 ; mulx r5, r6, r7 }
	{ subx r15, r16, r17 ; st2 r25, r26 ; cmpeqi r5, r6, 5 }
	{ subx r15, r16, r17 ; st2 r25, r26 ; shli r5, r6, 5 }
	{ subx r15, r16, r17 ; st4 r25, r26 ; mul_lu_lu r5, r6, r7 }
	{ subx r15, r16, r17 ; sub r5, r6, r7 ; ld2u r25, r26 }
	{ subx r15, r16, r17 ; tblidxb0 r5, r6 ; ld4s r25, r26 }
	{ subx r15, r16, r17 ; tblidxb2 r5, r6 ; prefetch r25 }
	{ subx r15, r16, r17 ; v1cmplts r5, r6, r7 }
	{ subx r15, r16, r17 ; v2avgs r5, r6, r7 }
	{ subx r15, r16, r17 ; v4addsc r5, r6, r7 }
	{ subx r5, r6, r7 ; add r15, r16, r17 ; prefetch_l2 r25 }
	{ subx r5, r6, r7 ; addx r15, r16, r17 ; prefetch_l2_fault r25 }
	{ subx r5, r6, r7 ; and r15, r16, r17 ; prefetch_l2_fault r25 }
	{ subx r5, r6, r7 ; cmpeq r15, r16, r17 ; prefetch_l3_fault r25 }
	{ subx r5, r6, r7 ; cmples r15, r16, r17 ; prefetch_l3_fault r25 }
	{ subx r5, r6, r7 ; cmplts r15, r16, r17 ; st1 r25, r26 }
	{ subx r5, r6, r7 ; cmpltu r15, r16, r17 ; st4 r25, r26 }
	{ subx r5, r6, r7 ; fnop ; ld1s r25, r26 }
	{ subx r5, r6, r7 ; info 19 ; ld1u r25, r26 }
	{ subx r5, r6, r7 ; jalrp r15 ; ld1s r25, r26 }
	{ subx r5, r6, r7 ; jrp r15 ; ld2s r25, r26 }
	{ subx r5, r6, r7 ; ld r25, r26 ; move r15, r16 }
	{ subx r5, r6, r7 ; ld1s r25, r26 ; ill }
	{ subx r5, r6, r7 ; ld1u r25, r26 ; cmpeq r15, r16, r17 }
	{ subx r5, r6, r7 ; ld1u r25, r26 }
	{ subx r5, r6, r7 ; ld2s r25, r26 ; shl3addx r15, r16, r17 }
	{ subx r5, r6, r7 ; ld2u r25, r26 ; or r15, r16, r17 }
	{ subx r5, r6, r7 ; ld4s r25, r26 ; jr r15 }
	{ subx r5, r6, r7 ; ld4u r25, r26 ; cmplts r15, r16, r17 }
	{ subx r5, r6, r7 ; ldna_add r15, r16, 5 }
	{ subx r5, r6, r7 ; mnz r15, r16, r17 ; ld2u r25, r26 }
	{ subx r5, r6, r7 ; movei r15, 5 ; ld4u r25, r26 }
	{ subx r5, r6, r7 ; nop ; ld4u r25, r26 }
	{ subx r5, r6, r7 ; or r15, r16, r17 ; prefetch_l1 r25 }
	{ subx r5, r6, r7 ; prefetch r25 ; nor r15, r16, r17 }
	{ subx r5, r6, r7 ; prefetch_l1 r25 ; cmpne r15, r16, r17 }
	{ subx r5, r6, r7 ; prefetch_l1_fault r25 ; andi r15, r16, 5 }
	{ subx r5, r6, r7 ; prefetch_l1_fault r25 ; xor r15, r16, r17 }
	{ subx r5, r6, r7 ; prefetch_l2 r25 ; shl3addx r15, r16, r17 }
	{ subx r5, r6, r7 ; prefetch_l2_fault r25 ; rotl r15, r16, r17 }
	{ subx r5, r6, r7 ; prefetch_l3 r25 ; lnk r15 }
	{ subx r5, r6, r7 ; prefetch_l3_fault r25 ; cmpne r15, r16, r17 }
	{ subx r5, r6, r7 ; rotl r15, r16, r17 ; ld4s r25, r26 }
	{ subx r5, r6, r7 ; shl r15, r16, r17 ; prefetch r25 }
	{ subx r5, r6, r7 ; shl1addx r15, r16, r17 ; prefetch_l1 r25 }
	{ subx r5, r6, r7 ; shl2addx r15, r16, r17 ; prefetch_l2 r25 }
	{ subx r5, r6, r7 ; shl3addx r15, r16, r17 ; prefetch_l3 r25 }
	{ subx r5, r6, r7 ; shrs r15, r16, r17 ; prefetch_l3 r25 }
	{ subx r5, r6, r7 ; shru r15, r16, r17 ; st r25, r26 }
	{ subx r5, r6, r7 ; st r25, r26 ; cmpne r15, r16, r17 }
	{ subx r5, r6, r7 ; st1 r25, r26 ; andi r15, r16, 5 }
	{ subx r5, r6, r7 ; st1 r25, r26 ; xor r15, r16, r17 }
	{ subx r5, r6, r7 ; st2 r25, r26 ; shl3add r15, r16, r17 }
	{ subx r5, r6, r7 ; st4 r25, r26 ; nor r15, r16, r17 }
	{ subx r5, r6, r7 ; sub r15, r16, r17 ; prefetch_l2 r25 }
	{ subx r5, r6, r7 ; v1cmpne r15, r16, r17 }
	{ subx r5, r6, r7 ; v2shl r15, r16, r17 }
	{ subx r5, r6, r7 ; xori r15, r16, 5 }
	{ subxsc r15, r16, r17 ; fdouble_addsub r5, r6, r7 }
	{ subxsc r15, r16, r17 ; mula_ls_lu r5, r6, r7 }
	{ subxsc r15, r16, r17 ; v1addi r5, r6, 5 }
	{ subxsc r15, r16, r17 ; v1shru r5, r6, r7 }
	{ subxsc r15, r16, r17 ; v2shlsc r5, r6, r7 }
	{ subxsc r5, r6, r7 ; dblalign2 r15, r16, r17 }
	{ subxsc r5, r6, r7 ; ld4u_add r15, r16, 5 }
	{ subxsc r5, r6, r7 ; prefetch_l2 r15 }
	{ subxsc r5, r6, r7 ; sub r15, r16, r17 }
	{ subxsc r5, r6, r7 ; v2cmpltu r15, r16, r17 }
	{ swint3 ; nop }
	{ tblidxb0 r5, r6 ; addx r15, r16, r17 ; ld r25, r26 }
	{ tblidxb0 r5, r6 ; and r15, r16, r17 ; ld r25, r26 }
	{ tblidxb0 r5, r6 ; cmpeq r15, r16, r17 ; ld1u r25, r26 }
	{ tblidxb0 r5, r6 ; cmples r15, r16, r17 ; ld1u r25, r26 }
	{ tblidxb0 r5, r6 ; cmplts r15, r16, r17 ; ld2u r25, r26 }
	{ tblidxb0 r5, r6 ; cmpltu r15, r16, r17 ; ld4u r25, r26 }
	{ tblidxb0 r5, r6 ; fetchadd4 r15, r16, r17 }
	{ tblidxb0 r5, r6 ; ill ; prefetch_l2 r25 }
	{ tblidxb0 r5, r6 ; jalr r15 ; prefetch_l1_fault r25 }
	{ tblidxb0 r5, r6 ; jr r15 ; prefetch_l2_fault r25 }
	{ tblidxb0 r5, r6 ; ld r25, r26 ; cmpltu r15, r16, r17 }
	{ tblidxb0 r5, r6 ; ld1s r25, r26 ; and r15, r16, r17 }
	{ tblidxb0 r5, r6 ; ld1s r25, r26 ; subx r15, r16, r17 }
	{ tblidxb0 r5, r6 ; ld1u r25, r26 ; shl2addx r15, r16, r17 }
	{ tblidxb0 r5, r6 ; ld2s r25, r26 ; nop }
	{ tblidxb0 r5, r6 ; ld2u r25, r26 ; jalr r15 }
	{ tblidxb0 r5, r6 ; ld4s r25, r26 ; cmples r15, r16, r17 }
	{ tblidxb0 r5, r6 ; ld4u r15, r16 }
	{ tblidxb0 r5, r6 ; ld4u r25, r26 ; shrs r15, r16, r17 }
	{ tblidxb0 r5, r6 ; lnk r15 ; st r25, r26 }
	{ tblidxb0 r5, r6 ; move r15, r16 ; st r25, r26 }
	{ tblidxb0 r5, r6 ; mz r15, r16, r17 ; st r25, r26 }
	{ tblidxb0 r5, r6 ; nor r15, r16, r17 ; st2 r25, r26 }
	{ tblidxb0 r5, r6 ; prefetch r25 ; info 19 }
	{ tblidxb0 r5, r6 ; prefetch_l1 r25 ; addx r15, r16, r17 }
	{ tblidxb0 r5, r6 ; prefetch_l1 r25 ; shrui r15, r16, 5 }
	{ tblidxb0 r5, r6 ; prefetch_l1_fault r25 ; shl2add r15, r16, r17 }
	{ tblidxb0 r5, r6 ; prefetch_l2 r25 ; nop }
	{ tblidxb0 r5, r6 ; prefetch_l2_fault r25 ; jalrp r15 }
	{ tblidxb0 r5, r6 ; prefetch_l3 r25 ; cmplts r15, r16, r17 }
	{ tblidxb0 r5, r6 ; prefetch_l3_fault r25 ; addx r15, r16, r17 }
	{ tblidxb0 r5, r6 ; prefetch_l3_fault r25 ; shrui r15, r16, 5 }
	{ tblidxb0 r5, r6 ; rotli r15, r16, 5 ; st1 r25, r26 }
	{ tblidxb0 r5, r6 ; shl1add r15, r16, r17 ; st2 r25, r26 }
	{ tblidxb0 r5, r6 ; shl2add r15, r16, r17 }
	{ tblidxb0 r5, r6 ; shl3addx r15, r16, r17 ; ld1s r25, r26 }
	{ tblidxb0 r5, r6 ; shrs r15, r16, r17 ; ld1s r25, r26 }
	{ tblidxb0 r5, r6 ; shru r15, r16, r17 ; ld2s r25, r26 }
	{ tblidxb0 r5, r6 ; st r25, r26 ; addx r15, r16, r17 }
	{ tblidxb0 r5, r6 ; st r25, r26 ; shrui r15, r16, 5 }
	{ tblidxb0 r5, r6 ; st1 r25, r26 ; shl2add r15, r16, r17 }
	{ tblidxb0 r5, r6 ; st2 r25, r26 ; mz r15, r16, r17 }
	{ tblidxb0 r5, r6 ; st4 r25, r26 ; info 19 }
	{ tblidxb0 r5, r6 ; stnt_add r15, r16, 5 }
	{ tblidxb0 r5, r6 ; v1add r15, r16, r17 }
	{ tblidxb0 r5, r6 ; v2int_h r15, r16, r17 }
	{ tblidxb0 r5, r6 ; xor r15, r16, r17 ; prefetch_l1 r25 }
	{ tblidxb1 r5, r6 ; addi r15, r16, 5 ; prefetch_l1_fault r25 }
	{ tblidxb1 r5, r6 ; addxi r15, r16, 5 ; prefetch_l2 r25 }
	{ tblidxb1 r5, r6 ; andi r15, r16, 5 ; prefetch_l2 r25 }
	{ tblidxb1 r5, r6 ; cmpeqi r15, r16, 5 ; prefetch_l3 r25 }
	{ tblidxb1 r5, r6 ; cmpleu r15, r16, r17 ; prefetch_l3 r25 }
	{ tblidxb1 r5, r6 ; cmpltsi r15, r16, 5 ; st r25, r26 }
	{ tblidxb1 r5, r6 ; cmpne r15, r16, r17 ; st1 r25, r26 }
	{ tblidxb1 r5, r6 ; icoh r15 }
	{ tblidxb1 r5, r6 ; inv r15 }
	{ tblidxb1 r5, r6 ; jr r15 ; ld r25, r26 }
	{ tblidxb1 r5, r6 ; ld r25, r26 ; addi r15, r16, 5 }
	{ tblidxb1 r5, r6 ; ld r25, r26 ; shru r15, r16, r17 }
	{ tblidxb1 r5, r6 ; ld1s r25, r26 ; shl1addx r15, r16, r17 }
	{ tblidxb1 r5, r6 ; ld1u r25, r26 ; movei r15, 5 }
	{ tblidxb1 r5, r6 ; ld2s r25, r26 ; ill }
	{ tblidxb1 r5, r6 ; ld2u r25, r26 ; cmpeq r15, r16, r17 }
	{ tblidxb1 r5, r6 ; ld2u r25, r26 }
	{ tblidxb1 r5, r6 ; ld4s r25, r26 ; shl3addx r15, r16, r17 }
	{ tblidxb1 r5, r6 ; ld4u r25, r26 ; or r15, r16, r17 }
	{ tblidxb1 r5, r6 ; lnk r15 ; ld2s r25, r26 }
	{ tblidxb1 r5, r6 ; move r15, r16 ; ld2s r25, r26 }
	{ tblidxb1 r5, r6 ; mz r15, r16, r17 ; ld2s r25, r26 }
	{ tblidxb1 r5, r6 ; nor r15, r16, r17 ; ld4s r25, r26 }
	{ tblidxb1 r5, r6 ; prefetch r25 ; andi r15, r16, 5 }
	{ tblidxb1 r5, r6 ; prefetch r25 ; xor r15, r16, r17 }
	{ tblidxb1 r5, r6 ; prefetch_l1 r25 ; shl r15, r16, r17 }
	{ tblidxb1 r5, r6 ; prefetch_l1_fault r25 ; move r15, r16 }
	{ tblidxb1 r5, r6 ; prefetch_l2 r25 ; ill }
	{ tblidxb1 r5, r6 ; prefetch_l2_fault r25 ; cmpeqi r15, r16, 5 }
	{ tblidxb1 r5, r6 ; prefetch_l3 r15 }
	{ tblidxb1 r5, r6 ; prefetch_l3 r25 ; shrs r15, r16, r17 }
	{ tblidxb1 r5, r6 ; prefetch_l3_fault r25 ; shl r15, r16, r17 }
	{ tblidxb1 r5, r6 ; rotli r15, r16, 5 ; ld2u r25, r26 }
	{ tblidxb1 r5, r6 ; shl1add r15, r16, r17 ; ld4s r25, r26 }
	{ tblidxb1 r5, r6 ; shl2add r15, r16, r17 ; prefetch r25 }
	{ tblidxb1 r5, r6 ; shl3add r15, r16, r17 ; prefetch_l1_fault r25 }
	{ tblidxb1 r5, r6 ; shli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ tblidxb1 r5, r6 ; shrsi r15, r16, 5 ; prefetch_l2_fault r25 }
	{ tblidxb1 r5, r6 ; shrui r15, r16, 5 ; prefetch_l3_fault r25 }
	{ tblidxb1 r5, r6 ; st r25, r26 ; shl r15, r16, r17 }
	{ tblidxb1 r5, r6 ; st1 r25, r26 ; move r15, r16 }
	{ tblidxb1 r5, r6 ; st2 r25, r26 ; fnop }
	{ tblidxb1 r5, r6 ; st4 r25, r26 ; andi r15, r16, 5 }
	{ tblidxb1 r5, r6 ; st4 r25, r26 ; xor r15, r16, r17 }
	{ tblidxb1 r5, r6 ; subx r15, r16, r17 ; prefetch_l1_fault r25 }
	{ tblidxb1 r5, r6 ; v2addi r15, r16, 5 }
	{ tblidxb1 r5, r6 ; v4sub r15, r16, r17 }
	{ tblidxb2 r5, r6 ; add r15, r16, r17 ; st4 r25, r26 }
	{ tblidxb2 r5, r6 ; addx r15, r16, r17 }
	{ tblidxb2 r5, r6 ; and r15, r16, r17 }
	{ tblidxb2 r5, r6 ; cmpeqi r15, r16, 5 ; ld1s r25, r26 }
	{ tblidxb2 r5, r6 ; cmpleu r15, r16, r17 ; ld1s r25, r26 }
	{ tblidxb2 r5, r6 ; cmpltsi r15, r16, 5 ; ld2s r25, r26 }
	{ tblidxb2 r5, r6 ; cmpne r15, r16, r17 ; ld2u r25, r26 }
	{ tblidxb2 r5, r6 ; fnop ; prefetch_l1 r25 }
	{ tblidxb2 r5, r6 ; info 19 ; prefetch_l1_fault r25 }
	{ tblidxb2 r5, r6 ; jalrp r15 ; prefetch_l1 r25 }
	{ tblidxb2 r5, r6 ; jrp r15 ; prefetch_l2 r25 }
	{ tblidxb2 r5, r6 ; ld r25, r26 ; rotli r15, r16, 5 }
	{ tblidxb2 r5, r6 ; ld1s r25, r26 ; mnz r15, r16, r17 }
	{ tblidxb2 r5, r6 ; ld1u r25, r26 ; cmpne r15, r16, r17 }
	{ tblidxb2 r5, r6 ; ld2s r25, r26 ; and r15, r16, r17 }
	{ tblidxb2 r5, r6 ; ld2s r25, r26 ; subx r15, r16, r17 }
	{ tblidxb2 r5, r6 ; ld2u r25, r26 ; shl2addx r15, r16, r17 }
	{ tblidxb2 r5, r6 ; ld4s r25, r26 ; nop }
	{ tblidxb2 r5, r6 ; ld4u r25, r26 ; jalr r15 }
	{ tblidxb2 r5, r6 ; ldnt2s_add r15, r16, 5 }
	{ tblidxb2 r5, r6 ; mnz r15, r16, r17 ; prefetch_l2_fault r25 }
	{ tblidxb2 r5, r6 ; movei r15, 5 ; prefetch_l3_fault r25 }
	{ tblidxb2 r5, r6 ; nop ; prefetch_l3_fault r25 }
	{ tblidxb2 r5, r6 ; or r15, r16, r17 ; st1 r25, r26 }
	{ tblidxb2 r5, r6 ; prefetch r25 ; shl2add r15, r16, r17 }
	{ tblidxb2 r5, r6 ; prefetch_l1 r25 ; jrp r15 }
	{ tblidxb2 r5, r6 ; prefetch_l1_fault r25 ; cmpltu r15, r16, r17 }
	{ tblidxb2 r5, r6 ; prefetch_l2 r25 ; and r15, r16, r17 }
	{ tblidxb2 r5, r6 ; prefetch_l2 r25 ; subx r15, r16, r17 }
	{ tblidxb2 r5, r6 ; prefetch_l2_fault r25 ; shl3add r15, r16, r17 }
	{ tblidxb2 r5, r6 ; prefetch_l3 r25 ; or r15, r16, r17 }
	{ tblidxb2 r5, r6 ; prefetch_l3_fault r25 ; jrp r15 }
	{ tblidxb2 r5, r6 ; rotl r15, r16, r17 ; prefetch_l3 r25 }
	{ tblidxb2 r5, r6 ; shl r15, r16, r17 ; st r25, r26 }
	{ tblidxb2 r5, r6 ; shl1addx r15, r16, r17 ; st1 r25, r26 }
	{ tblidxb2 r5, r6 ; shl2addx r15, r16, r17 ; st4 r25, r26 }
	{ tblidxb2 r5, r6 ; shli r15, r16, 5 ; ld r25, r26 }
	{ tblidxb2 r5, r6 ; shrsi r15, r16, 5 ; ld r25, r26 }
	{ tblidxb2 r5, r6 ; shrui r15, r16, 5 ; ld1u r25, r26 }
	{ tblidxb2 r5, r6 ; st r25, r26 ; jrp r15 }
	{ tblidxb2 r5, r6 ; st1 r25, r26 ; cmpltu r15, r16, r17 }
	{ tblidxb2 r5, r6 ; st2 r25, r26 ; addxi r15, r16, 5 }
	{ tblidxb2 r5, r6 ; st2 r25, r26 ; sub r15, r16, r17 }
	{ tblidxb2 r5, r6 ; st4 r25, r26 ; shl2add r15, r16, r17 }
	{ tblidxb2 r5, r6 ; sub r15, r16, r17 ; st4 r25, r26 }
	{ tblidxb2 r5, r6 ; v1mnz r15, r16, r17 }
	{ tblidxb2 r5, r6 ; v2sub r15, r16, r17 }
	{ tblidxb3 r5, r6 ; add r15, r16, r17 ; ld4u r25, r26 }
	{ tblidxb3 r5, r6 ; addx r15, r16, r17 ; prefetch r25 }
	{ tblidxb3 r5, r6 ; and r15, r16, r17 ; prefetch r25 }
	{ tblidxb3 r5, r6 ; cmpeq r15, r16, r17 ; prefetch_l1_fault r25 }
	{ tblidxb3 r5, r6 ; cmples r15, r16, r17 ; prefetch_l1_fault r25 }
	{ tblidxb3 r5, r6 ; cmplts r15, r16, r17 ; prefetch_l2_fault r25 }
	{ tblidxb3 r5, r6 ; cmpltu r15, r16, r17 ; prefetch_l3_fault r25 }
	{ tblidxb3 r5, r6 ; finv r15 }
	{ tblidxb3 r5, r6 ; ill ; st4 r25, r26 }
	{ tblidxb3 r5, r6 ; jalr r15 ; st2 r25, r26 }
	{ tblidxb3 r5, r6 ; jr r15 }
	{ tblidxb3 r5, r6 ; ld r25, r26 ; jr r15 }
	{ tblidxb3 r5, r6 ; ld1s r25, r26 ; cmpltsi r15, r16, 5 }
	{ tblidxb3 r5, r6 ; ld1u r25, r26 ; addx r15, r16, r17 }
	{ tblidxb3 r5, r6 ; ld1u r25, r26 ; shrui r15, r16, 5 }
	{ tblidxb3 r5, r6 ; ld2s r25, r26 ; shl1addx r15, r16, r17 }
	{ tblidxb3 r5, r6 ; ld2u r25, r26 ; movei r15, 5 }
	{ tblidxb3 r5, r6 ; ld4s r25, r26 ; ill }
	{ tblidxb3 r5, r6 ; ld4u r25, r26 ; cmpeq r15, r16, r17 }
	{ tblidxb3 r5, r6 ; ld4u r25, r26 }
	{ tblidxb3 r5, r6 ; mnz r15, r16, r17 ; ld r25, r26 }
	{ tblidxb3 r5, r6 ; movei r15, 5 ; ld1u r25, r26 }
	{ tblidxb3 r5, r6 ; nop ; ld1u r25, r26 }
	{ tblidxb3 r5, r6 ; or r15, r16, r17 ; ld2u r25, r26 }
	{ tblidxb3 r5, r6 ; prefetch r25 ; move r15, r16 }
	{ tblidxb3 r5, r6 ; prefetch_l1 r25 ; cmpleu r15, r16, r17 }
	{ tblidxb3 r5, r6 ; prefetch_l1_fault r25 ; addi r15, r16, 5 }
	{ tblidxb3 r5, r6 ; prefetch_l1_fault r25 ; shru r15, r16, r17 }
	{ tblidxb3 r5, r6 ; prefetch_l2 r25 ; shl1addx r15, r16, r17 }
	{ tblidxb3 r5, r6 ; prefetch_l2_fault r25 ; mz r15, r16, r17 }
	{ tblidxb3 r5, r6 ; prefetch_l3 r25 ; jalr r15 }
	{ tblidxb3 r5, r6 ; prefetch_l3_fault r25 ; cmpleu r15, r16, r17 }
	{ tblidxb3 r5, r6 ; rotl r15, r16, r17 ; ld1s r25, r26 }
	{ tblidxb3 r5, r6 ; shl r15, r16, r17 ; ld2s r25, r26 }
	{ tblidxb3 r5, r6 ; shl1addx r15, r16, r17 ; ld2u r25, r26 }
	{ tblidxb3 r5, r6 ; shl2addx r15, r16, r17 ; ld4u r25, r26 }
	{ tblidxb3 r5, r6 ; shl3addx r15, r16, r17 ; prefetch_l1 r25 }
	{ tblidxb3 r5, r6 ; shrs r15, r16, r17 ; prefetch_l1 r25 }
	{ tblidxb3 r5, r6 ; shru r15, r16, r17 ; prefetch_l2 r25 }
	{ tblidxb3 r5, r6 ; st r25, r26 ; cmpleu r15, r16, r17 }
	{ tblidxb3 r5, r6 ; st1 r25, r26 ; addi r15, r16, 5 }
	{ tblidxb3 r5, r6 ; st1 r25, r26 ; shru r15, r16, r17 }
	{ tblidxb3 r5, r6 ; st2 r25, r26 ; shl1add r15, r16, r17 }
	{ tblidxb3 r5, r6 ; st4 r25, r26 ; move r15, r16 }
	{ tblidxb3 r5, r6 ; sub r15, r16, r17 ; ld4u r25, r26 }
	{ tblidxb3 r5, r6 ; v1cmplts r15, r16, r17 }
	{ tblidxb3 r5, r6 ; v2mz r15, r16, r17 }
	{ tblidxb3 r5, r6 ; xor r15, r16, r17 ; st1 r25, r26 }
	{ v1add r15, r16, r17 ; dblalign2 r5, r6, r7 }
	{ v1add r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ v1add r15, r16, r17 ; tblidxb1 r5, r6 }
	{ v1add r15, r16, r17 ; v1shl r5, r6, r7 }
	{ v1add r15, r16, r17 ; v2sads r5, r6, r7 }
	{ v1add r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ v1add r5, r6, r7 ; ld2u_add r15, r16, 5 }
	{ v1add r5, r6, r7 ; prefetch_add_l3 r15, 5 }
	{ v1add r5, r6, r7 ; stnt2_add r15, r16, 5 }
	{ v1add r5, r6, r7 ; v2cmples r15, r16, r17 }
	{ v1add r5, r6, r7 ; xori r15, r16, 5 }
	{ v1addi r15, r16, 5 ; fdouble_addsub r5, r6, r7 }
	{ v1addi r15, r16, 5 ; mula_ls_lu r5, r6, r7 }
	{ v1addi r15, r16, 5 ; v1addi r5, r6, 5 }
	{ v1addi r15, r16, 5 ; v1shru r5, r6, r7 }
	{ v1addi r15, r16, 5 ; v2shlsc r5, r6, r7 }
	{ v1addi r5, r6, 5 ; dblalign2 r15, r16, r17 }
	{ v1addi r5, r6, 5 ; ld4u_add r15, r16, 5 }
	{ v1addi r5, r6, 5 ; prefetch_l2 r15 }
	{ v1addi r5, r6, 5 ; sub r15, r16, r17 }
	{ v1addi r5, r6, 5 ; v2cmpltu r15, r16, r17 }
	{ v1adduc r15, r16, r17 ; addx r5, r6, r7 }
	{ v1adduc r15, r16, r17 ; fdouble_sub_flags r5, r6, r7 }
	{ v1adduc r15, r16, r17 ; mz r5, r6, r7 }
	{ v1adduc r15, r16, r17 ; v1cmpeq r5, r6, r7 }
	{ v1adduc r15, r16, r17 ; v2add r5, r6, r7 }
	{ v1adduc r15, r16, r17 ; v2shrui r5, r6, 5 }
	{ v1adduc r5, r6, r7 ; exch r15, r16, r17 }
	{ v1adduc r5, r6, r7 ; ldnt r15, r16 }
	{ v1adduc r5, r6, r7 ; raise }
	{ v1adduc r5, r6, r7 ; v1addi r15, r16, 5 }
	{ v1adduc r5, r6, r7 ; v2int_l r15, r16, r17 }
	{ v1adiffu r5, r6, r7 ; and r15, r16, r17 }
	{ v1adiffu r5, r6, r7 ; jrp r15 }
	{ v1adiffu r5, r6, r7 ; nop }
	{ v1adiffu r5, r6, r7 ; st2 r15, r16 }
	{ v1adiffu r5, r6, r7 ; v1shru r15, r16, r17 }
	{ v1adiffu r5, r6, r7 ; v4packsc r15, r16, r17 }
	{ v1avgu r5, r6, r7 ; fetchand r15, r16, r17 }
	{ v1avgu r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
	{ v1avgu r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ v1avgu r5, r6, r7 ; v1cmplts r15, r16, r17 }
	{ v1avgu r5, r6, r7 ; v2mz r15, r16, r17 }
	{ v1cmpeq r15, r16, r17 ; cmoveqz r5, r6, r7 }
	{ v1cmpeq r15, r16, r17 ; fsingle_sub1 r5, r6, r7 }
	{ v1cmpeq r15, r16, r17 ; shl r5, r6, r7 }
	{ v1cmpeq r15, r16, r17 ; v1ddotpua r5, r6, r7 }
	{ v1cmpeq r15, r16, r17 ; v2cmpltsi r5, r6, 5 }
	{ v1cmpeq r15, r16, r17 ; v4shrs r5, r6, r7 }
	{ v1cmpeq r5, r6, r7 ; finv r15 }
	{ v1cmpeq r5, r6, r7 ; ldnt4s_add r15, r16, 5 }
	{ v1cmpeq r5, r6, r7 ; shl3addx r15, r16, r17 }
	{ v1cmpeq r5, r6, r7 ; v1cmpne r15, r16, r17 }
	{ v1cmpeq r5, r6, r7 ; v2shl r15, r16, r17 }
	{ v1cmpeqi r15, r16, 5 ; cmples r5, r6, r7 }
	{ v1cmpeqi r15, r16, 5 ; mnz r5, r6, r7 }
	{ v1cmpeqi r15, r16, 5 ; shl2add r5, r6, r7 }
	{ v1cmpeqi r15, r16, 5 ; v1dotpa r5, r6, r7 }
	{ v1cmpeqi r15, r16, 5 ; v2dotp r5, r6, r7 }
	{ v1cmpeqi r15, r16, 5 ; xor r5, r6, r7 }
	{ v1cmpeqi r5, r6, 5 ; icoh r15 }
	{ v1cmpeqi r5, r6, 5 ; lnk r15 }
	{ v1cmpeqi r5, r6, 5 ; shrs r15, r16, r17 }
	{ v1cmpeqi r5, r6, 5 ; v1maxui r15, r16, 5 }
	{ v1cmpeqi r5, r6, 5 ; v2shrsi r15, r16, 5 }
	{ v1cmples r15, r16, r17 ; cmpltu r5, r6, r7 }
	{ v1cmples r15, r16, r17 ; mul_hs_hs r5, r6, r7 }
	{ v1cmples r15, r16, r17 ; shli r5, r6, 5 }
	{ v1cmples r15, r16, r17 ; v1dotpusa r5, r6, r7 }
	{ v1cmples r15, r16, r17 ; v2maxs r5, r6, r7 }
	{ v1cmples r5, r6, r7 ; addli r15, r16, 0x1234 }
	{ v1cmples r5, r6, r7 ; inv r15 }
	{ v1cmples r5, r6, r7 ; move r15, r16 }
	{ v1cmples r5, r6, r7 ; shrux r15, r16, r17 }
	{ v1cmples r5, r6, r7 ; v1mz r15, r16, r17 }
	{ v1cmples r5, r6, r7 ; v2subsc r15, r16, r17 }
	{ v1cmpleu r15, r16, r17 ; cmula r5, r6, r7 }
	{ v1cmpleu r15, r16, r17 ; mul_hu_hu r5, r6, r7 }
	{ v1cmpleu r15, r16, r17 ; shrsi r5, r6, 5 }
	{ v1cmpleu r15, r16, r17 ; v1maxui r5, r6, 5 }
	{ v1cmpleu r15, r16, r17 ; v2mnz r5, r6, r7 }
	{ v1cmpleu r5, r6, r7 ; addxsc r15, r16, r17 }
	{ v1cmpleu r5, r6, r7 ; jr r15 }
	{ v1cmpleu r5, r6, r7 ; mz r15, r16, r17 }
	{ v1cmpleu r5, r6, r7 ; st1_add r15, r16, 5 }
	{ v1cmpleu r5, r6, r7 ; v1shrsi r15, r16, 5 }
	{ v1cmpleu r5, r6, r7 ; v4int_l r15, r16, r17 }
	{ v1cmplts r15, r16, r17 ; cmulh r5, r6, r7 }
	{ v1cmplts r15, r16, r17 ; mul_ls_lu r5, r6, r7 }
	{ v1cmplts r15, r16, r17 ; shruxi r5, r6, 5 }
	{ v1cmplts r15, r16, r17 ; v1multu r5, r6, r7 }
	{ v1cmplts r15, r16, r17 ; v2mz r5, r6, r7 }
	{ v1cmplts r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ v1cmplts r5, r6, r7 ; ld1s_add r15, r16, 5 }
	{ v1cmplts r5, r6, r7 ; ori r15, r16, 5 }
	{ v1cmplts r5, r6, r7 ; st4_add r15, r16, 5 }
	{ v1cmplts r5, r6, r7 ; v1subuc r15, r16, r17 }
	{ v1cmplts r5, r6, r7 ; v4shrs r15, r16, r17 }
	{ v1cmpltsi r15, r16, 5 ; ctz r5, r6 }
	{ v1cmpltsi r15, r16, 5 ; mula_hs_ls r5, r6, r7 }
	{ v1cmpltsi r15, r16, 5 ; subxsc r5, r6, r7 }
	{ v1cmpltsi r15, r16, 5 ; v1sadau r5, r6, r7 }
	{ v1cmpltsi r15, r16, 5 ; v2sadas r5, r6, r7 }
	{ v1cmpltsi r5, r6, 5 ; cmpleu r15, r16, r17 }
	{ v1cmpltsi r5, r6, 5 ; ld2s_add r15, r16, 5 }
	{ v1cmpltsi r5, r6, 5 ; prefetch_add_l2 r15, 5 }
	{ v1cmpltsi r5, r6, 5 ; stnt1_add r15, r16, 5 }
	{ v1cmpltsi r5, r6, 5 ; v2cmpeq r15, r16, r17 }
	{ v1cmpltsi r5, r6, 5 ; wh64 r15 }
	{ v1cmpltu r15, r16, r17 ; dblalign6 r5, r6, r7 }
	{ v1cmpltu r15, r16, r17 ; mula_hu_lu r5, r6, r7 }
	{ v1cmpltu r15, r16, r17 ; tblidxb3 r5, r6 }
	{ v1cmpltu r15, r16, r17 ; v1shrs r5, r6, r7 }
	{ v1cmpltu r15, r16, r17 ; v2shl r5, r6, r7 }
	{ v1cmpltu r5, r6, r7 ; cmpltui r15, r16, 5 }
	{ v1cmpltu r5, r6, r7 ; ld4s_add r15, r16, 5 }
	{ v1cmpltu r5, r6, r7 ; prefetch_l1 r15 }
	{ v1cmpltu r5, r6, r7 ; stnt4_add r15, r16, 5 }
	{ v1cmpltu r5, r6, r7 ; v2cmplts r15, r16, r17 }
	{ v1cmpltui r15, r16, 5 ; addi r5, r6, 5 }
	{ v1cmpltui r15, r16, 5 ; fdouble_pack1 r5, r6, r7 }
	{ v1cmpltui r15, r16, 5 ; mulax r5, r6, r7 }
	{ v1cmpltui r15, r16, 5 ; v1adiffu r5, r6, r7 }
	{ v1cmpltui r15, r16, 5 ; v1sub r5, r6, r7 }
	{ v1cmpltui r15, r16, 5 ; v2shrsi r5, r6, 5 }
	{ v1cmpltui r5, r6, 5 ; dblalign6 r15, r16, r17 }
	{ v1cmpltui r5, r6, 5 ; ldna r15, r16 }
	{ v1cmpltui r5, r6, 5 ; prefetch_l3 r15 }
	{ v1cmpltui r5, r6, 5 ; subxsc r15, r16, r17 }
	{ v1cmpltui r5, r6, 5 ; v2cmpne r15, r16, r17 }
	{ v1cmpne r15, r16, r17 ; addxli r5, r6, 0x1234 }
	{ v1cmpne r15, r16, r17 ; fdouble_unpack_min r5, r6, r7 }
	{ v1cmpne r15, r16, r17 ; nor r5, r6, r7 }
	{ v1cmpne r15, r16, r17 ; v1cmples r5, r6, r7 }
	{ v1cmpne r15, r16, r17 ; v2addsc r5, r6, r7 }
	{ v1cmpne r15, r16, r17 ; v2subsc r5, r6, r7 }
	{ v1cmpne r5, r6, r7 ; fetchadd r15, r16, r17 }
	{ v1cmpne r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
	{ v1cmpne r5, r6, r7 ; rotli r15, r16, 5 }
	{ v1cmpne r5, r6, r7 ; v1cmpeq r15, r16, r17 }
	{ v1cmpne r5, r6, r7 ; v2maxsi r15, r16, 5 }
	{ v1ddotpu r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ v1ddotpu r5, r6, r7 ; ld1s r15, r16 }
	{ v1ddotpu r5, r6, r7 ; or r15, r16, r17 }
	{ v1ddotpu r5, r6, r7 ; st4 r15, r16 }
	{ v1ddotpu r5, r6, r7 ; v1sub r15, r16, r17 }
	{ v1ddotpu r5, r6, r7 ; v4shlsc r15, r16, r17 }
	{ v1ddotpua r5, r6, r7 ; fetchor r15, r16, r17 }
	{ v1ddotpua r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
	{ v1ddotpua r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ v1ddotpua r5, r6, r7 ; v1cmpltu r15, r16, r17 }
	{ v1ddotpua r5, r6, r7 ; v2packl r15, r16, r17 }
	{ v1ddotpus r5, r6, r7 ; cmplts r15, r16, r17 }
	{ v1ddotpus r5, r6, r7 ; ld2u r15, r16 }
	{ v1ddotpus r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
	{ v1ddotpus r5, r6, r7 ; stnt2 r15, r16 }
	{ v1ddotpus r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
	{ v1ddotpus r5, r6, r7 ; xor r15, r16, r17 }
	{ v1ddotpusa r5, r6, r7 ; icoh r15 }
	{ v1ddotpusa r5, r6, r7 ; lnk r15 }
	{ v1ddotpusa r5, r6, r7 ; shrs r15, r16, r17 }
	{ v1ddotpusa r5, r6, r7 ; v1maxui r15, r16, 5 }
	{ v1ddotpusa r5, r6, r7 ; v2shrsi r15, r16, 5 }
	{ v1dotp r5, r6, r7 ; dblalign4 r15, r16, r17 }
	{ v1dotp r5, r6, r7 ; ld_add r15, r16, 5 }
	{ v1dotp r5, r6, r7 ; prefetch_l2_fault r15 }
	{ v1dotp r5, r6, r7 ; subx r15, r16, r17 }
	{ v1dotp r5, r6, r7 ; v2cmpltui r15, r16, 5 }
	{ v1dotpa r5, r6, r7 ; addxi r15, r16, 5 }
	{ v1dotpa r5, r6, r7 ; jalr r15 }
	{ v1dotpa r5, r6, r7 ; moveli r15, 0x1234 }
	{ v1dotpa r5, r6, r7 ; st r15, r16 }
	{ v1dotpa r5, r6, r7 ; v1shli r15, r16, 5 }
	{ v1dotpa r5, r6, r7 ; v4addsc r15, r16, r17 }
	{ v1dotpu r5, r6, r7 ; fetchadd4 r15, r16, r17 }
	{ v1dotpu r5, r6, r7 ; ldnt1u r15, r16 }
	{ v1dotpu r5, r6, r7 ; shl r15, r16, r17 }
	{ v1dotpu r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
	{ v1dotpu r5, r6, r7 ; v2mins r15, r16, r17 }
	{ v1dotpua r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ v1dotpua r5, r6, r7 ; ld1s_add r15, r16, 5 }
	{ v1dotpua r5, r6, r7 ; ori r15, r16, 5 }
	{ v1dotpua r5, r6, r7 ; st4_add r15, r16, 5 }
	{ v1dotpua r5, r6, r7 ; v1subuc r15, r16, r17 }
	{ v1dotpua r5, r6, r7 ; v4shrs r15, r16, r17 }
	{ v1dotpus r5, r6, r7 ; fetchor4 r15, r16, r17 }
	{ v1dotpus r5, r6, r7 ; ldnt4s r15, r16 }
	{ v1dotpus r5, r6, r7 ; shl3add r15, r16, r17 }
	{ v1dotpus r5, r6, r7 ; v1cmpltui r15, r16, 5 }
	{ v1dotpus r5, r6, r7 ; v2packuc r15, r16, r17 }
	{ v1dotpusa r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ v1dotpusa r5, r6, r7 ; ld2u_add r15, r16, 5 }
	{ v1dotpusa r5, r6, r7 ; prefetch_add_l3 r15, 5 }
	{ v1dotpusa r5, r6, r7 ; stnt2_add r15, r16, 5 }
	{ v1dotpusa r5, r6, r7 ; v2cmples r15, r16, r17 }
	{ v1dotpusa r5, r6, r7 ; xori r15, r16, 5 }
	{ v1int_h r15, r16, r17 ; fdouble_addsub r5, r6, r7 }
	{ v1int_h r15, r16, r17 ; mula_ls_lu r5, r6, r7 }
	{ v1int_h r15, r16, r17 ; v1addi r5, r6, 5 }
	{ v1int_h r15, r16, r17 ; v1shru r5, r6, r7 }
	{ v1int_h r15, r16, r17 ; v2shlsc r5, r6, r7 }
	{ v1int_h r5, r6, r7 ; dblalign2 r15, r16, r17 }
	{ v1int_h r5, r6, r7 ; ld4u_add r15, r16, 5 }
	{ v1int_h r5, r6, r7 ; prefetch_l2 r15 }
	{ v1int_h r5, r6, r7 ; sub r15, r16, r17 }
	{ v1int_h r5, r6, r7 ; v2cmpltu r15, r16, r17 }
	{ v1int_l r15, r16, r17 ; addx r5, r6, r7 }
	{ v1int_l r15, r16, r17 ; fdouble_sub_flags r5, r6, r7 }
	{ v1int_l r15, r16, r17 ; mz r5, r6, r7 }
	{ v1int_l r15, r16, r17 ; v1cmpeq r5, r6, r7 }
	{ v1int_l r15, r16, r17 ; v2add r5, r6, r7 }
	{ v1int_l r15, r16, r17 ; v2shrui r5, r6, 5 }
	{ v1int_l r5, r6, r7 ; exch r15, r16, r17 }
	{ v1int_l r5, r6, r7 ; ldnt r15, r16 }
	{ v1int_l r5, r6, r7 ; raise }
	{ v1int_l r5, r6, r7 ; v1addi r15, r16, 5 }
	{ v1int_l r5, r6, r7 ; v2int_l r15, r16, r17 }
	{ v1maxu r15, r16, r17 ; and r5, r6, r7 }
	{ v1maxu r15, r16, r17 ; fsingle_add1 r5, r6, r7 }
	{ v1maxu r15, r16, r17 ; ori r5, r6, 5 }
	{ v1maxu r15, r16, r17 ; v1cmplts r5, r6, r7 }
	{ v1maxu r15, r16, r17 ; v2avgs r5, r6, r7 }
	{ v1maxu r15, r16, r17 ; v4addsc r5, r6, r7 }
	{ v1maxu r5, r6, r7 ; fetchaddgez r15, r16, r17 }
	{ v1maxu r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
	{ v1maxu r5, r6, r7 ; shl16insli r15, r16, 0x1234 }
	{ v1maxu r5, r6, r7 ; v1cmples r15, r16, r17 }
	{ v1maxu r5, r6, r7 ; v2minsi r15, r16, 5 }
	{ v1maxui r15, r16, 5 ; bfins r5, r6, 5, 7 }
	{ v1maxui r15, r16, 5 ; fsingle_pack1 r5, r6 }
	{ v1maxui r15, r16, 5 ; rotl r5, r6, r7 }
	{ v1maxui r15, r16, 5 ; v1cmpne r5, r6, r7 }
	{ v1maxui r15, r16, 5 ; v2cmpleu r5, r6, r7 }
	{ v1maxui r15, r16, 5 ; v4shl r5, r6, r7 }
	{ v1maxui r5, r6, 5 ; fetchor r15, r16, r17 }
	{ v1maxui r5, r6, 5 ; ldnt2u_add r15, r16, 5 }
	{ v1maxui r5, r6, 5 ; shl2addx r15, r16, r17 }
	{ v1maxui r5, r6, 5 ; v1cmpltu r15, r16, r17 }
	{ v1maxui r5, r6, 5 ; v2packl r15, r16, r17 }
	{ v1minu r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ v1minu r15, r16, r17 ; infol 0x1234 }
	{ v1minu r15, r16, r17 ; shl1add r5, r6, r7 }
	{ v1minu r15, r16, r17 ; v1ddotpusa r5, r6, r7 }
	{ v1minu r15, r16, r17 ; v2cmpltui r5, r6, 5 }
	{ v1minu r15, r16, r17 ; v4sub r5, r6, r7 }
	{ v1minu r5, r6, r7 ; flushwb }
	{ v1minu r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
	{ v1minu r5, r6, r7 ; shlx r15, r16, r17 }
	{ v1minu r5, r6, r7 ; v1int_l r15, r16, r17 }
	{ v1minu r5, r6, r7 ; v2shlsc r15, r16, r17 }
	{ v1minui r15, r16, 5 ; cmplts r5, r6, r7 }
	{ v1minui r15, r16, 5 ; movei r5, 5 }
	{ v1minui r15, r16, 5 ; shl3add r5, r6, r7 }
	{ v1minui r15, r16, 5 ; v1dotpua r5, r6, r7 }
	{ v1minui r15, r16, 5 ; v2int_h r5, r6, r7 }
	{ v1minui r5, r6, 5 ; add r15, r16, r17 }
	{ v1minui r5, r6, 5 ; info 19 }
	{ v1minui r5, r6, 5 ; mfspr r16, 0x5 }
	{ v1minui r5, r6, 5 ; shru r15, r16, r17 }
	{ v1minui r5, r6, 5 ; v1minui r15, r16, 5 }
	{ v1minui r5, r6, 5 ; v2shrui r15, r16, 5 }
	{ v1mnz r15, r16, r17 ; cmpne r5, r6, r7 }
	{ v1mnz r15, r16, r17 ; mul_hs_ls r5, r6, r7 }
	{ v1mnz r15, r16, r17 ; shlxi r5, r6, 5 }
	{ v1mnz r15, r16, r17 ; v1int_l r5, r6, r7 }
	{ v1mnz r15, r16, r17 ; v2mins r5, r6, r7 }
	{ v1mnz r5, r6, r7 ; addxi r15, r16, 5 }
	{ v1mnz r5, r6, r7 ; jalr r15 }
	{ v1mnz r5, r6, r7 ; moveli r15, 0x1234 }
	{ v1mnz r5, r6, r7 ; st r15, r16 }
	{ v1mnz r5, r6, r7 ; v1shli r15, r16, 5 }
	{ v1mnz r5, r6, r7 ; v4addsc r15, r16, r17 }
	{ v1multu r5, r6, r7 ; fetchadd4 r15, r16, r17 }
	{ v1multu r5, r6, r7 ; ldnt1u r15, r16 }
	{ v1multu r5, r6, r7 ; shl r15, r16, r17 }
	{ v1multu r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
	{ v1multu r5, r6, r7 ; v2mins r15, r16, r17 }
	{ v1mulu r5, r6, r7 ; cmpeqi r15, r16, 5 }
	{ v1mulu r5, r6, r7 ; ld1s_add r15, r16, 5 }
	{ v1mulu r5, r6, r7 ; ori r15, r16, 5 }
	{ v1mulu r5, r6, r7 ; st4_add r15, r16, 5 }
	{ v1mulu r5, r6, r7 ; v1subuc r15, r16, r17 }
	{ v1mulu r5, r6, r7 ; v4shrs r15, r16, r17 }
	{ v1mulus r5, r6, r7 ; fetchor4 r15, r16, r17 }
	{ v1mulus r5, r6, r7 ; ldnt4s r15, r16 }
	{ v1mulus r5, r6, r7 ; shl3add r15, r16, r17 }
	{ v1mulus r5, r6, r7 ; v1cmpltui r15, r16, 5 }
	{ v1mulus r5, r6, r7 ; v2packuc r15, r16, r17 }
	{ v1mz r15, r16, r17 ; cmpeqi r5, r6, 5 }
	{ v1mz r15, r16, r17 ; mm r5, r6, 5, 7 }
	{ v1mz r15, r16, r17 ; shl1addx r5, r6, r7 }
	{ v1mz r15, r16, r17 ; v1dotp r5, r6, r7 }
	{ v1mz r15, r16, r17 ; v2cmpne r5, r6, r7 }
	{ v1mz r15, r16, r17 ; v4subsc r5, r6, r7 }
	{ v1mz r5, r6, r7 ; fnop }
	{ v1mz r5, r6, r7 ; ldnt_add r15, r16, 5 }
	{ v1mz r5, r6, r7 ; shlxi r15, r16, 5 }
	{ v1mz r5, r6, r7 ; v1maxu r15, r16, r17 }
	{ v1mz r5, r6, r7 ; v2shrs r15, r16, r17 }
	{ v1sadau r5, r6, r7 ; dblalign2 r15, r16, r17 }
	{ v1sadau r5, r6, r7 ; ld4u_add r15, r16, 5 }
	{ v1sadau r5, r6, r7 ; prefetch_l2 r15 }
	{ v1sadau r5, r6, r7 ; sub r15, r16, r17 }
	{ v1sadau r5, r6, r7 ; v2cmpltu r15, r16, r17 }
	{ v1sadu r5, r6, r7 ; addx r15, r16, r17 }
	{ v1sadu r5, r6, r7 ; iret }
	{ v1sadu r5, r6, r7 ; movei r15, 5 }
	{ v1sadu r5, r6, r7 ; shruxi r15, r16, 5 }
	{ v1sadu r5, r6, r7 ; v1shl r15, r16, r17 }
	{ v1sadu r5, r6, r7 ; v4add r15, r16, r17 }
	{ v1shl r15, r16, r17 ; cmulaf r5, r6, r7 }
	{ v1shl r15, r16, r17 ; mul_hu_ls r5, r6, r7 }
	{ v1shl r15, r16, r17 ; shru r5, r6, r7 }
	{ v1shl r15, r16, r17 ; v1minu r5, r6, r7 }
	{ v1shl r15, r16, r17 ; v2mulfsc r5, r6, r7 }
	{ v1shl r5, r6, r7 ; and r15, r16, r17 }
	{ v1shl r5, r6, r7 ; jrp r15 }
	{ v1shl r5, r6, r7 ; nop }
	{ v1shl r5, r6, r7 ; st2 r15, r16 }
	{ v1shl r5, r6, r7 ; v1shru r15, r16, r17 }
	{ v1shl r5, r6, r7 ; v4packsc r15, r16, r17 }
	{ v1shli r15, r16, 5 ; cmulhr r5, r6, r7 }
	{ v1shli r15, r16, 5 ; mul_lu_lu r5, r6, r7 }
	{ v1shli r15, r16, 5 ; shufflebytes r5, r6, r7 }
	{ v1shli r15, r16, 5 ; v1mulu r5, r6, r7 }
	{ v1shli r15, r16, 5 ; v2packh r5, r6, r7 }
	{ v1shli r5, r6, 5 ; cmpexch r15, r16, r17 }
	{ v1shli r5, r6, 5 ; ld1u r15, r16 }
	{ v1shli r5, r6, 5 ; prefetch r15 }
	{ v1shli r5, r6, 5 ; st_add r15, r16, 5 }
	{ v1shli r5, r6, 5 ; v2add r15, r16, r17 }
	{ v1shli r5, r6, 5 ; v4shru r15, r16, r17 }
	{ v1shrs r15, r16, r17 ; dblalign r5, r6, r7 }
	{ v1shrs r15, r16, r17 ; mula_hs_lu r5, r6, r7 }
	{ v1shrs r15, r16, r17 ; tblidxb0 r5, r6 }
	{ v1shrs r15, r16, r17 ; v1sadu r5, r6, r7 }
	{ v1shrs r15, r16, r17 ; v2sadau r5, r6, r7 }
	{ v1shrs r5, r6, r7 ; cmplts r15, r16, r17 }
	{ v1shrs r5, r6, r7 ; ld2u r15, r16 }
	{ v1shrs r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
	{ v1shrs r5, r6, r7 ; stnt2 r15, r16 }
	{ v1shrs r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
	{ v1shrs r5, r6, r7 ; xor r15, r16, r17 }
	{ v1shrsi r15, r16, 5 ; fdouble_add_flags r5, r6, r7 }
	{ v1shrsi r15, r16, 5 ; mula_ls_ls r5, r6, r7 }
	{ v1shrsi r15, r16, 5 ; v1add r5, r6, r7 }
	{ v1shrsi r15, r16, 5 ; v1shrsi r5, r6, 5 }
	{ v1shrsi r15, r16, 5 ; v2shli r5, r6, 5 }
	{ v1shrsi r5, r6, 5 ; cmpne r15, r16, r17 }
	{ v1shrsi r5, r6, 5 ; ld4u r15, r16 }
	{ v1shrsi r5, r6, 5 ; prefetch_l1_fault r15 }
	{ v1shrsi r5, r6, 5 ; stnt_add r15, r16, 5 }
	{ v1shrsi r5, r6, 5 ; v2cmpltsi r15, r16, 5 }
	{ v1shru r15, r16, r17 ; addli r5, r6, 0x1234 }
	{ v1shru r15, r16, r17 ; fdouble_pack2 r5, r6, r7 }
	{ v1shru r15, r16, r17 ; mulx r5, r6, r7 }
	{ v1shru r15, r16, r17 ; v1avgu r5, r6, r7 }
	{ v1shru r15, r16, r17 ; v1subuc r5, r6, r7 }
	{ v1shru r15, r16, r17 ; v2shru r5, r6, r7 }
	{ v1shru r5, r6, r7 ; dtlbpr r15 }
	{ v1shru r5, r6, r7 ; ldna_add r15, r16, 5 }
	{ v1shru r5, r6, r7 ; prefetch_l3_fault r15 }
	{ v1shru r5, r6, r7 ; v1add r15, r16, r17 }
	{ v1shru r5, r6, r7 ; v2int_h r15, r16, r17 }
	{ v1shrui r15, r16, 5 ; addxsc r5, r6, r7 }
	{ v1shrui r15, r16, 5 ; fnop }
	{ v1shrui r15, r16, 5 ; or r5, r6, r7 }
	{ v1shrui r15, r16, 5 ; v1cmpleu r5, r6, r7 }
	{ v1shrui r15, r16, 5 ; v2adiffs r5, r6, r7 }
	{ v1shrui r15, r16, 5 ; v4add r5, r6, r7 }
	{ v1shrui r5, r6, 5 ; fetchadd4 r15, r16, r17 }
	{ v1shrui r5, r6, 5 ; ldnt1u r15, r16 }
	{ v1shrui r5, r6, 5 ; shl r15, r16, r17 }
	{ v1shrui r5, r6, 5 ; v1cmpeqi r15, r16, 5 }
	{ v1shrui r5, r6, 5 ; v2mins r15, r16, r17 }
	{ v1sub r15, r16, r17 ; bfextu r5, r6, 5, 7 }
	{ v1sub r15, r16, r17 ; fsingle_mul2 r5, r6, r7 }
	{ v1sub r15, r16, r17 ; revbytes r5, r6 }
	{ v1sub r15, r16, r17 ; v1cmpltui r5, r6, 5 }
	{ v1sub r15, r16, r17 ; v2cmples r5, r6, r7 }
	{ v1sub r15, r16, r17 ; v4packsc r5, r6, r7 }
	{ v1sub r5, r6, r7 ; fetchand4 r15, r16, r17 }
	{ v1sub r5, r6, r7 ; ldnt2u r15, r16 }
	{ v1sub r5, r6, r7 ; shl2add r15, r16, r17 }
	{ v1sub r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
	{ v1sub r5, r6, r7 ; v2packh r15, r16, r17 }
	{ v1subuc r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ v1subuc r15, r16, r17 ; info 19 }
	{ v1subuc r15, r16, r17 ; shl16insli r5, r6, 0x1234 }
	{ v1subuc r15, r16, r17 ; v1ddotpus r5, r6, r7 }
	{ v1subuc r15, r16, r17 ; v2cmpltu r5, r6, r7 }
	{ v1subuc r15, r16, r17 ; v4shru r5, r6, r7 }
	{ v1subuc r5, r6, r7 ; flush r15 }
	{ v1subuc r5, r6, r7 ; ldnt4u r15, r16 }
	{ v1subuc r5, r6, r7 ; shli r15, r16, 5 }
	{ v1subuc r5, r6, r7 ; v1int_h r15, r16, r17 }
	{ v1subuc r5, r6, r7 ; v2shli r15, r16, 5 }
	{ v2add r15, r16, r17 ; cmpleu r5, r6, r7 }
	{ v2add r15, r16, r17 ; move r5, r6 }
	{ v2add r15, r16, r17 ; shl2addx r5, r6, r7 }
	{ v2add r15, r16, r17 ; v1dotpu r5, r6, r7 }
	{ v2add r15, r16, r17 ; v2dotpa r5, r6, r7 }
	{ v2add r15, r16, r17 ; xori r5, r6, 5 }
	{ v2add r5, r6, r7 ; ill }
	{ v2add r5, r6, r7 ; mf }
	{ v2add r5, r6, r7 ; shrsi r15, r16, 5 }
	{ v2add r5, r6, r7 ; v1minu r15, r16, r17 }
	{ v2add r5, r6, r7 ; v2shru r15, r16, r17 }
	{ v2addi r15, r16, 5 ; cmpltui r5, r6, 5 }
	{ v2addi r15, r16, 5 ; mul_hs_hu r5, r6, r7 }
	{ v2addi r15, r16, 5 ; shlx r5, r6, r7 }
	{ v2addi r15, r16, 5 ; v1int_h r5, r6, r7 }
	{ v2addi r15, r16, 5 ; v2maxsi r5, r6, 5 }
	{ v2addi r5, r6, 5 ; addx r15, r16, r17 }
	{ v2addi r5, r6, 5 ; iret }
	{ v2addi r5, r6, 5 ; movei r15, 5 }
	{ v2addi r5, r6, 5 ; shruxi r15, r16, 5 }
	{ v2addi r5, r6, 5 ; v1shl r15, r16, r17 }
	{ v2addi r5, r6, 5 ; v4add r15, r16, r17 }
	{ v2addsc r15, r16, r17 ; cmulaf r5, r6, r7 }
	{ v2addsc r15, r16, r17 ; mul_hu_ls r5, r6, r7 }
	{ v2addsc r15, r16, r17 ; shru r5, r6, r7 }
	{ v2addsc r15, r16, r17 ; v1minu r5, r6, r7 }
	{ v2addsc r15, r16, r17 ; v2mulfsc r5, r6, r7 }
	{ v2addsc r5, r6, r7 ; and r15, r16, r17 }
	{ v2addsc r5, r6, r7 ; jrp r15 }
	{ v2addsc r5, r6, r7 ; nop }
	{ v2addsc r5, r6, r7 ; st2 r15, r16 }
	{ v2addsc r5, r6, r7 ; v1shru r15, r16, r17 }
	{ v2addsc r5, r6, r7 ; v4packsc r15, r16, r17 }
	{ v2adiffs r5, r6, r7 ; fetchand r15, r16, r17 }
	{ v2adiffs r5, r6, r7 ; ldnt2s_add r15, r16, 5 }
	{ v2adiffs r5, r6, r7 ; shl1addx r15, r16, r17 }
	{ v2adiffs r5, r6, r7 ; v1cmplts r15, r16, r17 }
	{ v2adiffs r5, r6, r7 ; v2mz r15, r16, r17 }
	{ v2avgs r5, r6, r7 ; cmples r15, r16, r17 }
	{ v2avgs r5, r6, r7 ; ld2s r15, r16 }
	{ v2avgs r5, r6, r7 ; prefetch_add_l1_fault r15, 5 }
	{ v2avgs r5, r6, r7 ; stnt1 r15, r16 }
	{ v2avgs r5, r6, r7 ; v2addsc r15, r16, r17 }
	{ v2avgs r5, r6, r7 ; v4subsc r15, r16, r17 }
	{ v2cmpeq r15, r16, r17 ; dblalign4 r5, r6, r7 }
	{ v2cmpeq r15, r16, r17 ; mula_hu_ls r5, r6, r7 }
	{ v2cmpeq r15, r16, r17 ; tblidxb2 r5, r6 }
	{ v2cmpeq r15, r16, r17 ; v1shli r5, r6, 5 }
	{ v2cmpeq r15, r16, r17 ; v2sadu r5, r6, r7 }
	{ v2cmpeq r5, r6, r7 ; cmpltu r15, r16, r17 }
	{ v2cmpeq r5, r6, r7 ; ld4s r15, r16 }
	{ v2cmpeq r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ v2cmpeq r5, r6, r7 ; stnt4 r15, r16 }
	{ v2cmpeq r5, r6, r7 ; v2cmpleu r15, r16, r17 }
	{ v2cmpeqi r15, r16, 5 ; add r5, r6, r7 }
	{ v2cmpeqi r15, r16, 5 ; fdouble_mul_flags r5, r6, r7 }
	{ v2cmpeqi r15, r16, 5 ; mula_lu_lu r5, r6, r7 }
	{ v2cmpeqi r15, r16, 5 ; v1adduc r5, r6, r7 }
	{ v2cmpeqi r15, r16, 5 ; v1shrui r5, r6, 5 }
	{ v2cmpeqi r15, r16, 5 ; v2shrs r5, r6, r7 }
	{ v2cmpeqi r5, r6, 5 ; dblalign4 r15, r16, r17 }
	{ v2cmpeqi r5, r6, 5 ; ld_add r15, r16, 5 }
	{ v2cmpeqi r5, r6, 5 ; prefetch_l2_fault r15 }
	{ v2cmpeqi r5, r6, 5 ; subx r15, r16, r17 }
	{ v2cmpeqi r5, r6, 5 ; v2cmpltui r15, r16, 5 }
	{ v2cmples r15, r16, r17 ; addxi r5, r6, 5 }
	{ v2cmples r15, r16, r17 ; fdouble_unpack_max r5, r6, r7 }
	{ v2cmples r15, r16, r17 ; nop }
	{ v2cmples r15, r16, r17 ; v1cmpeqi r5, r6, 5 }
	{ v2cmples r15, r16, r17 ; v2addi r5, r6, 5 }
	{ v2cmples r15, r16, r17 ; v2sub r5, r6, r7 }
	{ v2cmples r5, r6, r7 ; exch4 r15, r16, r17 }
	{ v2cmples r5, r6, r7 ; ldnt1s r15, r16 }
	{ v2cmples r5, r6, r7 ; rotl r15, r16, r17 }
	{ v2cmples r5, r6, r7 ; v1adduc r15, r16, r17 }
	{ v2cmples r5, r6, r7 ; v2maxs r15, r16, r17 }
	{ v2cmpleu r15, r16, r17 ; andi r5, r6, 5 }
	{ v2cmpleu r15, r16, r17 ; fsingle_addsub2 r5, r6, r7 }
	{ v2cmpleu r15, r16, r17 ; pcnt r5, r6 }
	{ v2cmpleu r15, r16, r17 ; v1cmpltsi r5, r6, 5 }
	{ v2cmpleu r15, r16, r17 ; v2cmpeq r5, r6, r7 }
	{ v2cmpleu r15, r16, r17 ; v4int_h r5, r6, r7 }
	{ v2cmpleu r5, r6, r7 ; fetchaddgez4 r15, r16, r17 }
	{ v2cmpleu r5, r6, r7 ; ldnt2s r15, r16 }
	{ v2cmpleu r5, r6, r7 ; shl1add r15, r16, r17 }
	{ v2cmpleu r5, r6, r7 ; v1cmpleu r15, r16, r17 }
	{ v2cmpleu r5, r6, r7 ; v2mnz r15, r16, r17 }
	{ v2cmplts r15, r16, r17 ; clz r5, r6 }
	{ v2cmplts r15, r16, r17 ; fsingle_pack2 r5, r6, r7 }
	{ v2cmplts r15, r16, r17 ; rotli r5, r6, 5 }
	{ v2cmplts r15, r16, r17 ; v1ddotpu r5, r6, r7 }
	{ v2cmplts r15, r16, r17 ; v2cmplts r5, r6, r7 }
	{ v2cmplts r15, r16, r17 ; v4shlsc r5, r6, r7 }
	{ v2cmplts r5, r6, r7 ; fetchor4 r15, r16, r17 }
	{ v2cmplts r5, r6, r7 ; ldnt4s r15, r16 }
	{ v2cmplts r5, r6, r7 ; shl3add r15, r16, r17 }
	{ v2cmplts r5, r6, r7 ; v1cmpltui r15, r16, 5 }
	{ v2cmplts r5, r6, r7 ; v2packuc r15, r16, r17 }
	{ v2cmpltsi r15, r16, 5 ; cmpeqi r5, r6, 5 }
	{ v2cmpltsi r15, r16, 5 ; mm r5, r6, 5, 7 }
	{ v2cmpltsi r15, r16, 5 ; shl1addx r5, r6, r7 }
	{ v2cmpltsi r15, r16, 5 ; v1dotp r5, r6, r7 }
	{ v2cmpltsi r15, r16, 5 ; v2cmpne r5, r6, r7 }
	{ v2cmpltsi r15, r16, 5 ; v4subsc r5, r6, r7 }
	{ v2cmpltsi r5, r6, 5 ; fnop }
	{ v2cmpltsi r5, r6, 5 ; ldnt_add r15, r16, 5 }
	{ v2cmpltsi r5, r6, 5 ; shlxi r15, r16, 5 }
	{ v2cmpltsi r5, r6, 5 ; v1maxu r15, r16, r17 }
	{ v2cmpltsi r5, r6, 5 ; v2shrs r15, r16, r17 }
	{ v2cmpltu r15, r16, r17 ; cmpltsi r5, r6, 5 }
	{ v2cmpltu r15, r16, r17 ; moveli r5, 0x1234 }
	{ v2cmpltu r15, r16, r17 ; shl3addx r5, r6, r7 }
	{ v2cmpltu r15, r16, r17 ; v1dotpus r5, r6, r7 }
	{ v2cmpltu r15, r16, r17 ; v2int_l r5, r6, r7 }
	{ v2cmpltu r5, r6, r7 ; addi r15, r16, 5 }
	{ v2cmpltu r5, r6, r7 ; infol 0x1234 }
	{ v2cmpltu r5, r6, r7 ; mnz r15, r16, r17 }
	{ v2cmpltu r5, r6, r7 ; shrui r15, r16, 5 }
	{ v2cmpltu r5, r6, r7 ; v1mnz r15, r16, r17 }
	{ v2cmpltu r5, r6, r7 ; v2sub r15, r16, r17 }
	{ v2cmpltui r15, r16, 5 ; cmul r5, r6, r7 }
	{ v2cmpltui r15, r16, 5 ; mul_hs_lu r5, r6, r7 }
	{ v2cmpltui r15, r16, 5 ; shrs r5, r6, r7 }
	{ v2cmpltui r15, r16, 5 ; v1maxu r5, r6, r7 }
	{ v2cmpltui r15, r16, 5 ; v2minsi r5, r6, 5 }
	{ v2cmpltui r5, r6, 5 ; addxli r15, r16, 0x1234 }
	{ v2cmpltui r5, r6, 5 ; jalrp r15 }
	{ v2cmpltui r5, r6, 5 ; mtspr 0x5, r16 }
	{ v2cmpltui r5, r6, 5 ; st1 r15, r16 }
	{ v2cmpltui r5, r6, 5 ; v1shrs r15, r16, r17 }
	{ v2cmpltui r5, r6, 5 ; v4int_h r15, r16, r17 }
	{ v2cmpne r15, r16, r17 ; cmulfr r5, r6, r7 }
	{ v2cmpne r15, r16, r17 ; mul_ls_ls r5, r6, r7 }
	{ v2cmpne r15, r16, r17 ; shrux r5, r6, r7 }
	{ v2cmpne r15, r16, r17 ; v1mnz r5, r6, r7 }
	{ v2cmpne r15, r16, r17 ; v2mults r5, r6, r7 }
	{ v2cmpne r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ v2cmpne r5, r6, r7 ; ld1s r15, r16 }
	{ v2cmpne r5, r6, r7 ; or r15, r16, r17 }
	{ v2cmpne r5, r6, r7 ; st4 r15, r16 }
	{ v2cmpne r5, r6, r7 ; v1sub r15, r16, r17 }
	{ v2cmpne r5, r6, r7 ; v4shlsc r15, r16, r17 }
	{ v2dotp r5, r6, r7 ; fetchor r15, r16, r17 }
	{ v2dotp r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
	{ v2dotp r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ v2dotp r5, r6, r7 ; v1cmpltu r15, r16, r17 }
	{ v2dotp r5, r6, r7 ; v2packl r15, r16, r17 }
	{ v2dotpa r5, r6, r7 ; cmplts r15, r16, r17 }
	{ v2dotpa r5, r6, r7 ; ld2u r15, r16 }
	{ v2dotpa r5, r6, r7 ; prefetch_add_l2_fault r15, 5 }
	{ v2dotpa r5, r6, r7 ; stnt2 r15, r16 }
	{ v2dotpa r5, r6, r7 ; v2cmpeqi r15, r16, 5 }
	{ v2dotpa r5, r6, r7 ; xor r15, r16, r17 }
	{ v2int_h r15, r16, r17 ; fdouble_add_flags r5, r6, r7 }
	{ v2int_h r15, r16, r17 ; mula_ls_ls r5, r6, r7 }
	{ v2int_h r15, r16, r17 ; v1add r5, r6, r7 }
	{ v2int_h r15, r16, r17 ; v1shrsi r5, r6, 5 }
	{ v2int_h r15, r16, r17 ; v2shli r5, r6, 5 }
	{ v2int_h r5, r6, r7 ; cmpne r15, r16, r17 }
	{ v2int_h r5, r6, r7 ; ld4u r15, r16 }
	{ v2int_h r5, r6, r7 ; prefetch_l1_fault r15 }
	{ v2int_h r5, r6, r7 ; stnt_add r15, r16, 5 }
	{ v2int_h r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ v2int_l r15, r16, r17 ; addli r5, r6, 0x1234 }
	{ v2int_l r15, r16, r17 ; fdouble_pack2 r5, r6, r7 }
	{ v2int_l r15, r16, r17 ; mulx r5, r6, r7 }
	{ v2int_l r15, r16, r17 ; v1avgu r5, r6, r7 }
	{ v2int_l r15, r16, r17 ; v1subuc r5, r6, r7 }
	{ v2int_l r15, r16, r17 ; v2shru r5, r6, r7 }
	{ v2int_l r5, r6, r7 ; dtlbpr r15 }
	{ v2int_l r5, r6, r7 ; ldna_add r15, r16, 5 }
	{ v2int_l r5, r6, r7 ; prefetch_l3_fault r15 }
	{ v2int_l r5, r6, r7 ; v1add r15, r16, r17 }
	{ v2int_l r5, r6, r7 ; v2int_h r15, r16, r17 }
	{ v2maxs r15, r16, r17 ; addxsc r5, r6, r7 }
	{ v2maxs r15, r16, r17 ; fnop }
	{ v2maxs r15, r16, r17 ; or r5, r6, r7 }
	{ v2maxs r15, r16, r17 ; v1cmpleu r5, r6, r7 }
	{ v2maxs r15, r16, r17 ; v2adiffs r5, r6, r7 }
	{ v2maxs r15, r16, r17 ; v4add r5, r6, r7 }
	{ v2maxs r5, r6, r7 ; fetchadd4 r15, r16, r17 }
	{ v2maxs r5, r6, r7 ; ldnt1u r15, r16 }
	{ v2maxs r5, r6, r7 ; shl r15, r16, r17 }
	{ v2maxs r5, r6, r7 ; v1cmpeqi r15, r16, 5 }
	{ v2maxs r5, r6, r7 ; v2mins r15, r16, r17 }
	{ v2maxsi r15, r16, 5 ; bfextu r5, r6, 5, 7 }
	{ v2maxsi r15, r16, 5 ; fsingle_mul2 r5, r6, r7 }
	{ v2maxsi r15, r16, 5 ; revbytes r5, r6 }
	{ v2maxsi r15, r16, 5 ; v1cmpltui r5, r6, 5 }
	{ v2maxsi r15, r16, 5 ; v2cmples r5, r6, r7 }
	{ v2maxsi r15, r16, 5 ; v4packsc r5, r6, r7 }
	{ v2maxsi r5, r6, 5 ; fetchand4 r15, r16, r17 }
	{ v2maxsi r5, r6, 5 ; ldnt2u r15, r16 }
	{ v2maxsi r5, r6, 5 ; shl2add r15, r16, r17 }
	{ v2maxsi r5, r6, 5 ; v1cmpltsi r15, r16, 5 }
	{ v2maxsi r5, r6, 5 ; v2packh r15, r16, r17 }
	{ v2mins r15, r16, r17 ; cmovnez r5, r6, r7 }
	{ v2mins r15, r16, r17 ; info 19 }
	{ v2mins r15, r16, r17 ; shl16insli r5, r6, 0x1234 }
	{ v2mins r15, r16, r17 ; v1ddotpus r5, r6, r7 }
	{ v2mins r15, r16, r17 ; v2cmpltu r5, r6, r7 }
	{ v2mins r15, r16, r17 ; v4shru r5, r6, r7 }
	{ v2mins r5, r6, r7 ; flush r15 }
	{ v2mins r5, r6, r7 ; ldnt4u r15, r16 }
	{ v2mins r5, r6, r7 ; shli r15, r16, 5 }
	{ v2mins r5, r6, r7 ; v1int_h r15, r16, r17 }
	{ v2mins r5, r6, r7 ; v2shli r15, r16, 5 }
	{ v2minsi r15, r16, 5 ; cmpleu r5, r6, r7 }
	{ v2minsi r15, r16, 5 ; move r5, r6 }
	{ v2minsi r15, r16, 5 ; shl2addx r5, r6, r7 }
	{ v2minsi r15, r16, 5 ; v1dotpu r5, r6, r7 }
	{ v2minsi r15, r16, 5 ; v2dotpa r5, r6, r7 }
	{ v2minsi r15, r16, 5 ; xori r5, r6, 5 }
	{ v2minsi r5, r6, 5 ; ill }
	{ v2minsi r5, r6, 5 ; mf }
	{ v2minsi r5, r6, 5 ; shrsi r15, r16, 5 }
	{ v2minsi r5, r6, 5 ; v1minu r15, r16, r17 }
	{ v2minsi r5, r6, 5 ; v2shru r15, r16, r17 }
	{ v2mnz r15, r16, r17 ; cmpltui r5, r6, 5 }
	{ v2mnz r15, r16, r17 ; mul_hs_hu r5, r6, r7 }
	{ v2mnz r15, r16, r17 ; shlx r5, r6, r7 }
	{ v2mnz r15, r16, r17 ; v1int_h r5, r6, r7 }
	{ v2mnz r15, r16, r17 ; v2maxsi r5, r6, 5 }
	{ v2mnz r5, r6, r7 ; addx r15, r16, r17 }
	{ v2mnz r5, r6, r7 ; iret }
	{ v2mnz r5, r6, r7 ; movei r15, 5 }
	{ v2mnz r5, r6, r7 ; shruxi r15, r16, 5 }
	{ v2mnz r5, r6, r7 ; v1shl r15, r16, r17 }
	{ v2mnz r5, r6, r7 ; v4add r15, r16, r17 }
	{ v2mulfsc r5, r6, r7 ; fetchadd r15, r16, r17 }
	{ v2mulfsc r5, r6, r7 ; ldnt1s_add r15, r16, 5 }
	{ v2mulfsc r5, r6, r7 ; rotli r15, r16, 5 }
	{ v2mulfsc r5, r6, r7 ; v1cmpeq r15, r16, r17 }
	{ v2mulfsc r5, r6, r7 ; v2maxsi r15, r16, 5 }
	{ v2muls r5, r6, r7 ; cmpeq r15, r16, r17 }
	{ v2muls r5, r6, r7 ; ld1s r15, r16 }
	{ v2muls r5, r6, r7 ; or r15, r16, r17 }
	{ v2muls r5, r6, r7 ; st4 r15, r16 }
	{ v2muls r5, r6, r7 ; v1sub r15, r16, r17 }
	{ v2muls r5, r6, r7 ; v4shlsc r15, r16, r17 }
	{ v2mults r5, r6, r7 ; fetchor r15, r16, r17 }
	{ v2mults r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
	{ v2mults r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ v2mults r5, r6, r7 ; v1cmpltu r15, r16, r17 }
	{ v2mults r5, r6, r7 ; v2packl r15, r16, r17 }
	{ v2mz r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ v2mz r15, r16, r17 ; infol 0x1234 }
	{ v2mz r15, r16, r17 ; shl1add r5, r6, r7 }
	{ v2mz r15, r16, r17 ; v1ddotpusa r5, r6, r7 }
	{ v2mz r15, r16, r17 ; v2cmpltui r5, r6, 5 }
	{ v2mz r15, r16, r17 ; v4sub r5, r6, r7 }
	{ v2mz r5, r6, r7 ; flushwb }
	{ v2mz r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
	{ v2mz r5, r6, r7 ; shlx r15, r16, r17 }
	{ v2mz r5, r6, r7 ; v1int_l r15, r16, r17 }
	{ v2mz r5, r6, r7 ; v2shlsc r15, r16, r17 }
	{ v2packh r15, r16, r17 ; cmplts r5, r6, r7 }
	{ v2packh r15, r16, r17 ; movei r5, 5 }
	{ v2packh r15, r16, r17 ; shl3add r5, r6, r7 }
	{ v2packh r15, r16, r17 ; v1dotpua r5, r6, r7 }
	{ v2packh r15, r16, r17 ; v2int_h r5, r6, r7 }
	{ v2packh r5, r6, r7 ; add r15, r16, r17 }
	{ v2packh r5, r6, r7 ; info 19 }
	{ v2packh r5, r6, r7 ; mfspr r16, 0x5 }
	{ v2packh r5, r6, r7 ; shru r15, r16, r17 }
	{ v2packh r5, r6, r7 ; v1minui r15, r16, 5 }
	{ v2packh r5, r6, r7 ; v2shrui r15, r16, 5 }
	{ v2packl r15, r16, r17 ; cmpne r5, r6, r7 }
	{ v2packl r15, r16, r17 ; mul_hs_ls r5, r6, r7 }
	{ v2packl r15, r16, r17 ; shlxi r5, r6, 5 }
	{ v2packl r15, r16, r17 ; v1int_l r5, r6, r7 }
	{ v2packl r15, r16, r17 ; v2mins r5, r6, r7 }
	{ v2packl r5, r6, r7 ; addxi r15, r16, 5 }
	{ v2packl r5, r6, r7 ; jalr r15 }
	{ v2packl r5, r6, r7 ; moveli r15, 0x1234 }
	{ v2packl r5, r6, r7 ; st r15, r16 }
	{ v2packl r5, r6, r7 ; v1shli r15, r16, 5 }
	{ v2packl r5, r6, r7 ; v4addsc r15, r16, r17 }
	{ v2packuc r15, r16, r17 ; cmulf r5, r6, r7 }
	{ v2packuc r15, r16, r17 ; mul_hu_lu r5, r6, r7 }
	{ v2packuc r15, r16, r17 ; shrui r5, r6, 5 }
	{ v2packuc r15, r16, r17 ; v1minui r5, r6, 5 }
	{ v2packuc r15, r16, r17 ; v2muls r5, r6, r7 }
	{ v2packuc r5, r6, r7 ; andi r15, r16, 5 }
	{ v2packuc r5, r6, r7 ; ld r15, r16 }
	{ v2packuc r5, r6, r7 ; nor r15, r16, r17 }
	{ v2packuc r5, r6, r7 ; st2_add r15, r16, 5 }
	{ v2packuc r5, r6, r7 ; v1shrui r15, r16, 5 }
	{ v2packuc r5, r6, r7 ; v4shl r15, r16, r17 }
	{ v2sadas r5, r6, r7 ; fetchand4 r15, r16, r17 }
	{ v2sadas r5, r6, r7 ; ldnt2u r15, r16 }
	{ v2sadas r5, r6, r7 ; shl2add r15, r16, r17 }
	{ v2sadas r5, r6, r7 ; v1cmpltsi r15, r16, 5 }
	{ v2sadas r5, r6, r7 ; v2packh r15, r16, r17 }
	{ v2sadau r5, r6, r7 ; cmpleu r15, r16, r17 }
	{ v2sadau r5, r6, r7 ; ld2s_add r15, r16, 5 }
	{ v2sadau r5, r6, r7 ; prefetch_add_l2 r15, 5 }
	{ v2sadau r5, r6, r7 ; stnt1_add r15, r16, 5 }
	{ v2sadau r5, r6, r7 ; v2cmpeq r15, r16, r17 }
	{ v2sadau r5, r6, r7 ; wh64 r15 }
	{ v2sads r5, r6, r7 ; fnop }
	{ v2sads r5, r6, r7 ; ldnt_add r15, r16, 5 }
	{ v2sads r5, r6, r7 ; shlxi r15, r16, 5 }
	{ v2sads r5, r6, r7 ; v1maxu r15, r16, r17 }
	{ v2sads r5, r6, r7 ; v2shrs r15, r16, r17 }
	{ v2sadu r5, r6, r7 ; dblalign2 r15, r16, r17 }
	{ v2sadu r5, r6, r7 ; ld4u_add r15, r16, 5 }
	{ v2sadu r5, r6, r7 ; prefetch_l2 r15 }
	{ v2sadu r5, r6, r7 ; sub r15, r16, r17 }
	{ v2sadu r5, r6, r7 ; v2cmpltu r15, r16, r17 }
	{ v2shl r15, r16, r17 ; addx r5, r6, r7 }
	{ v2shl r15, r16, r17 ; fdouble_sub_flags r5, r6, r7 }
	{ v2shl r15, r16, r17 ; mz r5, r6, r7 }
	{ v2shl r15, r16, r17 ; v1cmpeq r5, r6, r7 }
	{ v2shl r15, r16, r17 ; v2add r5, r6, r7 }
	{ v2shl r15, r16, r17 ; v2shrui r5, r6, 5 }
	{ v2shl r5, r6, r7 ; exch r15, r16, r17 }
	{ v2shl r5, r6, r7 ; ldnt r15, r16 }
	{ v2shl r5, r6, r7 ; raise }
	{ v2shl r5, r6, r7 ; v1addi r15, r16, 5 }
	{ v2shl r5, r6, r7 ; v2int_l r15, r16, r17 }
	{ v2shli r15, r16, 5 ; and r5, r6, r7 }
	{ v2shli r15, r16, 5 ; fsingle_add1 r5, r6, r7 }
	{ v2shli r15, r16, 5 ; ori r5, r6, 5 }
	{ v2shli r15, r16, 5 ; v1cmplts r5, r6, r7 }
	{ v2shli r15, r16, 5 ; v2avgs r5, r6, r7 }
	{ v2shli r15, r16, 5 ; v4addsc r5, r6, r7 }
	{ v2shli r5, r6, 5 ; fetchaddgez r15, r16, r17 }
	{ v2shli r5, r6, 5 ; ldnt1u_add r15, r16, 5 }
	{ v2shli r5, r6, 5 ; shl16insli r15, r16, 0x1234 }
	{ v2shli r5, r6, 5 ; v1cmples r15, r16, r17 }
	{ v2shli r5, r6, 5 ; v2minsi r15, r16, 5 }
	{ v2shlsc r15, r16, r17 ; bfins r5, r6, 5, 7 }
	{ v2shlsc r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ v2shlsc r15, r16, r17 ; rotl r5, r6, r7 }
	{ v2shlsc r15, r16, r17 ; v1cmpne r5, r6, r7 }
	{ v2shlsc r15, r16, r17 ; v2cmpleu r5, r6, r7 }
	{ v2shlsc r15, r16, r17 ; v4shl r5, r6, r7 }
	{ v2shlsc r5, r6, r7 ; fetchor r15, r16, r17 }
	{ v2shlsc r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
	{ v2shlsc r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ v2shlsc r5, r6, r7 ; v1cmpltu r15, r16, r17 }
	{ v2shlsc r5, r6, r7 ; v2packl r15, r16, r17 }
	{ v2shrs r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ v2shrs r15, r16, r17 ; infol 0x1234 }
	{ v2shrs r15, r16, r17 ; shl1add r5, r6, r7 }
	{ v2shrs r15, r16, r17 ; v1ddotpusa r5, r6, r7 }
	{ v2shrs r15, r16, r17 ; v2cmpltui r5, r6, 5 }
	{ v2shrs r15, r16, r17 ; v4sub r5, r6, r7 }
	{ v2shrs r5, r6, r7 ; flushwb }
	{ v2shrs r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
	{ v2shrs r5, r6, r7 ; shlx r15, r16, r17 }
	{ v2shrs r5, r6, r7 ; v1int_l r15, r16, r17 }
	{ v2shrs r5, r6, r7 ; v2shlsc r15, r16, r17 }
	{ v2shrsi r15, r16, 5 ; cmplts r5, r6, r7 }
	{ v2shrsi r15, r16, 5 ; movei r5, 5 }
	{ v2shrsi r15, r16, 5 ; shl3add r5, r6, r7 }
	{ v2shrsi r15, r16, 5 ; v1dotpua r5, r6, r7 }
	{ v2shrsi r15, r16, 5 ; v2int_h r5, r6, r7 }
	{ v2shrsi r5, r6, 5 ; add r15, r16, r17 }
	{ v2shrsi r5, r6, 5 ; info 19 }
	{ v2shrsi r5, r6, 5 ; mfspr r16, 0x5 }
	{ v2shrsi r5, r6, 5 ; shru r15, r16, r17 }
	{ v2shrsi r5, r6, 5 ; v1minui r15, r16, 5 }
	{ v2shrsi r5, r6, 5 ; v2shrui r15, r16, 5 }
	{ v2shru r15, r16, r17 ; cmpne r5, r6, r7 }
	{ v2shru r15, r16, r17 ; mul_hs_ls r5, r6, r7 }
	{ v2shru r15, r16, r17 ; shlxi r5, r6, 5 }
	{ v2shru r15, r16, r17 ; v1int_l r5, r6, r7 }
	{ v2shru r15, r16, r17 ; v2mins r5, r6, r7 }
	{ v2shru r5, r6, r7 ; addxi r15, r16, 5 }
	{ v2shru r5, r6, r7 ; jalr r15 }
	{ v2shru r5, r6, r7 ; moveli r15, 0x1234 }
	{ v2shru r5, r6, r7 ; st r15, r16 }
	{ v2shru r5, r6, r7 ; v1shli r15, r16, 5 }
	{ v2shru r5, r6, r7 ; v4addsc r15, r16, r17 }
	{ v2shrui r15, r16, 5 ; cmulf r5, r6, r7 }
	{ v2shrui r15, r16, 5 ; mul_hu_lu r5, r6, r7 }
	{ v2shrui r15, r16, 5 ; shrui r5, r6, 5 }
	{ v2shrui r15, r16, 5 ; v1minui r5, r6, 5 }
	{ v2shrui r15, r16, 5 ; v2muls r5, r6, r7 }
	{ v2shrui r5, r6, 5 ; andi r15, r16, 5 }
	{ v2shrui r5, r6, 5 ; ld r15, r16 }
	{ v2shrui r5, r6, 5 ; nor r15, r16, r17 }
	{ v2shrui r5, r6, 5 ; st2_add r15, r16, 5 }
	{ v2shrui r5, r6, 5 ; v1shrui r15, r16, 5 }
	{ v2shrui r5, r6, 5 ; v4shl r15, r16, r17 }
	{ v2sub r15, r16, r17 ; crc32_32 r5, r6, r7 }
	{ v2sub r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ v2sub r15, r16, r17 ; sub r5, r6, r7 }
	{ v2sub r15, r16, r17 ; v1mulus r5, r6, r7 }
	{ v2sub r15, r16, r17 ; v2packl r5, r6, r7 }
	{ v2sub r5, r6, r7 ; cmpexch4 r15, r16, r17 }
	{ v2sub r5, r6, r7 ; ld1u_add r15, r16, 5 }
	{ v2sub r5, r6, r7 ; prefetch_add_l1 r15, 5 }
	{ v2sub r5, r6, r7 ; stnt r15, r16 }
	{ v2sub r5, r6, r7 ; v2addi r15, r16, 5 }
	{ v2sub r5, r6, r7 ; v4sub r15, r16, r17 }
	{ v2subsc r15, r16, r17 ; dblalign2 r5, r6, r7 }
	{ v2subsc r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ v2subsc r15, r16, r17 ; tblidxb1 r5, r6 }
	{ v2subsc r15, r16, r17 ; v1shl r5, r6, r7 }
	{ v2subsc r15, r16, r17 ; v2sads r5, r6, r7 }
	{ v2subsc r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ v2subsc r5, r6, r7 ; ld2u_add r15, r16, 5 }
	{ v2subsc r5, r6, r7 ; prefetch_add_l3 r15, 5 }
	{ v2subsc r5, r6, r7 ; stnt2_add r15, r16, 5 }
	{ v2subsc r5, r6, r7 ; v2cmples r15, r16, r17 }
	{ v2subsc r5, r6, r7 ; xori r15, r16, 5 }
	{ v4add r15, r16, r17 ; fdouble_addsub r5, r6, r7 }
	{ v4add r15, r16, r17 ; mula_ls_lu r5, r6, r7 }
	{ v4add r15, r16, r17 ; v1addi r5, r6, 5 }
	{ v4add r15, r16, r17 ; v1shru r5, r6, r7 }
	{ v4add r15, r16, r17 ; v2shlsc r5, r6, r7 }
	{ v4add r5, r6, r7 ; dblalign2 r15, r16, r17 }
	{ v4add r5, r6, r7 ; ld4u_add r15, r16, 5 }
	{ v4add r5, r6, r7 ; prefetch_l2 r15 }
	{ v4add r5, r6, r7 ; sub r15, r16, r17 }
	{ v4add r5, r6, r7 ; v2cmpltu r15, r16, r17 }
	{ v4addsc r15, r16, r17 ; addx r5, r6, r7 }
	{ v4addsc r15, r16, r17 ; fdouble_sub_flags r5, r6, r7 }
	{ v4addsc r15, r16, r17 ; mz r5, r6, r7 }
	{ v4addsc r15, r16, r17 ; v1cmpeq r5, r6, r7 }
	{ v4addsc r15, r16, r17 ; v2add r5, r6, r7 }
	{ v4addsc r15, r16, r17 ; v2shrui r5, r6, 5 }
	{ v4addsc r5, r6, r7 ; exch r15, r16, r17 }
	{ v4addsc r5, r6, r7 ; ldnt r15, r16 }
	{ v4addsc r5, r6, r7 ; raise }
	{ v4addsc r5, r6, r7 ; v1addi r15, r16, 5 }
	{ v4addsc r5, r6, r7 ; v2int_l r15, r16, r17 }
	{ v4int_h r15, r16, r17 ; and r5, r6, r7 }
	{ v4int_h r15, r16, r17 ; fsingle_add1 r5, r6, r7 }
	{ v4int_h r15, r16, r17 ; ori r5, r6, 5 }
	{ v4int_h r15, r16, r17 ; v1cmplts r5, r6, r7 }
	{ v4int_h r15, r16, r17 ; v2avgs r5, r6, r7 }
	{ v4int_h r15, r16, r17 ; v4addsc r5, r6, r7 }
	{ v4int_h r5, r6, r7 ; fetchaddgez r15, r16, r17 }
	{ v4int_h r5, r6, r7 ; ldnt1u_add r15, r16, 5 }
	{ v4int_h r5, r6, r7 ; shl16insli r15, r16, 0x1234 }
	{ v4int_h r5, r6, r7 ; v1cmples r15, r16, r17 }
	{ v4int_h r5, r6, r7 ; v2minsi r15, r16, 5 }
	{ v4int_l r15, r16, r17 ; bfins r5, r6, 5, 7 }
	{ v4int_l r15, r16, r17 ; fsingle_pack1 r5, r6 }
	{ v4int_l r15, r16, r17 ; rotl r5, r6, r7 }
	{ v4int_l r15, r16, r17 ; v1cmpne r5, r6, r7 }
	{ v4int_l r15, r16, r17 ; v2cmpleu r5, r6, r7 }
	{ v4int_l r15, r16, r17 ; v4shl r5, r6, r7 }
	{ v4int_l r5, r6, r7 ; fetchor r15, r16, r17 }
	{ v4int_l r5, r6, r7 ; ldnt2u_add r15, r16, 5 }
	{ v4int_l r5, r6, r7 ; shl2addx r15, r16, r17 }
	{ v4int_l r5, r6, r7 ; v1cmpltu r15, r16, r17 }
	{ v4int_l r5, r6, r7 ; v2packl r15, r16, r17 }
	{ v4packsc r15, r16, r17 ; cmpeq r5, r6, r7 }
	{ v4packsc r15, r16, r17 ; infol 0x1234 }
	{ v4packsc r15, r16, r17 ; shl1add r5, r6, r7 }
	{ v4packsc r15, r16, r17 ; v1ddotpusa r5, r6, r7 }
	{ v4packsc r15, r16, r17 ; v2cmpltui r5, r6, 5 }
	{ v4packsc r15, r16, r17 ; v4sub r5, r6, r7 }
	{ v4packsc r5, r6, r7 ; flushwb }
	{ v4packsc r5, r6, r7 ; ldnt4u_add r15, r16, 5 }
	{ v4packsc r5, r6, r7 ; shlx r15, r16, r17 }
	{ v4packsc r5, r6, r7 ; v1int_l r15, r16, r17 }
	{ v4packsc r5, r6, r7 ; v2shlsc r15, r16, r17 }
	{ v4shl r15, r16, r17 ; cmplts r5, r6, r7 }
	{ v4shl r15, r16, r17 ; movei r5, 5 }
	{ v4shl r15, r16, r17 ; shl3add r5, r6, r7 }
	{ v4shl r15, r16, r17 ; v1dotpua r5, r6, r7 }
	{ v4shl r15, r16, r17 ; v2int_h r5, r6, r7 }
	{ v4shl r5, r6, r7 ; add r15, r16, r17 }
	{ v4shl r5, r6, r7 ; info 19 }
	{ v4shl r5, r6, r7 ; mfspr r16, 0x5 }
	{ v4shl r5, r6, r7 ; shru r15, r16, r17 }
	{ v4shl r5, r6, r7 ; v1minui r15, r16, 5 }
	{ v4shl r5, r6, r7 ; v2shrui r15, r16, 5 }
	{ v4shlsc r15, r16, r17 ; cmpne r5, r6, r7 }
	{ v4shlsc r15, r16, r17 ; mul_hs_ls r5, r6, r7 }
	{ v4shlsc r15, r16, r17 ; shlxi r5, r6, 5 }
	{ v4shlsc r15, r16, r17 ; v1int_l r5, r6, r7 }
	{ v4shlsc r15, r16, r17 ; v2mins r5, r6, r7 }
	{ v4shlsc r5, r6, r7 ; addxi r15, r16, 5 }
	{ v4shlsc r5, r6, r7 ; jalr r15 }
	{ v4shlsc r5, r6, r7 ; moveli r15, 0x1234 }
	{ v4shlsc r5, r6, r7 ; st r15, r16 }
	{ v4shlsc r5, r6, r7 ; v1shli r15, r16, 5 }
	{ v4shlsc r5, r6, r7 ; v4addsc r15, r16, r17 }
	{ v4shrs r15, r16, r17 ; cmulf r5, r6, r7 }
	{ v4shrs r15, r16, r17 ; mul_hu_lu r5, r6, r7 }
	{ v4shrs r15, r16, r17 ; shrui r5, r6, 5 }
	{ v4shrs r15, r16, r17 ; v1minui r5, r6, 5 }
	{ v4shrs r15, r16, r17 ; v2muls r5, r6, r7 }
	{ v4shrs r5, r6, r7 ; andi r15, r16, 5 }
	{ v4shrs r5, r6, r7 ; ld r15, r16 }
	{ v4shrs r5, r6, r7 ; nor r15, r16, r17 }
	{ v4shrs r5, r6, r7 ; st2_add r15, r16, 5 }
	{ v4shrs r5, r6, r7 ; v1shrui r15, r16, 5 }
	{ v4shrs r5, r6, r7 ; v4shl r15, r16, r17 }
	{ v4shru r15, r16, r17 ; crc32_32 r5, r6, r7 }
	{ v4shru r15, r16, r17 ; mula_hs_hs r5, r6, r7 }
	{ v4shru r15, r16, r17 ; sub r5, r6, r7 }
	{ v4shru r15, r16, r17 ; v1mulus r5, r6, r7 }
	{ v4shru r15, r16, r17 ; v2packl r5, r6, r7 }
	{ v4shru r5, r6, r7 ; cmpexch4 r15, r16, r17 }
	{ v4shru r5, r6, r7 ; ld1u_add r15, r16, 5 }
	{ v4shru r5, r6, r7 ; prefetch_add_l1 r15, 5 }
	{ v4shru r5, r6, r7 ; stnt r15, r16 }
	{ v4shru r5, r6, r7 ; v2addi r15, r16, 5 }
	{ v4shru r5, r6, r7 ; v4sub r15, r16, r17 }
	{ v4sub r15, r16, r17 ; dblalign2 r5, r6, r7 }
	{ v4sub r15, r16, r17 ; mula_hu_hu r5, r6, r7 }
	{ v4sub r15, r16, r17 ; tblidxb1 r5, r6 }
	{ v4sub r15, r16, r17 ; v1shl r5, r6, r7 }
	{ v4sub r15, r16, r17 ; v2sads r5, r6, r7 }
	{ v4sub r5, r6, r7 ; cmpltsi r15, r16, 5 }
	{ v4sub r5, r6, r7 ; ld2u_add r15, r16, 5 }
	{ v4sub r5, r6, r7 ; prefetch_add_l3 r15, 5 }
	{ v4sub r5, r6, r7 ; stnt2_add r15, r16, 5 }
	{ v4sub r5, r6, r7 ; v2cmples r15, r16, r17 }
	{ v4sub r5, r6, r7 ; xori r15, r16, 5 }
	{ v4subsc r15, r16, r17 ; fdouble_addsub r5, r6, r7 }
	{ v4subsc r15, r16, r17 ; mula_ls_lu r5, r6, r7 }
	{ v4subsc r15, r16, r17 ; v1addi r5, r6, 5 }
	{ v4subsc r15, r16, r17 ; v1shru r5, r6, r7 }
	{ v4subsc r15, r16, r17 ; v2shlsc r5, r6, r7 }
	{ v4subsc r5, r6, r7 ; dblalign2 r15, r16, r17 }
	{ v4subsc r5, r6, r7 ; ld4u_add r15, r16, 5 }
	{ v4subsc r5, r6, r7 ; prefetch_l2 r15 }
	{ v4subsc r5, r6, r7 ; sub r15, r16, r17 }
	{ v4subsc r5, r6, r7 ; v2cmpltu r15, r16, r17 }
	{ wh64 r15 ; addx r5, r6, r7 }
	{ wh64 r15 ; fdouble_sub_flags r5, r6, r7 }
	{ wh64 r15 ; mz r5, r6, r7 }
	{ wh64 r15 ; v1cmpeq r5, r6, r7 }
	{ wh64 r15 ; v2add r5, r6, r7 }
	{ wh64 r15 ; v2shrui r5, r6, 5 }
	{ xor r15, r16, r17 ; addi r5, r6, 5 ; ld4s r25, r26 }
	{ xor r15, r16, r17 ; addxi r5, r6, 5 ; ld4u r25, r26 }
	{ xor r15, r16, r17 ; andi r5, r6, 5 ; ld4u r25, r26 }
	{ xor r15, r16, r17 ; cmoveqz r5, r6, r7 ; ld4s r25, r26 }
	{ xor r15, r16, r17 ; cmpeq r5, r6, r7 ; prefetch r25 }
	{ xor r15, r16, r17 ; cmples r5, r6, r7 ; prefetch_l1_fault r25 }
	{ xor r15, r16, r17 ; cmplts r5, r6, r7 ; prefetch_l2_fault r25 }
	{ xor r15, r16, r17 ; cmpltu r5, r6, r7 ; prefetch_l3_fault r25 }
	{ xor r15, r16, r17 ; ctz r5, r6 ; ld4s r25, r26 }
	{ xor r15, r16, r17 ; fnop ; st r25, r26 }
	{ xor r15, r16, r17 ; info 19 ; prefetch_l2 r25 }
	{ xor r15, r16, r17 ; ld r25, r26 ; mula_ls_ls r5, r6, r7 }
	{ xor r15, r16, r17 ; ld1s r25, r26 ; cmoveqz r5, r6, r7 }
	{ xor r15, r16, r17 ; ld1s r25, r26 ; shl2addx r5, r6, r7 }
	{ xor r15, r16, r17 ; ld1u r25, r26 ; mul_hs_hs r5, r6, r7 }
	{ xor r15, r16, r17 ; ld2s r25, r26 ; addi r5, r6, 5 }
	{ xor r15, r16, r17 ; ld2s r25, r26 ; rotl r5, r6, r7 }
	{ xor r15, r16, r17 ; ld2u r25, r26 ; fnop }
	{ xor r15, r16, r17 ; ld2u r25, r26 ; tblidxb1 r5, r6 }
	{ xor r15, r16, r17 ; ld4s r25, r26 ; nop }
	{ xor r15, r16, r17 ; ld4u r25, r26 ; cmpleu r5, r6, r7 }
	{ xor r15, r16, r17 ; ld4u r25, r26 ; shrsi r5, r6, 5 }
	{ xor r15, r16, r17 ; move r5, r6 ; prefetch_l1_fault r25 }
	{ xor r15, r16, r17 ; mul_hs_hs r5, r6, r7 ; prefetch_l2 r25 }
	{ xor r15, r16, r17 ; mul_ls_ls r5, r6, r7 ; prefetch r25 }
	{ xor r15, r16, r17 ; mula_hs_hs r5, r6, r7 ; prefetch_l1 r25 }
	{ xor r15, r16, r17 ; mula_ls_ls r5, r6, r7 ; ld4s r25, r26 }
	{ xor r15, r16, r17 ; mulax r5, r6, r7 ; ld4u r25, r26 }
	{ xor r15, r16, r17 ; mz r5, r6, r7 ; prefetch_l1 r25 }
	{ xor r15, r16, r17 ; nor r5, r6, r7 ; prefetch_l2 r25 }
	{ xor r15, r16, r17 ; pcnt r5, r6 ; prefetch_l2_fault r25 }
	{ xor r15, r16, r17 ; prefetch r25 ; mulax r5, r6, r7 }
	{ xor r15, r16, r17 ; prefetch_l1 r25 ; cmpeq r5, r6, r7 }
	{ xor r15, r16, r17 ; prefetch_l1 r25 ; shl3addx r5, r6, r7 }
	{ xor r15, r16, r17 ; prefetch_l1_fault r25 ; mul_ls_ls r5, r6, r7 }
	{ xor r15, r16, r17 ; prefetch_l2 r25 ; addxi r5, r6, 5 }
	{ xor r15, r16, r17 ; prefetch_l2 r25 ; shl r5, r6, r7 }
	{ xor r15, r16, r17 ; prefetch_l2_fault r25 ; info 19 }
	{ xor r15, r16, r17 ; prefetch_l2_fault r25 ; tblidxb3 r5, r6 }
	{ xor r15, r16, r17 ; prefetch_l3 r25 ; or r5, r6, r7 }
	{ xor r15, r16, r17 ; prefetch_l3_fault r25 ; cmpltsi r5, r6, 5 }
	{ xor r15, r16, r17 ; prefetch_l3_fault r25 ; shrui r5, r6, 5 }
	{ xor r15, r16, r17 ; revbytes r5, r6 ; prefetch_l3 r25 }
	{ xor r15, r16, r17 ; rotli r5, r6, 5 ; st r25, r26 }
	{ xor r15, r16, r17 ; shl1add r5, r6, r7 ; st1 r25, r26 }
	{ xor r15, r16, r17 ; shl2add r5, r6, r7 ; st4 r25, r26 }
	{ xor r15, r16, r17 ; shl3addx r5, r6, r7 ; ld r25, r26 }
	{ xor r15, r16, r17 ; shrs r5, r6, r7 ; ld r25, r26 }
	{ xor r15, r16, r17 ; shru r5, r6, r7 ; ld1u r25, r26 }
	{ xor r15, r16, r17 ; st r25, r26 ; addi r5, r6, 5 }
	{ xor r15, r16, r17 ; st r25, r26 ; rotl r5, r6, r7 }
	{ xor r15, r16, r17 ; st1 r25, r26 ; fnop }
	{ xor r15, r16, r17 ; st1 r25, r26 ; tblidxb1 r5, r6 }
	{ xor r15, r16, r17 ; st2 r25, r26 ; nop }
	{ xor r15, r16, r17 ; st4 r25, r26 ; cmpleu r5, r6, r7 }
	{ xor r15, r16, r17 ; st4 r25, r26 ; shrsi r5, r6, 5 }
	{ xor r15, r16, r17 ; subx r5, r6, r7 ; prefetch_l2 r25 }
	{ xor r15, r16, r17 ; tblidxb1 r5, r6 ; prefetch_l2_fault r25 }
	{ xor r15, r16, r17 ; tblidxb3 r5, r6 ; prefetch_l3_fault r25 }
	{ xor r15, r16, r17 ; v1mz r5, r6, r7 }
	{ xor r15, r16, r17 ; v2packuc r5, r6, r7 }
	{ xor r15, r16, r17 ; xor r5, r6, r7 ; st1 r25, r26 }
	{ xor r5, r6, r7 ; addi r15, r16, 5 ; st2 r25, r26 }
	{ xor r5, r6, r7 ; addxi r15, r16, 5 ; st4 r25, r26 }
	{ xor r5, r6, r7 ; andi r15, r16, 5 ; st4 r25, r26 }
	{ xor r5, r6, r7 ; cmpexch r15, r16, r17 }
	{ xor r5, r6, r7 ; cmplts r15, r16, r17 ; ld r25, r26 }
	{ xor r5, r6, r7 ; cmpltu r15, r16, r17 ; ld1u r25, r26 }
	{ xor r5, r6, r7 ; dtlbpr r15 }
	{ xor r5, r6, r7 ; ill ; ld4u r25, r26 }
	{ xor r5, r6, r7 ; jalr r15 ; ld4s r25, r26 }
	{ xor r5, r6, r7 ; jr r15 ; prefetch r25 }
	{ xor r5, r6, r7 ; ld r25, r26 ; cmples r15, r16, r17 }
	{ xor r5, r6, r7 ; ld1s r25, r26 ; add r15, r16, r17 }
	{ xor r5, r6, r7 ; ld1s r25, r26 ; shrsi r15, r16, 5 }
	{ xor r5, r6, r7 ; ld1u r25, r26 ; shl r15, r16, r17 }
	{ xor r5, r6, r7 ; ld2s r25, r26 ; mnz r15, r16, r17 }
	{ xor r5, r6, r7 ; ld2u r25, r26 ; cmpne r15, r16, r17 }
	{ xor r5, r6, r7 ; ld4s r25, r26 ; and r15, r16, r17 }
	{ xor r5, r6, r7 ; ld4s r25, r26 ; subx r15, r16, r17 }
	{ xor r5, r6, r7 ; ld4u r25, r26 ; shl2addx r15, r16, r17 }
	{ xor r5, r6, r7 ; lnk r15 ; prefetch_l2 r25 }
	{ xor r5, r6, r7 ; move r15, r16 ; prefetch_l2 r25 }
	{ xor r5, r6, r7 ; mz r15, r16, r17 ; prefetch_l2 r25 }
	{ xor r5, r6, r7 ; nor r15, r16, r17 ; prefetch_l3 r25 }
	{ xor r5, r6, r7 ; prefetch r25 ; cmpltu r15, r16, r17 }
	{ xor r5, r6, r7 ; prefetch_add_l3_fault r15, 5 }
	{ xor r5, r6, r7 ; prefetch_l1 r25 ; shli r15, r16, 5 }
	{ xor r5, r6, r7 ; prefetch_l1_fault r25 ; rotli r15, r16, 5 }
	{ xor r5, r6, r7 ; prefetch_l2 r25 ; mnz r15, r16, r17 }
	{ xor r5, r6, r7 ; prefetch_l2_fault r25 ; fnop }
	{ xor r5, r6, r7 ; prefetch_l3 r25 ; cmpeq r15, r16, r17 }
	{ xor r5, r6, r7 ; prefetch_l3 r25 }
	{ xor r5, r6, r7 ; prefetch_l3_fault r25 ; shli r15, r16, 5 }
	{ xor r5, r6, r7 ; rotli r15, r16, 5 ; prefetch_l2_fault r25 }
	{ xor r5, r6, r7 ; shl1add r15, r16, r17 ; prefetch_l3 r25 }
	{ xor r5, r6, r7 ; shl2add r15, r16, r17 ; st r25, r26 }
	{ xor r5, r6, r7 ; shl3add r15, r16, r17 ; st2 r25, r26 }
	{ xor r5, r6, r7 ; shli r15, r16, 5 }
	{ xor r5, r6, r7 ; shrsi r15, r16, 5 }
	{ xor r5, r6, r7 ; shruxi r15, r16, 5 }
	{ xor r5, r6, r7 ; st r25, r26 ; shli r15, r16, 5 }
	{ xor r5, r6, r7 ; st1 r25, r26 ; rotli r15, r16, 5 }
	{ xor r5, r6, r7 ; st2 r25, r26 ; lnk r15 }
	{ xor r5, r6, r7 ; st4 r25, r26 ; cmpltu r15, r16, r17 }
	{ xor r5, r6, r7 ; stnt2 r15, r16 }
	{ xor r5, r6, r7 ; subx r15, r16, r17 ; st2 r25, r26 }
	{ xor r5, r6, r7 ; v2cmpltsi r15, r16, 5 }
	{ xor r5, r6, r7 ; xor r15, r16, r17 ; ld2u r25, r26 }
	{ xori r15, r16, 5 ; cmul r5, r6, r7 }
	{ xori r15, r16, 5 ; mul_hs_lu r5, r6, r7 }
	{ xori r15, r16, 5 ; shrs r5, r6, r7 }
	{ xori r15, r16, 5 ; v1maxu r5, r6, r7 }
	{ xori r15, r16, 5 ; v2minsi r5, r6, 5 }
	{ xori r5, r6, 5 ; addxli r15, r16, 0x1234 }
	{ xori r5, r6, 5 ; jalrp r15 }
	{ xori r5, r6, 5 ; mtspr 0x5, r16 }
	{ xori r5, r6, 5 ; st1 r15, r16 }
	{ xori r5, r6, 5 ; v1shrs r15, r16, r17 }
	{ xori r5, r6, 5 ; v4int_h r15, r16, r17 }
