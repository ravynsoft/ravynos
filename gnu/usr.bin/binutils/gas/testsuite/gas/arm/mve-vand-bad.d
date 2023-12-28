#name: bad MVE VAND instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vand-bad.l

.*: +file format .*arm.*
