#name: bad MVE VPSEL instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vpsel-bad.l

.*: +file format .*arm.*
