#name: bad MVE VQDMULH and VQRDMULH instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vqdmulh-bad.l

.*: +file format .*arm.*
