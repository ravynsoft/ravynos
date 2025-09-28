#name: bad MVE VMUL instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vmul-bad-1.l

.*: +file format .*arm.*
