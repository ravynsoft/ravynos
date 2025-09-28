# Tests for mixing pseudo-C and normal eBPF instructions
beg:
        .text
	add %r1,0xaa
	r1 += 0xaa
	add %r1,%r2
	r1 += r2
	sub %r1,0xaa
	r1 -= 0xaa
	sub %r1,%r2
	r1 -= r2
	mul %r1,0xaa
	r1 *= 0xaa
	mul %r1,%r2
	r1 *= r2
	div %r1,0xaa
	r1 /= 0xaa
	div %r1,%r2
	r1 /= r2
	or %r1,0xaa
	r1 |= 0xaa
	or %r1,%r2
	r1 |= r2
	and %r1,0xaa
	r1 &= 0xaa
	and %r1,%r2
	r1 &= r2
	lsh %r1,0xaa
	r1 <<= 0xaa
	lsh %r1,%r2
	r1 <<= r2
	rsh %r1,0xaa
	r1 >>= 0xaa
	rsh %r1,%r2
	r1 >>= r2
	xor %r1,0xaa
	r1 ^= 0xaa
	xor %r1,%r2
	r1 ^= r2
	mov %r1,0xaa
	r1 = 0xaa
	mov %r1,%r2
	r1 = r2
	arsh %r1,0xaa
	r1 s>>= 0xaa
	arsh %r1,%r2
	r1 s>>= r2
	neg %r1
	r1 = -r1
	add32 %r1,0xaa
	w1 += 0xaa
	add32 %r1,%r2
	w1 += w2
	sub32 %r1,0xaa
	w1 -= 0xaa
	sub32 %r1,%r2
	w1 -= w2
	mul32 %r1,0xaa
	w1 *= 0xaa
	mul32 %r1,%r2
	w1 *= w2
	div32 %r1,0xaa
	w1 /= 0xaa
	div32 %r1,%r2
	w1 /= w2
	or32 %r1,0xaa
	w1 |= 0xaa
	or32 %r1,%r2
	w1 |= w2
	and32 %r1,0xaa
	w1 &= 0xaa
	and32 %r1,%r2
	w1 &= w2
	lsh32 %r1,0xaa
	w1 <<= 0xaa
	lsh32 %r1,%r2
	w1 <<= w2
	rsh32 %r1,0xaa
	w1 >>= 0xaa
	rsh32 %r1,%r2
	w1 >>= w2
	xor32 %r1,0xaa
	w1 ^= 0xaa
	xor32 %r1,%r2
	w1 ^= w2
	mov32 %r1,0xaa
	w1 = 0xaa
	mov32 %r1,%r2
	w1 = w2
	arsh32 %r1,0xaa
	w1 s>>= 0xaa
	arsh32 %r1,%r2
	w1 s>>= w2
	neg32 %r1
	w1 = -w1
	endle %r1,16
	r1 = le16 r1
	endle %r1,32
	r1 = le32 r1
	endle %r1,64
	r1 = le64 r1
	endbe %r1,16
	r1 = be16 r1
	endbe %r1,32
	r1 = be32 r1
	endbe %r1,64
	r1 = be64 r1
	ldxb %r1,[%r2+0xaa]
	r1 = *(u8 *)(r2 + 0xaa)
	ldxh %r1,[%r2+0xaa]
	r1 = *(u16 *)(r2 + 0xaa)
	ldxw %r1,[%r2+0xaa]
	r1 = *(u32 *)(r2 + 0xaa)
	ldxdw %r1,[%r2+0xaa]
	r1 = *(u64 *)(r2 + 0xaa)
	stxb [%r1+0xaa],%r2
	*(u8 *)(r1 + 0xaa) = r2
	stxh [%r1+0xaa],%r2
	*(u16 *)(r1 + 0xaa) = r2
	stxw [%r1+0xaa],%r2
	*(u32 *)(r1 + 0xaa) = r2
	stxdw [%r1+0xaa],%r2
	*(u64 *)(r1 + 0xaa) = r2
	ja 187
	goto 0xbb
	jeq %r1,0xaa,187
	if r1 == 0xaa goto 0xbb
	jeq %r1,%r2,187
	if r1 == r2 goto 0xbb
	jgt %r1,0xaa,187
	if r1 > 0xaa goto 0xbb
	jgt %r1,%r2,187
	if r1 > r2 goto 0xbb
	jge %r1,0xaa,187
	if r1 >= 0xaa goto 0xbb
	jge %r1,%r2,187
	if r1 >= r2 goto 0xbb
	jlt %r1,0xaa,187
	if r1 < 0xaa goto 0xbb
	jlt %r1,%r2,187
	if r1 < r2 goto 0xbb
	jle %r1,0xaa,187
	if r1 <= 0xaa goto 0xbb
	jle %r1,%r2,187
	if r1 <= r2 goto 0xbb
	jset %r1,0xaa,187
	if r1 & 0xaa goto 0xbb
	jset %r1,%r2,187
	if r1 & r2 goto 0xbb
	jne %r1,0xaa,187
	if r1 != 0xaa goto 0xbb
	jne %r1,%r2,187
	if r1 != r2 goto 0xbb
	jsgt %r1,0xaa,187
	if r1 s> 0xaa goto 0xbb
	jsgt %r1,%r2,187
	if r1 s> r2 goto 0xbb
	jsge %r1,0xaa,187
	if r1 s>= 0xaa goto 0xbb
	jsge %r1,%r2,187
	if r1 s>= r2 goto 0xbb
	jslt %r1,0xaa,187
	if r1 s< 0xaa goto 0xbb
	jslt %r1,%r2,187
	if r1 s< r2 goto 0xbb
	jsle %r1,0xaa,187
	if r1 s<= 0xaa goto 0xbb
	jsle %r1,%r2,187
	if r1 s<= r2 goto 0xbb
	call 170
	call 0xaa
	exit
	exit
	mov %r6,main - beg
        exit
	ldabsw 0xaa
	r0 = *(u32 *)skb[0xaa]
	ldindb %r7,0xaa
	r0 = *(u8 *)skb[r7 + 0xaa]
	ldabsw 0xaa
	r0 = *(u32 *)skb[0xaa]
	ldindb %r7,0xaa
	r0 = *(u8 *)skb[r7 + 0xaa]
	lddw %r3,1
	r3 =  1 ll
	lddw %r4,0xaabbccddeeff7788
	r4 =  0xaabbccddeeff7788 ll
	r5 =  0x1122334455667788 ll
	lddw %r5,0x1122334455667788
	lddw %r6,main
	r6 = main ll
	main:
	lock *(u32 *)(r1 + 0xaa) += r2
	xaddw [%r1+0xaa],%r2
	lock *(u64 *)(r1 + 0xaa) += r2
	xadddw [%r1+0xaa],%r2
