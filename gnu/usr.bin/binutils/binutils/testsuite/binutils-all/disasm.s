	.text
	
	.globl start_of_text
start_of_text:
	.type start_of_text, "function"
	.long	1
	.size start_of_text, . - start_of_text

	.globl func
func:
	.type func, "function"
	.long	2
	.global global_non_func_sym
global_non_func_sym:
	.long	3
local_non_func_sym:
	.long	4
	.size func, . - func

	.globl next_func
next_func:	
	.type next_func, "function"
	.long	5
	.size next_func, . - next_func
