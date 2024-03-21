#name: bad MVE VQABS and VQNEG instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vqabsneg-bad.l

.*: +file format .*arm.*
