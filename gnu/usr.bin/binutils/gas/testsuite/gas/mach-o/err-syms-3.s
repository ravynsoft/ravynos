# { dg-do assemble }

	.weak_definition a
a:	.space 1

b:	.space 1
	.weak_definition b

# { dg-error ".a. can.t be a weak_definition .currently only supported in sections of type coalesced." "" { target *-*-darwin*} 4 }
# { dg-error ".b. can.t be a weak_definition .currently only supported in sections of type coalesced." "" { target *-*-darwin*} 7 }
