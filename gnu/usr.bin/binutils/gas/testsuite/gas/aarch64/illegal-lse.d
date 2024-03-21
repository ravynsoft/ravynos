#name: LSE Illegal Instruction Operands
#source: illegal-lse.s
#as: -march=armv8-a+lse -mno-verbose-error
#error_output: illegal-lse.l
