	.section .text
	.globl	_text_pointer
	.section	.data.rel
	.type	_text_pointer, @object
_text_pointer:
	.long	_text_address@c
	.section	.text
	.globl	_main
	.type	_main, @function
_main:
	######################
	# Data symbol with GOT
	######################
	loadd 	_text_pointer@GOT(r12), (r1,r0)
	######################
	# Code symbol with cGOT
	######################
	loadd 	_text_address_1@cGOT(r12), (r1,r0)

	.globl	_text_address_1
	.type	_text_address_1, @function
_text_address_1:
	jump	(ra)
