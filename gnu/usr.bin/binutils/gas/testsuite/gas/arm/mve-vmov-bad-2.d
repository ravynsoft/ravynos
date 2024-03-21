#name: bad MVE VMOV (between two 32-bit vector lanes to two general-purpose registers)
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmov-bad-2.l

.*: +file format .*arm.*
