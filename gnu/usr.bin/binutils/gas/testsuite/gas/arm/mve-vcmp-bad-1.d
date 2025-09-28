#name: bad MVE VCMP instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vcmp-bad-1.l

.*: +file format .*arm.*
