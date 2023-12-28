#name: bad MVE FP VADD, VSUB and VABD instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vaddsubabd-bad-2.l

.*: +file format .*arm.*

