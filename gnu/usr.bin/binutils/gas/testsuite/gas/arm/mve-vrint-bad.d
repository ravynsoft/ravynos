#name: bad MVE VRINT instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vrint-bad.l

.*: +file format .*arm.*
