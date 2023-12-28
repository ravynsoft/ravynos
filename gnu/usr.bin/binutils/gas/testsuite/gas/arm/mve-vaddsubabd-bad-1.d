#name: bad MVE VADD, VSUB and VABD instructions
#as: -march=armv8.1-m.main+mve
#error_output: mve-vaddsubabd-bad-1.l

.*: +file format .*arm.*
