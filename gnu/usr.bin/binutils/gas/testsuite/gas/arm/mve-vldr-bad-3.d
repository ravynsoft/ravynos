#name: bad MVE VLDR with [R, #imm] addressing mode
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vldr-bad-3.l

.*: +file format .*arm.*
