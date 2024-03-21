#name: Illegal Bfloat16 instructions
#as: -march=armv8.6-a+bf16+sve
#source: illegal-bfloat16.s
#error_output: illegal-bfloat16.l
