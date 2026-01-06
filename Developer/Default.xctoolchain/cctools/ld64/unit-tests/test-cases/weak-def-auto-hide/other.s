

			.text
			.align 4
			
			.globl _my_weak
			.weak_def_can_be_hidden _my_weak
_my_weak:	nop
			nop
			
			
				.globl _my_other_weak
				.weak_def_can_be_hidden _my_other_weak
_my_other_weak:	nop
				nop
	
