#name: bad MVE VMAXNM and VMINNM instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmaxnm-vminnm-bad.l

.*: +file format .*arm.*
