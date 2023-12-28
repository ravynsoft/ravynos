#name: bad MVE VMOV (between general-purpose register and vector lane)
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmov-bad-1.l

.*: +file format .*arm.*
