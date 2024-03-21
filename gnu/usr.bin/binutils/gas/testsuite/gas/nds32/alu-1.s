foo:
	add $r0, $r1, $r2
	and $r0, $r1, $r2
	cmovn $r0, $r1, $r2
	cmovz $r0, $r1, $r2
	nop
	nor $r0, $r1, $r2
	or $r0, $r1, $r2
	rotr $r0, $r1, $r2
	rotri $r0, $r1, 1
	seb $r0, $r1
	seh $r0, $r1
	sll $r0, $r1, $r2
	slli $r0, $r1, 1
	slt $r0, $r1, $r2
	slts $r0, $r1, $r2
	sra $r0, $r1, $r2
	srai $r0, $r1, 1
	srl $r0, $r1, $r2
	srli $r0, $r1, 1
	sub $r0, $r1, $r2
	sva $r0, $r1, $r2
	svs $r0, $r1, $r2
	wsbh $r0, $r1
	xor $r0, $r1, $r2
	zeh $r0, $r1
	divr $r0, $r1, $r2, $r3
	divsr $r0, $r1, $r2, $r3
	add_slli $r0, $r1, $r2, 1
	add_srli $r0, $r1, $r2, 1
	and_slli $r0, $r1, $r2, 1
	and_srli $r0, $r1, $r2, 1
	bitc $r0, $r1, $r2
	or_slli $r0, $r1, $r2, 1
	or_srli $r0, $r1, $r2, 1
	sub_slli $r0, $r1, $r2, 1
	sub_srli $r0, $r1, $r2, 1
	xor_slli $r0, $r1, $r2, 1
	xor_srli $r0, $r1, $r2, 1
