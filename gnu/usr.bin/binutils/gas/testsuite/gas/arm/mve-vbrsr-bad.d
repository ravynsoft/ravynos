#name: bad MVE VBRSR instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vbrsr-bad.l

.*: +file format .*arm.*
