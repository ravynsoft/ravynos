#name: bad MVE FP VCMP instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcmp-bad-2.l

.*: +file format .*arm.*
