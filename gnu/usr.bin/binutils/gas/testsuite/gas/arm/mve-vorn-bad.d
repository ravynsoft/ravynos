#name: bad MVE VORN instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vorn-bad.l

.*: +file format .*arm.*
