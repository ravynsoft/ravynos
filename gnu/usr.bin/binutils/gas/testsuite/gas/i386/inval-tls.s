	.text
# All the following should be illegal
	kmovd	foo@gotntpoff(%eax), %k0
	kmovd	foo@tpoff(%eax), %k0
