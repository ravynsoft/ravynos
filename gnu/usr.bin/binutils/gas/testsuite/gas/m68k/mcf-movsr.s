.text
|*****************************************************************
| Test all permutations of movew sr and movew ccr
|*****************************************************************
	.global test_movsr
test_movsr:
	move.w	%d3,%sr			| Mode 0
	move.w	#-1,%sr			| Mode 7.4
	move.w	%sr,%d3			| Mode 0

	move.w	%d3,%ccr		| Mode 0
	move.w	#-1,%ccr		| Mode 7.4
	move.w	%ccr,%d3		| Mode 0
