#name: bad MVE VCLZ instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vclz-bad.l

.*: +file format .*arm.*
