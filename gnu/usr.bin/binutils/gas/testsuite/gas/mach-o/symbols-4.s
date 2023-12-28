# Reference & dead strip permutations.

	.text
	
	.reference ua
	.lazy_reference ub
	
	.reference ua1
	.private_extern ua1

	.private_extern ub1
	.lazy_reference ub1
	
c:	.space 1
	.reference c

d:	.space 1
	.lazy_reference d

	.reference c1
c1:	.space 1

	.lazy_reference d1
d1:	.space 1

	.private_extern e
	.reference e
	
	.private_extern f
	.lazy_reference f

g:	.space 1
	.private_extern g
	.reference g

h:	.space 1
	.private_extern h
	.lazy_reference h

	.private_extern g1
	.reference g1
g1:	.space 1

	.private_extern h1
	.lazy_reference h1
h1:	.space 1

	.no_dead_strip n
	
	.globl m
	.no_dead_strip m

	.private_extern p
	.no_dead_strip p

n1:	.space 1
	.no_dead_strip n1
	
m1:	.space 1
	.globl m1
	.no_dead_strip m1

p1:	.space 1
	.private_extern p1
	.no_dead_strip p1
