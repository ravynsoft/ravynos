#name: bad MVE VLDR with [R, Q] addressing mode
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vldr-bad-1.l

.*: +file format .*arm.*
