#name: bad MVE FP VCMLA instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcmla-bad-2.l

.*: +file format .*arm.*
