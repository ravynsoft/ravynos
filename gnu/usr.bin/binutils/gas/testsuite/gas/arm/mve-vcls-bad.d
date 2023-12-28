#name: bad MVE VCLS instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vcls-bad.l

.*: +file format .*arm.*
