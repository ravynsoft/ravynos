# { dg-do assemble }

	.globl a
a:	.space 1
	.local a

	.private_extern b
b:	.space 1
	.local b

# { dg-error ".a. previously declared as .global." "" { target *-*-darwin*} 5 }
# { dg-error ".b. previously declared as .private extern." "" { target *-*-darwin*} 9 }
