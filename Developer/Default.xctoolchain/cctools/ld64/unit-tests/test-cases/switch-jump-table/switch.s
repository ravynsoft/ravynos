
	.section __TEXT,__textcoal_nt,coalesced,pure_instructions



/* 
	Simulate a switch statement in a weak function compiled
	to a jump table
*/
	.globl _foo
	.weak_definition _foo
	.align 4
_foo:
	nop
	nop
#if __arm__ || __i386__
	.long	L1
	.long	L2
	.long	L3
#endif
	nop
L1: nop
L2: nop
L3: nop
	nop
	
	
/* 
	Simulate a switch statement in a regular function compiled
	to a jump table
*/
	.text
	.align 4
	.globl _bar
_bar: nop
	nop
	nop
	nop
#if __arm__ || __i386__
	.long	L5
	.long	L6
	.long	L7
#endif
	nop
L5: nop
L6: nop
L7: nop
	nop
	
	
	
