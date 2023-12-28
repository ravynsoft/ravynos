#name: bad MVE VLDR with [Q, #imm] addressing mode
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vldr-bad-2.l

.*: +file format .*arm.*
