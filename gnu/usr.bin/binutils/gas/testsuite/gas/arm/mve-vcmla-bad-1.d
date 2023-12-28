#name: bad MVE VCMLA instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vcmla-bad-1.l

.*: +file format .*arm.*
