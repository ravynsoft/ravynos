	.text
test_msa64:
	fill.d	$w17,$18
	copy_s.d	$23,$w24[0]
	copy_s.d	$25,$w26[1]
	copy_u.w	$3,$w4[0]
	copy_u.w	$5,$w6[3]
	insert.d	$w23[0],$24
	insert.d	$w25[1],$26
	dlsa	$25,$26,$27,1
	dlsa	$28,$29,$30,4

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  2
	.space  8
