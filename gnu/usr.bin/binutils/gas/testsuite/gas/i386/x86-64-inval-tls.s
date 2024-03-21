	.text
# All the following should be illegal
	kmovq	foo@gottpoff(%rip), %k0
	kmovq	foo@tlsld(%rip), %k0
