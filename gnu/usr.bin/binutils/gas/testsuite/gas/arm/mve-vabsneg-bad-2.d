#name: bad MVE FP VABS and VNEG instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vabsneg-bad-2.l

.*: +file format .*arm.*

