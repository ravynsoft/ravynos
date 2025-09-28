#name: Invalid armv8.2-a scalar fp16
#source: armv8-2-fp16-scalar-bad.s
#as: -march=armv8.2-a+fp16 -mwarn-restrict-it
#error_output: armv8-2-fp16-scalar-bad.l
