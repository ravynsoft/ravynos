#name: bad MVE VADDV instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vaddv-bad.l

.*: +file format .*arm.*
