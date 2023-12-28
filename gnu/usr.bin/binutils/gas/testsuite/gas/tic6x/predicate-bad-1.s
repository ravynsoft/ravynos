# Test bad syntax for predicates.
.text
.globl f
f:
	[not a predicate
	[A1] ; no instruction
	[unknown] nop
	[a3] nop
	[b500] nop
	[] nop
	[!] nop
	[!a] nop
	[!A] nop
	[!b] nop
	[!B] nop
	[!x] nop
	[a] nop
	[B] nop
	[a1] [!B1] nop
	[A2] .word 0
	[!B2] label:
	[!A1] .word 1
