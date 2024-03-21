# { dg-do assemble }

	.section __weak, __weak, coalesced

a:	.space 1
	.weak_definition a

	.weak_definition b
b:	.space 1
	
	.weak_definition c

# { dg-error "Non-global symbol: .a. can.t be a weak_definition." "" { target *-*-darwin*} 0 }
# { dg-error "Non-global symbol: .b. can.t be a weak_definition." "" { target *-*-darwin*} 0 }
# { dg-error ".c. can.t be a weak_definition .since it is undefined." "" { target *-*-darwin*} 0 }
