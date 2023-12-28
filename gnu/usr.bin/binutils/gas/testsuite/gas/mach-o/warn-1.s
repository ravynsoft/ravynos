# { dg-do assemble }

	.lcomm align_too_big,9,100
	.comm outofrangealign, 9, 17

# { dg-warning "Alignment \\(100\\) too large: 15 assumed" "" { target *-*-darwin*} 3 }
# { dg-warning "Alignment \\(17\\) too large: 15 assumed" "" { target *-*-darwin*} 4 }
