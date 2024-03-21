#name: bad MVE VCMUL instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vcmul-bad-1.l

.*: +file format .*arm.*
