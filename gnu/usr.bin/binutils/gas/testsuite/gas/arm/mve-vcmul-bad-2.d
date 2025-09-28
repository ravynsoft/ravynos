#name: bad MVE FP VCMUL instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcmul-bad-2.l

.*: +file format .*arm.*
