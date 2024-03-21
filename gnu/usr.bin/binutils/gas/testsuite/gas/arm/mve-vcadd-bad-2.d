#name: bad MVE FP VCADD instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcadd-bad-2.l

.*: +file format .*arm.*
