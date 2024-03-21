#name: bad MVE FP VMUL instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmul-bad-2.l

.*: +file format .*arm.*
