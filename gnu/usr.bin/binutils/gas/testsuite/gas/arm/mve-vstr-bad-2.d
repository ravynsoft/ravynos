#name: bad MVE VSTR with [Q, #imm] addressing mode
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vstr-bad-2.l

.*: +file format .*arm.*
