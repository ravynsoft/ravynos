#name: bad MVE VMAXNMA and VMINNMA instructions
#as: -march=armv8.1-m.main+mve.fp
#error_output: mve-vmaxnma-vminnma-bad.l

.*: +file format .*arm.*
