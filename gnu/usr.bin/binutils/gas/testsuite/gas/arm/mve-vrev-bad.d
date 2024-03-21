#name: bad MVE VREV16, VREV32 and VREV64 instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vrev-bad.l

.*: +file format .*arm.*
