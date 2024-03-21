#name: bad MVE VEOR instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-veor-bad.l

.*: +file format .*arm.*
