	.text
	.align 5
_start:
L0:
	ldw .d1t1	*a0,a1
	ldw .d1t1	*a0,a1
	ldw .d1t1	*a0,a1
L1:
	MVC .s2    PCE1, b0
	ldw .d1t1	*a0,a2
	mvk .s2		$PCR_OFFSET (S0,L1), b2
	mvkh .s2	$PCR_OFFSET (S0,L1), b2
	mvk .s2		$PCR_OFFSET (S0,L2), b2
	mvkh .s2	$PCR_OFFSET (S0,L2), b2
	mvk .s2		$PCR_OFFSET (S1,L1), b2
	mvkh .s2	$PCR_OFFSET (S1,L1), b2
	mvk .s2		$PCR_OFFSET (S1,L2), b2
	mvkh .s2	$PCR_OFFSET (S1,L2), b2

S0:
	ldw .d1t1	*a0,a1
L2:
	MVC .s2    PCE1, b0

S1:
	ldw .d1t1	*a0,a1
	mvkl .s2	$PCR_OFFSET (L0,L2), b2
	mvkh .s2	$PCR_OFFSET (L0,L2), b2
