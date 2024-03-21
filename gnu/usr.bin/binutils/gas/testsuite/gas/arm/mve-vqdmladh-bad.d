#name: bad MVE VQDMLADH and VQRDMLADH instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vqdmladh-bad.l

.*: +file format .*arm.*
